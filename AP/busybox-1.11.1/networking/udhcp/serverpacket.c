/* vi: set sw=4 ts=4: */
/* serverpacket.c
 *
 * Construct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "common.h"
#include "dhcpc.h"
#include "dhcpd.h"
#include "options.h"

//#define _SDMZ_

#ifdef _SDMZ_	// SJC
#include <stdlib.h>

FILE		*sjclog = NULL;
static	int		fromSuperDmz = 0;
static	int		superDmzEnabled = 0;
static	u_int32_t	wanIP, wanRouter;
static	u_int32_t	nameServer[16];
static	int		nscnt = 0;
static	u_int	leaseTime;
static	u_char	sDmzMac[6];

static void getSuperDmz(void)
{
	u_char		zero[] = {0, 0, 0, 0, 0, 0};
	FILE	*fp;
	char	buf[129], *p, *pstr;
	char	trash1[33], trash2[33], tmp[33];
	unsigned long long int	lltmp;

	do {
		memcpy(sDmzMac, zero, sizeof(sDmzMac));
		if ((fp = fopen("/proc/sys/net/ipv4/super_dmz_control", "r")) == NULL)
			break;
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;
		fclose(fp);
		fp = NULL;

		if (sscanf(buf, "%llx %s %s %s", &lltmp, trash1, trash2, tmp) < 4)
			break;

		wanIP = strtoul(tmp, NULL, 16);
		wanIP = htonl(wanIP);
		p = (char *)&lltmp;
		sDmzMac[0] = p[5], sDmzMac[1] = p[4], sDmzMac[2] = p[3];
		sDmzMac[3] = p[2], sDmzMac[4] = p[1], sDmzMac[5] = p[0];

		if ((fp = fopen("/etc/resolv.conf", "r")) == NULL)
			break;
		pstr = buf;
		memset(nameServer, 0, sizeof(nameServer));
		nscnt = 0;
		while (fgets(buf, sizeof(buf), fp)) {
			if (buf[0] == '#') continue;
			sscanf(buf, "%s %s", trash1, tmp);
			if (strcmp(trash1, "nameserver") == 0) {
				inet_aton(tmp, (struct in_addr *)&nameServer[nscnt++]);
			}
		}
		fclose(fp);
		fp = NULL;
//		if (!nscnt)
//			break;

		if ((fp = fopen("/proc/net/route", "r")) == NULL)
			break;
		while (fgets(buf, sizeof(buf), fp)) {
			u_int32_t	dst, gw;
			
			sscanf(buf, "%s %x %x", trash1, &dst, &gw);
			if (memcmp(trash1, "Iface", 5) == 0)
				continue;
			if (dst == 0L) {
				wanRouter = gw;
				break;
			}
		}
		fclose(fp);
		fp = NULL;
		if ((p = strchr(buf, '\n'))) *p = '\0';
		inet_aton(buf, (struct in_addr *)&wanRouter);

		if ((fp = fopen("/var/sDmz.conf", "r")) == NULL)
			break;
		fscanf(fp, "%d", &leaseTime);
		leaseTime = htonl(leaseTime);
		fclose(fp);
		fp = NULL;
	} while (0);
	if (fp) fclose(fp);
	superDmzEnabled = memcmp(sDmzMac, zero, sizeof(sDmzMac));
}
#endif

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG("Forwarding packet to relay");

	return udhcp_send_kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	const uint8_t *chaddr;
	uint32_t ciaddr;

	if (force_broadcast) {
		DEBUG("broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG("unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG("broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		DEBUG("unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return udhcp_send_raw_packet(payload, server_config.server, SERVER_PORT,
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	if (payload->giaddr)
		return send_packet_to_relay(payload);
	return send_packet_to_client(payload, force_broadcast);
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	udhcp_init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config.server);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy((char*)packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy((char*)packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}


/* send a DHCP OFFER to a DHCP DISCOVER */
int send_offer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	uint32_t req_align, lease_time_align = server_config.lease;
	uint8_t *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;

	uint32_t static_lease_ip;

	init_packet(&packet, oldpacket, DHCPOFFER);

	static_lease_ip = getIpByMac(server_config.static_leases, oldpacket->chaddr);

	/* ADDME: if static, short circuit */
	if (!static_lease_ip) {
		/* the client is in our lease/offered table */
		lease = find_lease_by_chaddr(oldpacket->chaddr);
		if (lease) {
			if (!lease_expired(lease))
				lease_time_align = lease->expires - time(0);
			packet.yiaddr = lease->yiaddr;
		/* Or the client has a requested ip */
		} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP))
		 /* Don't look here (ugly hackish thing to do) */
		 && memcpy(&req_align, req, 4)
		 /* and the ip is in the lease range */
		 && ntohl(req_align) >= server_config.start_ip
		 && ntohl(req_align) <= server_config.end_ip
		 && !static_lease_ip /* Check that its not a static lease */
		 /* and is not already taken/offered */
		 && (!(lease = find_lease_by_yiaddr(req_align))
			/* or its taken, but expired */ /* ADDME: or maybe in here */
			|| lease_expired(lease))
		) {
// { ReHux 
#ifdef _SDMZ_	
                        fromSuperDmz = 0;
                        getSuperDmz();
                        if ((memcmp(oldpacket->chaddr, sDmzMac, 6) == 0) && (superDmzEnabled)) {
                                fromSuperDmz = 1;
                                packet.yiaddr = wanIP;	
                        } else
				packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
#else
			packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
#endif	
// } RexHua
			/* otherwise, find a free IP */
		} else {
#ifdef _SDMZ_	// SJC
			fromSuperDmz = 0;
			getSuperDmz();
			if ((memcmp(oldpacket->chaddr, sDmzMac, 6) == 0) && (superDmzEnabled)) {
				fromSuperDmz = 1;
				packet.yiaddr = wanIP;
			} else
				packet.yiaddr = find_address(0);
#else		// _SDMZ_
			/* Is it a static lease? (No, because find_address skips static lease) */
			packet.yiaddr = find_address(0);
#endif		// _SDMZ_
			/* try for an expired lease */
			if (!packet.yiaddr)
				packet.yiaddr = find_address(1);
//if (sjclog == NULL) sjclog = fopen("/tmp/sjclog.txt", "w");
//fprintf(sjclog, "%s:%d chaddr=%02x%02x%02x%02x%02x%02x fromSuperDmz=%d superDmzEnabled=%d\n", __func__, __LINE__,
//oldpacket->chaddr[0], oldpacket->chaddr[1],oldpacket->chaddr[2],oldpacket->chaddr[3],oldpacket->chaddr[4],oldpacket->chaddr[5],
//fromSuperDmz, superDmzEnabled);
//fflush(sjclog);

		}

		if (!packet.yiaddr) {
			bb_error_msg("no IP addresses to give - OFFER abandoned");
			return -1;
		}
		if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
			bb_error_msg("lease pool is full - OFFER abandoned");
			return -1;
		}
		lease_time = get_option(oldpacket, DHCP_LEASE_TIME);
		if (lease_time) {
			memcpy(&lease_time_align, lease_time, 4);
			lease_time_align = ntohl(lease_time_align);
			if (lease_time_align > server_config.lease)
				lease_time_align = server_config.lease;
		}

		/* Make sure we aren't just using the lease time from the previous offer */
		if (lease_time_align < server_config.min_lease)
			lease_time_align = server_config.lease;
		/* ADDME: end of short circuit */
	} else {
		/* It is a static lease... use it */
		packet.yiaddr = static_lease_ip;
	}

