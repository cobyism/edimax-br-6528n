/*
 *	Forwarding database
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_fdb.c,v 1.20.2.2 2010/12/22 02:36:11 pluswang Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/if_bridge.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include "br_private.h"

// for fast-nat module, 2005-12-23
#ifdef NAT_SPEEDUP
static int br_nat_speedup=1;
#endif
static int fdb_num=0;
extern unsigned int fdb_max;

#ifdef CONFIG_RTL865X_ETH
#define FDB_DYNAMIC	0x02	/* flag for FDB: process dynamic entry only */
extern int rtl8651_lookupL2table(unsigned short fid, unsigned char *macAddr, int flags);
extern int rtl8651_delFilterDatabaseEntry(unsigned short fid, unsigned char *macAddr);

static void update_hw_l2table(struct net_bridge_port *source, unsigned char *addr)
{		   
	if (source->dev && memcmp(source->dev->name, "wlan", 4) == 0 &&
			rtl8651_lookupL2table(0, addr, FDB_DYNAMIC) == 0) 
		rtl8651_delFilterDatabaseEntry(0, addr); 	
}
#endif


#ifdef	IGMP_SNOOPING
extern int IGMPProxyOpened;
#endif


static __inline__ unsigned long __timeout(struct net_bridge *br)
{
	unsigned long timeout;

	timeout = jiffies - br->ageing_time;
	if (br->topology_change)
		timeout = jiffies - br->forward_delay;

	return timeout;
}
// 0407-2009
#if	0	//def	IGMP_SNOOPING
static __inline__ unsigned long igmp_timeout(struct net_bridge *br)
{
	unsigned long timeout2;

	timeout2 = jiffies - IGMP_EXPIRE_TIME;
	if (br->topology_change)
		timeout2 = jiffies - br->forward_delay;

	return timeout2;
}

#endif

static __inline__ int has_expired(struct net_bridge *br,
				  struct net_bridge_fdb_entry *fdb)
{
	if (!fdb->is_static &&
	    time_before_eq(fdb->ageing_timer, __timeout(br)))
		return 1;

	return 0;
}

static __inline__ void copy_fdb(struct __fdb_entry *ent, struct net_bridge_fdb_entry *f)
{
	memset(ent, 0, sizeof(struct __fdb_entry));
	memcpy(ent->mac_addr, f->addr.addr, ETH_ALEN);
	ent->port_no = f->dst?f->dst->port_no:0;
	ent->is_local = f->is_local;
	ent->ageing_timer_value = 0;
	if (!f->is_static)
		ent->ageing_timer_value = jiffies - f->ageing_timer;
}

static __inline__ int br_mac_hash(unsigned char *mac)
{
	unsigned long x;

	x = mac[0];
	x = (x << 2) ^ mac[1];
	x = (x << 2) ^ mac[2];
	x = (x << 2) ^ mac[3];
	x = (x << 2) ^ mac[4];
	x = (x << 2) ^ mac[5];

	x ^= x >> 8;

	return x & (BR_HASH_SIZE - 1);
}

static __inline__ void __hash_link(struct net_bridge *br,
				   struct net_bridge_fdb_entry *ent,
				   int hash)
{
	ent->next_hash = br->hash[hash];
	if (ent->next_hash != NULL)
		ent->next_hash->pprev_hash = &ent->next_hash;
	br->hash[hash] = ent;
	ent->pprev_hash = &br->hash[hash];
}

static __inline__ void __hash_unlink(struct net_bridge_fdb_entry *ent)
{
	*(ent->pprev_hash) = ent->next_hash;
	if (ent->next_hash != NULL)
		ent->next_hash->pprev_hash = ent->pprev_hash;
	ent->next_hash = NULL;
	ent->pprev_hash = NULL;
}



