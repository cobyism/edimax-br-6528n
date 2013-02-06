/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/swCore.c,v 1.95.2.13 2011/05/13 06:18:31 keith_huang Exp $
*
* Abstract: Switch core driver source code.
*
* $Author: keith_huang $
* ---------------------------------------------------------------
*/
#include <linux/autoconf.h>
#include <linux/delay.h>

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "phy.h"
#include "swTable.h"

#include "rtl865xC_tblAsicDrv.h"
#include "rtl8651_layer2.h"
#include "swNic_poll.h"
#ifdef CONFIG_RTL8196_RTL8366
#include "RTL8366RB_DRIVER/gpio.h"
#include "RTL8366RB_DRIVER/rtl8366rb_apiBasic.h"
#endif

#include "swCore.h"

#ifdef CONFIG_RTL865X_SUPPORT_IPV6_MLD
/*for support ipv6 multicast snooping*/
void setACL_trap_ipv6_mcast(int index);
void setACL_fwd_ipv6_mcast(int index);
#endif
#if defined(CONFIG_RTL865X_SUPPORT_IPV6_MLD) || defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
void EasyACLRule(int index, int action);
#endif

#define BICOLOR_LED 1

#define RTL8651_ETHER_AUTO_100FULL	0x00
#define RTL8651_ETHER_AUTO_100HALF	0x01
#define RTL8651_ETHER_AUTO_10FULL		0x02
#define RTL8651_ETHER_AUTO_10HALF	0x03
#define RTL8651_ETHER_AUTO_1000FULL	0x08
#define RTL8651_ETHER_AUTO_1000HALF	0x09

#define tick_Delay10ms(x) { int i=x; while(i--) __delay(5000); }
static int32 miiPhyAddress = -1;
extern unsigned int wan_port; 
extern const uint32 cPVCR[6][4];

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
extern uint32 port_link_sts2;
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
extern uint32 port_link_sts, port_linkpartner_eee;
#endif

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
#ifdef CONFIG_RTL8198
int eee_enabled = 0;
#else
int eee_enabled = 1;
#endif
static void qos_init(void);
void eee_phy_enable(void);
void eee_phy_disable(void);
void eee_phy_enable_by_port(int port);
#endif

#ifdef CONFIG_RTK_VOIP_865xC_QOS
#if !defined(RTL865XC_MNQUEUE_OUTPUTQUEUE) && !defined(RTL865XC_QOS_OUTPUTQUEUE)
typedef struct  {
 	uint32  ifg;					// default: Bandwidth Control Include/exclude Preamble & IFG 
	uint32	gap;					// default: Per Queue Physical Length Gap = 20 
	uint32	drop;					// default: Descriptor Run Out Threshold = 500 

	uint32	systemSBFCOFF;				//*System shared buffer flow control turn off threshold
	uint32	systemSBFCON;				//*System shared buffer flow control turn on threshold

	uint32	systemFCOFF;				// system flow control turn off threshold 
	uint32	systemFCON;				// system flow control turn on threshold 

	uint32	portFCOFF;				// port base flow control turn off threshold 
	uint32	portFCON;				// port base flow control turn on threshold 	

	uint32	queueDescFCOFF;				// Queue-Descriptor=Based Flow Control turn off Threshold  
	uint32	queueDescFCON;				// Queue-Descriptor=Based Flow Control turn on Threshold  

	uint32	queuePktFCOFF;				// Queue-Packet=Based Flow Control turn off Threshold  
	uint32	queuePktFCON;				// Queue-Packet=Based Flow Control turn on Threshold  
}	rtl865xC_outputQueuePara_t;
#endif
#endif

#define GIGA_P0_PHYID	0x16  //for P0 8211C
/*
	0: UTP [embedded PHY]
	1: others [external PHY]
 */
#ifdef CONFIG_RTL8197B_PANA
static int ExtP0GigaPhyMode=1;
#else
static int ExtP0GigaPhyMode=0;
#endif

/*
	0x00: RGMII mode,
	0x01: GMII/MII MAC auto mode
	0x02: GMII/MII PHY auto mode
	0x10: RGMII, force mode   //8196B Mode, because sw MDC/MDIO fail
	0x11: GMII/MII mac, force mode
	0x12: GMII/MII phy,  force mode
 */
static int SW_P0Mode=0x12;		//when ExtP0GigaPhyMode=1, this setting is valid

//------------------------------------------------------------------------
unsigned int read_gpio_hw_setting(void)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE) || defined(CONFIG_RTL865X_CMO)
	return 0;
#endif

	unsigned int tmp, b2;

	/* 
		read GPIO PORT F3 to determine the LED mode, 0 LED scan mode, 1 LED matrix mode
		PEFGHCNR_REG: Port EFGH control, b.31-b.24 port H, b.23-b.16 port G, b.15-b.8 port F, b.7-b.0 port E 
		GPIOF[7]= HCI_RDY[0]
		GPIOF[6]= HCI_CS[0]
		GPIOF[5]= HCI_RDY[2]
		GPIOF[4]= HCI_RDY[1]
		GPIOF[3]= HCI_INT#  <<=========== 0 LED scan mode, 1 LED matrix mode
		GPIOF[2]= HCI_CS[2]
		GPIOF[1]= reserved
		GPIOF[0]= reserved
	*/
	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG) & (~(0x8<<8) ); //set (GP2)=(F3)= gpio
	REG32(PEFGHPTYPE_REG) = REG32(PEFGHPTYPE_REG) & (~(0x8<<8) );  //change to GPIO mode
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) & (~(0x8<<8) );  //0 input, 1 output, set  inpur
	tmp=REG32(PEFGHDAT_REG);
	b2 = (tmp&(0x08<<8))>>11;
	//printk(">> ................GPIO val=%08x.\n",b2); 
	return b2;	
}

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) 
{
    struct { uint32 _content[8]; } entry;
    uint32 idx;
    
    bzero(&entry, sizeof(entry));
    for (idx=0; idx<count; idx++)// Write into hardware
        swTable_addEntry(type, idx, &entry);

}

int32 rtl8651_clearAsicAllTable(void) 
{

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

	#if !defined(CONFIG_RTL8196C_REVISION_B) && !defined(CONFIG_RTL8198)
	// SERVER_PORT_TABLE, ALG_TABLE and L4_ICMP_TABLE are removed
	_rtl8651_clearSpecifiedAsicTable(TYPE_SERVER_PORT_TABLE, RTL8651_SERVERPORTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ALG_TABLE, RTL8651_ALGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_ICMP_TABLE, RTL8651_ICMPTBL_SIZE);
	#endif

	return 0;
}
#endif

#define CPUSSR 		0xb8019004		/* bit 31~16: reserved */
#define _MAGIC_FORM_BOOT		0x00007DE8

void FullAndSemiReset( void )
{
#ifdef CONFIG_RTL865XC

	/* FIXME: Currently workable for FPGA, may need further modification for real chip */

	/* Perform full-reset for sw-core. */ 
	#ifdef CONFIG_RTL8196B_TLD
	if ((REG32(CPUSSR) & 0x0000ffff) != _MAGIC_FORM_BOOT) 
	#endif
	{
		REG32(SIRR) |= FULL_RST;
		tick_Delay10ms(50);
	}

	/* Enable TRXRDY */
	REG32(SIRR) |= TRXRDY;

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

#if defined(CONFIG_RTL8198)
	mdelay(10);   
#elif defined(CONFIG_RTL8196C_REVISION_B)
	if (REG32(REVR) == RTL8196C_REVISION_A)
		mdelay(10);	//wei add, for 8196C revision A. mdio data read will delay 1 mdc clock.	
#endif

	do { status = READ_MEM32( MDCIOSR ); } while ( ( status & STATUS ) != 0 );
#endif

	status &= 0xffff;
	*rData = status;

	return SUCCESS;
}

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

#ifdef CONFIG_RTK_VOIP_865xC_QOS

void rtl865xC_set_DSCP(int SIP, int RTP) 
{
	//printk("enter rtl865xC_set_DSCP \n");
	//clear dscp priority assignment, otherwise pkt with dscp value 0 will be assign priority 1		
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);	         
	rtl8651_setAsicDscpPriority(RTP, PRI7);
	rtl8651_setAsicDscpPriority(SIP, PRI7);
	//printk("left rtl865xC_set_DSCP \n"); 
} 

