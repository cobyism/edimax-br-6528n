/*
 * tcprequest.c - handle TCP request by transparent proxying.
 *
 * Copyright (C) 1999 Wolfgang Zekoll <wzk@quietsche-entchen.de>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>

#include "common.h"
#include "relay.h"
#include "cache.h"
#include "pthread.h"


typedef struct tcp_handle_info {
    int	               connect;
    struct sockaddr_in client;
    unsigned           len;
} tcp_handle_t;

/*
 * alarm_handler() - Timeout handler for the connect() function in ip_open().
 *		It's purpose is just to interrupt the connect() system call.
 */
static void alarm_handler(int dummy)
{
    return;
}


/*
 * ip_open() - Open a tcp socket to the given DNS server
 *
 * In:      server - The address to which to open a connection
 *          port   - The tcp port to which to open a connection
 *
 * Returns: the opened socket on success, -1 on failure.
 */
static int ip_open(struct sockaddr_in server, unsigned int port)
{
    int	sock;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) return (-1);

    server.sin_family = AF_INET;
    server.sin_port   = htons(port);

    signal(SIGALRM, alarm_handler);
    alarm(10);
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
	return (-1);
    }

    alarm(0);
    signal(SIGALRM, SIG_DFL);

    return (sock);
}	

/*
 * tcp_handler() - This function accepts an incoming connection on
 *		   tcpsock and handles the client request by transparent
 *                 proxying.  After the request is served the thread
 *                 terminates.
 */
static void *tcp_handler(void *dummy)
{
    tcp_handle_t      *arg = (tcp_handle_t *)dummy;
    int	               rc, bytes;
    int                maxsock;
    int                server[MAX_SERV];
    char	       buffer[4096];
    struct timeval     tov;
    fd_set	       connection, available;
    unsigned short     tcpsize;
    int                i;
    int	               connect = arg->connect;
    struct sockaddr_in client  = arg->client;

    free(arg);

    for(i = 0; i < MAX_SERV; i++) {
	server[i] = -1;
    }

    maxsock = connect + 1;
    FD_ZERO(&connection);
    FD_SET(connect, &connection);

    while (1) {
	int child_die = 0;

	memcpy(&available, &connection, sizeof(fd_set));
	tov.tv_sec  = 120;	/* Give the DNS 2min to answer. */
	tov.tv_usec = 0;

	rc = select(maxsock, &available, NULL, NULL, &tov);
	if (rc < 0) {
	    log_msg(LOG_ERR, "[%d] select() error: %m\n", getpid());

#ifdef PTHREAD_LIB
	    pthread_exit(0);
#else
		return (void *)0;
#endif
	}
	else if (rc == 0) {
	    log_msg(LOG_NOTICE, "[%d] connection timed out", getpid());
#ifdef PTHREAD_LIB
	    pthread_exit(0);
#else
                return (void *)0;
#endif
	}

	/* Forward replies from DNS server to client */
	for(i = 0; i < serv_cnt; i++) {
	    if (server[i] == -1) continue;
	    if (FD_ISSET(server[i], &available)) {
		log_debug("[%d] Received tcp reply.  Forwarding...", getpid());
		if ((bytes = read(server[i], buffer, sizeof(buffer))) <= 0) {
		    child_die = 1;
		    break;
		}
		dump_dnspacket("reply", buffer + 2, bytes - 2);
#ifdef ENABLE_CACHE
		cache_dnspacket(buffer + 2, bytes - 2);
#endif
		if (write(connect, buffer, bytes) != bytes) {
		    child_die = 1;
		    break;
		}
	    }
	}
	if (child_die) break;

	/* Forward requests from client to DNS server */
	if (FD_ISSET(connect, &available)) {
	    int retn;
	    unsigned srvridx;

	    bytes = read(connect, &tcpsize, sizeof(tcpsize));
	    /* check for connection close */
	    if (bytes == 0) break;
	    log_debug("[%d] Received tcp DNS query...", getpid());

	    if (bytes < 0) {
		log_debug("[%d] tcp read error %s", getpid(), strerror(errno));
		break;
	    }
	    memcpy(buffer, &tcpsize, sizeof(tcpsize));
	    tcpsize = ntohs(tcpsize);
	    if (tcpsize > (sizeof(buffer) + 2)) {
		log_msg(LOG_WARNING,
			"[%d] Received tcp message is too big to process",
			getpid());
		break;
	    }
	    bytes = read(connect, buffer + 2, (size_t)tcpsize);
	    if (bytes == 0) {
		log_msg(LOG_ERR, "[%d] tcp DNS query is mangled", getpid());
		break;
	    }
	    if (bytes < 0) {
		log_debug("[%d] tcp read error %s", getpid(), strerror(errno));
		break;
	    }
	    retn = handle_query(&client, &buffer[2], &bytes, &srvridx);

	    /* If we can reply locally (master, cache, no servers), do so. */
	    if (retn == 0) {
		unsigned short len = htons((unsigned short)bytes);
		memcpy(buffer, &len, sizeof(len));
		if (write(connect, buffer, bytes + 2) != (bytes + 2)) break;
	    }

	    /*
	     * Forward DNS request to the appropriate server.
	     * Open a socket if one doesn't already exist.
	     */
	    else {
		if (server[srvridx] == -1) {
		    server[srvridx] = ip_open(dns_srv[srvridx].addr, 53);
		    if (server[srvridx] < 0) {
			log_msg(LOG_ERR, "[%d] Can't connect to server",
				getpid());
			break;
		    }
		    if (server[srvridx] > (maxsock - 1)) {
			maxsock = server[srvridx] + 1;
		    }
		    FD_SET(server[srvridx], &connection);
		}
		if (write(server[srvridx], buffer, bytes + 2) != (bytes + 2)) {
		    break;
		}
	    }
	}

    }
    /* The child process is done.  It can now die */
    log_debug("[%d] Closing tcp connection", getpid());
    for(i = 0; i < MAX_SERV; i++) {
	if (server[i] != -1) close(server[i]);
    }
    close(connect);
#ifdef PTHREAD_LIB
    pthread_exit(0);
#endif
    /* NOTREACHED */
    return (void *)0;
}

/*
 * handle_tcprequest()
 *
 * This function spawns a thread to actually handle the tcp request.
 * A thread is used instead of a process so that any DNS replies can
 * be placed in cache of the parent process.
 */
void handle_tcprequest()
{
    tcp_handle_t *arg;
#ifdef PTHREAD_LIB
    pthread_t t;
#endif

    arg = (tcp_handle_t *)malloc(sizeof(tcp_handle_t));
    arg->len = sizeof(arg->client);

    arg->connect = accept(tcpsock, (struct sockaddr *) &(arg->client), 
			  &(arg->len));
    if (arg->connect < 0) {
	log_msg(LOG_ERR, "accept error: %m");
	return;
    }

#ifdef PTHREAD_LIB
    if (pthread_create(&t, NULL, tcp_handler, (void *)arg)) {
	log_msg(LOG_ERR, "Couldn't spawn thread to handle tcp connection");
    }
#else
	{
     int pid ;
     pid = fork();
     if (pid < 0)
         log_msg(LOG_ERR, "Socket Fork Error");
     if (pid == 0)  {
         close(tcpsock);
         tcp_handler(arg);
         exit(0);
     }
	}
#endif
}
