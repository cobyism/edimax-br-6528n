/* $Id: vr4121.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999 by Michael Klar
 */
#ifndef __ASM_MIPS_VR4121_H 
#define __ASM_MIPS_VR4121_H 

#include <asm/addrspace.h>

// CPU interrupts
#define VR41XX_IRQ_SW1       0  // IP0 - Software interrupt
#define VR41XX_IRQ_SW2       1  // IP1 - Software interrupt
#define VR41XX_IRQ_INT0      2  // IP2 - All but battery, high speed modem, and real time clock
#define VR41XX_IRQ_INT1      3  // IP3 - RTC Long1 (system timer)
#define VR41XX_IRQ_INT2      4  // IP4 - RTC Long2
#define VR41XX_IRQ_INT3      5  // IP5 - High Speed Modem
#define VR41XX_IRQ_INT4      6  // IP6 - Unused
#define VR41XX_IRQ_TIMER     7  // IP7 - Timer interrupt from CPO_COMPARE (Note: RTC Long1 is the system timer.)

// Cascaded from VR41XX_IRQ_INT0 (ICU mapped interrupts)
#define VR41XX_IRQ_BATTERY   8
#define VR41XX_IRQ_POWER     9
#define VR41XX_IRQ_RTCL1     10  // Use VR41XX_IRQ_INT1 instead.
#define VR41XX_IRQ_ETIMER    11
#define VR41XX_IRQ_RFU12     12
#define VR41XX_IRQ_PIU       13
#define VR41XX_IRQ_AIU       14
#define VR41XX_IRQ_KIU       15
#define VR41XX_IRQ_GIU       16  // This is a cascade to IRQs 40-71. Do not use.
#define VR41XX_IRQ_SIU       17
#define VR41XX_IRQ_WRBERR    18
#define VR41XX_IRQ_SOFT      19
#define VR41XX_IRQ_RFU20     20
#define VR41XX_IRQ_DOZEPIU   21
#define VR41XX_IRQ_RFU22     22
#define VR41XX_IRQ_RFU23     23
#define VR41XX_IRQ_RTCL2     24  // Use VR41XX_IRQ_INT2 instead.
#define VR41XX_IRQ_LED       25
#define VR41XX_IRQ_HSP       26  // Use VR41XX_IRQ_INT3 instead.
#define VR41XX_IRQ_TCLK      27
#define VR41XX_IRQ_FIR       28
#define VR41XX_IRQ_DSIU      29
#define VR41XX_IRQ_RFU30     30
#define VR41XX_IRQ_RFU31     31
#define VR41XX_IRQ_RFU32     32
#define VR41XX_IRQ_RFU33     33
#define VR41XX_IRQ_RFU34     34
#define VR41XX_IRQ_RFU35     35
#define VR41XX_IRQ_RFU36     36
#define VR41XX_IRQ_RFU37     37
#define VR41XX_IRQ_RFU38     38
#define VR41XX_IRQ_RFU39     39

// Cascaded from VR41XX_IRQ_GIU
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

// Alternative to above GPIO IRQ defines
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

// Embedded CPU peripheral registers

// Bus Control Unit (BCU)
#define VR41XX_BCUCNTREG1	__preg16(KSEG1 + 0x0B000000)	/* BCU Control Register 1 (R/W) */
#define VR41XX_BCUCNTREG2	__preg16(KSEG1 + 0x0B000002)	/* BCU Control Register 2 (R/W) */
#define VR41XX_ROMSIZEREG	__preg16(KSEG1 + 0x0B000004)	/* ROM Size Register (R/W) */
#define VR41XX_RAMSIZEREG	__preg16(KSEG1 + 0x0B000006)	/* DRAM Size Register (R/W) */
#define VR41XX_BCUSPEEDREG	__preg16(KSEG1 + 0x0B00000A)	/* BCU Access Cycle Change Register (R/W) */
#define VR41XX_BCUERRSTREG	__preg16(KSEG1 + 0x0B00000C)	/* BCU BUS ERROR Status Register (R/W) */
#define VR41XX_BCURFCNTREG	__preg16(KSEG1 + 0x0B00000E)	/* BCU Refresh Control Register (R/W) */
#define VR41XX_REVIDREG		__preg16(KSEG1 + 0x0B000010)	/* Revision ID Register (R) */
#define VR41XX_BCURFCOUNTREG	__preg16(KSEG1 + 0x0B000012)	/* BCU Refresh Count Register (R/W) */
#define VR41XX_CLKSPEEDREG	__preg16(KSEG1 + 0x0B000014)	/* Clock Speed Register (R) */
#define VR41XX_BCUCNTREG3	__preg16(KSEG1 + 0x0B000016)	/* BCU Control Register 3 (R/W) */
#define VR41XX_SDRAMMODEREG	__preg16(KSEG1 + 0x0B00001A)	/* SDRAM Mode Register */
#define VR41XX_SROMMODEREG	__preg16(KSEG1 + 0x0B00001C)	/* SROM Mode Register */
#define VR41XX_SDRAMCNTREG	__preg16(KSEG1 + 0x0B00001E)	/* SDRAM Control Register */
#define VR41XX_BCUTOUTCNTREG	__preg16(KSEG1 + 0x0B000300)	/* BCU Timeout Control Register */
#define VR41XX_BCUTOUTCOUNTREG	__preg16(KSEG1 + 0x0B000302)	/* BCU Timeout Count Register */

