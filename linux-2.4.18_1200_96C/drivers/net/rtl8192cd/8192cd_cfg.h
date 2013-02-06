/*
 *	 Headler file defines some configure options and basic types
 *
 *	 $Id: 8192cd_cfg.h,v 1.59.2.31 2011/05/18 03:56:55 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_CFG_H_
#define _8192CD_CFG_H_

#ifdef __MIPSEB__
	#define _BIG_ENDIAN_
#else
	#define _LITTLE_ENDIAN_
#endif

#ifdef __KERNEL__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= 0x020616 // linux 2.6.22
	#define LINUX_2_6_22_
#endif

#if LINUX_VERSION_CODE >= 0x020618 // linux 2.6.24
	#define LINUX_2_6_24_
#endif

#if LINUX_VERSION_CODE >= 0x02061B // linux 2.6.27
	#define LINUX_2_6_27_
#endif

#if LINUX_VERSION_CODE > 0x020600
	#define __LINUX_2_6__
#endif

#ifdef LINUX_2_6_22_
#include <linux/autoconf.h>
#include <linux/jiffies.h>
#include <asm/param.h>
#else
#include <linux/config.h>
#endif
#endif // __KERNEL__

#ifdef CONFIG_RTK_MESH
#include "mesh_ext/mesh_cfg.h"
#endif

#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
	#if defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196B_GW) || defined(CONFIG_RTL_8196C_GW) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196B_TLD) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_AP_HCM) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY) || defined(CONFIG_RTL_8198_NFBI_BOARD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
		#define USE_RTL8186_SDK
	#endif
#endif

#if defined(_BIG_ENDIAN_) || defined(_LITTLE_ENDIAN_)
#elif defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
	#error "One one ENDIAN should be specified\n"
#else
	#error "Please specify your ENDIAN type!\n"
#endif

#define PCI_CONFIG_COMMAND			(wdev->conf_addr+4)
#define PCI_CONFIG_LATENCY			(wdev->conf_addr+0x0c)
#define PCI_CONFIG_BASE0			(wdev->conf_addr+0x10)
#define PCI_CONFIG_BASE1			(wdev->conf_addr+0x18)


//-------------------------------------------------------------
// Driver version information
//-------------------------------------------------------------
#define DRV_VERSION_H	1
#define DRV_VERSION_L	4
#define DRV_RELDATE		"2011-02-09"
#define DRV_NAME		"RTL8192C/RTL8188C"


//-------------------------------------------------------------
// Will check type for endian issue when access IO and memory
//-------------------------------------------------------------
#define CHECK_SWAP


//-------------------------------------------------------------
// Defined when include proc file system
//-------------------------------------------------------------
#define INCLUDE_PROC_FS
#if defined(CONFIG_PROC_FS) && defined(INCLUDE_PROC_FS)
	#define _INCLUDE_PROC_FS_
#endif


//-------------------------------------------------------------
// Debug function
//-------------------------------------------------------------
//#define _DEBUG_RTL8192CD_		// defined when debug print is used
#define _IOCTL_DEBUG_CMD_		// defined when read/write register/memory command is used in ioctl


//-------------------------------------------------------------
// Defined when internal DRAM is used for sw encryption/decryption
//-------------------------------------------------------------
#ifdef __MIPSEB__
	// disable internal ram for nat speedup
	//#define _USE_DRAM_
#endif


//-------------------------------------------------------------
// Support 8188C/8192C test chip
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M)
	#define TESTCHIP_SUPPORT
#endif


//-------------------------------------------------------------
// Support Tx Report
//-------------------------------------------------------------
#define TXREPORT
#ifdef TXREPORT
#define DETECT_STA_EXISTANCE
#endif

//-------------------------------------------------------------
// PCIe power saving function
//-------------------------------------------------------------
#ifndef CONFIG_RTL_8198
#ifdef CONFIG_PCIE_POWER_SAVING
#if	!defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_NET_PCI)
#define PCIE_POWER_SAVING
#endif
#endif
#endif
#ifdef _SINUX_
#define PCIE_POWER_SAVING
#endif

#ifdef PCIE_POWER_SAVING
	#define GPIO_WAKEPIN
	#define FIB_96C
//	#define ASPM_ENABLE	
//	#define PCIE_POWER_SAVING_DEBUG
#ifdef PCIE_POWER_SAVING_DEBUG
	#define PCIE_L2_ENABLE
#endif
#endif


//-------------------------------------------------------------
// WDS function support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_WDS_SUPPORT)
#define WDS
//	#define LAZY_WDS
#endif


//-------------------------------------------------------------
// Pass EAP packet by event queue
//-------------------------------------------------------------
#define EAP_BY_QUEUE
#undef EAPOLSTART_BY_QUEUE	// jimmylin: don't pass eapol-start up
							// due to XP compatibility issue
//#define USE_CHAR_DEV
#define USE_PID_NOTIFY


//-------------------------------------------------------------
// Client mode function support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_CLIENT_MODE_SUPPORT)
#define CLIENT_MODE
#endif

#ifdef CLIENT_MODE
	#define RTK_BR_EXT		// Enable NAT2.5 and MAC clone support
	#define CL_IPV6_PASS	// Enable IPV6 pass-through. RTK_BR_EXT must be defined
#endif


//-------------------------------------------------------------
// Defined when WPA2 is used
//-------------------------------------------------------------
#define RTL_WPA2
#define RTL_WPA2_PREAUTH


//-------------------------------------------------------------
// MP test
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196B_GW_MP)
	#define MP_TEST
#endif


//-------------------------------------------------------------
// MIC error test
//-------------------------------------------------------------
//#define MICERR_TEST


//-------------------------------------------------------------
// Log event
//-------------------------------------------------------------
#define EVENT_LOG


//-------------------------------------------------------------
// Tx/Rx data path shortcut
//-------------------------------------------------------------
#define TX_SHORTCUT
#define RX_SHORTCUT
#if defined(CONFIG_RTK_MESH) && defined(RX_SHORTCUT)
#define RX_RL_SHORTCUT
#endif
#define BR_SHORTCUT
#if defined(CONFIG_RTK_MESH) && defined(TX_SHORTCUT)
	#define MESH_TX_SHORTCUT
#endif

#define TX_SC_ENTRY_NUM		1


//-------------------------------------------------------------
// back to back test
//-------------------------------------------------------------
//#define B2B_TEST


//-------------------------------------------------------------
// enable e-fuse read write
//-------------------------------------------------------------
#ifdef CONFIG_ENABLE_EFUSE
	#define EN_EFUSE
#endif

//-------------------------------------------------------------
// new Auto channel
//-------------------------------------------------------------
#define CONFIG_RTL_NEW_AUTOCH

//-------------------------------------------------------------
// new IQ calibration for 92c / 88c
//-------------------------------------------------------------
#define CONFIG_RTL_NEW_IQK

//-------------------------------------------------------------
// noise control
//-------------------------------------------------------------
//#ifdef CONFIG_RTL_92C_SUPPORT
//#define CONFIG_RTL_NOISE_CONTROL_92C
//#endif

//-------------------------------------------------------------
// Universal Repeater (support AP + Infra client concurrently)
//-------------------------------------------------------------
#if defined(CONFIG_RTL_REPEATER_MODE_SUPPORT)
	#define UNIVERSAL_REPEATER
#endif


//-------------------------------------------------------------
// Check hangup for Tx queue
//-------------------------------------------------------------
#define CHECK_HANGUP
#ifdef CHECK_HANGUP
	#define CHECK_TX_HANGUP
	#define FAST_RECOVERY
#endif


//-------------------------------------------------------------
// DFS
//-------------------------------------------------------------
//#define DFS


//-------------------------------------------------------------
// Driver based WPA PSK feature
//-------------------------------------------------------------
#define INCLUDE_WPA_PSK


//-------------------------------------------------------------
// RF Fine Tune
//-------------------------------------------------------------
//#define RF_FINETUNE


//-------------------------------------------------------------
// Wifi WMM
//-------------------------------------------------------------
#define WIFI_WMM
#ifdef WIFI_WMM
	#define WMM_APSD			// WMM Power Save
	#define RTL_MANUAL_EDCA		// manual EDCA parameter setting
#endif


//-------------------------------------------------------------
// Hostapd 
//-------------------------------------------------------------
//#ifdef CONFIG_RTL_HOSTAPD_SUPPORT 
//#define WIFI_HAPD
//#endif

#ifdef WIFI_HAPD

//#define HAPD_DRV_PSK_WPS

#ifndef CONFIG_RTL_COMAPI_WLTOOLS
#define CONFIG_RTL_COMAPI_WLTOOLS
#endif

#ifndef HAPD_DRV_PSK_WPS
#undef EAP_BY_QUEUE
#undef INCLUDE_WPA_PSK
#endif

#endif


//-------------------------------------------------------------
// IO mapping access
//-------------------------------------------------------------
//#define IO_MAPPING


//-------------------------------------------------------------
// Wifi Simple Config support
//-------------------------------------------------------------
#define WIFI_SIMPLE_CONFIG


//-------------------------------------------------------------
// Support Multiple BSSID
//-------------------------------------------------------------
#if defined(CONFIG_RTL_VAP_SUPPORT)
#define MBSSID
#endif
#ifdef MBSSID
#define RTL8192CD_NUM_VWLAN  4
//#define FW_SW_BEACON
#else
#ifdef CONFIG_RTL8671
#define RTL8192CD_NUM_VWLAN  0
#endif
#endif


//-------------------------------------------------------------
// Support Tx Descriptor Reservation for each interface
//-------------------------------------------------------------
#ifdef CONFIG_RTL_TX_RESERVE_DESC
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
//#define RESERVE_TXDESC_FOR_EACH_IF
#endif
#endif


//-------------------------------------------------------------
// Group BandWidth Control
//-------------------------------------------------------------
//#define GBWC


//-------------------------------------------------------------
// Support 802.11 SNMP MIB
//-------------------------------------------------------------
//#define SUPPORT_SNMP_MIB


//-------------------------------------------------------------
// Driver-MAC loopback
//-------------------------------------------------------------
//#define DRVMAC_LB


//-------------------------------------------------------------
// Use perfomance profiling
//-------------------------------------------------------------
//#define PERF_DUMP


//-------------------------------------------------------------
// 1x1 Antenna Diversity
//-------------------------------------------------------------
#ifdef CONFIG_ANT_SWITCH
//#define SW_ANT_SWITCH
#define HW_ANT_SWITCH
#endif

//-------------------------------------------------------------
// WPAI performance issue
//-------------------------------------------------------------
#ifdef CONFIG_RTL_WAPI_SUPPORT
	//#define IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
#endif


//-------------------------------------------------------------
// Use local ring for pre-alloc Rx buffer.
// If no defined, will use kernel skb que
//-------------------------------------------------------------
#define RTK_QUE


//-------------------------------------------------------------
//Support IP multicast->unicast
//-------------------------------------------------------------
#define SUPPORT_TX_MCAST2UNI

#ifdef CLIENT_MODE
	#define SUPPORT_RX_UNI2MCAST
#endif

/* for cameo feature*/
#ifdef CONFIG_RTL865X_CMO
	#define IGMP_FILTER_CMO
