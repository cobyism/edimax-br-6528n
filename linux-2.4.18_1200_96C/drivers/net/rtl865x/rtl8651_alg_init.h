/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : 
* Abstract : 
* Creator : 
* Author :  
* $Id: rtl8651_alg_init.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
* $Log: rtl8651_alg_init.h,v $
* Revision 1.1  2007/12/04 11:57:45  joeylin
* add hardware NAT feature
*
* Revision 1.14  2006/09/15 09:38:52  darcy_lu
* *:add code for distribute algtype
*
* Revision 1.13  2005/12/21 09:57:08  hao_yu
* *: add declaration of alg_QosRatio[]
*
* Revision 1.12  2005/11/15 16:11:06  chenyl
* +: ALG for TFTP (RFC 1350)
*
* Revision 1.11  2005/10/04 15:31:59  jzchen
* *: Add ratio parameter in rtl8651_setAlgQosQueueId and rtl8651a_setAlgQosQueueId
*
* Revision 1.10  2005/03/01 08:20:34  tony
* *: support PPTP/IPSec ALG config dividedly.
*
* Revision 1.9  2004/12/22 05:19:15  tony
* +: support new features: make ALG(FTP,SIP,NetMeeting) support MNQueue.
*
* Revision 1.8  2004/09/16 11:50:20  tony
* +: add PPTP pass-throught software (not change call ID) case.
*
* Revision 1.7  2004/09/03 03:00:09  chhuang
* +: add new feature: pseudo VLAN
*
* Revision 1.6  2004/07/23 13:19:57  tony
* *: remove all warning messages.
*
* Revision 1.5  2004/04/20 03:44:02  tony
* if disable define "RTL865X_OVER_KERNEL" and "RTL865X_OVER_LINUX", __KERNEL__ and __linux__ will be undefined.
*
* Revision 1.4  2004/03/19 08:12:53  tony
* make IPSec, PPTP, L2TP are able to set session numbers dynamically by user.
*
* Revision 1.3  2004/03/18 09:22:26  tony
* add alg reinit function, it will be called when fwdEng ReInit.
*
* Revision 1.2  2004/03/12 07:38:21  tony
* support for new ALGs
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.2  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.21  2004/02/12 06:25:17  rupert
* Rename pppoeId to SessionID
* Fixed  rtl8651_flushPolicyRoute about dynamic route
*
* Revision 1.20  2004/02/11 05:55:42  tony
* add rtl8651a_flushAlgServerIn(dsid) and  rtl8651_flushAlgServerIn()
* :It can disable all the server ports of ALG server incoming call.
*
* Revision 1.19  2004/02/06 11:42:15  tony
* add _rtl8651_DefaultDialSessionId
*
* Revision 1.18  2004/01/29 09:40:51  tony
* add return value in  rtl8651a_setAlgStatus(when the session isn't up, return NOT_OK)
*
* Revision 1.17  2004/01/28 07:58:25  tony
* make DirectX7 ALG support Server-In function.
*
* Revision 1.16  2004/01/16 12:19:18  tony
* add rtl8651_getAlgStatus() and rtl8651a_getAlgStatus(),
* make setAlgStatus() support server in by server port.
*
* Revision 1.15  2004/01/09 08:03:21  tony
* make the code architecture of ALG is able to support multiple dial session.
*
* Revision 1.14  2004/01/07 07:37:54  tony
* Support multiple session UI plugin for ALG.
*
* Revision 1.13  2003/12/31 04:59:26  tony
* fix bug: turn L2TP ALG on/off dynamically.
*
* Revision 1.12  2003/12/08 13:56:51  tony
* add new ALG: support SIP protocol (for single session)
*
* Revision 1.11  2003/12/08 03:37:26  tony
* add new ALG: L2TP v2 multiple-session
*
* Revision 1.10  2003/12/02 12:30:45  tony
* the common.h is replace by rtl8651_dos.h and rtl8651_alg_init.h
*
* Revision 1.9  2003/12/01 12:35:52  tony
* make ALG is able to be configured by users(Web GUI).
*
* Revision 1.8  2003/12/01 06:20:20  tony
* support dynamic load alg.
*
* Revision 1.7  2003/12/01 06:14:00  tony
* remove old irc alg
*
* Revision 1.6  2003/10/23 07:10:18  hiwu
* add pptp alg support
*
* Revision 1.5  2003/10/02 10:39:52  hiwu
* fix header conflict problem
*
* Revision 1.4  2003/10/02 07:05:41  hiwu
* merge with new table driver
*
* Revision 1.3  2003/09/23 04:35:13  hiwu
* add #define _RTL8651_ALG_INIT
*
* Revision 1.2  2003/09/18 03:35:46  hiwu
* dos2unix file formant tralslation
*
* Revision 1.1  2003/09/17 06:45:57  hiwu
* initial version
*
*
*
*/

#ifndef _RTL8651_ALG_INIT
#define _RTL8651_ALG_INIT
#include "rtl8651_tblDrv.h"

#define RTL8651_ALG_PPTP_IDX				0
#define RTL8651_ALG_IPSEC_IDX				1
#define RTL8651_ALG_L2TP_IDX				2
#define RTL8651_ALG_FTP_IDX				3
#define RTL8651_ALG_NETMEETING_IDX		4
#define RTL8651_ALG_ICQ_IDX				5
#define RTL8651_ALG_YAHOOMSG_IDX			6
#define RTL8651_ALG_IRC_IDX				7
#define RTL8651_ALG_DX7_IDX				8
#define RTL8651_ALG_SIP_IDX				9
#define RTL8651_ALG_ICUII_IDX				10
#define RTL8651_ALG_VDOLIVE_IDX			11
#define RTL8651_ALG_QUAKE_IDX				12
#define RTL8651_ALG_CS_IDX					13
#define RTL8651_ALG_BATTLENET_IDX			14
#define RTL8651_ALG_REALAUDIO_IDX			15
#define RTL8651_ALG_CUSEEME_IDX			16
#define RTL8651_ALG_TFTP_IDX				17
#define RTL8651_ALG_MAX_IDX				18	/* MAX ALG functions */

#define RTL8651_ALG_PPTP_SERVER_IDX		29	/* for client / server setting divided */
#define RTL8651_ALG_IPSEC_SERVER_IDX		30
#define RTL8651_ALG_PPTP_SOFTWARE_IDX	31	/* special define for PPTP software */


#define RTL8651_ALG_PPTP				0x00000001
#define RTL8651_ALG_IPSEC				0x00000002
#define RTL8651_ALG_L2TP				0x00000004
#define RTL8651_ALG_FTP				0x00000008
#define RTL8651_ALG_NETMEETING		0x00000010
#define RTL8651_ALG_ICQ					0x00000020
#define RTL8651_ALG_YAHOOMSG			0x00000040
#define RTL8651_ALG_IRC					0x00000080
#define RTL8651_ALG_DX7				0x00000100
#define RTL8651_ALG_SIP					0x00000200
#define RTL8651_ALG_ICUII				0x00000400
#define RTL8651_ALG_VDOLIVE			0x00000800
#define RTL8651_ALG_QUAKE				0x00001000
#define RTL8651_ALG_CS					0x00002000	/* Counter-Strike /Half Life */
#define RTL8651_ALG_BATTLENET			0x00004000	/* Blizzard Battlenet: StarCraft , Diablo II */
#define RTL8651_ALG_REALAUDIO			0x00008000
#define RTL8651_ALG_CUSEEME			0x00010000
#define RTL8651_ALG_TFTP				0x00020000

#define RTL8651_ALG_PPTP_SERVER		0x20000000	/* special define for PPTP server-in-LAN */
#define RTL8651_ALG_IPSEC_SERVER		0x40000000	/* special define for IPSec server-in-LAN */
#define RTL8651_ALG_PPTP_SOFTWARE	0x80000000	/* special define for PPTP software */

#define _ALG_TYPE_CLIENT	 	1	/* support ALG client */
#define _ALG_TYPE_SERVER 		2	/* support ALG server */
#define _ALG_TYPE_SERVERPORT	4	/* add ALG server port */

#define _ALG_TYPE_NAT           		8	/*support nat alg*/    
#define _ALG_TYPE_NAPT         		16	/*support napt alg*/ 
#define _ALG_TYPE_PORTBOUNCING   	32	/*support port bouncing alg*/

extern uint32 alg_supports[RTL8651_MAX_DIALSESSION];
extern uint32 alg_serverips[RTL8651_MAX_DIALSESSION][32];
extern uint32 alg_qosqid[RTL8651_MAX_DIALSESSION][32];
extern uint32 alg_qoshl[RTL8651_MAX_DIALSESSION][32];
extern uint32 alg_qosRatio[RTL8651_MAX_DIALSESSION][32];
extern uint32 _rtl8651_DefaultDialSessionId;

struct rtl8651_algMaxSession
{
	uint8 l2tpMaxSession;
	uint8 pptpMaxSession;
	uint8 ipsecMaxSession;
};



extern struct rtl8651_algMaxSession algMaxSession;


void rtl8651_setAlgStatus(uint32 alg_support,ipaddr_t *alg_serverip);
uint32 rtl8651a_setAlgStatus(uint32 SessionID,uint32 alg_support,ipaddr_t *alg_serverip);
void rtl8651_setAlgConfig(uint32 alg_support,ipaddr_t *alg_serverip,struct rtl8651_algMaxSession maxSession);
uint32 rtl8651a_setAlgConfig(uint32 SessionID,uint32 alg_support,ipaddr_t *alg_serverip,struct rtl8651_algMaxSession maxSession);
int32 rtl8651_algOneSecondTimer(void);
uint32 rtl8651_getAlgStatus(void);
uint32 rtl8651a_getAlgStatus(uint32 SessionID);
uint32 rtl8651_flushAlgServerIn(void);
uint32 rtl8651a_flushAlgServerIn(uint32 SessionID);
void rtl8651_algReInit(void);
int32 rtl8651_setAlgQosQueueId(uint32 alg_idx,uint32 queueID,uint32 isHigh, uint32 ratio);
int32 rtl8651a_setAlgQosQueueId(uint32 SessionID,uint32 alg_idx,uint32 queueID,uint32 isHigh, uint32 ratio);
int32 _rtl8651_addAlgQosNaptConnection(int8 fromDrv, int16 assigned, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort,uint32 AlgIdx);

#endif /* _RTL8651_ALG_INIT */


