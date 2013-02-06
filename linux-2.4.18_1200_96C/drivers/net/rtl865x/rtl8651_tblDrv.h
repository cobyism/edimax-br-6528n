/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver rtl8651_tblDrv.c
* Abstract : 
* Author : Edward Jin-Ru Chen (jzchen@realtek.com.tw)               
* $Id: rtl8651_tblDrv.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
*
*/

#ifndef RTL8651_TBLDRV_H
#define RTL8651_TBLDRV_H

#include "types.h"

#include "rtl8651_layer2.h"
#include "rtl_errno.h"
#if defined(__KERNEL__) || defined(RTL865X_MODEL_USER) || defined(RTL865X_TEST)
#include "rtl8651_tblDrvProto.h"
#include "mbuf.h"
#endif
#include "rtl8651_debug.h"
#define RTL8651_IPINTF_NAME_LEN		 16
#define RTL8651_NETINTF_NAME_LEN     RTL8651_IPINTF_NAME_LEN
#define RTL8651_MAX_DIALSESSION      2
#define RTL8651_DSID_ALL			RTL8651_MAX_DIALSESSION
#define RTL8651_DSID_SUPPORT	     1

#if 1
#define ENHANCED_HASH1
#undef DebugNaptEHsh1
#endif

#ifndef GET_ARP_FOR_ISP
#define GET_ARP_FOR_ISP
#endif

#ifdef GET_ARP_FOR_ISP
typedef struct rtl8651_extArpEntry_s {
	uint16		vid;
	ipaddr_t		ipAddr;
	ether_addr_t	macAddr;
	uint32		portMask;
	uint32		age;
} rtl8651_extArpEntry_t;
#endif

#ifndef GET_PORTMAPPING_FOR_ISP
#define GET_PORTMAPPING_FOR_ISP
#endif

#ifdef DebugNaptEHsh1
int32 _rtl8651_dumpAllNaptConnection(void);
#endif

#define MODIFY_ACL_MODULE

/*	Enable 865xC MNQueue (implement with output queue) */
#ifdef	CONFIG_RTL865XC
#define	RTL865XC_MNQUEUE_OUTPUTQUEUE	1
#define	RTL865XC_QOS_OUTPUTQUEUE	1
#endif

#ifndef NEW_DOS_RULE
#define NEW_DOS_RULE
#endif

/*
* DEFAULT_DSID tag is mainly for multiple session/multiple wan,
* in old version, we always use zero to indicate the deault session, in the featue,
* _rtl8651_DefaultDialSessionId should be used.
*/
#define RTL8651_DEFAULT_DSID_FIXED 1

/*
* IDLETIMEOUT tag is mainly for session based wantype.
* in route mode, 865x will idletime out even there is some pkt flows when silent time out > 0.
* To fix this bug, we use VLAN + Ratelimit ACL.
*/
#define RTL8651_IDLETIMEOUT_FIXED 1

#define	RTL8651_ENABLETTLMINUSINE		1
/*************************************************/
/*
 * Turn on content filter.
 */
//#define RTL865XB_WEB_CONTENT_HDR_FILTER

#define _RTL_FLOW_CACHE					/*enable Realtek's new flow cache*/
/*************************************************/
/* url filter setting
*/
#define RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT
#define RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT
#define RTL865XB_URLFILTER_TRUSTED_USER /* enable trusted users: do not drop or log by fwdEngine when url filtered */
#define RTL865XB_URLFILTER_LOGMOREINFO

#define RTL8651_NAPT_USR_MAPPING /*Hyking:2006-08-09:enable new naptFlows:napt_usr_mapping module.*/

   
/*************************************************/
/*
 * If defined CACHE_FIND_NAPT_RESULT, ROME driver will caches the result of getOutboundNaptEntry.
 * If not defined, ROME driver do not cache.
 */
#undef CACHE_FIND_NAPT_RESULT


/*************************************************/
/*
 * If defined FLUSH_NAPT_CONNECTION_WHEN_MORE_THAN_LIMIT,
 *   addNaptConnection() will delete an existed connection 
 *   when connection number exceeds the connection limit.
 * If not defined, ROME driver does not delete any connection.
 */
#define FLUSH_NAPT_CONNECTION_WHEN_MORE_THAN_LIMIT


/*************************************************/
/*
 * If defined FLUSH_NAPT_OLDEST_CONNECTION2,
 *    _rtl8651_softwareNaptAging() will record the connection with minimum age value.
 *    So that we can delete connection immediately when addNaptConnection() overflow.
 * If not defined, ROME driver will delete a connection that created first (FIFO).
 */
#define FLUSH_NAPT_OLDEST_CONNECTION2


/*************************************************/
/*
 * ALLOW_SP is defined for _rtl8651_findNaptConnection() and _rtl8651_findNaptConnectionByExtIpPort().
 * This is a bitmask for isTcp parameter.
 * Defined non-zero can enable server port functionality of NAPT.
 * Defined zero can roll-back to original code.
 */
#define ALLOW_SP (1<<1)


/*************************************************/
/*
 * SERVERPORT_PARK_TCPUDP is defined for ASIC Server Port Table.
 * In ASIC design, Server Port Table will forward packets no matter TCP or UDP.
 * However, it will cause error in test tools, such as CDRouter.
 * If defined SERVERPORT_PARK_TCPUDP, tblDrv uses ASIC Server Port Table only when both TCP and UDP connections are added.
 *    Otherwise, fwdEng uses HASH2 to handle TCP/UDP connections.
 * If not defined, fwdEng uses ASIC Server Port Table to accelerate even only TCP or UDP connection is added.
 */
#define SERVERPORT_PARK_TCPUDP


/*************************************************/
/*
 * ADD_FINITE_HASH2_SP is defined for HASH2 that simulates Server Port Function.
 * We can use HASH2 entry just like Server Port do.
 * If defined ADD_FINITE_HASH2_SP, addNaptConnection() adds the 5-tuple connection with finite age (will be deleted).
 * If not defined, addNaptConnection() adds the 5-tuple connection that will never be aged out.
 */
#define ADD_FINITE_HASH2_SP


/*************************************************/
/*
 * PPPOE_PASSTHRU_NEW_MODEL is defined for PPPoE Passthru.
 * If defined 0, the original model is compiled (complex model).
 * If defined 1, new model is forwarding packet according to PPPoE Passthru VLAN.
 */
#define PPPOE_PASSTHRU_NEW_MODEL 1


/*************************************************/
/*
 * TRAP_TCP_SYN_FIN_RST is defined to trap TCP SYN/FIN/RST packets.
 * If not defined, the original model is only trapping SYN packet at WAN side.
 * If defined, new model is trapping SYN/FIN/RST packets at both WAN/LAN side.
 */
#undef TRAP_TCP_SYN_FIN_RST

/*************************************************/
/*
 * RTL865xB_MCAST_CUTCD_TTL_PATCH is patch 865xB Cut C/D
 * multicast WAN to LAN forwarding problem.
 * If not defined, the Multicast forwarding for 865xB Cut C/D would
 * have IP checksum fail.
 * Note: for 865xB Cut B , 865x, ... etc. It's ok.
 */
#ifndef CONFIG_RTL865XC
#define RTL865xB_MCAST_CUTCD_TTL_PATCH
#endif

/*************************************************/
/*
 * SUPPORT_HASH3 is defined to support HASH3.
 * If not defined, we dont support HASH3.
 * If defined, we support HASH3.
 */
#ifndef RTL865X_TEST
#undef SUPPORT_HASH3
#endif

/*************************************************/
/*
 * RTL8651_ALG_QOS is defined to implement ALG QoS control
 * added by yuhao (Realsil), 2005-12-20
 */
#define RTL8651_ALG_QOS

/*************************************************/
/*
 *    RTL8651_DSCP_QOS is defined to support dscp remarking
 *   added by luyang(Realsil),2005-12-22
 */
#define RTL8651_DSCP_QOS

/*************************************************/
/*
 *  TCP_SUPPORT_NAT_TYPE  
 *  
 *  The RFC 3489 (STUN) only defines UDP behavior of gateway.
 *  To enable TCP support STUN, define the following.
 */
#undef TCP_SUPPORT_NAT_TYPE

/*************************************************/
/*
*turn on route entry referrence count 
*/
#undef RTL865X_ROUTE_REFERRENCE_CONUT

typedef struct rtl8651_tblDrvInitPara_s {
	uint32	macAddressDbSize;
	uint32	filterDbSize;
	uint32	networkIntfTableSize;
	uint32	ipIntfEntryNumber;
	uint32	ipIntfIpAddrNumber;
	uint32	arpTableSize;
	uint32	routingTableSize;
	uint32	natSize;
	uint32	natCtlSize;
	uint32	naptServerPortEntryNumber;
	uint32	naptTcpUdpFlowSize;
	uint32	naptIcmpFlowSize;
	uint32	aclSize;
	uint32	algSize;
	uint32 	urlfilterDBSize;
	uint32 	urlfilterStringLen;
	uint32	pathfilterStringLen;
	uint32	urlPktfilterDBSize;	/* for url filter extension: filter for IP/MAC....etc */
	uint32     dialSessionNumber; /* supported number of dial sessions. valid dsid: 0,1,...,(dialSessionNumber-1) */
	uint32     dmzHostsNum;
	uint32	policyRules;
	uint32	dynamicPolicy;
	uint32	pptpSize;
	uint32	l2tpSize;
	uint32	pppoePassthruSize;
	uint32	protocolBasedNatSize;
	uint32	mCastTblSize;
	uint32	macAccessControllSize;
	uint32	ipUnnumberSize;
#ifdef CONFIG_RTL865X_IPSEC
	uint32	ipSecTunnelNumber;	/* supported number of ipsec tunnels. */
#endif
#ifdef RTL8651_ALG_QOS
	uint32	algQosTblSize;
#endif
#ifdef RTL865XB_URLFILTER_TRUSTED_USER
	uint32	urlfilterTrustedUserEntryCnt;
#endif
	uint32	advRouteEntryCnt;
	uint32	advDynamicRouteEntryCnt;

#ifdef RTL865XB_WEB_CONTENT_HDR_FILTER
	uint32	contentfilterStringLen;
	uint32	contentfilterEntryNum;
	uint32	httpHdrfilterStringLen;
	uint32	httpHdrfilterEntryNum;
#endif
#ifdef RTL8651_NAPT_USR_MAPPING
	uint32	naptUsrMappingEntryNum;
	uint32	naptUsrMappingTypePriorityNum;
#endif
	uint32	naptUsrMappingIpFilterNum;

#ifdef CONFIG_RTL865XC
	uint32	externalPHYProperty;
	uint32	externalPHYId[RTL8651_MAC_NUMBER];
#endif

} rtl8651_tblDrvInitPara_t;

