/*
 *  MP routines
 *
 *  $Id: 8192cd_mp.c,v 1.25.2.9 2011/04/08 06:44:28 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_MP_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/circ_buf.h>
#endif

#include "./8192cd_cfg.h"

#ifndef __KERNEL__
#include "./sys-support.h"
#endif

#include "./8192cd_headers.h"
#include "./8192cd_tx.h"
#include "./8192cd_debug.h"

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif

#ifdef MP_TEST

#ifdef _MP_TELNET_SUPPORT_
extern void mp_pty_write_monitor(int en);
extern int mp_pty_is_hit(void);
extern int mp_pty_write(const unsigned char *buf, int count);
void mp_printk(const char *fmt, ...)
{
	va_list args;
	int r;
	int mp_printed_len;
	static char mp_printk_buf[1024];

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	va_start(args, fmt);
	mp_printed_len = vscnprintf(mp_printk_buf, sizeof(mp_printk_buf), fmt, args);
	va_end(args);
	mp_pty_write( mp_printk_buf, mp_printed_len );

	return r;
}
#define printk mp_printk
#else
#define printk panic_printk
#endif //_MP_TELNET_SUPPORT_

#ifdef _LITTLE_ENDIAN_
typedef struct _R_ANTENNA_SELECT_OFDM {
	unsigned int		r_tx_antenna:4;
	unsigned int		r_ant_l:4;
	unsigned int		r_ant_non_ht:4;
	unsigned int		r_ant_ht1:4;
	unsigned int		r_ant_ht2:4;
	unsigned int		r_ant_ht_s1:4;
	unsigned int		r_ant_non_ht_s1:4;
	unsigned int		OFDM_TXSC:2;
	unsigned int		Reserved:2;
} R_ANTENNA_SELECT_OFDM;

typedef struct _R_ANTENNA_SELECT_CCK {
	unsigned char		r_cckrx_enable_2:2;
	unsigned char		r_cckrx_enable:2;
	unsigned char		r_ccktx_enable:4;
} R_ANTENNA_SELECT_CCK;

#else // _BIG_ENDIAN_

typedef struct _R_ANTENNA_SELECT_OFDM {
	unsigned int		Reserved:2;
	unsigned int		OFDM_TXSC:2;
	unsigned int		r_ant_non_ht_s1:4;
	unsigned int		r_ant_ht_s1:4;
	unsigned int		r_ant_ht2:4;
	unsigned int		r_ant_ht1:4;
	unsigned int		r_ant_non_ht:4;
	unsigned int		r_ant_l:4;
	unsigned int		r_tx_antenna:4;
} R_ANTENNA_SELECT_OFDM;

typedef struct _R_ANTENNA_SELECT_CCK {
	unsigned char		r_ccktx_enable:4;
	unsigned char		r_cckrx_enable:2;
	unsigned char		r_cckrx_enable_2:2;
} R_ANTENNA_SELECT_CCK;
#endif


extern int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath);
static void mp_chk_sw_ant(struct rtl8192cd_priv *priv);


extern unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len];
extern unsigned char TxPwrTrk_CCK_SwingTbl[TxPwrTrk_CCK_SwingTbl_Len][8];
extern unsigned char TxPwrTrk_CCK_SwingTbl_CH14[TxPwrTrk_CCK_SwingTbl_Len][8];


#ifdef CONFIG_RTL8671
#define _GIMR_			0xb9c03010
#define _UART_RBR_		0xb9c00000
#define _UART_LSR_		0xb9c00014
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
#define _GIMR_				BSP_GIMR
#define _ICU_UART0_MSK_	BSP_UART0_IE
#define _UART0_RBR_			BSP_UART0_RBR
#define _UART0_LSR_			BSP_UART0_LSR
#else
#define _GIMR_				GIMR
#define _ICU_UART0_MSK_	UART0_IE
#define _UART0_RBR_			UART0_RBR
#define _UART0_LSR_			UART0_LSR
#endif
#endif

#ifdef B2B_TEST
#define MP_PACKET_HEADER		("wlan-tx-test")
#define MP_PACKET_HEADER_LEN	12
#endif


#ifdef CONFIG_RTL8671
#define RTL_UART_W16(reg, val16) writew ((val16), (unsigned int)((reg)))
#define RTL_UART_R16(reg) readw ((unsigned int)((reg)))
#define RTL_UART_R32(reg) readl ((unsigned int)((reg)))

#define DISABLE_UART0_INT() \
	do { \
		RTL_UART_W16(_GIMR_, RTL_UART_R16(_GIMR_) & ~0x8000); \
		RTL_UART_R32(_UART_RBR_); \
		RTL_UART_R32(_UART_RBR_); \
	} while (0)

#define RESTORE_UART0_INT() \
	do { \
		RTL_UART_W16(_GIMR_, RTL_UART_R16(_GIMR_) | 0x8000); \
	} while (0)

static inline int IS_KEYBRD_HIT(void)
{
	if (RTL_UART_R32(_UART_LSR_) & 0x01000000) { // data ready
		RTL_UART_R32(_UART_RBR_);	 // clear rx FIFO
		return 1;
	}
	return 0;
}
#endif // CONFIG_RTL8671


#ifdef USE_RTL8186_SDK
#ifdef __LINUX_2_6__
#define RTL_UART_R8(reg)		readb((unsigned char *)reg)
#define RTL_UART_R32(reg)		readl((unsigned char *)reg)
#define RTL_UART_W8(reg, val)	writeb(val, (unsigned char *)reg)
#define RTL_UART_W32(reg, val)	writel(val, (unsigned char *)reg)
#else
#define RTL_UART_R8(reg)		readb((unsigned int)reg)
#define RTL_UART_R32(reg)		readl((unsigned int)reg)
#define RTL_UART_W8(reg, val)	writeb(val, (unsigned int)reg)
#define RTL_UART_W32(reg, val)	writel(val, (unsigned int)reg)
#endif

#define DISABLE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) & ~_ICU_UART0_MSK_); \
		RTL_UART_R8(_UART0_RBR_); \
		RTL_UART_R8(_UART0_RBR_); \
	} while (0)

#define RESTORE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) | _ICU_UART0_MSK_); \
	} while (0)

static inline int IS_KEYBRD_HIT(void)
{
	if (RTL_UART_R8(_UART0_LSR_) & 1) { // data ready
		RTL_UART_R8(_UART0_RBR_);	 // clear rx FIFO
		return 1;
	}
	return 0;
}
#endif // USE_RTL8186_SDK


/*
 *  find a token in a string. If succes, return pointer of token next. If fail, return null
 */
#ifndef EN_EFUSE
static
#endif
char *get_value_by_token(char *data, char *token)
{
		int idx=0, src_len=strlen(data), token_len=strlen(token);

		while (src_len >= token_len) {
			if (!memcmp(&data[idx], token, token_len))
				return (&data[idx+token_len]);
			src_len--;
			idx++;
		}
		return NULL;
}


unsigned char * get_bssid_mp(unsigned char *pframe)
{
	unsigned char 	*bssid;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			bssid = GetAddr3Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			bssid = GetAddr2Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			bssid = GetAddr1Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			bssid = GetAddr2Ptr(pframe);
			break;
	}

	return bssid;
}


static __inline__ int isLegalRate(unsigned int rate)
{
	unsigned int res = 0;

	switch(rate)
	{
	case _1M_RATE_:
	case _2M_RATE_:
	case _5M_RATE_:
	case _6M_RATE_:
	case _9M_RATE_:
	case _11M_RATE_:
	case _12M_RATE_:
	case _18M_RATE_:
	case _24M_RATE_:
	case _36M_RATE_:
	case _48M_RATE_:
	case _54M_RATE_:
		res = 1;
		break;
	case _MCS0_RATE_:
	case _MCS1_RATE_:
	case _MCS2_RATE_:
	case _MCS3_RATE_:
	case _MCS4_RATE_:
	case _MCS5_RATE_:
	case _MCS6_RATE_:
	case _MCS7_RATE_:
	case _MCS8_RATE_:
	case _MCS9_RATE_:
	case _MCS10_RATE_:
	case _MCS11_RATE_:
	case _MCS12_RATE_:
	case _MCS13_RATE_:
	case _MCS14_RATE_:
	case _MCS15_RATE_:
		res = 1;
		break;
	default:
		res = 0;
		break;
	}

	return res;
}


#if 0
static void mp_RL5975e_Txsetting(struct rtl8192cd_priv *priv)
{
	RF92CD_RADIO_PATH_E eRFPath;
	unsigned int rfReg0x14, rfReg0x15, rfReg0x2c;

	// reg0x14
	rfReg0x14 = 0x5ab;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		//channel = 1, 11, in 20MHz mode, set RF-reg[0x14] = 0x59b
		if(priv->pshare->working_channel == 1 || priv->pshare->working_channel == 11)
		{
			if(!is_CCK_rate(priv->pshare->mp_datarate)) //OFDM, MCS rates
				rfReg0x14 = 0x59b;
		}
	}
	else
	{
		//channel = 3, 9, in 40MHz mode, set RF-reg[0x14] = 0x59b
		if(priv->pshare->working_channel == 3 || priv->pshare->working_channel == 9)
			rfReg0x14 = 0x59b;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x14, bMask20Bits, rfReg0x14);
		delay_us(100);
	}

	// reg0x15
	rfReg0x15 = 0xf80;
	if(priv->pshare->mp_datarate == 4)
	{
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
			rfReg0x15 = 0xfc0;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x15, bMask20Bits, rfReg0x15);
		delay_us(100);
	}

	//reg0x2c
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		rfReg0x2c = 0x3d7;
		if(is_CCK_rate(priv->pshare->mp_datarate)) //CCK rate
		{
			if(priv->pshare->working_channel == 1 || priv->pshare->working_channel == 11)
				rfReg0x2c = 0x3f7;
		}
	}
	else
	{
		rfReg0x2c = 0x3ff;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x2c, bMask20Bits, rfReg0x2c);
		delay_us(100);
	}

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
}


