/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995  Linus Torvalds
 * Copyright (C) 1995  Waldorf Electronics
 * Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001  Ralf Baechle
 * Copyright (C) 1996  Stoned Elipot
 * Copyright (C) 2000, 2001  Maciej W. Rozycki
 */
#include <linux/config.h>
#include <linux/module.h>

#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/utsname.h>
#include <linux/a.out.h>
#include <linux/tty.h>
#include <linux/bootmem.h>
#include <linux/blk.h>
#include <linux/ide.h>
#include <linux/timex.h>

#include <asm/asm.h>
#include <asm/bootinfo.h>
#include <asm/cachectl.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/stackframe.h>
#include <asm/system.h>
#ifdef CONFIG_SGI_IP22
#include <asm/sgialib.h>
#endif

#ifndef CONFIG_SMP
struct cpuinfo_mips cpu_data[1];
#endif

#ifdef JACKSON_NET_WORK
#if 0
#include <linux/config.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/route.h>
#include <linux/ip.h>
#endif
#endif

/*
 * Not all of the MIPS CPUs have the "wait" instruction available. Moreover,
 * the implementation of the "wait" feature differs between CPU families. This
 * points to the function that implements CPU specific wait. 
 * The wait instruction stops the pipeline and reduces the power consumption of
 * the CPU very much.
 */
void (*cpu_wait)(void) = NULL;

/*
 * There are several bus types available for MIPS machines.  "RISC PC"
 * type machines have ISA, EISA, VLB or PCI available, DECstations
 * have Turbochannel or Q-Bus, SGI has GIO, there are lots of VME
 * boxes ...
 * This flag is set if a EISA slots are available.
 */
int EISA_bus = 0;

struct screen_info screen_info;

extern struct fd_ops no_fd_ops;
struct fd_ops *fd_ops;

#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
extern struct ide_ops no_ide_ops;
struct ide_ops *ide_ops;
#endif

extern void * __rd_start, * __rd_end;

extern struct rtc_ops no_rtc_ops;
struct rtc_ops *rtc_ops;

#ifdef CONFIG_PC_KEYB
extern struct kbd_ops no_kbd_ops;
struct kbd_ops *kbd_ops;
#endif

/*
 * Setup information
 *
 * These are initialized so they are in the .data section
 */
unsigned long mips_machtype = MACH_UNKNOWN;
unsigned long mips_machgroup = MACH_GROUP_UNKNOWN;

struct boot_mem_map boot_mem_map;

unsigned char aux_device_present;
extern char _ftext, _etext, _fdata, _edata, _end;

static char command_line[CL_SIZE];
       char saved_command_line[CL_SIZE];
extern char arcs_cmdline[CL_SIZE];

/*
 * mips_io_port_base is the begin of the address space to which x86 style
 * I/O ports are mapped.
 */
const unsigned long mips_io_port_base = -1;
EXPORT_SYMBOL(mips_io_port_base);


/*
 * isa_slot_offset is the address where E(ISA) busaddress 0 is is mapped
 * for the processor.
 */
unsigned long isa_slot_offset;
EXPORT_SYMBOL(isa_slot_offset);

extern void sgi_sysinit(void);
extern void SetUpBootInfo(void);
extern void loadmmu(void);
extern asmlinkage void start_kernel(void);
extern void prom_init(int, char **, char **, int *);

static struct resource code_resource = { "Kernel code" };
static struct resource data_resource = { "Kernel data" };

#if CONFIG_WIRELESS_LAN_MODULE
extern int (*wirelessnet_hook)(void);
EXPORT_SYMBOL(wirelessnet_hook);
int (*wirelessnet_hook)(void)=NULL;
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include <linux/netdevice.h>
void shutdown_netdev()
{
	struct net_device *dev;
	printk("Shutdown network interface\n");
	read_lock(&dev_base_lock);
	for (dev = dev_base; dev != NULL; dev = dev->next) {
		if (dev->flags&IFF_UP && dev->stop){
			printk("%s:===>\n",dev->name);
			rtnl_lock();
			dev_close(dev);
			rtnl_unlock();
		}
	}
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8192CD)
	{
		extern void force_stop_wlan_hw(void);
		force_stop_wlan_hw();
	}
