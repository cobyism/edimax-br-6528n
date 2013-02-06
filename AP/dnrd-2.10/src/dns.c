
/*

    File: dns.c
    
    Copyright (C) 1999 by Wolfgang Zekoll <wzk@quietsche-entchen.de>

    This source is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This source is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns.h"
#include "lib.h"
#include "common.h"


static int get_objectname(unsigned char *msg, unsigned char **here,
			  char *string, int k);


int free_packet(dnsheader_t *x)
{
    free(x->packet);
    free(x);
    return (0);
}

static dnsheader_t *alloc_packet(void *packet, int len)
{
    dnsheader_t *x;

    x = malloc(sizeof(dnsheader_t));
    memset(x, 0, sizeof(dnsheader_t));

    x->packet = malloc(len + 2);
    x->len    = len;
    memcpy(x->packet, packet, len);

    return (x);
}

static dnsheader_t *decode_header(void *packet, int len)
{
    unsigned short int *p;
    dnsheader_t *x;

    x = alloc_packet(packet, len);
    p = (unsigned short int *) x->packet;

    x->id      = ntohs(p[0]);
    x->u       = ntohs(p[1]);
    x->qdcount = ntohs(p[2]);
    x->ancount = ntohs(p[3]);
    x->nscount = ntohs(p[4]);
    x->arcount = ntohs(p[5]);

    x->here    = (char *) &x->packet[12];
    return (x);
}

static int raw_dump(dnsheader_t *x)
{
    unsigned int c;
    int	start, i, j;

    start = x->here - x->packet;
    for (i = 0; i < x->len; i += 16) {
	fprintf(stderr, "%03X -", i);

	for (j = i; j < x->len  &&  j < i+16; j++) {
	    fprintf(stderr, " %02X", ((unsigned int) x->packet[j]) & 0XFF);
	}
	for (; j < i+16; j++) {
	    fprintf(stderr, "   ");
	}

	fprintf(stderr, "  ");
	for (j = i; j < x->len  &&  j < i+16; j++) {
	    c = ((unsigned int) x->packet[j]) & 0XFF;
	    fprintf(stderr, "%c", (c >= 32  &&  c < 127) ? c : '.');
	}
	
	fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");
    return (0);
}

static int get_objectname(unsigned char *msg, unsigned char **here,
			  char *string, int k)
{
    unsigned int len;
    int	i;

    while ((len = **here) != 0) {
	*here += 1;

	if (len & 0xC0) {
	    unsigned offset;
	    unsigned char *p;

	    offset = ((len & ~0xc0) << 8) + **here;
	    p = &msg[offset];
	    k = get_objectname(msg, &p, string, k);

	    break;
	}
	else if (len < 64) {
	    for (i=0; i < len; i++) {
		string[k++] = **here;
		*here += 1;
	    }

	    string[k++] = '.';
	}
    }

    *here += 1;
    string[k] = 0;

    return (k);
}

static unsigned char *read_record(dnsheader_t *x, rr_t *y,
				  unsigned char *here, int question)
{
    int	k, len;
    unsigned short int conv;

    /*
     * Read the name of the resource ...
     */

    k = get_objectname(x->packet, &here, y->name, 0);
    y->name[k] = 0;

    /*
     * ... the type of data ...
     */

    memcpy(&conv, here, sizeof(unsigned short int));
    y->type = ntohs(conv);
    here += 2;

    /*
     * ... and the class type.
     */

    memcpy(&conv, here, sizeof(unsigned short int));
    y->class = ntohs(conv);
    here += 2;

    /*
     * Question blocks stop here ...
     */

    if (question != 0) return (here);

    /*
     * ... while answer blocks carry a TTL and the actual data.
     */

    memcpy(&y->ttl, here, sizeof(unsigned long int));
    y->ttl = ntohl(y->ttl);
    here += 4;

    /*
     * Fetch the resource data.
     */

    memcpy(&y->len, here, sizeof(unsigned short int));
    len = y->len = ntohs(y->len);
    here += 2;
    if (y->len > sizeof(y->data) - 4) {
	y->len = sizeof(y->data) - 4;
    }

    memcpy(y->data, here, y->len);
    here += len;
    y->data[y->len] = 0;

    return (here);
}


