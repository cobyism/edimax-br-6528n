/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver local header rtl8651_tblDrvLocal.h
* Abstract : 
* Creator : Edward Jin-Ru Chen (jzchen@realtek.com.tw)
* Author :  
* $Id: rtl8651_tblDrvLocal.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $

*
*/

#ifndef RTL8651_TBLDRV_LOCAL_H
#define RTL8651_TBLDRV_LOCAL_H


/* Please follow the header file inclusion order below.... */

/* first include common data types */
#include "rtl_types.h"
#include "mbuf.h"
#include "rtl_queue.h"

/* macro definitions */
#include "rtl8651_config.h"
/* then include exported interface */
#include "rtl8651_layer2.h"
#include "rtl8651_tblDrv.h"

/* finally include internal data types and interfaces. */
#include "rtl8651_tblAsicDrv.h"
#include "rtl8651_aclLocal.h"
#include "rtl8651_layer2local.h"
#include "rtl8651_layer3local.h"
#include "rtl8651_layer4local.h"


// CONFIG_BRIDGE is undefined in 865x SDK
#undef CONFIG_BRIDGE

#if defined(CONFIG_BRIDGE) ||defined(CONFIG_BRIDGE_MODULE)
#define STP_PATCH /*@qy_wang add multi-extension device STP issue*/
#endif

#if	RTL8651_ENABLETTLMINUSINE
extern int32	_rtl8651_ttlMinusOneEnable;
#endif

#define RTL8651_TBLDRV_PRIVATE_DEFINITIONS
//---------------------------------------------------------------
//All other non-layer specific private datatypes, macros, and  #define 
//---------------------------------------------------------------

/***************************************/
/*	NAPT Flows related		*/
/*	Hyking				*/
/***************************************/
/*define the default size of naptUsrMapping table*/
#define RTL8651_NAPTUSRMAPPING_TABLESIZE		128
#define RTL8651_NAPTUSRMAPPING_TYPE_NUM		5
#define RTL8651_NAPTUSRMAPPING_PRIORITY_DEFAULT	0x10234
typedef struct rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_s
{
	/*external information.*/
	rtl8651_tblDrv_naptUsrMapping_ipFilter_t ipFilter_t;
	
	/*for filter list*/
	struct rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_s *nextUseFilter;
	struct rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_s *preUseFilter;
	
	/*for memory management!*/
	SLIST_ENTRY(rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_s) next;
}_rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_t;

#ifdef RTL8651_NAPT_USR_MAPPING

#define RTL8651_NAPTUSRMAPPING_DEFAULT_AGE	3600
/*why the value is 8848, please guest....*/
#define RTL8651_NAPTUSRMAPPING_SAMEINFO	8848

/*for active trigger port, we define a new naptUsrMap type*/
/*1<< 0,1, 2, 3  is used for serverPort,triggerPort,upnp, and dmz*/
#define RTL8651_NAPTUSRMAPPING_PROTOSTACKACTION		1<<4
#define RTL8651_NAPTUSRMAPPING_GETIP			1<<5
#define RTL8651_NAPTUSRMAPPING_TRIGGERFLOW	1<<31
#define RTL8651_NAPTUSRMAPINFO_INRANGE		-1
#define RTL8651_NAPTUSRMAPINFO_EQUAL	  		0
#define RTL8651_NAPTUSRMAPINFO_OUTRANGE		1

/*define a table for merging the upnp/dmz/serverport/triggerport table....*/
typedef struct rtl8651_tblDrv_naptUsrMapping_internal_s
{
	/*define for expansibility*/
	uint16 valid;		/*entry is valid? 0: Invalid, 1: valid*/	
	/*information for external using is defined in tbldrv.h*/
}_rtl8651_tblDrv_naptUsrMapping_internal_t;

typedef struct _rtl8651_tblDrv_naptUsrMapping_entry_s
{
	/*information for internal using*/
	_rtl8651_tblDrv_naptUsrMapping_internal_t intInfo_naptUsrMappingEntry;
	/*information for external using*/
	rtl8651_tblDrv_naptUsrMapping_t extInfo_naptUsrMappingEntry;

	/*information for ip filter*/
	_rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_t *IpFilterHdr;	

	/*List of napt_flows_entry*/
	STAILQ_ENTRY(_rtl8651_tblDrv_naptUsrMapping_entry_s) nextTblEntry;	
}_rtl8651_tblDrv_naptUsrMapping_entry_t;

typedef struct _rtl8651_tblDrv_naptUsrMappingType_priority_s
{
	uint32 type;
	uint32 priority;
	
	SLIST_ENTRY(_rtl8651_tblDrv_naptUsrMappingType_priority_s) next;
}_rtl8651_tblDrv_naptUsrMappingType_priority_t;
#endif

/*remove from layer4local.h because we will merger it with server port/upnp/...*/
#define dmzHostNumInOneSession 6

typedef struct rtl8651DmzHostStruct_s {
	/* This is for multiple DMZ-host spec */
	ipaddr_t naptIp[dmzHostNumInOneSession];
	ipaddr_t dmzHost[dmzHostNumInOneSession];
	/*
	This is for filtering some sip to DMZ
	Hyking
	2006-07-28
	*/
	_rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_t *ipFilterHdr[dmzHostNumInOneSession];
} rtl8651DmzHostStruct_t;


/* Collective Driver table structure*/

struct rtl8651_driverTables {
	
	/*Driver tables (with Valid bit)*/
	rtl8651_tblDrv_ethernet_t *phyPort;	/*Pjysical Port Table. was  rtl8651EthernetPortTable */
	rtl8651_tblDrv_linkAggregation_t *aggregator; /*Link Aggregator. was  rtl8651LinkAggregationTable */
	rtl8651_tblDrv_filterDbTable_t *filterDB; /*Filter database table.  was rtl8651FilterDbTable*/
	rtl8651_tblDrv_spanningTreeTable_t *spanningTree;  /*Spanning Tree Instance Table. wasrtl8651SpanningTreeTable*/

	rtl8651_tblDrv_vlanTable_t *vlan; 	/* rtl8651VlanTable*/
#if defined (RTL865XB_DCUT_SWVLAN) || defined(CONFIG_RTL865XC)
	rtl8651_tblDrv_swVlanTable_t *swVlan;	/* For RTL865xB d-cut: software vlan table */
#endif

