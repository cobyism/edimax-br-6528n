/*
 * Copyright c                Realtek Semiconductor Corporation, 2003
 * All rights reserved.                                                    
 * 
 * $Header: /cvs/AP/rtl865x/linux-2.4.18/include/asm-mips/rtl865x/re865x.h,v 1.1.1.1 2007/08/06 10:04:57 root Exp $
 *
 * $Author: root $
 *
 * Abstract:
 *
 *   re865x.h  -- ioctl symbol definitions
 *
 * $Log: re865x.h,v $
 * Revision 1.1.1.1  2007/08/06 10:04:57  root
 * Initial import source to CVS
 *
 * Revision 1.76  2007/04/23 13:05:40  bo_zhao
 * +: add new qos support
 *
 * Revision 1.75  2007/04/04 03:44:39  alva_zhang
 * +: support rtl865xC MIB Couter function
 *
 * Revision 1.74  2007/04/04 02:29:03  bo_zhao
 * *: update MNQueue
 *
 * Revision 1.73  2007/03/28 03:04:56  bo_zhao
 * :Add 865xC MNQueue
 *
 * Revision 1.72  2007/03/12 05:40:34  darcy_lu
 * +: add macro definition for get l2 table
 *
 * Revision 1.71  2007/03/06 06:56:06  chihhsing
 * +: add new IOCTRL for port-base VLAN tagged/untagged
 *
 * Revision 1.70  2007/03/05 06:07:18  qjj_qin
 * *:865xc igmp snooping support
 *
 * Revision 1.69  2007/02/14 09:23:21  qjj_qin
 * *:macro for igmp snooping
 *
 * Revision 1.68  2007/02/13 07:37:44  qjj_qin
 * +:igmp proxy to snooping
 *
 * Revision 1.67  2007/01/08 01:57:27  hyking_liu
 * +: add tag RTL8651_IOCTL_SETMULTIPPPOESESSIONSTATUS for idle time out
 *
 * Revision 1.66  2006/12/28 02:45:07  hyking_liu
 * +: add tag SETNAPTTCPUDPPORTRANGE
 *
 * Revision 1.65  2006/12/05 05:56:59  qjj_qin
 * *:add SDK API to specify detail multicast upstream
 *
 * Revision 1.64  2006/11/28 02:53:47  hyking_liu
 * +: Add Micro for ioctl_setvlanPromiscuous
 *
 * Revision 1.63  2006/11/13 06:36:25  qjj_qin
 * +:igmp proxy for multi-port wan port
 *
 * Revision 1.62  2006/10/25 07:11:58  qy_wang
 * +:add an IOCTRL function to get extension device name & vid
 *
 * Revision 1.61  2006/08/16 08:33:31  cw_du
 * +:Add #define RTL8651_IOCTL_ENABLE_UNNUMBERNAPTPROC	2797.
 *
 * Revision 1.60  2006/08/15 05:38:56  hyking_liu
 * +: Add FLUSHGENERICDMZFILTER for flush dmzfilter.
 *
 * Revision 1.59  2006/08/14 05:25:44  hyking_liu
 * *: Modify for DMZ ipFilter API
 *
 * Revision 1.58  2006/08/08 08:56:29  hyking_liu
 * +: add define of IOCTL_ADDGENERICDMZHOST
 *
 * Revision 1.57  2006/08/07 07:20:52  jiucai_wang
 * +:add RTL8651_IOCTL_MANUALKEYFLAGSEND for ipsec manual key mode
 *
 * Revision 1.56  2006/08/02 02:20:45  hyking_liu
 * +: Add the define of RTL8651_IOCTL_ADDDMZHOSTEXT used in re_core.c
 *
 * Revision 1.55  2006/05/30 02:13:06  jiucai_wang
 * +:add the micro declaration for reinit route list(line 2795)
 *
 * Revision 1.54  2006/05/24 09:45:45  jiucai_wang
 * +:add some micro definition(line:541-546)
 *
 * Revision 1.53  2006/04/13 05:54:58  chenyl
 * +: KERNELPROC for kernel procdure testing
 *
 * Revision 1.52  2006/03/16 03:12:37  yjlou
 * +: Support various NAT Cone Type:
 *    Naive, Symmetric, Restricted, Port-Restricted, and Full Cone.
 *
 * Revision 1.51  2006/01/27 07:10:48  shliu
 * *: add definition of RTL8651_IOCTL_SETURLFORWARDLOGGING
 *
 * Revision 1.50  2006/01/26 16:04:49  shliu
 * *: define RTL8651_IOCTL_FLUSHURLFILTERRULE
 *
 * Revision 1.49  2006/01/25 14:53:07  yjlou
 * *: fixed the bug of pbnat(Protocol-Base NAT): auto enable when any entry is enabled.
 *
 * Revision 1.48  2006/01/24 13:52:52  shliu
 * *: add ioctl definition RTL8651_IOCTL_SETURLDEFAULTACTION
 *
 * Revision 1.47  2006/01/23 02:05:40  shliu
 * *: add RTL8651_IOCTL_URLUNKNOWNTYPECASESENSITIVE
 *
 * Revision 1.46  2006/01/23 01:57:15  shliu
 * *: add RTL8651_IOCTL_ADDGENERICURLFILTERRULE & RTL8651_IOCTL_DELGENERICURLFILTERRULE
 *
 * Revision 1.45  2005/12/12 03:25:00  tony
 * +: New Feature: support trusted user in url filter.
 *
 * Revision 1.44  2005/12/02 15:41:11  chenyl
 * *: circular buffer write protection by kernel mutex lock / unlock
 *
 * Revision 1.43  2005/12/02 10:04:46  chenyl
 * +: capture Log buffer's snapshot to user space memory under mutex lock/unlock protection.
 * *: shliu's new circ_msg write mechanism.
 *
 * Revision 1.42  2005/11/21 12:40:55  chenyl
 * *: always turn OFF interrupts when R/W flash
 *
 * Revision 1.41  2005/10/26 12:54:40  shliu
 * *: add RTL8651_IOCTL_REPROBE
 *
 * Revision 1.40  2005/10/26 04:57:45  shliu
 * *: new API for deletion of proto stack action table entry
 *
 * Revision 1.39  2005/09/25 17:02:11  shliu
 * *: add definitions for mac filter APIs
 *
 * Revision 1.38  2005/09/09 06:11:05  shliu
 * *: add an API of add generic proto stack action
 *
 * Revision 1.37  2005/09/07 09:54:21  chenyl
 * +: set pure-sw forwarding via http-daemon
 *
 * Revision 1.36  2005/08/04 06:50:15  shliu
 * *: add getChipVersion system call
 *
 * Revision 1.35  2005/08/03 07:35:07  rupert
 * +: Add IOCTL for rtl8651a_addAclRuleExt and rtl8651a_delAclRuleExt
 *
 * Revision 1.34  2005/07/29 09:44:00  chenyl
 * +: dns module in ROMEDRV
 * +: domain blocking (SDK + RomeDrv)
 *
 * Revision 1.33  2005/06/07 14:44:50  shliu
 * *: add rtl8651_addDriverNaptMapping() API.
 *
 * Revision 1.32  2005/06/01 07:10:22  tony
 * *:add a addarp ioctl
 *
 * Revision 1.31  2005/05/04 02:42:00  shliu
 * *: define reinit ROMEDRV for ioctl case
 *
 * Revision 1.30  2005/01/18 13:33:37  ghhuang
 * *: Replace Diffserv API
 *
 * Revision 1.29  2005/01/18 08:50:22  ghhuang
 * +: Diffserv (DSCP) remarking support
 *
 * Revision 1.28  2005/01/11 03:22:55  ghhuang
 * *:MNQ bug fix - Correct ACL undeletion
 *
 * Revision 1.27  2005/01/03 02:52:22  tony
 * +: add API: rtl8651_queryUpnpMapTimeAge
 *
 * Revision 1.26  2004/12/22 08:41:49  tony
 * *: support for alg qos
 *
 * Revision 1.25  2004/12/02 03:08:37  yjlou
 * +: support IPX/Netbios Passthru function
 *
 * Revision 1.24  2004/11/01 08:22:21  tony
 * *: support for rtl8651_addProtoStackServerPortRange
 *
 * Revision 1.23  2004/10/21 11:48:39  chenyl
 * +: web-based icmp/l4 protocol DMZ forwarding
 *
 * Revision 1.22  2004/10/11 05:20:27  yjlou
 * +: add RTL8651_IOCTL_ENABLE_DROP_UNKNOWN_PPPOE_DROP
 *
 * Revision 1.21  2004/10/05 09:20:42  chenyl
 * +: web page to turn ON/OFF ip-multicast system
 *
 * Revision 1.20  2004/10/01 08:37:32  yjlou
 * +: RTL8651_IOCTL_ENABLE_PPPOE_PASSTHRU and RTL8651_IOCTL_ENABLE_IPV6_PASSTHRU
 *
 * Revision 1.19  2004/09/23 08:29:33  tony
 * new features: when wan port's link is changed, the DHCPC will auto renew.
 *
 * Revision 1.18  2004/09/15 14:52:19  chenyl
 * *: enable multicast upload cache
 * *: modify PPTP's default MTU
 *
 * Revision 1.17  2004/09/03 03:00:09  chhuang
 * +: add new feature: pseudo VLAN
 *
 * Revision 1.16  2004/09/02 06:15:38  chenyl
 * *: multicast :
 * 	- when turning-on ip-multicast, periodic query will be sent immediately
 * +: tbldrv
 * 	- add options when run addNaptConnection
 * 		- PURE_SW: pure software entry
 * 		- DONT_CHECK_INTIP: ignore internal IP checking
 * 		- TO_PROTOCOL_STACK: this napt packet must be trapped to protocol stack
 * +: forwarding engine:
 * 	- protocol stack TCP/UDP flow cache
 * 	- napt redirect register/unregister/query
 *
 * Revision 1.15  2004/08/20 12:29:17  cfliu
 * +: Add loose UDP ctrl item
 *
 * Revision 1.14  2004/08/18 05:39:59  chenyl
 * +: napt special option web-based setting
 *
 * Revision 1.13  2004/07/27 10:48:52  cfliu
 * *: Fix all compile warnings
 *
 * Revision 1.12  2004/07/27 03:06:16  chenyl
 * +: Add DoS ignore type setting ioctl items
 *
 * Revision 1.11  2004/07/19 02:12:26  chhuang
 * *: modify M-N-1 queue
 *
 * Revision 1.10  2004/07/14 13:55:59  chenyl
 * +: web page for MN queue
 *
 * Revision 1.9  2004/07/12 04:25:36  chenyl
 * *: extend existing port scan mechanism
 *
 * Revision 1.8  2004/07/08 10:58:40  tony
 * +: add new ioctl api.
 *
 * Revision 1.7  2004/07/06 07:22:13  chhuang
 * +: add rate limit
 *
 * Revision 1.6  2004/06/29 07:37:13  chenyl
 * +: igmp-proxy filter
 *
 * Revision 1.5  2004/06/21 03:49:17  chhuang
 * +: define RTL8651_IOCTL_NETMTU
 *
 * Revision 1.4  2004/06/16 04:43:11  tony
 * *: support reboot and get loader version for MMU.
 *
 * Revision 1.3  2004/06/14 09:05:40  rupert
 * *: serial definition
 *
 * Revision 1.58  2004/06/11 11:07:17  cfliu
 * +: Add Cable meter and Interface counter webpage
 *
 * Revision 1.57  2004/05/31 14:41:04  jzchen
 * Add Counter setting IO control definition
 *
 * Revision 1.56  2004/05/28 06:11:51  yjlou
 * +: add IOCTL for rtl8651_addProtocolBasedNAT() and rtl8651_delProtocolBasedNAT()
 *
 * Revision 1.55  2004/05/27 05:14:18  cfliu
 * Remove INIT_HARDWARE
 *
 * Revision 1.54  2004/05/20 08:45:49  chhuang
 * add Web Page for QoS
 *
 * Revision 1.53  2004/05/19 08:49:22  orlando
 * add RTL8651_IOCTL_DIAG_LED
 *
 * Revision 1.52  2004/05/17 07:22:16  tony
 * fix pppoe silent timeout bug and make PPTP/L2TP support silent timeout.
 *
 * Revision 1.51  2004/05/14 10:18:15  orlando
 * add RTL8651_IOCTL_DIAG_LED
 *
 * Revision 1.50  2004/05/12 07:20:15  chenyl
 * +: source ip blocking
 * *: modify dos mechanism
 *
 * Revision 1.49  2004/05/10 05:53:45  chhuang
 * add new IOCTL: RTL8651_IOCTL_ADDURLFILTER
 *
 * Revision 1.48  2004/05/05 08:27:07  tony
 * new features: add remote management spec
 *
 * Revision 1.47  2004/04/29 05:47:52  chhuang
 * add new ioctl for rtl8651_addNaptMappingExt()
 *
 * Revision 1.46  2004/04/27 07:05:02  chenyl
 * +: ioctl number: RTL8651_IOCTL_SETDEFAULTIGMPUPSTREAM
 *
 * Revision 1.45  2004/04/19 13:45:18  tony
 * add resetPptpProprity and resetL2tpProprity IOCTL.
 *
 * Revision 1.44  2004/04/14 02:45:50  tony
 * add ioctl RTL8651_IOCTL_SETLOOPBACKPORTPHY
 *
 * Revision 1.43  2004/04/13 08:15:53  tony
 * fix bug: make PPTP/L2TP run stably.
 *
 * Revision 1.42  2004/04/09 02:26:12  tony
 * remove rtl8651_specifyNetworkIntfLinkLayerType_1 IOCTL.
 *
 * Revision 1.41  2004/04/08 13:18:12  tony
 * add PPTP/L2TP routine for MII lookback port.
 *
 * Revision 1.40  2004/04/08 12:25:42  cfliu
 * Add del ACL ioctl()
 *
 * Revision 1.39  2004/02/24 05:12:33  cfliu
 * Add symbols required for WLAN HW acceleration code
 *
 * Revision 1.38  2004/02/10 13:00:34  orlando
 * add lanType field in pppoeCfg_t
 *
 * Revision 1.37  2004/02/09 09:18:00  tony
 * add function: user is able to add protocol stack Server used TCP ports by
 * rtl8651_addProtoStackServerUsedTcpPort(incoming_src_port); and flush the
 * table by rtl8651_flushProtoStackServerUsedTcpPort();
 *
 * Revision 1.36  2004/02/04 06:24:19  rupert
 * Add UDP IOCTL
 *
 * Revision 1.35  2004/01/29 09:38:02  tony
 * add case RTL8651_IOCTL_ADDNAPTSERVERPORTRANGE ,RTL8651_IOCTL_DELNAPTSERVERPORTRANGE
 *
 * Revision 1.34  2004/01/29 02:03:34  orlando
 * correct duplicate conflicts
 *
 * Revision 1.33  2004/01/27 02:03:04  tony
 * add RTL8651_IOCTL_FLUSHNAPTSERVERPORTBYEXTIP
 *
 * Revision 1.32  2004/01/12 12:32:23  orlando
 * add RTL8651_IOCTL_SETPPPOEDEFAULTDIALSESSIONID
 *
 * Revision 1.31  2003/12/12 02:40:44  tony
 * add rtl8651_setMtu() function.
 *
 * Revision 1.30  2003/12/10 03:08:58  tony
 * make logging function support time zone in kernel space.
 *
 * Revision 1.29  2003/12/08 13:53:25  orlando
 * add RTL8651_IOCTL_ENABLELOGGING
 *
 * Revision 1.28  2003/12/04 10:39:02  hiwu
 * add RTL8651_ADDNAPTUPNPPORTMAPPING
 * add RTL8651_DELNAPTUPNPPORTMAPPING
 *
 * Revision 1.27  2003/12/03 14:29:07  cfliu
 * comment out old debug commands when CLE is enabled.
 *
 * Revision 1.26  2003/12/02 10:24:47  tony
 * Add Routine: support DoS is able to set threshold by user(Webs GUI).
 *
 * Revision 1.25  2003/12/01 12:35:52  tony
 * make ALG is able to be configured by users(Web GUI).
 *
 * Revision 1.24  2003/11/28 01:38:17  rupert
 * add PPPOEUP_CFG PPPOEDOWN_CFG BATCH IOCTL
 *
 * Revision 1.23  2003/11/21 01:56:47  tony
 * add function: user is able to add protocol stack used UDP ports by
 * rtl8651_addProtoStackUsedUdpPort(incoming_src_port);
 *
 * Revision 1.22  2003/11/18 13:28:28  rupert
 * add ioctl for dial demand and timeout
 *
 * Revision 1.21  2003/11/18 09:04:47  tony
 * add routine: support mtu configure by user in pppoe.
 *
 * Revision 1.20  2003/11/06 12:15:56  rupert
 * add delete policy route  ioctl
 *
 * Revision 1.19  2003/10/30 10:20:49  orlando
 * add definitions for command "addflow"
 *
 * Revision 1.18  2003/10/28 09:58:35  rupert
 * add policy route
 *
 * Revision 1.17  2003/10/28 08:32:36  orlando
 * add RTL8651_IOCOL_ENABLENAPTAUTOADD
 *
 * Revision 1.16  2003/10/24 13:37:42  orlando
 * add debug command "dhs","dftcp","dficmp"
 *
 * Revision 1.15  2003/10/24 10:25:58  tony
 * add DoS attack interactive webpage,
 * FwdEngine is able to get WAN status by rtl8651_wanStatus(0:disconnect,1:connect)
 *
 * Revision 1.14  2003/10/20 10:13:08  orlando
 * add define for debug command "dl2","dgidx","dfwd"
 *
 * Revision 1.13  2003/10/20 04:52:11  orlando
 * add _RTL_LOGGING related stuff
 *
 * Revision 1.12  2003/10/09 13:23:44  tony
 * TRIGGER_PORT: add function rtl8651_flushTriggerPortRules,rtl8651_addTriggerPortRule
 *
 * Revision 1.11  2003/10/01 09:43:12  orlando
 * add RTL8651_IOCTL_SETPPPOESESSIONHANGUP
 *
 * Revision 1.10  2003/10/01 06:17:57  orlando
 * add cvs file header
 *
 */

