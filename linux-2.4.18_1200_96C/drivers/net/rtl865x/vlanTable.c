/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/vlanTable.c,v 1.4 2010/04/09 07:59:58 joeylin Exp $
*
* Abstract: Switch core vlan table access driver source code.
*
* $Author: joeylin $
*
* $Log: vlanTable.c,v $
* Revision 1.4  2010/04/09 07:59:58  joeylin
* fix the issue: the DUT's DHCP client can not get IP from DHCP server when the WAN mac is 94:c0:02:12:34:56
*
* Revision 1.3  2009/01/13 13:36:09  bradhuang
* modify include path for new tool chain
*
* Revision 1.2  2007/12/04 12:00:18  joeylin
* add hardware NAT feature
*
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.6  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.5  2006/01/16 02:40:41  bo_zhao
* +: add some support for gcc '-O' flag
*
* Revision 1.4  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:25  alva
* initial import for add TFTP server
*
* Revision 1.3  2005/01/28 02:03:48  yjlou
* *: loader version migrates to "00.00.19".
* +: support Hub mode
* +: Ping mode support input IP address
* *: clear WDTIND always.
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.2  2004/03/09 00:46:12  danwu
* remove unused code to shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
*
* Revision 1.1.1.1  2003/09/25 08:16:55  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:06  danwu
* no message
*
* ---------------------------------------------------------------
*/
//#include <asm/rtl8196.h>
#include <linux/string.h>
#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "swCore.h"
#include "vlanTable.h"
#include "swTable.h"

#include "rtl865xC_tblAsicDrv.h"

extern void tableAccessForeword(uint32, uint32, void *);
extern int32 swTable_addEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
extern int32 swTable_modifyEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
extern int32 swTable_forceAddEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
extern int32 swTable_readEntry(uint32 tableType, uint32 eidx, void *entryContent_P);

#if 0
#include <asm/mipsregs.h>

int lx4180_ReadStatus(void)
{
   volatile unsigned int reg;
	reg= read_32bit_cp0_register(CP0_STATUS);
	__asm__ volatile("nop");	// david
   	__asm__ volatile("nop");   
	return reg;

}
void lx4180_WriteStatus(int s)
{
   volatile unsigned int reg=s;
	write_32bit_cp0_register(CP0_STATUS, reg);
	__asm__ volatile("nop");	// david
   	__asm__ volatile("nop");   
	return ;

}
#endif


/* STATIC VARIABLE DECLARATIONS
 */



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

