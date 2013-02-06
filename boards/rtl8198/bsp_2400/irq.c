/*
 * Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp0/irq.c
 *   Interrupt and exception initialization for RLX OCP Platform
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/irq.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/irq_cpu.h>
#include <asm/irq_vec.h>
#include <asm/system.h>

#include <asm/rlxregs.h>
#include <asm/rlxbsp.h>

#include "bspchip.h"

static struct irqaction irq_cascade = { 
  .handler = no_action,
  .mask = CPU_MASK_NONE,
  .name = "cascade",
};

static void bsp_ictl_irq_mask(unsigned int irq)
{
    REG32(BSP_GIMR) &= ~(1 << (irq - BSP_IRQ_ICTL_BASE));
}

static void bsp_ictl_irq_unmask(unsigned int irq)
{
    REG32(BSP_GIMR) |= (1 << (irq - BSP_IRQ_ICTL_BASE));
}

static struct irq_chip bsp_ictl_irq = {
    .typename = "ICTL",
    .ack = bsp_ictl_irq_mask,
    .mask = bsp_ictl_irq_mask,
    .mask_ack = bsp_ictl_irq_mask,
    .unmask = bsp_ictl_irq_unmask,
};

static void bsp_ictl_irq_dispatch(void)
{
    unsigned int pending;
  
    pending = REG32(BSP_GIMR) & REG32(BSP_GISR);

    if (pending & BSP_UART0_IP)
        do_IRQ(BSP_UART0_IRQ);
    else if (pending & BSP_UART1_IP)
        do_IRQ(BSP_UART1_IRQ);
    else if (pending & BSP_TC1_IP)
 	    do_IRQ(BSP_TC1_IRQ);
    else {
        printk("Unknown Interrupt:%x\n",pending);
        spurious_interrupt();
    }
}

void bsp_irq_dispatch(void)
{
    unsigned int pending;
   
    pending = read_c0_cause() & read_c0_status() & ST0_IM;

    if (pending & CAUSEF_IP2)
        bsp_ictl_irq_dispatch();
    else if (pending & CAUSEF_IP0)
        do_IRQ(0);
    else if (pending & CAUSEF_IP1)
        do_IRQ(1);
    else
        spurious_interrupt();
}

static void __init bsp_ictl_irq_init(unsigned int irq_base)
{
    int i;

    for (i=0; i < BSP_IRQ_ICTL_NUM; i++) 
        set_irq_chip_and_handler(irq_base + i, &bsp_ictl_irq, handle_level_irq);

    setup_irq(BSP_ICTL_IRQ, &irq_cascade);
}

#ifdef CONFIG_RTL_8198_NFBI_BOARD
extern void (*flush_icache_range)(unsigned long start, unsigned long end);

int get_dram_type(void)
{
	// read hw_strap register
	if (REG32(0xb8000008) & 0x2) //bit 1
		return 1; //DDR
	else
		return 0; //SDR
}

void setup_reboot_addr(unsigned long addr)
{
	unsigned int dramcode[20]={
                    		0x3c080f0a,  // lui t0,0f0a   
                    		0x3508dfff,  // ori t0,t0,0xdfff
                    		0x3c09b800,  // lui t1,0xb800
                    		0x35290048,  // ori t1,t1,0x0048
                     		0xad280000,  // sw t0,0(t1)
                     		
                    		0x3c0801FF,  // lui t0,01FF   
                    		0x3508FF8A,  // ori t0,t0,0xFF8A 
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
        dramcode[10]=(dramcode[10] &0xffff0000) | 0xffff;
        dramcode[11]=(dramcode[11] &0xffff0000) | 0x05c0;
        //if (check_ddr_tmp_file())
	if (get_dram_type()) { //DDR
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5448; //DDR, 32M
		//8198:1.set bigger current for DDR
		dramcode[0]=(dramcode[0] &0xffff0000) | 0x0b0a;
		// TX RX delay
		dramcode[5]=(dramcode[5] &0xffff0000) | 0x01ff;
		dramcode[6]=(dramcode[6] &0xffff0000) | 0xfc70;
        }
        else {
            	dramcode[15]=(dramcode[15] &0xffff0000) | 0x5208; //SDR, 8M
            	//8198:1.set bigger current for DDR
		dramcode[0]=(dramcode[0] &0xffff0000) | 0x0f0a;
		// TX RX delay
		dramcode[5]=(dramcode[5] &0xffff0000) | 0x01ff;
		dramcode[6]=(dramcode[6] &0xffff0000) | 0xff8a;
        }
        dramcode[16]=(dramcode[16] &0xffff0000) | 0x0000;
        

	for (i=0, offset=0; i<20; i++, offset++)
		*(volatile u32 *)(KSEG0 + 0x8000 + offset*4) = dramcode[i];
	
	// set jump command		
	jmpcode[0] = (jmpcode[0]&0xffff0000) | ((addr>>16)&0xffff);
	jmpcode[1] = (jmpcode[1]&0xffff0000) | (addr&0xffff);	
	
	for (i=0; i<4; i++, offset++)
		*(volatile u32 *)(KSEG0 + 0x8000 + offset*4) = jmpcode[i];

	flush_icache_range(KSEG0+0x8000, KSEG0 + offset*4);
}
#endif	//CONFIG_RTL_8198_NFBI_BOARD

void __init bsp_irq_init(void)
{
	/* disable ict interrupt */
	REG32(BSP_GIMR) = 0;

	/* initialize IRQ action handlers */
	rlx_cpu_irq_init(BSP_IRQ_CPU_BASE);
	rlx_vec_irq_init(BSP_IRQ_LOPI_BASE);
	bsp_ictl_irq_init(BSP_IRQ_ICTL_BASE);

	/* Set IRR */
	REG32(BSP_IRR0) = BSP_IRR0_SETTING;
	REG32(BSP_IRR1) = BSP_IRR1_SETTING;
	REG32(BSP_IRR2) = BSP_IRR2_SETTING;
	REG32(BSP_IRR3) = BSP_IRR3_SETTING;  

	/* enable global interrupt mask */
	REG32(BSP_GIMR) = BSP_TC0_IE | BSP_UART0_IE;
#if defined(CONFIG_PCI) || defined(CONFIG_RTL_PCIE_SIMPLE_INIT)
	REG32(BSP_GIMR) |= (BSP_PCIE_IE | BSP_PCIE2_IE);
#endif
#if defined(CONFIG_USB)
	REG32(BSP_GIMR) |= BSP_USB_H_IE;
#endif
#if defined(CONFIG_RTL_819X_SWCORE) || defined(CONFIG_RTL_ICTEST_SWITCH) || defined(CONFIG_RTL_865X_ETH)
	REG32(BSP_GIMR) |= (BSP_SW_IE);
#endif
#if defined(CONFIG_RTL_NFBI_MDIO)
	REG32(BSP_GIMR) |= BSP_NFBI_IE;
#endif

#ifdef CONFIG_RTL_8198_NFBI_BOARD
	setup_reboot_addr(0x80700000);
#endif
}