/* ======================== Definitions for ASIC driver initial parameters  ======================== */

/*	external PHY Property :
		=> It sould be SYNC with data in "rtl8651_tblAsic_InitPara_t" */
#define RTL8651_TBLDRV_EXTPHYPROPERTY_PORT1234_RTL8212			(1 << 0)
#define RTL8651_TBLDRV_EXTPHYPROPERTY_PORT5_RTL8211B			(1 << 1)

/* =============================================================================== */

#if defined(RTL865XC_MNQUEUE_OUTPUTQUEUE)
typedef struct rtl8651_tblDrv_mnqueue_sys_s {
	uint8	   queueNum;
	uint32 wanBandwidth;
	uint32 defaultQueueRatio;
} rtl8651_tblDrv_mnqueue_sys_t;
#else
typedef struct rtl8651_tblDrv_mnqueue_sys_s {
	uint32 wanBandwidth;
	uint32 mVRate, mEVRate, nVRate;
	uint32 nAvailableRate;
	uint32 eagerNQueueNumber, nQueueNumber;
} rtl8651_tblDrv_mnqueue_sys_t;
#endif


#if  defined(RTL865XC_QOS_OUTPUTQUEUE)
typedef enum rtl865xC_qos_opLayer_s
{
	QOS_OP_LAYER2 = 0x1,
	QOS_OP_LAYER3 = 0x2,
	QOS_OP_LAYER4 = 0x4,
}rtl865xC_qos_opLayer_t;

typedef struct rtl8651_tblDrv_qos_sys_s {
	uint8	enable;
	uint8	queueNum;
	uint8	opLayer;
	uint8	PriDecArray[RTL8651_PRIORITYBASE_NUM];
	uint16	ppr;							/* default: disable ppr control */
	uint16	burstSize;					/* default: disable burst */
	uint32 wanBandwidth;
	uint32 defaultQueueRatio;
	uint32	wanport;
} rtl8651_tblDrv_qos_sys_t;

#endif

#if defined(RTL865XC_MNQUEUE_OUTPUTQUEUE) || defined(RTL865XC_QOS_OUTPUTQUEUE)
typedef struct rtl865xC_outputQueuePara_s {

	uint32	ifg;							/* default: Bandwidth Control Include/exclude Preamble & IFG */
	uint32	gap;							/* default: Per Queue Physical Length Gap = 20 */
	uint32	drop;						/* default: Descriptor Run Out Threshold = 500 */

	uint32	systemSBFCOFF;				/*System shared buffer flow control turn off threshold*/
	uint32	systemSBFCON;				/*System shared buffer flow control turn on threshold*/

	uint32	systemFCOFF;				/* system flow control turn off threshold */
	uint32	systemFCON;					/* system flow control turn on threshold */

	uint32	portFCOFF;					/* port base flow control turn off threshold */
	uint32	portFCON;					/* port base flow control turn on threshold */	

	uint32	queueDescFCOFF;				/* Queue-Descriptor=Based Flow Control turn off Threshold  */
	uint32	queueDescFCON;				/* Queue-Descriptor=Based Flow Control turn on Threshold  */

	uint32	queuePktFCOFF;				/* Queue-Packet=Based Flow Control turn off Threshold  */
	uint32	queuePktFCON;				/* Queue-Packet=Based Flow Control turn on Threshold  */
}	rtl865xC_outputQueuePara_t;
#endif
/*************************************************/
/*
 * External data structure declaration for Advanced Routing.
 * It includes Policy route and Demand route.
 * chenyl (2006-2-7)
 *	We just rewrite this module for better performance,
 *    code readability and bug fix. And we just keep backward
 *	compatibility exist policy route and demand route.
 *
 */

/* For Session definition */
extern uint32 _rtl8651_DefaultDialSessionId;

/* bit-mask */
#define ADVRT_PROPERTY_MASK	0xf000
#define ADVRT_RULETYPE_MASK	0x0fff

/* advance route property */
#define DYNAMIC_ADV_ROUTE		0x1000
#define STATIC_ADV_ROUTE		0x2000

#define DYNAMIC_POLICY_ROUTE	DYNAMIC_ADV_ROUTE
#define STATIC_POLICY_ROUTE	STATIC_ADV_ROUTE

#define DYNAMIC_DEMAND_ROUTE	DYNAMIC_ADV_ROUTE
#define STATIC_DEMAND_ROUTE	STATIC_ADV_ROUTE

/* advance route rule type */
#define TRIGGER_DSTIP			0x01
#define TRIGGER_DSTTCPPORT		0x02
#define TRIGGER_SRCIP			0x03
#define TRIGGER_DSTUDPPORT	0x04
#define TRIGGER_SRCGLOBALIP	0x05
#define TRIGGER_SIPDIPPAIR		0x06
#define TRIGGER_TCP				0x07
#define TRIGGER_UDP				0x08
#define TRIGGER_IP				0x09

typedef struct rtl8651_tblDrvPolicyRoute_s
{
	uint32 type;
	union
	{
		/*
			For TRIGGER_DSTTCPPORT/TRIGGER_DSTUDPPORT
		*/
		struct
		{
			uint16 start;
			uint16 end;
		} dport;

		/*
			For TRIGGER_SIPDIPPAIR
		*/
		struct
		{
			ipaddr_t sip;
			ipaddr_t dip;
		} sipDipPair;		/* For a specific source/destination IP pair */

		/*
			ACL liked rule:

			For TRIGGER_TCP/TRIGGER_UDP/TRIGGER_IP

			===================================================================
		*/
		struct
		{
			struct
			{
				ipaddr_t _sipStart;
				ipaddr_t _sipEnd;
				ipaddr_t _dipStart;
				ipaddr_t _dipEnd;

				uint8 _ipProto;
			} _l3;

			union
			{
				struct
				{
					uint16 _sportStart;
					uint16 _sportEnd;
					uint16 _dportStart;
					uint16 _dportEnd;
				} _tcpUdp;
			} _l4;
		} _aclLikedRule;

		/* TRIGGER_TCP */
		#define rtl8651_advRt_tcpRule_sipStart	_aclLikedRule._l3._sipStart
		#define rtl8651_advRt_tcpRule_sipEnd		_aclLikedRule._l3._sipEnd
		#define rtl8651_advRt_tcpRule_dipStart	_aclLikedRule._l3._dipStart
		#define rtl8651_advRt_tcpRule_dipEnd		_aclLikedRule._l3._dipEnd
		#define rtl8651_advRt_tcpRule_sportStart	_aclLikedRule._l4._tcpUdp._sportStart
		#define rtl8651_advRt_tcpRule_sportEnd	_aclLikedRule._l4._tcpUdp._sportEnd
		#define rtl8651_advRt_tcpRule_dportStart	_aclLikedRule._l4._tcpUdp._dportStart
		#define rtl8651_advRt_tcpRule_dportEnd	_aclLikedRule._l4._tcpUdp._dportEnd
		/* TRIGGER_UDP */
		#define rtl8651_advRt_udpRule_sipStart	_aclLikedRule._l3._sipStart
		#define rtl8651_advRt_udpRule_sipEnd		_aclLikedRule._l3._sipEnd
		#define rtl8651_advRt_udpRule_dipStart	_aclLikedRule._l3._dipStart
		#define rtl8651_advRt_udpRule_dipEnd	_aclLikedRule._l3._dipEnd
		#define rtl8651_advRt_udpRule_sportStart	_aclLikedRule._l4._tcpUdp._sportStart
		#define rtl8651_advRt_udpRule_sportEnd	_aclLikedRule._l4._tcpUdp._sportEnd
		#define rtl8651_advRt_udpRule_dportStart	_aclLikedRule._l4._tcpUdp._dportStart
		#define rtl8651_advRt_udpRule_dportEnd	_aclLikedRule._l4._tcpUdp._dportEnd
		/* TRIGGER_IP */
		#define rtl8651_advRt_ipRule_sipStart		_aclLikedRule._l3._sipStart
		#define rtl8651_advRt_ipRule_sipEnd		_aclLikedRule._l3._sipEnd
		#define rtl8651_advRt_ipRule_dipStart		_aclLikedRule._l3._dipStart
		#define rtl8651_advRt_ipRule_dipEnd		_aclLikedRule._l3._dipEnd
		#define rtl8651_advRt_ipRule_ipProto		_aclLikedRule._l3._ipProto

	/* ======================================================================= */

	} trigger;

	/*
		For TRIGGER_DSTIP/TRIGGER_SRCIP/TRIGGER_SRCGLOBALIP
	*/
	ipaddr_t ip_start;
	ipaddr_t ip_end;

	/*
		For TRIGGER_DSTIP/TRIGGER_SRCIP	: (0) - widecard
	*/
	uint8 ip_proto;

	/* ============================================================
		Policy route specific fields
	    ============================================================ */
	/*
		For All types - to indicate the ALIAS IP/Session for this policy route.
	*/
	ipaddr_t ip_alias;
	/*
		For All dynamic -	to indicate the AGE of dynamic entry. 0 indicate this entry would use system
						pre-defined age number (RTL8651_DPOLICYRT_EXT_AGE) as its age.
	*/
	int32 age;
} rtl8651_tblDrvPolicyRoute_t;

