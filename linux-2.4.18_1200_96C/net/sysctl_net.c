/* -*- linux-c -*-
 * sysctl_net.c: sysctl interface to net subsystem.
 *
 * Begun April 1, 1996, Mike Shaver.
 * Added /proc/sys/net directories for each protocol family. [MS]
 *
 * $Log: sysctl_net.c,v $
 * Revision 1.1.1.1  2007/08/06 10:05:00  root
 * Initial import source to CVS
 *
 * Revision 1.1.1.1  2004/07/28 06:27:37  ysc
 *
 * Initial version
 *
 * Revision 1.1.1.1  2004/07/14 13:34:44  ysy
 *
 *
 * Revision 1.1.1.1  2004/07/13 21:58:48  ysy
 *
 *
 * Revision 1.1.1.1  2004/07/13 15:58:28  ysy
 * no message
 *
 * Revision 1.2  2003/05/12 04:02:57  john
 * JACKSON_NET_WORK tuning!
 *
 * Revision 1.1.1.1  2003/03/12 06:25:29  john
 * rtl8181_prj initially build_up
 *
 * Revision 1.1.1.1  2003/03/12 06:19:42  john
 * rtl8181_prj initially build_up
 *
 * Revision 1.1.1.1  2003/03/11 06:59:11  john
 * RTL8181 Initical Check In
 *
 * Revision 1.1.1.1  2002/03/04 11:13:35  carstenl
 * Linux kernel version 2.4.18 from cvs@oss.sgi.com, 4 March, 2002
 *
 * Revision 1.2  1996/05/08  20:24:40  shaver
 * Added bits for NET_BRIDGE and the NET_IPV4_ARP stuff and
 * NET_IPV4_IP_FORWARD.
 *
 *
 */

#include <linux/config.h>
#include <linux/mm.h>
#include <linux/sysctl.h>

#ifdef CONFIG_INET
extern ctl_table ipv4_table[];
#endif

extern ctl_table core_table[];

#ifdef CONFIG_NET
extern ctl_table ether_table[], e802_table[];
#endif

#ifdef CONFIG_IPV6
extern ctl_table ipv6_table[];
#endif

#ifdef CONFIG_TR
extern ctl_table tr_table[];
#endif

ctl_table net_table[] = {
	{NET_CORE,   "core",      NULL, 0, 0555, core_table},      
#ifdef CONFIG_NET
	{NET_802,    "802",       NULL, 0, 0555, e802_table},
	{NET_ETHER,  "ethernet",  NULL, 0, 0555, ether_table},
#endif
#ifdef CONFIG_INET
	{NET_IPV4,   "ipv4",      NULL, 0, 0555, ipv4_table},
#endif
#ifdef CONFIG_IPV6
	{NET_IPV6, "ipv6", NULL, 0, 0555, ipv6_table},
#endif
#ifdef CONFIG_TR
	{NET_TR, "token-ring", NULL, 0, 0555, tr_table},
#endif
	{0}
};
