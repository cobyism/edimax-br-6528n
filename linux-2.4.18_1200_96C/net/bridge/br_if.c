/*
 *	Userspace interface
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_if.c,v 1.12.4.3 2010/12/20 02:15:32 keith_huang Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/if_bridge.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>
#include <asm/uaccess.h>
#include "br_private.h"

static struct net_bridge *bridge_list;
#ifndef STP_ADDCOST_ETH
static 
#endif
int br_initial_port_cost(struct net_device *dev)
{
	if (!strncmp(dev->name, "lec", 3))
		return 7;

	if (!strncmp(dev->name, "eth", 3))
		return 100;			/* FIXME handle 100Mbps */

	if (!strncmp(dev->name, "plip", 4))
		return 2500;

	return 100;
}


/* called under bridge lock */
static int __br_del_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	struct net_bridge_port **pptr;

	if ((p = dev->br_port) == NULL)
		return -EINVAL;

#if 0
#ifdef STP_DISABLE_ETH
	//Chris: stp+mesh
	p->disable_by_mesh = 0;
#endif
#endif 
	br_stp_disable_port(p);

	dev_set_promiscuity(dev, -1);
	dev->br_port = NULL;

	pptr = &br->port_list;
	while (*pptr != NULL) {
		if (*pptr == p) {
			*pptr = p->next;
			break;
		}

		pptr = &((*pptr)->next);
	}

	br_fdb_delete_by_port(br, p);
	kfree(p);
	dev_put(dev);

	return 0;
}

static struct net_bridge **__find_br(char *name)
{
	struct net_bridge **b;
	struct net_bridge *br;

	b = &bridge_list;
	while ((br = *b) != NULL) {
		if (!strncmp(br->dev.name, name, IFNAMSIZ))
			return b;

		b = &(br->next);
	}

	return NULL;
}

// for video streaming refine
#if defined(CONFIG_RTK_MESH) || defined(IGMP_SNOOPING)
struct net_bridge *find_br_by_name(char *name)
{
	struct net_bridge **b;
	b = __find_br(name);

	return *b;
}
#endif

static void del_ifs(struct net_bridge *br)
{
	write_lock_bh(&br->lock);
	while (br->port_list != NULL)
		__br_del_if(br, br->port_list->dev);
	write_unlock_bh(&br->lock);
}

static struct net_bridge *new_nb(char *name)
{
	struct net_bridge *br;
	struct net_device *dev;

	if ((br = kmalloc(sizeof(*br), GFP_KERNEL)) == NULL)
		return NULL;

	memset(br, 0, sizeof(*br));
	dev = &br->dev;

	strncpy(dev->name, name, IFNAMSIZ);
	dev->priv = br;
	ether_setup(dev);
	br_dev_setup(dev);

	br->lock = RW_LOCK_UNLOCKED;
	br->hash_lock = RW_LOCK_UNLOCKED;

	br->bridge_id.prio[0] = 0x80;
	br->bridge_id.prio[1] = 0x00;
	memset(br->bridge_id.addr, 0, ETH_ALEN);

	br->stp_enabled = 1;
	br->designated_root = br->bridge_id;
	br->root_path_cost = 0;
	br->root_port = 0;
	br->bridge_max_age = br->max_age = 20 * HZ;
	br->bridge_hello_time = br->hello_time = 2 * HZ;
	br->bridge_forward_delay = br->forward_delay = 15 * HZ;
	br->topology_change = 0;
	br->topology_change_detected = 0;
	br_timer_clear(&br->hello_timer);
	br_timer_clear(&br->tcn_timer);
	br_timer_clear(&br->topology_change_timer);

	br->ageing_time = 300 * HZ;
// 0407-2009
#if defined(IGMP_SNOOPING) 
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
//	br->igmp_ageing_time = 35 * HZ;
//	DEBUG_PRINT("bridge igmp_ageing_time=%d\n\n",br->igmp_ageing_time);
#endif
#endif
	br->gc_interval = 4 * HZ;

#ifdef CONFIG_RTK_MESH

	br->eth0_monitor_interval = MESH_PORTAL_EXPIRE * HZ;

#ifdef STP_ADDCOST_ETH
	br->is_cost_changed = 0;
#endif
#endif

#ifdef MULTICAST_FILTER
	br->fltr_portlist_num = 0;
	memset(&(br->fltr_maclist[0][0]), 0, sizeof(br->fltr_maclist));
#endif

	return br;
}

/* called under bridge lock */
static struct net_bridge_port *new_nbp(struct net_bridge *br, struct net_device *dev)
{
	int i;
	struct net_bridge_port *p;

	p = kmalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL)
		return p;

	memset(p, 0, sizeof(*p));
	p->br = br;
	p->dev = dev;
	p->path_cost = br_initial_port_cost(dev);
	p->priority = 0x80;


//Chris: stp+mesh
#if 0
 #ifdef STP_DISABLE_ETH
	p->disable_by_mesh = 0;
 #endif
#endif


#ifdef RTL_BRIDGE_MAC_CLONE
	p->enable_mac_clone = 0;
	p->mac_clone_completed = 0;
	p->macCloneTargetPort = NULL;
#endif

	dev->br_port = p;

	for (i=1;i<255;i++)
		if (br_get_port(br, i) == NULL)
			break;

	if (i == 255) {
		kfree(p);
		return NULL;
	}

	p->port_no = i;
	br_init_port(p);
	p->state = BR_STATE_DISABLED;

	p->next = br->port_list;
	br->port_list = p;

	return p;
}

