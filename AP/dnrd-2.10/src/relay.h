/*
 * relay.h - Functions used to do the actual processing
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

#ifndef _DNRD_RELAY_H_
#define _DNRD_RELAY_H_

#include <netinet/in.h>

/* The main loop */
void run();

/* Determine what to do with a DNS request */
int handle_query(const struct sockaddr_in *fromaddrp, char *msg, int *len,
		 unsigned *srvidx);

#endif  /* _DNRD_RELAY_H_ */
