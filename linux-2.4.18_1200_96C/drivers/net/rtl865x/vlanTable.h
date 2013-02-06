/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/vlanTable.h,v 1.1.1.1 2007/08/06 10:04:52 root Exp $
*
* Abstract: Switch core vlan table access header file.
*
* $Author: root $
*
* $Log: vlanTable.h,v $
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.4  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.3  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/

#ifndef _VLANTABLE_H_
#define _VLANTABLE_H_



/* VLAN table access routines 
*/

/* Create vlan 
Return: EEXIST- Speicified vlan already exists.
        ENFILE- Destined slot occupied by another vlan.*/
int vlanTable_create(unsigned int vid, rtl_vlan_param_t * param);

/* Destroy vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_destroy(unsigned int vid);

/* Add a member port
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_addMemberPort(unsigned int vid, unsigned int portNum);

/* Remove a member port 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_removeMemberPort(unsigned int vid, unsigned int portNum);

/* Set a member port list 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setMemberPort(unsigned int vid, unsigned int portList);

/* Set ACL rule 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setAclRule(unsigned int vid, unsigned int inACLStart, unsigned int inACLEnd,
                                unsigned int outACLStart, unsigned int outACLEnd);

/* Get ACL rule 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getAclRule(unsigned int vid, unsigned int *inACLStart_P, unsigned int *inACLEnd_P,
                                unsigned int *outACLStart_P, unsigned int *outACLEnd_P);

/* Set vlan as internal interface 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setInternal(unsigned int vid);

/* Set vlan as external interface 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setExternal(unsigned int vid);

/* Enable hardware routing for this vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_enableHardwareRouting(unsigned int vid);

/* Disable hardware routing for this vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_disableHardwareRouting(unsigned int vid);

/* Set spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setPortStpStatus(unsigned int vid, unsigned int portNum, unsigned int STPStatus);

/* Get spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getPortStpStatus(unsigned int vid, unsigned int portNum, unsigned int *STPStatus_P);

/* Set spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_setStpStatus(unsigned int vid, unsigned int STPStatus);

/* Get information 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getInformation(unsigned int vid, rtl_vlan_param_t * param_P);

/* Get hardware information 
Return: ENOENT- Specified vlan id does not exist.*/
int vlanTable_getHwInformation(unsigned int vid, rtl_vlan_param_t * param_P);

/* Get vlan id 
Return: ENOENT- Specified slot does not exist.*/
int vlanTable_getVidByIndex(unsigned int eidx, unsigned int * vid_P);

int32 swCore_aclCreate(uint32 idx, rtl_acl_param_t * rule);
int32 swCore_netifCreate(uint32 idx, rtl_netif_param_t * param);

#define CONFIG_RTL865XC 1
#ifdef CONFIG_RTL865XC
/* Hardware bit allocation of VLAN table 
*/
typedef struct {
	 /* word 0 */
	unsigned int	reserved1:12;
	unsigned int	fid:2;
	unsigned int     extEgressUntag  : 3;
	unsigned int     egressUntag : 6;
	unsigned int     extMemberPort   : 3;
	unsigned int     memberPort  : 6;

    /* word 1 */
    unsigned int          reservw1;
    /* word 2 */
    unsigned int          reservw2;
    /* word 3 */
    unsigned int          reservw3;
    /* word 4 */
    unsigned int          reservw4;
    /* word 5 */
    unsigned int          reservw5;
    /* word 6 */
    unsigned int          reservw6;
    /* word 7 */
    unsigned int          reservw7;
} vlan_table_t;

typedef struct {
    /* word 0 */
    unsigned int          mac18_0:19;
    unsigned int          vid		 : 12;
    unsigned int          valid       : 1;	
    /* word 1 */
    unsigned int         inACLStartL:2;	
    unsigned int         enHWRoute : 1;	
    unsigned int         mac47_19:29;

    /* word 2 */
    unsigned int         mtuL       : 3;
    unsigned int         macMask :3;	
    unsigned int         outACLEnd : 7;	
    unsigned int         outACLStart : 7;	
    unsigned int         inACLEnd : 7;	
    unsigned int         inACLStartH: 5;	
    /* word 3 */
    unsigned int          reserv10   : 20;
    unsigned int          mtuH       : 12;

    /* word 4 */
    unsigned int          reservw4;
    /* word 5 */
    unsigned int          reservw5;
    /* word 6 */
    unsigned int          reservw6;
    /* word 7 */
    unsigned int          reservw7;
} netif_table_t;


