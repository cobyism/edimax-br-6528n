/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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

 /*Porting from udhcpd. hf_shi*/

#include <asm/system.h>
#include "etherboot.h"
#include "nic.h"
#include "rtk.h"


#if defined(RTL8196B)	
#include <asm/rtl8196.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif

#include "dhcp.h"

extern char eth0_mac[6];

static struct server_config_t server_config;
static struct dhcpOfferedAddr leases[MAX_LEASE];

/* supported options are easily added here */
struct dhcp_option options[] = {
	/* name[10]	flags					code */
	{"subnet",	OPTION_IP | OPTION_REQ,			0x01},
	{"timezone",	OPTION_S32,				0x02},
	{"router",	OPTION_IP | OPTION_LIST | OPTION_REQ,	0x03},
	{"timesvr",	OPTION_IP | OPTION_LIST,		0x04},
	{"namesvr",	OPTION_IP | OPTION_LIST,		0x05},
	{"dns",		OPTION_IP | OPTION_LIST | OPTION_REQ,	0x06},
	{"logsvr",	OPTION_IP | OPTION_LIST,		0x07},
	{"cookiesvr",	OPTION_IP | OPTION_LIST,		0x08},
	{"lprsvr",	OPTION_IP | OPTION_LIST,		0x09},
	{"hostname",	OPTION_STRING | OPTION_REQ,		0x0c},
	{"bootsize",	OPTION_U16,				0x0d},
	{"domain",	OPTION_STRING | OPTION_REQ,		0x0f},
	{"swapsvr",	OPTION_IP,				0x10},
	{"rootpath",	OPTION_STRING,				0x11},
	{"ipttl",	OPTION_U8,				0x17},
	{"mtu",		OPTION_U16,				0x1a},
	{"broadcast",	OPTION_IP | OPTION_REQ,			0x1c},
	{"ntpsrv",	OPTION_IP | OPTION_LIST,		0x2a},
	{"wins",	OPTION_IP | OPTION_LIST,		0x2c},
	{"requestip",	OPTION_IP,				0x32},
	{"lease",	OPTION_U32,				0x33},
	{"dhcptype",	OPTION_U8,				0x35},
	{"serverid",	OPTION_IP,				0x36},
	{"message",	OPTION_STRING,				0x38},
	{"tftp",	OPTION_STRING,				0x42},
	{"bootfile",	OPTION_STRING,				0x43},
	{"",		0x00,				0x00}
};

/* Lengths of the different option types */
int option_lengths[] = {
	[OPTION_IP] =		4,
	[OPTION_IP_PAIR] =	8,
	[OPTION_BOOLEAN] =	1,
	[OPTION_STRING] =	1,
	[OPTION_U8] =		1,
	[OPTION_U16] =		2,
	[OPTION_S16] =		2,
	[OPTION_U32] =		4,
	[OPTION_S32] =		4
};

Int32 time(int time)
{
	return (get_timer_jiffies()+time);
}

int lease_expired(struct dhcpOfferedAddr *lease)
{
	return (lease->expires < (unsigned long) time(0));
}	
/* get an option with bounds checking (warning, not aligned). */
unsigned char *get_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	optionptr = packet->options;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			LOG(LOG_WARNING, "bogus packet, option fields too long.");
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return NULL;
			}
			return optionptr + i + 2;
		}			
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}

int check_ip(Int32 ret)
{
	/*need to check ip is used or not by arp.*/
	return 0;
}
struct dhcpOfferedAddr *find_lease_by_yiaddr(Int32 yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);
	
	return NULL;
}

Int32 find_address(int check_expired) 
{
	Int32 addr, ret;
	struct dhcpOfferedAddr *lease = NULL;		

	addr = ntohl(server_config.start); /* addr is in host order here */
	for (;addr <= ntohl(server_config.end); addr++) {

		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;

		{
			/* lease is not taken */
			ret = htonl(addr);
			if (!(lease = find_lease_by_yiaddr(ret))  &&

			     /* and it isn't on the network */
		    	     !check_ip(ret)) {
				return ret;
				break;
			}
		}
	}
	return 0;
}

