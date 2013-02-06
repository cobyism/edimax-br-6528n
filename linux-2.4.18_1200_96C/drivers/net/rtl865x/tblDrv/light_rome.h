

#ifndef __LIGHT_ROME__
#define __LIGHT_ROME__

#include "rtl_queue.h"

#define RTL8651_TBLDRV_LOCAL_H
#include "rtl8651_aclLocal.h"


#define LR_INIT_CHECK(expr) do {\
	if(((int32)expr)!=SUCCESS){\
		lr_printf("Error >>> %s:%d failed !\n", __FUNCTION__,__LINE__);\
			return FAILED;\
	}\
	}while(0)

#ifdef RTL865X_TEST
	#define lr_printf						printf
#else
	#define lr_printf						printk
#endif


#ifdef CONFIG_RTL865XC
#define GET_IF_VID(ifa)					(TBLFIELD(vlan_tbl, vhash)[rtl8651_vlanTableIndex((ifa)->vid)].fid)
#else
#define GET_IF_VID(ifa)					(TBLFIELD(vlan_tbl, vhash)[(ifa)->vid].fid)
#endif
#define LR_CONFIG_CHECK					LR_INIT_CHECK
#define VLAN_NUMBER						4096
#define NETIF_NUMBER						4
/* for FDB */
#define FDB_STATIC						0x01		/* flag for FDB: process static entry only */
#define FDB_DYNAMIC						0x02		/* flag for FDB: process dynamic entry only */

#define TBLFIELD(tbl, field)					((tbl).field)
#ifdef CONFIG_RTL865XC
#define L2_AGING_TIME					450
#else
#define L2_AGING_TIME					300
#endif

/* NAT timeout value */
#define TCP_TIMEOUT						60	 	/* 60 secs */
#define UDP_TIMEOUT						30		/* 30 secs */

/* ACL Numbers */
#define RTL865x_ACL_NUMBER				128
#define DEFAULT_INC						124
#define SET_DEFAULT_ACL(idx, action)					\
{													\
	_rtl8651_tblDrvAclRule_t def_rule;					\
	memset(&def_rule, 0, sizeof(_rtl8651_tblDrvAclRule_t));	\
	def_rule.actionType_  = action;						\
	TBLFIELD(acl_tbl, acl_asic_set)((idx), &def_rule);		\
}






/* for dowhat field of rt_arp_sync() */
enum SYNC_FLAG
{
	SYNC_ADD = 0,
	SYNC_DEL = 1,
};


struct lr_cpu_stats {
	uint32				rx_packets;
	uint32				rx_bytes;
	uint32				rx_drop;
	uint32				rx_mcast;
	uint32				rx_bcast;

	uint32				tx_packets;
	uint32				tx_bytes;
	uint32				tx_drop;
	uint32				tx_mcast;
	uint32				tx_bcast;

	uint32				br_packets;
};



struct if_entry {
	uint8				name[MAX_IFNAMESIZE];
#ifdef CONFIG_RTL865XC
	int8                            isWan;
#endif
	uint16 				vid;
	//uint16				fid;
	uint16				mtu;
	//ether_addr_t			mac;
	uint16				up;
	enum IF_TYPE			if_type;
	
	union {
		struct {
			ipaddr_t		ipaddr;
			ipaddr_t		mask;
			uint16		arp_start;
			uint16		arp_end;
		} eth;
		struct {
			uint16		sid;
			ether_addr_t	pmac;
		} pppoe;
	} un;
	
#define IF_UP(ife)			(ife->up)
#define ipaddr_			un.eth.ipaddr
#define mask_			un.eth.mask
#define arp_start_			un.eth.arp_start
#define arp_end_			un.eth.arp_end
#define sid_				un.pppoe.sid
#define pmac_			un.pppoe.pmac
};


struct if_table {
	struct if_entry				if_hash[NETIF_NUMBER];
	struct if_entry *				(*if_lookup)(uint8 *, ipaddr_t);
	int32 						(*if_init)(void);
	int32						(*if_attach)(struct rtl865x_lrConfig *);
	int32						(*if_detach)(uint8 *);
	int32						(*if_up)(uint8 *, ipaddr_t, ipaddr_t, uint32, ether_addr_t *);
	int32						(*if_down)(uint8 *);
};


struct nat_host_info {
	ipaddr_t						ip;
	uint16						port;
};

struct nat_tuple {
	struct nat_host_info			int_host;
	struct nat_host_info			ext_host;
	struct nat_host_info			rem_host;
	enum NP_PROTOCOL			proto;
};



struct nat_entry {
	struct nat_tuple				tuple_info;

