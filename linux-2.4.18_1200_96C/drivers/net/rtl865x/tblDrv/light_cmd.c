
#include <linux/string.h>
#include <linux/autoconf.h>
#include "rtl_types.h"
#include "types.h"
#include "rtl_cle.h" 
#include "cle_utility.h"
#include "assert.h"
#include "asicregs.h"
#include "mbuf.h"
#define RTL8651_TBLDRV_LOCAL_H
#include "rtl8651_aclLocal.h"
#ifdef CONFIG_RTL865XC
#include "rtl865xC_tblAsicDrv.h"
#include "rtl865xC_hs.h"
#else
#include "rtl8651_tblAsicDrv.h"
#include "hs.h"
#endif
#include "rtl865x_lightrome.h"
#include "light_rome.h"

extern int32 rtl8651_totalExtPortNum;

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#ifdef CONFIG_RTL865XC

void rtl865xC_dump_l2(void)
{
		rtl865x_tblAsicDrv_l2Param_t asic_l2;
 		uint32 row, col, port, m=0;

		rtlglue_printf(">>ASIC L2 Table:\n");

		for(row=0x0; row<RTL8651_L2TBL_ROW; row++)
		{
			for(col=0; col<RTL8651_L2TBL_COLUMN; col++)
			{
				memset((void*)&asic_l2, 0, sizeof(asic_l2));
				if (rtl8651_getAsicL2Table(row, col, &asic_l2) == FAILED)
				{
					continue;
				}

				if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0 &&asic_l2.auth==0)
				{
					continue;
				}

				rtlglue_printf("%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, row, col, 
						asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], 
						asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5],asic_l2.fid
				);

				m++;

				for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
				{
					if (asic_l2.memberPortMask & (1<<port))
					{
						rtlglue_printf("%d ", port);
					}
				}

				rtlglue_printf(")");
				rtlglue_printf("%s %s %s %s age:%d ",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"", asic_l2.nhFlag?"NH":"", asic_l2.ageSec);

	/*			if (	(0 == (asic_l2.macAddr.octet[0]&1)) &&
					(asic_l2.memberPortMask&peripheralExtPortMask))
				{
					rtl8651_tblDrv_filterDbTableEntry_t * entry;
					uint8 queryMac[6];
					int32 vid;

					memcpy(queryMac, &asic_l2.macAddr, 6);
					queryMac[5]=row^asic_l2.macAddr.octet[0]^asic_l2.macAddr.octet[1]^asic_l2.macAddr.octet[2]^asic_l2.macAddr.octet[3]^asic_l2.macAddr.octet[4];

					for ( vid = 1 ; vid < 4095 ; vid ++ )
					{
						entry=_rtl8651_getVlanFilterDatabaseEntry(vid, (ether_addr_t *)queryMac) ;

						if (	(entry) &&
							(entry->linkId))
						{
							rtlglue_printf("W:%d[VID %d]",entry->linkId, vid );
							break;
						}
					}
				}*/
				if (asic_l2.auth)
				{
					rtlglue_printf("AUTH:%d",asic_l2.auth);
				}
				rtlglue_printf("\n");
			}
		}

}