typedef struct rtl8651_tblDrvDemandRoute_s
{
	uint32 type;
	union
	{
		/*
			For TRIGGER_DSTTCPPORT/TRIGGER_DSTUDPPORT
		*/
		struct
		{
			uint16 start;
			uint16 end;
		} dport;

		/*
			For TRIGGER_SIPDIPPAIR
		*/
		struct
		{
			ipaddr_t sip;
			ipaddr_t dip;
		} sipDipPair;		/* For a specific source/destination IP pair */

		/*
			ACL liked rule:

			For TRIGGER_TCP/TRIGGER_UDP/TRIGGER_IP

			===================================================================
		*/

		/*
			NOTE: Please refer the MACRO in rtl8651_tblDrvPolicyRoute_t !!
		*/

		struct
		{
			struct
			{
				ipaddr_t _sipStart;
				ipaddr_t _sipEnd;
				ipaddr_t _dipStart;
				ipaddr_t _dipEnd;

				uint8 _ipProto;
			} _l3;

			union
			{
				struct
				{
					uint16 _sportStart;
					uint16 _sportEnd;
					uint16 _dportStart;
					uint16 _dportEnd;
				} _tcpUdp;
			} _l4;
		} _aclLikedRule;

	/* ======================================================================= */

	} trigger;

	/*
		For TRIGGER_DSTIP/TRIGGER_SRCIP/TRIGGER_SRCGLOBALIP
	*/
	ipaddr_t ip_start;
	ipaddr_t ip_end;

	/*
		For TRIGGER_DSTIP/TRIGGER_SRCIP	: (0) - widecard
	*/
	uint8 ip_proto;

	/* ============================================================
		Demand route specific fields
	    ============================================================ */
	uint32 identity;
	int32 (*p_handler)(uint32);	/* call-back handler */
	/*
		For All dynamic -	to indicate the AGE of dynamic entry. 0 indicate this entry would use system
						pre-defined age number (RTL8651_DPOLICYRT_EXT_AGE) as its age.
	*/
	int32 age;

} rtl8651_tblDrvDemandRoute_t;

/*************************************************/
/*
 * Policy based QoS.
 *
 */

typedef struct rtl8651_tblDrvPolicy_s {
	uint32 		proto;					
	ipaddr_t	sip, sipMask,			
				dip, dipMask;
	uint16		sport_start, sport_end,
				dport_start, dport_end;
	ipaddr_t	naptIp;
	uint32 	advRtType;	/*converted from demand route or policy route*/

	struct _rtl8651_tblDrvAdvRoute_s * advRoute_t;
	//int32 hwAcclrt;		
						/*  hw accelerating related information.
						  *		bit 0: this rule is converted from advRoute rule.
						  *		bit 1: this rule has added to HW acl table.
						  */


//	uint32		sessionId;
} rtl8651_tblDrvPolicy_t;

/* proto: */
#define RTL8651_POLICY_IP		0x01
#define RTL8651_POLICY_TCP		0x02
#define RTL8651_POLICY_UDP		0x03