#endif
	read_unlock(&dev_base_lock);

}
#endif

static inline void check_wait(void)
{
	printk("check_wait... unavailable.\n");
}

void __init check_bugs(void)
{
	check_wait();
}

/*
 * Probe whether cpu has config register by trying to play with
 * alternate cache bit and see whether it matters.
 * It's used by cpu_probe to distinguish between R3000A and R3081.
 */


/* declaration of the global struct */
struct mips_cpu mips_cpu = {
    processor_id:	PRID_IMP_UNKNOWN,
    fpu_id:		FPIR_IMP_NONE,
    cputype:		CPU_UNKNOWN
};

/* Shortcut for assembler access to mips_cpu.options */
int *cpuoptions = &mips_cpu.options;

#define R4K_OPTS (MIPS_CPU_TLB | MIPS_CPU_4KEX | MIPS_CPU_4KTLB \
		| MIPS_CPU_COUNTER | MIPS_CPU_CACHE_CDEX)

static inline void cpu_probe(void)
{
	mips_cpu.cputype = CPU_R3000;
	mips_cpu.isa_level = MIPS_CPU_ISA_I;
	mips_cpu.options = MIPS_CPU_TLB;
#ifdef CONFIG_RTL_EB8186	
	mips_cpu.tlbsize = 64;
#endif
#ifdef CONFIG_RTL865XC	
	mips_cpu.tlbsize = 32;
#endif	
}

static inline void cpu_report(void)
{
#ifndef JACKSON_NET_WORK	
// comment this for code sizing down
	printk("CPU revision is: %08x\n", mips_cpu.processor_id);
	if (mips_cpu.options & MIPS_CPU_FPU)
		printk("FPU revision is: %08x\n", mips_cpu.fpu_id);
#endif	
}


