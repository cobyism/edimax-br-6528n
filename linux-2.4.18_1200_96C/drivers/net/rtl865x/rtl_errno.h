
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of all error numbers
* Abstract :                                                           
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)               
* $Id: rtl_errno.h,v 1.2 2007/12/04 12:00:18 joeylin Exp $
* $log$
* -------------------------------------------------------
*/

#ifndef _RTL_ERRNO_H
#define _RTL_ERRNO_H

#if 1
#define	EINVAL		22				   /* Invalid argument */
#define	EEXIST		17				   /* File exists */
#define	EEMPTY		87				   /* Empty slot */
#define	ECOLLISION	88				   /* Table entry collision */
#else
#define	EPERM		1				   /* Operation not permitted */
#define	ENOENT		2				   /* No such file or directory */
#define	ESRCH		3				   /* No such process */
#define	EINTR		4				   /* Interrupted system call */
#define	EIO		5					   /* Input/output error */
#define	ENXIO		6				   /* Device not configured */
#define	E2BIG		7				   /* Argument list too long */
#define	ENOEXEC		8				   /* Exec format error */
#define	EBADF		9				   /* Bad file descriptor */
#define	ECHILD		10				   /* No child processes */
#define	EDEADLK		11				   /* Resource deadlock avoided */
					/*
					   11 was EAGAIN 
					 */
#define	ENOMEM		12				   /* Cannot allocate memory */
#define	EACCES		13				   /* Permission denied */
#define	EFAULT		14				   /* Bad address */
#define	ENOTBLK		15				   /* Block device required */
#define	EBUSY		16				   /* Device busy */
#define	EEXIST		17				   /* File exists */
#define	EXDEV		18				   /* Cross-device link */
#define	ENODEV		19				   /* Operation not supported by device */
#define	ENOTDIR		20				   /* Not a directory */
#define	EISDIR		21				   /* Is a directory */
#define	EINVAL		22				   /* Invalid argument */
#define	ENFILE		23				   /* Too many open files in system */
#define	EMFILE		24				   /* Too many open files */
#define	ENOTTY		25				   /* Inappropriate ioctl for device */
#define	ETXTBSY		26			   /* Text file busy */
#define	EFBIG		27				   /* File too large */
#define	ENOSPC		28				   /* No space left on device */
#define	ESPIPE		29				   /* Illegal seek */
#define	EROFS		30				   /* Read-only file system */
#define	EMLINK		31				   /* Too many links */
#define	EPIPE		32				   /* Broken pipe */

/* math software */
#define	EDOM		33				   /* Numerical argument out of domain */
#define	ERANGE		34				   /* Result too large */

/* non-blocking and interrupt i/o */
#define	EAGAIN		35				   /* Resource temporarily unavailable */
#define	EWOULDBLOCK	EAGAIN		   /* Operation would block */
#define	EINPROGRESS	36			   /* Operation now in progress */
#define	EALREADY	37				   /* Operation already in progress */

/* ipc/network software -- argument errors */
#define	ENOTSOCK	38				   /* Socket operation on non-socket */
#define	EDESTADDRREQ	39			   /* Destination address required */
#define	EMSGSIZE	40				   /* Message too long */
#define	EPROTOTYPE	41				   /* Protocol wrong type for socket */
#define	ENOPROTOOPT	42			   /* Protocol not available */
#define	EPROTONOSUPPORT	43		   /* Protocol not supported */
#define	ESOCKTNOSUPPORT	44		   /* Socket type not supported */
#define	EOPNOTSUPP	45				   /* Operation not supported */
#define ENOTSUP		EOPNOTSUPP		   /* Operation not supported */
#define	EPFNOSUPPORT	46			   /* Protocol family not supported */
#define	EAFNOSUPPORT	47			   /* Address family not supported by protocol family */
#define	EADDRINUSE	48				   /* Address already in use */
#define	EADDRNOTAVAIL	49			   /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define	ENETDOWN	50				   /* Network is down */
#define	ENETUNREACH	51			   /* Network is unreachable */
#define	ENETRESET	52				   /* Network dropped connection on reset */
#define	ECONNABORTED	53			   /* Software caused connection abort */
#define	ECONNRESET	54				   /* Connection reset by peer */
#define	ENOBUFS		55			   /* No buffer space available */
#define	EISCONN		56			   /* Socket is already connected */
#define	ENOTCONN	57				   /* Socket is not connected */
#define	ESHUTDOWN	58				   /* Can't send after socket shutdown */
#define	ETOOMANYREFS	59			   /* Too many references: can't splice */
#define	ETIMEDOUT	60				   /* Operation timed out */
#define	ECONNREFUSED	61			   /* Connection refused */

