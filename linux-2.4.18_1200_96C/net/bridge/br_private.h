/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_private.h,v 1.36.2.1 2010/12/14 06:44:50 pluswang Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/miscdevice.h>
#include <linux/if_bridge.h>
#include "br_private_timer.h"
#ifdef CONFIG_RTL8197B_PANA
#undef IGMP_SNOOPING
#else
#define IGMP_SNOOPING
#endif

//#define DEBUG_TRACE
//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) panic_printk(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#ifdef DEBUG_TRACE
#define BDBFENTER		panic_printk("----->%s\n", __FUNCTION__)
#define BDBFEXIT			panic_printk("%s----->\n", __FUNCTION__)
#else
#define BDBFENTER
#define BDBFEXIT
#endif
#if 0 //Brad open for customer turnkey package
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
#undef IGMP_SNOOPING
#endif
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)
#undef IGMP_SNOOPING
#endif
#endif

#ifdef	IGMP_SNOOPING
#define MCAST_TO_UNICAST

//plus test 0508-2009
#define FDB_FINETUNE	
#define IGMP_EXPIRE_TIME (130*HZ)

#ifdef	MCAST_TO_UNICAST
#define IPV6_MCAST_TO_UNICAST
#define IPV6_MULTICAST_MAC(mac) 	   ((mac[0]==0x33)&&(mac[1]==0x33) && mac[2]!=0xff)
#endif

#endif

// interface to set port number of multicast filter
//#define MULTICAST_FILTER
#define MLCST_FLTR_ENTRY	16
#define MLCST_MAC_ENTRY		64
#ifdef IGMP_SNOOPING
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
#define MAX_BR_PORT_NUM		2
#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)
#define MAX_BR_PORT_NUM		2
#else
#define MAX_BR_PORT_NUM		12
#endif
#endif

#ifdef CONFIG_RTK_GUEST_ZONE
	#define MAX_LOCK_CLIENT		32
//	#define DEBUG_GUEST_ZONE
#endif

// interface to set multicast bandwidth control
//#define MULTICAST_BWCTRL

// interface to enable MAC clone function
//#define RTL_BRIDGE_MAC_CLONE
//#define RTL_BRIDGE_DEBUG

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#define BR_HOLD_TIME (1*HZ)

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

#define MULTICAST_MAC(mac) 	   ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
// for fast-nat module, 2005-12-23
//#ifdef NAT_SPEEDUP
//extern unsigned int br_nat_speedup;
//#endif

#ifdef CONFIG_RTK_MESH
#define STP_ADDCOST_ETH

#define MESH_PORTAL_EXPIRE 300 //seconds
#if 0
#ifdef STP_DISABLE_ETH
//Chris: stp+mesh
#define ETH_CHK_INTVL		(30*HZ)
#endif
#endif
#endif

#ifdef RTL_BRIDGE_MAC_CLONE
#define MAC_CLONE_TIMEOUT	300

#if defined (RTL_BRIDGE_DEBUG)
#define	TRACE( fmt, args...)		printk("RT BR: " fmt, ## args);
#else
#define	TRACE( fmt, args...)
#endif
#endif

#ifdef	IGMP_SNOOPING
#define FDB_IGMP_EXT_NUM 8
struct fdb_igmp_ext_entry
{
	int valid;
	unsigned long ageing_time;
	unsigned char SrcMac[6];	
	unsigned char port;

};

struct fdb_igmp_ext_array
{
	struct fdb_igmp_ext_entry igmp_fdb_arr[FDB_IGMP_EXT_NUM];
};

#endif

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
	unsigned char	pad[2];
};

#ifdef RTL_BRIDGE_MAC_CLONE
struct mac_clone_pair
{
	struct mac_addr				mac;
	struct net_bridge_port		*port;
};

extern struct mac_clone_pair		clone_pair;
#endif

#ifdef NAT_SPEEDUP
#include <linux/netdevice.h>
#endif

struct net_bridge_fdb_entry
{
	struct net_bridge_fdb_entry	*next_hash;
	struct net_bridge_fdb_entry	**pprev_hash;
	atomic_t			use_count;
	mac_addr			addr;
	struct net_bridge_port		*dst;
	unsigned long			ageing_timer;
#ifdef IGMP_SNOOPING
	unsigned short		group_src;
	unsigned char			portlist;
	int 					portUsedNum[8];	// be used with portlist, for record each port has how many client
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
	int							index;	
	unsigned char			vlan_id;
	int 			hw_table_enable;
#endif	
	//struct fdb_igmp_ext_array	igmp_ext_array;
	struct fdb_igmp_ext_entry igmp_fdb_arr[FDB_IGMP_EXT_NUM];
	unsigned char ipv6_multicast;
#endif

#ifdef NAT_SPEEDUP
	struct hh_cache		*hh_ptr;
#endif
	unsigned			is_local:1;
	unsigned			is_static:1;
};

