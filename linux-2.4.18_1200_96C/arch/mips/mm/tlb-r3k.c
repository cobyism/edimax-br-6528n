/*
 * r2300.c: R2000 and R3000 specific mmu/cache code.
 *
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 *
 * with a lot of changes to make this thing work for R3000s
 * Tx39XX R4k style caches added. HK
 * Copyright (C) 1998, 1999, 2000 Harald Koerfgen
 * Copyright (C) 1998 Gleb Raiko & Vladimir Roganov
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/mmu_context.h>
#include <asm/system.h>
#include <asm/isadep.h>
#include <asm/io.h>
#include <asm/wbflush.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>

extern char except_vec0_r2300;

#undef DEBUG_TLB

int r3k_have_wired_reg = 0;	/* should be in mips_cpu? */

/* TLB operations. */
void local_flush_tlb_all(void)
{
	unsigned long flags;
	unsigned long old_ctx;
	int entry;

#ifdef DEBUG_TLB
	printk("[tlball]");
#endif

	save_and_cli(flags);
	old_ctx = (get_entryhi() & 0xfc0);
	write_32bit_cp0_register(CP0_ENTRYLO0, 0);
#ifdef CONFIG_CPU_TX39XX
	entry = r3k_have_wired_reg ? get_wired() : 8;
#else
	//entry = 8;
	entry = 4;
#endif
	for (; entry < mips_cpu.tlbsize; entry++) {
		write_32bit_cp0_register(CP0_INDEX, entry << 8);
		write_32bit_cp0_register(CP0_ENTRYHI, ((entry | 0x80000) << 12));
		__asm__ __volatile__("tlbwi");
	}
	set_entryhi(old_ctx);
	restore_flags(flags);
}

void local_flush_tlb_mm(struct mm_struct *mm)
{
	if (mm->context != 0) {
		unsigned long flags;

#ifdef DEBUG_TLB
		printk("[tlbmm<%lu>]", (unsigned long) mm->context);
#endif
		save_and_cli(flags);
		get_new_mmu_context(mm, smp_processor_id());
		if (mm == current->active_mm)
			set_entryhi(mm->context & 0xfc0);
		restore_flags(flags);
	}
}

void local_flush_tlb_range(struct mm_struct *mm, unsigned long start,
                     unsigned long end)
{
	if (mm->context != 0) {
		unsigned long flags;
		int size;

#ifdef DEBUG_TLB
		printk("[tlbrange<%lu,0x%08lx,0x%08lx>]",
			(mm->context & 0xfc0), start, end);
#endif
		save_and_cli(flags);
		size = (end - start + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
		if (size <= mips_cpu.tlbsize) {
			int oldpid = (get_entryhi() & 0xfc0);
			int newpid = (mm->context & 0xfc0);

			start &= PAGE_MASK;
			end += (PAGE_SIZE - 1);
			end &= PAGE_MASK;
			while (start < end) {
				int idx;

				set_entryhi(start | newpid);
				start += PAGE_SIZE;
				tlb_probe();
				idx = get_index();
				set_entrylo0(0);
				set_entryhi(KSEG0);
				if (idx < 0)
					continue;
				tlb_write_indexed();
			}
			set_entryhi(oldpid);
		} else {
			get_new_mmu_context(mm, smp_processor_id());
			if (mm == current->active_mm)
				set_entryhi(mm->context & 0xfc0);
		}
		restore_flags(flags);
	}
}

void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long page)
{
	if (!vma || vma->vm_mm->context != 0) {
		unsigned long flags;
		int oldpid, newpid, idx;

#ifdef DEBUG_TLB
		printk("[tlbpage<%lu,0x%08lx>]", vma->vm_mm->context, page);
#endif
		newpid = (vma->vm_mm->context & 0xfc0);
		page &= PAGE_MASK;
		save_and_cli(flags);
		oldpid = (get_entryhi() & 0xfc0);
		set_entryhi(page | newpid);
		tlb_probe();
		idx = get_index();
		set_entrylo0(0);
		set_entryhi(KSEG0);
		if (idx < 0)
			goto finish;
		tlb_write_indexed();

finish:
		set_entryhi(oldpid);
		restore_flags(flags);
	}
}

void update_mmu_cache(struct vm_area_struct * vma, unsigned long address,
                      pte_t pte)
{
	unsigned long flags;
	pgd_t *pgdp;
	pmd_t *pmdp;
	pte_t *ptep;
	int idx, pid;

	/*
	 * Handle debugger faulting in for debugee.
	 */
	if (current->active_mm != vma->vm_mm)
		return;

	pid = get_entryhi() & 0xfc0;

#ifdef DEBUG_TLB
	if ((pid != (vma->vm_mm->context & 0xfc0)) || (vma->vm_mm->context == 0)) {
		printk("update_mmu_cache: Wheee, bogus tlbpid mmpid=%lu tlbpid=%d\n",
		       (vma->vm_mm->context & 0xfc0), pid);
	}
#endif

	save_and_cli(flags);
	address &= PAGE_MASK;
	set_entryhi(address | (pid));
	pgdp = pgd_offset(vma->vm_mm, address);
	tlb_probe();
	pmdp = pmd_offset(pgdp, address);
	idx = get_index();
	ptep = pte_offset(pmdp, address);
	set_entrylo0(pte_val(*ptep));
	set_entryhi(address | (pid));
	if (idx < 0) {
		tlb_write_random();
#if 0
		printk("[MISS]");
#endif
	} else {
		tlb_write_indexed();
#if 0
		printk("[HIT]");
#endif
	}
	set_entryhi(pid);
	restore_flags(flags);
}

void __init add_wired_entry(unsigned long entrylo0, unsigned long entrylo1,
                     unsigned long entryhi, unsigned long pagemask)
{
	unsigned long flags;
	unsigned long old_ctx;
	static unsigned long wired = 0;
	
#ifdef CONFIG_CPU_TX39XX
	if (r3k_have_wired_reg) {
		unsigned long old_pagemask;
		unsigned long w;
	
#ifdef DEBUG_TLB
		printk("[tlbwired]");
		printk("ently lo0 %8x, hi %8x\n, pagemask %8x\n",
		       entrylo0, entryhi, pagemask);
#endif
		save_and_cli(flags);
		/* Save old context and create impossible VPN2 value */
		old_ctx = (get_entryhi() & 0xff);
		old_pagemask = get_pagemask();
		w = get_wired();
		set_wired (w + 1);
		if (get_wired() != w + 1) {
			printk("[tlbwired] No WIRED reg?\n");
			return;
		}
		set_index (w << 8);
		set_pagemask (pagemask);
		set_entryhi(entryhi);
		set_entrylo0(entrylo0);
		tlb_write_indexed();
    
		set_entryhi(old_ctx);
		set_pagemask (old_pagemask);
		local_flush_tlb_all();
		restore_flags(flags);
		return;
	}
#endif

	if (wired < 8) {
		__save_and_cli(flags);
		old_ctx = get_entryhi() & 0xfc0;
		set_entrylo0(entrylo0);
		set_entryhi(entryhi);
		set_index(wired);
		wired++;
		tlb_write_indexed();
		set_entryhi(old_ctx);
	        local_flush_tlb_all();    
		__restore_flags(flags);
	}
}

void __init r3k_tlb_init(void)
{
	local_flush_tlb_all();
	memcpy((void *)KSEG0, &except_vec0_r2300, 0x80);
	flush_icache_range(KSEG0, KSEG0 + 0x80);
}
