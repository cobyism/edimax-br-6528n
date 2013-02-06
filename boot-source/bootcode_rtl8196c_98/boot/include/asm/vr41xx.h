/* $Id: vr41xx.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999 by Michael Klar
 */
#ifndef __ASM_MIPS_VR41XX_H 
#define __ASM_MIPS_VR41XX_H 

#ifdef __KERNEL__

#include <linux/config.h>

#ifdef CONFIG_CPU_VR4181
#include <asm/vr4181.h>
#else
#ifdef CONFIG_CPU_VR4122
#include <asm/vr4122.h>
#else
#include <asm/vr4121.h>
#endif
#endif

#endif /* __KERNEL__ */

#endif
