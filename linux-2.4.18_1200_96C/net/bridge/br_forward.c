/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.11.4.6 2011/05/13 03:33:55 keith_huang Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/if_bridge.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#ifdef IGMP_SNOOPING
#include <linux/ip.h>
#include <linux/in.h>
#endif

static inline int should_deliver(struct net_bridge_port *p, struct sk_buff *skb)
{
	if (skb->dev == p->dev ||
	    p->state != BR_STATE_FORWARDING)
		return 0;

#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
	if (skb->src_info) {
		struct vlan_info_item *pitem = search_vlan_info(p->dev);

		/* clone packet (dev has been change to eth5) can send to vlan_nat */
		if (skb->src_info->idx) {
			if (pitem && pitem->info.forwarding!=1)
				return 0;
		}

		/* vlan_br can't send packet to vlan_nat */
		if (skb->src_info->forwarding==1) {
			if (pitem && pitem->info.forwarding==2)
				return 0;
		}

		/* vlan_nat can't send packet to vlan_br */
		if (skb->src_info->forwarding==2) {
			if (pitem && pitem->info.forwarding==1)
				return 0;
		}
	}
#endif

#ifdef CONFIG_RTK_GUEST_ZONE	
	if (skb->dev && skb->dev->br_port && p->br) {
		// skb to/from different zone case, check zone isolation
		if ((skb->dev->br_port->is_guest_zone != p->is_guest_zone) && p->br->is_zone_isolated) {
#ifdef DEBUG_GUEST_ZONE			
			panic_printk("zone block!\n");		
#endif
			return 0;
		}
	
		// skb to/from guest zone case, check guest isolation
		if (skb->dev->br_port->is_guest_zone && p->is_guest_zone && p->br->is_guest_isolated) {
#ifdef DEBUG_GUEST_ZONE			
			panic_printk("guest block!\n");		
#endif
			return 0;
		}
	}
#endif // CONFIG_RTK_GUEST_ZONE

#ifdef MULTICAST_BWCTRL
	if (p->bandwidth != 0)
	{
		if ((p->accumulation + skb->len) > p->bandwidth)
			return 0;
		p->accumulation += skb->len;
	}
#endif

#ifdef MULTICAST_FILTER
	if ((p->br->fltr_portlist_num) &&
		!memcmp(skb->mac.ethernet->h_dest, "\x01\x00\x5e", 3))
	{
		int i, filter = 0;
		unsigned short frag_offset = *((unsigned short *)&(skb->data[6]));
		unsigned short port = *((unsigned short *)&(skb->data[22]));
		unsigned long x;

		if ((frag_offset & 0x1fff) == 0) {	// check fragment offset
			for (i=0; i<p->br->fltr_portlist_num; i++) {
				if (port == p->br->fltr_portlist[i]) {
					filter = 1;
					break;
				}
			}
		}

		x = skb->mac.ethernet->h_dest[3] ^ skb->mac.ethernet->h_dest[4] ^ skb->mac.ethernet->h_dest[5];
		x = x & (MLCST_MAC_ENTRY - 1);

		if (!strcmp(p->dev->name, "wlan0") ||
			!strcmp(p->dev->name, "wlan1") ||
			!strcmp(p->dev->name, "wlan2"))
		{
			if (filter) {
				if (p->br->fltr_maclist[x][3] == 0) {
					memcpy(&(p->br->fltr_maclist[x][0]), &(skb->mac.ethernet->h_dest[3]), 3);
					p->br->fltr_maclist[x][3] = 1;
				}
				return 0;
			}
			else {
				if ((p->br->fltr_maclist[x][3] != 0) &&
					!memcmp(&(p->br->fltr_maclist[x][0]), &(skb->mac.ethernet->h_dest[3]), 3))
					return 0;
				else
					return 1;
			}
		}
		else
			return 1;
	}
	else
		return 1;
#else
	return 1;
#endif
}

static int __dev_queue_push_xmit(struct sk_buff *skb)
{
	skb_push(skb, ETH_HLEN);
	dev_queue_xmit(skb);

	return 0;
}

static int __br_forward_finish(struct sk_buff *skb)
{
#if 0
	if (!(list_empty(&nf_hooks[(PF_BRIDGE)][(NF_BR_POST_ROUTING)])))
		printk("netfilter not empty in (PF_BRIDGE)][(NF_BR_POST_ROUTING\n");	
#endif

	NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
			__dev_queue_push_xmit);

	return 0;
}

static void __br_deliver(struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
#if 0	
	if (!(list_empty(&nf_hooks[(PF_BRIDGE)][(NF_BR_LOCAL_OUT)])))
		                  printk("netfilter not empty in (PF_BRIDGE)][(NF_BR_POST_ROUTING\n");
#endif

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, indev, skb->dev,
			__br_forward_finish);
}

