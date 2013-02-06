/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program : Patching Switch core table driver for RTL865xC
* Abstract : 
* Author : Yi-Lun Chen (chenyl@realtek.com.tw)               
*
*/


#ifndef RTL865XC_TBLDRV_PATCH_H
#define RTL865XC_TBLDRV_PATCH_H

#include "rtl8651_tblDrvLocal.h"

#define __INLINE__ inline

int32 RTL8651_SETASICVLAN( rtl8651_tblDrv_vlanTable_t *vt );
#if defined (RTL865XB_DCUT_SWVLAN) || defined(CONFIG_RTL865XC)
int32 RTL865XC_SETASICVLAN ( rtl8651_tblDrv_swVlanTable_t *swVlanPtr );
#endif
__INLINE__ int32 convert_getAsicVlan(	uint16 vid, ether_addr_t * mac, uint32 * mbr,
										uint32 * inAclStart, uint32 * inAclEnd, uint32 * outAclStart,
										uint32 * outAclEnd, int8 * internalIntf, int8 * enableRoute,
										int8 *portState, int8 * broadcastToCpu, int8 * promiscuous, 
										uint32 * untagPortMask, uint32 * macNumber, uint32 * mtu );
__INLINE__ int32 convert_setAsicPppoe( uint32 index, uint16 sid );
__INLINE__ int32 convert_getAsicPppoe( uint32 index, uint16 *sid );
__INLINE__ int32 convert_setAsicExtIntIpTable(	uint32 ipIdx, ipaddr_t ExtIp, ipaddr_t IntIp,
												uint32 localPublic, uint32 nat);
__INLINE__ int32 convert_getAsicExtIntIpTable(	uint32 index, ipaddr_t *ExtIp, ipaddr_t *IntIp, 
												int8 *localPublic, int8 *nat );
__INLINE__ int32 convert_setAsicArp( uint32 ArpPos, uint32 ArpIdx, uint32 asicPos );
__INLINE__ int32 convert_getAsicArp( uint32 index, uint32 *nextHopRow, uint32 *nextHopColumn );
__INLINE__ int32 convert_setAsicRouting(	uint32 idx, ipaddr_t ipAddr, ipaddr_t ipMask,
											uint32 process, uint32 vidx, uint32 arpStart, uint32 arpEnd,
											uint32 nextHopRow, uint32 nextHopColumn, uint32 pppoeIdx );
__INLINE__ int32 convert_getAsicRouting(	uint32 index, ipaddr_t * ipAddr, ipaddr_t * ipMask, uint32 * process, //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 
											uint32 * vidx, uint32 * arpStart, uint32 * arpEnd, 
											uint32 * nextHopRow, uint32 * nextHopColumn, uint32 * pppoeIdx) ;
__INLINE__ int32 convert_setAsicServerPortTable(	uint32 index, ipaddr_t ExtIp, uint16 ExtPort,
													ipaddr_t IntIp, uint16 IntPort );
__INLINE__ int32 convert_getAsicServerPortTable(	uint32 index, ipaddr_t *ExtIp, uint16 *ExtPort,
													ipaddr_t *IntIp, uint16 *IntPort ) ;
__INLINE__ int32 convert_setAsicNaptTcpUdpTable(	int8 forced,
														ipaddr_t IntIp, uint16 IntPort, uint16 ExtPort,
														uint32 ageSec, int8 entryType, int8 isTcp, int8 tcpFlag, 
														int8 isCollision, int8 isCollision2, int8 isValid );
__INLINE__ int32 convert_getAsicNaptTcpUdpTable(	uint8 *extIpIdx, uint16 index,
														ipaddr_t *IntIp, uint16 *IntPort, uint16 *ExtPort,
														uint32 *ageSec,  int8 * entryType, int8 *isTcp, int8 *tcpFlag,
														int8 *isCollision, int8 *isValid, int8 *isCollision2,
														int8 *isDedicated, uint16 *selEIdx, uint8 *selExtIpIdx );
__INLINE__ int32 convert_setAsicNaptIcmpTable(	int8 forced,
													ipaddr_t IntIp, uint16 IntId, uint16 ExtId,
													uint32 ageSec, int8 entryType, int16 count, int8 isCollision, int8 isValid );
__INLINE__ int32 convert_getAsicNaptIcmpTable(	int8 precisePort, uint16 tarId,
													ipaddr_t *IntIp, uint16 *IntId,
													uint16 *ExtId, uint32 *ageSec, int8 *entryType,
													uint16 *count, int8 *isCollision, int8 *isValid );
__INLINE__ int32 convert_setAsicAlg( uint32 index, uint16 port );
__INLINE__ int32 convert_getAsicAlg( uint32 index, uint16 *port );
__INLINE__ int32 convert_setAsicL2Table(	uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
											int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag);
int32 rtl8651_setAsicL2Table_Patch(	uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
										int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag);
int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *asic_l2_t);


__INLINE__ int32 convert_getAsicProtoTrap( uint32 index, uint8 *type, uint16 *content);



#endif /* RTL865XC_TBLDRV_PATCH_H */


