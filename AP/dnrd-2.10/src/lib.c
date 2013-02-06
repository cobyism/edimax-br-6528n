/*

    File: lib.c
    
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "lib.h"
#include "common.h"

#define	DEBUG(x)		

#ifndef COMPACK_SIZE
void *allocate(size_t size)
{
    void	*p;

    if ((p = malloc(size)) == NULL) {
	log_msg(LOG_ALERT, "memory allocation error");
	exit (1);
    }

    memset(p, 0, size);
    return (p);
}
#endif

#ifndef COMPACK_SIZE
void *reallocate(void *p, size_t size)
{
    if ((p = realloc(p, size)) == NULL) {
	log_msg(LOG_ALERT, "memory allocation error");
	exit (1);
    }

    return (p);
}
#endif


#ifndef COMPACK_SIZE
char *strlwr(char *string)
{
    unsigned int c;
    unsigned char *p;

    p = string;
    while ((c = *p) != 0) {
	*p++ = tolower(c);
    }

    return (string);
}	
#endif
#ifndef COMPACK_SIZE
char *strupr(char *string)
{
    unsigned int c;
    unsigned char *p;

    p = string;
    while ((c = *p) != 0) {
	*p++ = toupper(c);
    }

    return (string);
}	
#endif


#ifndef COMPACK_SIZE
char *skip_ws(char *string)
{
    unsigned int c;

    while ((c = *string) == ' '  ||  c == '\t') {
	string++;
    }

    return (string);
}
#endif

#ifndef COMPACK_SIZE
char *noctrl(char *buffer)
{
    int	len, i;
    unsigned char *p;

    if ((p = buffer) == NULL) {
	return (NULL);
    }

    len = strlen(p);
    for (i=len-1; i>=0; i--) {
	if (p[i] <= 32) {
	    p[i] = '\0';
	}
	else {
	    break;
	}
    }

    return (p);
}
#endif
#ifndef COMPACK_SIZE
char *get_word(char **from, char *to, int maxlen)
{
    unsigned int c;
    unsigned char *p;
    int	k;

    maxlen -= 2;
    while ((c = **from) != 0  &&  c <= 32) {
	*from += 1;
    }

    *(p = to) = k = 0;
    while ((c = **from) != 0) {
	if (c == ' '  ||  c == '\t'  ||  c < 32) {
	    break;
	}

	*from += 1;
	if (k < maxlen) {
	    p[k++] = c;
	}
    }

    p[k] = 0;
    return (to);
}
#endif

#ifndef COMPACK_SIZE
char *get_quoted(char **from, int delim, char *to, int max)
{
    unsigned int c;
    int	k;

    to[0] = k = 0;
    max -= 2;
    
    while ((c = **from) != 0) {
	*from += 1;
	if (c == delim) {
	    break;
	}

	if (k < max) {
	    to[k++] = c;
	}
    }

    to[k] = 0;
    return (to);
}
#endif

#ifndef COMPACK_SIZE
char *copy_string(char *y, char *x, int len)
{
    x = skip_ws(x);
    noctrl(x);

    len -= 2;
    if (strlen(x) >= len) {
	x[len] = 0;
    }

    if (y != x) {
	strcpy(y, x);
    }
	    
    return (y);
}
#endif


#ifndef COMPACK_SIZE
unsigned int get_stringcode(char *string)
{
    unsigned int c, code;
    int	i;

    code = 0;
    for (i=0; (c = (unsigned char) string[i]) != 0; i++) {
	if (isupper(c)) {
	    c = tolower(c);
	}

	code = code + c;
    }

    code = (code & 0xFF) + (strlen(string) << 8);
    return (code);
}
#endif