#ifndef _LINUX_IF_RE865X_H
#define _LINUX_IF_RE865X_H

#include <linux/types.h>
#include <linux/config.h>
#define RE865X_VERSION 1

#define GET_VERSION         0
#define ADD_VLAN            2
#define DEL_VLAN            3
#define ADD_IF              5
#define SET_VLAN_IP         6
#define SET_NET_PARA        7
#define ADD_GW	            8
#define GET_IF              9
#define CON_IFF             10
#define GET_MAC             11
#define SET_EXT_INTERFACE	12 
#define GET_VLAN_STAT	    13
#define DEL_VLAN_IP	        14
#define SET_VLAN_TYPE       15
#define GET_IP	            16
#define SET_VLAN_PPPOE_PARA 17
#define SET_DNS_GW          18
#define GET_PPPOE_STATUS	19
#define DEL_PPPOE	        20
#define SET_PPPOE_STATUS	21
#define SET_PPPOE_IP		22
#define SET_PPPD_IDLE_TIME	23
#define GET_PPPD_IDLE_TIME	24

#define RTL8651_FLUSHACLRULE             50
#define RTL8651_SETDEFAULTACL            51
#define RTL8651_ADDACL                   52
#define RTL8651_ADDNAPTSERVERPORTMAPPING 53
#define RTL8651_DELNAPTSERVERPORTMAPPING 54
#define SET_SOFTPPPD_IDLE_TIME		 55
#define ADD_SIGNAL_RECEIVE_PROCESS	 56
#define ADD_DEMAND_ROUTE		 57
#define PPPOEDOWN_CFG			58
#define PPPOEUP_CFG			59
#define RTL8651_ADDNAPTUPNPPORTMAPPING 60
#define RTL8651_DELNAPTUPNPPORTMAPPING 61
#define RTL8651_DELACL                   62
#define RTL8651_IOCTL_DIAG_LED  63


