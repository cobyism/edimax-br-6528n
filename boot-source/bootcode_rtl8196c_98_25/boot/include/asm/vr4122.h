/* $Id: vr4122.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999 by Michael Klar
 * Copyright (C) 2000 by Michael R. McDonald
 */
#ifndef __ASM_MIPS_VR4122_H 
#define __ASM_MIPS_VR4122_H 

#include <asm/addrspace.h>

/* CPU interrupts */
#define VR41XX_IRQ_SW1       0  /* IP0 - Software interrupt */
#define VR41XX_IRQ_SW2       1  /* IP1 - Software interrupt */
#define VR41XX_IRQ_INT0      2  /* IP2 - All but battery, high speed modem, and real time clock */
#define VR41XX_IRQ_INT1      3  /* IP3 - RTC Long1 (system timer) */
#define VR41XX_IRQ_INT2      4  /* IP4 - RTC Long2 */
#define VR41XX_IRQ_INT3      5  /* IP5 - High Speed Modem */
#define VR41XX_IRQ_INT4      6  /* IP6 - Unused */
#define VR41XX_IRQ_TIMER     7  /* IP7 - Timer interrupt from CPO_COMPARE (Note: RTC Long1 is the system timer.) */

/* Cascaded from VR41XX_IRQ_INT0 (ICU mapped interrupts) */
#define VR41XX_IRQ_BATTERY   8
#define VR41XX_IRQ_POWER     9
#define VR41XX_IRQ_RTCL1     10  /* Use VR41XX_IRQ_INT1 instead. */
#define VR41XX_IRQ_ETIMER    11
#define VR41XX_IRQ_RFU12     12
#define VR41XX_IRQ_RFU13     13
#define VR41XX_IRQ_RFU14     14
#define VR41XX_IRQ_RFU15     15
#define VR41XX_IRQ_GIU       16  /* This is a cascade to IRQs 40-71. Do not use. */
#define VR41XX_IRQ_SIU       17
#define VR41XX_IRQ_WRBERR    18
#define VR41XX_IRQ_SOFT      19
#define VR41XX_IRQ_RFU20     20
#define VR41XX_IRQ_DOZEPIU   21
#define VR41XX_IRQ_RFU22     22
#define VR41XX_IRQ_RFU23     23
#define VR41XX_IRQ_RTCL2     24  /* Use VR41XX_IRQ_INT2 instead. */
#define VR41XX_IRQ_LED       25
#define VR41XX_IRQ_HSP       26  /* Use VR41XX_IRQ_INT3 instead. */
#define VR41XX_IRQ_TCLK      27
#define VR41XX_IRQ_FIR       28
#define VR41XX_IRQ_DSIU      29
#define VR41XX_IRQ_PCIU      30
#define VR41XX_IRQ_RFU31     31
#define VR41XX_IRQ_RFU32     32
#define VR41XX_IRQ_RFU33     33
#define VR41XX_IRQ_RFU34     34
#define VR41XX_IRQ_RFU35     35
#define VR41XX_IRQ_RFU36     36
#define VR41XX_IRQ_RFU37     37
#define VR41XX_IRQ_RFU38     38
#define VR41XX_IRQ_RFU39     39

/* Cascaded from VR41XX_IRQ_GIU */
#define VR41XX_IRQ_GPIO0     40
#define VR41XX_IRQ_GPIO1     41
#define VR41XX_IRQ_GPIO2     42
#define VR41XX_IRQ_GPIO3     43
#define VR41XX_IRQ_GPIO4     44
#define VR41XX_IRQ_GPIO5     45
#define VR41XX_IRQ_GPIO6     46
#define VR41XX_IRQ_GPIO7     47
#define VR41XX_IRQ_GPIO8     48
#define VR41XX_IRQ_GPIO9     49
#define VR41XX_IRQ_GPIO10    50
#define VR41XX_IRQ_GPIO11    51
#define VR41XX_IRQ_GPIO12    52
#define VR41XX_IRQ_GPIO13    53
#define VR41XX_IRQ_GPIO14    54
#define VR41XX_IRQ_GPIO15    55
#define VR41XX_IRQ_GPIO16    56
#define VR41XX_IRQ_GPIO17    57
#define VR41XX_IRQ_GPIO18    58
#define VR41XX_IRQ_GPIO19    59
#define VR41XX_IRQ_GPIO20    60
#define VR41XX_IRQ_GPIO21    61
#define VR41XX_IRQ_GPIO22    62
#define VR41XX_IRQ_GPIO23    63
#define VR41XX_IRQ_GPIO24    64
#define VR41XX_IRQ_GPIO25    65
#define VR41XX_IRQ_GPIO26    66
#define VR41XX_IRQ_GPIO27    67
#define VR41XX_IRQ_GPIO28    68
#define VR41XX_IRQ_GPIO29    69
#define VR41XX_IRQ_GPIO30    70
#define VR41XX_IRQ_GPIO31    71

