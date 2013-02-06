/* hfload.c
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License.  See the file "COPYING" in the main
 * directory of this archive for more details.
 *
 * Copyright (C) 2000, Jay Carlson
 */

/*
 * Boot loader main program.
 */
#include <linux/config.h>

#include <unistd.h>
#include <linux/elf.h>
#include "hfload.h"

#define REG32(reg)   (*(volatile unsigned int   *)((unsigned int)reg))

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	#define BASE_ADDR 0xB8000000
	#define set_io_port_base(base)	\
		do { * (unsigned long *) &mips_io_port_base = (base); } while (0)	
	
	const unsigned long mips_io_port_base;
#endif


int file_offset;

int old_stack_pointer;

#define MAX_PHDRS_SIZE 8

Elf32_Ehdr header;
Elf32_Phdr phdrs[MAX_PHDRS_SIZE];

extern void flush_cache(void);

void
zero_region(char *start, char *end)
{
	char *addr;
	int count;

	count = end - start;
#ifndef __DO_QUIET__
	printf("zeroing from %08x to to %08x, 0x%x bytes\n", start, end, count);
#endif

#ifndef FAKE_COPYING
	memset(start, 0, count);
#endif
}

void
load_phdr(Elf32_Phdr *phdr)
{
	char *addr, *end;
	
	seek_forward(phdr->p_offset);
	
	addr = (char *)phdr->p_vaddr;
	end = ((char *)addr) + phdr->p_memsz;
	
	copy_to_region(addr, phdr->p_filesz);
	
	addr = ((char *)addr) + phdr->p_filesz;
	
	zero_region(addr, end);
}

void main(unsigned long stack_start_addr)
{
	int i;
	file_offset = 0;

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
 	set_io_port_base(BASE_ADDR);
#endif

#ifndef __DO_QUIET__
	printf("decompressing kernel:\n");
#endif

#ifdef CONFIG_RTL8197B_PANA
	extern int is_vmlinux_checksum_ok();
	
	REG32(0xB801900C) =  REG32(0xB801900C) & (~0x0400); //SYSSR, AllSoftwareReady=0
	if (!is_vmlinux_checksum_ok()) {
		printf("Linux image corrupted!\n");
		for (;;);
	}
#endif

#ifndef BZ2_COMPRESS
	decompress_kernel(UNCOMPRESS_OUT, stack_start_addr+4096, FREEMEM_END, 0);
#else
	decompress_kernel(UNCOMPRESS_OUT, stack_start_addr+4096, FREEMEM_END, 0);
#endif

#ifndef __DO_QUIET__
	printf("done decompressing kernel.\n");
#endif

	flush_cache();
	start_kernel(0x80000000);
}