void br_fdb_changeaddr(struct net_bridge_port *p, unsigned char *newaddr)
{
	struct net_bridge *br;
	int i;

	br = p->br;
	write_lock_bh(&br->hash_lock);
	for (i=0;i<BR_HASH_SIZE;i++) {
		struct net_bridge_fdb_entry *f;

		f = br->hash[i];
		while (f != NULL) {
			if (f->dst == p && f->is_local) {
				__hash_unlink(f);
				memcpy(f->addr.addr, newaddr, ETH_ALEN);
				__hash_link(br, f, br_mac_hash(newaddr));
				write_unlock_bh(&br->hash_lock);
				return;
			}
			f = f->next_hash;
		}
	}
	write_unlock_bh(&br->hash_lock);
}


#ifdef	IGMP_SNOOPING
static void br_igmp_fdb_expired(struct net_bridge_fdb_entry *ent)
{
	int i2;
	unsigned long igmp_walktimeout;	
	unsigned char *DA;
	unsigned char *SA;
	
	
	igmp_walktimeout = 	jiffies - IGMP_EXPIRE_TIME;;	
	    
	//IGMP_EXPIRE_TIME
	
	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++)
	{
		if(ent->igmp_fdb_arr[i2].valid == 1){
			// when timeout expire
			if(time_before_eq(ent->igmp_fdb_arr[i2].ageing_time, igmp_walktimeout))
			{
			
				SA = ent->igmp_fdb_arr[i2].SrcMac;					
				DEBUG_PRINT("expired src mac:%02x,%02x,%02x,%02x,%02x,%02x\n",
					SA[0],SA[1],SA[2],SA[3],SA[4],SA[5]);								

				DA = ent->addr.addr;					
				DEBUG_PRINT("fdb:%02x:%02x:%02x-%02x:%02x:%02x\n",
					DA[0],DA[1],DA[2],DA[3],DA[4],DA[5]);				



				/*2010-1222 under wlan case ; maybe be Query-Report mechanism loss 
				 because under wireless env	 have the risks of packet drop*/
				#if	0	//def	MCAST_TO_UNICAST
				struct net_device *dev = __dev_get_by_name("wlan0");	
				if (dev) {			
					unsigned char StaMacAndGroup[20];
					memcpy(StaMacAndGroup, DA , 6);
					memcpy(StaMacAndGroup+6, SA, 6);
					if (dev->do_ioctl != NULL) {
						dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);							
						DEBUG_PRINT("%s %d DEL M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
							__FUNCTION__ , __LINE__ ,
							StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
							StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
					}
				}
				#endif			
				
				if(ent->igmp_fdb_arr[i2].port != 0x80 )	// 2010-1222		
					del_igmp_ext_entry(ent , SA , ent->igmp_fdb_arr[i2].port);

	
				if ( (ent->portlist & 0x7f)==0){
					ent->group_src &=  ~(1 << 1); // eth0 all leave
				}
				
				/*	2010-1222
				if ( (ent->portlist & 0x80)==0){
					ent->group_src &=  ~(1 << 2); // wlan0 all leave
				}
				*/			
			

			#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)			

				if ( (ent->portlist & 0x80)==0  && ( ent->index >= 0)) {

					if( (ent->portlist & 0x7f) == 0){
						//printk("delete ip multicast table, idx=%d\n", ent->index);
						DEBUG_PRINT("expire! rtl8651_delAsicIpMulticastTable(index:%d)\n",ent->index);									
						rtl8651_delAsicIpMulticastTable(ent->index);
						ent->index = -1; 				
						ent->hw_table_enable = 0;					
					}else{
						/*still some one record by this entry so just need update this entry's context*/
						DEBUG_PRINT("expire!! update entry(%d)'s context for port(%d)\n",ent->index ,bitmask_to_id(ent->igmp_fdb_arr[i2].port));						
						update_ip_multicast_tbl(ent->index, (ent->portlist & 0x7f), 1);			
						ent->hw_table_enable = 1;
					}			
				}						
			#endif				
			
			}			
			
		}		
		
	}		
	
}
#endif

/*for GW/Bridge mode info*/ 
extern int savOP_MODE_value ;