#ifdef CONFIG_NEC_HARRIER
#define VR41XX_IRQ_4173		VR41XX_IRQ_GPIO1
#define VR41XX_IRQ_MQ200	VR41XX_IRQ_GPIO4
#define VR41XX_IRQ_PCI		VR41XX_IRQ_GPIO5
#define VR41XX_IRQ_SIO		VR41XX_IRQ_GPIO8
#endif

/* Alternative to above GPIO IRQ defines */
#define VR41XX_IRQ_GPIO(pin) ((VR41XX_IRQ_GPIO0) + (pin))

#define VR41XX_IRQ_MAX       71

#ifndef _LANGUAGE_ASSEMBLY
#define __preg8		(volatile unsigned char*)
#define __preg16	(volatile unsigned short*)
#define __preg32	(volatile unsigned int*)
#else
#define __preg8
#define __preg16
#define __preg32
#endif

/* Embedded CPU peripheral registers */

/* Bus Control Unit (BCU) */
#define VR41XX_BCUCNTREG1	__preg16(KSEG1 + 0x0F000000)	/* BCU Control Register 1 */
#define VR41XX_ROMSIZEREG	__preg16(KSEG1 + 0x0F000004)	/* ROM Size Register */
#define VR41XX_ROMSPEEDREG	__preg16(KSEG1 + 0x0F000006)	/* BCU Access Cycle Change Register */
#define VR41XX_BCUSPEEDREG	VR41XX_ROMSPEEDREG		/* BCU Access Cycle Change Register */
#define VR41XX_IO0SPEEDREG	__preg16(KSEG1 + 0x0F000008)	/* I/O Access Cycle Change Register 0 */
#define VR41XX_IO1SPEEDREG	__preg16(KSEG1 + 0x0F00000A)	/* I/O Access Cycle Change Register 1 */
#define VR41XX_REVIDREG		__preg16(KSEG1 + 0x0F000010)	/* Revision ID Register */
#define VR41XX_CLKSPEEDREG	__preg16(KSEG1 + 0x0F000014)	/* Clock Speed Register */
#define VR41XX_BCUCNTREG3	__preg16(KSEG1 + 0x0F000016)	/* BCU Control Register 3 */
#define VR41XX_BCUCACHECNTREG	__preg16(KSEG1 + 0x0F000018)	/* BCU Cache Control Register */

/* DMA Address Unit (DMAAU) */
#define VR41XX_CSIIBALREG	__preg16(KSEG1 + 0x0F000020)	/* CSI reception DMA base address register low */
#define VR41XX_CSIIBAHREG	__preg16(KSEG1 + 0x0F000022)	/* CSI reception DMA base address register high */
#define VR41XX_CSIIALREG	__preg16(KSEG1 + 0x0F000024)	/* CSI reception DMA address register low */
#define VR41XX_CSIIAHREG	__preg16(KSEG1 + 0x0F000026)	/* CSI reception DMA address register high */
#define VR41XX_CSIOBALREG	__preg16(KSEG1 + 0x0F000028)	/* CSI transmission DMA base address register low */
#define VR41XX_CSIOBAHREG	__preg16(KSEG1 + 0x0F00002A)	/* CSI transmission DMA base address register high */
#define VR41XX_CSIOALREG	__preg16(KSEG1 + 0x0F00002C)	/* CSI transmission DMA address register low */
#define VR41XX_CSIOAHREG	__preg16(KSEG1 + 0x0F00002E)	/* CSI transmission DMA address register high */
#define VR41XX_FIRBALREG	__preg16(KSEG1 + 0x0F000030)	/* FIR DMA Base Address Register Low */
#define VR41XX_FIRBAHREG	__preg16(KSEG1 + 0x0F000032)	/* FIR DMA Base Address Register High */
#define VR41XX_FIRALREG		__preg16(KSEG1 + 0x0F000034)	/* FIR DMA Address Register Low */
#define VR41XX_FIRAHREG		__preg16(KSEG1 + 0x0F000036)	/* FIR DMA Address Register High */
#define VR41XX_RAMBALREG	__preg16(KSEG1 + 0x0F0001E0)	/* RAM base address lower address between IO space and RAM */
#define VR41XX_RAMBAHREG	__preg16(KSEG1 + 0x0F0001E2)	/* RAM base address higher address between IO space and RAM */
#define VR41XX_RAMALREG		__preg16(KSEG1 + 0x0F0001E4)	/* RAM address lower address between IO space and RAM */
#define VR41XX_RAMAHREG		__preg16(KSEG1 + 0x0F0001E6)	/* RAM address higher address between IO space and RAM */
#define VR41XX_IOBALREG		__preg16(KSEG1 + 0x0F0001E8)	/* IO base address lower address between IO space and RAM */
#define VR41XX_IOBAHREG		__preg16(KSEG1 + 0x0F0001EA)	/* IO base address higher address between IO space and RAM */
#define VR41XX_IOALREG		__preg16(KSEG1 + 0x0F0001EC)	/* IO address lower address between IO space and RAM */
#define VR41XX_IOAHREG		__preg16(KSEG1 + 0x0F0001EE)	/* IO address higher address between IO space and RAM */

