/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
*
* Abstract: Switch core driver source code.
*
* $Author: jasonwang $
*
* ---------------------------------------------------------------
*/

#include <rtl_types.h>
#include <rtl_errno.h>
#include <rtl8196x/loader.h>  //wei edit
#include <linux/config.h>
#include <rtl8196x/asicregs.h>
#include <rtl8196x/swCore.h>
#include <rtl8196x/phy.h>

#define BICOLOR_LED 1

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *) (addr)) = (val)
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))

#define RTL8651_ETHER_AUTO_100FULL	0x00
#define RTL8651_ETHER_AUTO_100HALF	0x01
#define RTL8651_ETHER_AUTO_10FULL		0x02
#define RTL8651_ETHER_AUTO_10HALF	0x03
#define RTL8651_ETHER_AUTO_1000FULL	0x08
#define RTL8651_ETHER_AUTO_1000HALF	0x09
#define GIGA_PHY_ID	0x16
#define tick_Delay10ms(x) { int i=x; while(i--) __delay(5000); }
static int32 miiPhyAddress;

#define  REVR  0xB8000000
#define  RTL8196C_REVISION_A  0x80000001
#define  RTL8196C_REVISION_B  0x80000002




#ifndef RTL8197B
#if CONFIG_RTL865XC
static uint8 fidHashTable[]={0x00,0x0f,0xf0,0xff};

/*#define rtl8651_asicTableAccessAddrBase(type) (RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x10000 * (type)) */
#define		RTL8651_ASICTABLE_BASE_OF_ALL_TABLES		0xBB000000
#define		rtl8651_asicTableAccessAddrBase(type) (RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + ((type)<<16) )
#define 		RTL865X_FAST_ASIC_ACCESS
#define		RTL865XC_ASIC_WRITE_PROTECTION				/* Enable/Disable ASIC write protection */
#define		RTL8651_ASICTABLE_ENTRY_LENGTH (8 * sizeof(uint32))
#define		RTL865X_TLU_BUG_FIXED		1


#ifdef RTL865X_FAST_ASIC_ACCESS
static uint32 _rtl8651_asicTableSize[] =
{
	2 /*TYPE_L2_SWITCH_TABLE*/,
	1 /*TYPE_ARP_TABLE*/,
    2 /*TYPE_L3_ROUTING_TABLE*/,
	3 /*TYPE_MULTICAST_TABLE*/,
	1 /*TYPE_PROTOCOL_TRAP_TABLE*/,
	5 /*TYPE_VLAN_TABLE*/,
	3 /*TYPE_EXT_INT_IP_TABLE*/,
    1 /*TYPE_ALG_TABLE*/,
    4 /*TYPE_SERVER_PORT_TABLE*/,
    3 /*TYPE_L4_TCP_UDP_TABLE*/,
    3 /*TYPE_L4_ICMP_TABLE*/,
    1 /*TYPE_PPPOE_TABLE*/,
    8 /*TYPE_ACL_RULE_TABLE*/,
    1 /*TYPE_NEXT_HOP_TABLE*/,
    3 /*TYPE_RATE_LIMIT_TABLE*/,
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

static int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

 	WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return SUCCESS;
}

uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr,uint16 fid) {
    return ( macAddr->octet[0] ^ macAddr->octet[1] ^
                    macAddr->octet[2] ^ macAddr->octet[3] ^
                    macAddr->octet[4] ^ macAddr->octet[5] ^fidHashTable[fid]) & 0xFF;
}

static int32 rtl8651_setAsicL2Table(ether_addr_t	*mac, uint32 column)
{
	rtl865xc_tblAsic_l2Table_t entry;
	uint32	row;

	row = rtl8651_filterDbIndex(mac, 0);
	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN))
		return FAILED;
	if(mac->octet[5] != ((row^(fidHashTable[0])^ mac->octet[0] ^ mac->octet[1] ^ mac->octet[2] ^ mac->octet[3] ^ mac->octet[4] ) & 0xff))
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.mac47_40 = mac->octet[0];
	entry.mac39_24 = (mac->octet[1] << 8) | mac->octet[2];
	entry.mac23_8 = (mac->octet[3] << 8) | mac->octet[4];