	rtl8651_tblDrv_networkIntfTable_t *netIface; /*rtl8651NetworkIntfTable*/
	rtl8651_tblDrv_routeTable_t *route; /* rtl8651RoutingTable*/
	rtl8651_tblDrv_ipEntry_t *iptbl; /* Internal External Table */
	
	rtl8651_tblDrv_naptIcmpFlowEntry_t **icmp; /*rtl8651NaptIcmpFlowTable*/
	rtl8651DrvNaptLine_t * tcpudp; /*rtl8651NaptTcpUdpFlowTable*/

	_rtl8651_tblDrvDynamicPolicy_t dynamicPolicy;
	rtl8651_tblDrv_pppTable_t *pppoe;
	rtl8651_tblDrv_pppTable_t *pptp;
	rtl8651_tblDrv_pppTable_t *l2tp;

	rtl8651_tblDrv_macContrl_t *macACInfo; /* MAC Access Control */
	rtl8651_tblDrv_rateLimit_t *rateLimit; /* Rate Limit Table */
	rtl8651_tblDrv_nextHopEntry_t *nhtbl; /* nexthop Table */
#if defined(RTL865XC_QOS_OUTPUTQUEUE) || defined(RTL865XC_MNQUEUE_OUTPUTQUEUE)
	rtl8651_tblDrv_outputQueue_t	*outputQueue;
#endif		
	
	/*Linked list for dynamic resources allocation. Should all be rewritten with standard macros in queue.h to avoid bugs */
	struct freeEntryList_s {

		SLIST_HEAD( _FreeFDBEntry, rtl8651_tblDrv_filterDbTableEntry_s) filterDBentry;
		CTAILQ_HEAD( _FreeAclEntry_t, _rtl8651_tblDrvAclRule_s) aclRule;
		CTAILQ_HEAD( _FreeMACAccessEntry, rtl8651_tblDrv_macACEntry_s) freeMacAccessControl;
		CSLIST_HEAD( _FreeArpEntry, rtl8651_tblDrv_arpEntry_s) arp;
		SLIST_HEAD( _FreeIpIfaceEntry, rtl8651_tblDrv_ipIntfEntry_s) ipIface; /* Linked through nextIp pointer */
		SLIST_HEAD( _FreeIpAddrEntry, rtl8651_tblDrv_ipIntfIpAddrEntry_s) ipAddr;
		SLIST_HEAD( _FreeNatEntry, rtl8651_tblDrv_natEntry_s) natMapping;

		SLIST_HEAD( _FreeAlgEntry, rtl8651_tblDrv_algEntry_s) alg;
		CSLIST_HEAD( _FreeAlgSlot, rtl8651_tblDrv_algSlot_t) asicAlgSlot;
		SLIST_HEAD( _FreeServerPortEntry, rtl8651_tblDrv_naptServerPortEntry_s) serverPort;		
		
		/*ipFilter list For napt usr mapping
		* Hyking
		*2008-08-09
		*/
#ifdef RTL8651_NAPT_USR_MAPPING
		STAILQ_HEAD(_FreeNaptUsrMappingEntry, _rtl8651_tblDrv_naptUsrMapping_entry_s) freeNaptUsrMappingEntry;
		SLIST_HEAD(_FreeNaptUsrMappingTypePriority, _rtl8651_tblDrv_naptUsrMappingType_priority_s) freeNaptUsrMappingTypePriority;
#endif
		SLIST_HEAD(_FreeNaptUsrMappingIpFilterEntry,rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_s) freeNaptUsrMappingIpFilter;

		rtl8651_tblDrv_naptIcmpFlowEntry_t *icmp; /* rtl8651FreeNaptIcmpFlowEntryList*/
		CTAILQ_HEAD( _FreeNaptEntry, rtl8651_tblDrv_naptTcpUdpFlowEntry_s) tcpudp;

		TAILQ_HEAD(_FreeSessionEntry, rtl8651_tblDrvSession_s) freeSession;
		/* policy */
		TAILQ_HEAD( _FreePolicyEntry, _rtl8651_tblDrvPolicy_s) freePolicy;
		TAILQ_HEAD( _FreeMultiCastEntry, rtl8651_tblDrv_mCast_s) freeMultiCast;

		SLIST_HEAD( _FreeProtocolBasedNat, rtl8651_tblDrv_protocolBasedNat_s) protocolBasedNat;
		SLIST_HEAD(_FreeNextHopEntry, rtl8651_tblDrv_nextHopEntry_s) nexthop;

		SLIST_HEAD(_FreeIpUnnumberEntry, rtl8651_tblDrv_ipUnnumbered_s) freeIpUnnumber;

		SLIST_HEAD(_FreeUrlPktFilterEntry, urlPktFiltering_s) freeUrlPktFilter;

		SLIST_HEAD(_FreeAdvRouteEntry, _rtl8651_tblDrvAdvRoute_s) freeAdvRoute;
		SLIST_HEAD(_FreeDynamicAdvRouteEntry, _rtl8651_tblDrvAdvRoute_s) freeDynamicAdvRoute;
#ifdef RTL865XB_WEB_CONTENT_HDR_FILTER
		SLIST_HEAD(_FreeContentFilterEntry, _rtl8651_tblDrv_contentFilter_s) freeContentFilter;
		SLIST_HEAD(_FreeHttpHdrFilterEntry, _rtl8651_tblDrv_httpHdrFilter_s) freeHttpHdrFilter;
#endif		
	} freeList;

	struct inuseEntryList_s {
		SLIST_HEAD( _InuseAlgEntry, rtl8651_tblDrv_algEntry_s) alg;
		CTAILQ_HEAD( _InuseNaptEntry, rtl8651_tblDrv_naptTcpUdpFlowEntry_s) tcpudp;

		/* policy */
		TAILQ_HEAD(_InusePolicyEntry, _rtl8651_tblDrvPolicy_s) policyRoute;
		//TAILQ_HEAD(_InusePolicyQos, _rtl8651_tblDrvPolicy_s) policyQos;

