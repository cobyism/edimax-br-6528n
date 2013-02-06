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

#include <asm/sibyte/64bit.h>
#include <asm/sibyte/sb1250.h>
#include <asm/sibyte/sb1250_regs.h>
#include <asm/addrspace.h>
#include <asm/smp.h>
#include <linux/sched.h>

/*
 * These are routines for dealing with the sb1250 smp capabilities
 * independent of board/firmware
 */

static u64 mailbox_set_regs[] = {
	KSEG1 + A_IMR_CPU0_BASE + R_IMR_MAILBOX_SET_CPU, 
	KSEG1 + A_IMR_CPU1_BASE + R_IMR_MAILBOX_SET_CPU
};

static u64 mailbox_clear_regs[] = {
	KSEG1 + A_IMR_CPU0_BASE + R_IMR_MAILBOX_CLR_CPU, 
	KSEG1 + A_IMR_CPU1_BASE + R_IMR_MAILBOX_CLR_CPU
};

static u64 mailbox_regs[] = {
	KSEG1 + A_IMR_CPU0_BASE + R_IMR_MAILBOX_CPU, 
	KSEG1 + A_IMR_CPU1_BASE + R_IMR_MAILBOX_CPU
};


/*
 * Simple enough; everything is set up, so just poke the appropriate mailbox
 * register, and we should be set
 */
void core_send_ipi(int cpu, unsigned int action)
{
	out64((((u64)action)<< 48), mailbox_set_regs[cpu]);
}


void sb1250_smp_finish(void)
{
	extern void sb1_sanitize_tlb(void);

	sb1_sanitize_tlb();
	sb1250_time_init();
}

void sb1250_mailbox_interrupt(struct pt_regs *regs)
{
	int cpu = smp_processor_id();
	unsigned int action;
	
	/* Load the mailbox register to figure out what we're supposed to do */
	action = (in64(mailbox_regs[cpu]) >> 48) & 0xffff;

	/* Clear the mailbox to clear the interrupt */
	out64(((u64)action)<<48, mailbox_clear_regs[cpu]);

	/*
	 * Nothing to do for SMP_RESCHEDULE_YOURSELF; returning from the
	 * interrupt will do the reschedule for us
	 */

	if (action & SMP_CALL_FUNCTION) {
		smp_call_function_interrupt();
	}
}
