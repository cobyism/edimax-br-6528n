
/*

    File: dns.h
    
    Copyright (C) 1999 by Wolfgang Zekoll  <wzk@quietsche-entchen.de>

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

#ifndef _DNRD_DNS_H_
#define _DNRD_DNS_H_


	/* Internet DNS is the only class we support. */

#define	DNS_CLASS_INET			1


	/* Here's where the packet's first field starts. */
	
#define	PACKET_DATABEGIN		12


/* Macros for manipulating the flags field */
#define MASK_RCODE  0x000f
#define MASK_RA     0x0080
#define MASK_RD     0x0100
#define MASK_TC     0x0200
#define MASK_AA     0x0400
#define MASK_OPCODE 0xe800
#define MASK_QR     0x8000

#define GET_RCODE(x)    ((x) & MASK_RCODE)
#define GET_RA(x)       (((x) & MASK_RA) >> 7)
#define GET_RD(x)       (((x) & MASK_RD) >> 8)
#define GET_TC(x)       (((x) & MASK_TC) >> 9)
#define GET_AA(x)       (((x) & MASK_AA) >> 10)
#define GET_OPCODE(x)   (((x) & MASK_OPCODE) >> 11)
#define GET_QR(x)       (((x) & MASK_QR) >> 15)

#define SET_RCODE(x, y)     ((x) = ((x) & ~MASK_RCODE) | ((y) & MASK_RCODE))
#define SET_RA(x, y)        ((x) = ((x) & ~MASK_RA) | (((y) << 7) & MASK_RA))
#define SET_RD(x, y)        ((x) = ((x) & ~MASK_RD) | (((y) << 8) & MASK_RD))
#define SET_TC(x, y)        ((x) = ((x) & ~MASK_TC) | (((y) << 9) & MASK_TC))
#define SET_AA(x, y)        ((x) = ((x) & ~MASK_AA) | (((y) << 10) & MASK_AA))
#define SET_OPCODE(x, y)    ((x) = ((x) & ~MASK_OPCODE) | \
                             (((y) << 11) & MASK_OPCODE))
#define SET_QR(x, y)        ((x) = ((x) & ~MASK_QR) | (((y) << 15) & MASK_QR))

typedef struct _rr {
    unsigned short      flags;
    char	  name[300];
    
    unsigned int  type;
    unsigned int  class;

    unsigned long ttl;
    int		  len;
    char	  data[300];
} rr_t;


typedef struct _header {
    unsigned short int	id;
    unsigned short      u;

    short int	qdcount;
    short int	ancount;
    short int	nscount;
    short int	arcount;	/* Till here it would fit perfectly to a real
    				 * DNS packet if we had big endian. */

    char	*here;		/* For packet parsing. */
    char	*packet;	/* The actual data packet ... */
    int		len;		/* ... with this size in bytes. */

    char	*rdata;		/* For packet assembly. */
} dnsheader_t;


int free_packet(dnsheader_t *x);

/* static dnsheader_t *decode_header(void *packet, int len); */

dnsheader_t *parse_packet(unsigned char *packet, int len);
int get_dnsquery(dnsheader_t *x, rr_t *query);
unsigned char *parse_query(rr_t *query, unsigned char *msg, int len);

#endif /* _DNRD_DNS_H_ */

