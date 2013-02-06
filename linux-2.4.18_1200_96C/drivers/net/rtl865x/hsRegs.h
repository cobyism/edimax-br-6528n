/*
*-------------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/hsRegs.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
*
* Abstract: This package defines the naming constants for the header stamps
*           hsb (header-stamp-before) and hsa (header-stamp-after).
*
* $Log: hsRegs.h,v $
* Revision 1.1  2007/12/04 11:57:45  joeylin
* add hardware NAT feature
*
* Revision 1.5  2006/07/03 03:05:03  chenyl
* *: convert files from DOS format to UNIX format
*
* Revision 1.4  2006/02/16 05:57:17  yjlou
* *: fix HS compile error in 865xC
*
* Revision 1.3  2004/04/02 06:44:28  jzchen
* Support both A version and B vresion
*
* Revision 1.2  2004/03/31 15:21:37  jzchen
* Remove 8651B flag and all fields using 8650B HSB/HSA definition
*
* Revision 1.1  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/26 04:27:57  cfliu
* move hs.* into table driver directory
*
* Revision 1.1  2003/10/24 13:40:52  orlando
* porting this file for debug command "dhs"
*
* Revision 1.8  2003/09/26 07:16:29  danwu
* add llc_other and url_trap fields in HSB
*
* Revision 1.7  2003/08/26 09:56:12  cfliu
* add cpursn2
*
* Revision 1.6  2003/06/16 03:00:54  cfliu
* update HSA field definition
*
* Revision 1.5  2003/06/13 08:03:43  danwu
* support rtl8651b
*
* Revision 1.4  2002/10/03 12:40:43  henryCho
* add l3ok and l4ok in hs_param_t.
*
* Revision 1.3  2002/09/24 07:29:13  henryCho
* Correct header stamp structure.
*
* Revision 1.2  2002/09/10 07:16:16  danwu
* Debug declaration of hsa_param_t.
*
* Revision 1.1  2002/08/28 03:41:34  orlando
* Header stamp data type declarations.
*
*-------------------------------------------------------------------------
*/

#ifndef _HSREGS_H_
#define _HSREGS_H_

//#include <core/types.h>
#include "types.h"

#undef  HS_NO_HW
//#define   HS_NO_HW

#if defined(CONFIG_RTL865XB)
#ifndef HS_NO_HW
#define HSB_BASE                 0xBC0F0000
#define HSA_BASE                 0xBC0F0040
#else
#define HSB_BASE                 0x80600000
#define HSA_BASE                 0x80600040
#endif /* HS_NO_HW*/
#endif


/* HSB access parameters
*/
typedef struct {

    uint32 sip2_0       : 3;  // w0[31:29]
    uint32 pppoe        : 1;  // w0[28]    
    uint32 vlan         : 1;  // w0[27]    
    uint32 vid          : 12; // w0[26:15]    
    uint32 bc           : 11; // w0[14:4]    
    uint32 spa          : 4;  // w0[3:0]
                       
	uint32 sprt2_0      : 3;  // w1[31:29]
	uint32 sip31_3      : 29; // w1[28:0]	
                       
    uint32 dip18_0      : 19; // w2[31:13]	
	uint32 sprt15_3     : 13; // w2[12:0]
                       
    uint32 ipptl2_0     : 3;  // w3[31:29]
    uint32 dprt         : 16; // w3[28:13]    
    uint32 dip31_19     : 13; // w3[12:0]

	uint8  ethrtype3_0  : 4;  // w4[31:28]
    uint8  prtnmat      : 1;  // w4[27]
    uint8  type         : 3;  // w4[26:24]
    uint8  tcpflg          ;  // w4[23:16]
    uint8  iptos           ;  // w4[15:8]
    uint8  ipflg        : 3;  // w4[7:5]
    uint8  ipptl7_3     : 5;  // w4[4:0]

    uint32 da19_0       : 20; // w5[31:12]    
    uint32 ethrtype15_4 : 12; // w5[11:0]

    uint32 sa3_0        : 4;  // w6[31:28]
    uint32 da47_20      : 28; // w6[27:0]

    uint32 sa35_4           ; // w7[31:0]
    
    uint32 ext_srcPrtL  : 1;
    uint32 ipfragif     : 1;  // w8[30]
    uint32 l4crcok      : 1;  // w8[29]
    uint32 l3crcok      : 1;  // w8[28]
    uint32 dirtx        : 1;  // w8[27]
    uint32 pktend       : 10; // w8[26:17]
    uint32 ttlst        : 2;  // w8[16:15]	
    uint32 udp_nocs     : 1;  // w8[14]
    uint32 llc          : 1;  // w8[13]
    uint32 hp           : 1;  // w8[12]
    uint32 sa47_36      : 12; // w8[11:0]

    uint32 reserved2    : 12; // w9[31:20]
    uint32 pppoesid     : 16; // w9[19:4]
    uint32 urltrap      : 1;  // w9[3]
    uint32 llc_other    : 1;  // w9[2]
    uint32 cpu_l2       : 1;  // w9[1]
    uint32 ext_srcPrtH  : 1;  // w9[0]
    
    uint32 reserved3    : 24; // w10[31:8]
    uint32 hp2          : 1;  // w10[7]
    uint32 dp           : 7;  // w10[6:0]

} hsb_param_t;