#define	ELOOP		62				   /* Too many levels of symbolic links */

#define	ENAMELENMISMATCH	63		   /* socket addr length mismatch */
#define ENAMETOOLONG  63  /* file name too long */

/* should be rearranged */
#define	EHOSTDOWN	64				   /* Host is down */
#define	EHOSTUNREACH	65			   /* No route to host */
#define	ENOTEMPTY	66				   /* Directory not empty */

/* quotas & mush */
#define	EPROCLIM	67				   /* Too many processes */
#define	EUSERS		68				   /* Too many users */
#define	EDQUOT		69				   /* Disc quota exceeded */

/* Network File System */
#define	ESTALE		70				   /* Stale NFS file handle */
#define	EREMOTE		71				   /* Too many levels of remote in path */
#define	EBADRPC		72				   /* RPC struct is bad */
#define	ERPCMISMATCH	73			   /* RPC version wrong */
#define	EPROGUNAVAIL	74			   /* RPC prog. not avail */
#define	EPROGMISMATCH	75			   /* Program version wrong */
#define	EPROCUNAVAIL	76			   /* Bad procedure for program */

#define	ENOLCK		77				   /* No locks available */
#define	ENOSYS		78				   /* Function not implemented */

#define	EFTYPE		79				   /* Inappropriate file type or format */
#define	EAUTH		80				   /* Authentication error */
#define	ENEEDAUTH	81				   /* Need authenticator */
#define	EIDRM		82				   /* Identifier removed */
#define	ENOMSG		83				   /* No message of desired type */
#define	EOVERFLOW	84				   /* Value too large to be stored in data type */
#define	ECANCELED	85				   /* Operation canceled */
#define	EILSEQ		86				   /* Illegal byte sequence */
#define EEMPTY      87                 /* Empty slot */
#define ECOLLISION  88                 /* Table entry collision */
#define	ELAST		88				   /* Must be equal largest errno */

/* pseudo-errors returned inside kernel to modify return to process */
#define	ERESTART	(-1)			   /* restart syscall */
#define	EJUSTRETURN	(-2)			   /* don't modify regs, just return */
#define	ENOIOCTL	(-3)			   /* ioctl not handled by this layer */
#endif

//Table Driver ERROR NUMBER Definition

/* Error Number for common use */
#define TBLDRV_DUMMY						-1 // NEVER use me!!! '-1' is used by FAILED.
#define TBLDRV_EENTRYALREADYEXIST			-2 //The specifyied entry already exists.
#define TBLDRV_EENTRYNOTFOUND			-3 //The specified entry was not found.
#define TBLDRV_EINVALIDPORT				-4 //Unknown port number
#define TBLDRV_EINVALIDVLANID				-5 //Invalid VLAN ID.
#define TBLDRV_EINVALIDINPUT				-6 //Invalid input parameter.
#define TBLDRV_EINVNETIFNAME				-7 //Invalid Network Interface name
#define TBLDRV_ENEEDLOOPBACKSET			-8 //Need loopback port set
#define TBLDRV_ENOFREEBUFFER				-9 //No Free buffer for new request
#define TBLDRV_EINVALIDIPRANGE				-10//Invalid IP RANGE
#define TBLDRV_EINVALIDIPFILTER			-11//Invalid Ip filter which for filter pkt's sip or dip