/* start new naming conventions 2003/09/16 */
#define RTL8651_IOCTL_SETPPPOESESSIONPROPERTY     2000
#define RTL8651_IOCTL_RESETPPPOESESSIONPROPERTY   2001
#define RTL8651_IOCTL_BINDPPPOESESSION            2002
#define RTL8651_IOCTL_ADDPPPOESESSION             2003
#define RTL8651_IOCTL_DELPPPOESESSION             2004
#define RTL8651_IOCTL_ADDIPINTF                   2005
#define RTL8651_IOCTL_DELIPINTF                   2006
#define RTL8651_IOCTL_ADDNAPTMAPPING              2007
#define RTL8651_IOCTL_DELNAPTMAPPING              2008
#define RTL8651_IOCTL_ADDEXTNETWORKINTERFACE      2009
#define RTL8651_IOCTL_ADDROUTE                    2010
#define RTL8651_IOCTL_DELROUTE                    2011
#define RTL8651_IOCTL_ADDNATMAPPING               2014
#define RTL8651_IOCTL_DELNATMAPPING               2015
#define RTL8651_IOCTL_ADDNATMAPPING               2014
#define RTL8651_IOCTL_DELNATMAPPING               2015
#define RTL8651_IOCTL_ADDNETWORKINTF              2016
#define RTL8651_IOCTL_DELNETWORKINTF              2017
#define RTL8651_IOCTL_SPECIFYNETWORKINTFLINKLAYERTYPE 2018
#define RTL8651_IOCTL_SETASICNAPTAUTOADDDELETE    2019
#define RTL8651_IOCTL_ADDIPUNNUMBERED             2020
#define RTL8651_IOCTL_DELIPUNNUMBERED             2021
#define RTL8651_IOCTL_SETLANSIDEEXTERNALIPINTERFACE 2022
#define RTL8651_IOCTL_ADDDMZHOST		2023
#define RTL8651_IOCTL_DELDMZHOST	        2024
#define RTL8651_IOCTL_ADDURLFILTERSTRING		  2025
#define RTL8651_IOCTL_DELURLFILTERSTRING		  2026
#define RTL8651_IOCTL_SETPPPOESESSIONHANGUP		  2027
#define RTL8651_IOCTL_FLUSHTRIGGERPORTRULES		  2028
#define RTL8651_IOCTL_ADDTRIGGERPORTRULE		  2029
#define RTL8651_IOCTL_INSTALLLOGGINGFUNCTION      2030
#define RTL8651_IOCTL_SETWANSTATUS	              2031
#define RTL8651_IOCTL_SETDOSSTATUS	              2032
#define RTL8651_IOCTL_ENABLENAPTAUTOADD           2033
#define RTL8651_IOCTL_ADDNAPTCONNECTION           2034
#define RTL8651_IOCTL_ADDPOLICYROUTE	          2035
#define RTL8651_IOCTL_FLUSHPOLICYROUTE	          2036
#define RTL8651_IOCTL_SETPPPOEMTU	              2037
#define RTL8651_IOCTL_ADDDEMANDROUTE	          2038
#define RTL8651_IOCTL_FLUSHDEMANDROUTE	          2039
#define RTL8651_IOCTL_FLUSHPROTOSTACKUSEDUDPPORTS 2040
#define RTL8651_IOCTL_ADDPROTOSTACKUSEDUDPPORT	  2041
#define RTL8651_IOCTL_SETALGSTATUS                2042
#define RTL8651_IOCTL_SETDOSTHRESHOLD	          2043
#define RTL8651_IOCTL_ENABLELOGGING  	          2044
#define RTL8651_IOCTL_SETTIMEZONE				  2045
#define RTL8651_IOCTL_SETMTU	                  2046
#define RTL8651_IOCTL_SETPPPOEDEFAULTDIALSESSIONID 2047
#define RTL8651_IOCTL_FLUSHNAPTSERVERPORTBYEXTIP  2048
#define RTL8651_IOCTL_ADDNAPTSERVERPORTRANGE      2049
#define RTL8651_IOCTL_DELNAPTSERVERPORTRANGE      2050
#define RTL8651_IOCTL_SETUDPSIZETHRESH            2051
#define RTL8651_IOCTL_FLUSHPROTOSTACKSERVERUSEDTCPPORTS 2052
#define RTL8651_IOCTL_ADDPROTOSTACKSERVERUSEDTCPPORT 2053
#define RTL8651_IOCTL_SETLOOPBACKPORT				2054
#define RTL8651_IOCTL_ADDSESSION					2055
#define RTL8651_IOCTL_DELSESSION					2056
#define RTL8651_IOCTL_BINDSESSION					2057
#define RTL8651_IOCTL_SETPPTPPROPERTY				2059
#define RTL8651_IOCTL_SETL2TPPROPERTY				2060
#define RTL8651_IOCTL_SETLOOPBACKPORTPHY			2061
#define RTL8651_IOCTL_SETDEFAULTIGMPUPSTREAM		2062
#define RTL8651_IOCTL_RESETPPTPPROPERTY				2063
#define RTL8651_IOCTL_RESETL2TPPROPERTY				2064
#define RTL8651_IOCTL_ADDNAPTMAPPINGEXT				2065
#define RTL8651_IOCTL_FLUSHPROTOSTACKACTIONS		2066
#define RTL8651_IOCTL_ADDPROTOSTACKACTIONS			2067
#define RTL8651_IOCTL_DELURLFILTER				2068
#define RTL8651_IOCTL_ADDURLFILTER				2069
#define RTL8651_IOCTL_SETPERSRCDOSTHRESHOLD			2070
#define RTL8651_IOCTL_SETSOUCEIPBLOCKTIMEOUT			2071
#define RTL8651_IOCTL_FREEBLOCKEDSOURCEIP			2072
#define RTL8651_IOCTL_ENABLESOURCEIPBLOCK			2073
#define RTL8651_IOCTL_ADDGENERICDMZHOST         2074
#define RTL8651_IOCTL_ADDGENERICDMZFILTER       2075
#define RTL8651_IOCTL_DELGENERICDMZFILTER       2076
#define RTL8651_IOCTL_FLUSHGENERICDMZFILTER     2077 