		TAILQ_HEAD( _InuseMCast, rtl8651_tblDrv_mCast_s) *mCastTbl;//[RTL8651_MULTICASTTBL_SIZE];
		SLIST_HEAD( _InuseProtocolBasedNat, rtl8651_tblDrv_protocolBasedNat_s) protocolBasedNat;
		SLIST_HEAD(_InuseNextHopEntry, rtl8651_tblDrv_nextHopEntry_s) nexthop;

		SLIST_HEAD(_InuseIpUnnumberEntry, rtl8651_tblDrv_ipUnnumbered_s) ipUnnumber;

		SLIST_HEAD(_InuseUrlPktFilterEntry, urlPktFiltering_s) urlPktFilter;

		/*ipFilter list For napt usr mapping
		* Hyking
		*2008-08-09
		*/
#ifdef RTL8651_NAPT_USR_MAPPING
		STAILQ_HEAD(_InuseNaptUsrMappingEntry, _rtl8651_tblDrv_naptUsrMapping_entry_s) inuseNaptUsrMappingEntry; //add for napt flow
		SLIST_HEAD(_InuseNaptUsrMappingTypePriority, _rtl8651_tblDrv_naptUsrMappingType_priority_s) inuseNaptUsrMappingTypePriority;
#endif
		SLIST_HEAD(_InuseNaptUsrMappingIpFilterEntry,rtl8651_tblDrv_naptUsrMapping_ipFilter_internal_s) inuseNaptUsrMappingIpFilter;

		SLIST_HEAD(_InuseAdvRouteEntry, _rtl8651_tblDrvAdvRoute_s) inuseAdvRoute;
		SLIST_HEAD(_InuseDynamicAdvRouteEntry, _rtl8651_tblDrvAdvRoute_s) inuseDynamicAdvRoute;
#ifdef RTL865XB_WEB_CONTENT_HDR_FILTER
		SLIST_HEAD(_InuseContentFilterEntry, _rtl8651_tblDrv_contentFilter_s) inuseContentFilter;
		SLIST_HEAD(_InuseHttpHdrFilterEntry, _rtl8651_tblDrv_httpHdrFilter_s) inuseHttpHdrFilter;
#endif
	} inuseList;

};

struct AsicTableUsage_S{
	rtl8651_tblDrv_macAddressEntry_t * mac;
//	rtl8651_tblDrv_ipTableAsicArrangementEntry_t * ip;
	rtl8651_tblDrv_arpAsicArrangementEntry_t * arp;
	//rtl8651_tblDrv_routeAsicArrangementEntry_t * route;
	rtl8651_tblDrv_serverPortAsicArrangementEntry_t *serverPort;
	rtl8651_tblDrv_algAsicArrangementEntry_t *alg; 
//	rtl8651_tblDrv_aclAsicArrangementEntry_t *acl; 
	////rtl8651_tblDrvprotoTrapAsicArrangementEnty_t *protoTrap;
};

#define RTL8651_TBLDRV_PRIVATE_EXTERN_VARIABLES
//---------------------------------------------------------------
//Driver private extern variables.
//---------------------------------------------------------------

extern rtl8651_tblDrv_global_t	rtl8651GlobalControl;
extern void (*rtl8651_linkStatusNotifier)(uint32 port, int8 linkUp);
extern struct AsicTableUsage_S AsicTableUsage;
extern uint32 linkChangePendingCount;
extern int32 miiPhyAddress;
extern rtl8651_tblAsic_ethernet_t 	rtl8651AsicEthernetTable[9];//RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum
extern rtl8651_tblDrvInitPara_t rtl8651_tblDrvPara;
extern rtl8651_tblAsic_InitPara_t rtl8651_tblDrvParaToAsicDrv;	/* Parameter to store the configuration for ASIC driver */
extern uint32 arpAgingTime; /* default 5 mins */
extern struct rtl8651_driverTables DrvTbl;
extern uint32 *_loggingModule;
extern int8 pureSoftFwd;
extern void (* rtl8651_tblDrvFwdSend)(void * data);//void * assumed to be packet header
extern ether_addr_t cachedDA;
extern uint32 phyPortLinkStatus;
extern struct rtl8651_driverTables DrvTbl;
extern uint32 _sysUpSeconds; /* System uptime in seconds */
//extern rtl8651_tblDrvFwdEngineCounts_t rtl8651_fwdEngineCounter;
extern int32 (*p_mtuHandler)(struct rtl_pktHdr *);
extern int32 rtl8651_mss; 
extern uint16 pppoe_mss[];

#if	RTL8651_IDLETIMEOUT_FIXED
typedef struct _rtl8651_sessionVlanId_s {
	uint16	vlanIDStart;
	uint16	vlanIDEnd;
	uint16	*vlanID;
	uint8 	idleTimeFlag;
	} _rtl8651_sessionVlanId_t;
#endif

/****************************************/
/*        url filter related            */
/****************************************/
#ifdef RTL865XB_URL_FILTER
extern int32 _urlFilterAclRuleNum;
#endif
extern urlFiltering_t _urlFilter;
#ifdef RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT
extern int8 rtl8651_urlStringUnknownType_caseSensitive;
#endif
#ifdef RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT
extern uint8 *rtl8651_urlDefaultAction;
#endif
extern uint8 *rtl8651_urlForwardLogging;

extern uint32 rtl8651_externalMulticastPortMask;

/* spanning tree */
extern uint32 rtl8651_tblDrvSpanningTreeEnable;			/* enable/disable spanning tree */
extern uint32 rtl8651_tblDrvSpanningTreePortFwdStatus;	/* the port list at the forwarding state */

/* idle timeout */
extern _rtl8651_sessionHangUp_t rtl8651_sessionHangUp[RTL8651_MAX_DIALSESSION];

/* URL filter */
extern urlFiltering_t _urlFilter;

#define RTL8651_TBLDRV_PRIVATE_MACROS
//---------------------------------------------------------------
//Driver private macros
//---------------------------------------------------------------


#define GET_NETIF_BY_VID(_netif, _vid)	(_netif) = DrvTbl.vlan[rtl8651_vlanTableIndex(_vid)].netif_t

#define TBL_MEM_ALLOC(tbl, type, size)	 \
	{ \
	(tbl) = (type *)rtlglue_malloc((size) * sizeof(type)); \
		if(!(tbl)){\
			rtlglue_printf("MEM alloc failed at line %d\n", __LINE__);\
			return FAILED;\
		}\
	}