static void mp_RF_RxLPFsetting(struct rtl8192cd_priv *priv)
{
	unsigned int rfBand_A=0, rfBand_B=0, rfBand_C=0, rfBand_D=0;

	//==================================================
	//because the EVM issue, CCK ACPR spec, asked by bryant.
	//when BAND_20MHZ_MODE, should overwrite CCK Rx path RF, let the bandwidth
	//from 10M->8M, we should overwrite the following values to the cck rx rf.
	//RF_Reg[0xb]:bit[11:8] = 0x4, otherwise RF_Reg[0xb]:bit[11:8] = 0x0
	switch(priv->pshare->mp_antenna_rx)
	{
	case ANTENNA_A:
	case ANTENNA_AC:
	case ANTENNA_ABCD:
		rfBand_A = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_B:
	case ANTENNA_BD:
		rfBand_B = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_C:
	case ANTENNA_CD:
		rfBand_C = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_D:
		rfBand_D = 0x500; //for TxEVM, CCK ACPR
		break;
	}

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		if(!rfBand_A)
			rfBand_A = 0x100;
		if(!rfBand_B)
			rfBand_B = 0x100;
		if(!rfBand_C)
			rfBand_C = 0x100;
		if(!rfBand_D)
			rfBand_D = 0x100;
	}
	else
	{
		rfBand_A = 0x300;
		rfBand_B = 0x300;
		rfBand_C = 0x300;
		rfBand_D = 0x300;
	}

	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x0b, bMask20Bits, rfBand_A);
	delay_us(100);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x0b, bMask20Bits, rfBand_B);
	delay_us(100);
/*
	PHY_SetRFReg(priv, RF90_PATH_C, 0x0b, bMask20Bits, rfBand_C);
	delay_us(100);
	PHY_SetRFReg(priv, RF90_PATH_D, 0x0b, bMask20Bits, rfBand_D);
	delay_us(100);
*/
}
#endif


static void mp_8192CD_tx_setting(struct rtl8192cd_priv *priv)
{
	unsigned int odd_pwr = 0;

	if (is_CCK_rate(priv->pshare->mp_datarate) && !IS_TEST_CHIP(priv) &&
		(GET_CHIP_VER(priv) == VERSION_8192C)) {
		if (RTL_R8(0xa07) & 0x80) {
			if (priv->pshare->mp_txpwr_patha % 2)
				odd_pwr++;
		} else {
			if (priv->pshare->mp_txpwr_pathb % 2)
				odd_pwr++;
		}

#if 0
		if (odd_pwr) {
			RTL_W32(0xa20, 0x1c1d0000);
			RTL_W32(0xa24, 0xa0f1418);
			RTL_W32(0xa28, 0x205);
		} else {
			RTL_W32(0xa20, 0x1a1b0000);
			RTL_W32(0xa24, 0x090e1317);
			RTL_W32(0xa28, 0x204);
		}
#else
		extern int get_CCK_swing_index(struct rtl8192cd_priv*);
		extern void set_CCK_swing_index(struct rtl8192cd_priv*, short );

		switch((odd_pwr<<1)| priv->pshare->mp_cck_txpwr_odd) {
		case 1:	set_CCK_swing_index(priv, get_CCK_swing_index(priv)+1);
				break;
		case 2: set_CCK_swing_index(priv, get_CCK_swing_index(priv)-1);
				break;
		default: ;
		};

		priv->pshare->mp_cck_txpwr_odd = odd_pwr;

#endif
	}
}


static void mpt_StartCckContTx(struct rtl8192cd_priv *priv)
{
	unsigned int cckrate;

	// 1. if CCK block on?
	if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bCCKEn))
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, bEnable);//set CCK block on

	//Turn Off All Test Mode
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	//Set CCK Tx Test Rate
	switch (priv->pshare->mp_datarate)
	{
		case 2:
			cckrate = 0;
			break;
		case 4:
			cckrate = 1;
			break;
		case 11:
			cckrate = 2;
			break;
		case 22:
			cckrate = 3;
			break;
		default:
			cckrate = 0;
			break;
	}
	PHY_SetBBReg(priv, rCCK0_System, bCCKTxRate, cckrate);

	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x2);    //transmit mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
}


static void mpt_StopCckCoNtTx(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x0);    //normal mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

	PHY_SetBBReg(priv, 0x820, 0x400, 0x0);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x0);
}


static void mpt_StartOfdmContTx(struct rtl8192cd_priv *priv)
{
	// 1. if OFDM block on?
	if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bOFDMEn))
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, bEnable);//set OFDM block on

	// 2. set CCK test mode off, set to CCK normal mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

	// 3. turn on scramble setting
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);

	// 4. Turn On Continue Tx and turn off the other test modes.
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bEnable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);

	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
}


static void mpt_StopOfdmContTx(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	//Delay 10 ms
	delay_ms(10);

	PHY_SetBBReg(priv, 0x820, 0x400, 0x0);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x0);
}


static void mpt_ProSetCarrierSupp(struct rtl8192cd_priv *priv, int enable)
{
	if (enable)
	{ // Start Carrier Suppression.
		// 1. if CCK block on?
		if(!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bCCKEn))
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, bEnable);//set CCK block on

		//Turn Off All Test Mode
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);

		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x2);    //transmit mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x0);  //turn off scramble setting
   		//Set CCK Tx Test Rate
		//PHY_SetBBReg(pAdapter, rCCK0_System, bCCKTxRate, pMgntInfo->ForcedDataRate);
		PHY_SetBBReg(priv, rCCK0_System, bCCKTxRate, 0x0);    //Set FTxRate to 1Mbps
	}
	else
	{ // Stop Carrier Suppression.
		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x0);    //normal mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

		//BB Reset
/*
		PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x0);
		PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x1);
*/
	}
}


/*
 * start mp testing. stop beacon and change to mp mode
 */
void mp_start_test(struct rtl8192cd_priv *priv)
{
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (OPMODE & WIFI_MP_STATE)
	{
		printk("\nFail: already in MP mode\n");
		return;
	}

#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv)) {
		printk("\nFail: only root interface supports MP mode\n");
		return;
	}
	else if (IS_ROOT_INTERFACE(priv) && IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		rtl8192cd_close(GET_VXD_PRIV(priv)->dev);
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable &&
		IS_VAP_INTERFACE(priv)) {
		printk("\nFail: only root interface supports MP mode\n");
		return;
	} else if (IS_ROOT_INTERFACE(priv)) {
		int i;
		if (priv->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					rtl8192cd_close(priv->pvap_priv[i]->dev);
			}
		}
	}
#endif

	// initialize rate to 54M (or 1M ?)
	priv->pshare->mp_datarate = _54M_RATE_;

	// initialize antenna
	priv->pshare->mp_antenna_tx = ANTENNA_A;
	priv->pshare->mp_antenna_rx = ANTENNA_A;
	mp_chk_sw_ant(priv);

	// initialize swing index
	{
		unsigned int regval, i;

//		regval = PHY_QueryBBReg(priv, rOFDM0_XCTxIQImbalance, bMaskDWord);
		regval = PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord);
		for (i=0; i<TxPwrTrk_OFDM_SwingTbl_Len; i++) {
			if (regval == TxPwrTrk_OFDM_SwingTbl[i]) {
				priv->pshare->mp_ofdm_swing_idx = (unsigned char)i;
				break;
			}
		}

		priv->pshare->mp_cck_swing_idx = get_cck_swing_idx(priv->pshare->CurrentChannelBW, priv->pshare->mp_ofdm_swing_idx);
	}

	// change mode to mp mode
	OPMODE |= WIFI_MP_STATE;

	// disable beacon
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
	RTL_W8(TXPAUSE, STOP_BCN);

	priv->pmib->dot11StationConfigEntry.autoRate = 0;
	priv->pmib->dot11StationConfigEntry.protectionDisabled = 1;
	priv->pmib->dot11ErpInfo.ctsToSelf = 0;
	priv->pmib->dot11ErpInfo.protection = 0;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	OPMODE &= ~WIFI_STATION_STATE;
	OPMODE |= WIFI_AP_STATE;

	// stop site survey
	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);

	// stop receiving packets
//	RTL_W32(RCR, _NO_ERLYRX_);
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

	// stop dynamic mechanism
//	if ((get_rf_mimo_mode(priv) == MIMO_2T4R) && (priv->pmib->dot11BssType.net_work_type != WIRELESS_11B))
//		rx_path_by_rssi(priv, NULL, FALSE);
//	tx_power_control(priv, NULL, FALSE);

	// DIG off and set initial gain
	//RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_DIG);
	//set_fw_reg(priv, 0xfd00cc00, 0, 0);	// DIG off actually
	priv->pshare->rf_ft_var.dig_enable = 0;
	set_DIG_state(priv, 0);
	delay_ms(1);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W8(0xc50, 0x2e);
		RTL_W8(0xc58, 0x2e);
	}
	else
#endif
	if (priv->pshare->rf_ft_var.use_ext_lna) {
		RTL_W8(0xc50, 0x2a);
		RTL_W8(0xc58, 0x2a);
	}

	RTL_W8(0xa0a, 0x83);

	priv->pshare->rf_ft_var.tpt_period=5;

	mp_8192CD_tx_setting(priv);

#ifdef GREEN_HILL
	printk("Enter testing mode\n");
#else
	printk("\nUsage:\n");
	printk("  iwpriv wlanx mp_stop\n");
	printk("  iwpriv wlanx mp_rate {2-108,128-143}\n");
	printk("  iwpriv wlanx mp_channel {1-14}\n");
	printk("  iwpriv wlanx mp_bandwidth [40M={0|1},shortGI={0|1}]\n");
	printk("        - default: 40M=0, shortGI=0\n");
	printk("  iwpriv wlanx mp_txpower [patha=x,pathb=y]\n");
	printk("        - if no parameters, driver will set tx power according to flash setting.\n");
