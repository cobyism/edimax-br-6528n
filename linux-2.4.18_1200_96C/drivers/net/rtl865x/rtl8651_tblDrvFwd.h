/*
* Copyright c                  Realtek Semiconductor Corporation, 2003
* All rights reserved.
* 
* Program : Header file for  fowrading engine rtl8651_tblDrvFwd.c
* Abstract : 
* Author :  Chun-Feng Liu(cfliu@realtek.com.tw)
* $Id: rtl8651_tblDrvFwd.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
* $Log: rtl8651_tblDrvFwd.h,v $
* Revision 1.1  2007/12/04 11:57:45  joeylin
* add hardware NAT feature
*
* Revision 1.123  2007/01/03 06:17:56  cw_du
* +: Add declaration of func int8 _rtl8651_l4CheckOccupiedConnection(uint32 isTcp, uint32 dsid, ipaddr_t IntIp, uint16 IntPort, ipaddr_t ExtIp, uint16 ExtPort, ipaddr_t RemoteIp, uint16 RemotePort).
*
* Revision 1.122  2006/11/20 06:43:29  qy_wang
* *:add a macro FWDENG_EXTDEV_TO_PS for extension device in forwarding engine
*
* Revision 1.121  2006/10/23 08:31:33  darcy_lu
* *:open new port bouncing module compile flag
*
* Revision 1.120  2006/10/19 03:34:55  hyking_liu
* new user map module for naptFlow
*
* Revision 1.119  2006/09/15 09:48:18  darcy_lu
* +: add code for new port bouncing module
*
* Revision 1.118  2006/07/13 15:55:49  chenyl
* *: modify code for RTL865XC driver compilation.
* *: dos2unix process
*
* Revision 1.117  2006/07/12 06:46:52  chenyl
* *: remove unknown TCP-SYN-TRAPPING ACL rule for unnumber WAN-type
* *: add new-line for rtl8651_tblDrvFwd.h to prevent from compiling warning.
*
* Revision 1.116  2006/07/10 06:57:35  darcy_lu
* +:add port bouncing alg flag
*
* Revision 1.115  2006/07/03 12:26:34  chenyl
* +: add the declaration of rtl8651_resetFwdEngineCounter();
*
* Revision 1.114  2006/06/28 03:20:27  bo_zhao
* *: refine drop error message from PS
*
* Revision 1.113  2006/06/22 05:57:14  bo_zhao
* +: add feature: drop error pkt from LAN => WAN
*
* Revision 1.112  2006/06/21 02:30:20  chenyl
* +: Port Bouncing ALG support for FTP traffic.
*
* Revision 1.111  2006/05/19 07:24:25  bo_zhao
* *: enable new napt module
*
* Revision 1.110  2006/05/19 07:12:58  bo_zhao
* +: support new Napt Module
*
* Revision 1.109  2006/03/30 09:30:11  hao_yu
* *: Remove dead code
*
* Revision 1.108  2006/02/13 07:58:36  hao_yu
* *: change declaration of fwdEngineRegisterAlgModule and fwdEngineRemoveAlgModule
*
* Revision 1.107  2006/01/25 13:59:15  tony
* +: New Feature: URL Filter Trusted user must be logged.
*
* Revision 1.106  2006/01/18 02:36:55  bo_zhao
* +: add DSCP remark
*
* Revision 1.105  2006/01/02 02:52:14  chenyl
* +: VLAN-tag support extension port forwarding and software Rome Driver forwarding.
*
* Revision 1.104  2005/12/12 03:19:45  tony
* +: New Feature: support Trusted User in URL Filter.
*
* Revision 1.103  2005/12/02 13:27:22  chenyl
* *: flow based DSID decision
* +: dynamic policy route setting for WAN-to-LAN NAPT flow.
*
* Revision 1.102  2005/11/22 17:02:31  chenyl
* +: new API rtl8651_fwdEnginePsDontTimeout() to enable/disable
*    DONT TIMEOUT switch about idle-timer update for ALL packets from protocol stack to WAN
*
* Revision 1.101  2005/11/21 15:58:58  chenyl
* +: For packet from protocol stack and sent to WAN, don't refresh idle-timer if it is...
* 	1. TCP RST packet
* 	2. ICMP error messages: ICMP_UNREACH/ICMP_SOURCEQUENCH/ICMP_TIMXCEED/ICMP_PARAMPROB
*
* Revision 1.100  2005/11/08 02:46:27  chenyl
* +: new return value for "Demand Route found" and "No any route found"
*
* Revision 1.99  2005/10/26 05:20:50  shliu
* *: modify the error message related to PSA addition/deletion.
*
* Revision 1.98  2005/10/26 04:53:42  shliu
* *: new error messages for PSA action table entry deletion
*
* Revision 1.97  2005/10/17 16:06:43  chenyl
* *: modify some data-path of Protocol Stack Action process.
* +: protocol stack action testing case for ICMP error message.
*
* Revision 1.96  2005/09/28 08:40:55  chenyl
* *: add internal function to set the permission of each forwarding decision
* 	- if it would apply SMAC filtering?
* 	- blocking this decision
* 	=> API:
* 		int32 _rtl8651_l34AdditionalExec_setParam(srcType,dstType,para)
* 		int32 _rtl8651_l34AdditionalExec_getParam(srcType,dstType,para)
*
* *: modify return value
* 	- WAN to WAN routing
* 		- if it be blocked, original return value is FWDENG_IP_WAN2WAN_ROUTING
* 			=> Modified to "FWDENG_DECISION_BLOCK"
*
* 	- Port bouncing
* 		- if it be blocked, original return value is FWDENG_L4_PORTBOUNCING
* 			=> Modified to "FWDENG_TO_GW"
*
* Revision 1.95  2005/09/25 16:57:47  shliu
* *: modify PSA rule error message
*
* Revision 1.94  2005/09/23 13:05:12  chenyl
* *: modify packet processing flow:
* 	make the forwarding decision before doing ACL due to we need to know if its
* 	L3 or L4 process to do ACL lookup.
*
* *: move the DNS demand route triggering code from l34packetProcess to the place where
* 	packet would be trapped to protocol stack.
* 	- add switch to turn ON/OFF it: rtl8651_fwdEngineTrapPktDemandRouteTrigger()
*
* *: add API to turn ON/OFF protocol stack action module when packet need doing
* 	port-bouncing.
* 	- rtl8651_fwdEnginePortBouncingDoProtocolStackAction()
*
* *: add code to prevent fragment packet processing error in Logging system
*
* Revision 1.93  2005/09/09 06:07:36  shliu
* *: add some fwd eng PS action table addition error code
*
* Revision 1.92  2005/08/25 04:01:36  chenyl
* *: rearrange code, add MUTEX lock/unlock for external functions, add `inline' property to some critical functions
*
* Revision 1.91  2005/08/24 15:44:47  chenyl
* *: add DRAM-use code
*
* Revision 1.90  2005/08/15 10:53:28  chenyl
* +: new TRIGGER PORT mechanism
* 	- sip check when activate Trigger Port.
* 	- age timer for Triggered Port Mapping.
*
* Revision 1.89  2005/07/29 09:43:59  chenyl
* +: dns module in ROMEDRV
* +: domain blocking (SDK + RomeDrv)
*
* Revision 1.88  2005/07/08 02:38:01  chenyl
* *: modify ip when doning RSVP NAPT in/out H.323 related process.
* *: Let RSVP callback function returning value to indicate RSVP process was been ignored if it don't do anything.
*
* Revision 1.87  2005/05/06 12:23:16  chenyl
* *: for outbound packet which match upnp mapping, ROME Drvier would use the mapped external ip/port
* 	to contribute this connection.
*
* *: tony: fix tcp upnp qos support problem.
*
* Revision 1.86  2005/05/04 13:19:06  chenyl
* *: bug fix: when turn the "TCP/UDP static port mapping" and the source IP/Port
* 	    is in uPnP/triggerPort map and the internal Port == External Port in that map,
* 	    then in these cases, static port mapping is still work.
*
* Revision 1.85  2005/04/30 04:19:49  chenyl
* +: support (L2-Packet-filling + L3-Forwarding) offload Engine
*
* Revision 1.84  2005/04/18 03:21:47  chenyl
* *: unify the extended device's LinkID type to uint32
* 	- register/unregister
* 	- packet from extended device to ROMEDRV
*
* 	- Note that: the packet from ROMEDRV to extended device is via
* 		     rtlglue_ interface, and for the control purpose,
* 		     'int32' type of LinkID is used there.
*
* Revision 1.83  2005/04/12 13:48:05  chenyl
* +: declare 'rtl8651_fwdEngineExtPortUcastFastRecv'
*
* Revision 1.82  2005/04/01 10:41:57  chenyl
* *: when decision table result is 'DROP', we return it with return value 'FWDENG_DECISION_DROP'
*    and default drop it in Forwarding Engine.
*
* Revision 1.81  2005/03/28 12:17:17  chenyl
* +: Exception List for NAPT-Redirect system
*
* Revision 1.80  2005/03/28 06:01:51  chenyl
* *: NAPT redirect without DIp/Dport modification
*
* Revision 1.79  2005/02/25 07:49:06  chenyl
* *: spanning tree function patch up
* 	- update multicast spanning tree port state with unicast port state simultaneously.
* 	- trap 80211D packet instead of broadcast it.
* 	- set FWD state to non-bridge port
* 	- Don't support Extension port spanning tree management, so we set FWD state to Extension ports.
* 	- enable spanning tree process in SW ROMEDRV packet forwarding ( filter all NON-FWD-State port for
* 	  ROMEDRV packet forwarding packets and protocol stack sending packet).
*
* Revision 1.78  2005/02/22 08:30:31  rupert
* +: add two prototype function
*
* Revision 1.77  2005/02/16 02:15:12  chenyl
* +: Forwarding Engine Trapping Dispatch system:
* 	When one packet must to be trapped to upper-layer protocol stack,
* 	User can decide how to dispatch this packet into different "categories"
* 	according to its 'l4protocol/source IP/destination IP/source Port (TCP/UDP)/
* 	destination Port (TCP/UDP)' with wide-card allowed. The category
* 	number is decided by user and will be set into ph_category in packet header.
* 	Fragment packet's test is passed.
*
* +: Forwarding Engine QoS Send:
* 	Protocol stack packet send with high TX queue used.
*
* Revision 1.76  2005/01/28 12:11:54  yjlou
* +: define constants for fwdEngineSend() intrnal use.
*
* Revision 1.75  2005/01/14 16:22:37  chenyl
* *: split rtlglue_extDeviceSend into unicast and broadcast
* *: bug fix, skip closed-state TCP napt entry when checking piggyback SYN packet
* +: return value statistic system
*
* Revision 1.74  2005/01/03 02:48:36  tony
* +: add API:rtl8651_queryUpnpMapTimeAge to query leaseTime
*
* Revision 1.73  2004/12/07 02:15:41  chenyl
* *: bug fix : Protocol Stack Flow cache
* +: hooking code for ACL-process dispatch
* +: add testing cases for Policy Route and Remote Management
*
* Revision 1.72  2004/11/29 07:35:43  chenyl
* *: fix bug : multiple-PPPoE MTU setting bug
* *: fix bug : Never add second session's ip into ASIC in multiple-PPPoE WAN type.
* *: fix bug : Tunekey abnormal behavior when multiple-PPPoE Dial-on-demand Dynamically turn ON/OFF
* *: in forwarding engine Init function, the parameter == 0xffffffff means user want to set as default value
* *: add Mutex-Lock/Unlock checking in testing code
*
* Revision 1.71  2004/11/11 09:03:45  chenyl
* *: continue checking trigger port entry after one-entry check fail
* *: some minor-modify
*
* Revision 1.70  2004/11/01 06:34:25  tony
* *: support rtl8651_addProtoStackServerPortRange
*
* Revision 1.69  2004/10/22 11:51:28  cfliu
* *: Export rtl8651_fwdEngineRemoveHostsOnExtLinkID(), remove ticksPerSecond
* from fwdeng.
*
* Revision 1.68  2004/10/07 13:25:13  chenyl
* +: apply uPnP entry check to Port Bouncing process
*
* Revision 1.67  2004/10/04 02:16:23  chenyl
* +: pptp/l2tp header refill function
*
* Revision 1.66  2004/09/30 10:38:12  chhuang
* *: regular update
*
* Revision 1.65  2004/09/30 07:59:12  chenyl
* *: function mtuFragment() modification
* +: apply forwarding engine to generate ICMP-Host-Unreachable message for packets fragmented by gateway
* +: add the switch to turn on/off "always reply ICMP error message for all packets fragmented by gateway regardless of
* 	Dont fragment bit"
*
* Revision 1.64  2004/09/29 08:39:41  yjlou
* +: provide precious aging machanism by Timer1.
*    +: EVENT_TRIGGER_TIMEUPDATE and LEGACY_AGING_CODE
*
* Revision 1.63  2004/09/15 16:14:28  chenyl
* +: enable multicast forward cache
*
* Revision 1.62  2004/09/07 14:52:13  chenyl
* *: bug fix: napt-redirect fragment packet checksum-recalculate
* *: bug fix: conflict between protocol stack flow cache and protocol stack action
* *: bug fix: protocol stack action mis-process to UDP-zero-checksum packet
* *: separate the header file:
*         - internal : rtl8651_tblDrvFwdLocal.h
*         - external : rtl8651_tblDrvFwd.h
*
* Revision 1.61  2004/09/02 09:04:59  chenyl
* *: napt-redirect: fragment packet support
*
* Revision 1.60  2004/09/02 06:15:38  chenyl
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
* Revision 1.59  2004/08/24 03:49:41  tony
* *: add function: rtl8651_pptpPppCompress
*
* Revision 1.58  2004/08/24 03:09:06  tony
* *: support pptp ppp un-compress mode.
*
* Revision 1.57  2004/08/12 03:14:02  jzchen
* +: Add Realtek protocl return value and modify affected variable definitions
*
* Revision 1.56  2004/08/11 05:19:06  chenyl
* +: weak TCP napt checking
* *: modify: for TCP napt outbound packet, if packet is (SYN+ACK) and no any entry found,
* 		forwarding engine will trap this packet and return FWDENG_TCP_NAPT_OUT_NOTFOUND
* 		insteads of dropping it silently.
*
* Revision 1.55  2004/08/06 15:14:08  chenyl
* *: fully checked TCP/UDP static-port-translation
*
* Revision 1.54  2004/08/06 09:40:50  chenyl
* *: enable TCP/UDP NAPT static-port-translation
* *: rename from looseUDP to InexactUDP
*
* Revision 1.53  2004/08/05 08:31:40  chenyl
* +: time-out of shortest path forwarding cache
* *: rewrite some code in ipQueue.c
*
* Revision 1.52  2004/08/05 06:39:11  chenyl
* +: fragment shortest path forwarding
* *: UDPBomb don't aware ignore Type Checking result
*
* Revision 1.51  2004/08/03 10:21:56  chenyl
* +: loose UDP
*
* Revision 1.50  2004/07/28 10:48:20  chenyl
* *: bug fix : [re_core.c]
* 		- rtl865x_fastRx: queue-packets to Protocol Stack and failed, free Mbuf ERROR
* *: bug fix : [swNic2.c]
* 		- swNic_write: process ERROR when mbuf padding failed
* *: bug fix : [rtl8651_tblDrvFwd.c]
* 		- _rtl8651_l34ActExec: process _RTL8651_NAT_IGNORE in _RTL8651_NPT case
* *: bug fix : [rtl8651_tblDrvFwd.c]
* 		- rtl8651_fwdEngineInput: when process result is FWDENG_ACL_DROP_NOTIFY,
* 				we must either generate ICMP Error message with newly allocated packet and trap
* 				the original packet, or generate ICMP Error message using original packet and return
* 				SUCCESS.
* 				we can not send ICMP Error message with the original packet and trap it to Procotol Stack.
*
* Revision 1.49  2004/07/27 10:46:42  cfliu
* -: Remove RTL865x external loopback port related code for PPTP/L2TP/WLAN
*
* Revision 1.48  2004/07/26 14:45:43  chenyl
* +: new logging model
* +: dos Ignore Case
* 	- ignore packets from LAN
* 	- ignore packets from WAN
*
* Revision 1.47  2004/07/23 13:19:57  tony
* *: remove all warning messages.
*
* Revision 1.46  2004/07/19 12:54:43  chenyl
* *: bug fix: process udp packets withc zero-checksum
* *: bug fix: process L3/L4 packets whose ip-header length is not 20 byte
* +: add testing functions to compare/clear checksum field of mbuf/pkthdr
*
* Revision 1.45  2004/07/13 02:53:16  tony
* *: fix protostack packets be drop in pptp/l2tp mode.
*
* Revision 1.44  2004/07/12 04:25:36  chenyl
* *: extend existing port scan mechanism
*
* Revision 1.43  2004/07/09 05:20:17  cfliu
* +: Redefine FWDENG_XXXX type code
*
* Revision 1.42  2004/07/08 14:35:09  cfliu
* +: Add new param mbufheadroom in fwd init data structure
*
* Revision 1.41  2004/07/05 11:32:47  chenyl
* +: dynamically turn-ON/OFF port bouncing
*
* Revision 1.40  2004/07/04 15:04:29  cfliu
* +:put protocol stack send function from L2 to L3/4 fwd engine
*
* Revision 1.39  2004/06/30 03:16:49  chenyl
* *: bug fix of Multicast Filter
* *: remove one unused parameter in multicast system
*
* Revision 1.38  2004/06/29 07:34:23  chenyl
* +: IGMPv3
* +: igmp proxy filter :
* 		protocol stack can let igmp-proxy
* 			1: trap specific multicast group
* 			2: ignore specific multicast group
*
* Revision 1.37  2004/06/29 07:31:12  cfliu
* +:Add a switch to turn on/off WAN->WAN routing
*
* Revision 1.36  2004/06/18 08:11:15  tony
* +: add new features: SYN,FIN,ACK Port Scan.
*
* Revision 1.35  2004/06/14 07:32:44  yjlou
* *: fixed the bug of setting protocol-based VLAN for PPPoE when disabled in boot time.
* *: move rtl8651_drvProtocolBasedNatEnable from 'rtl8651_tblDrvFwd.c' to 'rtl8651_tblDrv.c'.
* *: move rtl8651_drvPppoePassthruEnable and rtl8651_drvIpv6PassthruEnable from 'rtl8651_tblDrvFwd.c' to 'rtl8651_layer2.c'.
*
* Revision 1.34  2004/06/08 14:17:51  jzchen
* +: Define enable/disable pppoe session ID not exist termination feature API
* +: Define active PPPoE session ID tracking data structure
* +: Define active PPPoE session ID tracking number in forwarding engine initial parameter
*
* Revision 1.33  2004/06/07 13:05:49  yjlou
* +: Supporting IPv6 Passthru
* *: By default, Protocol-Based NAT, PPPoE Passthru, and IPv6 Passthru is disabled.
* *: PPPoE Passthru and IPv6 Passthru share the same VLAN
* *: The code changing VLAN ID is moved to forwording engine.
*
* Revision 1.32  2004/06/01 09:36:27  chhuang
* add a new type: FWDENG_ACL_DROP_NOTIFY
*
* Revision 1.31  2004/05/20 12:28:35  chenyl
* *: for fast path packets, only check SYN/FIN/UDP/ICMP flooding DOS
* *: fast path packets don't run Egress ACL
* *: BUG FIX: refresh age-time of UDP flows in forwarding engine
*
* Revision 1.30  2004/05/20 06:41:33  chenyl
* *: fragment system : if (DOS)/(UDP blocking) is disabled, don't queue in-the-order packets
* *: check all dos attacks for each packet
* *: bug fix in port bouncing
* *: apply TCP state tracking in port bouncing
* *: bug fix in multicast system
*
* Revision 1.29  2004/05/19 16:09:57  yjlou
* +: add rtl8651_EnablePppoePassthru() to enable/disable PPPoE passthru.
*
* Revision 1.28  2004/05/19 14:01:26  yjlou
* +: support Protocol-Based NAT
*
* Revision 1.27  2004/05/17 12:13:19  cfliu
* no message
*
* Revision 1.26  2004/05/12 07:20:15  chenyl
* +: source ip blocking
* *: modify dos mechanism
*
* Revision 1.25  2004/05/06 08:03:35  chhuang
* add MAC filtering
*
* Revision 1.24  2004/05/05 09:00:03  chhuang
* add MAC filter
*
* Revision 1.23  2004/05/05 08:27:07  tony
* new features: add remote management spec
*
* Revision 1.22  2004/05/05 04:28:50  chenyl
* +: arp proxy
* *: bug fix (L2TP/PPTP) in layer2fwd.c
*
* Revision 1.21  2004/05/04 12:46:27  chenyl
* *: modify ipQueue/multicast system
*         +:  ipQueue:   can constraint total queued packet only
*         +:  multicast: can constraint total member only
*         *:  allocate table when ipQueue/Multicast init()
*         *:  if table size == 0 , ipQueue/Multicast system become disable
*         *:  switch to turn on/off Multicast system dynamically
*
* *: add CLE command to turn on/off ipQueue/ Multicast system
*
* *: change the init method: init ipQueue/multicast system in rtl8651_fwdEngineInit(configPara)
*    and set initial value using "configPara"
*
* Revision 1.20  2004/05/03 14:54:17  cfliu
* Add 8650B extension port support.
* Revise all WLAN/extport related code.
*
* Revision 1.19  2004/04/30 08:58:08  chenyl
* +: ip multicast/igmp proxy
*
* Revision 1.18  2004/04/21 08:44:43  tony
* PPTP/L2TP support remote management for CyberTAN spec.
*
* Revision 1.17  2004/04/14 08:43:56  tony
* make L2TP support auto reconnect.
*
* Revision 1.16  2004/04/08 13:18:12  tony
* add PPTP/L2TP routine for MII lookback port.
*
* Revision 1.15  2004/04/08 12:12:53  cfliu
* Add WDS support on extension port API
*
* Revision 1.14  2004/04/07 11:29:33  chenyl
* *: bug fix in fwd-engine : remove "isToEthBcast"
* +: rtl8651_fwdEngineProcessIPFragment(enable) to enable/disable ipQueue system
* 	enable : fragment packets are processed by fwdeng
* 	disable: fragment packets are trapped to protocol stack
*
* Revision 1.13  2004/04/06 01:50:32  chenyl
* *: for pkts generated/queued in fwdeng, free MbufChain and return SUCCESS when fwdengSend Fail
*
* Revision 1.12  2004/03/31 09:36:23  cfliu
* Add WDS support
*
* Revision 1.11  2004/03/24 09:46:44  chenyl
* +: queued packet to CPU
*
* Revision 1.10  2004/03/22 12:29:53  chenyl
* +: _RTL_QUEUE_FRAG_PKT for fragment packet Queue system
*
* Revision 1.9  2004/03/19 13:13:35  cfliu
* Reorganize ROME driver local header files. Put all private data structures into different .h file corrsponding to its layering
* Rename printf, printk, malloc, free with rtlglue_XXX prefix
*
* Revision 1.8  2004/03/18 07:24:42  chenyl
* +: port bouncing for DMZ Host / NAT
*
* Revision 1.7  2004/03/16 15:10:53  chenyl
* +: support IP-unnumbered -> DMZ Host port bouncing
*
* Revision 1.6  2004/03/12 08:18:18  chenyl
* *: add rtl8651_fwdEngineIcmpRoutingMsg()
*
* Revision 1.5  2004/03/12 01:39:33  chenyl
* * Rename
* 	rtl8651_IcmpToDMZHostEnable -> rtl8651_DMZHostIcmpPassthroughEnable
* 	rtl8651_fwdEngineIcmpToDMZHost -> rtl8651_fwdEngineDMZHostIcmpPassThrough
*
* * Bug Fix
* 	in FwdengineSend()
* 		remove:
* 			retval &=~FWDENG_DROP;
*                		mBuf_driverFreeMbufChain(mbuf);
*
* 	_rtl8651_drvIcmpAgent()
* 		rewrite
*
* 	l4TcpNaptAliasIn
* 		fix key-in bug
* * Add Features
* 	_rtl8651_drvIcmpErrorGeneration
* 		FwdEngine will be able to reply ICMP TIME EXCEED Message
*
* * Some counter name is changed
*
* Revision 1.4  2004/03/11 04:08:57  cfliu
* fix layer2*.c compile problems
*
* Revision 1.3  2004/03/03 03:20:52  chenyl
* *: Policy route by SIP
* *: modify _rtl8651_drvIcmp, bounce packet back to ingress interface
* +: FwdEngineReinit() for reset fwdeng's variable and data structures
*
* Revision 1.2  2004/02/27 13:37:17  chenyl
* +: napt-icmp path for
* 	+: outbound TSTAMP
* 	+: outbound ERROR Message
* 	+: outbound TSTAMPREPLY/ECHOREPLY
* 	+: inbound TSTAMP/ECHO forwarding to dmzhost
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.4  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.31  2004/02/24 04:24:48  cfliu
* Remove WLAN pure bridging variables.
* Remove some FWDENG error number due to change in fwd engine
*
* Revision 1.30  2004/02/12 06:58:39  tony
* add those define in this header file
* rtl8651_flushProtoStackUsedUdpPorts(),rtl8651_flushProtoStackServerUsedTcpPorts()
* rtl8651_addProtoStackUsedUdpPort(),rtl8651_addProtoStackServerUsedTcpPort()
*
* Revision 1.29  2004/02/12 06:25:17  rupert
* Rename pppoeId to SessionID
* Fixed  rtl8651_flushPolicyRoute about dynamic route
*
* Revision 1.28  2004/02/09 02:49:58  tony
* enable Port Bouncing routine.
*
* Revision 1.27  2004/02/06 12:33:37  tony
* add rtl8651a_addTriggerPortRule and rtl8651a_flushTriggerPortRules API
*
* Revision 1.26  2004/02/06 07:23:47  rupert
* Add rtl8651a_setUdpSizeThreshValue prototype
*
* Revision 1.25  2004/02/05 12:11:52  tony
* add Port Bouncing Routine(support port change).
* (please define _RTL_PORT_BOUNCING in FwdEng header file to turn it on)
*
* Revision 1.24  2004/01/30 12:03:05  tony
* make Special Application is able to support multiple session.
*
* Revision 1.23  2004/01/15 13:41:49  tony
* modify UPNP_ITEM from 64 to 96.
*
* Revision 1.22  2004/01/13 12:51:21  hiwu
* remove PPTP WAN port type support (because bug)
*
* Revision 1.21  2004/01/09 09:30:35  hiwu
* add PPTP WAN type support
*
* Revision 1.20  2004/01/07 06:49:19  hiwu
* FIX VPN passthrough support
*
* Revision 1.19  2003/12/31 04:59:26  tony
* fix bug: turn L2TP ALG on/off dynamically.
*
* Revision 1.18  2003/12/26 10:54:22  hiwu
* merge RTL865x_Bridge back
*
* Revision 1.17  2003/12/17 10:24:56  hiwu
* add checksum function
*
* Revision 1.16  2003/12/16 12:09:41  tony
* disable debug message
*
* Revision 1.15  2003/12/16 12:07:22  tony
* fix bug: make DirectX ALG support game AOE2.
*
* Revision 1.14  2003/12/09 15:25:25  cfliu
* MUST add linux/config.h....
*
* Revision 1.13  2003/12/09 12:05:45  hiwu
* add RTL865x_Bridge
*
* Revision 1.12  2003/11/07 05:34:36  tony
* modify YMSG ALG, it'll not have to cooperate with trigger port.
*
* Revision 1.11  2003/10/23 07:10:18  hiwu
* add pptp alg support
*
* Revision 1.10  2003/10/17 04:33:12  hiwu
* add new DOS callback function
*
* Revision 1.9  2003/10/15 08:04:47  tony
* move Trigger Port range checking routine from FastAlias to NaptAlias
*
* Revision 1.8  2003/10/09 13:23:44  tony
* TRIGGER_PORT: add function rtl8651_flushTriggerPortRules,rtl8651_addTriggerPortRule
*
* Revision 1.7  2003/10/06 05:54:19  orlando
* check in final-round table driver merging result
*
* Revision 1.9  2003/10/04 14:59:02  cfliu
* remove some FWDENG_XXX types and change rtl8651_fwdEngineSend function prototype
*
* Revision 1.8  2003/10/03 16:48:30  cfliu
* no message
*
* Revision 1.7  2003/09/26 09:34:46  cfliu
* no message
*
* Revision 1.6  2003/09/24 12:21:15  cfliu
* add url filter reason
*
* Revision 1.5  2003/09/23 05:42:11  cfliu
* no message
*
* Revision 1.4  2003/09/20 10:43:49  cfliu
* no message
*
* Revision 1.3  2003/09/19 05:03:02  cfliu
* Add new fwd engine error code
* Add _rtl8651_fwdEngineScanDoS()
* rename _rtl8651_l4AddAlgLink as _rtl8651_fwdEngineRegisterAlgModule and change its prototype.
*
*/
#ifndef RTL8651_FWDENGINE_H
#define RTL8651_FWDENGINE_H

