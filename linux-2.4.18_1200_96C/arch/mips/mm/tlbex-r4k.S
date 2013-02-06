/*
 * TLB exception handling code for r4k.
 *
 * Copyright (C) 1994, 1995, 1996 by Ralf Baechle and Andreas Busse
 *
 * Multi-cpu abstraction and reworking:
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 *
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/init.h>

#include <asm/asm.h>
#include <asm/current.h>
#include <asm/offset.h>
#include <asm/cachectl.h>
#include <asm/fpregdef.h>
#include <asm/mipsregs.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

#define TLB_OPTIMIZE /* If you are paranoid, disable this. */

#ifdef CONFIG_64BIT_PHYS_ADDR
#define PTE_L		ld
#define PTE_S		sd
#define PTE_SRL		dsrl
#define P_MTC0		dmtc0
#define PTE_SIZE	8
#define PTEP_INDX_MSK	0xff0
#define PTE_INDX_MSK	0xff8
#define PTE_INDX_SHIFT	9
#else
#define PTE_L		lw
#define PTE_S		sw
#define PTE_SRL		srl
#define P_MTC0		mtc0
#define PTE_SIZE	4
#define PTEP_INDX_MSK	0xff8
#define PTE_INDX_MSK	0xffc
#define PTE_INDX_SHIFT	10
#endif

	__INIT

#ifdef CONFIG_64BIT_PHYS_ADDR
#define GET_PTE_OFF(reg)
#elif CONFIG_CPU_VR41XX
#define GET_PTE_OFF(reg)	srl	reg, reg, 3
#else
#define GET_PTE_OFF(reg)	srl	reg, reg, 1
#endif

/*	
 * These handlers much be written in a relocatable manner
 * because based upon the cpu type an arbitrary one of the
 * following pieces of code will be copied to the KSEG0
 * vector location.
 */
	/* TLB refill, EXL == 0, R4xx0, non-R4600 version */
	.set	noreorder
	.set	noat
	LEAF(except_vec0_r4000)
	.set	mips3
#ifdef CONFIG_SMP
	mfc0	k1, CP0_CONTEXT
	la	k0, pgd_current
	srl	k1, 23
	sll	k1, 2				# log2(sizeof(pgd_t)
	addu	k1, k0, k1
	lw	k1, (k1)
#else 
	lw	k1, pgd_current			# get pgd pointer
#endif	
	mfc0	k0, CP0_BADVADDR		# Get faulting address
	srl	k0, k0, PGDIR_SHIFT		# get pgd only bits

	sll	k0, k0, 2
	addu	k1, k1, k0			# add in pgd offset
	mfc0	k0, CP0_CONTEXT			# get context reg
	lw	k1, (k1)
	GET_PTE_OFF(k0)				# get pte offset
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0			# add in offset
	PTE_L	k0, 0(k1)			# get even pte
	PTE_L	k1, PTE_SIZE(k1)		# get odd pte
	PTE_SRL	k0, k0, 6			# convert to entrylo0
	P_MTC0	k0, CP0_ENTRYLO0		# load it
	PTE_SRL	k1, k1, 6			# convert to entrylo1
	P_MTC0	k1, CP0_ENTRYLO1		# load it
	b	1f
	tlbwr					# write random tlb entry
