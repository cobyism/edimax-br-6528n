

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
#include <linux/romedrv_linux.h>

extern struct lr_cpu_stats 					lr_stats;
extern struct if_table						if_tbl;
extern struct natip_table					natip_tbl;
extern struct nat_table						nat_tbl;
extern struct rt_table						rt_tbl;
extern struct arp_table						arpt_tbl;
extern struct pppoe_table					pppoe_tbl;
extern struct vlan_table					vlan_tbl;
extern struct acl_table						acl_tbl;
extern struct fdb_table						fdb_tbl;
extern struct port_attribute					port_attr;
extern proc_input_pkt_funcptr_t				__drvRx;
static int32 __LR_Config;

unsigned char _L2_mac[6];
unsigned char *get_l2_mac(int port)
{
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
 	uint32 row;

	for(row=0; row<RTL8651_L2TBL_ROW; row++)
	{
		memset((void*)&asic_l2, 0, sizeof(asic_l2));
		if (rtl8651_getAsicL2Table(row, 0, &asic_l2) == FAILED) // column: 0 ???
		{
			continue;
		}

		if (asic_l2.memberPortMask & (1<< port))
		{
					//rtlglue_printf("get_l2_mac_with_port_zero [%d] %02x:%02x:%02x:%02x:%02x:%02x \n",row,  
					//		asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], 
					//		asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5]);
			memcpy(_L2_mac, asic_l2.macAddr.octet, 6);
			return _L2_mac;
		}
	}
	return NULL;
}

#if 0
/*
 Usage: MUST define CONFIG_RTL865X_LIGHT_ROMEDRV in defconfig-rtl865x-gw-xxx file, 
 		no matter what CONFIG_RTL865X_HW_TABLES is defined or not.

	argument 1: vfid: always 1 if CONFIG_RTL865X_HW_TABLES is defined
					0: for LAN(eth0) and CONFIG_RTL865X_HW_TABLES is un-defined
					1: for WAN(eth1) and CONFIG_RTL865X_HW_TABLES is un-defined
	argument 2: port_number: 0 ~ 5
	argument 3: flags: always FDB_FWD
*/
enum LR_RESULT rtl865x_addL2TableStaticEntry(uint32 vfid, 
	uint32 port_number, enum FDB_FLAGS flags, char *mac_addr)
{
	ether_addr_t mac;
	uint32 way, hash;
	rtl865x_tblAsicDrv_l2Param_t l2;

	if (port_number >= RTL8651_MAC_NUMBER)
		return LR_INVAPARAM;
	
	memcpy(mac.octet, mac_addr, 6);
#ifdef CONFIG_RTL865XC
	hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(&mac, vfid);
#else
	hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(&mac);
#endif	

	if (flags != FDB_FWD && flags != FDB_SRCBLK && flags != FDB_TRAPCPU)
		return LR_INVAPARAM; /* Invalid parameter */

	/* check duplicate entry */
	if (!TBLFIELD(fdb_tbl, fdb_lookup)(vfid, &mac, FDB_DYNAMIC, &way)) {
		for (way = 0; way < RTL8651_L2TBL_COLUMN; way++)
			if (TBLFIELD(fdb_tbl, fdb_asic_get)(hash, way, &l2))
				break;
		if (way == RTL8651_L2TBL_COLUMN)
			way = 0;
	}
	TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way]++;
	memset(&l2, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	l2.isStatic			= 1;
	l2.ageSec			= L2_AGING_TIME;
	l2.macAddr			= mac;
	l2.memberPortMask		= 1 << port_number;
	l2.cpu				= (flags == FDB_TRAPCPU) ? 1: 0;
	l2.srcBlk				= (flags == FDB_SRCBLK) ? 1: 0;
#ifdef CONFIG_RTL865XC
	l2.auth				=  0;
	l2.fid				=  vfid;
#endif
	LR_CONFIG_CHECK(TBLFIELD(fdb_tbl, fdb_asic_set)(hash, way, &l2));
	LR_DEBUG("LR(addL2TableStaticEntry):  [ %d,%d ] %02x:%02x:%02x:%02x:%02x:%02x, fid = %d, mbr = %d\n", 
			hash, way, mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3], mac->octet[4], 
			mac->octet[5], vfid, port_number);
	return LR_SUCCESS;
}
#endif

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
static ether_addr_t pppMac = { { 0x00, 0xe0, 0x4c, 0x00, 0x00, 0x01 } };
static enum LR_RESULT rtl865x_addPppFdbEntry(uint32 vfid, uint32 portmask, enum FDB_FLAGS flags)
{
	ether_addr_t *mac = &pppMac;

#ifdef CONFIG_RTL865XC
	uint32 way, hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
#else
	uint32 way, hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
	rtl865x_tblAsicDrv_l2Param_t l2;

	if (flags != FDB_FWD && flags != FDB_SRCBLK && flags != FDB_TRAPCPU)
		return LR_INVAPARAM; /* Invalid parameter */

	/* check duplicate entry */
	if (TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_STATIC, &way))
		return LR_DUPENTRY;	
	if (!TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_DYNAMIC, &way)) {
		for (way = 0; way < RTL8651_L2TBL_COLUMN; way++)
			if (TBLFIELD(fdb_tbl, fdb_asic_get)(hash, way, &l2))
				break;
		if (way == RTL8651_L2TBL_COLUMN)
			way = 0;
	}
	TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way]++;
	memset(&l2, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	l2.isStatic				= 1;
	l2.ageSec				= L2_AGING_TIME;
	l2.macAddr				= *(mac);
	l2.memberPortMask		= portmask;
	l2.cpu					= (flags == FDB_TRAPCPU) ? 1: 0;
	l2.srcBlk				= (flags == FDB_SRCBLK) ? 1: 0;
#ifdef CONFIG_RTL865XC
	l2.auth					=  0;
	l2.fid					=  vfid;
#endif
	LR_CONFIG_CHECK(TBLFIELD(fdb_tbl, fdb_asic_set)(hash, way, &l2));
	LR_DEBUG("LR(addPppFdbEntry):  [ %d,%d ] %02x:%02x:%02x:%02x:%02x:%02x, fid = %d, mbr = 0x%x\n", 
			hash, way, mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3], mac->octet[4], mac->octet[5], vfid, portmask);
	return LR_SUCCESS;
}
#endif

enum LR_RESULT rtl865x_addFdbEntry( uint32 vfid, ether_addr_t * mac, uint32 portmask, enum FDB_FLAGS flags )
{
#ifdef CONFIG_RTL865XC
	uint32 way, hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
#else
	uint32 way, hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
	rtl865x_tblAsicDrv_l2Param_t l2;
	