#include "rtl8651_config.h"
#include "rtl8651_tblDrv.h"
#define	_RTL_NEW_NAPT_MODULE  1 
#define _RTL_NEW_PORT_BOUNCING_MODULE 	1
#if 0
#define	_RTL_DROP_ERROR_MESSAGE	1
#endif

/*********************************************************************************************************
	Forwarding Engine switches
**********************************************************************************************************/
#define _RTL_IP_FRAG						/* enable ip fragmenet */
#define _RTL_NAPT_FRAG						/* enable napt ip fragment process */
#define _RTL_NAPT_SHORT_CUT				/*enable napt short cut forward */
#define _RTL_PORT_BOUNCING_SHORT_CUT		/*enable port bouncing short cut forward */
#define _RTL_DMZ 							/* enable Realtek define DMZ */
#define _RTL_BRIDGE 	 						/* enable Realtek's software bridge */
#define _RTL_UPNP 							/* enable Realtek's software UPNP IDG support */
#define _RTL_QUEUE_ARP_PACKET				/* enable Realtek's arp queue packet support */
#define _RTL_TRIGGER_PORT					/* enable Realtek's software Trigger Port support */
#define _RTL_DOS_PLUGIN					/* enable Realtek's software DOS plugin function */
#define _RTL_PORT_BOUNCING					/* enable Realtek's software DOS plugin function */
#define _RTL_PS_ACTION						/* enable Realtek's Protocol Stack Action function */
#define _RTL_NAPT_ADVANCED				/* enable Realtek's Advanced NAPT options */
#define _RTL_WEAK_NAPT_CTRL				/* enable Realtek's weak NAPT control function */
#define _RTL_TOS_REMARKING	/* enalbe DSCP remark */
#define _RTL_PORT_BOUNCING_ALG /*enable port bouncing alg*/

