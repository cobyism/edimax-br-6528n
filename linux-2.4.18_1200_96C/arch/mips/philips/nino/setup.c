/*
 *  arch/mips/philips/nino/setup.c
 *
 *  Copyright (C) 2001 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Interrupt and exception initialization for Philips Nino
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/reboot.h>
#include <asm/time.h>

#ifdef CONFIG_RTL_EB8186
#include <asm/rtl8181.h>
#endif

#ifdef CONFIG_RTL865X
#include <asm/rtl865x/rtl_types.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>
#else
#include <linux/netdevice.h>
#endif

#ifdef CONFIG_BLK_DEV_IDE
#include <linux/ide.h>
#endif

#define EARLY_PRINTK_ENABLE

#ifdef CONFIG_RTL865X
extern  void shutdown_netdev();

#else
static void shutdown_netdev()
{
	struct net_device *dev;
	printk("Shutdown network interface\n");
	read_lock(&dev_base_lock);
      	for (dev = dev_base; dev != NULL; dev = dev->next) {
		if (dev->flags&IFF_UP && dev->stop){
			printk("%s:===>\n",dev->name);
			rtnl_lock();
			dev_close(dev);
			rtnl_unlock();
		}
      	}
	read_unlock(&dev_base_lock);
}
#endif

static void nino_machine_restart(char *command)
{
	unsigned long flags;
	static void (*back_to_prom)(void) = (void (*)(void)) 0xbfc00000;
#ifdef CONFIG_RTL_EB8186	
	outl(0,GIMR0);  
#endif

#ifdef CONFIG_RTL865X
	REG32(GIMR)=0;	
#endif

	save_flags(flags); cli();
	shutdown_netdev();
	printk("Enable Watch Dog to Reset whole system\n");

#ifdef CONFIG_RTL_EB8186
	*(volatile unsigned long *)(0xBD010058)=0x10; // this is to enable watch dog reset
	*(volatile unsigned long *)(0xBD01005c)=0x00; // this is to enable watch dog reset
#endif

#ifdef CONFIG_RTL865X
	*(volatile unsigned long *)(0xB800311c)=0; /*this is to enable 865xc watch dog reset*/
#endif
	
	for(;;);
	/* Reboot */
	back_to_prom();
}

static void nino_machine_halt(void)
{
	printk("Nino halted.\n");
	while(1);
}

static void nino_machine_power_off(void)
{
	printk("Nino halted. Please turn off power.\n");
	while(1);
}

static void __init nino_board_init()
{
	/*
	 * Set up the master clock module. The value set in
	 * the Clock Control Register by WindowsCE is 0x00432ba.
	 * We set a few values here and let the device drivers
	 * handle the rest.
	 *
	 * NOTE: The UART clocks must be enabled here to provide
	 *       enough time for them to settle.
	 */
}

static __init void nino_time_init(void)
{
}

static __init void nino_timer_setup(struct irqaction *irq)
{
#ifdef CONFIG_RTL_EB8186	
	irq->dev_id = (void *) irq;
	setup_irq(0, irq);

    outl(0x0, TCCNR);
    outl(0, TC0DATA); // what does this counter mean?
    outl(0, TC1DATA); // what does this counter mean?
    outl(0, TC2DATA); // what does this counter mean?
    outl(0, TC3DATA); // what does this counter mean?
    //outl(0x30D40, TC0DATA); // what does this counter mean?
    outl(0x35B60, TC0DATA); // correct setting for 200MHz
    //outl(0x35B6, TC0DATA); // what does this counter mean?
    //outl(0x30D4, TC0DATA); // what does this counter mean?
   
#ifdef CONFIG_RTL8181_WTDOG 
    *(volatile unsigned long *)(0xbd010058)=0xfff; // watch dog time-out for about 12 s
    *(volatile unsigned long *)(0xbd01005c)=0x700;
#endif    

    outl(0x3, TCCNR);
    outl(0x01, TCIR);
#endif //CONFIG_RTL_EB8186

#ifdef CONFIG_RTL865X
	int c;
	unsigned int cpu_clock_rate;

	cpu_clock_rate = 200000000;	/* HS1 clock : 200 MHz */

	irq->dev_id = (void *) irq;

	setup_irq(9, irq);
	REG32(TCCNR) = 0; /* disable timer before setting CDBR */
	REG32(CDBR)=(DIVISOR*6) << DIVF_OFFSET;
	REG32(TC0DATA) = (((cpu_clock_rate/(DIVISOR*6))/HZ)) << TCD_OFFSET;	

#ifdef CONFIG_RTL865X_WTDOG
	*(volatile unsigned long *)(0xb800311C)=0x00600000;
#endif

    /* We must wait n cycles for timer to re-latch the new value of TC1DATA. */
	for(c = 0; c < DIVISOR; c++ );
	
	REG32(TCCNR) = TC0EN | TC0MODE_TIMER;
	REG32(TCIR)=TC0IE;
#endif // CONFIG_RTL865X
}

#ifdef EARLY_PRINTK_ENABLE
extern void serial_console_init();
static int __init early_printk_setup(char* arg) 
{ 
 /* early printk only works for serial ports now */ 
 console_setup(arg); 
 serial_console_init(); 
 
 printk( "early printk enabled \n" ); 
 return 1; 
} 
#endif

extern struct ide_ops std_ide_ops;

#ifdef CONFIG_RTL865X
void timer_ack()
{	
	REG32(TCIR) |= TC0IP;
}
#endif

void __init nino_setup(void)
{
	extern void nino_irq_setup(void);
	//clear GIMR first!

	irq_setup = nino_irq_setup;
	
#ifdef CONFIG_RTL_EB8186	
	set_io_port_base(KSEG1ADDR(0x1d010000));
	outl(0,GIMR0);	
	//mips_io_port_base=RTL8181_REG_BASE;
#endif	

#ifdef CONFIG_RTL865X
	extern void (*mips_timer_ack)(void);
	set_io_port_base(KSEG1ADDR(0x18000000));

	REG32(GIMR)=0;	
	REG32(IRR)=0;
	REG32(IRR1)=0;
	mips_timer_ack=timer_ack;	
#endif

	_machine_restart = nino_machine_restart;
	_machine_halt = nino_machine_halt;
	_machine_power_off = nino_machine_power_off;

	board_time_init = nino_time_init;
	board_timer_setup = nino_timer_setup;

#ifdef CONFIG_FB
	conswitchp = &dummy_con;
#endif

	nino_board_init();

#ifdef EARLY_PRINTK_ENABLE
	//sc_yang early printk 
	early_printk_setup("0");
#endif

#ifdef CONFIG_BLK_DEV_IDE
        ide_ops = &std_ide_ops;
#endif

}