void rtl865xC_dump_netif(void)
{
	int8	*pst[] = { "DIS/BLK",  "LIS", "LRN", "FWD" };
	uint8 *mac;
	int32 i, j;

	rtlglue_printf(">>ASIC Netif Table:\n\n");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_intfParam_t intf;
		rtl865x_tblAsicDrv_vlanParam_t vlan;

		if ( rtl8651_getAsicNetInterface( i, &intf ) == FAILED )
			continue;

		if ( intf.valid )
		{
			mac = (uint8 *)&intf.macAddr.octet[0];
			rtlglue_printf("[%d]  VID[%d] %x:%x:%x:%x:%x:%x", 
				i, intf.vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			rtlglue_printf("  Routing %s \n",
				intf.enableRoute==TRUE? "enabled": "disabled" );

			rtlglue_printf("      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					rtlglue_printf("permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					rtlglue_printf("all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					rtlglue_printf("drop all,");
			}
			else
				rtlglue_printf("ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			rtlglue_printf("  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					rtlglue_printf("permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					rtlglue_printf("all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					rtlglue_printf("drop all,");
			}
			else
				rtlglue_printf("ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);

			rtlglue_printf("\n      %d MAC Addresses, MTU %d Bytes\n", intf.macAddrNumber, intf.mtu);

			rtl8651_getAsicVlan( intf.vid, &vlan );

			rtlglue_printf("\n      Untag member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.untagPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}
			rtlglue_printf("\n      Active member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}
			
			rtlglue_printf("\n      Port state(");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( ( vlan.memberPortMask & ( 1 << j ) ) == 0 )
					continue;
				if ((( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET ) > 4 )
					rtlglue_printf("--- ");
				else
					rtlglue_printf("%d:%s ", j, pst[(( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET )]);

			}
			rtlglue_printf(")\n\n");
		}

	}
}

void rtl865xC_dump_l3(void)
{
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	int8 *strNetType[] = { "WAN", "DMZ", "LAN",  "RLAN"};
	uint32 idx, mask;
	int netIdx;

	rtlglue_printf(">>L3 Routing Table:\n");
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++) {
		if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED) {
			rtlglue_printf("\t[%d]  (Invalid)\n", idx);
			continue;
		}
		if (idx == RTL8651_ROUTINGTBL_SIZE-1)
			mask = 0;
		else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
				mask--;
		netIdx = asic_l3.internal<<1|asic_l3.DMZFlag;
		rtlglue_printf("\t[%d]  %d.%d.%d.%d/%d process(%s) %s \n", idx, (asic_l3.ipAddr>>24),
			((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff), 
			mask, str[asic_l3.process],strNetType[netIdx]
		);
		switch(asic_l3.process) 
		{
		case 0x00:	/* PPPoE */
			rtlglue_printf("\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;
			
		case 0x01:	/* L2 */
			rtlglue_printf("              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x02:	/* ARP */
			rtlglue_printf("             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart, asic_l3.arpEnd, asic_l3.arpIpIdx);
			break;

		case 0x03:	/* Reserved */
			assert(0);

		case 0x04:	/* CPU */
			rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x05:	/* NAPT Next Hop */
			rtlglue_printf("              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
				asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
			break;

		case 0x06:	/* DROP */
			rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x07:	/* Reserved */
			/* pass through */
		default: assert(0);
		}
	}

}



void rtl865xC_dump_acl(void)
{
		int8 *actionT[] = { "", "permit", "drop", "cpu", "drop log", 
						"drop notify", 	"redirect to ethernet","redirect to pppoe", "mirror", "mirro keep match", 
						"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","policy ",
						"priority selection"};
		_rtl8651_tblDrvAclRule_t asic_acl;
		rtl865x_tblAsicDrv_intfParam_t asic_intf;
		uint32 start, end;

		uint16 vid;
		int8 outRule;


				
		rtlglue_printf(">>ASIC ACL Table:\n\n");
		for(vid=0; vid<8; vid++ ) {
			/* Read VLAN Table */
			if (rtl8651_getAsicNetInterface(vid, &asic_intf) == FAILED)
				continue;
			if (asic_intf.valid==FALSE)
				continue;

			outRule = FALSE;
			start = asic_intf.inAclStart; end = asic_intf.inAclEnd;
	again:
			if (outRule == FALSE)
				rtlglue_printf("\n<<Ingress Rule for Netif  %d: (VID %d)>>\n", vid,asic_intf.vid);
			else rtlglue_printf("\n<<Egress Rule for Netif %d (VID %d)>>:\n", vid,asic_intf.vid);
			for( ; start<=end; start++) {
				if (rtl8651_getAsicAclRule(start, &asic_acl) == FAILED)
					assert(0);
				switch(asic_acl.ruleType_)
				{
				case RTL8651_ACL_MAC:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Ethernet", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
					rtlglue_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
					);
					rtlglue_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
					break;

				case RTL8651_ACL_IP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "IP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);
					rtlglue_printf("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
					);
					rtlglue_printf("\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;			
				case RTL8652_ACL_IP_RANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "IP Range", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);
					rtlglue_printf("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
					);
					rtlglue_printf("\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;			
				case RTL8651_ACL_ICMP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "ICMP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
					break;


				case RTL8652_ACL_ICMP_IPRANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
					break;


				case RTL8651_ACL_IGMP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "IGMP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
					);
					break;


				case RTL8652_ACL_IGMP_IPRANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
					);
					break;

				case RTL8651_ACL_TCP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "TCP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
					);
					rtlglue_printf("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
					break;

				case RTL8652_ACL_TCP_IPRANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
					);
					rtlglue_printf("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
					break;

				case RTL8651_ACL_UDP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start,"UDP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
					break;				


				case RTL8652_ACL_UDP_IPRANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
					break;				

				case RTL8651_ACL_IFSEL:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "UDP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tgidxSel: %x\n", asic_acl.gidxSel_);
					break;
				case RTL8651_ACL_SRCFILTER:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Source Filter", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
					);
					rtlglue_printf("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
					break;


				case RTL8652_ACL_SRCFILTER_IPRANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
					);
					rtlglue_printf("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
					);
					rtlglue_printf("\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
					break;

				case RTL8651_ACL_DSTFILTER:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Deatination Filter", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
					);
					rtlglue_printf("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
					);
					rtlglue_printf("\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
					break;
				case RTL8652_ACL_DSTFILTER_IPRANGE:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
					);
					rtlglue_printf("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
					);
					rtlglue_printf("\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
					break;


				default: assert(0);

				}


				/* Action type */
			switch (asic_acl.actionType_) {

			case RTL8651_ACL_PERMIT: /* 0x00 */
			case RTL8651_ACL_REDIRECT: /* 0x01 */
			case RTL8651_ACL_CPU: /* 0x03 */
			case RTL8651_ACL_DROP: /* 0x02, 0x04 */
			case RTL8651_ACL_DROP_LOG: /* 0x05 */
			case RTL8651_ACL_MIRROR: /* 0x06 */
			case RTL8651_ACL_REDIRECT_PPPOE: /* 0x07 */
			case RTL8651_ACL_MIRROR_KEEP_MATCH: /* 0x09 */
				rtlglue_printf("\tdvidx: %d   hp: %d   pppoeIdx: %d   nxtHop:%d  ", asic_acl.dvid_, asic_acl.priority_,
						asic_acl.pppoeIdx_, asic_acl.nextHop_);
				break;

			case RTL8651_ACL_POLICY: /* 0x08 */
				rtlglue_printf("\thp: %d   nxtHopIdx: %d  ", asic_acl.priority_, asic_acl.nhIndex);
				break;

			case RTL8651_ACL_PRIORITY: /* 0x08 */
				rtlglue_printf("\tprioirty: %d   ", asic_acl.priority) ;
				break;

			case RTL8651_ACL_DROP_RATE_EXCEED_PPS: /* 0x0a */
			case RTL8651_ACL_LOG_RATE_EXCEED_PPS: /* 0x0b */
			case RTL8651_ACL_DROP_RATE_EXCEED_BPS: /* 0x0c */
			case RTL8651_ACL_LOG_RATE_EXCEED_BPS: /* 0x0d */
				rtlglue_printf("\trlIdx: %d  ", asic_acl.rlIndex);
				break;
			default: assert(0);
			
			}
			rtlglue_printf("pktOpApp: %d\n", asic_acl.pktOpApp);
			
#if 0
				rtlglue_printf("\tpkgopt: %x\n", asic_acl.pktOpApp);
				if (asic_acl.actionType_ < 8 || asic_acl.actionType_ == 9) {
					rtlglue_printf("\tdvidx: %d   hp: %d   pppoeIdx: %d   nxtHop:%d\n", asic_acl.dvid_, asic_acl.priority_,
						asic_acl.pppoeIdx_, asic_acl.nextHop_
					);
				} else if (asic_acl.actionType_ == 8) {
					rtlglue_printf("\thp: %d   nxtHopIdx: %d\n", asic_acl.priority_, asic_acl.nhIndex);
				} else {
					rtlglue_printf("\trlIdx: %d\n", asic_acl.rlIndex);
				}
#endif
			}
			if (outRule == FALSE) {
				start = asic_intf.outAclStart; end = asic_intf.outAclEnd;
				outRule = TRUE;
				goto again;
			}

	}
}

#endif	/* CINFIG_RTL865XC */

void rtl865xC_dump_icmp(void)
{
	uint32 i;
	rtl865x_tblAsicDrv_naptIcmpParam_t naptIcmp;
	uint32 total = 0;
	rtlglue_printf(">>ICMP Table:\n");
	for(i=0; i<32; i++) {
		if (rtl8651_getAsicNaptIcmpTable(i, &naptIcmp) == FAILED)
			continue;

		if (naptIcmp.isValid ||naptIcmp.isStatic) {
			rtlglue_printf("[%02d]GId(%04x) %s Local %d.%d.%d.%d-(ID) %04x %s %s %s %s %s Age:%d\n",
					i, naptIcmp.offset, naptIcmp.isPptp?"PPTP": "ICMP",
					(naptIcmp.insideLocalIpAddr>>24)&0xff, (naptIcmp.insideLocalIpAddr>>16)&0xff, (naptIcmp.insideLocalIpAddr>>8)&0xff, naptIcmp.insideLocalIpAddr&0xff,
					naptIcmp.insideLocalId, 
					naptIcmp.direction==0?"B":naptIcmp.direction==1?"O":naptIcmp.direction==2?"I":"B",
					naptIcmp.isCollision? "Col":"",
					naptIcmp.isStatic? "STA": "", 
					naptIcmp.isSpi? "SPI": "", 
					naptIcmp.isValid? "VAL":"",
					naptIcmp.ageSec
					);
			total++;
		}
	}
	rtlglue_printf("Total entry: %u\n", total);

}


static int32 _rtl865x_dumpCmds(uint32 userId,  int32 argc,int8 **saved)
{
	int32 size;
	int8 *nextToken;
	uint32 tblType=0;	/* 1: ASIc Table, 2: Software Table */

	
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if (strcmp(nextToken, "asic") == 0)
		tblType = 1;
	if (strcmp(nextToken, "lrome") == 0)
		tblType = 2;
	
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if (strcmp(nextToken, "ifdev") == 0) {
		uint32 ifindex, ip1, ip2;
		extern struct if_table if_tbl;
		extern struct vlan_table vlan_tbl;
		
		rtlglue_printf(">>Layer3 Interface:\n");
		if (tblType == 1) {
			rtlglue_printf("ASIC has no layer-3 interface\n\n");
			return SUCCESS;
		}
		for (ifindex=0; ifindex<NETIF_NUMBER; ifindex++) {
			if (if_tbl.if_hash[ifindex].name[0] == '\0')
				continue;
                     if (if_tbl.if_hash[ifindex].if_type == IF_PPPOE) {
			    rtlglue_printf("%s\tvid: %d  HWaddr: %x:%x:%x:%x:%x:%x  MTU: %d, sid: %d\n", if_tbl.if_hash[ifindex].name, if_tbl.if_hash[ifindex].vid,
					 if_tbl.if_hash[ifindex].pmac_.octet[0],  if_tbl.if_hash[ifindex].pmac_.octet[1], 
					 if_tbl.if_hash[ifindex].pmac_.octet[2],  if_tbl.if_hash[ifindex].pmac_.octet[3], 
					 if_tbl.if_hash[ifindex].pmac_.octet[4],  if_tbl.if_hash[ifindex].pmac_.octet[5],
					if_tbl.if_hash[ifindex].mtu, if_tbl.if_hash[ifindex].sid_);
                        
                     } else {
			    rtlglue_printf("%s\tvid: %d  HWaddr: %x:%x:%x:%x:%x:%x  MTU: %d\n", if_tbl.if_hash[ifindex].name, if_tbl.if_hash[ifindex].vid,
					vlan_tbl.vhash[ifindex].mac.octet[0], vlan_tbl.vhash[ifindex].mac.octet[1], 
					vlan_tbl.vhash[ifindex].mac.octet[2], vlan_tbl.vhash[ifindex].mac.octet[3], 
					vlan_tbl.vhash[ifindex].mac.octet[4], vlan_tbl.vhash[ifindex].mac.octet[5],
					if_tbl.if_hash[ifindex].mtu);

			    if (if_tbl.if_hash[ifindex].ipaddr_ == 0)
				    continue;
			    ip1 = if_tbl.if_hash[ifindex].ipaddr_;
			    ip2 = if_tbl.if_hash[ifindex].mask_;
			    rtlglue_printf("			inet addr: %d.%d.%d.%d   Mask: %d.%d.%d.%d\n", (ip1&0xff000000)>>24, (ip1&0x00ff0000)>>16,
					(ip1&0x0000ff00)>>8, (ip1&0xff), (ip2&0xff000000)>>24, (ip2&0x00ff0000)>>16,  (ip2&0x0000ff00)>>8, (ip2&0xff));
                     }
		}	
	}
	else if (strcmp(nextToken, "nat") == 0) {
		/* Dump NAT Table */
		rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_tcpudp;
		int32 idx, entry=0;

		if (tblType == 1) {
			rtlglue_printf(">>ASIC NAPT TCP/UDP Table:\n");
			
			for(idx=0; idx<RTL8651_TCPUDPTBL_SIZE; idx++) {
				if (rtl8651_getAsicNaptTcpUdpTable(idx, &asic_tcpudp) == FAILED)
					continue;

				if (asic_tcpudp.isValid == 1 || asic_tcpudp.isDedicated == 1 ) {
					rtlglue_printf("[%4d] %d.%d.%d.%d:%d {V,D}={%d,%d} col1(%d) col2(%d) static(%d) tcp(%d)\n",
					       idx,
					       asic_tcpudp.insideLocalIpAddr>>24, (asic_tcpudp.insideLocalIpAddr&0x00ff0000) >> 16,
					       (asic_tcpudp.insideLocalIpAddr&0x0000ff00)>>8, asic_tcpudp.insideLocalIpAddr&0x000000ff,
					       asic_tcpudp.insideLocalPort, 
					       asic_tcpudp.isValid, asic_tcpudp.isDedicated,
					       asic_tcpudp.isCollision, asic_tcpudp.isCollision2, asic_tcpudp.isStatic, asic_tcpudp.isTcp );

					rtlglue_printf("         age(%d) offset(%d) tcpflag(%d) SelEIdx(%d) SelIPIdx(%d)  priValid:%d pri(%d)\n",
					        asic_tcpudp.ageSec, asic_tcpudp.offset<<10, asic_tcpudp.tcpFlag, 
					        asic_tcpudp.selEIdx, asic_tcpudp.selExtIPIdx,asic_tcpudp.priValid,asic_tcpudp.priority );
					entry++;
				}
			}
			rtlglue_printf("Total entry: %d\n\n", entry);

		}
		else {
			extern struct nat_table nat_tbl;
			uint8 mask[1024];
			
			rtlglue_printf(">>Software NAPT TCP/UDP Table:\n");

			memset(mask, 0, 1024);
			for(entry=idx=0; idx<RTL8651_TCPUDPTBL_SIZE; idx++) {
				if (!NAT_INUSE(&TBLFIELD(nat_tbl, nat_bucket)[idx]))
					continue;
				if (mask[idx])
					continue;
				mask[TBLFIELD(nat_tbl, nat_bucket)[idx].in] = 1;
				mask[TBLFIELD(nat_tbl, nat_bucket)[idx].out] = 1;
				rtlglue_printf("[%d]: %s (%u.%u.%u.%u:%u->%u.%u.%u.%u:%u) g:(%u.%u.%u.%u:%u)\n", entry,
					(TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.proto==1)? "tcp": "udp",
					NIPQUAD(TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.int_host.ip),
					TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.int_host.port,
					NIPQUAD(TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.rem_host.ip),
					TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.rem_host.port,
					NIPQUAD(TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.ext_host.ip),
					TBLFIELD(nat_tbl, nat_bucket)[idx].tuple_info.ext_host.port);
				entry ++;
			}
			rtlglue_printf("\nTotal entry: %d\n\n", entry);
		}
		return SUCCESS;
	}
	else if (strcmp(nextToken, "ip") == 0) {
		rtl865x_tblAsicDrv_extIntIpParam_t asic_ip;
		uint32 idx;

		rtlglue_printf(">>ASIC IP Table:\n");
		for(idx=0; idx<RTL8651_IPTABLE_SIZE; idx++) {
			if (rtl8651_getAsicExtIntIpTable(idx, &asic_ip) == FAILED) {
				rtlglue_printf("\t[%d]  (Invalid)\n", idx);
				continue;
			}
			rtlglue_printf("\t[%d]  intip(%d.%d.%d.%d) extip(%d.%d.%d.%d) type(%s) nhIdx(%d)\n", idx, 
				(asic_ip.intIpAddr>>24), ((asic_ip.intIpAddr&0x00ff0000)>>16), ((asic_ip.intIpAddr&0x0000ff00)>>8), (asic_ip.intIpAddr&0xff),
				(asic_ip.extIpAddr>>24), ((asic_ip.extIpAddr&0x00ff0000)>>16), ((asic_ip.extIpAddr&0x0000ff00)>>8), (asic_ip.extIpAddr&0xff),
				(asic_ip.localPublic==TRUE? "lp": (asic_ip.nat==TRUE? "nat": "napt")), asic_ip.nhIndex);
		}
		rtlglue_printf("\n");
	}
#ifdef CONFIG_RTL865XB
	else if (strcmp(nextToken, "route") == 0) {
		/* Dump Routing Table */
		rtl865x_tblAsicDrv_routingParam_t asic_l3;

		int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
		uint32 idx, mask;

		rtlglue_printf(">>L3 Routing Table:\n");
		for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++) {
			if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED) {
				rtlglue_printf("\t[%d]  (Invalid)\n", idx);
				continue;
			}
			if (idx == RTL8651_ROUTINGTBL_SIZE-1)
				mask = 0;
			else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
					mask--;
			rtlglue_printf("\t[%d]  %d.%d.%d.%d/%d process(%s)\n", idx, (asic_l3.ipAddr>>24),
				((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff), 
				mask, str[asic_l3.process]
			);
			switch(asic_l3.process) 
			{
			case 0x00:	/* PPPoE */
				rtlglue_printf("\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
				break;
				
			case 0x01:	/* L2 */
				rtlglue_printf("              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
				break;

			case 0x02:	/* ARP */
				rtlglue_printf("             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart, asic_l3.arpEnd, asic_l3.arpIpIdx);
				break;

			case 0x03:	/* Reserved */
				assert(0);

			case 0x04:	/* CPU */
				rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
				break;

			case 0x05:	/* NAPT Next Hop */
				rtlglue_printf("              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
					asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
				break;

			case 0x06:	/* DROP */
				rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
				break;

			case 0x07:	/* Reserved */
				/* pass through */
			default: assert(0);
			}
		}
	}
#endif
	else if (strcmp(nextToken, "arp") == 0) {
		/* Dump ARP Table */
		rtl865x_tblAsicDrv_routingParam_t asic_l3;
		rtl865x_tblAsicDrv_arpParam_t asic_arp;
		rtl865x_tblAsicDrv_l2Param_t asic_l2;
		uint32	i, j, port;
		ipaddr_t ipAddr;
		int8 ipBuf[sizeof"255.255.255.255"];

		
		rtlglue_printf(">>Arp Table:\n");
		for(i=0; i<RTL8651_ARPTBL_SIZE; i++) {
			if (rtl8651_getAsicArp(i,  &asic_arp) == FAILED)
				continue;
			for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++) {
				if (rtl8651_getAsicRouting(j, &asic_l3) == FAILED || asic_l3.process!= 2)
					continue;
				if(asic_l3.arpStart <= (i>>3) &&  (i>>3) <= asic_l3.arpEnd) {
					ipAddr = (asic_l3.ipAddr & asic_l3.ipMask) + (i - (asic_l3.arpStart<<3));
					if(rtl8651_getAsicL2Table(asic_arp.nextHopRow, asic_arp.nextHopColumn, &asic_l2) == FAILED){
						inet_ntoa_r(ipAddr, ipBuf);
						rtlglue_printf("%-16s [%3d,%d] ", ipBuf, asic_arp.nextHopRow, asic_arp.nextHopColumn);
					}else {
						inet_ntoa_r(ipAddr, ipBuf);
						rtlglue_printf("%-16s %02x-%02x-%02x-%02x-%02x-%02x (", ipBuf, asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5]);
						for(port=0; port< RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; port++){
							if(asic_l2.memberPortMask& (1<<port))
								rtlglue_printf("%d ", port);
							else
								rtlglue_printf("  ");
						}							
						rtlglue_printf(") %us", asic_l2.ageSec);
					}
					continue;
				}
			}
			rtlglue_printf(" ARP:%3d  L2:%3d,%d\n", i, asic_arp.nextHopRow, asic_arp.nextHopColumn);
		}
	}
#ifdef CONFIG_RTL865XC
	else if (strcmp(nextToken, "vlan") == 0) {

		int i, j;

		rtlglue_printf(">>ASIC VLAN Table:\n\n");
		for ( i = 0; i < RTL865XC_VLAN_NUMBER; i++ )
		{
			rtl865x_tblAsicDrv_vlanParam_t vlan;

			if ( rtl8651_getAsicVlan( i, &vlan ) == FAILED )
				continue;
			
			rtlglue_printf("  VID[%d] ", i);
			rtlglue_printf("\n\tmember ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}

			rtlglue_printf("\n\tUntag member ports:");				

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if( vlan.untagPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}

			rtlglue_printf("\n\tFID:\t%d\n",vlan.fid);
		}
	}

#endif	/* CONFIG_RTL865XC */
#ifdef CONFIG_RTL865XB
	else if (strcmp(nextToken, "vlan") == 0) {
		/* Dump ASIC VLAN Table */
		uint8	*pst[] = { "DIS", "BLK", "LIS", "LRN", "FWD" };
		uint8	*mac;
		int32	i, j;
		extern struct port_attribute port_attr;

		rtlglue_printf(">>ASIC Vlan Table:\n\n");
		for(i=1; i<4096; i++) {
			rtl865x_tblAsicDrv_vlanParam_t vlan;

			if (rtl8651_getAsicVlan( i, &vlan ) == FAILED)
				continue;

			if(vlan.promiscuous)
				rtlglue_printf("  VID[%d] Promiscuous Mode\n", i);
			else {
				mac = (uint8 *)&vlan.macAddr.octet[0];
				rtlglue_printf("  VID[%d] %x:%x:%x:%x:%x:%x", 
					i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				rtlglue_printf("  %s, L3/4 HW acc %s %s\n",
					vlan.internal==TRUE? "Internal": "External", vlan.enableRoute==TRUE? "enabled": "disabled",
					vlan.broadcastToCpu==TRUE?",L2 bcast to SW": "");
				rtlglue_printf("      ingress ");
				if(RTL8651_ACLTBL_PERMIT_ALL<=vlan.inAclStart){
					if(vlan.inAclStart==RTL8651_ACLTBL_PERMIT_ALL)
						rtlglue_printf("permit all,");
					if(vlan.inAclStart==RTL8651_ACLTBL_ALL_TO_CPU)
						rtlglue_printf("all to cpu,");
					if(vlan.inAclStart==RTL8651_ACLTBL_DROP_ALL)
						rtlglue_printf("drop all,");
				}else
					rtlglue_printf("ACL %d-%d, ",vlan.inAclStart, vlan.inAclEnd);
				rtlglue_printf("  egress ");
				if(RTL8651_ACLTBL_PERMIT_ALL<=vlan.outAclStart){
					if(vlan.outAclStart==RTL8651_ACLTBL_PERMIT_ALL)
						rtlglue_printf("permit all,");
					if(vlan.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU)
						rtlglue_printf("all to cpu,");
					if(vlan.outAclStart==RTL8651_ACLTBL_DROP_ALL)
						rtlglue_printf("drop all,");
				}else
					rtlglue_printf("ACL %d-%d, ",vlan.outAclStart, vlan.outAclEnd);
				rtlglue_printf("\n      Untag member ports:");
				for(j=0; j<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; j++)
					if(vlan.untagPortMask & (1<<j))
						rtlglue_printf("%d ", j);
				rtlglue_printf("\n      VLAN member ports:");
				for(j=0; j<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; j++)
					if(vlan.memberPortMask & (1<<j))
						rtlglue_printf("%d ", j);
				rtlglue_printf("\n      Active member ports:");
				for(j=0; j<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; j++)
					if((vlan.memberPortMask&port_attr.activePortMask) & (1<<j))
						rtlglue_printf("%d ", j);
				
				rtlglue_printf("\n      %d MAC Addresses, MTU %d Bytes", vlan.macAddrNumber, vlan.mtu);
				#ifdef CONFIG_RTL865XB_ENRT		/* refer to _rtl8651_addVlan() for the meaning of this compile flag */
				rtlglue_printf(", macNonExist: %d", vlan.macNonExist);
				#endif
				rtlglue_printf("\n");
				rtlglue_printf("      Port state(");
				for(j=0; j<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; j++)
				{
					if((vlan.memberPortMask & (1<<j))==0)
						continue;
					if ( vlan.portState[j] > 4 )
						rtlglue_printf("--- ");
					else
						rtlglue_printf("%d:%s ", j, pst[(int32)vlan.portState[j]]);
				}
				rtlglue_printf(")\n\n");
			}
		}
	}
	else if (strcmp(nextToken, "l2") ==0) {
		/* Dump MAC Address Table */
		rtl865x_tblAsicDrv_l2Param_t asic_l2;
 		uint32 row, col, port, m=0;
  			
		rtlglue_printf(">>ASIC L2 Table:\n");
		for(row=0; row<RTL8651_L2TBL_ROW; row++) {
			for(col=0; col<RTL8651_L2TBL_COLUMN; col++) {
				if (rtl8651_getAsicL2Table(row, col, &asic_l2) == FAILED)
					continue;
				if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0)
					continue;
				rtlglue_printf("%4d.[%3d,%d] %2x:%2x:%2x:%2x:%2x:%2x mbr(",m, row, col, 
						asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], 
						asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5]
				);
				m++;
				for(port=0; port<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; port++){
					if(asic_l2.memberPortMask & (1<<port))
						rtlglue_printf("%d ", port);
					else 
						rtlglue_printf("  ");
				}
				rtlglue_printf(")");
				rtlglue_printf("%s %s %s %s age:%d \n",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"   ", asic_l2.nhFlag?"NH":"  ", asic_l2.ageSec);
			}
		}
	}
	else if (strcmp(nextToken, "acl") == 0) {
		//int8 *actionT[] = { "", "permit", "drop", "cpu", "drop and log", "redirect to ethernet", "redirect to pppoe" };
		int8 *actionT[] = { "", "permit", "drop", "cpu", "drop log", "drop notify", "redirect to ethernet", "redirect to pppoe", "mirror", "mirro keep match", "drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps" };
		_rtl8651_tblDrvAclRule_t asic_acl;
		rtl865x_tblAsicDrv_vlanParam_t asic_vlan;
		uint32 start, end;
		uint16 vid;
		int8 outRule;


				
		rtlglue_printf(">>ASIC ACL Table:\n\n");
		for(vid=0; vid<4095; vid++ ) {
			/* Read VLAN Table */
			if (rtl8651_getAsicVlan(vid, &asic_vlan) == FAILED)
				continue;

			outRule = FALSE;
			start = asic_vlan.inAclStart; end = asic_vlan.inAclEnd;
	again:
			if (outRule == FALSE)
				rtlglue_printf("\n<<Ingress Rule for Vlan %d:>>\n", vid);
			else rtlglue_printf("\n<<Egress Rule for Vlan %d>>:\n", vid);
			for( ; start<=end; start++) {
				if (rtl8651_getAsicAclRule(start, &asic_acl) == FAILED)
					assert(0);
				switch(asic_acl.ruleType_)
				{
				case RTL8651_ACL_MAC:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Ethernet", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
					rtlglue_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
					);
					rtlglue_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
					break;

				case RTL8651_ACL_IP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "IP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);
					rtlglue_printf("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
					);
					rtlglue_printf("\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;			

				case RTL8651_ACL_ICMP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "ICMP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
					break;

				case RTL8651_ACL_IGMP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "IGMP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
					);
					break;

				case RTL8651_ACL_TCP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "TCP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
					);
					rtlglue_printf("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
					break;

				case RTL8651_ACL_UDP:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "UDP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
					);
					rtlglue_printf("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
					break;				

				case RTL8651_ACL_IFSEL:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "UDP", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tgidxSel: %x\n", asic_acl.gidxSel_);
					break;

				case RTL8651_ACL_SRCFILTER:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Source Filter", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
					);
					rtlglue_printf("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
					);
					rtlglue_printf("\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
					);
					rtlglue_printf("\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
					break;

				case RTL8651_ACL_DSTFILTER:
					rtlglue_printf(" [%d] rule type: %s   rule action: %s\n", start, "Deatination Filter", actionT[asic_acl.actionType_]);
					rtlglue_printf("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
					);
					rtlglue_printf("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
					);
					rtlglue_printf("\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
					break;

				default: assert(0);

				}


				/* Action type */
				switch (asic_acl.actionType_) {

				case RTL8651_ACL_PERMIT: /* 0x00 */
				case RTL8651_ACL_REDIRECT: /* 0x01 */
				case RTL8651_ACL_CPU: /* 0x03 */
				case RTL8651_ACL_DROP: /* 0x02, 0x04 */
				case RTL8651_ACL_DROP_LOG: /* 0x05 */
				case RTL8651_ACL_MIRROR: /* 0x06 */
				case RTL8651_ACL_REDIRECT_PPPOE: /* 0x07 */
				case RTL8651_ACL_MIRROR_KEEP_MATCH: /* 0x09 */
					rtlglue_printf("\tdvidx: %d   hp: %d   pppoeIdx: %d   nxtHop:%d  ", asic_acl.dvid_, asic_acl.priority_,
							asic_acl.pppoeIdx_, asic_acl.nextHop_);
					break;

				case RTL8651_ACL_POLICY: /* 0x08 */
					rtlglue_printf("\thp: %d   nxtHopIdx: %d  ", asic_acl.priority_, asic_acl.nhIndex);
					break;

				case RTL8651_ACL_DROP_RATE_EXCEED_PPS: /* 0x0a */
				case RTL8651_ACL_LOG_RATE_EXCEED_PPS: /* 0x0b */
				case RTL8651_ACL_DROP_RATE_EXCEED_BPS: /* 0x0c */
				case RTL8651_ACL_LOG_RATE_EXCEED_BPS: /* 0x0d */
					rtlglue_printf("\trlIdx: %d  ", asic_acl.rlIndex);
					break;
				default: assert(0);
				
				}
				rtlglue_printf("pktOpApp: %d\n", asic_acl.pktOpApp);
			
			}

			if (outRule == FALSE) {
				start = asic_vlan.outAclStart; end = asic_vlan.outAclEnd;
				outRule = TRUE;
				goto again;
			}
		}
	}
	else if ( strcmp(nextToken, "hstamp") == 0 ){
		hs_dumpHsb(0);
		hs_dumpHsa(0);
	}
#endif
#ifdef CONFIG_RTL865XC
	 else if ( strcmp(nextToken, "netif") == 0 )
	{
		rtl865xC_dump_netif();
	}
	 else if ( strcmp(nextToken, "l2") == 0 )
	 {
	 	rtl865xC_dump_l2();
	 }
	 else if ( strcmp(nextToken, "route") == 0 )
	 {
	 	rtl865xC_dump_l3();
	 }
	 else if ( strcmp(nextToken, "hstamp") == 0 )
	 {
		dump_hs();	
	 }
	 else if ( strcmp(nextToken, "acl") == 0 )
	 {
	 	rtl865xC_dump_acl();
	 }
	else if (strcmp(nextToken, "mcast") == 0) {
		rtl865x_tblAsicDrv_multiCastParam_t asic;
		uint32 entry;
		
		rtlglue_printf(">>ASIC Multicast Table:\n");
		for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) {
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				//rtlglue_printf("\t[%d]  (Invalid Entry)\n", entry);
				continue;
			}
			if (asic.sip != 0) {
			  rtlglue_printf("\t[%d]  Mip(%d.%d.%d.%d) sip(%d.%d.%d.%d) MBR(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff), 
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			  rtlglue_printf("\t       svid:%d, spa:%d, EXtIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
			}
		}
	}
	 
#endif
	else if (strcmp(nextToken, "pppoe") == 0) {
		rtl865x_tblAsicDrv_pppoeParam_t asic_pppoe;

		int32	i;

		rtlglue_printf(">>PPPOE Table:\n");
		for(i=0; i<RTL8651_PPPOETBL_SIZE; i++) {
			if (rtl8651_getAsicPppoe(i,  &asic_pppoe) == FAILED)
				continue;

			rtlglue_printf("\t[%d]  sessionID(%d)  ageSec(%d)\n", i, asic_pppoe.sessionId, asic_pppoe.age);
		}
	}
	else if (strcmp(nextToken, "nexthop") == 0) {
			rtl865x_tblAsicDrv_nextHopParam_t asic_nxthop;

			uint32 idx, refcnt, rt_flag;

			rtlglue_printf(">>ASIC Next Hop Table:\n");
			for(idx=0; idx<RTL8651_NEXTHOPTBL_SIZE; idx++) {
				refcnt = rt_flag = 0;
				if (rtl8651_getAsicNextHopTable(idx, &asic_nxthop) == FAILED)
					continue;
				rtlglue_printf("  [%d]  type(%s) IPIdx(%d) dstVid(%d) pppoeIdx(%d) nextHop(%d) rf(%d) rt(%d)\n", idx,
					(asic_nxthop.isPppoe==TRUE? "pppoe": "ethernet"), asic_nxthop.extIntIpIdx, 
					asic_nxthop.dvid, asic_nxthop.pppoeIdx, (asic_nxthop.nextHopRow<<2)+asic_nxthop.nextHopColumn, refcnt, rt_flag);
			}
	}
       else if(strcmp(nextToken, "icmp") == 0) {
		rtl865xC_dump_icmp();
       }    
       else if(strcmp(nextToken, "bandwidth") == 0) {       
		uint32 asicRate;
		uint32 rate;
		uint32 port;
		for(port=0; port<RTL8651_PORT_NUMBER; port++) {
			if (rtl8651_getAsicPortIngressBandwidth(port,&asicRate)!=SUCCESS)
			{
				return FAILED;
			}

			rate = asicRate*16;
			if (asicRate == 0)
			{
				rtlglue_printf("port %d => in(Unlimited), ", port);
			}
			else if (rate > 1024)
			{
				rtlglue_printf("port %d => in(%3d.%03dMbps), ", port, rate/1024, (rate%1024) * 1000 /1024);
			}
			else
			{
				rtlglue_printf("port %d => in(%3d.%03dKbps), ", port, rate, 0);
			}
			
			if (rtl8651_getAsicPortEgressBandwidth(port, &asicRate) != SUCCESS)
				return FAILED;

			rate = asicRate*64000/1024;
			if (asicRate == 0x3fff)
			{
				rtlglue_printf("out(Unlimited)\n");
			}
			else if (rate > 1024) 
			{
				
				rtlglue_printf("out(%3d.%03dMbps)\n", rate/1024, (rate%1024) *1000 /1024);
			}
			else
			{
				rtlglue_printf("out(%3d.%03dKbps)\n", rate, 0);
			}
		}
		rtlglue_printf("Bandwidth control not available for extension port.\n");
              return SUCCESS;
       }
	else if (strcmp(nextToken, "qos") == 0) {
		uint32 qoscr;

		if (rtl8651_getAsicQoSControlRegister(&qoscr) == FAILED)
			return FAILED;
		rtlglue_printf(">> QoS:\n\n");
		rtlglue_printf("\tQoS : %s\n", ((qoscr & (1<<20))? "enable": "disable"));
		switch((qoscr&0xc0000000)>>30)
		{
		case RTL8651_RR_H_FIRST:	rtlglue_printf("\tRR Ratio: Always high priority queue first\n"); break;
		case RTL8651_RR_H16_L1:	rtlglue_printf("\tRR Ratio: H16:L1\n"); break; 
		case RTL8651_RR_H8_L1:	rtlglue_printf("\tRR Ratio: H8:L1\n"); break;
		case RTL8651_RR_H4_L1:	rtlglue_printf("\tRR Ratio: H4:L1\n"); break;
		default: assert(0);
		}
		rtlglue_printf("\t802.1p Vlan Tag Priority: %s\n", ((qoscr&(1<<27))? "disable": "enable")); /* 0: enable, 1: disable */
		rtlglue_printf("\tPort 5 Priority: %s\n", ((qoscr&(1<<26))? "high": "low"));
		rtlglue_printf("\tPort 4 Priority: %s\n", ((qoscr&(1<<25))? "high": "low"));
		rtlglue_printf("\tPort 3 Priority: %s\n", ((qoscr&(1<<24))? "high": "low"));
		rtlglue_printf("\tPort 2 Priority: %s\n", ((qoscr&(1<<23))? "high": "low"));
		rtlglue_printf("\tPort 1 Priority: %s\n", ((qoscr&(1<<22))? "high": "low"));
		rtlglue_printf("\tPort 0 Priority: %s\n", ((qoscr&(1<<21))? "high": "low"));

		rtlglue_printf("\n");	
	}
       else if (strcmp(nextToken, "flow-control") == 0) {
		rtl865xC_dump_flowCtrlRegs();
	}
	else if (strcmp(nextToken, "rate-limit") == 0) {
		rtl865x_tblAsicDrv_rateLimitParam_t asic_rl;
		uint32 entry;

		for(entry=0; entry<RTL8651_RATELIMITTBL_SIZE; entry++) {
			if (rtl8651_getAsicRateLimitTable(entry, &asic_rl) == SUCCESS) {
				rtlglue_printf(" [%d]  Token(%u)  MaxToken(%u)  remainTime Unit(%u)  \n\trefillTimeUnit(%u)  refillToken(%u)\n",
					entry, asic_rl.token, asic_rl.maxToken, asic_rl.t_remainUnit, asic_rl.t_intervalUnit, asic_rl.refill_number);
			}
			else rtlglue_printf(" [%d]  Invalid entry\n", entry);
		}
		rtlglue_printf("\n");
	}       
	else if (strcmp(nextToken, "l3") == 0){
		rtl865x_tblAsicDrv_routingParam_t asic_l3;
		int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
		uint32 idx, mask;

		rtlglue_printf(">>L3 Routing Table:\n");
		for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++) {
			if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED) {
				rtlglue_printf("\t[%d]  (Invalid)\n", idx);
				continue;
			}
			if (idx == RTL8651_ROUTINGTBL_SIZE-1)
				mask = 0;
			else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
					mask--;
			rtlglue_printf("\t[%d]  %d.%d.%d.%d/%d process(%s)\n", idx, (asic_l3.ipAddr>>24),
				((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff), 
				mask, str[asic_l3.process]
			);
			switch(asic_l3.process) 
			{
			case 0x00:	/* PPPoE */
				rtlglue_printf("\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
				break;
				
			case 0x01:	/* L2 */
				rtlglue_printf("              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
				break;

			case 0x02:	/* ARP */
				rtlglue_printf("             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart, asic_l3.arpEnd, asic_l3.arpIpIdx);
				break;

			case 0x03:	/* Reserved */
				assert(0);

			case 0x04:	/* CPU */
				rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
				break;

			case 0x05:	/* NAPT Next Hop */
				rtlglue_printf("              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
					asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
				break;

			case 0x06:	/* DROP */
				rtlglue_printf("             dvidx(%d)\n", asic_l3.vidx);
				break;

			case 0x07:	/* Reserved */
				/* pass through */
			default: assert(0);
			}
		}
	}
	else return FAILED;
	
	return SUCCESS;
}

struct ext_port_stats {
	uint32				rx_packets; //means WLAN Rx
	uint64				rx_bytes;
	uint32				tx_packets; //means WLAN Tx
	uint64				tx_bytes;
};
struct ext_port_stats ep_stats;

extern struct lr_cpu_stats lr_stats;

static int32 _rtl865x_counterCmds(uint32 userId,  int32 argc,int8 **saved)
{
	int32 size;
	int8 *nextToken;
	
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if (strcmp(nextToken, "asic") == 0) {
              #if 1 // for extension port
		rtlglue_printf("The bytes and packets through hardware forwarding...\n");
		rtlglue_printf("  WLAN Rx Bytes: %llu, Rx Packets: %u\n", ep_stats.rx_bytes, ep_stats.rx_packets);
		rtlglue_printf("  WLAN Tx Bytes: %llu, Tx Packets: %u\n\n", ep_stats.tx_bytes, ep_stats.tx_packets);
        
              return rtl865xC_dumpAsicCounter();
              #else
		uint32 i;

		rtlglue_printf("Port: ");
		for(i=0; i<RTL8651_PORT_NUMBER;i++)
			if(READ_MEM32(MIB_CONTROL) & (1<<(i+24)))
				rtlglue_printf("%d ", i);
		for(i=0; i<rtl8651_totalExtPortNum;i++)
			if(READ_MEM32(MIB_CONTROL) & (1<<(i+21)))
				rtlglue_printf("%d ", i+RTL8651_PORT_NUMBER);
		rtlglue_printf("\n");
		rtlglue_printf("Rx %u Bytes, Rx %u Pkts, Drop %u pkts, ToCPU %u pkts, Rx Pause %u pkts\n", rtl8651_returnAsicCounter(0),
			rtl8651_returnAsicCounter(0x20) + rtl8651_returnAsicCounter(0x24) + rtl8651_returnAsicCounter(0x28) + rtl8651_returnAsicCounter(0x2c) + rtl8651_returnAsicCounter(0x30) + rtl8651_returnAsicCounter(0x34) + rtl8651_returnAsicCounter(0x38) + rtl8651_returnAsicCounter(0x3c),
			rtl8651_returnAsicCounter(0x8), rtl8651_returnAsicCounter(0x74), rtl8651_returnAsicCounter(0x44));
		rtlglue_printf("Tx %u Bytes, Tx %u Pkts Tx Pause %u pkts\n", rtl8651_returnAsicCounter(0x48),
			rtl8651_returnAsicCounter(0x4c) + rtl8651_returnAsicCounter(0x50) + rtl8651_returnAsicCounter(0x54),
			rtl8651_returnAsicCounter(0x6c));

		rtlglue_printf("Port: ");
		return SUCCESS;
              #endif

	}
       else if(strcmp(nextToken, "complex") == 0) {
	      return rtl865xC_dumpAsicDiagCounter();
       }
#ifdef CONFIG_RTK_VOIP
	// tim: add dbg by joey
	else if (strcmp(nextToken, "dbg") == 0) {
		print_desc_own_bit();
		return SUCCESS;
       }
#endif
	else if (strcmp(nextToken, "clear_asic") == 0) {
     		memset(&ep_stats, 0, sizeof(struct ext_port_stats));

		return rtl8651_clearAsicCounter();
	}
	else if (strcmp(nextToken, "soft") == 0) {
		rtlglue_printf("\n");
		rtlglue_printf("CPU Rx Packets: %u,  Rx Bytes: %u,  Rx Drops: %u\n", lr_stats.rx_packets, lr_stats.rx_bytes, lr_stats.rx_drop);
		rtlglue_printf("CPU Rx Broadcast: %u,  Rx Multicast: %u\n", lr_stats.rx_bcast, lr_stats.rx_mcast);
		rtlglue_printf("CPU Tx Packets: %u,  Tx Bytes: %u,  Tx Drops: %u\n", lr_stats.tx_packets, lr_stats.tx_bytes, lr_stats.tx_drop);
		rtlglue_printf("CPU Tx Broadcast: %u,  Tx Multicast: %u\n", lr_stats.tx_bcast, lr_stats.tx_mcast);
		rtlglue_printf("CPU Bridging Packets: %u\n\n", lr_stats.br_packets);
		return SUCCESS;
	}
	else if (strcmp(nextToken, "clear_soft") == 0) {
		memset(&lr_stats, 0, sizeof(struct lr_cpu_stats));
		return SUCCESS;
	}
	return FAILED;
}





#ifdef CONFIG_RTL865XC
cle_exec_t rtl865x_lightRome_cmds[] = {
	{	"dump",
		"Dump a specified ASIC Table or Light-Rome Table",
		" { asic'Dump ASIC Table' | lrome'Light-Rome Software Table' } { ifdev'Layer3 interface' | nat'NAT Table' | ip'IP table' | route'Routing Table' | arp'Arp Table' | vlan'Vlan Table' | l2'MAC Address Table' | acl'Access Control List' | pppoe'Pppoe Table' | hstamp'Hsb hsa' | netif'Net interface' | nexthop'Nexthop Table' | l3'L3'  | icmp'ICMP' | bandwidth'bandwidth' | qos'qos' | mcast'mcast'  | flow-control'flow-control' | rate-limit'rate-limit'}  ",
		_rtl865x_dumpCmds,		
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},
	{
		"counter",
		"Display ASIC static information",
/*		"{ asic'Show ASIC Counter' | clear_asic'Reset ASIC Counter' | soft'Show Software Counter' | clear_soft'Reset Software Counter' } ", */
#ifdef CONFIG_RTK_VOIP
		// tim: add dbg'test' by joey
		"{ asic'Show ASIC Counter' | complex'Show complex ASIC Counter' | clear_asic'Reset ASIC Counter' | soft'Show Software Counter' | clear_soft'Reset Software Counter'  | dbg'test'} ",
#else
		"{ asic'Show ASIC Counter' | complex'Show complex ASIC Counter' | clear_asic'Reset ASIC Counter' | soft'Show Software Counter' | clear_soft'Reset Software Counter' } ",
#endif
		_rtl865x_counterCmds,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},

};
#else
cle_exec_t rtl865x_lightRome_cmds[] = {
	{	"dump",
		"Dump a specified ASIC Table or Light-Rome Table",
		" { asic'Dump ASIC Table' | lrome'Light-Rome Software Table' } { ifdev'Layer3 interface' | nat'NAT Table' | ip'IP table' | route'Routing Table' | arp'Arp Table' | vlan'Vlan Table' | l2'MAC Address Table' | acl'Access Control List' | pppoe'Pppoe Table' | hstamp'Hsb&hsa'  | nexthop'Nexthop Table'  | l3'l3 table'}  ",
		_rtl865x_dumpCmds,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},
	{
		"counter",
		"Display ASIC static information",
		"{ asic'Show ASIC Counter' | clear_asic'Reset ASIC Counter' | soft'Show Software Counter' | clear_soft'Reset Software Counter' } ",
		_rtl865x_counterCmds,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},

};
#endif


