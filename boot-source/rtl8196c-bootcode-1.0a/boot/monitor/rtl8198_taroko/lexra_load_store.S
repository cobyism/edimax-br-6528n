#include <asm/regdef.h>
//#include "mips.h"
#include "radiax.h"

/******************************************************************
void memset64s(Word32 *Dst,int c, int Len)
	a0 -> Dst
	a1 -> c
	a2 -> Len , Note: Len is counted by 2-bytes unit 
*/
	.text
	.section ".memset" , "ax"
	.set	noat
	.set	noreorder
	.globl	memset64s
	.ent	memset64s

memset64s:
	la		t0, 1f
	la		t1, 2f-4
	sra		t2, a2, 2
	subu	t2, t2, 1			// t2 = Len/4 - 1
	mtru	t0, lps0
	mtru	t1, lpe0
	mtru	t2, lpc0
	move	t0, a1
	move	t1, a1
1:
	stp		t0, (a0)+8
	nop
2:
	jr		ra
	nop

	.end	memset64s

/******************************************************************
void memcpy64s(Word32 *Dst, Word32 *Src, int Len)
	a0 -> Dst
	a1 -> Src
	a2 -> Len, Note: Len is counted by 2-bytes unit
*/
	.text
	.section ".memset" , "ax"
	.set	noat
	.set	noreorder
	.globl	memcpy64s
	.ent	memcpy64s

memcpy64s:
	la		t0, 1f
	la		t1, 2f-4
	sra		t2, a2, 2
	subu	t2, t2, 1			// t2 = Len/4 - 1
	mtru	t0, lps0
	mtru	t1, lpe0
	mtru	t2, lpc0
1:
	ltp		t0, (a1)+8
	nop
	stp		t0, (a0)+8
	nop
2:
	jr		ra
	nop

	.end	memcpy64s


