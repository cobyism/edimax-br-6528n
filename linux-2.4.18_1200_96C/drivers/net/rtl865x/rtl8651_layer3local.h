/////////////////////////////////////////////////////////////////
////
//// This file is to be included only by rtl8651_tblDrvLocal.h
//// This file exists only to split lengthy driver definitions to several organized header files
////
////  !!!Only  put Layer 3 driver specific 
////              1) local data types, 2) #define, and 3)macros 
////        in this file
////
//// Leave all "externs" and functions prototypes to rtl8651_tblDrvlocal.h 
//// If the API or datatype is to be exported outside the driver,
//// Please put it in rtl8651_layer2.h or rtl8651_tblDrv.h 
////
/////////////////////////////////////////////////////////////////

#ifdef RTL8651_TBLDRV_LOCAL_H   

#ifndef RTL86561_LAYER3_LOCAL_H
#define RTL86561_LAYER3_LOCAL_H

struct rtl8651_tblDrv_ipUnnumbered_s;

#ifdef CONFIG_RTL865XB_EXP_INVALID
typedef struct rtl8651_tblDrv_policyRoute_s {
	rtl8651_tblDrvAclRule_t			rule;				/* ACL Rule */
	ipaddr_t						naptip;			/* NAPT IP address */
	int32 (*p_callBack)(uint32);						/* call back function, meaningful if demand route */
	uint32						age;				/* againg timer, 0: already age out */
	uint32 						aclIndex;			/* index of ASIC ACL table */
	rtl8651_tblDrv_nextHopEntry_t	*nexthop_t;		/* pointer to nexthop table entry */
	SLIST_ENTRY(rtl8651_tblDrv_policyRoute_s) 	next;
} rtl8651_tblDrv_policyRoute_t;


typedef struct rtl8651_tblDrv_natEntry_s {
	ipaddr_t localIp;								/* Internal IP address */
	ipaddr_t globalIp;								/* External IP address */
	rtl8651_tblDrv_nextHopEntry_t *nexthop_t;			/* pointer to nexthop table entry */
	SLIST_ENTRY(rtl8651_tblDrv_natEntry_s) nextNatEntry;
} rtl8651_tblDrv_natEntry_t;

#endif /* CONFIG_RTL865XB_EXP_INVALID */




/*=============================================
  * IP Address Entry Structure
  *=============================================*/
#define RTL865X_IP_ADDRESS_ENTRY_DATA_STRUCTURE

typedef struct rtl8651_tblDrv_ipIntfIpAddrEntry_s {
	ipaddr_t ipAddr;
	SLIST_ENTRY(rtl8651_tblDrv_ipIntfIpAddrEntry_s) nextIpAddr;
}rtl8651_tblDrv_ipIntfIpAddrEntry_t;





/*=============================================
  * PPPoE, PPTP, L2TP Data Structure
  *=============================================*/
#define RTL865X_PPPOE_PPTP_L2TP_DATA_STRUCTURE

struct rtl8651_tblDrvSession_s;

typedef struct rtl8651_tblDrv_pppTable_s {
	struct rtl8651_tblDrv_networkIntfTable_s 	*netif_t;
	struct rtl8651_tblDrvSession_s			*session_t;
	uint32 	sessionId;
	uint32 	mss;
	uint16	vid;
	uint16	type:5,				/* RTL8651_PPPOE, RTL8651_PPTP, RTL8651_L2TP */
			valid:1,
			initiated:1;
	rtl8651_tblDrv_filterDbTableEntry_t * macInfo;
	uint8	dsid; //SHOULD remove it in new model.
	uint32	txPackets, txBytes;
	uint32	rxPackets, rxBytes, rxErrors;
	uint32	drops;
	union {
		struct {
			uint16 	pppoeSessionId__;
		} pppoe;
		struct {
			uint16 	callId__;
			uint16	peerCallId__;
			uint32 	lastSeq__;
			uint32	lastAck__;
			uint32	delta__;
			uint32	lastReceiveSeq__;
			ipaddr_t	serverIp__;
			ipaddr_t	defaultGwIp__;
			uint8*	greBuf1__;
			uint8*	greBuf2__;
			uint8	greOffset2__;
		} pptp;
		struct {
			uint16	tunnelId__;
			uint16	l2tpSessionId__;
			ipaddr_t	serverIp__;
			ipaddr_t	defaultGwIp__;
			uint8*	l2tpBuf1__;
			uint8	l2tpLengthBit__;
			uint8	l2tpOffset__;
			uint8	l2tpBuf1State__;
			uint8*	l2tpBuf2__;
			uint8	l2tpOffset2__;
		} l2tp;
	} un;
} rtl8651_tblDrv_pppTable_t;

