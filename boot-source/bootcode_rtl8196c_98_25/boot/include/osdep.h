#ifndef	__OSDEP_H__
#define __OSDEP_H__

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#ifndef __BIG_ENDIAN
	#define	__BIG_ENDIAN
#endif	

/* Taken from /usr/include/linux/hfs_sysdep.h */
#if defined(__BIG_ENDIAN)
#	if !defined(__constant_htonl)
#		define __constant_htonl(x) (x)
#	endif
#	if !defined(__constant_htons)
#		define __constant_htons(x) (x)
#	endif
#elif defined(__LITTLE_ENDIAN)
#	if !defined(__constant_htonl)
#		define __constant_htonl(x) \
        ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                             (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                             (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                             (((unsigned long int)(x) & 0xff000000U) >> 24)))
#	endif
#	if !defined(__constant_htons)
#		define __constant_htons(x) \
        ((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                              (((unsigned short int)(x) & 0xff00) >> 8)))
#	endif
#else
#	error "Don't know if bytes are big- or little-endian!"
#endif

#ifdef __LITTLE_ENDIAN 
#define ntohl(x) \
(__builtin_constant_p(x) ? \
 __constant_htonl((x)) : \
 __swap32(x))
#define htonl(x) \
(__builtin_constant_p(x) ? \
 __constant_htonl((x)) : \
 __swap32(x))
#define ntohs(x) \
(__builtin_constant_p(x) ? \
 __constant_htons((x)) : \
 __swap16(x))
#define htons(x) \
(__builtin_constant_p(x) ? \
 __constant_htons((x)) : \
 __swap16(x))
#endif


static inline unsigned long int __swap32(unsigned long int x)
{

#if 0
	__asm__("xchgb %b0,%h0\n\t"
		"rorl $16,%0\n\t"
		"xchgb %b0,%h0"
		: "=q" (x)
		: "0" (x));
	return x;
#endif
	volatile unsigned long rx;
	
	rx =((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
		(((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
	    (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
	    (((unsigned long int)(x) & 0xff000000U) >> 24)));
	return rx;	
}

static inline unsigned short int __swap16(unsigned short int x)
{

#if 0	
	__asm__("xchgb %b0,%h0"
		: "=q" (x)
		: "0" (x));
	return x;
#endif

	volatile unsigned short rx;
	rx  =
	((volatile unsigned short int)((((volatile unsigned short int)(x) & 0x00ff) << 8) | \
    (((volatile unsigned short int)(x) & 0xff00) >> 8)));

	return rx;
	
}

/* Make routines available to all */
#define	swap32(x)	__swap32(x)
#define	swap16(x)	__swap16(x)


#ifdef __BIG_ENDIAN
#ifndef ntohl
	#define ntohl(x) 	(x)
	#define ntohs(x) 	(x)
	#define htonl(x) 	(x)
	#define htons(x) 	(x)
#endif	
#endif


//#include <asm/string.h>
#include <asm/io.h>

typedef	unsigned long Address;

/* ANSI prototyping macro */
#ifdef	__STDC__
#define	P(x)	x
#else
#define	P(x)	()
#endif

#endif

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */
