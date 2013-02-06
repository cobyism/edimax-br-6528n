/*
 *  Header file for hardware related definitions
 *
 *  $Id: 8192cd_hw.h,v 1.11.2.6 2010/11/11 07:54:36 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_HW_H_
#define _8192CD_HW_H_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#include <asm/io.h>
#endif

#ifdef __DRAYTEK_OS__
#include <draytek/skbuff.h>
#endif

#include "./8192c_reg.h"
#include "./8192cd_cfg.h"

#ifndef __KERNEL__
#include "./sys-support.h"
#endif

#include "./wifi.h"
#include "./8192cd_phyreg.h"


typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

//#define RTL8192CD_REGS_SIZE        ((0xff + 1) * 16)


#if 0
enum _RF_TYPE_	{
	_11BG_RF_ZEBRA_		= 0x07,
	_11ABG_RF_OMC8255_	= 0x08,
	_11ABG_RF_OMC8255B_	= 0x09,
	_11BGN_RF_8256_		= 0x0a,
};
#endif

#define VERSION_MASK  0xf00f
enum _CHIP_VERSION_ {
	VERSION_8188C = 0x1000,
	VERSION_8192C = 0x1001,
};

#if 0
enum _ZEBRA_VERSION_ {
	VERSION_ZEBRA_A	= 0,
	VERSION_ZEBRA_B	= 1,
	VERSION_ZEBRA_C	= 2,
	VERSION_ZEBRA_E	= 3,
	VERSION_ZEBRA2_A = 4,
};

enum _TCR_CONFIG_ {
	_DIS_REQ_QSZ_ = BIT(28),
	_SAT_		= BIT(24),

	_TX_DMA16_	= 0x0,
	_TX_DMA32_	= BIT(21),
	_TX_DMA64_	= BIT(22),
	_TX_DMA128_	= BIT(22) | BIT(21),
	_TX_DMA256_	= BIT(23),
	_TX_DMA512_	= BIT(23) | BIT(21),
	_TX_DMA1K_	= BIT(23) | BIT(22),
	_TX_DMA2K_	= BIT(23) | BIT(22) | BIT(21),

	_DISCW_		= BIT(20),
	_ICV_APP_	= BIT(19),

	_MAC_LBK_	= BIT(17),
	_BB_LBK_	= BIT(18),
	_CNT_TX_	= BIT(18) | BIT(17),

	_DURPROCMODE_SHIFT_ = 30,
	_SRL_SHIFT_	= 8,
	_LRL_SHIFT_ = 0,
};

enum _RCR_CONFIG_ {
	_ERLYRXEN_		= BIT(31),
	_ENCS2_			= BIT(30),
	_ENCS1_			= BIT(29),
	_ENMARP_		= BIT(28),
	_ENMBID_		= BIT(27),
	_CAM_SEARCH_	= BIT(26),

	_CBSSID_		= BIT(23),
	_APWRMGT_		= BIT(22),
	_ADD3_			= BIT(21),
	_AMF_			= BIT(20),
	_ACF_			= BIT(19),
	_ADF_			= BIT(18),

	_RX_ERLY64_		= BIT(14),
	_RX_ERLY128_	= BIT(13) | BIT(14),
	_RX_ERLY256_	= BIT(15),
	_RX_ERLY512_	= BIT(15) | BIT(13),
	_RX_ERLY1K_		= BIT(15) | BIT(14),
	_NO_ERLYRX_		= BIT(15) | BIT(14) | BIT(13),

	_AICV_			= BIT(12),

    _RX_DMA16_		= 0x0,
    _RX_DMA32_		= BIT(8),
    _RX_DMA64_		= BIT(9),
    _RX_DMA128_		= BIT(9) | BIT(8),
    _RX_DMA256_		= BIT(10),
    _RX_DMA512_		= BIT(10) | BIT(8),
    _RX_DMA1K_		= BIT(10) | BIT(9),
    _RX_DMA2K_		= BIT(10) | BIT(9) | BIT(8),

	_9356SEL_		= BIT(6),
	_ACRC32_		= BIT(5),
	_AB_			= BIT(3),
	_AM_			= BIT(2),
	_APM_			= BIT(1),
	_AAP_			= BIT(0),
};

enum _SCR_CONFIG_ {
	_PRIVACY_WEP40_		= BIT(0),
	_PRIVACY_WEP104_	= BIT(4),
};


#define ENC_NONE		0
#define ENC_WEP40		BIT(2)//|BIT(5)//FIX for DEFAULT KEY
#define ENC_TKIP_NO_MIC	BIT(3)
#define ENC_TKIP_MIC	BIT(3)|BIT(2)
#define ENC_AES			BIT(4)
#define ENC_WEP104		BIT(4)|BIT(2)//|BIT(5)//FIX for DEFAULT KEY

//#if defined(RTL8190) || defined(RTL8192E)
#if 0
enum _RTL8190_AP_HW_ {
	_IDR0_		= 0x0,
	_PCIF_		= 0x9,		// PCI Function Register
	_9346CR_	= 0x0E,
	_ANAPAR_	= 0x17,	//NOTE: 8192 only register, for triggering pll on ... joshua
	_BBGLBRESET_ = 0x20,

	_BSSID_		= 0x2e,
	_CR_		= 0x37,
	_SIFS_CCK_	= 0x3c,
	_SIFS_OFDM_	= 0x3e,
	_TCR_		= 0x40,
	_RCR_		= 0x44,
	_SLOT_		= 0x49,
	_EIFS_		= 0x4a,
	_ACKTIMEOUT_ = 0x4c,

	_ACBE_PARM_	= 0x50,		// BE Parameter
	_ACBK_PARM_	= 0x54,		// BK Parameter
	_ACVO_PARM_	= 0x58,		// VO Parameter
	_ACVI_PARM_	= 0x5C,		// VI Parameter

	_BCNTCFG_	= 0x62,
	_TIMER1_	= 0x68,
	_TIMER2_	= 0x6c,
	_BCNITV_	= 0x70,
	_ATIMWIN_	= 0x72,
	_DRVERLYINT_ = 0x74,
	_BCNDMA_	= 0x76,

	_MBIDCAMCFG_	= 0xc0,
	_MBIDCAMCONTENT_ = 0xc4,

	_IMR_		= 0xF4,		// Interrupt Mask Register
	_ISR_		= 0xF8,		// Interrupt Status Register
	_DBS_		= 0xFC,		// Debug Select
	_TXPOLL_	= 0xFD,		// Transmit Polling
	_TXPOLL_H_	= 0xFE,
	_PSR_		= 0xFF,		// Page Select Register
	_CPURST_	= 0x100,	// CPU Reset
	_BLDTIME_	= 0x124,
//	_BLDUSER0_	= 0x128,
//	_BLDUSER1_	= 0x12c,
	_TXPKTNUM_	= 0x128,
	_RXPKTNUM_	= 0x130,

	_LED1CFG_	= 0x154,
	_LED0CFG_	= 0x155,

	_ACM_CTRL_	= 0x171,

	_RQPN1_		= 0x180,
	_RQPN2_		= 0x184,
	_RQPN3_		= 0x188,

	_TBDA_		= 0x200,	// Transmit Beacon Desc Addr
	_THPDA_		= 0x204,	// Transmit High Priority Desc Addr
	_TCDA_		= 0x208,	// Transmit Command Desc Addr
	_TMGDA_		= 0x20C,	// Transmit Management Desc Addr
	_HDA_		= 0x210,	// HCCA Desc Addr
	_TNPDA_		= 0x214,	// Transmit VO Desc Addr
	_TLPDA_		= 0x218,	// Transmit VI Desc Addr
	_TBEDA_		= 0x21C,	// Transmit BE Desc Addr
	_TBKDA_		= 0x220,	// Transmit BK Desc Addr
	_RCDSA_		= 0x224,	// Receive Command Desc Addr
	_RDSAR_		= 0x228,	// Receive Desc Starting Addr
	_MAR0_		= 0x240,
	_MAR4_		= 0x244,
	_MBIDCTRL_		= 0x260,
	_BWOPMODE_	= 0x300,
	_MSR_		= 0x303,
	_RETRYCNT_	= 0x304,
	_TSFTR_L_	= 0x308,
	_TSFTR_H_	= 0x30c,
	_RRSR_		= 0x310,
	_RATR_POLL_	= 0x318,
	_RATR0_		= 0x320,
	_RATR1_		= 0x324,
	_RATR2_		= 0x328,
	_RATR3_		= 0x32c,
	_RATR4_		= 0x330,
	_RATR5_		= 0x334,
	_RATR6_		= 0x338,
	_RATR7_		= 0x33c,
#ifdef RTL8190
	_MCS_TXAGC_0_	= 0x340,
	_MCS_TXAGC_1_	= 0x344,
	_CCK_TXAGC_		= 0x348,
#elif defined(RTL8192E)
	_ISRD_CPU_	= 0x350,
	_FWIMR_		= 0x354,
#endif
	_FWPSR_		= 0x3FF,
};
#endif
//#elif defined(RTL8192SE)

enum _RTL8190_AP_HW_ {
	_RCR_           = RCR,
	_MBIDCTRL_		= MBIDCTRL,
	_BSSID_		= BSSIDR,
	_MBIDCAMCONTENT_ = MBIDCAMCONTENT,
	_MBIDCAMCFG_	= MBIDCAMCFG,
	_SLOT_		= SLOT_TIME,
	_ACBE_PARM_	= EDCAPARA_BE,		// BE Parameter
	_ACBK_PARM_	= EDCAPARA_BK,		// BK Parameter
	_ACVO_PARM_	= EDCAPARA_VO,		// VO Parameter
	_ACVI_PARM_	= EDCAPARA_VI,		// VI Parameter
	_TIMER1_	= TIMER0,
	_TIMER2_	= TIMER1,
	_IMR_		= IMR,				// Interrupt Mask Register
	_ISR_		= ISR,				// Interrupt Status Register
	_ACM_CTRL_	= ACMHWCTRL,
	_TBDA_		= TBDA,				// Transmit Beacon Desc Addr
	_THPDA_		= THPDA,			// Transmit High Priority Desc Addr
	_TCDA_		= TCDA,				// Transmit Command Desc Addr
	_TMGDA_		= TMDA,				// Transmit Management Desc Addr
	_HDA_		= HDA,				// HCCA Desc Addr
	_TNPDA_		= TVODA,			// Transmit VO Desc Addr
	_TLPDA_		= TVIDA,			// Transmit VI Desc Addr
	_TBEDA_		= TBEDA,			// Transmit BE Desc Addr
	_TBKDA_		= TBKDA,			// Transmit BK Desc Addr
	_RCDSA_		= RCDA,				// Receive Command Desc Addr
	_RDSAR_		= RDSA,				// Receive Desc Starting Addr
	_BWOPMODE_	= BW_OPMODE,
	_MSR_		= MSR,
	_TSFTR_L_	= TSFR,
	_TSFTR_H_	= (TSFR+4),
#ifdef CONFIG_RTK_MESH
	_RRSR_		= RRSR,
#endif
	_RATR_POLL_	= 0x320,			// need to fix
	_RATR0_		= 0x320,			// need to fix
	_RATR1_		= 0x320,			// need to fix
	_RATR2_		= 0x320,			// need to fix
	_RATR3_		= 0x320,			// need to fix
	_RATR4_		= 0x320,			// need to fix
	_RATR5_		= 0x320,			// need to fix
	_RATR6_		= 0x320,			// need to fix
	_RATR7_		= 0x320,			// need to fix
};
//#endif


//#if defined(RTL8190) || defined(RTL8192E)
//enum _AP_SECURITY_REGS_ {
//	_CAMCMD_	= 0xa0,
//	_CAM_W_		= 0xa4,
//	_CAM_R_		= 0xa8,
//	_CAMDBG_	= 0xac,
//	_WPACFG_	= 0xb0,
//};
//#elif defined(RTL8192SE)

enum _AP_SECURITY_REGS_ {
	_CAMCMD_	= RWCAM,
	_CAM_W_		= WCAMI,
	_CAM_R_		= RCAMO,
	_CAMDBG_	= CAMDBG,
	_WPACFG_	= SECR,
};

//#endif

enum _RTL8190_DESC_CMD_ {
	// TX and common
	_OWN_			= BIT(31),
	_LINIP_			= BIT(30),
	_FS_			= BIT(29),
	_LS_			= BIT(28),
	_CMDINIT_		= BIT(27),
	_PIFS_			= BIT(15),
	_NOENC_			= BIT(14),
	_MFRAG_			= BIT(13),
	_USERATE_		= BIT(12),
	_DISFB_			= BIT(11),
	_RATID_			= BIT(8),
	_RATIDSHIFT_	= 8,
	_OFFSETSHIFT_	= 16,
	_QSELECTSHIFT_	= 16,

	// RX
	_EOR_			= BIT(30),
	_SWDEC_			= BIT(27),
	_PHYST_			= BIT(26),
	_SHIFT1_		= BIT(25),
	_SHIFT0_		= BIT(24),
	_ICV_			= BIT(15),
	_CRC32_			= BIT(14),
	_RXFRLEN_MSK_	= 0x3fff,
	_RXDRVINFOSZ_SHIFT_ = 16,
};

enum _IMR_BITFIELD_90_ {
//#ifdef RTL8190
//	_CPUERR_		= BIT(29),
//#elif defined(RTL8192E)
	_IllAcess_		= BIT(30),
	_BTEvent_		= BIT(29),
//#endif
	_ATIMEND_		= BIT(28),
	_TBDOK_			= BIT(27),
	_TBDER_			= BIT(26),
	_BCNDMAINT5_	= BIT(25),
	_BCNDMAINT4_	= BIT(24),
	_BCNDMAINT3_	= BIT(23),
	_BCNDMAINT2_	= BIT(22),
	_BCNDOK5_		= BIT(21),
	_BCNDOK4_		= BIT(20),
	_BCNDOK3_		= BIT(19),
	_BCNDOK2_		= BIT(18),
	_TIMEOUT2_		= BIT(17),
	_TIMEOUT1_		= BIT(16),
	_TXFOVW_		= BIT(15),
	_PSTIMEOUT_		= BIT(14),
	_BCNDMAINT_		= BIT(13),
	_RXFOVW_		= BIT(12),
	_RDU_			= BIT(11),
	_RXCMDOK_		= BIT(10),
	_BCNDOK_		= BIT(9),
	_THPDOK_		= BIT(8),
	_COMDOK_		= BIT(7),
	_MGTDOK_		= BIT(6),
	_HCCADOK_		= BIT(5),
	_TBKDOK_		= BIT(4),
	_TBEDOK_		= BIT(3),
	_TVIDOK_		= BIT(2),
	_TVODOK_		= BIT(1),
	_ROK_			= BIT(0),
};
enum _AP_SECURITY_SETTINGS_ {
	//CAM CMD
	_CAM_POLL_			= BIT(31),
	_CAM_CLR_			= BIT(30),
	_CAM_WE_			= BIT(16),

	//CAM DBG
	_CAM_INFO_			= BIT(31),
	_KEY_FOUND_			= BIT(30),

	//SEC CFG
	_NO_SK_MC_			= BIT(5),
	_SK_A2_				= BIT(4),
	_RX_DEC_			= BIT(3),
	_TX_ENC_			= BIT(2),
	_RX_USE_DK_			= BIT(1),
	_TX_USE_DK_			= BIT(0),
};
#endif

enum _RF_TX_RATE_ {
	_1M_RATE_	= 2,
	_2M_RATE_	= 4,
	_5M_RATE_	= 11,
	_6M_RATE_	= 12,
	_9M_RATE_	= 18,
	_11M_RATE_	= 22,
	_12M_RATE_	= 24,
	_18M_RATE_	= 36,
	_22M_RATE_	= 44,
	_24M_RATE_	= 48,
	_33M_RATE_	= 66,
	_36M_RATE_	= 72,
	_48M_RATE_	= 96,
	_54M_RATE_	= 108,
	_MCS0_RATE_	= (0x80 |  0),
	_MCS1_RATE_	= (0x80 |  1),
	_MCS2_RATE_	= (0x80 |  2),
	_MCS3_RATE_	= (0x80 |  3),
	_MCS4_RATE_	= (0x80 |  4),
	_MCS5_RATE_	= (0x80 |  5),
	_MCS6_RATE_	= (0x80 |  6),
	_MCS7_RATE_	= (0x80 |  7),
	_MCS8_RATE_	= (0x80 |  8),
	_MCS9_RATE_	= (0x80 |  9),
	_MCS10_RATE_= (0x80 | 10),
	_MCS11_RATE_= (0x80 | 11),
	_MCS12_RATE_= (0x80 | 12),
	_MCS13_RATE_= (0x80 | 13),
	_MCS14_RATE_= (0x80 | 14),
	_MCS15_RATE_= (0x80 | 15),
};

#if 0
enum _HW_STATE_		{
	_HW_STATE_STATION_	= 0x02,
	_HW_STATE_ADHOC_	= 0x01,
	_HW_STATE_AP_		= 0x03,
	_HW_STATE_NOLINK_	= 0x0,
};
#endif

enum BANDWIDTH_MODE
{
	BW_OPMODE_11J	= BIT(0),
	BW_OPMODE_5G	= BIT(1),
	BW_OPMODE_20MHZ	= BIT(2)
};

#if 0
//#ifdef RTL8192SE
enum _FW_REG364_MASK_
{
	FW_REG364_DIG	= BIT(0),
	FW_REG364_HP	= BIT(1),
	FW_REG364_RSSI = BIT(2),
//	FW_REG364_IQK	= BIT(3)
};
#endif
enum _ARFR_TABLE_SET_
{
	ARFR_2T_40M = 0,
	ARFR_2T_20M = 2,
	ARFR_1T_40M = 12,
	ARFR_1T_20M = 13,
	ARFR_BG_MIX = 4,
	ARFR_G_ONLY = 5,
	ARFR_B_ONLY = 6,
	ARFR_BMC = 6,
};

//#define	HP_UPPER			0x4B //SS = 75%
#define	HP_LOWER			0x46 //SS = 70%
#define RSSI_DIFF_PA		10
#define RSSI_DIFF_LNA		8

//----------------------------------------------------------------------------
//       8192C Rate Definition
//----------------------------------------------------------------------------
//CCK
#define		RATR_1M				0x00000001
#define		RATR_2M				0x00000002
#define		RATR_55M				0x00000004
#define		RATR_11M				0x00000008
//OFDM 		
#define		RATR_6M				0x00000010
#define		RATR_9M				0x00000020
#define		RATR_12M				0x00000040
#define		RATR_18M				0x00000080
#define		RATR_24M				0x00000100
#define		RATR_36M				0x00000200
#define		RATR_48M				0x00000400
#define		RATR_54M				0x00000800
//MCS 1 Spatial Stream	
#define		RATR_MCS0				0x00001000
#define		RATR_MCS1				0x00002000
#define		RATR_MCS2				0x00004000
#define		RATR_MCS3				0x00008000
#define		RATR_MCS4				0x00010000
#define		RATR_MCS5				0x00020000
#define		RATR_MCS6				0x00040000
#define		RATR_MCS7				0x00080000
//MCS 2 Spatial Stream
#define		RATR_MCS8				0x00100000
#define		RATR_MCS9				0x00200000
#define		RATR_MCS10				0x00400000
#define		RATR_MCS11				0x00800000
#define		RATR_MCS12				0x01000000
#define		RATR_MCS13				0x02000000
#define		RATR_MCS14				0x04000000
#define		RATR_MCS15				0x08000000
// ALL CCK Rate
#define	RATE_ALL_CCK				RATR_1M|RATR_2M|RATR_55M|RATR_11M 
#define	RATE_ALL_OFDM_AG			RATR_6M|RATR_9M|RATR_12M|RATR_18M|RATR_24M|\
									RATR_36M|RATR_48M|RATR_54M	
#define	RATE_ALL_OFDM_1SS			RATR_MCS0|RATR_MCS1|RATR_MCS2|RATR_MCS3 |\
									RATR_MCS4|RATR_MCS5|RATR_MCS6	|RATR_MCS7	
#define	RATE_ALL_OFDM_2SS			RATR_MCS8|RATR_MCS9	|RATR_MCS10|RATR_MCS11|\
									RATR_MCS12|RATR_MCS13|RATR_MCS14|RATR_MCS15


#if 0
//----------------------------------------------------------------------------
//       8139 (CR9346) 9346 command register bits (offset 0x50, 1 byte)
//----------------------------------------------------------------------------
#define CR9346_EEDO     0x01            // 9346 data out
#define CR9346_EEDI     0x02            // 9346 data in
#define CR9346_EESK     0x04            // 9346 serial clock
#define CR9346_EECS     0x08            // 9346 chip select
#define CR9346_EEM0     0x40            // select 8139 operating mode
#define CR9346_EEM1     0x80            // 00: normal
                                        // 01: autoload
                                        // 10: 9346 programming
                                        // 11: config write enable
#define	CR9346_CFGRW	0xC0			// Config register write
#define	CR9346_NORM		0x0				//
//-------------------------------------------------------------------------
// EEPROM bit definitions
//-------------------------------------------------------------------------
//- EEPROM control register bits
#define EN_TRNF                     0x10    // Enable turnoff
#define EEDO                        CR9346_EEDO    // EEPROM data out
#define EEDI                        CR9346_EEDI    // EEPROM data in (set for writing data)
#define EECS                        CR9346_EECS    // EEPROM chip select (1=high, 0=low)
#define EESK                        CR9346_EESK    // EEPROM shift clock (1=high, 0=low)

//- EEPROM opcodes
#define EEPROM_READ_OPCODE          06
#define EEPROM_WRITE_OPCODE         05
#define EEPROM_ERASE_OPCODE         07
#define EEPROM_EWEN_OPCODE          19      // Erase/write enable
#define EEPROM_EWDS_OPCODE          16      // Erase/write disable

//- EEPROM data locations
#define	RTL8180_EEPROM_ID			0x8129
#define EEPROM_ID					0x0
//#define EEPROM_RF_CHIP_ID  			0x0C
#define EEPROM_RF_CHIP_ID			0x28
//#define EEPROM_NODE_ADDRESS_BYTE_0  0x0E
#define EEPROM_NODE_ADDRESS_BYTE_0  0x0C // modified by joshua
#define EEPROM_CONFIG2				0x18
#define EEPROM_ANA_PARM				0x1a
//#define	EEPROM_TX_POWER_LEVEL_0		0x20
#define       EEPROM_TX_POWER_LEVEL_0		0x2C // modified by joshua
//#define	EEPROM_CHANNEL_PLAN			0x2E
#define       EEPROM_CHANNEL_PLAN		0x7C
#define EEPROM_CS_THRESHOLD			0x2F
#define EEPROM_ANA_PARM2			0x32
#define EEPROM_RF_PARAM				0x32
#define EEPROM_VERSION				0x3c
#define EEPROM_CIS_DATA				0x80
//#define	EEPROM_11G_CHANNEL_OFDM_TX_POWER_LEVEL_OFFSET	0x40
#define EEPROM_11G_CHANNEL_OFDM_TX_POWER_LEVEL_OFFSET 0x3A
//#define	EEPROM_11A_CHANNEL_TX_POWER_LEVEL_OFFSET		0x4e
#define EEPROM_11A_CHANNEL_TX_POWER_LEVEL_OFFSET 0x2C

#define EEPROM_FLAGS_WORD_3         3
#define EEPROM_FLAG_10MC            BIT(0)
#define EEPROM_FLAG_100MC           BIT(1)


#ifdef RTL8192E
#define EEPROM_RFInd_PowerDiff                  0x28
#define EEPROM_ThermalMeter                     0x29
#define EEPROM_TxPwDiff_CrystalCap              0x2A    //0x2A~0x2B
#define EEPROM_TxPwIndex_CCK                    0x2C    //0x2C~0x39
#define EEPROM_TxPwIndex_OFDM_24G       0x3A    //0x3A~0x47
#endif


//----------------------------------------------------------------------------
//       8180 Config3 Regsiter 			(offset 0x59, 1 byte)
//----------------------------------------------------------------------------
#define	Config3_GNTSel			0x80
#define	Config3_ParmEn			0x40			// enable write to ANA_PARM
												//	(0x54) register
#define	Config3_Magic			0x20			// Enable Magic Packet Wakeup
#define	Config3_CardBEn			0x08			// Cardbus Enable
#define	Config3_CLKRUN_En		0x04			// CLKRUN(clock run) Enable
#define	Config3_FuncRegEn		0x02			// Function Register Enable
#define	Config3_FBtBEn			0x01			// Enable PCI fast-back-to-back
#endif

#ifdef RX_BUFFER_GATHER
enum {
	GATHER_FIRST = 1,
	GATHER_MIDDLE = 2,
	GATHER_LAST = 4,
};
#endif

struct rx_desc {

//#if	defined(RTL8190) || defined(RTL8192)
//	volatile unsigned int	cmd;
//	volatile unsigned int	rsvd0;
//	volatile unsigned int	rsvd1;
//	volatile unsigned int	paddr;
//#elif defined(RTL8192SE)

	volatile unsigned int	Dword0;
	volatile unsigned int	Dword1;
	volatile unsigned int	Dword2;
	volatile unsigned int	Dword3;
	volatile unsigned int	Dword4;  // IV1
	volatile unsigned int	Dword5;	 // TSFL
	volatile unsigned int	Dword6; // BufferAddress
	volatile unsigned int	Dword7; // NextRxDescAddress;
//	volatile unsigned int	Dword8;
//#endif

};

struct rx_desc_info {
	void*				pbuf;
	unsigned int		paddr;
};

struct rf_misc_info {
	unsigned char		mimorssi[2];
	signed char			mimosq[2];
	int					RxSNRdB[2];
};

#ifdef CONFIG_RTK_MESH
struct MESH_HDR {
	unsigned char 	mesh_flag;
	INT8 			TTL;
	UINT16 			segNum;
	unsigned char 	DestMACAddr[MACADDRLEN]; // modify for 6 address
	unsigned char 	SrcMACAddr[MACADDRLEN];
};
#endif

struct rx_frinfo {
	struct sk_buff*		pskb;
	struct list_head	mpdu_list;
	unsigned int		pktlen;
	struct list_head	rx_list;
	unsigned char		*da;
	unsigned char		*sa;
	unsigned int		hdr_len;
	unsigned short		seq;
	unsigned short		tid;
	unsigned char		to_fr_ds;
	unsigned char		rssi;
	unsigned char		sq;
	unsigned char		rx_rate;
	unsigned char		rx_bw;
	unsigned char		rx_splcp;
	unsigned char		driver_info_size;
	unsigned char		rxbuf_shift;
	unsigned char		sw_dec;
	unsigned char		faggr;
	unsigned char		paggr;
//#ifdef RTL8192SE
	unsigned int		physt;
//#endif
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	unsigned char		is_br_mgnt;	 // is a broadcast management frame (beacon and probe-rsp)
#endif
	struct RxFWInfo		*driver_info;
	struct rf_misc_info	rf_info;
	signed char			cck_mimorssi[4];

#ifdef CONFIG_RTK_MESH
	// it's a mandatory field, for rx (e.g., validate_mpdu) to distinguish an 11s frame
	unsigned char		is_11s;	///<  1: 11s

	struct  MESH_HDR mesh_header;		//modify by Joule for MESH HEADER

	unsigned char		prehop_11s[MACADDRLEN];

#endif // CONFIG_RTK_MESH

#ifdef CONFIG_RTL_WAPI_SUPPORT
        //unsigned short        padding;
#endif

#ifdef RX_BUFFER_GATHER
	int gather_flag;
	int gather_len;
#endif
};

struct tx_desc {
/*
#if	defined(RTL8190) || defined(RTL8192)
	volatile unsigned int	cmd;
	volatile unsigned int	opt;
	volatile unsigned int	flen;
	volatile unsigned int	paddr;
	volatile unsigned int	n_desc;
	volatile unsigned int	rsvd0;
	volatile unsigned int	rsvd1;
	volatile unsigned int	rsvd2;
#elif defined(RTL8192SE)
*/
	volatile unsigned int Dword0;
	volatile unsigned int Dword1;
	volatile unsigned int Dword2;
	volatile unsigned int Dword3;
	volatile unsigned int Dword4;
	volatile unsigned int Dword5;
	volatile unsigned int Dword6;
	volatile unsigned int Dword7;
	volatile unsigned int Dword8;  //TxBufferAddr;
	volatile unsigned int Dword9;
	volatile unsigned int Dword10;  //NextTxDescAddress;
	volatile unsigned int Dword11;

	// 2008/05/15 MH Because PCIE HW memory R/W 4K limit. And now,  our descriptor
	// size is 40 bytes. If you use more than 102 descriptor( 103*40>4096), HW will execute
	// memoryR/W CRC error. And then all DMA fetch will fail. We must decrease descriptor
	// number or enlarge descriptor size as 64 bytes.
	unsigned int		Reserve_Pass_92S_PCIE_MM_Limit[4];
