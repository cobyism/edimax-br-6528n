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
 * These are routines to set up and handle interrupts from the
 * sb1250 general purpose timer 0.  We're using the timer as a
 * system clock, so we set it up to run at 100 Hz.  On every
 * interrupt, we update our idea of what the time of day is, 
 * then call do_timer() in the architecture-independent kernel
 * code to do general bookkeeping (e.g. update jiffies, run
 * bottom halves, etc.)
 */

#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/spinlock.h>

#include <asm/irq.h>
#include <asm/ptrace.h>
#include <asm/addrspace.h>
#include <asm/sibyte/sb1250.h>
#include <asm/sibyte/sb1250_regs.h>
#include <asm/sibyte/sb1250_int.h>
#include <asm/sibyte/sb1250_scd.h>
#include <asm/sibyte/64bit.h>


void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);



#define IMR_IP2_VAL	K_INT_MAP_I0
#define IMR_IP3_VAL	K_INT_MAP_I1
#define IMR_IP4_VAL	K_INT_MAP_I2

void sb1250_time_init(void)
{
	int cpu = smp_processor_id();

	/* Only have 4 general purpose timers */
	if (cpu > 3) {
		BUG();
	}

	sb1250_mask_irq(cpu, K_INT_TIMER_0 + cpu);
	
	/* Map the timer interrupt to ip[4] of this cpu */
	out64(IMR_IP4_VAL, KSEG1 + A_IMR_REGISTER(cpu, R_IMR_INTERRUPT_MAP_BASE) 
	      + ((K_INT_TIMER_0 + cpu)<<3)); 

	/* the general purpose timer ticks at 1 Mhz independent if the rest of the system */
	/* Disable the timer and set up the count */
	out64(0, KSEG1 + A_SCD_TIMER_REGISTER(cpu, R_SCD_TIMER_CFG));
	out64(
#ifndef CONFIG_SIMULATION
		1000000/HZ
#else
		50000/HZ
#endif
		, KSEG1 + A_SCD_TIMER_REGISTER(cpu, R_SCD_TIMER_INIT));

	/* Set the timer running */
	out64(M_SCD_TIMER_ENABLE|M_SCD_TIMER_MODE_CONTINUOUS,
	      KSEG1 + A_SCD_TIMER_REGISTER(cpu, R_SCD_TIMER_CFG));

	sb1250_unmask_irq(cpu, K_INT_TIMER_0 + cpu);
	/*
	 * This interrupt is "special" in that it doesn't use the request_irq
	 * way to hook the irq line.  The timer interrupt is initialized early
	 * enough to make this a major pain, and it's also firing enough to
	 * warrant a bit of special case code.  sb1250_timer_interrupt is
	 * called directly from irq_handler.S when IP[4] is set during an
	 * interrupt
	 */
}

extern int set_rtc_mmss(unsigned long nowtime);
extern rwlock_t xtime_lock;
static long last_rtc_update = 0;

void sb1250_timer_interrupt(struct pt_regs *regs)
{
	int cpu = smp_processor_id();

	/* Reset the timer */
	out64(M_SCD_TIMER_ENABLE|M_SCD_TIMER_MODE_CONTINUOUS,
	      KSEG1 + A_SCD_TIMER_REGISTER(cpu, R_SCD_TIMER_CFG));  

	/*
	 * Need to do some stuff here with xtime, too, but that looks like
	 * it should be architecture independent...does it really belong here?
	 */
	if (!cpu) {
		do_timer(regs);

		read_lock(&xtime_lock);
		if ((time_status & STA_UNSYNC) == 0 
		    && xtime.tv_sec > last_rtc_update + 660 
		    && xtime.tv_usec >= 500000 - (tick >> 1) 
		    && xtime.tv_usec <= 500000 + (tick >> 1)) {
			if (set_rtc_mmss(xtime.tv_sec) == 0)
				last_rtc_update = xtime.tv_sec;
			else
				/* do it again in 60 s */
				last_rtc_update = xtime.tv_sec - 600; 
		}
		read_unlock(&xtime_lock);
	}

#ifdef CONFIG_SMP
	/*
	 * We need to make like a normal interrupt -- otherwise timer
	 * interrupts ignore the global interrupt lock, which would be
	 * a Bad Thing.
	 */
	irq_enter(cpu, 0);
	update_process_times(user_mode(regs));
	irq_exit(cpu, 0);

	if (softirq_pending(cpu))
		do_softirq();
#endif /* CONFIG_SMP */
}