	uint32						natip_idx;
	uint32						in;
	uint32						out;
	uint32 						flags;

#define int_ip_					tuple_info.int_host.ip
#define int_port_					tuple_info.int_host.port
#define ext_ip_					tuple_info.ext_host.ip
#define ext_port_					tuple_info.ext_host.port
#define rem_ip_					tuple_info.rem_host.ip
#define rem_port_					tuple_info.rem_host.port
#define proto_					tuple_info.proto

};



#define NAT_INBOUND				0x00000001
#define NAT_OUTBOUND			0x00000002
#define NAT_INUSE(n)				( ((n)->flags&(NAT_INBOUND|NAT_OUTBOUND)) )
#define SET_NAT_FLAGS(n, v)		(n)->flags |= v


struct nat_table {

	uint32						tcp_timeout;
	uint32						udp_timeout;
	struct nat_entry 				nat_bucket[RTL8651_TCPUDPTBL_SIZE];

	struct nat_entry *				(*nat_lookup)(struct nat_tuple *);
	int32						(*nat_init)(void);

	uint32						(*nat_asic_hash)(int8, ipaddr_t, uint16, ipaddr_t, uint16);
	int32						(*nat_asic_set)(int8, uint32, rtl865x_tblAsicDrv_naptTcpUdpParam_t *);
	int32						(*nat_asic_get)(uint32, rtl865x_tblAsicDrv_naptTcpUdpParam_t *);
	int32						(*nat_asic_del)(uint32, uint32);
	int32						(*nat_asic_tcpageL)(uint32);
	int32						(*nat_asic_tcpageM)(uint32);
	int32						(*nat_asic_tcpageF)(uint32);
	int32						(*nat_asic_udpage)(uint32);
};


struct natip_table {

	uint32						natip_no;
	rtl865x_tblAsicDrv_extIntIpParam_t __natipbuff;
	rtl865x_tblAsicDrv_extIntIpParam_t * (*natip_lookup)(ipaddr_t, uint32 *);
	int32						(*natip_init)(void);
	int32						(*natip_add)(ipaddr_t);
	int32						(*natip_del)(ipaddr_t);
	int32						(*natip_asic_set)(uint32, rtl865x_tblAsicDrv_extIntIpParam_t *);
	int32						(*natip_asic_get)(uint32, rtl865x_tblAsicDrv_extIntIpParam_t *);
	int32						(*natip_asic_del)(uint32);
#ifndef CONFIG_RTL865XC
	int32						(*gidx_asic_set)(uint32);
	int32						(*gidx_asic_get)(uint32 *);
#endif
};


struct rt_table {

	ipaddr_t						pendgw[RTL8651_ROUTINGTBL_SIZE];
	rtl865x_tblAsicDrv_routingParam_t __l3buff;
	rtl865x_tblAsicDrv_routingParam_t * (*route_lookup)(ipaddr_t, ipaddr_t, uint32 *);
	enum LR_RESULT				(*route_add)(ipaddr_t, ipaddr_t, ipaddr_t, uint8 *, enum RT_FLAGS);
	enum LR_RESULT				(*route_del)(ipaddr_t, ipaddr_t);
	int32						(*route_init)(void);

	int32						(*rt_asic_set)(uint32, rtl865x_tblAsicDrv_routingParam_t *);
	int32						(*rt_asic_get)(uint32, rtl865x_tblAsicDrv_routingParam_t *);
	int32						(*rt_asic_del)(uint32);
};


struct arp_table {

	uint8						arp_mask[64];
	rtl865x_tblAsicDrv_arpParam_t	__arpbuff;
	
	rtl865x_tblAsicDrv_arpParam_t *	(*arp_lookup)(ipaddr_t);
	enum LR_RESULT				(*arp_add)(ipaddr_t, ether_addr_t *, enum ARP_FLAGS);
	enum LR_RESULT				(*arp_del)(ipaddr_t);
	int32						(*arp_init)(void);
	int32						(*arp_hash)(ipaddr_t);
	int32						(*arp_tbl_alloc)(struct if_entry *);
	int32						(*arp_tbl_free)(struct if_entry *);

	int32						(*arp_asic_set)(uint32, rtl865x_tblAsicDrv_arpParam_t *);
	int32						(*arp_asic_get)(uint32, rtl865x_tblAsicDrv_arpParam_t *);
	int32						(*arp_asic_del)(uint32);
};


struct pppoe_table {
	
	struct if_entry				*ppps[RTL8651_PPPOETBL_SIZE];
	struct if_entry *				(*pppoe_lookup)(uint32, uint32 *);
	int32						(*pppoe_init)(void);
	