//	printk("  iwpriv wlanx mp_phypara xcap=x\n");
	printk("  iwpriv wlanx mp_bssid 001122334455\n");
	printk("  iwpriv wlanx mp_ant_tx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_ant_rx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_arx {start,stop}\n");
	printk("  iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]\n");
	printk("        - if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("        - if \"background\", it will tx continuously until \"stop\" is issued.\n");
	printk("        - if \"pkt\", send cck packet in packet mode (not h/w).\n");
	printk("        - if \"cs\", send cck packet with carrier suppression.\n");
	printk("        - if \"stone\", send packet in single-tone.\n");
	printk("        - default: tx infinitely (no background).\n");
	printk("  iwpriv wlanx mp_query\n");
	printk("  iwpriv wlanx mp_ther\n");
	printk("  iwpriv wlanx mp_pwrtrk [ther={7-29}, stop]\n");

#ifdef B2B_TEST
	printk("  iwpriv wlanx mp_tx [da=xxxxxx,time=n,count=n,len=n,retry=n,err=n]\n");
	printk("        - if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("        - if \"time=-1\", tx infinitely.\n");
	printk("        - If \"err=1\", display statistics when tx err.\n");
 	printk("        - default: da=ffffffffffff, time=0, count=1000, len=1500,\n");
 	printk("              retry=6(mac retry limit), err=1.\n");
#if 0
	printk("  iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]\n");
	printk("        - ra: rx mac. defulat is burn-in mac\n");
	printk("        - quiet_time: quit rx if no rx packet during quiet_time. default is 5s\n");
	printk("        - interval: report rx statistics periodically in sec.\n");
	printk("              default is 0 (no report).\n");
#endif
	printk("  iwpriv wlanx mp_brx {start[,ra=xxxxxx],stop}\n");
	printk("        - start: start rx immediately.\n");
 	printk("        - ra: rx mac. defulat is burn-in mac.\n");
	printk("        - stop: stop rx immediately.\n");
#endif
#endif // GREEN_HILL
}


/*
 * stop mp testing. recover system to previous status.
 */
void mp_stop_test(struct rtl8192cd_priv *priv)
{
	if (!netif_running(priv->dev))	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// enable beacon
	RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & ~STOP_BCN);
	OPMODE &= ~WIFI_MP_STATE;

	priv->pshare->ThermalValue = 0;

	printk("Please restart the interface\n");
}


/*
 * set data rate
 */
void mp_set_datarate(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char rate, rate_org;
	char tmpbuf[32];
	RF92CD_RADIO_PATH_E eRFPath;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	rate = _atoi((char *)data, 10);
	if(!isLegalRate(rate))
	{
		printk("(%d/2) Mbps data rate may not be supported\n", rate);
		return;
	}

	rate_org = priv->pshare->mp_datarate;
	priv->pshare->mp_datarate = rate;
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(11)|BIT(10)), 0x01);
		else
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(11)|BIT(10)), 0x00);
		delay_us(100);
	}

	if (is_CCK_rate(priv->pshare->mp_datarate)) {
		PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
	} else {
		if( IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C ) 
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
		else 
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);
	}

	mp_8192CD_tx_setting(priv);

	if (rate <= 108)
		sprintf(tmpbuf, "Set data rate to %d Mbps\n", rate/2);
	else
		sprintf(tmpbuf, "Set data rate to MCS %d\n", rate&0x7f);
	printk(tmpbuf);
}


/*
 * set channel
 */
void mp_set_channel(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char channel, channel_org;
	char tmpbuf[48];

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	channel = (unsigned char)_atoi((char *)data, 10);

	if (priv->pshare->is_40m_bw &&
		((channel < 3) || (channel > 12))) {
		sprintf(tmpbuf, "channel %d is invalid\n", channel);
		printk(tmpbuf);
		return;
	}

	channel_org = priv->pmib->dot11RFEntry.dot11channel;
	priv->pmib->dot11RFEntry.dot11channel = channel;

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		channel += 14;

	{
		unsigned int val_read;
		unsigned int val= channel;
/*
		if (priv->pshare->CurrentChannelBW) {
			if (priv->pshare->offset_2nd_chan == 1)
				val -= 2;
			else
				val += 2;
		}
*/
		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask20Bits, 1);
		val_read &= 0xfffffff0;
		PHY_SetRFReg(priv, 0, 0x18, bMask20Bits, val_read | val);
		PHY_SetRFReg(priv, 1, 0x18, bMask20Bits, val_read | val);
		channel = val;
	}

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		channel -= 14;

	priv->pshare->working_channel = channel;
//	mp_8192CD_tx_setting(priv);

	sprintf(tmpbuf, "Change channel %d to channel %d\n", channel_org, channel);
	printk(tmpbuf);
}


/*
 * set tx power
 */
void mp_set_tx_power(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int channel = priv->pmib->dot11RFEntry.dot11channel;
	char *val, tmpbuf[64];
	unsigned int writeVal;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	if (strlen(data) == 0) {
		priv->pshare->mp_txpwr_patha = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
		priv->pshare->mp_txpwr_pathb = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];
	}
	else
	{
		val = get_value_by_token((char *)data, "patha=");
		if (val) {
			priv->pshare->mp_txpwr_patha = _atoi(val, 10);
		}

		val = get_value_by_token((char *)data, "pathb=");
		if (val) {
			priv->pshare->mp_txpwr_pathb = _atoi(val, 10);
		}
	}

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		if(priv->pshare->mp_txpwr_patha > HP_OFDM_POWER_MAX)
			priv->pshare->mp_txpwr_patha = HP_OFDM_POWER_MAX;
		if(priv->pshare->mp_txpwr_pathb > HP_OFDM_POWER_MAX)
			priv->pshare->mp_txpwr_pathb = HP_OFDM_POWER_MAX;		

		sprintf(tmpbuf, "Set OFDM power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);

	}
#endif

	writeVal = (priv->pshare->mp_txpwr_patha << 24) |
	           (priv->pshare->mp_txpwr_patha << 16) |
	           (priv->pshare->mp_txpwr_patha << 8)  |
	            priv->pshare->mp_txpwr_patha;
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);

	writeVal = (priv->pshare->mp_txpwr_pathb << 24) |
	           (priv->pshare->mp_txpwr_pathb << 16) |
	           (priv->pshare->mp_txpwr_pathb << 8)  |
	            priv->pshare->mp_txpwr_pathb;
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		if(priv->pshare->mp_txpwr_patha > HP_CCK_POWER_MAX)
			priv->pshare->mp_txpwr_patha = HP_CCK_POWER_MAX;
		if(priv->pshare->mp_txpwr_pathb > HP_CCK_POWER_MAX)
			priv->pshare->mp_txpwr_pathb = HP_CCK_POWER_MAX;	

		sprintf(tmpbuf, "Set CCK power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);		
	}
#endif

	writeVal = priv->pshare->mp_txpwr_patha;
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
	writeVal = (priv->pshare->mp_txpwr_pathb << 16) |
	           (priv->pshare->mp_txpwr_pathb << 8)  |
	            priv->pshare->mp_txpwr_pathb;
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
	writeVal = (priv->pshare->mp_txpwr_patha << 24) |
	           (priv->pshare->mp_txpwr_patha << 16) |
	           (priv->pshare->mp_txpwr_patha << 8)  |
	            priv->pshare->mp_txpwr_pathb;
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);

	mp_8192CD_tx_setting(priv);

#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa) 
#endif
	{
		sprintf(tmpbuf, "Set power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);
	}
}


/*
 * continuous tx
 *  command: "iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]"
 *			  if "time" is set, tx in t sec. if "count" is set, tx with n packet
 *			  if "background", it will tx continuously until "stop" is issue
 *			  if "pkt", send cck packets with packet mode (not hardware)
 *			  if "cs", send cck packet with carrier suppression
 *			  if "stone", send packet in single-tone
 *			  default: tx infinitely (no background)
 */
void mp_ctx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int orgTCR = RTL_R32(TCR);
	unsigned char pbuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	int payloadlen=1500, time=-1;
	struct sk_buff *skb;
	struct wlan_ethhdr_t *pethhdr;
	int len, i=0, q_num;
	unsigned char pattern;
	char *val;
	unsigned long end_time=0;
	unsigned long flags=0;
	int tx_from_isr=0, background=0;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	volatile unsigned int head, tail;
	RF92CD_RADIO_PATH_E eRFPath;