/* DMA Control Unit (DCU) */
#define VR41XX_DMARSTREG	__preg16(KSEG1 + 0x0F000040)	/* DMA Reset Register */
#define VR41XX_DMAIDLEREG	__preg16(KSEG1 + 0x0F000042)	/* DMA Idle Register */
#define VR41XX_DMASENREG	__preg16(KSEG1 + 0x0F000044)	/* DMA Sequencer Enable Register */
#define VR41XX_DMAMSKREG	__preg16(KSEG1 + 0x0F000046)	/* DMA Mask Register */
#define VR41XX_DMAREQREG	__preg16(KSEG1 + 0x0F000048)	/* DMA Request Register */
#define VR41XX_TDREG		__preg16(KSEG1 + 0x0F00004A)	/* Transfer Direction Register */
#define VR41XX_DMAABITREG	__preg16(KSEG1 + 0x0F00004C)	/* DMA arbitration protocol selection register */
#define VR41XX_CONTROLREG	__preg16(KSEG1 + 0x0F00004E)	/* DMA control register */
#define VR41XX_BASSCNTLREG	__preg16(KSEG1 + 0x0F000050)	/* DMA transfer byte size register low */
#define VR41XX_BASSCNTHREG	__preg16(KSEG1 + 0x0F000052)	/* DMA transfer byte size register high */
#define VR41XX_CURRENTCNTLREG	__preg16(KSEG1 + 0x0F000054)	/* DMA remaining transfer byte size register low */
#define VR41XX_CURRENTCNTHREG	__preg16(KSEG1 + 0x0F000056)	/* DMA remaining transfer byte size register high */
#define VR41XX_TCINTR		__preg16(KSEG1 + 0x0F000058)	/* Terminal count interrupt request */

/* Clock Mask Unit (CMU) */
#define VR41XX_CMUCLKMSK	__preg16(KSEG1 + 0x0F000060)	/* CMU Clock Mask Register */
#define VR41XX_CMUCLKMSK_MSKPIUPCLK	0x0001
#define VR41XX_CMUCLKMSK_MSKSIU		0x0102

/* Interrupt Control Unit (ICU) */
#define VR41XX_SYSINT1REG	__preg16(KSEG1 + 0x0F000080)	/* Level 1 System interrupt register 1 */
#define VR41XX_GIUINTLREG	__preg16(KSEG1 + 0x0F000088)	/* Level 2 GIU interrupt register Low */
#define VR41XX_DSIUINTREG	__preg16(KSEG1 + 0x0F00008A)	/* Level 2 DSIU interrupt register */
#define VR41XX_MSYSINT1REG	__preg16(KSEG1 + 0x0F00008C)	/* Level 1 mask system interrupt register 1 */
#define VR41XX_MGIUINTLREG	__preg16(KSEG1 + 0x0F000094)	/* Level 2 mask GIU interrupt register Low */
#define VR41XX_MDSIUINTREG	__preg16(KSEG1 + 0x0F000096)	/* Level 2 mask DSIU interrupt register */
#define VR41XX_NMIREG		__preg16(KSEG1 + 0x0F000098)	/* NMI register */
#define VR41XX_SOFTINTREG	__preg16(KSEG1 + 0x0F00009A)	/* Software interrupt register */
#define VR41XX_SYSINT2REG	__preg16(KSEG1 + 0x0F0000A0)	/* Level 1 System interrupt register 2 */
#define VR41XX_GIUINTHREG	__preg16(KSEG1 + 0x0F0000A2)	/* Level 2 GIU interrupt register High */
#define VR41XX_FIRINTREG	__preg16(KSEG1 + 0x0F0000A4)	/* Level 2 FIR interrupt register */
#define VR41XX_MSYSINT2REG	__preg16(KSEG1 + 0x0F0000A6)	/* Level 1 mask system interrupt register 2 */
#define VR41XX_MGIUINTHREG	__preg16(KSEG1 + 0x0F0000A8)	/* Level 2 mask GIU interrupt register High */
#define VR41XX_MFIRINTREG	__preg16(KSEG1 + 0x0F0000AA)	/* Level 2 mask FIR interrupt register */
#define VR41XX_PCIINTREG	__preg16(KSEG1 + 0x0F0000AC)	/* Level 2 PCI interrupt register */
#define VR41XX_SCUINTREG	__preg16(KSEG1 + 0x0F0000AE)	/* Level 2 SCU interrupt register */
#define VR41XX_CSIINTREG	__preg16(KSEG1 + 0x0F0000B0)	/* Level 2 CSI interrupt register */
#define VR41XX_MPCIINTREG	__preg16(KSEG1 + 0x0F0000B2)	/* Level 2 mask PCI interrupt register */
#define VR41XX_MSCUINTREG	__preg16(KSEG1 + 0x0F0000B4)	/* Level 2 mask SCU interrupt register */
#define VR41XX_MCSIINTREG	__preg16(KSEG1 + 0x0F0000B6)	/* Level 2 mask CSI interrupt register */