void br_fdb_cleanup(struct net_bridge *br)
{
	int i;
	unsigned long timeout;
	timeout = __timeout(br);

	write_lock_bh(&br->hash_lock);
	for (i=0;i<BR_HASH_SIZE;i++) {
		struct net_bridge_fdb_entry *f;

		f = br->hash[i];
		while (f != NULL) {
			struct net_bridge_fdb_entry *g;

			g = f->next_hash;
	
			#ifdef	IGMP_SNOOPING
			if(!f->is_static &&
				MULTICAST_MAC(f->addr.addr) &&
				 ( (savOP_MODE_value == 2) && (IGMPProxyOpened == 1)) )

			{
				br_igmp_fdb_expired(f);
			}
			#endif
			
			if (!f->is_static &&
			    time_before_eq(f->ageing_timer, timeout)) {
				__hash_unlink(f);
				br_fdb_put(f);
			}

			
			
			f = g;
			
		}
	}
	write_unlock_bh(&br->hash_lock);
}

void br_fdb_delete_by_port(struct net_bridge *br, struct net_bridge_port *p)
{
	int i;

	write_lock_bh(&br->hash_lock);
	for (i=0;i<BR_HASH_SIZE;i++) {
		struct net_bridge_fdb_entry *f;

		f = br->hash[i];
		while (f != NULL) {
			struct net_bridge_fdb_entry *g;

			g = f->next_hash;
			if (f->dst == p) {
				__hash_unlink(f);
				br_fdb_put(f);
			}
			f = g;
		}
	}
	write_unlock_bh(&br->hash_lock);
}

struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br, unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;

	read_lock_bh(&br->hash_lock);
	fdb = br->hash[br_mac_hash(addr)];
	while (fdb != NULL) {
		if (!memcmp(fdb->addr.addr, addr, ETH_ALEN)) {
			if (!has_expired(br, fdb)) {
				atomic_inc(&fdb->use_count);
				read_unlock_bh(&br->hash_lock);
				return fdb;
			}

			read_unlock_bh(&br->hash_lock);
			return NULL;
		}

		fdb = fdb->next_hash;
	}

	read_unlock_bh(&br->hash_lock);
	return NULL;
}

void br_fdb_put(struct net_bridge_fdb_entry *ent)
{
	if (atomic_dec_and_test(&ent->use_count))
	{
#ifdef NAT_SPEEDUP		
		if (ent->hh_ptr && br_nat_speedup)
			ent->hh_ptr->fdb_cache=NULL;
#endif
		if(fdb_num > 0)
			fdb_num--;

		kfree(ent);		
	}
}

int br_fdb_get_entries(struct net_bridge *br,
		       unsigned char *_buf,
		       int maxnum,
		       int offset
#ifdef CONFIG_RTK_GUEST_ZONE
		       ,int for_guest
#endif		       
			)
{
	int i;
	int num;
	struct __fdb_entry *walk;

	num = 0;
	walk = (struct __fdb_entry *)_buf;

	read_lock_bh(&br->hash_lock);
	for (i=0;i<BR_HASH_SIZE;i++) {
		struct net_bridge_fdb_entry *f;

		f = br->hash[i];
		while (f != NULL && num < maxnum) {
			struct __fdb_entry ent;
			int err;
			struct net_bridge_fdb_entry *g;
			struct net_bridge_fdb_entry **pp; 

			if (has_expired(br, f)) {
				f = f->next_hash;
				continue;
			}

#ifdef CONFIG_RTK_GUEST_ZONE
			if (for_guest && !f->dst->is_guest_zone) {
				f = f->next_hash;
				continue;
			}			
#endif		       

			if (offset) {
				offset--;
				f = f->next_hash;
				continue;
			}

			copy_fdb(&ent, f);

			atomic_inc(&f->use_count);
			read_unlock_bh(&br->hash_lock);
			err = copy_to_user(walk, &ent, sizeof(struct __fdb_entry));
			read_lock_bh(&br->hash_lock);

			g = f->next_hash;
			pp = f->pprev_hash;
			br_fdb_put(f);

			if (err)
				goto out_fault;

			if (g == NULL && pp == NULL)
				goto out_disappeared;

			num++;
			walk++;

			f = g;
		}
	}

 out:
	read_unlock_bh(&br->hash_lock);
	return num;

 out_disappeared:
	num = -EAGAIN;
	goto out;

 out_fault:
	num = -EFAULT;
	goto out;
}

