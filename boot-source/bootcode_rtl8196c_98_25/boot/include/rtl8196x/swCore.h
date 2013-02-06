/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
*
* Abstract: Switch core header file.
*
*
* ---------------------------------------------------------------
*/

#ifndef _SWCORE_H
#define _SWCORE_H
#define CONFIG_RTL865XC 1


#define SWNIC_DEBUG
#define SWTABLE_DEBUG
#define SWCORE_DEBUG


#ifdef CONFIG_RTL8196C 
#define CONFIG_RTL8196C_ETH_IOT         1 
   extern void set_gray_code_by_port(int port); 
#endif 


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch core.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_init(void);
#ifdef _L2_MODE_
int32 L2_swCore_config( uint8* gmac, uint32 for_RTL8650A );
#endif//_L2_MODE_


#ifdef CONFIG_RTL865XC

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

/* For PktOpApp */
#define RTL8651_ACLTBL_BACKWARD_COMPATIBLE	0 /* For backward compatible */
#define RTL865XC_ACLTBL_ALL_LAYER			RTL8651_ACLTBL_BACKWARD_COMPATIBLE
#define RTL8651_ACLTBL_ONLY_L2				1 /* Only for L2 switch */
#define RTL8651_ACLTBL_ONLY_L3				2 /* Only for L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_L2_AND_L3			3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL8651_ACLTBL_ONLY_L4				4 /* Only for L4 translation packets */
#define RTL8651_ACLTBL_L3_AND_L4			6 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL8651_ACLTBL_NOOP				7 /* No operation. Don't apply this rule. */

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
#define srcIpAddrStart_			un_ty.L3L4._srcIpAddr
#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddrMask
#define dstIpAddrStart_			un_ty.L3L4._dstIpAddr
#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddrMask

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

/* ACL access parameters */
typedef struct {
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
			uint32 _ignoreL3L4:1, //L2 rule
				  	 _ignoreL4:1; //L3 rule
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
	}un_ty;
	uint32	ruleType_:4;
	uint32	actionType_:4;
#if 1	/* RTL8650B */
	uint32  	pktOpApp:3;
#endif	/* RTL8650B */
	uint32	isEgressRateLimitRule_:1;
	uint32	naptProcessType:4;
	uint32	naptProcessDirection:2;
	uint32	matchType_;
	
	uint16  	dsid; /* 2004/1/19 orlando */
	uint16	priority:3;
	uint32	dvid_:3;
	uint32	priority_:1;
	uint32	nextHop_:10;
	uint32  	pppoeIdx_:3;
	uint32	isIPRange_:1;			/* support IP Range ACL */
	uint32	isRateLimitCounter_:1;	/* support Rate Limit Counter Mode */
#if 1 //chhuang: #ifdef RTL8650B
	uint16  	nhIndex; //index of next hop table
	uint16  	rlIndex; //index of rate limit table
#endif /* RTL8650B */

	uint32	aclIdx;
} rtl_acl_param_t;


/* Netif access parameters */
typedef struct {
    macaddr_t       gMac;
    uint16          macAddrNumber;
    uint16          vid;
    uint32          inAclStart, inAclEnd, outAclStart, outAclEnd;
    uint32          mtu;
    uint32          enableRoute:1,
                    valid:1;
} rtl_netif_param_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint16          mac39_24;
    uint16          mac23_8;

    /* word 1 */
    uint32          reserv0: 6;
    uint32          auth: 1;
    uint32          fid:2;
    uint32          nxtHostFlag : 1;
    uint32          srcBlock    : 1;
    uint32          agingTime   : 2;
    uint32          isStatic    : 1;
    uint32          toCPU       : 1;
    uint32          extMemberPort   : 3;
    uint32          memberPort : 6;
    uint32          mac47_40    : 8;

#else /*LITTLE_ENDIAN*/
    /* word 0 */
    uint16          mac23_8;
    uint16          mac39_24;
		
    /* word 1 */
    uint32          mac47_40    : 8;
    uint32          memberPort : 6;
    uint32          extMemberPort   : 3;
    uint32          toCPU       : 1;
    uint32          isStatic    : 1;
    uint32          agingTime   : 2;
    uint32          srcBlock    : 1;
    uint32          nxtHostFlag : 1;
    uint32          fid:2;
    uint32          auth:1;	
    uint32          reserv0:6;	

#endif /*LITTLE_ENDIAN*/
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_l2Table_t;

#endif


/* VLAN service
*/

#define RTL_STP_DISABLE 0
#define RTL_STP_BLOCK   1
#define RTL_STP_LEARN   2
#define RTL_STP_FORWARD 3


#ifdef CONFIG_RTL865XC
/* VLAN access parameters */
typedef struct {
    uint32          memberPort;
    uint32          egressUntag;
    uint32          fid: 2;
} rtl_vlan_param_t;
#endif