//#endif

};

struct tx_desc_info {
	unsigned int		type;
	unsigned int		paddr;
	void				*pframe;
	unsigned int		len;
	struct stat_info	*pstat;
	unsigned int		rate;
	struct rtl8192cd_priv	*priv;
};

typedef struct _BB_REGISTER_DEFINITION {
	unsigned int rfintfs; 			// set software control:
									//		0x870~0x877[8 bytes]
	unsigned int rfintfi; 			// readback data:
									//		0x8e0~0x8e7[8 bytes]
	unsigned int rfintfo; 			// output data:
									//		0x860~0x86f [16 bytes]
	unsigned int rfintfe; 			// output enable:
									//		0x860~0x86f [16 bytes]
	unsigned int rf3wireOffset; 	// LSSI data:
									//		0x840~0x84f [16 bytes]
	unsigned int rfLSSI_Select; 	// BB Band Select:
									//		0x878~0x87f [8 bytes]
	unsigned int rfTxGainStage;		// Tx gain stage:
									//		0x80c~0x80f [4 bytes]
	unsigned int rfHSSIPara1;		// wire parameter control1 :
									//		0x820~0x823,0x828~0x82b, 0x830~0x833, 0x838~0x83b [16 bytes]
	unsigned int rfHSSIPara2;		// wire parameter control2 :
									//		0x824~0x827,0x82c~0x82f, 0x834~0x837, 0x83c~0x83f [16 bytes]
	unsigned int rfSwitchControl; 	//Tx Rx antenna control :
									//		0x858~0x85f [16 bytes]
	unsigned int rfAGCControl1; 	//AGC parameter control1 :
									//		0xc50~0xc53,0xc58~0xc5b, 0xc60~0xc63, 0xc68~0xc6b [16 bytes]
	unsigned int rfAGCControl2; 	//AGC parameter control2 :
									//		0xc54~0xc57,0xc5c~0xc5f, 0xc64~0xc67, 0xc6c~0xc6f [16 bytes]
	unsigned int rfRxIQImbalance; 	//OFDM Rx IQ imbalance matrix :
									//		0xc14~0xc17,0xc1c~0xc1f, 0xc24~0xc27, 0xc2c~0xc2f [16 bytes]
	unsigned int rfRxAFE;  			//Rx IQ DC ofset and Rx digital filter, Rx DC notch filter :
									//		0xc10~0xc13,0xc18~0xc1b, 0xc20~0xc23, 0xc28~0xc2b [16 bytes]
	unsigned int rfTxIQImbalance; 	//OFDM Tx IQ imbalance matrix
									//		0xc80~0xc83,0xc88~0xc8b, 0xc90~0xc93, 0xc98~0xc9b [16 bytes]
	unsigned int rfTxAFE; 			//Tx IQ DC Offset and Tx DFIR type
									//		0xc84~0xc87,0xc8c~0xc8f, 0xc94~0xc97, 0xc9c~0xc9f [16 bytes]
	unsigned int rfLSSIReadBack; 	//LSSI RF readback data
									//		0x8a0~0x8af [16 bytes]
	unsigned int rfLSSIReadBackPi;	//LSSI RF readback data
									//		0x8b8~0x8bc [8 bytes]
}BB_REGISTER_DEFINITION_T, *PBB_REGISTER_DEFINITION_T;


