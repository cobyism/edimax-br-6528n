/*
 * linux/include/asm-mips/vrc4172.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2000 SATO Kazumi. 
 */

/*
 * Vrc4172 (Vr4121 companion chip) device units definitions
 */

#ifndef __ASM_MIPS_VRC4172_H_
#define __ASM_MIPS_VRC4172_H_

#include <asm/vr41xx.h>

#define VRC4172_GPIOL_ADDR	0x15001080	/* GPIO (0..15) */
#define VRC4172_PCS_ADDR	0x15001090	/* PCS Programable chip selects */
#define VRC4172_GPIOH_ADDR	0x150010c0	/* GPIO (16..23) */
#define VRC4172_PMU_ADDR	0x15003800	/* PMU */
#define VRC4172_ICU_ADDR	0x15003808	/* ICU */
#define VRC4172_COM_ADDR	0x15003810	/* NS16550A compat */
#define VRC4172_PIO_ADDR	0x15003820	/* IEEE1284 parallel */
#define VRC4172_PS2_ADDR	0x15003870	/* PS/2 controler */
#define VRC4172_PWM_ADDR	0x15003880	/* PWM (backlight pulus) controller */
#define VRC4172_GPIOL_BASE	__preg16(KSEG1 + 0x15001080)	/* GPIO (0..15) */
#define VRC4172_PCS_BASE	__preg16(KSEG1 + 0x15001090)	/* PCS Programable chip selects */
#define VRC4172_GPIOH_BASE	__preg16(KSEG1 + 0x150010c0)	/* GPIO (16..23) */
#define VRC4172_PMU_BASE	__preg16(KSEG1 + 0x15003800)	/* PMU */
#define VRC4172_ICU_BASE	__preg16(KSEG1 + 0x15003808)	/* ICU */
#define VRC4172_COM_BASE	__preg16(KSEG1 + 0x15003810)	/* NS16550A compat */
#define VRC4172_PIO_BASE	__preg16(KSEG1 + 0x15003820)	/* IEEE1284 parallel */
#define VRC4172_PS2_BASE	__preg16(KSEG1 + 0x15003870)	/* PS/2 controler */
#define VRC4172_PWM_BASE	__preg16(KSEG1 + 0x15003880)	/* PWM (backlight pulus) controller */


/*
 *	Vrc4172 GPIO (General Purpose I/O) Unit Registers.
 */

#define VRC4172_EXGPDATA0	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x00)	/* I/O data (0..15) */
#define VRC4172_EXGPDIR0	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x02)	/* direction (0..15) */
#define VRC4172_EXGPINTEN0	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x04)	/* interrupt enable (0..15) */
#define VRC4172_EXGPINTST0	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x06)	/* interrupt status (0..15) */
#define VRC4172_EXGPINTTYP0	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x08)	/* interrupt type (0..15) */
#define VRC4172_EXGPINTLV0L	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x0a)	/* interrupt level low (0..15) */
#define VRC4172_EXGPINTLV0H	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x0c)	/* interrupt level high (0..15) */

#define VRC4172_EXGPDATA1	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x40)	/* I/O data (16..23) */
#define VRC4172_EXGPDIR1	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x42)	/* direction (16..23) */
#define VRC4172_EXGPINTEN1	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x44)	/* interrupt enable (16..23) */
#define VRC4172_EXGPINTST1	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x46)	/* interrupt status (16..23) */
#define VRC4172_EXGPINTTYP1	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x48)	/* interrupt type (16..23) */
#define VRC4172_EXGPINTLV1L	__preg16(KSEG1 + VRC4172_GPIOL_ADDR + 0x4a)	/* interrupt level low (16..23) */

#define VRC4172_EXGPINTTYP_EDGE	1
#define VRC4172_EXGPINTTYP_LEVEL	0
#define VRC4172_EXGPINTAL_HIGH	1
#define VRC4172_EXGPINTAL_LOW	0
#define VRC4172_EXGPINTHT_HOLD	1
#define VRC4172_EXGPINTHT_THROUGH	0

/*
 * Vrc4172 ICU unit register definition
 */
#define VRC4172_ICU_1284INTRQ	__preg16(KSEG1 + VRC4172_ICU_ADDR + 0x0)
#define 	VRC4172_ICU_1284IRQ	0x08	/* interrupt to IRQ */
#define 	VRC4172_ICU_1284INTRP	0x04	/* interrupt to INTRP */
#define 	VRC4172_ICU_1284THRU	0x02	/* turu status */
#define 	VRC4172_ICU_1284LATCH	0x01	/* latched status and clear */

#define VRC4172_ICU_16550INTRQ	__preg16(KSEG1 + VRC4172_ICU_ADDR + 0x2)
#define 	VRC4172_ICU_16550THRU	0x02	/* turu status */
#define 	VRC4172_ICU_16550LATCH	0x01	/* latched status and clear */