typedef struct rtl8651_tblDrvAclRule_s {
	union {
		/* MAC ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _typeLen, _typeLenMask;
		} MAC; 
		/* IFSEL ACL rule */
		struct {
			uint8 _gidxSel;
		} IFSEL; 
		/* IP Group ACL rule */
		struct {
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint8 _tos, _tosMask;
			union {
				/* IP ACL rle */
				struct {
					uint8 _proto, _protoMask, _flagMask;// flag & flagMask only last 3-bit is meaning ful
#if 1 
//chhuang: #ifdef RTL8650B
					uint32 _FOP:1, _FOM:1, _httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
#endif /* RTL8650B */
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
				/* ICMP ACL rule */
				struct {
					uint8 _type, _typeMask, _code, _codeMask;
				} icmp; 
				/* IGMP ACL rule */
				struct {
					uint8 _type, _typeMask;
				} igmp; 
				/* TCP ACL rule */
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
				/* UDP ACL rule */
				struct {					
					ether_addr_t _l2srcMac, _l2srcMacMask;
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;					
				}udp; 
			}is;
		}L3L4; 
#if 1 
//chhuang :#ifdef RTL8650B
		/* Source filter ACL rule */
		struct {
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _srcPort, _srcPortMask;
			uint16 _srcVlanIdx, _srcVlanIdxMask;
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			uint16 _srcPortUpperBound, _srcPortLowerBound;
			uint32  _ignoreL4:1, //L3 rule
				  	_ignoreL3L4:1; //L2 rule
		} SRCFILTER;
		/* Destination filter ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			uint16 _vlanIdx, _vlanIdxMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint16 _dstPortUpperBound, _dstPortLowerBound;
			uint32 _ignoreL4:1, //L3 rule
				  	 _ignoreL3L4:1; //L2 rule
		} DSTFILTER;
#endif /* RTL8650B */

#if RTL865XC_QOS_OUTPUTQUEUE
		struct {
			uint8	vlanTagPri;
		} VLANTAG;
#endif
	}un_ty;

	uint32	ruleType_:5;
	uint32	actionType_:4;
#if 1	/* RTL8650B */
	uint32  	pktOpApp:3;
#endif	/* RTL8650B */
	uint32	isEgressRateLimitRule_: 1;
	uint32	naptProcessType:4;
	uint32	naptProcessDirection:2;
	uint32	matchType_;
} rtl8651_tblDrvAclRule_t;



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
#define srcIpAddrUB_				un_ty.L3L4._srcIpAddr
#define srcIpAddrLB_				un_ty.L3L4._srcIpAddrMask
#define dstIpAddr_				un_ty.L3L4._dstIpAddr
#define dstIpAddrMask_			un_ty.L3L4._dstIpAddrMask
#define dstIpAddrUB_				un_ty.L3L4._dstIpAddr
#define dstIpAddrLB_				un_ty.L3L4._dstIpAddrMask
#define tos_					un_ty.L3L4._tos
#define tosMask_				un_ty.L3L4._tosMask
/* IP Rrange */
/*Hyking:Asic use Addr to store Upper address
	and use Mask to store Lower address
*/
#if 0
#define srcIpAddrStart_			un_ty.L3L4._srcIpAddr
#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddrMask
#define dstIpAddrStart_			un_ty.L3L4._dstIpAddr
#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddrMask
#else
// have defined in swCore.h
//#define srcIpAddrStart_			un_ty.L3L4._srcIpAddrMask
//#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddr
//#define dstIpAddrStart_			un_ty.L3L4._dstIpAddrMask
//#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddr
#endif

/* IP ACL Rule Definition */
#define ipProto_				un_ty.L3L4.is.ip._proto
#define ipProtoMask_			un_ty.L3L4.is.ip._protoMask
#define ipFlagMask_			un_ty.L3L4.is.ip._flagMask
#if 1 //chhuang: #ifdef RTL8650B
#define ipFOP_      				un_ty.L3L4.is.ip._FOP
#define ipFOM_      				un_ty.L3L4.is.ip._FOM
#define ipHttpFilter_      			un_ty.L3L4.is.ip._httpFilter
#define ipHttpFilterM_			un_ty.L3L4.is.ip._httpFilterM
#define ipIdentSrcDstIp_   		un_ty.L3L4.is.ip._identSrcDstIp
#define ipIdentSrcDstIpM_		un_ty.L3L4.is.ip._identSrcDstIpM
#endif /* RTL8650B */
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
#define tcpl2srcMac_				un_ty.L3L4.is.tcp._l2srcMac		// for srcMac & destPort ACL rule
#define tcpl2srcMacMask_			un_ty.L3L4.is.tcp._l2srcMacMask
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
#define udpl2srcMac_				un_ty.L3L4.is.udp._l2srcMac		// for srcMac & destPort ACL rule
#define udpl2srcMacMask_		un_ty.L3L4.is.udp._l2srcMacMask
#define udpSrcPortUB_			un_ty.L3L4.is.udp._srcPortUpperBound
#define udpSrcPortLB_			un_ty.L3L4.is.udp._srcPortLowerBound
#define udpDstPortUB_			un_ty.L3L4.is.udp._dstPortUpperBound
#define udpDstPortLB_			un_ty.L3L4.is.udp._dstPortLowerBound

#if 1 //chhuang: #ifdef RTL8650B
/* Source Filter ACL Rule Definition */
#define srcFilterMac_				un_ty.SRCFILTER._srcMac
#define srcFilterMacMask_		un_ty.SRCFILTER._srcMacMask
#define srcFilterPort_				un_ty.SRCFILTER._srcPort
#define srcFilterPortMask_		un_ty.SRCFILTER._srcPortMask
#define srcFilterVlanIdx_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanId_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanIdxMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterVlanIdMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterIpAddr_			un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrMask_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterIpAddrUB_		un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrLB_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterPortUpperBound_	un_ty.SRCFILTER._srcPortUpperBound
#define srcFilterPortLowerBound_	un_ty.SRCFILTER._srcPortLowerBound
#define srcFilterIgnoreL3L4_		un_ty.SRCFILTER._ignoreL3L4
#define srcFilterIgnoreL4_		un_ty.SRCFILTER._ignoreL4

/* Destination Filter ACL Rule Definition */
#define dstFilterMac_				un_ty.DSTFILTER._dstMac
#define dstFilterMacMask_		un_ty.DSTFILTER._dstMacMask
#define dstFilterVlanIdx_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdxMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterVlanId_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterIpAddr_			un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrMask_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortUpperBound_	un_ty.DSTFILTER._dstPortUpperBound
#define dstFilterIpAddrUB_		un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrLB_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortLowerBound_	un_ty.DSTFILTER._dstPortLowerBound
#define dstFilterIgnoreL3L4_		un_ty.DSTFILTER._ignoreL3L4
#define dstFilterIgnoreL4_		un_ty.DSTFILTER._ignoreL4
#endif /* RTL8650B */

#if RTL865XC_QOS_OUTPUTQUEUE
#define vlanTagPri_			un_ty.VLANTAG.vlanTagPri
#endif


/* ACL Rule Action type Definition */
#define RTL8651_ACL_PERMIT			0x01
#define RTL8651_ACL_DROP				0x02
#define RTL8651_ACL_CPU				0x03
#define RTL8651_ACL_DROP_LOG			0x04
#define RTL8651_ACL_DROP_NOTIFY		0x05
#define RTL8651_ACL_L34_DROP			0x06	/* special for default ACL rule */
#define RTL8651_ACL_DEFAULT_REDIRECT		0x08
#define RTL8651_ACL_PRIORITY				0x0f


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
#define RTL8651_ACL_SRCFILTER				0x07
#define RTL8651_ACL_DSTFILTER				0x08



/* For PktOpApp */
#define RTL8651_ACLTBL_BACKWARD_COMPATIBLE	0 /* For backward compatible */
#define RTL865XC_ACLTBL_ALL_LAYER			RTL8651_ACLTBL_BACKWARD_COMPATIBLE
#define RTL8651_ACLTBL_ONLY_L2				1 /* Only for L2 switch */
#define RTL8651_ACLTBL_ONLY_L3				2 /* Only for L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_L2_AND_L3			3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_ONLY_L4				4 /* Only for L4 translation packets */
#define RTL8651_ACLTBL_L3_AND_L4			6 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL8651_ACLTBL_NOOP				7 /* No operation. Don't apply this rule. */

/*	for NAPT process type	*/
/* it's value associated with the default napt process priority */
/* type must = priority */
/* 4 bits */
#define	RTL8651_ACL_L4_SERVERPORT	0x1 /* add flow for serverport match	*/
#define	RTL8651_ACL_L4_TRIGGERPORT	0x2	/* add flow for trigger port match	*/
#define	RTL8651_ACL_L4_UPNP			0x3	/* add flow for upnp match	*/
#define	RTL8651_ACL_L4_DMZ			0x5	/* add flow for dmz match	*/
#define	RTL8651_ACL_L4_GETEXTIP		0x6	/* add flow for get ip	*/

#define	RTL8651_ACL_L4_ICMP			0Xf		/* add flow for get ip	*/

/*	for Napt process direction	*/
/* 2 bits */
#define	RTL8651_ACL_L4_INBOUND			0x1
#define	RTL8651_ACL_L4_OUTBOUND		0x2


/* User ACL rule match type definition */
#define RTL8651_ACL_GENERIC		0x00
#define RTL8651_ACL_L4NEWFLOW		0x01




typedef struct rtl865x_tblDrv_basicCounterParam_s {
	uint32	txPackets;
	uint32	txBytes;
	uint32	rxPackets;
	uint32	rxBytes;
	uint32	rxErrors;
	uint32	drops;
} rtl865x_tblDrv_basicCounterParam_t;


/*===============================================================================
 * Exported API categorized by usage
 *===============================================================================*/

// system 
void  rtl8651_realtimeSchedule(void);
void rtl8651_timeUpdate(uint32 dummy);
int32 rtl8651_enableNaptAutoAdd(int8 enable);

//acl
int32 rtl8651a_addAclRule(uint32 SessionID,int8 * ifName, int8 Ingress, rtl8651_tblDrvAclRule_t *rule);
#define rtl8651_addAclRule(ifName,Ingress,rule) rtl8651a_addAclRule(0,ifName,Ingress,rule) 
int32 rtl8651a_delAclRule(uint32 SessionID,int8 *ifName, int8 Ingress, rtl8651_tblDrvAclRule_t *rule);
#define rtl8651_delAclRule(ifName,Ingress,rule) rtl8651a_delAclRule(0,ifName,Ingress,rule)
int32 rtl8651a_addAclRuleExt(uint32 sessionId, int8 *ifName, int8 Ingress, rtl8651_tblDrvAclRule_t *rule);
#define rtl8651_addAclRuleExt(ifName, Ingress, rule) rtl8651a_addAclRuleExt(0, ifName, Ingress, rule)
int32 rtl8651a_delAclRuleExt(uint32 sessionID,int8 *ifName, int8 Ingress, rtl8651_tblDrvAclRule_t *rule);
#define rtl8651_delAclRuleExt(ifName, Ingress, rule) rtl8651a_delAclRuleExt(0, ifName, Ingress, rule)


int32 rtl8651a_flushAclRule(uint32 SessionID,int8 *ifName, int8 Ingress);
#define rtl8651_flushAclRule(ifName,Ingress) rtl8651a_flushAclRule(0,ifName,Ingress)
int32 rtl8651a_setDefaultAcl(uint32 SessionID,int8 *ifName, int8 defAction);
#define rtl8651_setDefaultAcl(ifName,defAction) rtl8651a_setDefaultAcl(0,ifName,defAction)

//pppoe 
//add a PPPoE object, an ASIC PPPoE  entry allocated here. Session ID should be set later.
int32 rtl8651_addPppoeSession(uint32 SessionID, uint16 vid);
int32 rtl8651_delPppoeSession(uint32 SessionID);
//config properties of a PPPoE interface. PPPoE asic entry not written to ASIC until this function called.
int32 rtl8651_setPppoeSessionProperty(uint32 SessionID, uint16 sid, ether_addr_t * macAddr, uint32 port);
#if     RTL8651_IDLETIMEOUT_FIXED
int32 rtl8651_setMultiPppoeSessionVlanRange(uint16 vlanIdStart, uint16 vlanIdEnd);
uint8 rtl8651_setMultiPppoeSessionStatus(uint8 dsid,uint8 sessionStatus);
#endif

//linear scan routing entry to find which route refers to this PPPoE session. Both located routing entry 
//and PPPoE entry got reset.
int32 rtl8651_resetPppoeSessionProperty(uint32 SessionID);
int32 rtl8651_getPppoeSessionMac(uint32 SessionID, ether_addr_t * macAddr);
int32 rtl8651_bindPppoeSession(ipaddr_t ipaddr, uint32 SessionID);
int32 rtl8651_setPppoeSessionHangUp(uint32 SessionID, int32 enable, uint32 sec, int32 (*p_callBack)(uint32));

uint32 * rtl8651_getNextPppoeSession(uint32 * curSessionId, uint32 *retSessionId);
int32 rtl8651_getPppoeCounter(uint32 curSessionId, rtl865x_tblDrv_basicCounterParam_t * counter);
int32 rtl8651_setPppoeSessionHangUp(uint32 SessionID, int32 enable, uint32 sec, int32 (*p_callBack)(uint32)) ;
int32 rtl8651_setPppoeDefaultSessionId(uint32 SessionID);
int32 rtl8651_addDemandRoute(	rtl8651_tblDrvDemandRoute_t * addDemandRt_p,
									int32 identity,
									int32(*p_callBack)(uint32));
int32 rtl8651_delDemandRoute(rtl8651_tblDrvDemandRoute_t *delDemandRt_p);
int32 rtl8651_flushDemandRoute(int32 identity);
int32 rtl8651_flushPolicyRoute(ipaddr_t aliasIp);
int32 rtl8651_addPolicyRoute(rtl8651_tblDrvPolicyRoute_t *addPolicyRt_p);
int32 rtl8651_delPolicyRoute(rtl8651_tblDrvPolicyRoute_t *delPolicyRt_p);

//interface 
//network interface: an IP broadcast domain. This is a pseudo layer between IP interface and VLAN 
//network interface is "external" if at least one NAT host, or one NAPT address, or one DMZ host refers to this network interface, 
//otherwise it is an "internal " interface by default.
int32 rtl8651_addNetworkIntf(int8 *ifName); //a route will be created  for each IP interface added
//If a routing entry/DMZ host/NAT host refers to the network interface to be deleted, delete operation is forbidden. 
int32 rtl8651_delNetworkIntf(int8 *ifName);
//specify this network interface''s default ACL action is drop or permit for this vlan
int32 rtl8651_setNetIfAclMismatchDrop(int8 *ifName, int8 isDrop); //chhuang
int32 rtl8651_addExtNetworkInterface(int8 * ifName); //make this network interface  for NAPT
//free ExtIPTable table entries occupied and makes this interfafe internal
int32 rtl8651_delExtNetworkInterface(int8 * ifName);

//Add a DMZ network makes this internal interface external.
//a DMZ host won't be written to ExtIPTable unless its ARP is learned.
int32 rtl8651_addDmzNetworkInterface(int8 *ifName);
int32 rtl8651_delDmzNetworkInterface(int8 *ifName);
int32 rtl8651_specifyNetworkIntfLinkLayerType(int8 * ifName, uint32 llType, uint16 vid);
int32 rtl8651_removeNetworkIntfLinkLayerType(int8 * ifName);

//IP interface: an IP subnet. multiple IP subnets can bind on the same network interface
int32 rtl8651_addIpIntf(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask);//
int32 rtl8651_delIpIntf(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask);

//arp 
int32 rtl8651_addArp(ipaddr_t ipAddr, ether_addr_t * macAddr, int8 * ifName, uint32 port);
int32 rtl8651_delArp(ipaddr_t ipAddr);
int32 rtl8651_setArpAgingTime(uint16 agTime);

//nat 
int32 rtl8651_addNatMapping(ipaddr_t extIpAddr, ipaddr_t intIpAddr); //....for NAT addresses
int32 rtl8651_delNatMapping(ipaddr_t extIpAddr, ipaddr_t intIpAddr);

//local-server 
int32 rtl8651_addLocalServer(ipaddr_t ipAddr); // No NAT between Private IP and Local Public IPs
int32 rtl8651_delLocalServer(ipaddr_t ipAddr);

//route 
int32 rtl8651_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, int8 * ifName, ipaddr_t nextHop);
int32 rtl8651_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask);

//napt
int32 rtl8651_addNaptMapping(ipaddr_t extIpAddr);
int32 rtl8651_delNaptMapping(ipaddr_t extIpAddr);
int32 rtl8651_setNaptIcmpTimeout(uint32 timeout);
int32 rtl8651_setNaptIcmpFastTimeout(uint32 timeout);
int32 rtl8651_setNaptUdpTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpLongTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpMediumTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpFastTimeout(uint32 timeout);
int32 rtl8651_setTriggerConnectionTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpUdpPortRange(uint16 start, uint16 end);
int32 rtl8651_is4WayHashEnable(uint8 *isEnable);
int32 rtl8651_enable4WayHash(uint8 isEnable);
int32 rtl8651_addDriverNaptMapping(ipaddr_t naptip);
#if defined(CONFIG_RTL865XC) && defined(ENHANCED_HASH1)
int32 rtl8651_enableEnhancedHash1(void);
int32 rtl8651_disableEnhancedHash1(void);
#endif