//0 disable, 1 enable
void Enable_VOIP_QoS(int enable){
	/*
	effect throughput
	#ifdef CONFIG_RTK_VOIP_DRIVERS_WAN_PORT_0	
		REG32(FCCR0) &=0xfffffffd;//disable Qeueu 1 flow control
		REG32(FCCR0) |=0x20;	  //enable Qeueu 5 flow control
	#endif
	
	#ifdef CONFIG_FIX_WAN_TO_4	
		REG32(FCCR1) &=0xfffffffd;
		REG32(FCCR1) |=0x20;
	#endif
	*/
	if(enable)
	{
		REG32(FCCR1) &=0xfffdffff;//disable CPU Qeueu 1 flow control	
		REG32(PSRP6_RW) &= 0xffffffbf;
		printk("Enable QoS\n");
	}
	else
	{	REG32(FCCR1) |=0x20000;
		REG32(PSRP6_RW) |=0x40;	
		printk("Disable QoS\n");
	}
}

int32 rtl865x_QoS_mechanism()
{
	rtl865xC_outputQueuePara_t	outputQueuePara = {
	1, 	// default: Bandwidth Control Include/exclude Preamble & IFG 
	20, 	// default: Per Queue Physical Length Gap = 20 
	500, 	// default: Descriptor Run Out Threshold = 500 
	
	0xd4,   //default: System shared buffer flow control turn off threshold = 0x78 
	0xf8,	//default: System shared buffer flow control turn on threshold = 0x88 

	0x14a, 	//default: system flow control turn off threshold = 0x14a
	0x158,	//default: system flow control turn on threshold = 0x158
	
	0xf8,   //default: port base flow control turn off threshold = 0xf8
	0x108,  //default: port base flow control turn on threshold = 0x108
	
	0x14, 	//Queue-Descriptor=Based Flow Control turn off Threshold =0x14 
	0x21, 	//Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 
	
	0x03, 	// Queue-Packet=Based Flow Control turn off Threshold = 0x03 
	0x05	// Queue-Packet=Based Flow Control turn on Threshold =0x05 
	};
  	
    	REG32(QNUMCR) = P0QNum_6 | P1QNum_6 | P2QNum_6 | P3QNum_6 | P4QNum_6| P6QNum_6;

	printk("Qos threshold Start \n");
	/* Set the threshold value for qos sytem */
	int32 retval;
	int32	i,j;
	retval = rtl8651_setAsicSystemBasedFlowControlRegister(outputQueuePara.systemSBFCON, outputQueuePara.systemSBFCOFF, outputQueuePara.systemFCON, outputQueuePara.systemFCOFF, outputQueuePara.drop);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set System Base Flow Control Para Error.\n");
		return retval;
	}
	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		retval = rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(0, i, outputQueuePara.queueDescFCON, outputQueuePara.queueDescFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Descriptor Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(PHY0+j, i, outputQueuePara.queueDescFCON, outputQueuePara.queueDescFCOFF);


		retval = rtl8651_setAsicQueuePacketBasedFlowControlRegister(0, i, outputQueuePara.queuePktFCON, outputQueuePara.queuePktFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Packet Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueuePacketBasedFlowControlRegister(PHY0+j, i, outputQueuePara.queuePktFCON, outputQueuePara.queuePktFCOFF);

	}
// add for CPU port
	REG32(0xbb804570)= 0x140021;
	REG32(0xbb804574)= 0x140021;
	REG32(0xbb804578)= 0x140021;
	
	REG32(0xbb8045c4) = 0x038005;
	REG32(0xbb8045c8) = 0x038005;
	REG32(0xbb8045cc) = 0x038005;
		
	retval = rtl8651_setAsicPortBasedFlowControlRegister(0, outputQueuePara.portFCON, outputQueuePara.portFCOFF);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Port Base Flow Control Para Error.\n");
		return retval;
	}
	for(j=1;j<=CPU;j++)
		rtl8651_setAsicPortBasedFlowControlRegister(PHY0+j, outputQueuePara.portFCON, outputQueuePara.portFCOFF);
// add for CPU port	
	REG32(0xbb804524)= 0xf80108;
	retval = rtl8651_setAsicPerQueuePhysicalLengthGapRegister(outputQueuePara.gap);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Queue Physical Lenght Gap Reg Error.\n");
		return retval;
	}

	//clear dscp priority assignment, otherwise pkt with dscp value 0 will be assign priority 1		
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);			

	WRITE_MEM32(QRR,QRST);	

	//for 802.1Q mapping => Priority 1Q == absolutely priority 
	WRITE_MEM32(LPTM8021Q,0xFAC688);
	
	//For CPU port: Priority to Qid	
	WRITE_MEM32(CPUQIDMCR0,0x55432001);
	
	// open all port Queue 5 flow control
	WRITE_MEM32(FCCR0, 0x3f3f3f3f);
	WRITE_MEM32(FCCR1, 0x3f3f3f3f);
	
	printk("Qos threshold End \n");

	
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
extern int L2_table_disabled;
#endif

#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL8198)
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

		if(mask != 0)
		{
			rtl8651_getAsicEthernetPHYReg( wphyid, reg, &data);
			data = data&mask;
		}
		rtl8651_setAsicEthernetPHYReg( wphyid, reg, data|val  );


		//switch to page 0
		rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
	}
}
#endif

#ifdef CONFIG_RTL8196C_REVISION_B

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

void Setting_RTL8196C_PHY(void)
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

	/*
	  #=========patch for eee============================
	  #1. page4¡Breg24¡Glpi_rx_ti_timer_cnt change to f3     
	  phywb all 4 24 7-0 0xf3

	  #2. page4¡Breg16¡Grg_txqt_ps_sel change to 1    
	  phywb all 4 16 3 1  
	*/
	Set_GPHYWB(999, 4, 24, 0xff00, 0xf3);
	Set_GPHYWB(999, 4, 16, 0xffff-(1<<3), 1<<3);
	/*	
	  #=========patch for IOL Tx amp.=====================      
	  #<a>modify 100M DAC current default value:
	  #Port#0~#4(per port control)
	  #Page1,Reg19,bit[13:11]: 
	  #original value 200uA(3b'100),--> change to 205u(3b'000)   => change to 205u(3b010)

	  phywb all 1 19 13-11 0x2

	  #<b>modify bandgap voltage default value:  
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg23,bit[8:6],                            

	  #original value 1.312V(3b'110),-->change to 1.212V(3b'100).       

	  phywb all 1 23 8-6 0x4

	  #<c>modify TX CS cap default value:                                   
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg18,bit[5:3],                                               

	  #original value Reserved bits(3b'000),-->change to 600fF(3b'011). =>change to 750fF(3b'110) 
	  phywb all 1 18 5-3 0x6
	*/

	Set_GPHYWB(999, 1, 19, 0xffff-(7<<11), 0x2<<11);
	Set_GPHYWB(999, 1, 23, 0xffff-(7<<6)  , 0x4<<6);
	Set_GPHYWB(999, 1, 18, 0xffff-(7<<3), 0x6<<3);
	

	/* 20100223 from Maxod: 100M half duplex enhancement */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_RXIPG_MASK) | 0x05;

	/* fix the link down / link up issue with SmartBit 3101B when DUT(8196c) set to Auto-negotiation 
	    and SmartBit force to 100M Full-duplex */
 	REG32(MACCR)= (REG32(MACCR) & ~SELIPG_MASK) | SELIPG_11;

	/*20100222 from Anson:Switch Corner test pass setting*/
	 /*
		REG21 default=0x2c5
		After snr_ub(page0 reg21.7-4) = 3 and snr_lb(page0 reg21.3-0)=2 ,REG21=0x232
		REG22 default=0x5b85
		After adtune_lb(page0 reg22.6-4)=4 (10uA) ,REG21=0x5b45

		REG0 default=0x1100
		restart AN
		page0 reg0.9 =1 , ,REG0=0x1300
	*/
	//rtl8651_setAsicEthernetPHYReg( i, 0x15, 0x232 );
	//Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0x32<<0);
	// test 96C to 96C restart AN 100 times, result is pass ==> page0 reg21.14(disable the equlizar)=1      