	if (flags != FDB_FWD && flags != FDB_SRCBLK && flags != FDB_TRAPCPU)
		return LR_INVAPARAM; /* Invalid parameter */

	/* check duplicate entry */
	if (TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_STATIC, &way))
		return LR_DUPENTRY;
	if (!TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_DYNAMIC, &way)) {
		for(way=0; way<RTL8651_L2TBL_COLUMN; way++)
			if (TBLFIELD(fdb_tbl, fdb_asic_get)(hash, way, &l2))
				break;
		if (way == RTL8651_L2TBL_COLUMN)
			return LR_NOBUFFER;
	}
	TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way] ++;
	memset(&l2, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	l2.isStatic				= 1;
	l2.ageSec				= L2_AGING_TIME;
	l2.macAddr				= *(mac);
	l2.memberPortMask			= portmask;
	l2.cpu					= (flags==FDB_TRAPCPU)? 1: 0;
	l2.srcBlk					= (flags==FDB_SRCBLK)? 1: 0;
#ifdef CONFIG_RTL865XC
	l2.auth					=  0;
	l2.fid					=  vfid;
#endif
	LR_CONFIG_CHECK(TBLFIELD(fdb_tbl, fdb_asic_set)(hash, way, &l2));
	return LR_SUCCESS;
}

enum LR_RESULT rtl865x_addExtFdbEntry( uint32 vfid, ether_addr_t * mac, uint32 portmask, uint32 linkId, enum FDB_FLAGS flags )
{
#ifdef CONFIG_RTL865XC
	uint32 way, hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
#else
	uint32 way, hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
	rtl865x_tblAsicDrv_l2Param_t l2;
	
	if (flags != FDB_FWD && flags != FDB_SRCBLK && flags != FDB_TRAPCPU)
		return LR_INVAPARAM; /* Invalid parameter */

	/* check duplicate entry */
	if (TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_STATIC, &way))
		return LR_DUPENTRY;
	if (!TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_DYNAMIC, &way)) {
		for(way=0; way<RTL8651_L2TBL_COLUMN; way++)
			if (TBLFIELD(fdb_tbl, fdb_asic_get)(hash, way, &l2))
				break;
		if (way == RTL8651_L2TBL_COLUMN)
			return LR_NOBUFFER;
	}
	TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way] ++;
	TBLFIELD(fdb_tbl, LinkID)[(hash<<2)+way]  = linkId;
	memset(&l2, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	l2.isStatic				= 0;
	l2.ageSec				= L2_AGING_TIME;
	l2.macAddr				= *(mac);
	l2.memberPortMask			= portmask;
	l2.cpu					= (flags==FDB_TRAPCPU)? 1: 0;
	l2.srcBlk					= (flags==FDB_SRCBLK)? 1: 0;
#ifdef CONFIG_RTL865XC
	l2.fid                                   = vfid;
#endif
	LR_CONFIG_CHECK(TBLFIELD(fdb_tbl, fdb_asic_set)(hash, way, &l2));
	return LR_SUCCESS;
}

enum LR_RESULT lr_ext_fdb_lookup(uint32 vfid, ether_addr_t *mac,  uint32 *memport, uint32 *linkID, uint32 *way, enum FDB_FLAGS flags)
{
	uint32 hash0, way0;
#ifdef CONFIG_RTL865XC
	hash0 =TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
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
			*memport = TBLFIELD(fdb_tbl, __l2buff).memberPortMask;
			if (TBLFIELD(fdb_tbl, LinkID)[(hash0<<2)+way0] > 0)
				*linkID = TBLFIELD(fdb_tbl, LinkID)[(hash0<<2)+way0];
			else
				*linkID = 0;
			return LR_SUCCESS;
		}
	} 
	return LR_INVAPARAM;
}

enum LR_RESULT rtl865x_modifyExtFdbEntry( uint32 vfid, ether_addr_t * mac, uint32 portmask, uint32 linkId, enum FDB_FLAGS flags )
{
	uint32 way;
	uint32 memport;
	uint32 old_linkId;
	rtl865x_tblAsicDrv_l2Param_t l2;
	if (lr_ext_fdb_lookup(vfid, mac, &memport, &old_linkId, &way, FDB_DYNAMIC) != LR_SUCCESS)
		return LR_FAILED;
	if (portmask < 0 || linkId <= 0)
		return LR_FAILED;
	if (flags != FDB_FWD && flags != FDB_SRCBLK && flags != FDB_TRAPCPU)
		return LR_INVAPARAM; /* Invalid parameter */
#ifdef CONFIG_RTL865XC
	uint32 hash =TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
#else
	uint32 hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);	
#endif

	/* check duplicate entry */
	if (TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_STATIC, &way))
		return LR_DUPENTRY;

	TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way] ++;
	TBLFIELD(fdb_tbl, LinkID)[(hash<<2)+way]  = linkId;
	memset(&l2, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	l2.isStatic				= 0;
	l2.ageSec				= L2_AGING_TIME;
	l2.macAddr				= *(mac);
	l2.memberPortMask			= portmask;
	l2.cpu					= (flags==FDB_TRAPCPU)? 1: 0;
	l2.srcBlk					= (flags==FDB_SRCBLK)? 1: 0;
#ifdef CONFIG_RTL865XC
	l2.fid                                   = vfid;
#endif
	LR_CONFIG_CHECK(TBLFIELD(fdb_tbl, fdb_asic_set)(hash, way, &l2));
	
	return LR_SUCCESS;
}


enum LR_RESULT rtl865x_delFdbEntry( uint32 vfid, ether_addr_t * mac )
{
	enum LR_RESULT res = LR_NOTFOUND;
	uint32 way, hash;
	
	if (TBLFIELD(fdb_tbl, fdb_lookup)(vfid, mac, FDB_STATIC, &way)) {
		res = LR_SUCCESS;
#ifdef CONFIG_RTL865XC
		hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, vfid);
#else
		hash=TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
		if (--TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way] == 0){
			LR_CONFIG_CHECK(TBLFIELD(fdb_tbl, fdb_asic_del)(hash, way));
			TBLFIELD(fdb_tbl, LinkID)[(hash<<2)+way]  = 0;
		}
	} 
	return res;
}

extern unsigned int wan_port; 