#define LOCK_ERROR_CHECK(condition, reason)\
	if (condition) {\
		rtlglue_drvMutexUnlock();\
		return reason;\
	}
#define UNLOCK_ERROR_CHECK(condition, reason)\
	if (condition)\
		return reason;

#if defined (RTL865XB_DCUT_SWVLAN) || defined(CONFIG_RTL865XC)
#define rtl8651_swVlanTableIndex(vid) (vid)
#endif



#ifdef _RTL_FLOW_CACHE
#undef RTL8651_FlOWCACHE_DEBUG
typedef struct _rtl8651_flow {
	ipaddr_t		org_sip;
	uint16     		org_sport;
	ipaddr_t		org_dip;
	uint16 		org_dport;
	uint8 		isTcp;
	
	ipaddr_t		new_sip;
	uint16 		new_sport;
	ipaddr_t		new_dip;
	uint16 		new_dport;

	uint8		age;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *outboundFlow;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *inboundFlow;
} _rtl8651_flow_t;

#define DEFAULT_MAX_FLOWPKT_NUM  			1024
#define DEFAULT_MAX_FLOWCACHE_AGE			5
#endif



#define RTL8651_TBLDRV_PRIVATE_API_PROTOTYPE
//---------------------------------------------------------------
//Private API Prototype and related constant definition...
//---------------------------------------------------------------

int32 _rtl8651_findUrlFiltering(rtl8651_tblDrv_urlFilter_t *urlFilter);
int8 *_strncpy(int8 *dst0, const int8 *src0, int32 count);
int32 _strncmp(const int8 *s1, const int8 *s2, int32 n);

/* From rtl8651_tblDrv.c */
rtl8651_tblDrv_ipEntry_t  *_rtl8651_getIPTableEntry(ipaddr_t extip, ipaddr_t intip, uint8 attr);
rtl8651_tblDrv_arpEntry_t * _rtl8651_getArpEntry(rtl8651_tblDrv_networkIntfTable_t *netif_t, ipaddr_t ipAddr, int8 isRefresh);
rtl8651_tblDrv_networkIntfTable_t * _rtl8651_getNetworkIntfEntryByAddr(ipaddr_t ipAddr);
rtl8651_tblDrv_networkIntfTable_t * _rtl8651_getNetworkIntfEntryByVid(uint16 vid);
rtl8651_tblDrv_ipIntfEntry_t * _rtl8651_getIpIntfEntryByAddr(ipaddr_t ipAddr);
int32 _rtl8651_addLocalArp(int8 fromDrv, ipaddr_t ipAddr, ether_addr_t * macAddr, rtl8651_tblDrv_networkIntfTable_t *netIfPtr, uint32 port);
int8 _rtl8651_addLocalArpSpecifyNetworkInterface(int8 fromDrv, ipaddr_t ipAddr, ether_addr_t * macAddr, rtl8651_tblDrv_networkIntfTable_t * netIntfPtr, uint32 port);
int8 _rtl8651_getNetworkIntfLinkLayerType(int8 *ifName, uint32 *llType, uint32 *llIndex);
int8 _rtl8651_extIpAddr(ipaddr_t ipAddr, uint32 *netIdx);
int8 _rtl8651_extNetIpAddr(ipaddr_t ipAddr, uint32 *netIdx);

#define CHECK_SERVERPORT (1<<1)
int32 _rtl8651_ExtPortIsShared( uint32 isTcp, ipaddr_t ExtIp, uint16 ExtPort, uint16 assumedPort );

int8 _rtl8651_localServerIpAddr(ipaddr_t ipAddr);
int8 _rtl8651_intIpAddr(ipaddr_t ipAddr);
int8 _rtl8651_intNetIpAddr(ipaddr_t ipAddr);
int8 _rtl8651_isInternal(uint16 vid);
int32 _rtl8651_queryProtoTrap(uint8 protoType, uint16 protoContent);
int8 _rtl8651_getUsableExtIpAndPort(int8 , ipaddr_t , uint16 , ipaddr_t , uint16 , ipaddr_t *, uint16 *);
int8 _rtl8651_getIcmpUsableExtIpAndID(ipaddr_t sip, uint16 sID, ipaddr_t dip, ipaddr_t *gip, uint16 *gID, uint16 isNapt, uint16 isGipDefined);
int8 _rtl8651_getAllNatMapping(int32 *entry, rtl8651_tblDrv_natEntry_t *natMap);
int8 _rtl8651_getAllServerPort(int32 *entry, rtl8651_tblDrv_naptServerPortEntry_t *serverPort);
int8 _rtl8651_getUsableExtIpAndPort(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, ipaddr_t *extip, uint16 *extport);
int8 _rtl8651_getVIDByGWIpAddr(ipaddr_t gwIpAddr, uint32 *vid);
rtl8651_tblDrv_naptTcpUdpFlowEntry_t  * _rtl8651_addAlgCtrlConnection(int8 force, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
		ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort);
int32 _rtl8651_addNaptConnection(int8 fromDrv, int16 assigned, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort);
inline int32 _rtl8651_addAndGetNaptConnection(int8 fromDrv, int16 assigned, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort,
			struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s **ppe );
inline int32 _rtl8651_delNaptConnection( rtl8651_tblDrv_naptTcpUdpFlowEntry_t *delConn);
int32 _rtl8651_removeNaptConnectionByIP( ipaddr_t ip );
int32 _rtl8651_countNaptConnectionByIP( ipaddr_t ip, uint32 *tcpCnt, uint32 *udpCnt );
rtl8651_tblDrv_naptTcpUdpFlowEntry_t * _rtl8651_findNaptConnectionByExtIpPort( int8 isTcp, ipaddr_t ExtIp, uint16 ExtPort,
                                        ipaddr_t remoteIp, uint16 remotePort );
rtl8651_tblDrv_naptTcpUdpFlowEntry_t * _rtl8651_findNaptConnection( uint32 tblIndex, int8 isTcp, ipaddr_t intIp, uint16 intPort,
                                                          ipaddr_t remoteIp, uint16 remotePort );
