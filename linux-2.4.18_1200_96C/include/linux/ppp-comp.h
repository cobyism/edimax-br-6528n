/*
 * ppp-comp.h - Definitions for doing PPP packet compression.
 *
 * Copyright (c) 1994 The Australian National University.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, provided that the above copyright
 * notice appears in all copies.  This software is provided without any
 * warranty, express or implied. The Australian National University
 * makes no representations about the suitability of this software for
 * any purpose.
 *
 * IN NO EVENT SHALL THE AUSTRALIAN NATIONAL UNIVERSITY BE LIABLE TO ANY
 * PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * THE AUSTRALIAN NATIONAL UNIVERSITY HAVE BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * THE AUSTRALIAN NATIONAL UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE AUSTRALIAN NATIONAL UNIVERSITY HAS NO
 * OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
 * OR MODIFICATIONS.
 *
 * $Id: ppp-comp.h,v 1.2 2008/11/10 12:21:54 davidhsu Exp $
 */

/*
 *  ==FILEVERSION 20020715==
 *
 *  NOTE TO MAINTAINERS:
 *     If you modify this file at all, please set the above date.
 *     ppp-comp.h is shipped with a PPP distribution as well as with the kernel;
 *     if everyone increases the FILEVERSION number above, then scripts
 *     can do the right thing when deciding whether to install a new ppp-comp.h
 *     file.  Don't change the format of that line otherwise, so the
 *     installation script can recognize it.
 */

#ifndef _NET_PPP_COMP_H
#define _NET_PPP_COMP_H

/*
 * The following symbols control whether we include code for
 * various compression methods.
 */

#ifndef DO_BSD_COMPRESS
#define DO_BSD_COMPRESS	1	/* by default, include BSD-Compress */
#endif
#ifndef DO_DEFLATE
#define DO_DEFLATE	1	/* by default, include Deflate */
#endif
#define DO_PREDICTOR_1	0
#define DO_PREDICTOR_2	0

/*
 * Structure giving methods for compression/decompression.
 */

struct compressor {
	int	compress_proto;	/* CCP compression protocol number */

	/* Allocate space for a compressor (transmit side) */
	void	*(*comp_alloc) (unsigned char *options, int opt_len);

	/* Free space used by a compressor */
	void	(*comp_free) (void *state);

	/* Initialize a compressor */
	int	(*comp_init) (void *state, unsigned char *options,
			      int opt_len, int unit, int opthdr, int debug);

	/* Reset a compressor */
	void	(*comp_reset) (void *state);

	/* Compress a packet */
	int     (*compress) (void *state, unsigned char *rptr,
			      unsigned char *obuf, int isize, int osize);

	/* Return compression statistics */
	void	(*comp_stat) (void *state, struct compstat *stats);

	/* Allocate space for a decompressor (receive side) */
	void	*(*decomp_alloc) (unsigned char *options, int opt_len);

	/* Free space used by a decompressor */
	void	(*decomp_free) (void *state);

	/* Initialize a decompressor */
	int	(*decomp_init) (void *state, unsigned char *options,
				int opt_len, int unit, int opthdr, int mru,
				int debug);

	/* Reset a decompressor */
	void	(*decomp_reset) (void *state);

	/* Decompress a packet. */
	int	(*decompress) (void *state, unsigned char *ibuf, int isize,
				unsigned char *obuf, int osize);

	/* Update state for an incompressible packet received */
	void	(*incomp) (void *state, unsigned char *ibuf, int icnt);

	/* Return decompression statistics */
	void	(*decomp_stat) (void *state, struct compstat *stats);
};

/*
 * The return value from decompress routine is the length of the
 * decompressed packet if successful, otherwise DECOMP_ERROR
 * or DECOMP_FATALERROR if an error occurred.
 * 
 * We need to make this distinction so that we can disable certain
 * useful functionality, namely sending a CCP reset-request as a result
 * of an error detected after decompression.  This is to avoid infringing
 * a patent held by Motorola.
 * Don't you just lurve software patents.
 */

#define DECOMP_ERROR		-1	/* error detected before decomp. */
#define DECOMP_FATALERROR	-2	/* error detected after decomp. */

/*
 * CCP codes.
 */

#define CCP_CONFREQ	1
#define CCP_CONFACK	2
#define CCP_TERMREQ	5
#define CCP_TERMACK	6
#define CCP_RESETREQ	14
#define CCP_RESETACK	15

/*
 * Max # bytes for a CCP option
 */

#define CCP_MAX_OPTION_LENGTH	32

/*
 * Parts of a CCP packet.
 */

#define CCP_CODE(dp)		((dp)[0])
#define CCP_ID(dp)		((dp)[1])
#define CCP_LENGTH(dp)		(((dp)[2] << 8) + (dp)[3])
#define CCP_HDRLEN		4

#define CCP_OPT_CODE(dp)	((dp)[0])
#define CCP_OPT_LENGTH(dp)	((dp)[1])
#define CCP_OPT_MINLEN		2

/*
 * Definitions for MPPE.
 */

#define CI_MPPE			18	/* config option for MPPE */
#define CILEN_MPPE		6	/* length of config option */

#define MPPE_PAD		4	/* MPPE growth per frame */
#define MPPE_MAX_KEY_LEN	16	/* largest key length (128-bit) */

