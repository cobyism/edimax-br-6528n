/*
 *	$Id: sys_pool.c,v 1.1.1.1 2007/08/06 10:04:45 root Exp $
 *
 *	PCI Bus Services, see include/linux/pci.h for further explanation.
 *
 *	Copyright 1993 -- 1997 Drew Eckhardt, Frederic Potter,
 *	David Mosberger-Tang
 *
 *	Copyright 1997 -- 2000 Martin Mares <mj@ucw.cz>
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/pm.h>
#include <linux/kmod.h>		/* for hotplug_path */
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/cache.h>

#include <asm/page.h>
#include <asm/dma.h>	/* isa_dma_bridge_buggy */

#undef DEBUG

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif



#define	POOL_TIMEOUT_JIFFIES	((100 /* msec */ * HZ) / 1000)
#define	POOL_POISON_BYTE	0xa7

// #define CONFIG_RTLPOOL_DEBUG


/**
 * rtl_pool_create - Creates a pool of  consistent memory blocks, for dma.
 * @name: name of pool, for diagnostics
 * @pdev:  device that will be doing the DMA
 * @size: size of the blocks in this pool.
 * @align: alignment requirement for blocks; must be a power of two
 * @allocation: returned blocks won't cross this boundary (or zero)
 * @flags: SLAB_* flags (not all are supported).
 *
 * Returns a pci allocation pool with the requested characteristics, or
 * null if one can't be created.  Given one of these pools, rtl_pool_alloc()
 * may be used to allocate memory.  Such memory will all have "consistent"
 * DMA mappings, accessible by the device and its driver without using
 * cache flushing primitives.  The actual size of blocks allocated may be
 * larger than requested because of alignment.
 *
 * If allocation is nonzero, objects returned from rtl_pool_alloc() won't
 * cross that size boundary.  This is useful for devices which have
 * addressing restrictions on individual DMA transfers, such as not crossing
 * boundaries of 4KBytes.
 */
void *consistent_alloc(int gfp, size_t size, dma_addr_t *dma_handle)
{
	struct page *page, *end, *free;
	unsigned long order;
	void *ret, *virt;

	if (in_interrupt())
		BUG();

	size = PAGE_ALIGN(size);
	order = get_order(size);

	page = alloc_pages(gfp, order);
	if (!page)
		goto no_page;

	/*
	 * We could do with a page_to_phys and page_to_bus here.
	 */
	virt = page_address(page);
	*dma_handle = virt_to_bus(virt);
	ret = __ioremap(virt_to_phys(virt), size, 0);
	if (!ret)
		goto no_remap;

/*
	 * free wasted pages.  We skip the first page since we know
	 * that it will have count = 1 and won't require freeing.
	 * We also mark the pages in use as reserved so that
	 * remap_page_range works.
	 */
	page = virt_to_page(virt);
	free = page + (size >> PAGE_SHIFT);
	end  = page + (1 << order);

	for (; page < end; page++) {
		set_page_count(page, 1);
		if (page >= free)
			__free_page(page);
		else
			SetPageReserved(page);
	}
	return ret;

no_remap:
	__free_pages(page, order);
no_page:
	return NULL;
}


void *rtl_alloc_consistent(size_t size, dma_addr_t *handle)
{
	void *__ret;
	int __gfp = GFP_KERNEL;

		__gfp |= GFP_DMA;

	__ret = consistent_alloc(__gfp, (size),
				 (handle));
	return __ret;
}



struct rtl_pool *
rtl_pool_create (const char *name, size_t size, size_t align, size_t allocation, int flags)
{
	struct rtl_pool		*retval;

	if (align == 0)
		align = 1;
	if (size == 0)
		return 0;
	else if (size < align)
		size = align;
	else if ((size % align) != 0) {
		size += align + 1;
		size &= ~(align - 1);
	}

	if (allocation == 0) {
		if (PAGE_SIZE < size)
			allocation = size;
		else
			allocation = PAGE_SIZE;
		// FIXME: round up for less fragmentation
	} else if (allocation < size)
		return 0;

	if (!(retval = kmalloc (sizeof *retval, flags)))
		return retval;

#ifdef	CONFIG_RTLPOOL_DEBUG
	flags |= SLAB_POISON;
#endif

	strncpy (retval->name, name, sizeof retval->name);
	retval->name [sizeof retval->name - 1] = 0;

	retval->dev = pdev;
	INIT_LIST_HEAD (&retval->page_list);
	spin_lock_init (&retval->lock);
	retval->size = size;
	retval->flags = flags;
	retval->allocation = allocation;
	retval->blocks_per_page = allocation / size;
	init_waitqueue_head (&retval->waitq);

#ifdef CONFIG_RTLPOOL_DEBUG
	printk (KERN_DEBUG "pcipool create %s/%s size %d, %d/page (%d alloc)\n",
		pdev ? pdev->slot_name : NULL, retval->name, size,
		retval->blocks_per_page, allocation);
#endif

	return retval;
}