/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanCreate
 * --------------------------------------------------------------------
 * FUNCTION: This service creates a vlan.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
    EEXIST: Speicified vlan already exists.
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanCreate(uint32 vid, rtl_vlan_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanDestroy
 * --------------------------------------------------------------------
 * FUNCTION: This service destroys a vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanDestroy(uint32 vid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service sets port based vlan id.
 * INPUT   :
		portNum: Port number.
		pvid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPVid(uint32 portNum, uint32 pvid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port based vlan id.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : 
		pvid_P: Pointer to a variable to hold the PVid.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPVid(uint32 portNum, uint32 *pvid_P);

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetSTPStatusOfAllPorts
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status.
 * INPUT   :
		vid: Vlan ID.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetSTPStatusOfAllPorts(uint32 vid, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
 * OUTPUT  : 
		STPStatus_P: Pointer to a variable to hold the spanning tree 
		        status of the specified port.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 *STPStatus_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of the specified vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetInformation(uint32 vid, rtl_vlan_param_t * param_P);


/* Layer 2 service
*/

/* L2 forwarding table access parameters 
*/
typedef struct {
    macaddr_t       mac;
    uint16          isStatic    : 1;
    uint16          hPriority   : 1;
    uint16          toCPU       : 1;
    uint16          srcBlock    : 1;
    uint16          nxtHostFlag : 1;
    uint16          reserv0     : 11;
    uint32          memberPort;
    uint32          agingTime;
} rtl_l2_param_t;


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrAdd
 * --------------------------------------------------------------------
 * FUNCTION: This service adds the static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENFILE: Cannot allocate slot.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrAdd(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrRemove
 * --------------------------------------------------------------------
 * FUNCTION: This service removes the specified static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified MAC address does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrRemove(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   :
        entryIndex: Index of entry.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
        EEMPTY: Specified entry is empty.
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformation(uint32 entryIndex, rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformationByMac
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   : None.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformationByMac(rtl_l2_param_t * param_P);


/* Counter service 
*/

typedef struct {
    uint32  etherStatsOctets;
    uint32  etherStatsDropEvents;
    uint32  etherStatsCRCAlignErrors;
    uint32  etherStatsFragments;
    uint32  etherStatsJabbers;
    uint32  ifInUcastPkts;
    uint32  etherStatsMulticastPkts;
    uint32  etherStatsBroadcastPkts;
    uint32  etherStatsUndersizePkts;
    uint32  etherStatsPkts64Octets;
    uint32  etherStatsPkts65to127Octets;
    uint32  etherStatsPkts128to255Octets;
    uint32  etherStatsPkts256to511Octets;
    uint32  etherStatsPkts512to1023Octets;
    uint32  etherStatsPkts1024to1518Octets;
    uint32  etherStatsOversizepkts;
    uint32  dot3ControlInUnknownOpcodes;
    uint32  dot3InPauseFrames;
} rtl_ingress_counter_t;
typedef struct {
    uint32  ifOutOctets;
    uint32  ifOutUcastPkts;
    uint32  ifOutMulticastPkts;
    uint32  ifOutBroadcastPkts;
    uint32  dot3StatsLateCollisions;
    uint32  dot3StatsDeferredTransmissions;
    uint32  etherStatsCollisions;
    uint32  dot3StatsMultipleCollisionFrames;
    uint32  dot3OutPauseFrames;
} rtl_egress_counter_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : None.
 * OUTPUT  : 
        portList_P: List of member ports.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetMemberPort(uint32 *portList_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterSetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : 
        portList: List of member ports.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterSetMemberPort(uint32 portList);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetIngress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the ingress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the ingress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetIngress(rtl_ingress_counter_t *counters_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetEgress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the egress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the egress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetEgress(rtl_egress_counter_t *counters_P);


/* Port service 
*/

#define RTL_PORT_100M_FD        (1 << 0)
#define RTL_PORT_100M_HD        (1 << 1)
#define RTL_PORT_10M_FD         (1 << 2)
#define RTL_PORT_10M_HD         (1 << 3)

typedef struct {
    uint8   capableFlowCtrl : 1;
    uint8   capable100MFull : 1;
    uint8   capable100MHalf : 1;
    uint8   capable10MFull  : 1;
    uint8   capable10MHalf  : 1;
    uint8   reserv0         : 3;
} rtl_auto_nego_ability_t;
    
typedef struct {
    uint8   enAutoNego          : 1;
    uint8   enSpeed100M         : 1;
    uint8   enFullDuplex        : 1;
    uint8   enLoopback          : 1;
    uint8   linkEstablished     : 1;
    uint8   autoNegoCompleted   : 1;
    uint8   remoteFault         : 1;
    uint8   reserv0             : 1;
    rtl_auto_nego_ability_t   autoNegoAbility;
    rtl_auto_nego_ability_t   linkPartnerAutoNegoAbility;
    uint32  speedDuplex;
} rtl_port_status_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetSpeedDuplex
 * --------------------------------------------------------------------
 * FUNCTION: This service sets speed and duplex mode of specified port.
 * INPUT   :
		portNum: Port number.
		speedDuplex: Speed and duplex mode. Valid values are 
		    RTL_PORT_100M_FD, RTL_PORT_100M_HD, RTL_PORT_10M_FD and 
		    RTL_PORT_10M_HD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetSpeedDuplex(uint32 portNum, uint32 speedDuplex);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetAutoNegociationAbility
 * --------------------------------------------------------------------
 * FUNCTION: This service sets auto negociation pause, speed and duplex 
        mode capability of specified port.
 * INPUT   :
		portNum: Port number.
		anAbility_P: Pointer to the data structure which specifies the auto 
		    negociation abilities.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetAutoNegociationAbility(uint32 portNum, rtl_auto_nego_ability_t *anAbility_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portEnableAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service enables auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portEnableAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portDisableAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service disables auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portDisableAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portRestartAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service restarts auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portRestartAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to loopback mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portResetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to normal mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portResetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portGetStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port status of specified port.
 * INPUT   : 
		portNum: Port number.
 * OUTPUT  : 
    portStatus_P: Pointer to an area to hold the port status.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portGetStatus(uint32 portNum, rtl_port_status_t *portStatus_P);



#define swCore_vlanCreate vlanTable_create
#define swCore_vlanDestroy vlanTable_destroy
#define swCore_vlanSetPortSTPStatus vlanTable_setPortStpStatus
#define swCore_vlanSetSTPStatusOfAllPorts vlanTable_setStpStatusOfAllPorts
#define swCore_vlanGetPortSTPStatus vlanTable_getSTPStatus
#define swCore_vlanGetInformation vlanTable_getInformation

#endif /* _SWCORE_H */

