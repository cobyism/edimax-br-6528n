/* $Id: vr41xx-platdep.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999 by Michael Klar
 */
#ifndef __ASM_MIPS_VR41XX_PLATDEP_H 
#define __ASM_MIPS_VR41XX_PLATDEP_H 

/*
 * This file contains the device-specific defines for VR41xx CPU-based
 * platforms.  Anything CPU-specific should go in vr41xx.h instead.
 * Eventually, most, if not all, dependancies on CONFIG_[device_name]
 * should be moved in here, which should make it easier to add new
 * device support.
 */

#include <linux/config.h>
#include <asm/vr41xx.h>

/*
 * Here are the macros defined in this file and what they mean.  If not
 * defined for a platform, reasonable defaults are used.
 *
 * DEVICE_IRQ_MASKL	IRQ mask for GPIO ints, to disable some IRQs for
 *			autodetect.  A bitmask corresponding to IRQ #s
 *			40 through 55.  A 0 bit means disabled, a 1 bit
 *			will enable for autodetect only if the line is
 *			configured as input at boot.
 * DEVICE_IRQ_MASKH	Same for IRQ #s 56 through 71.  Note that a VR4111
 * 			or VR4121-based device that uses 32-bit data bus
 *			should probably set DEVICE_IRQ_MASKH to 0.
 * ADJUSTED_PORT_BASE	Some devices have ISA IO ports at a different
 *			offset than the standard.  In particular,
 *			VRC4171 PCMCIA controller needs an extra offset.
 *			This value is the virtual address corresponding
 *			to ISA IO port 0.
 * For VIDEORAM_* ad FB_*, see drivers/video/sfb.c for description, those
 * are not used unless Simple Frame Buffer (or maybe one of its
 * derivatives) is used.
 * GPIO_BTN_MAP		Default map of GPIO lines to button definition.
 * GPIO_BTN_PRESS_LOW	Define this if GPIO level 0 corresponds to a
 *			button press.  Leave undefined if 1 correspods
 *			to button press.
 * KBD_SCANLINES	For devices with keyboards, the number of KIU
 *			scanlines to use.  Default is all 12.
 *
 * (more to come...)
 */

#ifdef CONFIG_CASIO_E10
#define VIDEORAM_BASE     (KSEG1 + 0x0a000000)
#define FB_X_RES          240
#define FB_X_VIRTUAL_RES  1024
#define FB_Y_RES          320
#define FB_BPP            2
#define FB_IS_GREY        1
#endif

#ifdef CONFIG_CASIO_E15
#define VIDEORAM_BASE     (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE     (256 * 1024) /* ??? */
#define FB_X_RES          240
#define FB_X_VIRTUAL_RES  512
#define FB_Y_RES          320
#define FB_BPP            4
#define FB_IS_GREY        1
#define FB_IS_INVERSE     1
#endif

#ifdef CONFIG_CASIO_E105
#define VIDEORAM_BASE     (KSEG1 + 0x0a200000)
#define FB_X_RES          240
#define FB_X_VIRTUAL_RES  256
#define FB_Y_RES          320
#define FB_BPP            16
#endif

#ifdef CONFIG_COMPAQ_AERO_15XX
#define VIDEORAM_BASE     (KSEG1 + 0x0a000000)
#define ADJUSTED_PORT_BASE (VR41XX_PORT_BASE + 0x01000000)
#define FB_X_RES          320
#define FB_Y_RES          240
#define FB_BPP            4
#define FB_IS_GREY        1
#define FB_IS_INVERSE     1
#define GPIO_BTN_MAP { 0, BTN_AP5, 0, 0, 0, 0, 0, \
               BTN_EXIT, BTN_POWER, BTN_AP4, BTN_AP3, BTN_AP2, BTN_AP1, 0, \
               0, 0, BTN_UP, BTN_DOWN, BTN_ACTION, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= (VR41XX_GIUPODATL_GPIO33 | \
				      VR41XX_GIUPODATL_GPIO34); \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()	\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~(VR41XX_GIUPODATL_GPIO33 | \
				       VR41XX_GIUPODATL_GPIO34); \
		restore_flags(flags);	\
	}
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO15;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO15;
#endif

#ifdef CONFIG_COMPAQ_AERO_21XX
#define VIDEORAM_BASE     (KSEG1 + 0x0a000000)
#define DEVICE_IRQ_MASKL 0xfffb
#define DEVICE_IRQ_MASKH 0xffff
#define ADJUSTED_PORT_BASE (VR41XX_PORT_BASE + 0x01000000)
#define FB_X_RES          320
#define FB_Y_RES          240
#define FB_BPP            8
#endif