#define DESC_DMA_SIZE	(NUM_RX_DESC *(sizeof(struct rx_desc))+\
												NUM_TX_DESC *(sizeof(struct tx_desc))*6 +\
												NUM_CMD_DESC *(sizeof(struct rx_desc)) + \
						 NUM_CMD_DESC *(sizeof(struct tx_desc))) +\
						 6 * (sizeof(struct tx_desc))

#ifndef __KERNEL__
#define DESC_DMA_PAGE_SIZE ((DESC_DMA_SIZE + (PAGE_SIZE - 1)))
#else
//#define DESC_DMA_PAGE_SIZE ((DESC_DMA_SIZE + (2*PAGE_SIZE - 1)) & (~(PAGE_SIZE - 1)))
#define DESC_DMA_PAGE_SIZE ((DESC_DMA_SIZE + PAGE_SIZE))
#endif

struct rtl8192cd_tx_desc_info {
	struct tx_desc_info	tx_info0[NUM_TX_DESC];
	struct tx_desc_info	tx_info1[NUM_TX_DESC];
	struct tx_desc_info	tx_info2[NUM_TX_DESC];
	struct tx_desc_info	tx_info3[NUM_TX_DESC];
	struct tx_desc_info	tx_info4[NUM_TX_DESC];
	struct tx_desc_info	tx_info5[NUM_TX_DESC];
};

