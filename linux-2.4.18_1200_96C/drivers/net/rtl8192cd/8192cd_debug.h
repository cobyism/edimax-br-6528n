/*
 *  Debug headler file. It defines various print out method
 *
 *  $Id: 8192cd_debug.h,v 1.2.4.2 2010/12/01 13:38:00 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_DEBUG_H_
#define _8192CD_DEBUG_H_

#if 1
#define	_MESH_MOD_	//plus add 0119
#define printMac(da)	printk("%02X:%02X:%02X:%02X:%02X:%02X   ",  0xff&*(da), 0xff&*(da+1), 0xff&*(da+2), 0xff&*(da+3), 0xff&*(da+4), 0xff&*(da+5));

#define printMac4(pframe) 		{\
		 printMac(GetAddr1Ptr(pframe));\
		 printMac(GetAddr2Ptr(pframe));\
		 printMac(GetAddr3Ptr(pframe));\
		 printMac(GetAddr4Ptr(pframe));\
}


#define printHex(d,n)		{int i; \
	for(i=0; i<n; i++) 	{  printk("%02X:", *(d+i)); \
		if( i%40==39) printk("\n "); \
	} }
	
#endif

#ifdef _DEBUG_RTL8192CD_

extern unsigned long rtl8192cd_debug_err;		/* err flag */
extern unsigned long rtl8192cd_debug_info;	/* info flag */
extern unsigned long rtl8192cd_debug_trace;	/* trace flag */
extern unsigned long rtl8192cd_debug_warn;	/* warn flag */

/* Bit definition for bit31-bit8 of rtl8190_debug */
enum _module_define_ {
	_OSDEP_ =		0x00000001,
	_SME_ =			0x00000002,
	_IOCTL_ =		0x00000004,
	_TX_ =			0x00000008,
	_RX_ =			0x00000010,
	_HW_ =			0x00000020,
	_SECURITY_ =	0x00000040,
	_UTIL_ =		0x00000080,
	_TKIP_ =		0x00000100,
	_AES_ =			0x00000200,
	_CAM_ =			0x00000400,
	_BR_EXT_ =		0x00000800,
	_EEPROM_ =		0x00001000,
	_PSK_ =			0x00002000,
	_MP_ =			0x00004000,
	_MIB_=			0x00008000,
	_MESH_=			0x00010000,	//plus add 0119
	_WPS_ =			0x00020000,	
};

#if defined(_8192CD_OSDEP_C_)
	#define _MODULE_DEFINE	_OSDEP_
	#define _MODULE_NAME	"osdep"

#elif defined(_8192CD_SME_C_)
	#define _MODULE_DEFINE _SME_
	#define _MODULE_NAME	"sme"

#elif defined(_8192CD_IOCTL_C_)
	#define _MODULE_DEFINE _IOCTL_
	#define _MODULE_NAME	"ioctl"

#elif defined(_8192CD_TX_C_)
	#define _MODULE_DEFINE _TX_
	#define _MODULE_NAME	"tx"

#elif defined(_8192CD_RX_C_)
	#define _MODULE_DEFINE _RX_
	#define _MODULE_NAME	"rx"

#elif defined(_8192CD_HW_C_)
	#define _MODULE_DEFINE _HW_
	#define _MODULE_NAME	"hw"

#elif defined(_8192CD_SECURITY_C_)
	#define _MODULE_DEFINE _SECURITY_
	#define _MODULE_NAME	"security"

#elif defined(_8192CD_UTILS_C_)
	#define _MODULE_DEFINE _UTIL_
	#define _MODULE_NAME	"util"

#elif defined(_8192CD_TKIP_C_)
	#define _MODULE_DEFINE _TKIP_
	#define _MODULE_NAME	"tkip"

#elif defined(_8192CD_AES_C_)
	#define _MODULE_DEFINE _AES_
	#define _MODULE_NAME	"aes"

#elif defined(_8190N_CAM_C_)
	#define _MODULE_DEFINE _CAM_
	#define _MODULE_NAME	"cam"

#elif defined(_8192CD_BR_EXT_C_)
	#define _MODULE_DEFINE _BR_EXT_
	#define _MODULE_NAME	"br_ext"

#elif defined(_8192CD_EEPROM_C_)
	#define _MODULE_DEFINE _EEPROM_
	#define _MODULE_NAME	"eeprom"

#elif defined(_8192CD_PSK_C_)
	#define _MODULE_DEFINE _PSK_
	#define _MODULE_NAME	"psk"

#elif defined(_8192CD_MP_C_)
	#define _MODULE_DEFINE _MP_
	#define _MODULE_NAME	"mp"

#elif defined(_8192CD_MIB_C_)
	#define _MODULE_DEFINE _MIB_
	#define _MODULE_NAME	"mib"

#elif defined(_MESH_MOD_)	//plus add 0119
	#define _MODULE_DEFINE _MESH_
	#define _MODULE_NAME	"mesh"

#elif defined(_8192CD_A4_STA_C_)
	#define _MODULE_DEFINE _MIB_
	#define _MODULE_NAME	"a4_sta"

#elif defined(_8192CD_WSCD_C_)
	#define _MODULE_DEFINE _WPS_
	#define _MODULE_NAME	"wps"

