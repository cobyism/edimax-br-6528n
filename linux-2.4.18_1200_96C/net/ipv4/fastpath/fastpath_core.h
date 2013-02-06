#ifndef	__FASTPATH_CORE_H__
#define	__FASTPATH_CORE_H__
#include <linux/config.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/dst.h>
#include <net/route.h>

#include "./rtl_queue.h"			/* X-Queue Marco Function */


/*
	Virtual Rome Driver API & System (Light Rome Driver Simulator)
*/
#ifdef CONFIG_FAST_PATH_MODULE
#define	__IRAM_GEN	
#else
#define	__IRAM_GEN	__attribute__	((section(".speedup")))
#endif
#define	ipaddr_t		__u32
#define	uint8		__u8
#define	uint16		__u16
#define	uint32		__u32
/* ---------------------------------------------------------------------------------------------------- */

#define	IFNAME_LEN_MAX		16
#define	MAC_ADDR_LEN_MAX		18
#define	ARP_TABLE_LIST_MAX		32
#define	ARP_TABLE_ENTRY_MAX	128
#define	ROUTE_TABLE_LIST_MAX	16
#define	ROUTE_TABLE_ENTRY_MAX	64

#ifndef CONFIG_RTL_KERNEL_MIPS16_FASTPATH
#define	NAPT_TABLE_LIST_MAX	850
#define	NAPT_TABLE_ENTRY_MAX	850
#define	PATH_TABLE_LIST_MAX	850
#else
#define	NAPT_TABLE_LIST_MAX	128
#define	NAPT_TABLE_ENTRY_MAX	128
#define	PATH_TABLE_LIST_MAX	128
#endif

#define	PATH_TABLE_ENTRY_MAX	(NAPT_TABLE_ENTRY_MAX * 2)
#define	INTERFACE_ENTRY_MAX	8