#endif

// Support  IPV6 multicast->unicast
#ifdef	SUPPORT_TX_MCAST2UNI
	#define	TX_SUPPORT_IPV6_MCAST2UNI
#endif


//-------------------------------------------------------------
// Support  USB tx rate adaptive
//-------------------------------------------------------------
// define it always for object code release
#ifdef CONFIG_USB
	#define USB_PKT_RATE_CTRL_SUPPORT
#endif


//-------------------------------------------------------------
// Support Tx AMSDU
//-------------------------------------------------------------
//#define SUPPORT_TX_AMSDU


//-------------------------------------------------------------
// Mesh Network
//-------------------------------------------------------------
#ifdef CONFIG_RTK_MESH
#define _MESH_ACL_ENABLE_

/*need check Tx AMSDU dependency ; 8196B no support now */
#ifdef	SUPPORT_TX_AMSDU
#define MESH_AMSDU
#endif
//#define MESH_ESTABLISH_RSSI_THRESHOLD
//#define MESH_BOOTSEQ_AUTH
#endif // CONFIG_RTK_MESH


//-------------------------------------------------------------
// Realtek proprietary wake up on wlan mode
//-------------------------------------------------------------
//#define RTK_WOW


//-------------------------------------------------------------
// Support 802.11d
//-------------------------------------------------------------
//#define DOT11D


