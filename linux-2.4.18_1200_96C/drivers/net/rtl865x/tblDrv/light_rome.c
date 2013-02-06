

#include "rtl_types.h"
#include "mbuf.h"
#include "assert.h"
#ifdef CONFIG_RTL865XB
#include "../rtl865xb_swNic.h"
#include "rtl8651_tblAsicDrv.h"
#elif defined(CONFIG_RTL865XC)
#include "../rtl865xc_swNic.h"
#include "rtl865xC_tblAsicDrv.h"
#endif
#include "rtl865x_lightrome.h"
#include "light_rome.h"


//#ifndef RTL865X_TEST
proc_input_pkt_funcptr_t					__drvRx;
//#endif

static void rtl865x_lightromeLinkChange(uint32, int8);
static int32 lr_bridging_rcv(struct rtl_pktHdr *);
static int32 lr_route_init(void);
static rtl865x_tblAsicDrv_routingParam_t *	lr_route_lookup(ipaddr_t, ipaddr_t, uint32 *);
static struct if_entry *lr_pppoe_lookup(uint32, uint32 *);
static int32 lr_pppoe_init(void);
static int32 lr_vlan_init(void);
static int32 lr_vlan_create(struct rtl865x_lrConfig *);
static int32 lr_vlan_remove(uint32);
static int32 lr_vlan_hash(uint16);
static int32 lr_if_init(void);
static int32 lr_if_attach(struct rtl865x_lrConfig *);
static int32 lr_if_detach(uint8 *);
static int32 lr_if_up(uint8 *, ipaddr_t , ipaddr_t, uint32, ether_addr_t * );
static struct if_entry *lr_if_lookup(uint8 *, ipaddr_t );
static int32 lr_if_down(uint8 *name);
static rtl865x_tblAsicDrv_arpParam_t *lr_arp_lookup(ipaddr_t );
static int32 lr_arp_init(void);
static int32 lr_arp_hash(ipaddr_t );
static int32 lr_arp_tbl_alloc(struct if_entry *);
static int32 lr_arp_tbl_free(struct if_entry *);
static int32 lr_fdb_init(void);
static int32 lr_natip_init(void);
static rtl865x_tblAsicDrv_extIntIpParam_t * lr_natip_lookup(ipaddr_t, uint32 *);
static int32 lr_natip_add(ipaddr_t);
static int32 lr_natip_del(ipaddr_t);
static int32 lr_nat_init(void);
static struct nat_entry *lr_nat_lookup(struct nat_tuple *);
//static uint32 lr_nat_hash(int8, ipaddr_t, uint16);
static int32 lr_acl_init(void);
static int32 lr_acl_arrange(void);
static int32 lr_acl_add(uint32, _rtl8651_tblDrvAclRule_t *, enum ACL_FLAGS);
static int32 lr_acl_del(uint32, _rtl8651_tblDrvAclRule_t *, enum ACL_FLAGS);

static rtl865x_tblAsicDrv_l2Param_t *lr_fdb_lookup(uint32, ether_addr_t *,  uint32, uint32 *);


/*======================================
 *  Netwrok Interface Table - if_tbl
 *======================================*/
struct if_table if_tbl = {
	if_init:			lr_if_init,					/* Initialize if_table */
	if_attach:		lr_if_attach,				/* Create a network interface and bind to a specified vlan */
	if_detach:		lr_if_detach,				/* Remove an existing network interface and unbind from vlan */
	if_lookup:		lr_if_lookup,				/* Look up a specified network */
	if_up:			lr_if_up,					/* Assign IP/session to an interface */
	if_down:			lr_if_down,				/* Remove IP/session from an interface */
};


/*======================================
 *  External IP Table - extip_tbl
 *======================================*/
struct natip_table natip_tbl = {
	natip_init:		lr_natip_init,				/* Initialize external IP table */
	natip_lookup:		lr_natip_lookup,			/* External IP table lookup */
	natip_add:		lr_natip_add,
	natip_del:		lr_natip_del,
	
	natip_asic_set:	rtl8651_setAsicExtIntIpTable,
	natip_asic_get:	rtl8651_getAsicExtIntIpTable,
	natip_asic_del:	rtl8651_delAsicExtIntIpTable,
#ifndef CONFIG_RTL865XC
	gidx_asic_set:		rtl8651_setAsicGidxRegister,
	gidx_asic_get:		rtl8651_getAsicGidxRegister,
#endif
};



/*======================================
 *  nat table - nat_tbl
 *======================================*/
struct nat_table nat_tbl = {
	nat_init:				lr_nat_init,
	nat_lookup:			lr_nat_lookup,

	nat_asic_hash:		rtl8651_naptTcpUdpTableIndex,
	nat_asic_set:			rtl8651_setAsicNaptTcpUdpTable,
	nat_asic_del:			rtl8651_delAsicNaptTcpUdpTable,
	nat_asic_get:			rtl8651_getAsicNaptTcpUdpTable,
	nat_asic_tcpageL:		rtl8651_setAsicNaptTcpLongTimeout,
	nat_asic_tcpageM:		rtl8651_setAsicNaptTcpMediumTimeout,
	nat_asic_tcpageF:		rtl8651_setAsicNaptTcpFastTimeout,
	nat_asic_udpage:		rtl8651_setAsicNaptUdpTimeout,
};



/*======================================
 *  Routing Table - rt_tbl
 *======================================*/
struct rt_table rt_tbl = {
	route_init:		lr_route_init,				/* Initialize routing table */
	route_add:		rtl865x_addRoute,			/* Add a routing entry */
	route_del:		rtl865x_delRoute,			/* Remove a routing entry */
	route_lookup:		lr_route_lookup,			/* Routing entry lookup, not perform longest-prefix match */

	rt_asic_set:		rtl8651_setAsicRouting,		/* ASIC Interface: add asic route */
	rt_asic_get:		rtl8651_getAsicRouting,		/* ASIC Interface: query asic route */
	rt_asic_del:		rtl8651_delAsicRouting,		/* ASIC Interface: remove asic route */
};


/*======================================
 *  ARP Table - arpt_tbl
 *======================================*/
struct arp_table arpt_tbl = {
	arp_init:			lr_arp_init,				/* Initialize ARP table */
	arp_add:			rtl865x_addArp,			/* Add an arp entry */
	arp_del:			rtl865x_delArp,			/* Remove an arp entry */
	arp_lookup:		lr_arp_lookup,				/* ARP entry lookup */
	arp_hash:		lr_arp_hash,				/* Get a specified ARP entry position in ASIC */
	arp_tbl_alloc:		lr_arp_tbl_alloc,			/* Allocate ARP space for a network */
	arp_tbl_free:		lr_arp_tbl_free,			/* Release allocated ARP space */