//napt server port
int32 rtl8651a_addNaptServerPortMapping(uint32 SessionID,int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort);
#define rtl8651_addNaptServerPortMapping(isTcp,extIpAddr,extPort,intIpAddr,intPort) rtl8651a_addNaptServerPortMapping(0,isTcp,extIpAddr,extPort,intIpAddr,intPort)
int32 rtl8651a_delNaptServerPortMapping(uint32 SessionID,int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort);
#define rtl8651_delNaptServerPortMapping(isTcp,extIpAddr,extPort,intIpAddr,intPort) rtl8651a_delNaptServerPortMapping(0,isTcp,extIpAddr,extPort,intIpAddr,intPort)
int32 rtl8651a_addNaptServerPortRange(uint32 SessionID,int8 isTcp, ipaddr_t extIp, uint16 extPort, ipaddr_t intIp, uint16 intPort, uint16 nCount);
#define rtl8651_addNaptServerPortRange(isTcp,extIp,extPort,intIp,intPort,nCount) rtl8651a_addNaptServerPortRange(0,isTcp,extIp,extPort,intIp,intPort,nCount)
int32 rtl8651a_delNaptServerPortRange(uint32 SessionID,int8 isTcp, ipaddr_t extIp, uint16 extPort, ipaddr_t intIp, uint16 intPort, uint16 nCount);
#define rtl8651_delNaptServerPortRange(isTcp,extIp,extPort,intIp,intPort,nCount) rtl8651a_delNaptServerPortRange(0,isTcp,extIp,extPort,intIp,intPort,nCount)
int32 rtl8651_flushNaptServerPortbyExtIp(ipaddr_t extIp);



enum NAT_TYPE
{
	NAT_TYPE_NAIVE = 0, 
	NAT_TYPE_SYMMETRIC, 
	NAT_TYPE_PORT_RESTRICTED_CONE, 
	NAT_TYPE_RESTRICTED_CONE, 
	NAT_TYPE_FULL_CONE,
};
int32 rtl8651_getNatType( enum NAT_TYPE *type );
int32 rtl8651_setNatType( enum NAT_TYPE type );

int32 rtl8651_flushAllNaptConnections(void);
int32 rtl8651_addNaptConnection(int16 assigned, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort);
#define UDP_FLOW							0 	/* Create a UDP entry */
//All these values, when stored in ASIC, is decremented by 1
#define TCP_LISTEN_FLOW						1	/* Reserved entry for TCP connection in LISTEN_STATE*/
#define TCP_OUTBOUND_SYN_RCVD_FLOW			2	/* Create entry due to outbound SYN rcvd */
#define TCP_INBOUND_SYN_RCVD_FLOW			3	/* Create entry due to inbound SYN rcvd */
#define TCP_ESTABLISHED_FLOW				5	/* Create entry after 3-way handshaking completed */
#define TCP_OUTBOUND_FIN_RCVD_FLOW			6	/* Create entry due to outbound SYN rcvd */
#define TCP_INBOUND_FIN_RCVD_FLOW			7	/* Create entry due to inbound SYN rcvd */
#define TCP_CLOSED_FLOW						8	/*A terminated flow */

int32 rtl8651_delNaptConnection(	int8 isTcp, ipaddr_t insideLocalIpAddr,
									uint16 insideLocalPort, ipaddr_t dstIpAddr, uint16 dstPort);
void rtl8651_updateAsicNaptTable(void);
int32 rtl8651_addNaptIcmpFlow(	int8 assigned, ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
								ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalId, ipaddr_t dstIpAddr);
int32 rtl8651_delNaptIcmpFlow(ipaddr_t insideLocalIpAddr, uint16 insideLocalId, ipaddr_t dstIpAddr);
void rtl8651_updateAsicIcmpTable(void);

//alg-trap  
typedef struct  rtl8651_algEntry_property_s
{
int8 isServer; 
int8 isClient;
int8 isNat;
int8 isNapt;
int8 isPortBouncing;
}rtl8651_algEntry_property_t;
int32 rtl8651_addALGRule(uint16 startPort, uint16 endPort, int8 isTcp, int32 isServer, int32 isClient);
int32 rtl8651_delALGRule(uint16 startPort, uint16 endPort, int8 isTcp, int32 isServer, int32 isClient);

//url filter.

#define RTL8651_URLPKTFILTER_TYPE_SIP		0x01
#define RTL8651_URLPKTFILTER_TYPE_SMAC	0x02

#ifdef RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT
#define RTL8651_URLSTRING_KNOWNTYPE		0x0
#define RTL8651_URLSTRING_UNKNOWNTYPE	0x1
#endif

#ifdef RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT	/* URL Rule Action type Definition */
#define RTL8651_URL_DROP		0x0
#define RTL8651_URL_PERMIT		0x1
#endif


typedef void (*rtl8651_tblDrv_urlFiltering_CallBackFunc_t)(uint32 sessionId, struct rtl_pktHdr *pkt, struct ip *iphdr, const char *urlFiltered, const char *pathFiltered);	/* callBack function to process blocked-url request */

typedef struct rtl8651_tblDrv_urlFilter_s {
 	int8		*urlFilterString;
	int32	urlFilterStrLen;
	int8		*pathFilterString;
	int32	pathFilterStrLen;
	uint32	sessionId;
#ifdef RTL865XB_WEB_CONTENT_HDR_FILTER
	int8		*contentFilterString;
	int32	contentFilterStrLen;
	int8		*httpHdrFilterString;
	int32	httpHdrFilterStrLen;
#endif
	uint8	urlExactlyMatch;
	uint8	pathExactlyMatch;
#ifdef RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT
	uint8	ruleType:1;
#endif
#ifdef RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT
	uint8	actionType:1;
#endif
} rtl8651_tblDrv_urlFilter_t;

typedef struct rtl8651_tblDrv_urlPktFilter_s {
	uint32 rule_type;

	ipaddr_t sip_start;
	ipaddr_t sip_end;

	ether_addr_t smac;
 
} rtl8651_tblDrv_urlPktFilter_t;

int32 rtl8651_setUrlForwardLogging(uint32 SessionID, uint8 enable);
int32 rtl8651_flushUrlFilterRule(uint32 sessionId);
int32 rtl8651a_delURLfilterString(uint32 SessionID,int8 *string, int32 strlen);
#define rtl8651_delURLfilterString(string, strlen) rtl8651a_delURLfilterString(0,string, strlen)
int32 rtl8651a_delURLExactlyfilterString(uint32 SessionID,int8 *string, int32 strlen);
#define rtl8651_delURLExactlyfilterString(string, strlen) rtl8651a_delURLfilterString(0,string, strlen)
int32 rtl8651a_addURLfilterString(uint32 SessionID,int8 *string, int32 strlen);
#define rtl8651_addURLfilterString(string, strlen) rtl8651a_addURLfilterString(0,string, strlen)
int32 rtl8651a_addURLExactlyfilterString(uint32 SessionID,int8 *string, int32 strlen);
#define rtl8651_addURLExactlyfilterString(string, strlen) rtl8651a_addURLExactlyfilterString(0,string, strlen)
int32 rtl8651_addURLFilterRule(uint32 sessionId, int8 *string, int32 strlen, ipaddr_t ip_start, ipaddr_t ip_end);
int32 rtl8651_addURLExactlyFilterRule(uint32 sessionId, int8 *string, int32 strlen, ipaddr_t ip_start, ipaddr_t ip_end);
int32 rtl8651_delURLFilterRule(uint32 sessionId, int8 *string, int32 strlen, ipaddr_t ip_start, ipaddr_t ip_end);
int32 rtl8651_delURLExactlyFilterRule(uint32 sessionId, int8 *string, int32 strlen, ipaddr_t ip_start, ipaddr_t ip_end);
int32 rtl8651_addGenericURLFilterRule(rtl8651_tblDrv_urlFilter_t *urlFilter, rtl8651_tblDrv_urlPktFilter_t *pktFilter);
int32 rtl8651_delGenericURLFilterRule(rtl8651_tblDrv_urlFilter_t *urlFilter, rtl8651_tblDrv_urlPktFilter_t *pktFilter);
int32 rtl8651_registerURLFilterCallBackFunction(rtl8651_tblDrv_urlFiltering_CallBackFunc_t callBackFunc);
#ifdef RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT
int32 rtl8651_urlUnknownTypeCaseSensitive(int8 enable);
#endif
#ifdef RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT
int32 rtl8651_setUrlDefaultAction(uint32 SessionID, uint8 defAction);
#endif

#ifdef RTL865XB_URLFILTER_TRUSTED_USER
int rtl8651_addUrlFilterTrustedUser(uint32 ipaddr);
void rtl8651_flushUrlFilterTrustedUser(void);
#endif

#define _RTL_LOGGING        1 /* enable logging mechanism support */
#define _RTL_NEW_LOGGING_MODEL
#ifdef _RTL_LOGGING
#ifndef _RTL_NEW_LOGGING_MODEL
#define RTL8651_LOGACTION_DROP    1
#define RTL8651_LOGACTION_RESET   2
#define RTL8651_LOGACTION_FORWARD 4
#endif
/**********************************************************
	direction
**********************************************************/
#define RTL8651_DIRECTION_NO	2
#define RTL8651_LAN2WAN   		0
#define RTL8651_WAN2LAN   		1
/**********************************************************
	Module facility types
**********************************************************/
/* Rome Driver Related Log module */
#define RTL8651_LOGMODULE_SYS 		0x00000001
#define RTL8651_LOGMODULE_ACL 		0x00000002
#define RTL8651_LOGMODULE_DOS 		0x00000004
#define RTL8651_LOGMODULE_URL 		0x00000008
#define RTL8651_LOGMODULE_NEWFLOW	0x00000010
#define RTL8651_MAILALERT_SYS 			0x00000100
#define RTL8651_MAILALERT_ACL 			0x00000200
#define RTL8651_MAILALERT_DOS 			0x00000400
#define RTL8651_MAILALERT_URL 			0x00000800
#define RTL8651_MAILALERT_NEWFLOW	0x00001000
#define RTL8651_MAILALERT_MASK		0x00001f00
#define RTL8651_REMOTELOG_SYS 			0x00010000
#define RTL8651_REMOTELOG_ACL 			0x00020000
#define RTL8651_REMOTELOG_DOS 		0x00040000
#define RTL8651_REMOTELOG_URL 			0x00080000
#define RTL8651_REMOTELOG_NEWFLOW	0x00100000
#define RTL8651_REMOTELOG_MASK		0x001f0000

/**********************************************************
	Action types
**********************************************************/
#ifdef _RTL_NEW_LOGGING_MODEL
#define RTL8651_LOGACTION_NO			4
#define RTL8651_LOGACTION_NONE		0
#define RTL8651_LOGACTION_DROP    		1
#define RTL8651_LOGACTION_RESET   		2
#define RTL8651_LOGACTION_FORWARD 	3
#endif
/***************************************************************************
					Log No.
***************************************************************************/
#define RTL8651_TOTAL_USERLOG_NO	37	/* total number of added log */

