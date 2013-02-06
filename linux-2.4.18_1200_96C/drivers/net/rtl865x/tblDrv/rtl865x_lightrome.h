/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for Light ROME Driver
* Abstract : 
* $Id: rtl865x_lightrome.h,v 1.3 2008/03/26 12:23:06 davidhsu Exp $
*
*/

/*	@doc RTL865X_LIGHTROME_API

	@module rtl865x_lightrome.h - RTL865X Home gateway controller Light ROME API documentation	|
	This document explains the internal and external API interface for Light ROME module. Functions with _rtl865x prefix
	are internal functions and functions with rtl865x_ prefix are external functions.
	@normal Gateway team (gwhp@realtek.com.tw) <date>

	Copyright <cp>2006 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL865X_LIGHTROME_API
*/
#include "rtl_types.h"
//#include "rtl8651_tblDrvProto.h"




#define RTL8651_MAC_NUMBER				6
#define RTL8651_PORT_NUMBER				RTL8651_MAC_NUMBER
#define RTL8651_MII_PORTNUMBER                 	5
#define RTL8651_MII_PORTMASK                    	0x20
#define RTL8651_PHY_NUMBER				5
#define RTL8651_AGGREGATOR_NUMBER		(RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
#define RTL8651_PSOFFLOAD_RESV_PORT		(RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum+1)	/* port reserved for protocol stack offloading */
#define RTL8651_ALLPORTMASK				((1<<RTL8651_AGGREGATOR_NUMBER)-1)
#define RTL8651_PHYSICALPORTMASK			((1<<RTL8651_MAC_NUMBER)-1)
#define RTL8651_L2TBL_ROW					256
#define RTL8651_L2TBL_COLUMN				4
#define RTL8651_MAX_AGGREGATION_NUM		1
#define RTL8651_VLAN_NUMBER				8
#define RTL8651_PPPOETBL_SIZE			8
#define RTL8651_L2_NUMBER				1024
#define RTL8651_IPTABLE_SIZE				16
#define TCP_CLOSED_FLOW					8



/* ACL Rule type Definition */
#define RTL8651_ACL_MAC						0x00
#define RTL8651_ACL_IP						0x01
#define RTL8651_ACL_ICMP						0x02
#define RTL8651_ACL_IGMP						0x03
#define RTL8651_ACL_TCP						0x04
#define RTL8651_ACL_UDP						0x05
/* 6-8*/ 
#define RTL8652_ACL_IP_RANGE					0x0A
#define RTL8652_ACL_ICMP_IPRANGE				0x0B
#define RTL8652_ACL_TCP_IPRANGE				0x0C
#define RTL8652_ACL_IGMP_IPRANGE				0x0D
#define RTL8652_ACL_UDP_IPRANGE				0x0E
#define RTL8652_ACL_SRCFILTER_IPRANGE 		0x09
#define RTL8652_ACL_DSTFILTER_IPRANGE 		0x0F


typedef struct rtl865x_aclRule_s {
	union {
		/* (1) MAC ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _typeLen, _typeLenMask;
		} MAC; 
		
		/* (2) IP Group ACL rule */
		struct {
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint8 _tos, _tosMask;
			union {
				/* (2.1) IP ACL rle */
				struct {
					uint8 _proto, _protoMask, _flagMask;// flag & flagMask only last 3-bit is meaningful
					uint32 _FOP:1, _FOM:1, _httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
					union {
						uint8 _flag;
						struct {
							uint8 pend1:5,
								 pend2:1,
								 _DF:1,	//don't fragment flag
								 _MF:1;	//more fragments flag
						} s;
					} un;							
				} ip; 
				/* (2.2) ICMP ACL rule */
				struct {
					uint8 _type, _typeMask, _code, _codeMask;
				} icmp; 
				/* (2.3) IGMP ACL rule */
				struct {
					uint8 _type, _typeMask;
				} igmp; 
				/* (2.4) TCP ACL rule */
				struct {
					ether_addr_t _l2srcMac, _l2srcMacMask;
					uint8 _flagMask;
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;					
					union {
						uint8 _flag;
						struct {
							uint8 _pend:2,
								  _urg:1, //urgent bit
								  _ack:1, //ack bit
								  _psh:1, //push bit
								  _rst:1, //reset bit
								  _syn:1, //sync bit
								  _fin:1; //fin bit
						}s;
					}un;	
				}tcp; 
				/* (2.5) UDP ACL rule */
				struct {					
					ether_addr_t _l2srcMac, _l2srcMacMask;
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;					
				}udp; 
			}is;
		}L3L4; 
	}un_ty;

	uint32	ruleType_:4;
	uint32	actionType_:4;
} rtl865x_aclRule_t;


/* MAC ACL rule Definition */
#define dstMac_				un_ty.MAC._dstMac
#define dstMacMask_			un_ty.MAC._dstMacMask
#define srcMac_				un_ty.MAC._srcMac
#define srcMacMask_			un_ty.MAC._srcMacMask
#define typeLen_				un_ty.MAC._typeLen
#define typeLenMask_			un_ty.MAC._typeLenMask