	arp_asic_set:		rtl8651_setAsicArp,		/* ASIC Interface: add asic arp */
	arp_asic_get:		rtl8651_getAsicArp,		/* ASIC Interface: query asic arp */
	arp_asic_del:		rtl8651_delAsicArp,			/* ASIC Interface: remove asic arp */
};


/*======================================
 *  PPPoE Table - pppoe_tbl
 *======================================*/
struct pppoe_table pppoe_tbl = {
	pppoe_init:		lr_pppoe_init,				/* Initialize PPPoE table */
	pppoe_lookup:	lr_pppoe_lookup,			/* PPPoE session lookup */

	pppoe_asic_set:	rtl8651_setAsicPppoe,		/* ASIC Interface: configure asic pppoe session */
	pppoe_asic_get:	rtl8651_getAsicPppoe,		/* ASIC Interface: query asic pppoe session */
};


/*======================================
 *  VLAN Table - vlan_tbl
 *======================================*/
struct vlan_table vlan_tbl = {
	vlan_init:			lr_vlan_init,				/* Initialize VLAN table */
	vlan_create:		lr_vlan_create,			/* Create a vlan, total 4K vlan support */
	vlan_remove:		lr_vlan_remove,			/* Remove an existing vlan */
	vlan_hash:		lr_vlan_hash,				/* Mapping 4k vlan to 8-entry vlan space */

	vlan_asic_get:		rtl8651_getAsicVlan,		/* ASIC Interface: query asic vlan */
	vlan_asic_set:		rtl8651_setAsicVlan,		/* ASIC Interface: configure asic vlan */
	vlan_asic_del:		rtl8651_delAsicVlan,		/* ASIC Interface: remove asic vlan */

#ifdef CONFIG_RTL865XC
	intf_asic_set:         rtl8651_setAsicNetInterface,        /*ASIC INterface: configure asic net interface*/
	intf_asic_get:         rtl8651_getAsicNetInterface,      /*ASIC Interface: query asic net interface*/
#endif
};


/*======================================
 *  ACL Table - acl_tbl
 *======================================*/
struct acl_table acl_tbl = {
	acl_init:			lr_acl_init,				/* Initialize ACL table */
	acl_add:			lr_acl_add,				/* add ACL entry */
	acl_del:			lr_acl_del,				/* remove ACL entry */
	acl_arrange:		lr_acl_arrange,			/* arrange ACL entry to ASIC */

	acl_asic_set:		rtl8651_setAsicAclRule,		/* ASIC Interface: add acl entry to ASIC */
	acl_asic_get:		rtl8651_getAsicAclRule,		/* ASIC Interface: query acl entry */
};


/*======================================
 *  FDB(Filtering Database) - fdb_tbl
 *======================================*/
struct fdb_table fdb_tbl = {
	bridge_rcv:		lr_bridging_rcv,

	fdb_init:			lr_fdb_init,				/* Initialize FDB table */
	fdb_add:			rtl865x_addFdbEntry,		/* Add a FDB entry(static entry only) */
	fdb_del:			rtl865x_delFdbEntry,		/* Remove a FDB entry(static entry only) */
	fdb_lookup:		lr_fdb_lookup,				/* FDN entry lookup */

	fdb_asic_hash:	rtl8651_filterDbIndex,		/* Hash algorithm of FDB entry */
	fdb_asic_set:		rtl8651_setAsicL2Table,		/* ASIC Interface: configure asic fdb */
	fdb_asic_get:		rtl8651_getAsicL2Table,		/* ASIC Interface: query asic fdb */
	fdb_asic_del:		rtl8651_delAsicL2Table,		/* ASIC Interface: remove asic fdb */
};


/*======================================
 *  Port Attribute Table - port_attr
 *======================================*/
struct port_attribute port_attr = {
	activePortMask:	0,
	link_change:		rtl865x_lightromeLinkChange,/* Link change callback function */
	
	pvid_asic_set:	rtl8651_setAsicPvid,		/* ASIC Interface: congiure pvid */
	pvid_asic_get:	rtl8651_getAsicPvid,		/* ASIC Interface: query pvid */
};

struct lr_cpu_stats lr_stats;



static void rtl865x_lightromeLinkChange(uint32 port, int8 linkUp)
{
	port_attr.activePortMask = 
			(linkUp==TRUE)? 
			( port_attr.activePortMask | (1<<port) ) : 
			( port_attr.activePortMask & ~(1<<port) );
}



static int32 lr_if_init(void)
{
	memset(TBLFIELD(if_tbl, if_hash), 0, sizeof(struct if_entry)*NETIF_NUMBER);
	return 0;
}


static int32 lr_if_attach(struct rtl865x_lrConfig *lrconfig)
{
	struct if_entry *ife;
	int32 ifindex;

	if ( lrconfig->ifname[0]=='\0' ||lr_if_lookup(lrconfig->ifname, 0) )
		return (int32)LR_DUPENTRY;

	ife = &TBLFIELD(if_tbl, if_hash)[0];
	for(ifindex=0; ifindex<NETIF_NUMBER; ifindex++, ife++)
		if (ife->name[0] == '\0')
			break;
	if (ifindex == NETIF_NUMBER)
		return (int32)LR_NOBUFFER;

	memcpy(ife->name, lrconfig->ifname, MAX_IFNAMESIZE);
	ife->ipaddr_			= 0;
	ife->mask_			= 0;
	ife->if_type			= lrconfig->if_type;
	ife->mtu				= lrconfig->mtu;
	ife->vid				= lrconfig->vid;
#ifdef CONFIG_RTL865XC
	ife->isWan                   = lrconfig->isWan;
#endif
//	ife->fid				= lrconfig->fid;
//	ife->mac				= lrconfig->mac;
	return (int32)LR_SUCCESS;
}	


static int32 lr_if_detach(uint8 *name)
{
	return 0;
}


static struct if_entry *lr_if_lookup(uint8 *name, ipaddr_t ip)
{
	struct if_entry *ife;
	int32 ifindex;

