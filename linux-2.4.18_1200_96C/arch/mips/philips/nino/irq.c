/*
 *  arch/mips/philips/nino/irq.c
 *
 *  Copyright (C) 2001 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Interrupt service routines for Philips Nino
 */
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/mipsregs.h>

#ifdef CONFIG_RTL_EB8186
#include <asm/rtl8181.h>
#endif

#ifdef CONFIG_RTL865X
#include <asm/rtl865x/interrupt.h>
#include <asm/rtl865x/rtl_types.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>

static struct {
	int ext;
	int base;
	int idx;
} irqRoute[]={
	{ 9,4, 3 }, // 0:GPIO E,F,G,H
	{ 8,0, 3 }, // 1:GPIO A,B,C,H
	{ 7,28, 3 }, // 2: SW_Core
	{ 6,24, 4 }, // 3:PCI
	{ 5,20, 3 }, // 4:UART1
	{ 4,16, 3 }, // 5:UART0
	{ 3,12, 3 }, // 6. PCM
	{ 2,8, 3 }, // 7. USB Host
	{ 1,4, 3 },   // 8:Timer1
	{ 0,0, 3 },   // 9:Timer0
};
#endif

#ifdef CONFIG_RTL_EB8186
#define ALLINTS (IE_IRQ0 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)
#endif

#ifdef CONFIG_RTL865X
#define ALLINTS (IE_IRQ0 | IE_IRQ1 |IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)
#endif

static void  unmask_irq(unsigned int irq)
{
#ifdef CONFIG_RTL_EB8186
	outl((inl(GIMR0) | (1 << irq)),GIMR0);
    inl(GIMR0);
#endif

#ifdef CONFIG_RTL865X
#ifdef CONFIG_RTK_VOIP		
	if (irq == 6) // PCM
		REG32(GIMR) = (REG32(GIMR)) | (1 << (25-irq));
	else
#endif
	REG32(GIMR) = (REG32(GIMR)) | (1 << (17-irq));  
	if ( (irq == 0) || (irq == 1) || (irq == 6) )
		REG32(IRR2)|= ((irqRoute[irq].idx & 0xF)<<irqRoute[irq].base);
	else
		REG32(IRR1)|= ((irqRoute[irq].idx & 0xF)<<irqRoute[irq].base);
#endif	
}

static void  mask_irq(unsigned int irq)
{
#ifdef CONFIG_RTL_EB8186
	outl(inl(GIMR0) & (~(1 << irq)),GIMR0);
    inl(GIMR0);
#endif

#ifdef CONFIG_RTL865X
#ifdef CONFIG_RTK_VOIP		
	if (irq == 6) // PCM
		REG32(GIMR)=(REG32(GIMR)) & (~(1 << (25-irq)));
	else
#endif
	REG32(GIMR)=(REG32(GIMR)) & (~(1 << (17-irq)));
#endif	
}

extern asmlinkage void do_IRQ(int irq, struct pt_regs *regs);

static void enable_irq(unsigned int irq)
{

	return 0;
}

static unsigned int startup_irq(unsigned int irq)
{
	unmask_irq(irq);
	return 0;		/* Never anything pending  */
}

static void disable_irq(unsigned int irq)
{

	return 0;
}

#define shutdown_irq		disable_irq

static void mask_and_ack_irq(unsigned int irq)
{

	return 0;
}
static void end_irq(unsigned int irq)
{
	return 0;
}

static struct hw_interrupt_type irq_type = {
	"MIPS",
	startup_irq,
	shutdown_irq,
	enable_irq,
	disable_irq,
	mask_and_ack_irq,
	end_irq,
	NULL
};

