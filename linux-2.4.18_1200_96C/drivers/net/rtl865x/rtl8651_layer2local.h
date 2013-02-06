/////////////////////////////////////////////////////////////////
////
//// This file is to be included only by rtl8651_tblDrvLocal.h
//// This file exists only to split lengthy driver definitions to several organized header files
////
////  !!!Only  put Layer 2 driver specific 
////              1) local data types, 2) #define, and 3)macros 
////        in this file
////
//// Leave all "externs" and functions prototypes to rtl8651_tblDrvlocal.h 
//// If the API or datatype is to be exported outside the driver,
//// Please put it in rtl8651_layer2.h or rtl8651_tblDrv.h 
////
/////////////////////////////////////////////////////////////////

#ifdef RTL8651_TBLDRV_LOCAL_H   

#ifndef RTL8651_LAYER2_LOCAL_H
#define  RTL8651_LAYER2_LOCAL_H

typedef struct rtl8651_tblAsic_ethernet_s {

#ifdef CONFIG_RTL865XC
	uint8	linkUp: 1,
			phyId: 5,
			isGPHY: 1;
#else
	uint8 linkUp:1;
#endif

} rtl8651_tblAsic_ethernet_t;


typedef struct rtl8651_tblDrv_ethernet_s {
	uint32 	speed:2, //0: 10Mbps, 1:100Mbps, 2:1000Mbps
			duplex:1, //0: half-duplex 1:full-duplex
			autoNegotiation:1, //0:disable 1:Enable
			fake_forceMode:1,	// used to patch switch core crash bug (see rtl8651_setEthernetPortAutoNegotiation() for more details)
#ifdef CONFIG_RTL865XC
			autoAdvCapability:4, /* Compared to RTL865xB, add 1 more bit to stand for giga capability. */
#else
			autoAdvCapability:3, // RTL8651_ETHER_AUTO_xxx
#endif
			inputBandwidthControl:4, // RTL8651_BC_xxx
			outputBandwidthControl:4, // RTL8651_BC_xxx
			linkUp:1; //0: Down 1:Up
	uint16	aggregatorIndex;//The corresponding entry index in link aggregation table
} rtl8651_tblDrv_ethernet_t;
 
typedef struct rtl8651_tblDrv_linkAggregation_s {
	uint16 pvid;//Port vlan ID
	uint16 	individual:1, // Whether this aggregator is indivual aggregator or not
			aggregated:1;//Whether this aggregator aggregate some links
	uint32 ethernetPortMask; //port i at 1<<i position, from 0 to 31
	uint32 ethernetPortUpStatus; //port i at 1<<i position, from 0 to 31
} rtl8651_tblDrv_linkAggregation_t;



/*
Note:
1. Entry property toCpu, srcBlocking destination blocking (memberPortMask) and normal forwarding (Neither toCpu nor srcBlocking) are 
    mutually exclusive. Therefore, only 2-bit to represent this situation. 
2. All entries are static configuration
3. Unable to process conflict macAddress entry process, return fail when following operation are conflict with current one.
*/
typedef struct rtl8651_tblDrv_filterDbTableEntry_s {
	ether_addr_t	macAddr;
	uint16  l2type;			// see bleow definition
	uint16	process:2,		//0: Normal forwarding, 1: destination blocking 2: source blocking 3: toCpu
			//Management flag
			refCount:8,		//Referenced by other table, such as 
			configToAsic:1,	//This entry is configured to ASIC
			asicPos:2;		//The entry position of the ASIC. Since rtl8651 only provides 4-entry. Only 2-bit is required
	uint16 	vid;				//0: don't care. 1~4094: vid of this entry
	uint32	linkId;			//For WLAN WDS.
		//cfliu: What is link Id? link Id is a virtual port number which represents a WDS link for wireless application.
		//since 8650 uses MII loopback port and 8650B has only 3 ext ports, it is always insufficient to map 8650B ext ports
		//to typical 8 WDS links.

	uint32	memberPortMask; //port i at 1<<i position, from 0 to 31
	SLIST_ENTRY(rtl8651_tblDrv_filterDbTableEntry_s) nextFDB;
} rtl8651_tblDrv_filterDbTableEntry_t;

#define RTL8651_L2_TYPEI			0x0001		/* Referenced by ARP/PPPoE */
#define RTL8651_L2_TYPEII			0x0002		/* Referenced by Protocol */
#define RTL8651_L2_TYPEIII			0x0004		/* Referenced by PCI/Extension Port */

/* Definition for L2 entry mode */
#define L2_AUTO					0x00
#define L2_FORCE_STATIC				0x01
#define L2_FORCE_DYNAMIC			0x02

typedef struct rtl8651_tblDrv_filterDbTable_s {
	uint16 sid;// Spanning tree ID, 0: CIST id, 1-4096 MST ID
	uint32 valid:1;	//Whether this filter database is valid
	SLIST_HEAD(, rtl8651_tblDrv_filterDbTableEntry_s) database[RTL8651_L2TBL_ROW];
} rtl8651_tblDrv_filterDbTable_t;

typedef struct rtl8651_tblDrv_spanningTreeTable_s {
	uint32	protocolWorking:1,	//Whether protocol is working to configure this instance
			valid:1;	//Whether this spanning tree is valid
	//RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum
	int8	portState[9];// 0: disabled, 1: blocking, 2: listening, 3: Learning, 4: Forwarding
} rtl8651_tblDrv_spanningTreeTable_t;