enum LR_RESULT rtl865x_addArp( ipaddr_t ip, ether_addr_t * mac, enum ARP_FLAGS flags )
{
	rtl865x_tblAsicDrv_l2Param_t *fdbe;
	rtl865x_tblAsicDrv_arpParam_t arpe;
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };
	struct if_entry *ife;
	uint32 hash,way;
	int32 rc;

	if (TBLFIELD(arpt_tbl, arp_lookup)(ip))
		return LR_DUPENTRY;
	ife = TBLFIELD(if_tbl, if_lookup)(NULL, ip);
	if (ife==NULL)
		return LR_INVAPARAM;
	if (!IF_UP(ife))
		return LR_IFDOWN;

	if (mac == NULL) {
		if ((mac = (ether_addr_t *)get_l2_mac(wan_port)) == NULL)
			return LR_INVAPARAM;
	}
	
	hash = TBLFIELD(arpt_tbl, arp_hash)(ip);
	for(rc=0; rc<2; rc++) {
		//fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(ife->fid, mac, fdb_type[rc], &way);
		fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife), mac, fdb_type[rc], &way);
		if (fdbe) {/* present an fdb entry in the ASIC */
			arpe.nextHopColumn = way;
#ifdef CONFIG_RTL865XC
			arpe.aging    =    300;
			arpe.nextHopRow = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, GET_IF_VID(ife));
#else
			arpe.nextHopRow = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
			TBLFIELD(arpt_tbl, arp_asic_set)(hash, &arpe);
			TBLFIELD(fdb_tbl, FDB)[(arpe.nextHopRow<<2)+arpe.nextHopColumn] ++;
			if (fdb_type[rc]==FDB_DYNAMIC) {
				/* force dynamic entry to static entry */
#if 0
				fdbe->isStatic = 1;
#else
				fdbe->nhFlag  = 1;
#endif
				fdbe->ageSec = L2_AGING_TIME;
				TBLFIELD(fdb_tbl, fdb_asic_set)(arpe.nextHopRow, arpe.nextHopColumn, fdbe);
			}
			rt_arp_sync(ip, arpe.nextHopRow, arpe.nextHopColumn, SYNC_ADD);
			return LR_SUCCESS;
		}
	}	
	
	/* No specified mac address presents in fdb table, give up !! */
	return LR_FAILED;
}


enum LR_RESULT rtl865x_delArp( ipaddr_t ip )
{
	rtl865x_tblAsicDrv_arpParam_t *arpe;
	uint32 arp_hash, l2pos;
	
	arpe = TBLFIELD(arpt_tbl, arp_lookup)(ip);
	if (arpe == NULL)
		return LR_NOTFOUND;
	arp_hash = TBLFIELD(arpt_tbl, arp_hash)(ip);
	TBLFIELD(arpt_tbl, arp_asic_del)(arp_hash);
	l2pos = (arpe->nextHopRow<<2) + arpe->nextHopColumn;
	if (--TBLFIELD(fdb_tbl, FDB)[l2pos] == 0) {
		rt_arp_sync(ip, arpe->nextHopRow, arpe->nextHopColumn, SYNC_DEL);
		TBLFIELD(fdb_tbl, fdb_asic_del)(arpe->nextHopRow, arpe->nextHopColumn);
	}
	return LR_SUCCESS;
}


uint32 rtl865x_arpSync( ipaddr_t ip, uint32 refresh )
{
	rtl865x_tblAsicDrv_arpParam_t *arpe;
	rtl865x_tblAsicDrv_l2Param_t l2entry;
	uint32 age;

	arpe = TBLFIELD(arpt_tbl, arp_lookup)(ip);
	if (!arpe)
		return 0;
	TBLFIELD(fdb_tbl, fdb_asic_get)(arpe->nextHopRow, 
			arpe->nextHopColumn, &l2entry);
	age = l2entry.ageSec;
	if (refresh) {
		l2entry.ageSec = 300;
		TBLFIELD(fdb_tbl, fdb_asic_set)(arpe->nextHopRow, 
				arpe->nextHopColumn, &l2entry);
	}
	return age;
}



/*
*	dst/mask		gw		dev		Routing type
*	=======		==		===		========================================
*		0		 0		 0		(X) Invalid route
*		0		 0		 1		PPP type default route 
*		0		 1		 0		ether type default route
*		0		 1		 1		ppp/ether type default route, depends on dev type
*		1		 0		 0		(X) Invalid route
*		1		 0 		 1		ppp or interface route, depends on dev type
*		1		 1		 0		ether type route
*		1		 1		 1		ppp/ether type route, depends on dev type
*
*/

enum LR_RESULT rtl865x_addRoute( ipaddr_t ip, ipaddr_t mask, ipaddr_t gateway, uint8* ifname, enum RT_FLAGS flags )
{
	rtl865x_tblAsicDrv_routingParam_t rth, rth0;
	rtl865x_tblAsicDrv_arpParam_t *arpe;
	rtl865x_tblAsicDrv_l2Param_t *fdbe;
	struct if_entry *ife0, *ife1;
	int32 rtidx=0;
	uint32 way, pindex;
#ifdef CONFIG_RTL865X_HW_PPTPL2TP
	uint32 vid = 0;
#endif

	memset(&rth, 0, sizeof(rtl865x_tblAsicDrv_routingParam_t));
	ife0 = TBLFIELD(if_tbl, if_lookup)(ifname, 0);
	ife1 = TBLFIELD(if_tbl, if_lookup)(NULL, gateway);
	if (!ife0 && !ife1)
		return LR_INVIF;
	
#ifdef CONFIG_RTL865X_HW_PPTPL2TP
	if (ife0 && ife0->if_type == IF_PPP) {
		if (IF_UP(ife0)) {
			if (ife0->sid_) {
				/* PPPoE */
				ife1 = TBLFIELD(pppoe_tbl, pppoe_lookup)(ife0->sid_, &pindex);
				fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife0), &ife0->pmac_, FDB_STATIC, &way);
				if (IF_UP(ife0) && fdbe && ife1) {
					rth.process			= 0x0;
					rth.pppoeIdx		= pindex;
					rth.nextHopColumn	= way;
#ifdef CONFIG_RTL865XC
					rth.nextHopRow = TBLFIELD(fdb_tbl, fdb_asic_hash)(&ife0->pmac_, GET_IF_VID(ife0));
#else
					rth.nextHopRow = TBLFIELD(fdb_tbl, fdb_asic_hash)(&ife0->pmac_);
#endif
					for (rtidx = RTL8651_ROUTINGTBL_SIZE-2; rtidx >= 0; rtidx--)
						if (TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &rth0))
							break;
					if (rtidx < 0)
						return LR_NOBUFFER;
					else {
						vid = 8; /* hack to eth1 */
						goto add_rt;
					}
				}
				return LR_RTUNREACH;			
			}
			else {
				/* PPTP/L2TP */
				fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife0), &pppMac, FDB_STATIC, &way);
				rth.process = 0x1;
				rth.nextHopColumn = way;
#ifdef CONFIG_RTL865XC
				rth.nextHopRow = TBLFIELD(fdb_tbl, fdb_asic_hash)(&pppMac, GET_IF_VID(ife0));
