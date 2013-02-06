#ifndef __RTL_START_H__
#define __RTL_START_H__

#include <linux/config.h>

//----- System Registers ------------------------------------------------
#define REG_REVISION	0xbd01010c
#define	REG_PLLMNR		0xbd010104
#define	REG_SYSCLKR		0xbd010108
#define	REG_GPEFDIR		0xbd010144
#define	REG_GPEFDATA	0xbd010140
#define	REG_MTCR0		0xbd011004
#define	REG_MCR			0xbd011000
#define	REG_MTCR1		0xbd011008
#define	REG_TKNR		0xbd010110
#define	REG_GISR		0xbd010004
#define	REG_WDTCNR		0xbd01005c
#define	REG_CDBR		0xbd010058

//----- Revision bit ----------------------------------------------------
#define VERSION_B		0x0		//0000
#define	VERSION_C		0x8		//1000
#define VERSION_D		0x4		//0100

//----- DPLL and Clock Setting ------------------------------------------
#define VAL_PLLMNR_8186B	0x36704		// 160/128
#define VAL_SYSCLKR_8186B	0xa0b		// CPU=320/2 MEM=320/2.5

#define VAL_PLLMNR_8186C	0x35a03		// 180/108
#define VAL_SYSCLKR_8186C	0x090b		// CPU=270/1.5 MEM=270/2.5

//----- Memory Setting --------------------------------------------------
#define	VAL_FLASH_TIMING	0x88880000
//#define	VAL_FLASH_TIMING	0x33330000	//high speed.


#if (defined(CONFIG_SKIP_ADJUST_SYSCLK) )
	#define CONFIG_DONOT_ADJUST_SYSCLK_MEMSP
#endif
//----- Others ----------------------------------------------------------
#define PCI_LOW_COUNTER	0xFFFF
#define	BOOT_ADDR		0x80100000

#endif