#define RTL8651_IOCTL_FLUSHABQOS			2704
#define RTL8651_IOCTL_ADDPOLICYBASEDQOS			2705
#define RTL8651_IOCTL_DELPOLICYBASEDQOS			27051
#define RTL8651_IOCTL_SETFLOWCTRL			2706
#define RTL8651_IOCTL_SETPRIORITYQUEUE			2707
#define RTL8651_IOCTL_SETBANDWIDTHCTRL			2708
#define RTL8651_IOCTL_ADDPBNAT				2709
#define RTL8651_IOCTL_DELPBNAT				2710
#define RTL8651_IOCTL_CFGCONTER					2711
#define RTL8651_IOCTL_GETLINKSTATUS				2712
#define RTL8651_IOCTL_GETCOUNTER				2713
#define RTL8651_IOCTL_GETLDVER					2714
#define RTL8651_IOCTL_REBOOT					2715
#define RTL8651_IOCTL_SETNETMTU					2716
#define RTL8651_IOCTL_ADDMULTICASTFILTER			2717
#define RTL8651_IOCTL_DELMULITCASTFILTER			2718
#define RTL8651_IOCTL_ADDRATELIMITGROUP				2719
#define RTL8651_IOCTL_ADDRATELIMIT				2720
#define RTL8651_IOCTL_FLUSHRATELIMITGROUP			2721
#define RTL8651_IOCTL_REMOVENETWORKINTFLINKLAYERTYPE 	2722
#define RTL8651_IOCTL_DELEXTNETWORKINTERFACE 		2723
#define RTL8651_IOCTL_SETDOSSCANTRACKINGPARA		2724
#define RTL8651_IOCTL_SETMNQUEUEUPSTREAMBW			2725
#define RTL8651_IOCTL_ADDUNLIMITEDQUEUE				2726
#define RTL8651_IOCTL_ADDLIMITEDQUEUE				2727
#define RTL8651_IOCTL_ADDDOSIGNORETYPE				2728
#define RTL8651_IOCTL_DELDOSIGNORETYPE				2729
#define RTL8651_IOCTL_WEAKNAPTCTL				2730
#define RTL8651_IOCTL_LOOSEUDPCTL				2731
#define RTL8651_IOCTL_TCPNAPTSTATICPORTMAPPINGCTL	2732
#define RTL8651_IOCTL_UDPNAPTSTATICPORTMAPPINGCTL	2733
#define RTL8651_IOCTL_NAPTREDIRECT_REGISTER_RULE	2734
#define RTL8651_IOCTL_NAPTREDIRECT_UNREGISTER_RULE	2735
#define RTL8651_IOCTL_NAPTREDIRECT_QUERY_FLOW		2736
#define RTL8651_IOCTL_ADDSUBVLAN				2737
#define RTL8651_IOCTL_DELSUBVLAN				2738
#define RTL8651_IOCTL_ADDEXTMCASTPORT				2739
#define RTL8651_IOCTL_DELEXTMCASTPORT				2740
#define RTL8651_IOCTL_SET_DHCPC_PID				2741
#define RTL8651_IOCTL_ENABLE_PPPOE_PASSTHRU			2742
#define RTL8651_IOCTL_ENABLE_IPV6_PASSTHRU			2743
#define RTL8651_IOCTL_ENABLE_IPMULTICAST			2744
#define RTL8651_IOCTL_ENABLE_DROP_UNKNOWN_PPPOE_DROP 		2745
#define RTL8651_IOCTL_SETDMZHOSTL4FWD				2746
#define RTL8651_IOCTL_SETDMZHOSTICMPFWD				2747
#define RTL8651_IOCTL_ADDPROTOSTACKSERVERPORTRANGE		2748
#define RTL8651_IOCTL_ENABLE_IPX_PASSTHRU			2749
#define RTL8651_IOCTL_ENABLE_NETBIOS_PASSTHRU			2750
#define RTL8651_IOCTL_RTL8651_SETALGQOSQUEUEID			2751
#define RTL8651_IOCTL_DELRATELIMITGROUP				2752
#define RTL8651_IOCTL_GETMNQUEUEENTRY				2753
#define RTL8651_IOCTL_DELRATELIMITRULE				2754
#define RTL8651_IOCTL_QUERYUPNPMAPTIMEAGE			2755
#define RTL8651_IOCTL_DELFLOWFROMMNQUEUE			2756
#define RTL8651_IOCTL_SETDSCPTOMNQUEUE				2757
#define RTL8651_IOCTL_TBLDRVREINIT				2758
#define RTL8651_IOCTL_FWDENGREINIT				2759
#define RTL8651_IOCTL_FWDENGDROPPKTCASE				2760
#define RTL8651_IOCTL_ADDARP					2761
#define RTL8651_IOCTL_ADDDRIVERNAPTMAPPING			2762
#define RTL8651_IOCTL_ADDDOMAINBLOCKINGENTRY		2763
#define RTL8651_IOCTL_FLUSHDOMAINBLOCKINGENTRY		2764
#define RTL8651_IOCTL_ADDACLEXT			                2765
#define RTL8651_IOCTL_DELACLEXT			                2766
#define RTL8651_IOCTL_GETCHIPVERSION				2767
#define RTL8651_IOCTL_L34PURESOFTWAREPROCESS		2768
#define RTL8651_IOCTL_ADDGENERICPROTOSTACKACTIONS	2769
#define RTL8651_IOCTL_ADDMACACCESSCONTROL			2770
#define RTL8651_IOCTL_ENABLEMACACCESSCONTROL		2771
#define RTL8651_IOCTL_DELGENERICPROTOSTACKACTIONS	2772
#define RTL8651_IOCTL_DELPROTOSTACKACTIONS			2773
#define RTL8651_IOCTL_DELPROTOSTACKSERVERPORTRANGE	2774
#define RTL8651_IOCTL_REPROBE					2775
#define RTL8651_IOCTL_KERNELMUTEXLOCK				2776
#define RTL8651_IOCTL_KERNELMUTEXUNLOCK				2777
#define RTL8651_IOCTL_KERNELGETLOG					2778
#define RTL8651_IOCTL_KERNELCLRLOG					2779
#define RTL8651_IOCTL_ADDURLFILTERTRUSTEDUSER			2780
#define RTL8651_IOCTL_FLUSHURLFILTERTRUSTEDUSER		2781
#define RTL8651_IOCTL_ADDGENERICURLFILTERRULE			2782
#define RTL8651_IOCTL_DELGENERICURLFILTERRULE			2783
#define RTL8651_IOCTL_URLUNKNOWNTYPECASESENSITIVE		2784
#define RTL8651_IOCTL_SETURLDEFAULTACTION			2785
#define RTL8651_IOCTL_ENABLEPBNAT					2786
#define RTL8651_IOCTL_FLUSHURLFILTERRULE			2787
#define RTL8651_IOCTL_SETURLFORWARDLOGGING		2788
#define RTL8651_IOCTL_GETNATTYPE					2789
#define RTL8651_IOCTL_SETNATTYPE					2790
#define RTL8651_IOCTL_KERNELPROC				2791
#define RTL8651_IOCTL_ADDDOMAINADVROUTEENTRY 		2792
#define RTL8651_IOCTL_CHANGESESSIONPROPERTY     		2793
#define RTL8651_IOCTL_DELPOLICYORDEAMANDROUTEBYSESSIONID     	2794
#define RTL8651_IOCTL_REINITROUTELIST				2795
#define RTL8651_IOCTL_MANUALKEYFLAGSEND			2796
#define RTL8651_IOCTL_ENABLE_UNNUMBERNAPTPROC			2797
#define RTL8651_IOCTL_GETEXTDEVNAMEANDVID 2798
#define RTL8651_IOCTL_SETMCASTDETAILUPSTREAM 2799

