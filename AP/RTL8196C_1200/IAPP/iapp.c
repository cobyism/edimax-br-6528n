/*
 * User space daemon for IEEE 802.11f Inter-Access Point Protocol (IAPP)
 * Copyright (c) 2003 by Realtek Semiconductor
 * Written by Jimmy Lin <jimmylin@realtek.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* TODO:
 * - add support for MOVE-notify and MOVE-response (this requires support for
 *   finding out IP address for previous AP using RADIUS)
 * - add support for Send- and ACK-Security-Block to speedup IEEE 802.1X during
 *   reassociation to another AP
 * - implement counters etc. for IAPP MIB
 * - verify endianness of fields in IAPP messages; are they big-endian as
 *   used here?
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef USE_KERNEL_HEADERS
#include <linux/if_packet.h>
#else /* USE_KERNEL_HEADERS */
#include <netpacket/packet.h>
#endif /* USE_KERNEL_HEADERS */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <linux/wireless.h>

#include "common.h"
#include "iapp.h"
#include "misc.h"


/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP			0x0001          /* route usable                 */
#define RTF_GATEWAY		0x0002          /* destination is a gateway     */

// local variable
static char *pidfile = "/var/run/iapp.pid";
static char *routefile = "/proc/net/route";
static struct iapp_context iapp_cnt;
static struct iapp_context *hapd = &iapp_cnt;
extern char *fwVersion;

static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

static int iapp_init_fifo()
{
/* Here is an assumption that the fifo is created already by iwcontrol
 */
	int flags;
	struct stat status;

	if(stat(DAEMON_FIFO, &status) == 0)
		unlink(DAEMON_FIFO);
	if((mkfifo(DAEMON_FIFO, FILE_MODE) < 0)){
		printf("mkfifo %s fifo error: %s!\n", DAEMON_FIFO, strerror(errno));
		return -1;
	}

	hapd->readfifo = open(DAEMON_FIFO, O_RDONLY, 0);
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "hapd->readfifo = %d\n", hapd->readfifo);

	if ((flags = fcntl(hapd->readfifo, F_GETFL, 0)) < 0) {
		printf("F_GETFL: error\n");
		return -1;
	}
	else {
		flags |= O_NONBLOCK;
		if ((flags = fcntl(hapd->readfifo, F_SETFL, flags)) < 0) {
			printf("F_SETFL: error\n");
			return -1;
		}
	}

	return 0;
}