#else
				rth.nextHopRow = TBLFIELD(fdb_tbl, fdb_asic_hash)(&pppMac);
#endif
				for (rtidx = RTL8651_ROUTINGTBL_SIZE-2; rtidx >= 0; rtidx--)
					if (TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &rth0))
						break;
				if (rtidx < 0)
					return LR_NOBUFFER;
				else 
					goto add_rt;
			}
		}
		else
			return LR_IFDOWN;
	}
#else
	if (ife0 && ife0->if_type == IF_PPPOE) {
		ife1 = TBLFIELD(pppoe_tbl, pppoe_lookup)(ife0->sid_, &pindex);
		//fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(ife0->fid, &ife0->pmac_, FDB_STATIC, &way);
		fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife0), &ife0->pmac_, FDB_STATIC, &way);
		if (IF_UP(ife0) && fdbe && ife1) {
			rth.process		= 0x0;
			rth.pppoeIdx		= pindex;
			rth.nextHopColumn	= way;
#ifdef CONFIG_RTL865XC
			rth.nextHopRow	= TBLFIELD(fdb_tbl, fdb_asic_hash)(&ife0->pmac_, GET_IF_VID(ife0));
#else
			rth.nextHopRow	= TBLFIELD(fdb_tbl, fdb_asic_hash)(&ife0->pmac_);
#endif
			for(rtidx=RTL8651_ROUTINGTBL_SIZE-2; rtidx>=0; rtidx--)
				if (TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &rth0))
					break;
			if (rtidx<0)
				return LR_NOBUFFER;
			else goto add_rt;
		}
		return LR_RTUNREACH;
	}
#endif /* CONFIG_RTL865X_HW_PPTPL2TP */

	if ((ife0&&ife0->if_type==IF_ETHER) || (ife1&&ife1->if_type==IF_ETHER)) {
		if (ife0 && ife1 && ife0!=ife1)
			return LR_RTUNREACH;
		if (ife0 && !gateway) { 
			if (!(ip&mask) || TBLFIELD(if_tbl, if_up)(ifname, ip, mask, 0, NULL))
				return LR_INVAPARAM;
			rth.process	= 0x2; /* interface route */
			rth.arpStart	= ife0->arp_start_;
			rth.arpEnd	= ife0->arp_end_;
			for(rtidx=0; rtidx<RTL8651_ROUTINGTBL_SIZE-1; rtidx++)
				if (TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &rth0)) 
					break;
			if (rtidx==RTL8651_ROUTINGTBL_SIZE-1)
				return LR_NOBUFFER;
			else goto add_rt;
		}
		ife0 = (!ife0)? ife1: ife0;
		arpe = TBLFIELD(arpt_tbl, arp_lookup)(gateway);
		if (arpe) {
			rth.process		= 0x1; /* direct route */
			rth.nextHopRow	= arpe->nextHopRow;
			rth.nextHopColumn	= arpe->nextHopColumn;
		} else rth.process = 0x4; /* CPU */
		for(rtidx=RTL8651_ROUTINGTBL_SIZE-2; rtidx>=0; rtidx--)
			if (TBLFIELD(rt_tbl, rt_asic_get)(rtidx, &rth0))
				break;
		if (rtidx < 0) return LR_NOBUFFER;
	}
		
add_rt: /* set ASIC RT entry */
	rtidx = (ip&mask)?  rtidx: RTL8651_ROUTINGTBL_SIZE-1;	
	rth.ipAddr		= ip&mask;
	rth.ipMask		= mask;
#ifdef CONFIG_RTL865XC
#ifdef CONFIG_RTL865X_HW_PPTPL2TP
	rth.vidx			= vid ? rtl8651_vlanTableIndex(vid) : rtl8651_vlanTableIndex(ife0->vid);
#else
	rth.vidx			= rtl8651_vlanTableIndex(ife0->vid);
#endif
	rth.internal            = ife0->isWan? 0:1;
#else
	rth.vidx			= ife0->vid & 0x07;
#endif
	TBLFIELD(rt_tbl, rt_asic_set)(rtidx, &rth);
	TBLFIELD(rt_tbl, pendgw)[rtidx] = (rth.process=0x04)? gateway: 0;	
	return LR_SUCCESS;
}

/*
int  rtl865x_netif_isWan( uint8* ifname )
{
	struct if_entry *ife0;

	ife0 = TBLFIELD(if_tbl, if_lookup)(ifname, 0);
	if (!ife0)
		return 0;

	return (ife0->isWan? 1:0);
}
*/

void rtl865x_updateNetIfTabMac( uint8* ifname, uint8* mac )
{
	struct if_entry *ife;

	ife = TBLFIELD(if_tbl, if_lookup)(ifname, 0);
	if (ife)
		lr_vlan_update_mac(ife->vid, mac);
}
	
enum LR_RESULT rtl865x_delRoute( ipaddr_t ip, ipaddr_t mask )
{
	rtl865x_tblAsicDrv_routingParam_t *rth;
	struct if_entry *ife;
	uint32 rtidx;

	if (!(rth=TBLFIELD(rt_tbl, route_lookup)(ip, mask, &rtidx)))
		return LR_NOTFOUND;
	if (rth->process == 2) { /* Interface route */
		ife = TBLFIELD(if_tbl, if_lookup)(NULL, ip);
		assert( ife );
		TBLFIELD(if_tbl, if_down)(ife->name);
	}
	TBLFIELD(rt_tbl, rt_asic_del)(rtidx);
	TBLFIELD(rt_tbl, pendgw)[rtidx] = 0;
	/* The removed route is default route, add 7th entry to toCPU */
	if (rtidx == RTL8651_ROUTINGTBL_SIZE-1) {
		memset(&TBLFIELD(rt_tbl, __l3buff), 0, 
			sizeof(rtl865x_tblAsicDrv_routingParam_t));
		TBLFIELD(rt_tbl, __l3buff).process = 0x04; /* trap toCPU */
		TBLFIELD(rt_tbl, rt_asic_set)(RTL8651_ROUTINGTBL_SIZE-1, 
			&TBLFIELD(rt_tbl, __l3buff));
	}
	return LR_SUCCESS;
}