#define pppoeSessionId_		un.pppoe.pppoeSessionId__
#define callId_					un.pptp.callId__
#define peerCallId_				un.pptp.peerCallId__
#define pptpServerIp_			un.pptp.serverIp__
#define pptpDefaultGwIp_		un.pptp.defaultGwIp__
#define tunnelId_				un.l2tp.tunnelId__
#define l2tpSessionId_			un.l2tp.l2tpSessionId__
#define l2tpServerIp_			un.l2tp.serverIp__
#define l2tpDefaultGwIp_			un.l2tp.defaultGwIp__
#define lastSeq_				un.pptp.lastSeq__
#define lastAck_				un.pptp.lastAck__
#define delta_					un.pptp.delta__
#define lastReceiveSeq_			un.pptp.lastReceiveSeq__
#define greBuf1_				un.pptp.greBuf1__
#define greBuf2_				un.pptp.greBuf2__
#define greOffset2_			un.pptp.greOffset2__
#define l2tpBuf1_				un.l2tp.l2tpBuf1__
#define l2tpLengthBit_		un.l2tp.l2tpLengthBit__
#define l2tpOffset_			un.l2tp.l2tpOffset__
#define l2tpBuf1State_		un.l2tp.l2tpBuf1State__
#define l2tpBuf2_				un.l2tp.l2tpBuf2__
#define l2tpOffset2_			un.l2tp.l2tpOffset2__

typedef struct rtl8651_tblDrvSession_s {
	rtl8651_tblDrv_pppTable_t			*session_t;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t 		*ip_t; //point to Session IP address
	CTAILQ_ENTRY(rtl8651_tblDrvSession_s) nextSession;
} rtl8651_tblDrvSession_t;

#define PPPOE_TABLE_INDEX(entry)		(entry - DrvTbl.pppoe)



typedef struct rtl8651_tblDrv_arpEntry_s {
	ipaddr_t	ipAddr;
	rtl8651_tblDrv_filterDbTableEntry_t * macInfo;
	uint32	routingTablePos;//Meaningful when allocType == 2
	uint16	vid;
	uint32	//allocType:2,	//Allocation type, 1: Local arp table 2: Routing table
			fromDrv:1,	//Whether entry is recorded by driver arp engine. If fromDrv == 1, allocType == 1
			fromApp:1,	//Whether entry is recorded by applications
			dmzIpPending:1, //Arp pending due to out of IP Table
			isBcArp:1, //Broadcast arp
			routeAllocated:1;//ASIC routing table allocated
	uint32 age;//Seconds as unit
	SLIST_ENTRY(rtl8651_tblDrv_arpEntry_s) next;
} rtl8651_tblDrv_arpEntry_t;


typedef struct rtl8651_tblDrv_ipIntfEntry_s {
	ipaddr_t	ipMask;
	uint32	arpAllocated:1,		//ASIC arp table allocated
			routeAllocated:1,//ASIC routing table allocated
			lanSideExternal:1,//1: Lan Side external IP interface for IP Unnumbered
			networkType:3;	
	uint32 arpStartPos, arpEndPos;
	uint32 routingTablePos;
	struct rtl8651_tblDrv_ipUnnumbered_s *ipUnnumber;	// ip unnumber control block (exist when lanSideExternal == TRUE)
	SLIST_HEAD(, rtl8651_tblDrv_ipIntfIpAddrEntry_s) ipAddr;
	SLIST_HEAD(, rtl8651_tblDrv_ipIntfIpAddrEntry_s) localServer;
	SLIST_ENTRY(rtl8651_tblDrv_ipIntfEntry_s) nextIp;
	SLIST_HEAD( , rtl8651_tblDrv_arpEntry_s) nextArp;
} rtl8651_tblDrv_ipIntfEntry_t;

