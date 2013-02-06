/*
 * Copyright (C) 2001 Broadcom Corporation
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

#ifndef _SWARM_IDE_H
#define _SWARM_IDE_H

#include <asm/sibyte/sb1250_int.h>

#define SWARM_IDE_BASE        (0xb00b0000-mips_io_port_base)
#define SWARM_IDE_REG(pcaddr) (SWARM_IDE_BASE + ((pcaddr)<<5))
#define SWARM_IDE_INT         (K_INT_GPIO_4)

extern ide_ideproc_t swarm_ideproc;

#define swarm_outb(val,port)							\
do {									\
	*(volatile u8 *)(mips_io_port_base + (port)) = val;	\
} while(0)

#define swarm_outw(val,port)							\
do {									\
	*(volatile u16 *)(mips_io_port_base + (port)) = val;	\
} while(0)

#define swarm_outl(val,port)							\
do {									\
	*(volatile u32 *)(mips_io_port_base + (port)) = val;\
} while(0)

static inline unsigned char swarm_inb(unsigned long port)
{
	return (*(volatile u8 *)(mips_io_port_base + port));
}

static inline unsigned short swarm_inw(unsigned long port)
{
	return (*(volatile u16 *)(mips_io_port_base + port));
}

static inline unsigned int swarm_inl(unsigned long port)
{
	return (*(volatile u32 *)(mips_io_port_base + port));
}


static inline void swarm_outsb(unsigned long port, void *addr, unsigned int count)
{
	while (count--) {
		swarm_outb(*(u8 *)addr, port);
		addr++;
	}
}

static inline void swarm_insb(unsigned long port, void *addr, unsigned int count)
{
	while (count--) {
		*(u8 *)addr = swarm_inb(port);
		addr++;
	}
}

static inline void swarm_outsw(unsigned long port, void *addr, unsigned int count)
{
	while (count--) {
		swarm_outw(*(u16 *)addr, port);
		addr += 2;
	}
}

static inline void swarm_insw(unsigned long port, void *addr, unsigned int count)
{
	while (count--) {
		*(u16 *)addr = swarm_inw(port);
		addr += 2;
	}
}

static inline void swarm_outsl(unsigned long port, void *addr, unsigned int count)
{
	while (count--) {
		swarm_outl(*(u32 *)addr, port);
		addr += 4;
	}
}

static inline void swarm_insl(unsigned long port, void *addr, unsigned int count)
{
	while (count--) {
		*(u32 *)addr = swarm_inl(port);
		addr += 4;
	}
}

#endif