int8 _rtl8651_addNaptIcmpCandidateFlow(int8 fromDrv, ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
			ipaddr_t insideGlobalIpAddr, uint16 insideGlobalId, ipaddr_t dstIpAddr);
int32 _rtl8651_addNaptIcmpNonCandidateFlow(int8 fromDrv, ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
			ipaddr_t insideGlobalIpAddr, uint16 insideGlobalId, ipaddr_t dstIpAddr ,uint16 isNapt);
int32 _rtl8651_flushIcmpPortBouncingEntry(void);
int32 _rtl8651_addALGRule( uint16 startPort, uint16 endPort, int8 isTcp, rtl8651_algEntry_property_t algtypes ); 
int32 _rtl8651_delALGRule( uint16 startPort, uint16 endPort, int8 isTcp, rtl8651_algEntry_property_t algtypes); 
int32 _rtl8651_getInboundAlgConnection( int8 isTcp, ipaddr_t gip, uint16 gport,
			ipaddr_t dip, uint16 dport, 
			struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s **ppEntry );
int32 _rtl8651_getOutboundAlgConnection( int8 isTcp, ipaddr_t sip, uint16 sport, 
			ipaddr_t dip, uint16 dport,
			struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s **ppEntry);

rtl8651_tblDrv_natEntry_t * _rtl8651_getNatExternalMappingEntry(ipaddr_t extIpAddr,struct rtl_pktHdr *phdr);
rtl8651_tblDrv_natEntry_t * _rtl8651_getNatInternalMappingEntry(ipaddr_t intIpAddr,struct rtl_pktHdr *phdr);

rtl8651_tblDrv_naptServerPortEntry_t *  _rtl8651_getOutsideNaptServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, uint32 dsid);								   
rtl8651_tblDrv_naptServerPortEntry_t *  _rtl8651_getInsideNaptServerPortMapping(int8 isTcp, ipaddr_t intIpAddr, uint16 intPort, uint32 dsid);
rtl8651_tblDrv_algEntry_t *_rtl8651_getAlgEntry( int8 isTcp, uint16 startPort, uint16 endPort , uint16 AlgType);
int32 _rtl8651_flushALGRule( void );
rtl8651_tblDrv_routeTable_t * _rtl8651_getRoutingEntry(ipaddr_t dstIpAddr);
int32  _rtl8651_getOutboundNaptFlow(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s **ppEntry);
int32  _rtl8651_getInboundNaptFlow(int8 isTcp, ipaddr_t gip, uint16 gport, ipaddr_t dip, uint16 dport,  struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s **ppEntry);
rtl8651_tblDrv_naptIcmpFlowEntry_t * _rtl8651_getInsideNaptIcmpEntry(ipaddr_t sip, uint16 sID, ipaddr_t dip, int8 isRefresh, int8 needFastFin, int8 isPortBouncingFlow, int8 isPBIcmpErrMsg);
rtl8651_tblDrv_naptIcmpFlowEntry_t * _rtl8651_getOutsideNaptIcmpEntry(ipaddr_t gip, uint16 gID, ipaddr_t dip, int8 isRefresh, int8 needFastFin, int8 isPortBouncingFlow);
void _rtl8651_collideNaptEntry(rtl8651DrvNaptLine_t *line,int32 *c1, int32 *c2);
_rtl8651_tblDrvAclRule_t * _rtl8651_aclLookUp(	rtl8651_tblDrv_networkIntfTable_t *netIfPtr,
												uint32 startIdx, int8 *ether_hdr, int8 *ip_hdr,
												int8 *defPermit, _rtl8651_tblDrvAclLookupInfo_t *info
												);
int32 _rtl8651_aclMaskOperation(rtl8651_tblDrvAclRule_t *inRule, rtl8651_tblDrvAclRule_t *outRule, int8 flag);
int32 _rtl8651_getDefaultAcl(int8 *ifName, int8 *defAction);

rtl8651_tblDrv_pppTable_t *_rtl8651_getPppoeSessionByIpAddr(ipaddr_t ipaddr);
rtl8651_tblDrv_pppTable_t * _rtl8651_getPppoeSessionByPppoeSessionId(uint16 pppoeSessionId);

ipaddr_t _rtl8651_getExtIpAddrByDsid(uint32 dsid);

/*naptUsrMapping related*/
#ifdef RTL8651_NAPT_USR_MAPPING

/*for internal using*/
int32 _rtl8651_reActiveAllNaptUsrMapEntryByType(uint32 type);
int32 _rtl8651_activeAllNaptUsrMappingEntry(void);
int32 _rtl8651_naptUsrMappingEntryUpdateTime(uint32 secPassed);
int32 _rtl8651_findInRangeNaptUsrMapEntry(uint16 direction, rtl8651_tblDrv_naptUsrMapping_t *usrMappInfo,_rtl8651_tblDrv_naptUsrMapping_entry_t **retUsrMappingEntry,uint32 *retPriotiry);
int32 _rtl8651_findIpEqualNaptUsrMapEntry(rtl8651_tblDrv_naptUsrMapping_t *usrMappInfo,_rtl8651_tblDrv_naptUsrMapping_entry_t **retUsrMappingEntry);
int32 _rtl8651_findNextInRangeNaptUsrMapEntry(uint16 direction, rtl8651_tblDrv_naptUsrMapping_t *usrMappInfo,
													_rtl8651_tblDrv_naptUsrMapping_entry_t *curMapEntry,
													_rtl8651_tblDrv_naptUsrMapping_entry_t **retUsrMappingEntry);
int32 _rtl8651_findNextIpEqualNaptUsrMapEntry(rtl8651_tblDrv_naptUsrMapping_t *usrMappInfo,
										_rtl8651_tblDrv_naptUsrMapping_entry_t *curMapEntry,
										_rtl8651_tblDrv_naptUsrMapping_entry_t **retUsrMappingEntry);
int32 _rtl8651_findDmzFilterEntry(uint32 SessionID, ipaddr_t naptIp, ipaddr_t dmzHostIp, rtl8651_tblDrv_naptUsrMapping_ipFilter_t * ipFilterEntry);
int32 _rtl8651_findNaptUsrMapPriorityByType(uint32 type,uint32 *priority);
int32 _rtl8651_delNaptUsrMappingEntryByMapEntry(_rtl8651_tblDrv_naptUsrMapping_entry_t *delEntry);