struct rtl8192cd_hw {
	unsigned short	seq;	// sw seq

#ifdef WIFI_WMM
//	unsigned short	AC_seq[8];

// switch BE to VI
	unsigned int	VO_pkt_count;
	unsigned int	VI_pkt_count;
	unsigned int	BK_pkt_count;
#endif

#ifndef PRIV_STA_BUF
	unsigned long alloc_dma_buf;
#endif

	unsigned long	ring_dma_addr;
	unsigned long	ring_virt_addr;
	unsigned int	ring_buf_len;
	unsigned int	cur_rx;
#ifdef DELAY_REFILL_RX_BUF
	unsigned int	cur_rx_refill;
#endif
	struct	rx_desc			*rx_descL;
	unsigned long			rx_ring_addr;
	struct	rx_desc_info	rx_infoL[NUM_RX_DESC];

	/* For TX DMA Synchronization */
	unsigned long	rx_descL_dma_addr[NUM_RX_DESC];

	unsigned int	txhead0;
	unsigned int	txhead1;
	unsigned int	txhead2;
	unsigned int	txhead3;
	unsigned int	txhead4;
	unsigned int	txhead5;

	unsigned int	txtail0;
	unsigned int	txtail1;
	unsigned int	txtail2;
	unsigned int	txtail3;
	unsigned int	txtail4;
	unsigned int	txtail5;

	struct tx_desc	*tx_desc0;
	struct tx_desc	*tx_desc1;
	struct tx_desc	*tx_desc2;
	struct tx_desc	*tx_desc3;
	struct tx_desc	*tx_desc4;
	struct tx_desc	*tx_desc5;
	struct tx_desc	*tx_descB;

	/* For TX DMA Synchronization */
	unsigned long	tx_desc0_dma_addr[NUM_TX_DESC];
	unsigned long	tx_desc1_dma_addr[NUM_TX_DESC];
	unsigned long	tx_desc2_dma_addr[NUM_TX_DESC];
	unsigned long	tx_desc3_dma_addr[NUM_TX_DESC];
	unsigned long	tx_desc4_dma_addr[NUM_TX_DESC];
	unsigned long	tx_desc5_dma_addr[NUM_TX_DESC];
	unsigned long	tx_descB_dma_addr[NUM_TX_DESC];

	unsigned long	tx_ring0_addr;
	unsigned long	tx_ring1_addr;
	unsigned long	tx_ring2_addr;
	unsigned long	tx_ring3_addr;
	unsigned long	tx_ring4_addr;
	unsigned long	tx_ring5_addr;
	unsigned long	tx_ringB_addr;

	unsigned int		cur_rxcmd;
	struct rx_desc		*rxcmd_desc;
	unsigned long		rxcmd_ring_addr;
	struct rx_desc_info	rxcmd_info[NUM_CMD_DESC];
	unsigned long		rxcmd_desc_dma_addr[NUM_CMD_DESC];

	unsigned int		txcmdhead;
	unsigned int		txcmdtail;
	struct tx_desc		*txcmd_desc;
	unsigned long		txcmd_desc_dma_addr[NUM_CMD_DESC];
	unsigned long		txcmd_ring_addr;

	char				MCSTxAgcOffset_A[16];
	char				MCSTxAgcOffset_B[16];
	char				OFDMTxAgcOffset_A[8];
	char				OFDMTxAgcOffset_B[8];
	int					CCKTxAgc_A[4];
	int					CCKTxAgc_B[4];