/* Power Management Unit (PMU) */
#define VR41XX_PMUINTREG	__preg16(KSEG1 + 0x0F0000C0)	/* PMU Status Register */
#define VR41XX_PMUINT_POWERSW  0x1	/* Power switch */
#define VR41XX_PMUINT_BATT     0x2	/* Low batt during normal operation */
#define VR41XX_PMUINT_DEADMAN  0x4	/* Deadman's switch */
#define VR41XX_PMUINT_RESET    0x8	/* Reset switch */
#define VR41XX_PMUINT_RTCRESET 0x10	/* RTC Reset */
#define VR41XX_PMUINT_TIMEOUT  0x20	/* HAL Timer Reset */
#define VR41XX_PMUINT_BATTLOW  0x100	/* Battery low */
#define VR41XX_PMUINT_RTC      0x200	/* RTC Alarm */
#define VR41XX_PMUINT_DCD      0x400	/* DCD# */
#define VR41XX_PMUINT_GPIO0    0x1000	/* GPIO0 */
#define VR41XX_PMUINT_GPIO1    0x2000	/* GPIO1 */
#define VR41XX_PMUINT_GPIO2    0x4000	/* GPIO2 */
#define VR41XX_PMUINT_GPIO3    0x8000	/* GPIO3 */

#define VR41XX_PMUCNTREG	__preg16(KSEG1 + 0x0F0000C2)	/* PMU Control Register */
#define VR41XX_PMUINT2REG	__preg16(KSEG1 + 0x0F0000C4)	/* PMU Interrupt/Status 2 Register */
#define VR41XX_PMUCNT2REG	__preg16(KSEG1 + 0x0F0000C6)	/* PMU Control 2 Resister */
#define VR41XX_PMUWAITREG	__preg16(KSEG1 + 0x0F0000C8)	/* PMU Wait Counter Register */
#define VR41XX_PMUTCLKDIVREG	__preg16(KSEG1 + 0x0F0000CC)	/* PMU Tclk Div mode register */
#define VR41XX_PMUINTRCLKDIVREG	__preg16(KSEG1 + 0x0F0000CE)	/* PMU INT clock Div mode register */
#define VR41XX_PMUCLKRUNREG	__preg16(KSEG1 + 0x0F0000D6)	/* PMU CLKRUN control register */


/* Real Time Clock Unit (RTC) */
#define VR41XX_ETIMELREG	__preg16(KSEG1 + 0x0F000100)	/* Elapsed Time L Register */
#define VR41XX_ETIMEMREG	__preg16(KSEG1 + 0x0F000102)	/* Elapsed Time M Register */
#define VR41XX_ETIMEHREG	__preg16(KSEG1 + 0x0F000104)	/* Elapsed Time H Register */
#define VR41XX_ECMPLREG		__preg16(KSEG1 + 0x0F000108)	/* Elapsed Compare L Register */
#define VR41XX_ECMPMREG		__preg16(KSEG1 + 0x0F00010A)	/* Elapsed Compare M Register */
#define VR41XX_ECMPHREG		__preg16(KSEG1 + 0x0F00010C)	/* Elapsed Compare H Register */
#define VR41XX_RTCL1LREG	__preg16(KSEG1 + 0x0F000110)	/* RTC Long 1 L Register */
#define VR41XX_RTCL1HREG	__preg16(KSEG1 + 0x0F000112)	/* RTC Long 1 H Register */
#define VR41XX_RTCL1CNTLREG	__preg16(KSEG1 + 0x0F000114)	/* RTC Long 1 Count L Register */
#define VR41XX_RTCL1CNTHREG	__preg16(KSEG1 + 0x0F000116)	/* RTC Long 1 Count H Register */
#define VR41XX_RTCL2LREG	__preg16(KSEG1 + 0x0F000118)	/* RTC Long 2 L Register */
#define VR41XX_RTCL2HREG	__preg16(KSEG1 + 0x0F00011A)	/* RTC Long 2 H Register */
#define VR41XX_RTCL2CNTLREG	__preg16(KSEG1 + 0x0F00011C)	/* RTC Long 2 Count L Register */
#define VR41XX_RTCL2CNTHREG	__preg16(KSEG1 + 0x0F00011E)	/* RTC Long 2 Count H Register */
#define VR41XX_TCLKLREG		__preg16(KSEG1 + 0x0F000120)	/* TCLK L Register */
#define VR41XX_TCLKHREG		__preg16(KSEG1 + 0x0F000122)	/* TCLK H Register */
#define VR41XX_TCLKCNTLREG	__preg16(KSEG1 + 0x0F000124)	/* TCLK Count L Register */
#define VR41XX_TCLKCNTHREG	__preg16(KSEG1 + 0x0F000126)	/* TCLK Count H Register */
#define VR41XX_RTCINTREG	__preg16(KSEG1 + 0x0F00013E)	/* RTC Interrupt Register */

/* Deadman's Switch Unit (DSU) */
#define VR41XX_DSUCNTREG	__preg16(KSEG1 + 0x0F0000E0)	/* DSU Control Register */
#define VR41XX_DSUSETREG	__preg16(KSEG1 + 0x0F0000E2)	/* DSU Dead Time Set Register */
#define VR41XX_DSUCLRREG	__preg16(KSEG1 + 0x0F0000E4)	/* DSU Clear Register */
#define VR41XX_DSUTIMREG	__preg16(KSEG1 + 0x0F0000E6)	/* DSU Elapsed Time Register */