#ifdef CONFIG_RTL865X_SUSPEND
/* this function is refer to nino_wait() in \linux-2.4.18\arch\mips\philips\nino\power.c */
static inline void RTL865x_cpu_wait(void)
{
#ifdef CONFIG_RTL8197B_PANA
	extern int cpu_suspend_enabled;
	if (!cpu_suspend_enabled)
		return;	
#endif

/*
 * 08-12-2008, due to the WLAN performance is no difference in 10/100 and giga board when gCpuCanSuspend=1
 * (that means CPU always can suspend), the following code is disabled.
 * ==> only apply to RTL865X, it still need to check the WLAN throughput in RTL8196B
 */
#ifdef CONFIG_RTL8196B //michaelxxx
#ifndef CONFIG_RTL8196C
#ifdef CONFIG_NET_WIRELESS_AGN
#ifndef CONFIG_RTL8197B_PANA
#if CONFIG_WIRELESS_LAN_MODULE
	if(wirelessnet_hook!=NULL)
	{
		if(!wirelessnet_hook())
		{
			return;
		}
	}
#else
	extern int gCpuCanSuspend;
	if (!gCpuCanSuspend)
            return;
#endif
#endif
#endif
#endif
#endif

#ifdef CONFIG_RTL8196C //michaelxxx
#define CONFIG_RTL819X_SUSPEND_CHECK_INTERRUPT
#ifdef CONFIG_RTL819X_SUSPEND_CHECK_INTERRUPT
	extern int cpu_can_suspend, cpu_can_suspend_check_init;
	extern void suspend_check_interrupt_init(void);
	
	if (cpu_can_suspend_check_init) {
		if (!cpu_can_suspend){
	            return;
	        }
	}
	else {
		suspend_check_interrupt_init();
		cpu_can_suspend_check_init = 1;
	}
#endif //CONFIG_RTL819X_SUSPEND_CHECK_INTERRUPT
#endif //CONFIG_RTL8196C

	/* We stop the CPU to conserve power */
	
	#ifdef OPEN_RSDK_RTL865x
	#else
	__asm__ __volatile__ (    "sleep\n\t" ); 
	#endif
	/* 
	 * We wait until an interrupt happens...
	 */

	/* We resume here */

	/* Give ourselves a little delay */
	__asm__ __volatile__(
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t");

};
#endif

asmlinkage void __init
init_arch(int argc, char **argv, char **envp, int *prom_vec)
{
	unsigned int s;
#ifdef JACKSON_NET_WORK
	unsigned int dataram;
#endif
	/* Determine which MIPS variant we are running on. */
	cpu_probe();

	prom_init(argc, argv, envp, prom_vec);

#ifdef CONFIG_SGI_IP22
	sgi_sysinit();
#endif

	cpu_report();

	/*
	 * Determine the mmu/cache attached to this machine,
	 * then flush the tlb and caches.  On the r4xx0
	 * variants this also sets CP0_WIRED to zero.
	 */
	loadmmu();

	/* Disable coprocessors and set FPU for 16 FPRs */
	s = read_32bit_cp0_register(CP0_STATUS);
	s &= ~(ST0_CU1|ST0_CU2|ST0_CU3|ST0_KX|ST0_SX|ST0_FR);
	s |= ST0_CU0 ;
	write_32bit_cp0_register(CP0_STATUS, s);

#ifdef JACKSON_NET_WORK
#if 0	
	extern  struct ip_table packet_mangler;	
	dataram = (((unsigned int)&(packet_mangler) & 0x0fffffff)) ;
	write_32bit_cp3_register($4, dataram);	
	write_32bit_cp3_register($5, dataram+4096-1);	
#endif	
#endif

#ifdef CONFIG_RTL865X_SUSPEND
       {
       /* initial the cpu_wait function pointer, it is NULL originally. */
       void (RTL865x_cpu_wait)(void);
       cpu_wait = RTL865x_cpu_wait;
       }
#endif
	start_kernel();
}

void __init add_memory_region(phys_t start, phys_t size,
			      long type)
{
	int x = boot_mem_map.nr_map;

	if (x == BOOT_MEM_MAP_MAX) {
		printk("Ooops! Too many entries in the memory map!\n");
		return;
	}

	boot_mem_map.map[x].addr = start;
	boot_mem_map.map[x].size = size;
	boot_mem_map.map[x].type = type;
	boot_mem_map.nr_map++;
}

static void __init print_memory_map(void)
{
	int i;

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		printk(" memory: %08Lx @ %08Lx ",
			(unsigned long long) boot_mem_map.map[i].size,
			(unsigned long long) boot_mem_map.map[i].addr);
		switch (boot_mem_map.map[i].type) {
		case BOOT_MEM_RAM:
			printk("(usable)\n");
			break;
		case BOOT_MEM_ROM_DATA:
			printk("(ROM data)\n");
			break;
		case BOOT_MEM_RESERVED:
			printk("(reserved)\n");
			break;
		default:
			printk("type %lu\n", boot_mem_map.map[i].type);
			break;
		}
	}
}

static inline void parse_mem_cmdline(void)
{
	char c = ' ', *to = command_line, *from = saved_command_line;
	unsigned long start_at, mem_size;
	int len = 0;
	int usermem = 0;

	printk("Determined physical RAM map:\n");
	print_memory_map();
#ifndef JACKSON_NET_WORK
	for (;;) {
		/*
		 * "mem=XXX[kKmM]" defines a memory region from
		 * 0 to <XXX>, overriding the determined size.
		 * "mem=XXX[KkmM]@YYY[KkmM]" defines a memory region from
		 * <YYY> to <YYY>+<XXX>, overriding the determined size.
		 */
		if (c == ' ' && !memcmp(from, "mem=", 4)) {
			if (to != command_line)
				to--;
			/*
			 * If a user specifies memory size, we
			 * blow away any automatically generated
			 * size.
			 */
			if (usermem == 0) {
				boot_mem_map.nr_map = 0;
				usermem = 1;
			}
			mem_size = memparse(from + 4, &from);
			if (*from == '@')
				start_at = memparse(from + 1, &from);
			else
				start_at = 0;
			add_memory_region(start_at, mem_size, BOOT_MEM_RAM);
		}
		c = *(from++);
		if (!c)
			break;
		if (CL_SIZE <= ++len)
			break;
		*(to++) = c;
	}
	*to = '\0';

	if (usermem) {
		printk("User-defined physical RAM map:\n");
		print_memory_map();
	}
#endif	
}

