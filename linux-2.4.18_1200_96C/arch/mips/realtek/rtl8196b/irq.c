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

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include <asm/rtl865x/interrupt.h>
#include <asm/rtl865x/rtl_types.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>
#if defined(CONFIG_RTL8196B)
#include <asm-mips/rtl865x/platform.h>
#endif

static struct {
	int ext;
	int base;
	int idx;
} irqRoute[]={
#ifdef CONFIG_RTL8196C
	{ 8,4, 3 }, // 1:GPIO A,B,C,H
	{ 7,0, 3 }, // 2: SW_Core
	{ 6,24, 4 }, // 3:PCI
	{ 5,20, 3 }, // 4:UART1
	{ 4,28, 3 }, // 5:UART0
	{ 1,28, 3 },   // 8:Timer1
	{ 0,24, 4 },   // 9:Timer0
#else
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
#endif
};
#endif

#ifdef CONFIG_RTL_EB8186
#define ALLINTS (IE_IRQ0 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)
#endif

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#define ALLINTS (IE_IRQ0 | IE_IRQ1 |IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)
#endif

static void  unmask_irq(unsigned int irq)
{
#ifdef CONFIG_RTL_EB8186
	outl((inl(GIMR0) | (1 << irq)),GIMR0);
    inl(GIMR0);
#endif

#if defined(CONFIG_RTL8196B)
 #ifdef CONFIG_RTL8196C
  if (irq == PCIE_IRQ) {
        REG32(GIMR) = (REG32(GIMR)) | PCIE_IE;
        REG32(IRR1) = REG32(IRR1) | (PCIE_RS << 8);
        return;
    }
  if (irq == 9) {
        REG32(GIMR) = (REG32(GIMR)) | TC0_IE;
        REG32(IRR1) = REG32(IRR1) | (4 << 24);
        return;
    }
   if (irq == 5) {
        REG32(GIMR) = (REG32(GIMR)) | UART0_IE;
        REG32(IRR0) = REG32(IRR0) | ( 7<< 28);
        return;
    }
  if (irq == 2) {
        REG32(GIMR) = (REG32(GIMR)) | SW_IE;
        REG32(IRR1) = REG32(IRR1) | ( 6<< 0);
        return;
    }
#else
    if (irq == PCIE_IRQ) {
        REG32(GIMR) = (REG32(GIMR)) | PCIE_IE;
        REG32(IRR2) = REG32(IRR2) | (PCIE_RS << 20);
        return;
    }
    else if (irq == PCIE2_IRQ) {
        REG32(GIMR) = (REG32(GIMR)) | PCIE2_IE;
        REG32(IRR2) = REG32(IRR2) | (PCIE2_RS << 24);
        return;
    }
#endif
#ifdef CONFIG_RTL8197B_PANA
	else if (irq == HCI_IRQ) { // HCI
		REG32(GIMR) = (REG32(GIMR)) | HCI_IE;
		REG32(IRR2) = REG32(IRR2) | (IE_IRQ3 << 11);
		return;
	}	
#endif    
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#ifdef CONFIG_RTK_VOIP		
	if (irq == 6) // PCM
		REG32(GIMR) = (REG32(GIMR)) | (1 << (25-irq));
	else
#endif
#ifndef CONFIG_RTL8196C
	REG32(GIMR) = (REG32(GIMR)) | (1 << (17-irq));  
	if ( (irq == 0) || (irq == 1) || (irq == 6) )
		REG32(IRR2)|= ((irqRoute[irq].idx & 0xF)<<irqRoute[irq].base);
	//else
	else if (irq < 10) //RTL8196B, michael, irqRoute array has only 10 elements
		REG32(IRR1)|= ((irqRoute[irq].idx & 0xF)<<irqRoute[irq].base);
#endif
#endif
}
//RTL8196B
/*
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
*/
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

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	unsigned int gimr, gisr,irq_x;  

	gimr = REG32(GIMR);
