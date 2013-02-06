/*
 * query.h - Functions used to store & retrieve DNS queries.
 *
 * Assumptions: Time is measured as a 32-bit number in seconds since Jan 1 1970.
 *              Therefore, it is assumed that this program will be fixed before
 *              the year 2038 or so.
 *
 * Copyright (C) 1998 Brad M. Garcia <garsh@home.com>
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

#ifndef _DNSR_QUERY_H_
#define _DNSR_QUERY_H_

#include <time.h>
#include <netinet/in.h>

/*
 * These are function used for manipulating DNS queries.
 * Detailed descriptions of these functions appear in query.c
 */

/* Add a DNS query to our list */
int dnsquery_add(const struct sockaddr_in* client, char* msg, unsigned len);

/* Find the client to which this DNS reply should be sent */
int dnsquery_find(char* reply, struct sockaddr_in* client);

/* Remove DNS queries which have timed out */
int dnsquery_timeout(time_t age);

#ifdef DEBUG
/* Dump the current queue state */
void dnsquery_dump();
#endif /* DEBUG */

#endif  /* _DNSR_QUERY_H_ */
