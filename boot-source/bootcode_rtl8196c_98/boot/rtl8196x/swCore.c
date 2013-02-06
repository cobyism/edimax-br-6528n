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
#define gphy_rom_loop 0
#ifdef RTL8198
#define port5_rgmii_gmii 1
#define rgmii 1
#else
#define port5_rgmii_gmii 0
#define rgmii 0
#endif
#define tmii 0

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
#define GIGA_P5_PHYID	0x5
#define tick_Delay10ms(x) { int i=x; while(i--) __delay(5000); }
static int32 miiPhyAddress;

#define  REVR  0xB8000000
#define  RTL8196C_REVISION_A  0x80000001
#define  RTL8196C_REVISION_B  0x80000002
#define  RTL8198_REVISION_A  0xC0000000
#define  RTL8198_REVISION_B  0xC0000001




#ifndef CONFIG_NFBI
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
//#ifndef CONFIG_RTL8198
#if defined(CONFIG_RTL8198_REVISION_B)
 if (REG32(REVR) == RTL8198_REVISION_A)
#endif
	{
	/* FIXME: Currently workable for FPGA, may need further modification for real chip */


	    /* Perform full-reset for sw-core. */ 
	    REG32(SIRR) |= FULL_RST;

		tick_Delay10ms(50);

		/* Enable TRXRDY */
		REG32(SIRR) |= TRXRDY;

	}

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
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	delay_ms(10);   //wei add, for 8196C_test chip patch. mdio data read will delay 1 mdc clock.
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
#if port5_rgmii_gmii
void ProbeP5GigaPHYChip()	
{
	unsigned int uid,tmp;
	unsigned int i;
//need mask esc interrupt: cary
	dprintf("In Setting port5 \r\n");
	//	delay_ms(10000); 
	//REG32(0xB8000010)=0x01FFFCEF;

	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/* Read */
	rtl8651_getAsicEthernetPHYReg(GIGA_P5_PHYID, 0, &tmp );
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID,0x10,0x01FE);

	dprintf("Read port5 phyReg0= 0x%x \r\n",tmp);

	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 2, &tmp );
	dprintf("Read port5 UPChipID= 0x%x \r\n",tmp);
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 3, &tmp );
	dprintf("Read port5 downChipID= 0x%x \r\n",tmp);
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{	dprintf("Find Port5   have 8211 PHY Chip! \r\n");
		ExtP5GigaPhyMode=1;
		//return 1;
	}	
	else
	{	dprintf("NO Find Port5 8211 PHY Chip! \r\n");
//		ExtP5GigaPhyMode=1;
		//return 1;
	}	
	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#if rgmii
	REG32(0xbb804118)=0x16F7003F;
#endif

}

