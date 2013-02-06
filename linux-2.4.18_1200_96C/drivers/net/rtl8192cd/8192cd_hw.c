/*
 *  Routines to access hardware
 *
 *  $Id: 8192cd_hw.c,v 1.107.2.60 2011/05/16 13:17:15 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_HW_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#endif

#ifdef USE_RTL8186_SDK
#ifdef CONFIG_RTL8672
	#include <platform.h>
#else
#if defined(__LINUX_2_6__)
#include <bsp/bspchip.h>
#else
	#include <asm/rtl865x/platform.h>
#endif
#endif

#if defined(__LINUX_2_6__) && !defined(CONFIG_RTL8672)
#define _WDTCNR_			BSP_WDTCNR
#else
#define _WDTCNR_			WDTCNR
#endif
#endif

#ifndef REG32
	#define REG32(reg)	 		(*(volatile unsigned int *)(reg))
#endif

#define MAX_CONFIG_FILE_SIZE (20*1024) // for 8192, added to 20k

int rtl8192cd_fileopen(const char *filename, int flags, int mode);
void selectMinPowerIdex(struct rtl8192cd_priv *priv);
void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
static int LoadFirmware(struct rtl8192cd_priv *priv);
static void rtl8192cd_ReadFwHdr(struct rtl8192cd_priv * priv);


unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len] = {
	/*  +6.0dB */ 0x7f8001fe,
	/*  +5.5dB */ 0x788001e2,
	/*  +5.0dB */ 0x71c001c7,
	/*  +4.5dB */ 0x6b8001ae,
	/*  +4.0dB */ 0x65400195,
	/*  +3.5dB */ 0x5fc0017f,
	/*  +3.0dB */ 0x5a400169,
	/*  +2.5dB */ 0x55400155,
	/*  +2.0dB */ 0x50800142,
	/*  +1.5dB */ 0x4c000130,
	/*  +1.0dB */ 0x47c0011f,
	/*  +0.5dB */ 0x43c0010f,
	/*   0.0dB */ 0x40000100,
	/*  -0.5dB */ 0x3c8000f2,
	/*  -1.0dB */ 0x390000e4,
	/*  -1.5dB */ 0x35c000d7,
	/*  -2.0dB */ 0x32c000cb,
	/*  -2.5dB */ 0x300000c0,
	/*  -3.0dB */ 0x2d4000b5,
	/*  -3.5dB */ 0x2ac000ab,
	/*  -4.0dB */ 0x288000a2,
	/*  -4.5dB */ 0x26000098,
	/*  -5.0dB */ 0x24000090,
	/*  -5.5dB */ 0x22000088,
	/*  -6.0dB */ 0x20000080,
	/*  -6.5dB */ 0x1a00006c,
	/*  -7.0dB */ 0x1c800072,
	/*  -7.5dB */ 0x18000060,
	/*  -8.0dB */ 0x19800066,
	/*  -8.5dB */ 0x15800056,
	/*  -9.0dB */ 0x26c0005b,
	/*  -9.5dB */ 0x14400051,
	/* -10.0dB */ 0x24400051,
	/* -10.5dB */ 0x1300004c,
	/* -11.0dB */ 0x12000048,
	/* -11.5dB */ 0x11000044,
	/* -12.0dB */ 0x10000040
};

unsigned char TxPwrTrk_CCK_SwingTbl[TxPwrTrk_CCK_SwingTbl_Len][8] = {
	/*   0.0dB */ {0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},
	/*   0.5dB */ {0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},
	/*   1.0dB */ {0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},
	/*   1.5dB */ {0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},
	/*   2.0dB */ {0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},
	/*   2.5dB */ {0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},
	/*   3.0dB */ {0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},
	/*   3.5dB */ {0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},
	/*   4.0dB */ {0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},
	/*   4.5dB */ {0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},
	/*   5.0dB */ {0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},
	/*   5.5dB */ {0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},
	/*   6.0dB */ {0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},
	/*   6.5dB */ {0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},
	/*   7.0dB */ {0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},
	/*   7.5dB */ {0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},
	/*   8.0dB */ {0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},
	/*   8.5dB */ {0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},
	/*   9.0dB */ {0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},
	/*   9.5dB */ {0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	/*  10.0dB */ {0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	/*  10.5dB */ {0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},
	/*  11.0dB */ {0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01}
};

unsigned char TxPwrTrk_CCK_SwingTbl_CH14[TxPwrTrk_CCK_SwingTbl_Len][8] = {
	/*   0.0dB */ {0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},
	/*   0.5dB */ {0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},
	/*   1.0dB */ {0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},
	/*   1.5dB */ {0x2d, 0x2d, 0x27, 0x17, 0x00, 0x00, 0x00, 0x00},
	/*   2.0dB */ {0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},
	/*   2.5dB */ {0x28, 0x28, 0x22, 0x14, 0x00, 0x00, 0x00, 0x00},
	/*   3.0dB */ {0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},
	/*   3.5dB */ {0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},
	/*   4.0dB */ {0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},
	/*   4.5dB */ {0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},
	/*   5.0dB */ {0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},
	/*   5.5dB */ {0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},
	/*   6.0dB */ {0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},
	/*   6.5dB */ {0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},
	/*   7.0dB */ {0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},
	/*   7.5dB */ {0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},
	/*   8.0dB */ {0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},
	/*   8.5dB */ {0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},
	/*   9.0dB */ {0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},
	/*   9.5dB */ {0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	/*  10.0dB */ {0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	/*  10.5dB */ {0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},
	/*  11.0dB */ {0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00}
};

#ifdef ADD_TX_POWER_BY_CMD
#define ASSIGN_TX_POWER_OFFSET(offset, setting) { \
	if (setting != 0x7f) \
		offset = setting; \
}
#endif

#define POWER_MIN_CHECK(a,b)            (((a) > (b)) ? (b) : (a))
#define POWER_RANGE_CHECK(val)		(((val) > 0x3f)? 0x3f : ((val < 0) ? 0 : val))
#define COUNT_SIGN_OFFSET(val, oft)	(((oft & 0x08) == 0x08)? (val - (0x10 - oft)) : (val + oft))

#ifdef HW_ANT_SWITCH
#define RXDVY_A_EN		((priv->pshare->rf_ft_var.antHw_enable && !priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#define RXDVY_B_EN		((priv->pshare->rf_ft_var.antHw_enable &&  priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#endif
#ifdef MERGE_FW
#ifdef TESTCHIP_SUPPORT
#include "data_AGC_TAB.c"
#include "data_PHY_REG_1T.c"
#include "data_PHY_REG_2T.c"
#include "data_radio_a_1T.c"
#include "data_radio_a_2T.c"
#include "data_radio_b_2T.c"
#include "data_rtl8192cfw.c"
#endif

#include "data_AGC_TAB_n.c"
#include "data_PHY_REG_1T_n.c"
#include "data_PHY_REG_2T_n.c"
#include "data_radio_a_2T_n.c"
#include "data_radio_b_2T_n.c"
#include "data_radio_a_1T_n.c"
#include "data_rtl8192cfwn.c"
#include "data_rtl8192cfwua.c"


#include "data_PHY_REG_PG.c"
#include "data_MACPHY_REG.c"
#include "data_PHY_REG_MP_n.c"

#include "data_AGC_TAB_n_hp.c"
#include "data_PHY_REG_2T_n_hp.c"
#include "data_radio_a_2T_n_lna.c"
#include "data_radio_b_2T_n_lna.c"
#include "data_PHY_REG_1T_n_hp.c"

#ifdef HIGH_POWER_EXT_PA
#include "data_radio_a_2T_n_hp.c"
#include "data_radio_b_2T_n_hp.c"
#include "data_PHY_REG_PG_hp.c"
#endif


#define VAR_MAPPING(dst,src) \
	unsigned char *data_##dst##_start = &data_##src[0]; \
	unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \

#ifdef TESTCHIP_SUPPORT
VAR_MAPPING(AGC_TAB, AGC_TAB);
VAR_MAPPING(PHY_REG_1T, PHY_REG_1T);
VAR_MAPPING(PHY_REG_2T, PHY_REG_2T);
VAR_MAPPING(radio_a_1T, radio_a_1T);
VAR_MAPPING(radio_a_2T, radio_a_2T);
VAR_MAPPING(radio_b_2T, radio_b_2T);
VAR_MAPPING(rtl8192cfw, rtl8192cfw);
#endif

VAR_MAPPING(AGC_TAB_n, AGC_TAB_n);
VAR_MAPPING(PHY_REG_1T_n, PHY_REG_1T_n);
VAR_MAPPING(PHY_REG_2T_n, PHY_REG_2T_n);
VAR_MAPPING(radio_a_1T_n, radio_a_1T_n);
VAR_MAPPING(radio_a_2T_n, radio_a_2T_n);
VAR_MAPPING(radio_b_2T_n, radio_b_2T_n);
VAR_MAPPING(rtl8192cfw_n, rtl8192cfwn);
VAR_MAPPING(rtl8192cfw_ua, rtl8192cfwua);


VAR_MAPPING(MACPHY_REG, MACPHY_REG);
VAR_MAPPING(PHY_REG_PG, PHY_REG_PG);
VAR_MAPPING(PHY_REG_MP_n, PHY_REG_MP_n);

VAR_MAPPING(AGC_TAB_n_hp, AGC_TAB_n_hp);
VAR_MAPPING(PHY_REG_2T_n_hp, PHY_REG_2T_n_hp);
VAR_MAPPING(PHY_REG_1T_n_hp, PHY_REG_1T_n_hp);
VAR_MAPPING(radio_a_2T_n_lna, radio_a_2T_n_lna);
VAR_MAPPING(radio_b_2T_n_lna, radio_b_2T_n_lna);

#ifdef HIGH_POWER_EXT_PA
VAR_MAPPING(radio_a_2T_n_hp, radio_a_2T_n_hp);
VAR_MAPPING(radio_b_2T_n_hp, radio_b_2T_n_hp);
VAR_MAPPING(PHY_REG_PG_hp, PHY_REG_PG_hp);

#endif
#endif


/*-----------------------------------------------------------------------------
 * Function:	PHYCheckIsLegalRfPath8192cPci()
 *
 * Overview:	Check different RF type to execute legal judgement. If RF Path is illegal
 *			We will return false.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	11/15/2007	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath)
{
	unsigned int rtValue = TRUE;

	if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
		if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
			rtValue = TRUE;
		else
			rtValue = FALSE;
	}
	else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
		if (eRFPath == RF92CD_PATH_A)
			rtValue = TRUE;
		else
			rtValue = FALSE;
	}
	else {
		rtValue = FALSE;
	}

	return rtValue;
}


/**
* Function:	phy_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			u4Byte		BitMask,
*
* Output:	none
* Return:		u4Byte		Return the shift bit bit position of the mask
*/
unsigned int phy_CalculateBitShift(unsigned int BitMask)
{
	unsigned int i;

	for (i=0; i<=31; i++) {
		if (((BitMask>>i) & 0x1) == 1)
			break;
	}

	return (i);
}


/**
* Function:	PHY_QueryBBReg
*
* OverView:	Read "sepcific bits" from BB register
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be readback
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be readback
* Output:	None
* Return:		u4Byte			Data			//The readback register value
* Note:		This function is equal to "GetRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask)
{
  	unsigned int ReturnValue = 0, OriginalValue, BitShift;

	OriginalValue = RTL_R32(RegAddr);
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	return (ReturnValue);
}


/**
* Function:	PHY_SetBBReg
*
* OverView:	Write "Specific bits" to BB register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register value in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRegSetting" in PHY programming guide
*/
void PHY_SetBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	unsigned int OriginalValue, BitShift, NewValue;

	if (BitMask != bMaskDWord)
	{//if not "double word" write
		OriginalValue = RTL_R32(RegAddr);
		BitShift = phy_CalculateBitShift(BitMask);
		NewValue = ((OriginalValue & (~BitMask)) | (Data << BitShift));
		RTL_W32(RegAddr, NewValue);
	}
	else
		RTL_W32(RegAddr, Data);

	return;
}


/**
* Function:	phy_RFSerialWrite
*
* OverView:	Write data to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target to be read
*
* Output:	None
* Return:		None
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
 *
 * Note: 		  For RF8256 only
 *			 The total count of RTL8256(Zebra4) register is around 36 bit it only employs
 *			 4-bit RF address. RTL8256 uses "register mode control bit" (Reg00[12], Reg00[10])
 *			 to access register address bigger than 0xf. See "Appendix-4 in PHY Configuration
 *			 programming guide" for more details.
 *			 Thus, we define a sub-finction for RTL8526 register address conversion
 *		       ===========================================================
 *			 Register Mode		RegCTL[1]		RegCTL[0]		Note
 *								(Reg00[12])		(Reg00[10])
 *		       ===========================================================
 *			 Reg_Mode0				0				x			Reg 0 ~15(0x0 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode1				1				0			Reg 16 ~30(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode2				1				1			Reg 31 ~ 45(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
*/
void phy_RFSerialWrite(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int				DataAndAddr = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;
	Offset &= 0x3f;

	//
	// Switch page for 8256 RF IC
	//
		NewOffset = Offset;

	//
	// Put write addr in [5:0]  and write data in [31:16]
	//
	//DataAndAddr = (Data<<16) | (NewOffset&0x3f);
	DataAndAddr = ((NewOffset<<20) | (Data&0x000fffff)) & 0x0fffffff;	// T65 RF

	//
	// Write Operation
	//
	PHY_SetBBReg(priv, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);
}


/**
* Function:	phy_RFSerialRead
*
* OverView:	Read regster from RF chips
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			dbg_avoid,	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			reback value
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
*/
unsigned int phy_RFSerialRead(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int dbg_avoid)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int 				tmplong, tmplong2;
	unsigned int				retValue = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

	//
	// Make sure RF register offset is correct
	//
	Offset &= 0x3f;

	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;


	// For 92S LSSI Read RFLSSIRead
	// For RF A/B write 0x824/82c(does not work in the future)
	// We must use 0x824 for RF A and B to execute read trigger
	tmplong = RTL_R32(rFPGA0_XA_HSSIParameter2);
	tmplong2 = RTL_R32(pPhyReg->rfHSSIPara2);
	tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | ((NewOffset<<23) | bLSSIReadEdge);	//T65 RF

	RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong&(~bLSSIReadEdge));
	delay_us(20);
	RTL_W32(pPhyReg->rfHSSIPara2,tmplong2);
	delay_us(20);
	RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong|bLSSIReadEdge);
	delay_us(20);
	RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong&(~bLSSIReadEdge));

	//Read from BBreg8a0, 12 bits for 8190, 20 bits for T65 RF
	if (((eRFPath == RF92CD_PATH_A) && (RTL_R32(0x820)&BIT(8)))
		|| ((eRFPath == RF92CD_PATH_B) && (RTL_R32(0x828)&BIT(8))))
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBackPi, bLSSIReadBackData);
	else
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBack, bLSSIReadBackData);

	return retValue;
}


/**
* Function:	PHY_QueryRFReg
*
* OverView:	Query "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be read
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be read
*			u4Byte			dbg_avoid	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			Readback value
* Note:		This function is equal to "GetRFRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	unsigned int	Original_Value, Readback_Value, BitShift;

	Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, dbg_avoid);
   	BitShift =  phy_CalculateBitShift(BitMask);
   	Readback_Value = (Original_Value & BitMask) >> BitShift;

	return (Readback_Value);
}


/**
* Function:	PHY_SetRFReg
*
* OverView:	Write "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRFRegSetting" in PHY programming guide
*/
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data)
{
	unsigned int Original_Value, BitShift, New_Value;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	if (BitMask != bMask20Bits)
	{
		Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, 1);
		BitShift = phy_CalculateBitShift(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));

		phy_RFSerialWrite(priv, eRFPath, RegAddr, New_Value);
	}
	else
		phy_RFSerialWrite(priv, eRFPath, RegAddr, Data);

	RESTORE_INT(flags);
}


static int is_hex(char s)
{
	if (( s >= '0' && s <= '9') || ( s >= 'a' && s <= 'f') || (s >= 'A' && s <= 'F') || (s == 'x' || s == 'X'))
		return 1;
	else
		return 0;
}


static unsigned char *get_digit(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;

	while(buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	*data = &buf[i];

	while(buf[i]) {
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (!is_hex(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		return &buf[i+1];
}


static int get_offset_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bRegValue)
{
	unsigned char *next;
	int base, idx;
	int num=0;
	unsigned char *ch;
	extern int _atoi(char *s, int base);

	*u4bRegOffset = *u4bRegValue = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	}
	else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi(&line_head[idx], base);

	if (next) {
		if (!get_digit(&next))
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		*u4bRegValue = _atoi(&next[idx], base);
	}
	else
		*u4bRegValue = 0;

	return num;
}


static int get_offset_mask_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	unsigned char *next, *n1;
	int base, idx;
	int num=0;
	unsigned char *ch;
	extern int _atoi(char *s, int base);

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	}
	else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi(&line_head[idx], base);

	if (next) {
		n1 = get_digit(&next);
		if (n1 == NULL)
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		*u4bMask = _atoi(&next[idx], base);

		if (n1) {
			if (!get_digit(&n1))
				return num;
			num++;
			if ((!memcmp(n1, "0x", 2)) || (!memcmp(n1, "0X", 2))) {
				base = 16;
				idx = 2;
			}
			else {
				base = 10;
				idx = 0;
			}
			*u4bRegValue = _atoi(&n1[idx], base);
		}
		else
			*u4bRegValue = 0;
	}
	else
		*u4bMask = 0;

	return num;
}


unsigned char *get_line(unsigned char **line)
{
	unsigned char *p=*line;

	while (*p && ((*p == '\n') || (*p == '\r')))
		p++;

	if (*p == '\0') {
		*line = NULL;
		return NULL;
	}
	*line = p;

	while (*p && (*p != '\n') && (*p != '\r'))
			p++;

	*p = '\0';
	return p+1;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigBBWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:      	PADAPTER		Adapter
 *			ps1Byte 			pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 *---------------------------------------------------------------------------*/
static int PHY_ConfigBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file)
{
	int                read_bytes, num, len=0;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask;
	unsigned char      *mem_ptr, *line_head, *next_head;
	struct PhyRegTable *phyreg_table=NULL;
	struct MacRegTable *macreg_table=NULL;
	unsigned short     max_len=0;
	int                file_format=TWO_COLUMN;
#ifndef MERGE_FW
	int                fd=0;
	mm_segment_t       old_fs;
	unsigned char      *pFileName=NULL;
	extern ssize_t     sys_read(unsigned int fd, char * buf, size_t count);
#endif

#ifdef MERGE_FW
	if (reg_file == AGCTAB) {
		phyreg_table = (struct PhyRegTable *)priv->pshare->agc_tab_buf;
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv)) {
			next_head = data_AGC_TAB_start;
			read_bytes = (int)(data_AGC_TAB_end - data_AGC_TAB_start);

		} else
#endif
		if (priv->pshare->rf_ft_var.use_ext_lna
#ifdef HIGH_POWER_EXT_PA
			|| priv->pshare->rf_ft_var.use_ext_pa
#endif
		) {
			next_head = data_AGC_TAB_n_hp_start;
			read_bytes = (int)(data_AGC_TAB_n_hp_end - data_AGC_TAB_n_hp_start);
		}
		else
		{
			next_head = data_AGC_TAB_n_start;
			read_bytes = (int)(data_AGC_TAB_n_end - data_AGC_TAB_n_start);
		}
		max_len = AGC_TAB_SIZE;
	}
	else if (reg_file == PHYREG_PG) {
#ifdef HIGH_POWER_EXT_PA
		if( priv->pshare->rf_ft_var.use_ext_pa) {
			next_head = data_PHY_REG_PG_hp_start;
			read_bytes = (int)(data_PHY_REG_PG_hp_end - data_PHY_REG_PG_hp_start);

		} else
#endif
		{						
			next_head = data_PHY_REG_PG_start;
			read_bytes = (int)(data_PHY_REG_PG_end - data_PHY_REG_PG_start);
		}
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
	}
#if 0
	else if (reg_file == PHYREG_1T2R) {
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_2to1;
		max_len = PHY_REG_1T2R;
		file_format = THREE_COLUMN;
		if (priv->pshare->rf_ft_var.pathB_1T == 0) { // PATH A
			next_head = __PHY_to1T2R_start;
			read_bytes = (int)(__PHY_to1T2R_end - __PHY_to1T2R_start);
		}
		else { // PATH B
			next_head = __PHY_to1T2R_b_start;
			read_bytes = (int)(__PHY_to1T2R_b_end - __PHY_to1T2R_b_start);
		}
	}
#endif

#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {
		phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
		next_head = data_PHY_REG_MP_n_start;
		read_bytes = (int)(data_PHY_REG_MP_n_end - data_PHY_REG_MP_n_start);
		max_len = PHY_REG_SIZE;
	}
#endif
	else if (reg_file == PHYREG_1T1R) { // PATH A
		phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv)) {
			next_head = data_PHY_REG_1T_start;
			read_bytes = (int)(data_PHY_REG_1T_end - data_PHY_REG_1T_start);
		} else
#endif
		{

			if (priv->pshare->rf_ft_var.use_ext_lna
#ifdef HIGH_POWER_EXT_PA
			|| priv->pshare->rf_ft_var.use_ext_pa
#endif
			) {
				next_head = data_PHY_REG_1T_n_hp_start;
				read_bytes = (int)(data_PHY_REG_1T_n_hp_end - data_PHY_REG_1T_n_hp_start);
			}
			else {
				next_head = data_PHY_REG_1T_n_start;
				read_bytes = (int)(data_PHY_REG_1T_n_end - data_PHY_REG_1T_n_start);
			}
		}

		max_len = PHY_REG_SIZE;
#if 0
		if(priv->pshare->rf_ft_var.pathB_1T == 0) {
			next_head = __PHY_to1T1R_start;
			read_bytes = (int)(__PHY_to1T1R_end - __PHY_to1T1R_start);
		}
		else { // PATH B
			next_head = __PHY_to1T1R_b_start;
			read_bytes = (int)(__PHY_to1T1R_b_end - __PHY_to1T1R_b_start);
		}
#endif
	}
	else {
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv))		{
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			next_head = data_PHY_REG_2T_start;
			read_bytes = (int)(data_PHY_REG_2T_end - data_PHY_REG_2T_start);
		} else
#endif
		{
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			if (priv->pshare->rf_ft_var.use_ext_lna
#ifdef HIGH_POWER_EXT_PA
			|| priv->pshare->rf_ft_var.use_ext_pa
#endif
			) {
				next_head = data_PHY_REG_2T_n_hp_start;
				read_bytes = (int)(data_PHY_REG_2T_n_hp_end - data_PHY_REG_2T_n_hp_start);
			}
			else
			{
				next_head = data_PHY_REG_2T_n_start;
				read_bytes = (int)(data_PHY_REG_2T_n_end - data_PHY_REG_2T_n_start);
			}
		}
		max_len = PHY_REG_SIZE;
	}

#else	// !MERGE_FW

	switch (reg_file) {
	case AGCTAB:
		pFileName = "/usr/rtl8192Pci/AGC_TAB.txt";
		phyreg_table = (struct PhyRegTable *)priv->pshare->agc_tab_buf;
		max_len = AGC_TAB_SIZE;
		break;
/*
	case PHYREG_1T2R:
		if (priv->pshare->rf_ft_var.pathB_1T == 0)
			pFileName = "/usr/rtl8192Pci/PHY_to1T2R.txt";
		else
			pFileName = "/usr/rtl8192Pci/PHY_to1T2R_b.txt";
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		file_format = THREE_COLUMN;
		max_len = PHY_REG_SIZE;
		break;
*/
	case PHYREG_2T2R:
/*
#ifdef MP_TEST
		if (priv->pshare->rf_ft_var.mp_specific) {
			pFileName = "/usr/rtl8192Pci/phy_reg_mp.txt";
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			max_len = PHY_REG_SIZE;
		}
		else
#endif
*/
		{
			pFileName = "/usr/rtl8192Pci/phy_reg.txt";
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			max_len = PHY_REG_SIZE;
		}
		break;
	case PHYREG_1T1R:
		if (priv->pshare->rf_ft_var.pathB_1T == 0)
			pFileName = "/usr/rtl8192Pci/PHY_to1T1R.txt";
		else
			pFileName = "/usr/rtl8192Pci/PHY_to1T1R_b.txt";
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		file_format = THREE_COLUMN;
		max_len = PHY_REG_SIZE;
		break;
	case PHYREG_PG:
		pFileName = "/usr/rtl8192Pci/PHY_REG_PG.txt";
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
		break;
	}
#endif // MERGE_FW

	{
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef MERGE_FW
		memcpy(mem_ptr, next_head, read_bytes);
#else

		old_fs = get_fs();
		set_fs(KERNEL_DS);

		if ((fd = sys_open(pFileName, O_RDONLY, 0)) < 0)
		{
			printk("PHY_ConfigBBWithParaFile(): cannot open %s\n", pFileName);
			set_fs(old_fs);
			kfree(mem_ptr);
			return -1;
		}

		read_bytes = sys_read(fd, mem_ptr, MAX_CONFIG_FILE_SIZE);
		sys_close(fd);
		set_fs(old_fs);
#endif // MERGE_FW

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			}
			else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;
				}
			}
		}

		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {
#ifdef MERGE_FW
			printk("PHY REG table buffer not large enough!\n");
#else
			printk("PHY REG table buffer not large enough! (%s)\n", pFileName);
#endif
			return -1;
		}
	}

	num = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		}
		else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

		if (u4bRegOffset == 0xff)
			break;
		else if (file_format == THREE_COLUMN)
			PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
		else
			PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		num++;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigRFWithParaFile()
 *
 * Overview:    This function read RF parameters from general file format, and do RF 3-wire
 *
 * Input:      	PADAPTER			Adapter
 *			ps1Byte 				pFileName
 *			RF92CD_RADIO_PATH_E	eRFPath
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		Delay may be required for RF configuration
 *---------------------------------------------------------------------------*/
int PHY_ConfigRFWithParaFile(struct rtl8192cd_priv *priv,
#ifdef MERGE_FW
				unsigned char *start, int read_bytes,
#else
				unsigned char *pFileName,
#endif
				RF92CD_RADIO_PATH_E eRFPath)
{
	int           num;
#ifndef MERGE_FW
	int           fd=0, read_bytes;
	mm_segment_t  old_fs;
#endif
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;
#if !defined(CONFIG_X86) && !defined(MERGE_FW)
	extern ssize_t sys_read(unsigned int fd, char * buf, size_t count);
#endif

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigRFWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef MERGE_FW
	memcpy(mem_ptr, start, read_bytes);
#else
	old_fs = get_fs();
	set_fs(KERNEL_DS);

#if	defined(CONFIG_X86)
	if ((fd = rtl8192cd_fileopen(pFileName, O_RDONLY, 0)) < 0)
#else
	if ((fd = sys_open(pFileName, O_RDONLY, 0)) < 0)
#endif
	{
		printk("PHY_ConfigRFWithParaFile(): cannot open %s\n", pFileName);
		set_fs(old_fs);
		kfree(mem_ptr);
		return -1;
	}

	read_bytes = sys_read(fd, mem_ptr, MAX_CONFIG_FILE_SIZE);
	sys_close(fd);
	set_fs(old_fs);
#endif // MERGE_FW

	next_head = mem_ptr;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);
		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
		if (num > 0) {
			if (u4bRegOffset == 0xff)
				break;
			else if (u4bRegOffset == 0xfe)
				delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
			else if (num == 2) {
				PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
				delay_ms(1);
			}
		}
	}

	kfree(mem_ptr);

	return 0;
}


int PHY_ConfigMACWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len=0;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;
#ifndef MERGE_FW
	int fd=0;
	mm_segment_t  old_fs;
	unsigned char *pFileName = "/usr/rtl8192Pci/MACPHY_REG.txt";
#ifndef CONFIG_X86
	extern ssize_t sys_read(unsigned int fd, char * buf, size_t count);
#endif
#endif
	struct PhyRegTable *reg_table = (struct PhyRegTable *)priv->pshare->mac_reg_buf;

	{
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef MERGE_FW
		read_bytes = (int)(data_MACPHY_REG_end - data_MACPHY_REG_start);
		memcpy(mem_ptr, data_MACPHY_REG_start, read_bytes);
#else

		old_fs = get_fs();
		set_fs(KERNEL_DS);

#if	defined(CONFIG_X86)
		if ((fd = rtl8192cd_fileopen(pFileName, O_RDONLY, 0)) < 0)
#else
		if ((fd = sys_open(pFileName, O_RDONLY, 0)) < 0)
#endif
		{
			printk("PHY_ConfigMACWithParaFile(): cannot open %s\n", pFileName);
			set_fs(old_fs);
			kfree(mem_ptr);
			return -1;
		}

		read_bytes = sys_read(fd, mem_ptr, MAX_CONFIG_FILE_SIZE);
		sys_close(fd);
		set_fs(old_fs);
#endif // MERGE_FW

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
			if (num > 0) {
				reg_table[len].offset = u4bRegOffset;
				reg_table[len].value = u4bRegValue;
				len++;
				if (u4bRegOffset == 0xff)
					break;
				if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE)
					break;
			}
		}
		reg_table[len].offset = 0xff;

		kfree(mem_ptr);

		if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE) {
			printk("MAC REG table buffer not large enough!\n");
			return -1;
		}
	}

	num = 0;
	while (1) {
		u4bRegOffset = reg_table[num].offset;
		u4bRegValue = reg_table[num].value;

		if (u4bRegOffset == 0xff)
			break;
		else
			RTL_W8(u4bRegOffset, u4bRegValue);
		num++;
	}

	return 0;
}


#ifdef UNIVERSAL_REPEATER
static struct rtl8192cd_priv *get_another_interface_priv(struct rtl8192cd_priv *priv)
{
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		return GET_VXD_PRIV(priv);
	else if (IS_DRV_OPEN(GET_ROOT_PRIV(priv)))
		return GET_ROOT_PRIV(priv);
	else
		return NULL;
}


static int get_shortslot_for_another_interface(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *p_priv;

	p_priv = get_another_interface_priv(priv);
	if (p_priv) {
		if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)
			return (p_priv->pmib->dot11ErpInfo.shortSlot);
		else {
			if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
				return (p_priv->pmib->dot11ErpInfo.shortSlot);
		}
	}
	return -1;
}
#endif // UNIVERSAL_REPEATER


void set_slot_time(struct rtl8192cd_priv *priv, int use_short)
{
#ifdef UNIVERSAL_REPEATER
	int is_short;
	is_short = get_shortslot_for_another_interface(priv);
	if (is_short != -1) { // not abtained
		use_short &= is_short;
	}
#endif

	if (use_short)
		RTL_W8(SLOT_TIME, 0x09);
	else
		RTL_W8(SLOT_TIME, 0x14);
}


void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset)
{
	unsigned int val = channel;

	if (channel > 14)
		priv->pshare->curr_band = BAND_5G;
	else
		priv->pshare->curr_band = BAND_2G;

	if (priv->pshare->CurrentChannelBW) {
		if (offset == 1)
			val -= 2;
		else
			val += 2;
	}

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		val += 14;

	PHY_SetRFReg(priv, RF92CD_PATH_A, rRfChannel, 0x3ff, val);
	PHY_SetRFReg(priv, RF92CD_PATH_B, rRfChannel, 0x3ff, val);

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		val -= 14;

	priv->pshare->working_channel = val;

	PHY_RF6052SetOFDMTxPower(priv, val);
	if (priv->pshare->curr_band == BAND_2G)
		PHY_RF6052SetCCKTxPower(priv, val);

	selectMinPowerIdex(priv);

	return;
}

#if 0
// switch 1 spatial stream path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_1SS_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if(get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	switch(antPath){
	case 1:
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x01100000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x01100000; // Path A
		RTL_W32(0x90C, dword);
		break;
	case 2:
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x02200000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x02200000;	// Path B
		RTL_W32(0x90C, dword);
		break;

	case 3:
		if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1)// use one ANT for 1ss
			goto switch_1ss_end;// do nothing
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x03300000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x03300000; // Path A,B
		RTL_W32(0x90C, dword);
		break;

	default:// do nothing
		break;
	}
switch_1ss_end:
	return;

}

// switch OFDM path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_OFDM_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if(get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	switch(antPath){
	case 1:
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000010)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000010; // Path A
		RTL_W32(0x90C, dword);
		break;
	case 2:
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000020)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000020;	// Path B
		RTL_W32(0x90C, dword);
		break;

	case 3:
		if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1)// use one ANT for 1ss
			goto switch_OFDM_end;// do nothing
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000030)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000030; // Path A,B
		RTL_W32(0x90C, dword);
		break;

	default:// do nothing
		break;
	}
switch_OFDM_end:
	return;

}


#endif

void enable_hw_LED(struct rtl8192cd_priv *priv, unsigned int led_type)
{
	switch (led_type) {
	case LEDTYPE_HW_TX_RX:
		RTL_W32(LEDCFG, LED_RX_EVENT_ON<<LED1CM_SHIFT | LED_TX_EVENT_ON<<LED0CM_SHIFT);
		break;
	case LEDTYPE_HW_LINKACT_INFRA:
		RTL_W32(LEDCFG, LED_TX_RX_EVENT_ON<<LED0CM_SHIFT);
		if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_STATION_STATE))
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0x0ff);
		else
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
		break;
	default:
		break;
	}
}


/**
* Function:	phy_InitBBRFRegisterDefinition
*
* OverView:	Initialize Register definition offset for Radio Path A/B/C/D
*
* Input:
*			PADAPTER		Adapter,
*
* Output:	None
* Return:		None
* Note:		The initialization value is constant and it should never be changes
*/
void phy_InitBBRFRegisterDefinition(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw = GET_HW(priv);

	// RF Interface Sowrtware Control
	phw->PHYRegDef[RF92CD_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 LSBs if read 32-bit from 0x870
	phw->PHYRegDef[RF92CD_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 MSBs if read 32-bit from 0x870 (16-bit for 0x872)

	// RF Interface Readback Value
	phw->PHYRegDef[RF92CD_PATH_A].rfintfi = rFPGA0_XAB_RFInterfaceRB; // 16 LSBs if read 32-bit from 0x8E0
	phw->PHYRegDef[RF92CD_PATH_B].rfintfi = rFPGA0_XAB_RFInterfaceRB;// 16 MSBs if read 32-bit from 0x8E0 (16-bit for 0x8E2)

	// RF Interface Output (and Enable)
	phw->PHYRegDef[RF92CD_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x860
	phw->PHYRegDef[RF92CD_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x864

	// RF Interface (Output and)  Enable
	phw->PHYRegDef[RF92CD_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x860 (16-bit for 0x862)
	phw->PHYRegDef[RF92CD_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x864 (16-bit for 0x866)

	//Addr of LSSI. Wirte RF register by driver
	phw->PHYRegDef[RF92CD_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; //LSSI Parameter
	phw->PHYRegDef[RF92CD_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;

	// RF parameter
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSI_Select = rFPGA0_XAB_RFParameter;  //BB Band Select
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSI_Select = rFPGA0_XAB_RFParameter;

	// Tx AGC Gain Stage (same for all path. Should we remove this?)
	phw->PHYRegDef[RF92CD_PATH_A].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF92CD_PATH_B].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage

	// Tranceiver A~D HSSI Parameter-1
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara1 = rFPGA0_XA_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara1 = rFPGA0_XB_HSSIParameter1;  //wire control parameter1

	// Tranceiver A~D HSSI Parameter-2
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  //wire control parameter2

	// RF switch Control
	phw->PHYRegDef[RF92CD_PATH_A].rfSwitchControl = rFPGA0_XAB_SwitchControl; //TR/Ant switch control
	phw->PHYRegDef[RF92CD_PATH_B].rfSwitchControl = rFPGA0_XAB_SwitchControl;

	// AGC control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl1 = rOFDM0_XAAGCCore1;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl1 = rOFDM0_XBAGCCore1;

	// AGC control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl2 = rOFDM0_XAAGCCore2;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl2 = rOFDM0_XBAGCCore2;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxIQImbalance = rOFDM0_XARxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxIQImbalance = rOFDM0_XBRxIQImbalance;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxAFE = rOFDM0_XARxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxAFE = rOFDM0_XBRxAFE;

	// Tx AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfTxIQImbalance = rOFDM0_XATxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxIQImbalance = rOFDM0_XBTxIQImbalance;

	// Tx AFE control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfTxAFE = rOFDM0_XATxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxAFE = rOFDM0_XBTxAFE;

	// Tranceiver LSSI Readback SI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;

	// Tranceiver LSSI Readback PI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBackPi = TransceiverA_HSPI_Readback;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBackPi = TransceiverB_HSPI_Readback;
}


void check_chipID_MIMO(struct rtl8192cd_priv *priv)
{
	unsigned int val32;

	val32 = RTL_R32(SYS_CFG);
	if (val32 & BIT(27)) {
		priv->pshare->version_id = VERSION_8192C;
		priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
	} else {
		priv->pshare->version_id = VERSION_8188C;
		priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;

		if ((0x3 &(RTL_R32(0xec)>>22))== 0x01 )
			priv->pshare->version_id |= 0x200;		// 88RE
	}

	if (val32 & BIT(23)) {
		priv->pshare->version_id |= 0x100;
	}

	if (val32 & BIT(19)) {
		priv->pshare->version_id |= 0x400;			// UMC
		priv->pshare->version_id |= (0xf0 & (val32>>8));	//	0x00:  a-cut
	}
	if( ((0x0f & (val32>>16)) ==0 ) && ((0x0f & (val32>>12)) ==1 ) ) {		//6195B
		priv->pshare->version_id |= 0x400;	
		priv->pshare->version_id |= 0x10;					//	0x10:	b-cut
	}

	return;
}

void selectMinPowerIdex(struct rtl8192cd_priv *priv)
{
	int i=0,idx, pwr_min = 0xff;
	unsigned int val32;
	unsigned int pwr_regA[] = {0xe00,0xe04,0xe08,0x86c,0xe10,0xe14,0xe18,0xe1c};
	unsigned int pwr_regB[] = {0x830,0x834,0x838,0x86c,0x83c,0x848,0x84c,0x868};

	for (idx=0 ; idx < 8 ; idx++)
	{
		val32 = RTL_R32(pwr_regA[idx]);
		switch (pwr_regA[idx]) {
		case 0xe08:
			pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> 8) & 0xff);
			break;

		case 0x86c:
			for (i=8 ; i < 32 ; i+=8)
				pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
			break;

		default:
			for (i=0 ; i < 32 ; i+=8)
				pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
			break;
		}
	}	

	if (GET_CHIP_VER(priv) != VERSION_8188C)
	{
		for (idx=0 ; idx < 8 ; idx++)
		{
			val32 = RTL_R32(pwr_regB[idx]);
			switch (pwr_regB[idx]) {
			case 0x86c:
				pwr_min = POWER_MIN_CHECK(pwr_min,val32 & 0xff);
				break;

			case 0x838:
				for (i=8 ; i < 32 ; i+=8)
					pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
				break;

			default:
				for (i=0 ; i < 32 ; i+=8)
					pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
				break;
			}
		}			
	}

	priv->pshare->rf_ft_var.min_pwr_idex = pwr_min;
}

void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal, defValue =0x28 ;
	unsigned char  offset;
	char base, byte0, byte1, byte2, byte3;
	unsigned char pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
	unsigned char pwrlevelHT40_1S_B = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];

	if (pwrlevelHT40_1S_A == 0) {	// use default value

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa)
			defValue = HP_OFDM_POWER_DEFAULT ;
#endif
#ifndef ADD_TX_POWER_BY_CMD
		defValue = (defValue<<24)|(defValue<<16)|(defValue<<8)|(defValue);

		RTL_W32(rTxAGC_A_Rate18_06, defValue);
		RTL_W32(rTxAGC_A_Rate54_24, defValue);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, defValue);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, defValue);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, defValue);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, defValue);
		RTL_W32(rTxAGC_B_Rate18_06, defValue);
		RTL_W32(rTxAGC_B_Rate54_24, defValue);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, defValue);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, defValue);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, defValue);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, defValue);	
#else
		base = defValue;
		byte0 = byte1 = byte2 = byte3 = 0;

		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);
		
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);		
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);		
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);		
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);		
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);		
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);		
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);		
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);		
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

#endif // ADD_TX_POWER_BY_CMD
		return; // use default
	}

	/******************************  PATH A  ******************************/
	base = pwrlevelHT40_1S_A;
	offset = (priv->pmib->dot11RFEntry.pwrdiffOFDM[channel-1] & 0x0f);
	base = COUNT_SIGN_OFFSET(base, offset);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_A;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = (priv->pmib->dot11RFEntry.pwrdiffHT20[channel-1] & 0x0f);
		base = COUNT_SIGN_OFFSET(base, offset);
	}

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	offset = (priv->pmib->dot11RFEntry.pwrdiffHT40_2S[channel-1] & 0x0f);
	base = COUNT_SIGN_OFFSET(base, offset);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[8]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[9]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[10]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[11]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[12]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[13]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[14]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[15]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);

	/******************************  PATH B  ******************************/
	base = pwrlevelHT40_1S_B;
	offset = ((priv->pmib->dot11RFEntry.pwrdiffOFDM[channel-1] & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = ((priv->pmib->dot11RFEntry.pwrdiffHT20[channel-1] & 0xf0) >> 4);
		base = COUNT_SIGN_OFFSET(base, offset);
	}

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	offset = ((priv->pmib->dot11RFEntry.pwrdiffHT40_2S[channel-1] & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[8]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[9]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[10]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[11]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[12]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[13]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[14]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[15]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
}	/* PHY_RF6052SetOFDMTxPower */


void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal = 0;
	char byte, byte1, byte2;
	char pwrlevelCCK_A = priv->pmib->dot11RFEntry.pwrlevelCCK_A[channel-1];
	char pwrlevelCCK_B = priv->pmib->dot11RFEntry.pwrlevelCCK_B[channel-1];

	if (priv->pshare->rf_ft_var.cck_pwr_max) {
		byte = POWER_RANGE_CHECK(priv->pshare->rf_ft_var.cck_pwr_max);
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
		return;
	}

	if (pwrlevelCCK_A == 0) {	// use default value
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa)
			byte = HP_CCK_POWER_DEFAULT;
		else
#endif
			byte = 0x24;

#ifndef ADD_TX_POWER_BY_CMD
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);		
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#else
		pwrlevelCCK_A = pwrlevelCCK_B = byte;
		byte = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_B + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte2);
		writeVal = ((byte2 << 16) | (byte1 << 8) | byte);
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_11);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte2);
		writeVal = ((byte2 << 24) | (byte1 << 16) | (byte << 8) | byte2);
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#endif
		return; // use default
	}

	writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[3]);
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[1]) << 16) |
	           (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[2]) << 8)  |
	            POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[3]);
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[0]) << 24) |
	           (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[1]) << 16) |
	           (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[2]) << 8)  |
	            POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[0]);
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
}


static int phy_BB8192CD_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	unsigned short val16;
	unsigned int val32;

	phy_InitBBRFRegisterDefinition(priv);

	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16|BIT(13)|BIT(0)|BIT(1));

	// 20090923 Joseph: Advised by Steven and Jenyu. Power sequence before init RF.
	RTL_W8(REG_AFE_PLL_CTRL, 0x83);
	RTL_W8(REG_AFE_PLL_CTRL+1, 0xdb);
	RTL_W8(REG_RF_CTRL, RF_EN|RF_RSTB|RF_SDMRSTB);
	//RTL_W8(REG_SYS_FUNC_EN, FEN_PPLL|FEN_PCIEA|FEN_DIO_PCIE|FEN_USBA|FEN_BB_GLB_RST|FEN_BBRSTB);
	//RTL_W8(REG_LDOHCI12_CTRL, 0x1f);
	RTL_W8(REG_AFE_XTAL_CTRL+1, 0x80);

	val32 = RTL_R32(REG_AFE_XTAL_CTRL);
	val32 = (val32 & (~(BIT(11) | BIT(14)))) | (BIT(18) | BIT(19) | BIT(21) | BIT(22));
	RTL_W32(REG_AFE_XTAL_CTRL, val32);

	/*----Check chip ID and hw TR MIMO config----*/
	//check_chipID_MIMO(priv);

	/*----BB Register Initilazation----*/
	if (get_rf_mimo_mode(priv) == MIMO_2T2R)
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_2T2R);
	else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);

#ifdef MP_TEST
	if ((priv->pshare->rf_ft_var.mp_specific) && !IS_TEST_CHIP(priv)) {
		delay_ms(10);
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/

//	if (!priv->EE_AutoloadFail)
//	{
//		printk("need PG setup\n");
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);
//	}
	if(rtStatus){
		printk("phy_BB8192CD_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}

	/*----BB AGC table Initialization----*/
	rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);
	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}

#if 0
	/*----For 1T2R Config----*/
	if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T2R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T2R Fail!!\n");
			return rtStatus;
		}
	}else if (get_rf_mimo_mode(priv) == MIMO_1T1R){
		delay_ms(100);
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T1R Fail!!\n");
			return rtStatus;
		}
	}
#endif

	DEBUG_INFO("PHY-BB Initialization Success\n");
	return 0;
}


int phy_RF6052_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	RF92CD_RADIO_PATH_E eRFPath;
	BB_REGISTER_DEFINITION_T *pPhyReg;
	unsigned int  u4RegValue = 0;

	priv->pshare->phw->NumTotalRFPath = 2;

	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		pPhyReg = &priv->pshare->phw->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch(eRFPath)
		{
		case RF92CD_PATH_A:
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF92CD_PATH_B :
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV<<16);
			break;
		case RF92CD_PATH_MAX:
			break;
		}

		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfe, bRFSI_RFENV<<16, 0x1);

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);

		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath)
		{
#ifdef MERGE_FW
		case RF92CD_PATH_A:
			if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
#ifdef TESTCHIP_SUPPORT
				if (IS_TEST_CHIP(priv))
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_start,
								(int)(data_radio_a_2T_end - data_radio_a_2T_start), eRFPath);
				else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa)
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_hp_start,
										(int)(data_radio_a_2T_n_hp_end - data_radio_a_2T_n_hp_start), eRFPath);
					else
#endif
					if (priv->pshare->rf_ft_var.use_ext_lna)
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_lna_start,
										(int)(data_radio_a_2T_n_lna_end - data_radio_a_2T_n_lna_start), eRFPath);
					else
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_start,
										(int)(data_radio_a_2T_n_end - data_radio_a_2T_n_start), eRFPath);
				}
			} else if (get_rf_mimo_mode(priv) == MIMO_1T1R)	{
#ifdef TESTCHIP_SUPPORT
				if (IS_TEST_CHIP(priv))
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_start,
									(int)(data_radio_a_1T_end - data_radio_a_1T_start), eRFPath);
				else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_hp_start,
											(int)(data_radio_a_2T_n_hp_end - data_radio_a_2T_n_hp_start), eRFPath);
						else
#endif
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_n_start,
									(int)(data_radio_a_1T_n_end - data_radio_a_1T_n_start), eRFPath);

				}
			}
			break;
		case RF92CD_PATH_B:
			if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
#ifdef TESTCHIP_SUPPORT
				if (IS_TEST_CHIP(priv))
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_start,
								(int)(data_radio_b_2T_end - data_radio_b_2T_start), eRFPath);
				else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa)
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_hp_start,
										(int)(data_radio_b_2T_n_hp_end - data_radio_b_2T_n_hp_start), eRFPath);
					else
#endif
					if (priv->pshare->rf_ft_var.use_ext_lna)
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_lna_start,
										(int)(data_radio_b_2T_n_lna_end - data_radio_b_2T_n_lna_start), eRFPath);
					else
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_start,
									(int)(data_radio_b_2T_n_end - data_radio_b_2T_n_start), eRFPath);
				}
			} else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				rtStatus=0;
			break;
#else
		case RF92CD_PATH_A:
			rtStatus = PHY_ConfigRFWithParaFile(priv, "/usr/rtl8192Pci/radio_a.txt", eRFPath);
			break;
		case RF92CD_PATH_B:
			rtStatus = PHY_ConfigRFWithParaFile(priv, "/usr/rtl8192Pci/radio_b.txt", eRFPath);
			break;
#endif
		default:
			break;
		}

		/*----Restore RFENV control type----*/;
		switch(eRFPath)
		{
		case RF92CD_PATH_A:
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
				break;
		case RF92CD_PATH_B :
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV<<16, u4RegValue);
			break;
		case RF92CD_PATH_MAX:
			break;
		}
	}

	DEBUG_INFO("PHY-RF Initialization Success\n");

	return rtStatus;
}


//
// Description:
//	Set the MAC offset [0x09] and prevent all I/O for a while (about 20us~200us, suggested from SD4 Scott).
//	If the protection is not performed well or the value is not set complete, the next I/O will cause the system hang.
// Note:
//	This procudure is designed specifically for 8192S and references the platform based variables
//	which violates the stucture of multi-platform.
//	Thus, we shall not extend this procedure to common handler.
// By Bruce, 2009-01-08.
//
unsigned char
HalSetSysClk8192CD(	struct rtl8192cd_priv *priv,	unsigned char Data)
{
	RTL_W8((SYS_CLKR + 1), Data);
	delay_us(200);
	return TRUE;
}


static void LLT_table_init(struct rtl8192cd_priv *priv)
{

	unsigned int i, count = 0;

#if 1
	unsigned txpktbufSz = 0xf6;
#else
	unsigned txpktbufSz = 252; //174(0xAE) 120(0x78) 252(0xFC)
#endif

	// Joseph test!!
	// txpktbufSz = 174;

	for ( i = 0; i < txpktbufSz-1; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
			|((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

		count = 0;
		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT_init, section 01, i=%d\n", i);
				printk("LLT Polling failed 01 !!!\n");
				return;
			}
		} while(count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)
		|((txpktbufSz-1)&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT|(255&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 02 !!!\n");
			return;
		}
	} while(count < 100);


	for (i = txpktbufSz; i < 255; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
			|((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

		do{
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT Polling failed 03 !!!\n");
				return;
			}
		}while(count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(255&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
		|(txpktbufSz&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
			break;
		if(++count >= 100) {
			printk("LLT Polling failed 04 !!!\n");
			return;
		}
	} while(count < 100);

// Set reserved page for each queue

#if 1
	RTL_W32(RQPN, 0x809f2929);
#else
	if(txpktbufSz == 120)
		RTL_W32(RQPN, 0x80272828);
	else if(txpktbufSz == 252)
	{
		//RTL_W32(RQPN, 0x80c31c1c);

		// Joseph test
		//RTL_W32(RQPN, 0x80838484);
		RTL_W32(RQPN, 0x80bd1c1c);
	}
	else
		RTL_W32(RQPN, 0x80393a3a);
#endif

	//RTL_W32(TDECTRL, RTL_R32(TDECTRL)|(txpktbufSz&BCN_HEAD_Mask)<<BCN_HEAD_SHIFT);
	RTL_W8(TXPKTBUF_BCNQ_BDNY, txpktbufSz);
	RTL_W8(TXPKTBUF_MGQ_BDNY, txpktbufSz);
	RTL_W8(TRXFF_BNDY, txpktbufSz);
	RTL_W8(TDECTRL+1, txpktbufSz);
	RTL_W8(0x45D, txpktbufSz);

	RTL_W8(0x214, 4);		//RQPN_NPQ
}


static void MacInit(struct rtl8192cd_priv *priv)
{
	unsigned int bytetmp, retry;
	DEBUG_INFO("CP: MacInit===>>");
	RTL_W8(RSV_CTRL0, 0x00);

	// Power on when re-enter from IPS/Radio off/card disable
	RTL_W8(AFE_XTAL_CTRL, 0x0d);	// enable XTAL		// clk inverted
	RTL_W8(SPS0_CTRL, 0x2b);		// enable SPS into PWM
	delay_ms(1);
	
#if 0
	// Enable AFE BANDGAP
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC)|AFE_BGEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable AFE MBIAS
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC)|AFE_MBEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable PLL Power (LDOA15V)
	RTL_W8(LDOA15_CTRL, RTL_R8(LDOA15_CTRL)|LDA15_EN);

	// Enable VDDCORE (LDOD12V)
	RTL_W8(LDOV12D_CTRL, RTL_R8(LDOV12D_CTRL)|LDV12_EN);

	// Release XTAL Gated for AFE PLL
//	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL)|XTAL_GATE_AFE);
	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL) & ~XTAL_GATE_AFE);

	// Enable AFE PLL
	RTL_W32(AFE_PLL_CTRL, RTL_R32(AFE_PLL_CTRL)|APLL_EN);

	// Release Isolation AFE PLL & MD
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~ISO_MD2PP);

	// Enable WMAC Clock
	RTL_W16(SYS_CLKR, RTL_R16(SYS_CLKR)|MAC_CLK_EN|SEC_CLK_EN);

	// Release WMAC reset & register reset
	RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN)|FEN_MREGEN|FEN_DCORE);

	// Release IMEM Isolation
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~(BIT(10)|ISO_DIOR));	//	need to confirm

/*	// need double setting???
	// Enable MAC IO registers
	RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN)|FEN_MREGEN);
*/

	// Switch HWFW select
	RTL_W16(SYS_CLKR, (RTL_R16(SYS_CLKR)|CLKR_80M_SSC_DIS) & ~BIT(6));	//	need to confirm
#else
	// auto enable WLAN
	// Power On Reset for MAC Block
	bytetmp = RTL_R8(APS_FSMCO+1) | BIT(0);
	delay_us(2);
	RTL_W8(APS_FSMCO+1, bytetmp);
	delay_us(2);

	bytetmp = RTL_R8(APS_FSMCO+1);
	delay_us(2);
	retry = 0;
	while((bytetmp & BIT(0)) && retry < 1000){
		retry++;
		delay_us(50);
		bytetmp = RTL_R8(APS_FSMCO+1);
		delay_us(50);
	}
	
	if (bytetmp & BIT(0)) {
		DEBUG_ERR("%s ERROR: auto enable WLAN failed!!(0x%02X)\n", __FUNCTION__, bytetmp);
	}
	
	// Enable Radio off, GPIO, and LED function
	RTL_W16(APS_FSMCO, 0x1012);			// when enable HWPDN
	
	// release RF digital isolation
	RTL_W8(SYS_ISO_CTRL+1, 0x82);
	
	delay_us(2);
#endif

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR)|MACRXEN|MACTXEN|SCHEDULE_EN|PROTOCOL_EN
		|RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);

	//System init
	LLT_table_init(priv);

	// Clear interrupt and enable interrupt
	RTL_W32(HISR, 0xFFFFFFFF);
	RTL_W16(HISRE, 0xFFFF);

	// Set Rx FF0 boundary : 9K/10K
	RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x27FF&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);

	if (IS_TEST_CHIP(priv)) {
		// Set High priority queue select : HPQ:BC/H/VO/VI/MG, LPQ:BE/BK
		// [5]:H, [4]:MG, [3]:BK, [2]:BE, [1]:VI, [0]:VO
		RTL_W16(TRXDMA_CTRL, ((HPQ_SEL_HIQ|HPQ_SEL_MGQ|HPQ_SEL_VIQ|HPQ_SEL_VOQ)&HPQ_SEL_Mask)<<HPQ_SEL_SHIFT);

		/*
		 * Enable ampdu rx check error, and enable rx byte shift
		 */
		RTL_W8(TRXDMA_CTRL, RTL_R8(TRXDMA_CTRL) | RXSHFT_EN | RXDMA_ARBBW_EN);

	} else {
		RTL_W16(TRXDMA_CTRL, (0xB770 | RXSHFT_EN | RXDMA_ARBBW_EN));
	}


//	RTL_W8(TDECTRL, 0x11);	// need to confirm

	// Set Network type: ap mode
	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0);	//	need to confirm

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	RTL_W8(PBP, (PBP_128B&PSTX_Mask)<<PSTX_SHIFT|(PBP_128B&PSRX_Mask)<<PSRX_SHIFT);

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS|RCR_APP_MIC|RCR_APP_ICV|RCR_APP_PHYSTS|RCR_HTC_LOC_CTRL
		|RCR_AMF|RCR_ADF|RCR_AICV|RCR_ACRC32|RCR_AB|RCR_AM|RCR_APM|RCR_AAP);

	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// This part is not in WMAC InitMAC()
	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR)|CFE_FORM);

	// Set TCR to avoid deadlock
 	RTL_W32(TCR, RTL_R32(TCR)|BIT(15)|BIT(14)|BIT(13)|BIT(12));

	// Set RRSR (response rate set reg)
	//SetResponseRate();
	// Set RRSR (response rate set reg)
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M
	RTL_W16(RRSR, 0xFFFF);	//	need to confirm
	RTL_W8(RRSR+2, 0xFF);

	// Set Spec SIFS (used in NAV)
	// Joseph test
	RTL_W16(SPEC_SIFS_A, (0x0A&SPEC_SIFS_OFDM_Mask)<<SPEC_SIFS_OFDM_SHIFT
		|(0x0A&SPEC_SIFS_CCK_Mask)<<SPEC_SIFS_CCK_SHIFT);

	// Set SIFS for CCK
	// Joseph test
	RTL_W16(SIFS_CCK, (0x0A&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

	// Set SIFS for OFDM
	// Joseph test
	RTL_W16(SIFS_OFDM, (0x0A&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

	// Set retry limit
	// Joseph test
	priv->pshare->RLShort = 0x10;//0x30;
	priv->pshare->RLLong = 0x10; //0x30;
	RTL_W16(RL, (priv->pshare->RLShort&SRL_Mask)<<SRL_SHIFT|(priv->pshare->RLLong&LRL_Mask)<<LRL_SHIFT);

	//Set Desc Address
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
	RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
	RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
	RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
	RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
	RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
	RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
	RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
//	RTL_W32(RCDA, priv->pshare->phw->rxcmd_ring_addr);
//	RTL_W32(TCDA, priv->pshare->phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);
	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
//	RTL_W16(CMDR, 0x37FC);

	RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)|(0x07&MAX_RXDMA_Mask)<<MAX_RXDMA_SHIFT
		|(0x07&MAX_TXDMA_Mask)<<MAX_TXDMA_SHIFT | BCNQSTOP);
	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do{
		retry++;
		bytetmp = RTL_R8(APSD_CTRL);
	} while((retry<200) && (bytetmp&BIT(7))); //polling until BIT7 is 0. by tynli
	
	if (bytetmp & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, bytetmp);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);

	DEBUG_INFO("DONE\n");
}	//	MacInit


static void MacConfig(struct rtl8192cd_priv *priv)
{
	RTL_W8(INIRTS_RATE_SEL, 0x8); // 24M

	// 2007/02/07 Mark by Emily becasue we have not verify whether this register works
	//For 92C,which reg?
	//	RTL_W8(BWOPMODE, BW_20M);	//	set if work at 20m

	// Ack timeout.
	if ((priv->pmib->miscEntry.ack_timeout > 0) && (priv->pmib->miscEntry.ack_timeout < 0xff))
		RTL_W8(ACKTO, priv->pmib->miscEntry.ack_timeout);
	else
		RTL_W8(ACKTO, 0x40);

	// clear for mbid beacon tx
	RTL_W8(MULTI_BCNQ_EN, 0);
	RTL_W8(MULTI_BCNQ_OFFSET, 0);

	// set user defining ARFR table for 11n 1T
	RTL_W32(ARFR0, 0xFF015);	// 40M mode
	RTL_W32(ARFR1, 0xFF005);	// 20M mode

	/*
 	 * Disable TXOP CFE
 	 */
	RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);

	/*
	 *	RA try rate aggr limit
	 */
	RTL_W8(RA_TRY_RATE_AGG_LMT, 2);

	/*
	 *	Max mpdu number per aggr
	 */
	RTL_W16(PROT_MODE_CTRL+2, 0x0909);
}


unsigned int get_mean_of_2_close_value(unsigned int *val_array)
{
	unsigned int tmp1, tmp2;

	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	if (val_array[1] > val_array[2]) {
		tmp1 = val_array[2];
		val_array[2] = val_array[1];
		val_array[1] = tmp1;
	}
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);

	tmp1 = val_array[1] - val_array[0];
	tmp2 = val_array[2] - val_array[1];
	if (tmp1 < tmp2)
		tmp1 = (val_array[0] + val_array[1]) / 2;
	else
		tmp1 = (val_array[1] + val_array[2]) / 2;

	//printk("final %08x\n", tmp1);
	return tmp1;
}

#ifndef CONFIG_RTL_NEW_IQK
static void IQK_92C(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num=0, cal_retry=0, Oldval=0, temp_c04=0, temp_c08=0, temp_874=0, temp_eac;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, cal_eb4, cal_ebc, cal_ec4, cal_ecc;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3], val_eb4[3], val_ebc[3], val_ec4[3], val_ecc[3];
#ifdef HIGH_POWER_EXT_PA
	unsigned int temp_870=0, temp_860=0, temp_864=0;
#endif

	// step 1: save ADDA power saving parameters
	unsigned int temp_85c = RTL_R32(0x85c);
	unsigned int temp_e6c = RTL_R32(0xe6c);
	unsigned int temp_e70 = RTL_R32(0xe70);
	unsigned int temp_e74 = RTL_R32(0xe74);
	unsigned int temp_e78 = RTL_R32(0xe78);
	unsigned int temp_e7c = RTL_R32(0xe7c);
	unsigned int temp_e80 = RTL_R32(0xe80);
	unsigned int temp_e84 = RTL_R32(0xe84);
	unsigned int temp_e88 = RTL_R32(0xe88);
	unsigned int temp_e8c = RTL_R32(0xe8c);
	unsigned int temp_ed0 = RTL_R32(0xed0);
	unsigned int temp_ed4 = RTL_R32(0xed4);
	unsigned int temp_ed8 = RTL_R32(0xed8);
	unsigned int temp_edc = RTL_R32(0xedc);
	unsigned int temp_ee0 = RTL_R32(0xee0);
	unsigned int temp_eec = RTL_R32(0xeec);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		temp_870 = RTL_R32(0x870);
		temp_860 = RTL_R32(0x860);
		temp_864 = RTL_R32(0x864);
	}
#endif

	// step 2: Path-A ADDA all on
	RTL_W32(0x85c, 0x04db25a4);
	RTL_W32(0xe6c, 0x04db25a4);
	RTL_W32(0xe70, 0x04db25a4);
	RTL_W32(0xe74, 0x04db25a4);
	RTL_W32(0xe78, 0x04db25a4);
	RTL_W32(0xe7c, 0x04db25a4);
	RTL_W32(0xe80, 0x04db25a4);
	RTL_W32(0xe84, 0x04db25a4);
	RTL_W32(0xe88, 0x04db25a4);
	RTL_W32(0xe8c, 0x04db25a4);
	RTL_W32(0xed0, 0x04db25a4);
	RTL_W32(0xed4, 0x04db25a4);
	RTL_W32(0xed8, 0x04db25a4);
	RTL_W32(0xedc, 0x04db25a4);
	RTL_W32(0xee0, 0x04db25a4);
	RTL_W32(0xeec, 0x04db25a4);

	// step 3: IQ&LO calibration Setting
		// BB switch to PI mode
	//RTL_W32(0x820, 0x01000100);
	//RTL_W32(0x828, 0x01000100);
		//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x00204000);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		PHY_SetBBReg(priv, 0x870, BIT(26), 1);
		PHY_SetBBReg(priv, 0x860, BIT(10), 0);
		PHY_SetBBReg(priv, 0x864, BIT(10), 0);
	}
#endif
	RTL_W32(0x840, 0x00010000);
	RTL_W32(0x844, 0x00010000);

	//AP or IQK
//	RTL_W32(0xb68 , 0x0f600000);
//	RTL_W32(0xb6c , 0x0f600000);
	RTL_W32(0xb68 , 0x00080000);
	RTL_W32(0xb6c , 0x00080000);

		// IQK setting
	RTL_W32(0xe28, 0x80800000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
		// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	RTL_W32(0xe3c, 0x28160202);
		// path-B IQK setting
	RTL_W32(0xe50, 0x10008c22);
	RTL_W32(0xe54, 0x10008c22);
	RTL_W32(0xe58, 0x82140102);
	RTL_W32(0xe5c, 0x28160202);
		// LO calibration setting
	RTL_W32(0xe4c, 0x001028d1);

	// delay to ensure Path-A IQK success
	delay_ms(300);

	// step 4: One shot, path A LOK & IQK
	while (cal_num < 3) {
			// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
			// delay 1ms
		delay_ms(10);

		// check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_e94 = (RTL_R32(0xe94) >> 16) & 0x3ff;
		cal_e9c = (RTL_R32(0xe9c) >> 16) & 0x3ff;
		cal_ea4 = (RTL_R32(0xea4) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		cal_eac = (temp_eac >> 16) & 0x3ff;
		if (!(temp_eac & BIT(28)) && !(temp_eac & BIT(27)) &&
			(cal_e94 != 0x142) && (cal_e9c != 0x42) &&
			(cal_ea4 != 0x132) && (cal_eac != 0x36)) {
			val_e94[cal_num] = cal_e94;
			val_e9c[cal_num] = cal_e9c;
			val_ea4[cal_num] = cal_ea4;
			val_eac[cal_num] = cal_eac;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("IQK Path-A Check\n");
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_e94 = get_mean_of_2_close_value(val_e94);
		cal_e9c = get_mean_of_2_close_value(val_e9c);
		cal_ea4 = get_mean_of_2_close_value(val_ea4);
		cal_eac = get_mean_of_2_close_value(val_eac);

		priv->pshare->RegE94=cal_e94;
		priv->pshare->RegE9C=cal_e9c;

		Oldval = (RTL_R32(0xc80) >> 22) & 0x3ff;

		X = cal_e94;
		PHY_SetBBReg(priv, 0xc80, 0x3ff, X * (Oldval / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval) >> 7) & 0x1);

		Y = cal_e9c;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, ((Y * (Oldval / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, (Y * (Oldval / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	}else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

	// step 5: Path-A standby mode
	RTL_W32(0xe28, 0);
	RTL_W32(0x840, 0x00010000);
	RTL_W32(0xe28, 0x80800000);

	// step 6: Path-B ADDA all on
	RTL_W32(0x85c, 0x0b1b25a4);
	RTL_W32(0xe6c, 0x0b1b25a4);
	RTL_W32(0xe70, 0x0b1b25a4);
	RTL_W32(0xe74, 0x0b1b25a4);
	RTL_W32(0xe78, 0x0b1b25a4);
	RTL_W32(0xe7c, 0x0b1b25a4);
	RTL_W32(0xe80, 0x0b1b25a4);
	RTL_W32(0xe84, 0x0b1b25a4);
	RTL_W32(0xe88, 0x0b1b25a4);
	RTL_W32(0xe8c, 0x0b1b25a4);
	RTL_W32(0xed0, 0x0b1b25a4);
	RTL_W32(0xed4, 0x0b1b25a4);
	RTL_W32(0xed8, 0x0b1b25a4);
	RTL_W32(0xedc, 0x0b1b25a4);
	RTL_W32(0xee0, 0x0b1b25a4);
	RTL_W32(0xeec, 0x0b1b25a4);

	// step 7: One shot, path B LOK & IQK
	cal_num = 0;
	cal_retry = 0;
	while (cal_num < 3) {
			// One shot, path B LOK & IQK
		RTL_W32(0xe60, 2);
		RTL_W32(0xe60, 0);
			// delay 1ms
		delay_ms(10);

		// check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_eb4 = (RTL_R32(0xeb4) >> 16) & 0x3ff;
		cal_ebc = (RTL_R32(0xebc) >> 16) & 0x3ff;
		cal_ec4 = (RTL_R32(0xec4) >> 16) & 0x3ff;
		cal_ecc = (RTL_R32(0xecc) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac); 
		if (!(temp_eac & BIT(31)) && !(temp_eac & BIT(30)) &&
			(cal_eb4 != 0x142) && (cal_ebc != 0x42) &&
			(cal_ec4 != 0x132) && (cal_ecc != 0x36)) {
			val_eb4[cal_num] = cal_eb4;
			val_ebc[cal_num] = cal_ebc;
			val_ec4[cal_num] = cal_ec4;
			val_ecc[cal_num] = cal_ecc;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("IQK Path-B Check\n");
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_eb4 = get_mean_of_2_close_value(val_eb4);
		cal_ebc = get_mean_of_2_close_value(val_ebc);
		cal_ec4 = get_mean_of_2_close_value(val_ec4);
		cal_ecc = get_mean_of_2_close_value(val_ecc);

		priv->pshare->RegEB4=cal_eb4;
		priv->pshare->RegEBC=cal_ebc;

		Oldval = (RTL_R32(0xc88) >> 22) & 0x3ff;

		X = cal_eb4;
		PHY_SetBBReg(priv, 0xc88, 0x3ff, X * (Oldval / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(28), ((X * Oldval) >> 7) & 0x1);

		Y = cal_ebc;
		PHY_SetBBReg(priv, 0xc9c, 0xf0000000, ((Y * (Oldval / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc88, 0x003f0000, (Y * (Oldval / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(30), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc1c, 0x3ff, cal_ec4);

		PHY_SetBBReg(priv, 0xc1c, 0xfc00, cal_ecc & 0x3f);

		PHY_SetBBReg(priv, 0xc78, 0xf000, (cal_ecc >> 6) & 0xf);
		}else {
			priv->pshare->RegEB4=0x100;
			priv->pshare->RegEBC=0x00;
	}


	// step 8: back to BB mode, load original values
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0x874, temp_874);
	RTL_W32(0xc08, temp_c08);
	RTL_W32(0xe28, 0);
	RTL_W32(0x840, 0x32ed3);
	RTL_W32(0x844, 0x32ed3);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W32(0x870, temp_870);
		RTL_W32(0x860, temp_860);
		RTL_W32(0x864, temp_864);
	}
#endif
		// return to SI mode
	//RTL_W32(0x820, 0x01000000);
	//RTL_W32(0x828, 0x01000000);

	// step 9: reload ADDA power saving parameters
	RTL_W32(0x85c, temp_85c);
	RTL_W32(0xe6c, temp_e6c);
	RTL_W32(0xe70, temp_e70);
	RTL_W32(0xe74, temp_e74);
	RTL_W32(0xe78, temp_e78);
	RTL_W32(0xe7c, temp_e7c);
	RTL_W32(0xe80, temp_e80);
	RTL_W32(0xe84, temp_e84);
	RTL_W32(0xe88, temp_e88);
	RTL_W32(0xe8c, temp_e8c);
	RTL_W32(0xed0, temp_ed0);
	RTL_W32(0xed4, temp_ed4);
	RTL_W32(0xed8, temp_ed8);
	RTL_W32(0xedc, temp_edc);
	RTL_W32(0xee0, temp_ee0);
	RTL_W32(0xeec, temp_eec);
}	// IQK


static void IQK_88C(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num=0, cal_retry=0;
	unsigned int Oldval_0=0, temp_c04=0, temp_c08=0, temp_874=0;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, temp_eac;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3];
#ifdef HIGH_POWER_EXT_PA
	unsigned int temp_870=0, temp_860=0, temp_864=0;
#endif

	// step 1: save ADDA power saving parameters
	unsigned int temp_85c = RTL_R32(0x85c);
	unsigned int temp_e6c = RTL_R32(0xe6c);
	unsigned int temp_e70 = RTL_R32(0xe70);
	unsigned int temp_e74 = RTL_R32(0xe74);
	unsigned int temp_e78 = RTL_R32(0xe78);
	unsigned int temp_e7c = RTL_R32(0xe7c);
	unsigned int temp_e80 = RTL_R32(0xe80);
	unsigned int temp_e84 = RTL_R32(0xe84);
	unsigned int temp_e88 = RTL_R32(0xe88);
	unsigned int temp_e8c = RTL_R32(0xe8c);
	unsigned int temp_ed0 = RTL_R32(0xed0);
	unsigned int temp_ed4 = RTL_R32(0xed4);
	unsigned int temp_ed8 = RTL_R32(0xed8);
	unsigned int temp_edc = RTL_R32(0xedc);
	unsigned int temp_ee0 = RTL_R32(0xee0);
	unsigned int temp_eec = RTL_R32(0xeec);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		temp_870 = RTL_R32(0x870);
		temp_860 = RTL_R32(0x860);
		temp_864 = RTL_R32(0x864);
	}
#endif

	// step 2: ADDA all on
	RTL_W32(0x85c, 0x0b1b25a0);
	RTL_W32(0xe6c, 0x0bdb25a0);
	RTL_W32(0xe70, 0x0bdb25a0);
	RTL_W32(0xe74, 0x0bdb25a0);
	RTL_W32(0xe78, 0x0bdb25a0);
	RTL_W32(0xe7c, 0x0bdb25a0);
	RTL_W32(0xe80, 0x0bdb25a0);
	RTL_W32(0xe84, 0x0bdb25a0);
	RTL_W32(0xe88, 0x0bdb25a0);
	RTL_W32(0xe8c, 0x0bdb25a0);
	RTL_W32(0xed0, 0x0bdb25a0);
	RTL_W32(0xed4, 0x0bdb25a0);
	RTL_W32(0xed8, 0x0bdb25a0);
	RTL_W32(0xedc, 0x0bdb25a0);
	RTL_W32(0xee0, 0x0bdb25a0);
	RTL_W32(0xeec, 0x0bdb25a0);

	// step 3: start IQK
		// BB switch to PI mode
	//RTL_W32(0x820, 0x01000100);
	//RTL_W32(0x828, 0x01000100);
		//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x00204000);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		PHY_SetBBReg(priv, 0x870, BIT(26), 1);
		PHY_SetBBReg(priv, 0x860, BIT(10), 0);
		PHY_SetBBReg(priv, 0x864, BIT(10), 0);
	}
#endif

	//AP or IQK
//	RTL_W32(0xb68, 0x0f600000);
	RTL_W32(0xb68, 0x00080000);

		// IQK setting
	RTL_W32(0xe28, 0x80800000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
		// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	//RTL_W32(0xe3c, 0x28160502);
	RTL_W32(0xe3c, 0x28160202);

		// LO calibration setting
	RTL_W32(0xe4c, 0x001028d1);

	while (cal_num < 3) {
			// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
			// delay 1ms
		delay_ms(150);

		// step 4: check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_e94 = (RTL_R32(0xe94) >> 16) & 0x3ff;
		cal_e9c = (RTL_R32(0xe9c) >> 16) & 0x3ff;
		cal_ea4 = (RTL_R32(0xea4) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		cal_eac = (temp_eac >> 16) & 0x3ff;
		if (!(temp_eac & BIT(28)) && !(temp_eac & BIT(27)) &&
			(cal_e94 != 0x142) && (cal_e9c != 0x42) &&
			(cal_ea4 != 0x132) && (cal_eac != 0x36)) {
			val_e94[cal_num] = cal_e94;
			val_e9c[cal_num] = cal_e9c;
			val_ea4[cal_num] = cal_ea4;
			val_eac[cal_num] = cal_eac;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("IQK Check\n");
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_e94 = get_mean_of_2_close_value(val_e94);
		cal_e9c = get_mean_of_2_close_value(val_e9c);
		cal_ea4 = get_mean_of_2_close_value(val_ea4);
		cal_eac = get_mean_of_2_close_value(val_eac);

		priv->pshare->RegE94=cal_e94;
		priv->pshare->RegE9C=cal_e9c;

		Oldval_0 = (RTL_R32(0xc80) >> 22) & 0x3ff;

		X = cal_e94;
		PHY_SetBBReg(priv, 0xc80, 0x3ff, X * (Oldval_0 / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval_0) >> 7) & 0x1);

		Y = cal_e9c;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, ((Y * (Oldval_0 / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, (Y * (Oldval_0 / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval_0) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	}else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

		// back to BB mode
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0x874, temp_874);
	RTL_W32(0xc08, temp_c08);
	RTL_W32(0xe28, 0);
	RTL_W32(0x840, 0x00032ed3);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W32(0x870, temp_870);
		RTL_W32(0x860, temp_860);
		RTL_W32(0x864, temp_864);
	}
#endif
		// return to SI mode
	//RTL_W32(0x820, 0x01000000);
	//RTL_W32(0x828, 0x01000000);

	// step 5: reload ADDA power saving parameters
	RTL_W32(0x85c, temp_85c);
	RTL_W32(0xe6c, temp_e6c);
	RTL_W32(0xe70, temp_e70);
	RTL_W32(0xe74, temp_e74);
	RTL_W32(0xe78, temp_e78);
	RTL_W32(0xe7c, temp_e7c);
	RTL_W32(0xe80, temp_e80);
	RTL_W32(0xe84, temp_e84);
	RTL_W32(0xe88, temp_e88);
	RTL_W32(0xe8c, temp_e8c);
	RTL_W32(0xed0, temp_ed0);
	RTL_W32(0xed4, temp_ed4);
	RTL_W32(0xed8, temp_ed8);
	RTL_W32(0xedc, temp_edc);
	RTL_W32(0xee0, temp_ee0);
	RTL_W32(0xeec, temp_eec);
}	// IQK
#endif

void _PHY_SaveADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg,	unsigned int* ADDABackup, unsigned int RegisterNum)
{
	unsigned int	i;
	for( i = 0 ; i < RegisterNum ; i++){
		ADDABackup[i] = PHY_QueryBBReg(priv, ADDAReg[i], bMaskDWord);
	}
}

void _PHY_SaveMACRegisters(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup)
{
	unsigned int	i;
	for( i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++){
		MACBackup[i] = RTL_R8(MACReg[i]);
	}
	MACBackup[i] = RTL_R32( MACReg[i]);
}

void _PHY_ReloadADDARegisters(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, unsigned int*	ADDABackup, unsigned int RegiesterNum)
{
	unsigned int	i;
	for(i = 0 ; i < RegiesterNum; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, ADDABackup[i]);
	}
}

void _PHY_ReloadMACRegisters(struct rtl8192cd_priv *priv,unsigned int* MACReg, unsigned int*	 MACBackup)
{
	unsigned int	i;
	for(i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++){
		RTL_W8( MACReg[i], (unsigned char)MACBackup[i]);
	}
	RTL_W32( MACReg[i], MACBackup[i]);
}

void _PHY_MACSettingCalibration(struct rtl8192cd_priv *priv, unsigned int* MACReg, unsigned int* MACBackup)
{
	unsigned int	i = 0;
	RTL_W8(MACReg[i], 0x3F);
	for(i = 1 ; i < (IQK_MAC_REG_NUM - 1); i++){
		RTL_W8( MACReg[i], (unsigned char)(MACBackup[i]&(~ BIT(3))));
	}
	RTL_W8( MACReg[i], (unsigned char)(MACBackup[i]&(~ BIT(5))));
}

void _PHY_PathAStandBy(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x0);
	PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00010000);
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80800000);
}

void _PHY_PathADDAOn(struct rtl8192cd_priv *priv, unsigned int* ADDAReg, char isPathAOn, char is2T)
{
	unsigned int	pathOn;
	unsigned int	i;

	pathOn = isPathAOn ? 0x04db25a4 : 0x0b1b25a4;
	if(FALSE == is2T){
		pathOn = 0x0bdb25a0;
		PHY_SetBBReg(priv, ADDAReg[0], bMaskDWord, 0x0b1b25a0);
	}
	else{
		PHY_SetBBReg(priv, ADDAReg[0], bMaskDWord, pathOn);
	}

	for( i = 1 ; i < IQK_ADDA_REG_NUM ; i++){
		PHY_SetBBReg(priv, ADDAReg[i], bMaskDWord, pathOn);
	}

}

/*
 *	PA Analog Pre-distortion Calibration R06
 */
static void APK_MAIN(struct rtl8192cd_priv *priv, unsigned int is2T)
{
	unsigned int regD[PATH_NUM];
	unsigned int tmpReg, index, offset, path, i=0, pathbound = PATH_NUM, apkbound=6;
	unsigned int BB_backup[APK_BB_REG_NUM];
	unsigned int BB_REG[APK_BB_REG_NUM] = {0x904, 0xc04, 0x800, 0xc08, 0x874};
	unsigned int BB_AP_MODE[APK_BB_REG_NUM] = {0x00000020, 0x00a05430, 0x02040000, 0x000800e4, 0x00204000};
	unsigned int BB_normal_AP_MODE[APK_BB_REG_NUM] = {0x00000020, 0x00a05430, 0x02040000, 0x000800e4, 0x22204000};
	unsigned int AFE_backup[APK_AFE_REG_NUM];
	unsigned int AFE_REG[APK_AFE_REG_NUM] = {	0x85c, 0xe6c, 0xe70, 0xe74, 0xe78, 0xe7c, 0xe80, 0xe84,
												0xe88, 0xe8c, 0xed0, 0xed4, 0xed8, 0xedc, 0xee0, 0xeec};
	unsigned int MAC_backup[IQK_MAC_REG_NUM];
	unsigned int MAC_REG[IQK_MAC_REG_NUM] = {0x522, 0x550, 0x551, 0x040};
	unsigned int APK_RF_init_value[PATH_NUM][APK_BB_REG_NUM] = {{0x0852c, 0x1852c, 0x5852c, 0x1852c, 0x5852c},
																{0x2852e, 0x0852e, 0x3852e, 0x0852e, 0x0852e}};
	unsigned int		APK_normal_RF_init_value[PATH_NUM][APK_BB_REG_NUM] =
							{	{0x0852c, 0x0a52c, 0x3a52c, 0x5a52c, 0x5a52c},	//path settings equal to path b settings
								{0x0852c, 0x0a52c, 0x5a52c, 0x5a52c, 0x5a52c}	};

	unsigned int		APK_RF_value_0[PATH_NUM][APK_BB_REG_NUM] =
							{	{0x52019, 0x52014, 0x52013, 0x5200f, 0x5208d},
								{0x5201a, 0x52019, 0x52016, 0x52033, 0x52050}	};

	unsigned int		APK_normal_RF_value_0[PATH_NUM][APK_BB_REG_NUM] =
							{	{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a},	//path settings equal to path b settings
								{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a}	};

	unsigned int AFE_on_off[PATH_NUM] = {0x04db25a4, 0x0b1b25a4};	//path A on path B off / path A off path B on
	unsigned int APK_offset[PATH_NUM] = {0xb68, 0xb6c};
	unsigned int APK_normal_offset[PATH_NUM] = {0xb28, 0xb98};
	unsigned int APK_value[PATH_NUM] = {0x92fc0000, 0x12fc0000};
	unsigned int APK_normal_value[PATH_NUM] = {0x92680000, 0x12680000};
	char	APK_delta_mapping[APK_BB_REG_NUM][13] = {{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-6, -4, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-11, -9, -7, -5, -3, -1, 0, 0, 0, 0, 0, 0, 0}};
	unsigned int APK_normal_setting_value_1[13] =
		{	0x01017018, 0xf7ed8f84, 0x1b1a1816, 0x2522201e, 0x322e2b28,
			0x433f3a36, 0x5b544e49, 0x7b726a62, 0xa69a8f84, 0xdfcfc0b3,
			0x12680000, 0x00880000, 0x00880000		};
	unsigned int APK_normal_setting_value_2[16] =
		{	0x01c7021d, 0x01670183, 0x01000123, 0x00bf00e2, 0x008d00a3,
			0x0068007b, 0x004d0059, 0x003a0042, 0x002b0031, 0x001f0025,
			0x0017001b, 0x00110014, 0x000c000f, 0x0009000b, 0x00070008,
			0x00050006	};


	unsigned int APK_normal_RF_init_value_old[PATH_NUM][APK_BB_REG_NUM] =
			{{0x0852c, 0x5a52c, 0x0a52c, 0x5a52c, 0x4a52c}, //path settings equal to path b settings
			 {0x0852c, 0x5a52c, 0x0a52c, 0x5a52c, 0x4a52c}};
	unsigned int APK_normal_RF_value_0_old[PATH_NUM][APK_BB_REG_NUM] =
			{{0x52019, 0x52017, 0x52010, 0x5200d, 0x5200a}, //path settings equal to path b settings
			 {0x52019, 0x52017, 0x52010, 0x5200d, 0x5200a}};
	unsigned int APK_normal_setting_value_1_old[13] =
			{0x01017018, 0xf7ed8f84, 0x40372d20, 0x5b554e48, 0x6f6a6560,
			 0x807c7873, 0x8f8b8884, 0x9d999693, 0xa9a6a3a0, 0xb5b2afac,
			 0x12680000, 0x00880000, 0x00880000};
	unsigned int APK_normal_setting_value_2_old[16] =
			{0x00810100, 0x00400056, 0x002b0032, 0x001f0024, 0x0019001c,
			 0x00150017, 0x00120013, 0x00100011, 0x000e000f, 0x000c000d,
			 0x000b000c, 0x000a000b, 0x0009000a, 0x00090009, 0x00080008,
			 0x00080008};
	unsigned int AP_curve[PATH_NUM][APK_CURVE_REG_NUM];

	unsigned int APK_result[PATH_NUM][APK_BB_REG_NUM];	//val_1_1a, val_1_2a, val_2a, val_3a, val_4a
	unsigned int ThermalValue = 0;
	int BB_offset, delta_V, delta_offset;
	int newVerAPK = (IS_UMC_A_CUT_88C(priv)) ? 1 : 0;
	unsigned int *pAPK_normal_setting_value_1 = APK_normal_setting_value_1, *pAPK_normal_setting_value_2 = APK_normal_setting_value_2 ;
#ifdef HIGH_POWER_EXT_PA
	unsigned int tmp0x870=0, tmp0x860=0, tmp0x864=0;

	if(priv->pshare->rf_ft_var.use_ext_pa) 
		newVerAPK = 1;
#endif

	if(!newVerAPK) {
		apkbound = 12;
		pAPK_normal_setting_value_1 = APK_normal_setting_value_1_old;
		pAPK_normal_setting_value_2 = APK_normal_setting_value_2_old;
	}

	if(!is2T)
		pathbound = 1;

	for(index = 0; index < PATH_NUM; index ++) {
		APK_offset[index] = APK_normal_offset[index];
		APK_value[index] = APK_normal_value[index];
		AFE_on_off[index] = 0x6fdb25a4;
	}

	for(index = 0; index < APK_BB_REG_NUM; index ++) {
		for(path = 0; path < pathbound; path++) {
			if(newVerAPK) {
				APK_RF_init_value[path][index] = APK_normal_RF_init_value[path][index];
				APK_RF_value_0[path][index] = APK_normal_RF_value_0[path][index];
			} else {
				APK_RF_init_value[path][index] = APK_normal_RF_init_value_old[path][index];
				APK_RF_value_0[path][index] = APK_normal_RF_value_0_old[path][index];
			}

		}
		BB_AP_MODE[index] = BB_normal_AP_MODE[index];
	}

	/*
	 *	save BB default value
	 */
	for(index = 1; index < APK_BB_REG_NUM ; index++)
		BB_backup[index] = PHY_QueryBBReg(priv, BB_REG[index], bMaskDWord);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		tmp0x870 = PHY_QueryBBReg(priv, 0x870, bMaskDWord);
		tmp0x860 = PHY_QueryBBReg(priv, 0x860, bMaskDWord);
		tmp0x864 = PHY_QueryBBReg(priv, 0x864, bMaskDWord);
	}
#endif

		//save MAC default value
	_PHY_SaveMACRegisters(priv, MAC_REG, MAC_backup);

	//save AFE default value
	_PHY_SaveADDARegisters(priv, AFE_REG, AFE_backup, APK_AFE_REG_NUM);

	for(path = 0; path < pathbound; path++) {
		/*
		 *	save old AP curve
		 */
		if(path == RF92CD_PATH_A) {
			/*
			 *	path A APK
			 *	load APK setting
			 *	path-A
			 */
			offset = 0xb00;
			for(index = 0; index < 11; index ++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xb98, bMaskDWord, 0x12680000);

			offset = 0xb68;
			for(; index < 13; index ++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}

			/*
			 *	page-B1
			 */
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x40000000);

			/*
			 *path A
			 */
			offset = 0xb00;
			for(index = 0; index < 16; index++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_2[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);
		} else if(path == RF92CD_PATH_B) {
			/*
			 *	path B APK
			 *	load APK setting
			 *	path-B
			 */
			offset = 0xb70;
			for(index = 0; index < 10; index ++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xb28, bMaskDWord, 0x12680000);
			PHY_SetBBReg(priv, 0xb98, bMaskDWord, 0x12680000);

			offset = 0xb68;
			index = 11;
			for(; index < 13; index ++) {
				//offset 0xb68, 0xb6c
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}

			/*
			 *	page-B1
			 */
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x40000000);

			/*
			 *	path B
			 */
			offset = 0xb60;
			for(index = 0; index < 16; index++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_2[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);
		}

		if(!newVerAPK) {
			tmpReg = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x3, bMaskDWord, 1);

			AP_curve[path][0] = tmpReg & 0x1F;				//[4:0]

			tmpReg = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord, 1);
			AP_curve[path][1] = (tmpReg & 0xF8000) >> 15; 	//[19:15]
			AP_curve[path][2] = (tmpReg & 0x7C00) >> 10;	//[14:10]
			AP_curve[path][3] = (tmpReg & 0x3E0) >> 5;		//[9:5]
		}

		/*
		 *	save RF default value
		 */
		regD[path] = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xd, bMaskDWord, 1);

		/*
		 *	Path A AFE all on, path B AFE All off or vise versa
		 */
		for(index = 0; index < APK_AFE_REG_NUM ; index++)
			PHY_SetBBReg(priv, AFE_REG[index], bMaskDWord, AFE_on_off[path]);

		/*
		 *	BB to AP mode
		 */
		if(path == RF92CD_PATH_A) {
			for(index = 1; index < APK_BB_REG_NUM ; index++)
				PHY_SetBBReg(priv, BB_REG[index], bMaskDWord, BB_AP_MODE[index]);
		}

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(10), 1);
			PHY_SetBBReg(priv, 0x870, BIT(26), 1);
			PHY_SetBBReg(priv, 0x860, BIT(10), 0);
			PHY_SetBBReg(priv, 0x864, BIT(10), 0);
		}
#endif

		if(newVerAPK) {
			if(path == RF92CD_PATH_A) {
				PHY_SetBBReg(priv, 0xe30 , bMaskDWord, 0x01008c00);
				PHY_SetBBReg(priv, 0xe34 , bMaskDWord, 0x01008c00);
			} else if(path == RF92CD_PATH_B) {
				PHY_SetBBReg(priv, 0xe50 , bMaskDWord, 0x01008c00);
				PHY_SetBBReg(priv, 0xe54 , bMaskDWord, 0x01008c00);
			}
		}

		//MAC settings
		_PHY_MACSettingCalibration(priv, MAC_REG, MAC_backup);


		if(path == RF92CD_PATH_A) {
			//Path B to standby mode
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x0, bMaskDWord, 0x10000);
		} else {
			//Path A to standby mode
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMaskDWord, 0x10000);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x10, bMaskDWord, 0x1000f);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x11, bMaskDWord, 0x20103);
		}

		/*
		 *	Check Thermal value delta
		 */
		 if (priv->pmib->dot11RFEntry.ther) {
			ThermalValue = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, 0x1f, 1) & 0xff;
			 ThermalValue -= priv->pmib->dot11RFEntry.ther;
		 }

		 delta_offset = ((ThermalValue+14)/2);
		 if(delta_offset < 0)
			 delta_offset = 0;
		 else if (delta_offset > 12)
			 delta_offset = 12;

		//AP calibration
		for(index = 1; index < APK_BB_REG_NUM; index++) {
			tmpReg = APK_RF_init_value[path][index];
			if (priv->pmib->dot11RFEntry.ther) {
				BB_offset = (tmpReg & 0xF0000) >> 16;

				if(!(tmpReg & BIT(15))) //sign bit 0
					BB_offset = -BB_offset;
				delta_V = APK_delta_mapping[index][delta_offset];
				BB_offset += delta_V;

				if(BB_offset < 0) {
					tmpReg = tmpReg & (~BIT(15));
					BB_offset = -BB_offset;
				} else {
					tmpReg = tmpReg | BIT(15);
				}
				tmpReg = (tmpReg & 0xFFF0FFFF) | (BB_offset << 16);
			}

			if(newVerAPK)
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xc, bMaskDWord, 0x8992e);
			else
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xc, bMaskDWord, 0x8992f);

			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x0, bMaskDWord, APK_RF_value_0[path][index]);
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xd, bMaskDWord, tmpReg);

			/*
			 *	PA11+PAD01111, one shot
			 */
			i = 0;
			do {
				PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80000000);
				PHY_SetBBReg(priv, APK_offset[path], bMaskDWord, APK_value[0]);
				delay_ms(3);
				PHY_SetBBReg(priv, APK_offset[path], bMaskDWord, APK_value[1]);
				delay_ms(20);
				PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);

				if(!newVerAPK) {
					tmpReg = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xb, bMaskDWord, 1);
					tmpReg = (tmpReg & 0x3E00) >> 9;
				} else {
					if(path == RF92CD_PATH_A)
						tmpReg = PHY_QueryBBReg(priv, 0xbd8, 0x03E00000);
					else
						tmpReg = PHY_QueryBBReg(priv, 0xbd8, 0xF8000000);
				}
				i++;
			} while((tmpReg > apkbound) && i < 4);

			APK_result[path][index] = tmpReg;
		}
	}

	/*
	 *	reload MAC default value
	 */
	_PHY_ReloadMACRegisters(priv, MAC_REG, MAC_backup);

	/*
	 *	reload BB default value
	 */
	for(index = 1; index < APK_BB_REG_NUM ; index++)
		PHY_SetBBReg(priv, BB_REG[index], bMaskDWord, BB_backup[index]);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, bMaskDWord, tmp0x870);
		PHY_SetBBReg(priv, 0x860, bMaskDWord, tmp0x860);
		PHY_SetBBReg(priv, 0x864, bMaskDWord, tmp0x864);
	}
#endif

	/*
	 *	reload AFE default value
	 */
	_PHY_ReloadADDARegisters(priv, AFE_REG, AFE_backup, 16);


	/*
	 *	reload RF path default value
	 */
	for(path = 0; path < pathbound; path++) {
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xd, bMaskDWord, regD[path]);
		if(path == RF92CD_PATH_B) {
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x10, bMaskDWord, 0x1000f);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x11, bMaskDWord, 0x20101);
		}

		if(newVerAPK) {
			if (APK_result[path][1] > 6)
				APK_result[path][1] = 6;
		} else {
			if(APK_result[path][1] < 1)
				APK_result[path][1] = 1;
			else if (APK_result[path][1] > 5)
				APK_result[path][1] = 5;

			if(APK_result[path][2] < 2)
				APK_result[path][2] = 2;
			else if (APK_result[path][2] > 6)
				APK_result[path][2] = 6;

			if(APK_result[path][3] < 2)
				APK_result[path][3] = 2;
			else if (APK_result[path][3] > 6)
				APK_result[path][3] = 6;

			if(APK_result[path][4] < 5)
				APK_result[path][4] = 5;
			else if (APK_result[path][4] > 9)
				APK_result[path][4] = 9;
		}
	}

	for(path = 0; path < pathbound; path++) {
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x3, bMaskDWord,
		((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (APK_result[path][1] << 5) | APK_result[path][1]));
		if(newVerAPK) {
			if(path == RF92CD_PATH_A)
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord,
				((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (0x00 << 5) | 0x05));
			else
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord,
				((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (0x02 << 5) | 0x05));
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xe, bMaskDWord,
			((0x08 << 15) | (0x08 << 10) | (0x08 << 5) | 0x08));
		} else {
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord,
			((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (APK_result[path][2] << 5) | APK_result[path][3]));
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xe, bMaskDWord,
			((APK_result[path][4] << 15) | (APK_result[path][4] << 10) | (APK_result[path][4] << 5) | APK_result[path][4]));
		}
	}
}



/*
return FALSE => do IQK again
*/
char _PHY_SimularityCompare(struct rtl8192cd_priv *priv, int result[][8], unsigned char c1, unsigned char c2)
{
	unsigned int		i, j, diff, SimularityBitMap, bound = 0;
	unsigned char		final_candidate[2] = {0xFF, 0xFF};	//for path A and path B
	char				bResult = TRUE, is2T = (GET_CHIP_VER(priv) == VERSION_8192C ? 1 : 0);

	bound = (is2T) ? 8 : 4;
	SimularityBitMap = 0;

	for( i = 0; i < bound; i++ )	{
		diff = (result[c1][i] > result[c2][i]) ? (result[c1][i] - result[c2][i]) : (result[c2][i] - result[c1][i]);
		if (diff > MAX_TOLERANCE)		{
			if((i == 2 || i == 6) && !SimularityBitMap)		{
				if( result[c1][i]+ result[c1][i+1] == 0)
					final_candidate[(i>>2)] = c2;
				else if (result[c2][i]+result[c2][i+1] == 0)
					final_candidate[(i>>2)] = c1;
				else
					 SimularityBitMap |= (1<<i);
			}
			else
				SimularityBitMap |= (1<<i);
		}
	}

	if ( SimularityBitMap == 0)	{
		for( i = 0; i < (bound>>2); i++ )		{
			if(final_candidate[i] != 0xFF) 			{
				for( j = (i<<2); j < ((i+1)<<2)-2; j++)
					result[3][j] = result[final_candidate[i]][j];
				bResult = FALSE;
			}
		}
		return bResult;
	}
	else if (!(SimularityBitMap & 0x0F)) {			//path A OK
		for(i = 0; i < 4; i++)
			result[3][i] = result[c1][i];
		return FALSE;
	}
	else if (!(SimularityBitMap & 0xF0) && is2T) {	//path B OK
		for(i = 4; i < 8; i++)
			result[3][i] = result[c1][i];
		return FALSE;
	}
	else
		return FALSE;

}


//bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
unsigned char _PHY_PathA_IQK(struct rtl8192cd_priv *priv, char	configPathB)
{
	unsigned int regEAC, regE94, regE9C, regEA4;
	unsigned char result = 0x00;

	//path-A IQK setting
//	RTPRINT(FINIT, INIT_IQK, ("Path-A IQK setting!\n"));
	PHY_SetBBReg(priv, 0xe30, bMaskDWord, 0x10008c1f);
	PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x10008c1f);
	PHY_SetBBReg(priv, 0xe38, bMaskDWord, 0x82140102);
	PHY_SetBBReg(priv, 0xe3c, bMaskDWord, ((configPathB |IS_UMC_B_CUT_88C(priv)) ? 0x28160202 : 0x28160502));

#if 1
	//path-B IQK setting
	if(configPathB)	{
		PHY_SetBBReg(priv, 0xe50, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(priv, 0xe54, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(priv, 0xe58, bMaskDWord, 0x82140102);
		PHY_SetBBReg(priv, 0xe5c, bMaskDWord, 0x28160202);
	}
#endif
	//LO calibration setting
	PHY_SetBBReg(priv, 0xe4c, bMaskDWord, 0x001028d1);

	//One shot, path A LOK & IQK
	PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf9000000);
	PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf8000000);

	// delay x ms
	delay_ms(IQK_DELAY_TIME);

	// Check failed
	regEAC = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
	regE94 = PHY_QueryBBReg(priv, 0xe94, bMaskDWord);
	regE9C= PHY_QueryBBReg(priv, 0xe9c, bMaskDWord);
	regEA4= PHY_QueryBBReg(priv, 0xea4, bMaskDWord);

	if(!(regEAC & BIT(28)) &&
		(((regE94 & 0x03FF0000)>>16) != 0x142) &&
		(((regE9C & 0x03FF0000)>>16) != 0x42) )
		result |= 0x01;
	else							//if Tx not OK, ignore Rx
		return result;

	if(!(regEAC & BIT(27)) &&		//if Tx is OK, check whether Rx is OK
		(((regEA4 & 0x03FF0000)>>16) != 0x132) &&
		(((regEAC & 0x03FF0000)>>16) != 0x36))
		result |= 0x02;
	else {
//		RTPRINT(FINIT, INIT_IQK, ("Path A Rx IQK fail!!\n"));
	}

	return result;
}

//bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
unsigned char _PHY_PathB_IQK(struct rtl8192cd_priv *priv)
{
	unsigned int regEAC, regEB4, regEBC, regEC4, regECC;
	unsigned char	result = 0x00;
#if 0
	//path-B IQK setting
	RTPRINT(FINIT, INIT_IQK, ("Path-B IQK setting!\n"));
	PHY_SetBBReg(pAdapter, 0xe50, bMaskDWord, 0x10008c22);
	PHY_SetBBReg(pAdapter, 0xe54, bMaskDWord, 0x10008c22);
	PHY_SetBBReg(pAdapter, 0xe58, bMaskDWord, 0x82140102);
	PHY_SetBBReg(pAdapter, 0xe5c, bMaskDWord, 0x28160202);

	//LO calibration setting
	RTPRINT(FINIT, INIT_IQK, ("LO calibration setting!\n"));
	PHY_SetBBReg(pAdapter, 0xe4c, bMaskDWord, 0x001028d1);
#endif
	//One shot, path B LOK & IQK
//	RTPRINT(FINIT, INIT_IQK, ("One shot, path A LOK & IQK!\n"));
	PHY_SetBBReg(priv, 0xe60, bMaskDWord, 0x00000002);
	PHY_SetBBReg(priv, 0xe60, bMaskDWord, 0x00000000);

	// delay x ms
	delay_ms(IQK_DELAY_TIME);

	// Check failed
	regEAC = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
	regEB4 = PHY_QueryBBReg(priv, 0xeb4, bMaskDWord);
	regEBC= PHY_QueryBBReg(priv, 0xebc, bMaskDWord);
	regEC4= PHY_QueryBBReg(priv, 0xec4, bMaskDWord);
	regECC= PHY_QueryBBReg(priv, 0xecc, bMaskDWord);

	if(!(regEAC & BIT(31)) &&
		(((regEB4 & 0x03FF0000)>>16) != 0x142) &&
		(((regEBC & 0x03FF0000)>>16) != 0x42))
		result |= 0x01;
	else
		return result;

	if(!(regEAC & BIT(30)) &&
		(((regEC4 & 0x03FF0000)>>16) != 0x132) &&
		(((regECC & 0x03FF0000)>>16) != 0x36))
		result |= 0x02;
	else {
//		RTPRINT(FINIT, INIT_IQK, ("Path B Rx IQK fail!!\n"));
	}

	return result;

}

void _PHY_PathAFillIQKMatrix(struct rtl8192cd_priv *priv, char bIQKOK, int	result[][8], unsigned char final_candidate, char bTxOnly)
{
	int	Oldval_0, X, TX0_A, reg;
	int	Y, TX0_C;

	if(final_candidate == 0xFF)
		return;

	else if(bIQKOK) 	{
		Oldval_0 = (PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, 0x3FF, TX0_A);
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(31), ((X* Oldval_0>>7) & 0x1));

		Y = result[final_candidate][1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX0_C = (Y * Oldval_0) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XCTxAFE, 0xF0000000, ((TX0_C&0x3C0)>>6));
		PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, 0x003F0000, (TX0_C&0x3F));
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(29), ((Y* Oldval_0>>7) & 0x1));

		if(bTxOnly)		{
//			RTPRINT(FINIT, INIT_IQK, ("_PHY_PathAFillIQKMatrix only Tx OK\n"));
			return;
		}

		reg = result[final_candidate][2];
		PHY_SetBBReg(priv, rOFDM0_XARxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][3] & 0x3F;
		PHY_SetBBReg(priv, rOFDM0_XARxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][3] >> 6) & 0xF;
		PHY_SetBBReg(priv, 0xca0, 0xF0000000, reg);
	}
}


void _PHY_PathBFillIQKMatrix(struct rtl8192cd_priv *priv, char bIQKOK, int	result[][8], unsigned char final_candidate, char bTxOnly)
{
	int	Oldval_1, X, TX1_A, reg;
	int	Y, TX1_C;

	//RTPRINT(FINIT, INIT_IQK, ("Path B IQ Calibration %s !\n",(bIQKOK)?"Success":"Failed"));

	if(final_candidate == 0xFF)
		return;

	else if(bIQKOK)
	{
		Oldval_1 = (PHY_QueryBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * Oldval_1) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, 0x3FF, TX1_A);
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(27), ((X* Oldval_1>>7) & 0x1));

		Y = result[final_candidate][5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX1_C = (Y * Oldval_1) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XDTxAFE, 0xF0000000, ((TX1_C&0x3C0)>>6));
		PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, 0x003F0000, (TX1_C&0x3F));
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(25), ((Y* Oldval_1>>7) & 0x1));

		if(bTxOnly)
			return;

		reg = result[final_candidate][6];
		PHY_SetBBReg(priv, rOFDM0_XBRxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][7] & 0x3F;
		PHY_SetBBReg(priv, rOFDM0_XBRxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][7] >> 6) & 0xF;
		PHY_SetBBReg(priv, rOFDM0_AGCRSSITable, 0x0000F000, reg);
	}
}

void _PHY_IQCalibrate(struct rtl8192cd_priv *priv, int result[][8], unsigned char t, char is2T)
{
	unsigned int	i;
	unsigned char	PathAOK, PathBOK;
	unsigned int	ADDA_REG[IQK_ADDA_REG_NUM] =  {	0x85c, 0xe6c, 0xe70, 0xe74,
													0xe78, 0xe7c, 0xe80, 0xe84,
													0xe88, 0xe8c, 0xed0, 0xed4,
													0xed8, 0xedc, 0xee0, 0xeec };
	unsigned int	IQK_MAC_REG[IQK_MAC_REG_NUM] = {0x522, 0x550, 0x551, 0x040 };

	char isNormal = IS_TEST_CHIP(priv) ? 0 : 1;
	unsigned int	retryCount = 2;

// Note: IQ calibration must be performed after loading PHY_REG.txt , and radio_a, radio_b.txt


#ifdef MP_TEST
	if(priv->pshare->rf_ft_var.mp_specific)
		retryCount = 9;
#endif

	if(t==0)	{
	 	// Save ADDA parameters, turn Path A ADDA on
	 	_PHY_SaveADDARegisters(priv, ADDA_REG, priv->pshare->ADDA_backup, APK_AFE_REG_NUM);
		_PHY_SaveMACRegisters(priv, IQK_MAC_REG, priv->pshare->IQK_MAC_backup);
	}

 	_PHY_PathADDAOn(priv, ADDA_REG, TRUE, is2T);

	if(t==0)	{
	 	// Store 0xC04, 0xC08, 0x874 vale
	 	priv->pshare->RegC04 = PHY_QueryBBReg(priv, 0xc04, bMaskDWord);
	 	priv->pshare->RegC08 = PHY_QueryBBReg(priv, 0xc08, bMaskDWord);
	 	priv->pshare->Reg874 = PHY_QueryBBReg(priv, 0x874, bMaskDWord);
	}

	PHY_SetBBReg(priv, 0x800, bMaskDWord, (PHY_QueryBBReg(priv, 0x800, bMaskDWord)& ~ BIT(24)));
	PHY_SetBBReg(priv, 0xc04, bMaskDWord, 0x03a05600);
	PHY_SetBBReg(priv, 0xc08, bMaskDWord, 0x000800e4);
	PHY_SetBBReg(priv, 0x874, bMaskDWord, 0x22204000);

	PHY_SetBBReg(priv, 0x870, BIT(10), 1);
	PHY_SetBBReg(priv, 0x870, BIT(26), 1);
	PHY_SetBBReg(priv, 0x860, BIT(10), 0);
	PHY_SetBBReg(priv, 0x864, BIT(10), 0);

	if(is2T) {
		PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00010000);
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00010000);
	}

	//MAC settings
	_PHY_MACSettingCalibration(priv, IQK_MAC_REG, priv->pshare->IQK_MAC_backup);

	//Page B init
	if(isNormal)
		PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x00080000);
	else
		PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x0f600000);

	if(is2T)	{
		if(isNormal)
			PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x00080000);
		else
			PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x0f600000);
	}

	// IQ calibration setting
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80800000);
	PHY_SetBBReg(priv, 0xe40, bMaskDWord, 0x01007c00);
	PHY_SetBBReg(priv, 0xe44, bMaskDWord, 0x01004800);

	for(i = 0 ; i < retryCount ; i++){
		PathAOK = _PHY_PathA_IQK(priv, is2T);
		if(PathAOK == 0x03){
				result[t][0] = (PHY_QueryBBReg(priv, 0xe94, bMaskDWord)&0x3FF0000)>>16;
				result[t][1] = (PHY_QueryBBReg(priv, 0xe9c, bMaskDWord)&0x3FF0000)>>16;
				result[t][2] = (PHY_QueryBBReg(priv, 0xea4, bMaskDWord)&0x3FF0000)>>16;
				result[t][3] = (PHY_QueryBBReg(priv, 0xeac, bMaskDWord)&0x3FF0000)>>16;
			break;
		}
		else if (i == (retryCount-1) && PathAOK == 0x01)	//Tx IQK OK
		{
			result[t][0] = (PHY_QueryBBReg(priv, 0xe94, bMaskDWord)&0x3FF0000)>>16;
			result[t][1] = (PHY_QueryBBReg(priv, 0xe9c, bMaskDWord)&0x3FF0000)>>16;
		}
	}

	if(0x00 == PathAOK){
//		RTPRINT(FINIT, INIT_IQK, ("Path A IQK failed!!\n"));
	}

	if(is2T){
		_PHY_PathAStandBy(priv);

		// Turn Path B ADDA on
		_PHY_PathADDAOn(priv, ADDA_REG, FALSE, is2T);

		for(i = 0 ; i < retryCount ; i++){
			PathBOK = _PHY_PathB_IQK(priv);
			if(PathBOK == 0x03){
//				RTPRINT(FINIT, INIT_IQK, ("Path B IQK Success!!\n"));
				result[t][4] = (PHY_QueryBBReg(priv, 0xeb4, bMaskDWord)&0x3FF0000)>>16;
				result[t][5] = (PHY_QueryBBReg(priv, 0xebc, bMaskDWord)&0x3FF0000)>>16;
				result[t][6] = (PHY_QueryBBReg(priv, 0xec4, bMaskDWord)&0x3FF0000)>>16;
				result[t][7] = (PHY_QueryBBReg(priv, 0xecc, bMaskDWord)&0x3FF0000)>>16;
				break;
			}
			else if (i == (retryCount - 1) && PathBOK == 0x01)	//Tx IQK OK
			{
//				RTPRINT(FINIT, INIT_IQK, ("Path B Only Tx IQK Success!!\n"));
				result[t][4] = (PHY_QueryBBReg(priv, 0xeb4, bMaskDWord)&0x3FF0000)>>16;
				result[t][5] = (PHY_QueryBBReg(priv, 0xebc, bMaskDWord)&0x3FF0000)>>16;
			}
		}

		if(0x00 == PathBOK){
//			RTPRINT(FINIT, INIT_IQK, ("Path B IQK failed!!\n"));
		}
	}

	//Back to BB mode, load original value
//	RTPRINT(FINIT, INIT_IQK, ("IQK:Back to BB mode, load original value!\n"));
	PHY_SetBBReg(priv, 0xc04, bMaskDWord, priv->pshare->RegC04);
	PHY_SetBBReg(priv, 0x874, bMaskDWord, priv->pshare->Reg874);
	PHY_SetBBReg(priv, 0xc08, bMaskDWord, priv->pshare->RegC08);

	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0);

	// Restore RX initial gain
	PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00032ed3);
	
	if(is2T)
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00032ed3);	

	if(t!=0) {
	 	// Reload ADDA power saving parameters
	 	_PHY_ReloadADDARegisters(priv, ADDA_REG, priv->pshare->ADDA_backup, 16);

		// Reload MAC parameters
		_PHY_ReloadMACRegisters(priv, IQK_MAC_REG, priv->pshare->IQK_MAC_backup);
	}
}


void PHY_IQCalibrate_new(struct rtl8192cd_priv *priv)
{
	int				result[4][8];	//last is final result
	unsigned char	i, final_candidate;
	char			bPathAOK, bPathBOK;
	int				RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC, RegTmp = 0;
	char			is12simular, is13simular, is23simular;
	unsigned int 	temp_870, temp_860, temp_864, temp_800;
	
	temp_870 = PHY_QueryBBReg(priv, 0x870, bMaskDWord);
	temp_860 = PHY_QueryBBReg(priv, 0x860, bMaskDWord);
	temp_864 = PHY_QueryBBReg(priv, 0x864, bMaskDWord);
	temp_800 = PHY_QueryBBReg(priv, 0x800, bMaskDWord);

	memset(result, 0, sizeof(result));

	final_candidate = 0xff;
	bPathAOK = FALSE;
	bPathBOK = FALSE;
	is12simular = FALSE;
	is23simular = FALSE;
	is13simular = FALSE;

	for (i=0; i<3; i++)	{
		 _PHY_IQCalibrate(priv, result, i, (GET_CHIP_VER(priv) == VERSION_8192C ? 1 : 0));

		if(i == 1)	{
			is12simular = _PHY_SimularityCompare(priv, result, 0, 1);
			if(is12simular) 			{
				final_candidate = 0;
				break;
			}
		}

		if(i == 2) 		{
			is13simular = _PHY_SimularityCompare(priv, result, 0, 2);
			if(is13simular)		{
				final_candidate = 0;
				break;
			}

			is23simular = _PHY_SimularityCompare(priv, result, 1, 2);
			if(is23simular)
				final_candidate = 1;
			else
			{
				for(i = 0; i < 8; i++)
					RegTmp += result[3][i];

				if(RegTmp != 0)
					final_candidate = 3;
				else
					final_candidate = 0xFF;
			}
		}
	}


	RTL_W32(0x870, temp_870);
	RTL_W32(0x860, temp_860);
	RTL_W32(0x864, temp_864);
	RTL_W32(0x800, temp_800);

	//load 0xe30 IQC default value
	if(GET_CHIP_VER(priv) == VERSION_8188C) {
		RTL_W32(0xe30, 0x01008c00);
		RTL_W32(0xe34, 0x01008c00);
	}

	for (i=0; i<4; i++)	{
		RegE94 = result[i][0];
		RegE9C = result[i][1];
		RegEA4 = result[i][2];
		RegEAC = result[i][3];
		RegEB4 = result[i][4];
		RegEBC = result[i][5];
		RegEC4 = result[i][6];
		RegECC = result[i][7];
		DEBUG_INFO("IQK: RegE94=%lx RegE9C=%lx RegEA4=%lx RegEAC=%lx RegEB4=%lx RegEBC=%lx RegEC4=%lx RegECC=%lx\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC);
	}

	if(final_candidate != 0xff)	{
		priv->pshare->RegE94 = RegE94 = result[final_candidate][0];
		priv->pshare->RegE9C = RegE9C = result[final_candidate][1];
		RegEA4 = result[final_candidate][2];
		RegEAC = result[final_candidate][3];
		priv->pshare->RegEB4 = RegEB4 = result[final_candidate][4];
		priv->pshare->RegEBC = RegEBC = result[final_candidate][5];
		RegEC4 = result[final_candidate][6];
		RegECC = result[final_candidate][7];
		DEBUG_INFO ("IQK: final_candidate is %x\n",final_candidate);
		DEBUG_INFO ("IQK: RegE94=%lx RegE9C=%lx RegEA4=%lx RegEAC=%lx RegEB4=%lx RegEBC=%lx RegEC4=%lx RegECC=%lx\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC);
		bPathAOK = bPathBOK = TRUE;
	}
	else 	{
		priv->pshare->RegE94 = priv->pshare->RegEB4 = 0x100;	//X default value
		priv->pshare->RegE9C = priv->pshare->RegEBC = 0x0;		//Y default value
	}

	if((RegE94 != 0)/*&&(RegEA4 != 0)*/)
		_PHY_PathAFillIQKMatrix(priv, bPathAOK, result, final_candidate, (RegEA4 == 0)? 1 :0);
	if(GET_CHIP_VER(priv) == VERSION_8192C){
		if((RegEB4 != 0)/*&&(RegEC4 != 0)*/)
		_PHY_PathBFillIQKMatrix(priv, bPathBOK, result, final_candidate, (RegEC4 == 0)? 1 :0);
	}
}


void PHY_IQCalibrate(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_NEW_IQK
	PHY_IQCalibrate_new(priv);
#else

	if( IS_UMC_A_CUT_88C(priv)
#ifdef HIGH_POWER_EXT_PA
		||(priv->pshare->rf_ft_var.use_ext_pa) 
#endif
	) {
		 PHY_IQCalibrate_new(priv);
	} else {
		if (GET_CHIP_VER(priv) == VERSION_8192C) 
			IQK_92C(priv);
		else
			IQK_88C(priv);
	}
#endif	
}


void PHY_APCalibrate(struct rtl8192cd_priv *priv)
{
#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
	if(!IS_TEST_CHIP(priv)) 
		APK_MAIN(priv, (GET_CHIP_VER(priv) == VERSION_8192C ? 1 : 0));

}


static void PHY_LCCalibrate(struct rtl8192cd_priv *priv)
{
	unsigned char tmpReg, value_IGI;
	unsigned int LC_Cal;
	int isNormal;

#ifdef TESTCHIP_SUPPORT
	isNormal = (IS_TEST_CHIP(priv)? 0 : 1);
#else
	isNormal = 1;
#endif

	/* Check continuous TX and Packet TX */
	tmpReg = RTL_R8(0xd03);

	if ((tmpReg & 0x70) != 0)			/* Deal with contisuous TX case */
		RTL_W8(0xd03, tmpReg&0x8F);	/* disable all continuous TX */
	else								/* Deal with Packet TX case */
		RTL_W8(0x522, 0xFF);			/* block all queues */

	/* 2. Set RF mode = standby mode */
	if ((tmpReg & 0x70) != 0) {
		/* Path-A */
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x10000);

		/* Path-B */
		if (get_rf_mimo_mode(priv) != MIMO_1T1R)
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x00, bMask20Bits, 0x10000);
	}

	/* 3. Read RF reg18 */
	LC_Cal = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMask12Bits, 1);

	/* 4. Set LC calibration begin */
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, bMask12Bits, LC_Cal|0x08000);

	if (isNormal)
		delay_ms(100);
	else
		delay_ms(3);

	/* Restore original situation */
	if ((tmpReg & 0x70) != 0) {
		/* Deal with contisuous TX case */

		/* Path-A */
		RTL_W8(0xd03, tmpReg);

		/* Restore RF mdoe & RF gain by change IGI to trigger HW tristate */
		value_IGI = (RTL_R8(0xc50) & 0x7F);
		RTL_W8(0xc50, ((value_IGI!=0x30)?0x30:0x31));
		RTL_W8(0xc50, value_IGI);

		/* Path-B */
		if (get_rf_mimo_mode(priv) != MIMO_1T1R) {
			/* Restore RF mdoe & RF gain by change IGI to trigger HW tristate */
			value_IGI = (RTL_R8(0xc58) & 0x7F);
			RTL_W8(0xc58, ((value_IGI!=0x30)?0x30:0x31));
			RTL_W8(0xc58, value_IGI);
		}
	} else {
		/* Deal with Packet TX case */

		RTL_W8(0x522, 0x00);
	}
}


static unsigned int OFDMSwingTable[] = {
	0x7f8001fe, // 0, +6.0dB
	0x788001e2, // 1, +5.5dB
	0x71c001c7, // 2, +5.0dB
	0x6b8001ae, // 3, +4.5dB
	0x65400195, // 4, +4.0dB
	0x5fc0017f, // 5, +3.5dB
	0x5a400169, // 6, +3.0dB
	0x55400155, // 7, +2.5dB
	0x50800142, // 8, +2.0dB
	0x4c000130, // 9, +1.5dB
	0x47c0011f, // 10, +1.0dB
	0x43c0010f, // 11, +0.5dB
	0x40000100, // 12, +0dB
	0x3c8000f2, // 13, -0.5dB
	0x390000e4, // 14, -1.0dB
	0x35c000d7, // 15, -1.5dB
	0x32c000cb, // 16, -2.0dB
	0x300000c0, // 17, -2.5dB
	0x2d4000b5, // 18, -3.0dB
	0x2ac000ab, // 19, -3.5dB
	0x288000a2, // 20, -4.0dB
	0x26000098, // 21, -4.5dB
	0x24000090, // 22, -5.0dB
	0x22000088, // 23, -5.5dB
	0x20000080, // 24, -6.0dB
	0x1e400079, // 25, -6.5dB
	0x1c800072, // 26, -7.0dB
	0x1b00006c, // 27. -7.5dB
	0x19800066, // 28, -8.0dB
	0x18000060, // 29, -8.5dB
	0x16c0005b, // 30, -9.0dB
	0x15800056, // 31, -9.5dB
	0x14400051, // 32, -10.0dB
	0x1300004c, // 33, -10.5dB
	0x12000048, // 34, -11.0dB
	0x11000044, // 35, -11.5dB
	0x10000040, // 36, -12.0dB
};

static unsigned char CCKSwingTable_Ch1_Ch13[][8] = {
{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},	// 0, +0dB
{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	// 1, -0.5dB
{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	// 2, -1.0dB
{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	// 3, -1.5dB
{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	// 4, -2.0dB
{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	// 5, -2.5dB
{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	// 6, -3.0dB
{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	// 7, -3.5dB
{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	// 8, -4.0dB
{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	// 9, -4.5dB
{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	// 10, -5.0dB
{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	// 11, -5.5dB
{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	// 12, -6.0dB
{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	// 13, -6.5dB
{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	// 14, -7.0dB
{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	// 15, -7.5dB
{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	// 16, -8.0dB
{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	// 17, -8.5dB
{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	// 18, -9.0dB
{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 19, -9.5dB
{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 20, -10.0dB
{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 21, -10.5dB
{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 22, -11.0dB
{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	// 23, -11.5dB
{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	// 24, -12.0dB
{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	// 25, -12.5dB
{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	// 26, -13.0dB
{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	// 27, -13.5dB
{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	// 28, -14.0dB
{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	// 29, -14.5dB
{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	// 30, -15.0dB
{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	// 31, -15.5dB
{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01}	// 32, -16.0dB
};

static unsigned char CCKSwingTable_Ch14 [][8]= {
{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},	// 0, +0dB
{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	// 1, -0.5dB
{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	// 2, -1.0dB
{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	// 3, -1.5dB
{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	// 4, -2.0dB
{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	// 5, -2.5dB
{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	// 6, -3.0dB
{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	// 7, -3.5dB
{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	// 8, -4.0dB
{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	// 9, -4.5dB
{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	// 10, -5.0dB
{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 11, -5.5dB
{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 12, -6.0dB
{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	// 13, -6.5dB
{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	// 14, -7.0dB
{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 15, -7.5dB
{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 16, -8.0dB
{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 17, -8.5dB
{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 18, -9.0dB
{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 19, -9.5dB
{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 20, -10.0dB
{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	// 21, -10.5dB
{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	// 22, -11.0dB
{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 23, -11.5dB
{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 24, -12.0dB
{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	// 25, -12.5dB
{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 26, -13.0dB
{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 27, -13.5dB
{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 28, -14.0dB
{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 29, -14.5dB
{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 30, -15.0dB
{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 31, -15.5dB
{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00}	// 32, -16.0dB
};

const int OFDM_TABLE_SIZE= sizeof(OFDMSwingTable)/sizeof(int);
const int CCK_TABLE_SIZE= sizeof(CCKSwingTable_Ch1_Ch13) >>3;
#define abs(a,b) ((a>b) ? (a-b) : (b-a))

int get_CCK_swing_index(struct rtl8192cd_priv *priv)
{
	int TempCCk, index=12, i;
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);

	//Query CCK default setting From 0xa24
	TempCCk = PHY_QueryBBReg(priv, rCCK0_TxFilter2, bMaskDWord)&bMaskCCK;
	TempCCk = cpu_to_le32(TempCCk);
	for(i=0 ; i<CCK_TABLE_SIZE ; i++)		{
		if(channel==14) {
			if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0) {
				index = i;
				break;
			}
		} else {
			if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0) {
				index = i;
				break;
			}
		}
	}
	return index;
}

void set_CCK_swing_index(struct rtl8192cd_priv *priv, short CCK_index)
{
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);

	if(channel !=14) {
		RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13[CCK_index][0]);
		RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13[CCK_index][1]);
		RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13[CCK_index][2]);
		RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13[CCK_index][3]);
		RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13[CCK_index][4]);
		RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13[CCK_index][5]);
		RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13[CCK_index][6]);
		RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13[CCK_index][7]);
	}
	else{
		RTL_W8( 0xa22, CCKSwingTable_Ch14[CCK_index][0]);
		RTL_W8( 0xa23, CCKSwingTable_Ch14[CCK_index][1]);
		RTL_W8( 0xa24, CCKSwingTable_Ch14[CCK_index][2]);
		RTL_W8( 0xa25, CCKSwingTable_Ch14[CCK_index][3]);
		RTL_W8( 0xa26, CCKSwingTable_Ch14[CCK_index][4]);
		RTL_W8( 0xa27, CCKSwingTable_Ch14[CCK_index][5]);
		RTL_W8( 0xa28, CCKSwingTable_Ch14[CCK_index][6]);
		RTL_W8( 0xa29, CCKSwingTable_Ch14[CCK_index][7]);
	}
}

#ifdef HIGH_POWER_EXT_PA
void swingIndexRemap(int *a, int b)
{
	int d = (abs(*a, b) *3)>>1;
	if(*a < b )
		*a = b - d;
	else
		*a = b + d;
}
#endif

unsigned char getThermalValue(struct rtl8192cd_priv *priv)
{
	unsigned char	ThermalValue;
	int sum=0, i=0;
	PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 0x60);
	ThermalValue =(unsigned char)PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 1) & 0x01f;
	priv->pshare->Thermal_idx = (priv->pshare->Thermal_idx+1)%8;
	priv->pshare->Thermal_log[ priv->pshare->Thermal_idx ] = ThermalValue;
	for(i=0; i<8; i++) {
		if(!priv->pshare->Thermal_log[i])
			return ThermalValue;
		sum += priv->pshare->Thermal_log[i];
	}
	return (sum+4)>>3;
}
void tx_power_tracking(struct rtl8192cd_priv *priv)
{
	unsigned char	ThermalValue = 0, delta, delta_LCK, delta_IQK;
	int 			ele_A, ele_D, value32, X, Y, ele_C;
	int			OFDM_index[2]={0,0}, CCK_index;
	int	    		i = 0;
	char			is2T = ((GET_CHIP_VER(priv) == VERSION_8192C) ?1 :0);
	unsigned char		TxPwrLevel[2];
	unsigned char 		channel, OFDM_min_index = 6, rf=1; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		channel=priv->pshare->working_channel;
		if(priv->pshare->mp_txpwr_tracking == FALSE)
			return;
	} else
#endif
	{
		channel = (priv->pmib->dot11RFEntry.dot11channel);
	}

	ThermalValue = getThermalValue(priv);

	rf += is2T;
	if(ThermalValue)	{

		if(!priv->pshare->ThermalValue)	{
			priv->pshare->ThermalValue = priv->pmib->dot11RFEntry.ther;
			priv->pshare->ThermalValue_LCK = ThermalValue;
			priv->pshare->ThermalValue_IQK = ThermalValue;

			//Query OFDM path A default setting
			ele_D = PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord)&bMaskOFDM_D;
			for(i=0; i<OFDM_TABLE_SIZE; i++)	{
				if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))	{
					priv->pshare->OFDM_index[0] = i;
					priv->pshare->OFDM_index0[0] = i;
					break;
				}
			}

			//Query OFDM path B default setting
			if(is2T)	{
				ele_D = PHY_QueryBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord)&bMaskOFDM_D;
				for(i=0; i<OFDM_TABLE_SIZE; i++)			{
					if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))	{
						priv->pshare->OFDM_index[1] = i;
						priv->pshare->OFDM_index0[1] = i;
						break;
					}
				}
			}
			priv->pshare->CCK_index = get_CCK_swing_index(priv);
			priv->pshare->CCK_index0 = priv->pshare->CCK_index;

		}

		delta     = abs(ThermalValue, priv->pshare->ThermalValue);
		delta_LCK = abs(ThermalValue, priv->pshare->ThermalValue_LCK);
		delta_IQK = abs(ThermalValue, priv->pshare->ThermalValue_IQK);

//		printk("Readback Thermal Meter = 0x%lx pre thermal meter 0x%lx EEPROMthermalmeter 0x%lx delta 0x%lx delta_LCK 0x%lx delta_IQK 0x%lx\n",
//			ThermalValue, priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther, delta, delta_LCK, delta_IQK);

		if(delta_LCK > 1)	{
			priv->pshare->ThermalValue_LCK = ThermalValue;
			PHY_LCCalibrate(priv);
		}

		if(delta > 0)	{
			if(ThermalValue > priv->pshare->ThermalValue)	{
				for(i = 0; i < rf; i++)
				 	priv->pshare->OFDM_index[i] -= delta;
				priv->pshare->CCK_index -= delta;
			}
			else	{
				for(i = 0; i < rf; i++)
					priv->pshare->OFDM_index[i] += delta;
				priv->pshare->CCK_index += delta;
			}
			if(ThermalValue > priv->pmib->dot11RFEntry.ther)	{
				for(i = 0; i < rf; i++)
					OFDM_index[i] = priv->pshare->OFDM_index[i]+1;
				CCK_index = priv->pshare->CCK_index+1;
			}
			else		{
				for(i = 0; i < rf; i++)
					OFDM_index[i] = priv->pshare->OFDM_index[i];
				CCK_index = priv->pshare->CCK_index;
			}
#ifdef MP_TEST
			if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
				TxPwrLevel[0] = priv->pshare->mp_txpwr_patha;
				TxPwrLevel[1] = priv->pshare->mp_txpwr_pathb;
			} else
#endif
			{
				TxPwrLevel[0] = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
				TxPwrLevel[1] = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];

				if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
					unsigned char offset = (priv->pmib->dot11RFEntry.pwrdiffHT20[channel-1] & 0x0f);
					TxPwrLevel[0] = COUNT_SIGN_OFFSET(TxPwrLevel[0], offset);
					offset = ((priv->pmib->dot11RFEntry.pwrdiffOFDM[channel-1] & 0xf0) >> 4);
					TxPwrLevel[1] = COUNT_SIGN_OFFSET(TxPwrLevel[1], offset);
				}
			}

//			printk("TxPwrLevel[0]=%d, TxPwrLevel[1]=%d\n", TxPwrLevel[0], TxPwrLevel[1]);
			for(i = 0; i < rf; i++)		{
				if(/*TxPwrLevel[i] >=0 &&*/ TxPwrLevel[i] <=26)	{
					if(ThermalValue > priv->pmib->dot11RFEntry.ther) {
						if (delta < 5)
							OFDM_index[i] -= 1;
						else
							OFDM_index[i] -= 2;
					}
					else if(delta > 5 && ThermalValue < priv->pmib->dot11RFEntry.ther) {
						OFDM_index[i] += 1;
					}
				}
				else if (TxPwrLevel[i] >= 27 && TxPwrLevel[i] <= 32 && ThermalValue > priv->pmib->dot11RFEntry.ther) {
					if (delta < 5)
						OFDM_index[i] -= 1;
					else
						OFDM_index[i] -= 2;
				}
				else if (TxPwrLevel[i] >= 32 && TxPwrLevel[i] <= 38 && ThermalValue > priv->pmib->dot11RFEntry.ther && delta > 5)	{
					OFDM_index[i] -= 1;
				}
#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					OFDM_index[i] = priv->pshare->OFDM_index[i];
					swingIndexRemap(&OFDM_index[i], priv->pshare->OFDM_index0[i]);					
				}
#endif
				if(OFDM_index[i] > OFDM_TABLE_SIZE-1)
					OFDM_index[i] = OFDM_TABLE_SIZE-1;
				else if (OFDM_index[i] < OFDM_min_index)
					OFDM_index[i] = OFDM_min_index;
			}
			i=0;
			{
				if(/*TxPwrLevel[i] >=0 &&*/ TxPwrLevel[i] <=26)		{
					if(ThermalValue > priv->pmib->dot11RFEntry.ther)	{
						if (delta < 5)
							CCK_index -= 1;
						else
							CCK_index -= 2;
					}
					else if(delta > 5 && ThermalValue < priv->pmib->dot11RFEntry.ther)	{
						CCK_index += 1;
					}
				}
				else if (TxPwrLevel[i] >= 27 && TxPwrLevel[i] <= 32 && ThermalValue > priv->pmib->dot11RFEntry.ther){
					if (delta < 5)
						CCK_index -= 1;
					else
						CCK_index -= 2;
				}
				else if (TxPwrLevel[i] >= 32 && TxPwrLevel[i] <= 38 && ThermalValue > priv->pmib->dot11RFEntry.ther && delta > 5){
					CCK_index -= 1;
				}

#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					CCK_index = priv->pshare->CCK_index;
					swingIndexRemap( &CCK_index, priv->pshare->CCK_index0);				
				}
#endif					
				if(CCK_index > CCK_TABLE_SIZE-1)
					CCK_index = CCK_TABLE_SIZE-1;
				else if (CCK_index < 0)
					CCK_index = 0;
			}

			//Adujst OFDM Ant_A according to IQK result
			ele_D = (OFDMSwingTable[(unsigned int)OFDM_index[0]] & 0xFFC00000)>>22;
			X = priv->pshare->RegE94;
			Y = priv->pshare->RegE9C;

			if(X != 0)		{
				if ((X & 0x00000200) != 0)
					X = X | 0xFFFFFC00;
				ele_A = ((X * ele_D)>>8)&0x000003FF;

				//new element C = element D x Y
				if ((Y & 0x00000200) != 0)
					Y = Y | 0xFFFFFC00;
				ele_C = ((Y * ele_D)>>8)&0x000003FF;

				//wirte new elements A, C, D to regC80 and regC94, element B is always 0
				value32 = (ele_D<<22)|((ele_C&0x3F)<<16)|ele_A;
				PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

				value32 = (ele_C&0x000003C0)>>6;
				PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

				value32 = ((X * ele_D)>>7)&0x01;
				PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), value32);

			}	else	{
				PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable[(unsigned int)OFDM_index[0]]);
				PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
				PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), 0x00);
			}


			set_CCK_swing_index(priv, CCK_index);


			if(is2T)
			{
				ele_D = (OFDMSwingTable[(unsigned int)OFDM_index[1]] & 0xFFC00000)>>22;
				X = priv->pshare->RegEB4;
				Y = priv->pshare->RegEBC;

				if(X != 0){
					if ((X & 0x00000200) != 0)	//consider minus
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D)>>8)&0x000003FF;

					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D)>>8)&0x00003FF;

					//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
					value32=(ele_D<<22)|((ele_C&0x3F)<<16) |ele_A;
					PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

					value32 = (ele_C&0x000003C0)>>6;
					PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, value32);

					value32 = ((X * ele_D)>>7)&0x01;
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), value32);

				} else{
					PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable[(unsigned int)OFDM_index[1]]);
					PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), 0x00);
				}
			}
		}

		if(delta_IQK > 3) {
			priv->pshare->ThermalValue_IQK = ThermalValue;
			PHY_IQCalibrate(priv);
		}

		//update thermal meter value
		priv->pshare->ThermalValue = ThermalValue;

	}
}


#ifdef ADD_TX_POWER_BY_CMD
static void assign_txpwr_offset(struct rtl8192cd_priv *priv)
{
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
}
#endif


int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned int opmode;
	unsigned long val32;
	unsigned short val16;
	int i;
	unsigned int fwStatus=0, dwnRetry=5;

	pmib = GET_MIB(priv);
	opmode = priv->pmib->dot11OperationEntry.opmode;

	DBFENTER;

#if 0	// ==========>> later
//#ifdef DW_FW_BY_MALLOC_BUF
	if ((priv->pshare->fw_IMEM_buf = kmalloc(FW_IMEM_SIZE, GFP_ATOMIC)) == NULL) {
		DEBUG_ERR("alloc fw_IMEM_buf failed!\n");
		return -1;
	}
	if ((priv->pshare->fw_EMEM_buf = kmalloc(FW_EMEM_SIZE, GFP_ATOMIC)) == NULL) {
		DEBUG_ERR("alloc fw_EMEM_buf failed!\n");
		return -1;
	}
	if ((priv->pshare->fw_DMEM_buf = kmalloc(FW_DMEM_SIZE, GFP_ATOMIC)) == NULL) {
		DEBUG_ERR("alloc fw_DMEM_buf failed!\n");
		return -1;
	}
#endif


//1 For Test, Firmware Downloading

//	MacConfigBeforeFwDownload(priv);

#if 0 	//	==========>> later
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
			LoadPktSize, PCI_DMA_TODEVICE);

	if(LoadFirmware(priv) == FALSE){
//		panic_printk("Load Firmware Fail!\n");
		panic_printk("Load Firmware check!\n");
		return -1;
	}else {
//		delay_ms(20);
		PRINT_INFO("Load firmware successful! \n");
	}
#endif
//	MacConfigAfterFwDownload(priv);

#ifdef CONFIG_RTL_92D_SUPPORT
	check_chipID_MIMO(priv);
#endif
	MacInit(priv);

//	RTL_W32(AGGLEN_LMT, RTL_R32(AGGLEN_LMT)|(0x0F&AGGLMT_MCS15S_Mask)<<AGGLMT_MCS15S_SHIFT
//		|(0x0F&AGGLMT_MCS15_Mask)<<AGGLMT_MCS15_SHIFT);

//	RTL_W8(AGGR_BK_TIME, 0x10);

	//
	// 2. Initialize MAC/PHY Config by MACPHY_reg.txt
	//
	PHY_ConfigMACWithParaFile(priv);


	//
	// 3. Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	//
#ifdef DRVMAC_LB
	if (!priv->pmib->miscEntry.drvmac_lb)
#endif
	{
		val32 = phy_BB8192CD_Config_ParaFile(priv);
		if (val32)
			return -1;
	}

	// support up to MCS7 for 1T1R, modify rx capability here
	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		pmib->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;
/*
	// Set NAV protection length
	// CF-END Threshold
	if (priv->pmib->dot11OperationEntry.wifi_specific) {
		RTL_W16(NAV_PROT_LEN, 0x80);
//		RTL_W8(CFEND_TH, 0x2);
	}
	else {
		RTL_W16(NAV_PROT_LEN, 0x01C0);
//		RTL_W8(CFEND_TH, 0xFF);
	}
*/
	//
	// 4. Initiailze RF RAIO_A.txt RF RAIO_B.txt
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	// close loopback, normal mode

	// For RF test only from Scott's suggestion
//	RTL_W8(0x27, 0xDB);	//	==========>> ???
//	RTL_W8(0x1B, 0x07); // ACUT

/*
	// set RCR: RX_SHIFT and disable ACF
//	RTL_W8(0x48, 0x3e);
//	RTL_W32(0x48, RTL_R32(0x48) & ~ RCR_ACF  & ~RCR_ACRC32);
	RTL_W16(RCR, RCR_AAP | RCR_APM | RCR_ACRC32);
	RTL_W32(RCR, RTL_R32(RCR) & ~ RCR_ACF  & ~RCR_ACRC32);
	// for debug by victoryman, 20081119
//	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYST_RXFF);
	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYSTS);
*/

#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb) {
		RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		drvmac_loopback(priv);
	}
	else
#endif
	{
//#ifdef CHECK_HANGUP
#if 0
		if (!priv->reset_hangup
#ifdef CLIENT_MODE
				|| (!(OPMODE & WIFI_AP_STATE) &&
						(priv->join_res != STATE_Sta_Bss) && (priv->join_res != STATE_Sta_Ibss_Active))
#endif
			)
#endif
		{
			val32 = phy_RF6052_Config_ParaFile(priv);
			if (val32)
				return -1;

			if(IS_UMC_A_CUT_88C(priv))	{
				PHY_SetRFReg(priv, RF92CD_PATH_A, RF_RX_G1, bMask20Bits, 0x30255);
				PHY_SetRFReg(priv, RF92CD_PATH_A, RF_RX_G2, bMask20Bits, 0x50a00);
			} else if(IS_UMC_B_CUT_88C(priv))	{

				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x1e, bMask20Bits, 0x01 |(PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x1e, bMaskDWord, 1)&0xff0f0));
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x1f, bMask20Bits, 0x200|(PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x1f, bMaskDWord, 1)&0xff0ff));
			}

		}
	}

#ifdef DW_FW_BY_MALLOC_BUF
	kfree(priv->pshare->fw_IMEM_buf);
	kfree(priv->pshare->fw_EMEM_buf);
	kfree(priv->pshare->fw_DMEM_buf);
#endif

/*
	{
		// for test, switch to 40Mhz mode
		unsigned int val_read;
		val_read = PHY_QueryRFReg(priv, 0, 18, bMask20Bits, 1);
		val_read &= ~(BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 0, 18, bMask20Bits, val_read);
		val_read = PHY_QueryRFReg(priv, 1, 18, bMask20Bits, 1);
		val_read &= ~(BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 1, 18, bMask20Bits, val_read);

		RTL_W8(0x800,RTL_R8(0x800)|0x1);
		RTL_W8(0x800,RTL_R8(0x900)|0x1);

		RTL_W8(0xc04 , 0x33);
		RTL_W8(0xd04, 0x33);

	}
*/

	/*---- Set CCK and OFDM Block "ON"----*/
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);

	MacConfig(priv);

	/*
	 * 	Force CCK CCA for High Power products
	 */
	if (priv->pshare->rf_ft_var.use_ext_lna)
		RTL_W8(0xa0a, 0xcd);

//	RTL_W8(BW_OPMODE, BIT(2)); // 40Mhz:0 20Mhz:1
//	RTL_W32(MACIDR,0x0);

	// under loopback mode
//	RTL_W32(MACIDR,0xffffffff);		//	need to confirm
/*
#ifdef CONFIG_NET_PCI
	if (IS_PCIBIOS_TYPE)
		pci_unmap_single(priv->pshare->pdev, priv->pshare->cmdbuf_phyaddr,
			(LoadPktSize), PCI_DMA_TODEVICE);
#endif
*/
#if	0
//	RTL_W32(0x230, 0x40000000);	//for test
////////////////////////////////////////////////////////////

	printk("init_hw: 1\n");

	RTL_W16(SIFS_OFDM, 0x1010);
	RTL_W8(SLOT_TIME, 0x09);

	RTL_W8(MSR, MSR_AP);

//	RTL_W8(MSR,MSR_INFRA);
	// for test, loopback
//	RTL_W8(MSR, MSR_NOLINK);
//	RTL_W8(LBKMD_SEL, BIT(0)| BIT(1) );
//	RTL_W16(LBDLY, 0xffff);

	//beacon related
	RTL_W16(BCN_INTERVAL, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	RTL_W16(ATIMWND, 2); //0
	RTL_W16(BCN_DRV_EARLY_INT, 10<<4); // 2
	RTL_W16(BCN_DMATIME, 256); // 0xf
	RTL_W16(SIFS_OFDM, 0x0e0e);
	RTL_W8(SLOT_TIME, 0x10);

//	CamResetAllEntry(priv);
	RTL_W16(SECR, 0x0000);

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
//	if (OPMODE & WIFI_AP_STATE)
//		RTL_W16(BCNTCFG, 0x000a);
//	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
//	RTL_W16(BCNTCFG, 0x0204);


	init_beacon(priv);

	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | /*IMR_RXCMDOK | IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
	IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/) ;// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;

//	priv->pshare->InterruptMask = IMR_ROK| IMR_BDOK | IMR_BcnInt | IMR_MGNTDOK | IMR_TBDOK | IMR_RDU ;
//	priv->pshare->InterruptMask  = 0;
	priv->pshare->InterruptMaskExt = 0;
	RTL_W32(IMR, priv->pshare->InterruptMask);
	RTL_W32(IMR+4, priv->pshare->InterruptMaskExt);

//////////////////////////////////////////////////////////////
	printk("end of init hw\n");

	return 0;

#endif
// clear TPPoll
//	RTL_W16(TPPoll, 0x0);
// Should 8192SE do this initialize? I don't know yet, 080812. Joshua
	// PJ 1-5-2007 Reset PHY parameter counters
//	RTL_W32(0xD00, RTL_R32(0xD00)|BIT(27));
//	RTL_W32(0xD00, RTL_R32(0xD00)&(~(BIT(27))));
/*
	// configure timing parameter
	RTL_W8(ACK_TIMEOUT, 0x30);
	RTL_W8(PIFS_TIME,0x13);
//	RTL_W16(LBDLY, 0x060F);
//	RTL_W16(SIFS_OFDM, 0x0e0e);
//	RTL_W8(SLOT_TIME, 0x10);
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else { // WIRELESS_11B
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x14);
	}
*/
	init_EDCA_para(priv, pmib->dot11BssType.net_work_type);


	// we don't have EEPROM yet, Mark this for FPGA Platform
//	RTL_W8(_9346CR_, CR9346_CFGRW);

//	92SE Windows driver does not set the PCIF as 0x77, seems HW bug?
	// Set Tx and Rx DMA burst size
//	RTL_W8(PCIF, 0x77);
	// Enable byte shift
//	RTL_W8(_PCIF_+2, 0x01);

/*
	// Retry Limit
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		val16 = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 = 0x30;
		else
			val16 = 0x06;
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		val16 |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 |= (0x30 << 8);
		else
			val16 |= (0x06 << 8);
	}
	RTL_W16(RETRY_LIMIT, val16);

	This should be done later, but Windows Driver not done yet.
	// Response Rate Set
	// let ACK sent by highest of 24Mbps
	val32 = 0x1ff;
	if (pmib->dot11RFEntry.shortpreamble)
		val32 |= BIT(23);
	RTL_W32(_RRSR_, val32);
*/




//	panic_printk("0x2c4 = bitmap = 0x%08x\n", (unsigned int)val32);
//	panic_printk("0x2c0 = cmd | macid | band = 0x%08x\n", 0xfd0000a2 | (1<<9 | (sta_band & 0xf))<<8);
//	panic_printk("Add id %d val %08x to ratr\n", 0, (unsigned int)val32);

/*	for (i = 0; i < 8; i++)
		RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
*/

	//settting initial rate for control frame to 24M
//	RTL_W8(INIRTSMCS_SEL, 8);	//	==========>> later

	//setting MAR
	RTL_W32(MAR, 0xffffffff);
	RTL_W32((MAR+4), 0xffffffff);

	//setting BSSID, not matter AH/AP/station
	memcpy((void *)&val32, (pmib->dot11OperationEntry.hwaddr), 4);
	memcpy((void *)&val16, (pmib->dot11OperationEntry.hwaddr + 4), 2);
	RTL_W32(BSSIDR, cpu_to_le32(val32));
	RTL_W16((BSSIDR + 4), cpu_to_le16(val16));
//	RTL_W32(BSSIDR, 0x814ce000);
//	RTL_W16((BSSIDR + 4), 0xee92);

	//setting TCR
	//TCR, use default value

	//setting RCR // set in MacConfigAfterFwDownload
//	RTL_W32(_RCR_, _APWRMGT_ | _AMF_ | _ADF_ | _AICV_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
//	if (pmib->dot11OperationEntry.crc_log)
//		RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACRC32_);

	// setting network type
	if (opmode & WIFI_AP_STATE)
	{
		DEBUG_INFO("AP-mode enabled...\n");

#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence )
#else
		if (priv->pmib->dot11WdsInfo.wdsPure)
#endif
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		else
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
// Move init beacon after f/w download
#if 0
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
#endif
	}
#ifdef CLIENT_MODE
	else if (opmode & WIFI_STATION_STATE)
	{
		DEBUG_INFO("Station-mode enabled...\n");
		RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_INFRA & NETYPE_Mask) << NETYPE_SHIFT));
	}
	else if (opmode & WIFI_ADHOC_STATE)
	{
		DEBUG_INFO("Adhoc-mode enabled...\n");
		RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
	}
#endif
	else
	{
		printk("Operation mode error!\n");
		return 2;
	}

	CamResetAllEntry(priv);
	RTL_W16(SECCFG, 0);
	if ((OPMODE & (WIFI_AP_STATE|WIFI_STATION_STATE|WIFI_ADHOC_STATE)) &&
		!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		pmib->dot11GroupKeysTable.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			i = 13;
#ifdef USE_WEP_DEFAULT_KEY
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			&priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
		RTL_W16(SECCFG, RTL_R16(SECCFG) | NOSKMC);	// no search multicast
#else
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
#else
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], i);
#endif
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
#endif
	}

// for debug
#if 0
// when hangup reset, re-init TKIP/AES key in ad-hoc mode
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_ADHOC_STATE) && pmib->dot11OperationEntry.keep_rsnie &&
		(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
		DOT11_SET_KEY Set_Key;
		Set_Key.KeyType = DOT11_KeyType_Group;
		Set_Key.EncType = pmib->dot11GroupKeysTable.dot11Privacy;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
	}
	else
#endif
//-------------------------------------- david+2006-06-30
	// restore group key if it has been set before open, david
	if (pmib->dot11GroupKeysTable.keyInCam) {
		int retVal;
		retVal = CamAddOneEntry(priv, (unsigned char *)"\xff\xff\xff\xff\xff\xff",
					pmib->dot11GroupKeysTable.keyid,
					pmib->dot11GroupKeysTable.dot11Privacy<<2,
					0, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
		if (retVal)
			priv->pshare->CamEntryOccupied++;
		else {
			DEBUG_ERR("Add group key failed!\n");
		}
	}
#endif
	//here add if legacy WEP
	// if 1x is enabled, do not set default key, david
//#if 0	// marked by victoryman, use pairwise key at present, 20070627
#ifdef USE_WEP_DEFAULT_KEY
#ifdef MBSSID
	if (!(OPMODE & WIFI_AP_STATE) || !priv->pmib->miscEntry.vap_enable)
#endif
	{
		if(!SWCRYPTO && !IEEE8021X_FUN &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_))
			init_DefaultKey_Enc(priv, NULL, 0);
	}
#endif


	//Setup Beacon Interval/interrupt interval, ATIM-WIND ATIM-Interrupt
	RTL_W32(MBSSID_BCN_SPACE, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	//Setting BCNITV is done by firmware now
//	set_fw_reg(priv, (0xF1000000 | (pmib->dot11StationConfigEntry.dot11BeaconPeriod << 8)), 0, 0);
	// Set max AMPDU aggregation time
//	int max_aggre_time = 0xc0; // in 4us
//	set_fw_reg(priv, (0xFD0000B1|((max_aggre_time << 8) & 0xff00)), 0 ,0);

//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	delay_ms(1);
//	while(RTL_R32(0x2A0) != 0){};
	RTL_W16(TBTT_PROHIBIT, 0xc804);
	RTL_W8(DRVERLYINT, 10);
	RTL_W8(BCNDMATIM, 1);
	RTL_W16(ATIMWND, 5);
/*
	if (!((OPMODE & WIFI_AP_STATE)
#if defined(WDS) && defined(CONFIG_RTK_MESH)
		&& ((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure) || priv->pmib->dot1180211sInfo.meshSilence))
#elif defined(WDS)
		&& priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure )
#elif defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		&& priv->pmib->dot1180211sInfo.meshSilence )
#else
		)
#endif
	)

		RTL_W16(BCN_DRV_EARLY_INT, RTL_R16(BCN_DRV_EARLY_INT)|BIT(15)); // sw beacon
*/
#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable && RTL8192CD_NUM_VWLAN == 1 &&
					priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 30)
		//RTL_W16(BCN_DRV_EARLY_INT, (RTL_R16(BCN_DRV_EARLY_INT)&0xf00f) | ((6<<4)&0xff0));
		RTL_W8(DRVERLYINT, 6);
#endif

	if (IS_TEST_CHIP(priv)) {
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
		RTL_W8(BCN_CTRL, EN_BCN_FUNCTION);
	//	RTL_W16(BCN_DMATIME, 0x400); // 1ms

	// for debug
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_ADHOC_STATE)
			RTL_W8(BCN_MAX_ERR, 0xff);
#endif
	} else {
		RTL_W8(BCN_CTRL, DIS_TSF_UPDATE_N|  DIS_SUB_STATE_N  );
		RTL_W8(BCN_MAX_ERR, 0xff);
		RTL_W16(0x518, 0);
		RTL_W8(0x553, 3);
		if(OPMODE & WIFI_STATION_STATE)
			RTL_W8(0x422, RTL_R8(0x422)^BIT(6));

#ifdef MP_TEST
		if (!priv->pshare->rf_ft_var.mp_specific)
#endif
		RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | EN_BCN_FUNCTION | EN_TXBCN_RPT  );
	}


//--------------

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
	if (OPMODE & WIFI_AP_STATE)
		RTL_W16(BCNTCFG, 0x0001);
	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
	RTL_W16(BCNTCFG, 0x0204);


#if 1
	// Ack ISR, and then unmask IMR
#if 0
	RTL_W32(ISR, RTL_R32(ISR));
	RTL_W32(ISR+4, RTL_R16(ISR+4));
	RTL_W32(IMR, 0x0);
	RTL_W32(IMR+4, 0x0);
	priv->pshare->InterruptMask = _ROK_ | _BCNDMAINT_ | _RDU_ | _RXFOVW_ | _RXCMDOK_;
	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
	IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;
#endif
	//priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RDU | HIMR_RXFOVW;
	priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RXFOVW;
#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		priv->pshare->InterruptMask	|= HIMR_BEDOK;
#endif
	priv->pshare->InterruptMaskExt = 0;

	if (opmode & WIFI_AP_STATE)
		priv->pshare->InterruptMask |= HIMR_BCNDOK0 | HIMR_TXBCNERR;
#ifdef CLIENT_MODE
	else if (opmode & WIFI_ADHOC_STATE)
		priv->pshare->InterruptMaskExt |= (HIMR_TXBCNERR | HIMR_TXBCNOK);
#endif

#endif
	// FGPA does not have eeprom now
//	RTL_W8(_9346CR_, 0);
/*
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
			LoadPktSize, PCI_DMA_TODEVICE);
*/

	rtl8192cd_ReadFwHdr(priv);

	while(dwnRetry-- && !fwStatus) {
		fwStatus = LoadFirmware(priv);
		delay_ms(20);
	};
	if(fwStatus) {
		DEBUG_INFO("Load firmware successful!\n");
	}
	else {
		DEBUG_INFO("Load firmware check!\n");
#ifdef PCIE_POWER_SAVING
		priv->pshare->rf_ft_var.power_save &= ~( L1_en|L2_en);
#endif
	}


/*
	MacConfigAfterFwDownload(priv);
*/

	// Adaptive Rate Table for Basic Rate
	val32 = 0;
	for (i=0; i<32; i++) {
		if (AP_BSSRATE[i]) {
			if (AP_BSSRATE[i] & 0x80)
				val32 |= get_bit_value_from_ieee_value(AP_BSSRATE[i] & 0x7f);
		}
	}
	val32 |= (priv->pmib->dot11nConfigEntry.dot11nBasicMCS << 12);

	{
		unsigned int delay_count = 10;
		do {
			if (!is_h2c_buf_occupy(priv))
				break;
			delay_us(5);
			delay_count--;
		} while (delay_count);
	}

	set_RATid_cmd(priv, 0, ARFR_BMC, val32);

//	kfree(priv->pshare->txcmd_buf);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	if (opmode & WIFI_AP_STATE)
	{
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
	}

	//enable interrupt
	RTL_W32(HIMR, priv->pshare->InterruptMask);
//	RTL_W32(IMR+4, priv->pshare->InterruptMaskExt);
//	RTL_W32(IMR, 0xffffffff);
//	RTL_W8(IMR+4, 0x3f);

	// ===========================================================================================

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
	else
#endif
	{
		if (opmode & WIFI_AP_STATE)
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
		else
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
	}

#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
	{
		// get default Tx AGC offset
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
			| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));

		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
			| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
	}
#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif

	if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x1d)) {
		DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
		priv->pmib->dot11RFEntry.ther = 0;
	}

/*
	if (opmode & WIFI_AP_STATE)
	{
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
	}
*/
	/*---- Set CCK and OFDM Block "ON"----*/
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
	delay_ms(2);

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
	        RTL_W32(MACID, 0x87654321);
		delay_ms(150);
	}
#endif

	PHY_IQCalibrate(priv);		// IQK_92C IQK_88c
		
#if defined(__LINUX_2_6__)
	REG32(_WDTCNR_) |=  1 << 23;
#endif

	PHY_APCalibrate(priv);		// APK_92C  APK_88C
	PHY_LCCalibrate(priv);

	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

	/*
	 *	Set RF & RRSR depends on band in use
	 */
	if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G |WIRELESS_11N)) {
		if ((priv->pmib->dot11StationConfigEntry.autoRate) || !(priv->pmib->dot11StationConfigEntry.fixedTxRate & 0xf)) {

			if( IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C ) 
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
			else 
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);

//			RTL_W32(RRSR, RTL_R32(RRSR) & ~(0x0c));
		} else {
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
		}
	} else {
		PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
	}

/*
	if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1){// use one PATH for ofdm and 1SS
		Switch_1SS_Antenna(priv, 2);
		Switch_OFDM_Antenna(priv, 2);
	}
*/

	delay_ms(100);

// 2009.09.10
	if (priv->pshare->rf_ft_var.cck_tx_pathB) {
		RTL_W8(0xa07, 0x40);	// 0x80 -> 0x40    CCK path B Tx
		RTL_W8(0xa0b, 0x84);	// 0x88 -> 0x84    CCK path B Tx
	}

	//RTL_W32(0x100, RTL_R32(0x100) | BIT(14)); //for 8190 fw debug

	// init DIG variables
	val32 = 0x40020064;	// 0x20010020
	priv->pshare->threshold0 = (unsigned short)(val32&0x000000FF);
	priv->pshare->threshold1 = (unsigned short)((val32&0x000FFF00)>>8);
	priv->pshare->threshold2 = (unsigned short)((val32&0xFFF00000)>>20);
	priv->pshare->digDownCount = 0;
	priv->pshare->digDeadPoint = 0;
	priv->pshare->digDeadPointHitCount = 0;

// CCK path A Tx
#ifdef CONFIG_POCKET_ROUTER_SUPPORT
	RTL_W8(0xa07, (RTL_R8(0xa07) & 0xbf));
	RTL_W8(0xa0b, (RTL_R8(0xa0b) & 0xfb));
#endif

	RTL_W32(0x350, RTL_R32(0x350) | BIT(26));	// tx status check

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		priv->pmib->dot11RFEntry.trswitch = 1;
		PHY_SetBBReg(priv, 0x870, BIT(10), 0);
		if (GET_CHIP_VER(priv) == VERSION_8192C)
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
	}
#endif

#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)
	priv->pmib->dot11RFEntry.trswitch = 1;
#endif	

	if (priv->pmib->dot11RFEntry.trswitch)
		RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) | TRSW0EN);
	else
		RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) & ~TRSW0EN);

	DBFEXIT;

	return 0;

}


static void rtl8192cd_ReadFwHdr(struct rtl8192cd_priv *priv)
{
	struct __RTL8192C_FW_HDR__ *pFwHdr = NULL;
	unsigned char *swap_arr;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return;
#endif

	swap_arr = kmalloc(RT_8192CD_FIRMWARE_HDR_SIZE, GFP_ATOMIC);
	if (swap_arr == NULL)
		return;

#ifdef TESTCHIP_SUPPORT
	if (IS_TEST_CHIP(priv))
		memcpy(swap_arr, data_rtl8192cfw_start, RT_8192CD_FIRMWARE_HDR_SIZE);
	else
#endif
	{
		if( IS_UMC_A_CUT_88C(priv) )
			memcpy(swap_arr, data_rtl8192cfw_ua_start, RT_8192CD_FIRMWARE_HDR_SIZE);
		else
			memcpy(swap_arr, data_rtl8192cfw_n_start, RT_8192CD_FIRMWARE_HDR_SIZE);
	}

	pFwHdr = (struct __RTL8192C_FW_HDR__ *)swap_arr;
#ifdef _BIG_ENDIAN_
	pFwHdr->signature		= le16_to_cpu(pFwHdr->signature);
	pFwHdr->version		= le16_to_cpu(pFwHdr->version);
	pFwHdr->year		= le16_to_cpu(pFwHdr->year);	// ready on after v33.1
#endif

	priv->pshare->fw_signature	= pFwHdr->signature;
	priv->pshare->fw_category		= pFwHdr->category;
	priv->pshare->fw_function		= pFwHdr->function;
	priv->pshare->fw_version		= pFwHdr->version;
	priv->pshare->fw_sub_version	= pFwHdr->subversion;
	priv->pshare->fw_date_month	= pFwHdr->month;
	priv->pshare->fw_date_day	= pFwHdr->day;
	priv->pshare->fw_date_hour	= pFwHdr->hour;
	priv->pshare->fw_date_minute	= pFwHdr->minute;
	kfree(swap_arr);
/*
	printk("fw_signature: ");
	if (priv->pshare->fw_signature == RTL8192C_TEST_CHIP)
		printk("92C_TEST_CHIP");
	else if (priv->pshare->fw_signature == RTL8188C_TEST_CHIP)
		printk("88C_TEST_CHIP");
	else if (priv->pshare->fw_signature == RTL8192C_MP_CHIP_A)
		printk("92C_MP_CHIP_A");
	else if (priv->pshare->fw_signature == RTL8188C_MP_CHIP_A)
		printk("88C_MP_CHIP_A");
	else if (priv->pshare->fw_signature == RTL8192C_MP_CHIP_B)
		printk("92C_MP_CHIP_B");
	else if (priv->pshare->fw_signature == RTL8188C_MP_CHIP_B)
		printk("88C_MP_CHIP_B");
	printk(", ");

	printk("fw_category: ");
	if (priv->pshare->fw_category == RTL8192C_NIC_PCIE)
		printk("92C_NIC_PCIE");
	else if (priv->pshare->fw_category == RTL8192C_NIC_USB)
		printk("92C_NIC_USB");
	else if (priv->pshare->fw_category == RTL8192C_AP_PCIE)
		printk("92C_AP_PCIE");
	else if (priv->pshare->fw_category == RTL8192C_AP_USB)
		printk("92C_AP_USB");
	printk(", ");

	printk("fw_function: ");
	if (priv->pshare->fw_function == RTL8192C_NIC_NORMAL)
		printk("92C_NIC_NORMAL");
	else if (priv->pshare->fw_function == RTL8192C_NIC_WWLAN)
		printk("92C_NIC_WWLAN");
	else if (priv->pshare->fw_function == RTL8192C_AP_NORMAL)
		printk("92C_AP_NORMAL");
	else if (priv->pshare->fw_function == RTL8192C_AP_SUSPEND)
		printk("92C_AP_SUSPEND");
	printk("\n");

	printk("fw_version: %d.%d, ", priv->pshare->fw_version, priv->pshare->fw_sub_version);
	printk("fw_date: %02x-%02x %02x:%02x\n", priv->pshare->fw_date_month, priv->pshare->fw_date_day,
		priv->pshare->fw_date_hour, priv->pshare->fw_date_minute);
*/
}


static int LoadFirmware(struct rtl8192cd_priv *priv)
{
	int fw_len, wait_cnt=0;
	unsigned int CurPtr=0;
	unsigned int WriteAddr;
	unsigned int Temp;
	unsigned char *ptmp;

#ifdef CONFIG_RTL8672
	printk("val=%x\n", RTL_R8(0x80));
#endif

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return TRUE;
#endif

	DEBUG_INFO("===> LoadFirmware()\n");

#ifdef TESTCHIP_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		ptmp = data_rtl8192cfw_start + 32;
		fw_len = (int)(data_rtl8192cfw_end - ptmp);

	} else
#endif
	{
		if( IS_UMC_A_CUT_88C(priv) ) {
			ptmp = data_rtl8192cfw_ua_start + 32;
			fw_len = (int)(data_rtl8192cfw_ua_end - ptmp);

		} else {
			ptmp = data_rtl8192cfw_n_start + 32;
			fw_len = (int)(data_rtl8192cfw_n_end - ptmp);
		}
	}

	// Disable SIC
	RTL_W8(0x41, 0x40);
	delay_ms(1);

	// Enable MCU
	RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) | 0x04);
	delay_ms(1);

#ifdef CONFIG_RTL8672
	RTL_W8(0x04, RTL_R8(0x04) | 0x02);
	delay_ms(1);  //czyao
#endif

	// Load SRAM
	WriteAddr = 0x1000;
	RTL_W8(0x80, RTL_R8(0x80) | 0x01);
	delay_ms(1);

//	if (IS_TEST_CHIP(priv))
//		RTL_W8(0x82, RTL_R8(0x82) & 0xf7);
//	else
		RTL_W8(0x82, RTL_R8(0x82) & 0xf0);

	delay_ms(1);

	while (CurPtr < fw_len)
	{
		if ((CurPtr+4) > fw_len)
		{
			// Reach the end of file.
			while (CurPtr < fw_len)
			{
				Temp = *(ptmp + CurPtr);
				RTL_W8(WriteAddr, (unsigned char)Temp);
				WriteAddr++;
				CurPtr++;
			}
		}
		else
		{
			// Write FW content to memory.
			Temp = *((unsigned int *)(ptmp + CurPtr));
			Temp = cpu_to_le32(Temp);
			RTL_W32(WriteAddr, Temp);

			WriteAddr += 4;

			if ((IS_TEST_CHIP(priv) == 0) && (WriteAddr == 0x2000)) {
				unsigned char tmp = RTL_R8(0x82);
				tmp += 1;
				WriteAddr = 0x1000;
				RTL_W8(0x82, tmp) ;
				delay_ms(10);
//				printk("\n[CurPtr=%x, 0x82=%x]\n", CurPtr, RTL_R8(0x82));
			}

			CurPtr += 4;
		}
	}

	Temp = RTL_R8(0x80);
	Temp &= 0xfe;
	Temp |= 0x02;
	RTL_W8(0x80, (unsigned char)Temp);
	delay_ms(1);
	RTL_W8(0x81, 0x00);

	DEBUG_INFO("<=== LoadFirmware()\n");

	// check if firmware is ready
	while ((RTL_R8(0x80) & BIT(6)) == 0) {
		wait_cnt++;
		if (wait_cnt > 10)
			return FALSE;
		DEBUG_ERR("8192c firmware not ready\n");
		delay_ms(1);
	}
#ifdef CONFIG_RTL8672
	printk("val=%x\n", RTL_R8(0x80));
#endif

	return TRUE;
}


#define	SET_RTL8192CD_RF_HALT(priv)						\
{ 														\
	unsigned char u1bTmp;								\
														\
	do													\
	{													\
		u1bTmp = RTL_R8(LDOV12D_CTRL);					\
		u1bTmp |= BIT(0); 								\
		RTL_W8(LDOV12D_CTRL, u1bTmp);					\
		RTL_W8(SPS1_CTRL, 0x0);							\
		RTL_W8(TXPAUSE, 0xFF);							\
		RTL_W16(CMDR, 0x57FC);							\
		delay_us(100);									\
		RTL_W16(CMDR, 0x77FC);							\
		RTL_W8(PHY_CCA, 0x0);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x37FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x77FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x57FC);							\
		RTL_W16(CMDR, 0x0000);							\
		u1bTmp = RTL_R8((SYS_CLKR + 1));				\
		if (u1bTmp & BIT(7))							\
		{												\
			u1bTmp &= ~(BIT(6) | BIT(7));				\
			if (!HalSetSysClk8192CD(priv, u1bTmp))		\
			break;										\
		}												\
		RTL_W8(0x03, 0x71);								\
		RTL_W8(0x09, 0x70);								\
		RTL_W8(0x29, 0x68);								\
		RTL_W8(0x28, 0x00);								\
		RTL_W8(0x20, 0x50);								\
		RTL_W8(0x26, 0x0E);								\
	} while (FALSE);									\
}

void FirmwareSelfReset(struct rtl8192cd_priv *priv)
{
	unsigned char u1bTmp;
	unsigned char  Delay = 100;
	if(priv->pshare->fw_version > 0x21)	{
		RTL_W8(REG_HMETFR+3, 0x20);
	 	u1bTmp = RTL_R8( REG_SYS_FUNC_EN+1);
	 	while(u1bTmp& BIT(2)) {
		  	Delay--;
		  	DEBUG_INFO("polling 0x03[2] Delay = %d \n", Delay);
		  	if(Delay == 0)
		   		break;
		  	delay_us(50);
		  	u1bTmp = RTL_R8( REG_SYS_FUNC_EN+1);
		}
		if((u1bTmp& BIT(2)) && (Delay == 0)) {
			DEBUG_ERR("FirmwareSelfReset fail: 0x03 = %x\n", u1bTmp);
		}
	}
}
//Return Value:
//	1: Exception Case. Previous Operation is not terminated.
//	0: Correct Case.
int CheckNoResetHwExceptionCase(struct rtl8192cd_priv *priv)
{
	//Check PON register to decide
	return ( (RTL_R16(SYS_FUNC_EN) & (FEN_MREGEN|FEN_DCORE))==(FEN_MREGEN|FEN_DCORE) );
}

int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv)
{
	RTL_W32(HIMR, 0);
	RTL_W16(HIMRE, 0);
	RTL_W16(HIMRE+2, 0);
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));

		//3 2.) ==== RF Off Sequence ====
		phy_InitBBRFRegisterDefinition(priv);		// preparation for read/write RF register
		RTL_W8(TXPAUSE, 0xff);										// Pause MAC TX queue
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x00);	// disable RF
		RTL_W8(RF_CTRL, 0x00);
		RTL_W8(APSD_CTRL, 0x40);
		RTL_W8(SYS_FUNC_EN, 0xe2);									// reset BB state machine
		RTL_W8(SYS_FUNC_EN, 0xe0);									// reset BB state machine

		//3 3.) ==== Reset digital sequence ====
		if (RTL_R8(MCUFWDL) & BIT(1)) {
			//Make sure that Host Recovery Interrupt is handled by 8051 ASAP.
			RTL_W32(FSIMR, 0);				// clear FSIMR
			RTL_W32(FWIMR, 0x20);			// clear FWIMR except HRCV_INT
			RTL_W32(FTIMR, 0);				// clear FTIMR			
			FirmwareSelfReset(priv);

			//Clear FWIMR to guarantee if 8051 runs in ROM, it is impossible to run FWISR Interrupt handler
			RTL_W32(FWIMR, 0x0);			// clear All FWIMR
		} else {
			//Critical Error.
			//the operation that reset 8051 is necessary to be done by 8051
			DEBUG_ERR("%s %d ERROR: (RTL_R8(MCUFWDL) & BIT(1))=0\n", __FUNCTION__, __LINE__);
			DEBUG_ERR("%s %d ERROR: the operation that reset 8051 is necessary to be done by 8051,%d\n", __FUNCTION__, __LINE__, RTL_R8(MCUFWDL));
		}
		// ==== Reset digital sequence ====
		RTL_W8(SYS_FUNC_EN+1, 0x51);								// reset MCU, MAC register, DCORE
		RTL_W8(MCUFWDL, 0);											// reset MCU ready status

		//3 4.) ==== Disable analog sequence ====
		RTL_W8(AFE_PLL_CTRL, 0x80);			// disable PLL


	if(IS_UMC_B_CUT_88C(priv))
		RTL_W8(SPS0_CTRL, 0x2b);

	else
		RTL_W8(SPS0_CTRL, 0x23);

#ifdef CONFIG_RTL8672
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL)&~BIT(0));		// only for ADSL platform because 40M crystal is only used by WiFi chip // disable XTAL, if No BT COEX
#endif
		RTL_W8(APS_FSMCO+1, 0x10);
		RTL_W8(RSV_CTRL0, 0x0e);				// lock ISO/CLK/Power control register

		//3 5.) ==== interface into suspend ====
//		RTL_W16(APS_FSMCO, (RTL_R16(APS_FSMCO) & 0x00ff) | (0x18 << 8));	// PCIe suspend mode

	return SUCCESS;
}


void SwBWMode(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
	unsigned char regBwOpMode, regRRSR_RSC, nCur40MhzPrimeSC;

	DEBUG_INFO("SwBWMode(): Switch to %s bandwidth\n", bandwidth?"40MHz":"20MHz");

	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;

	//3 <1> Set MAC register
	regBwOpMode = RTL_R8(BWOPMODE);
	regRRSR_RSC = RTL_R8(RRSR+2);

	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		regBwOpMode |= BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		regBwOpMode &= ~BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		regRRSR_RSC = (regRRSR_RSC&0x90) | (nCur40MhzPrimeSC<<5);
		RTL_W8(RRSR+2, regRRSR_RSC);

		// Let 812cd_rx, re-assign value
		if (priv->pshare->is_40m_bw){
			priv->pshare->Reg_RRSR_2 = 0;
			priv->pshare->Reg_81b = 0;
		}
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

	//3 <2> Set PHY related register
	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x0);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x0);
		PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(10), 1);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x1);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x1);
		// Set Control channel to upper or lower. These settings are required only for 40MHz
		PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC>>1));
		PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
		PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(10), 0);
		PHY_SetBBReg(priv, 0x818, (BIT(26)|BIT(27)), (nCur40MhzPrimeSC==2)?1:2);
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

	//3<3> Set RF related register
	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		PHY_SetRFReg(priv, RF92CD_PATH_A, rRfChannel, (BIT(11)|BIT(10)), 1);
		PHY_SetRFReg(priv, RF92CD_PATH_B, rRfChannel, (BIT(11)|BIT(10)), 1);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetRFReg(priv, RF92CD_PATH_A, rRfChannel, (BIT(11)|BIT(10)), 0);
		PHY_SetRFReg(priv, RF92CD_PATH_B, rRfChannel, (BIT(11)|BIT(10)), 0);
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

#if 0
	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
#endif
}


void init_EDCA_para(struct rtl8192cd_priv *priv, int mode)
{
	static unsigned int slot_time, VO_TXOP, VI_TXOP, sifs_time;

     struct ParaRecord EDCA[4];
#ifdef RTL_MANUAL_EDCA
     //unsigned char acm_bitmap;
#endif

	slot_time = 20;
	sifs_time = 10;

	if (mode & WIRELESS_11N)
		sifs_time = 16;

#ifdef RTL_MANUAL_EDCA
	 if( priv->pmib->dot11QosEntry.ManualEDCA ) {
		 memset(EDCA, 0, 4*sizeof(struct ParaRecord));
		 if( OPMODE & WIFI_AP_STATE )
			 memcpy(EDCA, priv->pmib->dot11QosEntry.AP_manualEDCA, 4*sizeof(struct ParaRecord));
		 else
			 memcpy(EDCA, priv->pmib->dot11QosEntry.STA_manualEDCA, 4*sizeof(struct ParaRecord));


		if ((mode & WIRELESS_11N) ||
			(mode & WIRELESS_11G)) {
			slot_time = 9;
		}


		RTL_W32(EDCA_VO_PARA, (EDCA[VO].TXOPlimit << 16) | (EDCA[VO].ECWmax << 12) | (EDCA[VO].ECWmin << 8) | (sifs_time + EDCA[VO].AIFSN * slot_time));
#ifdef WIFI_WMM
		if (QOS_ENABLE)
			RTL_W32(EDCA_VI_PARA, (EDCA[VI].TXOPlimit << 16) | (EDCA[VI].ECWmax << 12) | (EDCA[VI].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));
		else
#endif
			RTL_W32(EDCA_VI_PARA, (EDCA[BE].TXOPlimit << 16) | (EDCA[BE].ECWmax << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));

		RTL_W32(EDCA_BE_PARA, (EDCA[BE].TXOPlimit << 16) | (EDCA[BE].ECWmax << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[BE].AIFSN * slot_time));

		RTL_W32(EDCA_BK_PARA, (EDCA[BK].TXOPlimit << 16) | (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[BK].AIFSN * slot_time));
	}else
#endif //RTL_MANUAL_EDCA
	{
		 memset(EDCA, 0, 4*sizeof(struct ParaRecord));
		 /* copy BE, BK from static data */
		 if( OPMODE & WIFI_AP_STATE )
				 memcpy(EDCA, rtl_ap_EDCA, 2*sizeof(struct ParaRecord));
		 else
				 memcpy(EDCA, rtl_sta_EDCA, 2*sizeof(struct ParaRecord));

		 /* VI, VO apply settings in AG by default */
		 if( OPMODE & WIFI_AP_STATE )
				 memcpy(&EDCA[2], &rtl_ap_EDCA[VI_AG], 2*sizeof(struct ParaRecord));
		 else
				 memcpy(&EDCA[2], &rtl_sta_EDCA[VI_AG], 2*sizeof(struct ParaRecord));

		 if ((mode & WIRELESS_11N) ||
				 (mode & WIRELESS_11G)) {
				 slot_time = 9;
		 } else {
				 /* replace with settings in B */
				 if( OPMODE & WIFI_AP_STATE )
						 memcpy(&EDCA[2], &rtl_ap_EDCA[VI], 2*sizeof(struct ParaRecord));
				 else
						 memcpy(&EDCA[2], &rtl_sta_EDCA[VI], 2*sizeof(struct ParaRecord));
		 }
		 VO_TXOP = EDCA[VO].TXOPlimit;
		 VI_TXOP = EDCA[VI].TXOPlimit;

		 RTL_W32(EDCA_VO_PARA, (VO_TXOP << 16) | (EDCA[VO].ECWmax << 12) | (EDCA[VO].ECWmin << 8) | (sifs_time + EDCA[VO].AIFSN * slot_time));
#ifdef WIFI_WMM
		 if (QOS_ENABLE)
				 RTL_W32(EDCA_VI_PARA, (VI_TXOP << 16) | (EDCA[VI].ECWmax << 12) | (EDCA[VI].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));
		 else
#endif
				 RTL_W32(EDCA_VI_PARA, (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));

		 RTL_W32(EDCA_BE_PARA, ((EDCA[BE].ECWmax) << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[BE].AIFSN * slot_time));
		 RTL_W32(EDCA_BK_PARA, (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[BK].AIFSN * slot_time));


		RTL_W8(ACMHWCTRL, 0x00);
	}

	priv->pshare->iot_mode_enable = 0;
	priv->pshare->iot_mode_VO_exist = 0;
}


#ifdef WIFI_WMM
void IOT_EDCA_switch(struct rtl8192cd_priv *priv, int mode, char enable)
{
	unsigned int slot_time = 20, sifs_time = 10, BE_TXOP = 47, VI_TXOP = 94;
	unsigned int vi_cw_max = 4, vi_cw_min = 3, vi_aifs;

	if ((mode & WIRELESS_11N) && (priv->pshare->ht_sta_num
#ifdef WDS
		|| ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
#endif
		))
		sifs_time = 16;

	if ((mode & WIRELESS_11N) || (mode & WIRELESS_11G)) {
		slot_time = 9;
	} else {
		BE_TXOP = 94;
		VI_TXOP = 188;
	}

	if (priv->pshare->iot_mode_VO_exist) {
		vi_cw_max = 6;
		vi_cw_min = 4;
		vi_aifs = 0x2b;
	} else {
		vi_aifs = (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time);
	}

	RTL_W32(EDCA_VI_PARA, ((VI_TXOP*(1-priv->pshare->iot_mode_VO_exist)) << 16)
		| (vi_cw_max << 12) | (vi_cw_min << 8) | vi_aifs);

	if (!enable) {
		RTL_W32(EDCA_BE_PARA, (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8)
			| (sifs_time + 3 * slot_time));
	} else {
		if (priv->pshare->ht_sta_num
#ifdef WDS
			|| ((OPMODE & WIFI_AP_STATE) && (mode & WIRELESS_11N) &&
			priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
#endif
			) {
/*
			if (priv->pshare->txop_enlarge == 0xf) {
				// is 8192S client
				RTL_W32(EDCA_BE_PARA, ((BE_TXOP*2) << 16) |
							(6 << 12) | (4 << 8) | (sifs_time + slot_time+ 0xf)); // 0xf is 92s circuit delay
				priv->pshare->txop_enlarge = 2;
			}
			else
*/
			if (priv->pshare->txop_enlarge == 0xe) {
				// is intel client, use a different edca value
				RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (4 << 8) | 0x1f);
				priv->pshare->txop_enlarge = 2;
			} else {
				if (get_rf_mimo_mode(priv) == MIMO_2T2R)
					RTL_W32(EDCA_BE_PARA, ((BE_TXOP*priv->pshare->txop_enlarge) << 16) |
						(6 << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
				else
					RTL_W32(EDCA_BE_PARA, ((BE_TXOP*priv->pshare->txop_enlarge) << 16) |
						(5 << 12) | (3 << 8) | (sifs_time + 2 * slot_time));
			}
		} else {
			RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
		}
/*
		if (priv->pmib->dot11OperationEntry.wifi_specific == 2) {
			RTL_W16(NAV_PROT_LEN, 0x01C0);
			RTL_W8(CFEND_TH, 0xFF);
			set_fw_reg(priv, 0xfd000ab0, 0, 0);
		}
*/
	}
}
#endif


#if 0
void setup_timer1(struct rtl8192cd_priv *priv, int timeout)
{
	RTL_W32(_TIMER1_, timeout);
	RTL_W32(_IMR_, RTL_R32(_IMR_) | _TIMEOUT1_);
}


void cancel_timer1(struct rtl8192cd_priv *priv)
{
	RTL_W32(_IMR_, RTL_R32(_IMR_) & ~_TIMEOUT1_);
}


void setup_timer2(struct rtl8192cd_priv *priv, unsigned int timeout)
{
	unsigned int current_value=RTL_R32(_TSFTR_L_);

	if (TSF_LESS(timeout, current_value))
		timeout = current_value+20;

	RTL_W32(_TIMER2_, timeout);
	RTL_W32(_IMR_, RTL_R32(_IMR_) | _TIMEOUT2_);
}


void cancel_timer2(struct rtl8192cd_priv *priv)
{
	RTL_W32(_IMR_, RTL_R32(_IMR_) & ~_TIMEOUT2_);
}
#endif


void check_EDCCA(struct rtl8192cd_priv *priv, short rssi)
{
	if ((priv->pshare->rf_ft_var.edcca_thd) && (priv->pmib->dot11RFEntry.dot11channel==14
		|| priv->pshare->is_40m_bw)) {
		if((rssi > priv->pshare->rf_ft_var.edcca_thd) && (priv->pshare->phw->EDCCA_on == 0)) {
			RTL_W32(0xc4c, 0xfc03fd);
			priv->pshare->phw->EDCCA_on =1;
		} else if( (rssi < priv->pshare->rf_ft_var.edcca_thd-5) && priv->pshare->phw->EDCCA_on) {
			RTL_W32(0xc4c, 0x7f037f);
			priv->pshare->phw->EDCCA_on =0;
		}
	}
}


/*
 * FA statistic functions
 */
#if !defined(CONFIG_RTL_NEW_AUTOCH)
static void reset_FA_reg(struct rtl8192cd_priv *priv)
{
	unsigned char value8;
	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 | 0x08);	// regD00[27]=1 to reset these OFDM FA counters
	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 & 0xF7);	// regD00[27]=0 to start counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 & 0x3F);	// regA2D[7:6]=00 to disable counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x80);	// regA2D[7:6]=10 to enable counting
}
#else
void reset_FA_reg(struct rtl8192cd_priv *priv)
{
	/* cck CCA */
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 2);
	/* cck FA*/
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 2);
	/* ofdm */
	PHY_SetBBReg(priv, 0xd00, BIT(27), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(27), 0);	
}

void hold_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
	/* hold cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 1);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 1);

	/* hold ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 1);
}

void release_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
	/* release cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 0);
	
	/* release ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 0);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 0);
}

void _FA_statistic(struct rtl8192cd_priv *priv)
{
	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	priv->pshare->cck_FA_cnt = (RTL_R8(0xa5b) << 8) + RTL_R8(0xa5c);

	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);
}
#endif

void FA_statistic(struct rtl8192cd_priv *priv)
{
#if !defined(CONFIG_RTL_NEW_AUTOCH)
	signed char value8;

	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	// read the CCK FA counters
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x40);	// regA2D[6]=1 to hold and read the CCK FA counters
	priv->pshare->cck_FA_cnt = RTL_R8(0xa5b);
	priv->pshare->cck_FA_cnt = priv->pshare->cck_FA_cnt << 8;
	priv->pshare->cck_FA_cnt += RTL_R8(0xa5c);

	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);
#else
	hold_CCA_FA_counter(priv);
	_FA_statistic(priv);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);

	release_CCA_FA_counter(priv);
#endif	

	reset_FA_reg(priv);
}


/*
 *
 * DIG related functions
 *
 */

int getIGIFor1RCCA(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF	8

	if (value_IGI < ONERCCA_LOW_TH)
	{
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	}
	else
		return value_IGI;
}


void set_DIG_state(struct rtl8192cd_priv *priv, int state)
{
	int value_IGI;

	if (state) {
		priv->pshare->DIG_on = 1;
		priv->pshare->restore = 0;
	}
	else {
		priv->pshare->DIG_on = 0;
		if (priv->pshare->restore == 0) {
			if (priv->pshare->rf_ft_var.use_ext_lna == 1) {
				value_IGI = 0x2A;
			}
			else {
				value_IGI = 0x20;
			}
			
#if defined(HW_ANT_SWITCH)
			// wirte new initial gain index into regC50/C58
			if (priv->pshare->rf_ft_var.one_path_cca == 0)	{
				RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
				RTL_W8(0xc58, value_IGI | RXDVY_B_EN);						
			} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
				RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
				RTL_W8(0xc58, getIGIFor1RCCA(value_IGI) | RXDVY_B_EN);
			} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
				RTL_W8(0xc50, getIGIFor1RCCA(value_IGI) | RXDVY_A_EN);
				RTL_W8(0xc58, value_IGI | RXDVY_B_EN);						
			}
#else					
			// Write IGI into HW
			if (priv->pshare->rf_ft_var.one_path_cca == 0) 	{
				RTL_W8(0xc50, value_IGI);
				RTL_W8(0xc58, value_IGI);
			} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
				RTL_W8(0xc50, value_IGI);
				RTL_W8(0xc58, getIGIFor1RCCA(value_IGI));
			} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
				RTL_W8(0xc50, getIGIFor1RCCA(value_IGI));
				RTL_W8(0xc58, value_IGI);
			}
#endif
			priv->pshare->restore = 1;
		}
	}
}


void DIG_process(struct rtl8192cd_priv *priv)
{
	#define DEAD_POINT_TH		10000
	#define DOWN_IG_HIT_TH		5
	#define DEAD_POINT_HIT_TH	3

	unsigned char value_IGI;
	signed char value8;

	if (priv->pshare->DIG_on == 1)
	{
		if (priv->pshare->rf_ft_var.use_ext_lna == 1) {
			priv->pshare->FA_upper = 0x3C;
			priv->pshare->FA_lower = 0x2A;
		} else {
			// Reset initial gain upper & lower bounds
			priv->pshare->FA_upper = 0x32;
			priv->pshare->FA_lower = 0x20;

			if (priv->pshare->rssi_min > 30)
				priv->pshare->FA_lower = 0x24;
			else if (priv->pshare->rssi_min > 25)
				priv->pshare->FA_lower = 0x22;
		}

		// determine a new initial gain index according to the sumation of all FA counters as well as upper & lower bounds
		value8 = RTL_R8(0xc50);
		value_IGI = (value8 & 0x7F);

#if  defined(CONFIG_RTL_NOISE_CONTROL_92C)
	if(priv->pshare->rf_ft_var.dnc_enable)
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17; 	
		if(priv->pshare->rf_ft_var.use_ext_lna) {			
			if( (priv->pshare->rssi_min > 50)  )  {				
				if((!priv->pshare->DNC_on) && (value_IGI >= priv->pshare->FA_upper) && (priv->pshare->FA_total_cnt > priv->pshare->threshold2)) {
					priv->pshare->DNC_on = 1;

					priv->ext_stats.tp_average_pre = tp_now;

					priv->pshare->FA_lower = 0x20;						
					PHY_SetBBReg(priv, 0x870, bMaskDWord, RTL_R32(0x870)|BIT(5)|BIT(6)|BIT(21)|BIT(22));

#ifdef HW_ANT_SWITCH
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, priv->pshare->FA_lower | RXDVY_A_EN);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, priv->pshare->FA_lower | RXDVY_B_EN);
#else
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, priv->pshare->FA_lower);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, priv->pshare->FA_lower);
#endif
					
				} else if(priv->pshare->DNC_on ==1)  {
					if(tp_now < priv->ext_stats.tp_average_pre + 2) {
						priv->pshare->DNC_on = 0;			
					} 
					else {
						priv->pshare->DNC_on =2;
						priv->ext_stats.tp_average_pre = tp_now;
					}
				} else if(priv->pshare->DNC_on >= 2 ) {
					if(( tp_now+10 < priv->ext_stats.tp_average_pre ) || (tp_now < 1) ) {
							priv->pshare->DNC_on = 0;
					} else if(priv->pshare->DNC_on<5) {
						priv->ext_stats.tp_average_pre = tp_now;
						++priv->pshare->DNC_on;
					}
				}			
			 }else {
				priv->pshare->DNC_on = 0;
			}
			
			if(	priv->pshare->DNC_on ) 
				return;
			else			
				PHY_SetBBReg(priv, 0x870, bMaskDWord, RTL_R32(0x870)&  ~(BIT(5)|BIT(6)|BIT(21)|BIT(22)));	

		} else {		
			if( (priv->pshare->rssi_min > 40) && (value_IGI >= priv->pshare->FA_upper) )  {
//				unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;			
				if((!priv->pshare->DNC_on) && (priv->pshare->FA_total_cnt > priv->pshare->threshold2)) {
					priv->pshare->DNC_on = 1;
					priv->ext_stats.tp_average_pre = tp_now;
				} else if(priv->pshare->DNC_on ==1)  {
					if(tp_now < priv->ext_stats.tp_average_pre + 2) {
						priv->pshare->DNC_on = 0;				
					} 
					else {
						priv->pshare->DNC_on = 2;
						priv->ext_stats.tp_average_pre = tp_now;
					}
				} else if(priv->pshare->DNC_on >= 2 ) {
					if((tp_now +10 < priv->ext_stats.tp_average_pre )
						|| ((priv->ext_stats.tp_average_pre < 10) && (priv->pshare->FA_total_cnt < priv->pshare->threshold1))) {
						priv->pshare->DNC_on = 0;
					} else 	if(priv->pshare->DNC_on<6) {
						priv->ext_stats.tp_average_pre = tp_now;
						++priv->pshare->DNC_on;
					}
				}			
				if(priv->pshare->DNC_on) {
					priv->pshare->FA_upper = 0x3e;			
				}
			}else {
				priv->pshare->DNC_on = 0;
			}
		}
	}
#endif

		if ((priv->pshare->digDeadPoint == 0) && (priv->pshare->FA_total_cnt > DEAD_POINT_TH))
		{
			if ((priv->pshare->digDeadPointHitCount > 0) && (priv->pshare->digDeadPointCandidate == value_IGI))
			{
				priv->pshare->digDeadPointHitCount++;
				if (priv->pshare->digDeadPointHitCount == DEAD_POINT_HIT_TH)
				{
					priv->pshare->digDeadPoint = priv->pshare->digDeadPointCandidate;
				}
			}
			else
			{
				priv->pshare->digDeadPointCandidate = value_IGI;
				priv->pshare->digDeadPointHitCount = 1;
			}
		}

		if (priv->pshare->FA_total_cnt < priv->pshare->threshold0)
		{
			priv->pshare->digDownCount++;
			if (priv->pshare->digDownCount > DOWN_IG_HIT_TH)
			{
				// Reset deadpoint hit count
				if ((priv->pshare->digDeadPoint == 0) && (priv->pshare->digDeadPointHitCount > 0) && (value_IGI == priv->pshare->digDeadPointCandidate))
					priv->pshare->digDeadPointHitCount = 0;

				value_IGI--;

				// Check if the new value is dead point
				if ((priv->pshare->digDeadPoint > 0) && (value_IGI == priv->pshare->digDeadPoint))
					value_IGI++;
			}
		}
		else if (priv->pshare->FA_total_cnt < priv->pshare->threshold1)
		{
			value_IGI += 0;
			priv->pshare->digDownCount = 0;
		}
		else if (priv->pshare->FA_total_cnt < priv->pshare->threshold2)
		{
			value_IGI++;
			priv->pshare->digDownCount = 0;
		}
		else if (priv->pshare->FA_total_cnt >= priv->pshare->threshold2)
		{
			value_IGI += 2;
			priv->pshare->digDownCount = 0;
		}
		else
			priv->pshare->digDownCount = 0;

		if (value_IGI > priv->pshare->FA_upper)
			value_IGI = priv->pshare->FA_upper;
		else if (value_IGI < priv->pshare->FA_lower)
			value_IGI = priv->pshare->FA_lower;

#if defined(HW_ANT_SWITCH)

		// wirte new initial gain index into regC50/C58
		if (priv->pshare->rf_ft_var.one_path_cca == 0)	{
			RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
			RTL_W8(0xc58, value_IGI | RXDVY_B_EN);						
		} else if (priv->pshare->rf_ft_var.one_path_cca == 1)	{
			RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
			RTL_W8(0xc58, getIGIFor1RCCA(value_IGI) | RXDVY_B_EN);						
		} else if (priv->pshare->rf_ft_var.one_path_cca == 2)		{
			RTL_W8(0xc50, getIGIFor1RCCA(value_IGI) | RXDVY_A_EN);
			RTL_W8(0xc58, value_IGI| RXDVY_B_EN);					
		}
#else
		// Write IGI into HW
		if (priv->pshare->rf_ft_var.one_path_cca == 0) {
			RTL_W8(0xc50, value_IGI);
			RTL_W8(0xc58, value_IGI);
		} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
			RTL_W8(0xc50, value_IGI);
			RTL_W8(0xc58, getIGIFor1RCCA(value_IGI));
		} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
			RTL_W8(0xc50, getIGIFor1RCCA(value_IGI));
			RTL_W8(0xc58, value_IGI);
		}
#endif
	
	}
}


void check_DIG_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	unsigned int dig_on = 0;

	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if ((rssi_strength > priv->pshare->rf_ft_var.digGoUpperLevel)
		&& (rssi_strength < HP_LOWER+1) && (priv->pshare->phw->signal_strength != 2)) {
		if (priv->pshare->is_40m_bw)
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 3)
			dig_on++;

		priv->pshare->phw->signal_strength = 2;
	}
	else if ((rssi_strength > HP_LOWER+5) && (priv->pshare->phw->signal_strength != 3)) {
		if (priv->pshare->is_40m_bw)
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 2)
			dig_on++;

		priv->pshare->phw->signal_strength = 3;
	}
	else if (((rssi_strength < priv->pshare->rf_ft_var.digGoLowerLevel)
		&& (priv->pshare->phw->signal_strength != 1)) || !priv->pshare->phw->signal_strength) {
		// DIG off
#ifndef CONFIG_RTL8672
		set_DIG_state(priv, 0);
#endif

		if (priv->pshare->is_40m_bw)
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		priv->pshare->phw->signal_strength = 1;
	}

	if (dig_on) {
		// DIG on
		set_DIG_state(priv, 1);
	}

	//check_DC_TH_by_rssi(priv, rssi_strength);
}


void DIG_for_site_survey(struct rtl8192cd_priv *priv, int do_ss)
{
	if (do_ss) {
		// DIG off
		set_DIG_state(priv, 0);
	}
	else {
		// DIG on
		if (priv->pshare->phw->signal_strength > 1) {
			set_DIG_state(priv, 1);
		}
	}
}


/*
 * dynamic CCK CCA enhance by rssi
 */
void CCK_CCA_dynamic_enhance(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
#if 1
	unsigned int cck_fa = priv->pshare->FA_total_cnt;
	int rssi_thd = 30;

	if (priv->pshare->rf_ft_var.use_ext_lna)
		rssi_thd += RSSI_DIFF_LNA;
	
	if (rssi_strength == 0xff) {
		if (cck_fa < 1000) {
			if (priv->pshare->phw->CCK_CCA_enhanced != 2) {
				RTL_W8(0xa0a, 0x40);
				priv->pshare->phw->CCK_CCA_enhanced = 2;
			}
		} else {
			if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
				RTL_W8(0xa0a, 0x83);
				priv->pshare->phw->CCK_CCA_enhanced = 1;
			}
		}
		return;
	}

	if (rssi_strength > rssi_thd+5) {
		if (priv->pshare->phw->CCK_CCA_enhanced != 0) {
			RTL_W8(0xa0a, 0xcd);
			priv->pshare->phw->CCK_CCA_enhanced = 0;
		}
	} else if (rssi_strength< rssi_thd) {
		if ((rssi_strength > 9) || (priv->assoc_num >1)) {
			if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
				RTL_W8(0xa0a, 0x83);
				priv->pshare->phw->CCK_CCA_enhanced = 1;
			}
		} else {
			if(cck_fa<1000) {
				if (priv->pshare->phw->CCK_CCA_enhanced != 2) {
					RTL_W8(0xa0a, 0x40);
					priv->pshare->phw->CCK_CCA_enhanced = 2;
				}
			} else {
				if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
					RTL_W8(0xa0a, 0x83);
					priv->pshare->phw->CCK_CCA_enhanced = 1;
				}
			}
		}
	}

#else

	if (rssi_strength == 0xff)
		return;

	if (!priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength < 30)) {
		priv->pshare->phw->CCK_CCA_enhanced = TRUE;
		RTL_W8(0xa0a, 0x83);
	}
	else if (priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength > 35)) {
		priv->pshare->phw->CCK_CCA_enhanced = FALSE;
		RTL_W8(0xa0a, 0xcd);
	}
#endif
}


#if 0
void tx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char enable){

	if((get_rf_mimo_mode(priv) != MIMO_2T2R))
		return; // 1T2R, 1T1R; do nothing

	if(pstat == NULL)
		return;

#ifdef	STA_EXT
	if ((pstat->remapped_aid == FW_NUM_STAT-1) ||
		(priv->pshare->has_2r_sta & BIT(pstat->remapped_aid)))// 2r STA
#else
	if (priv->pshare->has_2r_sta & BIT(pstat->aid))// 2r STA
#endif
		return; // do nothing

	// for debug, by victoryman 20090623
	if (pstat->tx_ra_bitmap & 0xff00000) {
		// this should be a 2r station!!!
		return;
	}

	if (pstat->tx_ra_bitmap & 0xffff000){// 11n 1R client
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		}
		else
			Switch_1SS_Antenna(priv, 3);
  }
	else if (pstat->tx_ra_bitmap & 0xff0){// 11bg client
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		}
		else
			Switch_OFDM_Antenna(priv, 3);
  }

#if 0  // original  setup
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N){ // for 11n 1ss sta
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		}
		else
			Switch_1SS_Antenna(priv, 3);
	}
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G){ // for 11g
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		}
		else
			Switch_OFDM_Antenna(priv, 3);
	}
#endif


}
//#endif


// dynamic Rx path selection by signal strength
void rx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	unsigned char highest_rssi=0, higher_rssi=0, under_ss_th_low=0;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest=0, eRFPath_higher=0;
	int ant_on_processing=0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	if (enable == FALSE) {
		if (priv->pshare->phw->ant_off_num) {
			priv->pshare->phw->ant_off_num = 0;
			priv->pshare->phw->ant_off_bitmap = 0;
			RTL_W8(rOFDM0_TRxPathEnable, 0x0f);
			RTL_W8(rOFDM1_TRxPathEnable, 0x0f);
			DEBUG_INFO("More than 1 sta, turn on all path\n");
		}
		return;
	}

	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath))
			continue;

		if (pstat->rf_info.mimorssi[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->rf_info.mimorssi[eRFPath] > higher_rssi) {
			higher_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_higher = eRFPath;
		}

		if (pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low)
			under_ss_th_low = 1;
	}

	// for OFDM
	if (priv->pshare->phw->ant_off_num > 0)
	{
		for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
			if (!(priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
				continue;

			if (highest_rssi >= priv->pshare->phw->ant_on_criteria[eRFPath]) {
				priv->pshare->phw->ant_off_num--;
				priv->pshare->phw->ant_off_bitmap &= (~BIT(eRFPath));
				RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				DEBUG_INFO("Path %c is on due to >= %d%%\n",
					path_name[eRFPath], priv->pshare->phw->ant_on_criteria[eRFPath]);
				ant_on_processing = 1;
			}
		}
	}

	if (!ant_on_processing)
	{
		if (priv->pshare->phw->ant_off_num < 2) {
			for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
				if ((eRFPath == eRFPath_highest) || (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
					continue;

				if ((pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low) &&
					((highest_rssi - pstat->rf_info.mimorssi[eRFPath]) > priv->pshare->rf_ft_var.diff_th)) {
					priv->pshare->phw->ant_off_num++;
					priv->pshare->phw->ant_off_bitmap |= BIT(eRFPath);
					priv->pshare->phw->ant_on_criteria[eRFPath] = highest_rssi + 5;
					RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					DEBUG_INFO("Path %c is off due to under th_low %d%% and diff %d%%, will be on at %d%%\n",
						path_name[eRFPath], priv->pshare->rf_ft_var.ss_th_low,
						(highest_rssi - pstat->rf_info.mimorssi[eRFPath]),
						priv->pshare->phw->ant_on_criteria[eRFPath]);
					break;
				}
			}
		}
	}

	// For CCK
	if (priv->pshare->rf_ft_var.cck_sel_ver == 1) {
		if (under_ss_th_low && (pstat->rx_pkts > 20)) {
			if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
				priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
				RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
				DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
					path_name[eRFPath_highest], path_name[eRFPath_higher]);
			}
		}
	}
}


// dynamic Rx path selection by signal strength
void rx_path_by_rssi_cck_v2(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int highest_rssi=-1000, higher_rssi=-1000;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest=0, eRFPath_higher=0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (pstat->cck_mimorssi_total[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->cck_mimorssi_total[eRFPath] > higher_rssi) {
			higher_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_higher = eRFPath;
		}
	}

	if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
		priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
		RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
		DEBUG_INFO("CCK rssi A:%d B:%d C:%d D:%d accu %d pkts\n", pstat->cck_mimorssi_total[0],
			pstat->cck_mimorssi_total[1], pstat->cck_mimorssi_total[2], pstat->cck_mimorssi_total[3], pstat->cck_rssi_num);
		DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
			path_name[eRFPath_highest], path_name[eRFPath_higher]);
	}
}


// Tx power control
void tx_power_control(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	if (enable) {
		if (!priv->pshare->phw->lower_tx_power) {
			// TX High power enable
//			set_fw_reg(priv, 0xfd000009, 0, 0);
			if (!priv->pshare->bcnTxAGC)
				RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_HP);
			priv->pshare->phw->lower_tx_power++;

		if ((!priv->pshare->is_40m_bw || (pstat->tx_bw == HT_CHANNEL_WIDTH_20)) &&
			(!pstat->is_rtl8190_sta && !pstat->is_broadcom_sta && !pstat->is_marvell_sta && !pstat->is_intel_sta))
			set_fw_reg(priv, 0xfd004314, 0, 0);
		else
			set_fw_reg(priv, 0xfd000015, 0, 0);
		}
	}
	else {
		if (priv->pshare->phw->lower_tx_power) {
			//TX High power disable
//			set_fw_reg(priv, 0xfd000008, 0, 0);
			RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_HP);
			priv->pshare->phw->lower_tx_power = 0;
		}
	}
}


void tx_power_tracking(struct rtl8192cd_priv *priv)
{
	if (priv->pmib->dot11RFEntry.ther) {
		DEBUG_INFO("TPT: triggered(every %d seconds)\n", priv->pshare->rf_ft_var.tpt_period);

		// enable rf reg 0x24 power and trigger, to get ther value in 1 second
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);
		mod_timer(&priv->pshare->phw->tpt_timer, jiffies + 100); // 1000ms
	}
}


void rtl8192cd_tpt_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int val32;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->pshare->phw->tpt_timer))
		del_timer_sync(&priv->pshare->phw->tpt_timer);

	if (priv->pmib->dot11RFEntry.ther) {
		// query rf reg 0x24[4:0], for thermal meter value
		val32 = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;

		if (val32) {
			set_fw_reg(priv, 0xfd000019|(priv->pmib->dot11RFEntry.ther & 0xff)<<8|val32<<16, 0, 0);
			DEBUG_INFO("TPT: finished once (ther: current=0x%02x, target=0x%02x)\n",
				val32, priv->pmib->dot11RFEntry.ther);
		}
		else{
			DEBUG_WARN("TPT: cannot finish, since wrong current ther value report\n");
		}
	}
}
#endif

#ifdef HIGH_POWER_EXT_PA
void tx_power_control(struct rtl8192cd_priv *priv)
{
	unsigned long x;

	int pwr_value = 0x10101010;
	if( priv->pshare->phw->signal_strength == 3 && priv->pshare->phw->lower_tx_power== 0) {
		SAVE_INT_AND_CLI(x);
		priv->pshare->phw->power_backup[0x00] = RTL_R32(rTxAGC_A_Rate18_06);
		priv->pshare->phw->power_backup[0x01] = RTL_R32(rTxAGC_A_Rate54_24);
		priv->pshare->phw->power_backup[0x02] = RTL_R32(rTxAGC_B_Rate18_06);
		priv->pshare->phw->power_backup[0x03] = RTL_R32(rTxAGC_B_Rate54_24);		
		priv->pshare->phw->power_backup[0x04] = RTL_R32(rTxAGC_A_Mcs03_Mcs00);
		priv->pshare->phw->power_backup[0x05] = RTL_R32(rTxAGC_A_Mcs07_Mcs04);
		priv->pshare->phw->power_backup[0x06] = RTL_R32(rTxAGC_A_Mcs11_Mcs08);
		priv->pshare->phw->power_backup[0x07] = RTL_R32(rTxAGC_A_Mcs15_Mcs12);		
		priv->pshare->phw->power_backup[0x08] = RTL_R32(rTxAGC_B_Mcs03_Mcs00);
		priv->pshare->phw->power_backup[0x09] = RTL_R32(rTxAGC_B_Mcs07_Mcs04);
		priv->pshare->phw->power_backup[0x0a] = RTL_R32(rTxAGC_B_Mcs11_Mcs08);
		priv->pshare->phw->power_backup[0x0b] = RTL_R32(rTxAGC_B_Mcs15_Mcs12);		
		priv->pshare->phw->power_backup[0x0c] = RTL_R32(rTxAGC_A_CCK11_2_B_CCK11);
		priv->pshare->phw->power_backup[0x0d] = RTL_R32(rTxAGC_A_CCK1_Mcs32);
		priv->pshare->phw->power_backup[0x0e] = RTL_R32(rTxAGC_B_CCK5_1_Mcs32);		
		RTL_W32(rTxAGC_A_Rate18_06, pwr_value);
		RTL_W32(rTxAGC_A_Rate54_24, pwr_value);
		RTL_W32(rTxAGC_B_Rate18_06, pwr_value);
		RTL_W32(rTxAGC_B_Rate54_24, pwr_value);		
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, pwr_value);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, pwr_value);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, pwr_value);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, pwr_value);		
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, pwr_value);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, pwr_value);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, pwr_value);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, pwr_value);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, pwr_value); 
		RTL_W32(rTxAGC_A_CCK1_Mcs32, (pwr_value & 0x0000ff00) | (priv->pshare->phw->power_backup[0x0d] &0xffff00ff)); 
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32, (pwr_value & 0xffffff00) | (priv->pshare->phw->power_backup[0x0e] &0x000000ff)); 
		priv->pshare->phw->lower_tx_power = 1;
		RESTORE_INT(x);	
	}
	else if( priv->pshare->phw->signal_strength != 3 && priv->pshare->phw->lower_tx_power) {
		SAVE_INT_AND_CLI(x);
		RTL_W32(rTxAGC_A_Rate18_06, priv->pshare->phw->power_backup[0x00]);
		RTL_W32(rTxAGC_A_Rate54_24, priv->pshare->phw->power_backup[0x01]);
		RTL_W32(rTxAGC_B_Rate18_06, priv->pshare->phw->power_backup[0x02]);
		RTL_W32(rTxAGC_B_Rate54_24, priv->pshare->phw->power_backup[0x03]);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x04]);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x05]);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x06]);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x07]);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x08]);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x09]);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x0a]);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x0b]);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, priv->pshare->phw->power_backup[0x0c]);
		RTL_W32(rTxAGC_A_CCK1_Mcs32,  priv->pshare->phw->power_backup[0x0d]); 
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32,  priv->pshare->phw->power_backup[0x0e]);
		priv->pshare->phw->lower_tx_power = 0;
		RESTORE_INT(x); 
	}
}
#endif

#ifdef WIFI_WMM
void check_NAV_prot_len(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned int disassoc)
{
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat
		&& pstat->ht_cap_len && pstat->is_intel_sta) {
		if (!disassoc && (pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_)) {
#ifdef STA_EXT
			if (pstat->aid <= FW_NUM_STAT)
				priv->pshare->mimo_ps_dynamic_sta |= BIT(pstat->aid - 1);
			else
				priv->pshare->mimo_ps_dynamic_sta_ext |= BIT(pstat->aid - 1 - FW_NUM_STAT);
#else
			priv->pshare->mimo_ps_dynamic_sta |= BIT(pstat->aid -1);
#endif
		} else {
#ifdef STA_EXT
			if (pstat->aid <= FW_NUM_STAT)
				priv->pshare->mimo_ps_dynamic_sta &= ~BIT(pstat->aid - 1);
			else
				priv->pshare->mimo_ps_dynamic_sta_ext &= ~BIT(pstat->aid - 1 - FW_NUM_STAT);
#else
			priv->pshare->mimo_ps_dynamic_sta &= ~BIT(pstat->aid -1);
#endif
		}

		if (priv->pshare->mimo_ps_dynamic_sta
#ifdef STA_EXT
			|| priv->pshare->mimo_ps_dynamic_sta_ext
#endif
			) {
			RTL_W8(NAV_PROT_LEN, 0x40);
		} else {
			RTL_W8(NAV_PROT_LEN, 0x20);
		}
	}
}
#endif



/*
 *
 * CAM related functions
 *
 */

/*******************************************************/
/*CAM related utility                                  */
/*CamAddOneEntry                                       */
/*CamDeleteOneEntry                                    */
/*CamResetAllEntry                                     */
/*******************************************************/
#define TOTAL_CAM_ENTRY 32

#define CAM_CONTENT_COUNT 8
#define CAM_CONTENT_USABLE_COUNT 6

#define CFG_VALID        BIT(15)


static UCHAR CAM_find_usable(struct rtl8192cd_priv *priv)
{
	unsigned long command = 0, content = 0;
	unsigned char index;
	int for_begin = 4;

	for (index=for_begin; index<TOTAL_CAM_ENTRY; index++) {
		// polling bit, and No Write enable, and address
		command = CAM_CONTENT_COUNT*index;
		RTL_W32(CAMCMD, (SECCAM_POLL|command));

	   	// Check polling bit is clear
		while (1) {
			command = RTL_R32(CAMCMD);
			if(command & SECCAM_POLL)
				continue;
			else
				break;
		}
		content = RTL_R32(CAMREAD);

		// check valid bit. if not valid,
		if ((content & CFG_VALID) == 0) {
			return index;
		}
	}
	return TOTAL_CAM_ENTRY;
}


static void CAM_program_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
	unsigned char* key128, unsigned short config)
{
	unsigned long target_command = 0, target_content = 0;
	unsigned char entry_i = 0;
	struct stat_info *pstat;

	for (entry_i=0; entry_i<CAM_CONTENT_USABLE_COUNT; entry_i++)
	{
		// polling bit, and write enable, and address
		target_command = entry_i + CAM_CONTENT_COUNT*index;
		target_command = target_command |SECCAM_POLL | SECCAM_WE;
		if (entry_i == 0) {
		    //first 32-bit is MAC address and CFG field
		    target_content = (ULONG)(*(macad+0))<<16
							|(ULONG)(*(macad+1))<<24
							|(ULONG)config;
		    target_content = target_content|config;
	    }
		else if (entry_i == 1) {
			//second 32-bit is MAC address
			target_content = (ULONG)(*(macad+5))<<24
							|(ULONG)(*(macad+4))<<16
							|(ULONG)(*(macad+3))<<8
							|(ULONG)(*(macad+2));
		}
		else {
			target_content = (ULONG)(*(key128+(entry_i*4-8)+3))<<24
							|(ULONG)(*(key128+(entry_i*4-8)+2))<<16
							|(ULONG)(*(key128+(entry_i*4-8)+1))<<8
							|(ULONG)(*(key128+(entry_i*4-8)+0));
		}

		RTL_W32(CAMWRITE, target_content);
		RTL_W32(CAMCMD, target_command);
	}

	pstat = get_stainfo(priv, macad);
	if (pstat) {
		pstat->cam_id = index;
	}

	target_content = RTL_R32(CR);
	if ((target_content & MAC_SEC_EN) == 0)
		RTL_W32(CR, (target_content | MAC_SEC_EN));
}


int CamAddOneEntry(struct rtl8192cd_priv *priv,unsigned char *pucMacAddr, unsigned long keyId,
	unsigned long encAlg, unsigned long useDK, unsigned char *pKey)
{
	unsigned char retVal = 0, camIndex = 0, wpaContent = 0;
	unsigned short usConfig = 0;

    //use Hardware Polling to check the valid bit.
    //in reality it should be done by software link-list
	if ((!memcmp(pucMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK))
		camIndex = keyId;
	else
		camIndex = CAM_find_usable(priv);

    if (camIndex==TOTAL_CAM_ENTRY)
    	return retVal;

	usConfig = usConfig|CFG_VALID|((USHORT)(encAlg))|(UCHAR)keyId;

    CAM_program_entry(priv, camIndex, pucMacAddr, pKey, usConfig);

	if (priv->pshare->CamEntryOccupied == 0) {
		if (useDK == 1)
			wpaContent = RXUSEDK | TXUSEDK;
		RTL_W16(SECCFG, RTL_R16(SECCFG) | RXDEC | TXENC | wpaContent | NOSKMC | CHK_KEYID);
	}

	if (camIndex < 4)
		RTL_W16(SECCFG, (RTL_R16(SECCFG) & ~NOSKMC) | (RXBCUSEDK | TXBCUSEDK));

    return 1;
}


void CAM_read_mac_config(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* pMacad,
	unsigned short* pTempConfig)
{
	unsigned long command = 0, content = 0;

	// polling bit, and No Write enable, and address
	// cam address...
	// first 32-bit
	command = CAM_CONTENT_COUNT*index+0;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

   	//Check polling bit is clear
	while (1) {
		command = RTL_R32(CAMCMD);
		if(command & SECCAM_POLL)
			continue;
		else
			break;
	}
	content = RTL_R32(CAMREAD);

	//first 32-bit is MAC address and CFG field
	*(pMacad+0) = (UCHAR)((content>>16)&0x000000FF);
	*(pMacad+1) = (UCHAR)((content>>24)&0x000000FF);
	*pTempConfig  = (USHORT)(content&0x0000FFFF);

	command = CAM_CONTENT_COUNT*index+1;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

   	//Check polling bit is clear
	while (1) {
		command = RTL_R32(CAMCMD);
		if(command & SECCAM_POLL)
			continue;
		else
			break;
	}
	content = RTL_R32(CAMREAD);

	*(pMacad+5) = (UCHAR)((content>>24)&0x000000FF);
	*(pMacad+4) = (UCHAR)((content>>16)&0x000000FF);
	*(pMacad+3) = (UCHAR)((content>>8)&0x000000FF);
	*(pMacad+2) = (UCHAR)((content)&0x000000FF);
}


#if 0
void CAM_mark_invalid(struct rtl8192cd_priv *priv,UCHAR ucIndex)
{
	ULONG ulCommand=0;
	ULONG ulContent=0;

	// polling bit, and No Write enable, and address
	ulCommand = CAM_CONTENT_COUNT*ucIndex;
	ulCommand = ulCommand | _CAM_POLL_ |_CAM_WE_;
	// write content 0 is equall to mark invalid
	RTL_W32(_CAM_W_, ulContent);
	RTL_W32(_CAMCMD_, ulCommand);
}
#endif


static void CAM_empty_entry(struct rtl8192cd_priv *priv,unsigned char index)
{
	unsigned long command=0, content=0;
	unsigned int i;

	for (i = 0; i < CAM_CONTENT_COUNT; i++) {
		// polling bit, and No Write enable, and address
		command = CAM_CONTENT_COUNT*index+i;
		command = command | SECCAM_POLL |SECCAM_WE;
		// write content 0 is equal to mark invalid
		RTL_W32(CAMWRITE, content);
		RTL_W32(CAMCMD, command);
	}
}


int CamDeleteOneEntry(struct rtl8192cd_priv *priv, unsigned char *pMacAddr, unsigned long keyId, unsigned int useDK)
{
	unsigned char ucIndex;
	unsigned char ucTempMAC[6];
	unsigned short usTempConfig=0;
	int for_begin=0;

	// group key processing
	if ((!memcmp(pMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK)) {
		CAM_read_mac_config(priv, keyId, ucTempMAC, &usTempConfig);
		if (usTempConfig&CFG_VALID) {
			CAM_empty_entry(priv, keyId);
			if (priv->pshare->CamEntryOccupied == 1)
				RTL_W16(SECCFG, 0);
			return 1;
		}
		else
			return 0;
	}

	for_begin = 4;

	// unicast key processing
	// key processing for RTL818X(B) series
	for(ucIndex = for_begin; ucIndex < TOTAL_CAM_ENTRY; ucIndex++) {
		CAM_read_mac_config(priv, ucIndex, ucTempMAC, &usTempConfig);
		if(!memcmp(pMacAddr, ucTempMAC, 6)) {

			CAM_empty_entry(priv, ucIndex);	// reset MAC address, david+2007-1-15

			if (priv->pshare->CamEntryOccupied == 1)
				RTL_W16(SECCFG, 0);

			return 1;
		}
	}
	return 0;
}


/*now use empty to fill in the first 4 entries*/
void CamResetAllEntry(struct rtl8192cd_priv *priv)
{
	unsigned char index;

	RTL_W32(CAMCMD, SECCAM_CLR);

	for(index = 0; index < TOTAL_CAM_ENTRY; index++)
		CAM_empty_entry(priv,index);

	priv->pshare->CamEntryOccupied = 0;
	priv->pmib->dot11GroupKeysTable.keyInCam = 0;

	RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
}


void CAM_read_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
	unsigned char* key128, unsigned short* config)
{
	unsigned long  target_command = 0, target_content = 0;
	unsigned char entry_i = 0;
	unsigned long status;

	for (entry_i=0; entry_i<CAM_CONTENT_USABLE_COUNT; entry_i++)
	{
		// polling bit, and No Write enable, and address
		target_command = (unsigned long)(entry_i+CAM_CONTENT_COUNT*index);
		target_command = target_command | SECCAM_POLL;

		RTL_W32(CAMCMD, target_command);
	   	//Check polling bit is clear
		while(1) {
			status = RTL_R32(CAMCMD);
			if(status & SECCAM_POLL)
				continue;
			else
				break;
		}
		target_content = RTL_R32(CAMREAD);

		if (entry_i==0) {
			//first 32-bit is MAC address and CFG field
		    *(config)  = (unsigned short)((target_content)&0x0000FFFF);
		    *(macad+0) = (unsigned char)((target_content>>16)&0x000000FF);
		    *(macad+1) = (unsigned char)((target_content>>24)&0x000000FF);
		}
		else if (entry_i==1) {
			*(macad+5) = (unsigned char)((target_content>>24)&0x000000FF);
		    *(macad+4) = (unsigned char)((target_content>>16)&0x000000FF);
	    	*(macad+3) = (unsigned char)((target_content>>8)&0x000000FF);
	    	*(macad+2) = (unsigned char)((target_content)&0x000000FF);
    		}
		else {
	    	*(key128+(entry_i*4-8)+3) = (unsigned char)((target_content>>24)&0x000000FF);
	    	*(key128+(entry_i*4-8)+2) = (unsigned char)((target_content>>16)&0x000000FF);
	    	*(key128+(entry_i*4-8)+1) = (unsigned char)((target_content>>8)&0x000000FF);
	    	*(key128+(entry_i*4-8)+0) = (unsigned char)(target_content&0x000000FF);
		}

		target_content = 0;
	}
}


#if 0
void debug_cam(UCHAR*TempOutputMac, UCHAR*TempOutputKey, USHORT TempOutputCfg)
{
	printk("MAC Address\n");
	printk(" %X %X %X %X %X %X\n",*TempOutputMac
					    ,*(TempOutputMac+1)
					    ,*(TempOutputMac+2)
					    ,*(TempOutputMac+3)
					    ,*(TempOutputMac+4)
					    ,*(TempOutputMac+5));
	printk("Config:\n");
	printk(" %X\n",TempOutputCfg);

	printk("Key:\n");
	printk("%X %X %X %X,%X %X %X %X,\n%X %X %X %X,%X %X %X %X\n"
	      ,*TempOutputKey,*(TempOutputKey+1),*(TempOutputKey+2)
	      ,*(TempOutputKey+3),*(TempOutputKey+4),*(TempOutputKey+5)
	      ,*(TempOutputKey+6),*(TempOutputKey+7),*(TempOutputKey+8)
	      ,*(TempOutputKey+9),*(TempOutputKey+10),*(TempOutputKey+11)
	      ,*(TempOutputKey+12),*(TempOutputKey+13),*(TempOutputKey+14)
	      ,*(TempOutputKey+15));
}


void CamDumpAll(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg=0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i=0; i<TOTAL_CAM_ENTRY; i++)
	{
		printk("%X-", i);
		CAM_read_entry(priv,i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}


void CamDump4(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg=0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i=0; i<4; i++)
	{
		printk("%X", i);
		CAM_read_entry(priv, i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}
#endif



/*
 *
 * Power Saving related functions
 *
 */
#ifdef PCIE_POWER_SAVING

extern void HostPCIe_SetPhyMdioWrite(unsigned int , unsigned int , unsigned short );

#ifdef ASPM_ENABLE
void ASPM_on_off(struct rtl8192cd_priv *priv) ;
#endif

#define CLK_MANAGE	0xb8000010
#ifdef PCIE_POWER_SAVING_DEBUG
int PCIE_PowerDown(struct rtl8192cd_priv *priv, unsigned char *data)
{
  #define PCIE_PHY0  0xb8b01008
  #define dprintf printk
  int portnum=0, tmp;
  int mode = _atoi(data, 16);

	  if(strlen(data)==0)
	 {
	  dprintf("epdn mode.\n");
	  dprintf("epdn 0: D0 ->L0 \n");
	  dprintf("epdn 3: D3hot ->L1 \n");
	  dprintf("epdn 4: board cast PME_TurnOff \n");
	  dprintf("epdn 7: enable aspm and L0 entry \n");
	  dprintf("epdn 8: enable aspm and L1 entry \n");
	  dprintf("epdn 9: diable  aspm \n");
	  dprintf("epdn 5a: pcie reset \n");
	  dprintf("epdn 6a: L0 -> L2 \n");
	  dprintf("epdn 6b: L2 -> L0\n");
	  dprintf("epdn 6c: L0 -> L1 \n");
	  dprintf("epdn 6d: L1 -> L0\n");
	  dprintf("epdn a3: wake pin test\n");
	  dprintf("epdn b: bar\n");
	  dprintf("epdn b1: offload enable \n");
	  dprintf("epdn b2: offload disable\n");
	  dprintf("epdn c1: swith to 1T\n");
  	  dprintf("epdn c2: switch to 2T\n");
	  dprintf("Link status=%x \n", REG32(0xb8b00728)&0x1f );
	  return 0;
	 }

	 if(mode==0)
	 {

#if 1 //saving more power, leave L1 write
	  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif

	  tmp = REG32(0xb8b10044) &( ~(3));  //D0

	  REG32(0xb8b10044) = tmp|	(0);  //D0
	  delay_ms(1);
	  REG32(0xb8b10044) = tmp|	(0);  //D0
	  dprintf("D0 \n");
		priv->pwr_state = L0;
	 }

	 if(mode==3) {
//RTL_W8(0x302, 0x2);

#if 0 //saving more power
	  REG32(0xb8b10080)|= (0x100);	//enable clock PM
#endif
	   tmp = REG32(0xb8b10044) &( ~(3));  //D0
	  REG32(0xb8b10044) = tmp|	(3);  //D3
	  //HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
	  dprintf("D3 hot \n");
	priv->pwr_state = L1;
#if 1 //saving more power
	  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
	 }

	  if(mode==4) {

		RTL_W8(0x1c, 0xe1); 	// reg lock, dis_prst
		RTL_W8(0x1c, 0xe1);

#if 1 //saving more power   leave L1 write
	   HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif

	   REG32(0xb8b01008) |= (0x200);
	   dprintf("Host boardcase PME_TurnOff \n");
		 priv->pwr_state = L2;
	  }


	 if(mode==0xd) {

		 RTL_W8(0x1c, 0x0);
		 priv->pshare->phw->cur_rx =0;
#ifdef DELAY_REFILL_RX_BUF		 
		 priv->pshare->phw->cur_rx_refill =0;
#endif		 
		memset(&(priv->pshare->phw->txhead0), 0, sizeof(int)*12);

		 RTL_W8(SPS0_CTRL, 0x2b);
		 RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
		 RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
		 RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
		 RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
		 RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
		 RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
		 RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
		 RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);

	 }



	 if(mode==7)	 {
	  REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
	  REG32(0xb8b1070c) |=	((3)<<27) | ((1)<<24);	 //L1=8us, L0s=2us

	  REG32(0xb8b00080) &= ~(0x3);
	  REG32(0xb8b10080) &= ~(0x3);

	  REG32(0xb8b00080) |= 1;	//L0s
	  REG32(0xb8b10080) |= 1;
	 priv->pwr_state=ASPM_L0s_L1;
	 }

	 if(mode==8)	 {
	  REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
	  REG32(0xb8b1070c) |=	((1)<<27) | ((3)<<24);	 //L1=2us, L0s=4us

	  REG32(0xb8b00080) &= ~(0x3);
	  REG32(0xb8b10080) &= ~(0x3);

	  REG32(0xb8b00080) |= 3;	//L1
	  REG32(0xb8b10080) |= 3; //L1
	  priv->pwr_state=ASPM_L0s_L1;

	 }

	 if(mode==9)	 {
	  REG32(0xb8b00080) &= ~(0x3);
	  REG32(0xb8b10080) &= ~(0x3);
	  priv->pwr_state=L0;
	 }


	 if(mode==0x6a)	{
		priv->ps_ctrl = 1 | 32 |0x80;
	  	 mod_timer(&priv->ps_timer, jiffies + 10);
	 }

	 if(mode==0x6c)	{
		 priv->ps_ctrl = 1 | 16 |0x80;
	  	mod_timer(&priv->ps_timer, jiffies + 10);
	 }

	 if((mode==0x6b) || (mode==0x6d))		{
		priv->ps_ctrl = 0x82 | (priv->pwr_state<<4);
		priv->pshare->rf_ft_var.power_save &=0xf0;
		PCIe_power_save_tasklet((unsigned long)priv);
		signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ , 0 );
	}

	if(mode==0xc1)	{
//		PHY_ConfigBBWithParaFile(priv,	PATHB_OFF);
		switch_to_1x1(priv, IN);

	}

	 if(mode==0xc2)	 {
//		 PHY_ConfigBBWithParaFile(priv,  PATHB_ON);
		switch_to_1x1(priv, OUT);
 	}

	 if(mode==0xb)	 {
		 REG32(0xb8b00004) = 0x00100007;
		 REG32(0xb8b10004) = 0x00100007;
		 REG32(0xb8b10010) = 0x18c00001;
		 REG32(0xb8b10018) = 0x19000004;
		printk("b1-00=%x, b0-04=%x, b1-04=%x, b1-10=%x, b1-18=%x\n", REG32(0xb8b10000),
			REG32(0xb8b00004),REG32(0xb8b10004), REG32(0xb8b10010), REG32(0xb8b10018) );
	 }

	 if(mode==0xb1)	 {
		RTL_W16(0x4dc, priv->pshare->phw->seq);
		RTL_W8(0x423, 0x7f);			// mac seq
		printk("seq=%d\n", priv->pshare->phw->seq);
		signin_h2c_cmd(priv, 0x0203, 0);
		delay_ms(100);
//		signin_h2c_cmd(priv, 0x0100, 0);		// offload

		{
			unsigned int cmd = _AP_OFFLOAD_CMD_ | (1<<8) | (HIDDEN_AP<<16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any)<<24;
			signin_h2c_cmd(priv,cmd, 0 );
			printk("sign in h2c cmd:%x\n", cmd);
		}

		delay_ms(100);
	 }

	 if(mode==0xb2)	 {
		signin_h2c_cmd(priv, 0x0000, 0);	// offload disable
		RTL_W8(0x423, 0x0);			// mac seq disable
		RTL_W8(0x286, 0 );
	 }

	 //static unsigned int Buffer[9];

	 if(mode==0xa3)
	 {
	 	unsigned char tmp;


#ifdef RTL8198_WAKE
		REG32(0xb8000030) |= BIT(26)| BIT(27);		//LEDPHASE4
		REG32(0xb8003500) &= ~(BIT(20));	//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(20));	//PABCD_DIR
		REG32(0xb8003518) &= (~(0x03 << 8));
		REG32(0xb8003518) |= (0x01 << 8);	// PCD_IMR		// enable interrupt in falling-edge
		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8000040) |= 0x0c00;		//LEDPHASE1 :GPIOB7
		REG32(0xb8003500) &= ~(BIT(15));;	//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(15));	//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 30));
		REG32(0xb8003514) |= (0x01 << 30);	// PAB_IMR		// enable interrupt in falling-edge
		REG32(0xb8003000) |= BIT(9);		// GIMR

#endif
		// clear wake pin status
		REG32(0xb8b10044) = 0x8108;
		tmp = RTL_R8(0x690);
		if(tmp&1)		{
			tmp ^=0x1;
			RTL_W8(0x690, tmp);
		}
		dprintf("0xb8b10044=%x,690=%x,3000=%x, 3514=%x\n",	REG32(0xb8b10044), RTL_R8(0x690), REG32(0xb8003000),REG32(0xb8003514) );
		RTL_W8(0x690, tmp |0x1 );
		dprintf("0xb8b10044=%x,690=%x\n",	REG32(0xb8b10044), RTL_R8(0x690) );
	 }

	 if(mode==0x5a)
	 {
	  PCIE_reset_procedure3();
	 }

		   //-------------------------------------------------------------
	 if(mode==0x010)
	 {
				 //L0->L1->L0
	  tmp = REG32(0xb8b10044) &( ~(3));  //D0
	  REG32(0xb8b10044) = tmp|	(3);  //D3
	  REG32(0xb8b10044) = tmp|	(0);  //D0, wakeup

	  while(1)
	  { if((REG32(0xb8b00728)&0x1f)==0x11)	 //wait to L0
	   break;
	  }

	  dprintf("DID/VID=%x\n", REG32(0xb8b10000));
	 }
	 //-------------------------------------------------------------
	 if(mode==0x020)
	 {
				 //L0->L2->L0
	  tmp = REG32(0xb8b10044) &( ~(3));  //D0

	  REG32(0xb8b10044) = tmp|	(3);  //D3
	  delay_ms(100);

	  REG32(0xb8b01008) |= (0x200);
		delay_ms(100);


	 //wakeup
		 REG32(CLK_MANAGE) &= ~(1<<12);	  //perst=0 off.
				//dprintf("CLK_MANAGE=%x \n",  REG32(CLK_MANAGE));
			delay_ms(100);
			delay_ms(100);
			delay_ms(100);

		REG32(CLK_MANAGE) |=  (1<<12);	//PERST=1
		//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

			//4. PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x01; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1	bit0 Enable LTSSM=1

	 while(1)	 {
	  if( (REG32(0xb8b00728)&0x1f)==0x11)
		 break;
	 }

	  dprintf("DID/VID=%x\n", REG32(0xb8b10000));
	 }

	  dprintf("Link status=%x \n", REG32(0xb8b00728)&0x1f );

return 0;
}
#endif


void switch_to_1x1(struct rtl8192cd_priv *priv, int mode)
{
	static int rf_phy_bb_backup[22];

	if(mode==IN) 	{

		rf_phy_bb_backup[21]= RTL_R32(0x88c);

		rf_phy_bb_backup[0]= RTL_R32(0x844);
		rf_phy_bb_backup[1]= RTL_R32(0x85c);
		rf_phy_bb_backup[2]= RTL_R32(0xe6c);

		rf_phy_bb_backup[3]= RTL_R32(0xe70);
		rf_phy_bb_backup[4]= RTL_R32(0xe74);
		rf_phy_bb_backup[5]= RTL_R32(0xe78);
		rf_phy_bb_backup[6]= RTL_R32(0xe7c);

		rf_phy_bb_backup[7] = RTL_R32(0xe80);
		rf_phy_bb_backup[8] = RTL_R32(0xe84);
		rf_phy_bb_backup[9] = RTL_R32(0xe88);
		rf_phy_bb_backup[10]= RTL_R32(0xe8c);

		rf_phy_bb_backup[11]= RTL_R32(0xed0);
		rf_phy_bb_backup[12]= RTL_R32(0xed4);
		rf_phy_bb_backup[13]= RTL_R32(0xed8);
		rf_phy_bb_backup[14]= RTL_R32(0xedc);

		rf_phy_bb_backup[15]= RTL_R32(0xee0);
		rf_phy_bb_backup[16]= RTL_R32(0xeec);

		rf_phy_bb_backup[17]= RTL_R32(0xc04);
		rf_phy_bb_backup[18]= RTL_R32(0xd04);
		rf_phy_bb_backup[19]= RTL_R32(0x90c);
		rf_phy_bb_backup[20]= RTL_R32(0x804);

		PHY_SetBBReg(priv, 0x88c, 0x00c00000 , 0x3);

#if 1
// standby
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00010000);
#else
// power off
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00000000);
#endif

		PHY_SetBBReg(priv, 0x85c, bMaskDWord, 0x00db25a4);
		PHY_SetBBReg(priv, 0xe6c, bMaskDWord, 0x20db25a4);

		PHY_SetBBReg(priv, 0xe70, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xe74, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe78, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe7c, bMaskDWord, 0x041b25a4);

		PHY_SetBBReg(priv, 0xe80, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe84, bMaskDWord, 0x63db25a4);
		PHY_SetBBReg(priv, 0xe88, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe8c, bMaskDWord, 0x20db25a4);

		PHY_SetBBReg(priv, 0xed0, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xed4, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xed8, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xedc, bMaskDWord, 0x001b25a4);

		PHY_SetBBReg(priv, 0xee0, bMaskDWord, 0x001b25a4);
		PHY_SetBBReg(priv, 0xeec, bMaskDWord, 0x24db25a4);

		PHY_SetBBReg(priv, 0xc04, 0x000000ff , 0x11);
		PHY_SetBBReg(priv, 0xd04, 0x0000000f , 0x1);
		PHY_SetBBReg(priv, 0x90c, 0x000000ff , 0x11);
		PHY_SetBBReg(priv, 0x90c, 0x0ff00000 , 0x11);
		PHY_SetBBReg(priv, 0x804, 0x000000f , 0x1);
	} else if(mode==OUT)	 {

		 PHY_SetBBReg(priv, 0x88c, bMaskDWord, rf_phy_bb_backup[21]);

		 PHY_SetBBReg(priv, 0x844, bMaskDWord, rf_phy_bb_backup[0]);
		 PHY_SetBBReg(priv, 0x85c, bMaskDWord, rf_phy_bb_backup[1]);
		 PHY_SetBBReg(priv, 0xe6c, bMaskDWord, rf_phy_bb_backup[2]);

		 PHY_SetBBReg(priv, 0xe70, bMaskDWord, rf_phy_bb_backup[3]);
		 PHY_SetBBReg(priv, 0xe74, bMaskDWord, rf_phy_bb_backup[4]);
		 PHY_SetBBReg(priv, 0xe78, bMaskDWord, rf_phy_bb_backup[5]);
		 PHY_SetBBReg(priv, 0xe7c, bMaskDWord, rf_phy_bb_backup[6]);

		 PHY_SetBBReg(priv, 0xe80, bMaskDWord, rf_phy_bb_backup[7]);
		 PHY_SetBBReg(priv, 0xe84, bMaskDWord, rf_phy_bb_backup[8]);
		 PHY_SetBBReg(priv, 0xe88, bMaskDWord, rf_phy_bb_backup[9]);
		 PHY_SetBBReg(priv, 0xe8c, bMaskDWord, rf_phy_bb_backup[10]);

		 PHY_SetBBReg(priv, 0xed0, bMaskDWord, rf_phy_bb_backup[11]);
		 PHY_SetBBReg(priv, 0xed4, bMaskDWord, rf_phy_bb_backup[12]);
		 PHY_SetBBReg(priv, 0xed8, bMaskDWord, rf_phy_bb_backup[13]);
		 PHY_SetBBReg(priv, 0xedc, bMaskDWord, rf_phy_bb_backup[14]);

		 PHY_SetBBReg(priv, 0xee0, bMaskDWord, rf_phy_bb_backup[15]);
		 PHY_SetBBReg(priv, 0xeec, bMaskDWord, rf_phy_bb_backup[16]);

		 PHY_SetBBReg(priv, 0xc04, bMaskDWord , rf_phy_bb_backup[17]);
		 PHY_SetBBReg(priv, 0xd04, bMaskDWord , rf_phy_bb_backup[18]);
		 PHY_SetBBReg(priv, 0x90c, bMaskDWord , rf_phy_bb_backup[19]);

		 PHY_SetBBReg(priv, 0x804, bMaskDWord , rf_phy_bb_backup[20]);
 	}
}


void gpio_wakeup_isr(int irq, void *dev_instance, struct pt_regs *regs);

//const unsigned int CLK_MANAGE =	0xb8000010;
//const unsigned int SYS_PCIE_PHY0   =(0xb8000000 +0x50);

void PCIE_reset_procedure3()
{
 	//PCIE Register
  	#define PCIE_PHY0_REG  0xb8b01000
  	//#define PCIE_PHY1_REG  0xb8b21000
  	#define PCIE_PHY0  0xb8b01008
 	// #define PCIE_PHY1  0xb8b21008

	int status=0, counter=0;
#if 0
	REG32(CLK_MANAGE) &= ~(1<<12);   //perst=0 off.
	//dprintf("CLK_MANAGE=%x \n",  REG32(CLK_MANAGE));
	delay_ms(3);
	delay_ms(3);

	REG32(CLK_MANAGE) |=  (1<<12);	//PERST=1
	//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x01; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1	bit0
	//
	delay_ms(3);

#else
	do{
		//2.Active LX & PCIE Clock
		REG32(CLK_MANAGE) &=  (~(1<<11));        //enable active_pcie0
		delay_ms(2);

		//4. PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		delay_ms(2);

		REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.
		delay_ms(5);

		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
		delay_ms(5);

		REG32(CLK_MANAGE) |=  (1<<11);		  //enable active_pcie0

		//---------------------------------------
		// 6. PCIE Device Reset

		delay_ms(5);
		REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
		delay_ms(5);
		status = REG32(0xb8b00728)&0x1f;

		if( status == 0x11 ) {
			break;
		}else  {
			delay_ms(100);
//			printk("status=%x\n", status);
			if( ++counter>1000) {
//				panic_printk("PCIe reset fail!!!!\n");
				break;
			}
		}
	}while(1);



#endif

//	printk("PCIE_reset_procedure3\t devid=%x\n",REG32(0xb8b10000));

}

#ifdef ASPM_ENABLE
void ASPM_on_off(struct rtl8192cd_priv *priv)
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);

	if(priv->pshare->rf_ft_var.power_save &ASPM_en) {
		if(priv->pwr_state==L0) {
			 REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
			 REG32(0xb8b1070c) |=  ((3)<<27) | ((1)<<24);	//L1=8us, L0s=2us
			 REG32(0xb8b00080) &= ~(0x3);
			 REG32(0xb8b10080) &= ~(0x3);
			 REG32(0xb8b00080) |= 1;   //L0s
			 REG32(0xb8b10080) |= 1;
			priv->pwr_state=ASPM_L0s_L1;
		}
	} else if(priv->pwr_state==ASPM_L0s_L1) {
		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);
		priv->pwr_state=L0;
	}

	RESTORE_INT(flags);
}
#endif

#ifdef GPIO_WAKEPIN
int request_irq_for_wakeup_pin(struct net_device *dev)
{
#ifdef RTL8198_WAKE
// GPIO C4
	REG32(0xb8000030) |= BIT(26)| BIT(27);		//LEDPHASE4
	REG32(0xb8003500) &= ~(BIT(20));			//PABCD_CNR , gpio pin
	REG32(0xb8003508) &= ~(BIT(20));			//PABCD_DIR
	REG32(0xb8003518) &= (~(0x03 << 8));
	REG32(0xb8003518) |= (0x01 << 8);			// PCD_IMR		// enable interrupt in falling-edge
	REG32(0xb8003000) |= BIT(16);				// GIMR
#else
// GPIO B7
	REG32(0xb8000040) |= 0x0c00;				//LEDPHASE1 :GPIOB7
	REG32(0xb8003500) &= ~(BIT(15));;			//PABCD_CNR , gpio pin
	REG32(0xb8003508) &= ~(BIT(15));			//PABCD_DIR
	REG32(0xb8003514) &= (~(0x03 << 30));
	REG32(0xb8003514) |= (0x01 << 30);			// PAB_IMR		// enable interrupt in falling-edge
	REG32(0xb8003000) |= BIT(9);				// GIMR
#endif

	REG32(0xb8b10044) = 0x8108; 				// clear pme status
	REG32(PABCD_ISR) = REG32(PABCD_ISR) ;		// clear int status
	return request_irq(1, gpio_wakeup_isr, SA_INTERRUPT, "rtl_gpio", dev);
}
#endif


void init_pcie_power_saving(struct rtl8192cd_priv *priv)
{
//printk("init_pcie_power_saving================>\n");

// Jason : clk req
#if 0
		REG32(0xb9000354)=8;
		REG32(0xb9000358)=0x30;
#endif

#ifdef FIB_96C
   if (REG32(SYSTEM_BASE ) == 0x80000001) {
   	   extern void HostPCIe_SetPhyMdioWrite(unsigned int , unsigned int , unsigned short );
   	   HostPCIe_SetPhyMdioWrite(0, 8, 0x18d5);	// 18dd -> 18d5
	   HostPCIe_SetPhyMdioWrite(0, 0xd, 0x15a6);	// 15b6 -> 15a6
   }
#endif

// Jason , for ASPM read_reg
	RTL_W16(0x354, 0x18e);
	RTL_W16(0x358, 0x23);

	if ((GET_CHIP_VER(priv) == VERSION_8188C) /*&& IS_TEST_CHIP(priv)*/) {
	    RTL_W16(0x354, 0x20eb);
	    RTL_W16(0x358, 0x3d);
	}

	REG32(0xb8b10080) |= 0x0100;
	REG32(0xb8b10080) |= 0x43;
	REG32(0xb8b10778) |= BIT(5)<<8;

// 92c backdoor
#if 1
	REG32(0xb8b1070c) |= BIT(7)<<24;
	REG32(0xb8b10718) |= (BIT(3) | BIT(4))<<8;
//	   dprintf("70f=%x,719=%x\n",  REG32(0xb8b1070f), REG32(0xb8b10719) );
#endif
	RTL_W8(0x690, 0x2); 	// WoW
	RTL_W8(0x302, 0x2); 	// sw L123
	RTL_W8(0x5, 0x0);

//	RTL_W16(0x558, 0x040a);
//	RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon


	if (IS_TEST_CHIP(priv)) {
		priv->pshare->rf_ft_var.power_save &= (~ L2_en);
	}
	else {

		RTL_W8(0x08, RTL_R8(0x08) | BIT(3));

#ifdef ASPM_ENABLE
		if(!IS_UMC_A_CUT_88C(priv))
		priv->pshare->rf_ft_var.power_save |= ASPM_en;
#endif
	}
#ifdef ASPM_ENABLE
	ASPM_on_off(priv);
#endif

}


void PCIe_power_save_timer(unsigned long task_priv)
{

	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	char force;
	force = priv->ps_ctrl & 0x80;
	priv->ps_ctrl &= 0x7f;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return ;
#endif

	if(force==0) {
		if( (priv->assoc_num==0) && (priv->pwr_state !=L1 && priv->pwr_state !=L2)
			&& (priv->pshare->rf_ft_var.power_save & (L1_en|L2_en))
#ifdef MBSSID
			&& (!priv->pmib->miscEntry.vap_enable)
#endif
#ifdef WDS
			&& (!priv->pmib->dot11WdsInfo.wdsEnabled)
#endif
#ifdef UNIVERSAL_REPEATER
			&& (!IS_DRV_OPEN(GET_VXD_PRIV(priv)))
#endif
#ifdef CLIENT_MODE
			&& !((OPMODE & WIFI_STATION_STATE) ||(OPMODE & WIFI_ADHOC_STATE))
#endif
			) {
				if((priv->offload_ctrl>>7)&&(priv->offload_ctrl&1)==0) {
					if (priv->pshare->rf_ft_var.power_save & L2_en)
						priv->ps_ctrl = 0x21;
					else
						priv->ps_ctrl = 0x11;
					tasklet_schedule(&priv->pshare->ps_tasklet);
				} else	{
					priv->offload_ctrl = 1;
					RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon
					mod_timer(&priv->ps_timer, jiffies + HZ);
					return;
				}
			}
	}

	mod_timer(&priv->ps_timer, jiffies + POWER_DOWN_T0);

#ifdef ASPM_ENABLE
	ASPM_on_off(priv);
#endif

}

void setBaseAddressRegister(void)
{
	int tmp32=0, status;
	while(++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) |( REG32(0xb8b10018) ^ 0x19000004);
		if(!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
			REG32(0xb8b00004) , REG32(0xb8b10004) ,REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}

void PCIe_power_save_tasklet(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	char in_out, L1_L2;
	unsigned int tmp32=0,status=0,portnum=0, page=0, ctr;
	unsigned long flags;
	priv->ps_ctrl &= 0x7f;
	in_out = priv->ps_ctrl & 0xf;
	L1_L2 = (priv->ps_ctrl>>4) & 0x7;

	DEBUG_INFO("PCIe_power_save tasklet, %s, L%d\n", (in_out==IN ? "in" : "out") , L1_L2);

	if( priv->pshare->has_triggered_rx_tasklet || priv->pshare->has_triggered_tx_tasklet) 		{
		printk("p");
		return;
	}

	if( in_out == IN) 	{
#ifdef ASPM_ENABLE
		if( priv->pwr_state==ASPM_L0s_L1) {
		  REG32(0xb8b00080) &= ~(0x3);
		  REG32(0xb8b10080) &= ~(0x3);
		}
#endif
#ifdef RTL8198_WAKE
		REG32(0xb8003000) &= ~ BIT(16); 	// GIMR
#else
		REG32(0xb8003000) &= ~ BIT(9);		// GIMR
#endif
		RTL_W8(0x286,  4);
		REG32(0xb8b10044) = 0x8108; 		// clear pme status
		SAVE_INT_AND_CLI(flags);

		RTL_W16(0x4dc, priv->pshare->phw->seq);
		RTL_W8(0x423, 0x7f);			// mac seq
//		printk("seq=%d\n", priv->pshare->phw->seq);

		if(priv->pshare->rf_ft_var.power_save&stop_dma_en) {
			RTL_W16(PCIE_CTRL_REG, 0xff00 );
			delay_ms(1);
		}
#ifdef PCIE_L2_ENABLE
		if (L1_L2 == L2) {
			int 	tx_head, tx_tail, q_num;
			struct tx_desc		*phdesc, *pdesc;
			for(q_num=MGNT_QUEUE; q_num<BEACON_QUEUE; q_num++) {
				tx_head 	= get_txhead(GET_HW(priv), q_num);
				tx_tail 	= get_txtail(GET_HW(priv), q_num);
				phdesc		= get_txdesc(GET_HW(priv), q_num);
				while (tx_tail != tx_head) {
					pdesc	  = phdesc + (tx_tail);
					pdesc->Dword0 &= set_desc(~TX_OWN);
					tx_tail = (tx_tail + 1) % NUM_TX_DESC;
				}
			}
			rtl8192cd_tx_dsr((unsigned long)priv);
			rtl8192cd_rx_isr(priv);
		}
#endif
		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save&_1x1_en))
			switch_to_1x1(priv,	IN);

		page = ((priv->offload_ctrl)>>7)&0xff;
		if(priv->pshare->rf_ft_var.power_save&offload_en)  {
			status |= signin_h2c_cmd(priv, _RSVDPAGE_CMD_ | page<<8, 0);
			status |= signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ | (1<<8) |(HIDDEN_AP<<16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any)<<24, 0 );
			if(status) {
				printk("signin_h2c_cmd fail(ap offload)\n");
				if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save&_1x1_en))
					switch_to_1x1(priv, OUT);
				if(priv->pshare->rf_ft_var.power_save&stop_dma_en) {
					RTL_W16(PCIE_CTRL_REG, RTL_R8(PCIE_CTRL_REG));
				}
				RTL_W8(0x423, 0x0); 		// mac seq disable
				RTL_W8(0x286, 0);
				RESTORE_INT(flags);
				return;
			}
		}

		if( L1_L2 == L1){

		  RTL_W8(0x302, 0x2);	  // sw L123
		  priv->pwr_state = L1;
		  tmp32= REG32(0xb8b10044) &( ~(3));  //D0
		  REG32(0xb8b10044) = tmp32|	(3);  //D3
		  //HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
		  printk("D3 hot -> L1\n");
		  delay_ms(1);
#if 1 //saving more power
		  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
		}
		RESTORE_INT(flags);

#ifdef PCIE_L2_ENABLE
		if( L1_L2 == L2) {
#if 1 //saving more power   leave L1 write
			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
			RTL_W8(RSV_CTRL0, 0xe1);		// reg lock, dis_prst
			RTL_W8(RSV_CTRL0, 0xe1);
			priv->pwr_state = L2;
			REG32(0xb8b01008) |= (0x200);
			printk("PME turn off -> L2\n");
		}
#endif
#ifdef RTL8198_WAKE
		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif
	}
	else if(in_out==OUT) {
#ifdef PCIE_L2_ENABLE
		if( L1_L2 == L2){
			PCIE_reset_procedure3();
			setBaseAddressRegister();

			SAVE_INT_AND_CLI(flags);
			priv->pwr_state = L0;
			RTL_W8(RSV_CTRL0, 0x0);
			tmp32 = 0;
			while(1) {
				if( !(RTL_R8(SPS0_CTRL) & BIT(3)) || (++tmp32 > 20) ) {
					RTL_W8(SPS0_CTRL, 0x2b);
					break;
				}
			}
		} else
#endif
		{
			ctr=3000;
#if 1 //saving more power, leave L1 write
			  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
			  tmp32 = REG32(0xb8b10044) &( ~(3));  //D0
			do {
				  REG32(0xb8b10044) = tmp32|	(0);  //D0
				  delay_us(100);
				  REG32(0xb8b10044) = tmp32|	(0);  //D0
				status = REG32(0xb8b00728)&0x1f;
//				printk("link status=%x\n", status);
				if(status == 0x11)
					break;
			} while(ctr--);

			 SAVE_INT_AND_CLI(flags);
			  priv->pwr_state = L0;

			if(status != 0x11)
				printk("change to L0 fail!!!, status=%x\n", status);

		}

		if(priv->pshare->rf_ft_var.power_save&offload_en) {
			signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ , 0 );		// offload
			delay_ms(1);
		}

		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) &&(priv->pshare->rf_ft_var.power_save&_1x1_en))
			switch_to_1x1(priv,	OUT);
#ifdef PCIE_L2_ENABLE
		if( L1_L2 == L2) {
			priv->pshare->phw->cur_rx =0;
#ifdef DELAY_REFILL_RX_BUF			
			priv->pshare->phw->cur_rx_refill =0;
#endif			
			memset(&(priv->pshare->phw->txhead0), 0, sizeof(int)*12);
			RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
			RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
			RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
			RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
			RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
			RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
			RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
			RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
		}
#endif
		if(priv->pshare->rf_ft_var.power_save&stop_dma_en) {
			RTL_W16(PCIE_CTRL_REG, RTL_R8(PCIE_CTRL_REG));
		}

		// wait until FW stop parsing packet
		ctr = 1000;
		do {
			if(!(RTL_R8(FWIMR) & FWIMR_RXDONE))
				break;
			delay_us(100);
		} while(ctr--) ;
		if(!ctr)
			DEBUG_ERR("stop offload fail\n");

		RTL_W8(0x423, 0x0); 		// mac seq disable
		RTL_W8(0x286, 0);			// RW_RELEASE_ENABLE
		RTL_W8(0x302, 0x3);			// sw L123
		RESTORE_INT(flags);
#ifdef ASPM_ENABLE
		ASPM_on_off(priv);
#endif

	}
}


void PCIeWakeUp(struct rtl8192cd_priv *priv, unsigned int expTime)
{
	unsigned long flags;
	if ((priv->pwr_state == L1) || (priv->pwr_state == L2)) {
		SAVE_INT_AND_CLI(flags);
		if (timer_pending(&priv->ps_timer))
			del_timer_sync(&priv->ps_timer);
		priv->ps_ctrl = 0x82 | (priv->pwr_state<<4);
		PCIe_power_save_tasklet((unsigned long)priv);
		mod_timer(&priv->ps_timer, jiffies + expTime);
		priv->offload_ctrl =0;
		RESTORE_INT(flags);
	}
}


void gpio_wakeup_isr(int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = NULL;
	struct rtl8192cd_priv *priv = NULL;

	dev = (struct net_device *)dev_instance;
	priv = (struct rtl8192cd_priv *)dev->priv;

	DEBUG_INFO("PABCD_ISR=%x\t", REG32(PABCD_ISR));
	REG32(PABCD_ISR) = REG32(PABCD_ISR) ;
	DEBUG_INFO("PABCD_ISR=%x,0xb8000040=%x\n", REG32(PABCD_ISR), REG32(0xb8b10044));

#ifdef GPIO_WAKEPIN
#ifdef PCIE_POWER_SAVING_DEBUG
	priv->firstPkt = 1;
#endif

	if( priv->pwr_state != L1 && priv->pwr_state != L2)
		return;
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);

	priv->ps_ctrl = 0x82 | (priv->pwr_state<<4);
	PCIe_power_save_tasklet((unsigned long)priv);
	priv->offload_ctrl =0;
	mod_timer(&priv->ps_timer, jiffies + (POWER_DOWN_T0));

#endif
}


void radio_off(struct rtl8192cd_priv *priv)
{
#if 0
	extern	void HostPCIe_Close(void);
	printk("Radio Off======>\n");
	HostPCIe_Close();
#endif
}
#endif


#ifdef EN_EFUSE
/*-----------------------------------------------------------------------------
 * Function:	efuse_PowerSwitch
 *
 * Overview:	When we want to enable write operation, we should change to
 *				pwr on state. When we stop write, we should switch to 500k mode
 *				and disable LDO 2.5V.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/17/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static	void efuse_PowerSwitch(struct rtl8192cd_priv *priv, unsigned char PwrState)
{
	unsigned char	tempval;
	short tmpV16;
	if (PwrState == TRUE)	{
		// 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid
		tmpV16 = RTL_R16(SYS_ISO_CTRL);
		if( ! (tmpV16 & PWC_EV12V ) ){
			tmpV16 |= PWC_EV12V ;
			 RTL_W16(SYS_ISO_CTRL,tmpV16);
		}
		// Reset: 0x0000h[28], default valid
		tmpV16 =  RTL_R16(REG_SYS_FUNC_EN);
		if( !(tmpV16 & FEN_ELDR) ){
			tmpV16 |= FEN_ELDR ;
			RTL_W16(REG_SYS_FUNC_EN,tmpV16);
		}
		// Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid
		tmpV16 = RTL_R16(SYS_CLKR);
		if( (!(tmpV16 & LOADER_CLK_EN) )  ||(!(tmpV16 & ANA8M) ) ){
			tmpV16 |= (LOADER_CLK_EN |ANA8M ) ;
			RTL_W16(SYS_CLKR,tmpV16);
		}
		// Enable LDO 2.5V before read/write action
		tempval = RTL_R8(EFUSE_TEST+3);
		RTL_W8(EFUSE_TEST+3, (tempval | 0x80));
	} else {
		// Disable LDO 2.5V after read/write action
		tempval = RTL_R8(EFUSE_TEST+3);
		RTL_W8(EFUSE_TEST+3, (tempval & 0x7F));
	}
}	/* efuse_PowerSwitch */

static	void ReadEFuseByte(struct rtl8192cd_priv *priv, unsigned short _offset, unsigned char *pbuf)
{
	unsigned int   	value32;
	unsigned char 	readbyte;
	unsigned short 	retry;

	//Write Address
	RTL_W8(EFUSE_CTRL+1, (_offset & 0xff));
	readbyte = RTL_R8(EFUSE_CTRL+2);
	RTL_W8(EFUSE_CTRL+2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	//Write bit 32 0
	readbyte = RTL_R8(EFUSE_CTRL+3);
	RTL_W8(EFUSE_CTRL+3, (readbyte & 0x7f));

	//Check bit 32 read-ready
	retry = 0;
	value32 = RTL_R32(EFUSE_CTRL);

	while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10000)) {
		value32 = RTL_R32(EFUSE_CTRL);
		retry++;
	}
	*pbuf = (unsigned char)(value32 & 0xff);
}	/* ReadEFuseByte */

//
//	Description:
//		1. Execute E-Fuse read byte operation according as map offset and
//		    save to E-Fuse table.
//		2. Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//	2008/12/12 MH 	1. Reorganize code flow and reserve bytes. and add description.
//					2. Add efuse utilization collect.
//	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1
//					write addr must be after sec5.
//
void ReadEFuse(struct rtl8192cd_priv *priv, unsigned short _offset, unsigned char _size_byte, unsigned char *pbuf)
{
	unsigned char  	efuseTbl[EFUSE_MAP_LEN];
	unsigned char  	rtemp8[1];
	unsigned short 	eFuse_Addr = 0;
	unsigned char  	offset, wren;
	unsigned short  	i, j;
	unsigned short 	eFuseWord[EFUSE_MAX_SECTION][EFUSE_MAX_WORD_UNIT];
	unsigned short	efuse_utilized = 0;

	//
	// Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	//
	if((_offset + _size_byte)>EFUSE_MAP_LEN)
	{// total E-Fuse table is 128bytes
		DEBUG_INFO("ReadEFuse(): Invalid offset(%#x) with read bytes(%#x)!!\n",_offset, _size_byte);
		return;
	}

	// 0. Refresh efuse init map as all oxFF.
	for (i = 0; i < EFUSE_MAX_SECTION; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	//
	// 1. Read the first byte to check if efuse is empty!!!
	//
	//
	ReadEFuseByte(priv, eFuse_Addr, rtemp8);
	if(*rtemp8 != 0xFF)	{
		efuse_utilized++;
		DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
		eFuse_Addr++;
	}

	//
	// 2. Read real efuse content. Filter PG header and every section data.
	//
	while((*rtemp8 != 0xFF) && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN)) 	{
		// Check PG header for section num.
		offset = ((*rtemp8 >> 4) & 0x0f);
		if(offset < EFUSE_MAX_SECTION)	{
			// Get word enable value from PG header
			wren = (*rtemp8 & 0x0f);
			for(i=0; i<EFUSE_MAX_WORD_UNIT; i++)	{
				// Check word enable condition in the section
				if(!(wren & 0x01))	{
					ReadEFuseByte(priv, eFuse_Addr, rtemp8);
					DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] = (*rtemp8 & 0xff);
					if(eFuse_Addr >= EFUSE_REAL_CONTENT_LEN)
						break;
					ReadEFuseByte(priv, eFuse_Addr, rtemp8);
					DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] |= (((UINT16)*rtemp8 << 8) & 0xff00);
					if(eFuse_Addr >= EFUSE_REAL_CONTENT_LEN)
						break;
				}
				wren >>= 1;
			}
		}

		// Read next PG header
		ReadEFuseByte(priv, eFuse_Addr, rtemp8);
		if(*rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN))	{
			efuse_utilized++;
			DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
			eFuse_Addr++;
		}
	}

	//
	// 3. Collect 16 sections and 4 word unit into Efuse map.
	//
	for(i=0; i<EFUSE_MAX_SECTION; i++)	{
		for(j=0; j<EFUSE_MAX_WORD_UNIT; j++) {
			efuseTbl[(i<<3)+(j<<1)]=(eFuseWord[i][j] & 0xff);
			efuseTbl[(i<<3)+(j<<1)+1]=((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	//
	// 4. Copy from Efuse map to output pointer memory!!!
	//
	for(i=0; i<_size_byte; i++)
		pbuf[i] = efuseTbl[_offset+i];

	//
	// 5. Calculate Efuse utilization.
	//
//	efuse_usage = (unsigned char)((efuse_utilized*100)/EFUSE_REAL_CONTENT_LEN);
	priv->EfuseUsedBytes = efuse_utilized;

}

/*-----------------------------------------------------------------------------
 * Function:	efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static	void efuse_ReadAllMap(struct rtl8192cd_priv *priv, unsigned char *Efuse)
{
	//
	// We must enable clock and LDO 2.5V otherwise, read all map will be fail!!!!
	//
	efuse_PowerSwitch(priv, TRUE);
	ReadEFuse(priv, 0, 128, Efuse);
	efuse_PowerSwitch(priv, FALSE);

}

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowMapUpdate(struct rtl8192cd_priv *priv)
{
/*
	if (priv->AutoloadFailFlag == TRUE )	{
		DEBUG_INFO("AutoloadFailFlag=TRUE");
		memset((&priv->EfuseMap[EFUSE_INIT_MAP][0]), 0xFF, 128);
	} else
*/
	{
		efuse_ReadAllMap(priv, &priv->EfuseMap[EFUSE_INIT_MAP][0]);
	}

	memcpy(&priv->EfuseMap[EFUSE_MODIFY_MAP][0],
	       &priv->EfuseMap[EFUSE_INIT_MAP][0], HWSET_MAX_SIZE_92C);

}

int isPGValueValid(unsigned char *hwinfo)
{
	int j;
	for(j=EEPROM_TxPowerCCK; j<EEPROM_TxPowerCCK+3; j++) {
		if(hwinfo[j]>63)
			return 0;
	}
	for(j=EEPROM_TxPowerHT40_1S; j<EEPROM_TxPowerHT40_1S+3; j++) {
		if(hwinfo[j]>63)
			return 0;
	}
	return 1;
}
void ReadTxPowerInfoFromHWPG(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	int i;
	if(!isPGValueValid(hwinfo))
		return;
	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse==1) {
		for(i=0;i<MAX_2G_CHANNEL_NUM;i++)	{
			if (i < 3) {
				priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[EEPROM_TxPowerCCK];
				priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[EEPROM_TxPowerCCK+3];

				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[EEPROM_TxPowerHT40_1S];
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[EEPROM_TxPowerHT40_1S+3];

				priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i]= hwinfo[EEPROM_TxPowerHT40_2SDiff];
				priv->pmib->dot11RFEntry.pwrdiffHT20[i]= hwinfo[EEPROM_TxPowerHT20Diff];
				priv->pmib->dot11RFEntry.pwrdiffOFDM[i]= hwinfo[EEPROM_TxPowerOFDMDiff];

			} else if (i < 9) {
				priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[EEPROM_TxPowerCCK+1];
				priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[EEPROM_TxPowerCCK+4];

				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[EEPROM_TxPowerHT40_1S+1];
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[EEPROM_TxPowerHT40_1S+4];

				priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i]= hwinfo[EEPROM_TxPowerHT40_2SDiff+1];
				priv->pmib->dot11RFEntry.pwrdiffHT20[i]= hwinfo[EEPROM_TxPowerHT20Diff+1];
				priv->pmib->dot11RFEntry.pwrdiffOFDM[i]= hwinfo[EEPROM_TxPowerOFDMDiff+1];
			} else {
				priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[EEPROM_TxPowerCCK+2];
				priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[EEPROM_TxPowerCCK+5];

				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[EEPROM_TxPowerHT40_1S+2];
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[EEPROM_TxPowerHT40_1S+5];

				priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i]= hwinfo[EEPROM_TxPowerHT40_2SDiff+2];
				priv->pmib->dot11RFEntry.pwrdiffHT20[i]= hwinfo[EEPROM_TxPowerHT20Diff+2];
				priv->pmib->dot11RFEntry.pwrdiffOFDM[i]= hwinfo[EEPROM_TxPowerOFDMDiff+2];

			}
		}
		DEBUG_INFO("EFUSE Autoload success!\n");
	}
}


void ReadMacAddressFromEfuse(struct rtl8192cd_priv *priv)
{

#ifdef __KERNEL__
	struct sockaddr addr;
#endif

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse==1) {
		unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
		unsigned char *mac = hwinfo+EEPROM_MACADDRESS;
		unsigned char zero[] = {0, 0, 0, 0, 0, 0};
		if(memcmp(mac, zero, MACADDRLEN) && !IS_MCAST(mac)) {
#ifdef __KERNEL__
			memcpy(addr.sa_data, mac, MACADDRLEN);
			rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
			rtl8192cd_set_hwaddr(priv->dev, (void *)mac);
#endif
		}
	}
}



void ReadThermalMeterFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	priv->pmib->dot11RFEntry.ther = (hwinfo[EEPROM_THERMAL_METER]&0x1f);	//[4:0]
	DEBUG_INFO("ThermalMeter = 0x%x\n", priv->pmib->dot11RFEntry.ther);

	if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x1d)) {
		priv->pmib->dot11RFEntry.ther = 0;
	}
}

//
//	Description:
//		Read HW adapter information by E-Fuse or EEPROM according CR9346 reported.
//
//	Assumption:
//		1. CR9346 regiser has verified.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
int ReadAdapterInfo8192CE(struct rtl8192cd_priv *priv)
{
	unsigned char			tmpU1b;
	tmpU1b = RTL_R8(CR9346);

	// To check system boot selection.
	if (tmpU1b & CmdEERPOMSEL)	{
		DEBUG_INFO("Boot from EEPROM\n");
	} else	{
		DEBUG_INFO("Boot from EFUSE\n");
	}

	// To check autoload success or not.
	if (tmpU1b & CmdEEPROM_En)	{
		DEBUG_INFO("Autoload OK!!\n");
		priv->AutoloadFailFlag=FALSE;
#if 0
		EFUSE_ShadowMapUpdate(priv);
		ReadTxPowerInfoFromHWPG(priv);
		ReadMacAddressFromEfuse(priv);
#endif
	} else { // Auto load fail.
		DEBUG_INFO("AutoLoad Fail reported from CR9346!!\n");
		priv->AutoloadFailFlag=TRUE;
	}
	EFUSE_ShadowMapUpdate(priv);
	ReadTxPowerInfoFromHWPG(priv);
	ReadThermalMeterFromEfuse(priv);
	ReadMacAddressFromEfuse(priv);


	return 0;
}


char FLASH_NAME_PARAM[][32] = {
	"HW_TX_POWER_CCK_A",
	"HW_TX_POWER_CCK_B",
	"HW_TX_POWER_HT40_1S_A",
	"HW_TX_POWER_HT40_1S_B",
	"HW_TX_POWER_DIFF_HT40_2S",
	"HW_TX_POWER_DIFF_HT20",
	"HW_TX_POWER_DIFF_OFDM",
	"HW_WLAN0_WLAN_ADDR"
};

int getEEPROMOffset(int type)
{
	int offset=0;
	switch(type) {
	case 0:	offset = EEPROM_TxPowerCCK;
			break;
	case 1: offset = EEPROM_TxPowerCCK+3;
			break;
	case 2: offset = EEPROM_TxPowerHT40_1S;
			break;
	case 3: offset = EEPROM_TxPowerHT40_1S+3;
			break;
	case 4: offset = EEPROM_TxPowerHT40_2SDiff;
			break;
	case 5: offset = EEPROM_TxPowerHT20Diff;
			break;
	case 6: offset = EEPROM_TxPowerOFDMDiff;
			break;
	case 7:	offset = EEPROM_MACADDRESS;
			break;
	default:;
			break;
	};
	return offset;
}


/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
static UINT8 efuse_CalculateWordCnts( UINT8	word_en)
{
	UINT8 word_cnts = 0;
	if(!(word_en & BIT(0)))	word_cnts++; // 0 : write enable
	if(!(word_en & BIT(1)))	word_cnts++;
	if(!(word_en & BIT(2)))	word_cnts++;
	if(!(word_en & BIT(3)))	word_cnts++;
	return word_cnts;
}	// efuse_CalculateWordCnts


/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static	UINT16 efuse_GetCurrentSize(struct rtl8192cd_priv *priv)
{
	INT32 bContinual = TRUE;
	UINT16 efuse_addr = 0;
	UINT8 hoffset=0,hworden=0;
	UINT8 efuse_data,word_cnts=0;

	do	{
		ReadEFuseByte(priv, efuse_addr, &efuse_data) ;
		if(efuse_data!=0xFF) {
			hoffset = (efuse_data>>4) & 0x0F;
			hworden =  efuse_data & 0x0F;
			word_cnts = efuse_CalculateWordCnts(hworden);
			//read next header
			efuse_addr = efuse_addr + (word_cnts*2)+1;
		} else {
			bContinual = FALSE ;
		}
	} while (bContinual  && (efuse_addr  < EFUSE_REAL_CONTENT_LEN) );

	return efuse_addr;

}	// efuse_GetCurrentSize}


/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 * 11/21/2008 	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
static	void
efuse_WordEnableDataRead(UINT8	word_en,
								UINT8	*sourdata,
								UINT8	*targetdata)
{
	if (!(word_en&BIT(0)))	{
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en&BIT(1)))	{
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en&BIT(2)))	{
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en&BIT(3)))	{
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}	// efuse_WordEnableDataRead

/*-----------------------------------------------------------------------------
 * Function:	efuse_PgPacketRead
 *
 * Overview:	Receive dedicated Efuse are content. For92s, we support 16
 *				area now. It will return 8 bytes content for every area.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize code Arch and assign as local API.
 *
 *---------------------------------------------------------------------------*/
static	INT32
efuse_PgPacketRead(struct rtl8192cd_priv *priv,
						UINT8			offset,
						UINT8			*data)
{
	UINT8 ReadState = PG_STATE_HEADER;
	INT32 bContinual = TRUE, bDataEmpty = TRUE ;
	UINT16 efuse_addr = 0;
	UINT8 hoffset=0,hworden=0, efuse_data,word_cnts=0, tmpidx=0;
	UINT8 tmpdata[8];

	if(data==NULL)
		return FALSE;
	if(offset>15)
		return FALSE;

	memset(data, 0xff, sizeof(UINT8)*PGPKT_DATA_SIZE);
	memset(tmpdata, 0xff, sizeof(UINT8)*PGPKT_DATA_SIZE);

	//
	// <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// Skip dummy parts to prevent unexpected data read from Efuse.
	// By pass right now. 2009.02.19.
	//
	while(bContinual && (efuse_addr  < EFUSE_REAL_CONTENT_LEN) )	{
		//-------  Header Read -------------
		if(ReadState & PG_STATE_HEADER)		{
			ReadEFuseByte(priv, efuse_addr ,&efuse_data);
			if(efuse_data!=0xFF){
				hoffset = (efuse_data>>4) & 0x0F;
				hworden =  efuse_data & 0x0F;
				word_cnts = efuse_CalculateWordCnts(hworden);
				bDataEmpty = TRUE ;

				if(hoffset==offset){
					for(tmpidx = 0;tmpidx< word_cnts*2 ;tmpidx++){
						ReadEFuseByte(priv, efuse_addr+1+tmpidx ,&efuse_data);
						tmpdata[tmpidx] = efuse_data;
						if(efuse_data!=0xff){
							bDataEmpty = FALSE;
						}
					}
					if(bDataEmpty==FALSE){
						ReadState = PG_STATE_DATA;
					}else{//read next header
						efuse_addr = efuse_addr + (word_cnts*2)+1;
						ReadState = PG_STATE_HEADER;
					}
				}
				else{//read next header
					efuse_addr = efuse_addr + (word_cnts*2)+1;
					ReadState = PG_STATE_HEADER;
				}
			}
			else{
				bContinual = FALSE ;
			}
		}
		//-------  Data section Read -------------
		else if(ReadState & PG_STATE_DATA)	{
			efuse_WordEnableDataRead(hworden,tmpdata,data);
			efuse_addr = efuse_addr + (word_cnts*2)+1;
			ReadState = PG_STATE_HEADER;
		}
	}
	if(	(data[0]==0xff) &&(data[1]==0xff) && (data[2]==0xff)  && (data[3]==0xff) &&
		(data[4]==0xff) &&(data[5]==0xff) && (data[6]==0xff)  && (data[7]==0xff))
		return FALSE;
	else
		return TRUE;

}	// efuse_PgPacketRead


/*  11/16/2008 MH Write one byte to reald Efuse. */
static	INT32
WriteEFuseByte(
	struct rtl8192cd_priv *priv,
		UINT16		addr,
		UINT8		data)
{
	UINT8 tmpidx = 0;
	INT32 bResult;
	UINT8 val8;

//	DEBUG_INFO("Addr = %x Data=%x\n", addr, data);

	// -----------------e-fuse reg ctrl ---------------------------------
	//address
	RTL_W8( EFUSE_CTRL+1, (UINT8)(addr&0xff));
	val8 = RTL_R8( EFUSE_CTRL+1);
	RTL_W8( EFUSE_CTRL+2, 	(RTL_R8(EFUSE_CTRL+2)&0xFC )|(UINT8)((addr>>8)&0x03));
	RTL_W8( EFUSE_CTRL, data);//data
#if 1
	RTL_W8( EFUSE_CTRL+3, 0xF2	);//write cmd
#else
	// use default program time 5000 ns
	PlatformEFIOWrite4Byte(pAdapter,  REG_EFUSE_CTRL, (PlatformEFIORead4Byte(pAdapter,REG_EFUSE_CTRL)|( EF_FLAG)));//write cmd
#endif

	while((0x80 &  RTL_R8(EFUSE_CTRL+3)) && (tmpidx<100) ){
		tmpidx++;
	}

	if(tmpidx<100)	{
		bResult = TRUE;
	} else	{
		bResult = FALSE;
	}

	return bResult;
}	//



/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataWrite
 *
 * Overview:	Write necessary word unit into current efuse section!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize Efuse operate flow!!.
 *
 *---------------------------------------------------------------------------*/
static	UINT8
efuse_WordEnableDataWrite(struct rtl8192cd_priv *priv,
								UINT16		efuse_addr,
								UINT8		word_en,
								UINT8		*data)
{
	UINT16 tmpaddr = 0;
	UINT16 start_addr = efuse_addr;
	UINT8 badworden = 0x0F;
	UINT8 tmpdata[8];

	//memset(tmpdata,0xff,PGPKT_DATA_SIZE);
	memset(tmpdata,  0xff, PGPKT_DATA_SIZE);
	DEBUG_INFO("word_en = %x efuse_addr=%x\n", word_en, efuse_addr);

	if(!(word_en&BIT(0)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[0]);
		WriteEFuseByte(priv,start_addr++, data[1]);
		ReadEFuseByte(priv,tmpaddr, &tmpdata[0]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[1]);
		if((data[0]!=tmpdata[0])||(data[1]!=tmpdata[1])){
			badworden &= (~ BIT(0));
		}
	}
	if(!(word_en&BIT(1)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[2]);
		WriteEFuseByte(priv,start_addr++, data[3]);
		ReadEFuseByte(priv,tmpaddr    , &tmpdata[2]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[3]);
		if((data[2]!=tmpdata[2])||(data[3]!=tmpdata[3])){
			badworden &=( ~ BIT(1));
		}
	}
	if(!(word_en&BIT(2)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[4]);
		WriteEFuseByte(priv,start_addr++, data[5]);
		ReadEFuseByte(priv,tmpaddr, &tmpdata[4]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[5]);
		if((data[4]!=tmpdata[4])||(data[5]!=tmpdata[5])){
			badworden &=( ~ BIT(2));
		}
	}
	if(!(word_en&BIT(3)))
	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[6]);
		WriteEFuseByte(priv,start_addr++, data[7]);
		ReadEFuseByte(priv,tmpaddr, &tmpdata[6]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[7]);
		if((data[6]!=tmpdata[6])||(data[7]!=tmpdata[7])){
			badworden &=( ~ BIT(3));
		}
	}
	return badworden;
}	// efuse_WordEnableDataWrite


//
//	Description:
//		This routine will calculate current shadow map that
//		how much bytes needs to be updated.
//
//	Assumption:
//		We shall call this routine before programming physical Efuse content.
//
//	Return Value:
//		TRUE: Efuse has enough capacity to program.
//		FALSE: Efuse do NOT has enough capacity to program.
//
//	Created by Roger, 2008.04.21.
//

int EFUSE_ShadowUpdateChk(struct rtl8192cd_priv *priv)
{
	UINT8	SectionIdx, i, Base;
	UINT16	WordsNeed = 0, HdrNum = 0, TotalBytes = 0, EfuseUsed = 0;
	UINT8	bWordChanged, bResult = TRUE;

	// Efuse contain total 16 sections.
	for (SectionIdx = 0; SectionIdx < 16; SectionIdx++)	{
		Base = SectionIdx * 8;
		bWordChanged = FALSE;

		// One section contain 4 words = 8 bytes.
		for (i = 0; i < 8; i=i+2)		{
			if((priv->EfuseMap[EFUSE_INIT_MAP][Base+i] != 	 priv->EfuseMap[EFUSE_MODIFY_MAP][Base+i]) ||
				(priv->EfuseMap[EFUSE_INIT_MAP][Base+i+1] != priv->EfuseMap[EFUSE_MODIFY_MAP][Base+i+1]))			{
				WordsNeed++;
				bWordChanged = TRUE;
			}
		}

		// We shall append Efuse header If any WORDs changed in this section.
		if( bWordChanged==TRUE )
			HdrNum++;
	}

	TotalBytes = HdrNum + WordsNeed*2;
	EfuseUsed = priv->EfuseUsedBytes;

	// Calculate whether updated map has enough capacity.
	if( (TotalBytes + EfuseUsed) >= (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES))
		bResult = FALSE;

	DEBUG_INFO ("EFUSE_ShadowUpdateChk(): TotalBytes(%#x), HdrNum(%#x), WordsNeed(%#x), EfuseUsed(%d)\n",
		TotalBytes, HdrNum, WordsNeed, EfuseUsed);

	return bResult;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_PgPacketWrite
 *
 * Overview:	Send A G package for different section in real efuse area.
 *				For 92S, One PG package contain 8 bytes content and 4 word
 *				unit. PG header = 0x[bit7-4=offset][bit3-0word enable]
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize code Arch and assign as local API.
 *
 *---------------------------------------------------------------------------*/
int efuse_PgPacketWrite(struct rtl8192cd_priv *priv,
						UINT8 			offset,
						UINT8			word_en,
						UINT8			*data)
{
	UINT8 WriteState = PG_STATE_HEADER;
	INT32 bContinual = TRUE, bDataEmpty= TRUE, bResult = TRUE;
	UINT16 efuse_addr = 0;
	UINT8 efuse_data, pg_header = 0;

	UINT8 tmp_word_cnts=0,target_word_cnts=0;
	UINT8 tmp_header,match_word_en,tmp_word_en;

	PGPKT_STRUCT target_pkt;
	PGPKT_STRUCT tmp_pkt;

	UINT8 originaldata[sizeof(UINT8)*8];
	UINT8 tmpindex = 0, badworden = 0x0F;
	static INT32 repeat_times = 0;

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. The total size is equal or smaller than 498bytes
	// (i.e., offset 0~497, and dummy 1bytes) expected after CP test.
	// 2009.02.19.
	//
	if( efuse_GetCurrentSize(priv) >= (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES))	{
		DEBUG_INFO ("efuse_PgPacketWrite error \n");
		return FALSE;
	}

	// Init the 8 bytes content as 0xff
	target_pkt.offset = offset;
	target_pkt.word_en= word_en;
	memset(target_pkt.data,  0xFF, sizeof(UINT8)*8);

	efuse_WordEnableDataRead(word_en,data,target_pkt.data);
	target_word_cnts = efuse_CalculateWordCnts(target_pkt.word_en);

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. Dummy 1bytes is additional.
	// 2009.02.19.
	//
	while( bContinual && (efuse_addr  < (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES)) )	{
		if(WriteState==PG_STATE_HEADER)		{
			bDataEmpty=TRUE;
			badworden = 0x0F;
			//************  so *******************
			DEBUG_INFO("EFUSE PG_STATE_HEADER\n");
			ReadEFuseByte(priv, efuse_addr ,&efuse_data);
			if (efuse_data!=0xFF)			{
				tmp_header  =  efuse_data;
				tmp_pkt.offset 	= (tmp_header>>4) & 0x0F;
				tmp_pkt.word_en 	= tmp_header & 0x0F;
				tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);

				//************  so-1 *******************
				if(tmp_pkt.offset  != target_pkt.offset)				{
					efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
#ifdef EFUSE_ERROE_HANDLE
					WriteState = PG_STATE_HEADER;
#endif
				} else	{
					//************  so-2 *******************
					for(tmpindex=0 ; tmpindex<(tmp_word_cnts*2) ; tmpindex++)	{
						ReadEFuseByte(priv, (efuse_addr+1+tmpindex) ,&efuse_data);
						if(efuse_data != 0xFF)
							bDataEmpty = FALSE;
					}
					//************  so-2-1 *******************
					if(bDataEmpty ==FALSE)	{
						efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
#ifdef EFUSE_ERROE_HANDLE
						WriteState=PG_STATE_HEADER;
#endif
					} else {
						//************  so-2-2 *******************
						match_word_en = 0x0F;
						if(   !( (target_pkt.word_en&BIT(0))|(tmp_pkt.word_en&BIT(0))  ))	{
							 match_word_en &= (~ BIT(0));
						}
						if(   !( (target_pkt.word_en&BIT(1))|(tmp_pkt.word_en&BIT(1))  ))	{
							 match_word_en &= (~ BIT(1));
						}
						if(   !( (target_pkt.word_en&BIT(2))|(tmp_pkt.word_en&BIT(2))  ))	{
							 match_word_en &= (~ BIT(2));
						}
						if(   !( (target_pkt.word_en&BIT(3))|(tmp_pkt.word_en&BIT(3))  ))	{
							 match_word_en &= (~ BIT(3));
						}

						//************  so-2-2-A *******************
						if((match_word_en&0x0F)!=0x0F)		{
							badworden = efuse_WordEnableDataWrite(priv,efuse_addr+1, tmp_pkt.word_en ,target_pkt.data);

							//************  so-2-2-A-1 *******************
							if(0x0F != (badworden&0x0F))	{
								UINT8 reorg_offset = offset;
								UINT8 reorg_worden=badworden;
								efuse_PgPacketWrite(priv,reorg_offset,reorg_worden,originaldata);
							}

							tmp_word_en = 0x0F;
							if(  (target_pkt.word_en&BIT(0))^(match_word_en&BIT(0))  )	{
								tmp_word_en &= (~ BIT(0));
							}
							if(   (target_pkt.word_en&BIT(1))^(match_word_en&BIT(1)) )	{
								tmp_word_en &=  (~ BIT(1));
							}
							if(   (target_pkt.word_en&BIT(2))^(match_word_en&BIT(2)) )	{
								tmp_word_en &= (~ BIT(2));
							}
							if(   (target_pkt.word_en&BIT(3))^(match_word_en&BIT(3)) )	{
								tmp_word_en &=(~ BIT(3));
							}

							//************  so-2-2-A-2 *******************
							if((tmp_word_en&0x0F)!=0x0F){
								//reorganize other pg packet
								efuse_addr = efuse_GetCurrentSize(priv);
								target_pkt.offset = offset;
								target_pkt.word_en= tmp_word_en;
							} else {
								bContinual = FALSE;
							}
#ifdef EFUSE_ERROE_HANDLE
							WriteState=PG_STATE_HEADER;
							repeat_times++;
							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = FALSE;
								bResult = FALSE;
							}
#endif
						}
						else{//************  so-2-2-B *******************
							//reorganize other pg packet
							efuse_addr = efuse_addr + (2*tmp_word_cnts) +1;//next pg packet addr
							target_pkt.offset = offset;
							target_pkt.word_en= target_pkt.word_en;
#ifdef EFUSE_ERROE_HANDLE
							WriteState=PG_STATE_HEADER;
#endif
						}
					}
				}
				DEBUG_INFO("EFUSE PG_STATE_HEADER-1\n");
			}else {
				//************  s1: header == oxff  *******************
				pg_header = ((target_pkt.offset << 4)&0xf0) |target_pkt.word_en;
				WriteEFuseByte(priv,efuse_addr, pg_header);
				ReadEFuseByte(priv, efuse_addr, &tmp_header);

				if(tmp_header == pg_header)	{ //************  s1-1*******************
					WriteState = PG_STATE_DATA;
				}	else
#ifdef EFUSE_ERROE_HANDLE
				 if(tmp_header == 0xFF){//************  s1-3: if Write or read func doesn't work *******************
					//efuse_addr doesn't change
					WriteState = PG_STATE_HEADER;
					repeat_times++;
					if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
						bContinual = FALSE;
						bResult = FALSE;
					}
				} else
#endif
				{//************  s1-2 : fixed the header procedure *******************
					tmp_pkt.offset = (tmp_header>>4) & 0x0F;
					tmp_pkt.word_en=  tmp_header & 0x0F;
					tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);

					//************  s1-2-A :cover the exist data *******************
					//memset(originaldata,0xff,sizeof(UINT8)*8);
					memset(originaldata, 0xff, sizeof(UINT8)*8);

					if(efuse_PgPacketRead( priv, tmp_pkt.offset,originaldata))	{	//check if data exist
						badworden = efuse_WordEnableDataWrite(priv,efuse_addr+1,tmp_pkt.word_en,originaldata);
						if(0x0F != (badworden&0x0F))						{
							UINT8 reorg_offset = tmp_pkt.offset;
							UINT8 reorg_worden=badworden;
							efuse_PgPacketWrite(priv,reorg_offset,reorg_worden,originaldata);
							efuse_addr = efuse_GetCurrentSize(priv);
						} else {
							efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
						}
					} else	{
						//************  s1-2-B: wrong address*******************
						efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
					}

#ifdef EFUSE_ERROE_HANDLE
					WriteState=PG_STATE_HEADER;
					repeat_times++;
					if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
						bContinual = FALSE;
						bResult = FALSE;
					}
#endif
					DEBUG_INFO("EFUSE PG_STATE_HEADER-2\n");
				}

			}
		}
		//write data state
		else if(WriteState==PG_STATE_DATA)
		{	//************  s1-1  *******************
			DEBUG_INFO("EFUSE PG_STATE_DATA\n");
			badworden = 0x0f;
			badworden = efuse_WordEnableDataWrite(priv,efuse_addr+1,target_pkt.word_en,target_pkt.data);
			if((badworden&0x0F)==0x0F) {
				//************  s1-1-A *******************
				bContinual =FALSE;
			} else {
				//reorganize other pg packet //************  s1-1-B *******************
				efuse_addr = efuse_addr + (2*target_word_cnts) +1;//next pg packet addr
				//===========================
				target_pkt.offset = offset;
				target_pkt.word_en= badworden;
				target_word_cnts =  efuse_CalculateWordCnts(target_pkt.word_en);
				//===========================
#ifdef EFUSE_ERROE_HANDLE
				WriteState=PG_STATE_HEADER;
				repeat_times++;
				if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
					bContinual = FALSE;
					bResult = FALSE;
				}
#endif
				DEBUG_INFO("EFUSE PG_STATE_HEADER-3\n");
			}
		}
	}

	if(efuse_addr  >= (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES)) 	{
		DEBUG_INFO("efuse_PgPacketWrite(): efuse_addr(%#x) Out of size!!\n", efuse_addr);
	}

	return TRUE;
}	// efuse_PgPacketWrite



/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowUpdate
 *
 * Overview:	Compare init and modify map to update Efuse!!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 * 12/11/2008	MHC		92SE PH workaround to prevent HW autoload fail.
 * 12/30/2008	Roger	Fix the bug that EFUSE will writed out-of boundary.
 * 02/16/2009	Roger	Revise PCIe autoload fail case and compatible with USB interface to
 *						overcome MP issue.
 *
 *---------------------------------------------------------------------------*/
int EFUSE_ShadowUpdate(	struct rtl8192cd_priv *priv)
{
	UINT16			i, offset, base = 0;
	UINT8			word_en = 0x0F;

	//
	// <Roger_Notes> We have to check whether current Efuse capacity is big enough to program!!
	// 2009.04.21.
	//
	if(!EFUSE_ShadowUpdateChk(priv))	{
		//
		// <Roger_Notes> We shall reload current Efuse all map and synchronize current modified
		// map to prevent inconsistent Efuse content.
		// 2009.04.21.
		//
		EFUSE_ShadowMapUpdate(priv);
		DEBUG_INFO("<---EFUSE_ShadowUpdate(): Efuse out of capacity!!\n");
		return FALSE;
	}
	// For Efuse write action, we must enable LDO2.5V and 40MHZ clk.
	efuse_PowerSwitch(priv,TRUE);

	//
	// Efuse support 16 write are with PG header packet!!!!
	//
	for (offset = 0; offset < 16; offset++) 	{
		// From section(0) to section(15) sequential write.
		word_en = 0x0F;
		base = offset * 8;
		//
		// Decide Word Enable Bit for the Efuse section
		// One section contain 4 words = 8 bytes!!!!!
		//
		for (i = 0; i < 8; i++)		{
			if (priv->EfuseMap[EFUSE_INIT_MAP][base+i] != priv->EfuseMap[EFUSE_MODIFY_MAP][base+i]) 	{
				word_en &= ~(BIT(i/2));
				DEBUG_INFO("Section(%#x) Addr[%x] %x update to %x, Word_En=%02x\n",
				offset, base+i, priv->EfuseMap[EFUSE_INIT_MAP][base+i],	priv->EfuseMap[EFUSE_MODIFY_MAP][base+i],word_en);
			}
		}

		//
		// This section will need to update, call Efuse real write section !!!!
		//
		if (word_en != 0x0F)	{
			UINT8	tmpdata[8];
			memcpy(tmpdata, (&priv->EfuseMap[EFUSE_MODIFY_MAP][base]), 8);

			//
			// <Roger_Notes> Break programming process if efuse capacity is NOT available.
			// 2009.04.20.
			//
			if(!efuse_PgPacketWrite(priv,(UINT8)offset,word_en,tmpdata))	{
				DEBUG_INFO("EFUSE_ShadowUpdate(): PG section(%#x) fail!!\n", offset);
				break;
			}
		}
	}

	// For warm reboot, we must resume Efuse clock to 500K.
	efuse_PowerSwitch(priv, FALSE);

	//
	// <Roger_Notes> We update both init shadow map again and modified map
	// while WPG do loading operation after previous programming.
	// 2008.12.30.
	//
	EFUSE_ShadowMapUpdate(priv);
	DEBUG_INFO ("<---EFUSE_ShadowUpdate()\n");
	return TRUE;
}	// EFUSE_ShadowUpdate


void shadowMapWrite(int type, char *value, unsigned char *hwinfo)
{
	int  i, offset ;
	offset = getEEPROMOffset(type);
	extern int get_array_val(unsigned char *dst, char *src, int len);

	if(offset == EEPROM_MACADDRESS) {
		for(i=0;i<MACADDRLEN;i++) {
			get_array_val(hwinfo+offset+i, value+i*2, 2);
		}
	} else {
		 get_array_val(hwinfo+offset,   value,     2);
		 get_array_val(hwinfo+offset+1, value+3*2, 2);
		 get_array_val(hwinfo+offset+2, value+9*2, 2);
	}
}

int converToFlashFormat(unsigned char *out, unsigned char *hwinfo, int type)
{
	int  i, offset, len=0;
	offset = getEEPROMOffset(type);

	sprintf(out,  "%s=", FLASH_NAME_PARAM[type]);
	len += 1+strlen(FLASH_NAME_PARAM[type]);

	if(offset == EEPROM_MACADDRESS) {
		for(i=0;i<MACADDRLEN;i++) {
			sprintf(out+len,  "%02x", hwinfo[offset+i]);
			len+=2;
		}
	} else {
		for(i=0;i<MAX_2G_CHANNEL_NUM;i++)	{
			if (i < 3) {
				sprintf(out+len,  "%02x", hwinfo[offset]);
				len +=2;
			} else if (i < 9) {
				sprintf(out+len,  "%02x", hwinfo[offset+1]);
				len +=2;
			} else {
				sprintf(out+len,  "%02x", hwinfo[offset+2]);
				len +=2;
			}
		}
	}
	out[len]='\0';
	return len+1;
}


int efuse_get(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int j, len;
	for(j=0; j<8;j++) {
		if (strcmp(data, FLASH_NAME_PARAM[j])==0) {
			len =  converToFlashFormat(data, &(priv->EfuseMap[EFUSE_INIT_MAP][0]), j);
			printk("%s\n", data);
			return len;
		}
	}
	return 0;
}

int efuse_set(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int j;
	extern char *get_value_by_token(char *data, char *token);

	for(j=0; j<8; j++) {
		val = get_value_by_token((char *)data, FLASH_NAME_PARAM[j]);
		if (val) {
			printk("%s=[%s]\n", FLASH_NAME_PARAM[j], val+1);
			shadowMapWrite(j, val+1, &(priv->EfuseMap[EFUSE_MODIFY_MAP][0]));
		}
	}
	return 0;
}

int efuse_sync(struct rtl8192cd_priv *priv, unsigned char *data)
{
	DEBUG_INFO("efuse sync\n");
	EFUSE_ShadowUpdate(priv);
	return 0;
}

#endif



#ifdef SW_ANT_SWITCH

//
// 20100514 Luke/Joseph:
// Add new function to reset antenna diversity state after link.
//

void resetSwAntDivVariable(struct rtl8192cd_priv *priv)
{
	priv->pshare->RSSI_sum_R = 0;
	priv->pshare->RSSI_cnt_R = 0;
	priv->pshare->RSSI_sum_L = 0;
	priv->pshare->RSSI_cnt_L = 0;
	priv->pshare->TXByteCnt_R = 0;
	priv->pshare->TXByteCnt_L = 0;
	priv->pshare->RXByteCnt_R = 0;
	priv->pshare->RXByteCnt_L = 0;	

}
void SwAntDivRestAfterLink(struct rtl8192cd_priv *priv)
{
	priv->pshare->RSSI_test = FALSE;
	priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_RESET;
	memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
	priv->pshare->DM_SWAT_Table.mapIndex = 0;	
	priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
	priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
	resetSwAntDivVariable(priv);
}


void dm_SW_AntennaSwitchInit(struct rtl8192cd_priv *priv)
{
	if (GET_CHIP_VER(priv) == VERSION_8188C) 
		priv->pshare->rf_ft_var.antSw_select = 0;
		
	//RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS:Init SW Antenna Switch\n"));
	resetSwAntDivVariable(priv);
	priv->pshare->DM_SWAT_Table.CurAntenna = Antenna_L;
	priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_RESET;
	memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
	priv->pshare->DM_SWAT_Table.mapIndex = 0;


	RTL_W32(LEDCFG, RTL_R32(LEDCFG) | BIT(23) );	//enable LED[1:0] pin as ANTSEL

	if ( !priv->pshare->rf_ft_var.antSw_select)	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) | BIT(13) );	 //select ANTESEL from path A
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(8)| BIT(9) );  // enable ANTSEL  A as SW control
		RTL_W32(rFPGA0_XA_RFInterfaceOE, (RTL_R32(rFPGA0_XA_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
	} else 	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) & ~ BIT(13) );	 //select ANTESEL from path B
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(24)| BIT(25) );  // enable ANTSEL B as SW control
		RTL_W32(rFPGA0_XB_RFInterfaceOE, (RTL_R32(rFPGA0_XB_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
	}
	RTL_W16(rFPGA0_TxInfo, (RTL_R16(rFPGA0_TxInfo)&0xf0ff) | BIT(8) );	// b11-b8=0001

	// Move the timer initialization to InitializeVariables function.
	//PlatformInitializeTimer(Adapter, &pMgntInfo->SwAntennaSwitchTimer, (RT_TIMER_CALL_BACK)dm_SW_AntennaSwitchCallback, NULL, "SwAntennaSwitchTimer");	
}
//
// 20100514 Luke/Joseph:
// Add new function for antenna diversity after link.
// This is the main function of antenna diversity after link.
// This function is called in HalDmWatchDog() and dm_SW_AntennaSwitchCallback().
// HalDmWatchDog() calls this function with SWAW_STEP_PEAK to initialize the antenna test.
// In SWAW_STEP_PEAK, another antenna and a 500ms timer will be set for testing.
// After 500ms, dm_SW_AntennaSwitchCallback() calls this function to compare the signal just
// listened on the air with the RSSI of original antenna.
// It chooses the antenna with better RSSI.
// There is also a aged policy for error trying. Each error trying will cost more 5 seconds waiting 
// penalty to get next try.
//
void dm_SW_AntennaSwitch(struct rtl8192cd_priv *priv, char Step)
{
	unsigned int	curTxOkCnt, curRxOkCnt;
	unsigned int	CurByteCnt, PreByteCnt;
	int 			Score_R=0, Score_L=0;
	int				RSSI_R, RSSI_L;
	char 			nextAntenna=priv->pshare->DM_SWAT_Table.CurAntenna;
	int				i;	

//1 1. Determine which condition should turn off Antenna Diversity

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif

//	if(!(GET_CHIP_VER(priv) == VERSION_8188C) || !priv->pshare->rf_ft_var.antSw_enable)
//		return;

	if((!priv->assoc_num) 
#ifdef PCIE_POWER_SAVING
	|| (priv->pwr_state == L2) || (priv->pwr_state == L1) 
#endif
	){
		SwAntDivRestAfterLink(priv);
		return;
	}

	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if( Step != priv->pshare->DM_SWAT_Table.try_flag)
	{
		SwAntDivRestAfterLink(priv);
	}

//1 2. Initialization: Select a assocaiated AP or STA as RSSI target
	if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_RESET) {
#ifdef CLIENT_MODE
		if((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE)) {
			// Target: Infrastructure mode AP.
			priv->pshare->RSSI_target = NULL;
		} 
#endif
		resetSwAntDivVariable(priv);
		priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_PEAK;
		return;
	}
	else  {

//1 3. Antenna Diversity

		//2 Calculate TX and RX OK bytes 

		curTxOkCnt = priv->net_stats.tx_bytes - priv->pshare->lastTxOkCnt;
		curRxOkCnt = priv->net_stats.rx_bytes - priv->pshare->lastRxOkCnt;
		priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
		priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;

		//2 Try State
		if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_DETERMINE)	{
			//3 1. Seperately caculate TX and RX OK byte counter for ant A and B
			if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R) {
				priv->pshare->TXByteCnt_R += curTxOkCnt;
				priv->pshare->RXByteCnt_R += curRxOkCnt;
			} else 	{
				priv->pshare->TXByteCnt_L += curTxOkCnt;
				priv->pshare->RXByteCnt_L += curRxOkCnt;
			}
		
			//3 2. Change anntena for testing
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying != 0) {
				nextAntenna = (priv->pshare->DM_SWAT_Table.CurAntenna ) ^ Antenna_MAX;
				priv->pshare->DM_SWAT_Table.RSSI_Trying--;
			}
			
			//2 Try State End: Determine the best antenna

			if(priv->pshare->DM_SWAT_Table.RSSI_Trying==0) {
				nextAntenna = priv->pshare->DM_SWAT_Table.CurAntenna;
				priv->pshare->DM_SWAT_Table.mapIndex = (priv->pshare->DM_SWAT_Table.mapIndex+1)%SELANT_MAP_SIZE;

				//3 TP Mode: Determine the best antenna by throuhgput
				if(priv->pshare->DM_SWAT_Table.TestMode == TP_MODE)  {



					//3 (1) Saperately caculate total byte count for two antennas
					if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R) {
						CurByteCnt = (priv->pshare->TXByteCnt_R + (priv->pshare->RXByteCnt_R<<1));
						PreByteCnt = (priv->pshare->TXByteCnt_L + (priv->pshare->RXByteCnt_L<<1));
					} else {
						CurByteCnt = (priv->pshare->TXByteCnt_L + (priv->pshare->RXByteCnt_L<<1));
						PreByteCnt = (priv->pshare->TXByteCnt_R + (priv->pshare->RXByteCnt_R<<1));
					}

					//3 (2) Throughput Normalization
					if(priv->pshare->TrafficLoad == TRAFFIC_HIGH)
						CurByteCnt >>=3;
					else if(priv->pshare->TrafficLoad == TRAFFIC_LOW)
						CurByteCnt >>=1;		

					if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R) {
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][priv->pshare->DM_SWAT_Table.mapIndex] = PreByteCnt;
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][priv->pshare->DM_SWAT_Table.mapIndex] = CurByteCnt;
					} else {
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][priv->pshare->DM_SWAT_Table.mapIndex] = CurByteCnt;
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][priv->pshare->DM_SWAT_Table.mapIndex] = PreByteCnt;
					}				

					Score_R = Score_L=0;
					for (i= 0; i<SELANT_MAP_SIZE; i++) {
						Score_L += priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][i];
						Score_R += priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][i];
					}
		
					nextAntenna = (Score_L > Score_R) ? Antenna_L : Antenna_R;			

					if(priv->pshare->rf_ft_var.ant_dump&8)
						panic_printk("Mode TP, select Ant%d, [Score1=%d,Score2=%d]\n", nextAntenna, Score_L, Score_R); 	

				}

				//3 RSSI Mode: Determine the best anntena by RSSI
				else if(priv->pshare->DM_SWAT_Table.TestMode == RSSI_MODE) {

					//2 Saperately caculate average RSSI for two antennas
					RSSI_L = RSSI_R = 0;

					if(priv->pshare->RSSI_cnt_R > 0)
						RSSI_R = priv->pshare->RSSI_sum_R/priv->pshare->RSSI_cnt_R; 		
					if(priv->pshare->RSSI_cnt_L > 0)
						RSSI_L = priv->pshare->RSSI_sum_L/priv->pshare->RSSI_cnt_L; 

					if(RSSI_L && RSSI_R )
						nextAntenna =  (RSSI_L > RSSI_R) ? Antenna_L : Antenna_R;		

					if(priv->pshare->rf_ft_var.ant_dump&8)
						panic_printk("Mode RSSI, RSSI_R=%d(%d), RSSI_L=%d(%d), Ant=%d\n",
						RSSI_R, priv->pshare->RSSI_cnt_R, RSSI_L,  priv->pshare->RSSI_cnt_L, nextAntenna);

				}
						
				//3 Reset state
				resetSwAntDivVariable(priv);
				priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_PEAK;
				priv->pshare->RSSI_test = FALSE;
			}			
		}
	
		//1 Normal State
		else if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_PEAK)	{
			
			//3 Determine TP/RSSI mode by TRX OK count
			if((curRxOkCnt+curTxOkCnt) > TP_MODE_THD) {			
				//2 Determine current traffic is high or low
				if((curTxOkCnt+curRxOkCnt) > TRAFFIC_THRESHOLD)
					priv->pshare->TrafficLoad = TRAFFIC_HIGH;
				else
					priv->pshare->TrafficLoad = TRAFFIC_LOW;		

				priv->pshare->DM_SWAT_Table.RSSI_Trying = 10;
				priv->pshare->DM_SWAT_Table.TestMode = TP_MODE;				
			} else	{

				int idx = 0;
				struct stat_info* pEntry = findNextSTA(priv, &idx);
				priv->pshare->RSSI_target = NULL;
				while(pEntry) {					
					if(pEntry && pEntry->expire_to) {
						if(!priv->pshare->RSSI_target)
							priv->pshare->RSSI_target = pEntry;
						else if( pEntry->rssi < priv->pshare->RSSI_target->rssi )
							priv->pshare->RSSI_target = pEntry;
					}
					pEntry = findNextSTA(priv, &idx);
				};		
			
				priv->pshare->DM_SWAT_Table.RSSI_Trying = 6;
				priv->pshare->DM_SWAT_Table.TestMode = RSSI_MODE;
				
				if(priv->pshare->RSSI_target == NULL)	{
					SwAntDivRestAfterLink(priv);
					return;
				}					

				//3 reset state				
				memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
			}

			//3 Begin  to enter Try State		
			nextAntenna = (priv->pshare->DM_SWAT_Table.CurAntenna ) ^ Antenna_MAX;
			priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_DETERMINE;
			priv->pshare->RSSI_test = TRUE;
			
			//3 Reset variables
			resetSwAntDivVariable(priv);
		}
	}

//1 4.Change TRX antenna
	if(nextAntenna != priv->pshare->DM_SWAT_Table.CurAntenna) {
		if (!priv->pshare->rf_ft_var.antSw_select)
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, nextAntenna);
		else
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, nextAntenna);
	}

//1 5.Reset Statistics
	priv->pshare->DM_SWAT_Table.CurAntenna = nextAntenna;

//1 6.Set next timer

	if(priv->pshare->DM_SWAT_Table.RSSI_Trying == 0) {
		return;
	}

	if(priv->pshare->DM_SWAT_Table.TestMode == RSSI_MODE)	{
		mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies +40);		// 400 ms
	} else if(priv->pshare->DM_SWAT_Table.TestMode == TP_MODE)	{

		if(priv->pshare->TrafficLoad == TRAFFIC_HIGH)	{
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying%2 == 0)	
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 1);	// 10 ms
			else
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 8);	// 80 ms

		} else if(priv->pshare->TrafficLoad == TRAFFIC_LOW) {
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying%2 == 0)	
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 4);	// 40 ms	
			else
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 8);	// 80 ms		
		}
	}

}


void dm_SW_AntennaSwitchCallback(unsigned long task_priv) 
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv*)task_priv;
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);
	dm_SW_AntennaSwitch(priv, SWAW_STEP_DETERMINE);
	RESTORE_INT(flags);
}

//
// 20100514 Luke/Joseph:
// This function is used to gather the RSSI information for antenna testing.
// It selects the RSSI of the peer STA that we want to know.
//
void dm_SWAW_RSSI_Check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct stat_info* pEntry = NULL;
	pEntry = get_stainfo(priv, GetAddr2Ptr(get_pframe(pfrinfo)));

	if((priv->pshare->RSSI_target==NULL)||(priv->pshare->RSSI_target==pEntry)) {
		//1 RSSI for SW Antenna Switch
		if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R)
		{
			priv->pshare->RSSI_sum_R += pfrinfo->rssi;
			priv->pshare->RSSI_cnt_R++;
		} else {
			priv->pshare->RSSI_sum_L += pfrinfo->rssi;
			priv->pshare->RSSI_cnt_L++;
		}
	}
}

#ifndef HW_ANT_SWITCH

int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data) 
{
	int ant = _atoi(data, 16);
//	if(GET_CHIP_VER(priv) != VERSION_8188C)
//		return 0;	

#ifdef PCIE_POWER_SAVING
		PCIeWakeUp(priv, POWER_DOWN_T0);
#endif		
	if(ant==Antenna_L || ant==Antenna_R) {
		if (!priv->pshare->rf_ft_var.antSw_select)
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, ant);
		else
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, ant);
		priv->pshare->DM_SWAT_Table.CurAntenna = ant;
		priv->pshare->rf_ft_var.antSw_enable = 0;		 
		SwAntDivRestAfterLink(priv);
		memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
		return 1;
	} else {
		priv->pshare->rf_ft_var.antSw_enable = 1;
		priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
		priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;

		return 0;
	}
}
#endif
#endif
#if defined(HW_ANT_SWITCH)

void dm_HW_AntennaSwitchInit(struct rtl8192cd_priv *priv)
{

#ifdef SW_ANT_SWITCH
	priv->pshare->rf_ft_var.antSw_enable =0;
#endif

	if (GET_CHIP_VER(priv) == VERSION_8188C) 
		priv->pshare->rf_ft_var.antSw_select = 0;		
	if ( !priv->pshare->rf_ft_var.antSw_select)	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) | BIT(13) );	 //select ANTESEL from path A
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(8)| BIT(9)) ); // ANTSEL as HW control		
		RTL_W32(rFPGA0_XA_RFInterfaceOE, (RTL_R32(rFPGA0_XA_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
		RTL_W8(0xc50, RTL_R8(0xc50) | BIT(7));	// Enable Hardware antenna switch
		RTL_W32(0xc54, RTL_R32(0xc54) | BIT(23) );	// Decide final antenna by comparing 2 antennas' pwdb
	} else 	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) & ~ BIT(13) );	 //select ANTESEL from path B
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(24)| BIT(25)) ); // ANTSEL as HW control	
		RTL_W32(rFPGA0_XB_RFInterfaceOE, (RTL_R32(rFPGA0_XB_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna	
		RTL_W8(0xc58, RTL_R8(0xc58) | BIT(7));	// Enable Hardware antenna switch		
		RTL_W32(0xc5C, RTL_R32(0xc5c) | BIT(23) );	// Decide final antenna by comparing 2 antennas' pwdb
	}

	priv->pshare->rf_ft_var.CurAntenna = 0;

	RTL_W32(LEDCFG, RTL_R32(LEDCFG) | BIT(23) );	//enable LED[1:0] pin as ANTSEL
	RTL_W16(0xca4, (RTL_R16(0xca4) & ~(0xfff))|0x0c0); 	// Pwdb threshold=12dB
	RTL_W32(0x874, RTL_R32(0x874) & ~ BIT(23) );	// No update ANTSEL during GNT_BT=1
	RTL_W16(rFPGA0_TxInfo, (RTL_R16(rFPGA0_TxInfo)&0xf0ff) | BIT(8) );	// b11-b8=0001
	RTL_W32(0x80c, RTL_R32(0x80c) | BIT(21) );		// assign antenna by tx desc

	// CCK setting
	RTL_W8(0xa01, RTL_R8(0xa01) | BIT(7));			// enable hw ant diversity
	RTL_W8(0xa0c, (RTL_R8(0xa0c) & 0xe0) | 0x0f );	// b4=0, b3:0 = 1111	32 sample
	RTL_W8(0xa11, RTL_R8(0xa11) | BIT(5));			// do not change default optional antenna
	RTL_W8(0xa14, (RTL_R8(0xa14) & 0xe0) | 0x08 );	// default : optional = 1:1

}

void setRxIdleAnt(struct rtl8192cd_priv *priv, char Ant) 
{
	if(priv->pshare->rf_ft_var.CurAntenna != Ant) {
		if(Ant) {
			RTL_W32(0x858, 0x65a965a9);
//			RTL_W8(0x6d8, RTL_R8(0x6d8) | BIT(6) );
		}
		else {
			RTL_W32(0x858, 0x569a569a); 		
//			RTL_W8(0x6d8, RTL_R8(0x6d8) & (~ BIT(6)));
		}
		priv->pshare->rf_ft_var.CurAntenna = Ant;			
	}
}



void dm_STA_Ant_Select(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ScoreA=0, ScoreB=0, i, nextAnt= pstat->CurAntenna, idleAnt=priv->pshare->rf_ft_var.CurAntenna;

	if((priv->pshare->rf_ft_var.CurAntenna & 0x80)
		|| ((pstat->hwRxAntSel[0] + pstat->hwRxAntSel[1])==0 && (pstat->cckPktCount[0] + pstat->cckPktCount[1])<10)	)
		return;	


	for(i=0; i<2; i++) {
		if(pstat->cckPktCount[i]==0 && pstat->hwRxAntSel[i]==0)
			pstat->AntRSSI[i] = 0;
	}

	if(pstat->hwRxAntSel[0] || pstat->hwRxAntSel[1]) {
		ScoreA = pstat->hwRxAntSel[0];
		ScoreB = pstat->hwRxAntSel[1];

		if(ScoreA != ScoreB) { 
			if(ScoreA > ScoreB)
				nextAnt = 0;
			else 
				nextAnt = 1;
		}
		
	} else {
		ScoreA = pstat->cckPktCount[0];
		ScoreB = pstat->cckPktCount[1];

		if(ScoreA > 5*ScoreB)
			nextAnt = 0;
		else if(ScoreB > 5*ScoreA)
			nextAnt = 1;
		else if(ScoreA > ScoreB)
			nextAnt = 1;
		else if(ScoreB > ScoreA)
			nextAnt = 0;
	} 

	pstat->CurAntenna = nextAnt;	
	
	if(priv->pshare->rf_ft_var.ant_dump&2) {
		panic_printk("id=%d, OFDM/CCK: (%d, %d/%d, %d), RSSI:(%d, %d), ant=%d, RxIdle=%d\n", 
			pstat->aid, 
			pstat->hwRxAntSel[1],
			pstat->hwRxAntSel[0],
			pstat->cckPktCount[1],
			pstat->cckPktCount[0],
			pstat->AntRSSI[1],
			pstat->AntRSSI[0],
			(pstat->CurAntenna==0 ? 2: 1)
			 ,((priv->pshare->rf_ft_var.CurAntenna&1)==0 ? 2 : 1)
			 );
	}

	if(pstat->AntRSSI[idleAnt]==0) 
		pstat->AntRSSI[idleAnt] = pstat->AntRSSI[idleAnt^1];


// reset variables
	pstat->hwRxAntSel[1] = pstat->hwRxAntSel[0] =0;
	pstat->cckPktCount[1]= pstat->cckPktCount[0] =0;

	
}

void dm_HW_IdleAntennaSelect(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat, *pstat_min=NULL;
	struct list_head	*phead, *plist;
	int rssi_min= 0xff, i;

	if(priv->pshare->rf_ft_var.CurAntenna & 0x80)
		return;	

	phead = &priv->asoc_list;
	plist = phead->next;

	while(plist != phead)	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if((pstat->expire_to) && (pstat->AntRSSI[0] || pstat->AntRSSI[1])) {
			int rssi = (pstat->AntRSSI[0] < pstat->AntRSSI[1]) ? pstat->AntRSSI[0] : pstat->AntRSSI[1];
			if((!pstat_min) || ( rssi < rssi_min) ) {
				pstat_min = pstat;
				rssi_min = rssi;
			}
		}

		if (plist == plist->next)
			break;
		plist = plist->next;
	};

	if(pstat_min)
		setRxIdleAnt(priv, pstat_min->CurAntenna);


#ifdef TX_SHORTCUT
	if (!priv->pmib->dot11OperationEntry.disable_txsc) {
		plist = phead->next;
		while(plist != phead)	{
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if(pstat->expire_to) {
				for (i=0; i<TX_SC_ENTRY_NUM; i++) {
					struct tx_desc *pdesc= &(pstat->tx_sc_ent[i].hwdesc1);	
					pdesc->Dword2 &= set_desc(~ (BIT(24)|BIT(25)));
					if((pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)
						pdesc->Dword2 |= set_desc(BIT(24)|BIT(25));
					pdesc= &(pstat->tx_sc_ent[i].hwdesc2);	
					pdesc->Dword2 &= set_desc(~ (BIT(24)|BIT(25)));
					if((pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)
						pdesc->Dword2 |= set_desc(BIT(24)|BIT(25));					
				}
			}		

			if (plist == plist->next)
				break;
			plist = plist->next;
		};
	}
#endif	


}



int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data) 
{
	int ant = _atoi(data, 16);
	
#ifdef PCIE_POWER_SAVING
		PCIeWakeUp(priv, POWER_DOWN_T0);
#endif		

		if (ant==Antenna_L || ant==Antenna_R) {
		if ( !priv->pshare->rf_ft_var.antSw_select) {
			RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(8)| BIT(9) );  //  ANTSEL A as SW control
			RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, ant);
		} else {
			RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(24)| BIT(25) ); // ANTSEL B as HW control
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, ant);		
			RTL_W8(0xc58, RTL_R8(0xc58) & (~ BIT(7)));		// rx OFDM SW control
		}
		RTL_W8(0xa01, RTL_R8(0xa01) & (~ BIT(7)));	// rx CCK SW control		
		RTL_W32(0x80c, RTL_R32(0x80c) & (~ BIT(21))); // select ant by tx desc	
		RTL_W32(0x858, 0x569a569a); 
		priv->pshare->rf_ft_var.antHw_enable = 0;
		priv->pshare->rf_ft_var.CurAntenna  = (ant%2);
#ifdef SW_ANT_SWITCH
		priv->pshare->rf_ft_var.antSw_enable = 0;
		priv->pshare->DM_SWAT_Table.CurAntenna = ant;
		priv->pshare->RSSI_test =0;
#endif		
	}	
	else if(ant==0){
		if ( !priv->pshare->rf_ft_var.antSw_select)  {
			RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(8)| BIT(9)) );
			RTL_W8(0xc50, RTL_R8(0xc50) | BIT(7));	// OFDM HW control	
		} else {
			RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(24)| BIT(25)) );
			RTL_W8(0xc58, RTL_R8(0xc58) | BIT(7));	// OFDM HW control
		}		
		
		RTL_W8(0xa01, RTL_R8(0xa01) | BIT(7));	// CCK HW control
		RTL_W32(0x80c, RTL_R32(0x80c) | BIT(21) ); // by tx desc
		priv->pshare->rf_ft_var.CurAntenna = 0;
		RTL_W32(0x858, 0x569a569a); 	
		priv->pshare->rf_ft_var.antHw_enable = 1;
#ifdef SW_ANT_SWITCH
		priv->pshare->rf_ft_var.antSw_enable = 0;
		priv->pshare->RSSI_test =0;
#endif		
	}
#ifdef SW_ANT_SWITCH
	else if(ant==3) {
		if(!priv->pshare->rf_ft_var.antSw_enable) {		
			dm_SW_AntennaSwitchInit(priv);
			RTL_W32(0x858, 0x569a569a); 
			priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
			priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
		}
		if ( !priv->pshare->rf_ft_var.antSw_select)			
			RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
		else 
			RTL_W8(0xc58, RTL_R8(0xc58) & (~ BIT(7)));	// rx OFDM SW control

		RTL_W8(0xa01, RTL_R8(0xa01) & (~ BIT(7)));		// rx CCK SW control				
		RTL_W32(0x80c, RTL_R32(0x80c) & (~ BIT(21))); 	// select ant by tx desc			
		priv->pshare->rf_ft_var.antHw_enable = 0;
		priv->pshare->rf_ft_var.antSw_enable = 1;

	}
#endif
	
	return 1;	
}

#endif