/*
// We need to turn off ADC before entering CTX mode
	RTL_W32(0xe70, (RTL_R32(0xe70) & 0xFE1FFFFF ) );
	delay_us(100);
*/
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// get count
	val = get_value_by_token((char *)data, "count=");
	if (val) {
		priv->pshare->mp_ctx_count = _atoi(val, 10);
		if (priv->pshare->mp_ctx_count)
			time = 0;
	}

	// get time
	val = get_value_by_token((char *)data, "time=");
	if (val) {
		if (!memcmp(val, "-1", 2))
			time = -1;
		else
			time = _atoi(val, 10)*100;
		if (time > 0)
			end_time = jiffies + time;
	}

	// get background
	val = get_value_by_token((char *)data, "background");
	if (val)
		background = 1;

	// get packet mode
	val = get_value_by_token((char *)data, "pkt");
	if (val)
		OPMODE |= WIFI_MP_CTX_PACKET;

	// get carrier suppression mode
	val = get_value_by_token((char *)data, "cs");
	if (val) {
		if (!is_CCK_rate(priv->pshare->mp_datarate)) {
			printk("Specify carrier suppression but not CCK rate!\n");
			return;
		}
		else
			OPMODE |= WIFI_MP_CTX_CCK_CS;
	}

	// get single-tone
	val = get_value_by_token((char *)data, "stone");
	if (val)
		OPMODE |= WIFI_MP_CTX_ST;

	// get single-carrier
	val = get_value_by_token((char *)data, "scr");
	if (val) {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			printk("Specify single carrier but CCK rate!\n");
			return;
		}
		else
			OPMODE |= WIFI_MP_CTX_SCR;
	}

	// get stop
	val = get_value_by_token((char *)data, "stop");
	if (val) {
		unsigned long flags;

		if (!(OPMODE & WIFI_MP_CTX_BACKGROUND)) {
			printk("Error! Continuous-Tx is not on-going.\n");
			return;
		}
		printk("Stop continuous TX\n");

		SAVE_INT_AND_CLI(flags);
		OPMODE &= ~(WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING);
		RESTORE_INT(flags);

		delay_ms(1000);
		for (i=0; i<NUM_MP_SKB; i++)
			kfree(priv->pshare->skb_pool[i]->head);
		kfree(priv->pshare->skb_pool_ptr);

		goto stop_tx;
	}


	// get tx-isr flag, which is set in ISR when Tx ok
	val = get_value_by_token((char *)data, "tx-isr");
	if (val) {
		if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
			if (OPMODE & WIFI_MP_CTX_OFDM_HW)
				return;

			tx_from_isr = 1;
			time = -1;
		}
	}

	if (!tx_from_isr && (OPMODE & WIFI_MP_CTX_BACKGROUND)) {
		printk("Continuous-Tx is on going. You can't issue any tx command except 'stop'.\n");
		return;
	}

	if (background) {
		priv->pshare->skb_pool_ptr = kmalloc(sizeof(struct sk_buff)*NUM_MP_SKB, GFP_KERNEL);
		if (priv->pshare->skb_pool_ptr == NULL) {
			printk("Allocate skb fail!\n");
			return;
		}
		memset(priv->pshare->skb_pool_ptr, 0, sizeof(struct sk_buff)*NUM_MP_SKB);
		for (i=0; i<NUM_MP_SKB; i++) {
			priv->pshare->skb_pool[i] = (struct sk_buff *)(priv->pshare->skb_pool_ptr + i * sizeof(struct sk_buff));
			priv->pshare->skb_pool[i]->head = kmalloc(RX_BUF_LEN, GFP_KERNEL);
			if (priv->pshare->skb_pool[i]->head == NULL) {
				for (i=0; i<NUM_MP_SKB; i++) {
					if (priv->pshare->skb_pool[i]->head)
						kfree(priv->pshare->skb_pool[i]->head);
					else
						break;
				}
				kfree(priv->pshare->skb_pool_ptr);
				printk("Allocate skb fail!\n");
				return;
			}
			else {
				priv->pshare->skb_pool[i]->data = priv->pshare->skb_pool[i]->head;
				priv->pshare->skb_pool[i]->tail = priv->pshare->skb_pool[i]->data;
				priv->pshare->skb_pool[i]->end = priv->pshare->skb_pool[i]->head + RX_BUF_LEN;
				priv->pshare->skb_pool[i]->len = 0;
			}
		}
		priv->pshare->skb_head = 0;
		priv->pshare->skb_tail = 0;

		/*disable interrupt and change OPMODE here to avoid re-enter*/
		SAVE_INT_AND_CLI(flags);
		OPMODE |= WIFI_MP_CTX_BACKGROUND;
		time = -1; // set as infinite
	}

	len = payloadlen + WLAN_ETHHDR_LEN;
	pattern = 0xAB;
	q_num = BE_QUEUE;

	if (!tx_from_isr) {
#ifdef GREEN_HILL
		printk("Start continuous TX");
#else
		if (time < 0) // infinite
			printk("Start continuous DA=%02x%02x%02x%02x%02x%02x len=%d infinite=yes",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5], payloadlen);
		else if (time > 0) // by time
			printk("Start continuous DA=%02x%02x%02x%02x%02x%02x len=%d time=%ds",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
				payloadlen, time/100);
		else // by count
			printk("Start TX DA=%02x%02x%02x%02x%02x%02x len=%d count=%d",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
				payloadlen, priv->pshare->mp_ctx_count);

#if defined(USE_RTL8186_SDK)
		if (!background) {
			printk(", press any key to escape.\n");
		} else
#endif
			printk(".\n");
#endif // GREEN_HILL

		if (OPMODE & WIFI_MP_CTX_PACKET) {
			RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) & ~DIS_CW);
			RTL_W32(EDCA_BE_PARA, (RTL_R32(EDCA_BE_PARA) & 0xffffff00) | (10 + 2 * 20));
		}
		else {
			RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) | DIS_CW);
			RTL_W32(EDCA_BE_PARA, (RTL_R32(EDCA_BE_PARA) & 0xffffff00) | 0x01);

			if (is_CCK_rate(priv->pshare->mp_datarate)) {
				if (OPMODE & WIFI_MP_CTX_CCK_CS)
					mpt_ProSetCarrierSupp(priv, TRUE);
				else
					mpt_StartCckContTx(priv);
			}
			else {
				if (!((OPMODE & WIFI_MP_CTX_ST) && (OPMODE & WIFI_MP_CTX_SCR)))
					mpt_StartOfdmContTx(priv);
				OPMODE |= WIFI_MP_CTX_OFDM_HW;
			}
		}
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

#if defined(USE_RTL8186_SDK)
		if (!background) {
			DISABLE_UART0_INT();
#ifdef _MP_TELNET_SUPPORT_
			mp_pty_write_monitor(1);
#endif //_MP_TELNET_SUPPORT_
		}
#endif

		memset(&priv->net_stats, 0, sizeof(struct net_device_stats));
	}


	i = 0;
	while (1)
	{

#if defined(USE_RTL8186_SDK)
#ifdef _MP_TELNET_SUPPORT_
		if (!in_atomic() && !tx_from_isr && !background)
			schedule();
		if (!tx_from_isr && !background && (IS_KEYBRD_HIT() || mp_pty_is_hit()))
			break;
#else
		if (!tx_from_isr && !background && IS_KEYBRD_HIT())
			break;
#endif //_MP_TELNET_SUPPORT_
#endif

		if (time) {
			if (time != -1) {
				if (jiffies > end_time)
					break;
			}
			else {
				if ((priv->pshare->mp_ctx_count > 0) && (priv->pshare->mp_ctx_pkt >= priv->pshare->mp_ctx_count)) {
					if (background)
						RESTORE_INT(flags);
					delay_ms(10);
					return;
				}
			}
		}
		else {
			if (i >= priv->pshare->mp_ctx_count)
				break;
		}
		i++;
		priv->pshare->mp_ctx_pkt++;

		if ((OPMODE & WIFI_MP_CTX_ST) &&
			(i == 1)) {
			i++;

			{
				switch (priv->pshare->mp_antenna_tx) {
				case ANTENNA_B:
					eRFPath = RF92CD_PATH_B;
					break;
				case ANTENNA_A:
				default:
					eRFPath = RF92CD_PATH_A;
					break;
				}
			}

			// Start Single Tone.
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x0);
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x0);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x21, bMask20Bits, 0xd4000);
			delay_us(100);
			PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20010);
			delay_us(100);

#ifdef HIGH_POWER_EXT_PA
			if(priv->pshare->rf_ft_var.use_ext_pa) {
				PHY_SetBBReg(priv, 0x860, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x864, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x870, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x870, BIT(26), 0x1);
			}
#endif

		}

		if ((OPMODE & WIFI_MP_CTX_SCR) &&
			(i == 1)) {

			// 1. if OFDM block on?
			if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bOFDMEn))
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, bEnable);//set OFDM block on

			// 2. set CCK test mode off, set to CCK normal mode
			PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

			// 3. turn on scramble setting
			PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);

			// 4. Turn On Continue Tx and turn off the other test modes.
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bEnable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bEnable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMDisPwsavTx, bEnable);

			PHY_SetBBReg(priv, rOFDM1_TRxMesaure1, 0xfff, 0x404);
		}

		if ((OPMODE & WIFI_MP_CTX_OFDM_HW) &&
			(i > 1)) {
			if (background) {
				RESTORE_INT(flags);
				return;
			}
			else
				continue;
		}

		if (background || tx_from_isr) {
			if (CIRC_SPACE(priv->pshare->skb_head, priv->pshare->skb_tail, NUM_MP_SKB) > 1) {
				skb = priv->pshare->skb_pool[priv->pshare->skb_head];
				priv->pshare->skb_head = (priv->pshare->skb_head + 1) & (NUM_MP_SKB - 1);
			}
			else {
				OPMODE |= WIFI_MP_CTX_BACKGROUND_PENDING;
				priv->pshare->mp_ctx_pkt--;
				if (background)
					RESTORE_INT(flags);
				return;
			}
		}
		else
			skb = dev_alloc_skb(len);

		if (skb != NULL)
		{
			DECLARE_TXINSN(txinsn);

			skb->dev = priv->dev;
			skb_put(skb, len);

			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
			memcpy((void *)pethhdr->daddr, pbuf, MACADDRLEN);
			memcpy((void *)pethhdr->saddr, BSSID, MACADDRLEN);
			pethhdr->type = payloadlen;

			memset(skb->data+WLAN_ETHHDR_LEN, pattern, payloadlen);

			txinsn.q_num	= q_num; //using low queue for data queue
			txinsn.fr_type	= _SKB_FRAME_TYPE_;
			txinsn.pframe	= skb;
			skb->cb[1] = 0;

			txinsn.tx_rate	= txinsn.lowest_tx_rate = priv->pshare->mp_datarate;
			txinsn.fixed_rate = 1;
			txinsn.retry	= 0;
			txinsn.phdr		= get_wlanllchdr_from_poll(priv);

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));
			SetFrDs(txinsn.phdr);
			SetFrameType(txinsn.phdr, WIFI_DATA);

			if(rtl8192cd_firetx(priv, &txinsn) == CONGESTED)
			{
				//printk("Congested\n");
				if (tx_from_isr) {
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail) // if Q empty,invoke 1s-timer to send
						OPMODE |= (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING);
					return;
				}
				i--;
				priv->pshare->mp_ctx_pkt--;
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);

				if (background || tx_from_isr) {
					skb->tail = skb->data = skb->head;
					skb->len = 0;
					priv->pshare->skb_head = (priv->pshare->skb_head + NUM_MP_SKB - 1) & (NUM_MP_SKB - 1);
				} else if (skb) {
					rtl_kfree_skb(priv, skb, _SKB_TX_);
				}

				if (!tx_from_isr) {
					SAVE_INT_AND_CLI(flags);
					rtl8192cd_tx_dsr((unsigned long)priv);
					RESTORE_INT(flags);
				}
			}
		}
		else
		{
			//printk("Can't allocate sk_buff\n");
			if (tx_from_isr) {
				head = get_txhead(phw, BE_QUEUE);
				tail = get_txtail(phw, BE_QUEUE);
				if (head == tail) // if Q empty,invoke 1s-timer to send
					OPMODE |= (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING);
				return;
			}
			i--;
			priv->pshare->mp_ctx_pkt--;
			delay_ms(1);
			SAVE_INT_AND_CLI(flags);
			rtl8192cd_tx_dsr((unsigned long)priv);
			RESTORE_INT(flags);
		}

		if ((background || tx_from_isr) && (i == NUM_TX_DESC/4)) {
			OPMODE &= ~WIFI_MP_CTX_BACKGROUND_PENDING;
			if (background)
				RESTORE_INT(flags);
			return;
		}
	}