//-------------------------------------------------------------
// Use static buffer for STA private buffer
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M)
#define PRIV_STA_BUF
#endif

//-------------------------------------------------------------
// Do not drop packet immediately when rx buffer empty
//-------------------------------------------------------------
#ifdef CONFIG_RTL8190_PRIV_SKB
	#define DELAY_REFILL_RX_BUF
#endif


//-------------------------------------------------------------
// WiFi 11n 20/40 coexistence
//-------------------------------------------------------------
#define WIFI_11N_2040_COEXIST


//-------------------------------------------------------------
// Add TX power by command
//-------------------------------------------------------------
#define ADD_TX_POWER_BY_CMD


//-------------------------------------------------------------
// Merge firmware and phy parameter files into binary
//-------------------------------------------------------------
#define MERGE_FW


//-------------------------------------------------------------
// Do Rx process in tasklet
//-------------------------------------------------------------
//#define RX_TASKLET


//-------------------------------------------------------------
// Support external high power PA
//-------------------------------------------------------------
#ifdef CONFIG_HIGH_POWER_EXT_PA
#define HIGH_POWER_EXT_PA
#endif


//-------------------------------------------------------------
// Cache station info for bridge
//-------------------------------------------------------------
//#define RTL_CACHED_BR_STA


//-------------------------------------------------------------
// Use default keys of WEP (instead of keymapping keys)
//-------------------------------------------------------------
//#define USE_WEP_DEFAULT_KEY