#ifdef _EZVIEW_
	if(	(packet.chaddr[0]==0 && packet.chaddr[1]==25 && packet.chaddr[2]==197) ||  //PS3: 00 19 C5 ?? ?? ??
//		(packet.chaddr[0]==0 && packet.chaddr[1]==26 && packet.chaddr[2]==160) ||  //xx
		(packet.chaddr[0]==0 && packet.chaddr[1]==27 && packet.chaddr[2]==234) )   //WII: 00 1B EA ?? ?? ??
	{
//		char asd[200];
//		addr.s_addr = packet.yiaddr;
//		sprintf(asd,"if [ -f /tmp/ezdhcptmp ]; then echo `cat /tmp/ezdhcptmp | grep -v ^%2X%2X%2X%2X%2X%2X` > /tmp/ezdhcptmp1; else > /tmp/ezdhcptmp1; fi; echo \"%2X%2X%2X%2X%2X%2X %s\" >> /tmp/ezdhcptmp1; cp -f /tmp/ezdhcptmp1 /tmp/ezdhcptmp",
//				packet.chaddr[0],packet.chaddr[1],packet.chaddr[2],packet.chaddr[3],packet.chaddr[4],packet.chaddr[5],
//				packet.chaddr[0],packet.chaddr[1],packet.chaddr[2],packet.chaddr[3],packet.chaddr[4],packet.chaddr[5],
//				inet_ntoa(addr)
//				);
//		system(asd);
//		system("cat /tmp/ezdhcptmp |  echo Received RELEASE >> /tmp/a");
		add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(45));
	}
	else