struct dhcpOfferedAddr *find_lease_by_chaddr(Int8 *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);
	
	return NULL;
}

void clear_lease(Int8 *chaddr, Int32 yiaddr)
{
	unsigned int i, j;
	
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}

struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = time(0);
	unsigned int i;

	
	for (i = 0; i < server_config.max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;
		
}

struct dhcpOfferedAddr *add_lease(Int8 *chaddr, Int32 yiaddr, unsigned long lease)
{
	struct dhcpOfferedAddr *oldest;
	
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);
		
	oldest = oldest_expired_lease();
	
	if (oldest) {
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		if(0xFFFFFFFF==server_config.lease)
			oldest->expires=0xFFFFFFFF;
		else
			oldest->expires = time(0) + lease;
	}
	
	return oldest;
}

int end_option(unsigned char *optionptr) 
{
	int i = 0;
	
	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) i++;
		else i += optionptr[i + OPT_LEN] + 2;
	}
	return i;
}

int add_option_string(unsigned char *optionptr, unsigned char *string)
{
	int end = end_option(optionptr);
	
	/* end position + string length + option code/length + end option */
	if (end + string[OPT_LEN] + 2 + 1 >= 308) {
		LOG(LOG_ERR, "Option 0x%02x did not fit into the packet!", string[OPT_CODE]);
		return 0;
	}
	DEBUG(LOG_INFO, "adding option 0x%0x", string[OPT_CODE]);
	memcpy(optionptr + end, string, string[OPT_LEN] + 2);
	optionptr[end + string[OPT_LEN] + 2] = DHCP_END;
	return string[OPT_LEN] + 2;
}

int add_simple_option(unsigned char *optionptr, unsigned char code, Int32 data)
{
	char length = 0;
	int i;
	unsigned char option[2 + 4];
	unsigned char *u8;
	Int16 *u16;
	Int32 *u32;
	Int32 aligned;
	u8 = (unsigned char *) &aligned;
	u16 = (Int16 *) &aligned;
	u32 = &aligned;

	for (i = 0; options[i].code; i++)
		if (options[i].code == code) {
			length = option_lengths[options[i].flags & TYPE_MASK];
		}
		
	if (!length) {
		DEBUG(LOG_ERR, "Could not add option 0x%02x", code);
		return 0;
	}
	
	option[OPT_CODE] = code;
	option[OPT_LEN] = length;

	switch (length) {
		case 1: *u8 =  data; break;
		case 2: *u16 = data; break;
		case 4: *u32 = data; break;
	}
	memcpy(option + 2, &aligned, length);
	return add_option_string(optionptr, option);
}

void init_header(struct dhcpMessage *packet, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	switch (type) {
	case DHCPDISCOVER:
	case DHCPREQUEST:
	case DHCPRELEASE:
	case DHCPINFORM:
		packet->op = BOOTREQUEST;
		break;
	case DHCPOFFER:
	case DHCPACK:
	case DHCPNAK:
		packet->op = BOOTREPLY;
	}
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
}

static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config.server);
}

int raw_packet(struct dhcpMessage *payload, Int32 source_ip, int source_port,
		   Int32 dest_ip, int dest_port, unsigned char *dest_arp)
{
	struct udp_dhcp_packet packet;	
	memset(&packet, 0, sizeof(packet));

	packet.ip.protocol = IP_UDP;
	packet.ip.src.s_addr = source_ip;
	packet.ip.dest.s_addr= dest_ip;
	packet.udp.src = htons(source_port);
	packet.udp.dest = htons(dest_port);
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
	packet.ip.len = packet.udp.len;
	memcpy(&(packet.data), payload, sizeof(struct dhcpMessage));
	packet.udp.chksum= ipheader_chksum(&packet, sizeof(struct udp_dhcp_packet));
	packet.ip.len = htons(sizeof(struct udp_dhcp_packet));
	packet.ip.verhdrlen= (4<<4) |(sizeof(packet.ip) >> 2);
	packet.ip.ttl = 64;
	packet.ip.chksum= ipheader_chksum(&packet.ip, sizeof(packet.ip));

	prepare_txpkt(0, FRAME_IP, dest_arp, (Int8 *)(&packet),packet.ip.len);
	return 0;
}
static int  send_packet_to_relay(struct dhcpMessage *payload)
{
	/*dhcp relay not support*/
	return 0;
}
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	unsigned char *chaddr;
	Int32 ciaddr;
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr);
}

