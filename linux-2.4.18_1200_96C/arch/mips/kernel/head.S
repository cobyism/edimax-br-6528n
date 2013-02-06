/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995 Waldorf Electronics
 * Written by Ralf Baechle and Andreas Busse
 * Copyright (C) 1995 - 1999 Ralf Baechle
 * Copyright (C) 1996 Paul M. Antoine
 * Modified for DECStation and hence R3000 support by Paul M. Antoine
 * Further modifications by David S. Miller and Harald Koerfgen
 * Copyright (C) 1999 Silicon Graphics, Inc.
 *
 * Kevin Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/current.h>
#include <asm/offset.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/regdef.h>
#include <asm/cachectl.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

		__INIT

		.text
		/*
		 * Reserved space for exception handlers.
		 * Necessary for machines which link their kernels at KSEG0.
		 */
#if defined(CONFIG_RTL8197B_PANA) || defined(CONFIG_RTL_8198_NFBI_BOARD)        
		.fill	0x8080
#else		 
		.fill	0x280
#endif		
		/*
		 * Kernel entry point
		 */
		NESTED(kernel_entry, 16, sp)
		.set	noreorder
		/* The following two symbols are used for kernel profiling. */
		EXPORT(stext)
		EXPORT(_stext)
		
#ifdef CONFIG_RTL_EB8186
#ifdef CONFIG_NAT_SPEEDUP
        /*--- initialize and start COP3*/
        mfc0    $8,$12
        nop
        or      $8,0x80000000
        mtc0    $8,$12
        nop
	nop
// kenny: DMEM start
	la   a0, __dmem_start
	nop
	and a0, 0xfffffff // convert to phy addr
	nop
	mtc3 a0, $4		// register 4 = DRAM base
	nop
	nop
	addiu a1, a0, 0xfff	// a0 offset 4k
	nop
	mtc3 a1, $5		// register 1 = DRAM top
	nop
	nop

// kenny: DMEM end
#if 1
	la   a0, __nat_speedup_start
	nop
	and a0, 0xfffffff // convert to phy addr
#else
	la a0, 0x18e000
#endif
	nop
	mtc3 a0, $0		// register 0 = IRAM base
	nop
	nop
#if 1
	addiu a1, a0, 0xfff	// a0 offset 4k
#else
	la a1, 0x18efff
#endif
	nop
	mtc3 a1, $1		// register 1 = IRAM top
	nop
	nop
	/*--- fill IRAM */
	mfc0	a0, $20
	nop
	or	a0, 0x10	// IMEMFill 0->1
	nop
	mtc0	a0, $20
	nop
	nop
#endif
#endif // CONFIG_RTL_EB8186

		/*
		 * Stack for kernel and init, current variable
		 */
		la	$28, init_task_union
		addiu	t0, $28, KERNEL_STACK_SIZE-32
		subu	sp, t0, 4*SZREG

		sw	t0, kernelsp
		/* The firmware/bootloader passes argc/argp/envp
		 * to us as arguments.  But clear bss first because
		 * the romvec and other important info is stored there
		 * by prom_init().
		 */
		la	t0, _edata
		sw	zero, (t0)
		la	t1, (_end - 4)
1:
		addiu	t0, 4
		bne	t0, t1, 1b
		 sw	zero, (t0)

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
/* reset CPU status */
	move s0, zero	
	mtc0 s0, $12	
	nop	
	nop
	
	jal		_rom_flush_cache
	nop

	/*--- flush the write buffer*/
	jal		_rom_wbflush
	nop		 
#endif		  
		 
		jal	init_arch
		 nop
		END(kernel_entry)

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8198)
	.text
	LEAF(_rom_flush_cache)
	.set noreorder
	/*--- transition to kseg1 from undetermined kernel segment*/
	la	$9,2f
	or	$9,0xa0000000
	jr	$9

	.set	noreorder