//	entry.extMemberPort = 0;   
	entry.memberPort = 7;
	entry.toCPU = 1;
	entry.isStatic = 1;
//	entry.nxtHostFlag = 1;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	entry.agingTime = 0x03;
	
//	entry.srcBlock = 0;
	entry.fid=0;
	entry.auth=1;

	return _rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}
#endif
#endif

//------------------------------------------------------------------------
static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) 
{
	struct { uint32 _content[8]; } entry;
	uint32 idx;
	
	bzero(&entry, sizeof(entry));
	for (idx=0; idx<count; idx++)// Write into hardware
		swTable_addEntry(type, idx, &entry);
}

void FullAndSemiReset( void )
{
#if CONFIG_RTL865XC

/* FIXME: Currently workable for FPGA, may need further modification for real chip */

#if 1
    /* Perform full-reset for sw-core. */ 
    REG32(SIRR) |= FULL_RST;

	tick_Delay10ms(50);

	/* Enable TRXRDY */
	REG32(SIRR) |= TRXRDY;
#endif

#endif /* CONFIG_RTL865XC */
}

int32 rtl865xC_setAsicEthernetMIIMode(uint32 port, uint32 mode)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( mode != LINK_RGMII && mode != LINK_MII_MAC && mode != LINK_MII_PHY )
		return FAILED;

	if ( port == 0 )
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P0GMIICR, ( READ_MEM32( P0GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	else
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	return SUCCESS;

}

int32 rtl865xC_setAsicEthernetRGMIITiming(uint32 port, uint32 Tcomp, uint32 Rcomp)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( Tcomp < RGMII_TCOMP_0NS || Tcomp > RGMII_TCOMP_7NS || Rcomp < RGMII_RCOMP_0NS || Rcomp > RGMII_RCOMP_2DOT5NS )
		return FAILED;
	
	if ( port == 0 )
	{
		WRITE_MEM32(P0GMIICR, ( ( ( READ_MEM32(P0GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}
	else
	{
		WRITE_MEM32(P5GMIICR, ( ( ( READ_MEM32(P5GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetMII(uint32 phyAddress, int32 mode, int32 enabled)
{
	/* Input validation */
	if ( phyAddress < 0 || phyAddress > 31 )
		return FAILED;
	if ( mode != P5_LINK_RGMII && mode != P5_LINK_MII_MAC && mode != P5_LINK_MII_PHY )
		return FAILED;
	
	/* Configure driver level information about mii port 5 */
	if ( enabled )
	{
		if ( miiPhyAddress >= 0 && miiPhyAddress != phyAddress )
			return FAILED;

		miiPhyAddress = phyAddress;
	}
	else
	{
		miiPhyAddress = -1;
	}

	/* MII port MAC interface mode configuration */
	WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << P5_LINK_OFFSET ) );

	return SUCCESS;
}


int32 rtl8651_getAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 *rData)
{
	uint32 status;
	
	WRITE_MEM32( MDCIOCR, COMMAND_READ | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) );

#ifdef RTL865X_TEST
	status = READ_MEM32( MDCIOSR );
#else
#if defined(CONFIG_RTL8196C)
    if (REG32(REVR) == RTL8196C_REVISION_A)
	delay_ms(10);   //wei add, for 8196C_test chip patch. mdio data read will delay 1 mdc clock.
#else if defined(CONFIG_RTL8198) 
	delay_ms(10);
#endif
	do { status = READ_MEM32( MDCIOSR ); } while ( ( status & STATUS ) != 0 );
#endif

	status &= 0xffff;
	*rData = status;

	return SUCCESS;
}


/* rtl8651_setAsicEthernetPHYReg( phyid, regnum, data );
    //dprintf("\nSet enable_10M_power_saving01!\n");
    rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );*/

int32 rtl8651_setAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 wData)
{
	WRITE_MEM32( MDCIOCR, COMMAND_WRITE | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) | wData );

#ifdef RTL865X_TEST
#else
	while( ( READ_MEM32( MDCIOSR ) & STATUS ) != 0 );		/* wait until command complete */
#endif

	return SUCCESS;
}

int32 rtl8651_restartAsicEthernetPHYNway(uint32 port, uint32 phyid)
{
	uint32 statCtrlReg0;

	/* read current PHY reg 0 */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	/* enable 'restart Nway' bit */
	statCtrlReg0 |= RESTART_AUTONEGO;

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetPHY(uint32 port, int8 autoNegotiation, uint32 advCapability, uint32 speed, int8 fullDuplex, 
	uint32 phyId, uint32 isGPHY) 
{
	uint32 statCtrlReg0, statCtrlReg4, statCtrlReg9;

	/* ====================
		Arrange PHY reg 0
	   ==================== */

	/* Read PHY reg 0 (control register) first */
	rtl8651_getAsicEthernetPHYReg(phyId, 0, &statCtrlReg0);

	if ( autoNegotiation == TRUE )	
	{
		statCtrlReg0 |= ENABLE_AUTONEGO;
	}
	else
	{
		statCtrlReg0 &= ~ENABLE_AUTONEGO;

		/* Clear speed & duplex setting */
		if ( isGPHY )
			statCtrlReg0 &= ~SPEED_SELECT_1000M;
		statCtrlReg0 &= ~SPEED_SELECT_100M;
		statCtrlReg0 &= ~SELECT_FULL_DUPLEX;

		if ( speed == 1 )	/* 100Mbps, assume 10Mbps by default */
			statCtrlReg0 |= SPEED_SELECT_100M;

		if ( fullDuplex == TRUE )
			statCtrlReg0 |= SELECT_FULL_DUPLEX;
	}

	/* =============================================================
		Arrange PHY reg 4, if GPHY, also need to arrange PHY reg 9.
	   ============================================================= */
	rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg4 );

	/* Clear all capability */
	statCtrlReg4 &= ~CAP_100BASE_MASK;

	if ( isGPHY )
	{
		rtl8651_getAsicEthernetPHYReg( phyId, 9, &statCtrlReg9 );

		/* Clear all 1000BASE capability */
		statCtrlReg9 &= ~ADVCAP_1000BASE_MASK;
	}
	else
	{
		statCtrlReg9 = 0;
	}
	
	if ( advCapability == RTL8651_ETHER_AUTO_1000FULL )
	{
		statCtrlReg9 = statCtrlReg9 | CAPABLE_1000BASE_TX_FD | CAPABLE_1000BASE_TX_HD;
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_1000HALF )
	{
		statCtrlReg9 = statCtrlReg9 | CAPABLE_1000BASE_TX_HD;
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_100FULL )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_100HALF )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_10FULL )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD;
	}
	else if ( advCapability == RTL8651_ETHER_AUTO_10HALF )
	{
		statCtrlReg4 = statCtrlReg4 | CAPABLE_10BASE_TX_HD;
	}
	else
	{
//		RTL_WARN(RTL_MSG_GENERIC, "Invalid advertisement capability!");
		return FAILED;
	}

	/* ===============================
		Set PHY reg 4.
		Set PHY reg 9 if necessary.
	   =============================== */
	rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg4 );

	if ( isGPHY )
	{
		rtl8651_setAsicEthernetPHYReg( phyId, 9, statCtrlReg9 );
	}

	/* =================
		Set PHY reg 0.
	   ================= */
	rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg0 );

	/* =======================================================
		Restart Nway.
		If 'Nway enable' is FALSE, ASIC won't execute Nway.
	   ======================================================= */
	rtl8651_restartAsicEthernetPHYNway(port, phyId);

	return SUCCESS;
}


int32 rtl8651_setAsicFlowControlRegister(uint32 port, uint32 enable, uint32 phyid)
{
	uint32 statCtrlReg4;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4 );

	if ( enable && ( statCtrlReg4 & CAPABLE_PAUSE ) == 0 )
	{
		statCtrlReg4 |= CAPABLE_PAUSE;		
	}
	else if ( enable == 0 && ( statCtrlReg4 & CAPABLE_PAUSE ) )
	{
		statCtrlReg4 &= ~CAPABLE_PAUSE;
	}
	else
		return SUCCESS;	/* The configuration does not change. Do nothing. */

	rtl8651_setAsicEthernetPHYReg( phyid, 4, statCtrlReg4 );
	
	/* restart N-way. */
	rtl8651_restartAsicEthernetPHYNway(port, phyid);

	return SUCCESS;
}
static unsigned int ExtP5GigaPhyMode=0;
//====================================================================

