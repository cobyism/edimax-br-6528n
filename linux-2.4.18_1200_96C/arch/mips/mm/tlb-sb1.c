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
#include <asm/mmu_context.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>

extern char except_vec0_r4000[];

/* Dump the current entry* and pagemask registers */
static inline void dump_cur_tlb_regs(void)
{
	unsigned int entryhihi, entryhilo, entrylo0hi, entrylo0lo, entrylo1hi;
	unsigned int entrylo1lo, pagemask;

	__asm__ __volatile__ (
		".set push             \n"
		".set noreorder        \n"
		".set mips64           \n"
		".set noat             \n"
		"     tlbr             \n"
		"     dmfc0  $1, $10   \n"
		"     dsrl32 %0, $1, 0 \n"
		"     sra    %1, $1, 0 \n"
		"     dmfc0  $1, $2    \n"
		"     dsrl32 %2, $1, 0 \n"
		"     sra    %3, $1, 0 \n"
		"     dmfc0  $1, $3    \n"
		"     dsrl32 %4, $1, 0 \n"
		"     sra    %5, $1, 0 \n"
		"     mfc0   %6, $5    \n"
		".set pop              \n"
		: "=r" (entryhihi),
		"=r" (entryhilo),
		"=r" (entrylo0hi),
		"=r" (entrylo0lo),
		"=r" (entrylo1hi),
		"=r" (entrylo1lo),
		"=r" (pagemask));
	printk("%08X%08X %08X%08X %08X%08X %08X",
	       entryhihi, entryhilo,
	       entrylo0hi, entrylo0lo,
	       entrylo1hi, entrylo1lo,
	       pagemask);
}

void sb1_dump_tlb(void)
{
	int entry;
	printk("Current TLB registers state:\n"
	       "      EntryHi       EntryLo0          EntryLo1     PageMask  Index\n"
	       "--------------------------------------------------------------------\n");
	dump_cur_tlb_regs();
	printk(" %08X\n", read_32bit_cp0_register(CP0_INDEX));
	printk("\n\nFull TLB Dump:"
	       "Idx      EntryHi       EntryLo0          EntryLo1     PageMask\n"
	       "--------------------------------------------------------------\n");
	for (entry = 0; entry < mips_cpu.tlbsize; entry++) {
		set_index(entry);
		printk("\n%02i ", entry);
		__asm__ __volatile__ (
			".set push             \n"
			".set mips64           \n"
			"     tlbr             \n"
			".set pop              \n");
		dump_cur_tlb_regs();
	}
	printk("\n");
}

void local_flush_tlb_all(void)
{
	unsigned long flags;
	unsigned long old_ctx;
	int entry;

	__save_and_cli(flags);
	/* Save old context and create impossible VPN2 value */
	old_ctx = (get_entryhi() & 0xff);
	set_entrylo0(0);
	set_entrylo1(0);
	for (entry = 0; entry < mips_cpu.tlbsize; entry++) {
		set_entryhi(KSEG0 + (PAGE_SIZE << 1) * entry);
		set_index(entry);
		tlb_write_indexed();
	}
	set_entryhi(old_ctx);
	__restore_flags(flags);	
}


/*
 * Use a bogus region of memory (starting at 0) to sanitize the TLB's.  
 * Use increments of the maximum page size (16MB), and check for duplicate
 * entries before doing a given write.  Then, when we're safe from collisions
 * with the firmware, go back and give all the entries invalid addresses with
 * the normal flush routine.
 */
void sb1_sanitize_tlb(void)
{
	int entry;
	long addr = 0;

	long inc = 1<<24;  /* 16MB */
	/* Save old context and create impossible VPN2 value */
	set_entrylo0(0);
	set_entrylo1(0);
	for (entry = 0; entry < mips_cpu.tlbsize; entry++) {
		do {
			addr += inc;
			set_entryhi(addr);
			tlb_probe();
		} while ((int)(get_index()) >= 0);
		set_index(entry);
		tlb_write_indexed();
	}
	/* Now that we know we're safe from collisions, we can safely flush
	   the TLB with the "normal" routine. */
	local_flush_tlb_all();
}


