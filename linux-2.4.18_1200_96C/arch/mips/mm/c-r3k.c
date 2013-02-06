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
#include <linux/module.h>	// for wlan driver module, david+2006-1-19
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

#undef DEBUG_CACHE

#ifdef CONFIG_RTL865XC
/* For Realtek RTL865XC Network platform series */
#define _ICACHE_SIZE		(16 * 1024)		/* 16K bytes */
#define _DCACHE_SIZE		(8 * 1024)		/* 8K bytes */
#define _CACHE_LINE_SIZE	4			/* 4 words */
extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);

#elif defined (CONFIG_RTL865XB)
/* For Realtek RTL865XB Network platform series */
#define _ICACHE_SIZE	(4 * 1024)			/* 4K bytes */
#define _DCACHE_SIZE	(4 * 1024)			/* 4K bytes */
#define _CACHE_LIN
#endif

/* copied from /uClinux-dist */
/*
 * 	Removing the paramter "__init" because
 * 	the function would be called by other functions
 * 	and can not be reused.
 */
unsigned long r3k_cache_size(unsigned long ca_flags)
{
#ifdef _CACHE_DISCOVERY
	unsigned long flags, status, dummy, size;
	volatile unsigned long *p;

	p = (volatile unsigned long *) KSEG0;

	flags = read_c0_status();

	/* isolate cache space */
	write_c0_status((ca_flags|flags)&~ST0_IEC);

	*p = 0xa5a55a5a;
	dummy = *p;
	status = read_c0_status();

	if (dummy != 0xa5a55a5a || (status & ST0_CM)) {
		size = 0;
	} else {
		for (size = 128; size <= 0x40000; size <<= 1)
			*(p + size) = 0;
		*p = -1;
		for (size = 128;
		     (size <= 0x40000) && (*(p + size) == 0);
		     size <<= 1)
			;
		if (size > 0x40000)
			size = 0;
	}

	write_c0_status(flags);

	return size * sizeof(*p);

#else /* _CACHE_DISCOVERY */

	unsigned long cacheSize;

	switch (ca_flags)
	{
		case ST0_ISC:
			/* D-cache size */
			cacheSize = _DCACHE_SIZE;
			break;
		case (ST0_ISC|ST0_SWC):
			/* I-cache size */
			cacheSize = _ICACHE_SIZE;
			break;
		default:
			cacheSize = 0;
	}

	return cacheSize;

#endif /* _CACHE_DISCOVERY */
}
#ifdef CONFIG_RTL8196B
__attribute__ ((section(".iram-gen")))
#endif
#ifdef GCCPATCH_1_3_X
void r3k_flush_dcache_range(unsigned long start, unsigned long end);
#endif
static void r3k_flush_icache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_RTL865XC

	/*
		Flush data cache at first in write-back platform.

		Ghhuang (2007/3/9):

		RD-Center suggest that we need to flush D-cache entries which
		might match to same address as I-cache ... when we flush
		I-cache.
		( Maybe some data is treated as data/instruction, both. )
	*/
	r3k_flush_dcache_range(start, end);

	/*Invalidate I-Cache*/
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,2\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);

#endif

#ifdef CONFIG_RTL_EB8186
   unsigned long flags;
    volatile unsigned int reg;
    save_flags(flags);cli();
    reg=read_32bit_cp0_register(CP0_XCONTEXT);
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_32bit_cp0_register(CP0_XCONTEXT, (reg &(~0x2))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_32bit_cp0_register(CP0_XCONTEXT, (reg | 0x2)); //wirte '1' to bit 0, 1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    restore_flags(flags);
#endif

}

void r3k_flush_dcache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_RTL_EB8186
    unsigned long flags;
    volatile unsigned int reg;
    save_flags(flags);cli();
    reg=read_32bit_cp0_register(CP0_XCONTEXT);
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_32bit_cp0_register(CP0_XCONTEXT, (reg & (~0x1))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_32bit_cp0_register(CP0_XCONTEXT, (reg | 0x1)); //wirte '1' to bit 0, 1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    restore_flags(flags);
#endif    
    
#ifdef CONFIG_RTL865XC
	/* Flush D-Cache using its range */
	unsigned char *p;
	unsigned int size;
	unsigned int flags;
	unsigned int i;
	unsigned long flush_ualign=0;
	if (start == 0 && end == 0) {
		start = KSEG0;
		end = KSEG0 + _DCACHE_SIZE;
	}

	size = end - start;

	/* correctness check : flush all if any parameter is illegal */
	if (	(size >= _DCACHE_SIZE) ||
		(KSEGX(start) != KSEG0)	)
	{
		/*
		 *	ghhguang
		 *		=> For Realtek Lextra CPU,
		 *		   the cache would NOT be flushed only if the Address to-be-flushed
		 *		   is the EXPLICIT address ( which is really stored in that cache line ).
		 *		   For the aliasd addresses, the cache entry would NOT be flushed even
		 *		   it matchs same cache-index.
		 *
		 *		   => This is different from traditional MIPS-based CPU's configuration.
		 *		      So if we want to flush ALL-cache entries, we would need to use "mtc0"
		 *		      instruction instead of simply modifying the "size" to "dcache_size"
		 *		      and "start" to "KSEG0".
		 *
		 */
		__asm__ volatile(
			"mtc0 $0,$20\n\t"
			"nop\n\t"
			"li $8,512\n\t"
			"mtc0 $8,$20\n\t"
			"nop\n\t"
			"nop\n\t"
			"mtc0 $0,$20\n\t"
			"nop"
			: /* no output */
			: /* no input */
				);
	} else
	{
	
		flush_ualign = ((unsigned long)start % 16);
		if(flush_ualign){
			start = start - flush_ualign;
			size = end - start;
		}
		 
		/* Start to isolate cache space */
		p = (char *)(PHYSADDR(start) | K0BASE);

		flags = read_c0_status();

		/* isolate cache space */
		write_c0_status( (ST0_ISC | flags) &~ ST0_IEC );

		for (i = 0; i < size; i += 0x040)
		{
			asm ( 	
				#ifdef OPEN_RSDK_RTL865x
				".word 0xbc750000\n\t"
				".word 0xbc750010\n\t"
				".word 0xbc750020\n\t"
				".word 0xbc750030\n\t"
			        #else
				"cache 0x15, 0x000(%0)\n\t"
			 	"cache 0x15, 0x010(%0)\n\t"
			 	"cache 0x15, 0x020(%0)\n\t"
			 	"cache 0x15, 0x030(%0)\n\t"
			        #endif
				:		/* No output registers */
				:"r"(p)		/* input : 'p' as %0 */
				);
			p += 0x040;
		}

		write_c0_status(flags);
	}    
#endif    

}