#endif
int32 _rtl8651_getDmzHost(uint32 dsid,ipaddr_t naptIp, ipaddr_t *dmzHostIp,ipaddr_t sip);
int32 _rtl8651_fromDmzHost(uint32 dsid, ipaddr_t dmzHostIp, ipaddr_t *naptIp,ipaddr_t dip);
int32 _rtl8651_findDmzFilterEntry(uint32 SessionID, ipaddr_t naptIp, ipaddr_t dmzHostIp, rtl8651_tblDrv_naptUsrMapping_ipFilter_t * ipFilterEntry);
int32 _rtl8651_findDmzEntry(uint32 SessionID, ipaddr_t naptIp, ipaddr_t dmzHostIp,int32 *index);

int32 _rtl8651_isLocalBcastAddr(uint32 vid, ipaddr_t ipAddr);
int32 _rtl8651_enableDoS(uint32 dostype, int8 enable, uint32 vlanMask);
#ifdef RTL865XB_WEB_CONTENT_HDR_FILTER
int32 _rtl8651_urlFilter(uint32 dsid, struct rtl_pktHdr *phdr, struct ip *pip, uint8 dir, int8 *string, int32 *strlen, rtl8651_tblDrv_naptTcpUdpFlowEntry_t *tb);
#else
int32 _rtl8651_urlFilter(uint32  dsid,struct rtl_pktHdr * phdr,struct ip * pip,uint8 dir, int8 * string,int32 * strlen);
#endif
void _rtl8651_removeArpAndNaptFlowWhenLinkDown(int32 port, uint32 *linkId) ;
void _rtl8651_removeArpByL2Entry(rtl8651_tblDrv_filterDbTableEntry_t *l2entry_t);

/* for 865xB B-Cut bug fix */
int32 _rtl8651_setVlanExtPortAsicAlwaysTag(rtl8651_tblDrv_vlanTable_t *vlanPtr, uint32 alwaysTag);

int32 _rtl8651_delVlanRefFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr);

rtl8651_tblDrv_algEntry_t*_rtl8651_l4FindLinkAlg(uint16 , uint16 , uint16);

int32 _rtl8651_updateNaptConnection(struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s *, int8 , uint32 );
int32 _rtl8651_getOutboundAlgCtrlFlow(int8 , ipaddr_t , uint16 , ipaddr_t , uint16 ,  struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s **); 

int32 _rtl8651_setl4NaptFindEmptyAsicRetry( uint16 retry );

int32 _rtl8651_getPppoeTblPtr(uint32 pppoeId,rtl8651_tblDrv_pppTable_t ** pppoePtr_P);
void _rtl8651_naptTimerAdaptiveAdjust(int8 enabled, uint32 threshold, uint32 secPassed);
void _rtl8651_naptSmartFlowSwapByPPS(int8 enabled, uint32 threshold);
int32 _rtl8651_isMacAccessControlPermit(ether_addr_t *sMac);

/* ip unnumber */
int32 _rtl8651_setLanSideExternalIpInterface(rtl8651_tblDrv_networkIntfTable_t *netif_t, ipaddr_t ipAddr, ipaddr_t ipMask, int8 isExternal);
void _rtl8651_reArrangeLanSideExternalIpInterfacePatch(void);
int8 _rtl8651_isLanSideIpUnnumberedIpAddr(ipaddr_t ipAddr);

/* multicast */
#ifdef RTL865xB_MCAST_CUTCD_TTL_PATCH
void _rtl8651_arrangeAllMulticast(void);
#endif
void _rtl8651_arrangeMulticastPortStatus(void);
void _rtl8651_timeUpdateMulticast(uint32 secPassed);
int32 _rtl8651_addMulticast(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport, uint32 dvidMask, uint32 mbr, ipaddr_t extIp, int8 toCpu, uint8 flag, uint32 linkMask);
int32 _rtl8651_addMulticastMember(ipaddr_t mcast_addr, uint32 vlanMask, uint32 mbr, uint32 linkMask);
int32 _rtl8651_delMulticast(ipaddr_t mcast_addr, uint32 vlanMask, uint32 mbr, uint32 linkMask);
rtl8651_tblDrv_mCast_t *_rtl8651_findMCast(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport);
uint32 _rtl8651_getExtPortMCastLinkMask(ipaddr_t mAddr, uint32 extPortMask);
int32 _rtl8651_delMulticastUpStream(ipaddr_t mcast_addr, ipaddr_t sip, uint16 svid, uint16 sport);
int32 _rtl8651_setMGroupAttribute(ipaddr_t groupIp, int8 toCpu);
int32 _rtl8651_delMulticastByVid(uint32 vid);
int32 _rtl8651_delMulticastByPort(uint32 port);

/* advance route */
int32 _rtl8651_addPolicyRoute(rtl8651_tblDrvPolicyRoute_t *rtAdd, uint32 dynamicPolicyRtAge);
int32 _rtl8651_getPolicyRoutingDSID(	struct rtl_pktHdr *pktHdr,
										ipaddr_t srcIp,
										ipaddr_t dstIp,
										uint8 ipProto,
										uint16 srcPort,
										uint16 dstPort);
rtl8651_tblDrv_routeTable_t *_rtl8651_getSoftRoutingEntry(	ipaddr_t srcIp,
															ipaddr_t dstIp,
															uint8 ipProto,
															uint16 srcPort,
															uint16 dstPort);
int32 _rtl8651_softNaptPolicyRoute(	ipaddr_t srcIp,
									ipaddr_t dstIp,
									uint8 ipProto,
									uint16 srcPort,
									uint16 dstPort,
									ipaddr_t *aliasIp,
									uint16 *aliasPort);
int32 _rtl8651_getExtIpByPolicyRoute(	ipaddr_t srcIp,
										ipaddr_t dstIp,
										uint8 ipProto,
										uint16 srcPort,
										uint16 dstPort,
										ipaddr_t *aliasIp);
int32 _rtl8651_softIcmpPolicyRoute(	ipaddr_t srcIp,
									ipaddr_t dstIp,
									uint16 Id,
									ipaddr_t *aliasIp,
									uint16 *aliasId);