/* IFSEL ACL rule Definition */
#define gidxSel_				un_ty.IFSEL._gidxSel

/* Common IP ACL Rule Definition */
#define srcIpAddr_				un_ty.L3L4._srcIpAddr
#define srcIpAddrMask_			un_ty.L3L4._srcIpAddrMask
#define srcIpAddrUB_			un_ty.L3L4._srcIpAddr
#define srcIpAddrLB_			un_ty.L3L4._srcIpAddrMask
#define dstIpAddr_				un_ty.L3L4._dstIpAddr
#define dstIpAddrMask_		un_ty.L3L4._dstIpAddrMask
#define dstIpAddrUB_			un_ty.L3L4._dstIpAddr
#define dstIpAddrLB_			un_ty.L3L4._dstIpAddrMask
#define tos_					un_ty.L3L4._tos
#define tosMask_				un_ty.L3L4._tosMask
/* IP Rrange */
#define srcIpAddrStart_		un_ty.L3L4._srcIpAddr
#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddrMask
#define dstIpAddrStart_		un_ty.L3L4._dstIpAddr
#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddrMask

/* IP ACL Rule Definition */
#define ipProto_				un_ty.L3L4.is.ip._proto
#define ipProtoMask_			un_ty.L3L4.is.ip._protoMask
#define ipFlagMask_			un_ty.L3L4.is.ip._flagMask
#define ipFOP_      			un_ty.L3L4.is.ip._FOP
#define ipFOM_      			un_ty.L3L4.is.ip._FOM
#define ipHttpFilter_      		un_ty.L3L4.is.ip._httpFilter
#define ipHttpFilterM_			un_ty.L3L4.is.ip._httpFilterM
#define ipIdentSrcDstIp_   		un_ty.L3L4.is.ip._identSrcDstIp
#define ipIdentSrcDstIpM_		un_ty.L3L4.is.ip._identSrcDstIpM
#define ipFlag_				un_ty.L3L4.is.ip.un._flag
#define ipDF_					un_ty.L3L4.is.ip.un.s._DF
#define ipMF_					un_ty.L3L4.is.ip.un.s._MF

/* ICMP ACL Rule Definition */
#define icmpType_				un_ty.L3L4.is.icmp._type
#define icmpTypeMask_			un_ty.L3L4.is.icmp._typeMask	
#define icmpCode_				un_ty.L3L4.is.icmp._code
#define icmpCodeMask_			un_ty.L3L4.is.icmp._codeMask

/* IGMP ACL Rule Definition */
#define igmpType_				un_ty.L3L4.is.igmp._type
#define igmpTypeMask_			un_ty.L3L4.is.igmp._typeMask

/* TCP ACL Rule Definition */
#define tcpl2srcMac_			un_ty.L3L4.is.tcp._l2srcMac		// for srcMac & destPort ACL rule
#define tcpl2srcMacMask_		un_ty.L3L4.is.tcp._l2srcMacMask
#define tcpSrcPortUB_			un_ty.L3L4.is.tcp._srcPortUpperBound
#define tcpSrcPortLB_			un_ty.L3L4.is.tcp._srcPortLowerBound
#define tcpDstPortUB_			un_ty.L3L4.is.tcp._dstPortUpperBound
#define tcpDstPortLB_			un_ty.L3L4.is.tcp._dstPortLowerBound
#define tcpFlagMask_			un_ty.L3L4.is.tcp._flagMask
#define tcpFlag_				un_ty.L3L4.is.tcp.un._flag
#define tcpURG_				un_ty.L3L4.is.tcp.un.s._urg
#define tcpACK_				un_ty.L3L4.is.tcp.un.s._ack
#define tcpPSH_				un_ty.L3L4.is.tcp.un.s._psh
#define tcpRST_				un_ty.L3L4.is.tcp.un.s._rst
#define tcpSYN_				un_ty.L3L4.is.tcp.un.s._syn
#define tcpFIN_				un_ty.L3L4.is.tcp.un.s._fin

/* UDP ACL Rule Definition */
#define udpl2srcMac_			un_ty.L3L4.is.udp._l2srcMac		// for srcMac & destPort ACL rule
#define udpl2srcMacMask_		un_ty.L3L4.is.udp._l2srcMacMask
#define udpSrcPortUB_			un_ty.L3L4.is.udp._srcPortUpperBound
#define udpSrcPortLB_			un_ty.L3L4.is.udp._srcPortLowerBound
#define udpDstPortUB_			un_ty.L3L4.is.udp._dstPortUpperBound
#define udpDstPortLB_			un_ty.L3L4.is.udp._dstPortLowerBound