/* Error Number for Port Mirror */
#define TBLDRV_EPORTISAMIRRORPORT		-1000 //The specified port is already configured as a mirror port
#define TBLDRV_EPORTISARXTXMIRRORED			-1001 //The specified port is a Rx/Tx mirrored port
#define TBLDRV_EPORTCANTBEAGGREGATED	-1002 //The Mirror port cannot be aggregated.
#define TBLDRV_EMIRRORPORTNOTFOUND		-1003 //The specified Mirror port not found.

/* DMZ Host error number */
#define TBLDRV_EDMZHOSTNOTFOUND			-1100 //DMZ host not found
#define TBLDRV_EDMZHOSTNEEDNAPTIP		-1101 //DMZ host needs NAPT Ip address present
#define TBLDRV_EDMZHOSTSHOULDBEINTIP		-1102 //DMZ host should be internal ip address
#define TBLDRV_EDMZHOSTCANNOTBEGWIP		-1103 //DMZ host cannot be gateway ip address
#define TBLDRV_EDMZHOSTFULL				-1104 //DMZ host array is full.
#define TBLDRV_EDMZHOSTDENYFILTEREDSIP	-1105 //filtered PKT to DMZ because of the PKT's Sip in filterring range

/* Error Number for ALG APIs */
#define TBLDRV_EINVALALGATTR			-1200 //Invalid ALG attribute. Neither Server nor Client is 
											  //specified to an ALG entry.
#define TBLDRV_EINVALPORTRANGE			-1201 //Invalid port range was specified.
#define TBLDRV_EALGPORTRANGEOVERLAP		-1202 //ALG port range overlap

/* Error Number for Protocol Trap APIs */
#define TBLDRV_EUNKNOWPROTOTYPE			-1300 //Unknown Protocol Trap's Protocol type


/* Error Number for ACL rule APIs */
#define TBLDRV_EACLRULEISNULL			-1400 //The specified ACL rule is NULL
#define TBLDRV_EINVALACTIONTYPE			-1401 //ACL rule with invalid action type
#define TBLDRV_EACLMODEISNOTALLOWED		-1402 //ACL mode is not match.
#define TBLDRV_EACLRULE_MATCHTYPEINCORRECT 	-1403 /* Incorrect match type */
#define TBLDRV_EACLEGRESSRULE_NOTSUPPORTPKTOP	-1404	/* Egress ACL not support packet operation */
#define TBLDRV_EACLINVALIDRULETYPE				-1405	/* Invalid acl rule type */

/* Error Number for NAPT APIs */
#define TBLDRV_ENAPTIPNOTEXTIP			-1500 //The Specified NAPT IP is not an external IP address
#define TBLDRV_ENAPTIPCANNOTBENAT		-1501 //The Specified NAPT IP can not a NAT IP address

/* Error Number for Ethernet port */
#define TBLDRV_EPORTOPMODECANNOTAGGREGATE	-1600 //Can not set a port's operation mode if the port is aggreaged.
#define TBLDRV_EUNKNOWPORTSPEED				-1601 //Unknow port speed. The port speed should be either 10 or 100.
#define TBLDRV_EPORTSPEEDCANNOTAGGREGATE 	-1602 //The specified port's speed can not be changed if the port is aggreaged.
#define TBLDRV_EPORTAUTONEGOCANNOTAGGREGATE	-1603 //Can not change a port's auto-negotiation capability if the port is aggregated.
#define TBLDRV_EINVALIDDATARATE				-1604 //Invalid data rate.
#define TBLDRV_ENOTVLANPORTMEMBER			-1605 //Specified port is not a member port of specified vlan.

/* Error Number for Spanning Tree */
#define TBLDRV_EINVALIDSID					-1700 //Invalid spanning tree ID.
#define TBLDRV_ESIDALREADYEXIST				-1701 //Spannding Tree ID already exists.
#define TBLDRV_EINVALIDPORTSTATE			-1702 //Invalid Port State.
#define TBLDRV_ESIDISREFERENCEEDBYFID		-1703 //Spanning Tree is being referenced by Filter Database.