//	REG32(GIMR)&= ~0x0000C000; //michael

	gisr = REG32(GISR);
	irq_x = (gimr & gisr);

	do
	{
#ifdef CONFIG_RTL8196C

		if( irq_x & 0x00004000)
		{
			do_IRQ(9, regs); //Timer
		}
#ifdef CONFIG_RTK_VOIP
		else if( irq_x & 0x00080000)
			do_IRQ(6, regs); //PCM
#endif
                else if( irq_x & PCIE_IE)
                //else if (cpuint_ip & CAUSEF_IP6)
		{
			do_IRQ(PCIE_IRQ, regs); //PCIE
		}
		#if 0
		else if( irq_x & PCIE2_IE)
		//else if (cpuint_ip & CAUSEF_IP4)
		{
			do_IRQ(PCIE2_IRQ, regs); //PCIE2
		}
		#endif
		else if( irq_x & 0x00000100)
			do_IRQ(2, regs); //NIC
		else if( irq_x & 0x00000200) 
		{
#if defined(CONFIG_RTL8196C_KLD)

			if((REG32(0xb8003510)&0x8)==0x8)
			{
				do_IRQ(0, regs);
			}
			else
#endif				
			do_IRQ(1, regs); //GPIO A,B,C,D
		}
			#if 0
		else if( irq_x & 0x00020000) 
			do_IRQ(0, regs); //GPIO E,F,G,H		
			#endif
		else if( irq_x & 0x00000080)
			do_IRQ(5, regs); //uart0
			
		else if( irq_x & 0x00002000)
			do_IRQ(4, regs); //uart1
			#if 0
		else if( irq_x & 0x00000400)
			do_IRQ(7, regs); //USB
			#endif			
#ifdef CONFIG_RTL8197B_PANA
		else if( irq_x & HCI_IE)
			do_IRQ(HCI_IRQ, regs); //HCI
#endif			
		else
		{
			printk("$$$ Unknown IRQ $$$  0x%08x\n",irq_x);
		}
#else
		if( irq_x & 0x00000100)
		{
			do_IRQ(9, regs); //Timer
		}
#ifdef CONFIG_RTK_VOIP
		else if( irq_x & 0x00080000)
			do_IRQ(6, regs); //PCM
#endif
                else if( irq_x & PCIE_IE)
                //else if (cpuint_ip & CAUSEF_IP6)
		{
			do_IRQ(PCIE_IRQ, regs); //PCIE
		}
		else if( irq_x & PCIE2_IE)
		//else if (cpuint_ip & CAUSEF_IP4)
		{
			do_IRQ(PCIE2_IRQ, regs); //PCIE2
		}
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
#ifdef CONFIG_RTL8197B_PANA
		else if( irq_x & HCI_IE)
			do_IRQ(HCI_IRQ, regs); //HCI
#endif			
		else
		{
			printk("$$$ Unknown IRQ $$$  0x%08x\n",irq_x);
		}
#endif
		gisr = REG32(GISR);
		irq_x = (gimr & gisr);
	}while(irq_x);

	REG32(GIMR)=gimr;

#endif
}

#ifdef CONFIG_RTL8197B_PANA
//#define DRAM_CONFIG_VAL 0x54480000  //default: 58080000 //32MB DRAM
#define DRAM_CONFIG_VAL 0x52080000  //default: 58080000   //8MB DRAM
//#define DRAM_TIMING_VAL 0xffff05c0  //default: FFFF0FC0
#define DRAM_TIMING_VAL 0x6cea0a80
#define DRAM_CONFIG_VALH ((DRAM_CONFIG_VAL>>16)&0x0000ffff)
#define DRAM_CONFIG_VALL (DRAM_CONFIG_VAL&0x0000ffff)
#define DRAM_TIMING_VALH ((DRAM_TIMING_VAL>>16)&0x0000ffff)
#define DRAM_TIMING_VALL (DRAM_TIMING_VAL&0x0000ffff)