typedef struct rtl8651_tblDrv_natEntry_s {
	ipaddr_t localIp;
	ipaddr_t globalIp;
	SLIST_ENTRY(rtl8651_tblDrv_natEntry_s) nextNatEntry;
} rtl8651_tblDrv_natEntry_t;




struct rtl8651_tblDrv_ipEntry_s;
typedef struct rtl8651_tblDrv_nextHopEntry_s {		/* Next Hop Table */
	uint16 	dvid; 							/* Destination VLAN ID */
	uint16	nextHopType:3,					/* 0:RTL8651_LL_NONE, 1: RTL8651_LL_VLAN, 2:RTL8651_LL_PPPOE */
			rt_flag:1,							/* 1: rt references */
			valid:1,							/* 0: Invalid, 1: Invalid */
			v:1;								/* For FPGA */
	uint32	refcount;							/* Reference Count */
	uint32	entryIndex;						/* Entry Index */
	ipaddr_t	nexthop;							/* Next Hop IP address */
	void		*nexthop_t;						/* ethernet: pointer to arp, pppoe: pointer to pppoe */
	struct rtl8651_tblDrv_ipEntry_s   *ip_t;			/* to find IP Table entry */
	SLIST_ENTRY(rtl8651_tblDrv_nextHopEntry_s) next;
} rtl8651_tblDrv_nextHopEntry_t;

#define NEXTHOP_LP					0x01
#define NEXTHOP_SP					0x02
#define NEXTHOP_NAT					0x03
#define NEXTHOP_NAPT_H2				0x04
#define NEXTHOP_L3					0x05
#define NEXTHOP_POLICY				0x06
#define NEXTHOP_NAPT_H1				0x07
#define SET_NEXTHOP_IPIDX			0x01
#define RESET_NEXTHOP_IPIDX			0x02



//#define IP_UNNUMBER_MAX_SESSION			4
#define IP_UNNUMBER_MAX_ASIC_IP_ENTRY	8

/*	max patch ACL rule count for IP unnumber, 64 is gotten from the air
	(if the ACL rule for unnumber patch has exceeded, we patch it by routing table) */
#define IP_UNNUMBER_MAX_ASIC_ACL_PATCH	(125 - 64)

/* global IP unnumber control block */
typedef struct rtl8651_ipUnnumbered_ctrlBlock_s {
	uint32 ip_unnumber_LS_budget;	/* total budget in ASIC IP Local Server entry for IP unnumber */
	uint32 ip_unnumber_ACL_budget;	/* total budget in ASIC ACL entry for IP unnumber (ACL rate limit count patch is not included) */
} rtl8651_ipUnnumbered_ctrlBlock_t;

/* per unnumbered IP interface */
typedef struct rtl8651_tblDrv_ipUnnumbered_s {
	ipaddr_t 	ipAddr;
	ipaddr_t	ipMask;
	//ipaddr_t 	gwIp;
	uint32 ip_as_LS;
	uint8 ip_as_ACL_budget;		/* ALL budget for unnumber IPs patched by ACL */
	rtl8651_tblDrv_ipIntfEntry_t *ipIntf_t;
	SLIST_ENTRY(rtl8651_tblDrv_ipUnnumbered_s) nextIpUnnumberEntry;
} rtl8651_tblDrv_ipUnnumbered_t;

#if 0
/* IpUn: */
#define RTL8651_UNNUMBER_LAN						0x01
#define RTL8651_UNNUMBER_WAN						0x02
#endif

/*=============================================
  * Internal External Table Data Structure
  *=============================================*/