#if defined(CONFIG_RTL8198)||defined(RTL8196C)
void Set_GPHYWB(unsigned int phyid, unsigned int page, unsigned int reg, unsigned int mask, unsigned int val)
{

	unsigned int data=0;
	unsigned int wphyid=0;	//start
	unsigned int wphyid_end=1;   //end
	if(phyid==999)
	{	wphyid=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	wphyid=phyid;
		wphyid_end=phyid+1;
	}

	for(; wphyid<wphyid_end; wphyid++)
	{
		//change page 
#ifdef RTL8196C
                if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}
		if(mask!=0)
		{
			rtl8651_getAsicEthernetPHYReg( wphyid, reg, &data);
			data=data&mask;
		}
#else
		if(page>=40)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}

		//rtl8651_getAsicEthernetPHYReg( wphyid, reg, &data);
		//data=data&mask;
#endif
		rtl8651_setAsicEthernetPHYReg( wphyid, reg, data|val  );


		//switch to page 0
		//if(page>=40)
		{	
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
			//rtl8651_setAsicEthernetPHYReg( wphyid, 30, 0  );
		}
		/*
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
		}
		*/
	}



}
#endif
#if defined(CONFIG_RTL8198)
void Setting_RTL8198_GPHY()
{
	int i=0;
	for(i=0; i<5; i++)
        REG32(PCRP0+i*4) |= (EnForceMode);

/*
#Access command format: phywb {all: phyID=0,1,2,3,4} {page} {RegAddr} {Bit location} {Bit value}
#寫page 72 必須先做 mdcmdio_cmd write $phyID 31 0x7 再寫 mdcmdio_cmd write $phyID 30 $PageNum
phywb all 72 21 15-0 0xe092
phywb all 72 22 15-0 0xe092
phywb all 72 23 15-0 0xe092
phywb all 72 24 15-0 0xe092
phywb all 72 25 15-0 0xe092
phywb all 72 26 15-0 0xe092
*/

	Set_GPHYWB(999, 72, 21, 0, 0xe092);
	Set_GPHYWB(999, 72, 22, 0, 0xe092);
	Set_GPHYWB(999, 72, 23, 0, 0xe092);
	Set_GPHYWB(999, 72, 24, 0, 0xe092);
	Set_GPHYWB(999, 72, 25, 0, 0xe092);	
	Set_GPHYWB(999, 72, 26, 0, 0xe092);	

/*
set PageNum 2; #All of GPHY register in the Page#2
#Array format = {{PhyID List1}  {RegAddr1 RegData1 RegAddr2 RegData2}, ...}

     set AFE_Reg 
 {{0 1 2 3 4}  { 4 0x80c8 6  0x0678 7  0x3620}
          {2}      {11 0x0063 12 0xeb65 13 0x51d1 14 0x5dcb 17  0x7e00}
          {3}      {23 0x0000 24 0x0000 26 0x0000}
          {4}      {19 0x0000 20 0x0000 21 0x0000 22 0x0000 23  0x0000 24 0x0f0a 25  0x5050    26 0x0000        27 0x000f}}


*/
       //phyid=all
	Set_GPHYWB(999, 2,  4, 0, 0x80c8);  
	Set_GPHYWB(999, 2,  6, 0, 0x0678);
	Set_GPHYWB(999, 2,  7, 0, 0x3620);	

       //phyid=2
	Set_GPHYWB( 2,   2, 11, 0, 0x0063);	
	Set_GPHYWB( 2,   2, 12, 0, 0xeb65);	
	Set_GPHYWB( 2,   2, 13, 0, 0x51d1);	
	Set_GPHYWB( 2,   2, 14, 0, 0x5dcb);	
	Set_GPHYWB( 2,   2, 17, 0, 0x7e00);		
	
       //phyid=3
	Set_GPHYWB( 3,   2, 23, 0, 0x0000);	
	Set_GPHYWB( 3,   2, 24, 0, 0x0000);	
	Set_GPHYWB( 3,   2, 26, 0, 0x0000);	

       //phyid=4
	Set_GPHYWB( 4,   2, 19, 0, 0x0000);	
	Set_GPHYWB( 4,   2, 20, 0, 0x0000);	
	Set_GPHYWB( 4,   2, 21, 0, 0x0000);	
	Set_GPHYWB( 4,   2, 22, 0, 0x0000);	
	Set_GPHYWB( 4,   2, 23, 0, 0x0000);	
	Set_GPHYWB( 4,   2, 24, 0, 0x0f0a);	
	Set_GPHYWB( 4,   2, 25, 0, 0x5050);	
	Set_GPHYWB( 4,   2, 26, 0, 0x0000);	
	Set_GPHYWB( 4,   2, 27, 0, 0x000f);		


	for(i=0; i<5; i++)
        REG32(PCRP0+i*4) &= ~(EnForceMode);
	
	dprintf("Set GPHY Parameter OK\n");
}
#endif
//====================================================================

