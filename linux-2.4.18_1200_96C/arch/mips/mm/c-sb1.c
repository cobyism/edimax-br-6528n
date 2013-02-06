/*
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 * Copyright (C) 1997, 2001 Ralf Baechle (ralf@gnu.org)
 * Copyright (C) 2000, 2001 Broadcom Corporation
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

#include <linux/init.h>
#include <asm/mmu_context.h>
#include <asm/bootinfo.h>
#include <asm/cacheops.h>
#include <asm/cpu.h>
#include <asm/uaccess.h>

/* These are probed at ld_mmu time */
static unsigned int icache_size;
static unsigned int dcache_size;

static unsigned int icache_line_size;
static unsigned int dcache_line_size;

static unsigned int icache_index_mask;

static unsigned int icache_assoc;
static unsigned int dcache_assoc;

static unsigned int icache_sets;
static unsigned int dcache_sets;

void pgd_init(unsigned long page)
{
	unsigned long *p = (unsigned long *) page;
	int i;

	for (i = 0; i < USER_PTRS_PER_PGD; i+=8) {
		p[i + 0] = (unsigned long) invalid_pte_table;
		p[i + 1] = (unsigned long) invalid_pte_table;
		p[i + 2] = (unsigned long) invalid_pte_table;
		p[i + 3] = (unsigned long) invalid_pte_table;
		p[i + 4] = (unsigned long) invalid_pte_table;
		p[i + 5] = (unsigned long) invalid_pte_table;
		p[i + 6] = (unsigned long) invalid_pte_table;
		p[i + 7] = (unsigned long) invalid_pte_table;
	}
}

/*
 * The dcache is fully coherent to the system, with one
 * big caveat:  the instruction stream.  In other words,
 * if we miss in the icache, and have dirty data in the
 * L1 dcache, then we'll go out to memory (or the L2) and
 * get the not-as-recent data.
 * 
 * So the only time we have to flush the dcache is when
 * we're flushing the icache.  Since the L2 is fully
 * coherent to everything, including I/O, we never have
 * to flush it
 */

static void sb1_flush_cache_all(void)
{
}

static void local_sb1___flush_cache_all(void)
{
	/*
	 * Haven't worried too much about speed here; given that we're flushing
	 * the icache, the time to invalidate is dwarfed by the time it's going
	 * to take to refill it.  Register usage:
	 * 
	 * $1 - moving cache index
	 * $2 - set count
	 */
	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set noat                  \n"
		".set mips4                 \n"
		"     move   $1, %2         \n" /* Start at index 0 */
		"1:   cache  %3, 0($1)      \n" /* WB/Invalidate this index */
		"     addiu  %1, %1, -1     \n" /* Decrement loop count */
		"     bnez   %1, 1b         \n" /* loop test */
		"      addu   $1, $1, %0    \n" /* Next address */
		".set pop                   \n"
		:
		: "r" (dcache_line_size), "r" (dcache_sets * dcache_assoc),
		  "r" (KSEG0), "i" (Index_Writeback_Inv_D));

	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set mips2                 \n"
		"sync                       \n"
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS		/* Bug 1384 */			
		"sync                       \n"
#endif
		".set pop                   \n");

	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set noat                  \n"
		".set mips4                 \n"
		"     move   $1, %2         \n" /* Start at index 0 */
		"1:   cache  %3, 0($1)       \n" /* Invalidate this index */
		"     addiu  %1, %1, -1     \n" /* Decrement loop count */
		"     bnez   %1, 1b         \n" /* loop test */
		"      addu   $1, $1, %0    \n" /* Next address */
		".set pop                   \n"
		:
		: "r" (icache_line_size), "r" (icache_sets * icache_assoc),
		  "r" (KSEG0), "i" (Index_Invalidate_I));
}

#ifdef CONFIG_SMP
extern void sb1___flush_cache_all_ipi(void *ignored);
asm("sb1___flush_cache_all_ipi = local_sb1___flush_cache_all");

