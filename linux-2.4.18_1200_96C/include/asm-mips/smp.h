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

#ifndef __ASM_MIPS_SMP_H
#define __ASM_MIPS_SMP_H

#include <linux/config.h>

#ifdef CONFIG_SMP

#include <asm/spinlock.h>
#include <asm/atomic.h>
#include <asm/current.h>


/* Mappings are straight across.  If we want
   to add support for disabling cpus and such,
   we'll have to do what the mips64 port does here */
#define cpu_logical_map(cpu)	(cpu)
#define cpu_number_map(cpu)     (cpu)

#define smp_processor_id()  (current->processor)


/* I've no idea what the real meaning of this is */
#define PROC_CHANGE_PENALTY	20

#define NO_PROC_ID	(-1)

struct call_data_struct {
	void		(*func)(void *);
	void		*info;
	atomic_t	started;
	atomic_t	finished;
	int		wait;
};

extern struct call_data_struct *call_data;

/* ipi types that boards must handle, one bit per type   */
#define SMP_RESCHEDULE_YOURSELF	0x1	/* XXX braindead */
#define SMP_CALL_FUNCTION	0x2

#if (NR_CPUS <= _MIPS_SZLONG)

typedef unsigned long   cpumask_t;

#define CPUMASK_CLRALL(p)	(p) = 0
#define CPUMASK_SETB(p, bit)	(p) |= 1 << (bit)
#define CPUMASK_CLRB(p, bit)	(p) &= ~(1ULL << (bit))
#define CPUMASK_TSTB(p, bit)	((p) & (1ULL << (bit)))

#elif (NR_CPUS <= 128)

/*
 * The foll should work till 128 cpus.
 */
#define CPUMASK_SIZE		(NR_CPUS/_MIPS_SZLONG)
#define CPUMASK_INDEX(bit)	((bit) >> 6)
#define CPUMASK_SHFT(bit)	((bit) & 0x3f)

typedef struct {
	unsigned long	_bits[CPUMASK_SIZE];
} cpumask_t;

#define	CPUMASK_CLRALL(p)	(p)._bits[0] = 0, (p)._bits[1] = 0
#define CPUMASK_SETB(p, bit)	(p)._bits[CPUMASK_INDEX(bit)] |= \
					(1ULL << CPUMASK_SHFT(bit))
#define CPUMASK_CLRB(p, bit)	(p)._bits[CPUMASK_INDEX(bit)] &= \
					~(1ULL << CPUMASK_SHFT(bit))
#define CPUMASK_TSTB(p, bit)	((p)._bits[CPUMASK_INDEX(bit)] & \
					(1ULL << CPUMASK_SHFT(bit)))

#else
#error cpumask macros only defined for 128p kernels
#endif

extern cpumask_t cpu_online_map;

#endif /* CONFIG_SMP */
#endif /* __ASM_MIPS_SMP_H */