void Setting_RTL8198_GPHY()
{
        int i=0;
        for(i=0; i<5; i++)
        REG32(PCRP0+i*4) |= (EnForceMode);
#if defined(CONFIG_RTL8198_REVISION_B)
 if (REG32(REVR) == RTL8198_REVISION_A)
#endif
{
/*
#Access command format: phywb {all: phyID=0,1,2,3,4} {page} {RegAddr} {Bit location} {Bit value}
#翹gpage 72 瞼簡繞繚瞼羸簞繕 mdcmdio_cmd write $phyID 31 0x7 礎A翹g mdcmdio_cmd write $phyID 30 $PageNum
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
 }
#if defined(CONFIG_RTL8198_REVISION_B)
 else if (REG32(REVR) == RTL8198_REVISION_B)
{
unsigned int p[]={
 //###################### PHY parameter patch ################################
0x1f, 0x0005, //Page 5
0x13, 0x0003, //Page 5 ########## EMA =3#############
//0x01, 0x0700; #Page 5 Reg 1 = 0x0700, NEC ON	(20100112)
0x05,0x8B82,//Page 5 Reg 5 = 0x8B82, Fix 100M re-link fail issue (20100110)
0x06,0x05CB,//Page 5 Reg 6 = 0x05CB, Fix 100M re-link fail issue (20100110)
0x1f,0x0002,//Page 2
0x04,0x80C2,//Page 2 Reg 4 0x80C2, Fix 100M re-link fail issue (20100110)
0x05,0x0938,//Page 2 Reg 5 0x0938, Disable 10M standby mode (20100112)

0x1F,0x0003,//Page 3
0x12,0xC4D2,//Page 3 Reg 18 = 0xC4D2, GAIN upper bond=24
0x0D,0x0207,//Page 3 Reg 13 = 0x0207 (20100112)
0x01,0x3554, //#Page 3 Reg  1 = 0x3554 (20100423)
0x02,0x63E8, //#Page 3 Reg  2 = 0x63E8 (20100423)
0x03,0x99C2, //#Page 3 Reg  3 = 0x99C2 (20100423)
0x04,0x0113, //#Page 3 Reg  4 = 0x0113 (20100423)

0x1f,0x0001,//Page 1
0x07,0x267E,//Page 1 Reg  7 = 0x267E, Channel Gain offset (20100111)
0x1C,0xE5F7,//Page 1 Reg 28 = 0xE5F7, Cable length offset (20100111)
0x1B,0x0424,//Page 1 Reg 27 = 0x0424, SD threshold (20100111)

//#Add by Gary for Channel Estimation fine tune 20100430            
//0x1f,0x0002, //# change to Page 1 (Global)
//0x08,0x0574, //# Page1 Reg8 (CG_INITIAL_MASTER)
//0x09,0x2724, //# Page1 Reg9 (CB0_INITIAL_GIGA) 
//0x1f,0x0003, //# change to Page 3 (Global)
//0x1a,0x06f6, //# Page3 Reg26 (CG_INITIAL_SLAVE)

//#Add by Gary for Channel Estimation fine tune 20100430            
//#Page1 Reg8 (CG_INITIAL_MASTER)
//0x1f, 0x0005,
//0x05, 0x83dd,
//0x06, 0x0574,
//#Page1 Reg9 (CB0_INITIAL_GIGA)
//0x1f, 0x0005,
//0x05, 0x83e0,
//0x06, 0x2724,
//#Page3 Reg26 (CG_INITIAL_SLAVE)
//0x1f, 0x0005,
//0x05, 0x843d,
//0x06, 0x06f6 ,

//#NC FIFO
0x1f,0x0007,//ExtPage
0x1e,0x0042,//ExtPage 66
0x18,0x0000,//Page 66 Reg 24 = 0x0000, NC FIFO (20100111)
0x1e,0x002D,//ExtPage 45
0x18,0xF010,//Page 45 Reg 24 = 0xF010, Enable Giga Down Shift to 100M (20100118)

0x1e,0x002c, //#ExtPage 44
0x18,0x008B, //#Page 44 Reg 24 = 0x008B, Enable deglitch circuit (20100426)
//############################ EEE giga patch ################################

//0x1f 0x0007;   
0x1e,0x0028,
0x16,0xf640,//phywb $phyID 40 22 15-0 0xF640

0x1e,0x0021,	
0x19,0x2929,//phywb $phyID 33 25 15-0 0x2929

0x1a,0x1005,//phywb $phyID 33 26 15-0 0x1005

0x1e,0x0020,	
0x17,0x000a,//phywb $phyID 32 23 15-0 0x000a

0x1b,0x2f4a,//Disable EEE PHY mode
0x15,0x0100,//EEE ability, Disable EEEP

0x1e,0x0040,//
0x1a,0x5110,//	phywb $phyID 64 26 15-0 0x5110
0x18,0x0000,// programable mode

0x1e,0x0041,//
0x15,0x0e02,//phywb $phyID 65 21 15-0 0x0e02

0x16,0x2185,//phywb $phyID 65 22 15-0 0x2185
0x17,0x000c,//phywb $phyID 65 23 15-0 0x000c
0x1c,0x0008,//phywb $phyID 65 28 15-0 0x0008
0x1e,0x0042,//
0x15,0x0d00,//phywb $phyID 66 21 15-0 0x0d00

};
unsigned int default_val[]={
//portid, reg, val	
//#Date: 20100421 by Garyyang  
//#This Patch is fix the RL6121 Analog AFE Default error peoblem
//#We need to force the AFE register value to default value
//{
999,0x1f,0x0002,
//}
2,0x11,0x7e00,

3,0x1f,0x0002,
3,0x17,0xff00,
3,0x18,0x0005,
3,0x19,0x0005,
3,0x1a,0x0005,
3,0x1b,0x0005,
3,0x1c,0x0005,

4,0x1f,0x0002,
4,0x13,0x00aa,
4,0x14,0x00aa,
4,0x15,0x00aa,
4,0x16,0x00aa,
4,0x17,0x00aa,
4,0x18,0x0f0a,
4,0x19,0x50ab,
4,0x1a,0x0000,
4,0x1b,0x0f0f,

999,0x1f,0x0000,

				};

int port;
	int len=sizeof(default_val)/sizeof(unsigned int);
	for(i=0;i<len;i=i+3)
	{	
	
		if(default_val[i]==999)
		{
			for(port=0; port<5; port++)
				rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
		}
		else
		{
			port=default_val[i];
			rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
				//__delay(delay_cnt);
		}
	}



	len=sizeof(p)/sizeof(unsigned int);

	for(port=0; port<5; port++)
	{
		for(i=0;i<len;i=i+2)
		{	
			rtl8651_setAsicEthernetPHYReg(port, p[i], p[i+1]);
			//__delay(delay_cnt);
		}
	}
}
#endif
 
        for(i=0; i<5; i++)
        REG32(PCRP0+i*4) &= ~(EnForceMode);
 
        dprintf("Set GPHY Parameter OK\n");
}
#endif

#if gphy_rom_loop
void Setting_RTL8198_GPHY_ROM_BIST_LOOP()
{
	int i=0;

	for(i=0; i<5; i++)
        REG32(PCRP0+i*4) |= (EnForceMode);

/*
write rg31 0x0000 (切到 page 0)
write rg21 0x0006 (關掉 [12]:enpwrsave)
write rg31 0x0005 (切到 page 5)
write rg05 0xfff6
write rg06 0x0000 (關掉所有 interrupt)
write rg15 0x0002 (開啟 hardware channel to indirectly access UC memory)
*/


	Set_GPHYWB(999, 0, 6, 0, 0x6);
	Set_GPHYWB(999, 5, 5, 0, 0xfff6);
	Set_GPHYWB(999, 5, 6, 0, 0);
	Set_GPHYWB(999, 5, 15, 0, 0x2);




//執行附檔的 patch code

	Set_GPHYWB(999, 5, 5 ,0 ,0x8000);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0280);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0702);
	Set_GPHYWB(999, 5, 6 ,0 ,0x803f);
	Set_GPHYWB(999, 5, 6 ,0 ,0x05f8);
	Set_GPHYWB(999, 5, 6 ,0 ,0xf9bf);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 6 ,0 ,0x06d8);
	Set_GPHYWB(999, 5, 6 ,0 ,0x2019);
	Set_GPHYWB(999, 5, 6 ,0 ,0xa940);
	Set_GPHYWB(999, 5, 6 ,0 ,0x00f9);
	Set_GPHYWB(999, 5, 6 ,0 ,0xcd8f);
	Set_GPHYWB(999, 5, 6 ,0 ,0xfabb);
	Set_GPHYWB(999, 5, 6 ,0 ,0x9f0f);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe085);
	Set_GPHYWB(999, 5, 6 ,0 ,0x00e1);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8501);
	Set_GPHYWB(999, 5, 6 ,0 ,0x14e4);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8500);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe585);
	Set_GPHYWB(999, 5, 6 ,0 ,0x01ae);
	Set_GPHYWB(999, 5, 6 ,0 ,0x13e0);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8502);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe185);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0414);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe485);
	Set_GPHYWB(999, 5, 6 ,0 ,0x02e5);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8504);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe685);
	Set_GPHYWB(999, 5, 6 ,0 ,0x06e7);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8507);
	Set_GPHYWB(999, 5, 6 ,0 ,0xfdfc);
	Set_GPHYWB(999, 5, 6 ,0 ,0x04f8);
	Set_GPHYWB(999, 5, 6 ,0 ,0xf9bf);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 6 ,0 ,0xd53f);
	Set_GPHYWB(999, 5, 6 ,0 ,0xff06);
	Set_GPHYWB(999, 5, 6 ,0 ,0xd820);
	Set_GPHYWB(999, 5, 6 ,0 ,0x1f95);
	Set_GPHYWB(999, 5, 6 ,0 ,0xd820);
	Set_GPHYWB(999, 5, 6 ,0 ,0x1f95);
	Set_GPHYWB(999, 5, 6 ,0 ,0x19a9);
	Set_GPHYWB(999, 5, 6 ,0 ,0x2000);
	Set_GPHYWB(999, 5, 6 ,0 ,0xf3cd);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8ffa);
	Set_GPHYWB(999, 5, 6 ,0 ,0xbb9f);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0fe0);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8500);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe185);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0114);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe485);
	Set_GPHYWB(999, 5, 6 ,0 ,0x00e5);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8501);
	Set_GPHYWB(999, 5, 6 ,0 ,0xae13);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe085);
	Set_GPHYWB(999, 5, 6 ,0 ,0x03e1);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8505);
	Set_GPHYWB(999, 5, 6 ,0 ,0x14e4);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8503);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe585);
	Set_GPHYWB(999, 5, 6 ,0 ,0x05e6);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8508);
	Set_GPHYWB(999, 5, 6 ,0 ,0xe785);
	Set_GPHYWB(999, 5, 6 ,0 ,0x09fd);
	Set_GPHYWB(999, 5, 6 ,0 ,0xfc04);
	Set_GPHYWB(999, 5, 5 ,0 ,0x8500);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0000);
	Set_GPHYWB(999, 5, 5 ,0 ,0x8bbc);
	Set_GPHYWB(999, 5, 6 ,0 ,0x8000);
	Set_GPHYWB(999, 5, 5 ,0 ,0xfff6);
	Set_GPHYWB(999, 5, 6 ,0 ,0x0040);