2:	
	#--- initialize and start COP3
	mfc0	$8,$12
	nop
	nop
	or		$8,0x80000000
	mtc0	$8,$12
	nop
	nop
	
	#--- invalidate the IRAM with a 0->1 transition
	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000020 # IRAM Off
	mtc0	$8, $20
	nop
	nop
	
	#--- invalidate the icache and dcache with a 0->1 transition
	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000202 # Invalid ICACHE and DCACHE
	mtc0	$8, $20
	nop
	nop

	#--- load iram base and top
	la		$8,__iram
	la		$9,0x0fffc000
	and		$8,$8,$9
	mtc3	$8,$0								# IW bas
	nop
	nop
	#ifdef CONFIG_RTL8198
				addiu   $8,$8,0xfff
        mtc3    $8,$1                                                           # IW top
        nop
        nop

        #--- Refill the IRAM with a 0->1 transition
        mtc0    $0, $20 # CCTL
        nop
        nop
        li              $8,0x00000010 # IRAM Fill
        mtc0    $8, $20
        nop
        nop
        #--- load iram base1 and top1
        la              $8,__iram
        add             $8,$8,0x1000
        la              $9,0x0fffc000
        and             $8,$8,$9
        mtc3    $8,$2                                                           # IW bas 1
        nop
        nop
        addiu   $8,$8,0xfff
        mtc3    $8,$3                                                           # IW top 1
        nop
        nop

        #--- Refill the IRAM with a 0->1 transition
        mtc0    $0, $20,1 # CCTL
        nop
        nop
        li              $8,0x00000010 # IRAM Fill
        mtc0    $8, $20,1
        nop
        nop
         #--- load dram base and top
        la              $8,__dram_start
        la              $9,__dram_end
        beq             $8,$9,skip_dramInit
        nop
        la              $9,0x0fffe000
        and             $8,$8,$9
        mtc3    $8,$4                                                           # DW bas
        nop
        nop
        addiu   $8,$8,0xfff
        mtc3    $8,$5                                                           # DW top
        nop
        nop
        #la              $8,__dram_start
        #la              $9,__dram_end
        #beq             $8,$9,skip_dramInit
        #nop
        #la              $9,0x0fffe000
        add             $8,$8,1
        #and             $8,$8,$9
        mtc3    $8,$6                                                           # DW bas 1
        nop
        nop
        addiu   $8,$8,0xfff
        mtc3    $8,$7                                                           # DW top 1
        nop
        nop
	#else
	addiu	$8,$8,0x3fff
	mtc3	$8,$1								# IW top
	nop
	nop

	#--- Refill the IRAM with a 0->1 transition
	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000010 # IRAM Fill
	mtc0	$8, $20
	nop
	nop

	#--- load dram base and top
	la		$8,__dram_start
	la		$9,__dram_end
	beq		$8,$9,skip_dramInit
	la		$9,0x0fffe000
	and		$8,$8,$9
	mtc3	$8,$4								# DW bas
	nop
	nop
	addiu	$8,$8,0x1fff
	mtc3	$8,$5								# DW top
	nop
	nop
	#endif
skip_dramInit:
	#--- enable icache and dcache
	mtc0	$0, $20	# CCTL
	nop
	nop

	.set	reorder
	j		$31


	END(_rom_flush_cache)

	LEAF(_rom_wbflush)
_rom_wbflush:
	li		$8, 0xa0000000				#/* load uncached address	*/
	lw		$8, 0($8)					#/* read in order to flush 	*/
	j		$31							#/* return to caller		*/
	END(_rom_wbflush)
#endif /* CONFIG_RTL865X */

#ifdef CONFIG_SMP

/*
 * SMP slave cpus entry point.  Board specific code for bootstrap calls this
 * function after setting up the stack and gp registers. 
 */
		LEAF(smp_bootstrap)
		.set push
		.set noreorder
		mtc0	zero, CP0_WIRED
		CLI
		mfc0	t0, CP0_STATUS
		li	t1, ~(ST0_CU1|ST0_CU2|ST0_CU3|ST0_KX|ST0_SX)
		and	t0, t1
		or	t0, (ST0_CU0|ST0_FR);
		jal	start_secondary
		mtc0	t0, CP0_STATUS
		.set pop
		END(smp_bootstrap)
#endif

		__FINIT

		/*
		 * This buffer is reserved for the use of the EJTAG debug 
		 * handler.
		 */
		.data
		EXPORT(ejtag_debug_buffer)
		.fill	4
	
		.comm	kernelsp,    NR_CPUS * 8, 8
		.comm	pgd_current, NR_CPUS * 8, 8

#undef PAGE_SIZE
#define PAGE_SIZE	0x1000

	.macro	page name, order=0
	.globl	\name
\name:	.size	\name, (PAGE_SIZE << \order)
	.org	. + (PAGE_SIZE << \order)
	.type	\name, @object
	.endm

	.data
	.align	12

	page	swapper_pg_dir, PGD_ORDER
	page	empty_bad_page, 0
	page	empty_bad_page_table, 0
	page	invalid_pte_table, 0