// DMA Address Unit (DMAAU)
#define VR41XX_AIUIBALREG	__preg16(KSEG1 + 0x0B000020)	/* AIU IN DMA Base Address Register Low (R/W) */
#define VR41XX_AIUIBAHREG	__preg16(KSEG1 + 0x0B000022)	/* AIU IN DMA Base Address Register High (R/W) */
#define VR41XX_AIUIALREG	__preg16(KSEG1 + 0x0B000024)	/* AIU IN DMA Address Register Low (R/W) */
#define VR41XX_AIUIAHREG	__preg16(KSEG1 + 0x0B000026)	/* AIU IN DMA Address Register High (R/W) */
#define VR41XX_AIUOBALREG	__preg16(KSEG1 + 0x0B000028)	/* AIU OUT DMA Base Address Register Low (R/W) */
#define VR41XX_AIUOBAHREG	__preg16(KSEG1 + 0x0B00002A)	/* AIU OUT DMA Base Address Register High (R/W) */
#define VR41XX_AIUOALREG	__preg16(KSEG1 + 0x0B00002C)	/* AIU OUT DMA Address Register Low (R/W) */
#define VR41XX_AIUOAHREG	__preg16(KSEG1 + 0x0B00002E)	/* AIU OUT DMA Address Register High (R/W) */
#define VR41XX_FIRBALREG	__preg16(KSEG1 + 0x0B000030)	/* FIR DMA Base Address Register Low (R/W) */
#define VR41XX_FIRBAHREG	__preg16(KSEG1 + 0x0B000032)	/* FIR DMA Base Address Register High (R/W) */
#define VR41XX_FIRALREG		__preg16(KSEG1 + 0x0B000034)	/* FIR DMA Address Register Low (R/W) */
#define VR41XX_FIRAHREG		__preg16(KSEG1 + 0x0B000036)	/* FIR DMA Address Register High (R/W) */

// DMA Control Unit (DCU)
#define VR41XX_DMARSTREG	__preg16(KSEG1 + 0x0B000040)	/* DMA Reset Register (R/W) */
#define VR41XX_DMAIDLEREG	__preg16(KSEG1 + 0x0B000042)	/* DMA Idle Register (R) */
#define VR41XX_DMASENREG	__preg16(KSEG1 + 0x0B000044)	/* DMA Sequencer Enable Register (R/W) */
#define VR41XX_DMAMSKREG	__preg16(KSEG1 + 0x0B000046)	/* DMA Mask Register (R/W) */
#define VR41XX_DMAREQREG	__preg16(KSEG1 + 0x0B000048)	/* DMA Request Register (R) */
#define VR41XX_TDREG		__preg16(KSEG1 + 0x0B00004A)	/* Transfer Direction Register (R/W) */

// Clock Mask Unit (CMU)
#define VR41XX_CMUCLKMSK	__preg16(KSEG1 + 0x0B000060)	/* CMU Clock Mask Register (R/W) */
#define VR41XX_CMUCLKMSK_MSKPIUPCLK	0x0001
#define VR41XX_CMUCLKMSK_MSKSIU		0x0102