	ife = &TBLFIELD(if_tbl, if_hash)[0];
	for(ifindex=0; ifindex<NETIF_NUMBER; ifindex++, ife++) {
		if ( (name&&ife->name[0] == '\0') ||
			(ip&&(ife->ipaddr_!=(ip&ife->mask_))) )		
			continue;
		if ( (name&&!strcmp(ife->name, name)) || 
			(IF_UP(ife)&&ip&&(ife->ipaddr_==(ip&ife->mask_))) ) {
			return ife;
		}
	}
	return (struct if_entry *)0;
}


static int32 lr_if_up(uint8 *name, ipaddr_t ip, ipaddr_t mask, uint32 sid, ether_addr_t *mac)
{
	struct if_entry *ife;
	
	ife = TBLFIELD(if_tbl, if_lookup)(name, 0);
	//if (ife == NULL || IF_UP(ife))
	if (ife == NULL)
		return LR_INVIF;
	if (ife->if_type == IF_ETHER) {
		ife->ipaddr_ = ip&mask;
		ife->mask_  = mask;	
		if (TBLFIELD(arpt_tbl, arp_tbl_alloc)(ife)) {
			ife->ipaddr_ = 0;
			ife->mask_  = 0;	
			return (int32)LR_NOARPSPACE;
		}
	} else {
		ife->sid_ = sid;
		ife->pmac_ = *mac;
	}
	ife->up = 1;
	return (int32)LR_SUCCESS;
}


static int32 lr_if_down(uint8 *name)
{
	struct if_entry *ife;

	ife = TBLFIELD(if_tbl, if_lookup)(name, 0);
	if (ife == NULL || !IF_UP(ife))
		return LR_INVIF;
	if (ife->if_type == IF_ETHER) {
		TBLFIELD(arpt_tbl, arp_tbl_free)(ife);
		ife->ipaddr_ = 0;
		ife->mask_  = 0;
	}
	else ife->sid_ = 0;
	ife->up = 0;
	return SUCCESS;
}



static int32 lr_natip_init(void)
{
	memset(&TBLFIELD(natip_tbl, __natipbuff), 0, sizeof(rtl865x_tblAsicDrv_extIntIpParam_t));
	TBLFIELD(natip_tbl, natip_no) = 0;
#ifndef CONFIG_RTL865XC
	/* IC Bug: always use the first IP table entry, set GIDX=0 */
	TBLFIELD(natip_tbl, gidx_asic_set)(0);
#endif
	return 0;
}


static rtl865x_tblAsicDrv_extIntIpParam_t * lr_natip_lookup(ipaddr_t natip, uint32 *pos)
{
	rtl865x_tblAsicDrv_extIntIpParam_t *natbuf;
	uint32 entry;

	natbuf = &TBLFIELD(natip_tbl, __natipbuff);
	for(entry=0; entry<RTL8651_IPTABLE_SIZE; entry++) {
		if (TBLFIELD(natip_tbl, natip_asic_get)(entry, natbuf))
			continue;
		if (natbuf->extIpAddr == natip) {
			if (pos) *pos = entry;
			return natbuf;
		}
	}
	return (rtl865x_tblAsicDrv_extIntIpParam_t *)0;
}


static int32 lr_natip_add(ipaddr_t natip)
{
	rtl865x_tblAsicDrv_extIntIpParam_t *natbuf;
	uint32 entry;

	if (TBLFIELD(natip_tbl, natip_lookup)(natip, NULL))
		return -1;
	/* Currently we only support one NAPT IP */
	if (TBLFIELD(natip_tbl,natip_no) > 0)
		return -1;
	natbuf = &TBLFIELD(natip_tbl, __natipbuff);
	for(entry=0; entry<RTL8651_IPTABLE_SIZE; entry++) 
		if (TBLFIELD(natip_tbl, natip_asic_get)(entry, natbuf))
			break;	
	if (entry < RTL8651_IPTABLE_SIZE) {
		memset(natbuf, 0, sizeof(*natbuf));
		natbuf->extIpAddr		= natip;
		natbuf->intIpAddr		= 0;
		natbuf->localPublic	= 0;
		natbuf->nat			= 0;
		natbuf->nhIndex		= 0;
		TBLFIELD(natip_tbl, natip_asic_set)(entry, natbuf);
		TBLFIELD(natip_tbl,natip_no) ++;
		LR_INIT_CHECK(rtl8651_setAsicOperationLayer(4));
		return 0;
	}
	return -1;	
}


static int32 lr_natip_del(ipaddr_t natip)
{
	uint32 pos;

	if (TBLFIELD(natip_tbl, natip_lookup)(natip, &pos)) {
		TBLFIELD(natip_tbl, natip_asic_del)(pos);
		TBLFIELD(natip_tbl,natip_no) --;
		if (!TBLFIELD(natip_tbl,natip_no))
			LR_INIT_CHECK(rtl8651_setAsicOperationLayer(3));
		return 0;
	}
	return -1;
}


static int32 lr_nat_init(void)
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t naptTcpUdp;
	uint32 flowTblIdx;
	
	memset((void*)TBLFIELD(nat_tbl, nat_bucket), 0, 
		sizeof(struct nat_entry)*RTL8651_TCPUDPTBL_SIZE);
	TBLFIELD(nat_tbl, tcp_timeout) = TCP_TIMEOUT; //24*60*60;
	TBLFIELD(nat_tbl, udp_timeout) = UDP_TIMEOUT; //60*15;

	/* Set ASIC timeout value */
#if 1
/* suggestion in the 865x datasheet:
    UDP Timeout. Suggested: 19 .. 320 seconds (5 mins)

    TCP Long Timeout Threshold. Suggested: 50 .. 86016 seconds (a day)

    TCP Medium Timeout Threshold. Suggested: 22 .. 512 seconds (10 mins)

    TCP Fast Timeout Threshold. Suggested: 15 .. 64 seconds
*/

    	//TBLFIELD(nat_tbl, nat_asic_tcpageL)(86400);
	//TBLFIELD(nat_tbl, nat_asic_tcpageM)(600);
	//TBLFIELD(nat_tbl, nat_asic_tcpageF)(60);

	// modify for SmartFlow TCP test, change 8 to 180
    	TBLFIELD(nat_tbl, nat_asic_tcpageL)(180); //8); // refer to Rome driver SDK
	TBLFIELD(nat_tbl, nat_asic_tcpageM)(180); //8);
	TBLFIELD(nat_tbl, nat_asic_tcpageF)(180); //8);
    
	//TBLFIELD(nat_tbl, nat_asic_udpage)(300);
	TBLFIELD(nat_tbl, nat_asic_udpage)(180);