	unsigned int				NumTotalRFPath;
	/*PHY related*/
	BB_REGISTER_DEFINITION_T	PHYRegDef[4];	//Radio A/B/C/D

	// Joseph test for shorten RF config.
	unsigned int				RfReg0Value[4];

	// for DIG checking
	unsigned char				signal_strength;	// 1=low and dig off, 2=normal and dig on, 3=high power and dig on

	// dynamic CCK Tx power by rssi
	unsigned char				CCKTxAgc_enhanced;

	// dynamic CCK CCA enhance by rssi
	unsigned char				CCK_CCA_enhanced;

	unsigned char				EDCCA_on;

	// for Multicast Rx dynamic mechanism
	unsigned char				rxmlcst_rssi;
	unsigned char				initial_gain;

	// MIMO TR hw support checking
	unsigned char				MIMO_TR_hw_support;

	// dynamic Rx path selection by signal strength
	unsigned char				ant_off_num;
	unsigned char				ant_off_bitmap;
	unsigned char				ant_on_criteria[4];
	unsigned char				ant_cck_sel;

	// Tx power control
	unsigned char				lower_tx_power;
	unsigned int				power_backup[15];
/*
	// Tx power tracking
#if defined(RTL8190) || defined(RTL8192E)
	unsigned char				tpt_inited;
	unsigned char				tpt_ofdm_swing_idx;
	unsigned char				tpt_cck_swing_idx;
	unsigned char				tpt_tssi_total;
	unsigned char				tpt_tssi_num;
	unsigned char				tpt_tssi_waiting;
	unsigned char				tpt_tracking_num;
#endif
*/
	struct timer_list			tpt_timer;
	struct rtl8192cd_tx_desc_info	tx_info;

	unsigned char				check_reg824;
	unsigned int				reg824_bit9;
};

//1-------------------------------------------------------------------
//1RTL_8192CD related definition
//1---------------------------------------------------------------------

//--------------------------------------------------------------------------------
// 8192S Firmware related
//--------------------------------------------------------------------------------
typedef struct __RTL8192C_FW_HDR__ {
    unsigned int signature:16;
    unsigned int category:8;
    unsigned int function:8;

    unsigned int version:16;
    unsigned int subversion:8;
    unsigned int rsvd1:8;

    unsigned int month:8;
    unsigned int day:8;
    unsigned int hour:8;
    unsigned int minute:8;

    unsigned int ram_code_size:16;
    unsigned int year:16;

    unsigned int svnidx;
    unsigned int rsvd5;
    unsigned int rsvd6;
    unsigned int rsvd7;
} RTL8192C_FW_HDR, *PRTL8192C_FW_HDR;

enum __RTL8192C_SIGNATURE__
{
    RTL8192C_TEST_CHIP = 0x92C0,
    RTL8188C_TEST_CHIP = 0x88C0,
    RTL8192C_MP_CHIP_A = 0x92C1,
    RTL8188C_MP_CHIP_A = 0x88C1,
    RTL8192C_MP_CHIP_B = 0x92C2,
    RTL8188C_MP_CHIP_B = 0x88C2
};

enum __RTL8192C_FW_CATEGORY__
{
    RTL8192C_NIC_PCIE = 0,
    RTL8192C_NIC_USB = 1,
    RTL8192C_AP_PCIE = 4,
    RTL8192C_AP_USB = 5
};

enum __RTL8192C_FW_FUNCTION__
{
    RTL8192C_NIC_NORMAL = 0,
    RTL8192C_NIC_WWLAN = 1,
    RTL8192C_AP_NORMAL = 2,
    RTL8192C_AP_SUSPEND = 3
};

#define	RT_8192CD_FIRMWARE_HDR_SIZE	32

enum _RTL8192CD_TX_DESC_ {
	// TX cmd desc
	// Dword 0
	TX_OWN				= BIT(31),
	TX_GF				= BIT(30),
	TX_NoACM			= BIT(29),
	TX_LINIP			= BIT(28),
	TX_FirstSeg			= BIT(27),
	TX_LastSeg			= BIT(26),
	TX_HTC				= BIT(25),
	TX_BMC				= BIT(24),
	TX_OffsetSHIFT		= 16,
	TX_OffsetMask		= 0x0FF,
	TX_PktSizeSHIFT		= 0,
	TX_PktSizeMask 		= 0x0FFFF,

//	TX_AMSDU			= BIT(29),
//	TX_TYPESHIFT		= 24,
//	TX_TYPEMask			= 0x3,


	// Dword 1
//	TX_PaddingSHIFT	= 24,
//	TX_PaddingMask		= 0x0FF,
	TX_PktOffsetSHIFT	= 26,
	TX_PktOffsetMask	= 0x01F,
	TX_SecTypeSHIFT	= 22,
	TX_SecTypeMask	= 0x03,
	TX_EnDescID		= BIT(21),
	TX_NAVUSEHDR		= BIT(20),
	TX_RateIDSHIFT		= 16 ,
	TX_RateIDMask		= 0x0F,
	TX_PIFS				= BIT(15),
	TX_LSigTxopEn		= BIT(14),
	TX_RdNavExt		= BIT(13),
	TX_QSelSHIFT		= 8,
	TX_QSelMask		= 0x01F,
	TX_RdEn			= BIT(7),
	TX_BK				= BIT(6),
	TX_AggEn			= BIT(5),
	TX_MacIdSHIFT		= 0,
	TX_MacIdMask		= 0x01F,
	
//	TX_MoreData			= BIT(5),
//	TX_MoreFrag			= BIT(6),
//	TX_AckPolicySHIFT	= 13,
//	TX_AckPolicyMask	= 0x3, // 2 bits
//	TX_NoACM			= BIT(15),
//	TX_NonQos			= BIT(16),
//	TX_KeyIDSHIFT		= 17 ,
//	TX_OUI 				= BIT(19),
//	TX_PktType			= BIT(20),
//	TX_HTC				= BIT(24),	//padding0
//	TX_WDS				= BIT(25),	//padding1
//	TX_HWPC				= BIT(31),


	// DWORD 2
	TX_TxAntHtSHIFT	= 30,
	TX_TxAntHtMask		= 0x03,
	TX_TxAntlSHIFT		= 28,
	TX_TxAntlMask		= 0x03,
	TX_TxAntCckSHIFT	= 26,
	TX_TxAntCckMask	= 0x03,
	TX_AntSelB			= BIT(25),
	TX_AntSelA			= BIT(24),
	TX_AmpduDstySHIFT	= 20,
	TX_AmpduDstyMask	= 0x07,
	TX_CCX				= BIT(19),
	TX_RAW				= BIT(18),
	TX_MoreFrag		= BIT(17),
	TX_BarRtyThSHIFT	= 14,
	TX_BarRtyThMask	= 0x03,
	TX_DataRcSHIFT		= 6,
	TX_DataRcMask		= 0x03F,
	TX_RtsRcSHIFT		= 0,
	TX_RtsRcMask		= 0x03F,

//	TX_DataRetryLmtSHIFT	= 0,
//	TX_DataRetryLmtMask	= 0x3F,	// 6 bits
//	TX_RetryLmtEn			= BIT(6),
//	TX_TSFLSHIFT			= 7,
//	TX_TSFLMask 			= 0x1f,
//	TX_RTSRCSHIFT			= 12,
//	TX_RTSRCMask			= 0x3F,	// Reserved for HW RTS Retry Count.
//	TX_DATARCSHIFT			= 18,
//	TX_DATARCMask			 = 0x3F ,	// Reserved for HW DATA Retry Count.
	//TX_Rsvd1:5;
//	TX_AggEn				=BIT(29),
//	TX_BK					= BIT(30),	//Aggregation break.
//	TX_OwnMAC				= BIT(31),


	//DWORD3
	TX_PktIdSHIFT			= 28,
	TX_PktIdMask			= 0x0F,
	TX_SeqSHIFT			= 16,
	TX_SeqMask				= 0x0FFF,
	TX_TailPageSHIFT		= 8,
	TX_TailPageMask		= 0x0FF,
	TX_NextHeadPageSHIFT	= 0,
	TX_NextHeadPageMask	= 0x0FF,

//	TX_FragSHIFT			= 28,
//	TX_FragMask				= 0xF,


	// DWORD 4
	TX_CtrlStbcSHIFT	= 30,
	TX_CtrlStbcMask		= 0x03,
	TX_RtsScSHIFT		= 28,
	TX_RtsScMask		= 0x3,
	TX_RtsBw			= BIT(27),
	TX_RtsShort			= BIT(26),
	TX_DataBw			= BIT(25),
	TX_DataShort		= BIT(24),
	TX_DataStbcSHIFT	= 22,
	TX_DataStbcMask	= 0x03,
	TX_DataScSHIFT		= 20,
	TX_DataScMask		= 0x03,
	TX_CTS2AP			= BIT(19),
	TX_WaitDCTS		= BIT(18),
	TX_HwRtsEn			= BIT(13),
	TX_RtsEn			= BIT(12),
	TX_CTS2Self			= BIT(11),
	TX_DisDataFB		= BIT(10),
	TX_DisRtsFB 		= BIT(9),
	TX_UseRate			= BIT(8),
	TX_HwSeqEn			= BIT(7),
	TX_QOS				= BIT(6),
	TX_ApDCFE			= BIT(5),
	TX_RtsRateSHIFT	= 0,
	TX_RtsRateMask		= 0x01F,

//	TX_RTSRateFBLmtSHIFT	= 7,
//	TX_RTSRateFBLmtMask	= 0xF,
//	TX_RaBRSRIDSHIFT		= 13,	//Rate adaptive BRSR ID.
//	TX_RaBRSRIDMask		= 0x7, // 3bits
//	TX_TXHT					= BIT(16),
//	TX_RD					= BIT(23),
//	TX_RTSHT				= BIT(24),
//	TX_RTSSTBCSHIFT		= 29,
//	TX_RTSSTBCMask		= 0x3,


