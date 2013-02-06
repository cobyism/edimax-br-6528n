/*
 * Copyright (C) 2001 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <linux/config.h>

#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>

	.text
	/* Special Cache Error handler for SB1 for now */
	LEAF(except_vec2_sb1)
	.set	noat
	.set	mips0
	/*
	 * This is a very bad place to be.  Our cache error detection has
	 * triggered.  If we have write-back data in the cache, we may not be
	 * able to recover.  As a first-order desperate measure, turn off KSEG0
	 * cacheing.
	 */
	.set	push
	#.set	mips64
	.set	mips4
	.set	reorder
	# look for signature of spurious CErr
	mfc0	k1, $26				# mfc0	k1, $26, 0
	# check if error was recoverable
	# XXXKW - count them
	bltz	k1, leave_cerr
	 nop

#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
	# look for signature of spurious CErr
	lui	k0, 0x4000
	bne	k0, k1, real_cerr
	.word	0x401Bd801			# mfc0	k1, $27, 1
	lui	k0, 0xffe0
	and	k1, k0, k1
	lui	k0, 0x0200
	bne	k0, k1, real_cerr
	 nop
#else
	j	real_cerr
	 nop
#endif
	# XXXKW - count spurious errors
leave_cerr:
	# clear/unlock the registers
	mtc0	zero, $26			# mtc0	zero, $26, 0
	mtc0	zero, $27			# mtc0	zero, $27, 0
	.word	0x4080d801			# mtc0	zero, $27, 1
	.word	0x4080d803			# mtc0	zero, $27, 3
	eret

real_cerr:
	mfc0	k0, CP0_CONFIG
	li	k1, ~CONF_CM_CMASK
	and	k0, k0, k1
	ori	k0, k0, CONF_CM_UNCACHED
	mtc0	k0, CP0_CONFIG
	/* Give it a few cycles to sink in... */
	sll     zero, zero, 0x1			# ssnop
	sll     zero, zero, 0x1			# ssnop
	sll     zero, zero, 0x1			# ssnop
	sll     zero, zero, 0x1			# ssnop
	sll     zero, zero, 0x1			# ssnop
	sll     zero, zero, 0x1			# ssnop

	j	sb1_cache_error
	 nop

	.set	pop
	END(except_vec2_sb1)