/*
請插線做 EEPROM 的 patch，
並讓 5 個 link partner 都 force 在 giga slave，
也就是讓我們的 5 個 ports 都 link 在 giga master，
*/
	Set_GPHYWB(999, 0, 9 ,0 ,0x1E00);


	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
	
	dprintf("Set GPHY_ROM_BIST_LOOP OK\n");
}
#endif

//====================================================================
#ifdef RTL8196C
#ifdef CONFIG_RTL8196C_ETH_IOT
void set_gray_code_by_port(int port) 
{ 
       uint32 val; 

       rtl8651_setAsicEthernetPHYReg( 4, 31, 1  ); 

       rtl8651_getAsicEthernetPHYReg( 4, 20, &val  ); 
       rtl8651_setAsicEthernetPHYReg( 4, 20, val + (0x1 << port)  ); 

       rtl8651_setAsicEthernetPHYReg( port, 31, 1  ); 

       rtl8651_setAsicEthernetPHYReg( port, 19,  0x5400 ); 
       if (port<4) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5440 ); 
       if (port<3) rtl8651_setAsicEthernetPHYReg( port, 19,  0x54c0 ); 
       if (port<2) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5480 ); 
       if (port<1) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5580 ); 

       rtl8651_setAsicEthernetPHYReg( 4, 20, 0xb20  ); 
       rtl8651_setAsicEthernetPHYReg( port, 31, 0  );
 
       rtl8651_setAsicEthernetPHYReg( 4, 31, 0  ); 
} 
#endif 	


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

        Set_GPHYWB(999, 1, 18, 0xffff-(0xffff<<0), 0x9004<<0);
 
        Set_GPHYWB(999, 0, 21, 0xffff-(1<<14), 1<<14);
        for(i=0; i<5; i++)
	        REG32(PCRP0+i*4) &= ~(EnForceMode);	