//-------------------------------------------------------------
// Concurrent support up to 64 clients
//-------------------------------------------------------------
//#define STA_EXT


//-------------------------------------------------------------
// Auto test support
//-------------------------------------------------------------
#define AUTO_TEST_SUPPORT


//-------------------------------------------------------------
// to prevent broadcast storm attacks
//-------------------------------------------------------------
#define PREVENT_BROADCAST_STORM	1

#ifdef PREVENT_BROADCAST_STORM
/*
 *	NOTE: The driver will skip the other broadcast packets if the system free memory is less than FREE_MEM_LOWER_BOUND 
 *		   and the broadcast packet amount is larger than BROADCAST_STORM_THRESHOLD in one second period.
 */

#define BROADCAST_STORM_THRESHOLD	8
#define FREE_MEM_LOWER_BOUND			800 //uint: KBytes
#endif


//-------------------------------------------------------------
// Video streaming refine
//-------------------------------------------------------------
#ifdef CONFIG_RTK_VLC_SPEEDUP_SUPPORT
	#define VIDEO_STREAMING_REFINE
#endif


//-------------------------------------------------------------
// Rx buffer gather feature
//-------------------------------------------------------------
#define RX_BUFFER_GATHER


//-------------------------------------------------------------
// A4 client support
//-------------------------------------------------------------
//#define A4_STA

#if defined(A4_STA) && defined(WDS)
	#error "A4_STA and WDS can't be used together\n"
#endif


/*********************************************************************/
/* some definitions in 8192cd driver, we set them as NULL definition */
/*********************************************************************/
#ifdef USE_RTL8186_SDK
#ifdef CONFIG_WIRELESS_LAN_MODULE
#define __DRAM_IN_865X
#define __IRAM_IN_865X
#else
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#endif

#define RTL8190_DIRECT_RX						/* For packet RX : directly receive the packet instead of queuing it */
#define RTL8190_ISR_RX							/* process RXed packet in interrupt service routine: It become useful only when RTL8190_DIRECT_RX is defined */

