/*

    File: cache.h
    
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

#ifndef _DNRD_CACHE_H_
#define	_DNRD_CACHE_H_

extern char cache_param[200];

/* Interface for DNS cache */
int cache_dnspacket(void *packet, int len);
int cache_lookup(void *packet, int len);
int cache_expire(void);
int cache_init(void);

#endif /* _DNRD_CACHE_H_ */

