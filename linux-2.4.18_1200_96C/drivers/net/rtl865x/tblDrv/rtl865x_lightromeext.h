/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program : Header file for  light rome rtl865x_lightromeext.c
* Abstract : 
* Author :  qy_wang(qy_wang@realsil.com.cn)
*
*/
#include "rtl_types.h"

/*********************************************************************************************************
	L2 forwarding engine configurations
**********************************************************************************************************/
//#undef  _RTL8651_L2ENG_EXTDEV_MACLEARNING_CACHE	/* Enable extension device SMAC learning cache */
#define  _RTL8651_L2ENG_EXTDEV_MACLEARNING_CACHE	/* Enable extension device SMAC learning cache */

#define RTL_MSG_EXTDEV					(1<<4)

/*********************************************************************************************************
	Extension device process
**********************************************************************************************************/
#define _RTL8651_EXTDEV_DEVCOUNT		16

/*by qjj*/
#define _RTL8651_EXT_LINK_MASK 0x0000ffff	
#define RTL8651_EXTDEV_VLANPROPER_ISMBR			0x01
#define RTL8651_EXTDEV_VLANPROPER_NOTMBR		0x02


/* we translate idx and linkID using these MACROs : to make sure LINKID > 0 (0 is reserved for broadcast) */
#define _RTL8651_EXTDEV_IDX2LINKID(idx)	((idx) + 1)
#define _RTL8651_EXTDEV_LINKID2IDX(linkId)	((linkId) - 1)


typedef struct _rtl8651_extDevice_s {

#if defined(STP_PATCH)
       uint32 portStat;           /*portStat for STP*/
	uint32 defaultVid;
#endif
	uint32 linkID;			/* LinkID of this extension device */
	uint32 portMask;		/* port mask of this extension device  */
	void *extDevice;		/* user own pointer for specific extension device */
} _rtl8651_extDevice_t;

/*********************************************************************************************************
	Extension device MAC learning cache
**********************************************************************************************************/

/*
	We use 64 entries to cache extension device smac learning result :
		It should be power of 2, and this value SHOULD NOT LARGER THAN 254,
		or the simple hash algorithm should be modified.
*/
#define _RTL8651_EXTDEV_MACLEARNING_CACHE_COUNT	64

typedef struct _rtl8651_extDevMacLearningCache_s {
	uint8		valid;			/* VALID bit */
	ether_addr_t	mac;			/* MAC */
	uint16		vid;				/* SVID */
	uint32		portMask;		/* Source Port Mask */
	uint32		linkId;			/* Source Link ID */
} _rtl8651_extDevMacLearningCache_t;


#define _RTL8651_EXTDEV_BCAST_LINKID	0		/* LinkID 0 is reserved for broadcast */

#define _RTL865XB_EXTPORTMASKS   7
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */

#define DONT_FRAG(off)				(ntohs((uint16)(off)) & (IP_DF))
#define IS_FRAGMENT(off)				(ntohs((uint16)(off)) & (IP_OFFMASK|IP_MF))
#define IS_FRAGMENT_FIRST(off)		((ntohs((uint16)(off)) & (IP_OFFMASK|IP_MF)) == IP_MF)
#define IS_FRAGMENT_REMAIN(off)	(ntohs((uint16)(off)) & IP_OFFMASK)

/* ip header length related */
#define IPHDRLEN_IS_TWENTY(iphdr)		(((*(uint8*)iphdr)&0xf) == 5)					/* check IP header Length is 20 or not */
#define IPHDR_LEN(iphdr)					(((*(uint8*)iphdr)&0xf) << 2)					/* get the ip length */
#define L4HDR_ADDR(iphdr)				((uint8 *)iphdr + (((*(uint8*)iphdr)&0xf) << 2))	/* calculate L4 header address by ip header */
#define TCPDATA_ADDR(tcpHdr)			((uint32)tcpHdr + (uint32)((((struct tcphdr*)tcpHdr)->th_off_x & 0xf0) >> 2))	/* calculate TCP Data address by tcp header */