#ifndef CONFIG_WIRELESS_LAN_MODULE
#define RTL8192CD_VARIABLE_USED_DMEM			/* Use DMEM for some critical variables */
#endif

#else // not USE_RTL8186_SDK

#define __DRAM_IN_865X
#define __IRAM_IN_865X
#endif


#undef __MIPS16
//#define __MIPS16
#define __MIPS16			__attribute__ ((mips16))

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
#define __IRAM_WLAN_HI		__attribute__ ((section(".iram-wapi")))
#define __DRAM_WLAN_HI		__attribute__ ((section(".dram-wapi")))
#endif


//-------------------------------------------------------------
// Kernel 2.6 specific config
//-------------------------------------------------------------
#ifdef __LINUX_2_6__

#define USE_RLX_BSP

#define RTL8192CD_VARIABLE_USED_DMEM

#ifndef RX_TASKLET
	#define	RX_TASKLET
#endif

#endif


#if 0
//-------------------------------------------------------------
// TR define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL8196B_TR


//-------------------------------------------------------------
// AC define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL865X_AC

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL865X_AC


//-------------------------------------------------------------
// X86 define flag
//-------------------------------------------------------------
#ifdef CONFIG_X86

#ifndef CONFIG_NET_PCI
	#define CONFIG_NET_PCI
#endif

#ifdef __MIPSEB__
	#undef __MIPSEB__
#endif

#ifdef _BIG_ENDIAN_
	#undef _BIG_ENDIAN_
#endif

#ifndef _LITTLE_ENDIAN_
	#define _LITTLE_ENDIAN_
#endif

#ifdef _USE_DRAM_
	#undef _USE_DRAM_
#endif

#ifdef CHECK_SWAP
	#undef CHECK_SWAP
#endif

#ifdef EVENT_LOG
	#undef EVENT_LOG
#endif

#ifdef BR_SHORTCUT
	#undef BR_SHORTCUT
#endif

#ifdef RTK_BR_EXT
	#undef RTK_BR_EXT
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifndef __LINUX_2_6__
	#define del_timer_sync del_timer
#endif

#ifdef USE_IO_OPS
	#undef USE_IO_OPS
#endif

#ifdef IO_MAPPING
	#undef IO_MAPPING
#endif

#ifdef MBSSID
	#undef MBSSID
#endif

#ifdef RTK_QUE
	#undef RTK_QUE
#endif

#endif // CONFIG_X86
#endif //#if 0


//-------------------------------------------------------------
// Define flag of EC system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196C_EC

#ifndef USE_WEP_DEFAULT_KEY
	#define USE_WEP_DEFAULT_KEY
#endif

#ifdef TESTCHIP_SUPPORT
	#undef TESTCHIP_SUPPORT
#endif

#endif


//-------------------------------------------------------------
// MSC define flag
//-------------------------------------------------------------
#ifdef _SINUX_
#define CONFIG_MSC 
#endif

#ifdef CONFIG_MSC
#define INCLUDE_WPS

#ifdef CHECK_HANGUP
	#undef CHECK_HANGUP
#endif
#ifndef USE_WEP_DEFAULT_KEY         //2010.4.23 Fred Fu open it to support wep key index 1 2 3
	#define USE_WEP_DEFAULT_KEY
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef	INCLUDE_WPS
#define USE_PORTING_OPENSSL
#endif
#endif

#endif

//-------------------------------------------------------------
// Define flag of 8672 system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8672

#undef DRV_RELDATE
#define DRV_RELDATE		"2009-11-18/2010-0104"

#ifndef RX_TASKLET
	#define RX_TASKLET
#endif

#ifdef RTL8190_DIRECT_RX
	#undef RTL8190_DIRECT_RX
#endif

#ifdef RTL8190_ISR_RX
	#undef RTL8190_ISR_RX
#endif

#ifdef USE_RLX_BSP
	#undef USE_RLX_BSP
#endif

#undef TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM		2

#ifdef __DRAM_IN_865X
	#undef __DRAM_IN_865X
#endif
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))