#ifdef CONFIG_RTL8196C_ETH_IOT
	// enable "equalizer reset", i.e. page 0 , reg21, bit14= 0
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x0032);
#else
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x4032);
#endif

	//rtl8651_setAsicEthernetPHYReg( i, 0x16, 0x5b45 );
	//Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x4<<4);
	Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x5<<4);
	//rtl8651_setAsicEthernetPHYReg( i, 0x0, 0x1300 );						
	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 0x1<<9);
	
	/*20100225 from Anson:Switch Force cailibration
	#change calibration update method for patch first pkt no update impedance
	phywb all 1 29 1 0
	#--------------Patch for impedance update fail cause rx crc error with long calbe-------- 
	#Froce cailibration
	phywb all 1 29 2 1
	#Force impedance value = 0x8888
	phywb all 1 28 15-0 0x8888			 	
	#-----------------------------------------------------------------------------------------
	#Select clock (ckt125[4]) edge trigger mlt3[1:0] = negative for patch four corner fail issue(only tx timing)
	phywb all 1 17 2-1 0x3
	*/
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<1), 0x0<<1);
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<2), 0x1<<2);
	//Set_GPHYWB(999, 1, 28, 0xffff-(0xffff), 0x8888);
	Set_GPHYWB(999, 1, 17, 0xffff-(3<<1), 0x3<<1);
		 		 
	/*20100222 from Yozen:AOI TEST pass setting*/
	Set_GPHYWB(999, 1, 18, 0xffff-(0xffff), 0x9004);				
		
	// for "DSP recovery fail when link partner = force 100F"
	Set_GPHYWB(999, 4, 26, 0xffff-(0xfff<<4), 0xff8<<4);
		
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
	
#ifdef CONFIG_RTL8196C_ETH_IOT
	for(i=0; i<5; i++) {
		set_gray_code_by_port(i);
	}
#endif
	printk("  Set 8196C PHY Patch OK\n");

}
#endif

#ifdef CONFIG_RTL8198
void Setting_RTL8198_GPHY(void)
{
	int i=0;
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
	  #Access command format: phywb {all: phyID=0,1,2,3,4} {page} {RegAddr} {Bit location} {Bit value}
	  # when writing page 72: must do "mdcmdio_cmd write $phyID 31 0x7" first, then write "mdcmdio_cmd write $phyID 30 $PageNum"
	  phywb all 72 21 15-0 0x7092
	  phywb all 72 22 15-0 0x7092
	  phywb all 72 23 15-0 0x7092
	  phywb all 72 24 15-0 0x7092
	  phywb all 72 25 15-0 0x7092
	  phywb all 72 26 15-0 0x7092
	*/

	Set_GPHYWB(999, 72, 21, 0, 0x7092);
	Set_GPHYWB(999, 72, 22, 0, 0x7092);
	Set_GPHYWB(999, 72, 23, 0, 0x7092);
	Set_GPHYWB(999, 72, 24, 0, 0x7092);
	Set_GPHYWB(999, 72, 25, 0, 0x7092);	
	Set_GPHYWB(999, 72, 26, 0, 0x7092);	

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
	printk("Set GPHY Parameter OK\n");
}

#ifdef PORT5_RGMII_GMII
unsigned int ExtP5GigaPhyMode=0;
void ProbeP5GigaPHYChip(void)	
{
	unsigned int uid,tmp;
	unsigned int i;

	//printk("In Setting port5 \r\n");

	REG32(0xB8000010)=0x01FFFCB9;

	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);


	/* Read */
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID,0x10,0x01FE);
	rtl8651_getAsicEthernetPHYReg(GIGA_P5_PHYID, 0, &tmp );

	//printk("Read port5 phyReg0= 0x%x \r\n",tmp);

	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 2, &tmp );
	//printk("Read port5 UPChipID= 0x%x \r\n",tmp);
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 3, &tmp );
	//printk("Read port5 downChipID= 0x%x \r\n",tmp);
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{	
		//printk("Find Port5   have 8211 PHY Chip! \r\n");
		ExtP5GigaPhyMode=1;
		//return 1;
	}	
	else
	{	
		//printk("NO Find Port5 8211 PHY Chip! \r\n");
		//ExtP5GigaPhyMode=0;
		//return 1;
	}	
	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

}
#endif
#endif

int32 swCore_init(int VlanCount)
{
#ifdef CONFIG_RTL865XC
	uint32 rev, chip_id;
	int port;

	if (VlanCount == -2) {
		rtl8651_clearAsicAllTable();
		return 0;
	}
	
	if (VlanCount == -1) {
		_rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
		return 0;			
	}

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
	port_link_sts2 = 0;
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
	port_link_sts = 0;
	port_linkpartner_eee = 0;
#endif

#ifdef CONFIG_RTL865X_HW_TABLES
    
	/*
	 * 8211 Giga port compatibility issue:
	 * after test Chariot (3D+3U) for one to two minutes, then issue reboot in console,
	 * sometimes the 8211 Giga port will not work (LAN PC shows 100Mbps, 
	 *      Target's LED is incorrect, ping 192.168.1.254 is failed)
	 * this issue is easy to happen when the LAN PC's Gigabit Ethernet is Marvell's.
	 * 
	 * do the external PHY reset and restart auto-negotiation to fix it.
	 */
#ifndef CONFIG_RTL8196B
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID, 0, (PHY_RESET | ENABLE_AUTONEGO));		
	rtl8651_restartAsicEthernetPHYNway(5, GIGA_P5_PHYID);		
#endif

	/*
	 * after switch operation mode from Gateway mode to Bridge/WISP mode,
	 * ping br0(192.168.1.254) with 32 bytes is okay, 
	 * but ping br0(192.168.1.254) with 65500 bytes is failed.
	 * the fragmented packets sent from 192.168.1.254 have no Ethernet header and IP header,
	 * they are started with ping payload, like:
	 *      0000   75 76 77 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d  uvwabcdefghijklm
	 *      0010   6e 6f 70 71 72 73 74 75 76 77 61 62 63 64 65 66  nopqrstuvwabcdef
	 *      0020   67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76  ghijklmnopqrstuv
	 *      0030   77 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  wabcdefghijklmno
	 *      0040   70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68  pqrstuvwabcdefgh
	 *      0050   69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 61  ijklmnopqrstuvwa
	 *      0060   62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71  bcdefghijklmnopq
	 *
	 * after do the switch semi-reset, the issue is fixed
	 */
	/* Perform the semi-reset */
#ifndef CONFIG_RTL8196B
	REG32(SIRR) |= SEMI_RST;
	tick_Delay10ms(10);        

#endif
#else
	/* Full reset and semreset */
	FullAndSemiReset();

#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
		Setting_RTL8196C_PHY();
	
#elif defined(CONFIG_RTL8198)
#if 0//def PORT5_RGMII_GMII
	ProbeP5GigaPHYChip();
#endif
	Setting_RTL8198_GPHY();
	
#endif

	rtl8651_clearAsicAllTable();

#endif /* CONFIG_RTL865X_HW_TABLES */
    

	chip_id = (READ_MEM32(CVIDR)) >> 16;

	rev = ((READ_MEM32(CRMR)) >> 12) & 0x0f ;/*Runtime determined patch for A cut revison. RLRevID_OFFSET = 12, RLRevID_MASK = 0x0f */
	if ((chip_id != 0x8196) && (rev < 0x01)) {
		rev = READ_MEM32((SYSTEM_BASE+0x3400+0x08));
		if(rev == 0x00)/*A Cut patch RTL865X_CHIP_REV_A = 0x00*/{
			REG32(PCRP6) = (6 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		}
	}   

#ifdef CONFIG_RTL865X_PANAHOST
        rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_MII_MAC); //Port 5 MII MAC mode
#elif !defined(CONFIG_RTL8196B)
	rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_RGMII);
#endif

	/*
		# According to Hardware SD: David & Maxod,			
		Set Port5_GMII Configuration Register.
		- RGMII Output Timing compensation control : 0 ns
		- RGMII Input Timing compensation control : 0 ns
	*/
#ifdef CONFIG_RTL865X_PANAHOST
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, (1<<4), RGMII_RCOMP_0NS);
		rtl8651_setAsicEthernetMII(GIGA_P5_PHYID, LINK_MII_MAC, TRUE);
#elif !defined(CONFIG_RTL8196B)
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);
		rtl8651_setAsicEthernetMII(GIGA_P5_PHYID, P5_LINK_RGMII, TRUE);
#endif

	/*
	 * do not modify PVCR0 ~ PVCR3 (vid) and MSCR (EN_L2/EN_L3/EN_L4) when CONFIG_RTL865X_HW_TABLES
	 * is defined.
	 * they will be set by rtl865x_lightRomeConfig().
	 */