#ifdef _RTL_IP_FRAG
#define _RTL_QUEUE_FRAG_PKT	/* enable Realtek's fragment packet queue support */
#endif /* _RTL_IP_FRAG */
#define _RTL_NAPT_REDIRECT		/* enable Realtek's specific napt-outbound-flow trapping support */
#define _RTL_PS_FLOWCACHE		/* enable Realtek's procotocol stack flow cache  */
//#define _TONY_DEBUG // just for testing
#define FWDENG_TICKS_PER_SECOND 100
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
#define _RTL_PPTPL2TP_HDR_REFILL	/* refill pptp/l2tp header for packet trapped to protocol stack */
#endif /* PPTPL2TP */
#define _RTL_TRAPPING_DISPATCH	/* trapping packet dispatch system */

#define DRAM_FWD					/* use DRAM */

/* forwarding engine return value statistic system */
#define _RTL8651_FWDENG_RETURN_STATISTIC
/*********************************************************************************************************
	Forwarding Engine error codes
**********************************************************************************************************/
#define FWDENG_DROP		0x1000

/* ACL related */
#define FWDENG_ACL_DROP_NOTIFY		1 	/* pkt dropped and a ICMP unreachable mesg sent to sender */
#define FWDENG_ACL_IN_USERTRAP		2	/* ingress pkt trapped to protocol stack */
#define FWDENG_ACL_IN_DROP			3	/* Pkt dropped by ingress ACL */
#define FWDENG_ACL_OUT_USERTRAP		4	/* egress pkt trapped to protocol stack */
#define FWDENG_ACL_OUT_DROP			5	/* Pkt dropped by egress ACL */