#ifdef __IRAM_IN_865X
	#undef __IRAM_IN_865X
#endif
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))

#ifdef __IRAM_IN_865X_HI
	#undef __IRAM_IN_865X_HI
#endif
#define __IRAM_IN_865X_HI	__attribute__ ((section(".iram-tx")))

#ifdef PCIE_POWER_SAVING
	#undef PCIE_POWER_SAVING
#endif

#endif // CONFIG_RTL8672


#if 0
//-------------------------------------------------------------
// TLD define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196B_TLD

#ifndef STA_EXT
	#define STA_EXT
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef HIGH_POWER_EXT_PA
	#undef HIGH_POWER_EXT_PA
#endif

#ifdef ADD_TX_POWER_BY_CMD
	#undef ADD_TX_POWER_BY_CMD
#endif

#endif // CONFIG_RTL8196B_TLD
#endif



//-------------------------------------------------------------
// KLD define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196C_KLD)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

//#ifdef HIGH_POWER_EXT_PA
//	#undef HIGH_POWER_EXT_PA
//#endif

//#ifdef ADD_TX_POWER_BY_CMD
//	#undef ADD_TX_POWER_BY_CMD
//#endif

#endif // CONFIG_RTL8196C_KLD

//-------------------------------------------------------------
// Dependence check of define flag
//-------------------------------------------------------------
#if defined(B2B_TEST) && !defined(MP_TEST)
	#error "Define flag error, MP_TEST is not defined!\n"
#endif


#if defined(UNIVERSAL_REPEATER) && !defined(CLIENT_MODE)
	#error "Define flag error, CLIENT_MODE is not defined!\n"
#endif


/*=============================================================*/
/*------ Compiler Portability Macros --------------------------*/
/*=============================================================*/
#ifdef EVENT_LOG
	extern int scrlog_printk(const char * fmt, ...);
#ifdef CONFIG_RTK_MESH
/*
 *	NOTE: dot1180211sInfo.log_enabled content from webpage MIB_LOG_ENABLED (bitmap) (in AP/goahead-2.1.1/LINUX/fmmgmt.c  formSysLog)
 */
	#define _LOG_MSG(fmt, args...)	if (1 & GET_MIB(priv)->dot1180211sInfo.log_enabled) scrlog_printk(fmt, ## args)
	#define LOG_MESH_MSG(fmt, args...)	if (16 & GET_MIB(priv)->dot1180211sInfo.log_enabled) _LOG_MSG("%s: " fmt, priv->mesh_dev->name, ## args)
#else
	#define _LOG_MSG(fmt, args...)	scrlog_printk(fmt, ## args)
#endif
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}

#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}
#elif defined(CONFIG_RTL8196B_TLD)
	#define LOG_MSG_DEL(fmt, args...)	_LOG_MSG(fmt, ## args)
	#define LOG_MSG(fmt, args...)	{}
#else
	#define LOG_MSG(fmt, args...)	_LOG_MSG("%s: "fmt, priv->dev->name, ## args)
#endif
#else
	#if defined(__GNUC__) || defined(GREEN_HILL)
		#define LOG_MSG(fmt, args...)	{}
	#else
		#define LOG_MSG
	#endif
#endif // EVENT_LOG

#ifdef _USE_DRAM_
	#define DRAM_START_ADDR		0x81000000	// start address of internal data ram
#endif

#ifdef __GNUC__
#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
#define __PACK
#endif

#ifdef __arm
#define __WLAN_ATTRIB_PACK__
#define __PACK	__packed
#endif

#ifdef GREEN_HILL
#define __WLAN_ATTRIB_PACK__
#define __PACK
#endif


/*=============================================================*/
/*-----------_ Driver module flags ----------------------------*/
/*=============================================================*/
#ifdef CONFIG_WIRELESS_LAN_MODULE
	#define	MODULE_NAME		"Realtek WirelessLan Driver"
	#define	MODULE_VERSION	"v1.00"

	#define MDL_DEVINIT
	#define MDL_DEVEXIT
	#define MDL_INIT
	#define MDL_EXIT
	#define MDL_DEVINITDATA