#if 1
/* Source Filter ACL Rule Definition */
#define srcFilterMac_			un_ty.SRCFILTER._srcMac
#define srcFilterMacMask_		un_ty.SRCFILTER._srcMacMask
#define srcFilterPort_			un_ty.SRCFILTER._srcPort
#define srcFilterPortMask_		un_ty.SRCFILTER._srcPortMask
#define srcFilterVlanIdx_		un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanId_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanIdxMask_	un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterVlanIdMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterIpAddr_			un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrMask_	un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterIpAddrUB_		un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrLB_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterPortUpperBound_	un_ty.SRCFILTER._srcPortUpperBound
#define srcFilterPortLowerBound_	un_ty.SRCFILTER._srcPortLowerBound
#define srcFilterIgnoreL3L4_		un_ty.SRCFILTER._ignoreL3L4
#define srcFilterIgnoreL4_		un_ty.SRCFILTER._ignoreL4

/* Destination Filter ACL Rule Definition */
#define dstFilterMac_			un_ty.DSTFILTER._dstMac
#define dstFilterMacMask_		un_ty.DSTFILTER._dstMacMask
#define dstFilterVlanIdx_		un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdxMask_	un_ty.DSTFILTER._vlanIdxMask
#define dstFilterVlanId_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterIpAddr_		un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrMask_	un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortUpperBound_	un_ty.DSTFILTER._dstPortUpperBound
#define dstFilterIpAddrUB_		un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrLB_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortLowerBound_	un_ty.DSTFILTER._dstPortLowerBound
#define dstFilterIgnoreL3L4_		un_ty.DSTFILTER._ignoreL3L4
#define dstFilterIgnoreL4_		un_ty.DSTFILTER._ignoreL4
#endif

/* ACL Rule Action type Definition */
#define RTL8651_ACL_PERMIT			0x01
#define RTL8651_ACL_DROP				0x02
#define RTL8651_ACL_CPU				0x03
#define RTL8651_ACL_DROP_LOG			0x04
#define RTL8651_ACL_DROP_NOTIFY		0x05
#define RTL8651_ACL_L34_DROP			0x06	/* special for default ACL rule */

/* ACL Rule type Definition */
#define RTL8651_ACL_MAC				0x00
#define RTL8651_ACL_IP					0x01
#define RTL8651_ACL_ICMP				0x02
#define RTL8651_ACL_IGMP				0x03
#define RTL8651_ACL_TCP					0x04
#define RTL8651_ACL_UDP				0x05
/* 6-8*/ 
#define RTL8652_ACL_IP_RANGE			0x0A
#define RTL8652_ACL_ICMP_IPRANGE		0x0B
#define RTL8652_ACL_TCP_IPRANGE		0x0C
#define RTL8652_ACL_IGMP_IPRANGE		0x0D
#define RTL8652_ACL_UDP_IPRANGE		0x0E
#define RTL8652_ACL_SRCFILTER_IPRANGE 0x09
#define RTL8652_ACL_DSTFILTER_IPRANGE 0x0F



/* Spanning Tree Port State Definition */
#define RTL8651_PORTSTA_DISABLED		0x00
#define RTL8651_PORTSTA_BLOCKING		0x01
#define RTL8651_PORTSTA_LISTENING		0x02
#define RTL8651_PORTSTA_LEARNING		0x03
#define RTL8651_PORTSTA_FORWARDING	0x04

/* For round robin ration */
#define RTL8651_RR_H_FIRST		0x03
#define RTL8651_RR_H16_L1		0x02
#define RTL8651_RR_H8_L1		0x01
#define RTL8651_RR_H4_L1		0x00

#define RTL8651_PORT_0                  0x00
#define RTL8651_PORT_1                  0x01
#define RTL8651_PORT_2                  0x02
#define RTL8651_PORT_3                  0x03
#define RTL8651_PORT_4                  0x04
#define RTL8651_MII_PORT                0x05
#define RTL8651_CPU_PORT                0x07

#define RTL8651_PORT_EXT1				0x06
#define RTL8651_PORT_EXT2				0x07
#define RTL8651_PORT_EXT3				0x08

//Ethernet port bandwidth control
#define RTL8651_BC_FULL		0x00
#define RTL8651_BC_128K		0x01
#define RTL8651_BC_256K		0x02
#define RTL8651_BC_512K		0x03
#define RTL8651_BC_1M		0x04
#define RTL8651_BC_2M		0x05
#define RTL8651_BC_4M		0x06
#define RTL8651_BC_8M		0x07
#define RTL8651_BC_16M		0x08
#define RTL8651_BC_32M		0x09
#define RTL8651_BC_64M		0x0A

#ifdef CONFIG_RTL865XC
uint32 rtl8651_vlanTableIndex(uint16 vid);
#define RTL8651_RR_H_FIRST		0x03
#else
#define rtl8651_vlanTableIndex(vid)  		(vid& (RTL8651_VLAN_NUMBER-1))
#endif
#define PKT_BUF_SZ					2048 /* Size of each temporary Rx buffer.*/