static void check_multicast_route()
{
	char buff[1024], iface[16];
	char net_addr[128], gate_addr[128], mask_addr[128];
	int num, iflags, refcnt, use, metric, mss, window, irtt;
	FILE *fp = fopen(routefile, "r");
	char *fmt;
	int found = 0;

	if (!fp) {
		printf("Open %s file error.\n", routefile);
		return;
	}

	fmt = "%16s %128s %128s %X %d %d %d %128s %d %d %d";

	while (fgets(buff, 1023, fp)) {
		num = sscanf(buff, fmt, iface, net_addr, gate_addr,
			&iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
		if (num < 10 || !(iflags & RTF_UP) || strcmp(iface, hapd->iapp_iface))
			continue;
		if (!strcmp(net_addr, "E00001B2")) {
			found = 1;
			break;
		}
	}

	fclose(fp);
	if (!found) {
		sprintf(buff, "route add -net 224.0.1.178 netmask 255.255.255.255 dev %s", hapd->iapp_iface);
		system(buff);
	}
}


static void iapp_send_add_d3(u8 *sta, u16 id)
{
	char buf[128];
	struct iapp_hdr *hdr;
	struct iapp_add_notify *add;
	struct sockaddr_in addr;
	int n;

	/* Send IAPP-ADD Packet to remove possible association from other APs
	 */

	hdr = (struct iapp_hdr *) buf;
	hdr->version = IAPP_VERSION;
	hdr->command = IAPP_CMD_ADD_notify;
	hdr->identifier = host_to_be16(id);
	hdr->length = host_to_be16(sizeof(*hdr) + sizeof(*add));

	add = (struct iapp_add_notify *) (hdr + 1);
	add->addr_len = ETH_ALEN;
	add->reserved = 0;
	memcpy(add->mac_addr, sta, ETH_ALEN);
	add->seq_num = 0;
	
	/* Send to local subnet address (UDP port IAPP_PORT) */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = hapd->iapp_broadcast.s_addr;
	addr.sin_port = htons(IAPP_PORT_D3);
	if ((n = sendto(hapd->iapp_udp_sock, buf, (char *) (add + 1) - buf, 0,
		   (struct sockaddr *) &addr, sizeof(addr))) < 0)
		perror("sendto[IAPP-ADD]");

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
	       "Broadcast %d byte IAPP frame of sta %02x%02x%02x%02x%02x%02x\n",
	       n, sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
}


static void iapp_send_add_d5(u8 *sta, u16 id)
{
	char buf[128];
	struct iapp_hdr *hdr;
	struct iapp_add_notify *add;
	struct sockaddr_in addr;
	int n;

	/* Send IAPP-ADD Packet to remove possible association from other APs
	 */

	hdr = (struct iapp_hdr *) buf;
	hdr->version = IAPP_VERSION;
	hdr->command = IAPP_CMD_ADD_notify;
	hdr->identifier = host_to_be16(id);
	hdr->length = host_to_be16(sizeof(*hdr) + sizeof(*add));

	add = (struct iapp_add_notify *) (hdr + 1);
	add->addr_len = ETH_ALEN;
	add->reserved = 0;
	memcpy(add->mac_addr, sta, ETH_ALEN);
	add->seq_num = 0;
	
	/* check routing entry of multicast and set if no */
	check_multicast_route();
	
	/* Send to local subnet address (UDP port IAPP_PORT) */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (!inet_aton(IAPP_MULTICAST_ADDR, &addr.sin_addr)) {
		perror("inet_aton[IAPP-ADD]");
		return;
	}
	addr.sin_port = htons(IAPP_PORT_D5);
	if ((n = sendto(hapd->iapp_mltcst_sock, buf, (char *) (add + 1) - buf, 0,
		   (struct sockaddr *) &addr, sizeof(addr))) < 0)
		perror("sendto[IAPP-ADD]");

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
	       "Multicast %d byte IAPP frame of sta %02x%02x%02x%02x%02x%02x\n",
	       n, sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
}


static void iapp_send_layer2_update(u8 *sta)
{
	struct iapp_layer2_update msg;
	int n;

	/* Send Level 2 Update Frame to update forwarding tables in layer 2
	 * bridge devices */

	/* 802.2 Type 1 Logical Link Control (LLC) Exchange Identifier (XID)
	 * Update response frame; IEEE Std 802.2-1998, 5.4.1.2.1 */

	memset(msg.da, 0xff, ETH_ALEN);
	memcpy(msg.sa, sta, ETH_ALEN);
	msg.len = host_to_be16(8);
	msg.dsap = 0;
	msg.ssap = 0;
	msg.control = 0xaf; /* XID response lsb.1111F101.
			     * F=0 (no poll command; unsolicited frame) */
	msg.xid_info[0] = 0x81; /* XID format identifier */
	msg.xid_info[1] = 1; /* LLC types/classes: Type 1 LLC */
	msg.xid_info[2] = 1 << 1; /* XID sender's receive window size (RW)
				   * FIX: what is correct RW with 802.11? */

	if ((n = send(hapd->iapp_packet_sock, &msg, sizeof(msg), 0)) < 0)
		perror("send[L2 Update]");

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS,
	       "Send %d byte L2 Update frame of sta %02x%02x%02x%02x%02x%02x\n",
	       n, sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
}


static void iapp_new_station(u8 *sta)
{
	if (hapd->spcApply & APPLY_D3)
		iapp_send_add_d3(sta, hapd->iapp_identifier);
	if (hapd->spcApply & APPLY_D5)
		iapp_send_add_d5(sta, hapd->iapp_identifier);
	hapd->iapp_identifier++;
	iapp_send_layer2_update(sta);
}


static void iapp_process_add_notify(struct sockaddr_in *from,
				    struct iapp_hdr *hdr, int len)
{
	struct iapp_add_notify *add = (struct iapp_add_notify *) (hdr + 1);
	unsigned char para[32];
	struct iwreq wrq;
	int i = 0;

	if (len != sizeof(*add)) {
		printf("Invalid IAPP-ADD packet length %d (expected %d)\n",
		       len, sizeof(*add));
		return;
	}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
	       "Received IAPP-ADD for STA " "%02x%02x%02x%02x%02x%02x" " (seq# %d) from %s:%d\n",
	       add->mac_addr[0], add->mac_addr[1], add->mac_addr[2], add->mac_addr[3], add->mac_addr[4],
	       add->mac_addr[5], be_to_host16(add->seq_num),
	       inet_ntoa(from->sin_addr), ntohs(from->sin_port));

	/* TODO: could use seq_num to try to determine whether last association
	 * to this AP is newer than the one advertised in IAPP-ADD. Although,
	 * this is not really a reliable verification. */

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
	       "Removing STA " "%02x%02x%02x%02x%02x%02x" " due to IAPP-ADD notification from "
	       "%s\n", add->mac_addr[0], add->mac_addr[1], add->mac_addr[2], add->mac_addr[3], add->mac_addr[4],
	       add->mac_addr[5], inet_ntoa(from->sin_addr));

	if (hapd->driver_ver == DRIVER_8180)
	{
		memset(para, 0, 32);
		sprintf(para, "delsta=%02x%02x%02x%02x%02x%02x",
			add->mac_addr[0],
			add->mac_addr[1],
			add->mac_addr[2],
			add->mac_addr[3],
			add->mac_addr[4],
			add->mac_addr[5]);
		wrq.u.data.pointer = para;
		wrq.u.data.length = strlen(para);
		strncpy(wrq.ifr_name, hapd->wlan_iface[i], IFNAMSIZ);
		ioctl(hapd->wlan_sock[i], 0x89f6/* RTL8180_IOCTL_SET_WLAN_PARA */, &wrq);
	}
	else
	{
		while(strlen(hapd->wlan_iface[i]) != 0)
		{
			memset(para, 0, 32);
			sprintf(para, "%02x%02x%02x%02x%02x%02x", add->mac_addr[0], add->mac_addr[1],
			add->mac_addr[2], add->mac_addr[3], add->mac_addr[4], add->mac_addr[5]);
			wrq.u.data.pointer = para;
			wrq.u.data.length = strlen(para);
			strncpy(wrq.ifr_name, hapd->wlan_iface[i], IFNAMSIZ);
			ioctl(hapd->wlan_sock[i], 0x89f7/* RTL8185_IOCTL_DEL_STA */, &wrq);
			i++;
		}
	}
}