/* L2 related */
#define FWDENG_BROADCAST				10	/* L2 broadcast, send to protocol stack */
#define FWDENG_VLANINVALID			11	/* Invalid VLAN. Maybe not yet configured. */
#define FWDENG_VLANCFI					12	/* Vlan tagged pkt with CFI=1 */
#define FWDENG_PPPOE8863				13	/* PPPoE discovery phase pkt */
#define FWDENG_PPPOE8864				14	/* PPPoE session phase pkt */
#define FWDENG_ARP_NOTREQ				15	/* Not a ARP request */
#define FWDENG_ARP_OTHERS				16	/* ARP request to other host */
#define FWDENG_ARP_REQUEST			17	/* ARP request, Let protocol stack send reply. */
#define FWDENG_NOTIPPACKET			18	/* destined to gateway, but not IP packet. */
#define FWDENG_MACBLOCK				19
#define FWDENG_RTKPROTO				20	/* When EtherType field is 0x8899, this return value will appear*/
#define FWDENG_L2_CTRLMSG				21	/* Layer2 control message, IEEE 802.11D or IEEE 802.11Q reserved */

/* L3 - Non-IP */
#define FWDENG_NETIFACENOTFOUND		30	/* No layer3 network interface attached on receiving VLAN */
#define FWDENG_L34DISABLED			31	/* Layer3/4 operation is disabled. All such pkts goes to protocol stack. */
#define FWDENG_IGMPPKT				32	/* IGMP pkt not processed. Send to protocol stack */
#define FWDENG_GRE_IGNORE				33
#define FWDENG_ESP_IGNORE				34
#define FWDENG_RSVP_IGNORE			35
#define FWDENG_L3UNKNOWNPROTO		36	/* unknown L3 protocol */

