/*
* RTL8187 USB Wireless LAN Driver (for RTL865xC FT2 Only)
* Copyright (c) 2007 Realtek Semiconductor Corporation.
*
* Program : Header File of RTL8187 Driver (for RTL865xC FT2 Only)
* Abstract :
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: rtl8187.h,v 1.1.1.1 2007/08/06 10:04:45 root Exp $
*/

#ifndef	__RTL8187_H__
#define	__RTL8187_H__

/* RTL8187 Basic Define */
#define	RTL8187_MODULE_NAME	"rtl8187"
#define	RTL8187_MTU		1500
#define	RTL8187_URB_RX_MAX	2500
#define	RTL8187_URB_TX_MAX	2500
#define	RTL8187_REQT_READ	0xc0
#define	RTL8187_REQT_WRITE	0x40
#define	RTL8187_REQ_GET_REGS	0x05
#define	RTL8187_REQ_SET_REGS	0x05
#define	RTL8187_TX_TIMEOUT	(HZ)

/* rtl8187 flags */
#define	RTL8187_FLAG_HWCRC	0
#define	RTL8187_UNPLUG		1

/* Define these values to match your device */
#define VENDOR_ID_REALTEK	0x0bda
#define PRODUCT_ID_RTL8187	0x8187

/* Registers */
#define	REG_IDR0		0xff00		/* (4 Bytes) */
#define	REG_IDR4		0xff04		/* (2 Bytes) */
#define	REG_TSFTR		0xff18		/* (8 Bytes) */
#define	REG_BRSR		0xff2c		/* (2 Bytes) */
#define	REG_BSSID		0xff2e		/* (6 Bytes) */
#define	REG_RR			0xff34		/* (1 Byte) */
#define	REG_CR			0xff37		/* (1 Byte) */
#define	REG_IMR			0xff3c		/* (2 Bytes) */
#define	REG_ISR			0xff3e		/* (2 Bytes) */
#define	REG_TCR			0xff40		/* (4 Bytes) */
#define	REG_RCR			0xff44		/* (4 Bytes) */
#define	REG_TIMERINT		0xff48		/* (4 Bytes) */
#define	REG_EEPROMCR		0xff50		/* (1 Byte) */
#define	REG_CONFIG0		0xff51		/* (1 Byte) */
#define	REG_CONFIG1		0xff52		/* (1 Byte) */
#define	REG_CONFIG2		0xff53		/* (1 Byte) */
#define	REG_MSR			0xff58		/* (1 Byte) */
#define	REG_CONFIG3		0xff59		/* (1 Byte) */
#define	REG_CONFIG4		0xff5a		/* (1 Byte) */
#define	REG_PSR			0xff5e		/* (1 Byte) */
#define	REG_BIR			0xff70		/* (2 Bytes) */
#define	REG_ATIMWR		0xff72		/* (2 Bytes) */
#define	REG_BIIR		0xff74		/* (2 Bytes) */
#define	REG_ATIMIIR		0xff77		/* (2 Bytes) */
#define	REG_RFCR		0xff80		/* (8 Bytes) */
#define	REG_RFPARA		0xff88		/* (4 Bytes) */
#define	REG_RFTR		0xff8c		/* (4 Bytes) */
#define	REG_PHYMUXPAR		0xff94		/* (4 Bytes) */
#define	REG_TXAGCCR		0xff9c		/* (3 Bytes) */
#define	REG_ANTSEL		0xff9f		/* (1 Byte) */
#define	REG_CAMRW		0xffa0		/* (4 Bytes) */
#define	REG_CAMDEBUG		0xffac		/* (4 Bytes) */
#define	REG_WPACONFIG		0xffb0		/* (2 Bytes) */
#define	REG_AESMASKFC		0xffb2		/* (2 Bytes) */
#define	REG_AESMASKSC		0xffe2		/* (2 Bytes) */
#define	REG_CWCONFIG		0xffbc		/* (1 Byte) */
#define	REG_CWVALUES		0xffbd		/* (1 Byte) */
#define	REG_RFC			0xffbe		/* (1 Byte) */
#define	REG_CONFIG5		0xffd8		/* (1 Byte) */
#define	REG_CWR			0xffdc		/* (2 Bytes) */
#define	REG_RCTR		0xffde		/* (1 Byte) */
#define	REG_TCTR		0xfffa		/* (2 Bytes) */
#define	REG_TSEL		0xfffc		/* (1 Byte) */
#define	REG_HWVERID		0xfffe		/* (1 Byte) */
#define	REG_MISC		0xffff		/* (1 Byte) */

/* EEPROM */
#define EPROM_93c46		0
#define EPROM_93c56		1

#define EPROM_CMD				0xff50
#define EPROM_CMD_RESERVED_MASK			((1<<5)|(1<<4))
#define EPROM_CMD_OPERATING_MODE_SHIFT		6
#define EPROM_CMD_OPERATING_MODE_MASK		((1<<7)|(1<<6))
#define EPROM_CMD_CONFIG			0x3
#define EPROM_CMD_NORMAL			0 
#define EPROM_CMD_LOAD				1
#define EPROM_CMD_PROGRAM			2
#define EPROM_CS_SHIFT				3
#define EPROM_CK_SHIFT				2
#define EPROM_W_SHIFT				1
#define EPROM_R_SHIFT				0
#define EPROM_DELAY				10
#define EPROM_ANAPARAM_ADDRLWORD		0xd
#define EPROM_ANAPARAM_ADDRHWORD		0xe
#define ENERGY_TRESHOLD				0x17
#define EPROM_VERSION				0x1E

/* EEPROM Memory */
#define	EPR_MACADDR		0x07		/* 0f-0eh (Word) */


#endif