int dump_dnspacket(char *type, unsigned char *packet, int len)
{
    int	i;
    rr_t	y;
    dnsheader_t *x;

    if (opt_debug < 2) return 0;
    x = decode_header(packet, len);

    fprintf(stderr, "\n");
    fprintf(stderr, "- -- %s\n", type);
    raw_dump(x);

    fprintf(stderr, "\n");
    fprintf(stderr, "id= %u, q= %d, opc= %d, aa= %d, wr/ra= %d/%d, "
	     "trunc= %d, rcode= %d [%04X]\n",
	     x->id, GET_QR(x->u), GET_OPCODE(x->u), GET_AA(x->u),
	     GET_RD(x->u), GET_RA(x->u), GET_TC(x->u), GET_RCODE(x->u), x->u);

    fprintf(stderr, "qd= %u\n", x->qdcount);
    x->here = read_record(x, &y, x->here, 1);
    fprintf(stderr, "  name= %s, type= %d, class= %d\n",
	     y.name, y.type, y.class);
    
    fprintf(stderr, "ans= %u\n", x->ancount);
    for (i = 0; i < x->ancount; i++) {
	x->here = read_record(x, &y, x->here, 0);
	fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
		 y.name, y.type, y.class, y.ttl);
    }
	    
    fprintf(stderr, "ns= %u\n", x->nscount);
    for (i = 0; i < x->nscount; i++) {
	x->here = read_record(x, &y, x->here, 0);
	fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
		 y.name, y.type, y.class, y.ttl);
    }
	    
    fprintf(stderr, "ar= %u\n", x->arcount);
    for (i = 0; i < x->arcount; i++) {
	x->here = read_record(x, &y, x->here, 0);
	fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
		 y.name, y.type, y.class, y.ttl);
    }
	    
    fprintf(stderr, "\n");
    free_packet(x);

    return (0);
}



#ifndef COMPACK_SIZE
dnsheader_t *parse_packet(unsigned char *packet, int len)
{
    dnsheader_t *x;

    x = decode_header(packet, len);
    return (x);
}
#endif

#ifndef COMPACK_SIZE
int get_dnsquery(dnsheader_t *x, rr_t *query)
{
    char	*here;

    if (x->qdcount == 0) {
	return (1);
    }

    here = &x->packet[12];
    read_record(x, query, here, 1);

    return (0);
}
#endif
#ifndef COMPACK_SIZE
/*
 * parse_query()
 *
 * The function get_dnsquery() returns us the query part of an
 * DNS packet.  For this we must already have a dnsheader_t
 * packet which is additional work.  To speed things a little
 * bit up (we use it often) parse_query() gets the query direct.
 */
unsigned char *parse_query(rr_t *y, unsigned char *msg, int len)
{
    int	k;
    unsigned char *here;
    unsigned short int conv;

    if (ntohs(((short int *) msg)[2]) == 0) {		/* C is nice. */
	return (0);
    }

    y->flags = ntohs(((short int *) msg)[1]);

    here = &msg[PACKET_DATABEGIN];
    k = get_objectname(msg, &here, y->name, 0);
    y->name[k] = 0;

    memcpy(&conv, here, sizeof(unsigned short int));
    y->type = ntohs(conv);
    here += 2;

    memcpy(&conv, here, sizeof(unsigned short int));
    y->class = ntohs(conv);
    here += 2;

    k = strlen(y->name);
    if (k > 0  &&  y->name[k-1] == '.') {
	y->name[k-1] = '\0';
    }

    strlwr(y->name);
	    
    return (here);
}
#endif

