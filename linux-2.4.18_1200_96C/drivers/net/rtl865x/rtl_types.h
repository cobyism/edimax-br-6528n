/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Abstract : realtek type definition
 *
 * $Author: joeylin $
 *
*/
#include <linux/kernel.h>


#ifndef _RTL_TYPES_H
#define _RTL_TYPES_H

#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#include <linux/config.h>
#include <linux/module.h>
#endif
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/string.h>

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */

#if 1
#define __IRAM_FWD
#define __IRAM_GEN
#define __IRAM_EXTDEV
#define __IRAM_L2_FWD
#define __DRAM_EXTDEV
#endif

#ifdef CONFIG_RTL8196C
//#define DBG_ASIC_COUNTER			1
//#define DBG_ASIC_MULTICAST_TBL		1
#define DBG_EEE					1
//#define DBG_PHY_REG				1
#define DBG_MEMORY				1
//#define DBG_DESC				1
#define ETH_NEW_FC				1  // flow control
#endif

/* ===============================================================================
		print macro
    =============================================================================== */
#define rtlglue_printf	printk

/*
typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;
typedef int			int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;
*/
#define uint64 unsigned long long
#define int64 long long
#define uint32 unsigned int 
#define int32 int
#define uint16 unsigned short
#define int16 short
#define uint8 unsigned char
#define int8 char



#define UINT32_MAX	UINT_MAX
#define INT32_MIN	INT_MIN
#define INT32_MAX	INT_MAX
#define UINT16_MAX	USHRT_MAX
#define INT16_MIN	SHRT_MIN
#define INT16_MAX	SHRT_MAX
#define UINT8_MAX	UCHAR_MAX
#define INT8_MIN		SCHAR_MIN
#define INT8_MAX	SCHAR_MAX

typedef uint32		memaddr;	
typedef uint32          ipaddr_t;

typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
    uint16		align;
} macaddr_t;


typedef int8*			calladdr_t;

typedef struct ether_addr_s {
	uint8 octet[6];
} ether_addr_t;

#define RX_OFFSET 2
#define MBUF_LEN	1600
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+16)

#ifdef CONFIG_RTL8196B
	#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	#define DELAY_REFILL_ETH_RX_BUF
#if defined(CONFIG_RTL8196B_GW_8M)
#ifdef CONFIG_RTL8196C
	#define ETH_REFILL_THRESHOLD	100
#else
	#define ETH_REFILL_THRESHOLD	140
#endif
#else	
	#define ETH_REFILL_THRESHOLD	140 // 24	// must < NUM_RX_PKTHDR_DESC
#endif	

#if defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
#undef ETH_REFILL_THRESHOLD
#define ETH_REFILL_THRESHOLD	140 // 24	// must < NUM_RX_PKTHDR_DESC
#endif	



	#endif
#endif

/* 
	CN SD6 Mantis issue #1085: NIC RX can't work correctly after runout.
	this case still happened in RTL8196B, the designer said it is the normal behavior.
 */
#define  RTL_ETH_RX_RUNOUT

//----------------------------------------------------
//#include "rtl_depend.h"
#ifndef _RTL_DEPEND_H_
#define _RTL_DEPEND_H_


/*
 * many standard C library API references "size_t"
 * cygwin's stdio.h and ghs's stdio.h will test
 * _SIZE_T symbol before typedef ...
 */
#ifndef _SIZE_T
#define _SIZE_T
	typedef unsigned int size_t;
#endif /* _SIZE_T */

#define RTL_EXTERN_INLINE
#define RTL_STATIC_INLINE   static __inline__

#endif   /* _RTL_DEPEND_H_ */
//------
#ifdef CONFIG_WIRELESS_LAN_MODULE
#define __IRAM_SECTION_	//__attribute__ ((section(".iram-fwd")))
#define __DRAM_SECTION_	//__attribute__ ((section(".dram-fwd")))
#else
#define __IRAM_SECTION_	__attribute__ ((section(".iram-fwd")))
#define __DRAM_SECTION_	__attribute__ ((section(".dram-fwd")))
#endif

#undef __MIPS16
#define __MIPS16			__attribute__ ((mips16))

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#define CLEARBITS(a,b)	((a) &= ~(b))
#define SETBITS(a,b)		((a) |= (b))
#define ISSET(a,b)		(((a) & (b))!=0)
#define ISCLEARED(a,b)	(((a) & (b))==0)

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif			   /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif			   /* min */