#define RTL8651_IOCTL_SETVLANPROMISCUOUS		2800
#define RTL8651_IOCTL_SETNAPTTCPUDPPORTRANGE		2801
#define RTL8651_IOCTL_SETMULTIPPPOESESSIONSTATUS	2802


#if 1
/*865xc igmp snooping,2007-03-05*/
/*for igmp proxy to snooping, by qjj, 2006-11-28*/
#define RTL8651_IOCTL_SETIPMULTICASTMODE		2803
#define RTL8651_IOCTL_SETSTATICROUTER			2804
#define RTL8651_IOCTL_REMOVESTATICROUTER		2805
#endif
#define RTL8651_IOCTL_SETVLANPORTUNTAG		2807
#define RTL8651_IOCTL_GETL2TABLE		2808

#define RTL8651_IOCTL_SETOUTPUTQUEUEWANPORT		2809
#define RTL8651_IOCTL_SETOUTPUTQUEUEPARA			2810
#define RTL8651_IOCTL_SETDEFAULTQUEUERATIO		2811

#ifdef CONFIG_RTL865XC
#define RTL8651_IOCTL_CLEARASICCOUNTER                   2812
#endif

#ifdef CONFIG_RTL865XC
#define	RTL8651_IOCTL_QOSINIT					2900
#define	RTL8651_IOCTL_QOSADDFLOW				2901
#define	RTL8651_IOCTL_QOSDELFLOW				2902
#define	RTL8651_IOCTL_QOSFLUSHFLOW				2903
#define	RTL8651_IOCTL_QOSSETREMARK				2904
#endif

