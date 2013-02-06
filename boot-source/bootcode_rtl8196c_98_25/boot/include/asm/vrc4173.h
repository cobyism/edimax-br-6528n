/* $Id: vrc4173.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2000 by Michael R. McDonald
 */
#ifndef __ASM_MIPS_VRC4173_H 
#define __ASM_MIPS_VRC4173_H 

#include <asm/addrspace.h>

#define VRC4173_IO_EXTENT	4096
#define VRC4173_SYSINT_MASK	0x0179	/* No doze, AC97, PCMCIA, nor KIU */
#define VRC4173_SELECTREG_MASK	0x7	/* Enable touch panel, PS2 1&2, and GPIO[0-15] */

/* VRC4173 subfunction devices */
#define USB_DEV		0
#define PCMCIA2_DEV	1
#define PCMCIA1_DEV	2
#define PS2AUX_DEV	3
#define PS2KBD_DEV	4
#define PIU_DEV		5
#define AIU_DEV		6
#define KIU_DEV		7
#define GIU_DEV		8
#define AC97_DEV	9

/* DMA Address Unit (DMAAU) */
#define VRC4173_AIUIBALREG	0x000	/* AIU IN DMA Base Address Register Low (R/W) */
#define VRC4173_AIUIBAHREG	0x002	/* AIU IN DMA Base Address Register High (R/W) */
#define VRC4173_AIUIALREG	0x004	/* AIU IN DMA Address Register Low (R/W) */
#define VRC4173_AIUIAHREG	0x006	/* AIU IN DMA Address Register High (R/W) */
#define VRC4173_AIUOBALREG	0x008	/* AIU OUT DMA Base Address Register Low (R/W) */
#define VRC4173_AIUOBAHREG	0x00A	/* AIU OUT DMA Base Address Register High (R/W) */
#define VRC4173_AIUOALREG	0x00C	/* AIU OUT DMA Address Register Low (R/W) */
#define VRC4173_AIUOAHREG	0x00E	/* AIU OUT DMA Address Register High (R/W) */
#define VRC4173_FIRBALREG	0x010	/* FIR DMA Base Address Register Low (R/W) */
#define VRC4173_FIRBAHREG	0x012	/* FIR DMA Base Address Register High (R/W) */
#define VRC4173_FIRALREG	0x014	/* FIR DMA Address Register Low (R/W) */
#define VRC4173_FIRAHREG	0x016	/* FIR DMA Address Register High (R/W) */

/* DMA Control Unit (DCU) */
#define VRC4173_DMARSTREG	0x020	/* DMA Reset Register */
#define VRC4173_DMAIDLEREG	0x022	/* DMA Sequencer Status Register */
#define VRC4173_DMASENREG	0x024	/* DMA Sequencer Enable Register */
#define VRC4173_DMAMSKREG	0x026	/* DMA Mask Register */
#define VRC4173_DMAREQREG	0x028	/* DMA Request Register */

/* Clock Mask Unit (CMU) */
#define VRC4173_CMUCLKMSK	0x040	/* 4173 CMU Clock Mask Register */
#define VRC4173_CMUSRST		0x042	/* 4173 CMU Soft Reset Register */

/* Interrupt Control Unit (ICU) */
#define VRC4173_SYSINT1REG	0x060	/* Level 1 System interrupt register 1 */
#define VRC4173_PIUINTREG_RO	0x062	/* Level 2 PIU interrupt register */
#define VRC4173_AIUINTREG	0x064	/* Level 2 AIU interrupt register */
#define VRC4173_KIUINTREG	0x066	/* Level 2 KIU interrupt register */
#define VRC4173_GIULINTREG	0x068	/* Level 2 GIU interrupt register Low */
#define VRC4173_GIUHINTREG	0x06A	/* Level 2 GIU interrupt register High */
#define VRC4173_MSYSINT1REG	0x06C	/* Level 1 mask system interrupt register 1 */
#define VRC4173_MPIUINTREG	0x06E	/* Level 2 mask PIU interrupt register */
#define VRC4173_MAIUINTREG	0x070	/* Level 2 mask AIU interrupt register */
#define VRC4173_MKIUINTREG	0x072	/* Level 2 mask KIU interrupt register */
#define VRC4173_MGIULINTREG	0x074	/* Level 2 mask GIU interrupt register Low */
#define VRC4173_MGIUHINTREG	0x076	/* Level 2 mask GIU interrupt register High */