1:
	nop
	eret					# return from trap
	END(except_vec0_r4000)

	/* TLB refill, EXL == 0, R4600 version */
	LEAF(except_vec0_r4600)
	.set	mips3
	mfc0	k0, CP0_BADVADDR
	srl	k0, k0, PGDIR_SHIFT
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0
	mfc0	k0, CP0_CONTEXT
	lw	k1, (k1)
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0
	PTE_L	k0, 0(k1)
	PTE_L	k1, PTE_SIZE(k1)
	PTE_SRL	k0, k0, 6
	P_MTC0	k0, CP0_ENTRYLO0
	PTE_SRL	k1, k1, 6
	P_MTC0	k1, CP0_ENTRYLO1
	nop
	tlbwr
	nop
	eret
	END(except_vec0_r4600)

	/* TLB refill, EXL == 0, R52x0 "Nevada" version */
        /*
         * This version has a bug workaround for the Nevada.  It seems
         * as if under certain circumstances the move from cp0_context
         * might produce a bogus result when the mfc0 instruction and
         * it's consumer are in a different cacheline or a load instruction,
         * probably any memory reference, is between them.  This is
         * potencially slower than the R4000 version, so we use this
         * special version.
         */
	.set	noreorder
	.set	noat
	LEAF(except_vec0_nevada)
	.set	mips3
	mfc0	k0, CP0_BADVADDR		# Get faulting address
	srl	k0, k0, PGDIR_SHIFT		# get pgd only bits
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0			# add in pgd offset
	lw	k1, (k1)
	mfc0	k0, CP0_CONTEXT			# get context reg
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1			# get pte offset
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0			# add in offset
	PTE_L	k0, 0(k1)			# get even pte
	PTE_L	k1, PTE_SIZE(k1)		# get odd pte
	PTE_SRL	k0, k0, 6			# convert to entrylo0
	P_MTC0	k0, CP0_ENTRYLO0		# load it
	PTE_SRL	k1, k1, 6			# convert to entrylo1
	P_MTC0	k1, CP0_ENTRYLO1		# load it
	nop					# QED specified nops
	nop
	tlbwr					# write random tlb entry
	nop					# traditional nop
	eret					# return from trap
	END(except_vec0_nevada)

	/* TLB refill, EXL == 0, R4[40]00/R5000 badvaddr hwbug version */
	LEAF(except_vec0_r45k_bvahwbug)
	.set	mips3
	mfc0	k0, CP0_BADVADDR
	srl	k0, k0, PGDIR_SHIFT
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0
	mfc0	k0, CP0_CONTEXT
	lw	k1, (k1)
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0
	PTE_L	k0, 0(k1)
	PTE_L	k1, PTE_SIZE(k1)
	nop				/* XXX */
	tlbp
	PTE_SRL	k0, k0, 6
	P_MTC0	k0, CP0_ENTRYLO0
	PTE_SRL	k1, k1, 6
	mfc0	k0, CP0_INDEX
	P_MTC0	k1, CP0_ENTRYLO1
	bltzl	k0, 1f
	tlbwr
1:
	nop
	eret
	END(except_vec0_r45k_bvahwbug)

#ifdef CONFIG_SMP
	/* TLB refill, EXL == 0, R4000 MP badvaddr hwbug version */
	LEAF(except_vec0_r4k_mphwbug)
	.set	mips3
	mfc0	k0, CP0_BADVADDR
	srl	k0, k0, PGDIR_SHIFT
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0
	mfc0	k0, CP0_CONTEXT
	lw	k1, (k1)
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0
	PTE_L	k0, 0(k1)
	PTE_L	k1, PTE_SIZE(k1)
	nop				/* XXX */
	tlbp
	PTE_SRL	k0, k0, 6
	P_MTC0	k0, CP0_ENTRYLO0
	PTE_SRL	k1, k1, 6
	mfc0	k0, CP0_INDEX
	P_MTC0	k1, CP0_ENTRYLO1
	bltzl	k0, 1f
	tlbwr
1:
	nop
	eret
	END(except_vec0_r4k_mphwbug)
#endif

	/* TLB refill, EXL == 0, R4000 UP 250MHZ entrylo[01] hwbug version */
	LEAF(except_vec0_r4k_250MHZhwbug)
	.set	mips3
	mfc0	k0, CP0_BADVADDR
	srl	k0, k0, PGDIR_SHIFT
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0
	mfc0	k0, CP0_CONTEXT
	lw	k1, (k1)
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0
	PTE_L	k0, 0(k1)
	PTE_L	k1, PTE_SIZE(k1)
	PTE_SRL	k0, k0, 6
	P_MTC0	zero, CP0_ENTRYLO0
	P_MTC0	k0, CP0_ENTRYLO0
	PTE_SRL	k1, k1, 6
	P_MTC0	zero, CP0_ENTRYLO1
	P_MTC0	k1, CP0_ENTRYLO1
	b	1f
	tlbwr
1:
	nop
	eret
	END(except_vec0_r4k_250MHZhwbug)

#ifdef CONFIG_SMP
	/* TLB refill, EXL == 0, R4000 MP 250MHZ entrylo[01]+badvaddr bug version */
	LEAF(except_vec0_r4k_MP250MHZhwbug)
	.set	mips3
	mfc0	k0, CP0_BADVADDR
	srl	k0, k0, PGDIR_SHIFT
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0
	mfc0	k0, CP0_CONTEXT
	lw	k1, (k1)
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0
	PTE_L	k0, 0(k1)
	PTE_L	k1, PTE_SIZE(k1)
	nop				/* XXX */
	tlbp
	PTE_SRL	k0, k0, 6
	P_MTC0  zero, CP0_ENTRYLO0
	P_MTC0  k0, CP0_ENTRYLO0
	mfc0    k0, CP0_INDEX
	PTE_SRL	k1, k1, 6
	P_MTC0	zero, CP0_ENTRYLO1
	P_MTC0	k1, CP0_ENTRYLO1
	bltzl	k0, 1f
	tlbwr
