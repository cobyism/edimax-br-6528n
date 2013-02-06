/*
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

/*
 * Setup code for the SWARM board 
 */

#include <linux/spinlock.h>
#include <linux/mc146818rtc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/blk.h>
#include <linux/init.h>
#include <linux/ide.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/sibyte/swarm.h>
#include <asm/sibyte/sb1250.h>
#include <asm/sibyte/sb1250_defs.h>
#include <asm/sibyte/sb1250_regs.h>
#include <asm/sibyte/swarm_ide.h>
#include <asm/reboot.h>
#include <asm/time.h>

#include "cfe_xiocb.h"
#include "cfe_api.h"
#include "cfe_error.h"

extern struct rtc_ops swarm_rtc_ops;
extern int cfe_console_handle;

#ifdef CONFIG_BLK_DEV_IDE_SWARM
extern struct ide_ops *ide_ops;
#endif


#ifdef CONFIG_L3DEMO
extern void *l3info;
#endif

/* Max ram addressable in 32-bit segments */
#ifdef CONFIG_HIGHMEM
#ifdef CONFIG_64BIT_PHYS_ADDR
#define MAX_RAM_SIZE (~0ULL)
#else
#define MAX_RAM_SIZE (0xffffffffULL)
#endif
#else
#ifdef CONFIG_MIPS64
#define MAX_RAM_SIZE (~0ULL)
#else
#define MAX_RAM_SIZE (0x1fffffffULL)
#endif
#endif


#ifndef CONFIG_SWARM_STANDALONE

phys_t swarm_mem_region_addrs[CONFIG_SIBYTE_SWARM_MAX_MEM_REGIONS];
phys_t swarm_mem_region_sizes[CONFIG_SIBYTE_SWARM_MAX_MEM_REGIONS];
unsigned int swarm_mem_region_count;

#endif

const char *get_system_type(void)
{
	return "SiByte Swarm";
}

#ifdef CONFIG_BLK_DEV_IDE_SWARM
static int swarm_ide_default_irq(ide_ioreg_t base)
{
        return 0;
}

static ide_ioreg_t swarm_ide_default_io_base(int index)
{
        return 0;
}

static void swarm_ide_init_hwif_ports (hw_regs_t *hw, ide_ioreg_t data_port,
                                     ide_ioreg_t ctrl_port, int *irq)
{
	ide_ioreg_t reg = data_port;
	int i;

	for (i = IDE_DATA_OFFSET; i <= IDE_STATUS_OFFSET; i++) {
		hw->io_ports[i] = reg;
		reg += 1;
	}
	if (ctrl_port) {
		hw->io_ports[IDE_CONTROL_OFFSET] = ctrl_port;
	} else {
		hw->io_ports[IDE_CONTROL_OFFSET] = hw->io_ports[IDE_DATA_OFFSET] + 0x206;
	}
	if (irq != NULL)
		*irq = 0;
	hw->io_ports[IDE_IRQ_OFFSET] = 0;
}

static int swarm_ide_request_irq(unsigned int irq,
                                void (*handler)(int,void *, struct pt_regs *),
                                unsigned long flags, const char *device,
                                void *dev_id)
{
	return request_irq(irq, handler, flags, device, dev_id);
}			

static void swarm_ide_free_irq(unsigned int irq, void *dev_id)
{
	free_irq(irq, dev_id);
}

static int swarm_ide_check_region(ide_ioreg_t from, unsigned int extent)
{
    /* Note: "check_region" and friends do conflict management on ISA I/O
       space.  Our disk is not in that space, so this check won't work */
    /* return check_region(from, extent); */
    return 0;
}

static void swarm_ide_request_region(ide_ioreg_t from, unsigned int extent,
                                    const char *name)
{
    /* request_region(from, extent, name); */
}

static void swarm_ide_release_region(ide_ioreg_t from, unsigned int extent)
{
    /* release_region(from, extent); */
}


void swarm_ideproc(ide_ide_action_t action, ide_drive_t *drive,
		   void *buffer, unsigned int count)
{
	/*  slow? vlb_sync? */
	switch (action) {
	case ideproc_ide_input_data:
		if (drive->io_32bit) {
			swarm_insl(IDE_DATA_REG, buffer, count);
		} else {
			swarm_insw(IDE_DATA_REG, buffer, count<<1);
		}
		break;
	case ideproc_ide_output_data:
		if (drive->io_32bit) {
			swarm_outsl(IDE_DATA_REG, buffer, count);
		} else {
			swarm_outsw(IDE_DATA_REG, buffer, count<<1);
		}
		break;
	case ideproc_atapi_input_bytes:
		count++;
		if (drive->io_32bit) {
			swarm_insl(IDE_DATA_REG, buffer, count>>2);
		} else {
			swarm_insw(IDE_DATA_REG, buffer, count>>1);
		}
		if ((count & 3) >= 2)
			swarm_insw(IDE_DATA_REG, (char *)buffer + (count & ~3), 1);
		break;
	case ideproc_atapi_output_bytes:
		count++;
		if (drive->io_32bit) {
			swarm_outsl(IDE_DATA_REG, buffer, count>>2);
		} else {
			swarm_outsw(IDE_DATA_REG, buffer, count>>1);
		}
		if ((count & 3) >= 2)
			swarm_outsw(IDE_DATA_REG, (char *)buffer + (count & ~3), 1);
		break;
	}
}