// Interrupt Control Unit (ICU)
#define VR41XX_SYSINT1REG	__preg16(KSEG1 + 0x0B000080)	/* Level 1 System interrupt register 1 (R) */
#define VR41XX_PIUINTREGro	__preg16(KSEG1 + 0x0B000082)	/* Level 2 PIU interrupt register (R) */
#define VR41XX_AIUINTREG	__preg16(KSEG1 + 0x0B000084)	/* Level 2 AIU interrupt register (R) */
#define VR41XX_KIUINTREG	__preg16(KSEG1 + 0x0B000086)	/* Level 2 KIU interrupt register (R) */
#define VR41XX_GIUINTLREG	__preg16(KSEG1 + 0x0B000088)	/* Level 2 GIU interrupt register Low (R) */
#define VR41XX_DSIUINTREG	__preg16(KSEG1 + 0x0B00008A)	/* Level 2 DSIU interrupt register (R) */
#define VR41XX_MSYSINT1REG	__preg16(KSEG1 + 0x0B00008C)	/* Level 1 mask system interrupt register 1 (R/W) */
#define VR41XX_MPIUINTREG	__preg16(KSEG1 + 0x0B00008E)	/* Level 2 mask PIU interrupt register (R/W) */
#define VR41XX_MAIUINTREG	__preg16(KSEG1 + 0x0B000090)	/* Level 2 mask AIU interrupt register (R/W) */
#define VR41XX_MKIUINTREG	__preg16(KSEG1 + 0x0B000092)	/* Level 2 mask KIU interrupt register (R/W) */
#define VR41XX_MGIUINTLREG	__preg16(KSEG1 + 0x0B000094)	/* Level 2 mask GIU interrupt register Low (R/W) */
#define VR41XX_MDSIUINTREG	__preg16(KSEG1 + 0x0B000096)	/* Level 2 mask DSIU interrupt register (R/W) */
#define VR41XX_NMIREG		__preg16(KSEG1 + 0x0B000098)	/* NMI register (R/W) */
#define VR41XX_SOFTINTREG	__preg16(KSEG1 + 0x0B00009A)	/* Software interrupt register (R/W) */
#define VR41XX_SYSINT2REG	__preg16(KSEG1 + 0x0B000200)	/* Level 1 System interrupt register 2 (R) */
#define VR41XX_GIUINTHREG	__preg16(KSEG1 + 0x0B000202)	/* Level 2 GIU interrupt register High (R) */
#define VR41XX_FIRINTREG	__preg16(KSEG1 + 0x0B000204)	/* Level 2 FIR interrupt register (R) */
#define VR41XX_MSYSINT2REG	__preg16(KSEG1 + 0x0B000206)	/* Level 1 mask system interrupt register 2 (R/W) */
#define VR41XX_MGIUINTHREG	__preg16(KSEG1 + 0x0B000208)	/* Level 2 mask GIU interrupt register High (R/W) */
#define VR41XX_MFIRINTREG	__preg16(KSEG1 + 0x0B00020A)	/* Level 2 mask FIR interrupt register (R/W) */

// Power Management Unit (PMU)
#define VR41XX_PMUINTREG	__preg16(KSEG1 + 0x0B0000A0)	/* PMU Status Register (R/W) */
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

#define VR41XX_PMUCNTREG	__preg16(KSEG1 + 0x0B0000A2)	/* PMU Control Register (R/W) */
#define VR41XX_PMUWAITREG	__preg16(KSEG1 + 0x0B0000A8)	/* PMU Wait Counter Register (R/W) */
#define VR41XX_PMUINT2REG	__preg16(KSEG1 + 0x0B0000A4)	/* PMU Interrupt/Status 2 Register (R/W) */
#define VR41XX_PMUCNT2REG	__preg16(KSEG1 + 0x0B0000A6)	/* PMU Control 2 Resister (R/W) */

// Real Time Clock Unit (RTC)
#define VR41XX_ETIMELREG	__preg16(KSEG1 + 0x0B0000C0)	/* Elapsed Time L Register (R/W) */
#define VR41XX_ETIMEMREG	__preg16(KSEG1 + 0x0B0000C2)	/* Elapsed Time M Register (R/W) */
#define VR41XX_ETIMEHREG	__preg16(KSEG1 + 0x0B0000C4)	/* Elapsed Time H Register (R/W) */
#define VR41XX_ECMPLREG		__preg16(KSEG1 + 0x0B0000C8)	/* Elapsed Compare L Register (R/W) */
#define VR41XX_ECMPMREG		__preg16(KSEG1 + 0x0B0000CA)	/* Elapsed Compare M Register (R/W) */
#define VR41XX_ECMPHREG		__preg16(KSEG1 + 0x0B0000CC)	/* Elapsed Compare H Register (R/W) */
#define VR41XX_RTCL1LREG	__preg16(KSEG1 + 0x0B0000D0)	/* RTC Long 1 L Register (R/W) */
#define VR41XX_RTCL1HREG	__preg16(KSEG1 + 0x0B0000D2)	/* RTC Long 1 H Register (R/W) */
#define VR41XX_RTCL1CNTLREG	__preg16(KSEG1 + 0x0B0000D4)	/* RTC Long 1 Count L Register (R) */
#define VR41XX_RTCL1CNTHREG	__preg16(KSEG1 + 0x0B0000D6)	/* RTC Long 1 Count H Register (R) */
#define VR41XX_RTCL2LREG	__preg16(KSEG1 + 0x0B0000D8)	/* RTC Long 2 L Register (R/W) */
#define VR41XX_RTCL2HREG	__preg16(KSEG1 + 0x0B0000DA)	/* RTC Long 2 H Register (R/W) */
#define VR41XX_RTCL2CNTLREG	__preg16(KSEG1 + 0x0B0000DC)	/* RTC Long 2 Count L Register (R) */
#define VR41XX_RTCL2CNTHREG	__preg16(KSEG1 + 0x0B0000DE)	/* RTC Long 2 Count H Register (R) */
#define VR41XX_RTCINTREG	__preg16(KSEG1 + 0x0B0001DE)	/* RTC Interrupt Register (R/W) */
#define VR41XX_TCLKLREG		__preg16(KSEG1 + 0x0B0001C0)	/* TCLK L Register (R/W) */
#define VR41XX_TCLKHREG		__preg16(KSEG1 + 0x0B0001C2)	/* TCLK H Register (R/W) */
#define VR41XX_TCLKCNTLREG	__preg16(KSEG1 + 0x0B0001C4)	/* TCLK Count L Register (R) */
#define VR41XX_TCLKCNTHREG	__preg16(KSEG1 + 0x0B0001C6)	/* TCLK Count H Register (R) */