#if defined(USE_RTL8186_SDK)
	if (!tx_from_isr && !background) {
		RESTORE_UART0_INT();
#ifdef _MP_TELNET_SUPPORT_
		mp_pty_write_monitor(0);
#endif //_MP_TELNET_SUPPORT_
	}
#endif

stop_tx:

	RTL_W32(TCR, orgTCR);
/*
// turn on ADC
	RTL_W32(0xe70, (RTL_R32(0xe70) | 0x01e00000) );
	delay_us(100);
*/

	priv->pshare->mp_ctx_count = 0;
	priv->pshare->mp_ctx_pkt = 0;

	if (OPMODE & WIFI_MP_CTX_PACKET)
		OPMODE &= ~WIFI_MP_CTX_PACKET;
	else {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			if (OPMODE & WIFI_MP_CTX_CCK_CS) {
				OPMODE &= ~WIFI_MP_CTX_CCK_CS;
				mpt_ProSetCarrierSupp(priv, FALSE);
			}
			else
				mpt_StopCckCoNtTx(priv);
		}
		else {
			mpt_StopOfdmContTx(priv);
			if (OPMODE & WIFI_MP_CTX_OFDM_HW)
				OPMODE &= ~WIFI_MP_CTX_OFDM_HW;
			if (OPMODE & WIFI_MP_CTX_ST) {
				OPMODE &= ~WIFI_MP_CTX_ST;

				if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R))
					eRFPath = RF92CD_PATH_A;
				else {
					switch (priv->pshare->mp_antenna_tx) {
					case ANTENNA_B:
						eRFPath = RF92CD_PATH_B;
						break;
					case ANTENNA_A:
					default:
						eRFPath = RF92CD_PATH_A;
						break;
					}
				}
				// Stop Single Tone.
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x21, bMask20Bits, 0x54000);
				delay_us(100);
				PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x30000); // PAD all on.
				delay_us(100);

#ifdef HIGH_POWER_EXT_PA
				if(priv->pshare->rf_ft_var.use_ext_pa) {
					PHY_SetBBReg(priv, 0x870, BIT(10), 0x0);
					PHY_SetBBReg(priv, 0x870, BIT(26), 0x0);
				}
#endif
			}

			if (OPMODE & WIFI_MP_CTX_SCR) {
				OPMODE &= ~WIFI_MP_CTX_SCR;

				//Turn off all test modes.
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMDisPwsavTx, bDisable);

				PHY_SetBBReg(priv, rOFDM1_TRxMesaure1, 0xfff, 0);
				//Delay 10 ms
				delay_ms(10);
				//BB Reset
/*
				PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x0);
				PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x1);
*/
			}
		}

		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter1, bContTxHSSI, 0);
	}
}


int mp_query_stats(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	sprintf(data, "Tx OK:%d, Tx Fail:%d, Rx OK:%lu, CRC error:%lu",
		(int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
		(int)priv->net_stats.tx_errors,
		priv->net_stats.rx_packets, priv->net_stats.rx_crc_errors);
	return strlen(data)+1;
}


void mp_txpower_tracking(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned int target_ther = 0;
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	val = get_value_by_token((char *)data, "stop");
	if (val) {
		if (priv->pshare->mp_txpwr_tracking==FALSE)
			return;
//		set_fw_reg(priv, 0xfd000018, 0, 0);
		priv->pshare->mp_txpwr_tracking = FALSE;
		printk("mp tx power tracking stop\n");
		return;
	}

	val = get_value_by_token((char *)data, "ther=");
	if (val)
		target_ther = _atoi(val, 10);
	else if (priv->pmib->dot11RFEntry.ther)
		target_ther = priv->pmib->dot11RFEntry.ther;
	target_ther &= 0xff;

	if (!target_ther) {
		printk("Fail: tx power tracking has no target thermal value\n");
		return;
	}

	if ((target_ther < 0x07) || (target_ther > 0x1d)) {
		printk("Warning: tx power tracking should have target thermal value 7-29\n");
		if (target_ther < 0x07)
			target_ther = 0x07;
		else
			target_ther = 0x1d;
		printk("Warning: reset target thermal value as %d\n", target_ther);
	}

//	set_fw_reg(priv, 0xfd000017 | (target_ther << 8), 0, 0);


	if(priv->pmib->dot11RFEntry.ther && priv->pshare->ThermalValue)
		priv->pshare->ThermalValue += (target_ther - priv->pmib->dot11RFEntry.ther );

	priv->pmib->dot11RFEntry.ther = target_ther;

	priv->pshare->mp_txpwr_tracking = TRUE;
	printk("mp tx power tracking start, target value=%d\n", target_ther);
}


int mp_query_tssi(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int i=0, j=0, val32, tssi, tssi_total=0, tssi_reg, reg_backup;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	if (priv->pshare->mp_txpwr_tracking) {
		priv->pshare->mp_txpwr_tracking = FALSE;
		sprintf(data, "8651");
		return strlen(data)+1;
	}

	if (is_CCK_rate(priv->pshare->mp_datarate)) {
		reg_backup = RTL_R32(rFPGA0_AnalogParameter4);
		RTL_W32(rFPGA0_AnalogParameter4, (reg_backup & 0xfffff0ff));

		while (i < 5) {
			j++;
			delay_ms(10);
			val32 = PHY_QueryBBReg(priv, rCCK0_TRSSIReport, bMaskByte0);
			tssi = val32 & 0x7f;
			if (tssi > 10) {
				tssi_total += tssi;
				i++;
			}
			if (j > 20)
				break;
		}

		RTL_W32(rFPGA0_AnalogParameter4, reg_backup);

		if (i > 0)
			tssi = tssi_total / i;
		else
			tssi = 0;
	}
	else
	{
//		if (priv->pshare->mp_antenna_tx == ANTENNA_A)
			tssi_reg = rFPGA0_XAB_RFInterfaceRB;
//		else
//			tssi_reg = rFPGA0_XCD_RFInterfaceRB;

		reg_backup = PHY_QueryBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f);
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, 0x0000000f);

		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 1);

		while (i < 5) {
			delay_ms(5);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 0);
			val32 = PHY_QueryBBReg(priv, tssi_reg, bMaskDWord);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 1);
			tssi = ((val32 & 0x04000000) >> 20) |
				   ((val32 & 0x00600000) >> 17) |
				   ((val32 & 0x00000c00) >> 8)  |
				   ((val32 & 0x00000060) >> 5);
			if (tssi) {
				tssi_total += tssi;
				i++;
			}
		}

		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, reg_backup);

		tssi = tssi_total / 5;
	}

	sprintf(data, "%d", tssi);
	return strlen(data)+1;
}


int mp_query_ther(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int ther=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	// enable power and trigger
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);

	// delay for 1 second
	delay_ms(1000);

	// query rf reg 0x24[4:0], for thermal meter value
	ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;

	sprintf(data, "%d", ther);
	return strlen(data)+1;
}


#ifdef B2B_TEST
/* Do checksum and verification for configuration data */
static unsigned char byte_checksum(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}

static int is_byte_checksum_ok(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}


static void mp_init_sta(struct rtl8192cd_priv *priv,unsigned char *da_mac)
{
	struct stat_info *pstat;
	unsigned char *da;

	da = da_mac;
	// prepare station info
	if (memcmp(da, "\x0\x0\x0\x0\x0\x0", 6) && !IS_MCAST(da))
	{
		pstat = get_stainfo(priv, da);
		if (pstat == NULL)
		{
			pstat = alloc_stainfo(priv, da, -1);
			pstat->state = WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE;
			memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
			pstat->bssratelen = AP_BSSRATE_LEN;
			pstat->expire_to = 30000;
			list_add_tail(&pstat->asoc_list, &priv->asoc_list);
			cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
			if (QOS_ENABLE)
				pstat->QosEnabled = 1;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				pstat->ht_cap_len = priv->ht_cap_len;
				memcpy(&pstat->ht_cap_buf, &priv->ht_cap_buf, priv->ht_cap_len);
			}
			pstat->current_tx_rate = priv->pshare->mp_datarate;
			update_fwtbl_asoclst(priv, pstat);
//			add_update_RATid(priv, pstat);
		}
	}
}


/*
 * tx pakcet.
 *  command: "iwpriv wlanx mp_tx,da=xxx,time=n,count=n,len=n,retry=n,tofr=n,wait=n,delay=n,err=n"
 *		default: da=ffffffffffff, time=0,count=1000, len=1500, retry=6, tofr=0, wait=0, delay=0(ms), err=1
 *		note: if time is set, it will take time (in sec) rather count.
 *		     if "time=-1", tx will continue tx until ESC. If "err=1", display statistics when tx err.
 */