enum PORT_BITS
{
	CPU_PORT = 1 << 6,
	PORTMII = 1<<5,		
	PORT5	 = 1 << 4,
	PORT4    = 1 << 3,
	PORT3	 = 1 << 2,
	PORT2	 = 1 << 1,
	PORT1 	 = 1 << 0,
};
#define EXTERNAL_INTERFACE	1
#define RTL8651_LL_VLAN				0x01
#define RTL8651_LL_PPPOE			0x02
#define LL_VLAN		RTL8651_LL_VLAN
#define LL_PPPOE	RTL8651_LL_PPPOE
#define LL_PPTP		RTL8651_LL_PPTP
#define LL_L2TP		RTL8651_LL_L2TP

/* Debug Command */
#ifndef CONFIG_RTL865X_CLE
#define DUMP_PPPOE	3000
#define DUMP_VLAN	3001
#define DUMP_L3		3002
#define DUMP_TCP	3003
#define DUMP_ICMP	3004
#define DUMP_ALG	3005
#define DUMP_IP		3006
#define DUMP_PVID	3007
#define DUMP_TRAP	3008
#define DUMP_ARP	3009
#define DUMP_ACL	3011
#define DUMP_SEVERPORT	3012
#define DUMP_GIDX	3013
#define DUMP_FWD	3014
#define DUMP_L2     3015
#define DUMP_HS     3016
#define DUMP_FTCP   3017
#define DUMP_FICMP  3018
#define ADDSTATICNAPTFLOW  3019
#endif /*CONFIG_RTL865X_CLE*/

typedef struct 
{
	unsigned char			defaultSession;   	/* default pppoe session  */
	unsigned short			sessionId;	   /* pppoe session id */
	unsigned int			silentTimeout; /* in seconds */	
	unsigned char           ipAddr[4];	   /* ip address, for multiple pppoe only */
	unsigned char           ipMask[4];	   /* network mask, for multiple pppoe only */
	unsigned char           gwAddr[4];     /* default gateway address, for multiple pppoe only */
	unsigned char           svrMac[6];     /* pppoe server mac address */
	unsigned int          lanType;
} pppoeCfg_t;

#endif