	int32						(*pppoe_asic_set)(uint32 ,rtl865x_tblAsicDrv_pppoeParam_t *);
	int32						(*pppoe_asic_get)(uint32, rtl865x_tblAsicDrv_pppoeParam_t *);
};


struct vlan_entry {
	uint32						valid;
	uint32						mbr;
	uint32 						untagSet;
	uint32						fid;
	ether_addr_t					mac;
	uint32                                         vid;
};

struct vlan_table {
	struct vlan_entry				vhash[VLAN_NUMBER];
	uint16						asic_vtbl[RTL8651_VLAN_NUMBER];

	/* Software Table Interface */
	int32						(*vlan_init)(void);
	int32						(*vlan_create)(struct rtl865x_lrConfig *);
	int32						(*vlan_remove)(uint32);
	int32						(*vlan_hash)(uint16);

	/* ASIC Table Interface */
	int32 						(*vlan_asic_get)(uint16, rtl865x_tblAsicDrv_vlanParam_t *);
#ifdef CONFIG_RTL865XC
	int32 						(*vlan_asic_set)(uint16 , rtl865x_tblAsicDrv_vlanParam_t *); 
#else
	int32 						(*vlan_asic_set)(rtl865x_tblAsicDrv_vlanParam_t *);
#endif
	int32						(*vlan_asic_del)(uint16);

#ifdef CONFIG_RTL865XC
	int32 						(*intf_asic_set)( uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp );
	int32 						(*intf_asic_get)( uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp );
#endif
};

struct acl_table {
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s)	free_head;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s)	acl_in_head[RTL8651_VLAN_NUMBER];
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s)	acl_eg_head[RTL8651_VLAN_NUMBER];
	
	_rtl8651_tblDrvAclRule_t				free_list[RTL865x_ACL_NUMBER];
	enum ACL_FLAGS						def_action;
	uint32								entry;

	int32						(*acl_init)(void);
	int32						(*acl_add)(uint32, _rtl8651_tblDrvAclRule_t *, enum ACL_FLAGS);
	int32						(*acl_del)(uint32, _rtl8651_tblDrvAclRule_t *, enum ACL_FLAGS);
	int32						(*acl_arrange)(void);
	
	int32						(*acl_asic_set)(uint32, _rtl8651_tblDrvAclRule_t *);
	int32						(*acl_asic_get)(uint32, _rtl8651_tblDrvAclRule_t *);
};

struct fdb_table {

	uint8						FDB[RTL8651_L2_NUMBER];
	uint8						LinkID[RTL8651_L2_NUMBER];
	rtl865x_tblAsicDrv_l2Param_t	__l2buff;
	rtl865x_tblAsicDrv_l2Param_t *	(*fdb_lookup)(uint32, ether_addr_t *, uint32, uint32 *);
	int32						(*fdb_init)(void);
	int32						(*bridge_rcv)(struct rtl_pktHdr *);
	enum LR_RESULT				(*fdb_add)(uint32, ether_addr_t *, uint32, enum FDB_FLAGS);
	enum LR_RESULT				(*fdb_del)(uint32, ether_addr_t *);
#ifdef CONFIG_RTL865XC
	uint32						(*fdb_asic_hash)(ether_addr_t *, uint16);
#else
	uint32						(*fdb_asic_hash)(ether_addr_t *);
#endif
	int32 						(*fdb_asic_get)(uint32, uint32, rtl865x_tblAsicDrv_l2Param_t *);
	int32 						(*fdb_asic_set)(uint32, uint32, rtl865x_tblAsicDrv_l2Param_t *);
	int32 						(*fdb_asic_del)(uint32, uint32);
};

struct port_attribute {
	uint32						activePortMask;
	uint8						ing_filter[RTL8651_PORT_NUMBER];
#if 0	
	uint8						acc_framtype[RTL8651_PORT_NUMBER];
								/* 00: Admit All Frames, 01: Admit Only VLAN-tagged frames 
								 * 11: Admit Only Untagged and Priority-Tagged frames
								 */								
#define ADMIT_ALL				0x00
#define ADMIT_TAG_ONLY			0x01
#define ADMIT_UNTAG_ONLY		0x02
#endif

	void							(*link_change)(uint32, int8);
	int32						(*pvid_asic_set)(uint32, uint32);
	int32						(*pvid_asic_get)(uint32, uint32 *);
};


struct vlan_entry*  lr_get_vlan(uint32 vid);
struct vlan_entry*  lr_get_vlan_byidx(uint32 vidx);
#endif /* __LIGHT_ROME__ */