/* General Purpose I/O Unit (GIU) */
#define VRC4173_GIUDIRL		0x080	/* GPIO Input/Output Select Register L */
#define VRC4173_GIUDIRH		0x082	/* GPIO Input/Output Select Register H */
#define VRC4173_GIUPIODL	0x084	/* GPIO Port Input/Output Data Register L */
#define VRC4173_GIUPIODH	0x086	/* GPIO Port Input/Output Data Register H */
#define VRC4173_GIUINTSTATL	0x088	/* GPIO Interrupt Status Register L */
#define VRC4173_GIUINTSTATH	0x08A	/* GPIO Interrupt Status Register H */
#define VRC4173_GIUINTENL	0x08C	/* GPIO Interrupt Enable Register L */
#define VRC4173_GIUINTENH	0x08E	/* GPIO Interrupt Enable Register H */
#define VRC4173_GIUINTTYPL	0x090	/* GPIO Interrupt Type (Edge/Level) Select Register */
#define VRC4173_GIUINTTYPH	0x092	/* GPIO Interrupt Type (Edge/Level) Select Register */
#define VRC4173_GIUINTALSELL	0x094	/* GPIO Interrupt Active Level Select Register L */
#define VRC4173_GIUINTALSELH	0x096	/* GPIO Interrupt Active Level Select Register H */
#define VRC4173_GIUINTHTSELL	0x098	/* GPIO Interrupt Hold/Through Select Register L */
#define VRC4173_GIUINTHTSELH	0x09A	/* GPIO Interrupt Hold/Through Select Register H */
#define VRC4173_SELECTREG	0x09E	/* GPIO Port Output Data Enable Register */

#define VRC4173_SELECT_GPIO_L	1
#define VRC4173_SELECT_PS2_2	2
#define VRC4173_SELECT_PS2_1	4
#define VRC4173_SELECT_GPIO_H	8

#define VRC4173_GIUPIODH_GPIO20		0x0010
#define VRC4173_GIUPIODH_GPIO19		0x0008
#define VRC4173_GIUPIODH_GPIO18		0x0004
#define VRC4173_GIUPIODH_GPIO17		0x0002
#define VRC4173_GIUPIODH_GPIO16		0x0001
#define VRC4173_GIUPIODL_GPIO15		0x8000
#define VRC4173_GIUPIODL_GPIO14		0x4000
#define VRC4173_GIUPIODL_GPIO13		0x2000
#define VRC4173_GIUPIODL_GPIO12		0x1000
#define VRC4173_GIUPIODL_GPIO11		0x0800
#define VRC4173_GIUPIODL_GPIO10		0x0400
#define VRC4173_GIUPIODL_GPIO9		0x0200
#define VRC4173_GIUPIODL_GPIO8		0x0100
#define VRC4173_GIUPIODL_GPIO7		0x0080
#define VRC4173_GIUPIODL_GPIO6		0x0040
#define VRC4173_GIUPIODL_GPIO5		0x0020
#define VRC4173_GIUPIODL_GPIO4		0x0010
#define VRC4173_GIUPIODL_GPIO3		0x0008
#define VRC4173_GIUPIODL_GPIO2		0x0004
#define VRC4173_GIUPIODL_GPIO1		0x0002
#define VRC4173_GIUPIODL_GPIO0		0x0001