#define RTL865XC_NETINTERFACE_NUMBER		8
#define RTL865XC_MAC_NETINTERFACE_NUMBER		4
#define RTL865XC_PORT_NUMBER				9
#define RTL865XC_VLAN_NUMBER				4096
#define RTL865XC_LAGHASHIDX_NUMBER			8	/* There are 8 hash values in RTL865xC Link Aggregation. */
#ifndef RTL865XC_ACLTBL_ALL_LAYER
#define RTL865XC_ACLTBL_ALL_LAYER    0
#endif
#define RTL8651_ACLTBL_NOOP				7 /* No operation. Don't apply this rule. */
#define RTL8651_ETHER_AUTO_100FULL	0x00
#define RTL8651_ETHER_AUTO_100HALF	0x01
#define RTL8651_ETHER_AUTO_10FULL	0x02
#define RTL8651_ETHER_AUTO_10HALF	0x03
#define RTL8651_ETHER_AUTO_1000FULL	0x08
#define RTL8651_ETHER_AUTO_1000HALF	0x09
/* chhuang: patch for priority issue */
#define RTL8651_ETHER_FORCE_100FULL	0x04
#define RTL8651_ETHER_FORCE_100HALF	0x05
#define RTL8651_ETHER_FORCE_10FULL	0x06
#define RTL8651_ETHER_FORCE_10HALF	0x07



#ifdef CONFIG_RTL865XC
extern  int32		rtl8651_totalExtPortNum; //this replaces all RTL8651_EXTPORT_NUMBER defines
#endif

#define MAX_IFNAMESIZE				16
#define LRCONFIG_END					{ "", 0, 0, 0, 0, 0, 0, 0, {{0}} }

enum IF_TYPE
{
	IF_ETHER = 0,
	IF_PPPOE = 1,
	IF_PPTP = 2,
	IF_L2TP = 3,
#ifdef CONFIG_RTL865X_HW_PPTPL2TP
	IF_PPP = 1,
#endif	
};


struct rtl865x_lrConfig {
	uint8 			ifname[MAX_IFNAMESIZE];
	uint8			isWan;
	enum IF_TYPE		if_type;
	uint16			vid;
	uint16			fid;
	uint32			memPort;
	uint32			untagSet;
	uint32			mtu;
	ether_addr_t		mac;
};














enum LR_RESULT
{
	/* Common error code */
	LR_SUCCESS = 0,                     		/* Function Success */
	LR_FAILED = -1,                     		/* General Failure, not recommended to use */
	LR_ERROR_PARAMETER = -2,            	/* The given parameter error */
	LR_EXIST = -3,                      		/* The entry you want to add has been existed, add failed */
	LR_NONEXIST = -4,                   	/* The specified entry is not found */

	LR_NOBUFFER = -1000,				/* Out of Entry Space */
	LR_INVAPARAM = -1001,			/* Invalid parameters */
	LR_NOTFOUND = -1002,			/* Entry not found */
	LR_DUPENTRY = -1003,				/* Duplicate entry found */
	LR_RTUNREACH = -1004,			/* Unreachable route */
	LR_INVVID = -1005,				/* Invalid VID */
	LR_IFDOWN = -1006,				/* Interface is down */
	LR_NOARPSPACE = -1007,			/* Out of free ARP space */
	LR_INVIF = -1008,					/* Invalid interface */
	LR_INUSE = -1009,					/* Forbidden due to in use */
	LR_INVEXTIP = -1010,				/* Invalid external IP address */
	LR_COLLISION = 1011,				/* NAPT entry collision */
};


/*==========================================================================
 *  Light ROME Driver may support the following two modes:
 *    a.Linux with bridge support
 *    b.Linux without bridge support
 *
 *  a.Linux With Bridge Support
 *    In this mode, the Light ROME Driver simulates 6 interfaces for Linux
 *    (from eth0 to eth5 and one to one mapping to physical port).
 *    Therefore, the Linux can utilize bridge to cascade every physical port of RTL865x.
 *
 *  +---------------------------------------------+
 *  |        a.Linux With Bridge Support          |
 *  |                                             |
 *  | +-----------+   +-------------------------+ |
 *  | |    br0    |   |           br1           | |
 *  | +-----------+   +-------------------------+ |
 *  |    |     |         |      |     |      |    |
 *  | +----+ +----+   +----+ +----+ +----+ +----+ |
 *  +-|eth0|-|eth1|---|eth2|-|eth3|-|eth4|-|eth5|-+
 *    +----+ +----+   +----+ +----+ +----+ +----+
 *       |     |         |      |     |      |
 *  +---------------------------------------------+
 *  |          Light ROME Driver NIC              |
 *  +---------------------------------------------+
 *       |     |         |      |     |      |
 *    +----+ +----+   +----+ +----+ +----+ +----+
 *    | P0 | | P1 |   | P2 | | P3 | | P4 | | P5 |
 *    +----+ +----+   +----+ +----+ +----+ +----+
 *
 *  [How to configure Light ROME Driver?]
 *     addInterface( "eth0", ... );
 *     addInterface( "eth1", ... );
 *     addInterface( "eth2", ... );
 *     addInterface( "eth3", ... );
 *     addInterface( "eth4", ... );
 *     addInterface( "eth5", ... );
 *     configInterface( "eth0", PORT0, ... );
 *     configInterface( "eth1", PORT1, ... );
 *     configInterface( "eth2", PORT2, ... );
 *     configInterface( "eth3", PORT3, ... );
 *     configInterface( "eth4", PORT4, ... );
 *     configInterface( "eth5", PORT5, ... );
 *
 *==========================================================================
 *  b.Linux WITHOUT Bridge Support
 *    In this mode, the Light ROME Driver simulates 2 interfaces for Linux (eth0 for WAN, and eth1 for LAN).
 *    The Light ROME Driver must provide bridge functionality, including MAC learning and bridge.
 *    The bridge functionality can be implemented by ASIC (both 865xB and 865xC).
 *
 *  +---------------------------------------------+
 *  |       b.Linux WITHOUT Bridge Support        |
 *  |                                             |
 *  |                                             |
 *  |                                             |
 *  | +-----------+   +-------------------------+ |
 *  +-|   eth0    |---|           eth1          |-+
 *    +-----------+   +-------------------------+
 *          |                      |          
 *  +---------------------------------------------+
 *  |      Light ROME Driver NIC (Bridging)       |
 *  +---------------------------------------------+
 *        |    |            |     |    |    |
 *     +----+----+        +----+----+----+----+
 *     | P0 | P1 |        | P2 | P3 | P4 | P5 |
 *     +----+----+        +----+----+----+----+
 *
 *   [How to configure Light ROME Driver?]
 *     addInterface( "eth0", ... );
 *     addInterface( "eth1", ... );
 *     configInterface( "eth0", PORT0|PORT1, ... );
 *     configInterface( "eth1", PORT2|PORT3|PORT4|PORT5, ... );
 *  
 */
