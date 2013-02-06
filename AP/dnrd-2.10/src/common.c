/*
 * common.c - includes global variables and functions.
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

#include "common.h"
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#ifdef DEBUG
#define OPT_DEBUG 1
#else
#define OPT_DEBUG 0
#endif /* DEBUG */

/*
 * These are all the global variables.
 */
int                 opt_debug = OPT_DEBUG;
int                 opt_serv = 0;
const char*         progname = 0;
#if defined(__sun__)
const char*         pid_file = "/var/tmp/dnrd.pid";
#else
const char*         pid_file = "/var/run/dnrd.pid";
#endif
int                 isock = -1;
int                 tcpsock = -1;
struct dnssrv_t     dns_srv[MAX_SERV];
int                 serv_act = 0;
int                 serv_cnt = 0;
uid_t               daemonuid = 0;
gid_t               daemongid = 0;
const char*         version = "2.10";
int                 gotterminal = 1; /* 1 if attached to a terminal */
sem_t               dnrd_sem;  /* Used for all thread synchronization */

/*
 * This is the address we listen on.  It gets initialized to INADDR_ANY,
 * which means we listen on all local addresses.  Both the address and
 * the port can be changed through command-line options.
 */
#if defined(__sun__)
struct sockaddr_in recv_addr = { AF_INET, 53, { { {0, 0, 0, 0} } } };
#else
struct sockaddr_in recv_addr = { AF_INET, 53, { INADDR_ANY } };
#endif

/*
 * kill_current()
 *
 * Returns: 1 if a currently running dnrd was found & killed, 0 otherwise.
 *
 * Abstract: This function sees if pid_file already exists and, if it does,
 *           will kill the current dnrd process and remove the file.
 */
int kill_current()
{
    int         pid;
    int         retn;
    struct stat finfo;
    FILE*       filep;

    if (stat(pid_file, &finfo) != 0) return 0;

    filep = fopen(pid_file, "r");
    if (!filep) {
	log_msg(LOG_ERR, "%s: Can't open %s\n", progname, pid_file);
	exit(-1);
    }
    if ((retn = (fscanf(filep, "%i%*s", &pid) == 1))) {
	if (kill(pid, SIGTERM)) {
	    log_msg(LOG_ERR, "Couldn't kill dnrd: %s", strerror(errno));
	}
    }
    fclose(filep);
    unlink(pid_file);
    return retn;
}

/*
 * log_msg()
 *
 * In:      type - a syslog priority
 *          fmt  - a formatting string, ala printf.
 *          ...  - other printf-style arguments.
 *
 * Sends a message to stdout or stderr if attached to a terminal, otherwise
 * it sends a message to syslog.
 */
void log_msg(int type, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    if (gotterminal) {
	const char *typestr;
	switch (type) {
	  case LOG_EMERG:   typestr = "EMERG: "; break;
	  case LOG_ALERT:   typestr = "ALERT: "; break;
	  case LOG_CRIT:    typestr = "CRIT:  "; break;
	  case LOG_ERR:     typestr = "ERROR: "; break;
	  case LOG_WARNING: typestr = "Warning: "; break;
	  case LOG_NOTICE:  typestr = "Notice: "; break;
	  case LOG_INFO:    typestr = "Info:  "; break;
	  case LOG_DEBUG:   typestr = "Debug: "; break;
	  default:          typestr = ""; break;
	}
	fprintf(stderr, typestr);
	vfprintf(stderr, fmt, ap);
	if (fmt[strlen(fmt) - 1] != '\n') fprintf(stderr, "\n");
    }
    else {
	vsyslog(type, fmt, ap);
    }
    va_end(ap);
}

/*
 * log_debug()
 *
 * In:      fmt - a formatting string, ala printf.
 *          ... - other printf-style arguments.
 *
 * Abstract: If debugging is turned on, this will send the message
 *           to syslog with LOG_DEBUG priority.
 */
void log_debug(const char *fmt, ...)
{
    va_list ap;

    if (!opt_debug) return;

    va_start(ap, fmt);
    if (gotterminal) {
	fprintf(stderr, "Debug: ");
	vfprintf(stderr, fmt, ap);
	if (fmt[strlen(fmt) - 1] != '\n') fprintf(stderr, "\n");
    }
    else {
	vsyslog(LOG_DEBUG, fmt, ap);
    }
    va_end(ap);
}

/*
 * cleanexit()
 *
 * In:      status - the exit code.
 *
 * Abstract: This closes our sockets, removes /var/run/dnrd.pid,
 *           and then calls exit.
 */
void cleanexit(int status)
{
    int i;
#ifdef PTHREAD_LIB
    /* Only let one process run this code) */
    sem_wait(&dnrd_sem);
#endif 

    log_debug("Shutting down...\n");
    if (isock >= 0) close(isock);
    if (tcpsock >= 0) close(tcpsock);
    for (i = 0; i < serv_cnt; i++) {
	close(dns_srv[i].sock);
    }
    exit(status);
}

/*
 * make_cname()
 *
 * In:       text - human readable domain name string
 *
 * Returns:  Pointer to the allocated, filled in character string on success,
 *           NULL on failure.
 *
 * Abstract: converts the human-readable domain name to the DNS CNAME
 *           form, where each node has a length byte followed by the
 *           text characters, and ends in a null byte.  The space for
 *           this new representation is allocated by this function.
 */
char* make_cname(const char *text)
{
    const char *tptr = text;
    const char *end = text;
    char *cname = (char*)malloc(strlen(text) + 2);
    char *cptr = cname;
    while (*end != 0) {
	size_t diff;
	end = strchr(tptr, '.');
	if (end == NULL) end = text + strlen(text);
	if (end < tptr + 2) {
	    free(cname);
	    return NULL;
	}
	diff = end - tptr;
	*cptr++ = diff;
	memcpy(cptr, tptr, diff);
	cptr += diff;
	tptr = end + 1;
    }
    *cptr = 0;
    assert((unsigned)(cptr - cname) == strlen(text) + 1);
    return cname;
}

void sprintf_cname(const char *cname, char *buf, int bufsize)
{
    const char *cptr = cname;

    if (strlen(cname) > (unsigned)bufsize) {
        if (bufsize > 10) {
            sprintf(buf, "(too long)");
        }
        else {
            buf[0] = 0;
        }
        return;
    }

    while (*cptr) {
        int i;
        int size = *cptr;

        if (cptr++ != cname) sprintf(buf++, ".");

        for(i = 0; i < size; i++, cptr++) {
            sprintf(buf++, "%c", *cptr);
        }
    }
}
