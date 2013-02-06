/* hfload.h
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License.  See the file "COPYING" in the main
 * directory of this archive for more details.
 *
 * Copyright (C) 2000, Jay Carlson
 */

#ifndef HELLOAD_H
#define HELLOAD_H

//sc_yang
#include <linux/config.h>
#ifdef EMBEDDED
#include <linux/string.h>
#define printf prom_printf
#else
#include <string.h>
#endif


#ifdef LANGUAGE_C
extern int file_offset;
#endif


#ifdef COMPRESSED_KERNEL
	#define UNCOMPRESS_OUT  0x80000000
#if 0
#ifndef boot32 
	#define FREEMEM_START	0x80680000
	#define FREEMEM_END	0x80800000
#else
	#define FREEMEM_START	0x80a00000
	#define FREEMEM_END	0x81000000
#endif
#endif //sc_yang
//Brad comment, since rtl865x platform use 16M && define CONFIG_RTL_GW_8M
//#if (defined(CONFIG_RTL8186_AP) || defined(CONFIG_RTL8186_GW_8M) || defined(CONFIG_RTL8186_KB))
#if (defined(CONFIG_RTL8186_AP) || defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL865X_PANAHOST))
	#define FREEMEM_END	0x80800000
#else
	#define FREEMEM_END	0x81000000
#endif
#endif

#ifdef CONFIG_RTK_VOIP
	#undef FREEMEM_END
	#define FREEMEM_END	0x81000000
#endif

// david -----------------
#ifdef BZ2_COMPRESS
	#define prom_printf
	#undef FREEMEM_START
	#define FREEMEM_START   0x80500000
#endif
//-----------------------   

#endif