#define RTL865X_IP_TABLE_DATA_STRUCTURE

typedef struct rtl8651_tblDrv_ipEntry_s {
	ipaddr_t	intip;						/* Internal IP Address */
	ipaddr_t	extip;						/* External Ip Address */
	uint32 		type:2,						/* see below definition */
				def_flag:1,					/* 1: use default route's nexthop */
				valid:1,						/* Is entry valid */
				defNaptIp:1;					/*1: this IP is default NATP IP*/
	
	uint32		ref_cnt;					/* for NAPT only, NAPT hash 2 */
	struct rtl8651_tblDrv_nextHopEntry_s *nextHop_t;
} rtl8651_tblDrv_ipEntry_t;

#define IP_TABLE_INDEX(entry)				(entry - DrvTbl.iptbl)

/* type: */
#define IP_ATTR_NAPT						0x00
#define IP_ATTR_NAT							0x01
#define IP_ATTR_LS							0x02
#define IP_ATTR_ALL							0x03

#define IP_ATTR_NONE						0xff
/*for default npat IP*/
#define IP_DEFNAPT_IP						0x80
#define IP_DEFNAPT_MASK						0x80



/*=============================================
  * Network interface 
  *=============================================*/
#define RTL865X_NETWORK_INTERFACE_DATA_STRUCTURE

struct rtl8651_tblDrv_rateLimit_s;
typedef struct rtl8651_tblDrv_networkIntfTable_s {
	int8		ifName[RTL8651_NETINTF_NAME_LEN];
	uint16	vid;
	uint32 	valid:1,//Whether this interface is valid
			flag:1, 			//0: added by rtl8651_addNetworkIntf() API
#ifdef CONFIG_RTL865XB_EXP_INVALID
			IpUn:2, 			//IP Unnumbered:  0: not IP unnumbered, otherwise see below
#endif /* CONFIG_RTL865XB_EXP_INVALID */
			linkLayerType:3; //Which kind of link layer. 0: Not attached, 1: VLAN 2: pppoe
	uint32	pppoeAge;
	uint16	mtu;
	int8		defAclAction;
	ipaddr_t netMask; /* for Ip Unnumbered NetMask */
	SLIST_HEAD(, rtl8651_tblDrv_ipIntfEntry_s) IpHead; //Network belong to this IP interface
	SLIST_HEAD(, rtl8651_tblDrv_natEntry_s) nextNat;
	SLIST_HEAD(, rtl8651_tblDrv_naptServerPortEntry_s) nextNaptServerPort;	
	CTAILQ_HEAD(, rtl8651_tblDrvSession_s) sessionHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s) inAclHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s) outAclHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvPolicy_s) policyQosHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s) inRateLimitHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s) outRateLimitHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s) mnqHead;
	CTAILQ_HEAD(, _rtl8651_tblDrvAclRule_s) qosHead;
} rtl8651_tblDrv_networkIntfTable_t;



/*=============================================
  * L3 Routing Table Data Structure
  *=============================================*/
#define RTL865X_ROUTING_TABLE_DATA_STRUCTURE