#ifdef CONFIG_EVEREX_FREESTYLE
#define DEVICE_IRQ_MASKL 0xfffb
#define DEVICE_IRQ_MASKH 0xffff
#define ADJUSTED_PORT_BASE (VR41XX_PORT_BASE + 0x01000000)
#define VIDEORAM_SIZE  (256 * 1024)
#define FB_X_RES       320
#define FB_Y_RES       240
#define FB_BPP         4
#define FB_IS_GREY     1
#define FB_IS_INVERSE  1
#define GPIO_BTN_MAP { 0, BTN_AP5, 0, 0, BTN_UP, BTN_DOWN, BTN_ACTION, \
		BTN_EXIT, BTN_CONTRAST, BTN_AP1, BTN_AP2, BTN_AP3, BTN_AP4, 0, \
		0, 0, 0, 0, 0, 0, BTN_BACKLIGHT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define GPIO_BTN_PRESS_LOW
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPODATH |= VR41XX_GIUPODATH_GPIO48;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPODATH &= ~VR41XX_GIUPODATH_GPIO48;

#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_IBM_WORKPAD
#define DEVICE_IRQ_MASKL 0xffff
#define DEVICE_IRQ_MASKH 0x0000
#define ADJUSTED_PORT_BASE (VR41XX_PORT_BASE + 0x01000000)
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define FB_X_RES       640
#define FB_Y_RES       480
#define FB_BPP         16
#define GPIO_BTN_MAP { 0, BTN_AP5, BTN_NOTIFICATION, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define GPIO_BTN_PRESS_LOW
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATH |= VR41XX_GIUPODATH_GPIO49; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATH &= ~VR41XX_GIUPODATH_GPIO49; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_AGENDA_VR3
#define GPIO_BTN_MAP { \
	0, 0, 0, 0, 0, 0, 0, BTN_PEN, \
	BTN_SYNC, BTN_BATT_DOOR, 0, BTN_POWER_GPIO, 0, 0, 0, 0 \
}
#undef GPIO_BTN_PRESS_LOW
#define VR41XX_ENABLE_IRDA()  irda_power(1);
#define VR41XX_DISABLE_IRDA() irda_power(0);
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GPDATHREG |= VR41XX_GPDATHREG_GPIO19; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()	\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GPDATHREG &= ~VR41XX_GPDATHREG_GPIO19; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_NEC_MOBILEPRO_700
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (64 * 1024)
#define FB_X_VIRTUAL_RES  1024
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         2
#define FB_IS_GREY     1
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_NEC_MOBILEPRO_750C
#define VIDEORAM_BASE  (KSEG1 + 0x13000000)
#define FB_X_VIRTUAL_RES  1024
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         8
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_NEC_MOBILEPRO_770
// Tested by Jean-Nicolas, thanks!
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  800
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         16
// GIUPODAT[12] is speaker power on/off bit confirmed by Checkstuff.exe
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_NEC_MOBILEPRO_800
#define VIDEORAM_BASE     (KSEG1 + 0x0a000000)
#define FB_X_RES          800
#define FB_Y_RES          600
#define FB_BPP            16
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#endif

#ifdef CONFIG_NEC_OSPREY
// AP4 did strange things, Backlight button GPIO currently used for IOCS16,
//  AP2 used for debug board Ethernet controller, AP3 just doesn't work
#define GPIO_BTN_MAP { BTN_CONTRAST, BTN_UP, BTN_DOWN, 0, BTN_AP1, BTN_EXIT, 0, \
		0, BTN_ACTION, 0, 0, 0, 0, 0, 0, 0 }
#undef GPIO_BTN_PRESS_LOW
#endif

#ifdef CONFIG_NEC_UEB30
#define VIDEORAM_BASE     (KSEG1 + 0x0a000000)
#define FB_X_RES          640
#define FB_Y_RES          480
#define FB_BPP            8
#endif

#ifdef CONFIG_VADEM_CLIO_1000
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO42;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO42;
#define KBD_SCANLINES 8
#endif

#ifdef CONFIG_VADEM_CLIO_1050
#define KBD_SCANLINES 8
#define VIDEORAM_BASE   (KSEG1 + 0x0a200000)
#define FB_X_RES        640
#define FB_Y_RES        480
#define FB_BPP          16
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R300
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (64 * 1024)
#define FB_X_VIRTUAL_RES  1024
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         2
#define FB_IS_GREY     1
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO14;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO14;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R320
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (64 * 1024)
#define FB_X_VIRTUAL_RES  640
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         2
#define FB_IS_GREY     1
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO14;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO14;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R430
#define DEVICE_IRQ_MASKL 0xffff
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a180100)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  640
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         16
// GIUPODAT[12] is speaker power on/off bit confirmed by Checkstuff.exe
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPODATL_GPIO46;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPODATL_GPIO46;
#endif


#ifdef CONFIG_NEC_MOBILEGEAR2_R500
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x13000000)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  1024
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         8
// #define FB_IS_GRAY     1
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO14;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO14;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R510
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  1024
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         8
// GIUPODAT[12] is speaker power on/off bit confirmed by Checkstuff.exe
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO14;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO14;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R520
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  800
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         16
// GIUPODAT[12] is speaker power on/off bit confirmed by Checkstuff.exe
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO14;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO14;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R530
#define DEVICE_IRQ_MASKL 0xffff
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a180100)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  640
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         16
// GIUPODAT[12] is speaker power on/off bit confirmed by Checkstuff.exe
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPODATL_GPIO46;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPODATL_GPIO46;
#endif

