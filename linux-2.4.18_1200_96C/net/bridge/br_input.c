/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_input.c,v 1.43.2.14 2011/04/26 12:22:24 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"

// for fast-nat module, 2005-12-23
#ifdef NAT_SPEEDUP		
static int br_nat_speedup=1;
#endif
#ifdef IGMP_SNOOPING
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/igmp.h>
#include <net/checksum.h>
extern int igmpsnoopenabled;

#ifdef	MCAST_TO_UNICAST
extern int IGMPProxyOpened;

#ifdef	IPV6_MCAST_TO_UNICAST
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/icmpv6.h>
//#define	DBG_ICMPv6	//enable it to debug icmpv6 check
static int ICMPv6_check(struct sk_buff *skb , unsigned char *gmac);
#endif	//end of IPV6_MCAST_TO_UNICAST

#endif	//end of MCAST_TO_UNICAST

#ifdef CONFIG_RTL_KERNEL_MIPS16_BRIDGE
#include <asm/mips16_lib.h>
#endif


#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
#include <net/udp.h>
#endif


static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac);
static void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *gmac 
									,struct sk_buff *skb);
#endif	//end of IGMP_SNOOPING

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
extern int br_filter_enter(struct sk_buff *skb);
extern unsigned char dut_br0_mac[];
extern unsigned char Filter_State;
extern int enable_filter;
#endif

unsigned char bridge_ula[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
static int br_pass_frame_up_finish(struct sk_buff *skb)
{
	netif_rx(skb);

	return 0;
}

#ifdef CONFIG_RTK_MESH

void br_signal_pathsel(struct net_bridge *br)
{
	struct task_struct *task;
	read_lock(&tasklist_lock);
    task = find_task_by_pid(br->mesh_pathsel_pid);
    read_unlock(&tasklist_lock);
    if(task){
    	//printk("Send signal from kernel\n");
        send_sig(SIGUSR2,task,0);
    }else {
        //printk("Path selection daemon pid: %d does not exist\n", br->mesh_pathsel_pid);
    }
}

/*
void br_signal_pathsel()
{
	struct task_struct *task;

	struct net_bridge *br;

	br = find_br_by_name("br0");
	
	read_lock(&tasklist_lock);
    task = find_task_by_pid(br->mesh_pathsel_pid);
    read_unlock(&tasklist_lock);
    if(task)
    {
    	//printk("Send signal from kernel\n");
        send_sig(SIGUSR2,task,0);
		//br->stp_enabled = 1; //now pathsel daemon can turn it on
	}
    else {
        //printk("Path selection daemon pid: %d does not exist\n", br->mesh_pathsel_pid);
    }
}
*/

#endif	//CONFIG_RTK_MESH


static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;
#ifdef IGMP_SNOOPING
        unsigned char *dest;
        struct net_bridge_port *p;
        unsigned char macAddr[6];
        char operation;
        struct iphdr *iph;
		unsigned char proto=0;  
                          
        iph = skb->nh.iph;
		proto =  iph->protocol;                                                                                                                            
        dest = skb->mac.ethernet->h_dest;
        p = skb->dev->br_port;
        if ( igmpsnoopenabled && MULTICAST_MAC(dest) 
			 && (skb->mac.ethernet->h_proto == ETH_P_IP))
		{
			if (proto == IPPROTO_IGMP || proto == 0x11){ //0x11:udp
				operation=igmp_type_check(skb, macAddr);
	
				if (operation > 0) {
      	            br_update_igmp_snoop_fdb(operation, br, p, macAddr, skb);
			}	
		}
        }
	#ifdef	MCAST_TO_UNICAST	
	#ifdef	IPV6_MCAST_TO_UNICAST
	else if(igmpsnoopenabled 
		&& IPV6_MULTICAST_MAC(dest) 
		&& (skb->mac.ethernet->h_proto == ETH_P_IPV6) )
	{		
		operation = ICMPv6_check(skb , macAddr);
		if (operation > 0) {
				#ifdef	DBG_ICMPv6
				if( operation == 1)
					panic_printk("ICMPv6 mac add (from frame_up)\n");
				else if(operation == 2)
					panic_printk("ICMPv6 mac del (from frame_up)\n");	
				#endif
               	br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
		}
	}
	#endif	//end of IPV6_MCAST_TO_UNICAST
	#endif		
#endif

#ifdef CONFIG_RTK_GUEST_ZONE
	skb->__unused = 0;
	if (p->is_guest_zone) {
		if (br->lock_client_num > 0) {
			int i, found=0;
			for (i=0; i<br->lock_client_num; i++) {
				if (!memcmp(skb->mac.ethernet->h_source, br->lock_client_list[i], 6)) {
					found = 1;
					break;
				}			
			}
			if (!found) {
#ifdef DEBUG_GUEST_ZONE
				panic_printk("Drop because lock client list!!\n");
#endif
				kfree_skb(skb);
				return;
			}
			skb->__unused = 0xe5;
		}
		else {
			if (!memcmp(dest, br->dev.dev_addr, 6))	
				skb->__unused = 0xe5;				
		}			
	}	
#endif	

	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	indev = skb->dev;
	skb->dev = &br->dev;
	skb->pkt_type = PACKET_HOST;
	skb_push(skb, ETH_HLEN);
	skb->protocol = eth_type_trans(skb, &br->dev);
#ifdef NAT_SPEEDUP
	// Directly call the function to enque skb
	if(br_nat_speedup) {
		br_pass_frame_up_finish(skb);
		return;
	}
#endif
#if 0
	if (!(list_empty(&nf_hooks[(PF_BRIDGE)][(NF_BR_LOCAL_IN)])))
		printk("netfilter no empty in PF_BRIDGE\n");
#endif	

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
			br_pass_frame_up_finish);
}