static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}

int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	Int32 req_align, lease_time_align = server_config.lease;
	unsigned char *req, *lease_time;
	struct option_set *curr;
	
	{ 
                   Int8 empty_haddr[16]; 
    
                   memset(empty_haddr, 0, 16); 
                   if (!memcmp(oldpacket->chaddr, empty_haddr, 16)) { 
                           LOG(LOG_WARNING, "Empty Client Hardware Addresses"); 
                           return -1; 
                   } 
        } 	


	init_packet(&packet, oldpacket, DHCPOFFER);
	
	{
		/* the client is in our lease/offered table */
		if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) {
#if 0
			if (!lease_expired(lease)) 
				lease_time_align = lease->expires - time(0);
#endif			
			packet.yiaddr = lease->yiaddr;
			
		/* Or the client has a requested ip */
		} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

			   /* Don't look here (ugly hackish thing to do) */
			   memcpy(&req_align, req, 4) &&

			   /* and the ip is in the lease range */
			   ntohl(req_align) >= ntohl(server_config.start) &&
			   ntohl(req_align) <= ntohl(server_config.end) &&
			   
			   /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
			   (!(lease = find_lease_by_yiaddr(req_align)))) {
					packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */

		/* otherwise, find a free IP */ /*ADDME: is it a static lease? */
		} else {
			packet.yiaddr = find_address(0);
			
			/* try for an expired lease */
			if (!packet.yiaddr) packet.yiaddr = find_address(1);
		}
		
		if(!packet.yiaddr) {
			LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
			return -1;
		}
		
		if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
			LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
			return -1;
		}		

		if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
			memcpy(&lease_time_align, lease_time, 4);
			lease_time_align = ntohl(lease_time_align);
			if (lease_time_align > server_config.lease) 
				lease_time_align = server_config.lease;
		}

		/* Make sure we aren't just using the lease time from the previous offer */
		if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}

	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

	//add_simple_option(packet.options, DHCP_SUBNET, htonl(0xFFFFFF00));
	//add_simple_option(packet.options, DHCP_ROUTER, htonl(0xc0a801fe));

#if 0
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);
#endif

	return send_packet(&packet, 1);		// jimmylin (for AP-client)
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
	
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1);
}

int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK);

#if 0	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);
#endif

	return send_packet(&packet, 1);		// jimmylin (for AP-client)
}