static struct rtl_page *
pool_alloc_page (struct rtl_pool *pool, int mem_flags)
{
	struct rtl_page	*page;
	int		mapsize;

	mapsize = pool->blocks_per_page;
	mapsize = (mapsize + BITS_PER_LONG - 1) / BITS_PER_LONG;
	mapsize *= sizeof (long);

	page = (struct rtl_page *) kmalloc (mapsize + sizeof *page, mem_flags);
	if (!page)
		return 0;
	page->vaddr = rtl_alloc_consistent (pool->allocation,
					    &page->dma);
	if (page->vaddr) {
		memset (page->bitmap, 0xff, mapsize);	// bit set == free
		if (pool->flags & SLAB_POISON)
			memset (page->vaddr, POOL_POISON_BYTE, pool->allocation);
		list_add (&page->page_list, &pool->page_list);
	} else {
		kfree (page);
		page = 0;
	}
	return page;
}


static inline int
is_page_busy (int blocks, unsigned long *bitmap)
{
	while (blocks > 0) {
		if (*bitmap++ != ~0UL)
			return 1;
		blocks -= BITS_PER_LONG;
	}
	return 0;
}

static void
pool_free_page (struct rtl_pool *pool, struct rtl_page *page)
{
	dma_addr_t	dma = page->dma;

	if (pool->flags & SLAB_POISON)
		memset (page->vaddr, POOL_POISON_BYTE, pool->allocation);
	kfree (pool->allocation, page->vaddr, dma);
	list_del (&page->page_list);
	kfree (page);
}


/**
 * rtl_pool_destroy - destroys a pool of pci memory blocks.
 * @pool: pci pool that will be destroyed
 *
 * Caller guarantees that no more memory from the pool is in use,
 * and that nothing will try to use the pool after this call.
 */
void
rtl_pool_destroy (struct rtl_pool *pool)
{
	unsigned long		flags;

#ifdef CONFIG_RTLPOOL_DEBUG
	printk (KERN_DEBUG "pcipool destroy %s/%s\n",
		pool->dev ? pool->dev->slot_name : NULL,
		pool->name);
#endif

	spin_lock_irqsave (&pool->lock, flags);
	while (!list_empty (&pool->page_list)) {
		struct rtl_page		*page;
		page = list_entry (pool->page_list.next,
				struct rtl_page, page_list);
		if (is_page_busy (pool->blocks_per_page, page->bitmap)) {
			printk (KERN_ERR "rtl_pool_destroy %s/%s, %p busy\n",
				pool->dev ? pool->dev->slot_name : NULL,
				pool->name, page->vaddr);
			/* leak the still-in-use consistent memory */
			list_del (&page->page_list);
			kfree (page);
		} else
			pool_free_page (pool, page);
	}
	spin_unlock_irqrestore (&pool->lock, flags);
	kfree (pool);
}


/**
 * rtl_pool_alloc - get a block of consistent memory
 * @pool: pci pool that will produce the block
 * @mem_flags: SLAB_KERNEL or SLAB_ATOMIC
 * @handle: pointer to dma address of block
 *
 * This returns the kernel virtual address of a currently unused block,
 * and reports its dma address through the handle.
 * If such a memory block can't be allocated, null is returned.
 */