void setup_reboot_addr(unsigned long addr)
{
	unsigned int dramcode[15]={
                    		0x3c080003,  // lui t0,0003   
                    		0x35088396,  // ori t0,t0,0x8396 
                    		0x3c09b800,  // lui t1,0xb800
                    		0x35290010,  // ori t1,t1,0x0010
                     		0xad280000,  // sw t0,0(t1)
 		
							0x3c086cea, 	// lui	t0,0x6cea
							0x35080a80, 	// ori	t0,t0,0x0a80
							0x3c09b800, 	// lui	t1,0xb800
							0x35291008, 	// ori	t1,t1,0x1008
							0xad280000, 	// sw	t0,0(t1)

							0x3c085208,     // lui	t0,0x5208 //8MB  DRAM
							0x35080000, 	// ori	t0,t0,0x0000							
							0x3c09b800, 	// lui	t1,0xb800
							0x35291004, 	// ori	t1,t1,0x1004
							0xad280000, 	// sw	t0,0(t1)
	};
	unsigned int jmpcode[4]={
		0x3c1aa070,	//  lui k0,0xa070 
		0x375a0000,	//   ori k0,k0,0x0000  
		0x03400008, //  jr k0 
   		0x0   		//   nop
	};
	int i, offset;

	// setting DCR and DTR register
	dramcode[5]=(dramcode[5]&0xffff0000) | DRAM_TIMING_VALH;
	dramcode[6]=(dramcode[6]&0xffff0000) | DRAM_TIMING_VALL;
	dramcode[10]=(dramcode[10]&0xffff0000) | DRAM_CONFIG_VALH;
	dramcode[11]=(dramcode[11]&0xffff0000) | DRAM_CONFIG_VALL;
	  
	for (i=0, offset=0; i<15; i++, offset++)
		*(volatile u32 *)(KSEG0 + 0x8000 + offset*4) = dramcode[i];
		
	// set jump command		
	jmpcode[0] = (jmpcode[0]&0xffff0000) | ((addr>>16)&0xffff);
	jmpcode[1] = (jmpcode[1]&0xffff0000) | (addr&0xffff);	
	
	for (i=0; i<4; i++, offset++)
		*(volatile u32 *)(KSEG0 + 0x8000 + offset*4) = jmpcode[i];

	flush_icache_range(KSEG0+0x8000, KSEG0 + offset*4);
}
#endif	// CONFIG_RTL8197B_PANA

