/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp/setup.c
 *   Interrupt and exception initialization for RLX OCP Platform
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/rlxbsp.h>

#include "bspchip.h"



static void bsp_machine_restart(char *command)
{
    local_irq_disable();
    REG32(BSP_WDTCNR) = 0; //enable watch dog
    while (1) ;
}
                                                                                                    
static void bsp_machine_halt(void)
{
    printk("RTL8196B halted.\n");
    while(1);
}
                                                                                                    
static void bsp_machine_power_off(void)
{
    printk("RTL8196B halted. Please turn off the power.\n");
    while(1);
}

/*
 * callback function
 */
extern void _imem_dmem_init(void);
void __init bsp_setup(void)
{
    /* define io/mem region */
    ioport_resource.start = 0x18000000; 
    ioport_resource.end = 0x1fffffff;

    iomem_resource.start = 0x18000000;
    iomem_resource.end = 0x1fffffff;

    /* set reset vectors */
    _machine_restart = bsp_machine_restart;
    _machine_halt = bsp_machine_halt;
    pm_power_off = bsp_machine_power_off;

    /* initialize uart */
    bsp_serial_init();
    _imem_dmem_init();
}
