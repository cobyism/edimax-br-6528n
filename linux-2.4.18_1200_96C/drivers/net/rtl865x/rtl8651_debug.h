/*
* Copyright c                  Realtek Semiconductor Corporation, 2005
* All rights reserved.
* 
* Program : Rome Driver header file rtl8651_debug.h to store all debugging related MACROs / UTILITIEs
* Abstract : 
* Creator : Yi-Lun Chen (chenyl@realtek.com.tw)
* Author :  
*
*/

#ifndef RTL8651_DEBUG_H
#define RTL8651_DEBUG_H

/* =====================================================
	Globally controlling definitions
   ===================================================== */
#define RTL_DEBUG_MSG		/* DEFINED: enable debugging message */

#ifdef RTL_DEBUG_MSG


/************************************************************************
 *  Two-dimension for debug information.
 *  Through the following two dimension, severity and module, we can con-
 *  trol the code size of debug information at compile time.
 *
 *  \   Severity|           RTL_MSG_LEVEL_MASK
 *   \________  |
 *   Module   \ | DEBUG | INFO | NOTICE | WARN | ERR
 *  ___________\|_______|______|________|______|______
 *              |       |      |        |      |
 *      SESSION |       |      |        |      |        
 *   T  --------+-------+------+--------+------+------  
 *   Y  POLICYRT|       |      |        |      |        
 *   P  --------+-------+------+--------+------+------  
 *   E  MCAST   |       |      |        |      |        
 *      --------+-------+------+--------+------+------  
 *   M  FWD     |       |      |        |      |        
 *   A  --------+-------+------+--------+------+------  
 *   S  EXTDEV  |       |      |        |      |        
 *   K  --------+-------+------+--------+------+------  
 *      NAPT    |       |      |        |      |        
 *
 *  The table follows 'AND' operation: only the debug code with both
 *  level mask and type mask are 1 will be compiled into runtime image.
 *
 ************************************************************************/

/************************************************************************
 *  Definitions for severity.
 *
 *  'RTL_MSG_MASK' controls the level of serverity. More severity level
 *  makes larger code size.
 ************************************************************************/
#ifdef RTL865X_TEST
#define RTL_MSG_MASK			0xfffffef8
#else
#define RTL_MSG_MASK			0xfffffef8
#endif
/* by level of importance */
#define RTL_MSG_LEVEL_MASK		0x000000ff
#define RTL_MSG_DEBUG			(1<<0)
#define RTL_MSG_INFO			(1<<1)
#define RTL_MSG_NOTICE			(1<<2)
#define RTL_MSG_WARN			(1<<3)
#define RTL_MSG_ERR				(1<<4)
#define RTL_MSG_CRIT			(1<<5)
#define RTL_MSG_ALERT			(1<<6)
#define RTL_MSG_EMERG			(1<<7)
/* by functionality */
#define RTL_MSG_FUNC_MASK		0x0000ff00
#define RTL_MSG_TRACE			(1<<8)


/************************************************************************
 *  Definitions for ROME Driver Modules
 *
 *  Please add your module below.
 *
 ************************************************************************/
#define RTL_MSG_SESSION_HANGUP		(1<<0)
#define RTL_MSG_POLICYRT				(1<<1)
#define RTL_MSG_MCAST					(1<<2)
#define RTL_MSG_FWD					(1<<3)
#define RTL_MSG_EXTDEV					(1<<4)
#define RTL_MSG_NAPT					(1<<5)
#define RTL_MSG_PORTBOUNCING			(1<<6)
#define RTL_MSG_UPNP					(1<<7)
#define RTL_MSG_PPPOE					(1<<8)
#define RTL_MSG_TUNNEL					(1<<9)				/* For Tunnel Connection: PPTP or L2TP */
#define RTL_MSG_FLOWCACHE				(1<<10)
#define RTL_MSG_GENERIC				(1<<11)
#define RTL_MSG_FWDFEATURE			(1<<12)
#define RTL_MSG_PROTOCOLSTACK			(1<<13)
#define RTL_MSG_TBLDRV_L2				(1<<14)
#define RTL_MSG_TYPE_MASK				0xffffffff

#endif	/* RTL_DEBUG_MSG */

#if (RTL_MSG_MASK & RTL_MSG_DEBUG)
#define RTL_DEBUG(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("(%x)[%s-%d]-debug-: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_DEBUG(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_INFO)
#define RTL_INFO(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("(%x)[%s-%d]-info-: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_INFO(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_NOTICE)
#define RTL_NOTICE(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("(%x)[%s-%d]-notice-: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_NOTICE(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_WARN)
#define RTL_WARN(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("(%x)[%s-%d]-warning-: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_WARN(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_ERR)
#define RTL_ERR(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("[=!!Error !!=] =>\n\t(%x)[%s-%d]: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_ERR(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_CRIT)
#define RTL_CRIT(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("[=!!Critical !!=] =>\n\t(%x)[%s-%d]: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_CRIT(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_ALERT)
#define RTL_ALERT(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("[=!!Alert !!=] =>\n\t(%x)[%s-%d]: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_ALERT(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_EMERG)
#define RTL_EMERG(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("[=!!Emergency !!=] =>\n\t(%x)[%s-%d]: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_EMERG(type, fmt, args...) do {} while(0)
#endif

#if (RTL_MSG_MASK & RTL_MSG_TRACE)
#define RTL_TRACE_IN(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("=TRACE IN=(%x)[%s-%d]: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#define RTL_TRACE_OUT(type, fmt, args...) \
	do {if (type & RTL_MSG_TYPE_MASK) rtlglue_printf("=TRACE OUT=(%x)[%s-%d]: " fmt "\n", type, __FUNCTION__, __LINE__, ## args);} while (0)
#else
#define RTL_TRACE_IN(type, fmt, args...) do {} while(0)
#define RTL_TRACE_OUT(type, fmt, args...) do {} while(0)
#endif

#define RTL_BUG(cause) \
	do {rtlglue_printf(" [= !! BUG !! =] at %s line %d\n\t=> Cause: %s\n\t=>-- system Halt\n", __FUNCTION__, __LINE__, cause); while(1);} while (0)

#endif /* RTL8651_DEBUG_H */


