/*
 * udp.c - handle upd connections
 *
 * Copyright (C) 1999 Brad M. Garcia <garsh@home.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "common.h"
#include "relay.h"
#include "cache.h"
#include "query.h"
#include "master.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/*
 * dnssend()						22OCT99wzk
 *
 * Abstract: A small wrapper for send()/sendto().  If an error occurs a
 *           message is written to syslog.
 *
 * Returns:  The return code from sendto().
 */
static int dnssend(int k, void *msg, int len)
{
    int	rc;

    rc = sendto(dns_srv[k].sock, msg, len, 0,
		(const struct sockaddr *) &dns_srv[k].addr,
		sizeof(struct sockaddr_in));
    if (rc != len) {
	log_msg(LOG_ERR, "sendto error: %s: %m",
		inet_ntoa(dns_srv[k].addr.sin_addr));
	return (rc);
    }

    return (rc);
}


/*
 * handle_udprequest()
 *
 * This function handles udp DNS requests by either replying to them (if we
 * know the correct reply via master, caching, etc.), or forwarding them to
 * an appropriate DNS server.
 */
void handle_udprequest()
{
    unsigned           addr_len;
    int                len;
    const int          maxsize = 512; /* According to RFC 1035 */
    char               msg[maxsize+4];/* Do we really want this on the stack?*/
    struct sockaddr_in from_addr;
    int                fwd, srvr;
    int	               i, thisdns, processed;

    /* Read in the message */
    addr_len = sizeof(struct sockaddr_in);
    len = recvfrom(isock, msg, sizeof(msg), 0,
		   (struct sockaddr *)&from_addr, &addr_len);
    if (len < 0) {
	log_debug("recvfrom error %s", strerror(errno));
	return;
    }
    if (len > maxsize) {
	log_msg(LOG_WARNING, "Received message is too big to process");
	return;
    }

    /* Determine how query should be handled */
    fwd = handle_query(&from_addr, msg, &len, &srvr);

    /* If we already know the answer, send it and we're done */
    if (fwd == 0) {
	if (sendto(isock, msg, len, 0, (const struct sockaddr *)&from_addr,
		   addr_len) != len) {
	    log_debug("sendto error %s", strerror(errno));
	}
	return;
    }

    /* If we have domains associated with our servers, send it to the
       appropriate server as determined by srvr */
    if (dns_srv[0].domain != NULL) {
	dnssend(srvr, msg, len);
	return;
    }

    /* 1 or more redundant servers.  Cycle through them as needed */
    processed = 0;
    thisdns   = serv_act;
    for (i = 0; i < serv_cnt; i++) {
	if (dnssend(serv_act, msg, len) == len) {
	    if (i != 0) {
		log_debug("switched to DNS server %s",
			  inet_ntoa(dns_srv[thisdns].addr.sin_addr));
	    }
	    processed = 1;
	    break;
	}
	thisdns = (thisdns + 1) % serv_cnt;
    }

    if (processed == 0) {
#ifdef ENABLE_CACHE
	int	packetlen;
	char	packet[maxsize+4];

	/*
	 * If we couldn't send the packet to our DNS servers,
	 * perhaps the `network is unreachable', we tell the
	 * client that we are unable to process his request
	 * now.  This will show a `No address (etc.) records
	 * available for host' in nslookup.  With this the
	 * client won't wait hang around till he gets his
	 * timeout.
	 * For this feature dnrd has to run on the gateway
	 * machine.
	 */

	if ((packetlen = master_dontknow(msg, len, packet)) > 0) {
	    if (!dnsquery_find(msg, &from_addr)) {
		log_debug("ERROR: couldn't find the original query");
		return;
	    }
	    if (sendto(isock, msg, len, 0, (const struct sockaddr *)&from_addr,
		       addr_len) != len) {
		log_debug("sendto error %s", strerror(errno));
		return;
	    }
	}
#endif	
    }
}

/*
 * dnsrecv()							22OCT99wzk
 *
 * Abstract: A small wrapper for recv()/recvfrom() with output of an
 *           error message if needed.
 *
 * Returns:  A positove number indicating of the bytes received, -1 on a
 *           recvfrom error and 0 if the received message is too large.
 */
static int dnsrecv(int k, void *msg, int len)
{
    int	rc, fromlen;
    struct sockaddr_in from;

    fromlen = sizeof(struct sockaddr_in);
    rc = recvfrom(dns_srv[k].sock, msg, len, 0,
		  (struct sockaddr *) &from, &fromlen);

    if (rc == -1) {
	log_msg(LOG_ERR, "recvfrom error: %s: %m",
		inet_ntoa(dns_srv[k].addr.sin_addr));
	return (-1);
    }
    else if (rc == len) {
	log_msg(LOG_NOTICE, "packet too large: %s",
		inet_ntoa(dns_srv[k].addr.sin_addr));
	return (0);
    }
    else if (memcmp(&from.sin_addr, &dns_srv[k].addr.sin_addr,
		    sizeof(from.sin_addr)) != 0) {
	log_msg(LOG_WARNING, "unexpected server: %s",
		inet_ntoa(from.sin_addr));
	return (0);
    }

    return (rc);
}

/*
 * handle_udpreply()
 *
 * This function handles udp DNS requests by either replying to them (if we
 * know the correct reply via master, caching, etc.), or forwarding them to
 * an appropriate DNS server.
 */
void handle_udpreply(int srvidx)
{
    const int          maxsize = 512; /* According to RFC 1035 */
    char               msg[maxsize+4];/* Do we really want this on the stack?*/
    int                len;
    struct sockaddr_in from_addr;
    unsigned           addr_len;

    len = dnsrecv(srvidx, msg, sizeof(msg));
    if (opt_debug) {
	char buf[80];
	sprintf_cname(&msg[12], buf, 80);
	log_debug("Received DNS reply for \"%s\"", buf);
    }
    if (len > 0) {
	dump_dnspacket("reply", msg, len);
#ifdef ENABLE_CACHE
	cache_dnspacket(msg, len);
#endif
	log_debug("Forwarding the reply to the host");
	addr_len = sizeof(struct sockaddr_in);
	if (!dnsquery_find(msg, &from_addr)) {
	    log_debug("ERROR: couldn't find the original query");
	}
	else if (sendto(isock, msg, len, 0,
			(const struct sockaddr *)&from_addr,
			addr_len) != len) {
	    log_debug("sendto error %s", strerror(errno));
	}
    }
}