/* General Purpose I/O Unit (GIU) */
#define VR41XX_GIUIOSELL	__preg16(KSEG1 + 0x0F000140)	/* GPIO Input/Output Select Register L */
#define VR41XX_GIUIOSELH	__preg16(KSEG1 + 0x0F000142)	/* GPIO Input/Output Select Register H */
#define VR41XX_GIUPIODL		__preg16(KSEG1 + 0x0F000144)	/* GPIO Port Input/Output Data Register L */
#define VR41XX_GIUPIODL_GPIO15  0x8000
#define VR41XX_GIUPIODL_GPIO14  0x4000
#define VR41XX_GIUPIODL_GPIO13  0x2000
#define VR41XX_GIUPIODL_GPIO12  0x1000
#define VR41XX_GIUPIODL_GPIO11  0x0800
#define VR41XX_GIUPIODL_GPIO10  0x0400
#define VR41XX_GIUPIODL_GPIO9  0x0200
#define VR41XX_GIUPIODL_GPIO8  0x0100
#define VR41XX_GIUPIODL_GPIO7  0x0080
#define VR41XX_GIUPIODL_GPIO6  0x0040
#define VR41XX_GIUPIODL_GPIO5  0x0020
#define VR41XX_GIUPIODL_GPIO4  0x0010
#define VR41XX_GIUPIODL_GPIO3  0x0008
#define VR41XX_GIUPIODL_GPIO2  0x0004
#define VR41XX_GIUPIODL_GPIO1  0x0002
#define VR41XX_GIUPIODL_GPIO0  0x0001
#define VR41XX_GIUPIODH		__preg16(KSEG1 + 0x0F000146)	/* GPIO Port Input/Output Data Register H */
#define VR41XX_GIUPIODH_GPIO31  0x8000
#define VR41XX_GIUPIODH_GPIO30  0x4000
#define VR41XX_GIUPIODH_GPIO29  0x2000
#define VR41XX_GIUPIODH_GPIO28  0x1000
#define VR41XX_GIUPIODH_GPIO27  0x0800
#define VR41XX_GIUPIODH_GPIO26  0x0400
#define VR41XX_GIUPIODH_GPIO25  0x0200
#define VR41XX_GIUPIODH_GPIO24  0x0100
#define VR41XX_GIUPIODH_GPIO23  0x0080
#define VR41XX_GIUPIODH_GPIO22  0x0040
#define VR41XX_GIUPIODH_GPIO21  0x0020
#define VR41XX_GIUPIODH_GPIO20  0x0010
#define VR41XX_GIUPIODH_GPIO19  0x0008
#define VR41XX_GIUPIODH_GPIO18  0x0004
#define VR41XX_GIUPIODH_GPIO17  0x0002
#define VR41XX_GIUPIODH_GPIO16  0x0001
#define VR41XX_GIUINTSTATL	__preg16(KSEG1 + 0x0F000148)	/* GPIO Interrupt Status Register L */
#define VR41XX_GIUINTSTATH	__preg16(KSEG1 + 0x0F00014A)	/* GPIO Interrupt Status Register H */
#define VR41XX_GIUINTENL	__preg16(KSEG1 + 0x0F00014C)	/* GPIO Interrupt Enable Register L */
#define VR41XX_GIUINTENH	__preg16(KSEG1 + 0x0F00014E)	/* GPIO Interrupt Enable Register H */
#define VR41XX_GIUINTTYPL	__preg16(KSEG1 + 0x0F000150)	/* GPIO Interrupt Type (Edge or Level) Select Register */
#define VR41XX_GIUINTTYPH	__preg16(KSEG1 + 0x0F000152)	/* GPIO Interrupt Type (Edge or Level) Select Register */
#define VR41XX_GIUINTALSELL	__preg16(KSEG1 + 0x0F000154)	/* GPIO Interrupt Active Level Select Register L */
#define VR41XX_GIUINTALSELH	__preg16(KSEG1 + 0x0F000156)	/* GPIO Interrupt Active Level Select Register H */
#define VR41XX_GIUINTHTSELL	__preg16(KSEG1 + 0x0F000158)	/* GPIO Interrupt Hold/Through Select Register L */
#define VR41XX_GIUINTHTSELH	__preg16(KSEG1 + 0x0F00015A)	/* GPIO Interrupt Hold/Through Select Register H */