static void sb1___flush_cache_all(void)
{
	smp_call_function(sb1___flush_cache_all_ipi, 0, 1, 1);
	local_sb1___flush_cache_all();
}
#else
extern void sb1___flush_cache_all(void);
asm("sb1___flush_cache_all = local_sb1___flush_cache_all");
#endif


/*
 * When flushing a range in the icache, we have to first writeback
 * the dcache for the same range, so new ifetches will see any
 * data that was dirty in the dcache.
 *
 * The start/end arguments are expected to be Kseg addresses.
 */

static void local_sb1_flush_icache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
	unsigned long flags;
	local_irq_save(flags);
#endif

	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set noat                  \n"
		".set mips4                 \n"
		"     move   $1, %0         \n" 
		"1:                         \n"
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
		".align 3                   \n"
		"     lw     $0,   0($1)    \n" /* Bug 1370, 1368            */
		"     sync                  \n"
#endif
		"     cache  %3, 0($1)      \n" /* Hit-WB{,-inval} this address */
		"     bne    $1, %1, 1b     \n" /* loop test */
		"      addu  $1, $1, %2     \n" /* next line */
		".set pop                   \n"
		:
		: "r" (start  & ~(dcache_line_size - 1)),
		  "r" ((end - 1) & ~(dcache_line_size - 1)),
		  "r" (dcache_line_size),
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
		  "i" (Hit_Writeback_Inv_D)
#else
		  "i" (Hit_Writeback_D)
#endif
		);
	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set mips2                 \n"
		"sync                       \n"
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS		/* Bug 1384 */
		"sync                       \n"
#endif
		".set pop                   \n");
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
	local_irq_restore(flags);
#endif

	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set noat                  \n"
		".set mips4                 \n"
		"     move   $1, %0         \n" 
		".align 3                   \n"
		"1:   cache  %3, (0<<13)($1) \n" /* Index-inval this address */
		"     cache  %3, (1<<13)($1) \n" /* Index-inval this address */
		"     cache  %3, (2<<13)($1) \n" /* Index-inval this address */
		"     cache  %3, (3<<13)($1) \n" /* Index-inval this address */
		"     bne    $1, %1, 1b     \n" /* loop test */
		"      addu  $1, $1, %2     \n" /* next line */
		".set pop                   \n"
		:
		: "r" (start & ~(icache_line_size - 1)),
		  "r" ((end - 1) & ~(dcache_line_size - 1)),
		  "r" (icache_line_size),
		  "i" (Index_Invalidate_I));
}

#ifdef CONFIG_SMP
struct flush_icache_range_args {
	unsigned long start;
	unsigned long end;
};

static void sb1_flush_icache_range_ipi(void *info)
{
	struct flush_icache_range_args *args = info;

	local_sb1_flush_icache_range(args->start, args->end);
}

void sb1_flush_icache_range(unsigned long start, unsigned long end)
{
	struct flush_icache_range_args args;

	args.start = start;
	args.end = end;
	smp_call_function(sb1_flush_icache_range_ipi, &args, 1, 1);
	local_sb1_flush_icache_range(start, end);
}
#else
void sb1_flush_icache_range(unsigned long start, unsigned long end);
asm("sb1_flush_icache_range = local_sb1_flush_icache_range");
#endif

/*
 * If there's no context yet, or the page isn't executable, no icache flush
 * is needed
 */
static void sb1_flush_icache_page(struct vm_area_struct *vma,
	struct page *page)
{
	if (!(vma->vm_flags & VM_EXEC))
		return;

	/*
	 * We're not sure of the virtual address(es) involved here, so
	 * conservatively flush the entire caches on all processors
	 * (ouch).
	 */
	sb1___flush_cache_all();
}

static inline void protected_flush_icache_line(unsigned long addr)
{
	__asm__ __volatile__(
		"    .set push                \n"
		"    .set noreorder           \n"
		"    .set mips4               \n"
		"1:  cache %1, (%0)           \n"
		"2:  .set pop                 \n"
		"    .section __ex_table,\"a\"\n"
		"     .word  1b, 2b          \n"
		"     .previous"
		:
		: "r" (addr),
		  "i" (Hit_Invalidate_I));
}