static int br_handle_frame_finish(struct sk_buff *skb)
{
	struct net_bridge *br;
	unsigned char *dest;
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_port *p;
	int passedup;

	dest = skb->mac.ethernet->h_dest;

	p = skb->dev->br_port;
	if (p == NULL)
		goto err_nolock;

	br = p->br;
	read_lock(&br->lock);
	if (skb->dev->br_port == NULL)
		goto err;

	passedup = 0;
	if (br->dev.flags & IFF_PROMISC) {
		struct sk_buff *skb2;

		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 != NULL) {
			passedup = 1;
			br_pass_frame_up(br, skb2);
		}
	}
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)	
	if(enable_filter){
		struct iphdr *iph_check;
		iph_check = skb->nh.iph;	
		if(Filter_State==0){
			if(memcmp(dut_br0_mac, dest, 6)){
				if(iph_check->protocol==IPPROTO_ICMP){
					goto err;
				}
			}
		}
#if 0		
		else if(Filter_State==1 && (dest[0] & 1 == 0)){
			struct udphdr *udph;
			udph=(void *)iph_check + iph_check->ihl*4;
			if(iph_check->protocol==IPPROTO_UDP && udph->dest ==68){ //if dhcp server packet if unicast
				if(br_filter_enter(skb))
					goto err;
			}
		}
#endif		
	}
#endif	

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	if(enable_filter){			
		if(br_filter_enter(skb))
			goto err;
	}	
#endif		

	if (dest[0] & 1) {
#ifdef IGMP_SNOOPING
		struct iphdr *iph;
		unsigned int ipaddr=0;
		unsigned char proto=0;
		//		unsigned char reserved=0;
		unsigned char macAddr[6];
		unsigned char operation;		
		
		if ( !(br->dev.flags & IFF_PROMISC) 
			 && MULTICAST_MAC(dest) 
             && (skb->mac.ethernet->h_proto == ETH_P_IP))
        {			
			iph = skb->nh.iph;
			ipaddr =  iph->daddr;
			proto =  iph->protocol;  
			
			if (proto == IPPROTO_IGMP || proto == 0x11){ //0x11:udp
				operation=igmp_type_check(skb, macAddr);
				
				if (operation > 0) {
					br_update_igmp_snoop_fdb(operation, br, p, macAddr, skb);
               }
            }
		}
		#ifdef	IPV6_MCAST_TO_UNICAST
		else if(!(br->dev.flags & IFF_PROMISC) 
				&& IPV6_MULTICAST_MAC(dest) //  for ipv6 case
				&& (skb->mac.ethernet->h_proto == ETH_P_IPV6))
		{
								
				operation = ICMPv6_check(skb , macAddr);
				if(operation > 0){
					#ifdef	DBG_ICMPv6
					if( operation == 1)
						panic_printk("icmpv6 add from frame finish\n");
					else if(operation == 2)
						panic_printk("icmpv6 del from frame finish\n");	
					#endif
					br_update_igmp_snoop_fdb(operation, br, p, macAddr, skb);
				}
				
		}
		#endif		
	    //Brad disable 20080619	      
		//		if((ipaddr&0xF0FFFF00)==0xE0000000)
		//	   		reserved=1;
	       
		if(igmpsnoopenabled 
			&& (MULTICAST_MAC(dest) || IPV6_MULTICAST_MAC(dest))
			&& proto !=IPPROTO_IGMP 
			&& ipaddr != 0xEFFFFFFA 
			//&& reserved ==0
			) 
		{	
			if ((dst = br_fdb_get(br, dest)) != NULL){
				br_multicast_forward(br, dst, skb, !passedup);
				br_fdb_put(dst);
			}else{
				br_flood_forward(br, skb, !passedup);
			}
		}
		else { // broadcast
			br_flood_forward(br, skb, !passedup);
 		}
#else

		br_flood_forward(br, skb, !passedup);
#endif
		if (!passedup)
#ifdef MULTICAST_FILTER
		{
			if ((br->fltr_portlist_num) &&
				!memcmp(dest, "\x01\x00\x5e", 3))
			{
				int i, pass_up = 1;
				unsigned short frag_offset = *((unsigned short *)&(skb->data[6]));
				unsigned short port = *((unsigned short *)&(skb->data[22]));
				unsigned long x;

				if ((frag_offset & 0x1fff) == 0) {	// check fragment offset
					for (i=0; i<br->fltr_portlist_num; i++) {
						if (port == br->fltr_portlist[i]) {
							pass_up = 0;
							break;
						}
					}
				}

				x = dest[3] ^ dest[4] ^ dest[5];
				x = x & (MLCST_MAC_ENTRY - 1);

				if (pass_up) {
					if ((br->fltr_maclist[x][3] != 0) &&
						!memcmp(&(br->fltr_maclist[x][0]), &(dest[3]), 3))
						kfree_skb(skb);
					else
						br_pass_frame_up(br, skb);
				}
				else {
					kfree_skb(skb);
					if (br->fltr_maclist[x][3] == 0) {
						memcpy(&(br->fltr_maclist[x][0]), &(dest[3]), 3);
						br->fltr_maclist[x][3] = 1;
					}
				}
			}
			else
				br_pass_frame_up(br, skb);
		}
#else
			br_pass_frame_up(br, skb);
#endif
		goto out;
	}

	dst = br_fdb_get(br, dest);
	if (dst != NULL && dst->is_local) {
		if (!passedup)
			br_pass_frame_up(br, skb);
		else
			kfree_skb(skb);
		br_fdb_put(dst);
		goto out;
	}

	if (dst != NULL) {
		br_forward(dst->dst, skb);
		br_fdb_put(dst);
		goto out;
	}

	br_flood_forward(br, skb, 0);