struct rtl8651_tblDrv_policyRoute_s;
typedef struct rtl8651_tblDrv_routeTable_s {
	ipaddr_t 	ipAddr;						/* Destination IP Address */
	ipaddr_t	ipMask;						/* Network mask */
	ipaddr_t 	nextHop;					/* next hop IP address */
	uint32 		process:4,					/* 000: PPPoE, 001: L2, 010: ARP, 100: CPU, 101: NextHop, 110: Drop*/
				ifrt_flag:1,				/* 1: Interface route */
				nextHopType:5,				/* Next Hop type */
				ipClass:1,						
				valid:1;
	uint16		dvid;							/* Destination Vlan ID */
	uint32            ref_count;                       /*referrence count*/
	rtl8651_tblDrv_ipIntfEntry_t *ipintf_t;		/* pointer to IP interface */
	rtl8651_tblDrv_networkIntfTable_t *netif_t;	/* pointer to network interface */

	/* nexthop informaiton */
	union {
		struct {
			uint32	arpsta;						/* ARP Table Starting address */
			uint32	arpend;						/* ARP Table Ending address */
#ifdef CONFIG_RTL865XC
			uint32	arpIpIdx;					/* External IP selection index */
#endif
		} arp;
		struct {
			union {
				struct {
					rtl8651_tblDrv_arpEntry_t *arp_t;			/* NextHop Information for process=direct */
				} direct;
				struct {
					rtl8651_tblDrv_pppTable_t *session_t;
				} session; 
			} un1;
			rtl8651_tblDrv_nextHopEntry_t *start_t;	/* pointer to Nexthop table: starting range */
			rtl8651_tblDrv_nextHopEntry_t *end_t;		/* pointer to Nexthop table: ending range */
			rtl8651_tblDrv_nextHopEntry_t *next_t;		
			uint8 nhalog;							/* algo. for load balance */
			uint8 ipDomain;						/* IP domain */
		} nxthop;
		struct {
			struct rtl8651_tblDrv_policyRoute_s *policy_t;
		} policy;
	} un;
	rtl8651_tblDrv_nextHopEntry_t *rt_nexthop_t;
#ifdef RTL865X_ROUTE_REFERRENCE_CONUT
	int32 referrence_count;
#endif
} rtl8651_tblDrv_routeTable_t;


#define L3_TABLE_INDEX(entry)		(entry-DrvTbl.route)

/* process: */
#define RT_PPPOE					0x00
#define RT_L2						0x01
#define RT_ARP					0x02
#define RT_CPU					0x04
#define RT_NEXTHOP				0x05
#define RT_DROP					0x06
#define RT_POLICY					0x07
#define RT_SESSION				0x08

/* nhalog: */
#define RT_ALOG_PACKET			0x00
#define RT_ALOG_SESSION			0x01
#define RT_ALOG_SIP				0x02

/* ipDomain: */
#define RT_DOMAIN_4_1				0x00
#define RT_DOMAIN_4_2				0x01
#define RT_DOMAIN_4_3				0x02
#define RT_DOMAIN_4_4				0x03
#define RT_DOMAIN_8_1				0x04
#define RT_DOMAIN_8_2				0x05
#define RT_DOMAIN_16_1				0x06

/* process: ARP */
#define rt_nharpsta					un.arp.arpsta
#define rt_nharpend					un.arp.arpend
#ifdef CONFIG_RTL865XC
#define rt_nharpIpIdx				un.arp.arpIpIdx
#endif

/* process: Direct */
#define rt_nharp_t					un.nxthop.un1.direct.arp_t
/* process: Session-Based */
#define rt_nhSession_t				un.nxthop.un1.session.session_t

/* process: Nexthop */
#define rt_nhstart_t				un.nxthop.start_t
#define rt_nhend_t					un.nxthop.end_t
#define rt_nhnext_t					un.nxthop.next_t
#define rt_nhalog					un.nxthop.nhalog
#define rt_ipDomain					un.nxthop.ipDomain
#define rt_policy_t					un.policy.policy_t




/*=============================================
  * IP CLASS A/B Data Structure
  * In order to have ASIC acceleration in Class A/B subnet, this 
  * data structure is used to monitor heavy load connection and
  * swap it to ASIC L3 Table.
  *=============================================*/
#define RTL865X_IP_CLASS_DATA_STRUCTURE

typedef struct ipClassABMonitorTable_s {
	ipaddr_t	dip;
	uint32	value;
	rtl8651_tblDrv_arpEntry_t *arp_t;
} ipClassABMonitorTable_t;

#define load 				value
#define l3Pos				value

#define IP_CLASS_MAX_ENTRY	10
#define IP_CLASS_THRESHOLD	500    /* packets per second */




typedef struct rtl8651_tblDrv_arpAsicArrangementEntry_s {
	int16	valid;
} rtl8651_tblDrv_arpAsicArrangementEntry_t;






/*================================================
  * Policy Structure
  *================================================*/
#define TBLDRV_POLICY_DATA_STRUCTURE