#endif
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

	curr = server_config.options;
	while (curr) {
#ifdef _SDMZ_		// SJC
		if (superDmzEnabled && fromSuperDmz) {
			u_char	tmp[66]; // 16*4+2
			int		x;

			tmp[OPT_CODE] = curr->data[OPT_CODE];
			tmp[OPT_LEN] = curr->data[OPT_LEN];
			switch (curr->data[OPT_CODE]) {
				case DHCP_ROUTER:
					memcpy(&tmp[OPT_DATA], &wanRouter, curr->data[OPT_LEN]);
					add_option_string(packet.options, tmp);
					break;
				case DHCP_DNS_SERVER:
					for (x = 0; x < nscnt; x++) 
						memcpy(&tmp[OPT_DATA+x*4], &nameServer[x], 4);
					
					tmp[OPT_LEN] = curr->data[OPT_LEN] + (nscnt-1)*4;
					add_option_string(packet.options, tmp);
					break;
				case DHCP_LEASE_TIME:
					if (leaseTime > 0) {
						memcpy(&tmp[OPT_DATA], &leaseTime, curr->data[OPT_LEN]);
						add_option_string(packet.options, tmp);
					}
					break;
				default:
					add_option_string(packet.options, curr->data);
			}
		} else {
			if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
				add_option_string(packet.options, curr->data);
		}
#else
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
#endif	// _SDMZ_
		curr = curr->next;
	}

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;
	bb_info_msg("Sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0);
}


int send_NAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);

	DEBUG("Sending NAK");
	return send_packet(&packet, 1);
}


int send_ACK(struct dhcpMessage *oldpacket, uint32_t yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	uint8_t *lease_time;
	uint32_t lease_time_align = server_config.lease;
	struct in_addr addr;

	init_packet(&packet, oldpacket, DHCPACK);
#ifdef _SDMZ_	// SJC
	fromSuperDmz = 0;
	getSuperDmz();

	if ((memcmp(oldpacket->chaddr, sDmzMac, 6) == 0) && (superDmzEnabled)) {
		fromSuperDmz = 1;
		yiaddr = wanIP;
	}
	
	if ((memcmp(oldpacket->chaddr, sDmzMac, 6) == 0) && !(superDmzEnabled) && (yiaddr == wanIP)) {
		send_NAK(oldpacket);
		return 0;
	}
//if (sjclog == NULL) sjclog = fopen("/tmp/sjclog.txt", "w");
//fprintf(sjclog, "%s:%d chaddr=%02x%02x%02x%02x%02x%02x fromSuperDmz=%d superDmzEnabled=%d\n", __func__, __LINE__,
//oldpacket->chaddr[0], oldpacket->chaddr[1],oldpacket->chaddr[2],oldpacket->chaddr[3],oldpacket->chaddr[4],oldpacket->chaddr[5],
//fromSuperDmz, superDmzEnabled);
//fflush(sjclog);

#endif
	packet.yiaddr = yiaddr;

	lease_time = get_option(oldpacket, DHCP_LEASE_TIME);
	if (lease_time) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease)
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease)
			lease_time_align = server_config.lease;
	}