//=====================================================================
#ifdef RTL8196C

void Setting_RTL8196C_PHY_REV_B()
{
	int i=0;
	for(i=0; i<5; i++)
        REG32(PCRP0+i*4) |= (EnForceMode);

/*
   #=========ADC Bias Current =========================
   #RG1X_P4~0 [12:10] = Reg_pi_fix [2:0], 5 ->7
   phywb all 1 17 12-10 0x7      

*/
	Set_GPHYWB(999, 1, 17, 0xffff-(7<<10), 0x7<<10);
	Set_GPHYWB(999, 4, 24, 0xff00, 0xf3);
	Set_GPHYWB(999, 4, 16, 0xffff-(1<<3), 1<<3);

	Set_GPHYWB(999, 1, 19, 0xffff-(7<<11), 0x2<<11);
	Set_GPHYWB(999, 1, 23, 0xffff-(7<<6)  , 0x4<<6);
	Set_GPHYWB(999, 1, 18, 0xffff-(7<<3), 0x6<<3);
	

	REG32(MACCR)= (REG32(MACCR)&0xfffffff0)|0x05;
	
	Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0x32<<0);

   	Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x4<<4);

   	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 0x1<<9);

	Set_GPHYWB(999, 1, 17, 0xffff-(3<<1), 0x3<<1);

	//Suggestion by Yozen
	Set_GPHYWB(999, 1, 18, 0xffff-(0xffff<<0), 0x9004<<0);

	Set_GPHYWB(999, 0, 21, 0xffff-(1<<14), 1<<14);
	for(i=0; i<5; i++)
        REG32(PCRP0+i*4) &= ~(EnForceMode);
	
	dprintf("Set 8196C PHY Patch OK\n");

}
#endif
int32 swCore_init()
{

#if CONFIG_RTL865XC
	uint32 rev;
	int port;
	
	/* Full reset and semreset */
	FullAndSemiReset();
#ifdef RTL8196C
		#ifdef CONFIG_RTL8196C_REVISION_B
		if (REG32(REVR) == RTL8196C_REVISION_B)
		Setting_RTL8196C_PHY_REV_B();
	#endif
#endif

#ifdef CONFIG_RTL8198
	Setting_RTL8198_GPHY();
#endif

	#if 1  /* May not be needed */
	/* rtl8651_clearAsicAllTable */
	#if defined(RTL8196C) || defined(RTL8198)
	// SERVER_PORT_TABLE, ALG_TABLE and L4_ICMP_TABLE are removed in real chip
	_rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_VLAN_TABLE, RTL865XC_VLAN_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, RTL8651_ACLTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);	
	#else		
	_rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_VLAN_TABLE, RTL865XC_VLAN_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ALG_TABLE, RTL8651_ALGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_SERVER_PORT_TABLE, RTL8651_SERVERPORTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_ICMP_TABLE, RTL8651_ICMPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, RTL8651_ACLTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);
	#endif
	#endif