enum LR_RESULT rtl865x_addPppoeSession( uint8* ifname, ether_addr_t *mac, uint32 sessionId, enum SE_FLAGS flags )
{
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };
	rtl865x_tblAsicDrv_pppoeParam_t pppoe;
	rtl865x_tblAsicDrv_l2Param_t *fdbe;
	struct if_entry *ife;
	enum LR_RESULT rc;
	uint32 idx, way, hash, pppidx;

	if (!(ife=TBLFIELD(if_tbl, if_lookup)(ifname, 0)))
		return LR_NOTFOUND;
	if (ife->if_type != IF_PPPOE)
		return LR_INVAPARAM;
	if (TBLFIELD(pppoe_tbl, pppoe_lookup)(sessionId, NULL))
		return LR_EXIST;	
	if ((rc=TBLFIELD(if_tbl, if_up)(ifname, 0, 0, sessionId, mac)))
		return rc;
	for(pppidx=0; pppidx<RTL8651_PPPOETBL_SIZE; pppidx++)
		if (!TBLFIELD(pppoe_tbl, ppps[pppidx]))
			break;
	if (pppidx == RTL8651_PPPOETBL_SIZE)
		return LR_NOBUFFER;
#ifdef CONFIG_RTL865XC
	hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac, GET_IF_VID(ife));
#else
	hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(mac);
#endif
	for(idx=0; idx<2; idx++) {
		//fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(ife->fid, mac, fdb_type[idx], &way);
		fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife), mac, fdb_type[idx], &way);
		if (fdbe) {
			TBLFIELD(fdb_tbl, FDB)[(hash<<2)+way] ++;
			if (fdb_type[idx] == FDB_DYNAMIC) {
				/* force dynamic entry to static entry */
				fdbe->isStatic = 1;
				fdbe->ageSec = L2_AGING_TIME;
				TBLFIELD(fdb_tbl, fdb_asic_set)(hash, way, fdbe);
			} break;
		}
	}
	if (!fdbe) {
		TBLFIELD(if_tbl, if_down)(ifname);
		return LR_FAILED;
	}
	memset(&pppoe, 0, sizeof(rtl865x_tblAsicDrv_pppoeParam_t));
	pppoe.age		= 300;
	pppoe.sessionId	= sessionId;
	TBLFIELD(pppoe_tbl, ppps[pppidx]) = ife;
	TBLFIELD(pppoe_tbl, pppoe_asic_set)(pppidx, &pppoe);
	return LR_SUCCESS;
}

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
enum LR_RESULT rtl865x_addPppSession(uint8 *ifname, ether_addr_t *mac, uint32 sessionId, enum SE_TYPE type)
{
	rtl865x_tblAsicDrv_l2Param_t *fdbe;
	struct if_entry *ife;
	enum LR_RESULT rc;
	uint32 way;

	if (type == SE_PPPOE)
		return rtl865x_addPppoeSession(ifname, mac, sessionId, SE_NONE);
	
	if (!(ife = TBLFIELD(if_tbl, if_lookup)(ifname, 0)))
		return LR_NOTFOUND;
	if (ife->if_type != IF_PPP)
		return LR_INVAPARAM;
	if ((rc = TBLFIELD(if_tbl, if_up)(ifname, 0, 0, 0, &pppMac)))
		return rc;
	fdbe = TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife), &pppMac, FDB_STATIC, &way);
	if (!fdbe) {
		rtl865x_addPppFdbEntry(GET_IF_VID(ife), 0x80 /* BIT(7) */, FDB_FWD);
	}
	LR_DEBUG("LR(addPppSession):  %s \n", ifname);
	return LR_SUCCESS;
}
#endif

enum LR_RESULT rtl865x_delPppoeSession( uint8* ifname )
{
	rtl865x_tblAsicDrv_routingParam_t rth;
	rtl865x_tblAsicDrv_pppoeParam_t pppoe;
	struct if_entry *ife, *ife1;
	uint32 idx, hash, way;

// fix code-dump issue when return null, david+2008-01-16
//	ife = TBLFIELD(if_tbl, if_lookup)(ifname, 0);
	if (!(ife = TBLFIELD(if_tbl, if_lookup)(ifname, 0)))
		return LR_NOTFOUND;
	
	if(ife->if_type != IF_PPPOE)
		return LR_INVAPARAM;
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++) {
		if (TBLFIELD(rt_tbl, rt_asic_get)(idx, &rth))
			continue;
		ife1 = TBLFIELD(pppoe_tbl, ppps[rth.pppoeIdx]);
		if (rth.process==0x00 && ife==ife1)
			return LR_INUSE;
	}
	memset(&pppoe, 0, sizeof(rtl865x_tblAsicDrv_pppoeParam_t));
	TBLFIELD(if_tbl, if_down)(ifname);

// fix code-dump issue when return null, david+2008-01-15	
//	TBLFIELD(pppoe_tbl, pppoe_lookup)(ife->sid_, &idx);
	if (TBLFIELD(pppoe_tbl, pppoe_lookup)(ife->sid_, &idx)) {
		TBLFIELD(pppoe_tbl, pppoe_asic_set)(idx, &pppoe);
		TBLFIELD(pppoe_tbl, ppps[idx]) = NULL;
	}
	//TBLFIELD(fdb_tbl, fdb_lookup)(ife->fid, &ife->pmac_, FDB_STATIC, &way);

// fix code-dump issue when return null, david+2008-01-15
//	TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife), &ife->pmac_, FDB_STATIC, &way);
	if (TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife), &ife->pmac_, FDB_STATIC, &way)) {
#ifdef CONFIG_RTL865XC
		hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(&ife->pmac_, GET_IF_VID(ife));
#else
		hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(&ife->pmac_);
#endif
		idx = (hash<<2) + way;
		if (--TBLFIELD(fdb_tbl, FDB)[idx] == 0)
			TBLFIELD(fdb_tbl, fdb_asic_del)(hash, way);
	}	
	return LR_SUCCESS;
}

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
enum LR_RESULT rtl865x_delPppSession(uint8 *ifname, enum SE_TYPE type)
{
	struct if_entry *ife;
	uint32 idx, hash, way;

	if (type == SE_PPPOE)
		return rtl865x_delPppoeSession(ifname);
		
	if (!(ife = TBLFIELD(if_tbl, if_lookup)(ifname, 0)))
		return LR_NOTFOUND;
	
	if (ife->if_type != IF_PPP)
		return LR_INVAPARAM;
	TBLFIELD(if_tbl, if_down)(ifname);

	if (TBLFIELD(fdb_tbl, fdb_lookup)(GET_IF_VID(ife), &pppMac, FDB_STATIC, &way)) {
#ifdef CONFIG_RTL865XC
		hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(&pppMac, GET_IF_VID(ife));
#else
		hash = TBLFIELD(fdb_tbl, fdb_asic_hash)(&pppMac);
#endif
		idx = (hash<<2) + way;
		if (--TBLFIELD(fdb_tbl, FDB)[idx] == 0)
			TBLFIELD(fdb_tbl, fdb_asic_del)(hash, way);
	}	
	LR_DEBUG("LR(delPppSession):  %s \n", ifname);
	return LR_SUCCESS;
}
#endif