typedef struct _rtl8651_tblDrvPolicy_s {
	uint32 		proto;					
	ipaddr_t	sip, sipMask,			
				dip, dipMask;
	uint16		sport_start, sport_end,
				dport_start, dport_end;
	ipaddr_t	naptIp; 
	uint32 	advRtType;	/*converted from demand route or policy route*/
	//uint32		sessionId;
	struct _rtl8651_tblDrvAdvRoute_s* advRoute_t;

	rtl8651_tblDrv_nextHopEntry_t *nexthop_t;
	uint32 aclIndex;					/*index of the corresponding acl rule in acl asic table*/
	rtl8651_tblDrv_pppTable_t *pppoe_t;
	int32 (*p_handler)(uint32);					/* call back function: not NULL: demand route */
	uint32		priority;

	SLIST_HEAD(, rtl8651_tblDrv_naptTcpUdpFlowEntry_s) naptFlow;
	CIRCLEQ_ENTRY(_rtl8651_tblDrvPolicy_s)	nextTime_t;
	CTAILQ_ENTRY(_rtl8651_tblDrvPolicy_s) 	nextPolicy_t;
} _rtl8651_tblDrvPolicy_t;

typedef struct _rtl8651_tblDrvDynamicPolicy_s {
	_rtl8651_tblDrvPolicy_t	*policy_t;
	TAILQ_HEAD(, _rtl8651_tblDrvPolicy_s) *hashTable;
	CIRCLEQ_HEAD(, _rtl8651_tblDrvPolicy_s) dyHead;
} _rtl8651_tblDrvDynamicPolicy_t;

#define qos_high		naptip

/*================================================
  * Multicast Data Structure
  *================================================*/
#define TBLDRV_MULTICAST_DATA_STRUCTURE

/* mcast entry flag */
#define RTL865X_MULTICAST_PPPOEPATCH_CPUBIT		0x01
#define RTL865X_MULTICAST_EXTIP_SET				0x02
#define RTL865X_MULTICAST_UPLOADONLY				0x04

typedef struct rtl8651_tblDrv_mCast_s {
	ipaddr_t	sip;
	ipaddr_t	dip;
	uint16	svid;
	uint16 	port;
	uint32	mbr;
	uint16	age;
	uint16	cpu;
	rtl8651_tblDrv_ipEntry_t *ipEntry_t;
	uint32	vlanMask;
	uint32	count;
	uint32	flag;
	uint32	inAsic;
	uint32 linkMask;		
	TAILQ_ENTRY(rtl8651_tblDrv_mCast_s) nextMCast;
} rtl8651_tblDrv_mCast_t;

/*================================================
  * MAC Access Control
  *================================================*/
#define TBLDRV_MAC_ACCESS_CONTROL_STRUCTURE

typedef struct rtl8651_tblDrv_macACEntry_s {
	ether_addr_t	mac;
	ether_addr_t	macMask;
	uint32		actionType;
	CTAILQ_ENTRY(rtl8651_tblDrv_macACEntry_s) nextMAC;
} rtl8651_tblDrv_macACEntry_t;


typedef struct rtl8651_tblDrv_macContrl_s {
	uint16	def_permit;
	uint16	enable;
	CTAILQ_HEAD( , rtl8651_tblDrv_macACEntry_s) macAccessControl;
} rtl8651_tblDrv_macContrl_t;


#if	1
/*=============================================
  * Output Queue Data Structure
  *=============================================*/

#define RTL865X_OUTPUTQUEUE_DATA_STRUCTURE