#define VR41XX_GIUPODATEN	__preg16(KSEG1 + 0x0F00015C)	/* GPIO Port Output Data Enable Register */
#define VR41XX_GIUPODATL	__preg16(KSEG1 + 0x0F00015E)	/* GPIO Port Output Data Register L */
#define VR41XX_GIUPODATL_GPIO47  0x8000
#define VR41XX_GIUPODATL_GPIO46  0x4000
#define VR41XX_GIUPODATL_GPIO45  0x2000
#define VR41XX_GIUPODATL_GPIO44  0x1000
#define VR41XX_GIUPODATL_GPIO43  0x0800
#define VR41XX_GIUPODATL_GPIO42  0x0400
#define VR41XX_GIUPODATL_GPIO41  0x0200
#define VR41XX_GIUPODATL_GPIO40  0x0100
#define VR41XX_GIUPODATL_GPIO39  0x0080
#define VR41XX_GIUPODATL_GPIO38  0x0040
#define VR41XX_GIUPODATL_GPIO37  0x0020
#define VR41XX_GIUPODATL_GPIO36  0x0010
#define VR41XX_GIUPODATL_GPIO35  0x0008
#define VR41XX_GIUPODATL_GPIO34  0x0004
#define VR41XX_GIUPODATL_GPIO33  0x0002
#define VR41XX_GIUPODATL_GPIO32  0x0001
#define VR41XX_GIUPODATL_PODAT15  0x8000
#define VR41XX_GIUPODATL_PODAT14  0x4000
#define VR41XX_GIUPODATL_PODAT13  0x2000
#define VR41XX_GIUPODATL_PODAT12  0x1000
#define VR41XX_GIUPODATL_PODAT11  0x0800
#define VR41XX_GIUPODATL_PODAT10  0x0400
#define VR41XX_GIUPODATL_PODAT9  0x0200
#define VR41XX_GIUPODATL_PODAT8  0x0100
#define VR41XX_GIUPODATL_PODAT7  0x0080
#define VR41XX_GIUPODATL_PODAT6  0x0040
#define VR41XX_GIUPODATL_PODAT5  0x0020
#define VR41XX_GIUPODATL_PODAT4  0x0010
#define VR41XX_GIUPODATL_PODAT3  0x0008
#define VR41XX_GIUPODATL_PODAT2  0x0004
#define VR41XX_GIUPODATL_PODAT1  0x0002
#define VR41XX_GIUPODATL_PODAT0  0x0001
#define VR41XX_SECIRQMASKL	VR41XX_GIUINTENL
#define VR41XX_SECIRQMASKH	VR41XX_GIUINTENH

/* SDRAM Control Unit (SDRAMU) */
#define VR41XX_SDRAMMODEREG	__preg16(KSEG1 + 0x0F000400)	/* SDRAM mode register */
#define VR41XX_SDRAMCNTREG	__preg16(KSEG1 + 0x0F000402)	/* SDRAM control register */
#define VR41XX_BCURFCNTREG	__preg16(KSEG1 + 0x0F000404)	/* BCU refresh control register */
#define VR41XX_BCURFCOUNTREG	__preg16(KSEG1 + 0x0F000406)	/* BCU refresh cycle count register */
#define VR41XX_RAMSIZEREG	__preg16(KSEG1 + 0x0F000408)	/* DRAM size register */

/* Debug Serial Interface Unit (DSIU) */
#define VR41XX_PORTREG		__preg16(KSEG1 + 0x0F000820)	/* Port Change Register */
#define VR41XX_MODEMREG		__preg16(KSEG1 + 0x0F000822)	/* Modem Control Register */
#define VR41XX_ASIM00REG	__preg16(KSEG1 + 0x0F000824)	/* Asynchronous Mode 0 Register */
#define VR41XX_ASIM01REG	__preg16(KSEG1 + 0x0F000826)	/* Asynchronous Mode 1 Register */
#define VR41XX_RXB0RREG		__preg16(KSEG1 + 0x0F000828)	/* Receive Buffer Register (Extended) */
#define VR41XX_RXB0LREG		__preg16(KSEG1 + 0x0F00082A)	/* Receive Buffer Register */
#define VR41XX_TXS0RREG		__preg16(KSEG1 + 0x0F00082C)	/* Transmit Data Register (Extended) */
#define VR41XX_TXS0LREG		__preg16(KSEG1 + 0x0F00082E)	/* Transmit Data Register */
#define VR41XX_ASIS0REG		__preg16(KSEG1 + 0x0F000830)	/* Status Register */
#define VR41XX_INTR0REG		__preg16(KSEG1 + 0x0F000832)	/* Debug SIU Interrupt Register */
#define VR41XX_BPRM0REG		__preg16(KSEG1 + 0x0F000836)	/* Baud rate Generator Prescaler Mode Register */
#define VR41XX_DSIURESETREG	__preg16(KSEG1 + 0x0F000838)	/* Debug SIU Reset Register */

/* LED Control Unit (LED) */
#define VR41XX_LEDHTSREG	__preg16(KSEG1 + 0x0F000180)	/* LED H Time Set register */
#define VR41XX_LEDLTSREG	__preg16(KSEG1 + 0x0F000182)	/* LED L Time Set register */
#define VR41XX_LEDCNTREG	__preg16(KSEG1 + 0x0F000188)	/* LED Control register */
#define VR41XX_LEDASTCREG	__preg16(KSEG1 + 0x0F00018A)	/* LED Auto Stop Time Count register */
#define VR41XX_LEDINTREG	__preg16(KSEG1 + 0x0F00018C)	/* LED Interrupt register */