#else
	TBLFIELD(nat_tbl, nat_asic_tcpageL)(TCP_TIMEOUT);
	TBLFIELD(nat_tbl, nat_asic_tcpageM)(TCP_TIMEOUT);
	TBLFIELD(nat_tbl, nat_asic_tcpageF)(TCP_TIMEOUT);
	TBLFIELD(nat_tbl, nat_asic_udpage)(UDP_TIMEOUT);
#endif

	/* Initial ASIC NAT Table */
	memset( &naptTcpUdp, 0, sizeof(naptTcpUdp) );
	naptTcpUdp.isCollision = 1;
	naptTcpUdp.isCollision2 = 1;
	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++)
		TBLFIELD(nat_tbl, nat_asic_set)( TRUE, flowTblIdx, &naptTcpUdp );
	return 0;
}


static struct nat_entry *lr_nat_lookup(struct nat_tuple *nat_tuple)
{
	struct nat_entry *nat_out;
	uint32 hash;

#ifdef CONFIG_RTL865XB
	hash = TBLFIELD(nat_tbl, nat_asic_hash)((uint8)nat_tuple->proto, nat_tuple->int_host.ip, nat_tuple->int_host.port, 0, 0);
#else
	hash = TBLFIELD(nat_tbl, nat_asic_hash)((uint8)nat_tuple->proto, nat_tuple->int_host.ip, nat_tuple->int_host.port, 
											nat_tuple->rem_host.ip, nat_tuple->rem_host.port);
#endif
	nat_out = &TBLFIELD(nat_tbl, nat_bucket)[hash];
	if (!memcmp(nat_out, nat_tuple, sizeof(*nat_tuple)) &&
		NAT_INUSE(nat_out))
		return nat_out;
	return (struct nat_entry *)0;
}


static int32 lr_arp_init(void)
{
	memset(TBLFIELD(arpt_tbl, arp_mask), 0, 64);
	return 0;
}


static int32 lr_arp_hash(ipaddr_t ip)
{
	struct if_entry *ife;

	ife = TBLFIELD(if_tbl, if_lookup)(NULL, ip);
	if (!ife)
		return -1;
	return ((ife->arp_start_<<3)+(ip&~ife->mask_));
}


static rtl865x_tblAsicDrv_arpParam_t *lr_arp_lookup(ipaddr_t ip)
{
	rtl865x_tblAsicDrv_arpParam_t *arpe;
	int32 hash;
	
	if ((hash=TBLFIELD(arpt_tbl, arp_hash)(ip)) == -1)
		return (rtl865x_tblAsicDrv_arpParam_t*)0;
	arpe = &TBLFIELD(arpt_tbl, __arpbuff);
	if (TBLFIELD(arpt_tbl, arp_asic_get)(hash, arpe))
		return (rtl865x_tblAsicDrv_arpParam_t*)0;
	return &TBLFIELD(arpt_tbl, __arpbuff);
}

static int32 lr_arp_tbl_alloc(struct if_entry *ife)
{
#if 1
	uint32 i;

	if (ife->isWan) {
		ife->arp_start_ = 32;
		ife->arp_end_  = 63;
	}
	else {
		ife->arp_start_ = 0;
		ife->arp_end_  = 31;
	}
	for(i=ife->arp_start_; i<=ife->arp_end_; i++) 
		TBLFIELD(arpt_tbl, arp_mask)[i] = ife-TBLFIELD(if_tbl, if_hash) + 1;
	return 0;

#else
	uint32 netSize, entry, bestSize=0, bestStartPos=0xffffffff;
	uint32 curSize=0, curStartPos=0, j;

	for(entry=0; entry<32; entry++)
		if(ife->mask_ & (1<<entry))	
			break;

	if ((netSize = (1<<entry)) > 1) {
		curStartPos = bestSize = curSize = 0;
		for(j = 0; j <= 64; j++) {
			if(j == 64 || TBLFIELD(arpt_tbl, arp_mask)[j]) {
				if(curSize > bestSize) {
					bestStartPos = curStartPos;
					bestSize = curSize;
				}
				curStartPos = j+1;
				curSize = 0;
			} else curSize++;
		}
	} 
	/* 12-03-2007, if (ife->mask_ == 255.255.254.0), this function will return -1 */
	if (entry >= 9) {
		netSize = 1<<8;
		bestSize = netSize >> 3;
	}
	if (netSize>1 && (bestSize<<3) >= netSize) {
		ife->arp_start_ = bestStartPos;
		ife->arp_end_	  = bestStartPos + (netSize>>3) - ((netSize&0x7)==0? 1: 0);
		/* 12-03-2007, prevent array overflow */        
		if (ife->arp_end_ > 63) 
			ife->arp_end_ = 63;
		for(entry=ife->arp_start_; entry<=ife->arp_end_; entry++) {
			TBLFIELD(arpt_tbl, arp_mask)[entry] = ife-TBLFIELD(if_tbl, if_hash) + 1;
          }
		return 0;
	}
	return -1;
#endif    
}


static int32 lr_arp_tbl_free(struct if_entry *ife)
{
	rtl865x_tblAsicDrv_arpParam_t arpe;
	uint32 index;
	int32 i, j, rc;

	if (!IF_UP(ife))
		return -1;
	for(i=ife->arp_start_; i<=ife->arp_end_; i++) {
		TBLFIELD(arpt_tbl, arp_mask)[i] = 0;
		for(j=0; j<8; j++) {
			index = (i<<3)+j;
			rc = TBLFIELD(arpt_tbl, arp_asic_get)(index, &arpe);
			if (!rc) {
				TBLFIELD(arpt_tbl, arp_asic_del)(index);
				TBLFIELD(fdb_tbl, fdb_asic_del)(arpe.nextHopRow, arpe.nextHopColumn);
			}
		}
	}
	return 0;
}


static int32 lr_pppoe_init(void)
{
	memset(TBLFIELD(pppoe_tbl, ppps), 0, sizeof(struct if_entry *)*RTL8651_PPPOETBL_SIZE);
	return 0;
}


static struct if_entry *lr_pppoe_lookup(uint32 sid, uint32 *index)
{
	struct if_entry *psif;
	uint32 entry;

	for(entry=0; entry<RTL8651_PPPOETBL_SIZE; entry++) {
		psif = TBLFIELD(pppoe_tbl, ppps)[entry];
		if (!psif)
			continue;
		if (psif->sid_ == sid) {
			if (index) *index=entry;
			return psif;
		}
	}
	return (struct if_entry *)0;
}


