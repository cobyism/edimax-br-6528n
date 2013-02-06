
/*	@doc RTL865XC_TBLASICDRV_API

	@module rtl865xC_tblAsicDrv.c - RTL8651 Home gateway controller ASIC table driver API documentation	|
	This document explains the internal and external API interface for ASIC table driver module. Functions with _rtl8651 prefix
	are internal functions and functions with rtl8651_ prefix are external functions.
	@normal Gateway team (gwhp@realtek.com.tw) <date>

	Copyright <cp>2006 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL865XC_TBLASICDRV_API
*/


#include "rtl_types.h"
#include "rtl_utils.h"
#include "assert.h"
#include "types.h"
#include "rtl865xC_tblAsicDrv.h"
#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#include "rtl8651_tblDrvLocal.h"
#else
#define RTL8651_TBLDRV_LOCAL_H
#include "rtl8651_aclLocal.h"
#include "rtl865x_lightrome.h"
#endif
#include "asicregs.h"
#include "rtl865xC_tblAsicDrv.h"
#ifdef __TO_BE_DELETED__
#include "rtl_bdinfo.h"
#include "board.h"
#endif
#ifdef RTL865X_TEST
#include <stdio.h>
#include "drvTest.h"
#endif
#ifdef RTL865X_MODEL_USER
#include "rtl_glue.h"
#include <stdio.h>
#include <string.h>
#endif
#include "mbuf.h" /* To get temporary storage*/
#ifdef RTL865X_MODEL_KERNEL
#define FPGA 1	/* This flag is only for RTL865xC FPGA verification. */
#endif
#include "rtl_errno.h"

#if defined(CONFIG_HW_MULTICAST_TBL) || defined(CONFIG_RTK_VLAN_SUPPORT)
	#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
	int32 rtl8651_totalExtPortNum=3;
	#endif
#endif

#ifdef CONFIG_HW_MULTICAST_TBL
uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr) {
#ifdef CONFIG_RTL8196C_REVISION_B
	uint32 idx;
	uint32 sip[32],dip[32];
	uint32 hash[7];
	uint32 i;

	for(i=0; i<7; i++) {
		hash[i]=0;
	}

	for(i=0; i<32; i++)	{
		if((srcAddr & (1<<i))!=0) {
			sip[i]=1;
		}
		else 	{
			sip[i]=0;
		}

		if((dstAddr & (1<<i))!=0) {
			dip[i]=1;
		}
		else {
			dip[i]=0;
		}			
	}

	hash[0] = sip[0] ^ sip[7]   ^ sip[14] ^ sip[21] ^ sip[28] ^ dip[1] ^ dip[8]   ^ dip[15] ^ dip[22] ^ dip[29];
	hash[1] = sip[1] ^ sip[8]   ^ sip[15] ^ sip[22] ^ sip[29] ^ dip[2] ^ dip[9]   ^ dip[16] ^ dip[23] ^ dip[30];
	hash[2] = sip[2] ^ sip[9]   ^ sip[16] ^ sip[23] ^ sip[30] ^ dip[3] ^ dip[10] ^ dip[17] ^ dip[24] ^ dip[31];
	hash[3] = sip[3] ^ sip[10] ^ sip[17] ^ sip[24] ^ sip[31] ^ dip[4] ^ dip[11] ^ dip[18] ^ dip[25];
	hash[4] = sip[4] ^ sip[11] ^ sip[18] ^ sip[25]               ^ dip[5] ^ dip[12] ^ dip[19] ^ dip[26];
	hash[5] = sip[5] ^ sip[12] ^ sip[19] ^ sip[26]               ^ dip[6] ^ dip[13] ^ dip[20] ^ dip[27];
	hash[6] = sip[6] ^ sip[13] ^ sip[20] ^ sip[27]   ^ dip[0] ^ dip[7] ^ dip[14] ^ dip[21] ^ dip[28];

	for(i=0; i<7; i++) {
		hash[i]=hash[i] & (0x01);
	}

	idx=0;
	for(i=0; i<7; i++) {
		idx=idx+(hash[i]<<i);
	}
	
	return idx;
#else
	uint32 idx;

	if(RTL865X_IP_MCAST_NEW_HASH)
	{
		uint32 sip[32],dip[32];
		uint32 hash[6];
		uint32 i;
		for(i=0; i<6; i++)
		{
			hash[i]=0;
		}

		for(i=0; i<32; i++)
		{
			if((srcAddr & (1<<i))!=0)
			{
				sip[i]=1;
			}
			else
			{
				sip[i]=0;
			}

			if((dstAddr & (1<<i))!=0)
			{
				dip[i]=1;
			}
			else
			{
				dip[i]=0;
			}
			
		}

		hash[0] = dip[0]^dip[6]^dip[12]^dip[18]^dip[24]^dip[26]^dip[28]^dip[30]^
	         		sip[23]^sip[5]^sip[11]^sip[17]^sip[31]^sip[25]^sip[27]^sip[29];
		hash[1] = dip[1]^dip[7]^dip[13]^dip[19]^dip[25]^dip[27]^dip[29]^dip[31]^
		         	sip[0]^sip[6]^sip[12]^sip[18]^sip[24]^sip[26]^sip[28]^sip[30];
		hash[2] = dip[2]^dip[8]^dip[14]^dip[20]^sip[1]^sip[7]^sip[13]^sip[19];
		hash[3] = dip[3]^dip[9]^dip[15]^dip[21]^sip[2]^sip[8]^sip[14]^sip[20];
		hash[4] = dip[4]^dip[10]^dip[16]^dip[22]^sip[3]^sip[9]^sip[15]^sip[21];
		hash[5] = dip[5]^dip[11]^dip[17]^dip[23]^sip[4]^sip[10]^sip[16]^sip[22];

		for(i=0; i<6; i++)
		{
			hash[i]=hash[i] & (0x01);
		}

		idx=0;
		for(i=0; i<6; i++)
		{
			idx=idx+(hash[i]<<i);
		}
	}
	else
	{
		idx = srcAddr ^ (srcAddr>>8) ^ (srcAddr>>16) ^ (srcAddr>>24) ^ dstAddr ^ (dstAddr>>8) ^ (dstAddr>>16) ^ (dstAddr>>24);
		idx = ((idx >> 2) ^ (idx & 0x3)) & (RTL8651_IPMULTICASTTBL_SIZE-1);
	}
	
	return idx;
#endif	
}

int add_ip_multicast_tbl(uint32 src_ip, uint32 dst_ip, int src_vlan, uint8 src_port, uint8 mem_port)
{
	rtl865x_tblAsicDrv_multiCastParam_t tbl;

	memset(&tbl, '\0', sizeof(tbl));
	tbl.mbr = (uint32)mem_port;
	tbl.port = (uint16)src_port;
	tbl.sip = src_ip;
	tbl.dip = dst_ip;
	tbl.svid = src_vlan;
	
	if (rtl8651_setAsicIpMulticastTable(&tbl, 1) != SUCCESS) {
		//printk("rtl8651_setAsicIpMulticastTable() failed!\n");
		return -1;
	}
	
	return ((int)rtl8651_ipMulticastTableIndex(src_ip, dst_ip));
}

int update_ip_multicast_tbl(uint32  index, uint8 mem_port, int valid)
{
	static rtl865x_tblAsicDrv_multiCastParam_t tbl;

	if(index < RTL8651_IPMULTICASTTBL_SIZE &&  index >= 0 ){
		if (rtl8651_getAsicIpMulticastTable(index, &tbl) == SUCCESS) {
			tbl.mbr = (uint32)mem_port;	
			if (rtl8651_setAsicIpMulticastTable(&tbl, valid) != SUCCESS) {
				printk("rtl8651_setAsicIpMulticastTable() failed!\n");
				return -1;
			}		
		}
		else {
			printk("rtl8651_getAsicIpMulticastTable() failed!\n");
			return -1;		
		}
	}else{
		return -1;		
	}
	
	return SUCCESS;
}

int32 rtl8651_setAsicIpMulticastTable(rtl865x_tblAsicDrv_multiCastParam_t *mCast_t, int is_valid) {
	uint32 idx;
 	rtl865xc_tblAsic_ipMulticastTable_t entry;
	int16 age;

	if(mCast_t->dip >>28 != 0xe || mCast_t->port >= RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		return FAILED;//Non-IP multicast destination address
	bzero(&entry, sizeof(entry));
	entry.srcIPAddr 		= mCast_t->sip;
	entry.destIPAddrLsbs 	= mCast_t->dip & 0xfffffff;

	idx = rtl8651_ipMulticastTableIndex(mCast_t->sip, mCast_t->dip);

	// prevent the multicast storm. do not create the entry if source port is equal to member port.
	if (mCast_t->mbr ==  BIT(mCast_t->port))
		return FAILED;		

	mCast_t->mbr &= ~(BIT(mCast_t->port));

#ifdef CONFIG_RTL8196C_REVISION_B
	entry.srcPort 			= mCast_t->port;
	entry.portList 			= mCast_t->mbr;

#else

	if (mCast_t->port >= RTL8651_PORT_NUMBER) {
		/* extension port */
		entry.srcPortExt = 1;
		entry.srcPort 			= (mCast_t->port-RTL8651_PORT_NUMBER);

	} else {
		entry.srcPortExt = 0;
		entry.srcPort 			= mCast_t->port;

	}

	entry.extPortList 		= mCast_t->mbr >> RTL8651_PORT_NUMBER;
	entry.srcVidH 			= ((mCast_t->svid)>>4) &0xff;
	entry.srcVidL 			= (mCast_t->svid)&0xf;
	entry.portList 			= mCast_t->mbr & (RTL8651_PHYSICALPORTMASK);
#endif

	entry.toCPU 			= 0;
	entry.valid 			= is_valid;
	entry.extIPIndex 		= mCast_t->extIdx;

	entry.ageTime			= 0;
	age = (int16)mCast_t->age;
	while ( age > 0 ) {
		if ( (++entry.ageTime) == 7)
			break;
		age -= 5;
	}
	#ifdef CONFIG_HW_MULTICAST_TBL
	entry.ageTime = 7;
	#endif
	
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
}

int32 rtl8651_delAsicIpMulticastTable(uint32 index) {
	rtl865xc_tblAsic_ipMulticastTable_t entry;

	bzero(&entry, sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
}

int32 rtl8651_getAsicIpMulticastTable(uint32 index, rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) {	
	static rtl865xc_tblAsic_ipMulticastTable_t entry;
	
	if (mCast_t == NULL)
		return FAILED;
   	_rtl8651_readAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);

	
 	mCast_t->sip	= entry.srcIPAddr;
 	mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;

#ifdef CONFIG_RTL8196C_REVISION_B
	mCast_t->svid = 0;
	mCast_t->port = entry.srcPort;
	mCast_t->mbr = entry.portList;
#else	
	mCast_t->svid = (entry.srcVidH<<4) | entry.srcVidL;

	if (entry.srcPortExt) {
		mCast_t->port = entry.srcPort + RTL8651_PORT_NUMBER;
	} else {
		mCast_t->port = entry.srcPort;
	}

	mCast_t->mbr = entry.extPortList<<RTL8651_PORT_NUMBER | entry.portList;
#endif

	mCast_t->extIdx = entry.extIPIndex ;
	mCast_t->age	= entry.ageTime * 5;
	mCast_t->cpu = entry.toCPU;
	return SUCCESS;
}
#endif

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#ifdef DBG_ASIC_MULTICAST_TBL
void dump_multicast_table(void)
{
		int32	i;
		rtl865x_tblAsicDrv_multiCastParam_t asic_mc;

		panic_printk(">> Multicast Table:\n");
		for(i=0; i<RTL8651_IPMULTICASTTBL_SIZE; i++) {
			if (rtl8651_getAsicIpMulticastTable(i,  &asic_mc) == FAILED)
				continue;

			if (asic_mc.sip != 0)
			  panic_printk("\t[%d]  %d.%d.%d.%d/%d.%d.%d.%d; %d, port:%d,mbr:0x%02x, idx:%d, %d, %d\n", i, (asic_mc.sip>>24),
				((asic_mc.sip&0x00ff0000)>>16), ((asic_mc.sip&0x0000ff00)>>8), (asic_mc.sip&0xff),
				(asic_mc.dip>>24),((asic_mc.dip&0x00ff0000)>>16), 
				((asic_mc.dip&0x0000ff00)>>8), (asic_mc.dip&0xff),
				asic_mc.svid, asic_mc.port, asic_mc.mbr, 
				asic_mc.extIdx, asic_mc.age, asic_mc.cpu
				);
		}	
}
#endif

#ifdef DBG_ASIC_COUNTER
int32 rtl8651_clearAsicCounter(void) 
{
	WRITE_MEM32(MIB_CONTROL, ALL_COUNTER_RESTART_MASK);
	return SUCCESS;
}

int32 rtl8651_returnAsicCounter(uint32 offset) 
{
	if(offset & 0x3)
		return 0;
	return  READ_MEM32(MIB_COUNTER_BASE + offset);
}

int32 rtl865xC_dumpAsicCounter(void)
{
	uint32 i;
	uint32 addrOffset_fromP0;

	for ( i = 0; i <= RTL8651_PORT_NUMBER; i++ )
	{
		addrOffset_fromP0 = i * MIB_ADDROFFSETBYPORT;

		if ( i == RTL8651_PORT_NUMBER )
			panic_printk("<CPU port>\n");
		else
			panic_printk("<Port: %d>\n", i);
        
		panic_printk("Rx %u * 2^32 + %u Bytes, Rx %u:%u(mc):%u(bc) Pkts, Drop %u pkts, Rx Pause %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 + 4),
			rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ) );
		
		panic_printk("Tx %u * 2^32 + %u Bytes, Tx %u:%u(mc):%u(bc) Pkts Tx Pause %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 + 4),
			rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ) );
		
      		if ( i == RTL8651_PORT_NUMBER ) {
		     panic_printk("EtherStatsOctets: %u * 2^32 + %u.\n", 
			  rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0 + 4),
			  rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0 ));                
              }

	}

	return SUCCESS;
}
#endif
#endif


/*
 * ============================================================================
 * ============================================================================
 *
 * The following code is for Protocol-based VLAN and copied from rtl865xC_layer2asic.c
 *
 * ============================================================================
 * ============================================================================
 */

#if defined(CONFIG_HW_PROTOCOL_VLAN_TBL) && !defined(CONFIG_RTL865X_LIGHT_ROMEDRV)
/*=========================================
 * ASIC DRIVER API: Protocol-based VLAN
 *=========================================*/