enum IF_FLAGS
{
	IF_NONE,
	IF_INTERNAL = (0<<1), /* This is an internal interface. */
	IF_EXTERNAL = (1<<1), /* This is an external interface. */
};
/*
@func enum LR_RESULT | rtl865x_addInterface | Add an interface
@parm uint8* | ifname | The interface name to add (maximum length: 15 octets, zero-terminated). 'ethN.V' for VLAN ID V. 'ethN:A' for IP alias A.
@parm ipaddr_t | ipAddr | The IP address of this interface
@parm ether_addr_t* | gmac | The Gateway MAC address
@parm uint32 | mtu | The MTU of this interface
@parm enum IF_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success
@rvalue LR_EXIST | Add an existed interface
@rvalue LR_ERROR_PARAMETER | Error parameter is given
@rvalue LR_FAILED | General failure
@comm 
	Add an interface for Linux
@devnote
	This function will be called when 'ifconfig' command is executed.
	When calling this function, the Light ROME Driver will write the given parameters into the following tables:
	  1. Net Interface Table (<p gmac>, <p mtu>)
	  2. IP Table (<p ipAddr>, if this is an external interface)
*/
enum LR_RESULT rtl865x_addInterface( uint8* ifname, ipaddr_t ipAddr, ether_addr_t* gmac, uint32 mtu, enum IF_FLAGS flags );


/*
@func enum LR_RESULT | rtl865x_configInterface | Configure an interface with 865x-depend parameters
@parm uint8* | ifname | The interface name to add (maximum length: 15 octets, zero-terminated). 'ethN.V' for VLAN ID V. 'ethN:A' for IP alias A.
@parm uint32 | vlanId | The VLAN ID (valid from 1 to 4095).
@parm uint32 | fid | The Filtering Database Index
@parm uint32 | mbr | The member ports of this interface belongs to (included extension ports).
@parm uint32 | untag | The port mask that untags VLAN tag when Tx packet.
@parm enum IF_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success
@rvalue LR_ERROR_PARAMETER | Error parameter is given
@rvalue LR_FAILED | General failure
@comm 
	Configure an existed interface for 865x-depend setting.
	
@devnote
	This function is called at boot time.
	When calling this function, the Light ROME Driver will write the given parameters into the following tables:
	  1. Net Interface Table (<p vlanId>)
	  2. VLAN Table (<p vlanId>, <p fid>, <p mbr>, <p untag>)
	  3. L3 Routing Table (update l3.INT if l3.dnetif matched)
*/
enum LR_RESULT rtl865x_configInterface( uint8* ifname, uint32 vlanId, uint32 fid, uint32 mbr, uint32 untag, enum IF_FLAGS flags );


/*
@func enum LR_RESULT | rtl865x_delInterface | Delete an interface
@parm uint8* | ifname | The interface name to delete
@rvalue LR_SUCCESS | Delete success
@rvalue LR_NONEXIST | Delete an existed interface
@rvalue LR_ERROR_PARAMETER | Error parameter is given
@rvalue LR_FAILED | General failure
@comm 
	Delete an interface for Linux
@devnote
	This function will be called when 'ifconfig' command is executed.
	When calling this function, the Light ROME Driver will delete entry from the following tables:
	  1. Net Interface Table 
	  2. VLAN Table 
	  3. IP Table 
*/
enum LR_RESULT rtl865x_delInterface( uint8* ifname );