/* IP */
#define FWDENG_IP_CANTFRAG					40	/* Can't handle IP fragment */
#define FWDENG_IP_WAN2WAN_ROUTING_LEGACY	41	/* pkt rcvd from WAN and should be routed to WAN but function disabled by user (legacy now) */
#define FWDENG_IP_CANTQUEUEFRAGMENT			42	/* Out of fragment queue space */
#define FWDENG_IP_BADCKSUM					43	/* IP cksum error */
#define FWDENG_IP_TTLEXCEED					44	/* TTL<2 and TTL-1 is required for routing */
#define FWDENG_IP_INVALID						45	/* invalid  IP in IP header */
#define FWDENG_IP_MULTICAST					46	/* IP multicast pkt.(not IGMP) */

/* L4 - misc */
#define FWDENG_L4_UNKNOWNPROTO			50	/* unsupported L4 protocol */
#define FWDENG_L4_NAPT_UNSUPPORTPROTO	51	/* must do NAPT but protocol is not TCP/UDP/ICMP */
#define FWDENG_L4_INVALIDPORT				52 	/* Src or dst Port number is 0 */
#define FWDENG_L4_NATNOENTRY				53	/* No matched entry in NAT table */
#define FWDENG_L4_PORTBOUNCING_LEGACY	54 	/* pkt rcvd from LAN destined to local server's public IP but function disabled. (legacy now) */

/* ICMP */
#define FWDENG_ICMP_OUT_ADDFAIL				60	/* out of ICMP entry space */
#define FWDENG_ICMP_OUT_INVALIDIPID			61	/* invalid ICMP ip or id */
#define FWDENG_ICMP_OUT_ERRFLOWNOTFOUND	62
#define FWDENG_ICMP_IN_ADDFAIL				63	/* can't add inbound ICMP echo req to DMZ host */
#define FWDENG_ICMP_IN_NOTFOUND  			64	/* inbound icmp flow not found */
#define FWDENG_ICMP_IN_ERRFLOWNOTFOUND		65	/* flow not found in inbound ICMP error mesg */
#define FWDENG_ICMP_BADCKSUM					66	/* bad icmp checksum */
#define FWDENG_ICMP_UNSUPPORTED				67

/* UDP */
#define FWDENG_UDP_BADCKSUM						70	/* bad udp cksum */
#define FWDENG_UDP_ECHOCHARGENBOMB				71	/* suspicous UDP echo/chargen bomb pkt. */
#define FWDENG_UDP_NAPT_OUT_NOTFOUND			72	/* Failed to find specified UDP flow in NAPT table */
#define FWDENG_UDP_NAPT_OUT_NOPORT				73	/* All usable port numbers exhausted. */
#define FWDENG_UDP_NAPT_OUT_ADDFAIL				74	/* Failed to add new UDP napt entry. Table full? */
#define FWDENG_UDP_NAPT_OUT_SVRALGADDFAIL		75	/* Failed to add new UDP server alg napt entry. Table full? */
#define FWDENG_UDP_NAPT_OUT_CLIALGADDFAIL		76	/* Failed to add new UDP client alg napt entry. Table full? */
#define FWDENG_UDP_NAPT_OUT_IGNORE				77
#define FWDENG_UDP_NAPT_IN_NOTFOUND			78
#define FWDENG_UDP_NAPT_IN_ALGADDFAIL			79
#define FWDENG_UDP_NAPT_IN_ADDFAIL				80
#define FWDENG_UDP_NAPT_IN_SERVERPORTNOTFOUND	81
#define FWDENG_UDP_NAPT_IN_IGNORE				82
#define FWDENG_UDP_NAT_IN_CLIALGADDFAIL 		83
#define FWDENG_UDP_NAT_IN_SVRALGADDFAIL 		84
#define FWDENG_UDP_NAT_IN_ALGNOTFOUND			85
#define FWDENG_UDP_NAT_IN_ALGIGNORE				86
#define FWDENG_UDP_NAT_OUT_ALGIGNORE			87
#define FWDENG_UDP_NAT_OUT_ALGNOTFOUND		88
#define FWDENG_UDP_NAT_OUT_CLIALGADDFAIL		89
#define FWDENG_UDP_NAT_OUT_SVRALGADDFAIL		90