static int32 lr_vlan_init(void)
{
	memset(TBLFIELD(vlan_tbl, vhash), 0, sizeof(struct vlan_entry)*VLAN_NUMBER);
	memset(TBLFIELD(vlan_tbl, asic_vtbl), 0, sizeof(uint16)*RTL8651_VLAN_NUMBER);
	return 0;
}

#ifdef CONFIG_RTL865XC
static int32 lr_vlan_create(struct rtl865x_lrConfig *lrconfig)
{
	rtl865x_tblAsicDrv_vlanParam_t vlan;
	rtl865x_tblAsicDrv_intfParam_t intf;
	struct vlan_entry *ventry;
	int32 retval;
	int32 idx;

	if (lrconfig->vid==0 || lrconfig->vid==0xfff)
		return (int32)LR_INVVID;
	
	for (idx = 0; idx < RTL865XC_NETINTERFACE_NUMBER; idx++)
	{
		ventry = &TBLFIELD(vlan_tbl, vhash)[idx];
		if (ventry->valid == 1 && ventry->vid == lrconfig->vid)
			return (int32)LR_DUPENTRY;
		if (ventry->valid == 0)
			break;
	}
	if (idx == RTL865XC_NETINTERFACE_NUMBER)
		return (int32)LR_NOBUFFER;

	ventry->valid			= 1;
	ventry->fid			= lrconfig->fid;
	ventry->mbr			= lrconfig->memPort;
	ventry->untagSet		= lrconfig->untagSet;
	ventry->mac			= lrconfig->mac;
	ventry->vid                  =lrconfig->vid;

	memset(&vlan, 0, sizeof(rtl865x_tblAsicDrv_vlanParam_t));
	memset(&intf, 0, sizeof(rtl865x_tblAsicDrv_intfParam_t));
	vlan.memberPortMask = lrconfig->memPort;
	vlan.untagPortMask = lrconfig->untagSet;
	vlan.fid = lrconfig->fid;

	intf.enableRoute = 1;
	intf.inAclEnd			= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	intf.inAclStart		= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	intf.outAclEnd		= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	intf.outAclStart		= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	intf.macAddr = lrconfig->mac;
	intf.macAddrNumber = 1;
	intf.mtu = lrconfig->mtu;
	intf.valid = 1;
	intf.vid = lrconfig->vid;
	retval =  TBLFIELD(vlan_tbl, intf_asic_set)( RTL865XC_NETIFTBL_SIZE, &intf );
	if ( retval != LR_SUCCESS ) return retval;
	LR_CONFIG_CHECK(TBLFIELD(vlan_tbl, vlan_asic_set)(lrconfig->vid, &vlan));
	return (int32)LR_SUCCESS;
}
#else

static int32 lr_vlan_create(struct rtl865x_lrConfig *lrconfig)
{
	rtl865x_tblAsicDrv_vlanParam_t vlan;
	struct vlan_entry *ventry;
	int32 vhash;

	if (lrconfig->vid==0 || lrconfig->vid==0xfff)
		return (int32)LR_INVVID;
	ventry = &TBLFIELD(vlan_tbl, vhash)[lrconfig->vid];
	if (ventry->valid == 1)
		return (int32)LR_DUPENTRY;
	
	ventry->valid			= 1;
	ventry->fid			= lrconfig->fid;
	ventry->vid                 = lrconfig->vid;
	ventry->mbr			= lrconfig->memPort;
	ventry->untagSet		= lrconfig->untagSet;
	ventry->mac			= lrconfig->mac;
//	ventry->ifnet			= TBLFIELD(if_tbl, if_lookup)(lrconfig->ifname, 0);

	vhash = TBLFIELD(vlan_tbl, vlan_hash)(lrconfig->vid);
	if (TBLFIELD(vlan_tbl, asic_vtbl)[vhash])
		return (int32)LR_SUCCESS;
	TBLFIELD(vlan_tbl, asic_vtbl)[vhash] = lrconfig->vid;

	/* Initialize allocated VLAN */
	memset(&vlan, 0, sizeof(rtl865x_tblAsicDrv_vlanParam_t));
	vlan.valid				= 1;
	vlan.vid				= lrconfig->vid;
	vlan.memberPortMask	= lrconfig->memPort;
	vlan.untagPortMask	= lrconfig->untagSet;
	vlan.enableRoute		= 1;
	vlan.internal			= lrconfig->isWan? 0: 1;
	vlan.inAclEnd			= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	vlan.inAclStart		= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	vlan.outAclEnd		= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	vlan.outAclStart		= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
	vlan.mtu				= lrconfig->mtu;
	vlan.macAddrNumber	= 1;
	vlan.macAddr			= lrconfig->mac;
	vlan.broadcastToCpu	= 1;	/* default set broadcast to CPU */
	LR_CONFIG_CHECK(TBLFIELD(vlan_tbl, vlan_asic_set)(&vlan));
	return (int32)LR_SUCCESS;
}
#endif

void lr_vlan_update_mac( uint32 vid, uint8* mac )
{
	rtl865x_tblAsicDrv_intfParam_t intf;
	struct vlan_entry *ventry;
	int32 idx;

	for (idx = 0; idx < RTL865XC_NETINTERFACE_NUMBER; idx++)
	{
		ventry = &TBLFIELD(vlan_tbl, vhash)[idx];
		if ( ventry->vid == vid){
			if (ventry->valid == 1)
				break;
			else
				return;
		}
	}
	if (idx == RTL865XC_NETINTERFACE_NUMBER)
		return;
	memcpy(ventry->mac.octet, mac, 6);
	intf.vid = vid;
	TBLFIELD(vlan_tbl, intf_asic_get)(RTL865XC_NETIFTBL_SIZE, &intf);
	memcpy(intf.macAddr.octet, mac, 6);
	TBLFIELD(vlan_tbl, intf_asic_set)( RTL865XC_NETIFTBL_SIZE, &intf );
	return;
}

static int32 lr_vlan_remove(uint32 vid)
{
	struct if_entry *ife;
	int32 ifindex;

	ife = &TBLFIELD(if_tbl, if_hash)[0];
	for(ifindex=0; ifindex<NETIF_NUMBER; ifindex++, ife++)
		if (IF_UP(ife) && ife->vid==vid)
			return -1;
	TBLFIELD(vlan_tbl, vlan_asic_del)(vid);
	return 0;
}