#ifdef RTL_8652_5PORT
	if(ExtP1P4GigaPhyMode)
	{
		//dprintf("Setting PIT register\r\n");
		//P5 P4 P3 P2 P1 P0, offset: 10,8,6,4,2,0  
		//REG32(PITCR)=(0<<10) | (0x01 <<8) | (0x01<<6) | (0x01<<4) | (0x01 <<2)  | (0x03<<0 ); //set P0:reserver,  P1-P4:serdes, P5:RGMII
		REG32(PITCR)=(0<<10) | (0x01 <<8) | (0x01<<6) | (0x01<<4) | (0x01 <<2)  | (0x00<<0 ); //for anson test port0
		//REG32(PCRP0) = (GIGA_P0_PHYID<< ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		REG32(PCRP1) = (GIGA_P1_PHYID << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		REG32(PCRP2) = (GIGA_P2_PHYID << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		REG32(PCRP3) = (GIGA_P3_PHYID << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		REG32(PCRP4) = (GIGA_P4_PHYID << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
	}
	else
#else
	{
#ifdef RTL8197B
        REG32(PITCR) = REG32(PITCR) | (1<<0); //configure port 0 to be a MII interface
        rtl865xC_setAsicEthernetRGMIITiming(0, (0<<4), RGMII_RCOMP_2DOT5NS);
        rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_PHY); //port0 MII PHY mode

        REG32(PCRP0)= 0 | (0x16<<ExtPHYID_OFFSET) |
    	    			EnForceMode| ForceLink|ForceSpeed10M |ForceDuplex |
    					MIIcfg_RXER | AcptMaxLen_16K|EnablePHYIf;
        rtl8651_setAsicEthernetPHYReg(3, 0, 0x0800); //Port 3, BMCR, power down