/* Error Number for Filter Database */
#define TBLDRV_EINVALIDFID					-1800 //Invalid Filter database ID
#define TBLDRV_EFIDALREADYEXISTS			-1801 //Filter Database already exists.
#define TBLDRV_EFIDISREFERENCEDBYVLAN		-1802 //Filter Database is being referenced by VLAN
#define TBLDRV_EFIDISNOTEMPTY				-1803 //Filter Database is not empty
#define TBLDRV_EINVALIDACTIONTYPE			-1804 //Invalid Port Action Type.
#define TBLDRV_ENULLMACADDR					-1805 //The specified MAC address is NULL.
#define TBLDRV_EL2ENTRYEXISTS				-1806 //The filter database entry already exists.
#define TBLDRV_EL2ENTRYNOTFOUND				-1807 //The specified filter database entry was not found.
//#ifdef TEST_GETL2TBL
#define TBLDRV_EGETL2_EMPTYL2TBL				-1808	/* The L2 table is empty */
#define TBLDRV_EGETL2_INVALIDL2ENTRY			-1809	/* User specifies an invalid MAC address */
#define TBLDRV_EGETL2_NOMOREVALIDL2ENTRY	-1810	/* No more valid L2 entry */
//#endif

/* Error Number for Aggregator */
#define TBLDRV_EINVALIDAGGREGATORID			-1900 //Invalid aggregator ID.
#define TBLDRV_EALREADYTRUNKING				-1901 //Aggregator already aggregates more than one port.
#define TBLDRV_EAGGREGATORHASNOPORT			-1902 //Not allow aggregator without port member 
#define TBLDRV_EAGGREGATOREXCEED			-1903 //The number of aggregator excees the system capability.
#define TBLDRV_EAGGREGATORIDGTPORTNO		-1904 //Aggreagator ID > Port Number. When aggregating ports, 
												  //any port No. > aggregator ID is disallowed.
#define TBLDRV_EAGGREGATORSETINDIVIDUAL		-1905 //Aggregator was set to individual, hence it can not 
												  //aggreate more then one port.
#define TBLDRV_EDIFFBROADCASTDOMAIN			-1906 //Aggregator and port are in different broadcast domain.											  

/* Error Number for VLAN */
#define TBLDRV_EVLANALREADYEXISTS			-2000 //Vlan already exists.
#define TBLDRV_ECANNOTREMOVEDEFVLAN			-2001 //Cannot remove default VLAN.
#define TBLDRV_EVLANISREFERENCEDBYNETIF		-2002 //Vlan is referenced by network interface.
#define TBLDRV_EVLANISREFERENCEDBYPPPOE		-2003 //Vlan is referenced by PPPoE.
#define TBLDRV_EVLANHASMACALLOCATED			-2004 //Vlan still has MAC address allocated.
#define TBLDRV_EVLANHASHPORTMEMBER			-2005 //Vlan can not have member port while specifying its Filter database.
#define TBLDRV_EVLANTXMIRRORSET				-2006 //Vlan has Tx mirror set hence can not set to promiscuous mode.
#define TBLDRV_EPORTNOTVLANMEMBER			-2007 //The specified port is not a member of specified VLAN.
#define TBLDRV_EVLANPROMISSET				-2008 //If VLAN promiscuous is set, a network interface cannot attach to the VLAN.
#define TBLDRV_EFWDTXMIRRORSET				-2009 //Vlan Forwarding Tx Mirror set.
#define TBLDRV_EVLANMACREFERENCEDBYPPPOE	-2010 //Vlan MAC is referenced by pppoe
#define TBLDRV_EVLANMACREFERENCEDBYNETIF	-2011 //VLan MAC is referenced by network interface.
#define TBLDRV_EINVVLANMAC					-2012 //Specified VLAN MAC number should be (0, 1, 2, 4, 8)
#define TBLDRV_ENOUSABLEMAC					-2013 //No usable MAC address can be allocated
#define TBLDRV_EVLANNOMAC					-2014 //Vlan has no MAC Address
#define TBLDRV_ENOVLAN						-2015 //No more availiable VLAN entry