struct ide_ops swarm_ide_ops = {
	&swarm_ide_default_irq,
	&swarm_ide_default_io_base,
	&swarm_ide_init_hwif_ports,
	&swarm_ide_request_irq,
	&swarm_ide_free_irq,
	&swarm_ide_check_region,
	&swarm_ide_request_region,
	&swarm_ide_release_region
};
#endif


#ifdef CONFIG_SMP
static void smp_cpu0_exit(void *unused)
{
	printk("swarm_linux_exit called (cpu1) - passing control back to CFE\n");
	cfe_exit(1,0);
}
#endif

static void swarm_linux_exit(void)
{
#ifdef CONFIG_SMP
	if (smp_processor_id()) {
		smp_call_function(smp_cpu0_exit,NULL,1,1);
		while(1);
	}
#endif
	printk("swarm_linux_exit called...passing control back to CFE\n");
	cfe_exit(1, 0);
	printk("cfe_exit returned??\n");
	while(1);
}

void __init bus_error_init(void)
{
}

extern void swarm_time_init(void);

void __init swarm_setup(void)
{
	extern int panic_timeout;

	rtc_ops = &swarm_rtc_ops;
	panic_timeout = 5;  /* For debug.  This should probably be raised later */
	_machine_restart   = (void (*)(char *))swarm_linux_exit;
	_machine_halt      = swarm_linux_exit;
	_machine_power_off = swarm_linux_exit;

#ifdef CONFIG_L3DEMO
	if (l3info != NULL) {
		printk("\n");
	}
#endif
	printk("This kernel optimized for "
#ifdef CONFIG_SIMULATION
	       "simulation"
#else
	       "board"
#endif
	       " runs\n");

	board_timer_setup = swarm_time_init;

#ifdef CONFIG_BLK_DEV_IDE_SWARM
	ide_ops = &swarm_ide_ops;
#endif
}  

/* This is the kernel command line.  Actually, it's 
   copied, eventually, to command_line, and looks to be
   quite redundant.  But not something to fix just now */
extern char arcs_cmdline[];

#ifdef CONFIG_EMBEDDED_RAMDISK
/* These are symbols defined by the ramdisk linker script */
extern unsigned char __rd_start;
extern unsigned char __rd_end;  
#endif

static __init void prom_meminit(void)
{
	u64 addr, size; /* regardless of 64BIT_PHYS_ADDR */
	long type;
	unsigned int idx;
	int rd_flag;

#ifdef CONFIG_BLK_DEV_INITRD
	unsigned long initrd_pstart; 
	unsigned long initrd_pend; 

#ifdef CONFIG_EMBEDDED_RAMDISK
	/* If we're using an embedded ramdisk, then __rd_start and __rd_end
	   are defined by the linker to be on either side of the ramdisk
	   area.  Otherwise, initrd_start should be defined by kernel command
	   line arguments */
	if (initrd_start == 0) {
		initrd_start = (unsigned long)&__rd_start;
		initrd_end = (unsigned long)&__rd_end;
	}
#endif

	initrd_pstart = __pa(initrd_start);
	initrd_pend = __pa(initrd_end);
	if (initrd_start &&
	    ((initrd_pstart > MAX_RAM_SIZE)
	     || (initrd_pend > MAX_RAM_SIZE))) {
		setleds("INRD");
		panic("initrd out of addressable memory");
	}
       
#endif /* INITRD */
		
	for (idx = 0; cfe_enummem(idx, &addr, &size, &type) != CFE_ERR_NOMORE;
	     idx++) {
		rd_flag = 0;
		if (type == CFE_MI_AVAILABLE) {
			/*
			 * See if this block contains (any portion of) the
			 * ramdisk
			 */
#ifdef CONFIG_BLK_DEV_INITRD
			if (initrd_start) {
				if ((initrd_pstart > addr) &&
				    (initrd_pstart < (addr + size))) {
					add_memory_region(addr,
					                  initrd_pstart - addr,
					                  BOOT_MEM_RAM);
					rd_flag = 1;
				}
				if ((initrd_pend > addr) &&
				    (initrd_pend < (addr + size))) {
					add_memory_region(initrd_pend,
						(addr + size) - initrd_pend,
						 BOOT_MEM_RAM);
					rd_flag = 1;
				}
			}
#endif
			if (!rd_flag) {
				if (addr > MAX_RAM_SIZE)
					continue;
				if (addr+size > MAX_RAM_SIZE)
					size = MAX_RAM_SIZE - (addr+size) + 1;
				add_memory_region(addr, size, BOOT_MEM_RAM);
			}
			swarm_mem_region_addrs[swarm_mem_region_count] = addr;
			swarm_mem_region_sizes[swarm_mem_region_count] = size;
			swarm_mem_region_count++;
			if (swarm_mem_region_count ==
			    CONFIG_SIBYTE_SWARM_MAX_MEM_REGIONS) {
				/*
				 * Too many regions.  Need to configure more
				 */
				while(1);
			}
		}
	}
#ifdef CONFIG_BLK_DEV_INITRD
	if (initrd_start) {
		add_memory_region(initrd_pstart, initrd_pend - initrd_pstart,
		                 BOOT_MEM_RESERVED);
	}
#endif
}