	// DWORD 5
	TX_UsbAggNumSHIFT	= 24,
	TX_UsbAggNumMask		= 0x0FF,
	TX_DataRtyLmtSHIFT		= 18,
	TX_DataRtyLmtMask		= 0x03F,
	TX_RtyLmtEn			= BIT(17),
	TX_RtsRateFBLmtSHIFT	= 13,
	TX_RtsRateFBLmtMask	= 0x0F,
	TX_DataRateFBLmtSHIFT	= 8,
	TX_DataRateFBLmtMask	= 0x01F,
	TX_CcxTag				= BIT(7),
	TX_SGI					= BIT(6),
	TX_DataRateSHIFT		= 0,
	TX_DataRateMask		= 0x03F,

//	TX_PktIDSHIFT		= 0,
//	TX_PktIDMask		= 0x1FF,
//	TX_DISFB			= BIT(15),
//	TX_TxAGCSHIFT			= 21,
//	TX_TxAGCMask			= 0x7FF,


	// DWORD 6
	TX_MCS7SgiMaxSHIFT	= 28,
	TX_MCS7SgiMaxMask		= 0x0F,
	TX_MCS3gMaxSHIFT		= 24,
	TX_MCS3gMaxMask		= 0x0F,
	TX_MCS2gMaxSHIFT		= 20,
	TX_MCS2gMaxMask		= 0x0F,
	TX_MCS1gMaxSHIFT		= 16,
	TX_MCS1gMaxMask		= 0x0F,
	TX_MaxAggNumSHIFT	= 11,
	TX_MaxAggNumMask		= 0x01F,
	TX_UseMaxLen			= BIT(10),
	TX_TxAgcBSHIFT			= 5,
	TX_TxAgcBMask			= 0x01F,
	TX_TxAgcASHIFT		= 0,
	TX_TxAgcAMask			= 0x01F,

//	TX_IPChkSumSHIFT		=  0,
//	TX_IPChkSumMask		=  0xFFFF,
//	TX_TCPChkSumSHIFT		=	16,
//	TX_TCPChkSumMask		=	0xFFFF,


	// DWORD 7
	TX_MCS15SgiMaxSHIFT	= 28,
	TX_MCS15SgiMaxMask	= 0x0F,
	TX_MCS6gMaxSHIFT		= 24,
	TX_MCS6gMaxMask		= 0x0F,
	TX_MCS5gMaxSHIFT		= 20,
	TX_MCS5gMaxMask		= 0x0F,
	TX_MCS4gMaxSHIFT		= 16,
	TX_MCS4gMaxMask		= 0x0F,
	TX_TxBufSizeSHIFT		= 0,
	TX_TxBufSizeMask		= 0x0FFFF,

//	TX_IPHdrOffsetSHIFT	= 16,
//	TX_IPHdrOffsetMask		= 0xFF,
//	unsigned int		Rsvd2:7;
//	TX_TCPEn				= BIT(31),
};



enum _RTL8192CD_RX_DESC_ {
	// RX cmd desc
	// Dword 0
	RX_OWN					= BIT(31),
	RX_EOR					= BIT(30),
	RX_FirstSeg				= BIT(29),
	RX_LastSeg				= BIT(28),
	RX_SwDec				= BIT(27),
	RX_PHYST				= BIT(26),
	RX_ShiftSHIFT			= 24,
	RX_ShiftMask			= 0x03,
	RX_QOS					= BIT(23),
	RX_SecuritySHIFT		= 20,
	RX_SecurityMask			= 0x07,
	RX_DrvInfoSizeSHIFT	= 16,
	RX_DrvInfoSizeMask		= 0x0F,
	RX_ICVERR				= BIT(15),
	RX_CRC32				= BIT(14),
	RX_PktLenSHIFT			= 0,
	RX_PktLenMask 			= 0x03FFF,


	// Dword 1
	RX_BC				= BIT(31),
	RX_MC				= BIT(30),
	RX_TypeSHIFT		= 28,
	RX_TypeMask		= 0x03,
	RX_MoreFrag		= BIT(27),
	RX_MoreData		= BIT(26),
	RX_PWRMGT			= BIT(25),
	RX_PAM				= BIT(24),
	RX_A2FitSHIFT		= 20,
	RX_A2FitSHIFMask	= 0x0F,
	RX_A1FitSHIFT		= 16,
	RX_A1FitSHIFMask	= 0x0F,
	RX_FAGGR			= BIT(15),
	RX_PAGGR			= BIT(14),
	RX_AMSDU			= BIT(13),
	RX_HwRsvdSHIFT	= 9,
	RX_HwRsvdMask		= 0x0F,
	RX_TidSHIFT			= 5,
	RX_TidMask			= 0x0F,
	RX_MacIdSHIFT		= 0,
	RX_MacIdMask		= 0x01F,


	// DWORD 2
	RX_NextInd				= BIT(30),
	RX_NextPktLenSHIFT		= 16,
	RX_NextPktLenMask		= 0x03FFF,
	RX_FragSHIFT			= 12,
	RX_FragMask			= 0x0F,
	RX_SeqSHIFT			= 0,
	RX_SeqMask				= 0x0FFF,


	//DWORD3
	RX_IV0SHIFT		= 16,
	RX_IV0Mask			= 0x0FFFF,
	RX_HwPCInd			= BIT(15),
	RX_HwPCErr			= BIT(14),
	RX_TCPChkValid		= BIT(13),
	RX_IPChkRpt			= BIT(12),
	RX_TCPChkRpt		= BIT(11),
	RX_HTC				= BIT(10),
	RX_BW				= BIT(9),
	RX_SPLCP			= BIT(8),
	RX_GF				= BIT(7),
	RX_RxHT			= BIT(6),
	RX_RxMcsSHIFT		= 0,
	RX_RxMcsMask		= 0x03F,


	//DWORD8
	RX_BaVld			= BIT(11),
	RX_BaSsnSHIFT		= 0,
	RX_BaSsnMask		= 0x07FF,
};


/*------------------------------------------------------------------------------
	Below we define some useful readline functions...
------------------------------------------------------------------------------*/
static __inline__ struct sk_buff *get_skb_frlist(struct list_head *list,unsigned int offset)
{
	unsigned long pobj;

	pobj = ((unsigned long)list - offset);

	return	((struct rx_frinfo *)pobj)->pskb;
}

static __inline__ int get_txhead(struct rtl8192cd_hw *phw, int q_num)
{
	return *(int *)((unsigned int)&(phw->txhead0) + sizeof(int)*q_num);
}

static __inline__ int get_txtail(struct rtl8192cd_hw *phw, int q_num)
{
	return *(int *)((unsigned int)&(phw->txtail0) + sizeof(int)*q_num);
}

static __inline__ int *get_txhead_addr(struct rtl8192cd_hw *phw, int q_num)
{
	return (int *)((unsigned int)&(phw->txhead0) + sizeof(int)*q_num);
}

static __inline__ int *get_txtail_addr(struct rtl8192cd_hw *phw, int q_num)
{
	return (int *)((unsigned int)&(phw->txtail0) + sizeof(int)*q_num);
}

static __inline__ struct tx_desc *get_txdesc(struct rtl8192cd_hw *phw, int q_num)
{
	return (struct tx_desc *)(*(unsigned int *)((unsigned int)&(phw->tx_desc0) + sizeof(struct tx_desc *)*q_num));
}

static __inline__ struct tx_desc_info *get_txdesc_info(struct rtl8192cd_tx_desc_info*pdesc, int q_num)
{
	return (struct tx_desc_info *)((unsigned int)(pdesc->tx_info0) + sizeof(struct tx_desc_info)*q_num*NUM_TX_DESC);
}

static __inline__ unsigned int *get_txdma_addr(struct rtl8192cd_hw *phw, int q_num)
{
	return (unsigned int *)((unsigned int)&(phw->tx_desc0_dma_addr) + (sizeof(unsigned long)*q_num*NUM_TX_DESC));
}


#define RTL8192CD_REGS_SIZE	((0xff + 1) * 16)		//16 pages


//----------------------------------------------------------------------------
// 8192SE EFUSE
//----------------------------------------------------------------------------
#ifdef EN_EFUSE
#define		HWSET_MAX_SIZE_92C			128
#define		EFUSE_REAL_CONTENT_LEN		512
#define		EFUSE_MAP_LEN				128
#define		EFUSE_MAX_SECTION			16
#define		EFUSE_MAX_WORD_UNIT			4
#define		EFUSE_INIT_MAP				0
#define		EFUSE_MODIFY_MAP			1

#define		EFUSE_ERROE_HANDLE
#define		PG_STATE_HEADER 				0x01
#define		PG_STATE_DATA					0x20
#define		EFUSE_REPEAT_THRESHOLD_			3
#define  	PGPKT_DATA_SIZE  				8
#define		EFUSE_OOB_PROTECT_BYTES 	 	15