static int32 lr_vlan_hash(uint16 vid)
{
	return (vid&(RTL8651_VLAN_NUMBER-1));
}

struct vlan_entry*  lr_get_vlan(uint32 vid)
{
	struct vlan_entry *ventry;
	if (vid==0 || vid==0xfff)
		return NULL;
	
#ifdef CONFIG_RTL865XC	
	uint32 idx;
	for (idx = 0; idx < RTL865XC_NETINTERFACE_NUMBER; idx++)
	{
		ventry = &TBLFIELD(vlan_tbl, vhash)[idx];
		if ( ventry->vid == vid){
			if (ventry->valid == 1)
				return ventry;
			else
				return NULL;
		}
	}
#else
	ventry = &TBLFIELD(vlan_tbl, vhash)[vid];
#endif
	if (ventry->valid == 1)
		return ventry;
	else
		return NULL;

}

struct vlan_entry*  lr_get_vlan_byidx(uint32 vidx)
{
	struct vlan_entry *ventry;
	if (vidx < 0 || vidx > RTL8651_VLAN_NUMBER -1)
		return NULL;
#ifdef CONFIG_RTL865XC
	ventry = &TBLFIELD(vlan_tbl, vhash)[vidx];	
#else
	uint32 vid;
	vid = TBLFIELD(vlan_tbl, asic_vtbl)[vidx];
	if (vid < 0 || vid > VLAN_NUMBER)
		return NULL;
	ventry = &TBLFIELD(vlan_tbl, vhash)[vid];	
#endif
	if (ventry->valid == 1)
		return ventry;
	else
		return NULL;

}

static int32 lr_acl_init(void)
{	
	int32 i;
	
	memset(TBLFIELD(acl_tbl, free_list), 0, sizeof(_rtl8651_tblDrvAclRule_t)*RTL865x_ACL_NUMBER);
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		CTAILQ_INIT(&TBLFIELD(acl_tbl, acl_in_head)[i]);
		CTAILQ_INIT(&TBLFIELD(acl_tbl, acl_eg_head)[i]);
	}
	CTAILQ_INIT(&TBLFIELD(acl_tbl, free_head));
	for(i=0; i<RTL865x_ACL_NUMBER; i++) {
		memset(&TBLFIELD(acl_tbl, free_list)[i], 0, sizeof(_rtl8651_tblDrvAclRule_t));
		CTAILQ_INSERT_HEAD(&TBLFIELD(acl_tbl, free_head), 
				&(acl_tbl.free_list[i]), nextRule); 
	}
	SET_DEFAULT_ACL((ACL_PERMIT+DEFAULT_INC), RTL8651_ACL_PERMIT);
	SET_DEFAULT_ACL((ACL_DROP+DEFAULT_INC), RTL8651_ACL_DROP);
	SET_DEFAULT_ACL((ACL_CPU+DEFAULT_INC), RTL8651_ACL_CPU);
	TBLFIELD(acl_tbl, def_action) = ACL_PERMIT;
	TBLFIELD(acl_tbl, acl_arrange)();
	return 0;
}


static int32 lr_acl_add(uint32 vid, _rtl8651_tblDrvAclRule_t *rule, enum ACL_FLAGS flags)
{
	CTAILQ_HEAD(_aclStruct, _rtl8651_tblDrvAclRule_s) *aclHead;
	_rtl8651_tblDrvAclRule_t *acl_rule;
	int32 vhash;
#ifdef CONFIG_RTL865XC
	struct vlan_entry *ventry;
	vhash = rtl8651_vlanTableIndex(vid);
	ventry = &TBLFIELD(vlan_tbl, vhash)[vhash];
	if (ventry->vid != vid)
		return LR_INVVID;
#else
	vhash = TBLFIELD(vlan_tbl, vlan_hash)(vid);
	if (vid == 0 || vid == 0xfff ||
	   TBLFIELD(vlan_tbl, asic_vtbl)[vhash] != vid)
		return LR_INVVID;
#endif
	acl_rule = CTAILQ_FIRST(&TBLFIELD(acl_tbl, free_head));
	if ( acl_rule == NULL || TBLFIELD(acl_tbl, entry)>=RTL865x_ACL_NUMBER)
		return LR_NOBUFFER;
	
	if (flags == ACL_INGRESS)
		aclHead = (struct _aclStruct *)&TBLFIELD(acl_tbl, acl_in_head)[vhash];
	else  aclHead = (struct _aclStruct *)&TBLFIELD(acl_tbl, acl_eg_head)[vhash];

	CTAILQ_REMOVE(&TBLFIELD(acl_tbl, free_head), 
			 acl_rule,  nextRule);

	memcpy(acl_rule, rule, sizeof(_rtl8651_tblDrvAclRule_t));
	CTAILQ_INSERT_TAIL(aclHead, acl_rule, nextRule);
	TBLFIELD(acl_tbl, acl_arrange)();
	return LR_SUCCESS;
}


static int32 lr_acl_del(uint32 vid, _rtl8651_tblDrvAclRule_t *rule, enum ACL_FLAGS flags)
{
	CTAILQ_HEAD(_aclStruct, _rtl8651_tblDrvAclRule_s) *aclHead;
	_rtl8651_tblDrvAclRule_t *acl_rule;
	int32 vhash;

#ifdef CONFIG_RTL865XC
	struct vlan_entry *ventry;
	vhash = rtl8651_vlanTableIndex(vid);
	ventry = &TBLFIELD(vlan_tbl, vhash)[vhash];
	if (ventry->vid != vid)
		return LR_INVVID;
#else
	vhash = TBLFIELD(vlan_tbl, vlan_hash)(vid);
	if (vid == 0 || vid == 0xfff ||
	   TBLFIELD(vlan_tbl, asic_vtbl)[vhash] != vid)
		return LR_INVVID;
#endif
	if (flags == ACL_INGRESS)
		aclHead = (struct _aclStruct *)&TBLFIELD(acl_tbl, acl_in_head)[vhash];
	else aclHead = (struct _aclStruct *)&TBLFIELD(acl_tbl, acl_eg_head)[vhash];
		
	CTAILQ_FOREACH(acl_rule, aclHead, nextRule) {
		if (memcmp(rule, acl_rule, 
		    sizeof(_rtl8651_tblDrvAclRule_t)-sizeof(rule->nextRule)))
		    continue;
		/* found, remove it! */
		CTAILQ_REMOVE(aclHead, acl_rule, nextRule);
		CTAILQ_INSERT_HEAD(&TBLFIELD(acl_tbl, free_head), acl_rule, nextRule);
		TBLFIELD(acl_tbl, acl_arrange)();
		return LR_SUCCESS;
	}
	return LR_NOTFOUND;
}