/* option bits for ccp_options.mppe */
#define MPPE_OPT_40		0x01	/* 40 bit */
#define MPPE_OPT_128		0x02	/* 128 bit */
#define MPPE_OPT_STATEFUL	0x04	/* stateful mode */
/* unsupported opts */
#define MPPE_OPT_56		0x08	/* 56 bit */
#define MPPE_OPT_MPPC		0x10	/* MPPC compression */
#define MPPE_OPT_D		0x20	/* Unknown */
#define MPPE_OPT_UNSUPPORTED (MPPE_OPT_56|MPPE_OPT_MPPC|MPPE_OPT_D)
#define MPPE_OPT_UNKNOWN	0x40	/* Bits !defined in RFC 3078 were set */

/*
 * This is not nice ... the alternative is a bitfield struct though.
 * And unfortunately, we cannot share the same bits for the option
 * names above since C and H are the same bit.  We could do a u_int32
 * but then we have to do a htonl() all the time and/or we still need
 * to know which octet is which.
 */
#define MPPE_C_BIT		0x01	/* MPPC */
#define MPPE_D_BIT		0x10	/* Obsolete, usage unknown */
#define MPPE_L_BIT		0x20	/* 40-bit */
#define MPPE_S_BIT		0x40	/* 128-bit */
#define MPPE_M_BIT		0x80	/* 56-bit, not supported */
#define MPPE_H_BIT		0x01	/* Stateless (in a different byte) */

/* Does not include H bit; used for least significant octet only. */
#define MPPE_ALL_BITS (MPPE_D_BIT|MPPE_L_BIT|MPPE_S_BIT|MPPE_M_BIT|MPPE_H_BIT)

/* Build a CI from mppe opts (see RFC 3078) */
#define MPPE_OPTS_TO_CI(opts, ci)		\
    do {					\
	u_char *ptr = ci; /* u_char[4] */	\
						\
	/* H bit */				\
	if (opts & MPPE_OPT_STATEFUL)		\
	    *ptr++ = 0x0;			\
	else					\
	    *ptr++ = MPPE_H_BIT;		\
	*ptr++ = 0;				\
	*ptr++ = 0;				\
						\
	/* S,L bits */				\
	*ptr = 0;				\
	if (opts & MPPE_OPT_128)		\
	    *ptr |= MPPE_S_BIT;			\
	if (opts & MPPE_OPT_40)			\
	    *ptr |= MPPE_L_BIT;			\
	/* M,D,C bits not supported */		\
    } while (/* CONSTCOND */ 0)

/* The reverse of the above */
#define MPPE_CI_TO_OPTS(ci, opts)		\
    do {					\
	u_char *ptr = ci; /* u_char[4] */	\
						\
	opts = 0;				\
						\
	/* H bit */				\
	if (!(ptr[0] & MPPE_H_BIT))		\
	    opts |= MPPE_OPT_STATEFUL;		\
						\
	/* S,L bits */				\
	if (ptr[3] & MPPE_S_BIT)		\
	    opts |= MPPE_OPT_128;		\
	if (ptr[3] & MPPE_L_BIT)		\
	    opts |= MPPE_OPT_40;		\
						\
	/* M,D,C bits */			\
	if (ptr[3] & MPPE_M_BIT)		\
	    opts |= MPPE_OPT_56;		\
	if (ptr[3] & MPPE_D_BIT)		\
	    opts |= MPPE_OPT_D;			\
	if (ptr[3] & MPPE_C_BIT)		\
	    opts |= MPPE_OPT_MPPC;		\
						\
	/* Other bits */			\
	if (ptr[0] & ~MPPE_H_BIT)		\
	    opts |= MPPE_OPT_UNKNOWN;		\
	if (ptr[1] || ptr[2])			\
	    opts |= MPPE_OPT_UNKNOWN;		\
	if (ptr[3] & ~MPPE_ALL_BITS)		\
	    opts |= MPPE_OPT_UNKNOWN;		\
    } while (/* CONSTCOND */ 0)

/* MPPE/MPPC definitions by J.D.*/
#define MPPE_STATELESS          MPPE_H_BIT	/* configuration bit H */
#define MPPE_40BIT              MPPE_L_BIT	/* configuration bit L */
#define MPPE_56BIT              MPPE_M_BIT	/* configuration bit M */
#define MPPE_128BIT             MPPE_S_BIT	/* configuration bit S */
#define MPPE_MPPC               MPPE_C_BIT	/* configuration bit C */

/*
 * Definitions for Stac LZS.
 */

#define CI_LZS			17	/* config option for Stac LZS */
#define CILEN_LZS		5	/* length of config option */

#define LZS_OVHD		4	/* max. LZS overhead */
#define LZS_HIST_LEN		2048	/* LZS history size */
#define LZS_MAX_CCOUNT		0x0FFF	/* max. coherency counter value */

#define LZS_MODE_NONE		0
#define LZS_MODE_LCB		1
#define LZS_MODE_CRC		2
#define LZS_MODE_SEQ		3
#define LZS_MODE_EXT		4

#define LZS_EXT_BIT_FLUSHED	0x80	/* bit A */
#define LZS_EXT_BIT_COMP	0x20	/* bit C */

/*
 * Definitions for other, as yet unsupported, compression methods.
 */

#define CI_PREDICTOR_1		1	/* config option for Predictor-1 */
#define CILEN_PREDICTOR_1	2	/* length of its config option */
#define CI_PREDICTOR_2		2	/* config option for Predictor-2 */
#define CILEN_PREDICTOR_2	2	/* length of its config option */

#ifdef __KERNEL__
extern int ppp_register_compressor(struct compressor *);
extern void ppp_unregister_compressor(struct compressor *);
#endif /* __KERNEL__ */

#endif /* _NET_PPP_COMP_H */