1:
	nop
	eret
	END(except_vec0_r4k_MP250MHZhwbug)
#endif

#ifdef CONFIG_MIPS_AU1000
	/* TLB refill, EXL == 0, Au1000 version */
        /* we'll worry about smp later */
	.set	noreorder
	.set	noat
	LEAF(except_vec0_au1000)
	.set	mips3
	mfc0	k0, CP0_BADVADDR		# Get faulting address
	srl	k0, k0, PGDIR_SHIFT		# get pgd only bits
	lw	k1, pgd_current			# get pgd pointer
	sll	k0, k0, 2			# log2(sizeof(pgd_t)
	addu	k1, k1, k0			# add in pgd offset
	mfc0	k0, CP0_CONTEXT			# get context reg
	lw	k1, (k1)
#ifndef CONFIG_64BIT_PHYS_ADDR
	srl	k0, k0, 1			# get pte offset
#endif
	and	k0, k0, PTEP_INDX_MSK
	addu	k1, k1, k0			# add in offset
	j	translate_pte
	nop
	END(except_vec0_au1000)
#endif


	__FINIT

/*
 * ABUSE of CPP macros 101.
 *
 * After this macro runs, the pte faulted on is
 * in register PTE, a ptr into the table in which
 * the pte belongs is in PTR.
 */

#ifdef CONFIG_SMP
#define GET_PGD(scratch, ptr)        \
	mfc0    ptr, CP0_CONTEXT;    \
	la      scratch, pgd_current;\
	srl     ptr, 23;             \
	sll     ptr, 2;              \
	addu    ptr, scratch, ptr;   \
	lw      ptr, (ptr);          
#else
#define GET_PGD(scratch, ptr)    \
	lw	ptr, pgd_current;
#endif

#define LOAD_PTE(pte, ptr) \
	GET_PGD(pte, ptr)          \
	mfc0	pte, CP0_BADVADDR; \
	srl	pte, pte, PGDIR_SHIFT; \
	sll	pte, pte, 2; \
	addu	ptr, ptr, pte; \
	mfc0	pte, CP0_BADVADDR; \
	lw	ptr, (ptr); \
	srl	pte, pte, PTE_INDX_SHIFT; \
	and	pte, pte, PTE_INDX_MSK; \
	addu	ptr, ptr, pte; \
	PTE_L	pte, (ptr);

	/* This places the even/odd pte pair in the page
	 * table at PTR into ENTRYLO0 and ENTRYLO1 using
	 * TMP as a scratch register.
	 */
#define PTE_RELOAD(ptr, tmp) \
	ori	ptr, ptr, PTE_SIZE; \
	xori	ptr, ptr, PTE_SIZE; \
	PTE_L	tmp, PTE_SIZE(ptr); \
	PTE_L	ptr, 0(ptr); \
	PTE_SRL	tmp, tmp, 6; \
	P_MTC0	tmp, CP0_ENTRYLO1; \
	PTE_SRL	ptr, ptr, 6; \
	P_MTC0	ptr, CP0_ENTRYLO0;

#define DO_FAULT(write) \
	.set	noat; \
	SAVE_ALL; \
	mfc0	a2, CP0_BADVADDR; \
	STI; \
	.set	at; \
	move	a0, sp; \
	jal	do_page_fault; \
	 li	a1, write; \
	j	ret_from_exception; \
	 nop; \
	.set	noat;

	/* Check is PTE is present, if not then jump to LABEL.
	 * PTR points to the page table where this PTE is located,
	 * when the macro is done executing PTE will be restored
	 * with it's original value.
	 */
#define PTE_PRESENT(pte, ptr, label) \
	andi	pte, pte, (_PAGE_PRESENT | _PAGE_READ); \
	xori	pte, pte, (_PAGE_PRESENT | _PAGE_READ); \
	bnez	pte, label; \
	 PTE_L	pte, (ptr);

	/* Make PTE valid, store result in PTR. */
#define PTE_MAKEVALID(pte, ptr) \
	ori	pte, pte, (_PAGE_VALID | _PAGE_ACCESSED); \
	PTE_S	pte, (ptr);

	/* Check if PTE can be written to, if not branch to LABEL.
	 * Regardless restore PTE with value from PTR when done.
	 */
#define PTE_WRITABLE(pte, ptr, label) \
	andi	pte, pte, (_PAGE_PRESENT | _PAGE_WRITE); \
	xori	pte, pte, (_PAGE_PRESENT | _PAGE_WRITE); \
	bnez	pte, label; \
	 PTE_L	pte, (ptr);

	/* Make PTE writable, update software status bits as well,
	 * then store at PTR.
	 */