static void iapp_receive_udp(int sockGot)
{
	int len, hlen;
	unsigned char buf[128];
	struct sockaddr_in from;
	socklen_t fromlen;
	struct iapp_hdr *hdr;

	/* Handle incoming IAPP frames (over UDP/IP) */

	fromlen = sizeof(from);
	len = recvfrom(sockGot, buf, sizeof(buf), 0,
		       (struct sockaddr *) &from, &fromlen);
	if (len < 0)
		perror("recvfrom");

	if (from.sin_addr.s_addr == hapd->iapp_own.s_addr)
		return; /* ignore own IAPP messages */

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "Received %d byte IAPP frame from %s\n",
		      len, inet_ntoa(from.sin_addr));

	if (len < sizeof(*hdr)) {
		printf("Too short IAPP frame (len=%d)\n", len);
		return;
	}

	hdr = (struct iapp_hdr *) buf;
	hlen = be_to_host16(hdr->length);
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "IAPP: version=%d command=%d id=%d len=%d\n",
		      hdr->version, hdr->command,
		      be_to_host16(hdr->identifier), hlen);
	if (hlen > len) {
		printf("Underflow IAPP frame (hlen=%d len=%d)\n", hlen, len);
		return;
	}
	if (hlen < len) {
		printf("Ignoring %d extra bytes from IAPP frame\n",
		       len - hlen);
		len = hlen;
	}

	if (hdr->command == IAPP_CMD_ADD_notify)
		iapp_process_add_notify(&from, hdr, hlen - sizeof(*hdr));
	else
		printf("Unknown IAPP command %d\n", hdr->command);
}