void *
rtl_pool_alloc (struct rtl_pool *pool, int mem_flags, dma_addr_t *handle)
{
	unsigned long		flags;
	struct list_head	*entry;
	struct rtl_page		*page;
	int			map, block;
	size_t			offset;
	void			*retval;

restart:
	spin_lock_irqsave (&pool->lock, flags);
	list_for_each (entry, &pool->page_list) {
		int		i;
		page = list_entry (entry, struct rtl_page, page_list);
		/* only cachable accesses here ... */
		for (map = 0, i = 0;
				i < pool->blocks_per_page;
				i += BITS_PER_LONG, map++) {
			if (page->bitmap [map] == 0)
				continue;
			block = ffz (~ page->bitmap [map]);
			if ((i + block) < pool->blocks_per_page) {
				clear_bit (block, &page->bitmap [map]);
				offset = (BITS_PER_LONG * map) + block;
				offset *= pool->size;
				goto ready;
			}
		}
	}
	if (!(page = pool_alloc_page (pool, mem_flags))) {
		if (mem_flags == SLAB_KERNEL) {
			DECLARE_WAITQUEUE (wait, current);

			current->state = TASK_INTERRUPTIBLE;
			add_wait_queue (&pool->waitq, &wait);
			spin_unlock_irqrestore (&pool->lock, flags);

			schedule_timeout (POOL_TIMEOUT_JIFFIES);

			current->state = TASK_RUNNING;
			remove_wait_queue (&pool->waitq, &wait);
			goto restart;
		}
		retval = 0;
		goto done;
	}

	clear_bit (0, &page->bitmap [0]);
	offset = 0;
ready:
	retval = offset + page->vaddr;
	*handle = offset + page->dma;
done:
	spin_unlock_irqrestore (&pool->lock, flags);
	return retval;
}


static struct rtl_page *
pool_find_page (struct rtl_pool *pool, dma_addr_t dma)
{
	unsigned long		flags;
	struct list_head	*entry;
	struct rtl_page		*page;

	spin_lock_irqsave (&pool->lock, flags);
	list_for_each (entry, &pool->page_list) {
		page = list_entry (entry, struct rtl_page, page_list);
		if (dma < page->dma)
			continue;
		if (dma < (page->dma + pool->allocation))
			goto done;
	}
	page = 0;
done:
	spin_unlock_irqrestore (&pool->lock, flags);
	return page;
}


/**
 * rtl_pool_free - put block back into pci pool
 * @pool: the pci pool holding the block
 * @vaddr: virtual address of block
 * @dma: dma address of block
 *
 * Caller promises neither device nor driver will again touch this block
 * unless it is first re-allocated.
 */
void
rtl_pool_free (struct rtl_pool *pool, void *vaddr, dma_addr_t dma)
{
	struct rtl_page		*page;
	unsigned long		flags;
	int			map, block;

	if ((page = pool_find_page (pool, dma)) == 0) {
		printk (KERN_ERR "rtl_pool_free %s/%s, %p/%x (bad dma)\n",
			pool->dev ? pool->dev->slot_name : NULL,
			pool->name, vaddr, (int) (dma & 0xffffffff));
		return;
	}
#ifdef	CONFIG_RTLPOOL_DEBUG
	if (((dma - page->dma) + (void *)page->vaddr) != vaddr) {
		printk (KERN_ERR "rtl_pool_free %s/%s, %p (bad vaddr)/%x\n",
			pool->dev ? pool->dev->slot_name : NULL,
			pool->name, vaddr, (int) (dma & 0xffffffff));
		return;
	}
#endif

	block = dma - page->dma;
	block /= pool->size;
	map = block / BITS_PER_LONG;
	block %= BITS_PER_LONG;

#ifdef	CONFIG_RTLPOOL_DEBUG
	if (page->bitmap [map] & (1UL << block)) {
		printk (KERN_ERR "rtl_pool_free %s/%s, dma %x already free\n",
			pool->dev ? pool->dev->slot_name : NULL,
			pool->name, dma);
		return;
	}
#endif
	if (pool->flags & SLAB_POISON)
		memset (vaddr, POOL_POISON_BYTE, pool->size);

	spin_lock_irqsave (&pool->lock, flags);
	set_bit (block, &page->bitmap [map]);
	if (waitqueue_active (&pool->waitq))
		wake_up (&pool->waitq);
	/*
	 * Resist a temptation to do
	 *    if (!is_page_busy(bpp, page->bitmap)) pool_free_page(pool, page);
	 * it is not interrupt safe. Better have empty pages hang around.
	 */
	spin_unlock_irqrestore (&pool->lock, flags);
}


/* Pool allocator */

EXPORT_SYMBOL (rtl_pool_create);
EXPORT_SYMBOL (rtl_pool_destroy);
EXPORT_SYMBOL (rtl_pool_alloc);
EXPORT_SYMBOL (rtl_pool_free);