int mp_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int orgTCR = RTL_R32(TCR);
	unsigned char increaseIFS=0; // set to 1 to increase the inter frame spacing while in PER test
	unsigned char pbuf[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	int count=1000, payloadlen=1500, retry=6, tofr=0, wait=0, delay=0, time=0;
	int err=1;
	struct sk_buff *skb;
	struct wlan_ethhdr_t *pethhdr;
	int len, i, q_num, ret, resent;
	unsigned char pattern=0xab;
	char *val;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	static int last_tx_err;
	unsigned long end_time=0, flags;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	// get da
	val = get_value_by_token((char *)data, "da=");
	if (val) {
		ret = get_array_val(pbuf, val, 12);
		if (ret != 6) {
			printk("Error da format\n");
			return 0;
		}
	}

	// get time
	val = get_value_by_token((char *)data, "time=");
	if (val) {
		if (!memcmp(val, "-1", 2))
			time = -1;
		else {
			time = _atoi(val, 10);
			time = time*100;	// in 10ms
		}
	}

	// get count
	val = get_value_by_token((char *)data, "count=");
	if (val) {
		count = _atoi(val, 10);
	}

	// get payload len
	val = get_value_by_token((char *)data, "len=");
	if (val) {
		payloadlen = _atoi(val, 10);
		if (payloadlen < 20) {
			printk("len should be greater than 20!\n");
			return 0;
		}
	}

	// get retry number
	val = get_value_by_token((char *)data, "retry=");
	if (val) {
		retry = _atoi(val, 10);
	}

	// get tofr
	val = get_value_by_token((char *)data, "tofr=");
	if (val) {
		tofr = _atoi(val, 10);
	}

	// get wait
	val = get_value_by_token((char *)data, "wait=");
	if (val) {
		wait = _atoi(val, 10);
	}

	// get err
	val = get_value_by_token((char *)data, "err=");
	if (val) {
		err = _atoi(val, 10);
	}

	len = payloadlen + WLAN_ETHHDR_LEN;
	q_num = BE_QUEUE;

	if (time)
		printk("Start TX DA=%02x%02x%02x%02x%02x%02x len=%d tofr=%d retry=%d wait=%s time=%ds",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
			payloadlen, tofr, retry, (wait ? "yes" : "no"), ((time > 0) ? time/100 : -1));
	else
		printk("Start TX DA=%02x%02x%02x%02x%02x%02x count=%d len=%d tofr=%d retry=%d wait=%s",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
			count, payloadlen, tofr, retry, (wait ? "yes" : "no"));

#if defined(USE_RTL8186_SDK)
	printk(", press any key to escape.\n");
#else
	printk("\n");
#endif

	RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_BEDOK);
//	RTL_W32(_RCR_, _NO_ERLYRX_);
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

	if (increaseIFS) {
//		RTL_W32(_TCR_, RTL_R32(_TCR_) | _DISCW_);
		RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) | DIS_CW);
	}

	memset(&priv->net_stats, 0, sizeof(struct net_device_stats));
	priv->ext_stats.tx_retrys=0;
	last_tx_err = 0;

	if (time > 0) {
		end_time = jiffies + time;
	}

	i = 0;
	resent = 0;

#if defined(USE_RTL8186_SDK)
	DISABLE_UART0_INT();
#endif

	mp_init_sta(priv, &pbuf[0]);

	while (1)
	{
#if defined(USE_RTL8186_SDK)
		if ( IS_KEYBRD_HIT())
			break;
#endif

		if (time) {
			if (time != -1) {
				if (jiffies > end_time)
					break;
			}
		}
		else {
			if (!resent && i >= count)
				break;
		}
		if (!resent)
			i++;

		skb = dev_alloc_skb(len);

		if (skb != NULL)
		{
			DECLARE_TXINSN(txinsn);

			skb->dev = priv->dev;
			skb_put(skb, len);

			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
			memcpy((void *)pethhdr->daddr, pbuf, MACADDRLEN);
			memcpy((void *)pethhdr->saddr, BSSID, MACADDRLEN);
			pethhdr->type = payloadlen;

			// construct tx patten
			memset(skb->data+WLAN_ETHHDR_LEN, pattern, payloadlen);

			memcpy(skb->data+WLAN_ETHHDR_LEN, MP_PACKET_HEADER, MP_PACKET_HEADER_LEN); // header
			memcpy(skb->data+WLAN_ETHHDR_LEN+12, &i, 4); // packet sequence
			skb->data[len-1] = byte_checksum(skb->data+WLAN_ETHHDR_LEN,	payloadlen-1); // checksum

			txinsn.q_num	= q_num; //using low queue for data queue
			txinsn.fr_type	= _SKB_FRAME_TYPE_;
			txinsn.pframe	= skb;
			txinsn.tx_rate	= txinsn.lowest_tx_rate = priv->pshare->mp_datarate;
			txinsn.fixed_rate = 1;
			txinsn.retry	= retry;
			txinsn.phdr		= get_wlanllchdr_from_poll(priv);
			skb->cb[1] = 0;

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));

			if (tofr & 2)
				SetToDs(txinsn.phdr);
			if (tofr & 1)
				SetFrDs(txinsn.phdr);

			SetFrameType(txinsn.phdr, WIFI_DATA);

			if (wait) {
				while (1) {
					volatile unsigned int head, tail;
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail)
						break;
					delay_ms(1);
				}
			}

			if(rtl8192cd_firetx(priv, &txinsn) == CONGESTED)
			{
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);
				if (skb)
					rtl_kfree_skb(priv, skb, _SKB_TX_);

				//printk("CONGESTED : busy waiting...\n");
				delay_ms(1);
				resent = 1;

				SAVE_INT_AND_CLI(flags);
				rtl8192cd_tx_dsr((unsigned long)priv);
				RESTORE_INT(flags);
			}
			else {
				SAVE_INT_AND_CLI(flags);
				rtl8192cd_tx_dsr((unsigned long)priv);
				RESTORE_INT(flags);

				if (err && ((int)priv->net_stats.tx_errors) != last_tx_err) { // err happen
					printk("\tout=%d\tfail=%d\n", (int)priv->net_stats.tx_packets,
							(int)priv->net_stats.tx_errors);
					last_tx_err = (int)priv->net_stats.tx_errors;
				}
				else {
					if ( (i%10000) == 0 )
						printk("Tx status: ok=%d\tfail=%d\tretry=%ld\n", (int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
							(int)priv->net_stats.tx_errors, priv->ext_stats.tx_retrys);
				}
				resent = 0;
			}
			if (delay)
				delay_ms(delay);
		}
		else
		{
			printk("Can't allocate sk_buff\n");
			delay_ms(1);
			resent = 1;

			SAVE_INT_AND_CLI(flags);
			rtl8192cd_tx_dsr((unsigned long)priv);
			RESTORE_INT(flags);
		}
	}

#if defined(USE_RTL8186_SDK)
	RESTORE_UART0_INT();
#endif

	// wait till all tx is done
	printk("\nwaiting tx is finished...");
	i = 0;
	while (1) {
		volatile unsigned int head, tail;
		head = get_txhead(phw, BE_QUEUE);
		tail = get_txtail(phw, BE_QUEUE);
		if (head == tail)
			break;

		SAVE_INT_AND_CLI(flags);
		rtl8192cd_tx_dsr((unsigned long)priv);
		RESTORE_INT(flags);

		delay_ms(1);

		if (i++ >10000)
			break;
	}
	printk("done.\n");

	RTL_W32(TCR, orgTCR);

	sprintf(data, "Tx result: ok=%d,fail=%d", (int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
			(int)priv->net_stats.tx_errors);
	return strlen(data)+1;
}


/*
 * validate rx packet. rx packet format:
 *	|wlan-header(24 byte)|MP_PACKET_HEADER (12 byte)|sequence(4 bytes)|....|checksum(1 byte)|
 *
 */
void mp_validate_rx_packet(struct rtl8192cd_priv *priv, unsigned char *data, int len)
{
	int tofr = get_tofr_ds(data);
	unsigned int type=GetFrameType(data);
	int header_size = 24;
	unsigned long sequence;
	unsigned short fr_seq;

	if (!priv->pshare->mp_rx_waiting)
		return;

	if (type != WIFI_DATA)
		return;

	fr_seq = GetTupleCache(data);
	if (GetRetry(data) && fr_seq == priv->pshare->mp_cached_seq) {
		priv->pshare->mp_rx_dup++;
		return;
	}

	if (tofr == 3)
		header_size = 30;

	if (len < (header_size+20) )
		return;

	// see if test header matched
	if (memcmp(&data[header_size], MP_PACKET_HEADER, MP_PACKET_HEADER_LEN))
		return;

	priv->pshare->mp_cached_seq = fr_seq;

	memcpy(&sequence, &data[header_size+MP_PACKET_HEADER_LEN], 4);

	if (!is_byte_checksum_ok(&data[header_size], len-header_size)) {
#if 0
		printk("mp_rx: checksum error!\n");
#endif
		printk("mp_brx: checksum error!\n");
	}
	else {
		if (sequence <= priv->pshare->mp_rx_sequence) {
#if 0
			printk("mp_rx: invalid sequece (%ld) <= current (%ld)!\n",
									sequence, priv->pshare->mp_rx_sequence);
#endif
			printk("mp_brx: invalid sequece (%ld) <= current (%ld)!\n",
									sequence, priv->pshare->mp_rx_sequence);
		}
		else {
			if (sequence > (priv->pshare->mp_rx_sequence+1))
				priv->pshare->mp_rx_lost_packet += (sequence-priv->pshare->mp_rx_sequence-1);
			priv->pshare->mp_rx_sequence = sequence;
			priv->pshare->mp_rx_ok++;
		}
	}
}


#if 0
/*
 * Rx test packet.
 *   command: "iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]"
 *	           - ra: rx mac. defulat is burn-in mac
 *             - quiet_time: quit rx if no rx packet during quiet_time. default is 5s
 *             - interval: report rx statistics periodically in sec. default is 0 (no report)
 */