#else
	#define MDL_DEVINIT		__devinit
	#define MDL_DEVEXIT		__devexit
	#define MDL_INIT		__init
	#define MDL_EXIT		__exit
	#define MDL_DEVINITDATA	__devinitdata
#endif


/*=============================================================*/
/*----------- System configuration ----------------------------*/
/*=============================================================*/
#if defined(CONFIG_RTL8196B_GW_8M)
#define NUM_TX_DESC		200
#else
#ifdef CONFIG_RTL8198
#define NUM_TX_DESC		512
#else
#define NUM_TX_DESC		256		// kmalloc max size issue
#endif
#endif

#ifdef CONFIG_RTL8198
#define NUM_RX_DESC		128

#elif defined(__ECOS)
#define NUM_RX_DESC		64
#undef NUM_TX_DESC
#define NUM_TX_DESC		512

#else
#ifdef RX_BUFFER_GATHER
#define NUM_RX_DESC		64

#else
#define NUM_RX_DESC		32
#endif
#endif

#ifdef DELAY_REFILL_RX_BUF
#define REFILL_THRESHOLD	NUM_RX_DESC
#endif

#define FW_NUM_STAT 32

#if (defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)) && defined(MBSSID)
#define NUM_CMD_DESC	2
#else
#define NUM_CMD_DESC	16
#endif

#ifdef STA_EXT
#define NUM_STAT		64
#else
#define NUM_STAT		31 //32
#endif
#define MAX_GUEST_NUM   NUM_STAT

#define NUM_TXPKT_QUEUE			64
#define NUM_APSD_TXPKT_QUEUE	32
#define NUM_DZ_MGT_QUEUE	16

#define PRE_ALLOCATED_HDR		NUM_TX_DESC
#ifdef DRVMAC_LB
#define PRE_ALLOCATED_MMPDU		32
#define PRE_ALLOCATED_BUFSIZE	(2048/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int
#else
#define PRE_ALLOCATED_MMPDU		64
#define PRE_ALLOCATED_BUFSIZE	((600+128)/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int
#endif

#define MAX_BSS_NUM		64

#define MAX_NUM_WLANIF		4
#define WLAN_MISC_MAJOR		13

#define MAX_FRAG_COUNT		10

#define NUM_MP_SKB		32

// unit of time out: 10 msec
#define AUTH_TO			500
#define ASSOC_TO		500
#define FRAG_TO			2000

#define SS_PSSV_TO		12			// passive scan for 120 ms
#define SS_TO			5

#ifdef CONFIG_RTL_NEW_AUTOCH
#define SS_AUTO_CHNL_TO	20
#endif

#ifdef CONFIG_RTK_MESH
//GANTOE for automatic site survey 2008/12/10
#define SS_RAND_DEFER		300
#endif
#ifdef LINUX_2_6_22_
#define EXPIRE_TO		HZ
#else
#define EXPIRE_TO		100
#endif
#define REAUTH_TO		500
#define REASSOC_TO		500
#define REAUTH_LIMIT	6
#define REASSOC_LIMIT	6

#define DEFAULT_OLBC_EXPIRE		60

#define GBWC_TO			25

#ifdef __DRAYTEK_OS__
#define SS_COUNT		2
#else
#define SS_COUNT		3
#endif

#define TUPLE_WINDOW	64

#define RC_TIMER_NUM	64
#define RC_ENTRY_NUM	128
#define AMSDU_TIMER_NUM	64
#define AMPDU_TIMER_NUM	64

#define ROAMING_DECISION_PERIOD_INFRA	5
#define ROAMING_DECISION_PERIOD_ADHOC	10
#define ROAMING_DECISION_PERIOD_ARRAY (ROAMING_DECISION_PERIOD_ADHOC+1)
#define ROAMING_THRESHOLD		1	// roaming will be triggered when rx
									// beacon percentage is less than the value
#define FAST_ROAMING_THRESHOLD	40

