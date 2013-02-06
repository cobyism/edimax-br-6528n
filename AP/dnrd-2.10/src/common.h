/*
 * common.h
 *
 * This file contains definitions useful in all sorts of places.
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

#ifndef _DNRD_COMMON_H_
#define _DNRD_COMMON_H_

#include <netinet/in.h>
#include <syslog.h>
#include <semaphore.h>

#define MAX_SERV           5          /* maximum number of DNS servers */

struct dnssrv_t {
    int                    sock;      /* for communication with server */
    struct sockaddr_in     addr;      /* IP address of server */
    char*                  domain;    /* optional domain to match.  Set to
					 zero for a default server */
};

extern const char*         version;   /* the version number for this program */
extern const char*         progname;  /* the name of this program */
extern int                 opt_debug; /* debugging option */
extern const char*         pid_file; /* File containing current daemon's PID */
extern int                 isock;     /* for communication with clients */
extern int                 tcpsock;   /* same as isock, but for tcp requests */
extern struct dnssrv_t     dns_srv[]; /* DNS server information struct */
extern int                 serv_act; /* index into dns_srv for active server */
extern int                 serv_cnt;  /* number of DNS servers */
extern struct sockaddr_in  recv_addr; /* address on which we receive queries */
extern uid_t               daemonuid; /* to switch to once daemonised */
extern gid_t               daemongid; /* to switch to once daemonised */
extern int                 gotterminal;
extern char		   master_param[200];
extern sem_t               dnrd_sem;  /* Used for all thread synchronization */

/* kill any currently running copies of dnrd */
int kill_current();

/* print messages to stderr or syslog */
void log_msg(int type, const char *fmt, ...);

/* same, but only if debugging is turned on */
void log_debug(const char *fmt, ...);

/* cleanup everything and exit */
void cleanexit(int status);

/* Reads in the domain name as a string, allocates space for the CNAME
   version of it */
char* make_cname(const char *text);
void sprintf_cname(const char *cname, char *buf, int bufsize);

/* Dumping DNS packets */
int dump_dnspacket(char *type, unsigned char *packet, int len);


#endif  /* _DNRD_COMMON_H_ */