out:
	read_unlock(&br->lock);
	return 0;

err:
	read_unlock(&br->lock);
err_nolock:
	kfree_skb(skb);
	return 0;
}

void br_handle_frame(struct sk_buff *skb)
{
	struct net_bridge *br;
	unsigned char *dest;
	struct net_bridge_port *p;

	dest = skb->mac.ethernet->h_dest;

	p = skb->dev->br_port;
	if (p == NULL)
		goto err_nolock;

	br = p->br;
	read_lock(&br->lock);
	if (skb->dev->br_port == NULL)
		goto err;

#if 0
#ifdef STP_DISABLE_ETH
//chris: auto stp on
	if (!(br->dev.flags & IFF_UP))
		goto err;

	if (p->state==BR_STATE_DISABLED && p->disable_by_mesh ==0){
		goto err;
	}else if (!strncmp(p->dev->name,"eth", 3)){
			br->stp_enabled = 1;
			br_timer_set(&br->eth0_autostp_timer, jiffies);
	}
#endif
#endif 

	if (!(br->dev.flags & IFF_UP) ||
	    p->state == BR_STATE_DISABLED)
		goto err;



	if (skb->mac.ethernet->h_source[0] & 1)
		goto err;

#ifdef RTL_BRIDGE_MAC_CLONE
	if (br_mac_clone_handle_frame(br, p, skb) == -1)
		goto err;
#endif

	if (p->state == BR_STATE_LEARNING ||
	    p->state == BR_STATE_FORWARDING)
		br_fdb_insert(br, p, skb->mac.ethernet->h_source, 0);

	if (br->stp_enabled &&
	    !memcmp(dest, bridge_ula, 5) &&
	    !(dest[5] & 0xF0))
		goto handle_special_frame;

	if (p->state == BR_STATE_FORWARDING) {

#ifdef CONFIG_RTK_MESH
		//brian modify for trigger portal-enable event
		if(!strncmp(p->dev->name,"eth", 3)){
			if(br->mesh_pathsel_pid!= 0){
				if( !(br->eth0_received) )
				{
					br->eth0_received = 1;
					br->stp_enabled = 1;
					br_signal_pathsel(br);					
					//printk(KERN_INFO,"eth0 learning, event pathsel daemon \n");
				}

				br_timer_set(&br->eth0_monitor_timer, jiffies);
			}
		}
#endif

		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		read_unlock(&br->lock);
		return;
	}

err:
	read_unlock(&br->lock);
err_nolock:
	kfree_skb(skb);
	return;

handle_special_frame:
	if (!dest[5]) {
		br_stp_handle_bpdu(skb);
		return;
	}

	kfree_skb(skb);
}



#ifdef IGMP_SNOOPING