typedef struct {
   union {
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
            uint16          dMacM15_0;
            uint16          dMacP47_32;
            /* word 2 */
            uint16          dMacM47_32;
            uint16          dMacM31_16;
            /* word 3 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 4 */
            uint16          sMacM15_0;
            uint16          sMacP47_32;
            /* word 5 */
            uint16          sMacM47_32;
            uint16          sMacM31_16;
            /* word 6 */
            uint16          ethTypeM;
            uint16          ethTypeP;
        } ETHERNET;
        struct {
            /* word 0 */
            unsigned int          reserv1     : 24;
            unsigned int          gidxSel     : 8;
            /* word 1~6 */
            unsigned int          reserv2[6];
        } IFSEL;
        struct {
            /* word 0 */
            ipaddr_t        sIPP;
            /* word 1 */
            ipaddr_t        sIPM;
            /* word 2 */
            ipaddr_t        dIPP;
            /* word 3 */
            ipaddr_t        dIPM;
            union {
                struct {
                    /* word 4 */
                    uint8           IPProtoM;
                    uint8           IPProtoP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    unsigned int          reserv0     : 20;
                    unsigned int          identSDIPM  : 1;
                    unsigned int          identSDIPP  : 1;
                    unsigned int          HTTPM       : 1;
                    unsigned int          HTTPP       : 1;
                    unsigned int          FOM         : 1;
                    unsigned int          FOP         : 1;
                    unsigned int          IPFlagM     : 3;
                    unsigned int          IPFlagP     : 3;
                    /* word 6 */
                    unsigned int          reserv1;
                } IP;
                struct {
                    /* word 4 */
                    uint8           ICMPTypeM;
                    uint8           ICMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          reserv0;
                    uint8           ICMPCodeM;
                    uint8           ICMPCodeP;
                    /* word 6 */
                    unsigned int          reserv1;
                } ICMP;
                struct {
                    /* word 4 */
                    uint8           IGMPTypeM;
                    uint8           IGMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5,6 */
                    unsigned int          reserv0[2];
                } IGMP;
                struct {
                    /* word 4 */
                    uint8           TCPFlagM;
                    uint8           TCPFlagP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          TCPSPLB;
                    uint16          TCPSPUB;
                    /* word 6 */
                    uint16          TCPDPLB;
                    uint16          TCPDPUB;
                } TCP;
                struct {
                    /* word 4 */
                    uint16          reserv0;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          UDPSPLB;
                    uint16          UDPSPUB;
                    /* word 6 */
                    uint16          UDPDPLB;
                    uint16          UDPDPUB;
                } UDP;
            } is;
        } L3L4;

        struct {
            /* word 0 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 1 */
	     uint16           reserv1:3;
	     uint16		   spaP:9;
             uint16         sMacM3_0:4;
            uint16          sMacP47_32;
	/* word 2 */
	     unsigned int	          reserv3:2;
            unsigned int          sVidM:12;
	     unsigned int          sVidP:12;
	     unsigned int		   reserv2:6;
            /* word 3 */
            unsigned int          reserv5     : 6;
            unsigned int          protoType   : 2;
     	     unsigned int          reserv4        : 24;
		/* word 4 */
            ipaddr_t        sIPP;
            /* word 5 */
            ipaddr_t        sIPM;
            /* word 6 */
            uint16          SPORTLB;
            uint16          SPORTUB;
        } SRC_FILTER;
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
	     uint16 	   vidM:12;	
            uint16          dMacM3_0:4;
            uint16          dMacP47_32;			
            /* word 2 */
	     unsigned int          reserv2:20;
	     unsigned int          vidP:12;			
            /* word 3 */
            unsigned int          reserv4     : 24;
            unsigned int          protoType   : 2;
	     unsigned int          reserv3:6;
            /* word 4 */
            ipaddr_t        dIPP;
            /* word 5 */
            ipaddr_t        dIPM;
            /* word 6 */
            uint16          DPORTLB;
            uint16          DPORTUB;
        } DST_FILTER;

    } is;
    /* word 7 */
    unsigned int          reserv0     : 5;
    unsigned int          pktOpApp    : 3;
    unsigned int          PPPoEIndex  : 3;
    unsigned int          vid         : 3;
    unsigned int          nextHop     : 10; //index of l2, next hop, or rate limit tables
    unsigned int          actionType  : 4;
    unsigned int          ruleType    : 4;
} acl_table_t;

#endif /* RTL865XC */


#endif /*_VLANTABLE_H_*/