typedef struct rtl8651_tblDrv_macAddressEntry_s {
	ether_addr_t mac;
	uint16 vid;
	uint32 	valid:1,
			allocated:1;
} rtl8651_tblDrv_macAddressEntry_t;

struct rtl8651_tblDrv_networkIntfTable_s;
typedef struct rtl8651_tblDrv_vlanTable_s {
	ether_addr_t macAddr;
	uint32 memberPortMask;		//port i at 1<<i position, from 0 to 31. The port means link aggregator instead of actual port
	uint32 memberPortUpStatus; //Whether member aggregator is up
	uint32 untagPortMask;		//port i at 1<<i position, 1 means enable untag 0 means disable untag.
	uint16 macAddrNumber;		// The number of consecutive mac address, 0 will disable ENRTR (Enable Routing)
	uint16 vid;				// VLAN ID
	uint16 fid;				//Filtering Database Index
	uint32  inAclStart, inAclEnd, outAclStart, outAclEnd;
	uint16	mtu;
	uint16 	internal:1,		//Whether vlan is internal VLAN. This is configured by NAT control APIs
			ipAttached:1,	//IP interface attached over it
			manualAssign:1,	//Whether MAC address is manually assigned
			promiscuous:1,
			fwdTxMirror:1,		//The port members of this VLAN is used to mirror CPU generated 
			DMZflag:1,
			valid:1,			//Whether entry is valid
			softRoute:1,		//pure software routing on this vlan
#if defined (CONFIG_RTL865XB_ENRT) || defined(CONFIG_RTL865XC)	/* refer to _rtl8651_addVlan() for the meaning of this compile flag */
			macNonExist:1,	/* If this bit is set, only L2 forwarding is performed over this VLAN. */
#endif
			extPortAsicAlwaysTag:1,	/* Extension Port would always be TAGGED in ASIC ? For 865xB B-Cut PPPoE DMA bug. */
			broadcastToCpu:1;	//Broadcast to CPU ?
	struct rtl8651_tblDrv_vlanTable_s *fvlan_t; //if not NULL, it is a sub vlan
#ifdef CONFIG_RTL865XB_EXP_INVALID
	SLIST_HEAD(, rtl8651_tblDrv_networkIntfTable_s) netHead;
#else
	struct rtl8651_tblDrv_networkIntfTable_s *netif_t;
#endif /* CONFIG_RTL865XB_EXP_INVALID */
} rtl8651_tblDrv_vlanTable_t;


#if defined (RTL865XB_DCUT_SWVLAN) || defined(CONFIG_RTL865XC)
typedef struct rtl8651_tblDrv_swVlanTable_s {
	uint32 memberPortMask;
	uint32 memberPortUpStatus;
	uint32 untagPortMask;
	uint16 vid;
	uint16 fid;
	uint16	valid:1;
}rtl8651_tblDrv_swVlanTable_t;
#endif


/*
	VLAN releated process
*/
rtl8651_tblDrv_vlanTable_t *_rtl8651_getVlanTableEntry(uint16 vid);
uint16 _rtl8651_getOneVidx(int8 internal);

int32 _rtl8651_setPvid(uint32 port, uint16 vid);
uint16 _rtl8651_getPvid(uint32 port);


extern uint32 peripheralExtPortMask;

#ifdef RTL8650B

/* Table:  Server Port */
struct rtl8651_tblDrv_serverPort_s;
typedef struct rtl8651_tblDrv_naptServerPortEntry_s 
{
	ipaddr_t 	intIp;							//Internal IP Address
	ipaddr_t 	extIp;							//External IP Address
	uint16		lowerIntPort, lowerExtPort;
	uint16		upperIntPort, upperExtPort;
	uint32 		tcp:1,							//Entry for TCP server
				valid:1,						//TRUE for in-used.
				usable:1;						//TRUE for usable, FALSE for broken.
	rtl8651_tblDrv_nextHopEntry_t *nxthop_t;		//Pointer to Nexthop Table, NULL: use routeing's nexthop info.
} rtl8651_tblDrv_naptServerPortEntry_t;


#endif /*RTL8650B*/

/* ================================================
	Co-work with ASIC driver
     ================================================ */
int32 _rtl8651_initAsicDrvParam( void );

/*================================================
  * PPTP/L2TP MII patch structure
  *================================================*/
#define TBLDRV_PPTP_L2TP_MII_STRUCTURE

typedef struct rtl8651_tblDrv_miiTunneling_s {
	uint32	valid;
	uint16	wanPort;
	uint16	wanVid;
	uint16	loopbackPort;
	uint16	loopBackVid;
} rtl8651_tblDrv_miiTunneling_t;

int32 rtl8651_setAsicAclRule(uint32 index, _rtl8651_tblDrvAclRule_t *rule);
int32 rtl8651_getAsicAclRule(uint32 index, _rtl8651_tblDrvAclRule_t *rule);
extern rtl8651_tblDrv_miiTunneling_t tunnel;
#endif

uint16 _rtl8651_getOneVidx(int8 internal);

int32 _rtl8651_setPvid(uint32 port, uint16 vid);
uint16 _rtl8651_getPvid(uint32 port);
 int32 _rtl8651_getL2InfoByMac(ether_addr_t *MacAddr, rtl8651_L2InfoTable_t *Info, uint16 fid );
 void _rtl8651_timeUpdateL2Table(void);
#endif