#ifdef	IPV6_MCAST_TO_UNICAST
/*Convert  MultiCatst IPV6_Addr to MAC_Addr*/
static void CIPV6toMac
	(unsigned char* icmpv6_McastAddr, unsigned char *gmac )
{
	/*ICMPv6 valid addr 2^32 -1*/
	gmac[0] = 0x33;
	gmac[1] = 0x33;
	gmac[2] = icmpv6_McastAddr[12];
	gmac[3] = icmpv6_McastAddr[13];
	gmac[4] = icmpv6_McastAddr[14];
	gmac[5] = icmpv6_McastAddr[15];			
}


/*
reference source
RFC2710 - Multicast Listener Discovery (MLD) for IPv6
Read more: http://www.faqs.org/rfcs/rfc2710.html#ixzz0ef3IkdKs
http://www.faqs.org/rfcs/rfc2710.html
*/
static int count=0;
static int ICMPv6_check(struct sk_buff *skb , unsigned char *gmac)
{
	
	struct ipv6hdr *ipv6h;
	char* protoType;	
	char* ModeChange;	
	unsigned char* icmpv6_McastAddr ;
	
	/* check IPv6 header information */
	ipv6h = skb->nh.ipv6h;
	if(ipv6h->version != 6){	
		//printk("ipv6h->version != 6\n");
		return -1;
	}


	/*Next header: IPv6 hop-by-hop option (0x00)*/
	if(ipv6h->nexthdr == 0 || ipv6h->nexthdr == 0x11)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );	
	}
	else{
		//printk("ipv6h->nexthdr != 0\n");
		return -1;
	}

	if(protoType[0] == 0x3a){
		
		//printk("recv icmpv6 packet\n");
		struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		
	
		if(icmpv6h->icmp6_type == 0x83){	//  Type = decimal 131
			
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8);
			#ifdef	DBG_ICMPv6					
			printk("(v6 Multicast listener report (Join) ; Type: 0x%x)\n",icmpv6h->icmp6_type);
			#endif

		}else if(icmpv6h->icmp6_type == 0x8f){		
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 + 4);
			#ifdef	DBG_ICMPv6					
			printk("(v6 Multicast listener report v2 (Join) ; Type: 0x%x)\n",icmpv6h->icmp6_type);
			#endif			
		}else if(icmpv6h->icmp6_type == 0x84){ //Type = decimal 143
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );			
			#ifdef	DBG_ICMPv6					
			printk("(v6 Multicast listener done (Leave) ; Type: 0x%x )\n",icmpv6h->icmp6_type);
			#endif			
		}
		else if(icmpv6h->icmp6_type == 0x82){  //  Type = decimal 130
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );			
			#ifdef	DBG_ICMPv6					
			printk("(v6 Multicast listener (Query) ; Type: 0x%x )\n",icmpv6h->icmp6_type);
			#endif			
		}			
		else{
			#ifdef	DBG_ICMPv6
			printk("(v6 unknow type ; Type: 0x%x )\n",icmpv6h->icmp6_type);
			#endif			
			return -1;
		}				

		#ifdef	DBG_ICMPv6			
		printk("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
			icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
			icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
			icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
		#endif

		CIPV6toMac(icmpv6_McastAddr, gmac);
		
		#ifdef	DBG_ICMPv6					
		printk("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
			gmac[0],gmac[1],gmac[2],
			gmac[3],gmac[4],gmac[5]);
		#endif
		if(gmac[2]==0 &&	gmac[3]==1 && gmac[4]==0 && gmac[5]==3)	
			return -1;
		if(gmac[2]==0 &&	gmac[3]==0 && gmac[4]==0 && gmac[5]==0xc)	
			return -1;
		


		if(icmpv6h->icmp6_type == 0x83){

			return 1;//icmpv6 listener report (add)
		}
		else if(icmpv6h->icmp6_type == 0x8f){
			//icmpv6 listener report(v2)
			
			ModeChange = protoType + 8 + 8;			
			if(ModeChange[0]==4){
				// change_to_exclude ; ( add ) 	
				return 1;					
			}else if(ModeChange[0]==3){
				//change_to_include ; ( del )
				return 2;	
			}
				
			
		}
		else if(icmpv6h->icmp6_type == 0x84){
			return 2;//icmpv6 Multicast listener done (del)
		}
	}		
#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
	else if(protoType[0] == 0x02 && protoType[1] == 0x22 && protoType[2] == 0x02 && protoType[3] == 0x23) //DHCPv6 Solicit
	{

		struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		
		icmpv6_McastAddr = ipv6h->daddr.in6_u.u6_addr8;
		//daddr
		CIPV6toMac(icmpv6_McastAddr, gmac);	
		
		return 3; //DHCPv6 Solicit
	}
	else if(ipv6h->nexthdr == 0x11)
	{
	
		count++;
		if(count%2000 == 0)
		{
			//printk("recv icmpv6 packet\n");
			struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		
			icmpv6_McastAddr = ipv6h->daddr.in6_u.u6_addr8;
		//daddr
			CIPV6toMac(icmpv6_McastAddr, gmac);		
			return 4;
		}					
	}