/* Error Number for NAT */
#define TBLDRV_ENATIPNOTEXTERNALIP			-2100 //The specified NAT IP address is not an External IP address.
#define TBLDRV_ENATIPNOTINTERNALIP			-2101 //The specified NAT Internal IP is not an Internal IP address.
#define TBLDRV_EDUPLOCATENATIP				-2102 //The specified NAT IP address already maps to an Internal IP address.
#define TBLDRV_ENATIPCANNOTBENAPTIP			-2103 //The specified NAT IP address cannot be a NAPT IP address.


/* Error Number for Routing */
#define TBLDRV_ERTALREADYEXIST				-2300 //The routing entry already exists.
#define TBLDRV_ERTINVALNEXTHOP				-2301 //Invalid nextHop
#define TBLDRV_ENOSESSIONALLOCATE			-2302 //Need PPPoE session allocate
/*#ifdef TEST_GETRTTBL*/
#define TBLDRV_ERTCREATEBYIF				-2303 //The route is added by IP interface creation. It can be removed.
#define TBLDRV_EGETRT_EMPTYRTTBL			-2304	/* The routing table is empty */
#define TBLDRV_EGETRT_NOMOREVALIDENTRY	-2305	/* No more valid routing entry */
#define TBLDRV_EGETRT_INVALIDRTENTRY		-2306	/* User specifies an invalid route */
/*#endif */



/* Error Number for Arp */
#define TBLDRV_ENONBROADCASTNET				-2400 //Non Boradcast network is forbidden to add ARP entry.
#define TBLDRV_EARPALREADYEXIST				-2401 //Arp entry already exists.
#define TBLDRV_EARPCANNOTADDL2ENTRY			-2402 //The L2 Entry cannot be added.
#define TBLDRV_EARPCANNOTDELL2ENTRY			-2403 //The L2 Entry cannot be deleted.
#define TBLDRV_ENOARPFOUND					-2404 //No ARP entry was found


/* Error Number for Network interface */
#define TBLDRV_ENETIFREFERENCEDBYIPIF			-2500 //The network interface is referenced by IP interface.
#define TBLDRV_ENETIFREFBYROUTE				-2501 //The network interface is referenced by a routing entry.
#define TBLDRV_ELINKTYPESHOULDBERESET			-2502 //The link layer type should be reset before removing.
#define TBLDRV_ENETIFREFERENCEDBYACL			-2503 //The network interface is referenced by ACL.
#define TBLDRV_EUNKNOWLINKLAYERTYPE			-2504 //Unknow link layer type.
#define TBLDRV_ECANNOTREMOVEIPUNNUMBER		-2505 //Can not remove IP unnumbered network itnerface. Use rtl8651_delIpUnnumbered() first.
#define TBLDRV_ENETHASNOLLTYPESPECIFY			-2506 //The specified network interface has no link-layer type specified.
#define TBLDRV_ENETISEXTERNAL					-2507 //The removed network interface is an external interface.
#define TBLDRV_ENAPTSTART						-2508 //When NAPT starts, no manipulation about IP interface
												  //should be done
#define TBLDRV_ENETIFALREADYEXTERNAL			-2509 //The specified network interface already is an external network interface.
#define TBLDRV_ESHOULDBEEXTIF					-2510 //The network interface should be an external network interface
#define TBLDRV_ESHOULDNOTDMZEXT				-2511 //The network interface should not be a DMZ external.
#define TBLDRV_ELIPUNNUMBEREDNOTALLOW		-2512 //The specified network itnerface cannot be IP Unnumbered network.
#define TBLDRV_ENETIFREFBYNATNAPT				-2513 //Network Interface is referenced BY NAT
#define TBLDRV_ENETIFLLTYPEALREADYSET			-2514 //Link layer of the specified network itnerface type already set.
#define TBLDRV_ENOLLTYPESPECIFY				-2515 //The network interface has no link layer type specified.