typedef struct PG_PKT_STRUCT_A{
	UINT8 offset;
	UINT8 word_en;
	UINT8 data[8];	
}PGPKT_STRUCT,*PPGPKT_STRUCT;

#endif


//#if defined(RTL8190) || defined(RTL8192E)

//#define LoadPktSize	1024

//#elif defined(RTL8192SE)

#define LoadPktSize 32000

//#endif

/*
typedef enum _HW90_BLOCK {
	HW90_BLOCK_MAC		= 0,
	HW90_BLOCK_PHY0		= 1,
	HW90_BLOCK_PHY1		= 2,
	HW90_BLOCK_RF		= 3,
	HW90_BLOCK_MAXIMUM	= 4, // Never use this
} HW90_BLOCK_E, *PHW90_BLOCK_E;
*/

typedef enum _RF92CD_RADIO_PATH_ {
	RF92CD_PATH_A = 0,			//Radio Path A
	RF92CD_PATH_B = 1,			//Radio Path B
	RF92CD_PATH_MAX				//Max RF number 92cd support
} RF92CD_RADIO_PATH_E, *PRF92CD_RADIO_PATH_E;
/*
typedef	enum _FW_LOAD_FILE {
	BOOT = 0,
	MAIN = 1,
	DATA = 2,
//#ifdef RTL8192SE
	LOAD_IMEM = 3,
	LOAD_EMEM = 4,
	LOAD_DMEM = 5,
//#endif
} FW_LOAD_FILE;
*/
typedef enum _PHY_REG_FILE {
	AGCTAB,
	PHYREG_1T2R,
	PHYREG_2T2R,
	PHYREG_1T1R,
	PHYREG_PG,
	PHYREG_MP
} PHY_REG_FILE;

enum REG_FILE_FORMAT {
	TWO_COLUMN,
	THREE_COLUMN
};

typedef enum _MIMO_TR_STATUS {
	MIMO_1T2R = 1,
//	MIMO_2T4R = 2,
	MIMO_2T2R = 3,
	MIMO_1T1R = 4
} MIMO_TR_STATUS;

struct MacRegTable {
	unsigned int	offset;
	unsigned int	mask;
	unsigned int	value;
};

struct PhyRegTable {
	unsigned int	offset;
	unsigned int	value;
};

#ifdef	_LITTLE_ENDIAN_
struct FWtemplate {
	unsigned char	txRate:7;
	unsigned char	ctsEn:1;
	unsigned char	rtsTxRate:7;
	unsigned char	rtsEn:1;
	unsigned char	txHt:1;   //txCtrl; // {MSB to LSB}
	unsigned char	txshort:1;
	unsigned char	txbw:1;
	unsigned char	txSC:2;
	unsigned char	txSTBC:2;
	unsigned char	aggren:1;
	unsigned char	rtsHt:1;
	unsigned char	rtsShort:1;
	unsigned char	rtsbw:1;
	unsigned char	rtsSC:2;
	unsigned char	rtsSTBC:2;
	unsigned char	enCPUDur:1;
	unsigned char	rxMF:2;
	unsigned char	rxAMD:3;
	unsigned char	ccx:1;
	unsigned char	rsvd0:2;
	unsigned char	txAGCOffset:4;
	unsigned char	txAGCSign:1;
	unsigned char	txRaw:1;
	unsigned char	retryLimit1:2;
	unsigned char	retryLimit2:2;
	unsigned char	rsvd1:6;
	unsigned char	rsvd2;
};

struct RxFWInfo {
	unsigned char	RSVD0;
	unsigned char	RSVD1:4;
	unsigned char	PAGGR:1;
	unsigned char	FAGGR:1;
	unsigned char	RSVD2:2;
	unsigned char	RxMCS:7;
	unsigned char	HT:1;
	unsigned char	BW:1;
	unsigned char	SPLCP:1;
	unsigned char	RSVD3:2;
	unsigned char	PAM:1;
	unsigned char	MC:1;
	unsigned char	BC:1;
	unsigned char	RxCmd:1;
	unsigned long	TSFL;
};

#else // _BIG_ENDIAN_

struct FWtemplate {
	unsigned char	ctsEn:1;
	unsigned char	txRate:7;
	unsigned char	rtsEn:1;
	unsigned char	rtsTxRate:7;
	unsigned char	aggren:1;
	unsigned char	txSTBC:2;
	unsigned char	txSC:2;
	unsigned char	txbw:1;
	unsigned char	txshort:1;
	unsigned char	txHt:1;   //txCtrl; // {MSB to LSB}
	unsigned char	enCPUDur:1;
	unsigned char	rtsSTBC:2;
	unsigned char	rtsSC:2;
	unsigned char	rtsbw:1;
	unsigned char	rtsShort:1;
	unsigned char	rtsHt:1;
	unsigned char	rsvd0:2;
	unsigned char	ccx:1;
	unsigned char	rxAMD:3;
	unsigned char	rxMF:2;
	unsigned char	retryLimit1:2;
	unsigned char	txRaw:1;
	unsigned char	txAGCSign:1;
	unsigned char	txAGCOffset:4;
	unsigned char	rsvd1:6;
	unsigned char	retryLimit2:2;
	unsigned char	rsvd2;
};

struct RxFWInfo {
	unsigned char	RSVD0;
	unsigned char	RSVD2:2;
	unsigned char	FAGGR:1;
	unsigned char	PAGGR:1;
	unsigned char	RSVD1:4;
	unsigned char	HT:1;
	unsigned char	RxMCS:7;
	unsigned char	RxCmd:1;
	unsigned char	BC:1;
	unsigned char	MC:1;
	unsigned char	PAM:1;
	unsigned char	RSVD3:2;
	unsigned char	SPLCP:1;
	unsigned char	BW:1;
	unsigned long	TSFL;
};
#endif

typedef struct _Phy_OFDM_Rx_Status_Report_8192cd
{
	unsigned char	trsw_gain_X[4];
	unsigned char	pwdb_all;
	unsigned char	cfosho_X[4];
	unsigned char	cfotail_X[4];
	unsigned char	rxevm_X[2];
	unsigned char	rxsnr_X[4];
	unsigned char	pdsnr_X[2];
	unsigned char	csi_current_X[2];
	unsigned char	csi_target_X[2];
	unsigned char	sigevm;
	unsigned char	max_ex_pwr;
//#ifdef RTL8192SE
#ifdef	_LITTLE_ENDIAN_
	unsigned char ex_intf_flg:1;
	unsigned char sgi_en:1;
	unsigned char rxsc:2;
	unsigned char rsvd:4;
#else	// _BIG_ENDIAN_
	unsigned char rsvd:4;
	unsigned char rxsc:2;
	unsigned char sgi_en:1;
	unsigned char ex_intf_flg:1;
#endif
//#else	// RTL8190, RTL8192E
//	unsigned char	sgi_en;
//	unsigned char	rxsc_sgien_exflg;
//#endif
} PHY_STS_OFDM_8192CD_T;

typedef struct _Phy_CCK_Rx_Status_Report_8192cd
{
	/* For CCK rate descriptor. This is a signed 8:1 variable. LSB bit presend
	   0.5. And MSB 7 bts presend a signed value. Range from -64~+63.5. */
	char			adc_pwdb_X[4];
	unsigned char	SQ_rpt;
	char			cck_agc_rpt;
} PHY_STS_CCK_8192CD_T;

/*
enum _8190_POLL_BITFIELD_ {
	POLL_BK		= BIT(0),
	POLL_BE		= BIT(1),
	POLL_VI		= BIT(2),
	POLL_VO		= BIT(3),
	POLL_BCN	= BIT(4),
	POLL_CMD	= BIT(5),
	POLL_MGT	= BIT(6),
	POLL_HIGH	= BIT(7),

	POLL_HCCA	= BIT(0),
	STOP_BK		= BIT(1),
	STOP_BE		= BIT(2),
	STOP_VI		= BIT(3),
	STOP_VO		= BIT(4),
	STOP_MGT	= BIT(5),
	STOP_HIGH	= BIT(6),
	STOP_HCCA	= BIT(7),
};

enum _8190_CPU_RESET_BITFIELD_ {
	CPURST_SysRst	= BIT(0),
	CPURST_RegRst	= BIT(1),
	CPURST_Pwron	= BIT(2),
	CPURST_FwRst	= BIT(3),
	CPURST_Brdy		= BIT(4),
	CPURST_FwRdy	= BIT(5),
	CPURST_BaseChg	= BIT(6),
	CPURST_PutCode	= BIT(7),
	CPURST_BBRst	= BIT(8),
	CPURST_EnUart	= BIT(14),
	CPURST_EnJtag	= BIT(15),
};
*/