//#define ASSERT_CSP(x) if (!(x)) {dprintf("\nAssertion fail at file %s, function %s, line number %d: (%s).\n", __FILE__, __FUNCTION__, __LINE__, #x); while(1);}
//#define ASSERT_ISR(x) if (!(x)) {printfByPolling("\nAssertion fail at file %s, function %s, line number %d: (%s).\n", __FILE__, __FUNCTION__, __LINE__, #x); while(1);}

//wei add, because we only use polling mode uart-print
extern void *memset(void *__s, int __c, size_t __count);

#define ASSERT_CSP(x) if (!(x)) {printk("\nAssertion fail!, file=%s, fun=%s, line=%d", __FILE__, __FUNCTION__, __LINE__); while(1);}
#define ASSERT_ISR(x) ASSERT_CSP(x)
#define bzero( p, s ) memset( p, 0, s )

//round down x to multiple of y.  Ex: ROUNDDOWN(20, 7)=14
#ifndef ROUNDDOWN
#define	ROUNDDOWN(x, y)	(((x)/(y))*(y))
#endif

//round up x to multiple of y. Ex: ROUNDUP(11, 7) = 14
#ifndef ROUNDUP
#define	ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

#ifndef ROUNDUP2
#define	ROUNDUP2(x, y)	(((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#endif

#ifndef ROUNDUP4
#define	ROUNDUP4(x)		((1+(((x)-1)>>2))<<2)
#endif

#ifndef IS4BYTEALIGNED
#define IS4BYTEALIGNED(x)	 ((((x) & 0x3)==0)? 1 : 0)
#endif

#ifndef __offsetof
#define __offsetof(type, field) ((unsigned long)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif

#ifndef RTL_PROC_CHECK
#define RTL_PROC_CHECK(expr, success) \
	do {\
			int __retval; \
			if ((__retval = (expr)) != (success))\
			{\
				rtlglue_printf("ERROR >>> [%s]:[%d] failed -- return value: %d\n", __FUNCTION__,__LINE__, __retval);\
				return __retval; \
			}\
		}while(0)
#endif

#ifndef RTL_STREAM_SAME
#define RTL_STREAM_SAME(s1, s2) \
	((strlen(s1) == strlen(s2)) && (strcmp(s1, s2) == 0))
#endif

#define RTL_STATIC_INLINE   static __inline__

#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
#define UNCACHE_MASK		0
#define UNCACHE(addr)		(addr)
#define CACHED(addr)			((uint32)(addr))
#else
#define UNCACHE_MASK		0x20000000
#define UNCACHE(addr)		((UNCACHE_MASK)|(uint32)(addr))
#define CACHED(addr)			((uint32)(addr) & ~(UNCACHE_MASK))
#endif

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

#ifdef CONFIG_RTL8196B
#define ALL_PHYSICAL_PORTS 		(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4))
#else
#define ALL_PHYSICAL_PORTS 		(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5))
#endif
#define EXT_PORT_HWLOOKUP 		BIT(6)
#define EXT_PORT_WLAN 				BIT(6)	// not support yet
#define EXT_PORT_PPTPL2TP 			BIT(7)	// not support yet
#define ALL_PORTS 	(ALL_PHYSICAL_PORTS | EXT_PORT_HWLOOKUP)

//#define TWO_VLANS_IN_BRIDGE_MODE
#define CPU_PORT 		BIT(6)

#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
#define PASSTHRU_VLAN_ID 100

#define IP6_PASSTHRU_RULEID 5
#define PPPOE_PASSTHRU_RULEID1 3
#define PPPOE_PASSTHRU_RULEID2 4

#define IP6_PASSTHRU_MASK 0x1
#define PPPOE_PASSTHRU_MASK 0x1<<1

extern int oldStatus;
#endif

// CONFIG_POCKET_ROUTER_SUPPORT
#define PORT_HW_AP_MODE		BIT(4)

#ifdef CONFIG_RTL8196C
#define CONFIG_RTL8196C_ETH_IOT		1
#define CONFIG_RTL_8196C_ESD_NEW		1

#ifdef CONFIG_RTL_8196C_ESD_NEW
#define CONFIG_RTK_CHECK_ETH_TX_HANG		1
#endif

#else	// RTL8196B
#define CONFIG_RTK_CHECK_ETH_TX_HANG		1
#endif

#define V4_MULTICAST_MAC(mac) 	   ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define V6_MULTICAST_MAC(mac) 	   ((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]!=0xff))

#define V4_IGMP_PROTO(mac)			((mac[12]==0x08) && (mac[13]==0x00) && (mac[23]==0x02))
#define V6_ICMPV6_PROTO(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[54]==0x3a || mac[20]==0x3a))
#define SSDP_PROTO(mac)			((mac[36]==0x07) && (mac[37]==0x6c))

#endif