static inline unsigned long get_phys_page (unsigned long addr,
					   struct mm_struct *mm)
{
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long physpage;

	pgd = pgd_offset(mm, addr);
	pmd = pmd_offset(pgd, addr);
	pte = pte_offset(pmd, addr);

	if ((physpage = pte_val(*pte)) & _PAGE_VALID)
		return KSEG0ADDR(physpage & PAGE_MASK);

	return 0;
}
#ifdef GCCPATCH_1_3_X
#else
static inline void r3k_flush_cache_all(void)
{
#ifdef CONFIG_RTL_EB8186	
	r3k_flush_icache_range(KSEG0, KSEG0 );
#endif	

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	r3k___flush_cache_all();
#endif

}
#endif
 
static inline void r3k___flush_cache_all(void)
{
#ifdef CONFIG_RTL_EB8186
	r3k_flush_icache_range(KSEG0, KSEG0 );
#endif	
	
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	r3k_flush_dcache_range(KSEG0, KSEG0 + _DCACHE_SIZE);
	r3k_flush_icache_range(KSEG0, KSEG0 + _ICACHE_SIZE);	
#endif	
}

static void r3k_flush_cache_mm(struct mm_struct *mm)
{
}

static void r3k_flush_cache_range(struct mm_struct *mm, unsigned long start,
				  unsigned long end)
{
}

static void r3k_flush_cache_page(struct vm_area_struct *vma,
				   unsigned long page)
{
}

static void r3k_flush_page_to_ram(struct page * page)
{
	/*
	 * Nothing to be done
	 */
}

static void r3k_flush_icache_page(struct vm_area_struct *vma, struct page *page)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long physpage;

	if (mm->context == 0)
		return;

	if (!(vma->vm_flags & VM_EXEC))
		return;

#ifdef DEBUG_CACHE
	printk("cpage[%d,%08lx]", (int)mm->context, page);
#endif

	physpage = (unsigned long) page_address(page);
	if (physpage)
		r3k_flush_icache_range(physpage, physpage + PAGE_SIZE);
}

static void r3k_flush_cache_sigtramp(unsigned long addr)
{
	
	unsigned long flags;

#ifdef DEBUG_CACHE
	printk("csigtramp[%08lx]", addr);
#endif
	save_and_cli(flags);
    
	r3k___flush_cache_all();
	restore_flags(flags);

#if 0
	flags = read_32bit_cp0_register(CP0_STATUS);

	write_32bit_cp0_register(CP0_STATUS, flags&~ST0_IEC);

	/* Fill the TLB to avoid an exception with caches isolated. */
	asm ( 	"lw\t$0, 0x000(%0)\n\t"
		"lw\t$0, 0x004(%0)\n\t"
		: : "r" (addr) );

	write_32bit_cp0_register(CP0_STATUS, (ST0_ISC|ST0_SWC|flags)&~ST0_IEC);

	asm ( 	"sb\t$0, 0x000(%0)\n\t"
		"sb\t$0, 0x004(%0)\n\t"
		: : "r" (addr) );

	write_32bit_cp0_register(CP0_STATUS, flags);
#endif
}

static void r3k_dma_cache_wback_inv(unsigned long start, unsigned long size)
{
	wbflush();
	r3k_flush_dcache_range(start, start + size);
}

void __init ld_mmu_r23000(void)
{
	unsigned long config;

	_clear_page = r3k_clear_page;
	_copy_page = r3k_copy_page;

	_flush_cache_all = r3k___flush_cache_all;
	___flush_cache_all = r3k___flush_cache_all;
	_flush_cache_mm = r3k_flush_cache_mm;
	_flush_cache_range = r3k_flush_cache_range;
	_flush_cache_page = r3k_flush_cache_page;
	_flush_cache_sigtramp = r3k_flush_cache_sigtramp;
	_flush_page_to_ram = r3k_flush_page_to_ram;
	_flush_icache_page = r3k_flush_icache_page;
	_flush_icache_range = r3k_flush_icache_range;

	_dma_cache_wback_inv = r3k_dma_cache_wback_inv;
}

// for wlan driver module, david+2006-1-19
EXPORT_SYMBOL(r3k_flush_dcache_range);