enum LR_RESULT rtl865x_addNaptConnection( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                        ipaddr_t extIp, uint32 extPort,
                        ipaddr_t remIp, uint32 remPort,
                        enum NP_FLAGS flags )
{
	rtl865x_tblAsicDrv_extIntIpParam_t *natip;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;
	struct nat_entry *nat_in, *nat_out;
	struct nat_tuple nat_tuple;
	uint32 in, out, offset, ipidx, i;
	uint16 very,selEidx_out;


	/* Make sure natip */
//	TBLFIELD(natip_tbl, natip_add)(extIp);
	natip = TBLFIELD(natip_tbl, natip_lookup)(extIp, &ipidx);
	if (!natip)
		return LR_INVEXTIP;
	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip			= intIp;
	nat_tuple.int_host.port			= intPort;
	nat_tuple.ext_host.ip			= extIp;
	nat_tuple.ext_host.port			= extPort;
	nat_tuple.rem_host.ip			= remIp;
	nat_tuple.rem_host.port		= remPort;
	nat_tuple.proto				= protocol;
	nat_out = TBLFIELD(nat_tbl, nat_lookup)(&nat_tuple);
	if (nat_out)
		return LR_DUPENTRY;

	offset = (extPort&0x0000ffff)>>10;
	very = TBLFIELD(nat_tbl, nat_asic_hash)(((uint8)protocol) |2 , remIp, remPort, 0, 0);
	selEidx_out = extPort&0x3ff;
	in = TBLFIELD(nat_tbl, nat_asic_hash)(((uint8)protocol |2 ), remIp, remPort, extIp, extPort);
	out = TBLFIELD(nat_tbl, nat_asic_hash)((uint8)protocol, intIp, intPort, remIp, remPort);

	/* temporarily fix for Chariot FTPget/put issue,
	    skip the hardware NAT entry creation when inbound is equal to outbound */
	if (in == out)
		return LR_COLLISION;

	nat_in = &TBLFIELD(nat_tbl, nat_bucket)[in];
	nat_out = &TBLFIELD(nat_tbl, nat_bucket)[out];
	if (NAT_INUSE(nat_in) || NAT_INUSE(nat_out))
		return LR_COLLISION;

	LR_DEBUG("LR(%s):  %s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) g:(%u.%u.%u.%u:%u)\n",
			("add_nat"), ((protocol)? "tcp": "udp"), 
			NIPQUAD(intIp), intPort, NIPQUAD(remIp), remPort, NIPQUAD(extIp), extPort
	);
	
	memset(nat_out, 0, sizeof(struct nat_entry));
	memset(nat_in, 0, sizeof(struct nat_entry));
	*((struct nat_tuple *)nat_out)	= *((struct nat_tuple *)nat_in) = nat_tuple;
	nat_out->out					= nat_in->out = out;
	nat_out->in					= nat_in->in = in;
	nat_out->natip_idx			= nat_in->natip_idx = ipidx;
	SET_NAT_FLAGS(nat_out, NAT_OUTBOUND);
	SET_NAT_FLAGS(nat_in, NAT_INBOUND);
	
	for(i=0; i<2; i++) {
		memset(&asic_nat, 0, sizeof(asic_nat));
		asic_nat.insideLocalIpAddr	= intIp;
		asic_nat.insideLocalPort		= intPort;
		asic_nat.isCollision			= 0;
		asic_nat.isCollision2		= 0;
		asic_nat.isDedicated		= 0;
		asic_nat.isStatic			= 1;
		// asic_nat.isStatic			= (protocol==NP_TCP)? 0: 1;
		asic_nat.isTcp			= (protocol==NP_TCP)? 1: 0;
		asic_nat.isValid			= 1;
		asic_nat.offset			= ((i==0)?offset : (extPort & 0x3f));
		asic_nat.selEIdx			= ((i==0)?selEidx_out: very &0x3ff);
		asic_nat.selExtIPIdx		= ((i==0)?ipidx:((extPort & 0x3ff) >> 6));
		asic_nat.tcpFlag			= (((in!=out)? 0x2:0x0) | ((i==0)? 1: 0));
		asic_nat.priValid                  =0;
		asic_nat.priority                   =0;
		asic_nat.ageSec			= ((protocol==NP_TCP)? 
								   TBLFIELD(nat_tbl, tcp_timeout):
								   TBLFIELD(nat_tbl, udp_timeout));
		TBLFIELD(nat_tbl, nat_asic_set)(1, ((i==0)?out: in), &asic_nat);
	}
	return LR_SUCCESS;
}

enum LR_RESULT rtl865x_delNaptConnection( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
                        ipaddr_t extIp, uint32 extPort,
                        ipaddr_t remIp, uint32 remPort )
{
	struct nat_entry *nat_out, *nat_in;
	struct nat_tuple nat_tuple;

	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip			= intIp;
	nat_tuple.int_host.port			= intPort;
	nat_tuple.ext_host.ip			= extIp;
	nat_tuple.ext_host.port			= extPort;
	nat_tuple.rem_host.ip			= remIp;
	nat_tuple.rem_host.port		= remPort;
	nat_tuple.proto				= protocol;
	nat_out = TBLFIELD(nat_tbl, nat_lookup)(&nat_tuple);
	if (!nat_out)
		return LR_NOTFOUND;
    
	nat_in = &TBLFIELD(nat_tbl, nat_bucket)[nat_out->in];
	TBLFIELD(nat_tbl, nat_asic_del)(nat_out->in, nat_out->in);
	TBLFIELD(nat_tbl, nat_asic_del)(nat_out->out, nat_out->out);
	memset(nat_in, 0, sizeof(*nat_in));
	memset(nat_out, 0, sizeof(*nat_out));
	LR_DEBUG("LR(%s):  %s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) g:(%u.%u.%u.%u:%u)\n",
			("del_nat"), ((protocol)? "tcp": "udp"), 
			NIPQUAD(intIp), intPort, NIPQUAD(remIp), remPort, NIPQUAD(extIp), extPort
	);
	return LR_SUCCESS;
}




uint32 rtl865x_naptSync( enum NP_PROTOCOL protocol, ipaddr_t intIp, uint32 intPort,
			ipaddr_t extIp, uint32 extPort,
			ipaddr_t remIp, uint32 remPort, uint32 refresh )
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat1;
	struct nat_entry *nat_out;
	struct nat_tuple nat_tuple;
	int32 rc;

	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip			= intIp;
	nat_tuple.int_host.port			= intPort;
	nat_tuple.ext_host.ip			= extIp;
	nat_tuple.ext_host.port			= extPort;
	nat_tuple.rem_host.ip			= remIp;
	nat_tuple.rem_host.port		= remPort;
	nat_tuple.proto				= protocol;
	nat_out = TBLFIELD(nat_tbl, nat_lookup)(&nat_tuple);
	if (!nat_out)
		return 0;
	rc = TBLFIELD(nat_tbl, nat_asic_get)(nat_out->out, &asic_nat);
	assert(rc==SUCCESS);
	rc = TBLFIELD(nat_tbl, nat_asic_get)(nat_out->in, &asic_nat1);
	assert(rc==SUCCESS);
	return (asic_nat.ageSec>asic_nat1.ageSec)? asic_nat.ageSec: asic_nat1.ageSec;
}