/*
@func int32 | rtl8651_defineProtocolBasedVLAN | configure user-definable protocol-based VLAN
@parm uint32 | ruleNo        | Valid values: RTL8651_PBV_RULE_USR1 and RTL8651_PBV_RULE_USR2
@parm uint8  | ProtocolType  | 00:ethernetII, 01:RFC-1042, 10: LLC-Other, 11:reserved
@parm uint16 | ProtocolValue | ethernetII:ether type, RFC-1042:ether type, LLC-Other:PDSAP(8),SSAP(8)}
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_defineProtocolBasedVLAN( uint32 ruleNo, uint8 ProtocolType, uint16 ProtocolValue )
{
	assert( ruleNo == RTL8651_PBV_RULE_USR1 ||
	        ruleNo == RTL8651_PBV_RULE_USR2 );

	if ( ruleNo == RTL8651_PBV_RULE_USR1 )
	{
		WRITE_MEM32( PBVCR0, ( ProtocolType << PBVCR_PROTO_TYPE_OFFSET ) |
		                    ( ProtocolValue << PBVCR_PROTO_VALUE_OFFSET ) );
	}
	else if ( ruleNo == RTL8651_PBV_RULE_USR2 )
	{
		WRITE_MEM32( PBVCR1, ( ProtocolType << PBVCR_PROTO_TYPE_OFFSET ) |
		                     ( ProtocolValue << PBVCR_PROTO_VALUE_OFFSET ) );
	}
	return SUCCESS;
}


/*
@func int32 | rtl8651_setProtocolBasedVLAN | set corresponding table index of protocol-based VLAN
@parm uint32 | ruleNo  | rule Number (1~6)
@parm uint8  | port    | 0~4:PHY  5:MII  6~8:ExtPort
@parm uint8  | vlanIdx | VLAN Table index (0~7)
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_setProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8 valid, uint16 vlanId )
{
	uint32 addr;
	uint32 value;

	assert( ruleNo > 0 && ruleNo < RTL8651_PBV_RULE_MAX );
//	assert( vlanId < RTL865XC_VLAN_NUMBER );
//	assert( port < RTL8651_AGGREGATOR_NUMBER );
	ruleNo = ruleNo-1;
	valid = valid ? TRUE : FALSE;
	if ( valid == FALSE )
	{
		vlanId = 0; // clear it for looking pretty.
	}

	if ( port < RTL865XC_PORT_NUMBER )
	{
		// Port0 ~ Port9
		addr=PBVR0_0 +(ruleNo*5*4) + ((port/2)*4) ;
		value = ( vlanId<<1 | valid );
		if (port&0x1)
			value =(value <<16) | (0x0000FFFF& READ_MEM32(addr));
		else
			value =value|( 0xFFFF0000& READ_MEM32(addr));		
		WRITE_MEM32(addr,value);
	}
	return SUCCESS;
}
#endif

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#define FDB_STATIC						0x01
#define FDB_DYNAMIC					0x02
static uint8 fidHashTable[]={0x00,0x0f,0xf0,0xff};
rtl865x_tblAsicDrv_l2Param_t __l2buff;
extern uint16 _fid;

__IRAM_L2_FWD uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr,uint16 fid) {
    return ( macAddr->octet[0] ^ macAddr->octet[1] ^
                    macAddr->octet[2] ^ macAddr->octet[3] ^
                    macAddr->octet[4] ^ macAddr->octet[5] ^fidHashTable[fid]) & 0xFF;
}

int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p) {
	rtl865xc_tblAsic_l2Table_t   entry;
 
	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);

	if(entry.agingTime == 0 && entry.isStatic == 0 &&entry.auth==0)
		return FAILED;
	l2p->macAddr.octet[0] = entry.mac47_40;
	l2p->macAddr.octet[1] = entry.mac39_24 >> 8;
	l2p->macAddr.octet[2] = entry.mac39_24 & 0xff;
	l2p->macAddr.octet[3] = entry.mac23_8 >> 8;
	l2p->macAddr.octet[4] = entry.mac23_8 & 0xff;
	l2p->macAddr.octet[5] = row ^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4]  ^(fidHashTable[entry.fid]);
	l2p->cpu = entry.toCPU==1? TRUE: FALSE;
	l2p->srcBlk = entry.srcBlock==1? TRUE: FALSE;
	l2p->nhFlag = entry.nxtHostFlag==1? TRUE: FALSE;
	l2p->isStatic = entry.isStatic==1? TRUE: FALSE;
	l2p->memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	l2p->ageSec = entry.agingTime * 150;

	l2p->fid=entry.fid;
	l2p->auth=entry.auth;
	return SUCCESS;
}

int32 rtl8651_lookupL2table(uint16 fid, ether_addr_t * macAddr, int flags)
{
	uint32 hash0, way0;
		
	hash0 = rtl8651_filterDbIndex(macAddr, fid);

	for(way0=0; way0<RTL8651_L2TBL_COLUMN; way0++) {
		if (rtl8651_getAsicL2Table(hash0, way0, &__l2buff)!=SUCCESS ||
			memcmp(&__l2buff.macAddr, macAddr, 6)!= 0)
			continue;
		
		return SUCCESS;			
	}
	return FAILED;
}

int32 rtl8651_delAsicL2Table(uint32 row, uint32 column) {
	rtl8651_tblAsic_l2Table_t entry;

	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;

	bzero(&entry, sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}

static rtl865x_tblAsicDrv_l2Param_t *lr_fdb_lookup(uint32 vfid, ether_addr_t *mac,  uint32 flags, uint32 *way)
{
	uint32 hash0, way0;

	hash0 = rtl8651_filterDbIndex(mac, vfid);
	for(way0=0; way0<RTL8651_L2TBL_COLUMN; way0++) {
		if (rtl8651_getAsicL2Table(hash0, way0, &__l2buff)!=SUCCESS ||
			memcmp(&__l2buff.macAddr, mac, 6)!= 0)
			continue;
		if (((flags&FDB_STATIC) && __l2buff.isStatic) ||
			((flags&FDB_DYNAMIC) && !__l2buff.isStatic)) {
			assert(way);
			*way = way0;
			return &__l2buff;
		}
	} return (rtl865x_tblAsicDrv_l2Param_t *)0;
}

int32 rtl8651_delFilterDatabaseEntry(uint16 fid, ether_addr_t * macAddr)
{
	int32 res = FAILED;
	uint32 way, hash;

	if (__l2buff.memberPortMask == EXT_PORT_HWLOOKUP) 
		return SUCCESS;

	if (lr_fdb_lookup(fid, macAddr, FDB_DYNAMIC, &way)) {
		res = SUCCESS;
		hash=rtl8651_filterDbIndex(macAddr, fid);
			rtl8651_delAsicL2Table(hash, way); 
	} 
	return res;
}
#endif















#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV

/*=========================================
  * Internal Data structure
  *=========================================*/


int8 rtl8651_tblAsicDrv_Id[] = "$Id: rtl865xC_tblAsicDrv.c,v 1.22.2.1 2010/06/25 09:15:58 joeylin Exp $";



/*=========================================
  * Function Prototype
  *=========================================*/
#define RTL8651_ASICDRV_FUNCTION_PROTOTYPE
uint32	 _rtl8651_NaptAgingToSec(uint32 value);
uint32 	_rtl8651_NaptAgingToUnit(uint32 sec);
void 		_rtl8651_asicTableAccessForward(uint32, uint32, void *);


/*=========================================
  * Static Function Prototype
  *=========================================*/
//static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count);
//static void _rtl8651_initialRead(void);


/*=========================================
  * Global Variable declaration
  *=========================================*/
int8 RtkHomeGatewayChipName[16];
int32 RtkHomeGatewayChipRevisionID;
int32 RtkHomeGatewayChipNameID;
rtl8651_tblAsic_InitPara_t rtl8651_tblAsicDrvPara;
#define RTL865X_ASICDRV_GLOBAL_VARIABLE

/*define for version control --Mark*/
#define RLRevID_OFFSET  12
#define RLRevID_MASK    0x0f
#define A_DIFF_B_ADDR   (PCI_CTRL_BASE+0x08) /*B800-3408*/


#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) || defined(RTL865X_MODEL_KERNEL)
void rtl8651_setChipVersion(int8 *name, int32 *rev){
	//rtlglue_printf("%s (%s %d) Not Implemnt Yet\n",__FUNCTION__,__FILE__,__LINE__);
    //Modified by Mark
    uint32 revision;
	WRITE_MEM32(CHIPID,0x86520000);	

	if(!rev)
		revision=0;
	else
		revision=(*rev)&RLRevID_MASK;
	
	WRITE_MEM32(CRMR, (0x59350000) | (revision << RLRevID_OFFSET));

	rtl8651_getChipVersion(RtkHomeGatewayChipName, sizeof(RtkHomeGatewayChipName), &RtkHomeGatewayChipRevisionID);
}
#endif

/* get CHIP related ID */
extern unsigned int chip_id;

int32 rtl8651_getChipNameID(int32 *id)
{
	if (chip_id == 0x8196)
		*id = RTL865X_CHIP_VER_RTL8196B;
	else
		*id = RTL865X_CHIP_VER_RTL865XC;

	return  SUCCESS;
}

int32 rtl865xc_setPortBasedNetif(uint32 port, uint32 netif)
{
	uint32 mask,value;
	
	mask = ~(0x7<<(port*3));
	value = (netif &0x7) <<(port*3);
	WRITE_MEM32(PLITIMR,(READ_MEM32(PLITIMR) & mask)|value);
	return SUCCESS;
}
		
/*
 * Get table index using the default hash algorithm of RTL865xC
 *
 *   {VERI,isTCP}  srcAddr  srcPort  destAddr  destPort
 *      0     *       	*        	*       	!=0       !=0       	HASH1 index (for tranditional and enhanced Hash1)
 *      0     *      	*        	*        	0         	0        	HASH2 index
 *      1     *       	*        	*        	*         	*        	Verification of Enhanced HASH1
 */
uint32 rtl8651_naptTcpUdpTableIndex(int8 isTCP, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort) {
	uint32 eidx;

	if(destAddr==0 && destPort==0 && ((isTCP&HASH_FOR_VERI)==0))
	{
		/* From alpha's mail (2006/05/02)
		hash 2
		hash2id = sip[9:0] ^ sip[19:10] ^ sip[29:20] ^
		          sport[9:0] ^
		          { 1'd0, tcp_pkt, sip[31:30], sport[15:10] } ;
		*/ 
		eidx = (srcAddr&0x3ff) ^ ((srcAddr>>10)&0x3ff) ^ ((srcAddr>>20)&0x3ff) ^
		       (srcPort&0x3ff) ^
		       (((isTCP&1)<<8) | ((srcAddr&0xc0000000)>>24) | ((srcPort>>10)&0x3f) );
		assert(eidx < RTL8651_TCPUDPTBL_SIZE);
		return eidx;
	} 

	/* From alpha's mail (2006/05/02) 
	  hash 1
	  idx[96:0] = { dip, dport, tcp_pkt, sip, sport } ;  
	  hash1id   = idx[9:0] ^ idx[19:10] ^ idx[29:20] ^
	              idx[39:30] ^ idx[49:40] ^ idx[59:50] ^ 
	              idx[69:60] ^ idx[79:70] ^ idx[89:80] ^
	              { 3'd0,  idx[96:90] } ; 
	*/
	eidx = (srcPort&0x3ff) ^ (((srcPort&0xfc00)>>10)|((srcAddr&0xf)<<6)) ^ ((srcAddr>>4)&0x3ff) ^
	       ((srcAddr>>14)&0x3ff) ^ (((srcAddr&0xff000000)>>24)|((isTCP&1)<<8)|((destPort&1)<<9)) ^
	       ((destPort>>1)&0x3ff) ^ (((destPort>>11)&0x1f)|((destAddr&0x1f)<<5)) ^ ((destAddr>>5)&0x3ff) ^
	       ((destAddr>>15)&0x3ff) ^ ((destAddr>>25)&0x7f);
	assert(eidx < RTL8651_TCPUDPTBL_SIZE);
	return eidx;
}

uint32 rtl8651_naptIcmpTableIndex(ipaddr_t srcAddr, uint16 icmpId, ipaddr_t destAddr, uint32 * tblIdx) 
{
        uint32 eidx;
        eidx = (icmpId&0x3ff) ^ (((icmpId&0xfc00)>>10)|((srcAddr&0xf)<<6)) ^ ((srcAddr>>4)&0x3ff) ^
               ((srcAddr>>14)&0x3ff) ^ (((srcAddr&0xff000000)>>24)|(0)) ^
               (0) ^ ((0)|((destAddr&0x1f)<<5)) ^ ((destAddr>>5)&0x3ff) ^
               ((destAddr>>15)&0x3ff) ^ ((destAddr>>25)&0x7f);
        *tblIdx = eidx &(RTL8651_ICMPTBL_SIZE-1);
	return eidx;
}

uint32 _rtl8651_NaptAgingToSec(uint32 value) {
	/* 
	 * Convert differentiated timer value to actual second value 
	 * Actual Timeout second = Theory timeout sec * 5/3 - (unit==1? 0: granularity)
	 */
	uint32 unit = 0, G = value>>3, S = 1;

	value &= 0x07;
	while (G > 0) {
		unit += (S << 3);
		S = S << 2;
		G--;
	}
	unit += (S * (value + 1));
	unit = (unit-1)*5/3 - (S>1? S: 0);
	return unit;
}

 uint32 _rtl8651_NaptAgingToUnit(uint32 sec) {
	/* 
	 * Convert actual second value to differentiated timer value
	 * Unit = naptAgingToUnit(sec*0.6) + 1
	 */
	uint32 value = 0, S = 1;

	sec++;
	sec = sec * 3 / 5;
	while(sec >= (S<<3)) {
		sec -= (S << 3);
		S = S << 2;
		value++;
	}
	return (sec%S > (S>>1))? ((value << 3) + (sec / S) + 1) : ((value << 3) + (sec / S) - 1 + 1);
}


/*
@func int8	| Is4WayEnabled	| This function returns if the 4-way hash function is enabled?
@rvalue TRUE	| Yes, 4-way hash is enabled.
@comm 
	(under construction)
@devnote
	(under construction)
 */
uint32 _Is4WayHashEnabled(void)
{
	/*
	rtlglue_printf("%s (%s %d) No More Support \n",__FUNCTION__,__FILE__,__LINE__);
	return 0;	
	*/
	uint32 regValue = READ_MEM32( SWTCR1 );
	return ( regValue & EN_TCPUDP_4WAY_HASH );
}

/*
@func int8	| Is4WayEnabled	| This function sets SWTECR register to enable or diable 4-way hash.
@parm int	| enable	| TRUE if you want to enable 4-way hash.
@rvalue SUCCESS	| Done
@comm 
This function sets SWTECR register to enable or diable 4-way hash.
@devnote
	(under construction)
 */