#endif //#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
	else{
		//printk("protoType[0] != 0x3a\n");		
		return -1;//not icmpv6 type
	}
		
	return -1;
}

#endif	//end of IPV6_MCAST_TO_UNICAST


#if	0
// 0407-2009
unsigned char check_MC_entry(struct net_bridge *br , 
	const unsigned char *SA ,
	const unsigned char *DA)
{
	int i;
	for(i=0 ; i<MLCST_MAC_ENTRY ;i++){
		if(br->HME[i].valid==1){
			if(!memcmp(br->HME[i].SrcMac , SA ,6) &&
				!memcmp(br->HME[i].DestMac, DA ,6) )
			{
				return br->HME[i].port;
			}			
		}
	}
	return 0;
}

void add_MC_entry(struct net_bridge *br ,
	unsigned char port ,
	const unsigned char *SA ,
	const unsigned char *DA )
{
	int i;
	BDBFENTER;
	for(i=0 ; i<MLCST_MAC_ENTRY ;i++){
		if(br->HME[i].valid !=1){
			memcpy(br->HME[i].SrcMac , SA ,6);
			memcpy(br->HME[i].DestMac, DA ,6);
			br->HME[i].port = port;
			br->HME[i].valid = 1;			
			DEBUG_PRINT("add mc_entry,SA:%02x:%02x:%02x:%02x:%02x:%02x ;DA:%02x:%02x:%02x:%02x:%02x:%02x port:%x\n",
				SA[0],SA[1],SA[2],SA[3],SA[4],SA[5],
				DA[0],DA[1],DA[2],DA[3],DA[4],DA[5],
				port);
			break;
		}
	}
}

void update_MC_entry(struct net_bridge *br , 
	unsigned char portnew ,
	const unsigned char *SA ,
	const unsigned char *DA )
{
	int i;
	BDBFENTER;	
	for(i=0 ; i<MLCST_MAC_ENTRY ;i++){
		if(br->HME[i].valid ==1){
			if(!memcmp(br->HME[i].SrcMac , SA ,6)
				&& !memcmp(br->HME[i].DestMac , DA ,6))
			{
				br->HME[i].port = portnew;						
				DEBUG_PRINT("update mc entry ,SA:%02x:%02x:%02x:%02x:%02x:%02x;DA:%02x:%02x:%02x:%02x:%02x:%02x ; now port:%x\n",
					 SA[0],SA[1],SA[2],SA[3],SA[4],SA[5],
		 			 DA[0],DA[1],DA[2],DA[3],DA[4],DA[5],
	 	 	 		 portnew);
				break;
			}
		}
	}
}

void del_MC_entry(struct net_bridge *br , 
	const unsigned char *SA ,
	const unsigned char *DA )
{
	int i;
	BDBFENTER;	
	for(i=0 ; i<MLCST_MAC_ENTRY ;i++){
		if(br->HME[i].valid ==1){
			if(!memcmp(br->HME[i].SrcMac , SA ,6)
				&& !memcmp(br->HME[i].DestMac , DA ,6))
			{
				br->HME[i].valid = 0;
				DEBUG_PRINT("del mc entry ,SA:%02x:%02x:%02x:%02x:%02x:%02x;DA:%02x:%02x:%02x:%02x:%02x:%02x ;port:%x\n",
					 SA[0],SA[1],SA[2],SA[3],SA[4],SA[5],
		 			 DA[0],DA[1],DA[2],DA[3],DA[4],DA[5],
	 	 	 		 br->HME[i].port);
				break;
			}
		}
	}
}
#endif