enum LR_RESULT rtl865x_addNaptIp(ipaddr_t natip)
{
	if (TBLFIELD(natip_tbl, natip_add)(natip) < 0)
		return LR_FAILED;
	return LR_SUCCESS;
}


enum LR_RESULT rtl865x_delNaptIp(ipaddr_t natip)
{
	uint32 ipidx;
	if (!TBLFIELD(natip_tbl, natip_lookup)(natip, &ipidx))
		return LR_INVEXTIP;
	if (TBLFIELD(natip_tbl, natip_del)(natip) < 0)
		return LR_FAILED;
	TBLFIELD(nat_tbl, nat_init)();
	return LR_SUCCESS;
}


enum LR_RESULT rtl865x_addAclRule(uint32 vid, rtl865x_aclRule_t *rule_t, enum ACL_FLAGS flags) 
{
	_rtl8651_tblDrvAclRule_t rule;
	int32 rc;

	if (rule_t->actionType_ > 0x03)
		return LR_INVAPARAM;	    
	memset(&rule, 0, sizeof(_rtl8651_tblDrvAclRule_t));
	memcpy(&rule, rule_t, sizeof(rtl865x_aclRule_t));
	rc = TBLFIELD(acl_tbl, acl_add)(vid, &rule, flags);
	return rc;
}


enum LR_RESULT rtl865x_delAclRule(uint32 vid, rtl865x_aclRule_t *rule_t, enum ACL_FLAGS flags)
{
	_rtl8651_tblDrvAclRule_t rule;
	int32 rc;
	if (rule_t->actionType_ > 0x03)
		return LR_INVAPARAM;
	memset(&rule, 0, sizeof(_rtl8651_tblDrvAclRule_t));
	memcpy(&rule, rule_t, sizeof(rtl865x_aclRule_t));
	rc = TBLFIELD(acl_tbl, acl_del)(vid, &rule, flags);
	return rc;
}



enum LR_RESULT rtl865x_setAclDefaultAction(enum ACL_ACFLAGS defAction)
{
	if (defAction == TBLFIELD(acl_tbl, def_action))
		return LR_SUCCESS;
	TBLFIELD(acl_tbl, def_action) = defAction;
	TBLFIELD(acl_tbl, acl_arrange)();
	return LR_SUCCESS;
}

static struct rtl865x_lrConfig __lrconfig[] = {
/*  ifName  W/L  If type      VID   FID	  Member Port   UntagSet  mtu	MAC Addr                                     */
/*  ======  ===  ===========  ====  ====  ============	========  ====  ===========================================  */
 { 	"br0",  0,   IF_ETHER,    9,    1,    (ALL_PORTS & (~BIT(0))),  (ALL_PORTS & (~BIT(0))),     1500, { { 0x00, 0xe0, 0x4c, 0x86, 0x51, 0xd1 } } },
 {	"eth1", 1,   IF_ETHER,    8,    1,    BIT(0),         BIT(0),     1500, { { 0x00, 0xe0, 0x4c, 0x86, 0x51, 0xd2 } } },
#ifdef CONFIG_RTL865X_HW_PPTPL2TP
 {  "ppp0", 1,   IF_PPP,     10,    1,    EXT_PORT_PPTPL2TP,         EXT_PORT_PPTPL2TP,     1500, { { 0x00, 0xe0, 0x4c, 0x00, 0x00, 0x00 } } },
#else
 { 	"ppp0", 1,   IF_PPPOE,    8,    1,    BIT(0),         BIT(0),     1500, { { 0x00, 0xe0, 0x4c, 0x86, 0x51, 0xd2 } } },
#endif 

	LRCONFIG_END,
};

struct rtl865x_lrConfig	*lrConfig = __lrconfig;

enum LR_RESULT rtl865x_lightRomeConfig(void)
{
       struct rtl865x_lrConfig *lrconfig = __lrconfig;
	enum LR_RESULT rc;
	uint16 pvid;
	int32 i, j;

	if (!lrconfig[0].vid)
		return LR_INVVID;
	LR_INIT_CHECK(rtl8651_setAsicOperationLayer(3));
	
	/* 
	 * Move from rtl865x_lightRomeInit().
	 */
 	rtl8651_clearAsicAllTable();
	LR_INIT_CHECK( TBLFIELD(if_tbl, if_init)() );	
	LR_INIT_CHECK( TBLFIELD(nat_tbl, nat_init)() );	    
	LR_INIT_CHECK( TBLFIELD(natip_tbl, natip_init)() );
	LR_INIT_CHECK( TBLFIELD(rt_tbl, route_init)() );	
	LR_INIT_CHECK( TBLFIELD(arpt_tbl, arp_init)() );
	LR_INIT_CHECK( TBLFIELD(acl_tbl, acl_init)() );
	LR_INIT_CHECK( TBLFIELD(vlan_tbl, vlan_init)() );
	LR_INIT_CHECK( TBLFIELD(fdb_tbl, fdb_init)() );
	LR_INIT_CHECK( TBLFIELD(pppoe_tbl, pppoe_init)() );
	LR_INIT_CHECK( rtl8651_regLinkChangeCallBackFun(port_attr.link_change) );
	
	for(i=0; lrconfig[i].vid != 0; i++) {
		if (lrconfig[i].ifname[0] != '\0') {
			rc = (TBLFIELD(if_tbl, if_attach)(&lrconfig[i]));
			if (rc != LR_SUCCESS)
				return rc;
		}
		if (lrconfig[i].memPort != 0) {
			rc = TBLFIELD(vlan_tbl, vlan_create)(&lrconfig[i]);
			if (rc != LR_SUCCESS && rc != LR_DUPENTRY)
				return rc;
		}
	}
	
	// if ((++__LR_Config) == 1) {
		for(i=0; i<RTL8651_PORT_NUMBER + 3; i++) { /* Set each port's PVID */
			for(j=0,pvid=1; lrconfig[j].vid != 0; j++)
				if ( (1<<i) & lrconfig[j].memPort ) {
					pvid = lrconfig[j].vid;
					break;
				}
			if (lrconfig[j].vid==pvid)	
#ifdef CONFIG_RTL865XC
			LR_CONFIG_CHECK(port_attr.pvid_asic_set(i, pvid));
#else
			LR_CONFIG_CHECK(port_attr.pvid_asic_set(i, (pvid&0x07)));
#endif
		}
	// }