int32 _set4WayHash( int32 enable )
{
	/*
	rtlglue_printf("%s (%s %d) No More Support \n",__FUNCTION__,__FILE__,__LINE__);
	*/
	uint32 regValue = READ_MEM32( SWTCR1 );
	if ( enable )
	{
		regValue |= EN_TCPUDP_4WAY_HASH;
	}
	else
	{
		//rtlglue_printf("%s (%s %d) Warning!! Disable 4 wayHash would bring on something unexpected! \n",__FUNCTION__,__FILE__,__LINE__);
		regValue &= ~EN_TCPUDP_4WAY_HASH;
	}
	#if 0
	// write back
	WRITE_MEM32( SWTECR, regValue );
	#else
	WRITE_MEM32( SWTCR1, regValue );
	#endif
	
	return SUCCESS;	
}

int32 _rtl8651_enableEnhancedHash1()
{
	uint32 regValue = READ_MEM32( SWTCR1 );
	regValue |= L4EnHash1;
	WRITE_MEM32( SWTCR1, regValue );
	return SUCCESS;
}

int32 _rtl8651_disableEnhancedHash1()
{
	uint32 regValue = READ_MEM32( SWTCR1 );
	regValue &= (~L4EnHash1);
	WRITE_MEM32( SWTCR1, regValue );
	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: ACL Table
  *=========================================*/
#define RTL865X_ASIC_DRIVER_ACL_TBL_API

/*
@func int32	| rtl865xC_setDefaultACLReg	| This function sets default ACL Rule Control Register.
@parm uint32	| isIngress	| TRUE if you want to set default ingress ACL register. FLASE if egress ACL register.
@parm uint32	| start	| The starting address in the ACL table.
@parm uint32	| end	| The ending address in the ACL table.
@rvalue SUCCESS	| Done
@comm 
This function sets the ACL range (starting & ending address) of default ACL Rule Control Register.
*/
int32 rtl865xC_setDefaultACLReg(uint32 isIngress, uint32 start, uint32 end)
{
	/* set default ingress ACL reg's start & end index of ASIC ACL table */
	if ( isIngress == TRUE )
	{
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLI_STA_MASK ) ) | start );
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLI_EDA_MASK ) ) | ( end << ACLI_EDA_OFFSET) );
	}
	else		/* egress */
	{
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLO_STA_MASK ) ) | ( start << ACLO_STA_OFFSET ) );
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLO_EDA_MASK ) ) | ( end << ACLO_EDA_OFFSET ) );
	}
	return SUCCESS;
}