#ifdef CONFIG_RTL865XC
static int32 lr_acl_arrange(void)
{
	CTAILQ_HEAD(_aclStruct, _rtl8651_tblDrvAclRule_s) *aclHead;
	rtl865x_tblAsicDrv_intfParam_t netIf;	
	_rtl8651_tblDrvAclRule_t *acl_rule;
	int32 vhash, acl_start=0, acl_end=0, egin;
	uint16 vid;
	
	TBLFIELD(acl_tbl, entry) = 3;
	for(vhash=0; vhash<RTL8651_VLAN_NUMBER; vhash++) {
		struct vlan_entry *ventry;
		ventry = &TBLFIELD(vlan_tbl, vhash)[vhash];
		if ( ventry->valid == 0 || !(vid = ventry->vid))
			continue;
		netIf.vid = vid;
		TBLFIELD(vlan_tbl, intf_asic_get)(RTL865XC_NETIFTBL_SIZE, &netIf);

		for(egin=0; egin<2; egin++) {
			aclHead = (!egin)? (struct _aclStruct *)&TBLFIELD(acl_tbl, acl_in_head)[vhash]:
					(struct _aclStruct *)&TBLFIELD(acl_tbl, acl_eg_head)[vhash];
			CTAILQ_FOREACH(acl_rule, aclHead, nextRule) {
				TBLFIELD(acl_tbl, acl_asic_set)(acl_end, acl_rule);
				TBLFIELD(acl_tbl, entry) ++;
				acl_end ++;
			}
			if (acl_end == acl_start) {	/* no rule */	
				if (!egin) {
					netIf.inAclEnd	 	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
					netIf.inAclStart 	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
				} else {
					netIf.outAclEnd 	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
					netIf.outAclStart	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
				}
			}
			else {
				SET_DEFAULT_ACL(acl_end, TBLFIELD(acl_tbl, def_action));
				if (!egin) {
					netIf.inAclEnd		= acl_end;
					netIf.inAclStart	= acl_start;
				} else {
					netIf.outAclEnd	= acl_end;
					netIf.outAclStart	= acl_start;
				}
				acl_start = acl_end + 1;
				acl_end = acl_start;
				TBLFIELD(acl_tbl, entry) ++;
			}
			TBLFIELD(vlan_tbl, intf_asic_set)(RTL865XC_NETIFTBL_SIZE, &netIf);
		}
	}
	return 0;
	
}
#else
static int32 lr_acl_arrange(void)
{
	CTAILQ_HEAD(_aclStruct, _rtl8651_tblDrvAclRule_s) *aclHead;
	rtl865x_tblAsicDrv_vlanParam_t vlan;
	_rtl8651_tblDrvAclRule_t *acl_rule;
	int32 vhash, acl_start=0, acl_end=0, egin;
	uint16 vid;

	TBLFIELD(acl_tbl, entry) = 3;
	for(vhash=0; vhash<RTL8651_VLAN_NUMBER; vhash++) {		
		if (!(vid=TBLFIELD(vlan_tbl, asic_vtbl)[vhash]))
			continue;
		TBLFIELD(vlan_tbl, vlan_asic_get)(vid, &vlan);

		for(egin=0; egin<2; egin++) {
			aclHead = (!egin)? (struct _aclStruct *)&TBLFIELD(acl_tbl, acl_in_head)[vhash]:
					(struct _aclStruct *)&TBLFIELD(acl_tbl, acl_eg_head)[vhash];
			CTAILQ_FOREACH(acl_rule, aclHead, nextRule) {
				TBLFIELD(acl_tbl, acl_asic_set)(acl_end, acl_rule);
				TBLFIELD(acl_tbl, entry) ++;
				acl_end ++;
			}
			if (acl_end == acl_start) {	/* no rule */	
				if (!egin) {
					vlan.inAclEnd	 	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
					vlan.inAclStart 	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
				} else {
					vlan.outAclEnd 	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
					vlan.outAclStart	= TBLFIELD(acl_tbl, def_action) + DEFAULT_INC;
				}
			}
			else {
				SET_DEFAULT_ACL(acl_end, TBLFIELD(acl_tbl, def_action));
				if (!egin) {
					vlan.inAclEnd		= acl_end;
					vlan.inAclStart	= acl_start;
				} else {
					vlan.outAclEnd	= acl_end;
					vlan.outAclStart	= acl_start;
				}
				acl_start = acl_end + 1;
				acl_end = acl_start;
				TBLFIELD(acl_tbl, entry) ++;
			}
			TBLFIELD(vlan_tbl, vlan_asic_set)(&vlan);
		}
	}
	return 0;
}

#endif

static int32 lr_fdb_init(void)
{
	memset(TBLFIELD(fdb_tbl, FDB), 0, sizeof(uint8)*RTL8651_L2_NUMBER);
	return 0;
}


static rtl865x_tblAsicDrv_l2Param_t *lr_fdb_lookup(uint32 vfid, ether_addr_t *mac,  uint32 flags, uint32 *way)
{
	uint32 hash0, way0;
#ifdef CONFIG_RTL865XC
	hash0 = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
#else
	hash0 = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
	for(way0=0; way0<RTL8651_L2TBL_COLUMN; way0++) {
		if (rtl8651_getAsicL2Table(hash0, way0, &TBLFIELD(fdb_tbl, __l2buff))!=SUCCESS ||
			memcmp(&TBLFIELD(fdb_tbl, __l2buff).macAddr, mac, 6)!= 0)
			continue;
		if (((flags&FDB_STATIC) && TBLFIELD(fdb_tbl, __l2buff).isStatic) ||
			((flags&FDB_DYNAMIC) && !TBLFIELD(fdb_tbl, __l2buff).isStatic)) {
			assert(way);
			*way = way0;
			return &TBLFIELD(fdb_tbl, __l2buff);
		}
	} return (rtl865x_tblAsicDrv_l2Param_t *)0;
}