/*
 *	Vrc4172 PCS (Programable Chip Select) Unit Registers.
 */
#define VRC4172_EXCS0SELL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x00)
#define VRC4172_EXCS0SELH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x02)
#define VRC4172_EXCS0MSKL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x04)
#define VRC4172_EXCS0MSKH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x06)
#define VRC4172_EXCS1SELL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x08)
#define VRC4172_EXCS1SELH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x0a)
#define VRC4172_EXCS1MSKL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x0c)
#define VRC4172_EXCS1MSKH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x0e)
#define VRC4172_EXCS2SELL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x10)
#define VRC4172_EXCS2SELH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x12)
#define VRC4172_EXCS2MSKL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x14)
#define VRC4172_EXCS2MSKH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x16)
#define VRC4172_EXCS3SELL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x18)
#define VRC4172_EXCS3SELH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x1a)
#define VRC4172_EXCS3MSKL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x1c)
#define VRC4172_EXCS3MSKH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x1e)
#define VRC4172_EXCS4SELL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x20)
#define VRC4172_EXCS4SELH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x22)
#define VRC4172_EXCS4MSKL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x24)
#define VRC4172_EXCS4MSKH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x26)
#define VRC4172_EXCS5SELL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x28)
#define VRC4172_EXCS5SELH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x2a)
#define VRC4172_EXCS5MSKL	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x2c)
#define VRC4172_EXCS5MSKH	__preg16(KSEG1 + VRC4172_PCS_ADDR + 0x2e)

/* for EXCSnSELL */
#define VRC4172_EXCSSELLMASK	0xfffe
/* for EXCSnSELH */
#define VRC4172_EXCSSELHMASK	0x01ff

/* for EXCSnMSKL */
#define VRC4172_EXCSENMASK	0x1
#define VRC4172_EXCSEN		0x1
#define VRC4172_EXCSDIS		0x00
#define VRC4172_EXCSMSKLMASK	0xfffe
/* for EXCSnMSKH */
#define VRC4172_EXCSMSKHMASK	0x01ff

/*
 * Vrc4172 PMU unit register definition
 */

#define VRC4172_PMU_SYSCLKCTRL	__preg16(KSEG1 + VRC4172_PMU_ADDR + 0x00)
#define 	VRC4172_PMU_IRST	0x20	/* internal reset */
#define		VRC4172_PMU_OSCDIS	0x10	/* OSC disable */
#define 	VRC4172_PMU_CKO48	0x01	/* CKO48 enable */
#define VRC4172_PMU_1284CTRL	__preg16(KSEG1 + VRC4172_PMU_ADDR + 0x02)
#define 	VRC4172_PMU_1284EN	0x04	/* 1284 enable */
#define 	VRC4172_PMU_1284RST	0x02	/* 1284 reset (>= 1us) */
#define 	VRC4172_PMU_1284CLKDIS	0x01	/* 1284 clock disanle */
#define VRC4172_PMU_16550CTRL	__preg16(KSEG1 + VRC4172_PMU_ADDR + 0x04)
#define 	VRC4172_PMU_16550RST	0x02	/* 16550 reset (>= 200ms) */
#define 	VRC4172_PMU_16550CLKDIS	0x01	/* 16550 clock disable */
#define VRC4172_PMU_USBCTL	__preg16(KSEG1 + VRC4172_PMU_ADDR + 0x0c)
#define 	VRC4172_PMU_USBCLKDIS	0x01	/* USB clock disable  */
#define VRC4172_PMU_PS2PWMCTL	__preg16(KSEG1 + VRC4172_PMU_ADDR + 0x0e)
#define 	VRC4172_PMU_PWMCLKDIS	0x10	/* PWM clock disable */
#define		VRC4172_PMU_PS2RST	0x02	/* PS2 reset */
#define		VRC4172_PMU_PS2CLKDIS	0x01	/* PS2 clock disable */


/*
 * Vrc4172 PWM unit register definition
 */

#define VRC4172_PWM_LCDDUTYEN	__preg16(KSEG1 + VRC4172_PWM_ADDR + 0x00)		/* LCDBAK control enable */
#define		VRC4172_PWM_LCDEN_MASK	0x01
#define 	VRC4172_PWM_LCD_EN	0x01	/* enable */
#define		VRC4172_PWM_LCD_DIS	0x00	/* disable */
#define VRC4172_PWM_LCDFREQ	__preg16(KSEG1 + VRC4172_PWM_ADDR + 0x02)
#define 	VRC4172_PWM_LCDFREQ_MASK	0xff
		/* f = 1(2^4/8000000 * 64 x (LCDFREQ+1) */
#define VRC4172_PWM_LCDDUTY	__preg16(KSEG1 + VRC4172_PWM_ADDR + 0x04)
#define 	VRC4172_PWM_LCDDUTY_MASK	0x3f
		/* T = 1/(64f) x (LCDDUTY+1) */

#endif /* __ASM_MIPS_VRC4172_H */
