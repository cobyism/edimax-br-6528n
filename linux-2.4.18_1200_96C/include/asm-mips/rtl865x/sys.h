struct rtl_pool {	/* the pool */
	struct list_head	page_list;
	spinlock_t		lock;
	size_t			blocks_per_page;
	size_t			size;
	int			flags;
	size_t			allocation;
	char			name [32];
	wait_queue_head_t	waitq;
};

struct rtl_page {	/* cacheable header for 'allocation' bytes */
	struct list_head	page_list;
	void			*vaddr;
	dma_addr_t		dma;
	unsigned long		bitmap [0];
};

#define	POOL_TIMEOUT_JIFFIES	((100 /* msec */ * HZ) / 1000)
#define	POOL_POISON_BYTE	0xa7


#undef DEBUG

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif



#define	POOL_TIMEOUT_JIFFIES	((100 /* msec */ * HZ) / 1000)
#define	POOL_POISON_BYTE	0xa7
#define CONFIG_RTLPOOL_DEBUG
void rtl_free_consistent(size_t size, void *vaddr, dma_addr_t handle);
void *consistent_alloc(int gfp, size_t size, dma_addr_t *dma_handle);
void *rtl_alloc_consistent(size_t size, dma_addr_t *handle);
struct rtl_pool* rtl_pool_create(const char *, size_t, size_t ,size_t,int);
void rtl_pool_destroy (struct rtl_pool *pool);
void *rtl_pool_alloc (struct rtl_pool *, int, dma_addr_t *);
void rtl_pool_free (struct rtl_pool *pool, void *vaddr, dma_addr_t dma);
		