/* RTL8651_LOGMODULE_NEWFLOW */
#define RTL8651_LOG_NEWFLOW_NewTcpNaptOutbound		1
#define RTL8651_LOG_NEWFLOW_NewTcpNaptInbound		2
#define RTL8651_LOG_NEWFLOW_NewUdpNaptOutbound		3
#define RTL8651_LOG_NEWFLOW_NewUdpNaptInbound		4
#define RTL8651_LOG_NEWFLOW_NewIcmpNaptOutbound	5
#define RTL8651_LOG_NEWFLOW_NewIcmpNaptInbound		6
/* RTL8651_LOGMODULE_ACL */
#define RTL8651_LOG_ACL_EgressAclDropLog			7
#define RTL8651_LOG_ACL_IngressAclDropLog			8
/* RTL8651_LOGMODULE_URL */
#define RTL8651_LOG_URL_MatchUrlFilter				9
/* RTL8651_LOGMODULE_DOS */
#define RTL8651_LOG_DOS_IpSpoof				10
#define RTL8651_LOG_DOS_UdpFlood				11
#define RTL8651_LOG_DOS_HostUdpFlood			12
#define RTL8651_LOG_DOS_UdpLand				13
#define RTL8651_LOG_DOS_UdpBomb				14
#define RTL8651_LOG_DOS_UdpEchoChargen		15
#define RTL8651_LOG_DOS_IcmpLand				16
#define RTL8651_LOG_DOS_IcmpPingOfDeath		17
#define RTL8651_LOG_DOS_IcmpFlood				18
#define RTL8651_LOG_DOS_HostIcmpFlood			19
#define RTL8651_LOG_DOS_IcmpSmurf				20
#define RTL8651_LOG_DOS_SynFlood				21
#define RTL8651_LOG_DOS_HostSynFlood			22
#define RTL8651_LOG_DOS_StealthFin				23
#define RTL8651_LOG_DOS_HostStealthFin			24
#define RTL8651_LOG_DOS_TcpLand				25
#define RTL8651_LOG_DOS_TcpScan				26
#define RTL8651_LOG_DOS_TcpSynWithData		27
#define RTL8651_LOG_DOS_TearDrop				28
#define RTL8651_LOG_DOS_TcpUdpScan_SYN		29
#define RTL8651_LOG_DOS_TcpUdpScan_FIN		30
#define RTL8651_LOG_DOS_TcpUdpScan_ACK		31
#define RTL8651_LOG_DOS_TcpUdpScan_UDP		32
#define RTL8651_LOG_DOS_TcpUdpScan_HYBRID	33
/***************************************************************************
					Logging Data structure definition
***************************************************************************/
#define RTL8651_LOG_INFO_PKT			1	/* record the pkt's information */
#define RTL8651_LOG_INFO_URL			2	/* record the url blocking's information */
typedef struct rtl8651_logInfo_s {
	uint32 infoType;
	/************ data structure different Log Information ************/
	union {
		struct urlLogInfo {
			uint32	dsid;
			uint8	protocol;
			uint8	direction;
			ipaddr_t	sip;
			ipaddr_t	dip;
			uint16	sport;
			uint16	dport;
			char *	string;	/* matched url string */
#ifdef RTL865XB_URLFILTER_LOGMOREINFO
			char *	pathString;	/* path string of the pkt */
			char *	urlFilterString;	/* url filter keyword string */
			char *	pathFilterString;	/* path filter keyword string */
#endif
		} url;
		struct packetLogInfo {
			uint32	dsid;
			uint8	protocol;
			uint8	direction;
			ipaddr_t	sip;
			ipaddr_t	dip;
			uint16	sport;
			uint16	dport;
			uint8	icmpType;
			uint16	icmpId;
		} pkt;
	}un;

	#define pkt_dsid			un.pkt.dsid
	#define pkt_proto		un.pkt.protocol
	#define pkt_direction		un.pkt.direction
	#define pkt_sip			un.pkt.sip
	#define pkt_dip			un.pkt.dip
	#define pkt_sport			un.pkt.sport
	#define pkt_dport		un.pkt.dport
	#define pkt_icmpType	un.pkt.icmpType
	#define pkt_icmpId		un.pkt.icmpId

	#define url_dsid			un.url.dsid
	#define url_proto			un.url.protocol
	#define url_direction		un.url.direction
	#define url_sip			un.url.sip
	#define url_dip			un.url.dip
	#define url_sport			un.url.sport
	#define url_dport			un.url.dport
	#define url_string		un.url.string
#ifdef RTL865XB_URLFILTER_LOGMOREINFO
	#define url_pathString	un.url.pathString
	#define url_urlFilterString	un.url.urlFilterString
	#define url_pathFilterString	un.url.pathFilterString
#endif
	uint32 action;	/* action of this logging event */
} rtl8651_logInfo_t;

//user logging function plugin
#ifndef _RTL_NEW_LOGGING_MODEL
typedef int32 (*rtl8651_loggingFuncPtr_t)
(
unsigned long  dsid,
unsigned long  moduleId,
unsigned char  proto,
char           direction,
unsigned long  sip,
unsigned long  dip,
unsigned short sport,
unsigned short dport,
unsigned char  type,
unsigned char  action,
char         * msg
);
#else
typedef int32 (*rtl8651_loggingFuncPtr_t)
(
	uint32			moduleId,
	uint32			logNo,
	rtl8651_logInfo_t*	info
);
#endif

int32 rtl8651_installLoggingFunction(void * pMyLoggingFunc);
int32 rtl8651a_enableLogging(uint32 SessionID,uint32 moduleId,int8 enable);
#define rtl8651_enableLogging(moduleId,enable) rtl8651a_enableLogging(0,moduleId,enable)
extern rtl8651_loggingFuncPtr_t _pUserLoggingFunc;
#endif /* _RTL_LOGGING */

// Other functions
int32 rtl8651_tblDrvReset(void);
int32 rtl8651_tblDrvReinit(void);

int32 rtl8651_tblDrvInit(rtl8651_tblDrvInitPara_t * configPara);

int32 rtl8651_getPppoeSessionSourceMac(uint32 SessionID, ether_addr_t * macAddr);
ipaddr_t _rtl8651_getNaptInsideGlobalIpAddr(int8 isTcp, ipaddr_t insideLocalIpAddr, uint16 insideLocalPortOrId, ipaddr_t dstIpAddr, uint16 dstPort, uint16 isNapt);

/*for serverport, triggerport, upnp,dmz...
*/
#define RTL8651_NAPTUSRMAPPING_IPFILTER_ACTION_DROP 	0x0001
#define RTL8651_NAPTUSRMAPPING_IPFILTER_ACTION_PERMIT 	0x0002
typedef struct rtl8651_tblDrv_naptUsrMapping_ipFilter_s
{
	ipaddr_t filter_ip_start;	/*define the ip range in which any ip would be droped*/
	ipaddr_t filter_ip_end;	/**/

	uint16 filter_port_start;	/*port range which would be drop*/
	uint16 filter_port_end;	/*NOTE: port range is valid only when the ip range is valid!*/

	/*
	*action for ip and port range match.
	*the value should be RTL8651_NAPTUSRMAPPING_IPFILTER_ACTION_DROP
	* or RTL8651_NAPTUSRMAPPING_IPFILTER_ACTION_PERMIT.
	*/
	uint16 action;

	/*indicator for ip range limitation
	* 0: means we want to limit the remote ip
	* 1: means we want to limit the local ip
	*/
	uint16 flag;
	
}rtl8651_tblDrv_naptUsrMapping_ipFilter_t;

#ifdef RTL8651_NAPT_USR_MAPPING
#define RTL8651_NAPTUSRMAPPING_IPFILTER_FLAG_REMOTE 	0
#define RTL8651_NAPTUSRMAPPING_IPFILTER_FLAG_LOCAL 		1

#define RTL8651_NAPTUSRMAPPING_SERVERPORT				1<<0
#define RTL8651_NAPTUSRMAPPING_TRIGGERPORT				1<<1
#define RTL8651_NAPTUSRMAPPING_UPNP						1<<2
#define RTL8651_NAPTUSRMAPPING_DMZ						1<<3

/*define the flag bit*/
/*Protocol*/
#define RTL8651_NAPTUSRMAPPING_FLAG_PROTOCOL_TCP		1<<0
#define RTL8651_NAPTUSRMAPPING_FLAG_PROTOCOL_UDP		1<<1

/*define triggerport: static and dynamic*/
#define RTL8651_NAPTUSRMAPPING_FLAG_TRIGGER_STATIC		1<<16
#define RTL8651_NAPTUSRMAPPING_FLAG_TRIGGER_DYNAMIC	1<<17

/*other:we reuse some bit for different type*/
#define RTL8651_NAPTUSRMAPPING_FLAG_TOASIC			1<<8

#define RTL8651_NAPTUSRMAPPING_FLAG_ONESHOT		1<<8
#define RTL8651_NAPTUSRMAPPING_FLAG_TIMEOUT		1<<9
#define RTL8651_NAPTUSRMAPPING_FLAG_PERSIST			1<<10
#define RTL8651_NAPTUSRMAPPING_FLAG_PORTBOUNCING	1<<11
#define RTL8651_NAPTUSRMAPPING_FLAG_NAT				1<<12
#define RTL8651_NAPTUSRMAPPING_FLAG_NAPT			1<<13


#define RTL8651_NAPTUSRMAPPING_FLAG_CLIENT			1<<8
#define RTL8651_NAPTUSRMAPPING_FLAG_SERVER			1<<9

#define RTL8651_NAPTUSRMAPPING_DIR_INBOUND			1<<0
#define RTL8651_NAPTUSRMAPPING_DIR_OUTBOUND		1<<1
#define RTL8651_NAPTUSRMAPPING_DIR_BOTH				(1<<0 |1<<1)