#ifndef CONFIG_RTL865X_HW_TABLES
#ifdef CONFIG_RTL8196_RTL8366
	{
	extern int savOP_MODE_value;
	int32 retval = 0;
	int iport;
	rtl8366rb_vlanConfig_t vlancfg_8366;
		if(savOP_MODE_value==2)
		{
			/* for lan */
			memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
			vlancfg_8366.fid = 0;
			vlancfg_8366.mbrmsk = (RTL8366RB_LANPORT&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
			vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
			vlancfg_8366.vid = RTL_LANVLANID;
			retval = rtl8366rb_setVlan(&vlancfg_8366);
			/*need set pvid??*/
			for(iport=0;iport<8;iport++)
				if  ((1<<iport)&vlancfg_8366.mbrmsk)
				{
					retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
				}
		
			/* for wan */
			iport=0;
			memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
			retval = 0;

			vlancfg_8366.fid = 0;
			vlancfg_8366.mbrmsk = (RTL8366RB_WANPORT&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
			vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
			vlancfg_8366.vid = RTL_WANVLANID;
			retval = rtl8366rb_setVlan(&vlancfg_8366);
			/*need set pvid??*/
			for(iport=0;iport<8;iport++)
				if  ((1<<iport)&vlancfg_8366.mbrmsk)
				{
					retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
				}
		}
		else
		{
			{
                        memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
                        vlancfg_8366.fid = 0;
                        vlancfg_8366.mbrmsk = ((RTL8366RB_WANPORT|RTL8366RB_LANPORT)&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
                        vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
                        vlancfg_8366.vid = RTL_LANVLANID;
                        retval = rtl8366rb_setVlan(&vlancfg_8366);
                        /*need set pvid??*/
                        for(iport=0;iport<8;iport++)
                                if  ((1<<iport)&vlancfg_8366.mbrmsk)
                                {
                                        retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
                                }
                	}
	                {
	                        /* for wan clear it */
	                        int iport;
	                        rtl8366rb_vlanConfig_t vlancfg_8366;

	                        vlancfg_8366.fid = 0;
	                        vlancfg_8366.mbrmsk = 0;
	                        vlancfg_8366.untagmsk = 0;
	                        vlancfg_8366.vid = RTL_WANVLANID;
	                        retval = rtl8366rb_setVlan(&vlancfg_8366);
	                }
			
		}
	}

	REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;

#elif defined(CONFIG_RTL8196B_GW_MP) || defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_ECAP) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
	REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
	REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;

#else
	if(VlanCount==1)    
	{
		//wei add, original all lan use VID=9
		/* Set PVID of all ports to 9 */
		REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
	}
	else if(VlanCount==2)
	{
		REG32(PVCR0) = cPVCR[wan_port][0];
		REG32(PVCR1) = cPVCR[wan_port][1];
		REG32(PVCR2) = cPVCR[wan_port][2];
		REG32(PVCR3) = cPVCR[wan_port][3];
	}

#if defined(CONFIG_RTK_GUEST_ZONE) 
	else
	{
#if defined(CONFIG_RTL8196C_KLD)		
		REG32(PVCR0) = (LAN3_VID << 16) | LAN4_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN2_VID;
		REG32(PVCR2) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
#else
		REG32(PVCR0) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR1) = (LAN3_VID << 16) | LAN2_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN4_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
#endif		
	}
#endif

/*
	8196B VLAN enabled:
	Physical port 	4		3		2		1		0		[Power]
				Vlan1	Vlan2	Vlan3	Vlan4	WAN
	8196C VLAN enabled:
	Physical port 	0		1		2		3		4		[Power]
				Vlan1	Vlan2	Vlan3	Vlan4	WAN
	===================================================
	8196B-KLD Guest Zone enabled:
	Physical port 	0		1		2		3		4		[Power]
				LAN1	LAN2	LAN3	LAN4	WAN
	8196C-KLD Guest Zone enabled:
	Physical port 	0		1		2		3		4		[Power]
				LAN1	LAN2	LAN3	LAN4	WAN
 */
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	else
	{
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
		REG32(PVCR0) = (LAN2_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN4_VID << 16) | LAN3_VID;
		REG32(PVCR2) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;		
#else
		REG32(PVCR0) = (LAN4_VID << 16) | WAN_VID;
		REG32(PVCR1) = (LAN2_VID << 16) | LAN3_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;		
#endif		
	}
#endif	
#endif
#endif

#ifdef CONFIG_RTL8197B_PANA
		REG32(PCRP1) = EnForceMode & (~ForceLink) & (~EnablePHYIf);
		REG32(PCRP2) = EnForceMode & (~ForceLink) & (~EnablePHYIf);
		REG32(PCRP3) = (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
		REG32(PCRP4) = EnForceMode & (~ForceLink) & (~EnablePHYIf);

#elif defined(CONFIG_RTL8198)
		REG32(0xb8000044)= 0;

		REG32(PCRP1) = REG32(PCRP1) & (0xFFFFFFFF-(MacSwReset));
		REG32(PCRP2) = REG32(PCRP2) & (0xFFFFFFFF-(MacSwReset));
		REG32(PCRP3) = REG32(PCRP3) & (0xFFFFFFFF-(MacSwReset));
		REG32(PCRP4) = REG32(PCRP4) & (0xFFFFFFFF-(MacSwReset));

		REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf |MacSwReset;
		REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf|MacSwReset;
		REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf|MacSwReset;
		REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf|MacSwReset;
#ifdef PORT5_RGMII_GMII
		if(ExtP5GigaPhyMode)
		{
			REG32(PCRP5) &= (0x83FFFFFF-(0x00000000|MacSwReset));
			REG32(PCRP5) = REG32(PCRP5) | (GIGA_P5_PHYID << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf | MacSwReset | 1<<20;
			REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done ;//| P5txdely;
		}
#endif

#elif defined(CONFIG_RTL8196C)

#ifndef CONFIG_POCKET_ROUTER_SUPPORT
		REG32(PCRP1) = REG32(PCRP1) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
		REG32(PCRP2) = REG32(PCRP2) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
		REG32(PCRP3) = REG32(PCRP3) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
		REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf |MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
		REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf|MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
		REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf|MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
#endif
		REG32(PCRP4) = REG32(PCRP4) & (0xFFFFFFFF-(0x00400000|MacSwReset));
		TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);
		REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf|MacSwReset;
		TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);

#else
		//REG32(PCRP0) = (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP1) = (1 << ExtPHYID_OFFSET) | AcptMaxLen_16K | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP2) = (2 << ExtPHYID_OFFSET) | AcptMaxLen_16K | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP3) = (3 << ExtPHYID_OFFSET) | AcptMaxLen_16K | IPMSTP_PortST_MASK | EnablePHYIf;
		REG32(PCRP4) = (4 << ExtPHYID_OFFSET) | AcptMaxLen_16K | IPMSTP_PortST_MASK | EnablePHYIf;
#endif

		if(ExtP0GigaPhyMode==0)
		{
#if defined(CONFIG_RTL8198)
			REG32(PCRP0) = REG32(PCRP0) & (0xFFFFFFFF-(MacSwReset));
			REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf |MacSwReset;
#elif defined(CONFIG_RTL8196C)
#ifndef CONFIG_POCKET_ROUTER_SUPPORT
			REG32(PCRP0) = REG32(PCRP0) & (0xFFFFFFFF-(0x00400000|MacSwReset));
			TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
			REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf |MacSwReset;
			TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
#endif
#else		
			//REG32(PITCR) = REG32(PITCR) | (0<<0);  //default, embedded PHY
			REG32(PCRP0) = (0 << ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
#endif
		}
		else if(ExtP0GigaPhyMode==1)
		{
			//unsigned int temp;

			REG32(PITCR) = REG32(PITCR) | (1<<0);
			rtl865xC_setAsicEthernetRGMIITiming(0, (0<<4), RGMII_RCOMP_2DOT5NS); //wei add, for 8652 demo board	
			//rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_7NS, RGMII_RCOMP_0NS); //wei add, for 8652 demo board	
		
			//8211C auto learning
			//rtl8651_getAsicEthernetPHYReg(GIGA_P0_PHYID, 3, &temp);
			
			if( (SW_P0Mode&0x0f)==0)  // RGMII mode
			{
				rtl865xC_setAsicEthernetMIIMode(0, LINK_RGMII); //wei add for 8652 demo board
			}
			else if( (SW_P0Mode&0x0f)==1)  //MII MAC  mode
			{
				rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_MAC); //wei add for 8652 demo board
			}
			else if( (SW_P0Mode&0x0f)==2)  //MII PHY mode
			{
				rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_PHY); //wei add for 8652 demo board
			}		

			if( (SW_P0Mode&0xf0)==0)  
			{
				REG32(PCRP0) = (GIGA_P0_PHYID<< ExtPHYID_OFFSET) | AcptMaxLen_16K | EnablePHYIf;
			}
			else if( (SW_P0Mode&0xf0)==0x10)  //GMII / MII mode
			{			
				REG32(PCRP0) = 0 | (GIGA_P0_PHYID<<ExtPHYID_OFFSET) |
							EnForceMode| ForceLink|ForceSpeed10M |ForceDuplex |
							MIIcfg_RXER | AcptMaxLen_16K|EnablePHYIf   ;			
			}						
		}	

#ifdef CONFIG_RTL865X_PANAHOST
		REG32(PCRP5) = 0 | (GIGA_P5_PHYID<<ExtPHYID_OFFSET) |
							EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex |
							MIIcfg_RXER | 				
							AcptMaxLen_16K|EnablePHYIf;
#elif !defined(CONFIG_RTL8196B)
		REG32(PCRP5) = (GIGA_P5_PHYID<<ExtPHYID_OFFSET)|AcptMaxLen_16K|IPMSTP_PortST_MASK |EnablePHYIf;
#endif

	/* Set forwarding status */
#ifdef CONFIG_RTL8197B_PANA
	REG32(PCRP0) = (REG32(PCRP0) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP1) = (REG32(PCRP1) & ~STP_PortST_MASK) | STP_PortST_DISABLE;
	REG32(PCRP2) = (REG32(PCRP2) & ~STP_PortST_MASK) | STP_PortST_DISABLE;
	REG32(PCRP3) = (REG32(PCRP3) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	REG32(PCRP4) = (REG32(PCRP4) & ~STP_PortST_MASK) | STP_PortST_DISABLE;
#else
	REG32(PCRP0) = (REG32(PCRP0) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
	REG32(PCRP1) = (REG32(PCRP1) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
	REG32(PCRP2) = (REG32(PCRP2) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
	REG32(PCRP3) = (REG32(PCRP3) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
	REG32(PCRP4) = (REG32(PCRP4) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);
#ifndef CONFIG_RTL8196B
	REG32(PCRP5) = (REG32(PCRP5) & ~STP_PortST_MASK) | STP_PortST_FORWARDING;
#endif		
#endif
	rev = ((READ_MEM32(CRMR)) >> 12) & 0x0f ;/*Runtime determined patch for A cut revison. RLRevID_OFFSET = 12, RLRevID_MASK = 0x0f */
	if ((chip_id != 0x8196) && (rev < 0x01))
	{
		rev = READ_MEM32((SYSTEM_BASE+0x3400+0x08));
		if(rev == 0x00)/*A Cut patch RTL865X_CHIP_REV_A = 0x00*/
		{
			REG32(PCRP6) = (6 << ExtPHYID_OFFSET) | AcptMaxLen_16K | IPMSTP_PortST_MASK |EnablePHYIf;
		}
	}

#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)
	// Add for ip-multicast forward
	REG32(PCRP0) = (REG32(PCRP0) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
	REG32(PCRP1) = (REG32(PCRP1) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;	
	TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
	REG32(PCRP2) = (REG32(PCRP2) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
	REG32(PCRP3) = (REG32(PCRP3) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
	REG32(PCRP4) = (REG32(PCRP4) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);
	#ifndef CONFIG_RTL8196B
	REG32(PCRP5) = (REG32(PCRP5) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
	#endif
	REG32(PCRP6) = (REG32(PCRP6) & ~IPMSTP_PortST_MASK) | IPMSTP_PortST_FORWARDING;
#endif

#ifdef CONFIG_HW_MULTICAST_TBL
//	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_MCAST | EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU);
	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_MCAST | EN_UNMCAST_TOCPU);
	WRITE_MEM32(CSCR, (READ_MEM32(CSCR) & ~(ALLOW_L2_CHKSUM_ERR|ALLOW_L3_CHKSUM_ERR|ALLOW_L4_CHKSUM_ERR)) | (EN_ETHER_L3_CHKSUM_REC | EN_ETHER_L4_CHKSUM_REC));
	UPDATE_MEM32(ALECR, 1522, MultiCastMTU_MASK, MultiCastMTU_OFFSET);
#endif

	#ifndef CONFIG_RTL865X_HW_TABLES
	/* Enable L2 lookup engine and spanning tree functionality */
	REG32(MSCR) = EN_L2;
	#endif
    
	REG32(QNUMCR) = P0QNum_1 | P1QNum_1 | P2QNum_1 | P3QNum_1 | P4QNum_1;

	#ifdef CONFIG_RTK_VOIP_865xC_QOS
	rev = rtl865x_QoS_mechanism();
	if(rev!= SUCCESS)
	{
		rtlglue_printf("rtl865x_QoS_mechanism Error.\n");
		return rev;
	}
	#endif

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	qos_init();
#endif


	/* Start normal TX and RX */
	REG32(SIRR) |= TRXRDY;

	/* Init PHY LED style */
#ifdef BICOLOR_LED
#ifdef CONFIG_RTL8196B
#ifdef CONFIG_RTL8197B_PANA
    REG32(0xb8000030) = REG32(0xb8000030) & (~0x00020000); //bit17, port 3 led
#else
	/*
	 * 0xb8000030: System pin mux selection
	 * 	change to LED-SW mode from DBG mode (chip initial value is DBG mode)
	 */
	//REG32(0xb8000030) = 0x0;
      #if defined(CONFIG_RTL8198)    
	REG32(LEDCR)  = 0x55500;
      #endif
#endif
#else
	if(read_gpio_hw_setting())	// 1 means using matrix mode
	{
	 	REG32(LEDCR)  = 0x155500;
	}

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

	#ifdef CONFIG_RTL8196B_TLD
	if ((REG32(CPUSSR) & 0x0000ffff) != _MAGIC_FORM_BOOT) 
	#endif
	{

	/*PHY FlowControl. Default enable*/
	for(port=0;port<MAX_PORT_NUMBER;port++)
	{
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
		if (eee_enabled) {
			uint32 reg;			
			eee_phy_enable_by_port(port);

			if (REG32(REVR) == RTL8196C_REVISION_B) {
				// enable EEE MAC
				reg = REG32(EEECR);
				REG32(EEECR) = (reg & ~(0x1f << (port * 5))) | 
					((FRC_P0_EEE_100|EN_P0_TX_EEE|EN_P0_RX_EEE) << (port * 5));
			}
		}
#endif

		/* Set Flow Control capability. */
	#ifndef CONFIG_RTL8196B
		if (port == MAX_PORT_NUMBER-1)
			rtl8651_setAsicFlowControlRegister(RTL8651_MII_PORTNUMBER, TRUE, GIGA_P5_PHYID);
		else				
	#endif
			rtl8651_restartAsicEthernetPHYNway(port+1, port);				
	}
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	if (eee_enabled == 0) {
		REG32(EEECR) = 0;
	}
#endif	
	}



	#if defined(CONFIG_RTL865X_DIAG_LED)
	/* diagnosis led (gpio-porta-6) on */
	REG32(PABDAT) |=  0x40000000; /* pull high by set portA-0(bit 30) as gpio-output-1, meaning: diag led OFF */
	#endif /* CONFIG_RTL865X_DIAG_LED */

#if !defined(CONFIG_RTL8196B)
	REG32(MDCIOCR) = 0x96181441;	// enable Giga port 8211B LED
#endif

//	REG32(FFCR) = EN_UNUNICAST_TOCPU | EN_UNMCAST_TOCPU; // rx broadcast and unicast packet
	REG32(FFCR) = EN_UNMCAST_TOCPU; // rx multicast packet

#ifdef CONFIG_RTL8196_RTL8366
	{
		rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_4NS, RGMII_RCOMP_2NS);
		WRITE_MEM32(PITCR, Port0_TypeCfg_GMII_MII_RGMII);
		WRITE_MEM32(PCRP0, 0xe80367);		/* force mode 1G */
	}
#endif


	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
/*
	Explanation for ACL setting:
			ACL range
	WAN: 	0~0
	LAN:		10~12

	L2_table_disabled=0, EN_IN_ACL = 0, all ACL rules are no used
	L2_table_disabled=1, EN_IN_ACL = 1, 
		ACL #0: RTL8651_ACL_CPU
		ACL #10: RTL8651_ACL_CPU
		ACL #11 ~ 12: don't care because rule #10 will trap all packets to CPU
 */
	EasyACLRule(0, RTL8651_ACL_CPU);	// WAN
	EasyACLRule(10, RTL8651_ACL_CPU); // LAN

	if (L2_table_disabled) {
	  	REG32(MSCR) |= EN_IN_ACL;
	}
	else 
	  	REG32(MSCR) &= (~EN_IN_ACL);
	#endif

	#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
	rtl8651_defineProtocolBasedVLAN( IP6_PASSTHRU_RULEID, 0x0, 0x86DD );
	updateProtocolBasedVLAN();
	#endif
	
#endif /* CONFIG_RTL865XC */

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	//enable RTL8196C 10M power saving mode
	{
		int tmp,idx;

		for(idx=0;idx<MAX_PORT_NUMBER;idx++) {
			rtl8651_getAsicEthernetPHYReg( idx, 0x18, &tmp );
#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
			tmp |= BIT(15); //enable power saving mode
#else
			tmp &= ~BIT(15); //disable power saving mode
#endif
			rtl8651_setAsicEthernetPHYReg( idx, 0x18, tmp );
		}
	}
	REG32(MPMR) |= PM_MODE_ENABLE_AUTOMATIC_POWER_DOWN;
#ifdef CONFIG_RTL8196C
	#define PIN_MUX_SEL 0xb8000040
	// Configure LED-SIG0/LED-SIG1/LED-SIG2/LED-SIG3/LED-PHASE0/LED-PHASE1/LED-PHASE2/LED-PHASE3 PAD as LED-SW

#ifndef CONFIG_POCKET_ROUTER_SUPPORT
//	REG32(PIN_MUX_SEL) &= ~(0xFFFF);		
#endif

#ifdef CONFIG_RTL8196C_ETH_LED_BY_GPIO
	// Configure LED-SIG0/LED-SIG1/LED-SIG2/LED-SIG3/LED-PHASE0 PAD as GPIO
	REG32(PIN_MUX_SEL) |= (0x3FF);
#endif

#endif
#endif

#ifdef CONFIG_RTL865X_SUPPORT_IPV6_MLD
/*
	Explanation for ACL setting:
			ACL range
	WAN: 	0~1
	LAN:		10~12

	EN_IN_ACL always be 1,
	L2_table_disabled=0, 
		ACL #0: ACL_IPV6_TO_CPU
		ACL #1: RTL8651_ACL_PERMIT
		ACL #10: RTL8651_ACL_PERMIT for DA = 33 33 ff xx xx xx
		ACL #11: RTL8651_ACL_CPU for DA = 33 33 xx xx xx xx
		ACL #12: RTL8651_ACL_PERMIT
	L2_table_disabled=1, 
		ACL #0: RTL8651_ACL_CPU
		ACL #10: RTL8651_ACL_CPU
		ACL #11 ~ 12: don't care because rule #10 will trap all packets to CPU
 */
	/* add for support ipv6 multicast snooping */
	{

	extern int savOP_MODE_value;

	EasyACLRule(0, ACL_IPV6_TO_CPU);	// WAN
	EasyACLRule(1, RTL8651_ACL_PERMIT);	// WAN

	setACL_fwd_ipv6_mcast(10);			// LAN for hw forward ipv6 multicast (DA=33-33-ff-xx-xx-xx)
	setACL_trap_ipv6_mcast(11);			// LAN for tarp ipv6 multicast (DA=33-33-xx-xx-xx-xx)

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	if (L2_table_disabled) {
		EasyACLRule(10, RTL8651_ACL_CPU); // LAN
		EasyACLRule(0, RTL8651_ACL_CPU);
	}		
	else 
#endif	
		EasyACLRule(12, RTL8651_ACL_PERMIT); // LAN
	
	if(savOP_MODE_value==2) {
		REG32(MSCR) |= EN_IN_ACL;
		REG32(SWTCR1) |= EN_FRAG_TO_ACLPT;
	}
	else
	{
		REG32(MSCR) &= ~EN_IN_ACL;
		REG32(FFCR) &= ~EN_MCAST;
	}
}

#endif	

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	REG32(VCR0) &= ~EnVlanInF_MASK;
#endif	

	return 0;
}

void swCore_start(void)
{
	int i;
	uint32 statCtrlReg;

	REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES;
	REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | LINK_CHANGE_IE;	

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	REG32(SBFCR2) = (REG32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (0x136 << S_Max_SBuf_FCON_OFFSET) | (0xfc << S_Max_SBuf_FCOFF_OFFSET);
#else
	REG32(SBFCR2) = 0xd400f8; // adjust internal buffer threshold
#endif

	REG32(ELBTTCR) = 0x00000400;/*mark_patch for correcting the Leaky bucket value*/	

	for (i=0; i<5; i++) {		
		rtl8651_getAsicEthernetPHYReg(i, 0, &statCtrlReg);
		statCtrlReg &= ~POWER_DOWN;
		rtl8651_setAsicEthernetPHYReg(i, 0, statCtrlReg);				
		REG32(PCRP0+i*4) &= ~(EnForceMode);				
	}	
}

void swCore_down(void)
{
	int i;
	uint32 statCtrlReg;
	
	REG32(CPUICR) = 0; 
	REG32(CPUIIMR) = 0;        	
	REG32(SIRR) = 0;

	for (i=0; i<5; i++) {
		REG32(PCRP0+i*4) |= (EnForceMode);		
		rtl8651_getAsicEthernetPHYReg(i, 0, &statCtrlReg);
		statCtrlReg |= POWER_DOWN;
		rtl8651_setAsicEthernetPHYReg(i, 0, statCtrlReg);				
	}	
}

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
#ifndef CONFIG_RTK_VOIP
int32 rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > PHY5))
		return FAILED; 

	if ((fcON > (QG_DSC_FCON_MASK >> QG_DSC_FCON_OFFSET)) || 
		(fcOFF > (QG_DSC_FCOFF_MASK >> QG_DSC_FCOFF_OFFSET)))
		return FAILED; 


	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QDBFCRP0G0+(port*0xC)), (READ_MEM32(QDBFCRP0G0+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET)); 		
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:		
			WRITE_MEM32((QDBFCRP0G1+(port*0xC)), (READ_MEM32(QDBFCRP0G1+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET)); 		
			break;
		case QUEUE5:
			WRITE_MEM32((QDBFCRP0G2+(port*0xC)), (READ_MEM32(QDBFCRP0G2+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET)); 		
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > PHY5))
		return FAILED; 

	if ((fcON > (QG_QLEN_FCON_MASK>> QG_QLEN_FCON_OFFSET)) || 
		(fcOFF > (QG_QLEN_FCOFF_MASK >> QG_QLEN_FCOFF_OFFSET)))
		return FAILED; 

	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QPKTFCRP0G0+(port*0xC)), (READ_MEM32(QPKTFCRP0G0+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET)); 		
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:		
			WRITE_MEM32((QPKTFCRP0G1+(port*0xC)), (READ_MEM32(QPKTFCRP0G1+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET)); 
			break;
		case QUEUE5:
			WRITE_MEM32((QPKTFCRP0G2+(port*0xC)), (READ_MEM32(QPKTFCRP0G2+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET)); 
			break;
		default:
			return FAILED;
	}

	return SUCCESS;
}
int32 rtl8651_setAsicPortBasedFlowControlRegister(enum PORTID port, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((fcON > (P_MaxDSC_FCON_MASK >> P_MaxDSC_FCON_OFFSET)) || 
		(fcOFF > (P_MaxDSC_FCOFF_MASK >> P_MaxDSC_FCOFF_OFFSET)))
		return FAILED; 

	switch (port)
	{
		case PHY0:
			WRITE_MEM32(PBFCR0, (READ_MEM32(PBFCR0) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY1:
			WRITE_MEM32(PBFCR1, (READ_MEM32(PBFCR1) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY2:
			WRITE_MEM32(PBFCR2, (READ_MEM32(PBFCR2) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY3:
			WRITE_MEM32(PBFCR3, (READ_MEM32(PBFCR3) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY4:
			WRITE_MEM32(PBFCR4, (READ_MEM32(PBFCR4) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case PHY5:
			WRITE_MEM32(PBFCR5, (READ_MEM32(PBFCR5) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
			break;
		case CPU:
			WRITE_MEM32(PBFCR6, (READ_MEM32(PBFCR6) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;			
		default:
			return FAILED;
	}

	return SUCCESS;
}
#endif

#define FC_S_DSC_RUNOUT        500
#define FC_S_DSC_FCON          400
#define FC_S_DSC_FCOFF         280
#define FC_S_MaxSBuf_FCON      310
#define FC_S_MaxSBuf_FCOFF     252
#define FC_P_MaxDSC_FCON        320
#define FC_P_MaxDSC_FCOFF       258

static void qos_init(void)
{
/* suggested value from David Lu:
set global_thrList  {\
            S_DSC_RUNOUT        500         \
            S_DSC_FCON          400         \ 
            S_DSC_FCOFF         280         \ 
            S_MaxSBuf_FCON      310         \
            S_MaxSBuf_FCOFF     252         \
            Q_P0_EN_FC          0x3F        \
            Q_P1_EN_FC          0x3F        \
            Q_P2_EN_FC          0x3F        \
            Q_P3_EN_FC          0x3F        \
            Q_P4_EN_FC          0x3F        \
            Q_P5_EN_FC          0x3F        \
            Q_P6_EN_FC          0x3F        \
            QLEN_GAP            72          \
            P0QNum              4           \
            P1QNum              4           \
            P2QNum              4           \
            P3QNum              4           \
            P4QNum              4           \
            P5QNum              4           \
            P6QNum              4           \
            };                               
        # -----------------------------------
set port_thrList   {\
            P_MaxDSC_FCON        320        \
            P_MaxDSC_FCOFF       258        \
            QG0_DSC_FCON         42         \
            QG0_DSC_FCOFF        33         \
            QG1_DSC_FCON         42         \
            QG1_DSC_FCOFF        33         \
            QG2_DSC_FCON         42         \
            QG2_DSC_FCOFF        33         \
            QG0_QLEN_FCON        24         \
            QG0_QLEN_FCOFF       20         \
            QG1_QLEN_FCON        24         \
            QG1_QLEN_FCOFF       20         \
            QG2_QLEN_FCON        24         \
            QG2_QLEN_FCOFF       20         \
            } ;

	Flow control turn off/on descriptor threshold:
		Flow Control OFF Threshold = 2 * QG_DSC_FCOFF[4:0]
		Flow Control ON Threshold = 2 * QG_DSC_FCON[4:0]
		so QG_DSC_FCON[4:0] = 42/2 = 21; QG_DSC_FCOFF[4:0] = 33/2 = 16

	Flow control turn off/on packet threshold:
		OFF Threshold = 4 * QG_QLEN_FCOFF[3:0]
		ON Threshold = 4 * QG_QLEN_FCON[3:0]
		so QG_QLEN_FCON[3:0] = 24/4 = 6; QG_QLEN_FCOFF[3:0] = 20/4 = 5
 */
	int i, j;

	/* System Based Flow Control Threshold Register */
	WRITE_MEM32(SBFCR0, (READ_MEM32(SBFCR0) & ~(S_DSC_RUNOUT_MASK)) | (FC_S_DSC_RUNOUT << S_DSC_RUNOUT_OFFSET));
	WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( FC_S_DSC_FCON<< S_DSC_FCON_OFFSET) | (FC_S_DSC_FCOFF << S_DSC_FCOFF_OFFSET));
	WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (FC_S_MaxSBuf_FCON << S_Max_SBuf_FCON_OFFSET) | (FC_S_MaxSBuf_FCOFF << S_Max_SBuf_FCOFF_OFFSET));

    	REG32(QNUMCR) = P0QNum_4 | P1QNum_4 | P2QNum_4 | P3QNum_4 | P4QNum_4| P6QNum_4;
	
	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(0, i, 21, 16);
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(PHY0+j, i, 21, 16);

		rtl8651_setAsicQueuePacketBasedFlowControlRegister(0, i, 6, 5);
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueuePacketBasedFlowControlRegister(PHY0+j, i, 6, 5);
	}
		
	rtl8651_setAsicPortBasedFlowControlRegister(0, FC_P_MaxDSC_FCON, FC_P_MaxDSC_FCOFF);
	for(j=1;j<=CPU;j++)
		rtl8651_setAsicPortBasedFlowControlRegister(PHY0+j, FC_P_MaxDSC_FCON, FC_P_MaxDSC_FCOFF);

	WRITE_MEM32(PQPLGR, (READ_MEM32(PQPLGR) & ~(QLEN_GAP_MASK)) | (0x48 << QLEN_GAP_OFFSET)); 		

	// open all port Queue 5 flow control
	WRITE_MEM32(FCCR0, 0x3f3f3f3f);
	WRITE_MEM32(FCCR1, 0x3f3f3f3f);
}

// EEE PHY -- Page 4
// register 16
#define P4R16_eee_10_cap                           (1 << 13)	// enable EEE 10M
#define P4R16_eee_nway_en                           (1 << 12)	// enable Next Page exchange in nway for EEE 100M
#define P4R16_tx_quiet_en                            (1 << 9)	// enable ability to turn off pow100tx when TX Quiet state
#define P4R16_rx_quiet_en                            (1 << 8)	// enable ability to turn off pow100rx when RX Quiet state

// register 25
#define P4R25_rg_dacquiet_en                            (1 << 10)	// enable ability to turn off DAC when TX Quiet state
#define P4R25_rg_ldvquiet_en                            (1 << 9)		// enable ability to turn off line driver when TX Quiet state
#define P4R25_rg_eeeprg_rst                            (1 << 6)		// reset for EEE programmable finite state machine
#define P4R25_rg_ckrsel                            		(1 << 5)		// select ckr125 as RX 125MHz clock
#define P4R25_rg_eeeprg_en                            (1 << 4)		// enable EEE programmable finite state machine

static const unsigned short phy_data[]={
	0x5000,  // write, address 0
	0x6000,  // write, address 1
	0x7000,  // write, address 2
	0x4000,  // write, address 3
	0xD36C,  // write, address 4
	0xFFFF,  // write, address 5
	0x5060,  // write, address 6
	0x61C5,  // write, address 7
	0x7000,  // write, address 8
	0x4001,  // write, address 9
	0x5061,  // write, address 10
	0x87F5,  // write, address 11
	0xCE60,  // write, address 12
	0x0026,  // write, address 13
	0x8E03,  // write, address 14
	0xA021,  // write, address 15
	0x300B,  // write, address 16
	0x58A0,  // write, address 17
	0x629C,  // write, address 18
	0x7000,  // write, address 19
	0x4002,  // write, address 20
	0x58A1,  // write, address 21
	0x87EA,  // write, address 22
	0xAE25,  // write, address 23
	0xA018,  // write, address 24
	0x3016,  // write, address 25
	0x6894,  // write, address 26
	0x6094,  // write, address 27
	0x5123,  // write, address 28
	0x63C2,  // write, address 29
	0x5127,  // write, address 30
	0x4003,  // write, address 31
	0x87E0,  // write, address 32
	0x8EF3,  // write, address 33
	0xA10E,  // write, address 34
	0xCC40,  // write, address 35
	0x0007,  // write, address 36
	0xCA40,  // write, address 37
	0xFFE0,  // write, address 38
	0xA202,  // write, address 39
	0x3020,  // write, address 40
	0x7008,  // write, address 41
	0x3020,  // write, address 42
	0xCC44,  // write, address 43
	0xFFF4,  // write, address 44
	0xCC44,  // write, address 45
	0xFFF2,  // write, address 46
	0x3000,  // write, address 47
	0x5220,  // write, address 48
	0x4004,  // write, address 49
	0x3000,  // write, address 50
	0x64A0,  // write, address 51
	0x5429,  // write, address 52
	0x4005,  // write, address 53
	0x87CA,  // write, address 54
	0xCE18,  // write, address 55
	0xFFC8,  // write, address 56
	0xCE64,  // write, address 57
	0xFFD0,  // write, address 58
	0x3036,  // write, address 59
	0x65C0,  // write, address 60
	0x50A9,  // write, address 61
	0x4006,  // write, address 62
	0xA3DB,  // write, address 63
	0x303F,  // write, address 64
};

static int ram_code_done=0;

void set_ram_code(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data)/sizeof(unsigned short);

	if (ram_code_done)
		return;
	
	rtl8651_getAsicEthernetPHYReg( 4, 0x19, &reg );
	
	// turn on rg_eeeprg_rst
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));

	// turn on mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	// begin to write all RAM
	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data[i]);
	}
	
	for(i=0;i<63;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, 0);
	}	
	
	// finish reading all RAM
	// turn off mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);

	// turn off rg_eeeprg_rst, enable EEE programmable finite state machine
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

static const unsigned short phy_data_b[]={
  0x5000,  // write, address 0
  0x6000,  // write, address 1
  0x7000,  // write, address 2
  0x4000,  // write, address 3
  0x8700,  // write, address 4
  0xD344,  // write, address 5
  0xFFFF,  // write, address 6
  0xCA6C,  // write, address 7
  0xFFFD,  // write, address 8
  0x5460,  // write, address 9
  0x61C5,  // write, address 10
  0x7000,  // write, address 11
  0x4001,  // write, address 12
  0x5461,  // write, address 13
  0x4001,  // write, address 14
  0x87F1,  // write, address 15
  0xCE60,  // write, address 16
  0x0026,  // write, address 17
  0x8E03,  // write, address 18
  0xA021,  // write, address 19
  0x300F,  // write, address 20
  0x5CA0,  // write, address 21
  0x629C,  // write, address 22
  0x7000,  // write, address 23
  0x4002,  // write, address 24
  0x5CA1,  // write, address 25
  0x87E6,  // write, address 26
  0xAE25,  // write, address 27
  0xA018,  // write, address 28
  0x301A,  // write, address 29
  0x6E94,  // write, address 30
  0x6694,  // write, address 31
  0x5523,  // write, address 32
  0x63C2,  // write, address 33
  0x5527,  // write, address 34
  0x4003,  // write, address 35
  0x87DC,  // write, address 36
  0x8EF3,  // write, address 37
  0xA10E,  // write, address 38
  0xCC40,  // write, address 39
  0x0007,  // write, address 40
  0xCA40,  // write, address 41
  0xFFDF,  // write, address 42
  0xA202,  // write, address 43
  0x3024,  // write, address 44
  0x7008,  // write, address 45
  0x3024,  // write, address 46
  0xCC44,  // write, address 47
  0xFFF4,  // write, address 48
  0xCC44,  // write, address 49
  0xFFF2,  // write, address 50
  0x3000,  // write, address 51
  0x5620,  // write, address 52
  0x4004,  // write, address 53
  0x3000,  // write, address 54
  0x64A0,  // write, address 55
  0x5429,  // write, address 56
  0x4005,  // write, address 57
  0x87C6,  // write, address 58
  0xCE18,  // write, address 59
  0xFFC4,  // write, address 60
  0xCE64,  // write, address 61
  0xFFCF,  // write, address 62
  0x303A,  // write, address 63
  0x65C0,  // write, address 64
  0x54A9,  // write, address 65
  0x4006,  // write, address 66
  0xA3DB,  // write, address 67
  0x3043,  // write, address 68
};

void set_ram_code_b(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data_b)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	rtl8651_getAsicEthernetPHYReg(4, 0x19, &reg );
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));	
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data_b[i]);
	}
	
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

void eee_phy_enable_by_port(int port)
{
	uint32 reg;

	// change to page 4
	rtl8651_setAsicEthernetPHYReg(port, 31, 4);

	// enable EEE N-way & set turn off power at quiet state
	rtl8651_getAsicEthernetPHYReg( port, 16, &reg );
	reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);

#ifdef CONFIG_RTL8196C_ETH_IOT	
	reg |= P4R16_eee_10_cap;	// enable 10M_EEE also.
#endif	
	rtl8651_setAsicEthernetPHYReg( port, 16, reg );

	// enable EEE turn off DAC and line driver when TX Quiet state
	rtl8651_getAsicEthernetPHYReg( port, 25, &reg );
//	reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
	reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);
	
	rtl8651_setAsicEthernetPHYReg( port, 25, reg );

	rtl8651_setAsicEthernetPHYReg( port, 17, 0xa2a2 );
	rtl8651_setAsicEthernetPHYReg( port, 19, 0xc5c2 );
	rtl8651_setAsicEthernetPHYReg( port, 24, 0xc0f3 );

	if ((REG32(REVR) == RTL8196C_REVISION_A) && (port == 4)) {
			set_ram_code();
	}
	else if ((REG32(REVR) == RTL8196C_REVISION_B) && (port == 4)) {
		set_ram_code_b();
	}
	
	// switch to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
}

#ifdef DBG_EEE
void eee_phy_enable(void)
{
	int i;
	uint32 reg;

	// EEE PHY enable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// enable EEE N-way & set turn off power at quiet state
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// enable EEE turn off DAC and line driver when TX Quiet state
		rtl8651_getAsicEthernetPHYReg( i, 25, &reg );
//		reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
		reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);
		rtl8651_setAsicEthernetPHYReg( i, 25, reg );

		rtl8651_setAsicEthernetPHYReg( i, 17, 0xa2a2 );
		rtl8651_setAsicEthernetPHYReg( i, 19, 0xc5c2 );
		rtl8651_setAsicEthernetPHYReg( i, 24, 0xc0f3 );

		if ((REG32(REVR) == RTL8196C_REVISION_A) && (i == 4)) {
				set_ram_code();
		}
		else if ((REG32(REVR) == RTL8196C_REVISION_B) && (i == 4)) {
			set_ram_code_b();
		}
		
		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1, i);				
	}
}

void eee_phy_disable(void)
{
	int i;
	uint32 reg;

	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// disable (EEE N-way & turn off power at quiet state)
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg = reg & 0xECFF;
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1, i);				
	}
}
#endif
#endif