static void ConvertMulticatIPtoMacAddr(__u32 group, unsigned char *gmac)
{
	__u32 u32tmp, tmp;
	int i;

	u32tmp = group & 0x007FFFFF;
	gmac[0]=0x01; gmac[1]=0x00; gmac[2]=0x5e;
	for (i=5; i>=3; i--) {
		tmp=u32tmp&0xFF;
		gmac[i]=tmp;
		u32tmp >>= 8;
	}
}
static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac)
{
        struct iphdr *iph;
	__u8 hdrlen;
	struct igmphdr *igmph;

#ifdef	MCAST_TO_UNICAST	
	unsigned int IGMP_Group;// add  for fit igmp v3
#endif
	
	/* check IP header information */
	iph = skb->nh.iph;
	hdrlen = iph->ihl << 2;
	if ((iph->version != 4) &&  (hdrlen < 20))
		return -1;
	if (ip_fast_csum((u8 *)iph, iph->ihl) != 0)
		return -1;
	{ /* check the length */
	__u32 len = ntohs(iph->tot_len);
	if (skb->len < len || len < hdrlen)
		return -1; 
	}
	
	if(iph->protocol == 0x11)
	{
		
		count++;
		if(count %9999 == 0)
		{
			IGMP_Group = iph->daddr;
		
		//daddr
			ConvertMulticatIPtoMacAddr(IGMP_Group, gmac);		
			return 4;
		}
		
	}
	
	
	/* parsing the igmp packet */
	igmph = (struct igmphdr *)((u8*)iph+hdrlen);
	
#ifdef	MCAST_TO_UNICAST
	/*IGMP-V3 type Report*/
	if(igmph->type == IGMPV3_HOST_MEMBERSHIP_REPORT)
	{
		//printk("rec v3 report 1\n");
		/*in 11n seem need no care igmpProxy is opened or not,plus 2008-0612*/
		#if	0
		if(IGMPProxyOpened==0){
			IGMP_Group = *(unsigned int*)((unsigned int*)igmph + 3);
			
			//printk("v3_group:%02X:%02X:%02X:%02X\n",
			//IGMP_Group>>24,(IGMP_Group<<8)>>24,(IGMP_Group<<16)>>24,(IGMP_Group<<24)>>24);
		}else{
			return -1;//don't care v3 report
		}
		#else
			IGMP_Group = *(unsigned int*)((unsigned int*)igmph + 3);
		#endif
		
	}else{	//4 V2 or V1
		//printk("igmph->group:%04X\n",igmph->group);	
		IGMP_Group = igmph->group;
	}
#endif
#ifdef	MCAST_TO_UNICAST

	/*check if it's protocol reserved group */
	if(!IN_MULTICAST(IGMP_Group))
	{			
			return -1;
	}
	//Brad disable 20080619
	/*
	if(reserved_chk(IGMP_Group)){			
		return -1;
	}
	*/
	
	ConvertMulticatIPtoMacAddr(IGMP_Group, gmac);
	
#else
	/*check if it's protocol reserved group */
	if(!IN_MULTICAST(igmph->group))
	{
			return -1;
	}
	//Brad disable 20080619
	/*
	if(reserved_chk(igmph->group)){			
		return -1;
	}
	*/
	
	ConvertMulticatIPtoMacAddr(igmph->group, gmac);
#endif


				
	if ((igmph->type==IGMP_HOST_MEMBERSHIP_REPORT) ||
	    (igmph->type==IGMPV2_HOST_MEMBERSHIP_REPORT)) 
	{
		return 1; /* report and add it */
	}

	else if (igmph->type==IGMPV3_HOST_MEMBERSHIP_REPORT)	{ 

	/*for support igmp v3 ; plusWang add 2009-0311*/
		
		struct igmpv3_grec	*v3grec = (struct igmpv3_grec *)((unsigned char*)igmph + 8);


		if(v3grec->grec_type == IGMPV3_CHANGE_TO_INCLUDE){
			
			//printk("igmp-v3 C2I\n");
			return 2; /* leave and delete it */
			
		}else	if(v3grec->grec_type == IGMPV3_CHANGE_TO_EXCLUDE){
		
			//printk("igmp-v3 C2E\n");			
			return 1;
		}
		/*
		else{
			printk("v3grec->grec_type =%d\n",v3grec->grec_type);
			printk("no yet support igmp-v3 type\n");
		}
		*/


	}
	else if (igmph->type==IGMP_HOST_LEAVE_MESSAGE){
		return 2; /* leave and delete it */
	}	
	
	
	return -1;
}


static void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *dest 
										,struct sk_buff *skb)
{
#ifdef	MCAST_TO_UNICAST
	if(!dest)	return;
	
	if( !MULTICAST_MAC(dest) 
#ifdef	IPV6_MCAST_TO_UNICAST
		&& !IPV6_MULTICAST_MAC(dest)
#endif		
	   ){ return; }
#endif

	struct net_bridge_fdb_entry *dst;
	unsigned short del_group_src=0;
	/* check whether entry exist */
	dst = br_fdb_get(br, dest);
	unsigned char *src;
	unsigned char port_comein;
	int tt1;

	
	port_comein	=(skb->cb[3] & 0x7f);
	//port_comein	=(skb->cb[3]);	
	src = skb->mac.ethernet->h_source;
	
	if (op == 1) /* add */
	{	
#ifdef	MCAST_TO_UNICAST		
		/*process wlan client join --- start*/
		if (dst && p && p->dev && p->dev->name && !memcmp(p->dev->name, "wlan", 4)) { 

			dst->portlist |= 0x80;
			port_comein = 0x80;
			//DEBUG_PRINT("... wlan0 added portlist=%02x\n",dst->portlist);
			struct net_device *dev = __dev_get_by_name("wlan0");	
			if (dev) {			
				unsigned char StaMacAndGroup[20];
				memcpy(StaMacAndGroup, dest, 6);
				memcpy(StaMacAndGroup+6, src, 6);				
				if (dev->do_ioctl != NULL) {
					dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B80);					
					DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
					StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
					StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);									
				}			
			}

			#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
			if ( ( dst->index >= 0) && dst->hw_table_enable != 0) {
				update_ip_multicast_tbl(dst->index, dst->portlist & 0x7f, 0);			
				dst->hw_table_enable = 0;
				DEBUG_PRINT("invalidate ip multicast table, index=%d, port=%x\n", dst->index, dst->portlist&0x7f); 
			}
			#endif			
		}		
		/*process wlan client join --- end*/		