static __inline__ void __fdb_possibly_replace(struct net_bridge_fdb_entry *fdb,
					      struct net_bridge_port *source,
					      int is_local)
{
	if (!fdb->is_static || is_local) {
		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
		fdb->ageing_timer = jiffies;
	}
}

void br_fdb_insert(struct net_bridge *br,
		   struct net_bridge_port *source,
		   unsigned char *addr,
		   int is_local)
{
	struct net_bridge_fdb_entry *fdb;
	int hash;
#ifdef	FDB_FINETUNE	
	int i3;
#endif
	hash = br_mac_hash(addr);

	write_lock_bh(&br->hash_lock);
	fdb = br->hash[hash];
	while (fdb != NULL) {
		if (!fdb->is_local &&
		    !memcmp(fdb->addr.addr, addr, ETH_ALEN)) {
		    	
#ifdef CONFIG_RTL865X_ETH
			if (((unsigned long)fdb->dst) != ((unsigned long)source)) 
				update_hw_l2table(source, addr);
#endif					    	
		    	
			__fdb_possibly_replace(fdb, source, is_local);
			write_unlock_bh(&br->hash_lock);
			return;
		}

		fdb = fdb->next_hash;
	}

	if(fdb_num> fdb_max) {
		write_unlock_bh(&br->hash_lock);
		return;
	}

	fdb = kmalloc(sizeof(*fdb), GFP_ATOMIC);
	if (fdb == NULL) {
		write_unlock_bh(&br->hash_lock);
		return;
	}
	fdb_num++;

	memcpy(fdb->addr.addr, addr, ETH_ALEN);
	atomic_set(&fdb->use_count, 1);
	fdb->dst = source;
	fdb->is_local = is_local;
	fdb->is_static = is_local;
	fdb->ageing_timer = jiffies;
#ifdef IGMP_SNOOPING	
	fdb->group_src = 0;

	if(IPV6_MULTICAST_MAC(addr))
		fdb->ipv6_multicast = 1;
	else
		fdb->ipv6_multicast = 0;
	
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
	fdb->index = -1; // must init it ; PlusWang
	fdb->hw_table_enable = 0;
#endif
#ifdef	FDB_FINETUNE
	for(i3=0 ; i3<FDB_IGMP_EXT_NUM ;i3++){
		fdb->igmp_fdb_arr[i3].valid = 0;
		fdb->portUsedNum[i3] = 0;		
	}
#endif
#endif

#ifdef NAT_SPEEDUP
	fdb->hh_ptr = NULL;
#endif

	__hash_link(br, fdb, hash);
	write_unlock_bh(&br->hash_lock);
	
#ifdef CONFIG_RTL865X_ETH
	update_hw_l2table(source, addr);
#endif	
}

#ifdef	IGMP_SNOOPING
int chk_igmp_ext_entry(
	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac)
{

	int i2;
	unsigned char *add;
	add = fdb->addr.addr;

	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 1){
			if(!memcmp(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6)){
				return 1;
			}
		}
	}
	return 0;
}


void add_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn)
{

//	if(fdb->igmp_ext_array == NULL)
//		return 0;

	int i2;
	unsigned char *add;
	add = fdb->addr.addr;	

	DEBUG_PRINT("add_igmp,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x\n",
		add[0],add[1],add[2],add[3],add[4],add[5],
		srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);
	
	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 0){
			fdb->igmp_fdb_arr[i2].valid = 1	;
			fdb->igmp_fdb_arr[i2].ageing_time = jiffies ;			
			memcpy(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6);
			fdb->igmp_fdb_arr[i2].port = portComeIn ;
			fdb->portlist |= portComeIn;
			fdb->portUsedNum[bitmask_to_id(portComeIn)]++;
			DEBUG_PRINT("portUsedNum[%d]=%d\n\n",bitmask_to_id(portComeIn) , fdb->portUsedNum[bitmask_to_id(portComeIn)]);
			return ;
		}
	}
	DEBUG_PRINT("%s:entry Rdy existed!!!\n", __FUNCTION__);	
}