#ifndef RTL865X_DEBUG
#define assert(expr) do {} while (0)
#else
#define assert(expr) \
        if(!(expr)) {					\
        rtlglue_printf( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
        __FILE__,__LINE__,#expr);		\
        }
#endif

/* Checksum adjustment */
#define	_RTL8651_ADJUST_CHECKSUM(acc, cksum) \
	do { \
		acc += ntohs(cksum); \
		if (acc < 0) { \
			acc = -acc; \
			acc = (acc >> 16) + (acc & 0xffff); \
			acc += acc >> 16; \
			cksum = htons((uint16) ~acc); \
		} else { \
			acc = (acc >> 16) + (acc & 0xffff); \
			acc += acc >> 16; \
			cksum = htons((uint16) acc); \
		} \
	} while (0)
#define _RTL8651_TTL_DEC_1(ip_ttl, ip_sum) \
	do { \
		int32 ttl_mod = 0x0100; \
		(ip_ttl) --; \
		_RTL8651_ADJUST_CHECKSUM(ttl_mod, (ip_sum)); \
	} while(0)

/* Internet address.  */
typedef uint32 in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };
struct ip {

#if 0 
union{
   uint8 _vhl;   /* version << 4 | header length >> 2 */
#ifdef _LITTLE_ENDIAN
	struct {
	  uint8  _hl:4,  /* header length */
	   _ver:4;	/* version */
	}s;
#else
	struct {
	  uint8 _ver:4,  /* version */
	  _hl:4;/* header length */
	}s;
#endif
 } vhl;

#define ip_vhl vhl._vhl
#define ip_hl  vhl.s._hl
#define ip_ver vhl.s._ver
#else
	/* replace bit field */
	uint8 ip_vhl;
#endif 

	uint8	ip_tos;			/* type of service */
	uint16	ip_len;			/* total length */
	uint16	ip_id;			/* identification */
	uint16	ip_off;			/* fragment offset field */
	uint8	ip_ttl;			/* time to live */
	uint8	ip_p;			/* protocol */
	uint16	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

struct udphdr
{
	uint16    uh_sport;				   /* source port */
	uint16    uh_dport;				   /* destination port */
	uint16    uh_ulen;				   /* udp length */
	uint16    uh_sum;				   /* udp checksum */
};

/* AF_INET Supported IP Protocols*/
#define IPPROTO_ICMP 1
#define IPPROTO_IGMP 2
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17

/*********************************************************************************************************
	VLAN process
**********************************************************************************************************/
#define VLAN_TAGGED(mdata)		( *(uint16*)(&((uint8*)mdata)[12]) == htons(0x8100))
#define VLAN_ID(mdata)			((ntohs(*((uint16 *)(mdata + 14))) & 0xe000) & 0x0fff)
#define VLAN_PRIORITY(mdata)	((ntohs(*((uint16 *)(mdata + 14))) & 0xe000) >> 13)

int32 rtl8651_fwdEngineRegExtDevice(	uint32 portNumber,
											uint16 defaultVID,
											uint32 *linkID_p,
											void *extDevice);
int32 rtl8651_fwdEngineUnregExtDevice(uint32 linkID);
int32 rtl8651_extDeviceinit(void);
int32 rtl8651_fwdEngineGetLinkIDByExtDevice(void *extDevice);
int32 rtl8651_fwdEngineSetExtDeviceVlanProperty(uint32 linkID, uint16 vid, uint32 property);

__IRAM_EXTDEV int32 rtl8651_fwdEngineExtPortRecv(	void *id,
														uint8 *data,
														uint32 len,
														uint16 myvid,
														uint32 myportmask,
														uint32 linkID);
__IRAM_L2_FWD int32 rtl8651_txPktPostProcessing(struct rtl_pktHdr * pPkt);
int32 rtl8651_fwdEngineInput(void * pkthdr);
int32 rtl8651_fwdEngineOutput(void * pkthdr, uint32 vid);
int32 rtl8651_getSourceLinkID(void * pkthdr, int32 *linkID);
int32 rtl8651_fwdEngineExtUnicat(void * pkthdr);
int32 rtl865x_bridge_rxProcess(void *pkthdr);


#ifdef _RTL8651_L2ENG_EXTDEV_MACLEARNING_CACHE
void rtl8651_LRtimeUpdate(uint32 dummy);
#endif