// Deadman's Switch Unit (DSU)
#define VR41XX_DSUCNTREG	__preg16(KSEG1 + 0x0B0000E0)	/* DSU Control Register (R/W) */
#define VR41XX_DSUSETREG	__preg16(KSEG1 + 0x0B0000E2)	/* DSU Dead Time Set Register (R/W) */
#define VR41XX_DSUCLRREG	__preg16(KSEG1 + 0x0B0000E4)	/* DSU Clear Register (W) */
#define VR41XX_DSUTIMREG	__preg16(KSEG1 + 0x0B0000E6)	/* DSU Elapsed Time Register (R/W) */

// General Purpose I/O Unit (GIU)
#define VR41XX_GIUIOSELL	__preg16(KSEG1 + 0x0B000100)	/* GPIO Input/Output Select Register L (R/W) */
#define VR41XX_GIUIOSELH	__preg16(KSEG1 + 0x0B000102)	/* GPIO Input/Output Select Register H (R/W) */
#define VR41XX_GIUPIODL		__preg16(KSEG1 + 0x0B000104)	/* GPIO Port Input/Output Data Register L (R/W) */
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
#define VR41XX_GIUPIODH		__preg16(KSEG1 + 0x0B000106)	/* GPIO Port Input/Output Data Register H (R/W) */
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
#define VR41XX_GIUINTSTATL	__preg16(KSEG1 + 0x0B000108)	/* GPIO Interrupt Status Register L (R/W) */
#define VR41XX_GIUINTSTATH	__preg16(KSEG1 + 0x0B00010A)	/* GPIO Interrupt Status Register H (R/W) */
#define VR41XX_GIUINTENL	__preg16(KSEG1 + 0x0B00010C)	/* GPIO Interrupt Enable Register L (R/W) */
#define VR41XX_GIUINTENH	__preg16(KSEG1 + 0x0B00010E)	/* GPIO Interrupt Enable Register H (R/W) */
#define VR41XX_GIUINTTYPL	__preg16(KSEG1 + 0x0B000110)	/* GPIO Interrupt Type (Edge or Level) Select Register (R/W) */
#define VR41XX_GIUINTTYPH	__preg16(KSEG1 + 0x0B000112)	/* GPIO Interrupt Type (Edge or Level) Select Register (R/W) */
#define VR41XX_GIUINTALSELL	__preg16(KSEG1 + 0x0B000114)	/* GPIO Interrupt Active Level Select Register L (R/W) */
#define VR41XX_GIUINTALSELH	__preg16(KSEG1 + 0x0B000116)	/* GPIO Interrupt Active Level Select Register H (R/W) */
#define VR41XX_GIUINTHTSELL	__preg16(KSEG1 + 0x0B000118)	/* GPIO Interrupt Hold/Through Select Register L (R/W) */
#define VR41XX_GIUINTHTSELH	__preg16(KSEG1 + 0x0B00011A)	/* GPIO Interrupt Hold/Through Select Register H (R/W) */