static inline void protected_writeback_dcache_line(unsigned long addr)
{
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
	/* Have to be sure the TLB entry exists for the cache op, 
	   so we have to be sure that nothing happens in between the
	   lw and the cache op 
	*/
	unsigned long flags;
	local_irq_save(flags);
#endif
	__asm__ __volatile__(
		"    .set push                \n"
		"    .set noreorder           \n"
		"    .set mips4               \n"
		"1:                           \n"
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
		"     lw    $0,   (%0)        \n"
		"     sync                  \n"
#endif
		"     cache  %1, 0(%0)    \n" /* Hit-WB{-inval} this address */
		/* XXX: should be able to do this after both dcache cache
		   ops, but there's no guarantee that this will be inlined,
		   and the pass1 restriction checker can't detect syncs
		   following cache ops except in the following basic block.
		*/
		"     sync                    \n"
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS		/* Bug 1384 */
		"     sync                    \n"
#endif
		"2:  .set pop                 \n"
		"    .section __ex_table,\"a\"\n"
		"     .word  1b, 2b          \n"
		"     .previous"
		:
		: "r" (addr),
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
		  "i" (Hit_Writeback_Inv_D)
#else
		  "i" (Hit_Writeback_D)
#endif
		  );
#ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
	local_irq_restore(flags);
#endif
}

/*
 * A signal trampoline must fit into a single cacheline.
 */
static inline void local_sb1_flush_cache_sigtramp(unsigned long addr)
{
	unsigned long daddr, iaddr;

	daddr = addr & ~(dcache_line_size - 1);
	protected_writeback_dcache_line(daddr);
	protected_writeback_dcache_line(daddr + dcache_line_size);
	iaddr = addr & ~(icache_line_size - 1);
	protected_flush_icache_line(iaddr);
	protected_flush_icache_line(iaddr + icache_line_size);
}

#ifdef CONFIG_SMP

static void sb1_flush_cache_sigtramp_ipi(void *info)
{
	unsigned long iaddr = (unsigned long) info;

	iaddr = iaddr & ~(icache_line_size - 1);
	protected_flush_icache_line(iaddr);
}

static void sb1_flush_cache_sigtramp(unsigned long addr)
{
	unsigned long tmp;

	/*
	 * Flush the local dcache, then load the instruction back into a
	 * register.  That will make sure that any remote CPU also has
	 * written back it's data cache to memory.
	 */
	local_sb1_flush_cache_sigtramp(addr);
	__get_user(tmp, (unsigned long *)addr);

	smp_call_function(sb1_flush_cache_sigtramp_ipi, (void *) addr, 1, 1);
}

#else
void sb1_flush_cache_sigtramp(unsigned long addr);
asm("sb1_flush_cache_sigtramp = local_sb1_flush_cache_sigtramp");
#endif

static void sb1_flush_icache_all(void)
{
	/*
	 * Haven't worried too much about speed here; given that we're flushing
	 * the icache, the time to invalidate is dwarfed by the time it's going
	 * to take to refill it.  Register usage:
	 * 
	 * $1 - moving cache index
	 * $2 - set count
	 */
	__asm__ __volatile__ (
		".set push                  \n"
		".set noreorder             \n"
		".set noat                  \n"
		".set mips4                 \n"
		"     move   $1, %2         \n" /* Start at index 0 */
		"1:   cache  %3, 0($1)      \n" /* Invalidate this index */
		"     addiu  %1, %1, -1     \n" /* Decrement loop count */
		"     bnez   %1, 1b         \n" /* loop test */
		"      addu   $1, $1, %0    \n" /* Next address */
		".set pop                   \n"
		:
		: "r" (icache_line_size), "r" (icache_sets * icache_assoc),
		  "r" (KSEG0), "i" (Index_Invalidate_I));
}

/*
 * Anything that just flushes dcache state can be ignored, as we're always
 * coherent in dcache space.  This is just a dummy function that all the
 * nop'ed routines point to
 */

static void sb1_nop(void)
{
}

