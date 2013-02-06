/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : 
* Abstract : 
* Creator : 
* Author :  
* $Id: rtl8651_dos.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
* $Log: rtl8651_dos.h,v $
* Revision 1.1  2007/12/04 11:57:45  joeylin
* add hardware NAT feature
*
* Revision 1.14  2007/05/15 07:53:03  qjj_qin
* *:modify dos control for napt connections
*
* Revision 1.13  2007/05/03 14:05:51  yjlou
* -: remove -Wno-implicit in Makefile. One must declare function first.
*
* Revision 1.12  2006/07/13 15:55:48  chenyl
* *: modify code for RTL865XC driver compilation.
* *: dos2unix process
*
* Revision 1.11  2004/12/14 09:48:09  chenyl
* +: NAPT-Connection Control system
* *: add "NAPT-AddFail" error messages into ignore case.
*
* Revision 1.10  2004/12/07 02:29:43  chenyl
* +: Connection tracking/counting system prototype
*
* Revision 1.9  2004/07/26 14:45:43  chenyl
* +: new logging model
* +: dos Ignore Case
* 	- ignore packets from LAN
* 	- ignore packets from WAN
*
* Revision 1.8  2004/07/12 04:25:36  chenyl
* *: extend existing port scan mechanism
*
* Revision 1.7  2004/07/02 10:30:08  chenyl
* *: bug fix: per-source Flooding check bug (counter error)
* *: bug fix: ip-spoofing ACL add function error
* *: bug fix: ICMP ping-of-death : threshold from 65500 to 65535 (follows cisco)
* *: bug fix: item name change of DOS check: UDP Bomb -> UDP EchoChargen (follows cisco)
* +: new feature: New DoS check item, UDP Bomb
* 		- Triggered if (Length of UDP header) < (Length of IP header)
* 		- allow (Length of UDP header) > (Length of IP header)
* 		- follows cisco
*
* Revision 1.6  2004/06/18 08:11:15  tony
* +: add new features: SYN,FIN,ACK Port Scan.
*
* Revision 1.5  2004/05/21 12:08:50  chenyl
* *: TCP/UDP/ICMP spoof -> check for (sip) == (dip)
* *: modify DOS prevention webpage
*
* Revision 1.4  2004/05/20 12:28:35  chenyl
* *: for fast path packets, only check SYN/FIN/UDP/ICMP flooding DOS
* *: fast path packets don't run Egress ACL
* *: BUG FIX: refresh age-time of UDP flows in forwarding engine
*
* Revision 1.3  2004/05/12 07:20:14  chenyl
* +: source ip blocking
* *: modify dos mechanism
*
* Revision 1.2  2004/03/24 09:50:37  chenyl
* +: Queued pkts to CPU
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.12  2004/01/08 13:30:49  orlando
* api change
*
* Revision 1.11  2004/01/07 03:42:08  orlando
* checkin sercomm phase2 spec change 1st milestone api
*
* Revision 1.10  2004/01/05 13:27:44  orlando
* dos spec change
*
* Revision 1.9  2003/12/03 01:47:16  tony
* move rtl8651_setDosStatus() to rtl8651_dos.c
*
* Revision 1.8  2003/12/02 12:30:45  tony
* the common.h is replace by rtl8651_dos.h and rtl8651_alg_init.h
*
* Revision 1.7  2003/12/02 10:24:47  tony
* Add Routine: support DoS is able to set threshold by user(Webs GUI).
*
* Revision 1.6  2003/10/24 10:25:57  tony
* add DoS attack interactive webpage,
* FwdEngine is able to get WAN status by rtl8651_wanStatus(0:disconnect,1:connect)
*
* Revision 1.5  2003/10/23 08:54:15  tony
* add direction info in DoS functions, fix ipSpoof DoS function.
*
* Revision 1.4  2003/10/23 07:10:04  hiwu
* arrange include file sequence
*
* Revision 1.3  2003/10/17 04:32:47  hiwu
* merge into forwarding engine
*
* Revision 1.2  2003/09/23 04:35:34  hiwu
* add #define _RTL8651_ALG_DOS
*
* Revision 1.1  2003/09/18 03:36:05  hiwu
* initial version
*
*
*
*/