static void __br_forward(struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
#if 0	
	if (!(list_empty(&nf_hooks[(PF_BRIDGE)][(NF_BR_FORWARD)])))
	printk("netfilter not empty in (PF_BRIDGE)][(NF_BR_POST_ROUTING\n");
#endif

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			__br_forward_finish);
}

/* called under bridge lock */
void br_deliver(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
void br_forward(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(struct net_bridge_port *p, struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	if (clone) {
		struct sk_buff *skb2;
		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}

	prev = NULL;

	p = br->port_list;
	while (p != NULL) {
		if (should_deliver(p, skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;
				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}

		p = p->next;
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
#ifdef IGMP_SNOOPING
inline int bitmask_to_id(unsigned char val)
{
	int i;
	for (i=0; i<8; i++) {
		if (val & (1 <<i))
			break;
	}
	return (i);
}

static void br_multicast(struct net_bridge *br, struct net_bridge_fdb_entry *dst, struct sk_buff *skb, int clone, 
		  void (*__packet_hook)(struct net_bridge_port *p, struct sk_buff *skb))
{
//	char i;
	struct net_bridge_port *prev;
	struct net_bridge_port *p;
	unsigned short port_bitmask=0;
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
	int come_from_same_eth_vlan = 0;
#endif	

	if (clone) {
		struct sk_buff *skb2;                                                                                                                                              
		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}                                                                                                                                              
		skb = skb2;
	}
                                                                                                                                              
	prev = NULL;
	p = br->port_list;
	while (p != NULL) {
        port_bitmask = 0;
        port_bitmask = (1 << p->port_no);

#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
		if ((skb->cb[4] == dst->vlan_id) && memcmp(skb->dev->name, "wlan", 4)) 
			come_from_same_eth_vlan = 1;

#ifdef CONFIG_RTL8196C_AP_ROOT
		come_from_same_eth_vlan = 0;
#elif defined(CONFIG_RTL865X_SUPPORT_IPV6_MLD) || defined(_ARSWITCH_) || defined(_SOFT_ARSWITCH_)
		extern char gSwNatSetting[16];
//panic_printk("\r\n gSwNatSetting=[%s],__[%s-%u]\r\n",gSwNatSetting,__FILE__,__LINE__);		
		if (gSwNatSetting[0] == '1')
		{
//panic_printk("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);			
			come_from_same_eth_vlan = 0;
		}
#endif
		
		if (come_from_same_eth_vlan ||		
			((port_bitmask & dst->group_src) && should_deliver(p, skb))) 
#else
		if ((port_bitmask & dst->group_src) && should_deliver(p, skb)) 
#endif			
		{			
			if (prev != NULL) {
				struct sk_buff *skb2;
                                                                                                                                 
				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
						br->statistics.tx_dropped++;
						kfree_skb(skb);
						return;
				}

				skb2->cb[0] = dst->portlist & 0x7f;
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
				if (come_from_same_eth_vlan)		
					skb2->cb[0] &= ~skb->cb[3]; // unmask come from port
#endif				
				__packet_hook(prev, skb2);
			}                                                                                                                    
			prev = p;
		}
		p = p->next;
	}

	if (prev != NULL) {		
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
		if (dst->portlist && !(dst->portlist & 0x80)) { // no wlan client joined
			struct iphdr *	iph =  skb->nh.iph;			
			int index;		
			if(!dst->ipv6_multicast){
			index = add_ip_multicast_tbl(iph->saddr, iph->daddr, 
					(int)skb->cb[4], bitmask_to_id(skb->cb[3]), dst->portlist & 0x7f);
			}
			if (index >= 0) 
				dst->index = index;	

//			DEBUG_PRINT("saddr=%u.%u.%u.%u, daddr=%u.%u.%u.%u, src_vlan=%d, src_port=%x, dst_port=%x, idx=%d\n", 
//					NIPQUAD(iph->saddr), NIPQUAD(iph->daddr), (int)skb->cb[4], bitmask_to_id(skb->cb[3]), dst->portlist&0x7f, index);
		}
#endif
		skb->cb[0] = dst->portlist & 0x7f;

#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
		if (come_from_same_eth_vlan)		
			skb->cb[0] &= ~skb->cb[3]; // unmask come-from port
#endif				

		__packet_hook(prev, skb);
		return;
	}
	
	kfree_skb(skb);
}

void br_multicast_deliver(struct net_bridge *br, struct net_bridge_fdb_entry *dst, struct sk_buff *skb, int clone)
{
	br_multicast(br, dst, skb, clone, __br_deliver);
}
void br_multicast_forward(struct net_bridge *br, struct net_bridge_fdb_entry *dst, struct sk_buff *skb, int clone)
{
	br_multicast(br, dst, skb, clone, __br_forward);
}

#endif