#endif		

		if (dst) {

			dst->group_src = dst->group_src | (1 << p->port_no);

			dst->ageing_timer = jiffies;
			
			if(skb->cb[4] != 0)
					dst->vlan_id = skb->cb[4];
					
			tt1 = chk_igmp_ext_entry(dst , src); 
			if(tt1 == 0){
				add_igmp_ext_entry(dst , src , port_comein);								
					
			}else{
				update_igmp_ext_entry(dst , src , port_comein);
				
			}			
			
			#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
			if ( dst->portlist && ((dst->portlist & 0x80)== 0) && ( dst->index >= 0)) {
				update_ip_multicast_tbl(dst->index, dst->portlist & 0x7f, 1);
				dst->hw_table_enable = 1;
				DEBUG_PRINT("(add) validate ip multicast table,  port=%x ,index=%d ; da:%02x:%02x:%02x:%02x:%02x:%02x:\n",
						 dst->portlist&0x7f , dst->index , 
						 dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]); 
			}			
			#endif

			
		}
		else {
			/* insert one fdb entry */
			DEBUG_PRINT("insert one fdb entry\n");
			br_fdb_insert(br, p, dest, 0);
			dst = br_fdb_get(br, dest);
			if(dst !=NULL){
				dst->group_src = dst->group_src | (1 << p->port_no);
				dst->portlist = skb->cb[3]; 
				#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
				dst->index = -1; // empty

				if(skb->cb[4] != 0)
					dst->vlan_id = skb->cb[4];
				#endif				
			}
		}	
	}
	else if (op == 2 && dst) /* delete */
	{

#if 0		
		DEBUG_PRINT("dst->group_src = %x change to ",dst->group_src);		
		del_group_src = ~(1 << p->port_no);
		dst->group_src = dst->group_src & del_group_src;
		DEBUG_PRINT(" %x ; p->port_no=%x \n",dst->group_src ,p->port_no);
#endif
		

		/*process wlan client leave --- start*/
		if (p && p->dev && p->dev->name && !memcmp(p->dev->name, "wlan", 4)) { 

			//dst->portlist &= ~0x80;	// move to del_igmp_ext_entry
			port_comein	= 0x80;
			#ifdef	MCAST_TO_UNICAST
			struct net_device *dev = __dev_get_by_name("wlan0");	
			if (dev) {			
				unsigned char StaMacAndGroup[12];
				memcpy(StaMacAndGroup, dest , 6);
				memcpy(StaMacAndGroup+6, src , 6);
				if (dev->do_ioctl != NULL) {
					dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);							
					DEBUG_PRINT("(del) wlan0 ioctl (del) M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
				}
			}
			#endif	
			
			#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
			if ( dst->portlist && (dst->index >= 0) && (dst->hw_table_enable == 0) ) {
				update_ip_multicast_tbl(dst->index, dst->portlist & 0x7f, 1);
				dst->hw_table_enable = 1;
				DEBUG_PRINT("(del)validate ip multicast table, index=%d, port=%x\n", dst->index, dst->portlist & 0x7f); 
			}			
			#endif
		}	
		/*process wlan client leave --- end*/

		/*process entry del , portlist update*/
		del_igmp_ext_entry(dst , src ,port_comein);		

		if ( (dst->portlist & 0x7f)==0){
			dst->group_src &=  ~(1 << 1); // eth0 all leave
		}
	
		if ( (dst->portlist & 0x80) == 0){
			dst->group_src &=  ~(1 << 2); // wlan0 all leave
		}

		#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)		
		if ( (dst->hw_table_enable == 1) &&	( dst->index >= 0) ) 
		{

			if(	(dst->portlist & 0x7f) == 0	){
				DEBUG_PRINT("1 delete ip multicast table, idx=%d\n", dst->index);
				rtl8651_delAsicIpMulticastTable(dst->index);
				dst->index = -1;
				dst->hw_table_enable = 0;
			}else{

				if(	dst->portlist && !(dst->portlist & 0x80)){	
					/*if wlan0 exist some client need not update hw-table*/ 
					/*still some one record by this entry so just need update this entry's context*/
					update_ip_multicast_tbl(dst->index, (dst->portlist & 0x7f), 1);			
					dst->hw_table_enable = 1;
					DEBUG_PRINT("1 update entry(%d)'s context for port(%d)\n",dst->index ,bitmask_to_id(skb->cb[3]));
				}				
			}
			
		}

		
		#endif		
		if (dst->portlist == 0)  // all joined sta are gone
		{
	#ifdef	IGMP_Proxy_DBG		
			printk("portlist is empty,expired this dst\n");
	#endif
			dst->ageing_timer -=  300*HZ; // make it expired		
		}

	}