#elif defined(RTL8196C)
		REG32(PCRP0) &= (0xFFFFFFFF-(0x00400000|MacSwReset));
                REG32(PCRP1) &= (0xFFFFFFFF-(0x00400000|MacSwReset));
                REG32(PCRP2) &= (0xFFFFFFFF-(0x00400000|MacSwReset));
                REG32(PCRP3) &= (0xFFFFFFFF-(0x00400000|MacSwReset));
                REG32(PCRP4) &= (0xFFFFFFFF-(0x00400000|MacSwReset));

		REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
#elif defined(RTL8198)
		REG32(PCRP0) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
                REG32(PCRP1) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
                REG32(PCRP2) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
                REG32(PCRP3) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
                REG32(PCRP4) &= (0xFFFFFFFF-(0x00000000|MacSwReset));

		REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
		REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
#else
#if 1 //96c remove
		//REG32(PCRP0) = (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		//REG32(PCRP1) = (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		//REG32(PCRP2) = (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		//REG32(PCRP3) = (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		//REG32(PCRP4) = (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		
		REG32(PCRP0) |= (AcptMaxLen_16K | EnablePHYIf);
		REG32(PCRP1) |= (AcptMaxLen_16K | EnablePHYIf);
		REG32(PCRP2) |= (AcptMaxLen_16K | EnablePHYIf);
		REG32(PCRP3) |= (AcptMaxLen_16K | EnablePHYIf);
		REG32(PCRP4) |= (AcptMaxLen_16K | EnablePHYIf);