#define	ETHER_ADDR_LEN		6
typedef struct ether_addr_s {
        uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

/* ########### API #################################################################################### */
enum LR_RESULT
{
	/* Common error code */
	LR_SUCCESS = 0,						/* Function Success */
	LR_FAILED = -1,						/* General Failure, not recommended to use */
	LR_ERROR_PARAMETER = -2,				/* The given parameter error */
	LR_EXIST = -3,							/* The entry you want to add has been existed, add failed */
	LR_NONEXIST = -4,						/* The specified entry is not found */
	
	LR_NOBUFFER = -1000,					/* Out of Entry Space */
	LR_INVAPARAM = -1001,					/* Invalid parameters */
	LR_NOTFOUND = -1002,					/* Entry not found */
	LR_DUPENTRY = -1003,					/* Duplicate entry found */
};

#if 0
enum IF_FLAGS
{
	IF_NONE,
	IF_INTERNAL = (0<<1),					/* This is an internal interface. */
	IF_EXTERNAL = (1<<1),					/* This is an external interface. */
};

enum FDB_FLAGS
{
	FDB_NONE = 0,
};
#endif

enum ARP_FLAGS
{
	ARP_NONE = 0,
};

enum RT_FLAGS
{
	RT_NONE = 0,
};

enum SE_TYPE
{
	SE_PPPOE = 1,
	SE_PPTP = 2,
	SE_L2TP = 3,
};
enum SE_FLAGS
{
	SE_NONE = 0,
};

enum NP_PROTOCOL
{
	NP_UDP = 1,
	NP_TCP = 2,
};
enum NP_FLAGS
{
	NP_NONE = 0,
};

/* ---------------------------------------------------------------------------------------------------- */
#if 0
enum LR_RESULT rtk_addInterface( uint8* ifname, ipaddr_t ipAddr, ether_addr_t* gmac, uint32 mtu, enum IF_FLAGS flags );
enum LR_RESULT rtk_configInterface( uint8* ifname, uint32 vlanId, uint32 fid, uint32 mbr, uint32 untag, enum IF_FLAGS flags );
enum LR_RESULT rtk_delInterface( uint8* ifname );
enum LR_RESULT rtk_addFdbEntry( uint32 vid, uint32 fid, ether_addr_t* mac, uint32 portmask, enum FDB_FLAGS flags );
enum LR_RESULT rtk_delFdbEntry( uint32 vid, uint32 fid, ether_addr_t* mac );
#endif
#ifdef CONFIG_FAST_PATH_MODULE

extern enum LR_RESULT (*FastPath_hook2)( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
extern enum LR_RESULT (*FastPath_hook1)( ipaddr_t ip, ipaddr_t mask );
extern int (*fast_path_hook)(struct sk_buff **pskb) ;
extern enum LR_RESULT (*FastPath_hook3)( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
extern  enum LR_RESULT (*FastPath_hook4)( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                               ipaddr_t extIp, uint32 extPort,
                                                               ipaddr_t remIp, uint32 remPort );
extern enum LR_RESULT (*FastPath_hook5)( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
extern enum LR_RESULT (*FastPath_hook6)( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                               ipaddr_t extIp, uint32 extPort,
                                                               ipaddr_t remIp, uint32 remPort,
                                                               enum NP_FLAGS flags);
extern enum LR_RESULT (*FastPath_hook7)( ipaddr_t ip );
extern enum LR_RESULT (*FastPath_hook8)( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
extern enum LR_RESULT (*FastPath_hook11)(enum NP_PROTOCOL protocol,
		ipaddr_t intIp, uint32 intPort,
		ipaddr_t extIp, uint32 extPort,
		ipaddr_t remIp, uint32 remPort,
		uint32 interval, uint32 *entry_last_used);
extern int (*FastPath_hook9)( void );
extern int Get_fast_pptp_fw(void);
extern int (*FastPath_hook10)(struct sk_buff *skb);	
enum LR_RESULT rtk_addSession( uint8* ifname, enum SE_TYPE seType, uint32 sessionId, enum SE_FLAGS flags );
enum LR_RESULT rtk_delSession( uint8* ifname );	


#else

enum LR_RESULT rtk_addArp( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
enum LR_RESULT rtk_modifyArp( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags );
enum LR_RESULT rtk_delArp( ipaddr_t ip );
enum LR_RESULT rtk_addRoute( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
enum LR_RESULT rtk_modifyRoute( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );
enum LR_RESULT rtk_delRoute( ipaddr_t ip, ipaddr_t mask );
enum LR_RESULT rtk_addSession( uint8* ifname, enum SE_TYPE seType, uint32 sessionId, enum SE_FLAGS flags );
enum LR_RESULT rtk_delSession( uint8* ifname );
enum LR_RESULT rtk_addNaptConnection( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                               ipaddr_t extIp, uint32 extPort,
                                                               ipaddr_t remIp, uint32 remPort,
                                                               enum NP_FLAGS flags);
enum LR_RESULT rtk_delNaptConnection( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                               ipaddr_t extIp, uint32 extPort,
                                                               ipaddr_t remIp, uint32 remPort );
enum LR_RESULT rtk_idleNaptConnection(enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                              ipaddr_t extIp, uint32 extPort,
                                                              ipaddr_t remIp, uint32 remPort,
                                                              uint32 interval,uint32 *entry_last_used);//mark_add
#endif 
/* [MARCO FUNCTION] ========================================================================= */
#define	MAC2STR(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[4], \
	((unsigned char *)&addr)[5]

#define	FASTPATH_MAC2STR(mac, hbuffer) \
	do { \
		int j,k; \
		const char hexbuf[] =  "0123456789ABCDEF"; \
		for (k=0,j=0;k<MAC_ADDR_LEN_MAX && j<6;j++) { \
			hbuffer[k++]=hexbuf[(mac->octet[j]>>4)&15 ]; \
			hbuffer[k++]=hexbuf[mac->octet[j]&15     ]; \
			hbuffer[k++]=':'; \
		} \
		hbuffer[--k]=0; \
	} while(0)	/* Mac Address to String */

#define FASTPATH_ADJUST_CHKSUM_NAT(ip_mod, ip_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_NPT(port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */


#define FASTPATH_ADJUST_CHKSUM_NAPT(ip_mod, ip_org, port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

/* ---------------------------------------------------------------------------------------------------- */
uint8 *FastPath_Route(ipaddr_t dIp);
int FastPath_Enter(struct sk_buff **skb);
int FastPath_Track(struct sk_buff *skb);
/* ---------------------------------------------------------------------------------------------------- */

#endif	/* __FASTPATH_CORE_H__ */