/* Error Number for IP Unnumbered */
#define TBLDRV_EONLYONEIPUNNUMBERISALLOWED	-2600 //The whole system only allows one IP unnumbered network.
#define TBLDRV_ENETIFINVALID				-2601 //One external and one internal network interfaces are needed.
#define TBLDRV_EWANIFCANNOTHAVEIPINTF		-2602 //The external network interface cannot have IP interface.
#define TBLDRV_NETIFTYPENOTMATCH			-2603 //The external network interface should be PPPoE type and internal interface should be VLAN type.
#define TBLDRV_ENETIFNEEDIPUNNUMBERED		-2604 //The specified network itnerface should be IP unnumbered.
#define TBLDRV_EIPUNNUMBEREDHASHIPINTF		-2605 //The IP Unnumebred still has external IP interface unnumebred attached.
#define TBLDRV_ENETIFNOTIPUNNUMBERED		-2606 //The specified network interface is not an IP Unnumbered Network interface.
#define TBLDRV_ENOTIPUNNUMBEREDNETIF	  	-2607 //The specified network interface is not IP Unnumbered Network interface.
#define TBLDRV_EIPUNINTIPINTFNEED			-2608 //The specified network interface should be IP Unnumbered internal network interface.
#define TBLDRV_EIPUNISREFERENCEDBYPPPOE		-2609 //The IP Unnumbered IP interface is referenced by PPPoE Session.
#define TBLDRV_EEXTINTFSHOULDBEPPPOE		-2610 //The specified external network interface should be PPPoE type.
#define TBLDRV_ESHOULDBEEXTNETIF			-2611 //The specified network interface should be external network interface.


/* Error Number for IP interface */
#define TBLDRV_ENETMASKCANNOTBEZERO			-2701 //The netmask of IP interface cannot be zero.
#define TBLDRV_EIPUNEXTIFCANNOTHASHIPINTF	-2702 //Cannot attach IP interface to External Network itnerface in an IP Unnumbered network.
#define TBLDRV_EIPIFALREADYEXIST			-2703 //IP Interface already exists
#define TBLDRV_EINVNETMASK					-2704 //Invalid network mask. The specified netmask may be 0xffffffff in a VLAN type network.
#define TBLDRV_ECANNOTREMOVEIPUNEXTIPINTF	-2705 //Global IP interface of IP Unnumbered cannot be removed. Use rtl8651_setLanSideExternalIpInterface() first.
#define TBLDRV_EIPINTFISREFERENCEDBYARP		-2706 //IP interface is referenced by ARP entry. It cannot be removed.
#define TBLDRV_EIPINTFISREFERENCEDBYLS		-2707 //IP interface is referenced by Local Server. It cannot be removed.
#define TBLDRV_EIPINTFISREFERENCEDBYRT		-2708 //IP interface is referenced by routing entry. It cannot be removed.
#define TBLDRV_EIPINTFISREFERENCEDNYNATNAPT	-2709 //IP interface is referenced by NAT/NAPT. It cannot be removed.
#define TBLDRV_ENOIPINTFFOUND				-2710 //The specified IP interface was not found.
#define TBLDRV_ENOTEXTIP					-2711 //Specified external IP address doesn't belong to external interface
#define TBLDRV_ENOTINTIP					-2712 //Specified internal IP address doesn't belong to internal interface

/* Error Number for PPPoE */
#define TBLDRV_EFORBIDPROMISCINPPPOE		-2800 //If promiscuous mode was set, PPPoE cannot be created.
#define TBLDRV_EVLANSHOULDBEPPPOE			-2801 //The specified VLAN should be PPPoE Type.
#define TBLDRV_EPPPOEALREADYEXISTS			-2802 //The specified PPPoE already exists.
#define TBLDRV_EOUTOFVLANMACADDR			-2803 //Out of VLAN MAC address.
#define TBLDRV_EINVALIDPPPOEID				-2804 //Invalid PPPoE ID.
#define TBLDRV_EPPPOEISINUSE				-2805 //The specified PPPoE is in use.
#define TBLDRV_EPPPOEISREFERENCEDBYRT		-2806 //PPPoE is referenced by routing entry.
#define TBLDRV_EINVALIDSESSIONID			-2807 //Invalid PPPoE Session ID.
#define TBLDRV_EPROPERTUALREADYSET			-2808 //The specified PPPoE property was already set to another PPPoE ID.
#define TBLDRV_EPPPOEHASPROPERTYSET			-2809 //The specified PPPoE ID already has property set.
#define TBLDRV_EPPPOECANNOTADDL2ENTRY		-2810 //Cannot add L2 entry for the specified PPPoE.