int32 swCore_aclCreate(uint32 idx, rtl_acl_param_t * rule)
{
	acl_table_t    entry;

	rule->aclIdx = idx;
	memset(&entry, 0, sizeof(entry));

	switch(rule->ruleType_) {
	case RTL8651_ACL_MAC: /* Etnernet type rule: 0x0000 */
		 entry.is.ETHERNET.dMacP47_32 = rule->dstMac_.octet[0]<<8 	| rule->dstMac_.octet[1];
		 entry.is.ETHERNET.dMacP31_16 = rule->dstMac_.octet[2]<<8 	| rule->dstMac_.octet[3];
		 entry.is.ETHERNET.dMacP15_0   = rule->dstMac_.octet[4]<<8 	| rule->dstMac_.octet[5];
	 	 entry.is.ETHERNET.dMacM47_32 = rule->dstMacMask_.octet[0]<<8 | rule->dstMacMask_.octet[1];
		 entry.is.ETHERNET.dMacM31_16 = rule->dstMacMask_.octet[2]<<8 | rule->dstMacMask_.octet[3];
		 entry.is.ETHERNET.dMacM15_0   = rule->dstMacMask_.octet[4]<<8 | rule->dstMacMask_.octet[5];
		 entry.is.ETHERNET.sMacP47_32  = rule->srcMac_.octet[0]<<8 	| rule->srcMac_.octet[1];
		 entry.is.ETHERNET.sMacP31_16  = rule->srcMac_.octet[2]<<8 	| rule->srcMac_.octet[3];
		 entry.is.ETHERNET.sMacP15_0    = rule->srcMac_.octet[4]<<8 	| rule->srcMac_.octet[5];
		 entry.is.ETHERNET.sMacM47_32  = rule->srcMacMask_.octet[0]<<8 | rule->srcMacMask_.octet[1];
		 entry.is.ETHERNET.sMacM31_16  = rule->srcMacMask_.octet[2]<<8 | rule->srcMacMask_.octet[3];
		 entry.is.ETHERNET.sMacM15_0    = rule->srcMacMask_.octet[4]<<8 | rule->srcMacMask_.octet[5];
		 entry.is.ETHERNET.ethTypeP       = rule->typeLen_;
		 entry.is.ETHERNET.ethTypeM       = rule->typeLenMask_;
		 entry.ruleType = 0x0;
		 break;
		 
	case RTL8651_ACL_IP: /* IP Rule Type: 0x0010 */
	case RTL8652_ACL_IP_RANGE:
		 entry.is.L3L4.is.IP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IP.IPProtoP = rule->ipProto_;
		 entry.is.L3L4.is.IP.IPProtoM = rule->ipProtoMask_;
		 entry.is.L3L4.is.IP.IPFlagP = rule->ipFlag_;
		 entry.is.L3L4.is.IP.IPFlagM = rule->ipFlagMask_;
 		 entry.is.L3L4.is.IP.FOP = rule->ipFOP_;
		 entry.is.L3L4.is.IP.FOM = rule->ipFOM_;
		 entry.is.L3L4.is.IP.HTTPP = entry.is.L3L4.is.IP.HTTPM = rule->ipHttpFilter_;
		 entry.is.L3L4.is.IP.identSDIPP = entry.is.L3L4.is.IP.identSDIPM = rule->ipIdentSrcDstIp_;
		 if (rule->ruleType_==RTL8651_ACL_IP)
			 entry.ruleType = 0x2;
		else
			 entry.ruleType = 0xa;
		 goto l3l4_shared;
		 
	case RTL8651_ACL_ICMP:
	case RTL8652_ACL_ICMP_IPRANGE:
		 entry.is.L3L4.is.ICMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.ICMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.ICMP.ICMPTypeP = rule->icmpType_;
		 entry.is.L3L4.is.ICMP.ICMPTypeM = rule->icmpTypeMask_;
		 entry.is.L3L4.is.ICMP.ICMPCodeP = rule->icmpCode_;
		 entry.is.L3L4.is.ICMP.ICMPCodeM = rule->icmpCodeMask_;
 		 if (rule->ruleType_==RTL8651_ACL_ICMP)
			 entry.ruleType = 0x4;
		 else
		 	entry.ruleType=0xc;
		 goto l3l4_shared;

	case RTL8651_ACL_IGMP:
	case RTL8652_ACL_IGMP_IPRANGE:
		 entry.is.L3L4.is.IGMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IGMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IGMP.IGMPTypeP = rule->igmpType_;
		 entry.is.L3L4.is.IGMP.IGMPTypeM = rule->igmpTypeMask_; 
  		 if (rule->ruleType_==RTL8651_ACL_IGMP)
			 entry.ruleType = 0x5;
   		 else
			entry.ruleType=0xd;
 		 goto l3l4_shared;

	case RTL8651_ACL_TCP:
	case RTL8652_ACL_TCP_IPRANGE:
		 entry.is.L3L4.is.TCP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.TCP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.TCP.TCPFlagP = rule->tcpFlag_;
		 entry.is.L3L4.is.TCP.TCPFlagM = rule->tcpFlagMask_;
		 entry.is.L3L4.is.TCP.TCPSPUB = rule->tcpSrcPortUB_;
		 entry.is.L3L4.is.TCP.TCPSPLB = rule->tcpSrcPortLB_;
		 entry.is.L3L4.is.TCP.TCPDPUB = rule->tcpDstPortUB_;
		 entry.is.L3L4.is.TCP.TCPDPLB = rule->tcpDstPortLB_;
 		 if (rule->ruleType_==RTL8651_ACL_TCP)
			 entry.ruleType = 0x6;
 		else
			entry.ruleType=0xe;
         goto l3l4_shared;

	case RTL8651_ACL_UDP:
	case RTL8652_ACL_UDP_IPRANGE:
		 entry.is.L3L4.is.UDP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.UDP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.UDP.UDPSPUB = rule->udpSrcPortUB_;
		 entry.is.L3L4.is.UDP.UDPSPLB = rule->udpSrcPortLB_;
		 entry.is.L3L4.is.UDP.UDPDPUB = rule->udpDstPortUB_;
		 entry.is.L3L4.is.UDP.UDPDPLB = rule->udpDstPortLB_;
  		 if (rule->ruleType_==RTL8651_ACL_UDP)
			 entry.ruleType = 0x7;
		 else
		 	entry.ruleType=0xf;

l3l4_shared:
		 entry.is.L3L4.sIPP = rule->srcIpAddr_;
		 entry.is.L3L4.sIPM = rule->srcIpAddrMask_;
		 entry.is.L3L4.dIPP = rule->dstIpAddr_;
		 entry.is.L3L4.dIPM = rule->dstIpAddrMask_;
		 break;

	default: return FAILED; /* Unknown rule type */
	
	}
	
	switch(rule->actionType_) {
	case RTL8651_ACL_PERMIT:			entry.actionType = 0x00;
		 goto _common_action;
	case RTL8651_ACL_DROP:			entry.actionType = 0x02;
		 goto _common_action;
	case RTL8651_ACL_CPU:		 	 	entry.actionType = 0x03;
		 goto _common_action;
	case RTL8651_ACL_DROP_LOG: /* fall thru */
	case RTL8651_ACL_DROP_NOTIFY: entry.actionType = 0x05;
		goto _common_action;

_common_action:
		/* handle pktOpApp */
		if ( rule->pktOpApp == RTL865XC_ACLTBL_ALL_LAYER )
			entry.pktOpApp = 0x7;
		else if ( rule->pktOpApp == RTL8651_ACLTBL_NOOP )
			entry.pktOpApp = 0;
		else
 			entry.pktOpApp = rule->pktOpApp;
 		break;
	}

    /* Write into hardware */
    if ( swTable_forceAddEntry(TYPE_ACL_RULE_TABLE, idx, &entry) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}


int32 swCore_netifCreate(uint32 idx, rtl_netif_param_t * param)
{
    netif_table_t    entryContent;
		
#if 0    
    uint32	temp,temp2;    

    // disable interrupt
    // I don't know the reason but if you want to use "-O" flag, must disalbe interrupt before swTable_readEntry();
    temp = lx4180_ReadStatus();
    if (0 != (temp&0x1)) {
	    temp2 = temp&0xfffffffe;
	    lx4180_WriteStatus(temp2);
    }
#endif		

    ASSERT_CSP(param);

    swTable_readEntry(TYPE_NETINTERFACE_TABLE, idx, &entryContent);

#if 0
    // restore status register
    if (0 != (temp&0x1)) {
	    lx4180_WriteStatus(temp);
    }
#endif
		
    if ( entryContent.valid )
    {
       return EEXIST;
    }

    bzero( (void *) &entryContent, sizeof(entryContent) );
    entryContent.valid = param->valid;
    entryContent.vid = param->vid;

    entryContent.mac47_19 = ((param->gMac.mac47_32 << 13) | (param->gMac.mac31_16 >> 3)) & 0x1FFFFFFF;
    entryContent.mac18_0 = ((param->gMac.mac31_16 << 16) | param->gMac.mac15_0) & 0x7FFFF;

    entryContent.inACLStartH = (param->inAclStart >> 2) & 0x1f;
    entryContent.inACLStartL = param->inAclStart & 0x3;
    entryContent.inACLEnd = param->inAclEnd;
    entryContent.outACLStart = param->outAclStart;
    entryContent.outACLEnd = param->outAclEnd;
    entryContent.enHWRoute = param->enableRoute;

    entryContent.macMask = 8 - (param->macAddrNumber & 0x7);

    entryContent.mtuH = param->mtu >> 3;
    entryContent.mtuL = param->mtu & 0x7;

    /* Write into hardware */
    if ( swTable_addEntry(TYPE_NETINTERFACE_TABLE, idx, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}




int32 vlanTable_create(uint32 vid, rtl_vlan_param_t * param)
{
    vlan_table_t    entryContent;
#if 0		
    uint32	temp,temp2;    

    // disable interrupt
    // I don't know the reason but if you want to use "-O" flag, must disalbe interrupt before swTable_readEntry();
    temp = lx4180_ReadStatus();
    if (0 != (temp&0x1)) {
	    temp2 = temp&0xfffffffe;
	    lx4180_WriteStatus(temp2);
    }
#endif
		
    ASSERT_CSP(param);
    swTable_readEntry(TYPE_VLAN_TABLE, vid, &entryContent);

#if 0		
    // restore status register
    if (0 != (temp&0x1)) {
	    lx4180_WriteStatus(temp);
    }
#endif		

    bzero( (void *) &entryContent, sizeof(entryContent) );

#if 1
#define RTL8651_MAC_NUMBER				6
#define RTL8651_PORT_NUMBER				RTL8651_MAC_NUMBER
#define RTL8651_PHYSICALPORTMASK			((1<<RTL8651_MAC_NUMBER)-1)

    if(param->memberPort > RTL8651_PHYSICALPORTMASK )
        entryContent.extMemberPort = param->memberPort >> RTL8651_PORT_NUMBER;
    if(param->egressUntag > RTL8651_PHYSICALPORTMASK )
        entryContent.extEgressUntag = param->egressUntag >> RTL8651_PORT_NUMBER;	
    entryContent.memberPort = param->memberPort & RTL8651_PHYSICALPORTMASK;
    entryContent.egressUntag = param->egressUntag & RTL8651_PHYSICALPORTMASK;
#else
    entryContent.memberPort = param->memberPort & ALL_PORT_MASK;
    entryContent.egressUntag = param->egressUntag;
#endif
    entryContent.fid = param->fid;

    /* Write into hardware */
    if ( swTable_addEntry(TYPE_VLAN_TABLE, vid, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}






int32 vlanTable_destroy(uint32 vid)
{
    vlan_table_t    entryContent;

#if 0		
    uint32	temp,temp2;
    
    // disable interrupt
    // I don't know the reason but if you want to use "-O" flag, must disalbe interrupt before swTable_readEntry();
    temp = lx4180_ReadStatus();

    if (0 != (temp&0x1)) {
	    temp2 = temp&0xfffffffe;
	    lx4180_WriteStatus(temp2);
    }
#endif		

    swTable_readEntry(TYPE_VLAN_TABLE, vid, &entryContent);

#if 0		
    // restore status register
    if (0 != (temp&0x1)) {
	    lx4180_WriteStatus(temp);
    }
#endif		
    
    bzero(&entryContent, sizeof(vlan_table_t));
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}






int32 vlanTable_setStpStatusOfAllPorts(uint32 vid, uint32 STPStatus)
{
    return 0;
}

