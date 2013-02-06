/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 - 1999 by Ralf Baechle
 * Copyright (C) 1996 by Paul M. Antoine
 * Copyright (C) 1994 - 1999 by Ralf Baechle
 */
#ifndef _ASM_SYSTEM_H
#define _ASM_SYSTEM_H

extern __inline__ void
__sti(void)
{
	__asm__ __volatile__(
		".set\tpush\n\t"
		".set\treorder\n\t"
		".set\tnoat\n\t"
		"mfc0\t$1,$12\n\t"
		"ori\t$1,0x1f\n\t"
		"xori\t$1,0x1e\n\t"
		"mtc0\t$1,$12\n\t"
		".set\tpop\n\t"
		: /* no outputs */
		: /* no inputs */
		: "$1", "memory");
}

/*
 * For cli() we have to insert nops to make shure that the new value
 * has actually arrived in the status register before the end of this
 * macro.
 * R4000/R4400 need three nops, the R4600 two nops and the R10000 needs
 * no nops at all.
 */
extern __inline__ void
__cli(void)
{
	__asm__ __volatile__(
		".set\tpush\n\t"
		".set\treorder\n\t"
		".set\tnoat\n\t"
		"mfc0\t$1,$12\n\t"
		"ori\t$1,1\n\t"
		"xori\t$1,1\n\t"
		".set\tnoreorder\n\t"
		"mtc0\t$1,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		".set\tpop\n\t"
		: /* no outputs */
		: /* no inputs */
		: "$1", "memory");
}

#define __save_flags(x)							\
__asm__ __volatile__(							\
	".set\tpush\n\t"						\
	".set\treorder\n\t"						\
	"mfc0\t%0,$12\n\t"						\
	".set\tpop\n\t"							\
	: "=r" (x))

#define __save_and_cli(x)						\
__asm__ __volatile__(							\
	".set\tpush\n\t"						\
	".set\treorder\n\t"						\
	".set\tnoat\n\t"						\
	"mfc0\t%0,$12\n\t"						\
	"ori\t$1,%0,1\n\t"						\
	"xori\t$1,1\n\t"						\
	".set\tnoreorder\n\t"						\
	"mtc0\t$1,$12\n\t"						\
	"nop\n\t"							\
	"nop\n\t"							\
	"nop\n\t"							\
	".set\tpop\n\t"							\
	: "=r" (x)							\
	: /* no inputs */						\
	: "$1", "memory")

#define __restore_flags(flags)						\
do {									\
	unsigned long __tmp1;						\
									\
	__asm__ __volatile__(						\
		".set\tnoreorder\t\t\t# __restore_flags\n\t"		\
		".set\tnoat\n\t"					\
		"mfc0\t$1, $12\n\t"					\
		"andi\t%0, 1\n\t"					\
		"ori\t$1, 1\n\t"					\
		"xori\t$1, 1\n\t"					\
		"or\t%0, $1\n\t"					\
		"mtc0\t%0, $12\n\t"					\
		"nop\n\t"						\
		"nop\n\t"						\
		"nop\n\t"						\
		".set\tat\n\t"						\
		".set\treorder"						\
		: "=r" (__tmp1)						\
		: "0" (flags)						\
		: "$1", "memory");					\
} while(0)

/*
 * Non-SMP versions ...
 */
#define sti() __sti()
#define cli() __cli()
#define save_flags(x) __save_flags(x)
#define save_and_cli(x) __save_and_cli(x)
#define restore_flags(x) __restore_flags(x)


#endif /* _ASM_SYSTEM_H */