/* Error Number for Local Server */
#define TBLDRV_EINVALIDLSIPADDR				-2900 //Invalid Local Server IP address.
#define TBLDRV_ELSCANNOTBEGWIP				-2901 //Local Server IP address cannot be gateway's IP address.
#define TBLDRV_ELSALREADYEXISTS				-2902 //Local Server already exists.

/* Error Number for NAPT */
#define TBLDRV_EDSTIPISLS					-3000 //NAPT DST IP is Local Server
#define TBLDRV_EGETOFFSETFAIL				-3001 //NAPT get offset fail
#define TBLDRV_EINVALIDEXTPORT				-3002 //Maybe the external port is in use or reserved
#define TBLDRV_ENOTPERMIT					-3003 //The action is not allowed
#define TBLDRV_EEXTIDINUSE					-3004 //NAPT ICMP external ID is in use
#define TBLDRV_EDRVNAPTEXIST				-3005 //NAPT connection already exist in driver table
#define TBLDRV_EINVL4PORTNUM				-3006 //Invalid UDP/TCP port number
#define TBLDRV_EINVEXTIP					-3007 //Invliad external port number for NAPT
#define TBLDRV_EDEFAULTROUTENOTFOUND		-3008 // Default Route is not defined.
#define TBLDRV_EADDHASH2NEXTHOP				-3009 // Add Hash2 nexthop error
#define TBLDRV_EDEFAULTNAPTIPEXIST			-3010 //Default Napt IP already exists
#define TBLDRV_ENAPTCONNECTIONFULL		-3011	// napt system is full and can not add any other new flows
#define TBLDRV_ENAPTNOTFOUNDWITHINFO		-3012 // Entry is not found, but return with infomation (for GetInbound()/Outbound())

/*Error Number for NAPTUSRMAPPING*/
#define TBLDRV_EIPFILTEROVERLAPPING		-3050	//ipfilter overlap: the ip range and port is overloapping
#define TBLDRV_ENAPTUSRMAPTYPEERROR		-3051	//napt user map entry's type is error
#define TBLDRV_ENAPTUSRMAPFLAGERROR		-3052	//napt user map entry's flag is error
#define TBLDRV_ENAPTUSRMAPFILTERNOTFOUND	-3053	// napt user map ipFilter not found
#define TBLDRV_ENAPTUSRMAPDIRECTIONERROR	-3054	// napt usr map info's direction is error
#define TBLDRV_ENAPTUSRMAPFILTERDROP		-3055	//pkt was drop by usr filter

/* Error Numebr for ALG */
#define TBLDRV_ERANGEOVERLAP				-3100	//ALG port range overlap

/* Error Number Session: PPPoE, L2TP, PPTP */
#define TBLDRV_ESESSIONNOTFOUND			-3200	//The specified Session ID is not found
#define TBLDRV_ESESSIONREFERENCEDBYRT		-3201	//The specified Session is referenced by routing entry
#define TBLDRV_ECANNOTMOVESESSION		-3202	//Can not move session to another network interface
#define TBLDRV_EALREADYHAVESESSION		-3203	//The specified network interface already has session attached
#define TBLDRV_ESESSIONISREFERENCED		-3204	//Session is referenced
#define TBLDRV_ECANNOTUSETHISVID			-3205	//No more than one PPTP/L2TP VLAN is allowed

/* Error Number for Protocol-based NAT */
#define TBLDRV_EPBNAT_ENTRY_EXIST			-3301 //The given {protocol,extip,intip} exists
#define TBLDRV_EPBNAT_NO_FREE_ENTRY			-3302 //No free entry for Protocol-Based NAT
#define TBLDRV_EPBNAT_ENTRY_NOT_FOUND		-3303 //The specific entry is not found
#define TBLDRV_EPBNAT_PROTOCOL_NOT_SUPPORTED	-3304 //The specific protocol is not supported by protocol-based NAT


