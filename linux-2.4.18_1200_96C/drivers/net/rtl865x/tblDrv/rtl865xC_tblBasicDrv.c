/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch table basic operation driver
* Abstract : 
* $Id: rtl865xC_tblBasicDrv.c,v 1.1 2007/12/04 11:54:01 joeylin Exp $
* $Log: rtl865xC_tblBasicDrv.c,v $
* Revision 1.1  2007/12/04 11:54:01  joeylin
* add hardware NAT feature
*
* Revision 1.9  2007/04/27 15:49:46  chenyl
* *: Control External PHY status and PHYID when initiating Driver. ( Originally its controlled by COMPILE FLAG)
* *: Move the "Large RateLimit Refill interval" chip version checking code into rtl8651_hwPatch.h
*
* Revision 1.8  2007/03/01 07:53:57  marklee
* Driver is modified for patch control
*
* Revision 1.7  2006/12/04 07:50:09  alva_zhang
* replace all printk with rtlglue_printf
*
* Revision 1.6  2006/11/20 05:17:30  shliu
* *: No need to stop HW table lookup process when accessing ASIC tables.
*
* Revision 1.5  2006/11/16 14:07:49  shliu
* *: not to stop TLU when reading ASIC table entries.
*
* Revision 1.4  2006/09/20 09:17:25  shliu
* *: change include "rtl8651_tblAsicDrv.h" to "rtl865xC_tblAsicDrv.h"
*
* Revision 1.3  2006/05/08 06:41:16  rupert
* *: START_TLU after Rread/Write Table
*
* Revision 1.2  2006/04/27 11:37:37  yjlou
* *: fixed endian bug of _rtl8651_readAsicEntry()
*
* Revision 1.1  2006/04/26 02:19:26  rupert
* +: Add rtl865xC Table Basic Driver
*
* Revision 1.8  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
*
* Revision 1.7  2005/02/03 03:09:36  yjlou
* +: add RTL865X_FAST_ASIC_ACCESS to accelerate ASIC table access.
*
* Revision 1.6  2004/08/09 02:15:55  chhuang
* *: code re-architecture--phase 1
*
* Revision 1.5  2004/07/27 10:45:05  cfliu
* no message
*
* Revision 1.4  2004/04/20 03:44:03  tony
* if disable define "RTL865X_OVER_KERNEL" and "RTL865X_OVER_LINUX", __KERNEL__ and __linux__ will be undefined.
*
* Revision 1.3  2004/03/26 07:17:21  danwu
* *** empty log message ***
*
* Revision 1.2  2004/03/19 13:13:35  cfliu
* Reorganize ROME driver local header files. Put all private data structures into different .h file corrsponding to its layering
* Rename printf, rtlglue_printf, malloc, free with rtlglue_XXX prefix
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.4  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/18 11:41:14  chhuang
* *** empty log message ***
*
* Revision 1.2  2004/02/18 07:02:04  chhuang
* *** empty log message ***
*
* Revision 1.1  2004/02/16 01:50:49  chhuang
* *** empty log message ***
*
* Revision 1.1  2003/12/30 14:16:23  cfliu
* initial version
*
* Revision 1.1  2003/12/26 05:26:40  chhuang
* init
*
* Revision 1.1  2003/12/18 02:30:06  danwu
* init
*
*/

#include "rtl_types.h"
#include "types.h"
#include "asicregs.h"
#include "asicTabs.h"
#include "rtl_glue.h"
#include "rtl865xC_tblAsicDrv.h"
/*
 *  According to ghhuang's suggest,
 *    we DO NOT need to access 8 ASIC entries at once.
 *  We just need to access several entries as we need.
 */