	return LR_SUCCESS;	
}

/* also define in asicregs.h */
#define GIGA_P5_PHYID	0x16

enum LR_RESULT rtl865x_lightRomeInit(void)
{	
#ifndef RTL865X_TEST
	lr_printf("    Initialize Hardware Tables Driver ......\n");
	__drvRx = NULL;
#endif
	__LR_Config = 0;
	memset(&lr_stats, 0, sizeof(struct lr_cpu_stats));

	{
		rtl8651_tblAsic_InitPara_t para;

		memset(&para, 0, sizeof(rtl8651_tblAsic_InitPara_t));

#ifdef CONFIG_RTL865XC
		/*
			For DEMO board layout, RTL865x platform define corresponding PHY setting and PHYID.
		*/
		para.externalPHYProperty |= RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B;
		para.externalPHYId[5] = GIGA_P5_PHYID;
#endif

		LR_INIT_CHECK( rtl8651_initAsic(&para) );
	}

	/* 
	 * Move to rtl865x_lightRomeConfig().
	 */
	/*
	LR_INIT_CHECK( TBLFIELD(if_tbl, if_init)() );	
	LR_INIT_CHECK( TBLFIELD(nat_tbl, nat_init)() );	    
	LR_INIT_CHECK( TBLFIELD(natip_tbl, natip_init)() );
	LR_INIT_CHECK( TBLFIELD(rt_tbl, route_init)() );	
	LR_INIT_CHECK( TBLFIELD(arpt_tbl, arp_init)() );
	LR_INIT_CHECK( TBLFIELD(acl_tbl, acl_init)() );
	LR_INIT_CHECK( TBLFIELD(vlan_tbl, vlan_init)() );
	LR_INIT_CHECK( TBLFIELD(fdb_tbl, fdb_init)() );
	LR_INIT_CHECK( TBLFIELD(pppoe_tbl, pppoe_init)() );
	LR_INIT_CHECK( rtl8651_regLinkChangeCallBackFun(port_attr.link_change) );
	*/

#ifndef CONFIG_RTL8196B
#ifndef RTL865X_TEST
#ifdef CONFIG_RTL865XC
	LR_INIT_CHECK(mBuf_init(1024,  0/* use external cluster pool */, 1024, PKT_BUF_SZ, 0)); 
#else
	LR_INIT_CHECK( mBuf_init(256,  0/* use external cluster pool */, 256, PKT_BUF_SZ, 0) ); 
#endif
#endif
#endif
	return LR_SUCCESS;
}

#ifdef CONFIG_RTL865XC
/*
@func uint32 | rtl8651_vlanTableIndex | Get VLAN table index for given VID
@parm uint16 | vid | VLAN ID.
@rvalue Index | Index in Rome Driver VLAN table for given VID.
@comm
Lookup utility. Use VLAN ID to find the corresponding VLAN table index in Rome Driver.
This API would return RTL865XC_NETINTERFACE_NUMBER if the VID does not exist in Rome Driver VLAN table.
*/
uint32 rtl8651_vlanTableIndex(uint16 vid)
{
	uint32 idx;
	struct vlan_entry *ventry;


	for ( idx = 0, ventry = &TBLFIELD(vlan_tbl, vhash)[0]; idx < RTL865XC_NETINTERFACE_NUMBER; idx++, ventry++ )
	{
		if ( ventry->valid == 1 && ventry->vid == vid )
			break;
	}

	return idx;
}

enum LR_RESULT rtl865x_rxPktProcess(void *pkt)
{
	struct rtl_pktHdr *m_pkthdr = (struct rtl_pktHdr *)pkt;
	uint32 rxPort = m_pkthdr->ph_portlist;
	uint32 idx;
	uint32 found = 0; 
	struct vlan_entry *ventry;
	for (idx = 0; idx < RTL865XC_VLAN_NUMBER; idx++)
	{
		ventry = &TBLFIELD(vlan_tbl, vhash)[idx];
		if (ventry->valid == 0 )
			continue;
		if (ventry->mbr & (1 << rxPort))
		{
			found = 1;
			m_pkthdr->ph_vlanIdx = idx;
			break;
		}			
	}
	if (found == 0)
	{
		return LR_FAILED;
	}
	return LR_SUCCESS;
}

/************ copy from rtl8651_layer2fwd.c: rtl8651_rxPktPreprocess() **************/
/* <---------------- RX-preProcess / TX-postProcess Functions ----------------> */
/*
ph_portlist:
Rx: source port number
0-5: port 0-Port 5, 6: Reserved, 7: CPU
(If pkt was sent by CPU/extension port (ex: from extension port),
ph_portlist=7. and need to refer ph_srcExtPortNum)

ph_srcExtPortNum:
Source extension port number
0: from CPU,
1~3: from extension port numbers
 */
#define RTL8651_CPU_PORT                0x07 /* in rtl8651_tblDrv.h */
extern uint8 extPortMaskToPortNum[];

inline enum LR_RESULT rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid)
{
	struct rtl_pktHdr *m_pkthdr = (struct rtl_pktHdr *)pkt;
	uint32 srcPortNum;
	//uint32 idx;
	//uint32 found = 0; 
	//struct vlan_entry *ventry;

       srcPortNum = m_pkthdr->ph_portlist;
	if (srcPortNum >= RTL8651_CPU_PORT)
	{        
		if (m_pkthdr->ph_extPortList == 0)
		{
			/* No any destination ( extension port or CPU) : ASIC's BUG */
			return LR_FAILED;
		}else if ((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
		{
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList]+RTL8651_PORT_NUMBER-1;
			m_pkthdr->ph_portlist = srcPortNum;

		}else
		{
			/* has CPU bit, pkt is original pkt from port 6~8 */
			srcPortNum = m_pkthdr->ph_srcExtPortNum + RTL8651_PORT_NUMBER - 1;
			m_pkthdr->ph_portlist = srcPortNum;
		}        
	}
       else
	{
		/* otherwise, pkt is rcvd from PHY */
		m_pkthdr->ph_srcExtPortNum = 0;
		if((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
		{	/* No CPU bit, only dest ext mbr port... */
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			if(m_pkthdr->ph_extPortList)
			{
				/* redefine src port number */
				srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList] + RTL8651_PORT_NUMBER - 1;
				m_pkthdr->ph_portlist = srcPortNum;
			}
		}
	}		

	return LR_SUCCESS;
}

#endif	/* CONFIG_RTL865XC */