static int32 lr_bridging_rcv(struct rtl_pktHdr *pPkt)
{
#if 1
    return 0;
#else
	rtl865x_tblAsicDrv_l2Param_t *l2_entry;
	struct rtl_mBuf *m = pPkt->ph_mbuf;
	//struct if_entry *ife;
	struct vlan_entry *ventry;
	uint32 vid, way, rxPort, tx_list;

	rxPort = pPkt->ph_portlist;

#ifdef CONFIG_RTL865XC
	int32 idx;
	int32 found = 0; 
	vid = 0;
	if (rxPort >= 7)
	{
		if (rtl865x_bridge_rxProcess(pPkt) == SUCCESS)
			rxPort = pPkt->ph_portlist;
		else{
			mBuf_freeMbufChain(m);
			lr_stats.rx_drop ++;
			return -1;
		}
	}

	for (idx =0; idx < RTL865XC_VLAN_NUMBER; idx++)
	{
		ventry = &TBLFIELD(vlan_tbl, vhash)[idx];
		if (ventry->valid == 0 )
			continue;
		if (ventry->mbr & (1 << rxPort))
		{
			found = 1;
			vid = ventry->vid;
			pPkt->ph_vlanIdx = idx;
			break;
		}			
	}
	if (found == 0)
	{
		mBuf_freeMbufChain(m);
		lr_stats.rx_drop ++;
		return -1;
	}

#else
	vid = TBLFIELD(vlan_tbl, asic_vtbl)[pPkt->ph_vlanIdx];
	ventry = &TBLFIELD(vlan_tbl, vhash)[vid];
#endif

	if (ventry->valid == 0) {
		mBuf_freeMbufChain(m);
		lr_stats.rx_drop ++;
		return -1;
	}

	/* if dmac is one of the interface's mac, trap to CPU */
	//if (((ife=ventry->ifnet)&&!memcmp(&ife->mac, m->m_data, 6))  ||
	if (!memcmp(&ventry->mac, m->m_data, 6) ||(m->m_data[0]&0x01)) {
		if (!__drvRx) {
			mBuf_freeMbufChain(m);
			lr_stats.rx_drop ++;
			return -1;
		}
		lr_stats.rx_packets ++;
		lr_stats.rx_bytes += m->m_len;
		if (m->m_data[0] == 0xff)
			lr_stats.rx_bcast ++;
		else if (m->m_data[0]&0x01)
			lr_stats.rx_mcast ++;
		return __drvRx(pPkt);
	}
	if (rtl8651_fwdEngineInput(pPkt) == SUCCESS)
	{
		lr_stats.br_packets ++;
		return SUCCESS;
	}

	/* otherwise, handle layer2 bridging */
	if (TBLFIELD(port_attr, ing_filter)[rxPort] && !((1<<rxPort)&ventry->mbr)) {
		mBuf_freeMbufChain(m);
		lr_stats.rx_drop ++;
		return -1;
	}
	l2_entry = TBLFIELD(fdb_tbl, fdb_lookup)(vid, (ether_addr_t *)m->m_data, (FDB_STATIC|FDB_DYNAMIC), &way);
	tx_list = (l2_entry)? (l2_entry->memberPortMask): (0xffffffff);
	tx_list &= ventry->mbr;
	lr_stats.br_packets ++;
	pPkt->ph_srcExtPortNum = 0;
	return rtl865x_lightRomeSend(pPkt, m->m_len, vid, tx_list);
#endif
}



static int32 lr_route_init(void)
{
	memset(TBLFIELD(rt_tbl, pendgw), 0, sizeof(ipaddr_t)*RTL8651_ROUTINGTBL_SIZE);
	memset(&TBLFIELD(rt_tbl, __l3buff), 0, sizeof(rtl865x_tblAsicDrv_routingParam_t));
	
	/* Set 7th route entry to toCPU if no default route is present */
	TBLFIELD(rt_tbl, __l3buff).process = 0x04; /* trap toCPU */
	TBLFIELD(rt_tbl, rt_asic_set)(RTL8651_ROUTINGTBL_SIZE-1, &TBLFIELD(rt_tbl, __l3buff));
	return 0;
}


static rtl865x_tblAsicDrv_routingParam_t *	lr_route_lookup(ipaddr_t ip, ipaddr_t mask, uint32 *rthidx) 
{
	int32 rtidx, rc;
	
	for(rtidx=0; rtidx<RTL8651_ROUTINGTBL_SIZE; rtidx++) {
		rc = TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &TBLFIELD(rt_tbl, __l3buff));
		if (rc) continue;
		/* The 7th entry should be taken care of specially */
		if (rtidx == RTL8651_ROUTINGTBL_SIZE-1) {
			if (TBLFIELD(rt_tbl, __l3buff).process == 0x04 &&
			    TBLFIELD(rt_tbl, pendgw)[RTL8651_ROUTINGTBL_SIZE-1] == 0 )
			    continue;
			else	TBLFIELD(rt_tbl, __l3buff).ipMask = 0;
		}
		if ( TBLFIELD(rt_tbl, __l3buff).ipAddr==ip && 
		     	TBLFIELD(rt_tbl, __l3buff).ipMask==mask ) {
		     	assert(rthidx);
			*rthidx = rtidx;
			return &TBLFIELD(rt_tbl, __l3buff);
		}
	}
	return (rtl865x_tblAsicDrv_routingParam_t *)0;
}




void rt_arp_sync(ipaddr_t ip, uint32 row, uint32 column, enum SYNC_FLAG dowhat)
{
	rtl865x_tblAsicDrv_routingParam_t l3entry;
	int32 rtidx, rc;
	
	for(rtidx=0; rtidx<RTL8651_ROUTINGTBL_SIZE; rtidx++) {
		if (dowhat == SYNC_DEL) {
			rc = TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &l3entry);
			if (rc || l3entry.process!=0x01) 
				continue;
			if (l3entry.nextHopRow!=row || 
				l3entry.nextHopColumn!=column)
				continue;
			/* update the route: lack gw info. */
			l3entry.process = 0x04;
			TBLFIELD(rt_tbl, rt_asic_set)(rtidx, &l3entry);
			TBLFIELD(rt_tbl, pendgw)[rtidx] = ip;
		} 
		else if (dowhat == SYNC_ADD) {
			if (TBLFIELD(rt_tbl, pendgw)[rtidx]!=ip) 
				continue;
			rc = TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &l3entry);
			assert(!rc && l3entry.process==0x04);
			l3entry.process 		= 0x01;
			l3entry.nextHopRow	= row;
			l3entry.nextHopColumn	= column;
			TBLFIELD(rt_tbl, rt_asic_set)(rtidx, &l3entry);
			TBLFIELD(rt_tbl, pendgw)[rtidx] = 0;
		}
	}
}