int32 _rtl8651_getDemandRoutingEntry(	ipaddr_t srcIp,
										ipaddr_t dstIp,
										uint8 ipProto,
										uint16 srcPort,
										uint16 dstPort);

uint32 _rtl8651_SessionIDToDSID(uint32 SessionID);
rtl8651_tblDrv_pppTable_t *_rtl8651_getPppoeSessionByDsid(uint32 dsid);
int32 _rtl8651_getpppDSID(uint32 ip_alias);

/* Type of Dos Definition */
#define RTL8651_TCP_SYNFLOOD			0x00000001
#define RTL8651_ICMP_FLOOD				0x00000002
#define RTL8651_UDP_FLOOD				0x00000004
#define RTL8651_IP_SPOOFING				0x00000008
/* #define RTL8651_TCP_SYNWITHDATA		0x00000010 */
/* #define RTL8651_TCP_LANDATTACK		0x00000020 */
/* #define RTL8651_ICMP_SMURF			0x00000040 */
#define RTL8651_UDP_ECHOCHARGENBOMB		0x00000080
#define RTL8651_TCP_FULLXMASSCAN		0x00000100
#define RTL8651_TCP_FULLSAPUSCAN		0x00000200
#define RTL8651_TCP_FINSCAN				0x00000400
#define RTL8651_TCP_SYNFINSCAN			0x00000800
#define RTL8651_TCP_NULLSCAN			0x00001000
#define RTL8651_TCP_NMAPXMASSCAN		0x00002000
#define RTL8651_TCP_VECNASCAN			0x00004000
#define RTL8651_TCP_STEALTHFIN			0x00008000
#define RTL8651_TCPUDP_PORTSCAN			0x00010000

/* Switch for Features */
#define RTL865X_PSEUDO_VLAN


/* For Netmeeting ALG */
int32 _rtl8651_removeNaptConnectionByIntIPandPort( int8 isTcp, ipaddr_t intIp, uint16 intPort );

/* Timer1 used */
extern uint32 tuRemind;
extern uint32 accJiffies;
extern uint32 timer1Sec;

/* From rtl8651_ayer2.c */
int32 _rtl8651_delVlanRefFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr);
void _rtl8651_refillOrClearAsicNaptEntry(uint32 tblIdx, rtl8651_tblDrv_naptTcpUdpFlowEntry_t *selectedRefillFlow);
int32 _rtl8651_addVlanRefFilterDatabaseEntry(uint16 l2Type, uint16 fid, uint16 vid, ether_addr_t *macAddr, uint32 type, uint32 portMask);
int32 _rtl8651_modifyVlanRefFilterDatabaseEntry(uint32 fid, ether_addr_t *mac, uint32 *memberPortMask, uint16 *process, uint32 *linkId, uint32 attr);
int32 _rtl8651_addVlanRefFilterDatabaseEntry(uint16 l2Type, uint16 fid, uint16 vid, ether_addr_t *macAddr, uint32 type, uint32 portMask) ;
rtl8651_tblDrv_filterDbTableEntry_t *_rtl8651_getVlanFilterDatabaseEntry(uint16 vid, ether_addr_t *macAddr);

/* for Extension device L2 process */
extern uint32 peripheralExtPortMask;
int32 _rtl8651_addPeripheralExtPort(uint32 port);
int32 _rtl8651_delPeripheralExtPort(uint32 port);

/* for test code */
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
int32 rtl8651_getDrvMutex(void);
int32 rtl8651_reinitDrvMutex(void);
int32 rtl8651_getMbufMutex(void);
int32 rtl8651_reinitMbufMutex(void);
#endif /* RTL865X_TEST */



/* ACL DB */
typedef int32 (*acldb_callback_t) (int32 , struct rtl_pktHdr *, struct ip *, void *);

struct rtl8651_ACLDB_s
{
	acldb_callback_t func;
	void *userDefined;
	uint32 count;
};
typedef struct rtl8651_ACLDB_s rtl8651_ACLDB_t;

extern rtl8651_ACLDB_t acldb[];

int32 _rtl8651_resetACLDB(void);
int32 _rtl8651_registerACLDB( int32 ruleNo, 
                              acldb_callback_t func,
                              void *userDefined );
int32 _rtl8651_jumptoACLDB( int32 ruleNo, struct rtl_pktHdr *pkthdr, struct ip *pip );
#define RTL865XB_ACLDB


/* rate limit counter */
#if	RTL8651_IDLETIMEOUT_FIXED
int32 _rtl8651_addRateLimitCounter(rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType, uint16 vid);
int32 _rtl8651_delRateLimitCounter(rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType, uint16 vid);
int32 _rtl8651_getRateLimitCounter(rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType, uint16 vid, uint8 refill);
int32 _rtl8651_addRateLimitCounterRule(rtl8651_tblDrvAclRule_t *rule, rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType, uint16 vid);
int32 _rtl8651_delRateLimitCounterRule(rtl8651_tblDrvAclRule_t *rule, rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType, uint16 vid);
#else
int32 _rtl8651_addRateLimitCounter(rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType);
int32 _rtl8651_delRateLimitCounter(rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType);
int32 _rtl8651_getRateLimitCounter(rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType, uint8 refill);
int32 _rtl8651_addRateLimitCounterRule(rtl8651_tblDrvAclRule_t *rule, rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType);
int32 _rtl8651_delRateLimitCounterRule(rtl8651_tblDrvAclRule_t *rule, rtl8651_tblDrv_networkIntfTable_t *netif_t, uint8 counterType);
#endif
void _rtl8651_rateLimitCounterTimeUpdate(uint32 secpassed);
void _rtl8651_flushRateLimitCounter(void);

int32 _rtl8651_changeNaptIpAndPort( struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s * thisFlow,
                                    ipaddr_t newIntIp, uint16 newIntPort,
                                    ipaddr_t newExtIp, uint16 newExtPort,
                                    ipaddr_t newRemIp, uint16 newRemPort
                                    );


rtl8651_tblDrv_networkIntfTable_t *_rtl8651_getIpInfo(ipaddr_t ipaddr, rtl8651_tblDrv_ipIntfEntry_t **ipintf_t, int8 *is_gip, int8 *is_gwip);