/*==========================================================================*/
/* FDB - Filtering Database                                                 */
/*==========================================================================*/
enum FDB_FLAGS
{
	FDB_FWD = 0,
	FDB_SRCBLK = 1,
	FDB_TRAPCPU = 2,
};
/*
@func enum LR_RESULT | rtl865x_addFdbEntry | Add an MAC address, said Filter Database Entry
@parm uint32 | vid | The VLAN ID (valid: 0~4095)
@parm uint32 | fid | The filter database index (valid: 0~3)
@parm ether_addr_t * | mac | The MAC address to be added
@parm uint32 | portmask | The portmask of this MAC address belongs to
@parm enum FDB_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success
@rvalue LR_FAILED | General failure
@comm 
	Add a Filter Database Entry to L2 Table(1024-Entry)
@devnote
	(under construction)
*/
enum LR_RESULT rtl865x_addFdbEntry( uint32 vfid, ether_addr_t * mac, uint32 portmask, enum FDB_FLAGS flags );


/*
@func enum LR_RESULT | rtl865x_delFdbEntry | Delete an MAC address, said Filter Database Entry
@parm uint32 | vid | The VLAN ID (valid: 0~4095)
@parm uint32 | fid | The filter database index (valid: 0~3)
@parm ether_addr_t * | mac | The MAC address to be deleted
@rvalue LR_SUCCESS | Delete success
@rvalue LR_FAILED | General failure
@comm 
	Delete a Filter Database Entry of L2 Table with MAC address
@devnote
	(under construction)
*/
enum LR_RESULT rtl865x_delFdbEntry( uint32 vfid, ether_addr_t * mac );

/*extension port*/
enum LR_RESULT rtl865x_addExtFdbEntry( uint32 vfid, ether_addr_t * mac, uint32 portmask, uint32 linkId, enum FDB_FLAGS flags );
enum LR_RESULT lr_ext_fdb_lookup(uint32 vfid, ether_addr_t *mac,  uint32 *memport, uint32 *linkID, uint32 *way, enum FDB_FLAGS flags);
enum LR_RESULT rtl865x_modifyExtFdbEntry( uint32 vfid, ether_addr_t * mac, uint32 portmask, uint32 linkId, enum FDB_FLAGS flags );


/*==========================================================================*/
/* ARP                                                                      */
/*==========================================================================*/
enum ARP_FLAGS
{
	ARP_NONE = 0,
};
/*
@func enum LR_RESULT | rtl865x_addArp | Add an ARP mapping
@parm ipaddr_t | ip | The IP address to add
@parm ether_addr_t * | mac | The MAC address mapping to this IP address
@parm enum ARP_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success
@rvalue LR_FAILED | General failure
@comm 
	Add an ARP mapping (IP and MAC) to H/W ARP Table
@devnote
	Insert into arp_process() function in file net/ipv4/arp.c
*/
enum LR_RESULT rtl865x_addArp( ipaddr_t ip, ether_addr_t * mac, enum ARP_FLAGS flags );


/*
@func enum LR_RESULT | rtl865x_delArp | Delete an ARP mapping
@parm ipaddr_t | ip | The IP address to delete
@rvalue LR_SUCCESS | Delete success
@rvalue LR_FAILED | General failure
@comm 
	Delete an ARP mapping entry with IP
@devnote
	Insert into arp_process() function in file net/ipv4/arp.c
*/
enum LR_RESULT rtl865x_delArp( ipaddr_t ip );


/*
@ func uint32 | rtl865x_arpSync | Sync asic arp with sofware arp entry.
@parm ipaddr_t | ip | IP address to sync.
@parm uint32 | refresh | Refresh arp entry.
@rvalue 0~300 | Current aging time. 
@comm
	Software sync arp entry with asic through periodically calling this API. Once the API returned 
	zero value, it means the entry has aged out in asic. In this case an attempt to remove asic
	entry should be made by software.
*/
uint32 rtl865x_arpSync( ipaddr_t ip, uint32 refresh );


/*==========================================================================*/
/* L3 Routing                                                               */
/*==========================================================================*/
enum RT_FLAGS
{
	RT_NONE = 0,
};
/*
@func enum LR_RESULT | rtl865x_addRoute | Add a Routing Entry
@parm ipaddr_t | ip | The IP address to add
@parm ipaddr_t | mask | The IP mask to add
@parm ipaddr_t | gateway | The nexthop to route
@parm uint8* | ifname | The interface name to route (maximum length: 15 octets, zero-terminated)
@parm enum RT_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success
@rvalue LR_FAILED | General failure
@comm 
	Add a Route Rule Entry to L3 Routing Table(8-Entry)
@devnote
	Insert into rt_intern_hash() function in file net/ipv4/route.c
*/
enum LR_RESULT rtl865x_addRoute( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags );


/*
@func enum LR_RESULT | rtl865x_delRoute | Delete a Routing Entry
@parm ipaddr_t | ip | The IP address to delete
@parm ipaddr_t | mask | The IP mask to delete
@rvalue LR_SUCCESS | Add success
@rvalue LR_FAILED | General failure
@comm 
	Delete a Routing Entry with IP Address and mask
@devnote
	Insert into SMP_TIMER_NAME(rt_check_expire)() function in file net/ipv4/route.c
*/
enum LR_RESULT rtl865x_delRoute( ipaddr_t ip, ipaddr_t mask );


/*==========================================================================*/
/* PPP Session                                                              */
/*==========================================================================*/
enum SE_TYPE
{
	SE_PPPOE = 1,
	SE_PPTP = 2,
	SE_L2TP = 3,
#ifdef CONFIG_RTL865X_HW_PPTPL2TP
	SE_PPP = 4,
#endif	
};
enum SE_FLAGS
{
	SE_NONE = 0,
};
/*
@func enum LR_RESULT | rtl865x_addPppoeSession | Add a PPPoE Session
@parm uint8* | ifname | The interface name
@parm enum ether_addr_t | *mac | The MAC address of PPPoE Server
@parm uint32 | sessionId | Session ID
@parm enum SE_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success
@rvalue LR_FAILED | General failure
@comm 
	Add a PPPoE Session mapping (Interface)
@devnote
	(under construction)
*/
enum LR_RESULT rtl865x_addPppoeSession( uint8* ifname, ether_addr_t *mac, uint32 sessionId, enum SE_FLAGS flags );


/*
@func enum LR_RESULT | rtl865x_delPppoeSession | Delete a PPPoE Session
@parm uint8* | ifname | The interface name
@rvalue LR_SUCCESS | Add success
@rvalue LR_FAILED | General failure
@comm 
	Delete a PPPoE Session mapping
@devnote
	(under construction)
*/
enum LR_RESULT rtl865x_delPppoeSession( uint8* ifname );


/*==========================================================================*/
/* NAPT Flow                                                                */
/*==========================================================================*/
enum NP_PROTOCOL
{
	NP_UDP = 0,
	NP_TCP = 1,
};
enum NP_FLAGS
{
	NP_NONE = 0,
};
/*
@func enum LR_RESULT | rtl865x_addNaptConnection | Add a NAPT Flow
@parm enum NP_PROTOCOL | protocol | The protocol to add
@parm ipaddr_t | intIp | Internal IP address
@parm uint32 | intPort | Internal Port
@parm ipaddr_t | extIp | External IP address
@parm uint32 | extPort | External Port
@parm ipaddr_t | remIp | Remote IP address
@parm uint32 | remPort | Remote Port
@parm enum NP_FLAGS | flags | reserved for future used
@rvalue LR_SUCCESS | Add success (can be ASIC-accelerated)
@rvalue LR_SUCCESS | Add success (cannot be ASIC-accelerated)
@rvalue LR_ERROR_PARAMETER | Error parameter is given
@rvalue LR_EXIST | Add an existed flow
@rvalue LR_FAILED | General failure
@comm 
	Add a NAPT Flow Entry to L4 TCP/UDP NAPT Table(1024-Entry)
@devnote
	Insert into ip_nat_setup_info() function in file net/ipv4/netfilter/ip_nat_core.c
*/
enum LR_RESULT rtl865x_addNaptConnection( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                                     ipaddr_t extIp, uint32 extPort,
                                                                     ipaddr_t remIp, uint32 remPort,
                                                                     enum NP_FLAGS flags );
/*
@func enum LR_RESULT | rtl865x_delNaptConnection | Delete a NAPT Flow
@parm enum NP_PROTOCOL | protocol | The protocol to delete
@parm ipaddr_t | intIp | Internal IP address
@parm uint32 | intPort | Internal Port
@parm ipaddr_t | extIp | External IP address
@parm uint32 | extPort | External Port
@parm ipaddr_t | remIp | Remote IP address
@parm uint32 | remPort | Remote Port
@rvalue LR_SUCCESS | Delete success
@rvalue LR_NONEXIST | Delete a non-existed flow
@rvalue LR_FAILED | General failure
@comm 
	Delete a NAPT Flow Entry of L4 TCP/UDP NAPT Table(1024-Entry)
@devnote
	Insert into ip_nat_cleanup_conntrack() function in file net/ipv4/netfilter/ip_nat_core.c
*/
enum LR_RESULT rtl865x_delNaptConnection( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                                                                     ipaddr_t extIp, uint32 extPort,
                                                                     ipaddr_t remIp, uint32 remPort );


enum REG_FLAGS
{
	REG_BR_RX_ACTION = 1,	/* Light Rome Driver doesn't support layer2 function */
	REG_RX_ACTION = 2,		/* Light Rome Driver supports layer2 function  */
	REG_LINK_CHANGE = 3,
};


enum ACL_FLAGS
{
	ACL_INGRESS = 1,
	ACL_EGRESS,
};