#ifdef CONFIG_RTL8196C_ETH_IOT 
       for(i=0; i<5; i++) { 
               set_gray_code_by_port(i); 
       } 
#endif 
	
	dprintf("Set 8196C PHY Patch OK\n");

}
#endif

int32 swCore_init()
{

#if CONFIG_RTL865XC
	uint32 rev;
	int port;
#if port5_rgmii_gmii
	int i;
#endif

	/* Full reset and semreset */
	FullAndSemiReset();
#ifdef RTL8196C
	#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
		Setting_RTL8196C_PHY_REV_B();
	#endif
#endif

#ifdef CONFIG_RTL8198
#if port5_rgmii_gmii
//	ProbeP5GigaPHYChip();
#endif

	Setting_RTL8198_GPHY();
#if gphy_rom_loop
	Setting_RTL8198_GPHY_ROM_BIST_LOOP();
#endif
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
#ifdef CONFIG_NFBI
#if defined(RTL8198)
	REG32(PCRP0) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP1) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP2) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP3) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP4) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP5) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	
	REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
	REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
	REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
	REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
	REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset;
	
	REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface

	 #ifdef  CONFIG_NFBI_SWITCH_P5_MII_PHY_MODE //8198 P5 MII
	 	dprintf("Set 8198 Switch Port5 as MII PHY mode OK\n");
		rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_PHY); //port 5 MII PHY mode
		REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1
		REG32(PCRP5) = 0 | (0x10<<ExtPHYID_OFFSET) |
				EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex |
				MIIcfg_RXER | AcptMaxLen_16K|EnablePHYIf | MacSwReset;
 	#endif

	 #ifdef  CONFIG_NFBI_SWITCH_P5_GMII_MAC_MODE //8198 P5 GMII
	 	dprintf("swcore.c:Set 8198 Switch Port5 as GMII MAC mode OK\n");
	 	rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_MAC); //port 5  GMII/MII MAC auto mode 

		REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1
		
	 		REG32(PCRP5) = 0 | (0x5<<ExtPHYID_OFFSET) |	 //JSW@20100309:For external 8211BN GMII ,PHYID must be 5		
				EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex |
				MIIcfg_RXER | AcptMaxLen_16K|EnablePHYIf | MacSwReset; 		

	 #endif