/* TCP */
#define FWDENG_TCP_BADCKSUM						100
#define FWDENG_TCP_URLFILTERED					101
#define FWDENG_TCP_NAPT_OUT_CLIALGADDFAIL 		102	/* adding a existing ALG control flow or ALG list full */
#define FWDENG_TCP_NAPT_OUT_SVRALGADDFAIL 		103	/* adding a existing ALG control flow or ALG list full */
#define FWDENG_TCP_NAPT_OUT_ALGNOTFOUND		104
#define FWDENG_TCP_NAPT_OUT_NOIPPORT			105
#define FWDENG_TCP_NAPT_OUT_NOTFOUND			106
#define FWDENG_TCP_NAPT_OUT_ADDFAIL				107
#define FWDENG_TCP_NAPT_OUT_IGNORE				108
#define FWDENG_TCP_NAPT_IN_CLIALGADDFAIL		109
#define FWDENG_TCP_NAPT_IN_IGNORE				110
#define FWDENG_TCP_NAPT_IN_SVRALGADDFAIL		111
#define FWDENG_TCP_NAPT_IN_NOTFOUND				112
#define FWDENG_TCP_NAPT_IN_ADDFAIL				113
#define FWDENG_TCP_NAT_IN_ALGNOTFOUND			114
#define FWDENG_TCP_NAT_IN_ALGADDFAIL			115
#define FWDENG_TCP_NAT_IN_ALGIGNORE				116
#define FWDENG_TCP_NAT_OUT_ALGNOTFOUND			117
#define FWDENG_TCP_NAT_OUT_ALGIGNORE			118
#define FWDENG_TCP_NAT_OUT_ADDFAIL				119
#define FWDENG_TCP_NAT_OUT_ENTRYUPDATE			120

/* misc */
#define FWDENG_USER_IDSCHECKDROP			130
#define FWDENG_USER_DROP					131
#define FWDENG_TO_GW						132	/* packet must be trapped to gateway */
#define FWDENG_QUEUED						133
#define FWDENG_QUEUED_TO_GW				134
#define FWDENG_NOTXPORT					135
#define FWDENG_DECISION_DROP				136	/* decision table indicate this packet is illegal and need to be dropped */
#define FWDENG_DECISION_BLOCK				137	/* the forwarding decision of this packet is being blocked */
#define FWDENG_DEMANDRT_FOUND			138	/* demand route found, and the hooked callBack function is forked and forwarding is stopped */
#define FWDENG_RT_NOTFOUND				139	/* can not match any routing table entry */

#define FWDENG_TOTALERRS					140


/* Error Number for Protocol Stack Action Table */
#define FWDENG_EPSA_IPPORTRANGE_CONFLICT	1000	/* PS action table entry addition failed due to there's conflict with existing entries */
#define FWDENG_EPSA_TABLEFULL					1001	/* When adding PSA, PS action table is full */
#define FWDENG_EPSA_IPRANGENOTCORRECT		1002	/* Invalid PSA, lower IP is larger than upper IP in IP range arguments */
#define FWDENG_EPSA_ICMPARGISNOTZERO		1003	/* Invalid PSA, ICMP protocol with incorrect arguments */
#define FWDENG_EPSA_PORTRANGENOTCORRECT	1004	/* Invalid PSA, lower port number is larger than upper port number */
#define FWDENG_EPSA_DSTIPNOTEXTGWIP			1005	/* Invalid PSA, assigned dstIp is not gateway ip. */
#define FWDENG_EPSA_PORTRANGETOOLARGE		1006	/* Specified port range is larger than (legal port upper bound - fwdPort) */
#define FWDENG_EPSA_PSAENTRYNOTFOUND		1007	/* Specified PSA entry not found in PSA table */
#define FWDENG_EPSA_INVALIDPSACTION		1008	/* Invalid PS action */
#define FWDENG_EPSA_INVALIDPSROLE		1009	/* Invalid PS role */

/* forwarding engine internal control number : start from 30000 */

/* used by fwdEngineSend process */
#define FWDENG_SEND_PASSED				30000
#define FWDENG_SEND_GOTO_SEND			30001
#define FWDENG_SEND_GOTO_TOSWCORE		30002


/* forwarding engine: Protocol Stack Offloading Engine control number : start from 31000 */

/* used by _rtl8651_offloadEngineL3Send L3 offloading process */
#define OFFENG_EINVALIDINPUT			31000			/* input parameters fail */
#define OFFENG_EHEADROOM2SMALL		31001			/* offload packet without enough headroom */

/*forwarding engine:napt user mapping control number: start from 32000*/
#ifdef RTL8651_NAPT_USR_MAPPING
#define FWDENG_NAPTUSRMAPPING_INVALIDINPUT		32000
#define FWDENG_NAPTUSRMAPPING_FILTERNOTFOUND	32001
#endif


/*used by extension device for forwarding engine: start from 35000*/
#define FWDENG_EXTDEV_TO_PS    35000    /*throw the package to linux protocol stack*/

/*********************************************************************************************************
	Forwarding Engine initial parameter's data structure
**********************************************************************************************************/
#define RTL8651_FWDENGINE_PARA_UNDEFINED_VALUE		0xffffffff	/* undefined value for forwarding engine parameters */
typedef struct rtl8651_fwdEngineInitPara_s {
	/* ip Fragment */
	uint32 MaxFragPktCnt;
	uint32 MaxFragSubPktCnt;
	uint32 MaxNegativeListEntryCnt;
	uint32 MaxPositiveListEntryCnt;
	uint32 MaxFragPoolCnt;
	uint32 MaxFragTimeOut;
	uint32 MaxNegativeListTimeOut;
	/* ip multicast */
	uint32 MCastDefaultVersion;				/* default igmp version supported by multicast system */
	uint32 MCastProcessOption;				/* set options for multicast system process */
	uint32 MaxMCastCnt;
	uint32 MaxMCastMemberCnt;
	uint32 MaxMCastTotalMemberCnt;
	uint32 MaxMCastTotalSourceCnt;
	uint32 MCastMember_ExpireTime;
	uint32 MCastUpstream_Timeout;
	uint32 MCastFilterEntryCnt;				/* Entry Count of Multicast Filter entry(for multicast group which need processed by protocol stack) */
	uint32 igmp_qi;
	uint32 igmp_qri;
	uint32 igmp_oqpi;
	uint32 igmp_sqc;
	uint32 igmp_sqi;
	uint32 igmp_lmqc;
	uint32 igmp_lmqi;
	uint32 igmp_gmi;							/* Group Member Interval */
	uint32 igmp_group_query_interval;
	uint32 igmp_response_tolerance_delay;
	/* dos */
	/* per-sip flood */
	uint32 dos_SynFlood_trackCnt;
	uint32 dos_StealthFin_trackCnt;
	uint32 dos_UdpFlood_trackCnt;
	uint32 dos_IcmpFlood_trackCnt;
	uint32 dos_total_trackCnt;
	/* port scan tracking */
	uint32 dos_total_portScan_trackCnt;
	/* per-sip connection control */
	uint32 dos_TcpConn_trackCnt;
	uint32 dos_UdpConn_trackCnt;
	uint32 dos_TcpUdpConn_trackCnt;
	uint32 dos_totalConn_trackCnt;
	/* dos proc : source ip blocking */
	uint32 dosProc_sipblock_count;
	/* pppoe NOT existed session termination */
	uint32 pppoeActiveSessionTrackingNumber;
	uint32 mbufHeadroom;//mbuf reserve headroom
	/* napt redirect */
#ifdef _RTL_NAPT_REDIRECT
	uint32 redirectTblSize;
	uint32 redirectFlowCacheSize;
#endif /* _RTL_NAPT_REDIRECT */
#ifdef _RTL_PPTPL2TP_HDR_REFILL
	uint32 pptpHdrCacheSize;
#endif /* _RTL_PPTPL2TP_HDR_REFILL */
#ifdef _RTL_PS_ACTION
	uint32 protoStackMaxActions;
#endif /* _RTL_PS_ACTION */
#ifdef _RTL_TRAPPING_DISPATCH
	uint32 trappingPktDispatchTblSize;
#endif /* _RTL_TRAPPING_DISPATCH */
	/* DNS process module */
	uint32 dns_dnsDB_processCnt;
	uint32 dns_dnsDB_tblSize;
	uint32 dns_dnsDB_cnameMapTblSize;
	uint32 dns_dnsDB_cnameMapTimeout;
	/* domain blocking */
	uint32 domainBlock_tblSize;
	uint32 domainBlock_maxBlockEntryCnt;
	uint32 domainBlock_blockEntryTimeout;

	/* trigger port */
#ifdef _RTL_TRIGGER_PORT
	uint32 triggerPortEntryCnt;	/* total entry count of trigger port entry */
	uint32 triggerPortCacheSize;	/* total cache count of trigger port flow */
#endif	/* _RTL_TRIGGER_PORT */
#ifdef RTL8651_NAPT_USR_MAPPING
	uint32 naptUsrMappingType_priority;
#endif

} rtl8651_fwdEngineInitPara_t;