static void mp_rx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned char pbuf[6];
	int quiet_time=5, interval_time=0, quiet_period=0, interval_period=0, ret;
	unsigned int o_rx_ok, o_rx_lost_packet, mac_changed=0;
	unsigned long reg, counter=0;

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// get ra
	val = get_value_by_token((char *)data, "ra=");
	if (val) {
		ret =0;
		if (strlen(val) >=12)
			ret = get_array_val(pbuf, val, 12);
		if (ret != 6) {
			printk("Error mac format\n");
			return;
		}
		printk("set ra to %02X:%02X:%02X:%02X:%02X:%02X\n",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

		memcpy(&reg, pbuf, 4);
		RTL_W32(_IDR0_, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
		mac_changed = 1;
	}

	// get quiet time
	val = get_value_by_token((char *)data, "quiet=");
	if (val)
		quiet_time = _atoi(val, 10);

	// get interval time
	val = get_value_by_token((char *)data, "interval=");
	if (val)
		interval_time = _atoi(val, 10);

	RTL_W32(_RCR_, _ENMARP_ | _APWRMGT_ | _AMF_ | _ADF_ | _NO_ERLYRX_ |
			_RX_DMA64_ | _ACRC32_ | _AB_ | _AM_ | _APM_ | _AAP_);

	priv->pshare->mp_cached_seq = 0;
	priv->pshare->mp_rx_ok = 0;
	priv->pshare->mp_rx_sequence = 0;
	priv->pshare->mp_rx_lost_packet = 0;
	priv->pshare->mp_rx_dup = 0;

	printk("Waiting for rx packet, quit if no packet in %d sec", quiet_time);

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
	printk(", or press any key to escape.\n");
	DISABLE_UART0_INT();
#else
	printk(".\n");
#endif

	priv->pshare->mp_rx_waiting = 1;

	while (1) {
		// save old counter
		o_rx_ok = priv->pshare->mp_rx_ok;
		o_rx_lost_packet = priv->pshare->mp_rx_lost_packet;

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
		if ( IS_KEYBRD_HIT())
			break;
#endif

		delay_ms(1000);

		if (interval_time && ++interval_period == interval_time) {
			printk("\tok=%ld\tlost=%ld\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet);
			interval_period=0;
		}
		else {
			if ((priv->pshare->mp_rx_ok-counter) > 10000) {
				printk("Rx status: ok=%ld\tlost=%ld, duplicate=%ld\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->pshare->mp_rx_dup);
				counter += 10000;
			}
		}

		if (o_rx_ok == priv->pshare->mp_rx_ok && o_rx_lost_packet == priv->pshare->mp_rx_lost_packet)
			quiet_period++;
		else
			quiet_period = 0;

		if (quiet_period >= quiet_time)
			break;
	}

//	printk("\nRx result: ok=%ld\tlost=%ld, duplicate=%ld\n\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->mp_rx_dup);
	printk("\nRx reseult: ok=%ld\tlost=%ld\n\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet);

	priv->pshare->mp_rx_waiting = 0;

	if (mac_changed) {
		memcpy(pbuf, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);

		memcpy(&reg, pbuf, 4);
		RTL_W32(_IDR0_, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
	}

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
	RESTORE_UART0_INT();
#endif

}
#endif


/*
 * Rx test packet.
 *   command: "iwpriv wlanx mp_brx start[,ra=xxxxxx]"
 *	           - ra: rx mac. defulat is burn-in mac
 *   command: "iwpriv wlanx mp_brx stop"
 *               - stop rx immediately
 */
int mp_brx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned char pbuf[6];
	int ret;
	unsigned long reg;
	unsigned long	flags;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	SAVE_INT_AND_CLI(flags);

	if (!strcmp(data, "stop"))
		goto stop_brx;

	// get start
	val = get_value_by_token((char *)data, "start");
	if (val) {
		// get ra if it exists
		val = get_value_by_token((char *)data, "ra=");
		if (val) {
			ret =0;
			if (strlen(val) >=12)
				ret = get_array_val(pbuf, val, 12);
			if (ret != 6) {
				printk("Error mac format\n");
				RESTORE_INT(flags);
				return 0;
			}
			printk("set ra to %02X:%02X:%02X:%02X:%02X:%02X\n",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

			memcpy(&reg, pbuf, 4);
			RTL_W32(MACID, (cpu_to_le32(reg)));

			memcpy(&reg, pbuf+4, 4);
			RTL_W32(MACID + 4, (cpu_to_le32(reg)));
			priv->pshare->mp_mac_changed = 1;
		}
	}
	else {
		RESTORE_INT(flags);
		return 0;
	}

	priv->pshare->mp_cached_seq = 0;
	priv->pshare->mp_rx_ok = 0;
	priv->pshare->mp_rx_sequence = 0;
	priv->pshare->mp_rx_lost_packet = 0;
	priv->pshare->mp_rx_dup = 0;
	priv->pshare->mp_rx_waiting = 1;

	// record the start time of MP throughput test
	priv->pshare->txrx_start_time = jiffies;

	OPMODE |= WIFI_MP_RX;
//	RTL_W32(RCR, _ENMARP_ | _APWRMGT_ | _AMF_ | _ADF_ | _NO_ERLYRX_ |
//			_RX_DMA64_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
	RTL_W32(RCR, RCR_HTC_LOC_CTRL | RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_APWRMGT | RCR_AB | RCR_AM | RCR_APM);

	memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));

	RESTORE_INT(flags);
	return 0;

stop_brx:
	OPMODE &= ~WIFI_MP_RX;

//	RTL_W32(RCR, _ENMARP_ | _NO_ERLYRX_ | _RX_DMA64_);
	RTL_W32(RCR, RCR_HTC_LOC_CTRL);
	priv->pshare->mp_rx_waiting = 0;

	//record the elapsed time of MP throughput test
	priv->pshare->txrx_elapsed_time = jiffies - priv->pshare->txrx_start_time;

	if (priv->pshare->mp_mac_changed) {
		memcpy(pbuf, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);

		memcpy(&reg, pbuf, 4);
		RTL_W32(MACID, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(MACID + 4, (cpu_to_le32(reg)));
	}
	priv->pshare->mp_mac_changed = 0;

	RESTORE_INT(flags);

	sprintf(data, "Rx reseult: ok=%ld,lost=%ld,elapsed time=%ld",
		priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->pshare->txrx_elapsed_time);
	return strlen(data)+1;
}
#endif // B2B_TEST


void mp_set_bandwidth(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int use40M=0, shortGI=0;
//	unsigned int regval, i, val32;
//	unsigned char *CCK_SwingEntry;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// get 20M~40M , 40M=1 or 40M=0(20M)
	val = get_value_by_token((char *)data, "40M=");
	if (val) {
		use40M = _atoi(val, 10);
	}

    // get shortGI=1 or 0.
	val = get_value_by_token((char *)data, "shortGI=");
	if (val) {
		shortGI = _atoi(val, 10);
	}

	// modify short GI
	if(shortGI)
	{
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
	}
	else
	{
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 0;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 0;
	}

	// modify 40M
	if (use40M) {
		priv->pshare->is_40m_bw = 1;
		priv->pshare->CurrentChannelBW = 1;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
	}
	else {
		priv->pshare->is_40m_bw = 0;
		priv->pshare->CurrentChannelBW = 0;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
	}

	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);

//	mp_8192CD_tx_setting(priv);
	return; // end here
}


/*
 * auto-rx
 * accept CRC32 error pkt
 * accept destination address pkt
 * drop tx pkt (implemented in other functions)
 */
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
int mp_arx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	if (!strcmp(data, "start")) {
		OPMODE |= WIFI_MP_RX;
		RTL_W32(RCR, RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP);
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			PHY_SetBBReg(priv, rCCK0_System, bCCKEqualizer, 0x0);

		memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
		memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));
	}
	else if (!strcmp(data, "stop")) {
		OPMODE &= ~WIFI_MP_RX;
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			PHY_SetBBReg(priv, rCCK0_System, bCCKEqualizer, 0x1);

//clear filter:
		if((priv->pshare->mp_filter_flag) || (OPMODE & WIFI_MP_ARX_FILTER))
		{
			OPMODE &= ~WIFI_MP_ARX_FILTER;
			priv->pshare->mp_filter_flag=0;
			memset(priv->pshare->mp_filter_DA,0,6);
			memset(priv->pshare->mp_filter_SA,0,6);
			memset(priv->pshare->mp_filter_BSSID,0,6);
		}

		sprintf(data, "Received packet OK:%lu  CRC error:%lu\n", priv->net_stats.rx_packets, priv->net_stats.rx_crc_errors);
		return strlen(data)+1;
	}
	else if (!memcmp(data, "filter_SA",9))
	{
		OPMODE |= WIFI_MP_ARX_FILTER;
		memset(priv->pshare->mp_filter_SA,'\0',6);

		priv->pshare->mp_filter_flag |=0x1;
		val = get_value_by_token((char *)data, "filter_SA=");
		val[2]='\0';
		priv->pshare->mp_filter_SA[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_SA[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_SA[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_SA[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_SA[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_SA[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nSA: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_SA[0],
															priv->pshare->mp_filter_SA[1],
															priv->pshare->mp_filter_SA[2],
															priv->pshare->mp_filter_SA[3],
															priv->pshare->mp_filter_SA[4],
															priv->pshare->mp_filter_SA[5]);
		return strlen(data)+1;

	}
	else if (!memcmp(data, "filter_DA",9))
	{
		OPMODE |= WIFI_MP_ARX_FILTER;
		priv->pshare->mp_filter_flag |=0x2;
		memset(priv->pshare->mp_filter_DA,'\0',6);

		val = get_value_by_token((char *)data, "filter_DA=");
		val[2]='\0';
		priv->pshare->mp_filter_DA[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_DA[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_DA[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_DA[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_DA[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_DA[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nDA: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_DA[0],
															priv->pshare->mp_filter_DA[1],
															priv->pshare->mp_filter_DA[2],
															priv->pshare->mp_filter_DA[3],
															priv->pshare->mp_filter_DA[4],
															priv->pshare->mp_filter_DA[5]);
		return strlen(data)+1;

	}
	else if (!memcmp(data, "filter_BSSID",12))
	{
		OPMODE |= WIFI_MP_ARX_FILTER;
		priv->pshare->mp_filter_flag |=0x4;
		memset(priv->pshare->mp_filter_BSSID,'\0',6);

		val = get_value_by_token((char *)data, "filter_BSSID=");
		val[2]='\0';
		priv->pshare->mp_filter_BSSID[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_BSSID[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_BSSID[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_BSSID[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_BSSID[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_BSSID[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_BSSID[0],
															priv->pshare->mp_filter_BSSID[1],
															priv->pshare->mp_filter_BSSID[2],
															priv->pshare->mp_filter_BSSID[3],
															priv->pshare->mp_filter_BSSID[4],
															priv->pshare->mp_filter_BSSID[5]);
		return strlen(data)+1;

	}
	else if( (!memcmp(data, "filter_clean",12))  || (!memcmp(data, "filter_init",11)) )
	{
		OPMODE &=  (~WIFI_MP_ARX_FILTER);
		priv->pshare->mp_filter_flag=0;
		memset(priv->pshare->mp_filter_SA,'\0',6);
		memset(priv->pshare->mp_filter_DA,'\0',6);
		memset(priv->pshare->mp_filter_BSSID,'\0',6);
		sprintf(data,"reset arx filter table\n");
		return strlen(data)+1;
	}
	else if (!memcmp(data, "filter_print",12))
	{

		sprintf(data,"flag: %x\nSA: %02x:%02x:%02x:%02x:%02x:%02x\nDA: %02x:%02x:%02x:%02x:%02x:%02x\nBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_SA[0],
															priv->pshare->mp_filter_SA[1],
															priv->pshare->mp_filter_SA[2],
															priv->pshare->mp_filter_SA[3],
															priv->pshare->mp_filter_SA[4],
															priv->pshare->mp_filter_SA[5],
															priv->pshare->mp_filter_DA[0],
															priv->pshare->mp_filter_DA[1],
															priv->pshare->mp_filter_DA[2],
															priv->pshare->mp_filter_DA[3],
															priv->pshare->mp_filter_DA[4],
															priv->pshare->mp_filter_DA[5],
															priv->pshare->mp_filter_BSSID[0],
															priv->pshare->mp_filter_BSSID[1],
															priv->pshare->mp_filter_BSSID[2],
															priv->pshare->mp_filter_BSSID[3],
															priv->pshare->mp_filter_BSSID[4],
															priv->pshare->mp_filter_BSSID[5]);

		return strlen(data)+1;
	}

	return 0;
}


/*
 * set bssid
 */
void mp_set_bssid(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char pbuf[6];
	int ret;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	ret = get_array_val(pbuf, (char *)data, strlen(data));

	if (ret != 6) {
		printk("Error bssid format\n");
		return;
	}
	else
		printk("set bssid to %02X:%02X:%02X:%02X:%02X:%02X\n",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

	memcpy(BSSID, pbuf, MACADDRLEN);
}


static void mp_chk_sw_ant(struct rtl8192cd_priv *priv)
{
	R_ANTENNA_SELECT_OFDM	*p_ofdm_tx;	/* OFDM Tx register */
	R_ANTENNA_SELECT_CCK	*p_cck_txrx;
	unsigned char			r_rx_antenna_ofdm=0, r_ant_select_cck_val=0;
	unsigned char			chgTx=0, chgRx=0;
	unsigned int			r_ant_sel_cck_val=0, r_ant_select_ofdm_val=0, r_ofdm_tx_en_val=0;

	p_ofdm_tx = (R_ANTENNA_SELECT_OFDM *)&r_ant_select_ofdm_val;
	p_cck_txrx = (R_ANTENNA_SELECT_CCK *)&r_ant_select_cck_val;

	p_ofdm_tx->r_ant_ht1			= 0x1;
	p_ofdm_tx->r_ant_non_ht 		= 0x3;
	p_ofdm_tx->r_ant_ht2			= 0x2;

	// 原因是Tx 3-wire enable需隨Tx Ant path打開才會開啟，
	// 所以需在設BB 0x824與0x82C時，同時將BB 0x804[3:0]設為3(同時打開Ant. A and B)。
	// 要省電的情況下，A Tx時 0x90C=0x11111111，B Tx時 0x90C=0x22222222，AB同時開就維持原來設定0x3321333


	switch(priv->pshare->mp_antenna_tx)
	{
	case ANTENNA_A:
		p_ofdm_tx->r_tx_antenna		= 0x1;
		r_ofdm_tx_en_val			= 0x1;
		p_ofdm_tx->r_ant_l 			= 0x1;
		p_ofdm_tx->r_ant_ht_s1 		= 0x1;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x1;
		p_cck_txrx->r_ccktx_enable	= 0x8;
		chgTx = 1;
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 2);
		PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 1);
		r_ofdm_tx_en_val			= 0x3;
		// Power save
		r_ant_select_ofdm_val		= 0x11111111;

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 1);
			PHY_SetBBReg(priv, 0x870, BIT(10), 0);
		}
#endif
		break;
	case ANTENNA_B:
		p_ofdm_tx->r_tx_antenna		= 0x2;
		r_ofdm_tx_en_val			= 0x2;
		p_ofdm_tx->r_ant_l 			= 0x2;
		p_ofdm_tx->r_ant_ht_s1 		= 0x2;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x2;
		p_cck_txrx->r_ccktx_enable	= 0x4;
		chgTx = 1;
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 1);
		PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 2);
		r_ofdm_tx_en_val			= 0x3;
		// Power save
		r_ant_select_ofdm_val		= 0x22222222;

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		}
#endif
		break;
	case ANTENNA_AB:
		p_ofdm_tx->r_tx_antenna		= 0x3;
		r_ofdm_tx_en_val			= 0x3;
		p_ofdm_tx->r_ant_l 			= 0x3;
		p_ofdm_tx->r_ant_ht_s1 		= 0x3;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x3;
		p_cck_txrx->r_ccktx_enable	= 0xC;
		chgTx = 1;
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 2);
		PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 2);
		// Disable Power save
		r_ant_select_ofdm_val		= 0x3321333;

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			PHY_SetBBReg(priv, 0x870, BIT(10), 0);
		}
#endif
		break;
	default:
		break;
	}

	//
	// r_rx_antenna_ofdm, bit0=A, bit1=B, bit2=C, bit3=D
	// r_cckrx_enable : CCK default, 0=A, 1=B, 2=C, 3=D
	// r_cckrx_enable_2 : CCK option, 0=A, 1=B, 2=C, 3=D
	//
	switch(priv->pshare->mp_antenna_rx)
	{
	case ANTENNA_A:
		r_rx_antenna_ofdm 			= 0x1;	// A
		p_cck_txrx->r_cckrx_enable 	= 0x0;	// default: A
		p_cck_txrx->r_cckrx_enable_2= 0x0;	// option: A
		chgRx = 1;
		PHY_SetRFReg(priv, 0, 7, 0x3, 0x0);
		break;
	case ANTENNA_B:
		r_rx_antenna_ofdm 			= 0x2;	// B
		p_cck_txrx->r_cckrx_enable 	= 0x1;	// default: B
		p_cck_txrx->r_cckrx_enable_2= 0x1;	// option: B
		chgRx = 1;
		PHY_SetRFReg(priv, 0, 7, 0x3, 0x1);
		break;
	case ANTENNA_AB:	// For 8192S and 8192E/U...
		r_rx_antenna_ofdm 			= 0x3;	// AB
		p_cck_txrx->r_cckrx_enable 	= 0x0;	// default:A
		p_cck_txrx->r_cckrx_enable_2= 0x1;		// option:B
		chgRx = 1;
		break;
	}

	if(chgTx && chgRx)
	{
		r_ant_sel_cck_val = r_ant_select_cck_val;//(r_ant_select_cck_val<<24);
		PHY_SetBBReg(priv, rFPGA1_TxInfo, 0x0fffffff, r_ant_select_ofdm_val);		//OFDM Tx
		PHY_SetBBReg(priv, rFPGA0_TxInfo, 0x0000000f, r_ofdm_tx_en_val);		//OFDM Tx
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	//OFDM Rx
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	//OFDM Rx
		PHY_SetBBReg(priv, rCCK0_AFESetting, bMaskByte3, r_ant_sel_cck_val);		//CCK TxRx
	}
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void mp_set_ant_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	if (!strcmp(data, "a"))
		priv->pshare->mp_antenna_tx = ANTENNA_A;
	else if (!strcmp(data, "b"))
		priv->pshare->mp_antenna_tx = ANTENNA_B;
	else if  (!strcmp(data, "ab"))
		priv->pshare->mp_antenna_tx = ANTENNA_AB;
	else {
		printk("Usage: mp_ant_tx {a,b,ab}\n");
		return;
	}

	mp_chk_sw_ant(priv);
	mp_8192CD_tx_setting(priv);

	printk("switch Tx antenna to %s\n", data);
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void mp_set_ant_rx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	if (!strcmp(data, "a"))
		priv->pshare->mp_antenna_rx = ANTENNA_A;
	else if (!strcmp(data, "b"))
		priv->pshare->mp_antenna_rx = ANTENNA_B;
	else if (!strcmp(data, "ab"))
		priv->pshare->mp_antenna_rx = ANTENNA_AB;
	else {
		printk("Usage: mp_ant_rx {a,b,ab}\n");
		return;
	}

	mp_chk_sw_ant(priv);
	printk("switch Rx antenna to %s\n", data);
}


/*
void mp_set_phypara(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int xcap=-32, sign;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}
	// get CrystalCap value
	val = get_value_by_token((char *)data, "xcap=");
	if (val) {
		if (*val == '-') {
			sign = 1;
			val++;
		}
		else
			sign = 0;

		xcap = _atoi(val, 10);
		if (sign)
			xcap = 0 - xcap;
	}

	// set CrystalCap value
	if (xcap != -32) {
		PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, bXtalCap01, (xcap & 0x00000003));
		PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, bXtalCap23, ((xcap & 0x0000000c) >> 2));
	}

	printk("Set xcap=%d\n", xcap);
}
*/


#endif	// MP_TEST