/*
@func int32	| rtl865xC_getDefaultACLReg	| This function gets default ACL Rule Control Register.
@parm uint32	| isIngress	| TRUE if you want to set default ingress ACL register. FLASE if egress ACL register.
@parm uint32 *	| start	| Memory to store the starting address in the ACL table.
@parm uint32 *	| end	| Memory to store the ending address in the ACL table.
@rvalue SUCCESS	| Done
@comm 
This function gets the ACL range (starting & ending address) of default ACL Rule Control Register.
*/
int32 rtl865xC_getDefaultACLReg(uint32 isIngress, uint32 *start, uint32 *end)
{
	/* set default ingress ACL reg's start & end index of ASIC ACL table */
	if ( isIngress == TRUE )
	{
		*start = READ_MEM32( DACLRCR ) & ACLI_STA_MASK;
		*end = ( READ_MEM32( DACLRCR ) & ACLI_EDA_MASK ) >> ACLI_EDA_OFFSET;
	}
	else		/* egress */
	{
		*start = ( READ_MEM32( DACLRCR ) & ACLO_STA_MASK ) >> ACLO_STA_OFFSET;
		*end = ( READ_MEM32( DACLRCR ) & ACLO_EDA_MASK ) >> ACLO_EDA_OFFSET;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicAclRule(uint32 index, _rtl8651_tblDrvAclRule_t *rule) {
	rtl865xc_tblAsic_aclTable_t    entry;

	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;

	rule->aclIdx = index;
	memset(&entry, 0, sizeof(entry));
	switch(rule->ruleType_) {

	case RTL8651_ACL_MAC: /* Etnernet type rule: 0x0000 */
		 entry.is.ETHERNET.dMacP47_32 = rule->dstMac_.octet[0]<<8 | rule->dstMac_.octet[1];
		 entry.is.ETHERNET.dMacP31_16 = rule->dstMac_.octet[2]<<8 | rule->dstMac_.octet[3];
		 entry.is.ETHERNET.dMacP15_0 = rule->dstMac_.octet[4]<<8 | rule->dstMac_.octet[5];
	 	 entry.is.ETHERNET.dMacM47_32 = rule->dstMacMask_.octet[0]<<8 | rule->dstMacMask_.octet[1];
		 entry.is.ETHERNET.dMacM31_16 = rule->dstMacMask_.octet[2]<<8 | rule->dstMacMask_.octet[3];
		 entry.is.ETHERNET.dMacM15_0 = rule->dstMacMask_.octet[4]<<8 | rule->dstMacMask_.octet[5];
		 entry.is.ETHERNET.sMacP47_32 = rule->srcMac_.octet[0]<<8 | rule->srcMac_.octet[1];
		 entry.is.ETHERNET.sMacP31_16 = rule->srcMac_.octet[2]<<8 | rule->srcMac_.octet[3];
		 entry.is.ETHERNET.sMacP15_0 = rule->srcMac_.octet[4]<<8 | rule->srcMac_.octet[5];
		 entry.is.ETHERNET.sMacM47_32 = rule->srcMacMask_.octet[0]<<8 | rule->srcMacMask_.octet[1];
		 entry.is.ETHERNET.sMacM31_16 = rule->srcMacMask_.octet[2]<<8 | rule->srcMacMask_.octet[3];
		 entry.is.ETHERNET.sMacM15_0 = rule->srcMacMask_.octet[4]<<8 | rule->srcMacMask_.octet[5];
		 entry.is.ETHERNET.ethTypeP = rule->typeLen_;
		 entry.is.ETHERNET.ethTypeM = rule->typeLenMask_;
		 entry.ruleType = 0x0;
		 break;

	case RTL8651_ACL_IFSEL: /* IFSEL type rule */
		rtlglue_printf("%s (%s %d) No Support Yet\n",__FUNCTION__,__FILE__,__LINE__);
		 break;
	
		 
	case RTL8651_ACL_IP: /* IP Rule Type: 0x0010 */
	case RTL8652_ACL_IP_RANGE:
		 entry.is.L3L4.is.IP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IP.IPProtoP = rule->ipProto_;
		 entry.is.L3L4.is.IP.IPProtoM = rule->ipProtoMask_;
		 entry.is.L3L4.is.IP.IPFlagP = rule->ipFlag_;
		 entry.is.L3L4.is.IP.IPFlagM = rule->ipFlagMask_;
 		 entry.is.L3L4.is.IP.FOP = rule->ipFOP_;
		 entry.is.L3L4.is.IP.FOM = rule->ipFOM_;
		 entry.is.L3L4.is.IP.HTTPP = entry.is.L3L4.is.IP.HTTPM = rule->ipHttpFilter_;
		 entry.is.L3L4.is.IP.identSDIPP = entry.is.L3L4.is.IP.identSDIPM = rule->ipIdentSrcDstIp_;
		 if (rule->ruleType_==RTL8651_ACL_IP)
			 entry.ruleType = 0x2;
		else
			 entry.ruleType = 0xa;
		 goto l3l4_shared;
		 
	case RTL8651_ACL_ICMP:
	case RTL8652_ACL_ICMP_IPRANGE:
		 entry.is.L3L4.is.ICMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.ICMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.ICMP.ICMPTypeP = rule->icmpType_;
		 entry.is.L3L4.is.ICMP.ICMPTypeM = rule->icmpTypeMask_;
		 entry.is.L3L4.is.ICMP.ICMPCodeP = rule->icmpCode_;
		 entry.is.L3L4.is.ICMP.ICMPCodeM = rule->icmpCodeMask_;
 		 if (rule->ruleType_==RTL8651_ACL_ICMP)
			 entry.ruleType = 0x4;
		 else
		 	entry.ruleType=0xc;
		 goto l3l4_shared;

	case RTL8651_ACL_IGMP:
	case RTL8652_ACL_IGMP_IPRANGE:
		 entry.is.L3L4.is.IGMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IGMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IGMP.IGMPTypeP = rule->igmpType_;
		 entry.is.L3L4.is.IGMP.IGMPTypeM = rule->igmpTypeMask_; 
  		 if (rule->ruleType_==RTL8651_ACL_IGMP)
			 entry.ruleType = 0x5;
   		 else
			entry.ruleType=0xd;
 		 goto l3l4_shared;

	case RTL8651_ACL_TCP:
	case RTL8652_ACL_TCP_IPRANGE:
		 entry.is.L3L4.is.TCP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.TCP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.TCP.TCPFlagP = rule->tcpFlag_;
		 entry.is.L3L4.is.TCP.TCPFlagM = rule->tcpFlagMask_;
		 entry.is.L3L4.is.TCP.TCPSPUB = rule->tcpSrcPortUB_;
		 entry.is.L3L4.is.TCP.TCPSPLB = rule->tcpSrcPortLB_;
		 entry.is.L3L4.is.TCP.TCPDPUB = rule->tcpDstPortUB_;
		 entry.is.L3L4.is.TCP.TCPDPLB = rule->tcpDstPortLB_;
 		 if (rule->ruleType_==RTL8651_ACL_TCP)
			 entry.ruleType = 0x6;
 		else
			entry.ruleType=0xe;
         goto l3l4_shared;

	case RTL8651_ACL_UDP:
	case RTL8652_ACL_UDP_IPRANGE:
		 entry.is.L3L4.is.UDP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.UDP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.UDP.UDPSPUB = rule->udpSrcPortUB_;
		 entry.is.L3L4.is.UDP.UDPSPLB = rule->udpSrcPortLB_;
		 entry.is.L3L4.is.UDP.UDPDPUB = rule->udpDstPortUB_;
		 entry.is.L3L4.is.UDP.UDPDPLB = rule->udpDstPortLB_;
  		 if (rule->ruleType_==RTL8651_ACL_UDP)
			 entry.ruleType = 0x7;
		 else
		 	entry.ruleType=0xf;

l3l4_shared:
		 entry.is.L3L4.sIPP = rule->srcIpAddr_;
		 entry.is.L3L4.sIPM = rule->srcIpAddrMask_;
		 entry.is.L3L4.dIPP = rule->dstIpAddr_;
		 entry.is.L3L4.dIPM = rule->dstIpAddrMask_;
		 break;

 	case RTL8651_ACL_SRCFILTER:
 	case RTL8652_ACL_SRCFILTER_IPRANGE:
 		 rule->srcFilterMac_.octet[0] = rule->srcFilterMac_.octet[0] & rule->srcFilterMacMask_.octet[0];
 		 rule->srcFilterMac_.octet[1] = rule->srcFilterMac_.octet[1] & rule->srcFilterMacMask_.octet[1];
 		 rule->srcFilterMac_.octet[2] = rule->srcFilterMac_.octet[2] & rule->srcFilterMacMask_.octet[2];
 		 rule->srcFilterMac_.octet[3] = rule->srcFilterMac_.octet[3] & rule->srcFilterMacMask_.octet[3];
 		 rule->srcFilterMac_.octet[4] = rule->srcFilterMac_.octet[4] & rule->srcFilterMacMask_.octet[4];
 		 rule->srcFilterMac_.octet[5] = rule->srcFilterMac_.octet[5] & rule->srcFilterMacMask_.octet[5];
 		 
		 entry.is.SRC_FILTER.sMacP47_32 = rule->srcFilterMac_.octet[0]<<8 | rule->srcFilterMac_.octet[1];
		 entry.is.SRC_FILTER.sMacP31_16 = rule->srcFilterMac_.octet[2]<<8 | rule->srcFilterMac_.octet[3];
		 entry.is.SRC_FILTER.sMacP15_0 = rule->srcFilterMac_.octet[4]<<8 | rule->srcFilterMac_.octet[5];
		 entry.is.SRC_FILTER.sMacM3_0 =rule->srcFilterMacMask_.octet[5] &0xf;
 #if 0			 
		 entry.is.SRC_FILTER.sMacM31_16 = rule->srcFilterMacMask_.octet[2]<<8 | rule->srcFilterMacMask_.octet[3];
		 entry.is.SRC_FILTER.sMacM15_0 = rule->srcFilterMacMask_.octet[4]<<8 | rule->srcFilterMacMask_.octet[5];
#endif	
		 rule->srcFilterVlanId_ = rule->srcFilterVlanId_&rule->srcFilterVlanIdMask_;
		 entry.is.SRC_FILTER.spaP = rule->srcFilterPort_;	
		 entry.is.SRC_FILTER.sVidP = rule->srcFilterVlanId_;
		 entry.is.SRC_FILTER.sVidM = rule->srcFilterVlanIdMask_;
		 if(rule->srcFilterIgnoreL3L4_) entry.is.SRC_FILTER.protoType = 1;
		 else if(rule->srcFilterIgnoreL4_) entry.is.SRC_FILTER.protoType = 2;
		 else entry.is.SRC_FILTER.protoType = 0;
		 entry.is.SRC_FILTER.sIPP = rule->srcFilterIpAddr_;
		 entry.is.SRC_FILTER.sIPM = rule->srcFilterIpAddrMask_;
		 entry.is.SRC_FILTER.SPORTUB = rule->srcFilterPortUpperBound_;
		 entry.is.SRC_FILTER.SPORTLB = rule->srcFilterPortLowerBound_;
   		 if (rule->ruleType_==RTL8651_ACL_SRCFILTER)
			entry.ruleType = 0x8;
		 else
		 	entry.ruleType = 0xB;
		 break;
		 
	case RTL8651_ACL_DSTFILTER:
 	case RTL8652_ACL_DSTFILTER_IPRANGE:
		 entry.is.DST_FILTER.dMacP47_32 = rule->dstFilterMac_.octet[0]<<8 | rule->dstFilterMac_.octet[1];
		 entry.is.DST_FILTER.dMacP31_16 = rule->dstFilterMac_.octet[2]<<8 | rule->dstFilterMac_.octet[3];
		 entry.is.DST_FILTER.dMacP15_0 = rule->dstFilterMac_.octet[4]<<8 | rule->dstFilterMac_.octet[5];
	 	 entry.is.DST_FILTER.dMacM3_0 =  rule->dstFilterMacMask_.octet[5]&0xf;
#if 0		 
		 entry.is.DST_FILTER.dMacM47_32 = rule->dstFilterMacMask_.octet[0]<<8 | rule->dstFilterMacMask_.octet[1];
		 entry.is.DST_FILTER.dMacM31_16 = rule->dstFilterMacMask_.octet[2]<<8 | rule->dstFilterMacMask_.octet[3];
		 entry.is.DST_FILTER.dMacM15_0 = rule->dstFilterMacMask_.octet[4]<<8 | rule->dstFilterMacMask_.octet[5];
#endif		 
		 entry.is.DST_FILTER.vidP = rule->dstFilterVlanIdx_;
		 entry.is.DST_FILTER.vidM = rule->dstFilterVlanIdxMask_;
		 if(rule->dstFilterIgnoreL3L4_) entry.is.DST_FILTER.protoType = 1;
		 else if(rule->dstFilterIgnoreL4_) entry.is.DST_FILTER.protoType = 2;
		 else entry.is.DST_FILTER.protoType = 0;
		 entry.is.DST_FILTER.dIPP = rule->dstFilterIpAddr_;
		 entry.is.DST_FILTER.dIPM = rule->dstFilterIpAddrMask_;
		 entry.is.DST_FILTER.DPORTUB = rule->dstFilterPortUpperBound_;
		 entry.is.DST_FILTER.DPORTLB = rule->dstFilterPortLowerBound_;
    		 if (rule->ruleType_==RTL8651_ACL_DSTFILTER)
			 entry.ruleType = 0x09;
		else
	 		 entry.ruleType = 0x01;
		 break;
 
	default: return FAILED; /* Unknown rule type */
	
	}
	
	switch(rule->actionType_) {
	case RTL8651_ACL_PERMIT:			entry.actionType = 0x00;
		 goto _common_action;
	case RTL8651_ACL_DROP:			entry.actionType = 0x02;
		 goto _common_action;
	case RTL8651_ACL_CPU:		 	 	entry.actionType = 0x03;
		 goto _common_action;
	case RTL8651_ACL_DROP_LOG: /* fall thru */
	case RTL8651_ACL_DROP_NOTIFY: entry.actionType = 0x05;
		goto _common_action;
	case RTL8651_ACL_REDIRECT:	 	entry.actionType = 0x01;
		 entry.vid = rule->dvid_;
		 entry.PPPoEIndex = rule->pppoeIdx_;
		 entry.nextHop = rule->nextHop_;
		 goto _common_action;
	case RTL8651_ACL_REDIRECT_PPPOE: 	entry.actionType = 0x07;
		 entry.vid = rule->dvid_;
		 entry.PPPoEIndex = rule->pppoeIdx_;
		 entry.nextHop = rule->nextHop_;
		 goto _common_action;
	case RTL8651_ACL_MIRROR:			entry.actionType = 0x06;
		goto _common_action;
	case RTL8651_ACL_MIRROR_KEEP_MATCH: entry.actionType = 0x09;
		goto _common_action;
	case RTL8651_ACL_DROP_RATE_EXCEED_PPS: entry.actionType = 0x0a;
		entry.nextHop = rule->rlIndex;
		goto _common_action;
	case RTL8651_ACL_LOG_RATE_EXCEED_PPS: entry.actionType = 0x0b;
		entry.nextHop = rule->rlIndex;
		goto _common_action;
	case RTL8651_ACL_DROP_RATE_EXCEED_BPS: entry.actionType = 0x0c;
		entry.nextHop = rule->rlIndex;
		goto _common_action;
	case RTL8651_ACL_LOG_RATE_EXCEED_BPS: entry.actionType = 0x0d;
		entry.nextHop = rule->rlIndex;
		goto _common_action;
	case RTL8651_ACL_PRIORITY: entry.actionType = 0x0e;
		entry.nextHop = rule->priority;
		goto _common_action;

	case RTL8651_ACL_POLICY: entry.actionType = 0x08;
		 entry.nextHop = rule->nhIndex;
		goto _common_action;

_common_action:
		/* handle pktOpApp */
		if ( rule->pktOpApp == RTL865XC_ACLTBL_ALL_LAYER )
			entry.pktOpApp = 0x7;
		else if ( rule->pktOpApp == RTL8651_ACLTBL_NOOP )
			entry.pktOpApp = 0;
		else
 			entry.pktOpApp = rule->pktOpApp;
 		break;
	}
	return _rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
}


int32 rtl8651_getAsicAclRule(uint32 index, _rtl8651_tblDrvAclRule_t *rule) {
	rtl865xc_tblAsic_aclTable_t    entry;

	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	bzero(rule, sizeof(_rtl8651_tblDrvAclRule_t));

	switch(entry.ruleType) {

	case 0x00: /* Ethernet rule type */
		 rule->dstMac_.octet[0]     = entry.is.ETHERNET.dMacP47_32 >> 8;
		 rule->dstMac_.octet[1]     = entry.is.ETHERNET.dMacP47_32 & 0xff;
		 rule->dstMac_.octet[2]     = entry.is.ETHERNET.dMacP31_16 >> 8;
	 	 rule->dstMac_.octet[3]     = entry.is.ETHERNET.dMacP31_16 & 0xff;
		 rule->dstMac_.octet[4]     = entry.is.ETHERNET.dMacP15_0 >> 8;
		 rule->dstMac_.octet[5]     = entry.is.ETHERNET.dMacP15_0 & 0xff;
		 rule->dstMacMask_.octet[0] = entry.is.ETHERNET.dMacM47_32 >> 8;
		 rule->dstMacMask_.octet[1] = entry.is.ETHERNET.dMacM47_32 & 0xff;
		 rule->dstMacMask_.octet[2] = entry.is.ETHERNET.dMacM31_16 >> 8;
		 rule->dstMacMask_.octet[3] = entry.is.ETHERNET.dMacM31_16 & 0xff;
	 	 rule->dstMacMask_.octet[4] = entry.is.ETHERNET.dMacM15_0 >> 8;
		 rule->dstMacMask_.octet[5] = entry.is.ETHERNET.dMacM15_0 & 0xff;
	 	 rule->srcMac_.octet[0]     = entry.is.ETHERNET.sMacP47_32 >> 8;
		 rule->srcMac_.octet[1]     = entry.is.ETHERNET.sMacP47_32 & 0xff;
		 rule->srcMac_.octet[2]     = entry.is.ETHERNET.sMacP31_16 >> 8;
		 rule->srcMac_.octet[3]     = entry.is.ETHERNET.sMacP31_16 & 0xff;
		 rule->srcMac_.octet[4]     = entry.is.ETHERNET.sMacP15_0 >> 8;
		 rule->srcMac_.octet[5]     = entry.is.ETHERNET.sMacP15_0 & 0xff;
		 rule->srcMacMask_.octet[0] = entry.is.ETHERNET.sMacM47_32 >> 8;
		 rule->srcMacMask_.octet[1] = entry.is.ETHERNET.sMacM47_32 & 0xff;
		 rule->srcMacMask_.octet[2] = entry.is.ETHERNET.sMacM31_16 >> 8;
		 rule->srcMacMask_.octet[3] = entry.is.ETHERNET.sMacM31_16 & 0xff;
		 rule->srcMacMask_.octet[4] = entry.is.ETHERNET.sMacM15_0 >> 8;
		 rule->srcMacMask_.octet[5] = entry.is.ETHERNET.sMacM15_0 & 0xff;
		 rule->typeLen_             = entry.is.ETHERNET.ethTypeP;
		 rule->typeLenMask_         = entry.is.ETHERNET.ethTypeM;
		 rule->ruleType_            = RTL8651_ACL_MAC;
		 break;

	case 0x02: /* IP mask rule type */
	case 0x0A: /* IP range rule type*/
		 rule->tos_         = entry.is.L3L4.is.IP.IPTOSP;
		 rule->tosMask_     = entry.is.L3L4.is.IP.IPTOSM;
		 rule->ipProto_     = entry.is.L3L4.is.IP.IPProtoP;
		 rule->ipProtoMask_ = entry.is.L3L4.is.IP.IPProtoM;
		 rule->ipFlag_      = entry.is.L3L4.is.IP.IPFlagP;
		 rule->ipFlagMask_  = entry.is.L3L4.is.IP.IPFlagM;
 		 rule->ipFOP_ = entry.is.L3L4.is.IP.FOP;
		 rule->ipFOM_ = entry.is.L3L4.is.IP.FOM;
		 rule->ipHttpFilterM_ = entry.is.L3L4.is.IP.HTTPM;
		 rule->ipHttpFilter_  = entry.is.L3L4.is.IP.HTTPP;
		 rule->ipIdentSrcDstIp_ = entry.is.L3L4.is.IP.identSDIPM;
		 if (entry.ruleType==0x2)
	 		rule->ruleType_=RTL8651_ACL_IP;
		 else 
			rule->ruleType_=RTL8652_ACL_IP_RANGE;
		 goto l3l4_shared;

	case 0x03: /* IFSEL rule type */
		 rule->gidxSel_ = entry.is.IFSEL.gidxSel;
		 rule->ruleType_ = RTL8651_ACL_IFSEL;
	     break;
	     
	case 0x04: /* ICMP  (ip is mask) rule type */
	case 0x0C: /* ICMP (ip is  range) rule type */
		 rule->tos_ = entry.is.L3L4.is.ICMP.IPTOSP;
		 rule->tosMask_ = entry.is.L3L4.is.ICMP.IPTOSM;
		 rule->icmpType_ = entry.is.L3L4.is.ICMP.ICMPTypeP;
		 rule->icmpTypeMask_ = entry.is.L3L4.is.ICMP.ICMPTypeM;
		 rule->icmpCode_ = entry.is.L3L4.is.ICMP.ICMPCodeP;
		 rule->icmpCodeMask_ = entry.is.L3L4.is.ICMP.ICMPCodeM;
 		 if (entry.ruleType==0x4)
			 rule->ruleType_ = RTL8651_ACL_ICMP;
		 else
			 rule->ruleType_ = RTL8652_ACL_ICMP_IPRANGE;
		 goto l3l4_shared;

	case 0x05: /* IGMP (ip is mask) rule type */
	case 0x0D: /* IGMP (ip is range) rule type */	
		 rule->tos_ = entry.is.L3L4.is.IGMP.IPTOSP;
		 rule->tosMask_ = entry.is.L3L4.is.IGMP.IPTOSM;
		 rule->igmpType_ = entry.is.L3L4.is.IGMP.IGMPTypeP;
		 rule->igmpTypeMask_ = entry.is.L3L4.is.IGMP.IGMPTypeM;
 		 if (entry.ruleType==0x5)		 
			 rule->ruleType_ = RTL8651_ACL_IGMP;
		 else
			 rule->ruleType_ = RTL8652_ACL_IGMP_IPRANGE;
		 goto l3l4_shared;

	case 0x06: /* TCP rule type */
	case 0x0E:
		 rule->tos_ = entry.is.L3L4.is.TCP.IPTOSP;
		 rule->tosMask_ = entry.is.L3L4.is.TCP.IPTOSM;
		 rule->tcpFlag_ = entry.is.L3L4.is.TCP.TCPFlagP;
		 rule->tcpFlagMask_ = entry.is.L3L4.is.TCP.TCPFlagM;
		 rule->tcpSrcPortUB_ = entry.is.L3L4.is.TCP.TCPSPUB;
		 rule->tcpSrcPortLB_ = entry.is.L3L4.is.TCP.TCPSPLB;
		 rule->tcpDstPortUB_ = entry.is.L3L4.is.TCP.TCPDPUB;
		 rule->tcpDstPortLB_ = entry.is.L3L4.is.TCP.TCPDPLB;
	 	 if (entry.ruleType==0x6)
		 	rule->ruleType_ = RTL8651_ACL_TCP;
		 else
		 	rule->ruleType_ = RTL8652_ACL_TCP_IPRANGE;		 
         goto l3l4_shared;

	case 0x07: /* UDP rule type */
	case 0x0F:
		 rule->tos_ = entry.is.L3L4.is.UDP.IPTOSP;
		 rule->tosMask_ = entry.is.L3L4.is.UDP.IPTOSM;
		 rule->udpSrcPortUB_ = entry.is.L3L4.is.UDP.UDPSPUB;
		 rule->udpSrcPortLB_ = entry.is.L3L4.is.UDP.UDPSPLB;
		 rule->udpDstPortUB_ = entry.is.L3L4.is.UDP.UDPDPUB;
		 rule->udpDstPortLB_ = entry.is.L3L4.is.UDP.UDPDPLB;
		 if (entry.ruleType==0x7)
			 rule->ruleType_ = RTL8651_ACL_UDP;
		 else
			 rule->ruleType_ = RTL8652_ACL_UDP_IPRANGE;
l3l4_shared:
		rule->srcIpAddr_ = entry.is.L3L4.sIPP;
		rule->srcIpAddrMask_ = entry.is.L3L4.sIPM;
		rule->dstIpAddr_ = entry.is.L3L4.dIPP;
		rule->dstIpAddrMask_ = entry.is.L3L4.dIPM;
		break;

 	case 0x08: /* Source Filter */
	case 0x0B:	
	 	 rule->srcFilterMac_.octet[0]     = entry.is.SRC_FILTER.sMacP47_32 >> 8;
		 rule->srcFilterMac_.octet[1]     = entry.is.SRC_FILTER.sMacP47_32 & 0xff;
		 rule->srcFilterMac_.octet[2]     = entry.is.SRC_FILTER.sMacP31_16 >> 8;
		 rule->srcFilterMac_.octet[3]     = entry.is.SRC_FILTER.sMacP31_16 & 0xff;
		 rule->srcFilterMac_.octet[4]     = entry.is.SRC_FILTER.sMacP15_0 >> 8;
		 rule->srcFilterMac_.octet[5]     = entry.is.SRC_FILTER.sMacP15_0 & 0xff;
		 if ( entry.is.SRC_FILTER.sMacM3_0&0x8)		 
	 	{
			 rule->srcFilterMacMask_.octet[0] = 0xff;
			 rule->srcFilterMacMask_.octet[1] = 0xff;
			 rule->srcFilterMacMask_.octet[2] = 0xff;
			 rule->srcFilterMacMask_.octet[3] = 0xff;
			 rule->srcFilterMacMask_.octet[4] = 0xff;
	 		 rule->srcFilterMacMask_.octet[5] = 0xF0|entry.is.SRC_FILTER.sMacM3_0;
	 	}
		 else
	 	{
			 rule->srcFilterMacMask_.octet[0] = 0x0;
			 rule->srcFilterMacMask_.octet[1] = 0x0;
			 rule->srcFilterMacMask_.octet[2] = 0x0;
			 rule->srcFilterMacMask_.octet[3] = 0x0;
			 rule->srcFilterMacMask_.octet[4] = 0x0;
  		 	 rule->srcFilterMacMask_.octet[5] = entry.is.SRC_FILTER.sMacM3_0;
	 	}

		 rule->srcFilterPort_ = entry.is.SRC_FILTER.spaP;
		 rule->srcFilterVlanIdx_ = entry.is.SRC_FILTER.sVidP;
		 rule->srcFilterVlanIdxMask_ = entry.is.SRC_FILTER.sVidM;
		 if(entry.is.SRC_FILTER.protoType == 2) rule->srcFilterIgnoreL4_ = 1;
		 else if(entry.is.SRC_FILTER.protoType == 1) rule->srcFilterIgnoreL3L4_ = 1;
		 rule->srcFilterIpAddr_ = entry.is.SRC_FILTER.sIPP;
		 rule->srcFilterIpAddrMask_ = entry.is.SRC_FILTER.sIPM;
		 rule->srcFilterPortUpperBound_ = entry.is.SRC_FILTER.SPORTUB;
		 rule->srcFilterPortLowerBound_ = entry.is.SRC_FILTER.SPORTLB;
	 	 if (entry.ruleType==0x8)
			rule->ruleType_ = RTL8651_ACL_SRCFILTER;
		 else
		 	rule->ruleType_ = RTL8652_ACL_SRCFILTER_IPRANGE;
		 break;
		 
	case 0x09: /* Destination Filter */
	case 0x01: /* Destination Filter(IP range) */
	 	 rule->dstFilterMac_.octet[0]     = entry.is.DST_FILTER.dMacP47_32 >> 8;
		 rule->dstFilterMac_.octet[1]     = entry.is.DST_FILTER.dMacP47_32 & 0xff;
		 rule->dstFilterMac_.octet[2]     = entry.is.DST_FILTER.dMacP31_16 >> 8;
		 rule->dstFilterMac_.octet[3]     = entry.is.DST_FILTER.dMacP31_16 & 0xff;
		 rule->dstFilterMac_.octet[4]     = entry.is.DST_FILTER.dMacP15_0 >> 8;
		 rule->dstFilterMac_.octet[5]     = entry.is.DST_FILTER.dMacP15_0 & 0xff;
		 if ( entry.is.DST_FILTER.dMacM3_0&0x8)		 
	 	{
			 rule->dstFilterMacMask_.octet[0] = 0xff;
			 rule->dstFilterMacMask_.octet[1] = 0xff;
			 rule->dstFilterMacMask_.octet[2] = 0xff;
			 rule->dstFilterMacMask_.octet[3] = 0xff;
			 rule->dstFilterMacMask_.octet[4] = 0xff;
	 		 rule->dstFilterMacMask_.octet[5] = 0xF0|entry.is.DST_FILTER.dMacM3_0;
	 	}
		 else
	 	{
			 rule->dstFilterMacMask_.octet[0] = 0x0;
			 rule->dstFilterMacMask_.octet[1] = 0x0;
			 rule->dstFilterMacMask_.octet[2] = 0x0;
			 rule->dstFilterMacMask_.octet[3] = 0x0;
			 rule->dstFilterMacMask_.octet[4] = 0x0;
  		 	 rule->dstFilterMacMask_.octet[5] = entry.is.DST_FILTER.dMacM3_0;
	 	}

		
		 rule->dstFilterVlanIdx_ = entry.is.DST_FILTER.vidP;
		 rule->dstFilterVlanIdxMask_ = entry.is.DST_FILTER.vidM;
		 if(entry.is.DST_FILTER.protoType == 1) rule->dstFilterIgnoreL3L4_ = 1;
		 else if(entry.is.DST_FILTER.protoType == 2) rule->dstFilterIgnoreL4_ = 1;
		 rule->dstFilterIpAddr_ = entry.is.DST_FILTER.dIPP;
		 rule->dstFilterIpAddrMask_ = entry.is.DST_FILTER.dIPM;
		 rule->dstFilterPortUpperBound_ = entry.is.DST_FILTER.DPORTUB;
		 rule->dstFilterPortLowerBound_ = entry.is.DST_FILTER.DPORTLB;
 	 	 if (entry.ruleType==0x9)
			 rule->ruleType_ = RTL8651_ACL_DSTFILTER;
		 else
		 	rule->ruleType_ = RTL8652_ACL_DSTFILTER_IPRANGE;
		 break;		
	default: return FAILED; /* Unknown rule type */

	}

	rule->aclIdx = index;

	switch(entry.actionType) {
	
	case 0x00: /* Permit. Stop matching another rules */
			rule->actionType_ = RTL8651_ACL_PERMIT;
		 	goto _common_action;
		 		
	case 0x01: /* Redirect to ether. Stop matching another rules */ 	
			rule->actionType_ 	= RTL8651_ACL_REDIRECT;
			rule->dvid_ 		= entry.vid;
 		 	rule->nextHop_ 	= entry.nextHop;
 		 	rule->pppoeIdx_ 	= entry.PPPoEIndex;
		 	goto _common_action;

	case 0x03: /* Trap to CPU. Stop matching antoher rules */
			rule->actionType_ = RTL8651_ACL_CPU;
			goto _common_action;

	case 0x02: /* Silently Drop. Stop matching another rules */
	case 0x04: /* Silently Drop. Stop matching another rules */	
			rule->actionType_ = RTL8651_ACL_DROP;
		 	goto _common_action;
		 	
	case 0x05: /* Drop to CPU for logging. Stop matching another rules */
			rule->actionType_ = RTL8651_ACL_DROP_NOTIFY;
			goto _common_action;
	
	case 0x06: /* Mirror to system configured mirror port. Stop matching another rules */
			rule->actionType_	= RTL8651_ACL_MIRROR;
			goto _common_action;
	
	case 0x07: /* PPPoE redirect. Stop matching another rules */
			rule->actionType_ 	= RTL8651_ACL_REDIRECT_PPPOE;
			rule->dvid_ 		= entry.vid;
 		 	rule->nextHop_ 	= entry.nextHop;
 		 	rule->pppoeIdx_ 	= entry.PPPoEIndex;
		 	goto _common_action;
		 	
	case 0x08: /* Default redirect if permitted */
			rule->actionType_	= RTL8651_ACL_POLICY;
			rule->nhIndex	= entry.nextHop;
			goto _common_action;

	case 0x09: /* Default mirror to system configured mirror port, and keep on matching following rules */
			rule->actionType_	= RTL8651_ACL_MIRROR_KEEP_MATCH;
			goto _common_action;

	case 0x0a: /* Drop if rate exceeded. (PacketPerSecond) */
			rule->actionType_	= RTL8651_ACL_DROP_RATE_EXCEED_PPS;
			rule->rlIndex		= entry.nextHop;
			goto _common_action;

	case 0x0b: /* Log to CPU if rate exceeded. (PacketPerSecond) */
			rule->actionType_	= RTL8651_ACL_LOG_RATE_EXCEED_PPS;
			rule->rlIndex		= entry.nextHop;
			goto _common_action;

	case 0x0c: /* Drop if rate exceeded. (BytePerSecond) */
			rule->actionType_	= RTL8651_ACL_DROP_RATE_EXCEED_BPS;
			rule->rlIndex		= entry.nextHop;
			goto _common_action;

	case 0x0d: /* Log to CPU if rate exceeded. (BytePerSecond) */
			rule->actionType_	= RTL8651_ACL_LOG_RATE_EXCEED_BPS;
			rule->rlIndex		= entry.nextHop;
			goto _common_action;

	case 0x0e: /* Priority Selection */
			rule->actionType_	= RTL8651_ACL_PRIORITY;
			rule->priority	= entry.nextHop;
			goto _common_action;

	case 0x0f: /* RESERVED */

	default: return FAILED;

_common_action:
		/* handle pktOpApp */
		if ( entry.pktOpApp == 0x7 )
			rule->pktOpApp = RTL865XC_ACLTBL_ALL_LAYER;
		else if ( entry.pktOpApp == 0 )
			rule->pktOpApp = RTL8651_ACLTBL_NOOP;
		else
 			rule->pktOpApp = entry.pktOpApp;
 		break;
	}
	return SUCCESS;
}



/*=========================================
  * ASIC DRIVER API: PPPoE TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_PPPoE_TBL_API

int32 rtl8651_setAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) {
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL) || (pppoep->sessionId == 0xffff))
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.sessionID = pppoep->sessionId;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	entry.ageTime = pppoep->age;
#endif /*CONFIG_RTL865XB*/

	return _rtl8651_forceAddAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
}

int32 rtl8651_getAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) {
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
	pppoep->sessionId = entry.sessionID;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	pppoep->age = entry.ageTime;
#endif /*CONFIG_RTL865XB*/

	return SUCCESS;
}