/* Serial Interface Unit (SIU / SIU1 and SIU2) */
#define VR41XX_SIURB		__preg8(KSEG1 + 0x0F000800)	/* Receiver Buffer Register (Read) DLAB = 0 */
#define VR41XX_SIUTH		__preg8(KSEG1 + 0x0F000800)	/* Transmitter Holding Register (Write) DLAB = 0 */
#define VR41XX_SIUDLL		__preg8(KSEG1 + 0x0F000800)	/* Divisor Latch (Least Significant Byte) DLAB = 1 */
#define VR41XX_SIUIE		__preg8(KSEG1 + 0x0F000801)	/* Interrupt Enable DLAB = 0 */
#define VR41XX_SIUDLM		__preg8(KSEG1 + 0x0F000801)	/* Divisor Latch (Most Significant Byte) DLAB = 1 */
#define VR41XX_SIUIID		__preg8(KSEG1 + 0x0F000802)	/* Interrupt Identification Register (Read) */
#define VR41XX_SIUFC		__preg8(KSEG1 + 0x0F000802)	/* FIFO Control Register (Write) */
#define VR41XX_SIULC		__preg8(KSEG1 + 0x0F000803)	/* Line Control Register */
#define VR41XX_SIUMC		__preg8(KSEG1 + 0x0F000804)	/* MODEM Control Register */
#define VR41XX_SIULS		__preg8(KSEG1 + 0x0F000805)	/* Line Status Register */
#define VR41XX_SIUMS		__preg8(KSEG1 + 0x0F000806)	/* MODEM Status Register */
#define VR41XX_SIUSC		__preg8(KSEG1 + 0x0F000807)	/* Scratch Register */
#define VR41XX_SIUIRSEL		__preg8(KSEG1 + 0x0F000808)	/* SIU/FIR IrDA Selector */
#define VR41XX_SIURESET		__preg8(KSEG1 + 0x0F000809)	/* SIU Reset Register */
#define VR41XX_SIUCSEL		__preg8(KSEG1 + 0x0F00080A)	/* SIU Echo-Back Control Register */

/* Fast IrDA Interface Unit (FIR) */
#define VR41XX_FRSTR		__preg16(KSEG1 + 0x0F000840)	/* FIR Reset register */
#define VR41XX_DPINTR		__preg16(KSEG1 + 0x0F000842)	/* DMA Page Interrupt register */
#define VR41XX_DPCNTR		__preg16(KSEG1 + 0x0F000844)	/* DMA Control register */
#define VR41XX_TDR		__preg16(KSEG1 + 0x0F000850)	/* Transmit Data register */
#define VR41XX_RDR		__preg16(KSEG1 + 0x0F000852)	/* Receive Data register */
#define VR41XX_IMR		__preg16(KSEG1 + 0x0F000854)	/* Interrupt Mask register */
#define VR41XX_FSR		__preg16(KSEG1 + 0x0F000856)	/* FIFO Setup register */
#define VR41XX_IRSR1		__preg16(KSEG1 + 0x0F000858)	/* Infrared Setup register 1 */
#define VR41XX_CRCSR		__preg16(KSEG1 + 0x0F00085C)	/* CRC Setup register */
#define VR41XX_FIRCR		__preg16(KSEG1 + 0x0F00085E)	/* FIR Control register */
#define VR41XX_MIRCR		__preg16(KSEG1 + 0x0F000860)	/* MIR Control register */
#define VR41XX_DMACR		__preg16(KSEG1 + 0x0F000862)	/* DMA Control register */
#define VR41XX_DMAER		__preg16(KSEG1 + 0x0F000864)	/* DMA Enable register */
#define VR41XX_TXIR		__preg16(KSEG1 + 0x0F000866)	/* Transmit Indication register */
#define VR41XX_RXIR		__preg16(KSEG1 + 0x0F000868)	/* Receive Indication register */
#define VR41XX_IFR		__preg16(KSEG1 + 0x0F00086A)	/* Interrupt Flag register */
#define VR41XX_RXSTS		__preg16(KSEG1 + 0x0F00086C)	/* Receive Status */
#define VR41XX_TXFL		__preg16(KSEG1 + 0x0F00086E)	/* Transmit Frame Length */
#define VR41XX_MRXF		__preg16(KSEG1 + 0x0F000870)	/* Maximum Receive Frame Length */
#define VR41XX_RXFL		__preg16(KSEG1 + 0x0F000874)	/* Receive Frame Length */