int sendACK(struct dhcpMessage *oldpacket, Int32 yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	unsigned char *lease_time;
	Int32 lease_time_align = server_config.lease;

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
	
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
	//add_simple_option(packet.options, DHCP_SUBNET, htonl(0xFFFFFF00));
	//add_simple_option(packet.options, DHCP_ROUTER, htonl(0xc0a801fe));
#if 0
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);	
#endif

	if (send_packet(&packet, 1) < 0) 	// jimmylin (for AP-client)
		return -1;

	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);

	return 0;
}
void dhcps_input()
{
	struct dhcpMessage *packet;
	unsigned char *state;
	struct dhcpOfferedAddr *lease;
	unsigned char *server_id, *requested;
	Int32 server_id_align, requested_align;
	packet = (struct dhcpMessage *)&nic.packet[ETH_HLEN+ sizeof(struct iphdr)+sizeof(struct udphdr)];
	/*check if request type*/
	if ((state = get_option(packet, DHCP_MESSAGE_TYPE)) == NULL) {
		return;
	}
	lease = find_lease_by_chaddr(packet->chaddr);
	switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			
			if (sendOffer(packet) < 0) {
				LOG(LOG_WARNING,"send OFFER failed");
			}
			break;			
 		case DHCPREQUEST:
			DEBUG(LOG_INFO,"received REQUEST");

			requested = get_option(packet, DHCP_REQUESTED_IP);
			server_id = get_option(packet, DHCP_SERVER_ID);
			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);
			
			if (lease) { /*ADDME: or static lease */
				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested && 
					    requested_align == lease->yiaddr) {
						sendACK(packet, lease->yiaddr);
					}
				} else {
					if (requested) {
						/* INIT-REBOOT State */
						if (lease->yiaddr == requested_align)
							sendACK(packet, lease->yiaddr);
						else sendNAK(packet);
					} else {
						/* RENEWING or REBINDING State */
						if (lease->yiaddr == packet->ciaddr)
							sendACK(packet, lease->yiaddr);
						else {
							/* don't know what to do!!!! */
							sendNAK(packet);
						}
					}						
				}
			/* what to do if we have no record of the client */
			} else if (server_id) {
				/* SELECTING State */

			} else if (requested) {
				/* INIT-REBOOT State */
				if ((lease = find_lease_by_yiaddr(requested_align))) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
					/* make some contention for this address */
					} else sendNAK(packet);
				} else if (requested_align < server_config.start || 
					   requested_align > server_config.end) {
					sendNAK(packet);
				}else{ /* else remain silent */
					 //Brad : we do not keep silent but we send NACK to client to cause client send discover again
					 sendNAK(packet);
				}

			} else {
				 /* RENEWING or REBINDING State */
				 sendNAK(packet);	// jimmylin 050920 - Fix no response while renewing
			}
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
				lease->expires = time(0) + server_config.decline_time;
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
			if (lease){
				 lease->expires = time(0);
			}
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}

}
extern char eth0_ip_httpd[];
extern struct arptable_t  arptable_tftp[3];
void dhcps_entry()
{		
	unsigned long	ipServer = *(unsigned long *)eth0_ip_httpd;
	unsigned long	ipAddr_s, ipAddr_e;

	if(ipServer == 0)
	{
		ipServer = arptable_tftp[TFTP_SERVER].ipaddr.s_addr;
	}
	
	if( ((ipServer)&0x000000C0) == 0xC0) // 11XXXXXX, pool(128~191)
	{		
		ipAddr_s = ((ipServer)&0xFFFFFF00)|0x80;
		ipAddr_e = ((ipServer)&0xFFFFFF00)|0xBF;
	}
	else if( ((ipServer)&0x00000080) == 0x80) // 10XXXXXX, pool(2~127)
	{
		ipAddr_s = ((ipServer)&0xFFFFFF00)|0x02;
		ipAddr_e = ((ipServer)&0xFFFFFF00)|0x7F;
	}
	else // 00XXXXXX, pool(192~254)
	{				
		ipAddr_s = ((ipServer)&0xFFFFFF00)|0xC0;
		ipAddr_e = ((ipServer)&0xFFFFFF00)|0xFE;
	}
	
	memset(&server_config, 0, sizeof(struct server_config_t));
	server_config.max_leases=MAX_LEASE;
	memcpy(server_config.arp,eth0_mac,6);
	//server_config.start=htonl(0xc0a80114);/*192.168.1.20*/
	server_config.start=htonl(ipAddr_s);
	//server_config.end=htonl(0xc0a801fe);/*192.168.1.254*/
	server_config.end=htonl(ipAddr_e);
	server_config.lease=LEASE_TIME;
	server_config.decline_time=3600;
	server_config.min_lease=60;
	server_config.offer_time=60;
#ifdef GR
	server_config.server=htonl(0xc0a80101);/*gateway ip 192.168.1.1*/
#else
	server_config.server=htonl(0xc0a80106);/*gateway ip 192.168.1.6*/
#endif
}