/* below is for security.h  */
#define MAXDATALEN		1560
#define MAXQUEUESIZE	4
#define MAXRSNIELEN		128
#define E_DOT11_2LARGE	-1
#define E_DOT11_QFULL	-2
#define E_DOT11_QEMPTY	-3
#ifdef WIFI_SIMPLE_CONFIG
#define PROBEIELEN		260
#endif

// for SW LED
#define LED_MAX_PACKET_CNT_B	400
#define LED_MAX_PACKET_CNT_AG	1200
#define LED_MAX_SCALE			100
#define LED_NOBLINK_TIME		110	// time more than watchdog interval
#define LED_INTERVAL_TIME		50	// 500ms
#define LED_ON_TIME				4	// 40ms
#define LED_ON					0
#define LED_OFF					1
#define LED_0 					0
#define LED_1 					1
#define LED_2					2

// for counting association number
#define INCREASE		1
#define DECREASE		0

// DFS
#define CH_AVAIL_CHK_TO			6200	 // 62 seconds


#ifdef RX_BUFFER_GATHER
#define MAX_SKB_BUF	2048
#define MAX_RX_BUF_LEN	(MAX_SKB_BUF -sizeof(struct skb_shared_info) - 128)
#define MIN_RX_BUF_LEN MAX_RX_BUF_LEN	

#else
#define MAX_RX_BUF_LEN	8400
#define MIN_RX_BUF_LEN	4400
#endif

/* for RTL865x suspend mode */
#define TP_HIGH_WATER_MARK 50 //80 /* unit: Mbps */
#define TP_LOW_WATER_MARK 30 //40 /* unit: Mbps */

#define FW_BOOT_SIZE	400
#define FW_MAIN_SIZE	52000
#define FW_DATA_SIZE	850
#define AGC_TAB_SIZE	1600

#define MAC_REG_SIZE	1024
#define	PHY_REG_SIZE	2048
#define PHY_REG_PG_SIZE 256
#define PHY_REG_1T2R	256
#define PHY_REG_1T1R	256
#define FW_IMEM_SIZE	40*(1024)
#define FW_EMEM_SIZE	50*(1024)
#define FW_DMEM_SIZE	48

// for PCIe power saving
#define POWER_DOWN_T0	(10*HZ)
#define PKT_PAGE_SZ 	128
#define TX_DESC_SZ 		32


#ifdef SUPPORT_TX_MCAST2UNI
#define MAX_IP_MC_ENTRY		8
#endif

#define IQK_ADDA_REG_NUM	16
#define MAX_TOLERANCE		5
#define	IQK_DELAY_TIME		1		//ms
#define IQK_MAC_REG_NUM		4

#define SKIP_MIC_NUM	300

//Analog Pre-distortion calibration
#define		APK_BB_REG_NUM	5
#define		APK_AFE_REG_NUM	16
#define		APK_CURVE_REG_NUM 4
#define		PATH_NUM		2

// for dynamic mechanism of reserving tx desc
#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#define IF_TXDESC_UPPER_LIMIT	70	// percentage
#endif

// for dynamic mechanism of retry count
#define RETRY_TRSHLD_H	3750000
#define RETRY_TRSHLD_L	3125000

//-------------------------------------------------------------
// Define flag for 8M gateway configuration
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_GW_8M)

//#ifdef MERGE_FW
//	#undef MERGE_FW
//#endif
//#define DW_FW_BY_MALLOC_BUF

#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

#undef NUM_STAT
#define NUM_STAT		16

#endif // CONFIG_RTL8196B_GW_8M


#if defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT)

#ifdef DOT11D
	#undef DOT11D
#endif


#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

#undef NUM_STAT
#define NUM_STAT		16


#endif //defined(CONFIG_RTL8196C_AP_ROOT)



#if defined(CONFIG_RTL8196C_CLIENT_ONLY)

#ifdef DOT11D
	#undef DOT11D
#endif

#endif

#ifdef WIFI_SIMPLE_CONFIG
#define MAX_WSC_IE_LEN		(256+128)
#define MAX_WSC_PROBE_STA	10

#endif
#endif // _8192CD_CFG_H_

