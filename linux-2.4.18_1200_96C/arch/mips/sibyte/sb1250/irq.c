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
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/errno.h>
#include <asm/signal.h>
#include <asm/system.h>
#include <asm/ptrace.h>
#include <asm/sibyte/sb1250_regs.h>
#include <asm/sibyte/sb1250_int.h>
#include <asm/sibyte/sb1250_scd.h>
#include <asm/sibyte/sb1250.h>
#include <asm/sibyte/64bit.h>

/*
 * These are the routines that handle all the low level interrupt stuff. 
 * Actions handled here are: initialization of the interrupt map, requesting of
 * interrupt lines by handlers, dispatching if interrupts to handlers, probing
 * for interrupt lines 
 */


#define shutdown_sb1250_irq	disable_sb1250_irq
static void end_sb1250_irq(unsigned int irq);
static void enable_sb1250_irq(unsigned int irq);
static void disable_sb1250_irq(unsigned int irq);
static unsigned int startup_sb1250_irq(unsigned int irq);
static void ack_sb1250_irq(unsigned int irq);

#ifdef CONFIG_REMOTE_DEBUG
extern void breakpoint(void);
#endif

#define NR_IRQS 64

static struct hw_interrupt_type sb1250_irq_type = {
	"SB1250-IMR",
	startup_sb1250_irq,
	shutdown_sb1250_irq,
	enable_sb1250_irq,
	disable_sb1250_irq,
	ack_sb1250_irq,
	end_sb1250_irq,
	NULL
};

spinlock_t sb1250_imr_lock = SPIN_LOCK_UNLOCKED;

void sb1250_mask_irq(int cpu, int irq)
{
	unsigned long flags;
	u64 cur_ints;

	spin_lock_irqsave(&sb1250_imr_lock, flags);
	cur_ints = in64(KSEG1 + A_IMR_MAPPER(cpu) + R_IMR_INTERRUPT_MASK);
	cur_ints |= (((u64) 1) << irq);
	out64(cur_ints, KSEG1 + A_IMR_MAPPER(cpu) + R_IMR_INTERRUPT_MASK);
	spin_unlock_irqrestore(&sb1250_imr_lock, flags);
}

void sb1250_unmask_irq(int cpu, int irq)
{
	unsigned long flags;
	u64 cur_ints;

	spin_lock_irqsave(&sb1250_imr_lock, flags);
	cur_ints = in64(KSEG1 + A_IMR_MAPPER(cpu) + R_IMR_INTERRUPT_MASK);
	cur_ints &= ~(((u64) 1) << irq);
	out64(cur_ints, KSEG1 + A_IMR_MAPPER(cpu) + R_IMR_INTERRUPT_MASK);
	spin_unlock_irqrestore(&sb1250_imr_lock, flags);
}



/* Defined in arch/mips/sibyte/sb1250/irq_handler.S */
extern void sb1250_irq_handler(void);

/*****************************************************************************/

static unsigned int startup_sb1250_irq(unsigned int irq)
{
	sb1250_unmask_irq(0, irq);

	return 0;		/* never anything pending */
}


static void disable_sb1250_irq(unsigned int irq)
{
	sb1250_mask_irq(0, irq);
}

static void enable_sb1250_irq(unsigned int irq)
{
	sb1250_unmask_irq(0, irq);
}


static void ack_sb1250_irq(unsigned int irq)
{
	u64 pending;

	/*
	 * If the interrupt was an LDT interrupt, now is the time
	 * to clear it.
	 */
	pending = in64(KSEG1 + A_IMR_REGISTER(0,R_IMR_LDT_INTERRUPT));
	pending &= ((u64)1 << (irq));
	if (pending)
		out64(pending, KSEG1+A_IMR_REGISTER(0,R_IMR_LDT_INTERRUPT_CLR));

	sb1250_mask_irq(0, irq);
}


static void end_sb1250_irq(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS))) {
		sb1250_unmask_irq(0, irq);
	}
}