/*
 * This only needs to make sure stores done up to this
 * point are visible to other agents outside the CPU.  Given 
 * the coherent nature of the ZBbus, all that's required here is 
 * a sync to make sure the data gets out to the caches and is
 * visible to an arbitrary A Phase from an external agent 
 *   
 * Actually, I'm not even sure that's necessary; the semantics
 * of this function aren't clear.  If it's supposed to serve as
 * a memory barrier, this is needed.  If it's only meant to 
 * prevent data from being invisible to non-cpu memory accessors
 * for some indefinite period of time (e.g. in a non-coherent 
 * dcache) then this function would be a complete nop.
 */
static void sb1_flush_page_to_ram(struct page *page)
{
	__asm__ __volatile__(
		"     sync  \n"  /* Short pipe */
		:::"memory");	
}

/*
 *  Cache set values (from the mips64 spec)
 * 0 - 64
 * 1 - 128
 * 2 - 256
 * 3 - 512
 * 4 - 1024
 * 5 - 2048
 * 6 - 4096
 * 7 - Reserved
 */
  
static unsigned int decode_cache_sets(unsigned int config_field)
{
	if (config_field == 7) {
		/* JDCXXX - Find a graceful way to abort. */
		return 0;
	} 
	return (1<<(config_field + 6));
}

/*
 *  Cache line size values (from the mips64 spec)
 * 0 - No cache present.
 * 1 - 4 bytes
 * 2 - 8 bytes
 * 3 - 16 bytes
 * 4 - 32 bytes
 * 5 - 64 bytes
 * 6 - 128 bytes
 * 7 - Reserved
 */

static unsigned int decode_cache_line_size(unsigned int config_field)
{
	if (config_field == 0) {
		return 0;
	} else if (config_field == 7) {
		/* JDCXXX - Find a graceful way to abort. */
		return 0;
	} 
	return (1<<(config_field + 1));
}

/*
 * Relevant bits of the config1 register format (from the MIPS32/MIPS64 specs)
 *
 * 24:22 Icache sets per way
 * 21:19 Icache line size
 * 18:16 Icache Associativity
 * 15:13 Dcache sets per way
 * 12:10 Dcache line size
 * 9:7   Dcache Associativity
 */

static __init void probe_cache_sizes(void)
{
	u32 config1;

	config1 = read_mips32_cp0_config1();
	icache_line_size = decode_cache_line_size((config1 >> 19) & 0x7);
	dcache_line_size = decode_cache_line_size((config1 >> 10) & 0x7);
	icache_sets = decode_cache_sets((config1 >> 22) & 0x7);
	dcache_sets = decode_cache_sets((config1 >> 13) & 0x7);
	icache_assoc = ((config1 >> 16) & 0x7) + 1;
	dcache_assoc = ((config1 >> 7) & 0x7) + 1;
	icache_size = icache_line_size * icache_sets * icache_assoc;
	dcache_size = dcache_line_size * dcache_sets * dcache_assoc;
	icache_index_mask = (icache_sets - 1) * icache_line_size;
}

/*
 * This is called from loadmmu.c.  We have to set up all the
 * memory management function pointers, as well as initialize
 * the caches and tlbs
 */
void ld_mmu_sb1(void)
{
	probe_cache_sizes();

	_clear_page = sb1_clear_page;
	_copy_page = sb1_copy_page;

	_flush_cache_all = sb1_flush_cache_all;
	___flush_cache_all = sb1___flush_cache_all;
	_flush_cache_mm = (void (*)(struct mm_struct *))sb1_nop;
	_flush_cache_range = (void *) sb1_nop;
	_flush_page_to_ram = sb1_flush_page_to_ram;
	_flush_icache_page = sb1_flush_icache_page;
	_flush_icache_range = sb1_flush_icache_range;

	/* None of these are needed for the sb1 */
	_flush_cache_page = (void *) sb1_nop;

	_flush_cache_sigtramp = sb1_flush_cache_sigtramp;
	_flush_icache_all = sb1_flush_icache_all;

	change_cp0_config(CONF_CM_CMASK, CONF_CM_CACHABLE_COW);
	flush_cache_all();
}