/* Touch Panel Interface Unit (PIU) */
#define VRC4173_PIUCNTREG	0x0A2	/* PIU Control register (R/W) */
#define VRC4173_PIUCNTREG_PIUSEQEN	0x0004
#define VRC4173_PIUCNTREG_PIUPWR	0x0002
#define VRC4173_PIUCNTREG_PADRST	0x0001
#define VRC4173_PIUCNTREG_STATE_DISABLE		0
#define VRC4173_PIUCNTREG_STATE_STANDBY		1
#define VRC4173_PIUCNTREG_STATE_PORTSCAN	2
#define VRC4173_PIUCNTREG_STATE_WAITPEN		4
#define VRC4173_PIUCNTREG_STATE_PENSCAN		5
#define VRC4173_PIUCNTREG_STATE_NEXTSCAN	6
#define VRC4173_PIUCNTREG_STATE_CMDSCAN		7

#define VRC4173_PIUINTREG		0x0A4	/* PIU Interrupt cause register (R/W) */
#define VRC4173_PIUINTREG_OVP		0x8000
#define VRC4173_PIUINTREG_PADCMD	0x0040
#define VRC4173_PIUINTREG_PADADP	0x0020
#define VRC4173_PIUINTREG_PADPAGE1	0x0010
#define VRC4173_PIUINTREG_PADPAGE0	0x0008
#define VRC4173_PIUINTREG_PADDLOST	0x0004
#define VRC4173_PIUINTREG_PENCHG	0x0001

#define VRC4173_PIUSIVLREG	0x0A6	/* PIU Data sampling interval register (R/W) */
#define VRC4173_PIUSTBLREG	0x0A8	/* PIU A/D converter start delay register (R/W) */
#define VRC4173_PIUCMDREG	0x0AA	/* PIU A/D command register (R/W) */
#define VRC4173_PIUASCNREG	0x0B0	/* PIU A/D port scan register (R/W) */
#define VRC4173_PIUAMSKREG	0x0B2	/* PIU A/D scan mask register (R/W) */
#define VRC4173_PIUCIVLREG	0x0BE	/* PIU Check interval register (R) */
#define VRC4173_PIUPB00REG	0x0C0	/* PIU Page 0 Buffer 0 register (R/W) */
#define VRC4173_PIUPB01REG	0x0C2	/* PIU Page 0 Buffer 1 register (R/W) */
#define VRC4173_PIUPB02REG	0x0C4	/* PIU Page 0 Buffer 2 register (R/W) */
#define VRC4173_PIUPB03REG	0x0C6	/* PIU Page 0 Buffer 3 register (R/W) */
#define VRC4173_PIUPB10REG	0x0C8	/* PIU Page 1 Buffer 0 register (R/W) */
#define VRC4173_PIUPB11REG	0x0CA	/* PIU Page 1 Buffer 1 register (R/W) */
#define VRC4173_PIUPB12REG	0x0CC	/* PIU Page 1 Buffer 2 register (R/W) */
#define VRC4173_PIUPB13REG	0x0CE	/* PIU Page 1 Buffer 3 register (R/W) */
#define VRC4173_PIUAB0REG	0x0D0	/* PIU A/D scan Buffer 0 register (R/W) */
#define VRC4173_PIUAB1REG	0x0D2	/* PIU A/D scan Buffer 1 register (R/W) */
#define VRC4173_PIUPB04REG	0x0DC	/* PIU Page 0 Buffer 4 register (R/W) */
#define VRC4173_PIUPB14REG	0x0DE	/* PIU Page 1 Buffer 4 register (R/W) */

/* Audio Interface Unit (AIU) */
#define VRC4173_MDMADATREG	0x0E0	/* Mike DMA Data Register (R/W) */
#define VRC4173_SDMADATREG	0x0E2	/* Speaker DMA Data Register (R/W) */
#define VRC4173_SODATREG	0x0E6	/* Speaker Output Data Register (R/W) */
#define VRC4173_SCNTREG		0x0E8	/* Speaker Output Control Register (R/W) */
#define VRC4173_SCNVRREG	0x0EA	/* Speaker Conversion Rate Register (R/W) */
#define VRC4173_MIDATREG	0x0F0	/* Mike Input Data Register (R/W) */
#define VRC4173_MCNTREG		0x0F2	/* Mike Input Control Register (R/W) */
#define VRC4173_MCNVRREG	0x0F4	/* Mike Conversion Rate Register (R/W) */
#define VRC4173_DVALIDREG	0x0F8	/* Data Valid Register (R/W) */
#define VRC4173_SEQREG		0x0FA	/* Sequential Register (R/W) */
#define VRC4173_INTREG		0x0FC	/* Interrupt Register (R/W) */