void __init init_sb1250_irqs(void)
{
	int i;

	for (i = 0; i < NR_IRQS; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = 0;
		irq_desc[i].depth = 1;
		irq_desc[i].handler = &sb1250_irq_type;
	}
}


/*
 *  init_IRQ is called early in the boot sequence from init/main.c.  It
 *  is responsible for setting up the interrupt mapper and installing the
 *  handler that will be responsible for dispatching interrupts to the
 *  "right" place. 
 */
/*
 * For now, map all interrupts to IP[2].  We could save
 * some cycles by parceling out system interrupts to different
 * IP lines, but keep it simple for bringup.  We'll also direct
 * all interrupts to a single CPU; we should probably route
 * PCI and LDT to one cpu and everything else to the other
 * to balance the load a bit. 
 * 
 * On the second cpu, everything is set to IP5, which is
 * ignored, EXCEPT the mailbox interrupt.  That one is 
 * set to IP[2] so it is handled.  This is needed so we
 * can do cross-cpu function calls, as requred by SMP
 */

#define IMR_IP2_VAL	K_INT_MAP_I0
#define IMR_IP3_VAL	K_INT_MAP_I1
#define IMR_IP4_VAL	K_INT_MAP_I2

void __init init_IRQ(void)
{

	unsigned int i;
	u64 tmp;

	/* Default everything to IP2 */
	for (i = 0; i < NR_IRQS; i++) {	/* was I0 */
		out64(IMR_IP2_VAL,
		      KSEG1 + A_IMR_REGISTER(0,
					     R_IMR_INTERRUPT_MAP_BASE) +
		      (i << 3));
		out64(IMR_IP2_VAL,
		      KSEG1 + A_IMR_REGISTER(1,
					     R_IMR_INTERRUPT_MAP_BASE) +
		      (i << 3));
	}

	init_sb1250_irqs();

	/*
	 * Map the high 16 bits of the mailbox registers to IP[3], for
	 * inter-cpu messages
	 */
	/* Was I1 */
	out64(IMR_IP3_VAL, KSEG1 + A_IMR_REGISTER(0, R_IMR_INTERRUPT_MAP_BASE) +
	                   (K_INT_MBOX_0 << 3));
	out64(IMR_IP3_VAL, KSEG1 + A_IMR_REGISTER(1, R_IMR_INTERRUPT_MAP_BASE) +
	                   (K_INT_MBOX_0 << 3));

	/* Clear the mailboxes.  The firmware may leave them dirty */
	out64(0xffffffffffffffff,
	      KSEG1 + A_IMR_REGISTER(0, R_IMR_MAILBOX_CLR_CPU));
	out64(0xffffffffffffffff,
	      KSEG1 + A_IMR_REGISTER(1, R_IMR_MAILBOX_CLR_CPU));

	/* Mask everything except the mailbox registers for both cpus */
	tmp = ~((u64) 0) ^ (((u64) 1) << K_INT_MBOX_0);
	out64(tmp, KSEG1 + A_IMR_REGISTER(0, R_IMR_INTERRUPT_MASK));
	out64(tmp, KSEG1 + A_IMR_REGISTER(1, R_IMR_INTERRUPT_MASK));

	/*
	 * Note that the timer interrupts are also mapped, but this is 
	 * done in sb1250_time_init()
	 */

#ifdef CONFIG_SIBYTE_SB1250_PROF
	/* Enable IP[7,4:0], disable the rest */
	clear_cp0_status(0x6000);
	set_cp0_status(0x9f00);
#else
	/* Enable IP[4:0], disable the rest */
	clear_cp0_status(0xe000);
	set_cp0_status(0x1f00);
#endif
	set_except_vector(0, sb1250_irq_handler);

#ifdef CONFIG_REMOTE_DEBUG
	/* If local serial I/O used for debug port, enter kgdb at once */
//      puts("Waiting for kgdb to connect...");
	set_debug_traps();
	breakpoint();
#endif
}