#else
	#error "error, no debug module is specified!\n"
#endif

/* Macro for DEBUG_ERR(), DEBUG_TRACE(), DEBUG_WARN(), DEBUG_INFO() */

#ifdef __GNUC__
#ifdef CONFIG_RTL8671
#define DEBUG_ERR		printk
#define DEBUG_TRACE		printk
#define DEBUG_INFO		printk
#define DEBUG_WARN		printk

#define _DEBUG_ERR		printk
#define _DEBUG_INFO		printk

#define DBFENTER
#define DBFEXIT
#define PRINT_INFO		printk

#else

#define __DEBUG_ERR(name, fmt, args...) \
	if (rtl8192cd_debug_err&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-err: " fmt, name, ## args);
#define __DEBUG_TRACE(name) \
	if (rtl8192cd_debug_trace&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-trace: %s----->\n", name, (char *)__FUNCTION__);
#define __DEBUG_INFO(name, fmt, args...) \
	if (rtl8192cd_debug_info&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-info: " fmt, name, ## args);
#define __DEBUG_WARN(name, fmt, args...) \
	if (rtl8192cd_debug_warn&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-warn: " fmt, name, ## args);

#define DEBUG_ERR(fmt, args...)		__DEBUG_ERR(priv->dev->name, fmt, ## args)
#define DEBUG_INFO(fmt, args...)	__DEBUG_INFO(priv->dev->name, fmt, ## args)
#define DEBUG_TRACE					__DEBUG_TRACE(priv->dev->name)
#define DEBUG_WARN(fmt, args...)	__DEBUG_WARN(priv->dev->name, fmt, ## args)

#define _DEBUG_ERR(fmt, args...)	__DEBUG_ERR("wlan", fmt, ## args)
#define _DEBUG_INFO(fmt, args...)	__DEBUG_INFO("wlan", fmt, ## args)
#define _DEBUG_TRACE				__DEBUG_TRACE("wlan")
#define _DEBUG_WARN(fmt, args...)	__DEBUG_WARN("wlan", fmt, ## args)

#define DBFENTER	printk("----->%s\n", (char *)__FUNCTION__)
#define DBFEXIT		printk("%s----->\n", (char *)__FUNCTION__)
#define PRINT_INFO(fmt, args...)	printk(fmt, ## args)
#endif
#endif	// __GNUC__

/*
#ifdef __DRAYTEK_OS__
#define __FUNCTION__	""

#define DEBUG_ERR		Print
#define DEBUG_INFO		Print
#define DEBUG_TRACE
#define DEBUG_WARN		Print

#define _DEBUG_ERR		DEBUG_ERR
#define _DEBUG_INFO		DEBUG_INFO
#define _DEBUG_TRACE	DEBUG_TRACE
#define _DEBUG_WARN		DEBUG_WARN

#define DBFENTER
#define DBFEXIT
#define PRINT_INFO		Print
#endif // __DRAYTEK_OS__

#ifdef GREEN_HILL
#define DEBUG_ERR		printk
#define DEBUG_INFO		printk
#define DEBUG_TRACE		printk
#define DEBUG_WARN		printk

#define _DEBUG_ERR		printk
#define _DEBUG_INFO		printk
#define _DEBUG_TRACE	printk
#define _DEBUG_WARN		printk

#define DBFENTER		printk
#define DBFEXIT			printk
#define PRINT_INFO		printk
#endif // GREEN_HILL
*/


#else // not _DEBUG_RTL8192CD_


#ifdef __GNUC__
#define DEBUG_ERR(fmt, args...) {}
#define DEBUG_INFO(fmt, args...) {}
#define DEBUG_TRACE {}
#define DEBUG_WARN(fmt, args...) {}

#define _DEBUG_ERR(fmt, args...) {}
#define _DEBUG_INFO(fmt, args...) {}
#define _DEBUG_TRACE {}
#define _DEBUG_WARN(fmt, args...) {}

#define DBFENTER	{}
#define DBFEXIT		{}
#define PRINT_INFO(fmt, args...)	{}
#endif // __GNUC__

/*
#ifdef __DRAYTEK_OS__
#define __FUNCTION__	""

#define DEBUG_ERR
#define DEBUG_INFO
#define DEBUG_TRACE
#define DEBUG_WARN

#define _DEBUG_ERR
#define _DEBUG_INFO
#define _DEBUG_TRACE
#define _DEBUG_WARN

#define DBFENTER
#define DBFEXIT
#define PRINT_INFO
#endif // __DRAYTEK_OS__

#ifdef GREEN_HILL
#define DEBUG_ERR(fmt, args...) {}
#define DEBUG_INFO(fmt, args...) {}
#define DEBUG_TRACE {}
#define DEBUG_WARN(fmt, args...) {}

#define _DEBUG_ERR(fmt, args...) {}
#define _DEBUG_INFO(fmt, args...) {}
#define _DEBUG_TRACE {}
#define _DEBUG_WARN(fmt, args...) {}

#define DBFENTER	{}
#define DBFEXIT		{}
#define PRINT_INFO(fmt, args...)	{}
#endif // GREEN_HILL
*/


#endif // _DEBUG_RTL8192CD_
#endif // _8192CD_DEBUG_H_