void update_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn)
{
	int i2;
	unsigned char *add;
	add = fdb->addr.addr;	



	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 1){
			if(!memcmp(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6)){

				fdb->igmp_fdb_arr[i2].ageing_time = jiffies ;
				DEBUG_PRINT("update_igmp,DA=%02x%02x%02x:%02x%02x%02x ; SA=%02x%02x%02x:%02x%02x%02x\n",
				add[0],add[1],add[2],add[3],add[4],add[5],
				srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);	


				
				if(fdb->igmp_fdb_arr[i2].port != portComeIn){

					unsigned char port_orig = fdb->igmp_fdb_arr[i2].port ;					
					int index = bitmask_to_id(port_orig);

					fdb->portUsedNum[index]-- ;
					DEBUG_PRINT("(--) portUsedNum[%d]=%d\n",index , fdb->portUsedNum[index] );					
					if(fdb->portUsedNum[index] <= 0){
						fdb->portlist &= ~(port_orig);
						if(fdb->portUsedNum[index]< 0){
							DEBUG_PRINT("!! portNum[%d] < 0 at (update_igmp_ext_entry)\n",index);
							fdb->portUsedNum[index] = 0 ;
						}
					}					

					
					fdb->portUsedNum[bitmask_to_id(portComeIn)]++;
					DEBUG_PRINT("(++) portUsedNum[%d]=%d\n",bitmask_to_id(portComeIn) , fdb->portUsedNum[bitmask_to_id(portComeIn)] );										
					fdb->portlist |= portComeIn;						

					
					fdb->igmp_fdb_arr[i2].port = portComeIn ;					
					DEBUG_PRINT("	!!! portlist be updated:%x !!!!\n",fdb->portlist);
					
				}
				return ;
			}			
		}		
	}

	DEBUG_PRINT("%s: ...fail!!\n", __FUNCTION__);
}


void del_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn )
{
	int i2;
	unsigned char *add;
	add = fdb->addr.addr;	
	
	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 1){
			if(!memcmp(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6))
			{
				fdb->igmp_fdb_arr[i2].ageing_time -=  300*HZ; // make it expired	
				fdb->igmp_fdb_arr[i2].valid = 0;
				DEBUG_PRINT("\ndel_igmp_ext_entry,DA=%02x%02x%02x:%02x%02x%02x ; SA=%02x%02x%02x:%02x%02x%02x success!!!\n",
				add[0],add[1],add[2],add[3],add[4],add[5],
				srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);				
				
				if(portComeIn != 0){
					int index;
					index = bitmask_to_id(portComeIn);
					fdb->portUsedNum[index]--;
					if(fdb->portUsedNum[index] <= 0){
						DEBUG_PRINT("portUsedNum[%d] == 0 ,update portlist from (%x)  " ,index ,fdb->portlist);
						fdb->portlist &= ~ portComeIn;
						DEBUG_PRINT("to (%x) \n" ,fdb->portlist);
						
						if(fdb->portUsedNum[index] < 0){
						DEBUG_PRINT("!! portUsedNum[%d]=%d < 0 at (del_igmp_ext_entry)  \n" ,index ,fdb->portUsedNum[index]);
						fdb->portUsedNum[index] = 0;
						}
					}else{
						DEBUG_PRINT("(del) portUsedNum[%d] = %d \n" ,index, fdb->portUsedNum[index]);
					}
				
				}	
				DEBUG_PRINT("\n");
				return ;
			}			
		}
	}

	DEBUG_PRINT("%s:entry not existed!!\n\n", __FUNCTION__);	
}


#endif


