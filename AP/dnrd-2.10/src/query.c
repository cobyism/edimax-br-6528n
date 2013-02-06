/*
 * query.c
 *
 * This file contains the data definitions, function definitions, and
 * variables used to implement our DNS query list.
 *
 * Assumptions: No multithreading.
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

#include "query.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* DEBUG */

/*
 * This is the data structure used to store DNS queries that haven't been
 * answered yet.
 */
struct dnsq_t {
    unsigned short     local_qid;  /* network byte-ordered */
    unsigned short     client_qid; /* network byte-ordered */
    struct dnsq_t *    next;
    struct sockaddr_in client;
    time_t             recvtime;
};
typedef struct dnsq_t dnsquery;

/*
 * Static variables.
 */
static dnsquery *     head = 0;
static dnsquery *     tail = 0;
static unsigned short qidcount = 0;

/*
 * dnsquery_add() - add a DNS query to our list.
 *
 * In:       client - the host that sent us the query.
 *           len    - the length of the DNS query
 * In/Out:   msg    - the DNS query
 *
 * Returns:  1 on success, 0 on failure.
 *
 * Abstract: This function takes the query msg and adds it to the end
 *           of our list.  In addition, it generates a new query id
 *           and updates msg accordingly.  If the query msg is already
 *           in our list (a retransmission), then we'll act as if we're
 *           adding it, but we won't actually add it again.
 */
int dnsquery_add(const struct sockaddr_in* client, char* msg, unsigned len)
{
    unsigned short client_qid;
    dnsquery *ptr;
    dnsquery *query;

    memcpy(&client_qid, msg, 2);
    
    /* If entry already exists, then don't actually add it again */
    for (ptr = head; ptr != 0; ptr = ptr->next) {
	if (ptr->client_qid == client_qid) {
	    memcpy(msg, &(ptr->local_qid), 2);
	    return 1;
	}
    }

    /* Create the new dnsquery entry */
    query = (dnsquery*)malloc(sizeof(dnsquery));
    query->local_qid = htons(qidcount++);
    query->client_qid = client_qid;
    query->next = 0;
    memcpy(&(query->client), client, sizeof(struct sockaddr_in));
    query->recvtime = time(0);

    /* Update the query number in msg */
    memcpy(msg, &(query->local_qid), 2);

    /* Update our dnsquery list */
    tail ? (tail->next = query) : (head = query);
    tail = query;
    return 1;
}

/*
 * dnsquery_find() - find the client to whom this reply should be sent.
 *
 * In/Out:   reply  - the DNS reply
 * Out:      client - a buffer where the client to whom this reply
 *                    should be sent will be copied.
 *
 * Returns:  1 on success, 0 on failure.
 *
 * Abstract: This function finds the client to whom this reply should be
 *           sent.  In addition, it finds the client's original query id
 *           and updates reply accordingly.  Once found, the dnsquery is
 *           removed from our list.
 *
 * Assumptions: reply is at least 2 bytes long.
 */
int dnsquery_find(char* reply, struct sockaddr_in* client)
{
    unsigned short qid;
    dnsquery *     ptr;
    dnsquery *     last = 0;

    memcpy(&qid, reply, 2);
    for (ptr = head; ptr != 0; ptr = ptr->next) {
	if (qid == ptr->local_qid) {
	    memcpy(client, &(ptr->client), sizeof(struct sockaddr_in));
	    memcpy(reply, &(ptr->client_qid), 2);
	    last ? (last->next = ptr->next) : (head = ptr->next);
	    if (tail == ptr) tail = last;
	    free(ptr);
	    return 1;
	}
	last = ptr;
    }
    return 0;
}

/*
 * dnsquery_timeout() - Remove queries that are too old.
 *
 * In:       age - remove all entries older than this many seconds.
 *
 * Returns:  The number of entries that were removed.
 *
 * Abstract: This function is used to remove queries that have timed out.
 *           This should prevent our query list from having memory leaks.
 */
int dnsquery_timeout(time_t age)
{
    int            count = 0;
    dnsquery *     ptr;
    time_t         tval;

    tval = time(0) - age;

    for (ptr = head; (ptr != 0) && (ptr->recvtime < tval); ptr = head) {
	head = ptr->next;
	if (tail == ptr) tail = 0;
	free(ptr);
	count++;
    }
    if (count) log_debug("dnsquery_timeout: removed %d entries", count);
    return count;
}

#ifdef DEBUG
/*
 * dnsquery_dump() - Display the current state of the query queue.
 *
 * Abstract:  This function is used for debugging purposes only.
 */
void dnsquery_dump()
{
    dnsquery *     ptr;

    printf("Current queue:\n");
    printf("  head = %p, tail = %p\n", head, tail);
    printf("  my_qid   h_qid  from_addr\n");
    printf("  ------  ------  ---------------\n");
    for (ptr = head; ptr != 0; ptr = ptr->next) {
	printf("  %-6d  %-6d  %s\n", ntohs(ptr->local_qid), ptr->client_qid,
	       inet_ntoa(ptr->client.sin_addr));
    }
    printf("\n");
}
#endif /* DEBUG */
