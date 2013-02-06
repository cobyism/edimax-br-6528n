/* $Id: betty.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * Definitions for "Betty" companion chip (audio/telecom/touch panel)
 * for the r39xx processors.  Known as the Toshiba TC35143F or Philips UCB1200.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef __BETTY_H__
#define __BETTY_H__


#define BIT(x)	(1 << x)


/* Registers */

#define BETTY_REG_IO_PORT_DATA                 0
#define BETTY_REG_IO_PORT_DIRECTION            1
#define BETTY_REG_RISING_INT_ENABLE            2
#define BETTY_REG_FALLING_INT_ENABLE           3
#define BETTY_REG_INT_CLEAR_STATUS             4
#define BETTY_REG_TEL_CTRL_A                   5
#define BETTY_REG_TEL_CTRL_B                   6
#define BETTY_REG_AUD_CTRL_A                   7
#define BETTY_REG_AUD_CTRL_B                   8
#define BETTY_REG_TOUCHSCREEN_CTRL             9
#define BETTY_REG_ADC_CTRL                     10
#define BETTY_REG_ADC_DATA                     11
#define BETTY_REG_ID                           12
#define BETTY_REG_MODE                         13

/* 0 - I/O port data register */

#define BETTY_IO_DATA_MASK            0x03ff
#define BETTY_IO_DATA_SHIFT           0

/* 1 - I/O port direction register */

#define BETTY_IO_DIR_MASK             0x03ff
#define BETTY_IO_DIR_SHIFT            0
#define BETTY_SIB_ZERO                BIT(15)

/* 2 - Rising edge interrupt enable register */

#define BETTY_IO_RIS_INT_MASK         0x03ff
#define BETTY_IO_RIS_INT_SHIFT        0
#define BETTY_ADC_RIS_INT             BIT(11)
#define BETTY_TSPX_RIS_INT            BIT(12)
#define BETTY_TSMX_RIS_INT            BIT(13)
#define BETTY_TCLIP_RIS_INT           BIT(14)
#define BETTY_ACLIP_RIS_INT           BIT(15)

/* 3 - Falling edge interrupt enable register */

#define BETTY_IO_FAL_INT_MASK         0x03ff
#define BETTY_IO_FAL_INT_SHIFT        0
#define BETTY_ADC_FAL_INT             BIT(11)
#define BETTY_TSPX_FAL_INT            BIT(12)
#define BETTY_TSMX_FAL_INT            BIT(13)
#define BETTY_TCLIP_FAL_INT           BIT(14)
#define BETTY_ACLIP_FAL_INT           BIT(15)

/* 4 - Interrupt clear/status register */

#define BETTY_IO_INT_MASK             0x03ff
#define BETTY_IO_INT_SHIFT            0
#define BETTY_ADC_INT                 BIT(11)
#define BETTY_TSPX_INT                BIT(12)
#define BETTY_TSMX_INT                BIT(13)
#define BETTY_TCLIP_INT               BIT(14)
#define BETTY_ACLIP_INT               BIT(15)

/* 5 - Telecom control register A */

#define BETTY_TEL_DIV_MASK            0x007f
#define BETTY_TEL_DIV_SHIFT           0
#define BETTY_TEL_LOOP                BIT(7)

/* 6 - Telecom control register B */

#define BETTY_TEL_VOICE_ENA           BIT(3)
#define BETTY_TEL_CLIP_DETECT         BIT(4)
#define BETTY_TEL_ATT                 BIT(6)
#define BETTY_TEL_SIDE_ENA            BIT(11)
#define BETTY_TEL_MUTE                BIT(13)
#define BETTY_TEL_IN_ENA              BIT(14)
#define BETTY_TEL_OUT_ENA             BIT(15)

/* 7 - Audio control register A */

#define BETTY_AUD_DIV_MASK            0x007f
#define BETTY_AUD_DIV_SHIFT           0
#define BETTY_AUD_GAIN_MASK           0x0f80
#define BETTY_AUD_GAIN_SHIFT          7

/* 8 - Audio control register B */

#define BETTY_AUD_ATT_MASK            0x001f
#define BETTY_AUD_ATT_SHIFT           0
#define BETTY_AUD_CLIP_DETECT         BIT(6)
#define BETTY_AUD_LOOP                BIT(8)
#define BETTY_AUD_MUTE                BIT(13)
#define BETTY_AUD_IN_ENA              BIT(14)
#define BETTY_AUD_OUT_ENA             BIT(15)

/* 9 - Touch screen control register */

#define BETTY_TSMX_POW                BIT(0)
#define BETTY_TSPX_POW                BIT(1)
#define BETTY_TSMY_POW                BIT(2)
#define BETTY_TSPY_POW                BIT(3)
#define BETTY_TSMX_GND                BIT(4)
#define BETTY_TSPX_GND                BIT(5)
#define BETTY_TSMY_GND                BIT(6)
#define BETTY_TSPY_GND                BIT(7)
#define BETTY_TSC_MODE_MASK           (BIT(8) | BIT(9))
#define BETTY_TSC_MODE_SHIFT          8
#define BETTY_TSC_BIAS_ENA            BIT(11)
#define BETTY_TSPX_LOW                BIT(12)
#define BETTY_TSMX_LOW                BIT(13)

/* 10 - ADC control register */

#define BETTY_ADC_SYNC_ENA            BIT(0)
#define BETTY_VREFBYP_CON             BIT(1)
#define BETTY_ADC_INPUT_MASK          (BIT(2) | BIT(3) | BIT(4))
#define BETTY_ADC_INPUT_SHIFT         2
#define BETTY_ADC_INPUT_TSPX          0
#define BETTY_ADC_INPUT_TSMX          1
#define BETTY_ADC_INPUT_TSPY          2
#define BETTY_ADC_INPUT_TSMY          3
#define BETTY_ADC_INPUT_AD0           4
#define BETTY_ADC_INPUT_AD1           5
#define BETTY_ADC_INPUT_AD2           6
#define BETTY_ADC_INPUT_AD3           7
#define BETTY_EXT_REF_ENA             BIT(5)
#define BETTY_ADC_START               BIT(7)
#define BETTY_ADC_ENA                 BIT(15)

/* 11 - ADC data register */

#define BETTY_ADC_DATA_MASK           0x7fe0
#define BETTY_ADC_DATA_SHIFT          5
#define BETTY_ADC_DAT_VAL             BIT(15)

/* 12 - ID register */

#define BETTY_VERSION_MASK            0x003f
#define BETTY_VERSION_SHIFT           0
#define BETTY_DEVICE_MASK             0x0fc0
#define BETTY_DEVICE_SHIFT            6
#define BETTY_SUPPLIER_MASK           0xf000
#define BETTY_SUPPLIER_SHIFT          12

/* 13 - Mode register */

#define BETTY_AUD_TEST                BIT(0)
#define BETTY_TEL_TEST                BIT(1)
#define BETTY_PROD_TEST_MODE          (BIT(2) | BIT(3) | BIT(4))
#define BETTY_DYN_VFLAG_ENA           BIT(12)
#define BETTY_AUD_OFF_CAN             BIT(13)

/* 14 - Reserved */

/* 15 - Null Register - Returns 0xffff */

#endif __BETTY_H__