/**/
typedef struct rtl8651_tblDrv_naptUsrMapping_s
{
	uint32 type;	/*indicate the naptflow type. the values are:
				  NAPTFLOW_DMZ, NAPTFLOW_UPNP, NAPTFLOW_SERVERPORT,NAPTFLOW_TRIGGERPORT
				  */
				  
	uint32 dsid;	/*dial Session id*/	
	/*different means for different type.
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           type				|31...............16		|	7...6		|	5	|	4	|	3	|	2	|	1	|	0	|	7	|	6	...3	|	1	|	0	|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-
	|  server port				|reserved			|				reserved									 	|out/in				|		|		|
	|						|					|															|protocol: reserved	|   udp	|   tcp	|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-
	|  trigger port				|reserved|dyn |static	|out		 	 	reserved						|	udp	|	tcp 	|in					|		|		|
	|						|		|amic|		|protocol	:									|		|		|proto:	reserved		|   udp	|   tcp	|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-
	|  trigger port	Flow			|reserved|dyn |static	|out		 	 	reserved						|	udp	|	tcp 	|in					|		|		|
	|						|		|amic|		|protocol										|		|		|proto:	reserved		|   udp	|   tcp	|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-
	|  		upnp			|reserved			|reserved	| napt	|   nat	|port	|persist	|time out	|oneshot	|out/in	reserved		|   udp	|   tcp	|
	|						|					|			|		|		|bouncing|		|		|		|protocol:			|		|		|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-
	|		DMZ				|						reserved													|			reserved					|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-	
	*/
	uint32 flag;
	
	uint32 age;	/*age timing*/
	uint32 maxAge;	/*Max age time*/
	uint32 algIndex;	/*for Qos...*/

	/*define the remote ip,0 means no limit!
	* if the remote ip range is need, please use the ipFilter
	*/
	ipaddr_t remote_ip;
	
	/*alias ip, generally, it's the gateway's ip*/
	ipaddr_t alias_ip;	
	
	/*local ip, 0 means no limit!
	* if the remote ip range is need, please use the ipFilter
	*/	
	ipaddr_t local_ip;

	uint16 remote_port_start;	/*define the remote port range*/
	uint16 remote_port_end;	/*0 means No limit!*/
	uint16 alias_port_start;	/*define the alias port range*/
	uint16 alias_port_end;	/*Note:the range need to be checked!*/
	uint16 local_port_start;	/*define the local port range*/
	uint16 local_port_end;	/*0 means No limit!*/

	uint16 direction;	/*value:*/
	
}rtl8651_tblDrv_naptUsrMapping_t;

/*napt user mapping related*/
/*for external using*/
int32 rtl8651_addNaptUsrMappingEntry(rtl8651_tblDrv_naptUsrMapping_t *naptFlowEntry, rtl8651_tblDrv_naptUsrMapping_ipFilter_t *ipFilterEntry);
int32 rtl8651_addNaptUsrMappingEntryIpFilter(rtl8651_tblDrv_naptUsrMapping_t * naptFlowEntry, rtl8651_tblDrv_naptUsrMapping_ipFilter_t * ipFilterEntry);

int32 rtl8651_delNaptUsrMappingEntry(rtl8651_tblDrv_naptUsrMapping_t * naptFlowEntry);
int32 rtl8651_delNaptUsrMappingEntryIpFilter(rtl8651_tblDrv_naptUsrMapping_t * naptFlowEntry, rtl8651_tblDrv_naptUsrMapping_ipFilter_t * ipFilterEntry);

int32 rtl8651_flushNaptUsrMappingEntryByType(uint32 type);
int32 rtl8651_flushAllNaptUsrMappingEntry(void);
int32 rtl8651_setNaptUsrMappingTypePriority(uint32 priority);

/*DMZ Host*/
int32 rtl8651_addGenericDmzHost(uint32 SessionID,ipaddr_t naptIp, ipaddr_t dmzHostIp, rtl8651_tblDrv_naptUsrMapping_ipFilter_t *ipFilterEntry);
int32 rtl8651_addGenericDmzFilter(uint32 SessionID,ipaddr_t naptIp, ipaddr_t dmzHostIp, rtl8651_tblDrv_naptUsrMapping_ipFilter_t *ipFilterEntry);
int32 rtl8651_delGenericDmzFilter(uint32 SessionID, ipaddr_t naptIp, ipaddr_t dmzHostIp, rtl8651_tblDrv_naptUsrMapping_ipFilter_t * ipFilterEntry);
#endif

int32 rtl8651a_addDmzHost(uint32 SessionID,ipaddr_t naptIp, ipaddr_t dmzHostIp);
#define rtl8651_addDmzHost(naptIp,dmzHostIp) rtl8651a_addDmzHost(0,naptIp,dmzHostIp)
int32 rtl8651a_delDmzHost(uint32 SessionID,ipaddr_t naptIp);
#define rtl8651_delDmzHost(naptIp) rtl8651a_delDmzHost(0,naptIp)

//IP Unnumbered
int32 rtl8651_addIpUnnumbered(int8 *wanIfName, int8 *lanIfName, ipaddr_t netMask);
int32 rtl8651_delIpUnnumbered(int8 *wanIfName, int8 *lanIfName);
int32 rtl8651_getIpUnnumbered(int8 *wanIfName, int8 *ifName, ipaddr_t *netMask);
int32 rtl8651_setLanSideExternalIpInterface(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask, int8 isExternal);
int32 rtl8651_enableUnnumberNaptProc( int32 enable );

/* QoS */
int32 rtl8651_setFlowControl(uint32 port, int8 enable);
int32 rtl8651_setQueueScheduling(uint32 type, uint32 weight);
int32 rtl8651_setPortPriorityQueue(uint32 port, int8 highPriority);
int32 rtl8651_setDiffServDSCP(uint32 dscp, int8 highPriority);
int32 rtl8651_getMNQueueEntry(rtl8651_tblDrvAclRule_t *rule,uint32 GroupId,uint32 index);
void rtl8651_dumpMNQueueSystemInfo(void);
int32 rtl8651_addFlowToUnlimitedQueue(int8 *ifName, rtl8651_tblDrvAclRule_t *rule, uint32 ratio, uint32 queueID);
int32 rtl8651_addFlowToLimitedQueue(int8 *ifName, rtl8651_tblDrvAclRule_t *rule, uint32 ratio, uint32 queueID);
int32 rtl8651_delFlowFromMNQueue(rtl8651_tblDrvAclRule_t *rule, uint32 GroupId);
#if defined(RTL865XC_MNQUEUE_OUTPUTQUEUE)  || defined(RTL865XC_QOS_OUTPUTQUEUE)
int32	rtl865xC_initOutputQueue(void);
int32	rtl865xC_exitOutputQueue(void);
int32 rtl865xC_setOutputQueueWanPort(uint32 wanPort);
int32 rtl865xC_setDefaultQueueRatio(uint32 ratio);
int32 rtl865xC_setQosPara(rtl8651_tblDrv_qos_sys_t *info);
#endif
int32 rtl8651_setUpstreamBandwidth(int8 *ifName, uint32 bps);
int32 rtl8651_setDscpToMNQueue(uint32 queueID, uint32 queueDSCP);

/* PPPoE Passthru */
int8 rtl8651_EnablePppoePassthru( int8 enable );
int8 rtl8651_enableDropUnknownPppoePADT( int8 enable );
int32 rtl8651_getPppoePassthruVid(void);
int32 rtl8651_getExternalInterfaceforPPPoEPassthru(void);

/* IPv6 passthru */
int8 rtl8651_EnableIpv6Passthru(int8 enable);
int32 rtl8651_getIpv6PassthruVid(void);
int32 rtl8651_getExternalInterfaceforIPv6Passthru(void);

/* IPX passthru */
int8 rtl8651_EnableIpxPassthru(int8 enable);
int32 rtl8651_getIpxPassthruVid(void);
int32 rtl8651_getExternalInterfaceforIPXPassthru(void);

/* Netbios passthru */
int8 rtl8651_EnableNetbiosPassthru(int8 enable);
int32 rtl8651_getNetbiosPassthruVid(void);
int32 rtl8651_getExternalInterfaceforNetbiosPassthru(void);

/* Passthru Generic API */
int32 rtl8651_transformPasstheuVlanId( uint32 oldVlanIdx, uint32* newVlanIdx );

void rtl8651_setMtu(uint16 mtu);
int32 rtl8651_setNetMtu(int8 *ifName, uint16 mtu);
void rtl8651_getMtu(uint16 * mtu);

uint8 rtl8651_EnableProtocolBasedNAT( uint8 enable );
int32 rtl8651_addProtocolBasedNAT( uint8 protocol, ipaddr_t ExtIp, ipaddr_t IntIp );
int32 rtl8651_delProtocolBasedNAT( uint8 protocol, ipaddr_t ExtIp, ipaddr_t IntIp );

int32 rtl8651_enableMacAccessControl(int8 enable);
int32 rtl8651_setMacAccessControlDefaultPermit(int8 isPermit);
int32 rtl8651_delMacAccessControl(ether_addr_t *sMac, ether_addr_t *macMask);
int32 rtl8651_flushMacAccessControl(void);
int32 rtl8651_addMacAccessControl(ether_addr_t *sMac, ether_addr_t *macMask, int8 isPermit);


// multicast
int32 rtl8651_addExternalMulticastPort(uint32 extMPort);
int32 rtl8651_delExternalMulticastPort(uint32 extMPort);

int32 rtl8651_autoCrossOverDetect(uint32 isEnable);
int32 rtl8651_portAutoCrossOverDetect(uint32 port, uint32 isEnable);
int32 rtl8651_setMdiMdix(uint32 isMdi);
int32 rtl8651_setPortMdiMdix(uint32 port, uint32 isMdi);




extern int8 rtl8651_tblDrv_Id[];
extern int8 rtl8651_tblDrvFwd_Id[];


/* For QoS: Scheduling type */
#define RTL8651_QOS_RR		0x01
#define RTL8651_QOS_ALG_HIGH_QUEUE 100
#define RTL8651_QOS_ALG_LOW_QUEUE 200

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

#define RTL8651_PPPOE			0x01
#define RTL8651_PPTP			0x02
#define RTL8651_L2TP			0x03

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


#define PS_ACTION_SERVER 0
#define PS_ACTION_CLIENT 1
#define PS_ACTION_NONE 0
#define PS_ACTION_DROP 1
#define PS_ACTION_TO_PS 2
#define PS_ACTION_DONT_TIMEOUT 3
#define RTL8651_MAX_PROTO_STACK_ACTIONS 16