#define VR41XX_GIUPODATL	__preg16(KSEG1 + 0x0B00011C)	/* GPIO Port Output Data Register L (R/W) */
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
#define VR41XX_GIUPODATH	__preg16(KSEG1 + 0x0B00011E)	/* GPIO Port Output Data Register H (R/W) */
#define VR41XX_GIUPODATH_GPIO51  0x0008
#define VR41XX_GIUPODATH_GPIO50  0x0004
#define VR41XX_GIUPODATH_GPIO49  0x0002
#define VR41XX_GIUPODATH_GPIO48  0x0001
#define VR41XX_GIUPODATH_PODAT3  0x0008
#define VR41XX_GIUPODATH_PODAT2  0x0004
#define VR41XX_GIUPODATH_PODAT1  0x0002
#define VR41XX_GIUPODATH_PODAT0  0x0001
#define VR41XX_GIUUSEUPDN	__preg16(KSEG1 + 0x0B0002E0)	/* GPIO Pullup/Down User Register (R/W) */
#define VR41XX_GIUTERMUPDN	__preg16(KSEG1 + 0x0B0002E2)	/* GPIO Terminal Pullup/Down Register (R/W) */
#define VR41XX_SECIRQMASKL	VR41XX_GIUINTENL
#define VR41XX_SECIRQMASKH	VR41XX_GIUINTENH

// Touch Panel Interface Unit (PIU)
#define VR41XX_PIUCNTREG	__preg16(KSEG1 + 0x0B000122)	/* PIU Control register (R/W) */
#define VR41XX_PIUCNTREG_PIUSEQEN	0x0004
#define VR41XX_PIUCNTREG_PIUPWR		0x0002
#define VR41XX_PIUCNTREG_PADRST		0x0001

#define VR41XX_PIUINTREG	__preg16(KSEG1 + 0x0B000124)	/* PIU Interrupt cause register (R/W) */
#define VR41XX_PIUINTREG_OVP		0x8000
#define VR41XX_PIUINTREG_PADCMD		0x0040
#define VR41XX_PIUINTREG_PADADP		0x0020
#define VR41XX_PIUINTREG_PADPAGE1	0x0010
#define VR41XX_PIUINTREG_PADPAGE0	0x0008
#define VR41XX_PIUINTREG_PADDLOST	0x0004
#define VR41XX_PIUINTREG_PENCHG		0x0001

#define VR41XX_PIUSIVLREG	__preg16(KSEG1 + 0x0B000126)	/* PIU Data sampling interval register (R/W) */
#define VR41XX_PIUSTBLREG	__preg16(KSEG1 + 0x0B000128)	/* PIU A/D converter start delay register (R/W) */
#define VR41XX_PIUCMDREG	__preg16(KSEG1 + 0x0B00012A)	/* PIU A/D command register (R/W) */
#define VR41XX_PIUASCNREG	__preg16(KSEG1 + 0x0B000130)	/* PIU A/D port scan register (R/W) */
#define VR41XX_PIUAMSKREG	__preg16(KSEG1 + 0x0B000132)	/* PIU A/D scan mask register (R/W) */
#define VR41XX_PIUCIVLREG	__preg16(KSEG1 + 0x0B00013E)	/* PIU Check interval register (R) */
#define VR41XX_PIUPB00REG	__preg16(KSEG1 + 0x0B0002A0)	/* PIU Page 0 Buffer 0 register (R/W) */
#define VR41XX_PIUPB01REG	__preg16(KSEG1 + 0x0B0002A2)	/* PIU Page 0 Buffer 1 register (R/W) */
#define VR41XX_PIUPB02REG	__preg16(KSEG1 + 0x0B0002A4)	/* PIU Page 0 Buffer 2 register (R/W) */
#define VR41XX_PIUPB03REG	__preg16(KSEG1 + 0x0B0002A6)	/* PIU Page 0 Buffer 3 register (R/W) */
#define VR41XX_PIUPB10REG	__preg16(KSEG1 + 0x0B0002A8)	/* PIU Page 1 Buffer 0 register (R/W) */
#define VR41XX_PIUPB11REG	__preg16(KSEG1 + 0x0B0002AA)	/* PIU Page 1 Buffer 1 register (R/W) */
#define VR41XX_PIUPB12REG	__preg16(KSEG1 + 0x0B0002AC)	/* PIU Page 1 Buffer 2 register (R/W) */
#define VR41XX_PIUPB13REG	__preg16(KSEG1 + 0x0B0002AE)	/* PIU Page 1 Buffer 3 register (R/W) */
#define VR41XX_PIUAB0REG	__preg16(KSEG1 + 0x0B0002B0)	/* PIU A/D scan Buffer 0 register (R/W) */
#define VR41XX_PIUAB1REG	__preg16(KSEG1 + 0x0B0002B2)	/* PIU A/D scan Buffer 1 register (R/W) */
#define VR41XX_PIUAB2REG	__preg16(KSEG1 + 0x0B0002B4)	/* PIU A/D scan Buffer 2 register (R/W) */
#define VR41XX_PIUAB3REG	__preg16(KSEG1 + 0x0B0002B6)	/* PIU A/D scan Buffer 3 register (R/W) */
#define VR41XX_PIUPB04REG	__preg16(KSEG1 + 0x0B0002BC)	/* PIU Page 0 Buffer 4 register (R/W) */
#define VR41XX_PIUPB14REG	__preg16(KSEG1 + 0x0B0002BE)	/* PIU Page 1 Buffer 4 register (R/W) */