/* idle timeout */
void _rtl8651_recordTraffic(int32 dsid, int32 isXmit);
int32 _rtl8651_setSessionHangUp(rtl8651_sessionHangUp_t *param);


/* NAT Type
 *
 */
extern enum NAT_TYPE rtl8651_drvNatType;

/* External Port Strategy 
 *  The strategy to select external port andress.
 */
enum EXTPORT_STRATEGY
{
	PORT_UNIQUE = 0, /* Each pair of {intIp, intPort, remIp, remPort} uses different pair of {extIp, extPort} */
	PORT_REUSE,      /* For the all pairs with the same {intIp, intPort} use the same {extIp, extPort} */
	PORT_KEEPING,    /* The same with PORT_REUSE and try to add connection with extPort equal to intPort */
};
extern enum EXTPORT_STRATEGY rtl8651_drvTcpExtPortStrategy;
extern enum EXTPORT_STRATEGY rtl8651_drvUdpExtPortStrategy;
extern uint8* _strExtPortStrategy[];

/* Inbound Check
 *  The rule to check the {remIp, remPort} of inbound packet.
 */
enum INBOUND_CHECK
{
	NAIVE_CHECK = 0,              /* Follow the original design: SW is symmetric. ASIC H1 is symmetric. ASIC H2 is Full-Cone. */ 
	ALLOW_ANY,                    /* Any {remIp, remPort} packet can enter into {intIp, intPort} */
	RESTRICTED_IP_ONLY,           /* Only restricted {remIp, *} packet can enter into {intIp, intPort} */
	RESTRICTED_IP_AND_PORT_ONLY,  /* Only restricted {remIp, remPort} packet can enter into {intIp, intPort} */
};
extern enum INBOUND_CHECK rtl8651_drvTcpInboundCheck;
extern enum INBOUND_CHECK rtl8651_drvUdpInboundCheck;
extern uint8* _strInboundCheck[];

void _rtl8651_updateTcpStrategy( void );
void _rtl8651_updateUdpStrategy( void );

int32 _rtl8651_RearrangeHWTrafficCounter(void);
int32 _rtl8651_createProtocolBasedVLANforIPXPassthru(void);
int32 _rtl8651_createProtocolBasedVLANforNetbiosPassthru(void);
int32 _rtl8651_delMulticastByVid(uint32 vid);
int32 _rtl8651_AvoidAmbiguousHash2InAsic( rtl8651_tblDrv_naptTcpUdpFlowEntry_t *newConn,
                                          int *bInboundAmbiguous, int *bOutboundAmbiguous,
                                          int *bInboundAsicEmpty, int *bOutboundAsicEmpty );
int32 _rtl8651_initACLDB(void);


#ifdef _RTL_FLOW_CACHE
uint32 _rtl8651_flowCacheInit(uint32 cache_num, uint32 age);
void _rtl8651_flowCacheReinit(void);
_rtl8651_flow_t *_rtl8651_getFlowCacheEntry(uint8 isTcp, ipaddr_t org_sip, uint16 org_sport, ipaddr_t org_dip, uint16 org_dport);
void _rtl8651_updateFlowCache(uint8 isTcp, ipaddr_t org_sip, uint16 org_sport, ipaddr_t org_dip, uint16 org_dport, 
											ipaddr_t new_sip, uint16 new_sport, ipaddr_t new_dip, uint16 new_dport,
											rtl8651_tblDrv_naptTcpUdpFlowEntry_t *outboundFlow, rtl8651_tblDrv_naptTcpUdpFlowEntry_t *inboundFlow);
void _rtl8651_flowCache_timeUpdate(uint32 secPassed);
#endif

/* Protocol Based NAT */
int32 _rtl8651_getInboundProtocolBasedNAT( uint8 protocol, ipaddr_t ExtIp, ipaddr_t *IntIp );
int32 _rtl8651_getOutboundProtocolBasedNAT( uint8 protocol, ipaddr_t *ExtIp, ipaddr_t IntIp );

/* CLASS A/B patching */
int32 _rtl8651_ipClassMonitoring(ipaddr_t dip, rtl8651_tblDrv_arpEntry_t *arp_t);

/* DiffServ DSCP remarking */
int _rtl8651_markDiffservPkt(_rtl8651_tblDrvAclRule_t *rule, struct ip *pip);
_rtl8651_tblDrvAclRule_t * _rtl8651_matchDiffservACL(	uint32 dsid,
														struct rtl_pktHdr *pkthdrPtr,
														struct ip *iphdr,
														rtl8651_tblDrv_vlanTable_t *pVlan);
#if RTL865XC_QOS_OUTPUTQUEUE
extern int8 rtl8651_diffServEnable ;
_rtl8651_tblDrvAclRule_t * _rtl8651_matchQosACL(	uint32 dsid,
														struct rtl_pktHdr *pkthdrPtr,
														struct ip *iphdr,
														rtl8651_tblDrv_vlanTable_t *pVlan);
int32 _rtl8651_setDSCPEnable(int8 enable);
int32 _rtl8651_set802D1PRemarkEnable(int8 enable);
int _rtl8651_remarkPkt(_rtl8651_tblDrvAclRule_t *rule, struct ip *pip,struct rtl_mBuf *mbuf);													
#endif														


int32 _rtl8651_delLocalArp(int8 fromDrv, ipaddr_t ipAddr, rtl8651_tblDrv_networkIntfTable_t *netIfPtr);
rtl8651_tblDrv_pppTable_t *_rtl8651_getPPPInfo(uint32 sessionId, ipaddr_t ipaddr);

#if RTL865XC_QOS_OUTPUTQUEUE
int32 _rtl865xC_setOutputQueue(struct rtl_pktHdr *pkthdrPtr);
#endif

int32 _rtl8651_findHash2ToNexthop(ipaddr_t nexthop, ipaddr_t naptip);
void _rtl8651_naptSmartFlowSwapByPPSTcp( uint32 threshold );
void _rtl8651_naptSmartFlowSwapByPPSUdp( uint32 threshold );
void _rtl8651_naptSmartFlowSwapByPPSCdrouter( uint32 threshold );
int32 rtl8651_setNaptTcpFinTimeout(uint32 timeout);




#endif /* RTL8651_TBLDRV_LOCAL_H */