static void do_daemon()
{
	fd_set netFD;
	int selret, nRead, max_sock;

	// while loop to listen socket and check event
	while (1)
	{
		max_sock = 0;
		FD_ZERO(&netFD);
		if (hapd->spcApply & APPLY_D3) {
			FD_SET(hapd->iapp_udp_sock, &netFD);
			max_sock = (max_sock > hapd->iapp_udp_sock)? max_sock : hapd->iapp_udp_sock;
		}

		if (hapd->spcApply & APPLY_D5) {
			FD_SET(hapd->iapp_mltcst_sock, &netFD);
			max_sock = (max_sock > hapd->iapp_mltcst_sock)? max_sock : hapd->iapp_mltcst_sock;
		}

		FD_SET(hapd->readfifo, &netFD);
		max_sock = (max_sock > hapd->readfifo)? max_sock : hapd->readfifo;

		selret = select(max_sock+1, &netFD, NULL, NULL, NULL);
		if (selret > 0)
		{
			if (FD_ISSET(hapd->iapp_udp_sock, &netFD))
				iapp_receive_udp(hapd->iapp_udp_sock);

			if (FD_ISSET(hapd->iapp_mltcst_sock, &netFD))
				iapp_receive_udp(hapd->iapp_mltcst_sock);

			if (FD_ISSET(hapd->readfifo, &netFD))
			{
				nRead = read(hapd->readfifo, hapd->RecvBuf, MAX_MSG_SIZE);
				if (nRead > 0)
				{
					u8 msg_type = *(hapd->RecvBuf + FIFO_HEADER_LEN);
					HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Fifo msg type: %d\n", msg_type);

					if (msg_type == DOT11_EVENT_ASSOCIATION_IND) {
						DOT11_ASSOCIATION_IND *msg = (DOT11_ASSOCIATION_IND *)(hapd->RecvBuf + FIFO_HEADER_LEN);
						iapp_new_station(msg->MACAddr);
					}
					else if (msg_type == DOT11_EVENT_REASSOCIATION_IND) {
						DOT11_REASSOCIATION_IND *msg = (DOT11_REASSOCIATION_IND *)(hapd->RecvBuf + FIFO_HEADER_LEN);
						iapp_new_station(msg->MACAddr);
					}
					else {
						// messages that we don't handle and drop
					}
				}
			}
		}
	}
}