void irq_dispatch(int irq_nr, struct pt_regs *regs)
{

#ifdef CONFIG_RTL_EB8186
   int i,j,irq;
   volatile unsigned int gimr, gisr,irq_x;  
    
    gimr = inl(GIMR0);
    // for(j=0; j<=1; j++)
    {	    
	gisr = inl(GISR);
	irq_x = (gimr & gisr & 0xff);
	irq=0;	
	//if (irq_x == 0)
       	//	break;
        for (i=0; i<=7; i++)
    	{
        	if (irq_x & 0x01)
		{
			do_IRQ(irq, regs);
		}  
        	irq++;
        	irq_x = irq_x >> 1;
    	}
     }
#endif

#ifdef CONFIG_RTL865X
	unsigned int gimr, gisr,irq_x;  

	gimr = REG32(GIMR);
	REG32(GIMR)&= ~0x0000C000;

	gisr = REG32(GISR);
	irq_x = (gimr & gisr);

	do{
		if( irq_x & 0x00000100)
			do_IRQ(9, regs); //Timer
#ifdef CONFIG_RTK_VOIP
		else if( irq_x & 0x00080000)
			do_IRQ(6, regs); //PCM
#endif
		else if( irq_x & 0x00004000)
			do_IRQ(3, regs); //PCI
		else if( irq_x & 0x00008000)
			do_IRQ(2, regs); //NIC
		else if( irq_x & 0x00010000) 
			do_IRQ(1, regs); //GPIO A,B,C,D
		else if( irq_x & 0x00020000) 
			do_IRQ(0, regs); //GPIO E,F,G,H			
		else if( irq_x & 0x00001000)
			do_IRQ(5, regs); //uart0
		else if( irq_x & 0x00002000)
			do_IRQ(4, regs); //uart1
		else if( irq_x & 0x00000400)
			do_IRQ(7, regs); //USB			
#ifndef CONFIG_RTK_VOIP
		else if( irq_x & 0x00080000)
			do_IRQ(6, regs); //PCM
#endif
		else
		{
			printk("$$$ Unknown IRQ $$$  0x%08x\n",irq_x);
		}
		gisr = REG32(GISR);
		irq_x = (gimr & gisr);
	}while(irq_x);

	REG32(GIMR)=gimr;

#endif
}

void __init nino_irq_setup(void)
{
	extern asmlinkage void ninoIRQ(void);

	unsigned int i;

	/* Disable all hardware interrupts */
	change_cp0_status(ST0_IM, 0x00);

	/* Initialize IRQ vector table */
	//init_generic_irq();

	/* Initialize IRQ action handlers */
	for (i = 0; i < 16; i++) {
		hw_irq_controller *handler = NULL;
		handler		= &irq_type;

		irq_desc[i].status	= IRQ_DISABLED;
		irq_desc[i].action	= 0;
		irq_desc[i].depth	= 1;
		irq_desc[i].handler	= handler;
	}

	/* Set up the external interrupt exception vector */
	set_except_vector(0, ninoIRQ);

	/* Enable all interrupts */
	change_cp0_status(ST0_IM, ALLINTS);
}

void (*irq_setup)(void);

void __init init_IRQ(void)
{
	/* Invoke board-specific irq setup */
	irq_setup();
	
#ifdef CONFIG_REMOTE_DEBUG
	extern void breakpoint(void);
	extern void set_debug_traps(void);
	extern void rs_kgdb_hook(void);
	rs_kgdb_hook();
	clear_cp0_status(STATUSF_IP2);
	clear_cp0_status(STATUSF_IP3);
	clear_cp0_status(STATUSF_IP4);
	clear_cp0_status(STATUSF_IP5);
	clear_cp0_status(STATUSF_IP6);
	clear_cp0_status(STATUSF_IP7);	
	printk("Wait for gdb client connection ...\n");
	set_debug_traps();
	set_cp0_status(STATUSF_IP2);
	set_cp0_status(STATUSF_IP3);
	set_cp0_status(STATUSF_IP4);
	set_cp0_status(STATUSF_IP5);
	set_cp0_status(STATUSF_IP6);
	set_cp0_status(STATUSF_IP7);	
	breakpoint();
#endif	
	
}