void set_phy_pwr_save(int id, int val)
{
	uint32 reg_val;	
	int i, start, end;
	
	if (id == 99)
		{ start=0; end=4; }
	else if (id <= 4)
		{ start = end = id; }
	else
		return;
	for(i=start; i<=end; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 24, &reg_val);

		if (val == 1)
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val | BIT(15)) );
		else 
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val & (~BIT(15))) );
	}
}

#ifdef DBG_PHY_REG
void phy_op(uint32 op, uint32 id, uint32 page, uint32 reg, uint32 *val)
{
	int i, start, end;
	
	if ((op == 1) && (id == 99))
		{ start=0; end=4; }
	else if (id <= 4)
		{ start = end = id; }
	else
		return;
	
	for(i=start; i<=end; i++)
	{
		//change page 
		if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( i, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( i, 30, page  );
		}
		else if (page)
		{
			rtl8651_setAsicEthernetPHYReg( i, 31, page  );
		}

		if (op == 1) //read
			rtl8651_getAsicEthernetPHYReg( i, reg, val);
		else 
			rtl8651_setAsicEthernetPHYReg( i, reg, *val);

		//switch to page 0
		if (page)
			rtl8651_setAsicEthernetPHYReg( i, 31, 0  );

		if (op == 0)
			rtl8651_restartAsicEthernetPHYNway(i+1, i);				
			
		panic_printk("  PHY: id= %d, page= %d, reg= %d, val= 0x%x.\n", 
			i, page, reg, *val);
	}
}
#endif