enum LR_RESULT rtl865x_addAclRule(uint32 vid, rtl865x_aclRule_t *rule_t, enum ACL_FLAGS flags);
enum LR_RESULT rtl865x_delAclRule(uint32 vid, rtl865x_aclRule_t *rule_t, enum ACL_FLAGS flags);

enum ACL_ACFLAGS
{
	ACL_PERMIT = 1, 	/* RTL8651_ACLTBL_PERMIT_ALL: 125 */
	ACL_DROP = 2, 	/* RTL8651_ACLTBL_DROP_ALL: 126 */
	ACL_CPU = 3, 	/* RTL8651_ACLTBL_ALL_TO_CPU: 127 */
};
	
enum LR_RESULT rtl865x_setAclDefaultAction(enum ACL_ACFLAGS defAction);






/*
@func enum LR_RESULT | rtl865x_lightRomeSend | Tx packets to network.
@parm void * | pkt | A pointer to a packet.
@parm uint32 | len | Packet length.
@parm uint16 | vid | VLAN ID the packet is sent to.
@parm uint32 | port_mask | port_mask != 0 means explicitly specify tx port. Othrewise, specify tx port by light rome layer 2 module.
@rvalue LR_SUCCESS | Successfully send out a packet.
@rvalue LR_FAILED | Failed to send packet.
@comm
	No matter success or failure, its no need to free memory of the outgoing packet. 
	Light rome driver will take care about it.
*/
enum LR_RESULT rtl865x_lightRomeSend(void *pkt, uint32 len, uint16 vid, uint32 port_mask);


/*
@func enum LR_RESULT | rtl865x_lightRomeRegister | Register a Rx call-back function.
@parm enum REG_FLAGS | flags | flags should be either REG_BR_RX_ACTION or REG_RX_ACTION.
@parm void * | func | A call-back function in (int32 (*fun)(struct rtl_pktHdr *)) type.
@rvalue LR_SUCCESS | Successfully register a rx call-back function to the driver.
@rvalue LR_FAILED | Registration failure. 
@comm
	Light Rome Driver provides two models to register a rx call-back function. Call the API with flags=REG_BR_RX_ACTION
	resuilts in lacking layer 2 switching function support in driver. In this case, upper layer module (OS Protocol Stack, generally)
	should take care this function by itself. Otherwise, with flags=REG_RX_ACTION, the layer 2 switching function is supported
	in the driver. Note that, once fail, no packet will be received by upper layer modulel.
*/
enum LR_RESULT rtl865x_lightRomeRegister(enum REG_FLAGS flags, void *fun);


/*
@func enum LR_RESULT | rtl865x_lightRomeInit | Initialize light rome driver and RTL865x ASIC.
@rvalue LR_SUCCESS | Initial success. 
@comm
	Its important to call this API before using the driver. Note taht you can not call this API twice !
*/
enum LR_RESULT rtl865x_lightRomeInit(void);


/*
@func enum LR_RESULT | rtl865x_lightRomeConfig | Configure light rome driver. Create VLAN and Network interface.
@parm struct rtl865x_lrConfig * | lrconfig | 
@rvlaue LR_SUCCESS | Sucessful configuration.
@rvalue LR_INVVID | Invalid VID.
@comm
	struct rtl865x_lrConfig is defined as follows:
	
			ifname:		Layer 3 Network Interface name, eg: eth0, eth1, ppp0...etc,. If it is specified, bothe layer 2 vlan and layer 3
						netwrok interface are created and bound together. It also can be a NULL value. In this case, only a layer 2 VLAN 
						is created.
			isWan:		1 for WAN interface and 0 for LAN interface in a layer 4 mode.
			if_type:		IF_ETHER sets a network interface to be ETHER type. Instead, IF_PPPOE sets a netwrok to be PPPoE type.
						This field is meaningful only when the ifname is specified.
			vid:			VLAN ID to create a vlan.
			memPort:		VLAN member port.
			untagSet:	VLAN untag Set.
			mtu:			MTU.
			mac:		MAC address of the VLAN or network interface.	
	eg1:

	struct rtl865x_lrConfig __lrconfig[] = {
		{ 	"eth0",	 1,   IF_ETHER, 	8, 	   1, 	0x01, 		0x01,		1500, 	{ { 0x00, 0x00, 0xda, 0xcc, 0xcc, 0x08 } }	},
		{	"eth1",	 0,   IF_ETHER,	9,	   1,		0x1e,		0x1e,		1500,	{ { 0x00, 0x00, 0xda, 0xcc, 0xcc, 0x09 } }	},

		LRCONFIG_END,
	}
*/
//enum LR_RESULT rtl865x_lightRomeConfig(struct rtl865x_lrConfig lrconfig[]);
enum LR_RESULT rtl865x_lightRomeConfig(void);

#ifndef CONFIG_RTL865X_CLE
void rtl8651_8185flashCfg(int8 *cmd, uint32 cmdLen);
#endif

extern void rtl865x_updateNetIfTabMac( uint8* ifname, uint8* mac);
extern int32 rtl8651_extDeviceinit(void);
extern void rtl8651_extDeviceInitTimer(void);


