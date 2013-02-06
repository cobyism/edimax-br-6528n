/*
* Copyright c                  Realtek Semiconductor Corporation, 2005
* All rights reserved.
* 
* Program : Internal Header file for fowrading engine rtl8651_layer2fwd.c
* Abstract : 
* Author :  Yi-Lun Chen (chenyl@realtek.com.tw)
*
*/
#ifndef RTL8651_L2FWD_LOCAL_H
#define RTL8651_L2FWD_LOCAL_H

#include "rtl_types.h"
#include "rtl_glue.h"
#include "rtl8651_tblDrvLocal.h"
#include "rtl8651_layer2fwd.h"

/*********************************************************************************************************
	L2 forwarding engine configurations
**********************************************************************************************************/
#define _RTL8651_L2ENG_EXTDEV_MACLEARNING_CACHE	/* Enable extension device SMAC learning cache */

/*********************************************************************************************************
	Main functions
**********************************************************************************************************/
int32 _rtl8651_layer2fwd_init(void);
void _rtl8651_layer2fwd_reinit(void);
void _rtl8651_layer2fwd_timeUpdate(uint32 secPassed);

/*********************************************************************************************************
	Extension device process
**********************************************************************************************************/
#define _RTL8651_EXTDEV_DEVCOUNT		16

/*by qjj*/
#define _RTL8651_EXT_LINK_MASK 0x0000ffff	


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

#define _RTL8651_EXTDEV_BCAST_LINKID	0		/* LinkID 0 is reserved for broadcast */

void _rtl8651_arrangeAllExtDeviceVlanMbr(void);

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

/*********************************************************************************************************
	VLAN process
**********************************************************************************************************/
#define VLAN_TAGGED(mdata)		( *(uint16*)(&((uint8*)mdata)[12]) == htons(0x8100))
#define VLAN_ID(mdata)			((ntohs(*((uint16 *)(mdata + 14))) & 0xe000) & 0x0fff)
#define VLAN_PRIORITY(mdata)	((ntohs(*((uint16 *)(mdata + 14))) & 0xe000) >> 13)


/*********************************************************************************************************
	Normal L2 forwarding process
**********************************************************************************************************/
int32 _rtl8651_fwdEngineSend(	uint32 property,
									void *data,
									int16 dvid,
									int32 iphdrOffset);

int32 _rtl8651_fwdEngineL2Input(	struct rtl_pktHdr *pkthdrPtr,
									rtl8651_tblDrv_vlanTable_t *local_vlanp,
									uint8 *m_data,
									uint16 ethtype);
uint32 _rtl8651_getExtPortLinkMask(uint32 extPortNum);/*by qjj*/
uint32 _rtl8651_mapLinkIdToPortMask(uint32 linkId);/*by qjj*/
uint32 _rtl8651_getVlanLinkMask(uint32 vid);
#endif /* RTL8651_L2FWD_LOCAL_H */
