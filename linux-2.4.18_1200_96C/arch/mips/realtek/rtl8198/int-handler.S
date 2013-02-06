/*
 *  arch/mips/philips/nino/int-handler.S
 *
 *  Copyright (C) 2001 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Interrupt exception dispatch code for Philips Nino
 */
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#ifdef CONFIG_RTL_EB8186        
#include <asm/rtl8181.h>
#endif

	.text
#ifdef CONFIG_RTL_EB8186  	
	.section ".irq_speedup"
#endif

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	.section .iram-gen
#endif	
	.set	noreorder
	.set	noat
	.align	5
	NESTED(ninoIRQ, PT_SIZE, sp)
	SAVE_ALL
	CLI
	.set	at
#if 0
	mfc0	s0, CP0_CAUSE		# determine cause

	andi	a0, s0, CAUSEF_IP6
	beq	a0, zero, 1f
	andi	a0, s0, CAUSEF_IP4	# delay slot
	move    a0, sp
	jal	irq6_dispatch
	nop				# delay slot
	j	ret_from_irq
	nop				# delay slot

	move	a0, sp
	j	irq_bad
	nop
#endif
		
        mfc0    t0,CP0_CAUSE        # get pending interrupts
        nop
        mfc0    t2,CP0_STATUS
        nop
        and     t0,t2           # isolate allowed ones
        andi    t2,t0, 0xfc00
       #if 0 
	la      t0, (GISR + RTL8181_REG_BASE)
        la      t1, (GIMR0 + RTL8181_REG_BASE)
        lw      a0, 0(t0)
        lw      a1, 0(t1)
        nop
        and     a0,a1
        andi    a0, 0xff
	#endif
        bne     t2,zero,handle_it
        nop
#ifdef CONFIG_RTL_EB8186        
#        la      t0,( UART_THR + RTL8181_REG_BASE)
#        li      t1,'m'
#        sb      t1, 0(t0)
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#        la      t0, 0xB8002000
#        li      t1,'m'
#        sb      t1, 0(t0)
#endif        
        j       ret_from_irq
        nop

handle_it:  jal irq_dispatch
         move   a1,sp

        j   ret_from_irq
		nop
	END(ninoIRQ)