void __init nino_irq_setup(void)
{
	extern asmlinkage void ninoIRQ(void);

	unsigned int i;

	/* Disable all hardware interrupts */
	change_cp0_status(ST0_IM, 0x00);

	/* Initialize IRQ vector table */
	//init_generic_irq();

	/* Initialize IRQ action handlers */
#ifdef CONFIG_RTL8196B
#ifdef CONFIG_RTL8196C

	for (i = 0; i < 16; i++) {
#else
	for (i = 0; i < 32; i++) {
#endif
#else
	for (i = 0; i < 16; i++) {
#endif		
		hw_irq_controller *handler = NULL;
		handler		= &irq_type;

		irq_desc[i].status	= IRQ_DISABLED;
		irq_desc[i].action	= 0;
		irq_desc[i].depth	= 1;
		irq_desc[i].handler	= handler;
	}

	/* Set up the external interrupt exception vector */
	set_except_vector(0, ninoIRQ);

#ifdef CONFIG_RTL8197B_PANA
	setup_reboot_addr(0x80700000);
#endif	

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

#ifdef CONFIG_RTL8196C //michaelxxx
#define CONFIG_RTL819X_SUSPEND_CHECK_INTERRUPT

#ifdef CONFIG_RTL819X_SUSPEND_CHECK_INTERRUPT
#include <linux/proc_fs.h>
#include <linux/kernel_stat.h>
#include <asm/uaccess.h>
//#define INT_HIGH_WATER_MARK 1850 //for window size = 1, based on LAN->WAN test result
//#define INT_LOW_WATER_MARK  1150
//#define INT_HIGH_WATER_MARK 9190 //for window size = 5, based on LAN->WAN test result
//#define INT_LOW_WATER_MARK  5500
//#define INT_HIGH_WATER_MARK 3200  //for window size = 5, based on WLAN->WAN test result
//#define INT_LOW_WATER_MARK  2200
/* revise the parameter for BT test environment (4 LAN PCs and 1 WLAN STA) */
#define INT_HIGH_WATER_MARK 1300	//iperf & BT
#define INT_LOW_WATER_MARK  800

#define INT_WINDOW_SIZE_MAX 10
static int suspend_check_enable = 1;
static int suspend_check_high_water_mark = INT_HIGH_WATER_MARK;
static int suspend_check_low_water_mark = INT_LOW_WATER_MARK;
static int suspend_check_win_size = 5;
static struct timer_list suspend_check_timer;
static int index=0;
static int eth_int_count[INT_WINDOW_SIZE_MAX];
static int wlan_int_count[INT_WINDOW_SIZE_MAX];
int cpu_can_suspend = 1;
int cpu_can_suspend_check_init = 0;

static int read_proc_suspend_check(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
    int len;

    len = sprintf(page, "enable=%d, winsize=%d(%d), high=%d, low=%d, suspend=%d\n", 
		suspend_check_enable, suspend_check_win_size, INT_WINDOW_SIZE_MAX,
		suspend_check_high_water_mark, suspend_check_low_water_mark, cpu_can_suspend);
			
    if (len <= off+count)
        *eof = 1;
    *start = page + off;
    len -= off;
    if (len > count)
        len = count;
    if (len < 0)
        len = 0;
    return len;
}
	
static int write_proc_suspend_check(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	char tmp[128];

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		int num = sscanf(tmp, "%d %d %d %d", 
			&suspend_check_enable, &suspend_check_win_size,
			&suspend_check_high_water_mark, &suspend_check_low_water_mark);
		if (suspend_check_win_size >= INT_WINDOW_SIZE_MAX)
			suspend_check_win_size = INT_WINDOW_SIZE_MAX - 1;
		if (suspend_check_enable) {
			mod_timer(&suspend_check_timer, jiffies + 100);
		}
		else {
			del_timer(&suspend_check_timer);
		}
	}
	return count;	
}

static void suspend_check_timer_fn(unsigned long arg)
{
	int count, i, j;
	
	index++;
	if (INT_WINDOW_SIZE_MAX <= index)
		index = 0;
	eth_int_count[index] = kstat_irqs(2);
	wlan_int_count[index] = kstat_irqs(PCIE_IRQ);
	j = index - suspend_check_win_size;
	if (j < 0)
		j += INT_WINDOW_SIZE_MAX;
	count = (eth_int_count[index] - eth_int_count[j]) + 
		(wlan_int_count[index]- wlan_int_count[j]); //unit: number of interrupt occurred

	if (cpu_can_suspend) {
		if (count > suspend_check_high_water_mark) {
			cpu_can_suspend = 0;
			//printk("\n<<<RTL8196C LEAVE SLEEP>>>\n"); /* for Debug Only*/
		}
	}
	else {
		if (count < suspend_check_low_water_mark) {
			cpu_can_suspend = 1;
			//printk("\n<<<RTL8196C ENTER SLEEP>>>\n"); /* for Debug Only*/
		}
	}
#if 0 /* for Debug Only*/	
	printk("###index=%d, count=%d (%d+%d) suspend=%d###\n",index, count, 
		(eth_int_count[index] - eth_int_count[j]), 
		(wlan_int_count[index]- wlan_int_count[j]),
		cpu_can_suspend);
#endif    	
    	mod_timer(&suspend_check_timer, jiffies + 100);
}

void suspend_check_interrupt_init(void)
{
	struct proc_dir_entry *res;
	int i;
	
	res = create_proc_entry("suspend_check", 0, NULL);
	if (res) {
		res->read_proc = read_proc_suspend_check;
		res->write_proc = write_proc_suspend_check;
	}
	else {
		printk("unable to create /proc/suspend_check\n");
	}

	for (i=0; i<INT_WINDOW_SIZE_MAX; i++) {
		wlan_int_count[i] = 0;
		eth_int_count[i] = 0;
	}
	init_timer(&suspend_check_timer);
	suspend_check_timer.data = 0;
	suspend_check_timer.function = suspend_check_timer_fn;
	suspend_check_timer.expires = jiffies + 100; /* in jiffies */
	add_timer(&suspend_check_timer);
}
#endif // CONFIG_RTL819X_SUSPEND_CHECK_INTERRUPT
#endif //CONFIG_RTL8196C