#define PTE_MAKEWRITE(pte, ptr) \
	ori	pte, pte, (_PAGE_ACCESSED | _PAGE_MODIFIED | \
			   _PAGE_VALID | _PAGE_DIRTY); \
	PTE_S	pte, (ptr);

	.set	noreorder

/*
 * From the IDT errata for the QED RM5230 (Nevada), processor revision 1.0:
 * 2. A timing hazard exists for the TLBP instruction.
 *
 *      stalling_instruction
 *      TLBP
 *
 * The JTLB is being read for the TLBP throughout the stall generated by the
 * previous instruction. This is not really correct as the stalling instruction
 * can modify the address used to access the JTLB.  The failure symptom is that
 * the TLBP instruction will use an address created for the stalling instruction
 * and not the address held in C0_ENHI and thus report the wrong results.
 *
 * The software work-around is to not allow the instruction preceding the TLBP
 * to stall - make it an NOP or some other instruction guaranteed not to stall.
 *
 * Errata 2 will not be fixed.  This errata is also on the R5000.
 *
 * As if we MIPS hackers wouldn't know how to nop pipelines happy ...
 */
#define R5K_HAZARD nop

	/*
	 * Note for many R4k variants tlb probes cannot be executed out
	 * of the instruction cache else you get bogus results.
	 */
	.align	5
	NESTED(handle_tlbl, PT_SIZE, sp)
	.set	noat
invalid_tlbl:
#ifdef TLB_OPTIMIZE
	/* Test present bit in entry. */
	LOAD_PTE(k0, k1)
	R5K_HAZARD
	tlbp
	PTE_PRESENT(k0, k1, nopage_tlbl)
	PTE_MAKEVALID(k0, k1)
	PTE_RELOAD(k1, k0)
	nop
	b	1f
	 tlbwi
1:
	nop
	.set	mips3	
	eret
	.set	mips0
#endif

nopage_tlbl:
	DO_FAULT(0)
	END(handle_tlbl)

	.align	5
	NESTED(handle_tlbs, PT_SIZE, sp)
	.set	noat
#ifdef TLB_OPTIMIZE
	.set	mips3
        li      k0,0
	LOAD_PTE(k0, k1)
	R5K_HAZARD
	tlbp				# find faulting entry
	PTE_WRITABLE(k0, k1, nopage_tlbs)
	PTE_MAKEWRITE(k0, k1)
	PTE_RELOAD(k1, k0)
	nop
	b	1f
	 tlbwi
1:
	nop
	.set	mips3
	eret
	.set	mips0
#endif

nopage_tlbs:
	DO_FAULT(1)
	END(handle_tlbs)

	.align	5
	NESTED(handle_mod, PT_SIZE, sp)
	.set	noat
#ifdef TLB_OPTIMIZE
	.set	mips3
	LOAD_PTE(k0, k1)
	R5K_HAZARD
	tlbp					# find faulting entry
	andi	k0, k0, _PAGE_WRITE
	beqz	k0, nowrite_mod
	 PTE_L	k0, (k1)

	/* Present and writable bits set, set accessed and dirty bits. */
	PTE_MAKEWRITE(k0, k1)

	/* Now reload the entry into the tlb. */
	PTE_RELOAD(k1, k0)
	nop
	b	1f
	 tlbwi
1:
	nop
	.set	mips3
	eret
	.set	mips0
#endif

nowrite_mod:
	DO_FAULT(1)
	END(handle_mod)

#ifdef CONFIG_MIPS_AU1000

#ifdef CONFIG_MIPS_PB1500
#define PSEUDO_ADDR_BASE 0x20000000
#endif

#ifdef CONFIG_MIPS_PB1000
#define PSEUDO_ADDR_BASE 0xC0000000
#endif

/*
 * On entry k0 contains the pte with the pseudo address.
 * On exit, k0 contains the "real" address, which is a
 * 36 bit physicall address.
 * This function is called only after it has been 
 * determined that the pte is a pseudo physical address.
 *
 * Destroys k0, k1, and at. It's assumed that the calling
 * function will preserve those.
 */
LEAF(get_real_pte)
	.set	mips3
	.set	at

	li	k1, 0xe0000000          # check lcd 
	bltu	k0, k1, check_pcmcia_socket_1
	nop
	# lcd pseudo access
	li	k1, 0x0fffffff
	and	k0, k0, k1              # get offset
