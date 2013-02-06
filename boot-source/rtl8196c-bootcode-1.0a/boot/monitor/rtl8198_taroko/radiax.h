
/* ==RELEASE== */

/* Copyright 1997-2000, Lexra Inc. */
/* ALL RIGHTS RESERVED */

#ifndef __RADIAX_H_INDLUDED
#define __RADIAX_H_INDLUDED


/* Accumulators */

#define m0l	$1		/* ma0l */
#define m0h	$2		/* ma0h */
#define m0	$3		/* ma0  */
#define m1l	$5		/* ma1l */
#define m1h	$6		/* ma1h */
#define m1	$7		/* ma1  */
#define m2l	$9		/* ma2l */
#define m2h	$10		/* ma2h */
#define m2	$11		/* ma2  */
#define m3l	$13		/* ma3l */
#define m3h	$14		/* ma3h */
#define m3	$15		/* ma3  */


/*
** These are the registers used with the MTRU/MFRU instructions.
*/

/* Circular buffer registers */

#define cbs0	$0		/* cs0 */
#define cbs1	$1		/* cs1 */
#define cbs2	$2		/* cs2 */
#define cbe0	$4		/* ce0 */
#define cbe1	$5		/* ce1 */
#define cbe2	$6		/* ce2 */

/* Zero Overhead Loop Control registers */

#define lps0	$16		/* ls0 */
#define lpe0	$17		/* le0 */
#define lpc0	$18		/* lc0 */

/* MAC Mode Register */

#define mmd	$24		/* md */
#define SET_MMD_MT	0x1		/* MAC 32*32 truncate mode */
#define SET_MMD_MS	0x2		/* MAC 32 bit saturate mode */
#define SET_MMD_MF	0x4		/* MAC fractional mode */

#endif /* __RADIAX_H_INDLUDED */