// Audio Interface Unit (AIU)
#define VR41XX_SODATREG		__preg16(KSEG1 + 0x0B000166)	/* Speaker Output Data Register (R/W) */
#define VR41XX_SCNTREG		__preg16(KSEG1 + 0x0B000168)	/* Speaker Output Control Register (R/W) */
#define VR41XX_MIDATREG		__preg16(KSEG1 + 0x0B000170)	/* Mike Input Data Register (R/W) */
#define VR41XX_MCNTREG		__preg16(KSEG1 + 0x0B000172)	/* Mike Input Control Register (R/W) */
#define VR41XX_DVALIDREG	__preg16(KSEG1 + 0x0B000178)	/* Data Valid Register (R/W) */
#define VR41XX_SEQREG		__preg16(KSEG1 + 0x0B00017A)	/* Sequential Register (R/W) */
#define VR41XX_INTREG		__preg16(KSEG1 + 0x0B00017C)	/* Interrupt Register (R/W) */
#define VR41XX_MDMADATREG	__preg16(KSEG1 + 0x0B000160)	/* Mike DMA Data Register (R/W) */
#define VR41XX_SDMADATREG	__preg16(KSEG1 + 0x0B000162)	/* Speaker DMA Data Register (R/W) */
#define VR41XX_SCNVRREG		__preg16(KSEG1 + 0x0B00016A)	/* Speaker Conversion Rate Register (R/W) */
#define VR41XX_MCNVRREG		__preg16(KSEG1 + 0x0B000174)	/* Mike Conversion Rate Register (R/W) */

// Keyboard Interface Unit (KIU)
#define VR41XX_KIUDAT0		__preg16(KSEG1 + 0x0B000180)	/* KIU Data0 Register (R/W) */
#define VR41XX_KIUDAT1		__preg16(KSEG1 + 0x0B000182)	/* KIU Data1 Register (R/W) */
#define VR41XX_KIUDAT2		__preg16(KSEG1 + 0x0B000184)	/* KIU Data2 Register (R/W) */
#define VR41XX_KIUDAT3		__preg16(KSEG1 + 0x0B000186)	/* KIU Data3 Register (R/W) */
#define VR41XX_KIUDAT4		__preg16(KSEG1 + 0x0B000188)	/* KIU Data4 Register (R/W) */
#define VR41XX_KIUDAT5		__preg16(KSEG1 + 0x0B00018A)	/* KIU Data5 Register (R/W) */
#define VR41XX_KIUSCANREP	__preg16(KSEG1 + 0x0B000190)	/* KIU Scan/Repeat Register (R/W) */
#define VR41XX_KIUSCANREP_KEYEN      0x8000
#define VR41XX_KIUSCANREP_SCANSTP    0x0008
#define VR41XX_KIUSCANREP_SCANSTART  0x0004
#define VR41XX_KIUSCANREP_ATSTP      0x0002
#define VR41XX_KIUSCANREP_ATSCAN     0x0001
#define VR41XX_KIUSCANS		__preg16(KSEG1 + 0x0B000192)	/* KIU Scan Status Register (R) */
#define VR41XX_KIUWKS		__preg16(KSEG1 + 0x0B000194)	/* KIU Wait Keyscan Stable Register (R/W) */
#define VR41XX_KIUWKI		__preg16(KSEG1 + 0x0B000196)	/* KIU Wait Keyscan Interval Register (R/W) */
#define VR41XX_KIUINT		__preg16(KSEG1 + 0x0B000198)	/* KIU Interrupt Register (R/W) */
#define VR41XX_KIUINT_KDATLOST       0x0004
#define VR41XX_KIUINT_KDATRDY        0x0002
#define VR41XX_KIUINT_SCANINT        0x0001
#define VR41XX_KIURST		__preg16(KSEG1 + 0x0B00019A)	/* KIU Reset Register (W) */
#define VR41XX_KIUGPEN		__preg16(KSEG1 + 0x0B00019C)	/* KIU General Purpose Output Enable (R/W) */
#define VR41XX_SCANLINE		__preg16(KSEG1 + 0x0B00019E)	/* KIU Scan Line Register (R/W) */