#endif		
#endif
	}
#endif	

#if 0//!defined(RTL8196B)
	rev = ((READ_MEM32(CRMR)) >> 12) & 0x0f ;/*Runtime determined patch for A cut revison. RLRevID_OFFSET = 12, RLRevID_MASK = 0x0f */
	if (rev < 0x01) {
		rev = READ_MEM32((SYSTEM_BASE+0x3400+0x08));
		if(rev == 0x00)/*A Cut patch RTL865X_CHIP_REV_A = 0x00*/{
			REG32(PCRP6) = (6 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		}
	}   
#endif

	if(ExtP5GigaPhyMode)
		rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_RGMII);

	/*
		# According to Hardware SD: David & Maxod,			
		Set Port5_GMII Configuration Register.
		- RGMII Output Timing compensation control : 0 ns
		- RGMII Input Timing compensation control : 0 ns
	*/
	if(ExtP5GigaPhyMode)
	{
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);
		rtl8651_setAsicEthernetMII(GIGA_PHY_ID, P5_LINK_RGMII, TRUE);		
		WRITE_MEM32(PCRP5, (GIGA_PHY_ID<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf);		
	}
	#if 0 //96c remove
	/* Set forwarding status */
	REG32(PCRP0) = (REG32(PCRP0) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP1) = (REG32(PCRP1) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP2) = (REG32(PCRP2) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP3) = (REG32(PCRP3) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP4) = (REG32(PCRP4) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
#if !defined(RTL8196B)
	REG32(PCRP5) = (REG32(PCRP5) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
#endif
	#endif

	/* Set PVID of all ports to 8 */
	REG32(PVCR0) = (0x8 << 16) | 0x8;
	REG32(PVCR1) = (0x8 << 16) | 0x8;
	REG32(PVCR2) = (0x8 << 16) | 0x8;
	REG32(PVCR3) = (0x8 << 16) | 0x8;

	
	/* Enable L2 lookup engine and spanning tree functionality */
	// REG32(MSCR) = EN_L2 | EN_L3 | EN_L4 | EN_IN_ACL;
	REG32(MSCR) = EN_L2;
	REG32(QNUMCR) = P0QNum_1 | P1QNum_1 | P2QNum_1 | P3QNum_1 | P4QNum_1;

	/* Start normal TX and RX */
	REG32(SIRR) |= TRXRDY;
	
	/* Init PHY LED style */
#ifdef BICOLOR_LED
	extern unsigned int read_gpio_hw_setting();

	// REG32(LEDCR) = 0x01180000; // for bi-color LED
	unsigned int hw_val = read_gpio_hw_setting();

#ifdef RTL8196B //jason 0829
#ifdef RTL8197B
  #ifndef CONFIG_RTL8198
    REG32(0xb8000030) = REG32(0xb8000030) & (~0x00020000); //bit17, port 3 led
  #endif
#else
	//REG32(0xB8000030)= 0x00000000;
#endif
#endif
#if 1
#ifdef CONFIG_RTL8196C
	REG32(0xb8000040) &= ~(0x3FFFF);   //set pin mux for sw led. bit [17:0] =0
#endif
#endif
#ifndef RTL8196C
	if (hw_val == 0x2 || hw_val == 0x3 || hw_val == 0x6 || hw_val == 0x7)  // LED in matrix mode

	  	REG32(LEDCR)  = 0x155500;
		
	REG32(TCR0) = 0x000002c2;
	REG32(SWTAA) = PORT5_PHY_CONTROL;
	REG32(SWTACR) = ACTION_START | CMD_FORCE;
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
#endif
#else
	#if defined(BICOLOR_LED_VENDOR_BXXX)
		REG32(LEDCR) |= 0x00080000;

		REG32(PABCNR) &= ~0xc01f0000; /* set port a-7/6 & port b-4/3/2/1/0 to gpio */
		REG32(PABDIR) |=  0x401f0000; /* set port a-6 & port b-4/3/2/1/0 gpio direction-output */
		REG32(PABDIR) &= ~0x80000000; /* set port a-7 gpio direction-input */
	#else /* BICOLOR_LED_VENDOR_BXXX */
		REG32(LEDCR) = 0x00000000;
		REG32(TCR0) = 0x000002c7;
		REG32(SWTAA) = PORT5_PHY_CONTROL;
		REG32(SWTACR) = ACTION_START | CMD_FORCE;
		while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
	#endif /* BICOLOR_LED_VENDOR_BXXX */
#endif

	
	/*PHY FlowControl. Default enable*/
	for(port=0;port<MAX_PORT_NUMBER;port++)
	{
		/* Set Flow Control capability. */

		#if defined(RTL8196B) || defined(RTL8198)
        #ifdef RTL8197B
        if ((port==0) || (port==3))
            rtl8651_restartAsicEthernetPHYNway(port+1, port);
        #else
		rtl8651_restartAsicEthernetPHYNway(port+1, port);
        #endif	
		#else
		if (port == MAX_PORT_NUMBER-1)
			rtl8651_setAsicFlowControlRegister(RTL8651_MII_PORTNUMBER, TRUE, GIGA_PHY_ID);
		else				
			rtl8651_restartAsicEthernetPHYNway(port+1, port);	
		#endif			
			
	}
#ifdef RTL8196B //jason 1210
#ifndef RTL8197B
#ifndef CONFIG_RTL8196C
        //Green Ethernet
	{
	int uid=0,tmp=0;
	rtl8651_setAsicEthernetPHYReg( 6, 8, 0x232 );
	rtl8651_setAsicEthernetPHYReg( 6, 0, 0x1200 );
 
	//check write register
	rtl8651_getAsicEthernetPHYReg( 6, 0, &tmp );
	//
	rtl8651_getAsicEthernetPHYReg( 6, 8, &tmp );
	}
#endif
#endif
#endif
	#if defined(CONFIG_RTL865X_DIAG_LED)
	/* diagnosis led (gpio-porta-6) on */
	REG32(PABDAT) |=  0x40000000; /* pull high by set portA-0(bit 30) as gpio-output-1, meaning: diag led OFF */
	#endif /* CONFIG_RTL865X_DIAG_LED */
	

#ifndef RTL8197B
	{		
		extern char eth0_mac[6];
		extern char eth0_mac_httpd[6];
		rtl8651_setAsicL2Table((ether_addr_t*)(&eth0_mac), 0);
		rtl8651_setAsicL2Table((ether_addr_t*)(&eth0_mac_httpd), 1);
	}
#endif
	REG32(FFCR) = EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU; // rx broadcast and unicast packet
	return 0;
#endif /* CONFIG_RTL865XC */
}



#ifdef _HUB_MODE_

uint32 getLinkStatus( void )
{
	uint32 link = 0;
	uint32 i, dummy;

	for( i = 0; i < 5; i++ )
	{
		dummy = REG32(PHY_BASE+(i<<5) + 0x4); /* To get the correct link status, this register must be read twice. */
		if(REG32(PHY_BASE+(i<<5) + 0x4) & 0x4)
		{
			//link is up
			link |= (1<<i);
		}
	}

	return link;
}

/*
 *  HubMode() --
 *
 *  Ocuppy all L2 table entries to force broadcast.
 *
 */
void HubMode()
{

}
#endif //_HUB_MODE_
#if 0//UNUSED
#define BIT(x)     (1 << (x))
void set_phy_pwr_save(int val)
{
	int i;
	uint32 reg_val;
	
	for(i=0; i<5; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 24, &reg_val);

		if (val == 1)
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val | BIT(15)) );
		else 
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val & (~BIT(15))) );
		
//		rtl8651_restartAsicEthernetPHYNway(i+1, i);							
			}
}

#endif
