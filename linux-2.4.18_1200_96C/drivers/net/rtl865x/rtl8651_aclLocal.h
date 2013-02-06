
/////////////////////////////////////////////////////////////////
////
//// This file is to be included only by rtl8651_tblDrvLocal.h
//// This file exists only to split lengthy driver definitions to several organized header files
////
////  !!!Only  put ACL driver specific 
////              1) local data types, 2) #define, and 3)macros 
////        in this file
////
//// Leave all "externs" and functions prototypes to rtl8651_tblDrvlocal.h 
//// If the API or datatype is to be exported outside the driver,
//// Please put it in rtl8651_layer2.h or rtl8651_tblDrv.h 
////
/////////////////////////////////////////////////////////////////

#ifdef RTL8651_TBLDRV_LOCAL_H
#ifndef RTL86561_ACL_LOCAL_H
#define RTL86561_ACL_LOCAL_H

#include "rtl_queue.h"

///////////////////////////////////////////////////////
//// Only  lenthy Layer 3 driver specific local 1) data types, 2) #define, and 3)macros here
//// Leave all "externs" and functions prototypes to rtl8651_tblDrvlocal.h 
///////////////////////////////////////////////////////

/* operation layer */
#define _RTL8651_OPERATION_OTHER		0
#define _RTL8651_OPERATION_LAYER2		2
#define _RTL8651_OPERATION_LAYER3		3
#define _RTL8651_OPERATION_LAYER4		4

typedef struct rtl8651_tblDrv_aclAsicArrangementEntry_s {
	uint16 	vidOrNum; //
	uint16	inOrOut; //0: Invalid entry, 1: Ingress rule, 2: Egress rule
	int32 	identy; //1: alg; 2: server port; 3: special rule
} rtl8651_tblDrv_aclAsicArrangementEntry_t;



typedef struct _rtl8651_tblDrvAclRule_s {
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
#if 1 //chhuang: #ifdef RTL8650B
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
					ether_addr_t _l2srcMac, _l2srcMacMask;	// for srcMac & destPort ACL rule
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
					ether_addr_t _l2srcMac, _l2srcMacMask;	// for srcMac & destPort ACL rule
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;										
				}udp; 
			}is;
		}L3L4; 
#if 1 //chhuang: #ifdef RTL8650B
		/* Source filter ACL rule */
		struct {
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _srcPort, _srcPortMask;
			uint16 _srcVlanIdx, _srcVlanIdxMask;
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			uint16 _srcPortUpperBound, _srcPortLowerBound;
			uint32 _ignoreL4:1, //L2 rule
				  	 _ignoreL3L4:1; //L3 rule
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
		struct {
			uint8	vlanTagPri;
		} VLANTAG;
	}un_ty;
	uint32	ruleType_:5;
	uint32	actionType_:4;
#if 1	/* RTL8650B */
	uint32  	pktOpApp:3;
#endif	/* RTL8650B */
	uint32	isEgressRateLimitRule_:1;
	uint32	naptProcessType:4;
	uint32	naptProcessDirection:2;
	uint32	matchType_;
	
	uint16	dsid; /* 2004/1/19 orlando */
	uint16	priority:3;
	uint32	dvid_:3;
	uint32	priority_:1;
	uint32	nextHop_:10; /* Index of L2 table */
	uint32	pppoeIdx_:3;
	uint32	isIPRange_:1;			/* support IP Range ACL */
	uint32	isRateLimitCounter_:1;	/* support Rate Limit Counter Mode */
#if 1 //chhuang: #ifdef RTL8650B
	uint16	nhIndex; /* Index of nexthop table (NOT L2 table) */
	uint16	rlIndex; /* Index of rate limit table */
#endif /* RTL8650B */

	uint32	aclIdx;
	CTAILQ_ENTRY(_rtl8651_tblDrvAclRule_s) nextRule;
} _rtl8651_tblDrvAclRule_t;


typedef	struct _rtl8651_tblDrvAclLookupInfo_s {
	uint32	naptProcessType:4;
	uint32	naptProcessDirection:2;
	int8		Ingress;
	uint16  	dsid;
	uint32	matchType_;
	uint32 	optLayer;
}	_rtl8651_tblDrvAclLookupInfo_t;

/* Private ACL rule type: */
#define RTL8651_ACL_IFSEL					0x06

#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
#define RTL8651_ACL_SRCFILTER				0x07
#define RTL8651_ACL_DSTFILTER				0x08
#endif 
/* Private ACL action type: */
#define RTL8651_ACL_REDIRECT				0x06
#define RTL8651_ACL_REDIRECT_PPPOE			0x07
#define RTL8651_ACL_MIRROR					0x08
#define RTL8651_ACL_MIRROR_KEEP_MATCH		0x09
#define RTL8651_ACL_DROP_RATE_EXCEED_PPS	0x0a
#define RTL8651_ACL_LOG_RATE_EXCEED_PPS		0x0b
#define RTL8651_ACL_DROP_RATE_EXCEED_BPS	0x0c
#define RTL8651_ACL_LOG_RATE_EXCEED_BPS		0x0d
#define RTL8651_ACL_POLICY					0x0e


/*	dummy acl type for qos	*/
#define	RTL8651_ACL_802D1P					0x1f


#define RTL8651_ACLTBL_ALL_TO_CPU			127  // This rule is always "To CPU"
#define RTL8651_ACLTBL_DROP_ALL				126 //This rule is always "Drop"
#define RTL8651_ACLTBL_PERMIT_ALL			125	// This rule is always "Permit"
#define RTL8651_ACLTBL_PPPOEPASSTHRU		124 //For PPPoE Passthru Only
#define RTL8651_ACLTBL_RESERV_SIZE			4	//this many of ACL rules are reserved for internal use


/*****************************
	ACL Function declaration
******************************/
void _rtl8651_arrangeAllACL(void);
#endif
#endif