/*=========================================
  * ASIC DRIVER API: L3 TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_L3_TBL_API

int32 rtl8651_setAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) {
	uint32 i, asicMask;
	rtl865xc_tblAsic_l3RouteTable_t entry;
#ifdef FPGA
	if (index==2) index=6;
	if (index==3) index=7;
	if (index>=4 && index <=5) 
		rtlglue_printf("Out of range\n");
#endif	
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	if (routingp->ipMask) {
		for(i=0; i<32; i++)
			if(routingp->ipMask & (1<<i)) break;
		asicMask = 31 - i;
	} else asicMask = 0;
    
	bzero(&entry, sizeof(entry));
	entry.IPAddr = routingp->ipAddr;
	switch(routingp->process) {
	case 0://PPPoE
		entry.linkTo.PPPoEEntry.PPPoEIndex = routingp->pppoeIdx;
		entry.linkTo.PPPoEEntry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.PPPoEEntry.IPMask = asicMask;
		entry.linkTo.PPPoEEntry.netif = routingp->vidx;
		entry.linkTo.PPPoEEntry.internal=routingp->internal;
		entry.linkTo.PPPoEEntry.isDMZ=routingp->DMZFlag;
		entry.linkTo.PPPoEEntry.process = routingp->process;
		entry.linkTo.PPPoEEntry.valid = 1;

		break;
	case 1://Direct
		entry.linkTo.L2Entry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.L2Entry.IPMask = asicMask;
		entry.linkTo.L2Entry.netif = routingp->vidx;
		entry.linkTo.L2Entry.internal=routingp->internal;
		entry.linkTo.L2Entry.isDMZ=routingp->DMZFlag;
		entry.linkTo.L2Entry.process = routingp->process;
		entry.linkTo.L2Entry.valid = 1;		
		break;
	case 2://Indirect
		entry.linkTo.ARPEntry.ARPEnd = routingp->arpEnd;
		entry.linkTo.ARPEntry.ARPStart = routingp->arpStart;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		entry.linkTo.ARPEntry.isDMZ = routingp->DMZFlag;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;

		entry.linkTo.ARPEntry.ARPIpIdx = routingp->arpIpIdx; /* for RTL8650B C Version Only */
		break;
	case 4://CPU
	case 6://DROP
		/*
		  *   Note:  although the process of this routing entry is CPU/DROP,
		  *             we still have to assign "vid" field for packet decision process use.
		  *                                                                                          - 2005.3.23 -
		  */
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		break;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	case 5://NAPT NextHop
		entry.linkTo.NxtHopEntry.nhStart = routingp->nhStart >> 1;
		switch (routingp->nhNum)
		{
		    case 2: entry.linkTo.NxtHopEntry.nhNum = 0; break;
		    case 4: entry.linkTo.NxtHopEntry.nhNum = 1; break;
		    case 8: entry.linkTo.NxtHopEntry.nhNum = 2; break;
		    case 16: entry.linkTo.NxtHopEntry.nhNum = 3; break;
		    case 32: entry.linkTo.NxtHopEntry.nhNum = 4; break;
		    default: return FAILED;
		}
		entry.linkTo.NxtHopEntry.nhNxt = routingp->nhNxt;
		entry.linkTo.NxtHopEntry.nhAlgo = routingp->nhAlgo;
		entry.linkTo.NxtHopEntry.IPMask = asicMask;
		entry.linkTo.NxtHopEntry.process = routingp->process;
		entry.linkTo.NxtHopEntry.valid = 1;
		entry.linkTo.NxtHopEntry.IPDomain = routingp->ipDomain;
		entry.linkTo.NxtHopEntry.internal = routingp->internal;
		entry.linkTo.NxtHopEntry.isDMZ = routingp->DMZFlag;
		break;
#endif /*CONFIG_RTL865XB*/
	default: return FAILED;
	}
    	return _rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

int32 rtl8651_delAsicRouting(uint32 index) {
	rtl8651_tblAsic_l3RouteTable_t entry;

	if(index >= RTL8651_ROUTINGTBL_SIZE)
		return FAILED;
	bzero(&entry, sizeof(entry));
	entry.linkTo.ARPEntry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

int32 rtl8651_getAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) {
	uint32 i;
	rtl865xc_tblAsic_l3RouteTable_t entry;
    
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	if(entry.linkTo.ARPEntry.valid == 0)
		return FAILED;

	routingp->ipAddr = entry.IPAddr;
	routingp->process = entry.linkTo.ARPEntry.process;
	for(i=0, routingp->ipMask = 0; i<=entry.linkTo.ARPEntry.IPMask; i++)
		routingp->ipMask |= 1<<(31-i);
    
    	routingp->vidx = entry.linkTo.ARPEntry.netif;
	routingp->internal= entry.linkTo.PPPoEEntry.internal;
	switch(routingp->process) {
	case 0://PPPoE
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = entry.linkTo.PPPoEEntry.PPPoEIndex;
		routingp->nextHopRow = entry.linkTo.PPPoEEntry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.PPPoEEntry.nextHop & 0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 1://Direct
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = entry.linkTo.L2Entry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.L2Entry.nextHop&0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 2://Indirect
		routingp->arpEnd = entry.linkTo.ARPEntry.ARPEnd;
		routingp->arpStart = entry.linkTo.ARPEntry.ARPStart;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->arpIpIdx = entry.linkTo.ARPEntry.ARPIpIdx;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 4: /* CPU */
	case 6: /* Drop */
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	case 5:
		routingp->nhStart = (entry.linkTo.NxtHopEntry.nhStart) << 1;
		switch (entry.linkTo.NxtHopEntry.nhNum)
		{
		case 0: routingp->nhNum = 2; break;
		case 1: routingp->nhNum = 4; break;
		case 2: routingp->nhNum = 8; break;
		case 3: routingp->nhNum = 16; break;
		case 4: routingp->nhNum = 32; break;
		default: return FAILED;
		}
		routingp->nhNxt = entry.linkTo.NxtHopEntry.nhNxt;
		routingp->nhAlgo = entry.linkTo.NxtHopEntry.nhAlgo;
		routingp->ipDomain = entry.linkTo.NxtHopEntry.IPDomain;
		routingp->internal= entry.linkTo.NxtHopEntry.internal;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		 
		break;
#endif
	default: return FAILED;
	}
    return SUCCESS;
}