void __init setup_arch(char **cmdline_p)
{
	void nino_setup(void);
	unsigned long bootmap_size;
	unsigned long start_pfn, max_pfn, max_low_pfn, first_usable_pfn;
#ifdef CONFIG_BLK_DEV_INITRD
	unsigned long tmp;
	unsigned long* initrd_header;
#endif

	int i;

#ifdef CONFIG_BLK_DEV_FD
	fd_ops = &no_fd_ops;
#endif

#ifdef CONFIG_BLK_DEV_IDE
	ide_ops = &no_ide_ops;
#endif

#ifdef CONFIG_PC_KEYB
	kbd_ops = &no_kbd_ops;
#endif
	
	rtc_ops = &no_rtc_ops;

	nino_setup();

	strncpy(command_line, arcs_cmdline, sizeof command_line);
	command_line[sizeof command_line - 1] = 0;
	strcpy(saved_command_line, command_line);
	*cmdline_p = command_line;

	parse_mem_cmdline();

#define PFN_UP(x)	(((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)	((x) << PAGE_SHIFT)

#define MAXMEM		HIGHMEM_START
#define MAXMEM_PFN	PFN_DOWN(MAXMEM)

#ifdef CONFIG_BLK_DEV_INITRD
	tmp = (((unsigned long)&_end + PAGE_SIZE-1) & PAGE_MASK) - 8; 
	if (tmp < (unsigned long)&_end) 
		tmp += PAGE_SIZE;
	initrd_header = (unsigned long *)tmp;
	if (initrd_header[0] == 0x494E5244) {
		initrd_start = (unsigned long)&initrd_header[2];
		initrd_end = initrd_start + initrd_header[1];
	}
	start_pfn = PFN_UP(__pa((&_end)+(initrd_end - initrd_start) + PAGE_SIZE));
#else
	/*
	 * Partially used pages are not usable - thus
	 * we are rounding upwards.
	 */
	start_pfn = PFN_UP(__pa(&_end));
#endif	/* CONFIG_BLK_DEV_INITRD */

	/* Find the highest page frame number we have available.  */
	max_pfn = 0;
	first_usable_pfn = -1UL;
	for (i = 0; i < boot_mem_map.nr_map; i++) {
		unsigned long start, end;

		if (boot_mem_map.map[i].type != BOOT_MEM_RAM)
			continue;

		start = PFN_UP(boot_mem_map.map[i].addr);
		end = PFN_DOWN(boot_mem_map.map[i].addr
		      + boot_mem_map.map[i].size);

		if (start >= end)
			continue;
		if (end > max_pfn)
			max_pfn = end;
		if (start < first_usable_pfn) {
			if (start > start_pfn) {
				first_usable_pfn = start;
			} else if (end > start_pfn) {
				first_usable_pfn = start_pfn;
			}
		}
	}

	/*
	 * Determine low and high memory ranges
	 */
	max_low_pfn = max_pfn;
	if (max_low_pfn > MAXMEM_PFN) {
		max_low_pfn = MAXMEM_PFN;
#ifndef CONFIG_HIGHMEM
		/* Maximum memory usable is what is directly addressable */
		printk(KERN_WARNING "Warning only %ldMB will be used.\n",
		       MAXMEM>>20);
		printk(KERN_WARNING "Use a HIGHMEM enabled kernel.\n");
#endif
	}

#ifdef CONFIG_HIGHMEM
	/*
	 * Crude, we really should make a better attempt at detecting
	 * highstart_pfn
	 */
	highstart_pfn = highend_pfn = max_pfn;
	if (max_pfn > MAXMEM_PFN) {
		highstart_pfn = MAXMEM_PFN;
		printk(KERN_NOTICE "%ldMB HIGHMEM available.\n",
		       (highend_pfn - highstart_pfn) >> (20 - PAGE_SHIFT));
	}
#endif

	/* Initialize the boot-time allocator with low memory only.  */
	bootmap_size = init_bootmem(first_usable_pfn, max_low_pfn);

	/*
	 * Register fully available low RAM pages with the bootmem allocator.
	 */
	for (i = 0; i < boot_mem_map.nr_map; i++) {
		unsigned long curr_pfn, last_pfn, size;

		/*
		 * Reserve usable memory.
		 */
		if (boot_mem_map.map[i].type != BOOT_MEM_RAM)
			continue;

		/*
		 * We are rounding up the start address of usable memory:
		 */
		curr_pfn = PFN_UP(boot_mem_map.map[i].addr);
		if (curr_pfn >= max_low_pfn)
			continue;
		if (curr_pfn < start_pfn)
			curr_pfn = start_pfn;

		/*
		 * ... and at the end of the usable range downwards:
		 */
		last_pfn = PFN_DOWN(boot_mem_map.map[i].addr
				    + boot_mem_map.map[i].size);

		if (last_pfn > max_low_pfn)
			last_pfn = max_low_pfn;

		/*
		 * Only register lowmem part of lowmem segment with bootmem.
		 */
		size = last_pfn - curr_pfn;
		if (curr_pfn > PFN_DOWN(HIGHMEM_START))
			continue;
		if (curr_pfn + size - 1 > PFN_DOWN(HIGHMEM_START))
			size = PFN_DOWN(HIGHMEM_START) - curr_pfn;
		if (!size)
			continue;

		/*
		 * ... finally, did all the rounding and playing
		 * around just make the area go away?
		 */
		if (last_pfn <= curr_pfn)
			continue;

		/* Register lowmem ranges */
		free_bootmem(PFN_PHYS(curr_pfn), PFN_PHYS(size));
	}

	/* Reserve the bootmap memory.  */
	reserve_bootmem(PFN_PHYS(first_usable_pfn), bootmap_size);

#ifdef CONFIG_BLK_DEV_INITRD
	/* Board specific code should have set up initrd_start and initrd_end */
	ROOT_DEV = MKDEV(RAMDISK_MAJOR, 0);
	if (&__rd_start != &__rd_end) {
		initrd_start = (unsigned long)&__rd_start;
		initrd_end = (unsigned long)&__rd_end;
	}
	initrd_below_start_ok = 1;
	if (initrd_start) {
		unsigned long initrd_size = ((unsigned char *)initrd_end) - ((unsigned char *)initrd_start); 
		printk("Initial ramdisk at: 0x%p (%lu bytes)\n",
		       (void *)initrd_start, 
		       initrd_size);
		if ((void *)initrd_end > phys_to_virt(PFN_PHYS(max_low_pfn))) {
			printk("initrd extends beyond end of memory "
			       "(0x%lx > 0x%p)\ndisabling initrd\n",
			       initrd_end,
			       phys_to_virt(PFN_PHYS(max_low_pfn)));
			initrd_start = initrd_end = 0;
		} 
	}
#endif /* CONFIG_BLK_DEV_INITRD  */

	paging_init();

	code_resource.start = virt_to_bus(&_ftext);
	code_resource.end = virt_to_bus(&_etext) - 1;
	data_resource.start = virt_to_bus(&_fdata);
	data_resource.end = virt_to_bus(&_edata) - 1;

	/*
	 * Request address space for all standard RAM.
	 */
	for (i = 0; i < boot_mem_map.nr_map; i++) {
		struct resource *res;
		unsigned long addr_pfn, end_pfn;

		res = alloc_bootmem(sizeof(struct resource));
		switch (boot_mem_map.map[i].type) {
		case BOOT_MEM_RAM:
		case BOOT_MEM_ROM_DATA:
			res->name = "System RAM";
			break;
		case BOOT_MEM_RESERVED:
		default:
			res->name = "reserved";
		}
		addr_pfn = PFN_UP(boot_mem_map.map[i].addr);
		end_pfn = PFN_UP(boot_mem_map.map[i].addr+boot_mem_map.map[i].size);
		if (addr_pfn > max_low_pfn)
			continue;
		res->start = boot_mem_map.map[i].addr;
		if (end_pfn < max_low_pfn) {
			res->end = res->start + boot_mem_map.map[i].size - 1;
		} else {
			res->end = max_low_pfn - 1;
		}
		res->flags = IORESOURCE_MEM | IORESOURCE_BUSY;
		request_resource(&iomem_resource, res);

		/*
		 *  We dont't know which RAM region contains kernel data,
		 *  so we try it repeatedly and let the resource manager
		 *  test it.
		 */
		request_resource(res, &code_resource);
		request_resource(res, &data_resource);
	}
}


int __init fpu_disable(char *s)
{
	mips_cpu.options &= ~MIPS_CPU_FPU;
	return 1;
}
__setup("nofpu", fpu_disable);