/* Keyboard Interface Unit (KIU) of the VRC4173 */
#define VRC4173_KIUDAT0		0x100	/* KIU Data0 Register (R/W) */
#define VRC4173_KIUDAT1		0x102	/* KIU Data1 Register (R/W) */
#define VRC4173_KIUDAT2		0x104	/* KIU Data2 Register (R/W) */
#define VRC4173_KIUDAT3		0x106	/* KIU Data3 Register (R/W) */
#define VRC4173_KIUDAT4		0x108	/* KIU Data4 Register (R/W) */
#define VRC4173_KIUDAT5		0x10A	/* KIU Data5 Register (R/W) */
#define VRC4173_KIUSCANREP	0x110	/* KIU Scan/Repeat Register (R/W) */
#define VRC4173_KIUSCANREP_KEYEN	0x8000
#define VRC4173_KIUSCANREP_SCANSTP	0x0008
#define VRC4173_KIUSCANREP_SCANSTART	0x0004
#define VRC4173_KIUSCANREP_ATSTP	0x0002
#define VRC4173_KIUSCANREP_ATSCAN	0x0001
#define VRC4173_KIUSCANS	0x112	/* KIU Scan Status Register (R) */
#define VRC4173_KIUWKS		0x114	/* KIU Wait Keyscan Stable Register (R/W) */
#define VRC4173_KIUWKI		0x116	/* KIU Wait Keyscan Interval Register (R/W) */
#define VRC4173_KIUINT		0x118	/* KIU Interrupt Register (R/W) */
#define VRC4173_KIUINT_KDATLOST		0x0004
#define VRC4173_KIUINT_KDATRDY		0x0002
#define VRC4173_KIUINT_SCANINT		0x0001
#define VRC4173_KIURST		0x11A	/* KIU Reset Register (W) */
#define VRC4173_KIUGPEN		0x11C	/* KIU General Purpose Output Enable (R/W) */
#define VRC4173_SCANLINE	0x11E	/* KIU Scan Line Register (R/W) */

// PS2 Interface Unit (PSIU) of the VRC4173
#define VRC4173_PS2CH1DATA	0x120	/* PS2 Channel 1 Data Register (R/W) */
#define VRC4173_PS2CH1CTRL	0x122	/* PS2 Channel 1 Control Register (R/W) */
#define VRC4173_PS2CH1RST	0x124	/* PS2 Channel 1 Reset Register (R/W) */

#define VRC4173_PS2CH2DATA	0x140	/* PS2 Channel 2 Data Register (R/W) */
#define VRC4173_PS2CH2CTRL	0x142	/* PS2 Channel 2 Control Register (R/W) */
#define VRC4173_PS2CH2RST	0x144	/* PS2 Channel 2 Reset Register (R/W) */

/* AC97 Unit (AC97U) */
#define VRC4173_AC97_INT_STATUS		0x000	/* Interrupt Clear/Status Register */
#define VRC4173_AC97_CODEC_WR		0x004	/* Codec Write Register */
#define VRC4173_AC97_CODEC_RD		0x008	/* Codec Read Register */
#define VRC4173_AC97_ACLINK_CTRL	0x01C	/* ACLINK Control Register */

#define VRC4173_AC97_CODEC_WR_RWC	(1<<23)	/* sets read/write command */
#define VRC4173_AC97_CODEC_WR_WRDY	(1<<31)	/* write ready */

#define VRC4173_AC97_CODEC_RD_RDRDY	(1<<30)	/* Read Data Ready */
#define VRC4173_AC97_CODEC_RD_DMASK	0xffff	/* Read Data Mask */

#define VRC4173_AC97_ACLINK_CTRL_SYNC_ON	(1<<30)	/* Codec sync bit */

#endif /* __ASM_MIPS_VRC4173_H */