/* PCI Interface Unit (PCIU) */
#define VR41XX_PCIMMAW1REG	__preg32(KSEG1 + 0x0F000C00)
#define VR41XX_PCIMMAW2REG	__preg32(KSEG1 + 0x0F000C04)
#define VR41XX_PCITAW1REG	__preg32(KSEG1 + 0x0F000C08)
#define VR41XX_PCITAW2REG	__preg32(KSEG1 + 0x0F000C0C)
#define VR41XX_PCIMIOAWREG	__preg32(KSEG1 + 0x0F000C10)
#define VR41XX_PCICONFDREG	__preg32(KSEG1 + 0x0F000C14)
#define VR41XX_PCICONFAREG	__preg32(KSEG1 + 0x0F000C18)
#define VR41XX_PCIMAILREG	__preg32(KSEG1 + 0x0F000C1C)
#define VR41XX_BUSERRADREG	__preg32(KSEG1 + 0x0F000C24)
#define VR41XX_INTCNTSTAREG	__preg32(KSEG1 + 0x0F000C28)
#define VR41XX_PCIEXACCREG	__preg32(KSEG1 + 0x0F000C2C)
#define VR41XX_PCIRECONTREG	__preg32(KSEG1 + 0x0F000C30)
#define VR41XX_PCIENREG		__preg32(KSEG1 + 0x0F000C34)
#define VR41XX_PCICLKSELREG	__preg32(KSEG1 + 0x0F000C38)
#define VR41XX_PCITRDYVREG	__preg32(KSEG1 + 0x0F000C3C)
#define VR41XX_PCICLKRUNREG	__preg16(KSEG1 + 0x0F000C60)

#define VR41XX_PCIVENDORIDREG	__preg16(KSEG1 + 0x0F000D00)
#define VR41XX_PCIDEVICEIDREG	__preg16(KSEG1 + 0x0F000D02)
#define VR41XX_PCICOMMABDREG	__preg32(KSEG1 + 0x0F000D04)
#define VR41XX_PCIREVREG	__preg32(KSEG1 + 0x0F000D08)
#define VR41XX_PCICACHELSREG	__preg32(KSEG1 + 0x0F000D0C)
#define VR41XX_PCIMAILBAREG	__preg32(KSEG1 + 0x0F000D10)
#define VR41XX_PCIMBA1REG	__preg32(KSEG1 + 0x0F000D14)
#define VR41XX_PCIMBA2REG	__preg32(KSEG1 + 0x0F000D18)
#define VR41XX_PCIINTLINEREG	__preg32(KSEG1 + 0x0F000D3C)
#define VR41XX_PCIRETVALREG	__preg32(KSEG1 + 0x0F000D40)

#ifdef CONFIG_NEC_HARRIER

/* physical address spaces */
#define VR41XX_LCD             0x0a000000
#define VR41XX_INTERNAL_IO_2   0x0b000000
#define VR41XX_INTERNAL_IO_1   0x0c000000
#define VR41XX_ISA_MEM         0x10000000
#define VR41XX_ISA_IO          0x16000000
#define VR41XX_ROM             0x18000000

#define NEC_HARRIER_SIO1	__preg16(KSEG1 + 0x0A000000)
#define NEC_HARRIER_SIO2	__preg16(KSEG1 + 0x0A000010)

#define NEC_HARRIER_FLSHCNTREG	__preg8(KSEG1 + 0x0DFFFFA0)
#define NEC_HARRIER_FLSHBANKREG	__preg8(KSEG1 + 0x0DFFFFA4)
#define NEC_HARRIER_SWSETREG	__preg8(KSEG1 + 0x0DFFFFA8)
#define NEC_HARRIER_LED1REG	__preg8(KSEG1 + 0x0DFFFFC0)
#define NEC_HARRIER_LED2REG	__preg8(KSEG1 + 0x0DFFFFC4)
#define NEC_HARRIER_SDBINTREG	__preg8(KSEG1 + 0x0DFFFFD0)
#define NEC_HARRIER_SDBINTMASK	__preg8(KSEG1 + 0x0DFFFFD4)
#define NEC_HARRIER_RSTREG	__preg8(KSEG1 + 0x0DFFFFD8)
#define NEC_HARRIER_PCIINTREG	__preg8(KSEG1 + 0x0DFFFFDC)
#define NEC_HARRIER_PCIINTMASK	__preg8(KSEG1 + 0x0DFFFFE0)
#define NEC_HARRIER_PCICLKREG	__preg32(KSEG1 + 0x0DFFFFE4)

#define NEC_HARRIER_DISPLAY_LEDS(x) \
    do { \
	*NEC_HARRIER_LED1REG = (unsigned char) ((x)       & 0xff); \
        *NEC_HARRIER_LED2REG = (unsigned char)(((x) >> 8) & 0xff); \
    } while (0)
 

/* This is the base address for IO port decoding to which the 16 bit IO port address */
/* is added.  Defining it to 0 will usually cause a kernel oops any time port IO is */
/* attempted, which can be handy for turning up parts of the kernel that make */
/* incorrect architecture assumptions (by assuming that everything acts like a PC), */
/* but we need it correctly defined to use the PCMCIA/CF controller: */
#define VR41XX_PORT_BASE	(KSEG1 + VR41XX_ISA_IO)
#define VR41XX_ISAMEM_BASE	(KSEG1 + VR41XX_ISA_MEM)

#else  /* CONFIG_NEC_HARRIER */

#define NEC_HARRIER_DISPLAY_LEDS(x) {}

#endif /* CONFIG_NEC_HARRIER */

#endif /* __ASM_MIPS_VR4122_H */