#ifndef _RTL8651_DOS_H
#define _RTL8651_DOS_H

/* bitmask for all dos attacks */
#define RTL8651_DOS_SYNFLOOD				0x00000001
#define RTL8651_DOS_STEALTHFIN			0x00000002
#define RTL8651_DOS_ICMPFLOOD				0x00000004
#define RTL8651_DOS_UDPFLOOD				0x00000008
#define RTL8651_DOS_SCAN					0x00000010
#define RTL8651_DOS_PINGOFDEATH			0x00000020
#define RTL8651_DOS_TEARDROP				0x00000040
#define RTL8651_DOS_SMURF					0x00000080
#define RTL8651_DOS_TCPLAND				0x00000100
#define RTL8651_DOS_UDPLAND				0x00000200
#define RTL8651_DOS_ICMPLAND				0x00000400
#define RTL8651_DOS_IPSPOOF				0x00000800
#define RTL8651_DOS_UDPBOMB				0x00001000
#define RTL8651_DOS_SYNWITHDATA			0x00002000

#define RTL8651_DOS_SRC_SYNFLOOD			0x00004000
#define RTL8651_DOS_SRC_STEALTHFIN		0x00008000
#define RTL8651_DOS_SRC_ICMPFLOOD			0x00010000
#define RTL8651_DOS_SRC_UDPFLOOD			0x00020000
#define RTL8651_DOS_ECHOCHARGEN			0x00040000
#define RTL8651_DOS_PORTSCAN				0x00080000

#define RTL8651_DOS_SRC_TCPCONNOVERFLOW		0x00100000
#define RTL8651_DOS_SRC_UDPCONNOVERFLOW		0x00200000
#define RTL8651_DOS_SRC_TCPUDPCONNOVERFLOW	0x00400000

#define RTL8651_DOS_TCPCONNOVERFLOW		0x00800000
#define RTL8651_DOS_UDPCONNOVERFLOW		0x01000000
#define RTL8651_DOS_TCPUDPCONNOVERFLOW	0x02000000

/* threshold */
/* used for flooding attack */
#define RTL8651_DOS_SYN_THRESHOLD			0x0001
#define RTL8651_DOS_FIN_THRESHOLD				0x0002
#define RTL8651_DOS_UDP_THRESHOLD			0x0003
#define RTL8651_DOS_ICMP_THRESHOLD			0x0004
/* used for connection control system */
#define RTL8651_DOS_TCPCONN_THRESHOLD		0x0008
#define RTL8651_DOS_UDPCONN_THRESHOLD		0x0010
#define RTL8651_DOS_TCPUDPCONN_THRESHOLD	0x0020

/* dos check filter */
#define RTL8651_DOS_IGNORETYPE_FROM_LAN		0x00000001	/* direction related: user can NOT modify this */
#define RTL8651_DOS_IGNORETYPE_FROM_WAN		0x00000002	/* direction related: user can NOT modify this */

/* define dos check type */
#define RTL8651_DOS_SLOW_CHECK		0	/* fully check all items */
#define RTL8651_DOS_FAST_CHECK		1	/* only check syn/fin/udp flood */

/* define port scan default value (used by board.c and dos.c) */
#define RTL8651_DOS_SCAN_MOINTOR_WIN_DEFAULT			5
#define RTL8651_DOS_SCAN_THRESHOLD_DEFAULT				500

/* referenced threshold value */
#define DOS_PORTSCAN_LOW_VAL		1000
#define DOS_PORTSCAN_HIGH_VAL		3

/**********************************
	External Function declaration
***********************************/
/*
	DOS check functions
*/
void rtl8651a_setDosStatus(uint32 dsid,uint32 enableItem);
#define rtl8651_setDosStatus(enableItem) rtl8651a_setDosStatus(0,enableItem)