/*=========================================
  * ASIC DRIVER API: ARP TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_ARP_TBL_API

int32 rtl8651_setAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) {
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.nextHop = (arpp->nextHopRow<<2) | (arpp->nextHopColumn&0x3);
	entry.valid = 1;
	entry.aging=arpp->aging;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_delAsicArp(uint32 index) {
	rtl8651_tblAsic_arpTable_t   entry;
	if(index >= RTL8651_ARPTBL_SIZE)
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_getAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) {
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ARP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;
	arpp->nextHopRow = entry.nextHop>>2;
	arpp->nextHopColumn = entry.nextHop&0x3;
	//arpp->aging=entry.aging&0x1f;

        // rtl865xC_tblAsicDrv.c,v 1.50
        arpp->aging=entry.aging;
	return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: IP TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_IP_TBL_API

int32 rtl8651_setAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp) {
	rtl8651_tblAsic_extIpTable_t   entry;
	
	if((index >= RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL) || 
	((extIntIpp->localPublic == TRUE) && (extIntIpp->nat == TRUE))) //Local public IP and NAT property cannot co-exist
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.externalIP = extIntIpp->extIpAddr;
	entry.internalIP = extIntIpp->intIpAddr;
	entry.isLocalPublic = extIntIpp->localPublic==TRUE? 1: 0;
	entry.isOne2One = extIntIpp->nat==TRUE? 1: 0;
    	entry.nextHop = extIntIpp->nhIndex;
	entry.valid = 1;
	return _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);

}

int32 rtl8651_delAsicExtIntIpTable(uint32 index) {
    	rtl8651_tblAsic_extIpTable_t   entry;

	if(index >= RTL8651_IPTABLE_SIZE)
		return FAILED;
        
	bzero(&entry, sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
}

int32 rtl8651_getAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp) {
    	rtl8651_tblAsic_extIpTable_t   entry;
    
	if((index>=RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL))
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;//Entry not found
	extIntIpp->extIpAddr = entry.externalIP;
	extIntIpp->intIpAddr = entry.internalIP;
	extIntIpp->localPublic = entry.isLocalPublic==1? TRUE: FALSE;
	extIntIpp->nat = entry.isOne2One==1? TRUE: FALSE;
    	extIntIpp->nhIndex = entry.nextHop;
    	return SUCCESS;
}


#if 0
/* Louis: for Hash2 
 * Louis: NEVER use this function when enabled multiple nexthop.
 */
int32  _rtl8651_findAsicExtIpTableIdx(ipaddr_t extIp) 
{
	rtl8651_tblAsic_extIpTable_t   entry;
	int32 i;
	for(i=0;  i<RTL8651_IPTABLE_SIZE; i++){
		_rtl8651_readAsicEntry(TYPE_EXT_INT_IP_TABLE, i, &entry);
		if(entry.valid == 1 && entry.externalIP== extIp && entry.isLocalPublic== 0 && entry.isOne2One==0)
			break;
	}
	if(i==RTL8651_IPTABLE_SIZE)
		return -1;
	return i;
}
#endif/*0*/


/*=========================================
  * ASIC DRIVER API: SERVER PORT TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_SERVER_PORT_TBL_API

int32 rtl8651_setAsicServerPortTable(uint32 index, rtl865x_tblAsicDrv_serverPortParam_t *serverPortp) {
	rtl865xc_tblAsic_srvPortTable_t   entry;
	if((index>=RTL8651_SERVERPORTTBL_SIZE) || (serverPortp == NULL))
		return FAILED;
        
	bzero(&entry, sizeof(entry));
	entry.externalIP = serverPortp->extIpAddr;
	entry.externalPort = serverPortp->extPort;
	entry.internalIP = serverPortp->intIpAddr;
	entry.internalPort = serverPortp->intPort;
	entry.PValid= serverPortp->pvaild;
	entry.PID= serverPortp->pid;	
	entry.isPortRange = serverPortp->portRange==TRUE? 1: 0;
	if ((serverPortp->protocol==0) && (serverPortp->valid==1))
		entry.protocol =3; /* Both TCP and UDP*/
      else
	  	entry.protocol =serverPortp->protocol;
	/* entry.traffic = serverPortp->traffic; Since rupert think software will NEVER write 1 to ASIC, so comment it. */
	return _rtl8651_forceAddAsicEntry(TYPE_SERVER_PORT_TABLE, index, &entry);
}

int32 rtl8651_delAsicServerPortTable(uint32 index) {
	rtl865xc_tblAsic_srvPortTable_t   entry;
	if(index>=RTL8651_SERVERPORTTBL_SIZE)
		return FAILED;
        
	bzero(&entry, sizeof(entry));
	entry.protocol= 0;
	return _rtl8651_forceAddAsicEntry(TYPE_SERVER_PORT_TABLE, index, &entry);
}

int32 rtl8651_getAsicServerPortTable(uint32 index, rtl865x_tblAsicDrv_serverPortParam_t *serverPortp) {
    	rtl865xc_tblAsic_srvPortTable_t   entry;
    
	if((index>=RTL8651_SERVERPORTTBL_SIZE) || (serverPortp == NULL))
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_SERVER_PORT_TABLE, index, &entry);
	if(entry.protocol == 0)
		return FAILED;
	serverPortp->extIpAddr = entry.externalIP;
	serverPortp->extPort = entry.externalPort;
	serverPortp->intIpAddr = entry.internalIP;
	serverPortp->intPort = entry.internalPort;
	serverPortp->portRange = entry.isPortRange==TRUE? 1: 0;
	serverPortp->valid = entry.protocol;
	serverPortp->protocol= entry.protocol;
	serverPortp->pid= entry.PID;	
	serverPortp->pvaild= entry.PValid;	
	serverPortp->traffic= entry.traffic;	
   	return SUCCESS;
}





int32 rtl8651_setAsicNaptAutoAddDelete(int8 autoAdd, int8 autoDelete) {
	if(autoAdd == TRUE)
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)|EN_NAPT_AUTO_LEARN);
	else
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~EN_NAPT_AUTO_LEARN);
	if(autoDelete == TRUE)
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)|EN_NAPT_AUTO_DELETE);
	else
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~EN_NAPT_AUTO_DELETE);
	return SUCCESS;
}

int32 rtl8651_getAsicNaptAutoAddDelete(int8 *autoAdd, int8 *autoDelete) {

	if(autoAdd){
		if(READ_MEM32(SWTCR0)&EN_NAPT_AUTO_LEARN)
			*autoAdd = TRUE;
		else
			*autoAdd = FALSE;
	}
	if(autoDelete){
		if(READ_MEM32(SWTCR0)&EN_NAPT_AUTO_DELETE)
			*autoDelete = TRUE;
		else
			*autoDelete = FALSE;
	}
	return SUCCESS;
}



/*=========================================
  * ASIC DRIVER API: NAPT TIMER
  *=========================================*/
#define RTL865X_ASIC_DRIVER_NAPT_TIMER_API

#if 1
int32 rtl8651_setAsicNaptIcmpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = (uint32)0x3f<< 24;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<24));
	return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>24) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptUdpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 18;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<18));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptUdpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>18) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpLongTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 12;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<12));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpLongTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>12) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpMediumTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 6;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<6));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpMediumTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>6) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpFastTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f ;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpFastTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)) & 0x3f);
	return SUCCESS;
}
#else
int32 rtl8651_setAsicNaptIcmpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = (uint32)0x3f<< 26;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<26));
	return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>26) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptUdpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 20;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<20));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptUdpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>20) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpLongTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 14;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<14));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpLongTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>14) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpMediumTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 8;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<8));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpMediumTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>8) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpFastTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 2;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<2));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpFastTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>2) & 0x3f);
	return SUCCESS;
}
#endif


/*=========================================
  * ASIC DRIVER API: NAPT TCP/UDP TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_TCPUDP_TBL_API

int32 rtl8651_setAsicLiberalNaptTcpUdpTable( int8 forced, uint16 index,
        ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, int8 selExtIPIdx, uint16 insideGlobalPort, 
        uint32 ageSec, int8 entryType, int8 isTcp, int8 isCollision, int8 isCollision2, int8 isValid ) 
{
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;
	int8 tcpQuiet, directional, outbound;

	//use liberal hash algorithm
	assert(entryType&RTL8651_STATIC_NAPT_ENTRY);
	assert(entryType & RTL8651_LIBERAL_NAPT_ENTRY);
	assert(index<RTL8651_TCPUDPTBL_SIZE);
	
	bzero(&entry, sizeof(entry));
	entry.collision = isCollision==TRUE? 1: 0;
	entry.collision2 = isCollision2==TRUE? 1: 0;
	entry.offset = insideGlobalPort>>RTL8651_TCPUDPTBL_BITS;

	// Edward said: col1 and col2 are always set.
	// To recover those code, cvs checkout version 1.3 (2004/01/14).
	entry.collision = entry.collision2 = 1;

	if(isValid){
		entry.valid = 1;
		entry.isStatic = entry.dedicate = 1;
		entry.intIPAddr = insideLocalIpAddr;
		entry.intPort = insideLocalPort;
		
		entry.agingTime = _rtl8651_NaptAgingToUnit(ageSec);

		/* TCPFlag is reused in RTL8651B when dedicate bit is set. 
		 * TCPFlag[2]= 1: Quiet	, 0: Trap SYN/FIN/RST to CPU
		 * TCPFlag[1]= 1: unidirectional,   0: bidirectional
		 * TCPFlag[0]= 1: outbound flow,   0: inbound flow
		 */
		tcpQuiet = (ISSET(entryType, RTL8651_NAPT_SYNFIN_QUIET))? 1:0;
		directional = (ISSET(entryType, RTL8651_NAPT_UNIDIRECTIONAL_FLOW))? 1:0;
		outbound = (directional&&ISSET(entryType, RTL8651_NAPT_OUTBOUND_FLOW))? 1:0;

		/* In fact, these two fields should be filled if entry is for outbound */
		entry.selEIdx = insideGlobalPort & (RTL8651_TCPUDPTBL_SIZE-1);
		entry.selIPIdx = selExtIPIdx;

		/* TCP flag field for HASH2 is {Quiet,Dir,Out}. */
		entry.TCPFlag = (tcpQuiet<<2)|(directional<<1) | (outbound);
		entry.isTCP= isTcp==TRUE? 1: 0;
	}
	else
	{
		entry.isStatic = entry.dedicate= entry.intIPAddr=entry.intPort =0;
		entry.agingTime =entry.selEIdx=entry.selIPIdx= entry.TCPFlag = 0;
	}

	if(forced == TRUE)
		return _rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	else if(_rtl8651_addAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry))
			return FAILED;
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpUdpTable(int8 forced, uint32 index, rtl865x_tblAsicDrv_naptTcpUdpParam_t *naptTcpUdpp) {
	rtl865xc_tblAsic_naptTcpUdpTable_t  entry;

	if((index >= RTL8651_TCPUDPTBL_SIZE) || (naptTcpUdpp == NULL))
		return FAILED;

	assert( naptTcpUdpp->tcpFlag < TCP_CLOSED_FLOW );

	memset(&entry, 0, sizeof(entry));
	entry.collision = 1; // always TRUE. naptTcpUdpp->isCollision==TRUE? 1: 0;
	entry.collision2 = 1; // always TRUE. naptTcpUdpp->isCollision2==TRUE? 1: 0;
	entry.valid = naptTcpUdpp->isValid==TRUE? 1: 0;
	entry.intIPAddr = naptTcpUdpp->insideLocalIpAddr;

	entry.intPort=naptTcpUdpp->insideLocalPort;
	entry.offset = naptTcpUdpp->offset;
	entry.agingTime = _rtl8651_NaptAgingToUnit(naptTcpUdpp->ageSec);
	entry.isStatic = naptTcpUdpp->isStatic==TRUE? 1: 0;
	entry.isTCP = naptTcpUdpp->isTcp==TRUE? 1: 0;
	entry.TCPFlag = naptTcpUdpp->tcpFlag;
	entry.dedicate = naptTcpUdpp->isDedicated==TRUE? 1: 0;
	entry.selIPIdx = naptTcpUdpp->selExtIPIdx;
	entry.selEIdx = naptTcpUdpp->selEIdx;

	// Edward said: col1 and col2 are always set.
	// To recover those code, cvs checkout version 1.3 (2004/01/14).
	entry.collision = entry.collision2 = 1;
	entry.priValid =  naptTcpUdpp->priValid;
	entry.priority = naptTcpUdpp->priority;
	
	if(forced == TRUE){
		_rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	}else if(_rtl8651_addAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry))
		return FAILED;
	return SUCCESS;
}


int32 rtl8651_getAsicRawNaptTable(uint32 index, void   *entry){
	/*entry is actually an rtl8651_tblAsic_naptTcpUdpTable_t pointer*/
	uint32  *pEntry=(uint32 *)entry;
	int32 retval;

	// WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) | 0x2); /*ASIC patch: disable L4 Aging aftrer reading L4 table */
	retval=_rtl8651_readAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, entry);
	//WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) & ~0x2); /*ASIC patch: enable L4 Aging aftrer reading NAPT table */

	//ASIC patch. static entry's valid bit would be cleared if aged out. 
	if(((pEntry[1]&0x1)==0)&&((pEntry[1]&0x10000)==0x10000))  
		pEntry[1] |=0x1;            

	return retval;
}