#ifdef	CONFIG_RTK_MESH
int br_set_meshpathsel_pid(int pid)
{
	struct net_bridge **b;
	struct net_bridge *br;
	
	if ((b = __find_br("br0")) == NULL)
		return -ENXIO;
	
	br = *b;

	br->mesh_pathsel_pid = pid;

	if( !pid )
		br->eth0_received = 0;

	//panic_printk("Receive Pathsel daemon pid:%d\n",br->mesh_pathsel_pid);
	return 1;
}
#endif

int br_add_bridge(char *name)
{
	struct net_bridge *br;

	if ((br = new_nb(name)) == NULL)
		return -ENOMEM;

	if (__dev_get_by_name(name) != NULL) {
		kfree(br);
		return -EEXIST;
	}

	br->next = bridge_list;
	bridge_list = br;

	br_inc_use_count();
	register_netdev(&br->dev);

	return 0;
}

int br_del_bridge(char *name)
{
	struct net_bridge **b;
	struct net_bridge *br;

	if ((b = __find_br(name)) == NULL)
		return -ENXIO;

	br = *b;

	if (br->dev.flags & IFF_UP)
		return -EBUSY;

	del_ifs(br);

	*b = br->next;

	unregister_netdev(&br->dev);
	kfree(br);
	br_dec_use_count();

	return 0;
}

int br_add_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;

	if (dev->br_port != NULL)
		return -EBUSY;

	if (dev->flags & IFF_LOOPBACK || dev->type != ARPHRD_ETHER)
		return -EINVAL;

	if (dev->hard_start_xmit == br_dev_xmit)
		return -ELOOP;

	dev_hold(dev);
	write_lock_bh(&br->lock);
	if ((p = new_nbp(br, dev)) == NULL) {
		write_unlock_bh(&br->lock);
		dev_put(dev);
		return -EXFULL;
	}

	dev_set_promiscuity(dev, 1);

	br_stp_recalculate_bridge_id(br);
	br_fdb_insert(br, p, dev->dev_addr, 1);
	if ((br->dev.flags & IFF_UP) && (dev->flags & IFF_UP))
		br_stp_enable_port(p);
	write_unlock_bh(&br->lock);

	return 0;
}

int br_del_if(struct net_bridge *br, struct net_device *dev)
{
	int retval;

	write_lock_bh(&br->lock);
	retval = __br_del_if(br, dev);
	br_stp_recalculate_bridge_id(br);
	write_unlock_bh(&br->lock);

	return retval;
}

int br_get_bridge_ifindices(int *indices, int num)
{
	struct net_bridge *br;
	int i;

	br = bridge_list;
	for (i=0;i<num;i++) {
		if (br == NULL)
			break;

		indices[i] = br->dev.ifindex;
		br = br->next;
	}

	return i;
}

/* called under ioctl_lock */
void br_get_port_ifindices(struct net_bridge *br, int *ifindices)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		ifindices[p->port_no] = p->dev->ifindex;
		p = p->next;
	}
}

#ifdef RTL_BRIDGE_MAC_CLONE
struct mac_clone_pair		clone_pair;

int br_mac_clone(struct net_bridge_port *p, unsigned char *addr)
{
	struct sockaddr sa;

	TRACE("MAC Clone: Addr=%02x-%02x-%02x-%02x-%02x-%02x, to Port=%s\n",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
		(p->dev == NULL)? "NULL" : p->dev->name);

	memcpy(sa.sa_data, addr, 6);

	if (p->dev != NULL) {
		p->dev->set_mac_address(p->dev, &sa);
		return 0;
	}
	else
		return -1;
}

int br_mac_clone_handle_frame(struct net_bridge *br,
			struct net_bridge_port *p,
			struct sk_buff *skb)
{
	if ((br == NULL) || (p == NULL) || (skb == NULL))
		return -1;

	if (p->enable_mac_clone && !p->mac_clone_completed)
	{
		if (!(skb->mac.ethernet->h_source[0] & 1))	//// check any other particular MAC add
		{
			if (p->macCloneTargetPort != NULL)
			{
				if (memcmp(skb->mac.ethernet->h_source, clone_pair.mac.addr, ETH_ALEN))
				{
					TRACE("Clone MAC from %s\n", p->dev->name);
					br_mac_clone(p->macCloneTargetPort, skb->mac.ethernet->h_source);
					memcpy(clone_pair.mac.addr, skb->mac.ethernet->h_source, ETH_ALEN);
				}
				p->mac_clone_completed = 1;
				p->macClone_expire_to = MAC_CLONE_TIMEOUT;
				br_timer_set(&p->macClone_timer, jiffies);
			}
		}
	}
	else if (p->enable_mac_clone && p->mac_clone_completed)
	{
		if (!memcmp(skb->mac.ethernet->h_source, clone_pair.mac.addr, ETH_ALEN))
			p->macClone_expire_to = MAC_CLONE_TIMEOUT;
	}

	return 0;
}

void br_mac_clone_unlink(struct net_bridge_port *port)
{
	struct net_bridge *br;
	struct net_bridge_port *p;

	if ((port == NULL) || (port->br == NULL))
		return;

	br = port->br;
	p = br->port_list;
	while (p != NULL) {

		if ((p->enable_mac_clone) && (p->macCloneTargetPort == port)) {
			TRACE("Disable MAC Clone: Port=%s, to Port=%s\n",
				p->dev->name,
				port->dev->name);

			p->enable_mac_clone = 0;
			p->mac_clone_completed = 0;
			p->macCloneTargetPort = NULL;
		}

		p = p->next;
	}
}
#endif	// RTL_BRIDGE_MAC_CLONE