#if 0	/* Replaced by structure "rtl8651_PS_Action_Entry_t" */
struct _RTL8651_PS_ACTION_ENTRY{
	ipaddr_t ip;
	ipaddr_t mask;
	uint16	orgPort;
	uint16	orgPortEnd;
	uint16	fwdPort;
	uint8	protocol; // id the same as IP header
	uint8	action; // PS_ACTION_NONE,PS_ACTION_DROP,PS_ACTION_TO_PS
	uint8	role; // PS_ACTION_SERVER,PS_ACTION_CLIENT
};
#endif

/* External PS action data structure. */
typedef struct rtl8651_PS_Action_Entry_s {
	ipaddr_t dstIp;
	ipaddr_t lowerIp;
	ipaddr_t upperIp;
	uint16	orgPort;
	uint16	orgPortEnd;
	uint16	fwdPort;
	uint8	protocol; // id the same as IP header
	uint8	action; // PS_ACTION_NONE,PS_ACTION_DROP,PS_ACTION_TO_PS
	uint8	role; // PS_ACTION_SERVER,PS_ACTION_CLIENT
} rtl8651_PS_Action_Entry_t;

extern int32 wlan_acc_debug;


/*CONFIG_RTL865XB_EXP_PPPOE_PASSTHRU*/
extern uint8 rtl8651_gPPPoEExternalVlanIndex;  /* External VLAN for PPPoE dialup */
extern ether_addr_t rtl8651_gPPPoEExternalGatewayMAC; 
extern uint16 rtl8651_gPPPoEPassthruVlanID;  /* VLAN ID used for PPPoE Passthru */
extern uint16 rtl8651_gVlanIDforPassthru; /*VLAN ID used for passthru*/
#define RTL8651_PPPOEPASSTHRU_VLANID_IS_NOT_SET (4096+1)

#define RTL8651_PPPOEPASSTHRU_DEFAULT_TIMEOUT 300 /* Unit: seconds */

uint32 _rtl8651_setPPPoEPassthruTimeout( uint32 timeout );
int32 rtl8651_addPPPoEPassthru( uint16 sid, ether_addr_t* clientMac, ether_addr_t* serverMac,
                                uint16 clientVid, uint16 serverVid );
int32 rtl8651_getPPPoEPassthru( uint16 sid, ether_addr_t* clientMac, ether_addr_t* serverMac,
                                uint16 *clientVid, uint16 *serverVid, int32 updateTouch );
int32 rtl8651_delPPPoEPassthru( uint16 sid );

extern uint32 gPPPoEPassthruTimeout;



/* CONFIG_RTL865XB_EXP_IPV6_PASSTHRU */
extern uint8 rtl8651_gIPv6ExternalVlanIndex;  /* External VLAN for PPPoE dialup */
extern ether_addr_t rtl8651_gIPv6ExternalGatewayMAC; 
extern uint16 rtl8651_gIPv6PassthruVlanID;  /* VLAN ID used for PPPoE Passthru */
extern uint32 rtl8651_pbv_ipv6passthru;      /* Protocol-based VLAN control register: 5 and 6 are valid. */
#define RTL8651_IPV6PASSTHRU_VLANID_IS_NOT_SET (4096+1)
#define RTL8651_IPV6PASSTHRU_PBVID_IS_NOT_SET RTL8651_PBV_RULE_MAX


/* CONFIG_RTL865XB_EXP_IPX_PASSTHRU */
extern uint8 rtl8651_gIPXExternalVlanIndex;  /* External VLAN for IPX */
extern ether_addr_t rtl8651_gIPXExternalGatewayMAC; 
extern uint16 rtl8651_gIPXPassthruVlanID;  /* VLAN ID used for PPPoE Passthru */
extern uint32 rtl8651_pbv_ipxpassthru;      /* Protocol-based VLAN control register: 5 and 6 are valid. */
#define RTL8651_IPXPASSTHRU_VLANID_IS_NOT_SET (4096+1)
#define RTL8651_IPXPASSTHRU_PBVID_IS_NOT_SET RTL8651_PBV_RULE_MAX


/* CONFIG_RTL865XB_EXP_NETBIOS_PASSTHRU */
extern uint8 rtl8651_gNetbiosExternalVlanIndex;  /* External VLAN for IPX */
extern ether_addr_t rtl8651_gNetbiosExternalGatewayMAC; 
extern uint16 rtl8651_gNetbiosPassthruVlanID;  /* VLAN ID used for PPPoE Passthru */
extern uint32 rtl8651_pbv_netbiospassthru;      /* Protocol-based VLAN control register: 5 and 6 are valid. */
#define RTL8651_NETBIOSPASSTHRU_VLANID_IS_NOT_SET (4096+1)
#define RTL8651_NETBIOSPASSTHRU_PBVID_IS_NOT_SET RTL8651_PBV_RULE_MAX


extern int8 rtl8651_drvProtocolBasedNatEnable;

/* for all kind of Passthru */
#define RTL8651_PASSTHRU_VLANID_IS_NOT_SET (4096+1)

/* for napt connection add */
#define	EXTIPPORT_ASSIGNED		0x0001
#define	PURE_SW					0x0002
#define	DONT_CHECK_INTIP			0x0004
#define	TO_PROTOCOL_STACK		0x0008
#define	DONT_BIRD_CDROUTER		0x0010
#define	DOS_NAPT_ENCOUNT			0x0020
#define	DOS_NAPT_FROMINTERNAL	        0x0040
#define	ALG_QOS					0x0080
#define	DO_REDUNDANCY_CHECK		0x0100
#define	STATIC_PORT_TRANSLATION	       0x0200
#define 	SET_TRIGGER_PORT_AGE_TIME	0x0400

/* Session Hang-up */
typedef struct rtl8651_sessionHangUp_s {
	int32 enable;							/* enable or disable ? */
	uint32 sessionID;						/* target session */
	uint32 idleTimeout_sec;				/* idle timeout second */					
	int32 (*idleTimeout_callBack)(uint32);	/* callBack function => must not be NULL if enable == TRUE */
} rtl8651_sessionHangUp_t;

int32 rtl8651_setSessionHangUp(rtl8651_sessionHangUp_t *param);
int32 rtl8651_initDrvParam(rtl8651_tblDrvInitPara_t * configPara);
int32 _rtl8651_destroyProtocolBasedVLANforIPXPassthru(void);
int32 _rtl8651_destroyProtocolBasedVLANforNetbiosPassthru(void);

#ifdef CONFIG_RTL865XC
void _rtl8651_setArpIpIdx(uint32 arpIpIdx);
void _rtl8651_resetArpIpIdx(void);
#endif

int32 rtl8651_bindSession(ipaddr_t ipAddr, uint32 sessionId);
int32 rtl8651_addSession(uint32 type, uint32 sessionId, int8 *ifName, uint16 vid);
int32 rtl8651_delSession(uint32 sessionId);

typedef struct rtl8651_extRouteTable_s
{
	ipaddr_t ipAddr; /* Destination IP Address */ 
	ipaddr_t ipMask; /* Network mask */ 
	ipaddr_t nextHop; /* next hop IP address */ 
	int8 ifName[RTL8651_NETINTF_NAME_LEN]; 
} rtl8651_extRouteTable_t;

int32 rtl8651_getRoutingTable(rtl8651_extRouteTable_t *curr, rtl8651_extRouteTable_t *next);

#ifdef NEW_DOS_RULE
int32 _rtl8651_flushOneSrcDosConnection(uint32 isTcp, ipaddr_t ip, uint32 direction, uint32 toFlushOldest);
int32 _rtl8651_flushOneWholeSysDosConnection(uint32 isTcp, uint32 toFlushOldest);
int32 _rtl8651_countSrcDosNaptConnection(ipaddr_t ip, uint32 *tcpCount, uint32 *udpCount, uint32 *tcpUdpCount);
int32 _rtl8651_countWholeSysDosNaptConnection(uint32 *tcpCount, uint32 *udpCount, uint32 *tcpUdpCount);
#endif

#if	RTL8651_ENABLETTLMINUSINE
int32	rtl8651_setTtlMinus(int32 enable);
#endif

#if RTL865XC_QOS_OUTPUTQUEUE

/* priority decision array index */
#define	PORT_BASE	0
#define	D1P_BASE	1
#define	DSCP_BASE	2
#define	ACL_BASE	3
#define	NAT_BASE	4




#define	RTL865XC_QOS_QUEUETYPE_MASK		0x1
#define	RTL865XC_QOS_QUEUETYPE_OFFSET	0x0

#define	RTL865XC_QOS_QUEUETYPE_STP		0x1
#define	RTL865XC_QOS_QUEUETYPE_WFQ		0x2

#define	RTL865XC_QOS_ENABLE_DSCPRMK_MASK		0x4
#define	RTL865XC_QOS_ENABLE_DSCPRMK_OFFSET		0x2

#define	RTL865XC_QOS_ENABLE_1PRMK_MASK		0x8
#define	RTL865XC_QOS_ENABLE_1PRMK_OFFSET		0x3

typedef struct rtl8651_qosInfo_s
{
	uint8 groupID;
	uint8 ratio;
	uint8 dscpRemark;
	uint8 vlanPriRemark;
	uint32 flags;	/*	
					*	the bit0 dedicated for queue type: 
					*		STR_PRIO	 for strict priority
					*		WFQ_PRIO	 for weighted fair queue
					*	the bit1-bit31 reserved
					*/
}	rtl8651_qosInfo_t;



int32	rtl865x_qosInit(int8 *ifName, uint32 bps);

#define	IsEntry1MoreImportant(__entry1__, __entry2__)	((((rtl8651_tblDrv_outputQueue_t*)(__entry1__))->ratio)>(((rtl8651_tblDrv_outputQueue_t*)(__entry1__))->ratio))


int32	rtl865x_qosAddFlow(int8* ifName, rtl8651_tblDrvAclRule_t *rule, rtl8651_qosInfo_t	*info);
int32	rtl865x_qosDelFlow(rtl8651_tblDrvAclRule_t *rule, rtl8651_qosInfo_t	*info);
int32	rtl865x_qosflushFlow(void);
int32  rtl865x_qosSetRemarkInfo(rtl8651_tblDrvAclRule_t *rule, rtl8651_qosInfo_t *info);
int32	rtl865x_qosSetQosOperationLayer(rtl865xC_qos_opLayer_t	opLayer);
#endif

#endif