int32 rtl8651_setAsicRawNaptTable( uint32 index, void *vEntry, int8 forced )
{
	rtl8651_tblAsic_naptTcpUdpTable_t *entry = (rtl8651_tblAsic_naptTcpUdpTable_t *)vEntry;

	// Edward said: col1 and col2 are always set.
	// To recover those code, cvs checkout version 1.3 (2004/01/14).
	entry->collision = entry->collision2 = 1;
	if( forced == TRUE )
		return _rtl8651_forceAddAsicEntry( TYPE_L4_TCP_UDP_TABLE, index, vEntry );
	else if( _rtl8651_addAsicEntry( TYPE_L4_TCP_UDP_TABLE, index, vEntry ) )
		return FAILED;
	return SUCCESS;
}

	
int32 rtl8651_getAsicNaptTcpUdpTable(uint32 index, rtl865x_tblAsicDrv_naptTcpUdpParam_t *naptTcpUdpp) {
	rtl865xc_tblAsic_naptTcpUdpTable_t  entry;
	if((index >= RTL8651_TCPUDPTBL_SIZE) || (naptTcpUdpp == NULL))
		return FAILED;

	rtl8651_getAsicRawNaptTable( index, &entry);
    
	naptTcpUdpp->insideLocalIpAddr = entry.intIPAddr;
	naptTcpUdpp->insideLocalPort = entry.intPort;
	naptTcpUdpp->offset = entry.offset;
	naptTcpUdpp->ageSec = _rtl8651_NaptAgingToSec(entry.agingTime);
	naptTcpUdpp->isStatic = entry.isStatic;
	naptTcpUdpp->isTcp = entry.isTCP;
	naptTcpUdpp->isCollision = entry.collision;
	naptTcpUdpp->isCollision2 = entry.collision2;
	naptTcpUdpp->isValid = entry.valid;
	naptTcpUdpp->isDedicated = entry.dedicate;
	naptTcpUdpp->selExtIPIdx = entry.selIPIdx;
	naptTcpUdpp->selEIdx = entry.selEIdx;
	naptTcpUdpp->tcpFlag = entry.TCPFlag;

   	 return SUCCESS;
}

int32 rtl8651_delAsicNaptTcpUdpTable(uint32 start, uint32 end) {
	if((start >= RTL8651_TCPUDPTBL_SIZE) || (end >= RTL8651_TCPUDPTBL_SIZE))
		return FAILED;

	_rtl8651_delAsicEntry(TYPE_L4_TCP_UDP_TABLE, start, end);
	return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: NAPT ICMP TABLE
  *=========================================*/
#define RTL865X_ASIC_DRIVER_ICMP_TBL_API

int32 rtl8651_setAsicNaptIcmpTable(int8 forced, uint32 index, rtl865x_tblAsicDrv_naptIcmpParam_t *naptIcmpp) {
	rtl865xc_tblAsic_naptIcmpTable_t  entry;
	if((index >= RTL8651_ICMPTBL_SIZE) || (naptIcmpp == NULL))
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.intIPAddr = naptIcmpp->insideLocalIpAddr;
	entry.ICMPIDH = naptIcmpp->insideLocalId>>15;
	entry.ICMPIDL = naptIcmpp->insideLocalId&0x7fff;
	entry.agingTime = _rtl8651_NaptAgingToUnit(naptIcmpp->ageSec);
	entry.isStatic = naptIcmpp->isStatic;
	entry.offsetL = naptIcmpp->offset;
	entry.offsetH = naptIcmpp->offset>>10;
	entry.dir= naptIcmpp->direction;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	// ICMP Table: Set collision bit to 1
	assert( naptIcmpp->isCollision == TRUE );
#endif /* CONFIG_RTL865XB */
	entry.collision = 1;
   	entry.valid = 1;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
    	entry.type = naptIcmpp->isSpi==TRUE? 1: naptIcmpp->isPptp==TRUE? 2: 0;
#endif /* CONFIG_RTL865XB */
	if(forced == TRUE)
		_rtl8651_forceAddAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry);
	else if(_rtl8651_addAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry))
        	return FAILED;
    return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpTable(uint32 index, rtl865x_tblAsicDrv_naptIcmpParam_t *naptIcmpp) {
	rtl865xc_tblAsic_naptIcmpTable_t  entry;
	if((index >= RTL8651_ICMPTBL_SIZE) || (naptIcmpp == NULL))
		return FAILED;
         
	WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) | 0x2);/*ASIC patch: disable L4 Aging aftrer reading L4 table */    
	_rtl8651_readAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry);
	WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) & ~0x2); /*ASIC patch: enable L4 Aging aftrer reading ICMP table */

	naptIcmpp->insideLocalIpAddr = entry.intIPAddr;
	naptIcmpp->insideLocalId = (entry.ICMPIDH << 15) | entry.ICMPIDL;
	naptIcmpp->ageSec = _rtl8651_NaptAgingToSec(entry.agingTime);
	naptIcmpp->isStatic = entry.isStatic==1? TRUE: FALSE;
	naptIcmpp->isCollision = entry.collision==1? TRUE: FALSE;
	naptIcmpp->offset = entry.offsetL | (entry.offsetH <<10);
	naptIcmpp->isValid = entry.valid == 1? TRUE: FALSE;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	naptIcmpp->isSpi = entry.type==1? TRUE: FALSE;
	naptIcmpp->isPptp = entry.type>=2? TRUE: FALSE;
	naptIcmpp->count = entry.count;
	naptIcmpp->direction= entry.dir;
#endif /* CONFIG_RTL865XB */
	    
	return SUCCESS;
}



/*=========================================
  * ASIC DRIVER API: NAPT OFFSET
  *=========================================*/
#define RTL865X_ASIC_DRIVER_NAPT_OFFSET_API

int32 rtl8651_setAsicL4Offset(uint16 start, uint16 end) {
	if(start > 0x3f || end > 0x3f || start > end)
		return FAILED;

	WRITE_MEM32(OCR,start<<26 | end<<20);	
	return SUCCESS;
}

int32 rtl8651_getAsicL4Offset(uint16 *start, uint16 *end) {
	if(start == NULL || end == NULL)
		return FAILED;
	*start = (READ_MEM32(OCR)>>26) & 0x3f;
	*end = (READ_MEM32(OCR)>>20) & 0x3f;
	return SUCCESS;
}


#if 0
int32 rtl8651_getAsicNaptTcpUdpOffset(uint16 index, uint16 * offset, int8 * isValid) {
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;

	if(offset == NULL || isValid == NULL || index >= RTL8651_TCPUDPTBL_SIZE)
		return FAILED;

	rtl8651_getAsicRawNaptTable( index, &entry);

	*offset = entry.offset;
	*isValid = entry.valid == 1? TRUE: FALSE;
	
	return SUCCESS;
}
#endif

int32 rtl8651_getAsicNaptIcmpOffset(uint16 index, uint16 * offset, int8 * isValid) {
	rtl865xc_tblAsic_naptIcmpTable_t  entry;

	if(offset == NULL || isValid == NULL || index>= RTL8651_ICMPTBL_SIZE)
		return FAILED;
	
	WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) | 0x2); /*ASIC patch: disable L4 Aging while reading ICMP table */
	_rtl8651_readAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry);
	WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) & ~0x2); /*ASIC patch: enable L4 Aging aftrer reading ICMP table */

	*offset = entry.offsetL|(entry.offsetH<<10);
	*isValid = entry.valid==1? TRUE: FALSE;
	
	return SUCCESS;
}





/*=========================================
  * ASIC DRIVER API: ALG Table
  *=========================================*/
#define RTL865X_ASIC_DRIVER_ALG_TBL_API

int32 rtl8651_setAsicAlg(uint32 index, rtl865x_tblAsicDrv_algParam_t *algp) {
	rtl865xc_tblAsic_algTable_t entry;

	if((index >= RTL865XC_ALGTBL_SIZE) || (algp == NULL))
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.L4Port = algp->port;
	entry.direction = algp->direction;
	entry.protocol=algp->protocol;
	return _rtl8651_forceAddAsicEntry(TYPE_ALG_TABLE, index, &entry);
}

int32 rtl8651_delAsicAlg(uint32 index) {
	rtl865xc_tblAsic_algTable_t entry;

	if(index >= RTL865XC_ALGTBL_SIZE)
		return FAILED;

	bzero(&entry, sizeof(entry));
	entry.direction= 0;
	return _rtl8651_forceAddAsicEntry(TYPE_ALG_TABLE, index, &entry);
}

int32 rtl8651_getAsicAlg(uint32 index, rtl865x_tblAsicDrv_algParam_t *algp) {
	rtl865xc_tblAsic_algTable_t entry;

	if((index >= RTL865XC_ALGTBL_SIZE) || (algp == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_ALG_TABLE, index, &entry);

	if(entry.direction == 0)
		return FAILED;
	algp->direction= entry.direction;
	algp->protocol= entry.protocol;
	algp->port= entry.L4Port;
	return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: Next Hop Table
  *=========================================*/
#define RTL865X_ASIC_DRIVER_NEXTHOP_TBL_API

int32 rtl8651_setAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp) {
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index>=RTL8651_NEXTHOPTBL_SIZE) || (nextHopp == NULL))
        return FAILED;

	/* for debug
	rtlglue_printf("[%s:%d] rtl8651_setAsicNextHopTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    bzero(&entry, sizeof(entry));
    entry.nextHop = (nextHopp->nextHopRow <<2) | nextHopp->nextHopColumn;
    entry.PPPoEIndex = nextHopp->pppoeIdx;
    entry.dstVid = nextHopp->dvid;
    entry.IPIndex = nextHopp->extIntIpIdx;
    entry.type = nextHopp->isPppoe==TRUE? 1: 0;
    return _rtl8651_forceAddAsicEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
}

int32 rtl8651_getAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp) {
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index>=RTL8651_NEXTHOPTBL_SIZE) || (nextHopp == NULL))
        return FAILED;

    _rtl8651_readAsicEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
    nextHopp->nextHopRow = entry.nextHop>>2;
    nextHopp->nextHopColumn = entry.nextHop&0x3;
    nextHopp->pppoeIdx = entry.PPPoEIndex;
    nextHopp->dvid = entry.dstVid;
    nextHopp->extIntIpIdx = entry.IPIndex;
    nextHopp->isPppoe = entry.type==1? TRUE: FALSE;
    return SUCCESS;
}




/*=========================================
  * ASIC DRIVER API: Rate Limit
  *=========================================*/
#define RTL865X_ASIC_DRIVER_RATE_LIMIT_API
/* Time */
int32 rtl8651_setAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (rateLimit_t == NULL || index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	bzero(&entry, sizeof(rtl8651_tblAsic_rateLimitTable_t));
	entry.maxToken				= rateLimit_t->maxToken&0xFFFFFF;
	entry.refill				= rateLimit_t->refill_number&0xFFFFFF;
	entry.refillTime			= rateLimit_t->t_intervalUnit&0x3F;
	entry.refillRemainTime		= rateLimit_t->t_remainUnit&0x3F;
	entry.token					= rateLimit_t->token&0xFFFFFF;
	return _rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
}