#else
        REG32(PITCR) = REG32(PITCR) | (1<<0); //configure port 0 to be a MII interface
        rtl865xC_setAsicEthernetRGMIITiming(0, (0<<4), RGMII_RCOMP_2DOT5NS);
        rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_PHY); //port0 MII PHY mode

        REG32(PCRP0)= 0 | (0x16<<ExtPHYID_OFFSET) |
    	    			EnForceMode| ForceLink|ForceSpeed10M |ForceDuplex |
    					MIIcfg_RXER | AcptMaxLen_16K|EnablePHYIf;
        rtl8651_setAsicEthernetPHYReg(3, 0, 0x0800); //Port 3, BMCR, power down
#endif
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
		//anson add
		REG32(0xb8000044)= 0;
		REG32(0xbb804300)= 0x00055500;

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

#if port5_rgmii_gmii
//	if(ExtP5GigaPhyMode)
	{
		REG32(PCRP5) &= (0x83FFFFFF-(0x00000000|MacSwReset));
		REG32(PCRP5) = REG32(PCRP5) | (GIGA_P5_PHYID << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset | 1<<20;
#if rgmii
		REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done | P5txdely;
#elif tmii
		REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done | 1<<23 | 1<<22 ;//| P5txdely; //bit24,23 is select RGMII, GMII/MII.
#else
		REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done | 1<<23 ;//| P5txdely; //bit24,23 is select RGMII, GMII/MII.
#endif
	}
#endif
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
#ifdef CONFIG_NFBI
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
        #ifdef CONFIG_NFBI
        //if ((port==0) || (port==3))
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
#ifndef CONFIG_NFBI
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
	

#ifndef CONFIG_NFBI
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


