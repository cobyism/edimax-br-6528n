/*
 *  arch/mips/philips/nino/prom.c
 *
 *  Copyright (C) 2001 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *  
 *  Early initialization code for the Philips Nino
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/page.h>

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include <asm/rtl865x/rtl_types.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>
#endif

char arcs_cmdline[CL_SIZE];

#ifdef CONFIG_FB_TX3912
extern unsigned long tx3912fb_paddr;
extern unsigned long tx3912fb_vaddr;
extern unsigned long tx3912fb_size;
#endif

const char *get_system_type(void)
{
	return "Philips Nino";
}

/* Do basic initialization */
void __init prom_init(int argc, char **argv, unsigned long magic, int *prom_vec)
{
	unsigned long mem_size;

	//strcpy(arcs_cmdline, "console=tty0 console=ttyS0,115200");
#ifdef  CONFIG_RTK_MTD_ROOT
#ifdef  CONFIG_RTL8196B_GW_MP
	strcat(arcs_cmdline,"root=/dev/ram console=0 single");
#else
	strcat(arcs_cmdline,"root=/dev/mtdblock1 console=0 single");
#endif
#else
	strcat(arcs_cmdline,"root=/dev/ram console=0 ramdisk_start=0 single");
#endif
	mips_machgroup = MACH_GROUP_PHILIPS;
	mips_machtype = MACH_PHILIPS_NINO;

#ifdef CONFIG_RTL_EB8186
#ifdef CONFIG_NINO_4MB
	mem_size = 4 << 20;
#elif CONFIG_NINO_8MB
	mem_size = 8 << 20;
#elif CONFIG_NINO_16MB
	mem_size = 16 << 20;
#elif CONFIG_NINO_32MB
	mem_size = 32 << 20;
#endif
#endif

#ifdef CONFIG_RTL865X
	unsigned int MCRsdram;
	switch ( MCRsdram = ( REG32( MCR ) & 0x1C100010 ) )
	{
		/* SDRAM 16-bit mode */
		case 0x00000000: mem_size =  2<<20; break;
		case 0x04000000: mem_size =  4<<20; break;
		case 0x08000000: mem_size =  8<<20; break;
		case 0x0C000000: mem_size = 16<<20; break;
		case 0x10000000: mem_size = 32<<20; break;
		case 0x14000000: mem_size = 64<<20; break;

		/* SDRAM 16-bit mode - 2 chip select */
		case 0x00000010: mem_size =  4<<20; break;
		case 0x04000010: mem_size =  8<<20; break;
		case 0x08000010: mem_size = 16<<20; break;
		case 0x0C000010: mem_size = 32<<20; break;
		case 0x10000010: mem_size = 64<<20; break;
		case 0x14000010: mem_size = 128<<20; break;

		/* SDRAM 32-bit mode */
		case 0x00100000: mem_size =  4<<20; break;
		case 0x04100000: mem_size =  8<<20; break;
		case 0x08100000: mem_size = 16<<20; break;
		case 0x0C100000: mem_size = 32<<20; break;
		case 0x10100000: mem_size = 64<<20; break;
		case 0x14100000: mem_size =128<<20; break;

		/* SDRAM 32-bit mode - 2 chip select */
		case 0x00100010: mem_size =  8<<20; break;
		case 0x04100010: mem_size = 16<<20; break;
		case 0x08100010: mem_size = 32<<20; break;
		case 0x0C100010: mem_size = 64<<20; break;
		case 0x10100010: mem_size =128<<20; break;
		/*
		case 0x14100010: mem_size =256<<20; break;
		*/

		default:
			printk( "SDRAM unknown(0x%08X)", MCRsdram ); 
			mem_size = 0;
			break;
	}
#endif	

#if defined(CONFIG_RTL8196B)
#define DCR                     0xB8001004    /* DRAM configuration register */
	unsigned int DCRsdram;
	switch ( DCRsdram = ( REG32( DCR ) & 0x37C80000 ) )
	{
		/* SDRAM 16-bit mode */
		case 0x12080000: mem_size =  8<<20; break; //8MB sdram size
		case 0x12480000: mem_size = 16<<20; break; //16MB sdram size
		case 0x14480000: mem_size = 32<<20; break; //32MB sdram size
		case 0x14880000: mem_size = 64<<20; break; //64MB sdram size
		default:
			printk( "SDRAM unknown(0x%08X)", DCRsdram ); 
			mem_size = 0;
			break;
	}
#endif

#ifdef CONFIG_RTL8197B_PANA
	mem_size = (7 << 20)-16; //reserve 16 byte for firmware header
#endif	
	
#if defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_AP_CLIENT)
	mem_size = (8 << 20);
#endif	
	
	add_memory_region(0, mem_size, BOOT_MEM_RAM); 
}

void __init prom_free_prom_memory (void)
{
}