void local_flush_tlb_range(struct mm_struct *mm, unsigned long start,
                      unsigned long end)
{
	unsigned long flags;
	int cpu;

	__save_and_cli(flags);
	cpu = smp_processor_id();
	if(CPU_CONTEXT(cpu, mm) != 0) {
		int size;
		size = (end - start + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
		size = (size + 1) >> 1;
		if(size <= (mips_cpu.tlbsize/2)) {
			int oldpid = (get_entryhi() & 0xff);
			int newpid = (CPU_CONTEXT(cpu, mm) & 0xff);

			start &= (PAGE_MASK << 1);
			end += ((PAGE_SIZE << 1) - 1);
			end &= (PAGE_MASK << 1);
			while(start < end) {
				int idx;

				set_entryhi(start | newpid);
				start += (PAGE_SIZE << 1);
				tlb_probe();
				idx = get_index();
				set_entrylo0(0);
				set_entrylo1(0);
				set_entryhi(KSEG0 + (idx << (PAGE_SHIFT+1)));
				if(idx < 0)
					continue;
				tlb_write_indexed();
			}
			set_entryhi(oldpid);
		} else {
			get_new_mmu_context(mm, cpu);
			if (mm == current->active_mm)
				set_entryhi(CPU_CONTEXT(cpu, mm) & 0xff);
		}
	}
	__restore_flags(flags);
}

void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long page)
{
	unsigned long flags;

#ifdef CONFIG_SMP
	/*
	 * This variable is eliminated from CPU_CONTEXT() if SMP isn't defined,
	 * so conditional it to get rid of silly "unused variable" compiler
	 * complaints
	 */
	int cpu = smp_processor_id();
#endif

	__save_and_cli(flags);
	if (CPU_CONTEXT(cpu, vma->vm_mm) != 0) {
		int oldpid, newpid, idx;
#ifdef DEBUG_TLB
		printk("[tlbpage<%d,%08lx>]", CPU_CONTEXT(cpu, vma->vm_mm), page);
#endif
		newpid = (CPU_CONTEXT(cpu, vma->vm_mm) & 0xff);
		page &= (PAGE_MASK << 1);
		oldpid = (get_entryhi() & 0xff);
		set_entryhi	(page | newpid);
		tlb_probe();
		idx = get_index();
		set_entrylo0(0);
		set_entrylo1(0);
		if(idx < 0)
			goto finish;
		/* Make sure all entries differ. */  
		set_entryhi(KSEG0+(idx<<(PAGE_SHIFT+1)));
		tlb_write_indexed();
	finish:
		set_entryhi(oldpid);
	}
	__restore_flags(flags);
}


/* All entries common to a mm share an asid.  To effectively flush
   these entries, we just bump the asid. */
void local_flush_tlb_mm(struct mm_struct *mm)
{
	unsigned long flags;
	int cpu;
	__save_and_cli(flags);
	cpu = smp_processor_id();
	if (CPU_CONTEXT(cpu, mm) != 0) {
		get_new_mmu_context(mm, smp_processor_id());
		if (mm == current->active_mm) {
			set_entryhi(CPU_CONTEXT(cpu, mm) & 0xff);
		}
	}
	__restore_flags(flags);
}

/* Stolen from mips32 routines */

void update_mmu_cache(struct vm_area_struct *vma, unsigned long address,
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

	__save_and_cli(flags);


	pid = get_entryhi() & 0xff;

#ifdef DEBUG_TLB
	if((pid != (CPU_CONTEXT(cpu, vma->vm_mm) & 0xff)) || (CPU_CONTEXT(cpu, vma->vm_mm) == 0)) {
		printk("update_mmu_cache: Wheee, bogus tlbpid mmpid=%d tlbpid=%d\n",
		       (int) (CPU_CONTEXT(cpu, vma->vm_mm) & 0xff), pid);
	}
#endif

	address &= (PAGE_MASK << 1);
	set_entryhi(address | (pid));
	pgdp = pgd_offset(vma->vm_mm, address);
	tlb_probe();
	pmdp = pmd_offset(pgdp, address);
	idx = get_index();
	ptep = pte_offset(pmdp, address);
	set_entrylo0(pte_val(*ptep++) >> 6);
	set_entrylo1(pte_val(*ptep) >> 6);
	set_entryhi(address | (pid));
	if(idx < 0) {
		tlb_write_random();
	} else {
		tlb_write_indexed();
	}
	set_entryhi(pid);
	__restore_flags(flags);
}

/*
 * This is called from loadmmu.c.  We have to set up all the
 * memory management function pointers, as well as initialize
 * the caches and tlbs
 */
void sb1_tlb_init(void)
{
	u32 config1;

	config1 = read_mips32_cp0_config1();
	mips_cpu.tlbsize = ((config1 >> 25) & 0x3f) + 1;

	/*
	 * We don't know what state the firmware left the TLB's in, so this is
	 * the ultra-conservative way to flush the TLB's and avoid machine
	 * check exceptions due to duplicate TLB entries 
	 */
	sb1_sanitize_tlb();

	memcpy((void *)KSEG0, except_vec0_r4000, 0x80);
	flush_icache_range(KSEG0, KSEG0 + 0x80);
}