/*********************************************************************************************************
	Forwarding Engine Initiation
**********************************************************************************************************/
int32 rtl8651_fwdEngineInit(struct rtl8651_fwdEngineInitPara_s *);
int32 rtl8651_fwdEngineReinit(void);

/*********************************************************************************************************
	Forwarding Engine/protocol stack Hooking API
**********************************************************************************************************/
int32 rtl8651_fwdEngineInput(void * pkthdr);
int32 rtl8651_fwdEngineExtPortRecv(void *id, uint8 *data,  uint32 len, uint16 myvid, uint32 myportmask, uint32 linkId);

/*********************************************************************************************************
	Forwarding Engine function switch
**********************************************************************************************************/
int32 rtl8651_fwdEngineDropPktCase(uint32 fwdengDropCase, int32 *flag);
int32 rtl8651_fwdEngineSend(void * pkthdr, int16 dvid, int32 iphdrOffset);
int32 rtl8651_fwdEngineQosSend(void *pkthdr, int16 dvid, int32 iphdrOffset, int32 qualityId);
int32 rtl8651_fwdEngineArp(int8 enable);
int32 rtl8651_fwdEngineIcmp(int8 enable);
int32 rtl8651_fwdEngineIcmpRoutingMsg(int8 enable);
int32 rtl8651_fwdEngineAlwaysReplyICMPErrForFragment(int8 enable);
int32 rtl8651_fwdEngineDMZHostIcmpPassThrough(int8 enable);
int32 rtl8651_fwdEngineArpProxy(int8 enable);
int32 rtl8651_fwdEngineProcessL34(int8 routingEnable);
int32 rtl8651_fwdEngineProcessIPFragment(int8 enable);
int32 rtl8651_fwdEngineProcessIPMulticast(int8 enable);
int32 rtl8651_fwdEnginePppoeNotExistedSessionTermination(int8 enable);
int32 rtl8651_fwdEngineProcessPortBouncing(int8 enable);
int32 rtl8651_fwdEngineProcessUpnpPortBouncing(int8 enable);
int32 rtl8651_fwdEngineEnableWanRouting(int8 enable);
int32 rtl8651_fwdEngineFwdGeneralL4ToDMZ(int8 enable);
int32 rtl8651_fwdEngineFwdUnicastIGMPPkt(int8 enable);
int32 rtl8651_fwdEnginePortBouncingDoProtocolStackAction(int8 enable);
int32 rtl8651_fwdEngineTrapPktDemandRouteTrigger(int8 enable);
int32 rtl8651a_setDSCPDefaultAction(int8 enable, uint8 value);

#ifdef _RTL_NAPT_ADVANCED
int32 rtl8651_fwdEngineInexactUdpFlow(int8 enable);
/* int32 rtl8651_fwdEngineInexactTcpFlow(int8 enable); */
int32 rtl8651_fwdEngineUdpStaticNaptPortTranslation(int8 enable);
int32 rtl8651_fwdEngineTcpStaticNaptPortTranslation(int8 enable);
#endif /* _RTL_NAPT_ADVANCED */

#ifdef _RTL_WEAK_NAPT_CTRL
int32 rtl8651_fwdEngineWeakTcpNaptProcess(int8 enable);
#endif /* _RTL_WEAK_NAPT_CTRL */

int32 rtl8651_fwdEnginePsDontTimeout(int enable);
int32 rtl8651_fwdEnginePsToWanErrMsgDontTimeout(int enable);
int32 rtl8651_fwdEnginePsToWanErrMsgDrop(int enable);

int8 _rtl8651_l4CheckOccupiedConnection(uint32 isTcp, uint32 dsid, ipaddr_t IntIp, uint16 IntPort, ipaddr_t ExtIp, uint16 ExtPort, ipaddr_t RemoteIp, uint16 RemotePort);

int32 rtl8651_fwdEngineDynamicRemoteHostPolicyRT(int enable);

int32 rtl8651_fwdEnginePortBouncingAlgSupport(int enable);

/*********************************************************************************************************
	ALG
**********************************************************************************************************/

/* modify the interface of function "rtl8651_fwdEngineRegisterAlgModule" and "rtl8651_fwdEngineRemoveAlgModule" */
struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s;	/* to define a NAPT Flow */
typedef int32 (*alg_funcptr)(struct rtl_pktHdr *, struct ip *, struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s *);

 int32 rtl8651_fwdEngineRegisterAlgModule( 
 	uint16 startPort, uint16 endPort, uint8 isTcp, 
 	rtl8651_algEntry_property_t algtypes,
 	int32 (*alg_init)(void *),
	alg_funcptr alg_outboundClientcb,
	alg_funcptr alg_inboundClientcb,
	alg_funcptr alg_outboundServercb,
	alg_funcptr alg_inboundServercb );

int32 rtl8651_fwdEngineRemoveAlgModule( uint16 startPort, uint16 endPort, uint16 isTcp );


/* rtl8651_layer2fwd.c function declare for EXPORT*/

int32 rtl8651_fwdEngineAddWlanSTA(uint8 *smac,uint16 myvid, uint32 myportmask, uint32 linkId);
int32 rtl8651_fwdEngineDelWlanSTA(uint8 *mac,uint16 myvid);
int32 rtl8651_fwdEngineExtPortUcastFastRecv(struct rtl_pktHdr *pkt,uint16 myvid, uint32 myportmask);
/*********************************************************************************************************
	Counter
**********************************************************************************************************/
typedef struct rtl8651_tblDrvFwdEngineCounts_s {
	uint32 arpReceive, arpAddSuccess, arpAddFail, arpDelete, arpReply, arpGenerate;
	uint32 ipReceive, ipErrorReceive;
	uint32 icmpReceive, icmpErrorReceive, icmpReply, icmpReplyFailed, icmpDrop, icmpSend;
	uint32 igmpReceive, igmpErrorReceive;
	uint32 udpReceive, udpErrorReceive,udpSend;
	uint32 tcpReceive, tcpErrorReceive,tcpSend;
	uint32 l34actionLookupFailed, l34actionFailed;
	uint32 pppoeDiscoveryReceive, pppoeDiscoveryProcessed;
	uint32 pppoeSessionReceive, pppoeSessionProcessed;
	uint32 inAclDrop, inAclPermit,inAclCpu;
	uint32 outAclDrop, outAclPermit, outAclCpu;
	uint32 dmzDrop;
	uint32 natdrop, wlan2lanRelay, lan2wlanRelay;
	uint32 dosDrop;
	uint32 fastFragUdpMeetCount, fastFragUdpMissCount;
} rtl8651_tblDrvFwdEngineCounts_t;

int32 rtl8651_resetFwdEngineCounter(void);
int32 rtl8651_getFwdEngineCounter(rtl8651_tblDrvFwdEngineCounts_t * counter);

/*********************************************************************************************************
	UPnP
**********************************************************************************************************/
#ifdef _RTL_UPNP

#define UPNP_VALID				(1<<0)
#define UPNP_TCP				(1<<1)
#define UPNP_ONESHOT			(1<<2)
#define UPNP_PERSIST			(1<<3)
#define UPNP_PORTBOUNCING		(1<<4)
/* we reserve 3 bits for internal control */
#define UPNP_INTERNAL_FLAG		0xe0