#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
        else if(op == 3 && dst == NULL)
	{
			skb->cb[0] = 0x9f & ~(skb->cb[3]); /* 1001111 */
	}
#endif	
	else if(op == 4 && dst)
	{
		dst->ageing_timer = jiffies;
		update_igmp_ext_entry(dst , src , port_comein);
	}
}

static int isIPv6UdpData(struct sk_buff *skb)
{
	if(skb->protocol == 0x86dd
		&& skb->data[20] == 0x11 //next header is udp		
	)
	{
		return 1;		
	}
	else
		return 0;	
}

#if defined(CONFIG_RTK_VLC_SPEEDUP_SUPPORT) && defined(CONFIG_RTL8192CD)
// for video streaming refine
struct net_device *is_wlan_streaming_only(struct sk_buff *skb)
{
	struct net_bridge_fdb_entry *dst;
	unsigned char *dest = skb->data;
	struct iphdr *iph = (struct iphdr *)(skb->data + 14);
	unsigned char	proto =  iph->protocol;  
	int i;
	struct net_device *dev = NULL;
	struct net_bridge *br;
	extern struct net_device *get_shortcut_dev(unsigned char *da);
	extern struct net_bridge *find_br_by_name(char *name);

	if (igmpsnoopenabled &&
		(MULTICAST_MAC(dest)
		  || isIPv6UdpData(skb)		
		) 
		&& proto !=IPPROTO_IGMP 
		&& iph->daddr != 0xEFFFFFFA
		) 
		{
		br = find_br_by_name("br0");
		dst = br_fdb_get(br, dest);
		if (dst != NULL) {
			for (i=0 ; i < FDB_IGMP_EXT_NUM ; i++) {
				if (dst->igmp_fdb_arr[i].valid) {
					dev = get_shortcut_dev(dst->igmp_fdb_arr[i].SrcMac);
					if (dev == NULL)
						break;						
				}
			}
		}
	}
	return dev;
}

#if defined(CONFIG_RTL_CLIENT_MODE_SUPPORT)
struct net_device *is_eth_streaming_only(struct sk_buff *skb)
{
	struct net_bridge_fdb_entry *dst;	
#if defined(__LINUX_2_6__) 
	unsigned char *dest = skb_mac_header(skb);
#else			
	unsigned char *dest = skb->mac.raw;
#endif
	struct iphdr *iph = (struct iphdr *)skb->data;
	unsigned char	proto =  iph->protocol;  
	int i, port_no;
	struct net_bridge *br;
	unsigned char	portlist;
	extern struct net_bridge *find_br_by_name(char *name);

	if (igmpsnoopenabled &&
		(MULTICAST_MAC(dest) || IPV6_MULTICAST_MAC(dest)) &&
			proto !=IPPROTO_IGMP &&
				iph->daddr != 0xEFFFFFFA) {
		br = find_br_by_name("br0");				
		dst = br_fdb_get(br, dest);		
		if (dst && !(dst->portlist & 0x80) && dst->portlist && dst->dst) {
			skb->cb[0] = dst->portlist;
			return dst->dst->dev;			
		}		
	}
	return NULL;	
}
#endif // CONFIG_RTL_CLIENT_MODE_SUPPORT
#endif // CONFIG_RTK_VLC_SPEEDUP_SUPPORT

extern struct net_bridge *find_br_by_name(char *name);
int get_eth_snooping_portlist(struct sk_buff *skb)
{
	struct net_bridge_fdb_entry *dst=NULL;	
	struct net_bridge *br=NULL;	
	unsigned char *dest=NULL;
	static int forkeepfdbalive=0;
	forkeepfdbalive++;
#if defined(__LINUX_2_6__) 
	dest = skb_mac_header(skb);
#else			
	dest = skb->data;
#endif
	
	br = find_br_by_name("br0");

	if(br)
		dst = br_fdb_get(br, dest);		
	
	if (dst) {
	
		if((forkeepfdbalive % 10000)==0 )
			dst->ageing_timer = jiffies;
			
		skb->cb[0] = dst->portlist;
		return 1;
	}			
	return 0;
}

#endif // IGMP_SNOOPING