struct net_bridge_port
{
	struct net_bridge_port		*next;
	struct net_bridge		*br;
	struct net_device		*dev;
	int				port_no;

	/* STP */
	port_id				port_id;
	int				state;
	int				path_cost;
	bridge_id			designated_root;
	int				designated_cost;
	bridge_id			designated_bridge;
	port_id				designated_port;
	unsigned			topology_change_ack:1;
	unsigned			config_pending:1;
#ifdef RTL_BRIDGE_MAC_CLONE
	unsigned			enable_mac_clone:1;
	unsigned			mac_clone_completed:1;
#endif

#if 0
#ifdef STP_DISABLE_ETH
//Chris:  stp+mesh
	int	 				disable_by_mesh; 
		//0: no, 1:eth port disabled because mesh entering blocking state
	struct br_timer		eth_disable_timer;
#endif
#endif //CONFIG_RTK_MESH
	int				priority;

	struct br_timer			forward_delay_timer;
	struct br_timer			hold_timer;
	struct br_timer			message_age_timer;

#ifdef MULTICAST_BWCTRL
	unsigned int		bandwidth;
	unsigned int		accumulation;
	struct br_timer		bwcrtl_timer;
#endif

#ifdef RTL_BRIDGE_MAC_CLONE
	struct net_bridge_port		*macCloneTargetPort;
	struct br_timer		macClone_timer;
	unsigned int		macClone_expire_to;
#endif

#ifdef CONFIG_RTK_GUEST_ZONE
	int	is_guest_zone;	// 0: host zone, 1: guest zone 
#endif
};

struct net_bridge
{
	struct net_bridge		*next;
	rwlock_t			lock;
	struct net_bridge_port		*port_list;
	struct net_device		dev;
	struct net_device_stats		statistics;
	rwlock_t			hash_lock;
	struct net_bridge_fdb_entry	*hash[BR_HASH_SIZE];
	struct timer_list		tick;

	/* STP */
	bridge_id			designated_root;
	int				root_path_cost;
	int				root_port;
	int				max_age;
	int				hello_time;
	int				forward_delay;
	bridge_id			bridge_id;
	int				bridge_max_age;
	int				bridge_hello_time;
	int				bridge_forward_delay;
	unsigned			stp_enabled:1;
	unsigned			topology_change:1;
	unsigned			topology_change_detected:1;

	struct br_timer			hello_timer;
	struct br_timer			tcn_timer;
	struct br_timer			topology_change_timer;
	struct br_timer			gc_timer;

	int				ageing_time;
	int				gc_interval;

#if defined(IGMP_SNOOPING) 
//	int	   igmp_ageing_time;
//	struct hw_multicast_entry HME[MLCST_MAC_ENTRY];
#endif

#ifdef MULTICAST_FILTER
	int				fltr_portlist_num;
	unsigned short	fltr_portlist[MLCST_FLTR_ENTRY];
	unsigned char	fltr_maclist[MLCST_MAC_ENTRY][4];
#endif

#ifdef CONFIG_RTK_GUEST_ZONE
	int	is_guest_isolated;	// isolate guest when 1
	int	is_zone_isolated;	// isolate host and guest zone when 1
	int	lock_client_num;
	unsigned char lock_client_list[MAX_LOCK_CLIENT][6];	
#endif

#ifdef CONFIG_RTK_MESH
	//by brian, record pid for dynamic enable portal
	int mesh_pathsel_pid;
	int eth0_received;

	int	eth0_monitor_interval;
	struct br_timer	eth0_monitor_timer;

#if 0
#ifdef STP_DISABLE_ETH
	struct br_timer	eth0_autostp_timer;
#endif
#endif

#ifdef STP_ADDCOST_ETH
	int is_cost_changed;
#endif

#endif //CONFIG_RTK_MESH
};

extern struct notifier_block br_device_notifier;
extern unsigned char bridge_ula[6];

/* br.c */
extern void br_dec_use_count(void);
extern void br_inc_use_count(void);

/* br_device.c */
extern void br_dev_setup(struct net_device *dev);
extern int br_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/* br_fdb.c */
extern void br_fdb_changeaddr(struct net_bridge_port *p,
		       unsigned char *newaddr);