#define RTL865X_FAST_ASIC_ACCESS
#ifdef RTL865X_FAST_ASIC_ACCESS
static uint32 _rtl8651_asicTableSize[] =
{
        2 /*TYPE_L2_SWITCH_TABLE*/,
        1 /*TYPE_ARP_TABLE*/,
	 2 /*TYPE_L3_ROUTING_TABLE*/,
        3 /*TYPE_MULTICAST_TABLE*/,
        5 /*TYPE_NETIF_TABLE*/,
        3 /*TYPE_EXT_INT_IP_TABLE*/,
        3 /*TYPE_VLAN_TABLE*/,
        3 /*TYPE_VLAN1_TABLE*/,          
    4 /*TYPE_SERVER_PORT_TABLE*/,
    3 /*TYPE_L4_TCP_UDP_TABLE*/,
    3 /*TYPE_L4_ICMP_TABLE*/,
    1 /*TYPE_PPPOE_TABLE*/,
    8 /*TYPE_ACL_RULE_TABLE*/,
    1 /*TYPE_NEXT_HOP_TABLE*/,
    3 /*TYPE_RATE_LIMIT_TABLE*/,
    1 /*TYPE_ALG_TABLE*/,

};
#endif
static void _rtl8651_asicTableAccessForward(uint32 tableType, uint32 eidx, void *entryContent_P) {
	ASSERT_CSP(entryContent_P);


	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS

	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), *((uint32 *)entryContent_P + index));
		}
	}
#else
	WRITE_MEM32(TCR0, *((uint32 *)entryContent_P + 0));
	WRITE_MEM32(TCR1, *((uint32 *)entryContent_P + 1));
	WRITE_MEM32(TCR2, *((uint32 *)entryContent_P + 2));
	WRITE_MEM32(TCR3, *((uint32 *)entryContent_P + 3));
	WRITE_MEM32(TCR4, *((uint32 *)entryContent_P + 4));
	WRITE_MEM32(TCR5, *((uint32 *)entryContent_P + 5));
	WRITE_MEM32(TCR6, *((uint32 *)entryContent_P + 6));
	WRITE_MEM32(TCR7, *((uint32 *)entryContent_P + 7));
#endif	
	WRITE_MEM32(SWTAA, ((uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH));//Fill address
}

int32 _rtl8651_addAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);
	if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
      	WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	WRITE_MEM32(SWTACR, ACTION_START | CMD_ADD );//Activate add command

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
	if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
	{
		if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
			WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));	    
		return FAILED;
	}
	if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));	    
	return SUCCESS;
}

int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {

	if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));	
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

 	WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));	    

	return SUCCESS;
}

int32 _rtl8651_readAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {
	uint32 *    entryAddr;
	uint32 tmp;/*dummy variable, don't remove it*/
	if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}    
	ASSERT_CSP(entryContent_P);
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase(tableType) + (eidx<<5 /*RTL8651_ASICTABLE_ENTRY_LENGTH*/) ) ;
		/*(uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);*/

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
    
#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
		}
	}
#else
	*((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
	*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
	*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
	*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
	*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
	*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
	*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
	*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
#endif

	/* Dummy read. Must read an un-used table entry to refresh asic latch */
	tmp = *(uint32 *)((uint32) rtl8651_asicTableAccessAddrBase(TYPE_ACL_RULE_TABLE) + 1024 * RTL8651_ASICTABLE_ENTRY_LENGTH);
	if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));	    

	return 0;
}

int32 _rtl8651_delAsicEntry(uint32 tableType, uint32 startEidx, uint32 endEidx) {
  uint32 eidx = startEidx;

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), 0);
		}
	}
#else
	WRITE_MEM32(TCR0, 0);
	WRITE_MEM32(TCR1, 0);
	WRITE_MEM32(TCR2, 0);
	WRITE_MEM32(TCR3, 0);
	WRITE_MEM32(TCR4, 0);
	WRITE_MEM32(TCR5, 0);
	WRITE_MEM32(TCR6, 0);
	WRITE_MEM32(TCR7, 0);
#endif	
	
	while (eidx <= endEidx) {
		WRITE_MEM32(SWTAA, (uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);//Fill address
        
		WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command

		while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
		if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
			return FAILED;
		
		++eidx;
	}
	return SUCCESS;
}