// Debug Serial Interface Unit (DSIU)
#define VR41XX_PORTREG		__preg16(KSEG1 + 0x0B0001A0)	/* Port Change Register (R/W) */
#define VR41XX_MODEMREG		__preg16(KSEG1 + 0x0B0001A2)	/* Modem Control Register (R) */
#define VR41XX_ASIM00REG	__preg16(KSEG1 + 0x0B0001A4)	/* Asynchronous Mode 0 Register (R/W) */
#define VR41XX_ASIM01REG	__preg16(KSEG1 + 0x0B0001A6)	/* Asynchronous Mode 1 Register (R/W) */
#define VR41XX_RXB0RREG		__preg16(KSEG1 + 0x0B0001A8)	/* Receive Buffer Register (Extended) (R) */
#define VR41XX_RXB0LREG		__preg16(KSEG1 + 0x0B0001AA)	/* Receive Buffer Register (R) */
#define VR41XX_TXS0RREG		__preg16(KSEG1 + 0x0B0001AC)	/* Transmit Data Register (Extended) (R/W) */
#define VR41XX_TXS0LREG		__preg16(KSEG1 + 0x0B0001AE)	/* Transmit Data Register (R/W) */
#define VR41XX_ASIS0REG		__preg16(KSEG1 + 0x0B0001B0)	/* Status Register (R) */
#define VR41XX_INTR0REG		__preg16(KSEG1 + 0x0B0001B2)	/* Debug SIU Interrupt Register (R/W) */
#define VR41XX_BPRM0REG		__preg16(KSEG1 + 0x0B0001B6)	/* Baud rate Generator Prescaler Mode Register (R/W) */
#define VR41XX_DSIURESETREG	__preg16(KSEG1 + 0x0B0001B8)	/* Debug SIU Reset Register (R/W) */

// LED Control Unit (LED)
#define VR41XX_LEDHTSREG	__preg16(KSEG1 + 0x0B000240)	/* LED H Time Set register (R/W) */
#define VR41XX_LEDLTSREG	__preg16(KSEG1 + 0x0B000242)	/* LED L Time Set register (R/W) */
#define VR41XX_LEDCNTREG	__preg16(KSEG1 + 0x0B000248)	/* LED Control register (R/W) */
#define VR41XX_LEDASTCREG	__preg16(KSEG1 + 0x0B00024A)	/* LED Auto Stop Time Count register (R/W) */
#define VR41XX_LEDINTREG	__preg16(KSEG1 + 0x0B00024C)	/* LED Interrupt register (R/W) */

// Serial Interface Unit (SIU / SIU1 and SIU2)
#define VR41XX_SIURB		__preg8(KSEG1 + 0x0C000000)	/* Receiver Buffer Register (Read) DLAB = 0 (R) */
#define VR41XX_SIUTH		__preg8(KSEG1 + 0x0C000000)	/* Transmitter Holding Register (Write) DLAB = 0 (W) */
#define VR41XX_SIUDLL		__preg8(KSEG1 + 0x0C000000)	/* Divisor Latch (Least Significant Byte) DLAB = 1 (R/W) */
#define VR41XX_SIUIE		__preg8(KSEG1 + 0x0C000001)	/* Interrupt Enable DLAB = 0 (R/W) */
#define VR41XX_SIUDLM		__preg8(KSEG1 + 0x0C000001)	/* Divisor Latch (Most Significant Byte) DLAB = 1 (R/W) */
#define VR41XX_SIUIID		__preg8(KSEG1 + 0x0C000002)	/* Interrupt Identification Register (Read) (R) */
#define VR41XX_SIUFC		__preg8(KSEG1 + 0x0C000002)	/* FIFO Control Register (Write) (W) */
#define VR41XX_SIULC		__preg8(KSEG1 + 0x0C000003)	/* Line Control Register (R/W) */
#define VR41XX_SIUMC		__preg8(KSEG1 + 0x0C000004)	/* MODEM Control Register (R/W) */
#define VR41XX_SIULS		__preg8(KSEG1 + 0x0C000005)	/* Line Status Register (R/W) */
#define VR41XX_SIUMS		__preg8(KSEG1 + 0x0C000006)	/* MODEM Status Register (R/W) */
#define VR41XX_SIUSC		__preg8(KSEG1 + 0x0C000007)	/* Scratch Register (R/W) */
#define VR41XX_SIUIRSEL		__preg8(KSEG1 + 0x0C000008)	/* SIU/FIR IrDA Selector (R/W) */
#define VR41XX_SIURESET		__preg8(KSEG1 + 0x0C000009)	/* SIU Reset Register (R/W) */
#define VR41XX_SIUCSEL		__preg8(KSEG1 + 0x0C00000A)	/* SIU Echo-Back Control Register (R/W) */