int32 rtl8651_delAsicRateLimitTable(uint32 index)
{
	rtl8651_tblAsic_rateLimitTable_t entry;
	
	if (index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	bzero(&entry, sizeof(rtl8651_tblAsic_rateLimitTable_t));
	return _rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
}


int32 rtl8651_getAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (rateLimit_t == NULL || index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
	if (entry.refillTime == 0)
		return FAILED;
	rateLimit_t->token			= entry.token & 0xFFFFFF;
	rateLimit_t->maxToken		= entry.maxToken & 0xFFFFFF;
	rateLimit_t->t_remainUnit = entry.refillRemainTime&0x3F;
	rateLimit_t->t_intervalUnit = entry.refillTime&0x3F;
	rateLimit_t->refill_number	= entry.refill&0xFFFFFF;
	return SUCCESS;
}



int32 rtl8651_flowContrlThreshold(uint32 a, uint32 value)
{	
	uint32 tempValue;
	tempValue = READ_MEM32(0xbc805018) & ~((0xff)<<(8*a));
	tempValue = tempValue | (value<<(a*8));
	WRITE_MEM32(0xbc805018, tempValue);
	return SUCCESS;
}

int32 rtl8651_flowContrlPrimeThreshold(uint32 a, uint32 value)
{
	uint32 tempValue;
	tempValue = READ_MEM32(0xbc805028) & ~((0xffff)<<(16*a));
	tempValue = tempValue | (value<<(a*16));
	WRITE_MEM32(0xbc805028, tempValue);
	return SUCCESS;
}

/*
@func int32 | rtl8651_setFlowControl | Set Flow Control.
@parm uint32 | port | Physical port number.
@parm int8 | enable | TRUE: Enable flow control, FALSE: Disable flow control.
@rvalue TBLDRV_EINVALIDPORT | Invalid Port Number.
@rvalue SUCCESS | Success API call.
@comm
This API is used to set a port's flow control. 
*/

int32 rtl8651_setFlowControl(uint32 port, int8 enable)
{
	int32 retval;

	if (port >= RTL8651_PORT_NUMBER && port != RTL8651_CPU_PORT)
		return TBLDRV_EINVALIDPORT;
	rtlglue_drvMutexLock();
	retval = rtl8651_setAsicFlowControlRegister0(port, enable);
	rtlglue_drvMutexUnlock();
	return retval;
}

uint32 * regStoragePtr[16];
struct rtl_mBuf *mbufStorage[16];

#ifdef RTL865X_MODEL_USER
int32 rtl8651_resetSwitchCoreStore(void)
{
	/* In model code, we DO NOT support this function.
	 * Becuase we don't want to deal mbuf stuff in current.
	 */
	assert( 0 );
	return FAILED;
}
#else
int32 rtl8651_resetSwitchCoreStore(void) {
	uint32 i, j, k;
	uint32 *dataPtr;

	rtlglue_printf("%s (%s %d) Not Implemnt Yet\n",__FUNCTION__,__FILE__,__LINE__);
	return 0;
	/*Initial register storage space. Need to change to cluster model */
	for(i=0; i<16; i++) {
		mbufStorage[i] = NULL;
		regStoragePtr[i] = NULL;
		if((mbufStorage[i] = mBuf_get(MBUF_DONTWAIT, MBUFTYPE_DATA, 1)) == NULL) {
			while(i) {
				assert((i-1)<16);
				mBuf_freeMbufChain(mbufStorage[i-1]);
				mbufStorage[i-1] = NULL;
				i--;
			}
			return FAILED;
		}
		regStoragePtr[i] = (uint32 *)mbufStorage[i]->m_data;
	}

	/*Stop switch core operation through set TRXRDY to 0 */
	WRITE_MEM32(SIRR, READ_MEM32(SIRR) & (~TRXRDY) );

	/* Store L2 table using block 0~3 */
	i = 0;
	dataPtr = (uint32 *)0xbc000000; /* Layer2 table*/
	for(j=0; j<1024; j++) 
		for(k=0; k<2; k++) {
			regStoragePtr[(i>>9)][i&0x1FF] = dataPtr[(j<<3)+k];
			i++;
		}
	/* Store ARP table using block 4 */
	dataPtr = (uint32 *)0xbc010000;
	for(j=0; j<512; j++)
		regStoragePtr[4][j] = dataPtr[j<<3];
	/* Store L3 table/IP multicast/Protocol trap/VLAN/IP/ALG/Server port using block 5*/
	i = 0;
	dataPtr = (uint32 *)0xbc020000; /* L3 table */
	for(j=0; j<8; j++) 
		for(k=0; k<2; k++) 
			regStoragePtr[5][i++] = dataPtr[(j<<3)+k];
	dataPtr = (uint32 *)0xbc030000; /* IP multicast table */
	for(j=0; j<64; j++) 
		for(k=0; k<3; k++) 
			regStoragePtr[5][i++] = dataPtr[(j<<3)+k];
	dataPtr = (uint32 *)0xbc040000; /* Protocol trap table */
	for(j=0; j<8; j++) 
		regStoragePtr[5][i++] = dataPtr[(j<<3)];
	dataPtr = (uint32 *)0xbc050000; /* VLAN table */
	for(j=0; j<8; j++) 
		for(k=0; k<5; k++) 
			regStoragePtr[5][i++] = dataPtr[(j<<3)+k];
	dataPtr = (uint32 *)0xbc060000; /* IP table */
	for(j=0; j<16; j++) 
		for(k=0; k<3; k++) 
			regStoragePtr[5][i++] = dataPtr[(j<<3)+k];
	dataPtr = (uint32 *)0xbc070000; /* ALG table */
	for(j=0; j<128; j++) 
		regStoragePtr[5][i++] = dataPtr[(j<<3)];
	dataPtr = (uint32 *)0xbc080000; /* Server port table */
	for(j=0; j<16; j++) 
		for(k=0; k<4; k++) 
			regStoragePtr[5][i++] = dataPtr[(j<<3)+k];
	assert(i<512);
	i = 0;
	dataPtr = (uint32 *)0xbc090000; /* TCP/UDP table */
	for(j=0; j<1024; j++) 
		for(k=0; k<3; k++) {
			regStoragePtr[6+(i>>9)][i&0x1FF] = dataPtr[(j<<3)+k];
			i++;
		}
	/* Store ICMP/PPPoE/NextHop/RateLimit port using block 12*/
	i = 0;
	dataPtr = (uint32 *)0xbc0a0000; /* ICMP table */
	for(j=0; j<32; j++) 
		for(k=0; k<3; k++) 
			regStoragePtr[12][i++] = dataPtr[(j<<3)+k];
	dataPtr = (uint32 *)0xbc0b0000; /* PPPoE table */
	for(j=0; j<8; j++) 
		regStoragePtr[12][i++] = dataPtr[(j<<3)];
	dataPtr = (uint32 *)0xbc0d0000; /* NextHop table */
	for(j=0; j<32; j++) 
		regStoragePtr[12][i++] = dataPtr[(j<<3)];
	dataPtr = (uint32 *)0xbc0e0000; /* Rate limit port table */
	for(j=0; j<32; j++) 
		for(k=0; k<3; k++) 
			regStoragePtr[12][i++] = dataPtr[(j<<3)+k];
	assert(i<512);
	i = 0;
	dataPtr = (uint32 *)0xbc0c0000; /* ACL table */
	for(j=0; j<128; j++) 
		for(k=0; k<8; k++) {
			regStoragePtr[13+(i>>9)][i&0x1FF] = dataPtr[(j<<3)+k];
			i++;
		}
	/* Store the control registers */
	i = 0;
	regStoragePtr[15][i++] = READ_MEM32(CPUICR);
	regStoragePtr[15][i++] = READ_MEM32(CPUIIMR);
	regStoragePtr[15][i++] = READ_MEM32(MIB_CONTROL);
/*	regStoragePtr[15][i++] = READ_MEM32(MIB_CONTROL_1);
	regStoragePtr[15][i++] = READ_MEM32(MIB_CONTROL_2);*/
	regStoragePtr[15][i++] = READ_MEM32(PORT0_PHY_AUTONEGO_ADVERTISEMENT);
	regStoragePtr[15][i++] = READ_MEM32(PORT0_PHY_CONTROL);
	regStoragePtr[15][i++] = READ_MEM32(PORT1_PHY_AUTONEGO_ADVERTISEMENT);
	regStoragePtr[15][i++] = READ_MEM32(PORT1_PHY_CONTROL);
	regStoragePtr[15][i++] = READ_MEM32(PORT2_PHY_AUTONEGO_ADVERTISEMENT);
	regStoragePtr[15][i++] = READ_MEM32(PORT2_PHY_CONTROL);
	regStoragePtr[15][i++] = READ_MEM32(PORT3_PHY_AUTONEGO_ADVERTISEMENT);
	regStoragePtr[15][i++] = READ_MEM32(PORT3_PHY_CONTROL);
	regStoragePtr[15][i++] = READ_MEM32(PORT4_PHY_AUTONEGO_ADVERTISEMENT);
	regStoragePtr[15][i++] = READ_MEM32(PORT4_PHY_CONTROL);
#ifndef CONFIG_RTL865XC	
	for(j=SYSTEM_BASE; j<= SWTMCR; j+=4)
		regStoragePtr[15][i++] = READ_MEM32(j);
	for(j=MISC_BASE; j<= PBVR6E; j+=4)
		regStoragePtr[15][i++] = READ_MEM32(j);
#endif
	return SUCCESS;
}
#endif

#ifdef RTL865X_MODEL_USER
int32 rtl8651_resetSwitchCoreActionAndConfigure(void)
{
	/* In model code, we DO NOT support this function.
	 * Becuase we don't want to deal mbuf stuff in current.
	 */
	assert( 0 );
	return FAILED;
}
#else

static void _rtl8651_phyRegConfig(uint32 address, uint32 data) {
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
	WRITE_MEM32(TCR0, data);
	WRITE_MEM32(SWTAA, address);//Fill address
 	WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
}

int32 rtl8651_resetSwitchCoreActionAndConfigure(void) {
	uint32 i, j, k;
	uint32 entry[8];

	rtlglue_printf("%s (%s %d) Not Implemnt Yet\n",__FUNCTION__,__FILE__,__LINE__);
	return 0;

	/* Stop action while register storage is empty*/
	for(i=0; i<16; i++)
		if(regStoragePtr[i] == NULL)
			return FAILED;
	
	/* Full & Semi-Reset switch core*/
	WRITE_MEM32(MACCR, 0x3000000);
	
	/* Write back Layer2 table */
	bzero(&entry, sizeof entry);
	i=0;
	for(j=0; j<1024; j++) {
		for(k=0; k<2; k++) {
			entry[k] = regStoragePtr[(i>>9)][i&0x1FF];
			i++;
		}
		_rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, j, &entry);
	}
	/* Write back ARP table using block 4 */
	bzero(&entry, sizeof entry);
	for(j=0; j<512; j++) {
		entry[0] = regStoragePtr[4][j];
		_rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, j, &entry);
	}
	/* Write back L3 table/IP multicast/Protocol trap/VLAN/IP/ALG/Server port using block 5*/
	i = 0;
	 /* L3 table */
	bzero(&entry, sizeof entry);
	for(j=0; j<8; j++) {
		for(k=0; k<2; k++) 
			entry[k] = regStoragePtr[5][i++];
		_rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, j, &entry);
	}
	/* IP multicast table */
	bzero(&entry, sizeof entry);
	for(j=0; j<64; j++) {
		for(k=0; k<3; k++) 
			entry[k] = regStoragePtr[5][i++];
		_rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, j, &entry);
	}
	/* VLAN table */
	bzero(&entry, sizeof entry);
	for(j=0; j<8; j++) {
		for(k=0; k<5; k++) 
			entry[k] = regStoragePtr[5][i++];
		_rtl8651_forceAddAsicEntry(TYPE_VLAN_TABLE, j, &entry);
	}
	/* IP table */
	bzero(&entry, sizeof entry);
	for(j=0; j<16; j++) {
		for(k=0; k<3; k++) 
			entry[k] = regStoragePtr[5][i++];
		_rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, j, &entry);
	}
	/* ALG table */
	bzero(&entry, sizeof entry);
	for(j=0; j<128; j++) {
		entry[0] = regStoragePtr[5][i++];
		_rtl8651_forceAddAsicEntry(TYPE_ALG_TABLE, j, &entry);
	}
	/* Server port table */
	bzero(&entry, sizeof entry);
	for(j=0; j<16; j++) {
		for(k=0; k<4; k++) 
			entry[k] = regStoragePtr[5][i++];
		_rtl8651_forceAddAsicEntry(TYPE_SERVER_PORT_TABLE, j, &entry);
	}
	/* Write back TCP/UDP table */
	i = 0;
	bzero(&entry, sizeof entry);
	for(j=0; j<1024; j++) {
		for(k=0; k<3; k++) {
			entry[k] = regStoragePtr[6+(i>>9)][i&0x1FF];
			i++;
		}
		_rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, j, &entry);
	}

	/* Write back ICMP/PPPoE/NextHop/RateLimit port using block 12*/
	/* ICMP table */
	i = 0;
	bzero(&entry, sizeof entry);
	for(j=0; j<32; j++) {
		for(k=0; k<3; k++) 
			entry[k] = regStoragePtr[12][i++];
		_rtl8651_forceAddAsicEntry(TYPE_L4_ICMP_TABLE, j, &entry);
	}
	/* PPPoE table */
	bzero(&entry, sizeof entry);
	for(j=0; j<8; j++) {
		entry[0] = regStoragePtr[12][i++];
		_rtl8651_forceAddAsicEntry(TYPE_PPPOE_TABLE, j, &entry);
	}
	/* NextHop table */
	bzero(&entry, sizeof entry);
	for(j=0; j<32; j++) {
		entry[0] = regStoragePtr[12][i++];
		_rtl8651_forceAddAsicEntry(TYPE_NEXT_HOP_TABLE, j, &entry);
	}
	/* Rate limit port table */
	bzero(&entry, sizeof entry);
	for(j=0; j<32; j++) {
		for(k=0; k<3; k++) 
			entry[k] = regStoragePtr[12][i++];
		_rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, j, &entry);
	}
	i = 0;
	/* ACL table */
	for(j=0; j<128; j++) {
		for(k=0; k<8; k++) {
			entry[k] = regStoragePtr[13+(i>>9)][i&0x1FF];
			i++;
		}
		_rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE, j, &entry);
	}

	i = 0;
	WRITE_MEM32(CPUICR, regStoragePtr[15][i++]);
	WRITE_MEM32(CPUIIMR, regStoragePtr[15][i++]);
	WRITE_MEM32(MIB_CONTROL, regStoragePtr[15][i++]);
/*	WRITE_MEM32(MIB_CONTROL_1, regStoragePtr[15][i++]);
	WRITE_MEM32(MIB_CONTROL_2, regStoragePtr[15][i++]);
*/	_rtl8651_phyRegConfig(PORT0_PHY_AUTONEGO_ADVERTISEMENT, regStoragePtr[15][i++]);
	_rtl8651_phyRegConfig(PORT0_PHY_CONTROL , regStoragePtr[15][i++] | RESTART_AUTONEGO);
	_rtl8651_phyRegConfig(PORT1_PHY_AUTONEGO_ADVERTISEMENT, regStoragePtr[15][i++]);
	_rtl8651_phyRegConfig(PORT1_PHY_CONTROL , regStoragePtr[15][i++] | RESTART_AUTONEGO);
	_rtl8651_phyRegConfig(PORT2_PHY_AUTONEGO_ADVERTISEMENT, regStoragePtr[15][i++]);
	_rtl8651_phyRegConfig(PORT2_PHY_CONTROL, regStoragePtr[15][i++] | RESTART_AUTONEGO);
	_rtl8651_phyRegConfig(PORT3_PHY_AUTONEGO_ADVERTISEMENT, regStoragePtr[15][i++]);
	_rtl8651_phyRegConfig(PORT3_PHY_CONTROL, regStoragePtr[15][i++] | RESTART_AUTONEGO);
	_rtl8651_phyRegConfig(PORT4_PHY_AUTONEGO_ADVERTISEMENT, regStoragePtr[15][i++]);
	_rtl8651_phyRegConfig(PORT4_PHY_CONTROL, regStoragePtr[15][i++] | RESTART_AUTONEGO);
#ifndef CONFIG_RTL865XC	
	for(j=SYSTEM_BASE; j<= SWTMCR; j+=4)
		WRITE_MEM32(j, regStoragePtr[15][i++]);
	for(j=MISC_BASE; j<= PBVR6E; j+=4)
		WRITE_MEM32(j, regStoragePtr[15][i++]);

	/*xxx. Re-Enable switch core operation through set TRXRDY to 1 */
	WRITE_MEM32(SIRR, READ_MEM32(SIRR)|TRXRDY );

#endif		
	for(i=0; i<16; i++)
		if(mbufStorage[i])
			mBuf_freeMbufChain(mbufStorage[i]);

	return SUCCESS;
}
#endif


/*=========================================
  * ASIC DRIVER API: SYSTEM INIT
  *=========================================*/
#define RTL865X_ASIC_DRIVER_SYSTEM_INIT_API
#if 0
static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) {
	rtl8651_tblAsic_aclTable_t entry;
	uint32 idx;
	
	bzero(&entry, sizeof(entry));
	for (idx=0; idx<count; idx++)// Write into hardware
		_rtl8651_forceAddAsicEntry(type, idx, &entry);

}
#endif

/*=========================================
  * ASIC DRIVER API: TTL - 1
  *=========================================*/
#define RTL865X_ASIC_DRIVER_TTL_MINUS_API

/*
@func int32	| rtl8651_setAsicTtlMinusStatus 	| Enable / Disable ASIC TTL-1 for L3/4 packets.
@parm uint32	| enable	|	TRUE: TTL-1 is enabled FALSE: TTL-1 is disabled
@rvalue SUCCESS		|	Success
@comm
Enable / disable ASIC TTL - 1 function for L3/4 packet process.
It includes TTL-1 process and (TTL == 1) check.
 */
int32 rtl8651_setAsicTtlMinusStatus(int32 enable)
{
	if (TRUE == enable)
	{
		WRITE_MEM32(ALECR, (READ_MEM32(TTLCR) | TTL_1Enable));
	} else
	{
		WRITE_MEM32(ALECR, (READ_MEM32(TTLCR) &~ (TTL_1Enable)));
	}

	return SUCCESS;
}

/*
@func int32		| rtl8651_getAsicTtlMinusStatus 	| Get the status of ASIC TTL-1 for L3/4 packets.
@parm uint32*	| enable	| Pointer to get the TTL-1 status. TRUE: TTL-1 is enabled FALSE: TTL-1 is disabled
@rvalue SUCCESS	|	Success
@rvalue FAILED	|	Failed
@comm
Get the status of ASIC TTL-1 function for L3/4 packet.
 */
int32 rtl8651_getAsicTtlMinusStatus(int32 *enable)
{
	if (enable == NULL)
	{
		return FAILED;
	}

	*enable = (READ_MEM32(ALECR) & TTL_1Enable) ? TRUE : FALSE;

	return SUCCESS;
}
#endif