/* Error Number for Rate Limit */
#define TBLDRV_EDUPGROUPID				-3401	//Duplicate Rate Limit Group ID
#define TBLDRV_ERLENTRYISREFERENCE		-3402	//The specified rate limit entry is being referenced.
#define TBLDRV_ENOGROUPIDFOUND			-3403	//The specified group ID was not found.
#define TBLDRV_EEXCEEDTOTALBW			-3404	//The specified ratio exceeds total allowed bandwidth

/* Error Number for IPSec */
#define TBLDRV_EIPSEC_PARAM_ERROR		-4001	//IPSec parameter error
#define TBLDRV_EIPSEC_SPI_EXISTED		-4002	//IPSec SPI has existed
#define TBLDRV_EIPSEC_SPI_NOT_FOUNT		-4003	//IPSec SPI not found
#define TBLDRV_EIPSEC_NO_MORE_SPI		-4004	//no more availiable IPSec SPI 
#define TBLDRV_EIPSEC_SPIGRP_EXISTED	-4007	//IPSec SPIGRP has existed
#define TBLDRV_EIPSEC_SPIGRP_NOT_FOUNT	-4008	//IPSec SPIGRP not found
#define TBLDRV_EIPSEC_NO_MORE_SPIGRP	-4009	//no more availiable IPSec SPIGRP
#define TBLDRV_EIPSEC_EROUTE_EXISTED	-4012	//IPSec SPIGRP has existed
#define TBLDRV_EIPSEC_EROUTE_NOT_FOUNT	-4013	//IPSec SPIGRP not found
#define TBLDRV_EIPSEC_NO_MORE_EROUTE	-4014	//no more availiable IPSec SPIGRP
#define TBLDRV_EIPSEC_INVALID_WINSIZE	-4015	//Invalid window size (See RFC2046 3.4.3)

/* Error Number for URL Filter */
#define TBLDRV_EURLFILTER_URLSTRINGLEN_EXCEEND	-4501	// url filter string lenght exceeds system buffer size.
#define TBLDRV_EURLFILTER_PATHSTRINGLEN_EXCEEND	-4502	// path string of url exceeds system buffer size
#define TBLDRV_EMEMALLOCATEFAILEDFORURLFILTER	-4503	/* memory allocation failed for url filter operation */
/* RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT */
#define TBLDRV_EURLFILTER_INVALIDRULETYPE			-4504	/* Invalid url rule type */
/* RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT */
#define TBLDRV_EURLFILTER_INVALIDACTIONTYPE		-4505	/* Invalid action type of url filter rule */
/* RTL865XB_WEB_CONTENT_HDR_FILTER */
#define TBLDRV_EURLFILTER_CONTENTSTRINGLEN_EXCEEND 	-4506	/* content string pattern exceeds system buffer size */
#define TBLDRV_EURLFILTER_HTTPHDRSTRINGLEN_EXCEEND 	-4507	/* http header string pattern exceeds system buffer size */
#define TBLDRV_EURLFILTER_CONTENTHDRFILTERINUSED		-4508	/* the content/httpHdr filter is used by an napt flow, cannot be deleted */

/* Error Number for PCM drivers */
#define TBLDRV_EPCM_QUEUE_UNAV    -5001		/* pcm queue not allocated */
#define TBLDRV_EPCM_PGAE_UNAV     -5002		/* pcm page not allocated */
#define TBLDRV_EPCM_QUEUE_FULL    -5003		/* pcm queue is full of data */
#define TBLDRV_EPCM_QUEUE_EMPTY   -5004		/* pcm queue is no   of data */
#define TBLDRV_EPCM_QUEUE_SUBSIZE -5005		/* pcm queue has not enough data */
#define TBLDRV_EPCM_BUF_UNAV      -5006		/* requested buffer not allocated */
#define TBLDRV_EPCM_CHANNEL_NULL  -5007     /* current channel is not allocated */

/*Error number for naptUsrMapping*/
#define TBLDRV_EVLANRANGETOOSMALL       -5100   /* set multiple pppoe id range too small RTL8651_IDLETIMEOUT_FIXED */
//#define TBLDRV_NAPTUSRMAP_



#endif