void rtl8651a_setDosThreshold(uint32 dsid,uint32 type,uint32 threshold);
#define rtl8651_setDosThreshold(type,threshold) rtl8651a_setDosThreshold(0,type,threshold)
void rtl8651a_setPerSrcDosThreshold(uint32 dsid, uint32 type, uint32 threshold);
#define rtl8651_setPerSrcDosThreshold(type,threshold) rtl8651a_setPerSrcDosThreshold(0, type, threshold)

int32 rtl8651_dosOneSecondTimer(void);

int32 rtl8651a_dosTearDropLogging(uint32 dsid, uint16 vid, struct ip* ip, uint8 direction, struct rtl_pktHdr* pktHdr);
int32 rtl8651a_dosFragmentCheck(uint32 dsid, struct ip* ip, struct rtl_pktHdr* pktHdr);
int32 rtl8651a_dosFragUdpBombCheck(uint32 dsid, uint16 vid, struct ip* ip, struct udphdr*ud, uint8 direction, uint32 IpTotalLen, struct rtl_pktHdr* pktHdr);
int32 rtl8651a_dosTcpCheck(uint32 dsid,uint16 vid, struct ip* ip, int32 *ret,uint8 direction,uint32 wanStatus,struct rtl_pktHdr* pktHdr, uint8 checkType);
#define rtl8651_dosTcpCheck(vid,ip,ret,direction,wanStatus, pktHdr, checkType) rtl8651a_dosTcpCheck(vid,ip,ret,direction,wanStatus, pktHdr, checkType)
int32 rtl8651a_dosUdpCheck(uint32 dsid,uint16 vid, struct ip* ip, int32 *ret,uint8 direction, uint32 wanStatus, struct rtl_pktHdr* pktHdr, uint8 checkType);
#define rtl8651_dosUdpCheck(vid,ip,ret,direction,wanStatus,pktHdr, checkType) rtl8651a_dosUdpCheck(0,vid,ip,ret,direction,wanStatus,pktHdr, checkType)
int32 rtl8651a_dosIcmpCheck(uint32 dsid,uint16 vid, struct ip* ip, int32 *ret,uint8 direction, uint32 wanStatus, struct rtl_pktHdr* pktHdr);
#define rtl8651_dosIcmpCheck(vid,ip,ret,direction,wanStatus,pktHdr) rtl8651a_dosIcmpCheck(0,vid,ip,ret,direction,wanStatus,pktHdr)
int32 rtl8651a_dosIpCheck(uint32 dsid,uint16 vid, struct ip* ip, int32 *ret,uint8 direction, uint32 wanStatus, struct rtl_pktHdr* pktHdr);
#define rtl8651_dosIpCheck(vid,ip,ret,direction,wanStatus,pktHdr) rtl8651a_dosIpCheck(0,vid,ip,ret,direction,wanStatus,pktHdr)
/*
	Connection Count Control system
*/
void rtl8651_dumpConnCtrl(void);
/*
	DOS Scan
*/
int32 rtl8651_dosScanTrackingSetPara(uint32 dsid, uint32 windowSize, uint32 threshold);
/*
	DOS process functions
*/
uint32 rtl8651_dosSIPBlockSw(uint32 item, uint32 dsid, uint32 *flag);
void rtl8651_dosProc_blockSip_freeAll(void);
int32 rtl8651_dosProc_blockSip_setPrisonTime(uint32 prisonTime);
void rtl8651_dosProc_blockSip_enable(uint32 dsid, uint32 enable);

#ifdef NEW_DOS_RULE
int32 _rtl8651_getSrcDosRecord(ipaddr_t ip, uint32 *tcpCount, uint32 *udpCount, uint32 *tcpUdpCount);
#endif

void rtl8651_dosProc_blockSip_dumpJail(void);


#endif /* _RTL8651_DOS_H */