#ifdef _EZVIEW_
	if(	(packet.chaddr[0]==0 && packet.chaddr[1]==25 && packet.chaddr[2]==197) ||  //PS3: 00 19 C5 ?? ?? ??
//		(packet.chaddr[0]==0 && packet.chaddr[1]==26 && packet.chaddr[2]==160) ||  //xx
		(packet.chaddr[0]==0 && packet.chaddr[1]==27 && packet.chaddr[2]==234) )   //WII: 00 1B EA ?? ?? ??
	{
//		char asd[200];
//		addr.s_addr = packet.yiaddr;
//		sprintf(asd,"if [ -f /tmp/ezdhcptmp ]; then echo `cat /tmp/ezdhcptmp | grep -v ^%2X%2X%2X%2X%2X%2X` > /tmp/ezdhcptmp1; else > /tmp/ezdhcptmp1; fi; echo \"%2X%2X%2X%2X%2X%2X %s\" >> /tmp/ezdhcptmp1; cp -f /tmp/ezdhcptmp1 /tmp/ezdhcptmp",
//				packet.chaddr[0],packet.chaddr[1],packet.chaddr[2],packet.chaddr[3],packet.chaddr[4],packet.chaddr[5],
//				packet.chaddr[0],packet.chaddr[1],packet.chaddr[2],packet.chaddr[3],packet.chaddr[4],packet.chaddr[5],
//				inet_ntoa(addr)
//				);
//		system(asd);
//		system("cat /tmp/ezdhcptmp |  echo Received RELEASE >> /tmp/a");
		add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(45));
	}
	else
#endif
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

	curr = server_config.options;
	while (curr) {
#ifdef _SDMZ_		// SJC
		if (superDmzEnabled && fromSuperDmz) {
			int		x;
			u_char	tmp[66]; // 16*4+2

			tmp[OPT_CODE] = curr->data[OPT_CODE];
			tmp[OPT_LEN] = curr->data[OPT_LEN];
			switch (curr->data[OPT_CODE]) {
				case DHCP_ROUTER:
					memcpy(&tmp[OPT_DATA], &wanRouter, curr->data[OPT_LEN]);
					add_option_string(packet.options, tmp);
					break;
				case DHCP_DNS_SERVER:
					for (x = 0; x < nscnt; x++) 
						memcpy(&tmp[OPT_DATA+x*4], &nameServer[x], 4);
					
					tmp[OPT_LEN] = curr->data[OPT_LEN] + (nscnt-1)*4;
					add_option_string(packet.options, tmp);
					break;
				case DHCP_LEASE_TIME:
					if (leaseTime > 0) {
						memcpy(&tmp[OPT_DATA], &leaseTime, curr->data[OPT_LEN]);
						add_option_string(packet.options, tmp);
					}
					break;
				default:
					add_option_string(packet.options, curr->data);
			}
		} else {
			if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
				add_option_string(packet.options, curr->data);
		}
#else
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
#endif	// _SDMZ
		curr = curr->next;
	}

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;
	bb_info_msg("Sending ACK to %s", inet_ntoa(addr));

	if (send_packet(&packet, 0) < 0)
		return -1;

	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);
	if (ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY) {
		/* rewrite the file with leases at every new acceptance */
		write_leases();
	}

	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK);

	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);

	return send_packet(&packet, 0);
}