#ifdef CONFIG_DOCOMO_SIGMARION
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  640
#define FB_X_RES       640
#define FB_Y_RES       240
#define FB_BPP         16
// GIUPODAT[12] is speaker power on/off bit confirmed by Checkstuff.exe
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO44; \
		restore_flags(flags);	\
	}
// GPIO[14] is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPIODL |= VR41XX_GIUPODATL_GPIO46;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPIODL &= ~VR41XX_GIUPODATL_GPIO46;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R700
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE  (KSEG1 + 0x0a000000)
#define VIDEORAM_SIZE  (640 * 240 * 8)
#define FB_X_VIRTUAL_RES  800
#define FB_X_RES       800
#define FB_Y_RES       600
#define FB_BPP         16
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()	\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
// PODAT[14](GPIO[46]) is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO46;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO46;
#endif

#ifdef CONFIG_NEC_MOBILEGEAR2_R730
// Not tested yet
#define DEVICE_IRQ_MASKH 0x0000
#define VIDEORAM_BASE	(KSEG1 + 0x0a0ea600)
#define VIDEORAM_SIZE	(800 * 600 * 16)
#define FB_X_VIRTUAL_RES	800
#define FB_X_RES		800
#define FB_Y_RES		600
#define FB_BPP			16
// GPIO[12] is speaker power on/off bit
#define VR41XX_ENABLE_SPEAKER()		\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL |= VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
#define VR41XX_DISABLE_SPEAKER()	\
	{				\
		unsigned long flags;	\
		save_and_cli(flags);	\
		*VR41XX_GIUPIODL &= ~VR41XX_GIUPIODL_GPIO12; \
		restore_flags(flags);	\
	}
// PODAT[14](GPIO[46]) is serial power on/off bit
#define VR41XX_ENABLE_SERIAL(x)		\
	*VR41XX_GIUPODATL |= VR41XX_GIUPODATL_GPIO46;
#define VR41XX_DISABLE_SERIAL(x)	\
	*VR41XX_GIUPODATL &= ~VR41XX_GIUPODATL_GPIO46;
#endif

#ifdef CONFIG_NEC_HARRIER
#define VIDEORAMBASE   (KSEG1 + 0x10400000)
#define FB_X_RES       800
#define FB_Y_RES       600
#define FB_BPP         8
#define VIDEORAM_SIZE (FB_X_RES * FB_Y_RES * FB_BPP / 8)
#define KBD_SCANLINES 8
#define GPIO_BTN_MAP { 					\
	    BTN_AP1, 0, 0, BTN_AP2,			\
	    0, 0, 0, BTN_BACKLIGHT,			\
	    0, BTN_AP3, 0, 0,				\
	    BTN_AP4, 0, 0, 0,				\
	    0, 0, 0, 0,					\
	    0, 0, 0, 0,					\
	    0, 0, 0, 0,					\
	    0, 0, 0, 0,					\
	    0, 0, 0, 0, 				\
	    0, 0, 0, 0, 				\
	    BTN_AP25, BTN_AP26, BTN_AP27, BTN_AP28,	\
	    BTN_AP29, BTN_AP30, BTN_AP31, BTN_AP32,	\
	    BTN_AP33, BTN_AP34, BTN_AP35, BTN_AP36	\
	}
#endif

// Some reasonable defaults

#ifndef DEVICE_IRQ_MASKL
#define DEVICE_IRQ_MASKL 0xffff
#endif

#ifndef DEVICE_IRQ_MASKH
#ifndef CONFIG_CPU_VR4181
#define DEVICE_IRQ_MASKH 0xffff
#endif
#endif

#ifndef ADJUSTED_PORT_BASE
#define ADJUSTED_PORT_BASE VR41XX_PORT_BASE
#endif

#ifndef VR41XX_ENABLE_SPEAKER
#define	VR41XX_ENABLE_SPEAKER()		do { } while (0)
#endif
#ifndef VR41XX_DISABLE_SPEAKER
#define	VR41XX_DISABLE_SPEAKER()	do { } while (0)
#endif

#ifndef VR41XX_ENABLE_SERIAL
#define VR41XX_ENABLE_SERIAL(x)		do { } while (0)
#endif
#ifndef VR41XX_DISABLE_SERIAL
#define VR41XX_DISABLE_SERIAL(x)	do { } while (0)
#endif

#ifndef VR41XX_ENABLE_IRDA
#define VR41XX_ENABLE_IRDA()		do { } while (0)
#endif
#ifndef VR41XX_DISABLE_IRDA
#define VR41XX_DISABLE_IRDA()		do { } while (0)
#endif

#ifndef KBD_SCANLINES
#define KBD_SCANLINES 12
#endif

#endif /* __ASM_MIPS_VR41XX_PLATDEP_H */