//
// Firmware Queue Layout
//
#define	NUM_OF_FIRMWARE_QUEUE			10
#define NUM_OF_PAGES_IN_FW				0x100
#define NUM_OF_PAGE_IN_FW_QUEUE_BK		0x006
#define NUM_OF_PAGE_IN_FW_QUEUE_BE		0x024
#define NUM_OF_PAGE_IN_FW_QUEUE_VI		0x024
#define NUM_OF_PAGE_IN_FW_QUEUE_VO		0x006
#define NUM_OF_PAGE_IN_FW_QUEUE_HCCA	0
#define NUM_OF_PAGE_IN_FW_QUEUE_CMD		0x2
#define NUM_OF_PAGE_IN_FW_QUEUE_MGNT	0x1d
#define NUM_OF_PAGE_IN_FW_QUEUE_HIGH	0
#define NUM_OF_PAGE_IN_FW_QUEUE_BCN		0x4
#define NUM_OF_PAGE_IN_FW_QUEUE_PUB		0x88
#define APPLIED_RESERVED_QUEUE_IN_FW	0x80000000
#define RSVD_FW_QUEUE_PAGE_BK_SHIFT		0x00
#define RSVD_FW_QUEUE_PAGE_BE_SHIFT		0x08
#define RSVD_FW_QUEUE_PAGE_VI_SHIFT		0x10
#define RSVD_FW_QUEUE_PAGE_VO_SHIFT		0x18
#define RSVD_FW_QUEUE_PAGE_MGNT_SHIFT	0x10
#define RSVD_FW_QUEUE_PAGE_BCN_SHIFT	0x00
#define RSVD_FW_QUEUE_PAGE_PUB_SHIFT	0x08

// Tx power tracking
#define TxPwrTrk_OFDM_SwingTbl_Len		37
#define TxPwrTrk_CCK_SwingTbl_Len		23
#define TxPwrTrk_E_Val					3


//
// For 8651C H/W MIC engine
//
#ifndef _ASICREGS_H
#define SYSTEM_BASE	(0xB8000000)
/* Generic DMA */
#define GDMA_BASE   (SYSTEM_BASE+0xA000)	/* 0xB800A000 */
#define GDMACNR		(GDMA_BASE+0x00)	/* Generic DMA Control Register */
#define GDMAIMR		(GDMA_BASE+0x04)	/* Generic DMA Interrupt Mask Register */
#define GDMAISR		(GDMA_BASE+0x08)	/* Generic DMA Interrupt Status Register */
#define GDMAICVL	(GDMA_BASE+0x0C)	/* Generic DMA Initial Checksum Value (Left Part) Register */
#define GDMAICVR	(GDMA_BASE+0x10)	/* Generic DMA Initial Checksum Value (Right Part) Register */
#define GDMASBP0	(GDMA_BASE+0x20)	/* Generic DMA Source Block Pointer 0 Register */
#define GDMASBL0	(GDMA_BASE+0x24)	/* Generic DMA Source Block Length 0 Register */
#define GDMASBP1	(GDMA_BASE+0x28)	/* Generic DMA Source Block Pointer 1 Register */
#define GDMASBL1	(GDMA_BASE+0x2C)	/* Generic DMA Source Block Length 1 Register */
#define GDMASBP2	(GDMA_BASE+0x30)	/* Generic DMA Source Block Pointer 2 Register */
#define GDMASBL2	(GDMA_BASE+0x34)	/* Generic DMA Source Block Length 2 Register */
#define GDMASBP3	(GDMA_BASE+0x38)	/* Generic DMA Source Block Pointer 3 Register */
#define GDMASBL3	(GDMA_BASE+0x3C)	/* Generic DMA Source Block Length 3 Register */
#define GDMASBP4	(GDMA_BASE+0x40)	/* Generic DMA Source Block Pointer 4 Register */
#define GDMASBL4	(GDMA_BASE+0x44)	/* Generic DMA Source Block Length 4 Register */
#define GDMASBP5	(GDMA_BASE+0x48)	/* Generic DMA Source Block Pointer 5 Register */
#define GDMASBL5	(GDMA_BASE+0x4C)	/* Generic DMA Source Block Length 5 Register */
#define GDMASBP6	(GDMA_BASE+0x50)	/* Generic DMA Source Block Pointer 6 Register */
#define GDMASBL6	(GDMA_BASE+0x54)	/* Generic DMA Source Block Length 6 Register */
#define GDMASBP7	(GDMA_BASE+0x58)	/* Generic DMA Source Block Pointer 7 Register */
#define GDMASBL7	(GDMA_BASE+0x5C)	/* Generic DMA Source Block Length 7 Register */
#define GDMADBP0	(GDMA_BASE+0x60)	/* Generic DMA Destination Block Pointer 0 Register */
#define GDMADBL0	(GDMA_BASE+0x64)	/* Generic DMA Destination Block Length 0 Register */
#define GDMADBP1	(GDMA_BASE+0x68)	/* Generic DMA Destination Block Pointer 1 Register */
#define GDMADBL1	(GDMA_BASE+0x6C)	/* Generic DMA Destination Block Length 1 Register */
#define GDMADBP2	(GDMA_BASE+0x70)	/* Generic DMA Destination Block Pointer 2 Register */
#define GDMADBL2	(GDMA_BASE+0x74)	/* Generic DMA Destination Block Length 2 Register */
#define GDMADBP3	(GDMA_BASE+0x78)	/* Generic DMA Destination Block Pointer 3 Register */
#define GDMADBL3	(GDMA_BASE+0x7C)	/* Generic DMA Destination Block Length 3 Register */
#define GDMADBP4	(GDMA_BASE+0x80)	/* Generic DMA Destination Block Pointer 4 Register */
#define GDMADBL4	(GDMA_BASE+0x84)	/* Generic DMA Destination Block Length 4 Register */
#define GDMADBP5	(GDMA_BASE+0x88)	/* Generic DMA Destination Block Pointer 5 Register */
#define GDMADBL5	(GDMA_BASE+0x8C)	/* Generic DMA Destination Block Length 5 Register */
#define GDMADBP6	(GDMA_BASE+0x90)	/* Generic DMA Destination Block Pointer 6 Register */
#define GDMADBL6	(GDMA_BASE+0x94)	/* Generic DMA Destination Block Length 6 Register */
#define GDMADBP7	(GDMA_BASE+0x98)	/* Generic DMA Destination Block Pointer 7 Register */
#define GDMADBL7	(GDMA_BASE+0x9C)	/* Generic DMA Destination Block Length 7 Register */

/* Generic DMA Control Register */
#define GDMA_ENABLE			(1<<31)		/* Enable GDMA */
#define GDMA_POLL			(1<<30)		/* Kick off GDMA */
#define GDMA_FUNCMASK		(0xf<<24)	/* GDMA Function Mask */
#define GDMA_MEMCPY			(0x0<<24)	/* Memory Copy */
#define GDMA_CHKOFF			(0x1<<24)	/* Checksum Offload */
#define GDMA_STCAM			(0x2<<24)	/* Sequential T-CAM */
#define GDMA_MEMSET			(0x3<<24)	/* Memory Set */
#define GDMA_B64ENC			(0x4<<24)	/* Base 64 Encode */
#define GDMA_B64DEC			(0x5<<24)	/* Base 64 Decode */
#define GDMA_QPENC			(0x6<<24)	/* Quoted Printable Encode */
#define GDMA_QPDEC			(0x7<<24)	/* Quoted Printable Decode */
#define GDMA_MIC			(0x8<<24)	/* Wireless MIC */
#define GDMA_MEMXOR			(0x9<<24)	/* Memory XOR */
#define GDMA_MEMCMP			(0xa<<24)	/* Memory Compare */
#define GDMA_BYTESWAP		(0xb<<24)	/* Byte Swap */
#define GDMA_PATTERN		(0xc<<24)	/* Pattern Match */
#define GDMA_SWAPTYPE0		(0<<22)		/* Original:{0,1,2,3} => {1,0,3,2} */
#define GDMA_SWAPTYPE1		(1<<22)		/* Original:{0,1,2,3} => {3,2,1,0} */
#define GDMA_ENTSIZMASK		(3<<20)		/* T-CAM Entry Size Mask */
#define GDMA_ENTSIZ32		(0<<20)		/* T-CAM Entry Size 32 bits */
#define GDMA_ENTSIZ64		(1<<20)		/* T-CAM Entry Size 64 bits */
#define GDMA_ENTSIZ128		(2<<20)		/* T-CAM Entry Size 128 bits */
#define GDMA_ENTSIZ256		(3<<20)		/* T-CAM Entry Size 256 bits */

/* Generic DMA Interrupt Mask Register */
#define GDMA_COMPIE			(1<<31)		/* Completed Interrupt Enable */
#define GDMA_NEEDCPUIE		(1<<28)		/* Need-CPU Interrupt Enable */

/* Generic DMA Interrupt Status Register */
#define GDMA_COMPIP			(1<<31)		/* Completed Interrupt Status (write 1 to clear) */
#define GDMA_NEEDCPUIP		(1<<28)		/* Need-CPU Interrupt Status (write 1 to clear) */

/* Generic DMA Source Block Length n. Register */
#define GDMA_LDB			(1<<31)		/* Last Data Block */
#define GDMA_BLKLENMASK		(0x1fff)	/* Block Length (valid value: from 1 to 8K-1 bytes) */

/*
 *	Some bits in GDMACNR are only for internal used.
 *	However, driver needs to configure them.
 *
 *	burstSize[7:6] -- 00:4W, 01:8W, 10:16W, 11:32W.
 *	enough[5:4]    -- 00:>16, 01:>10, 10:>4, 00:>0
 *	dlow[3:2]      -- 00:>24, 01:>20. 10:>16, 11:>8
 *	slow[1:0]      -- 00:>24, 01:>20. 10:>16, 11:>8
 */
#define internalUsedGDMACNR (0x000000C0)
#endif // _ASICREGS_H


#endif // _8192CD_HW_H_