/* HSB watch parameters
*/
typedef struct {

    uint32 spa;
    uint32 bc;
    uint32 vid;
    uint32 vlan;
    uint32 pppoe;
    uint8  sip[4];
    uint32 sprt;
                       
    uint8  dip[4];
    uint32 dprt;
    
    uint32 ipptl;
    uint32 ipflg;
    uint32 iptos;
    uint32 tcpflg;
    uint32 type;
    uint32 prtnmat;
	uint32 ethrtype;
    uint8  da[6];
    uint8  pad1[2];
    uint8  sa[6];
    uint8  pad2[2];
    uint32 hp;
    uint32 llc;
    uint32 udp_nocs;
    uint32 ttlst;
    uint32 pktend;
    uint32 dirtx;
    uint32 l4crcok;
    uint32 l3crcok;
    uint32 ipfragif;
    
    uint32 ext_srcPrt;
    uint32 cpu_l2;
    uint32 llc_other;
    uint32 urltrap;
    uint32 pppoesid;

    uint32 dp ;
    uint32 hp2;

    uint32 aDp ;
    uint32 aHp2;

} hsb_param_watch_t;


/* HSA access parameters
*/
typedef struct {

	uint32 mac31_0          ; // w0[31:0]

	uint16 ip15_0           ; // w1[31:16]	
	uint16 mac47_32         ; // w1[15:0]

	uint16 prt              ; // w2[31:16]
	uint16 ip31_16          ; // w2[15:0]

	uint16 l4cs             ; // w3[31:16]
	uint16 l3cs             ; // w3[15:0]
	
	uint32 bc4_0        : 5;  // w4[31:27]
	uint32 dpc          : 3;  // w4[26:24]
	uint32 ttl_1        : 6;  // w4[23:18]
	uint32 pppid        : 3;  // w4[17:15]
	uint32 pppoe        : 2;  // w4[14:13]
	uint32 dvid         : 3;  // w4[12:10]
	uint32 vlan         : 2;  // w4[9:8]
	uint32 llc          : 1;  // w4[7]
	uint32 type         : 3;  // w4[6:4]
	uint32 dirtx        : 1;  // w4[3]
	uint32 l34act       : 1;  // w4[2]
	uint32 l2act        : 1;  // w4[1]	
	uint32 egress       : 1;  // w4[0]

	
	uint32 spa          : 3;  // w5[31:29]
	uint32 cpursn       : 9;  // w5[28:20]
	uint32 svid         : 3;  // w5[19:17]
	uint32 mulcst       : 1;  // w5[16]
//#ifndef RTL8651B
//	uint32 pktend       : 10; // w5[15:6]
//#else
	uint32 extSpa       : 2;  // w5[15:14]
	uint32 dExtPrt      : 4;  // w5[13:10]
	uint32 lastFrag     : 1;  // w5[9]
	uint32 frag         : 1;  // w5[8]
	uint32 l4CrcOk      : 1;  // w5[7]
	uint32 l3CrcOk      : 1;  // w5[6]
//#endif /*RTL8651B*/
	uint32 bc10_5        : 6;  // w5[5:0]
	
	
//#ifdef RTL8651B
  /* word 6 */
	uint32 reserv0      : 23;
	uint32 cpursn2       : 6;//w6[8:3]
	uint32 ext_ttl_1if  : 3;	//w6[2:0]
//#endif /*RTL8651B*/
	
} hsa_param_t;


/* HSA watch parameters
*/
typedef struct {

	uint8  mac[6];
	uint8  pad1[2];
	uint8  ip[4];
	uint32 prt;
	uint32 l3cs;
	uint32 l4cs;
	uint32 egress;
	uint32 l2act;
	uint32 l34act;
	uint32 dirtx;
	uint32 type;
	uint32 llc;
	uint32 vlan;
	uint32 dvid;
	uint32 pppoe;
	uint32 pppid;
	uint32 ttl_1;
	uint32 dpc;									
	uint32 bc;
	uint32 pktend;
	uint32 l3CrcOk;
	uint32 l4CrcOk;
	uint32 frag;
	uint32 lastFrag;
//#ifdef RTL8651B
  uint32 extSpa;
  uint32 dExtPrt;
//#endif /*RTL8651B*/
	uint32 mulcst;
	uint32 svid;
	uint32 cpursn;
	uint32 spa;
	
//#ifdef RTL8651B
	uint32 ext_ttl_1if;
	uint32 cpursn2;
//#endif /*RTL8651B*/

} hsa_param_watch_t;

#endif   /* _HSREGS_H_ */