#ifdef CONFIG_BLK_DEV_INITRD
static int __init initrd_setup(char *str)
{
	/* 
	 *Initrd location comes in the form "<hex size of ramdisk in bytes>@<location in memory>"
	 *  e.g. initrd=3abfd@80010000.  This is set up by the loader.
	 */
	char *tmp, *endptr;
	unsigned long initrd_size;
	for (tmp = str; *tmp != '@'; tmp++) {
		if (!*tmp) {
			goto fail;
		}
	}
	*tmp = 0;
	tmp++;
	if (!*tmp) {
		goto fail;
	}
	initrd_size = simple_strtol(str, &endptr, 16);
	if (*endptr) {
		goto fail;
	}
	initrd_start = simple_strtol(tmp, &endptr, 16);
	if (*endptr) {
		goto fail;
	}
	initrd_end = initrd_start + initrd_size;
	printk("Found initrd of %lx@%lx\n", initrd_size, initrd_start);
	return 1;
 fail:
	printk("Bad initrd argument.  Disabling initrd\n");
	initrd_start = 0;
	initrd_end = 0;
	return 1;
}

#endif

/*
 * prom_init is called just after the cpu type is determined, from init_arch()
 */
__init int prom_init(int argc, char **argv, char **envp, int *prom_vec)
{
	/* 
	 * This should go away.  Detect if we're booting
	 * straight from cfe without a loader.  If we
	 * are, then we've got a prom vector in a0.  Otherwise,
	 * argc (and argv and envp, for that matter) will be 0) 
	 */
	if (argc < 0) {
		prom_vec = (int *)argc;
	}
	cfe_init((long)prom_vec);
	cfe_open_console();
	if (cfe_getenv("LINUX_CMDLINE", arcs_cmdline, CL_SIZE) < 0) {
		if (argc < 0) {
			/*
			 * It's OK for direct boot to not provide a
			 *  command line
			 */
			strcpy(arcs_cmdline, "root=/dev/ram0 ");
		} else {
			/* The loader should have set the command line */
			setleds("CMDL");
			panic("LINUX_CMDLINE not defined in cfe.");
		}
	}

#ifdef CONFIG_BLK_DEV_INITRD
	{
		char *ptr;
		/* Need to find out early whether we've got an initrd.  So scan
		   the list looking now */
		for (ptr = arcs_cmdline; *ptr; ptr++) {
			while (*ptr == ' ') {
				ptr++;
			}
			if (!strncmp(ptr, "initrd=", 7)) {
				initrd_setup(ptr+7);
				break;
			} else {
				while (*ptr && (*ptr != ' ')) {
					ptr++;
				}
			}
		}
	}
#endif /* CONFIG_BLK_DEV_INITRD */

	/* Not sure this is needed, but it's the safe way. */
	arcs_cmdline[CL_SIZE-1] = 0;

	mips_machgroup = MACH_GROUP_SIBYTE;
	prom_meminit();

	return 0;
}

void prom_free_prom_memory(void)
{
	/* Not sure what I'm supposed to do here.  Nothing, I think */
}

static void setled(unsigned int index, char c) 
{
	volatile unsigned char *led_ptr;

	led_ptr = (unsigned char *)(IO_SPACE_BASE | LED_BASE_ADDR);
	if (index < 4) {
		led_ptr[(3-index)<<3] = c;
	}
}

void setleds(char *str)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (!str[i]) {
			for (; i < 4; i++) { 
				setled(' ', str[i]);
			}
		} else {
			setled(i, str[i]);
		}
	}
}

#include <linux/timer.h>

static struct timer_list led_timer;
static unsigned char default_led_msg[] =
	"Today: the CSWARM.  Tomorrow: the WORLD!!!!           ";
static unsigned char *led_msg = default_led_msg;
static unsigned char *led_msg_ptr = default_led_msg;

void set_led_msg(char *new_msg)
{
	led_msg = new_msg;
	led_msg_ptr = new_msg;
	setleds("    ");
}

static void move_leds(unsigned long arg) 
{
	int i;
	unsigned char *tmp = led_msg_ptr;
	for (i = 0; i < 4; i++) {
		setled(i, *tmp);
		tmp++;
		if (!*tmp) { 
			tmp = led_msg; 
		}
	}
	led_msg_ptr++;
	if (!*led_msg_ptr) { 
 		led_msg_ptr = led_msg; 
	}
	del_timer(&led_timer);
	led_timer.expires = jiffies + (HZ/8);
	add_timer(&led_timer);
}

void hack_leds(void) 
{
	init_timer(&led_timer);
	led_timer.expires = jiffies + (HZ/8);
	led_timer.data = 0;
	led_timer.function = move_leds;
	add_timer(&led_timer);
}