#ifdef CONFIG_RTL865X_SUPPORT_IPV6_MLD
void setACL_trap_ipv6_mcast(int index)
{
    
        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));

	    //printk("setACL_trap_ipv6_mcast\n");		

        ap.actionType_ = RTL8651_ACL_CPU;
        ap.ruleType_ = RTL8651_ACL_MAC;
        
        ap.un_ty.MAC._dstMac.octet[0]=0x33;
        ap.un_ty.MAC._dstMac.octet[1]=0x33;
        ap.un_ty.MAC._dstMac.octet[2]=0x00;
        ap.un_ty.MAC._dstMac.octet[3]=0x00;
        ap.un_ty.MAC._dstMac.octet[4]=0x00;
        ap.un_ty.MAC._dstMac.octet[5]=0x00;	
		
        ap.un_ty.MAC._dstMacMask.octet[0]=0xFF;
        ap.un_ty.MAC._dstMacMask.octet[1]=0xFF;
		
        
        ap.un_ty.MAC._typeLenMask = 0;
        ap.un_ty.MAC._typeLen = 0;
        ret = swCore_aclCreate(index, &ap);
        
        if ( ret != 0 ) { 
            printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );

        }    
	
} 

void setACL_fwd_ipv6_mcast(int index)	// ping6_issue
{
        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));

        ap.actionType_ = RTL8651_ACL_PERMIT;
        ap.ruleType_ = RTL8651_ACL_MAC;
	
        ap.un_ty.MAC._dstMac.octet[0]=0x33;
        ap.un_ty.MAC._dstMac.octet[1]=0x33;
        ap.un_ty.MAC._dstMac.octet[2]=0xFF;
		
        ap.un_ty.MAC._dstMacMask.octet[0]=0xFF;
        ap.un_ty.MAC._dstMacMask.octet[1]=0xFF;
        ap.un_ty.MAC._dstMacMask.octet[2]=0xFF;		
	
        ret = swCore_aclCreate(index, &ap);
        
        if ( ret != 0 ) { 
            printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );
        }    	
}  
#endif

#if defined(CONFIG_RTL865X_SUPPORT_IPV6_MLD) || defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
void EasyACLRule(int index, int action)
{

        int ret;
        rtl_acl_param_t ap;			
        bzero((void *) &ap, sizeof(rtl_acl_param_t));		
 
 				switch(action)
 				{
 					case ACL_IPV6_TO_CPU:
		       	ap.actionType_ = RTL8651_ACL_CPU;
		        ap.ruleType_ = RTL8651_ACL_MAC;
            ap.un_ty.MAC._typeLenMask = 0xffff;
        		ap.un_ty.MAC._typeLen = 0x86dd;					
 						
 						break;
 						
 					default:
        ap.actionType_ = action;
			      break;
 				}
 				
 		ap.pktOpApp = RTL865XC_ACLTBL_ALL_LAYER;		
        ret = swCore_aclCreate(index, &ap);
        
        if ( ret != 0 ) { 
            printk("865x-nic: swCore_aclCreate() failed:%d\n", ret );
        } 
		
}
#endif