void rtl8651_addUpnpMap(uint32 ,ipaddr_t , uint16 , ipaddr_t , uint16 , ipaddr_t , uint16 );
void rtl8651_addUpnpMapLeaseTime(uint32, ipaddr_t, uint16, ipaddr_t, uint16, ipaddr_t, uint16,uint32);
void rtl8651_delUpnpMap(uint32 ,ipaddr_t , uint16 , ipaddr_t , uint16 ,	ipaddr_t , uint16 );
uint32 rtl8651_queryUpnpMapTimeAge(uint32, ipaddr_t, uint16 , ipaddr_t, uint16);
#endif /* _RTL_UPNP */

/*********************************************************************************************************
	Trigger port
**********************************************************************************************************/
#ifdef _RTL_TRIGGER_PORT

typedef struct _rtl8651_triggerPort_param_s
{
	uint8 isStaticMapping;
	uint32 age;

	union {
		struct {
			uint8		__outType;
			ipaddr_t		__triggeringInsideLocalIpAddrStart;
			ipaddr_t		__triggeringInsideLocalIpAddrEnd;
			uint16		__outPortStart;
			uint16		__outPortEnd;
		} DYNAMIC_MAPPING;
		struct {
			ipaddr_t		__mappedInsideLocalIpAddr;
			uint8		__isStatic;
		} STATIC_MAPPING;
	} UN_MAPPING;

	#define	tp_outType							UN_MAPPING.DYNAMIC_MAPPING.__outType
	#define	tp_triggeringInsideLocalIpAddrStart	UN_MAPPING.DYNAMIC_MAPPING.__triggeringInsideLocalIpAddrStart
	#define	tp_triggeringInsideLocalIpAddrEnd	UN_MAPPING.DYNAMIC_MAPPING.__triggeringInsideLocalIpAddrEnd
	#define	tp_outPortStart						UN_MAPPING.DYNAMIC_MAPPING.__outPortStart
	#define	tp_outPortEnd						UN_MAPPING.DYNAMIC_MAPPING.__outPortEnd

	#define	tp_mappedInsideLocalIpAddr			UN_MAPPING.STATIC_MAPPING.__mappedInsideLocalIpAddr
	#define	tp_isStatic							UN_MAPPING.STATIC_MAPPING.__isStatic

}_rtl8651_triggerPort_param_t;



void rtl8651a_flushTriggerPortRules(uint32 netIntfId);
void rtl8651_flushTriggerPortRules(void);
int32 rtl8651a_addTriggerPortRule(uint32 netIntfId, uint8 inType, uint16 inPortStart, uint16 inPortEnd, uint8 outType,uint16 outPortStart,uint16 outPortEnd, ipaddr_t localIpAddr);
int32 rtl8651_addTriggerPortRule(uint8 inType, uint16 inPortStart, uint16 inPortEnd, uint8 outType,uint16 outPortStart,uint16 outPortEnd, ipaddr_t localIpAddr);

#endif /* _RTL_TRIGGER_PORT */

void rtl8651a_setUdpSizeThreshValue(uint32 dsid, uint32 value);

/*********************************************************************************************************
	Protocol stack action
**********************************************************************************************************/
#if 0
void rtl8651_flushProtoStackUsedUdpPorts(void);
void rtl8651_flushProtoStackServerUsedTcpPorts(void);
int32 rtl8651_addProtoStackUsedUdpPort(uint16  s_port);
int32 rtl8651_addProtoStackServerUsedTcpPort(uint16  s_port);
#endif
void rtl8651_flushProtoStackActions(void);
int32 rtl8651_addProtoStackActions(ipaddr_t ip,ipaddr_t mask,uint8 action,uint8 role,uint8 protocol,uint16 orgPort,uint16 fwdPort);

/*********************************************************************************************************
	plug-in
**********************************************************************************************************/
#if 0
typedef int32 (*user_ids_chkFunc)(struct rtl_pktHdr *);
int32 rtl8651_fwdEngineRegUserIDScheckFunc(user_ids_chkFunc func);
#endif

/*********************************************************************************************************
	udp blocking
**********************************************************************************************************/
void rtl8651a_setUdpSizeThreshValue(uint32 SessionID, uint32 value);

/*********************************************************************************************************
	trapping dispatch system
**********************************************************************************************************/
#ifdef _RTL_TRAPPING_DISPATCH

#define RTL8651_DEFAULT_TRAPPING_DISPATCH_CATEGORY		0	/* default category or trapping dispatch system */

int32 rtl8651_registerTrappingDispatchEntry(uint8 l4Proto, ipaddr_t srcIp, ipaddr_t dstIp, int32 srcPort, int32 dstPort, uint16 category);
int32 rtl8651_unregisterTrappingDispatchEntry(uint8 l4Proto, ipaddr_t srcIp, ipaddr_t dstIp, int32 srcPort, int32 dstPort);
#endif /* _RTL_TRAPPING_DISPATCH */
/*********************************************************************************************************
	napt redirect
**********************************************************************************************************/
#ifdef _RTL_NAPT_REDIRECT
int32 rtl8651_registerRedirectOutboundNaptFlow(
		uint8	isTcp,			/* TRUE: tcp, FALSE: udp */
		ipaddr_t	sip,				/* val 0 for wildcard */
		uint16	sport,			/* val 0 for wildcard */
		ipaddr_t	originalDip,		/* val 0 for wildcard */
		uint16	originalDport,	/* val 0 for wildcard */
		ipaddr_t	newDip,			/* val 0 for don't care, automatically assigned by gateway's LAN IP */
		int32	newDport		/* must assign a new Dport to protocol stack, if < 0, both 'newDip' and 'newDport' will not be set(original dip/dport are kept) */
);
int32 rtl8651_unregisterRedirectOutboundNaptFlow(
		uint8	isTcp,			/* TRUE: tcp, FALSE: udp */
		ipaddr_t	sip,				/* val 0 for wildcard */
		uint16	sport,			/* val 0 for wildcard */
		ipaddr_t	originalDip,		/* val 0 for wildcard */
		uint16	originalDport,	/* val 0 for wildcard */
		ipaddr_t	newDip,			/* val 0 for wildcard */
		int32	newDport		/* val 0 for wildcard */
);
int32 rtl8651_queryRedirectOutboundNaptFlow(
		uint8	isTcp,
		ipaddr_t	sip,
		uint16	sport,
		ipaddr_t	*pOriginalDip,
		uint16	*pOriginalDport,
		ipaddr_t	newDip,
		uint16	newDport
);
int32 rtl8651_registerRedirectOutboundNaptExceptionList(
		uint8	isTcp,			/* TRUE: tcp, FALSE: udp */
		ipaddr_t	sip,				/* val 0 for wildcard */
		uint16	sport,			/* val 0 for wildcard */
		ipaddr_t	dip,				/* val 0 for wildcard */
		uint16	dport			/* val 0 for wildcard */
);
int32 rtl8651_unregisterRedirectOutboundNaptExceptionList(
		uint8	isTcp,			/* TRUE: tcp, FALSE: udp */
		ipaddr_t	sip,				/* val 0 for wildcard */
		uint16	sport,			/* val 0 for wildcard */
		ipaddr_t	dip,				/* val 0 for wildcard */
		uint16	dport			/* val 0 for wildcard */
);
#endif /* _RTL_NAPT_REDIRECT */

/*********************************************************************************************************
	Counter
**********************************************************************************************************/
int32 rtl8651_updateCounter1( void );

/*********************************************************************************************************
	Offload
**********************************************************************************************************/
#define OFFLOAD_IP_CKSUM		0x0001
#define OFFLOAD_L4_CKSUM		0x0002
int32 rtl8651_offloadEngineL3Send(void * pPkt, uint32 L3PktLength, uint32 l34CkumOffload, int32 highPriority);

#endif /* RTL8651_FWDENGINE_H */