extern void br_fdb_cleanup(struct net_bridge *br);
extern void br_fdb_delete_by_port(struct net_bridge *br,
			   struct net_bridge_port *p);
extern struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					unsigned char *addr);
extern void br_fdb_put(struct net_bridge_fdb_entry *ent);
extern int  br_fdb_get_entries(struct net_bridge *br,
			unsigned char *_buf,
			int maxnum,
			int offset
#ifdef CONFIG_RTK_GUEST_ZONE
			,int for_guest
#endif		       
			);
extern void br_fdb_insert(struct net_bridge *br,
		   struct net_bridge_port *source,
		   unsigned char *addr,
		   int is_local);

#ifdef	IGMP_SNOOPING
extern int chk_igmp_ext_entry(
	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac);

extern void add_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn);

extern void update_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn);

void del_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn);
#endif

/* br_forward.c */
extern void br_deliver(struct net_bridge_port *to,
		struct sk_buff *skb);
extern void br_forward(struct net_bridge_port *to,
		struct sk_buff *skb);
extern void br_flood_deliver(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);
extern void br_flood_forward(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);
#ifdef IGMP_SNOOPING
void br_multicast_deliver(struct net_bridge *br,
			struct net_bridge_fdb_entry *dst, 
			struct sk_buff *skb,
			int clone);
void br_multicast_forward(struct net_bridge *br,
                        struct net_bridge_fdb_entry *dst,
                        struct sk_buff *skb,
                        int clone);
extern int bitmask_to_id(unsigned char val);
#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
	extern unsigned char add_ip_multicast_tbl(int src_ip, int dst_ip, int src_vlan, char src_port, char mem_port);
	extern int update_ip_multicast_tbl(int index, unsigned char mem_port, int valid);	
	extern int rtl8651_delAsicIpMulticastTable(int index);
#endif	
#endif

/* br_if.c */
extern int br_add_bridge(char *name);
extern int br_del_bridge(char *name);
extern int br_add_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_del_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_get_bridge_ifindices(int *indices,
			    int num);
extern void br_get_port_ifindices(struct net_bridge *br,
			   int *ifindices);
#ifdef RTL_BRIDGE_MAC_CLONE
extern int br_mac_clone(struct net_bridge_port *p,
			unsigned char *addr);
extern int br_mac_clone_handle_frame(struct net_bridge *br,
			struct net_bridge_port *p,
			struct sk_buff *skb);
extern void br_mac_clone_unlink(struct net_bridge_port *port);
#endif
#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
extern int br_initial_port_cost(struct net_device *dev);
#endif



/* br_input.c */
extern void br_handle_frame(struct sk_buff *skb);
#if	0	//def	CONFIG_RTK_PORT_HW_IGMPSNOOPING
extern int br_portlist_update(	unsigned int IP , int port ,int sourceRM);
#endif

/* br_ioctl.c */
extern void br_call_ioctl_atomic(void (*fn)(void));
extern int br_ioctl(struct net_bridge *br,
	     unsigned int cmd,
	     unsigned long arg0,
	     unsigned long arg1,
	     unsigned long arg2);
extern int br_ioctl_deviceless_stub(unsigned long arg);

/* br_stp.c */
extern int br_is_root_bridge(struct net_bridge *br);
extern struct net_bridge_port *br_get_port(struct net_bridge *br,
				    int port_no);
extern void br_init_port(struct net_bridge_port *p);
extern port_id br_make_port_id(struct net_bridge_port *p);
extern void br_become_designated_port(struct net_bridge_port *p);

/* br_stp_if.c */
extern void br_stp_enable_bridge(struct net_bridge *br);
extern void br_stp_disable_bridge(struct net_bridge *br);
extern void br_stp_enable_port(struct net_bridge_port *p);
extern void br_stp_disable_port(struct net_bridge_port *p);
extern void br_stp_recalculate_bridge_id(struct net_bridge *br);
extern void br_stp_set_bridge_priority(struct net_bridge *br,
				int newprio);
extern void br_stp_set_port_priority(struct net_bridge_port *p,
			      int newprio);
extern void br_stp_set_path_cost(struct net_bridge_port *p,
			  int path_cost);
extern void br_stp_assign_bridge_id(struct net_bridge *br, unsigned char *addr);

/* br_stp_bpdu.c */
extern void br_stp_handle_bpdu(struct sk_buff *skb);

#endif