typedef	struct rtl8651_tblDrv_outputQueue_s {
	uint8		queueType;			/*	M-Queue = 1, N-Queue = 2, Default Queue = 3 invalid = 0	*/
	uint8		ppr;				/*	Peak Packet Rate of PPR LB in times of  APR---PPR = 2^(ppr)*APR	*/
	uint8		burstSize;		/*	Bucket burst size of APR LB in bytes	unit = 1KByte Bucket burst size = burstSize*1KB	*/
	uint8		queueId;			/*	which queue the entry representation	*/
	uint8		dscp;			/* Diffserv remarking value */
	uint8		mark;			/* Diffserv marking enabled/disabled*/
	uint8		vlanPri;			/* 802.1p remarking value */
	uint8		vlanRemark;			/* 802.1p remarking enabled/disabled*/
	uint16	apr;				/*	Average Packet Rate of APR LB in times of 64Kbps	*/
	uint32	groupId;
	uint32	ratio;
	uint32	ref_count;
	rtl8651_tblDrv_networkIntfTable_t *netif_t;
}rtl8651_tblDrv_outputQueue_t;

#endif


/*=============================================
  * Rate Limit Data Structure
  *=============================================*/

#define RTL865X_RATE_LIMIT_DATA_STRUCTURE

typedef struct rtl8651_tblDrv_rateLimit_s {
#ifdef CONFIG_RTL865XB_BCUT_RLREFILL_BYSW
	uint32	rate;	/* It is used to record the user-config rate constraint. Only used when software refills token. */
	uint32	burst;  /* User-config burst */
#endif
	uint32	maxToken;
	uint32	rateMappedToken;	/* For M queues, this means the ratio and WAN bandwidth mapped token number.
									For N queues, this means the WAN bandwidth minus M queue guaranteed bandwidth and all N
									queues are active shared bandwidth */

	uint32	lastConfigToken; /* Last time configured to ASIC token number*/
	uint32	sliceIncrementToken; /* Every fast interval (10ms) incresed token number */
	uint32	accumulatedToken; /* Accumulated used token bytes within 1 second */
	uint32	vRate;
	int32	eagerCounter;

	uint32	isByteCount;
	uint32 	addToken;
	uint32	interval;
									
	uint32	groupId;		/* GroupId=0: Invalid entry */
	uint8	queueType;
	uint8	dscp;			/* Diffserv remarking value */
	uint8	mark;			/* Diffserv marking enabled/disabled*/
	uint8	nQueueNeedProcess; /* Used to control N queue token calculation */
	uint32	ref_count;
	uint32	ratio;		/* > 0: MN Queueing model */
	rtl8651_tblDrv_networkIntfTable_t *netif_t;
} rtl8651_tblDrv_rateLimit_t;

#define RATELIMIT_TABLE_INDEX(entry)				(entry - DrvTbl.rateLimit)

#define RATELIMIT_GROUP_INTERNALRESV				100	/* 1~100 : reserved group Id for internal control */

/* use Rate Limit table entry as counter */
#define RATELIMIT_COUNTER_RESV_BASE			11	/* base ID */
#define RATELIMIT_COUNTER_RESV_SIZE			10	/* total ID to reserve */


/* Rate Limit Counter */
#define RTL865X_RATELIMIT_COUNTER_DATA_STRUCTURE

typedef struct rtl8651_tblDrv_rateLimitCounter_s {
	uint8	valid;
	uint8	counterType;
#if	RTL8651_IDLETIMEOUT_FIXED
	uint16	vid;
#endif
	uint32	rateLimitIndex;
	rtl8651_tblDrv_networkIntfTable_t *netif_t;
} rtl8651_tblDrv_rateLimitCounter_t;

#define RATELIMIT_COUNTER_TYPE_WANTRAFFICCOUNTER		0x01


#define RTL865X_MNQUEUE_DATA_STRUCTURE


rtl8651_tblDrv_networkIntfTable_t *GET_NETWORK_INTERFACE(int8 *ifname, uint32 *ifIdx);


/*================================================
  * Protocol Based NAT structure
  *================================================*/
typedef struct rtl8651_tblDrv_protocolBasedNat_s
{
	uint8 protocol;
	ipaddr_t ExtIp;
	ipaddr_t IntIp;

	SLIST_ENTRY( rtl8651_tblDrv_protocolBasedNat_s ) next;
} rtl8651_tblDrv_protocolBasedNat_t;









#endif /*RTL86561_LAYER3_LOCAL_H*/
#endif