int main(int argc, char *argv[])
{
	struct ifreq ifr;
	struct sockaddr_ll addr;
	int ifindex, one;
	struct sockaddr_in *paddr, uaddr;
	int opt, ttl = 64, loop = 0;
	struct ip_mreq multiaddr;
	struct in_addr adtp;
	struct iwreq wrq;
	char tmpbuf[32];
	int i;

	// destroy old process and create a PID file
	{
		int pid_fd;
		FILE *fp;
		char line[20];
		pid_t pid;

		if ((fp = fopen(pidfile, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if (sscanf(line, "%d", &pid)) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
		}
		pid_fd = pidfile_acquire(pidfile);
		if (pid_fd < 0)
			return 0;

		if (daemon(0,1) == -1) {		
			printf("fork iapp error!\n");
			exit(1);
		}
		pidfile_write_release(pid_fd);
	}

	// parse arguments
	memset(hapd->iapp_iface, 0, sizeof(hapd->iapp_iface));
	memset(hapd->wlan_iface, 0, sizeof(hapd->wlan_iface));
	hapd->spcApply = APPLY_D3 | APPLY_D5;
	hapd->debug = HOSTAPD_DEBUG_NO;

	while ((opt = getopt(argc, argv, "-d:n:")) > 0) {
		switch (opt) {
			case 1:
				if (!strncmp(optarg, "br", 2) ||
					!strncmp(optarg, "eth", 3) ||
					!strncmp(optarg, "wlan", 4)) {
					if (strlen(hapd->iapp_iface) == 0)
						strcpy(hapd->iapp_iface, optarg);
					else {
						i = 0;
						while(strlen(hapd->wlan_iface[i]) != 0)
							i++;
						strcpy(hapd->wlan_iface[i], optarg);
					}
				}
				else {
					printf("Wrong interface\n");
					return -1;
				}
				break;
			case 'd':
				hapd->debug = atoi(optarg);
				if (hapd->debug > HOSTAPD_DEBUG_MSGDUMPS)
					hapd->debug = HOSTAPD_DEBUG_MSGDUMPS;
				break;
			case 'n':
				if (!strcmp(optarg, "d3"))
					hapd->spcApply &= ~APPLY_D3;
				else if (!strcmp(optarg, "d5"))
					hapd->spcApply &= ~APPLY_D5;
				else {
					printf("Usage: iapp interface [-d debug_level] [-n d3|d5] [wlan0 ...]\n");
					return -1;
				}
				break;
			default:
				printf("Usage: iapp interface [-d debug_level] [-n d3|d5] [wlan0 ...]\n");
				return -1;
		}
	}
	if (strlen(hapd->iapp_iface) == 0) {
		printf("Usage: iapp interface [-d debug_level] [-n d3|d5] [wlan0 ...]\n");
		return -1;
	}

	// at least one wlan interface
	if (strlen(hapd->wlan_iface[0]) == 0)
		strcpy(hapd->wlan_iface[0], "wlan0");

	// fifo initialization
	if (iapp_init_fifo() < 0) {
		printf("Init fifo fail.\n");
		return -1;
	}

	/* TODO:
	 * open socket for sending and receiving IAPP frames over TCP
	 */

	/*
	 * create broadcast socket
	 */
	hapd->iapp_udp_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hapd->iapp_udp_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, hapd->iapp_iface, sizeof(ifr.ifr_name));
	while (ioctl(hapd->iapp_udp_sock, SIOCGIFINDEX, &ifr) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		       "iapp: ioctl(SIOCGIFINDEX) failed!\n");
		sleep(1);
	}
	ifindex = ifr.ifr_ifindex;

	while (ioctl(hapd->iapp_udp_sock, SIOCGIFADDR, &ifr) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		       "iapp: ioctl(SIOCGIFADDR) failed!\n");
		sleep(1);
	}
	paddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (paddr->sin_family != AF_INET) {
		printf("Invalid address family %i (SIOCGIFADDR)\n",
		       paddr->sin_family);
		return -1;
	}
	hapd->iapp_own.s_addr = paddr->sin_addr.s_addr;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Got ip addr %s\n", inet_ntoa(hapd->iapp_own));

	if (ioctl(hapd->iapp_udp_sock, SIOCGIFBRDADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFBRDADDR)");
		return -1;
	}
	paddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (paddr->sin_family != AF_INET) {
		printf("Invalid address family %i (SIOCGIFBRDADDR)\n",
		       paddr->sin_family);
		return -1;
	}
	hapd->iapp_broadcast.s_addr = paddr->sin_addr.s_addr;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Got bdcst addr %s\n", inet_ntoa(hapd->iapp_broadcast));

	one = 1;
	if (setsockopt(hapd->iapp_udp_sock, SOL_SOCKET, SO_BROADCAST,
		       (char *) &one, sizeof(one)) < 0) {
		perror("setsockopt[SOL_SOCKET,SO_BROADCAST]");
		return -1;
	}

	memset(&uaddr, 0, sizeof(uaddr));
	uaddr.sin_family = AF_INET;
	uaddr.sin_port = htons(IAPP_PORT_D3);
	if (bind(hapd->iapp_udp_sock, (struct sockaddr *) &uaddr,
		 sizeof(uaddr)) < 0) {
		perror("bind[UDP]");
		return -1;
	}

	/*
	 * create multicast socket
	 */
	hapd->iapp_mltcst_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hapd->iapp_mltcst_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}

	// Join a multicast group IAPP_MULTICAST_ADDR on this interface
	inet_aton(IAPP_MULTICAST_ADDR, &adtp);
	multiaddr.imr_multiaddr.s_addr = adtp.s_addr;
	multiaddr.imr_interface.s_addr = htonl(INADDR_ANY);
	while(1) {
		char cmd[128];
		sprintf(cmd, "route add -net 224.0.1.178 netmask 255.255.255.255 dev %s", hapd->iapp_iface);
		system(cmd);
		if (setsockopt(hapd->iapp_mltcst_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				&multiaddr, sizeof(multiaddr)) >= 0)
			break;
	}

	// don't loopback
	if (setsockopt(hapd->iapp_mltcst_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
		       &loop, sizeof(loop)) < 0) {
		perror("setsockopt[IPPROTO_IP, IP_MULTICAST_LOOP]");
		return -1;
	}

	// set TTL to 64
	if (setsockopt(hapd->iapp_mltcst_sock, IPPROTO_IP, IP_MULTICAST_TTL,
		       &ttl, sizeof(ttl)) < 0) {
		perror("setsockopt[IPPROTO_IP, IP_MULTICAST_TTL]");
		return -1;
	}

	memset(&uaddr, 0, sizeof(uaddr));
	uaddr.sin_family = AF_INET;
	uaddr.sin_port = htons(IAPP_PORT_D5);
	if (bind(hapd->iapp_mltcst_sock, (struct sockaddr *) &uaddr,
		 sizeof(uaddr)) < 0) {
		perror("bind[UDP]");
		return -1;
	}

	/*
	 * create layer 2 socket
	 */
	hapd->iapp_packet_sock = socket(PF_PACKET, SOCK_RAW, 0);
	if (hapd->iapp_packet_sock < 0) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifindex;
	if (bind(hapd->iapp_packet_sock, (struct sockaddr *) &addr,
		 sizeof(addr)) < 0) {
		perror("bind[PACKET]");
		return -1;
	}

	/*
	 * create wlan sockets
	 */
	i = 0;
	while(strlen(hapd->wlan_iface[i]) != 0)
	{
		hapd->wlan_sock[i] = socket(PF_INET, SOCK_DGRAM, 0);
		if (hapd->wlan_sock[i] < 0) {
			perror("socket[PF_INET,SOCK_DGRAM]");
			return -1;
		}
		strncpy(wrq.ifr_name, hapd->wlan_iface[i], IFNAMSIZ);
		if (ioctl(hapd->wlan_sock[i], SIOCGIWNAME, &wrq) < 0) {
			sprintf(tmpbuf, "ioctl[%s,SIOCGIWNAME]", hapd->wlan_iface[i]);
			perror(tmpbuf);
			return -1;
		}
		i++;
	}

	// determine which driver used
	{
		FILE *fp;
		if((fp = fopen("/proc/rtl8180/status", "r")) != NULL) {
			fclose(fp);
			hapd->driver_ver = DRIVER_8180;
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "The WLAN driver is RTK_8180\n");
		}
		else {
			hapd->driver_ver = DRIVER_8185;
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "The WLAN driver is RTK_8185\n");
		}
	}

	// Show message on console
	printf("IEEE 802.11f (IAPP) using interface %s (%s)\n",
		   hapd->iapp_iface, fwVersion);

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, "using wlan interface:");
	i = 0;
	while(strlen(hapd->wlan_iface[i]) != 0)
	{
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, " %s", hapd->wlan_iface[i]);
		i++;
	}
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, "\n");

	do_daemon();
	return 0;
}