// Modem Interface Unit (HSP)
// Not sure if some of these are right type, some may be 16 bit regs:
#define VR41XX_HSPINIT		__preg8(KSEG1 + 0x0C000020)	/* HSP Initialize Register (R/W) */
#define VR41XX_HSPDATAL		__preg8(KSEG1 + 0x0C000022)	/* HSP Data Register L (R/W) */
#define VR41XX_HSPDATAH		__preg8(KSEG1 + 0x0C000023)	/* HSP Data Register H (R/W) */
#define VR41XX_HSPINDEX		__preg8(KSEG1 + 0x0C000024)	/* HSP Index Register (W) */
#define VR41XX_HSPID		__preg8(KSEG1 + 0x0C000028)	/* HSP ID Register (R) */
#define VR41XX_HSPPCS		__preg8(KSEG1 + 0x0C000029)	/* HSP I/O Address Program Confirmation Register (R) */
#define VR41XX_HSPPCTEL		__preg8(KSEG1 + 0x0C000029)	/* HSP Signature Checking Port (W) */

// Fast IrDA Interface Unit (FIR)
#define VR41XX_FRSTR		__preg16(KSEG1 + 0x0C000040)	/* FIR Reset register (R/W) */
#define VR41XX_DPINTR		__preg16(KSEG1 + 0x0C000042)	/* DMA Page Interrupt register (R/W) */
#define VR41XX_DPCNTR		__preg16(KSEG1 + 0x0C000044)	/* DMA Control register (R/W) */
#define VR41XX_TDR		__preg16(KSEG1 + 0x0C000050)	/* Transmit Data register (W) */
#define VR41XX_RDR		__preg16(KSEG1 + 0x0C000052)	/* Receive Data register (R) */
#define VR41XX_IMR		__preg16(KSEG1 + 0x0C000054)	/* Interrupt Mask register (R/W) */
#define VR41XX_FSR		__preg16(KSEG1 + 0x0C000056)	/* FIFO Setup register (R/W) */
#define VR41XX_IRSR1		__preg16(KSEG1 + 0x0C000058)	/* Infrared Setup register 1 (R/W) */
#define VR41XX_CRCSR		__preg16(KSEG1 + 0x0C00005C)	/* CRC Setup register (R/W) */
#define VR41XX_FIRCR		__preg16(KSEG1 + 0x0C00005E)	/* FIR Control register (R/W) */
#define VR41XX_MIRCR		__preg16(KSEG1 + 0x0C000060)	/* MIR Control register (R/W) */
#define VR41XX_DMACR		__preg16(KSEG1 + 0x0C000062)	/* DMA Control register (R/W) */
#define VR41XX_DMAER		__preg16(KSEG1 + 0x0C000064)	/* DMA Enable register (R/W) */
#define VR41XX_TXIR		__preg16(KSEG1 + 0x0C000066)	/* Transmit Indication register (R) */
#define VR41XX_RXIR		__preg16(KSEG1 + 0x0C000068)	/* Receive Indication register (R) */
#define VR41XX_IFR		__preg16(KSEG1 + 0x0C00006A)	/* Interrupt Flag register (R) */
#define VR41XX_RXSTS		__preg16(KSEG1 + 0x0C00006C)	/* Receive Status (R) */
#define VR41XX_TXFL		__preg16(KSEG1 + 0x0C00006E)	/* Transmit Frame Length (R/W) */
#define VR41XX_MRXF		__preg16(KSEG1 + 0x0C000070)	/* Maximum Receive Frame Length (R/W) */
#define VR41XX_RXFL		__preg16(KSEG1 + 0x0C000074)	/* Receive Frame Length (R) */

// physical address spaces
#define VR41XX_LCD             0x0a000000
#define VR41XX_INTERNAL_IO_2   0x0b000000
#define VR41XX_INTERNAL_IO_1   0x0c000000
#define VR41XX_ISA_MEM         0x10000000
#define VR41XX_ISA_IO          0x14000000
#define VR41XX_ROM             0x18000000

// This is the base address for IO port decoding to which the 16 bit IO port address
// is added.  Defining it to 0 will usually cause a kernel oops any time port IO is
// attempted, which can be handy for turning up parts of the kernel that make
// incorrect architecture assumptions (by assuming that everything acts like a PC),
// but we need it correctly defined to use the PCMCIA/CF controller:
#define VR41XX_PORT_BASE	(KSEG1 + VR41XX_ISA_IO)
#define VR41XX_ISAMEM_BASE	(KSEG1 + VR41XX_ISA_MEM)

#endif /* __ASM_MIPS_VR4121_H */