#ifdef CONFIG_MIPS_PB1500
	lui	k1, 0x1b00
	addu	k0, k0, k1
#endif
	srl	k0, k0, 6
	lui	k1, 0xe000>>2
	or	k0, k0, k1
	j	ra
	nop
check_pcmcia_socket_1:
	li	k1, 0xD0000000
	bltu	k0, k1, pcmcia_socket_0
	nop
	# famous last words, should not happen ...
1:	
	b 1b       # fixme -- to something a little more useful
	# pcmcia socket 1 pseudo access

pcmcia_socket_0:
	# check mem access
	li	k1, 0xC8000000
	bltu	k0, k1, check_attr
	# handle pseudo memory access
	li	k1, 0x00ffffff
	and	k1, k0, k1              # get access offset
	lui	k0, 0x8000
	or	k0, k0, k1              
	# now we have the correct even pte ... bits 31:0
	srl	k0, k0, 6
	lui	k1, 0xf000>>2
	or	k0, k0, k1
	j	ra                      # done
	nop
check_attr:
	li	k1, 0xC4000000
	bltu	k0, k1, io_access
	# handle pseudo attribute access
	li	k1, 0x00ffffff
	and	k1, k0, k1              # get access offset
	lui	k0, 0x4000
	or	k0, k0, k1              
	# now we have the correct even pte ... bits 31:0
	srl	k0, k0, 6
	lui	k1, 0xf000>>2
	or	k0, k0, k1
	j	ra                      # done
	nop
io_access:
#ifdef CONFIG_MIPS_PB1500
	li	k1, 0xC0000000
	bltu	k0, k1, pci_access
#endif
	# handle pseudo io access
	li	k1, 0x00ffffff
	and	k0, k0, k1              # get access offset
	# now we have the correct even pte ... bits 31:0
	srl	k0, k0, 6
	lui	k1, 0xf000>>2
	or	k0, k0, k1
	j	ra                      # done
	nop
#ifdef CONFIG_MIPS_PB1500
pci_access:
	li	k1, 0x80000000
	bltu	k0, k1, pci_io_access
	lui	k1, 0x4000>>2
	# handle pseudo pci mem access
	srl	k0, k0, 6
	or	k0, k0, k1
	j	ra                      # done
	nop
pci_io_access:
	li	k1, 0x70000000
	bltu	k0, k1, pci_cfg_access
	lui	k1, 0x5000>>2
	# handle pseudo pci io access
	srl	k0, k0, 6
	or	k0, k0, k1
	j	ra                      # done
	nop
pci_cfg_access:
	# handle pseudo pci ext cfg access
	li	k1, 0x0fffffff
	and	k0, k0, k1              # get access offset
	srl	k0, k0, 6
	lui	k1, 0x6000>>2
	or	k0, k0, k1
	j	ra                      # done
	nop
#endif
	.set	noat
END(get_real_pte)

/*
 * On entry k1 contains pte pointer. Clobbers only k0 and k1.
 */
	LEAF(translate_pte)
	.set	mips3
	lui	k0, %hi(__saved_at)
	.set	noat
	sw	$at, %lo(__saved_at)(k0)	# save at
	.set	at
	sw	k1, %lo(__saved_pte)(k0)	# save pte pointer
	sw	ra, %lo(__saved_ra)(k0)		# save ra
	lw	k0, 0(k1)			# get even pte

	li	k1, PSEUDO_ADDR_BASE   		# check pseudo addr
	bltu	k0, k1, 1f
	nop
	bal	get_real_pte
	nop
	b	2f
	nop
1:
	srl	k0, k0, 6
2:
	mtc0	k0, CP0_ENTRYLO0		# load it

	lui	k1, %hi(__saved_pte)
	lw	k1, %lo(__saved_pte)(k1)	# recover pte pointer
	lw	k0, 4(k1)			# get odd pte

	li	k1, PSEUDO_ADDR_BASE   		# check pseudo addr
	bltu	k0, k1, 1f
	nop
	bal	get_real_pte
	nop
	b	2f
	nop

1:
	srl	k0, k0, 6			# convert to entrylo0
2:
	mtc0	k0, CP0_ENTRYLO1		# load it
	nop
	b	1f
	tlbwr					# write random tlb entry
1:
	lui	k0, %hi(__saved_at)
	.set	noat
	lw	$at, %lo(__saved_at)(k0)	# restore at
	.set	at
	lw	ra, %lo(__saved_ra)(k0)		# restore ra
	eret					# return from trap
	.set	noat
	END(translate_pte)

__saved_at:	PTR	0
__saved_pte:	PTR	0
__saved_ra:	PTR	0
#endif	
