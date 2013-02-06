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
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/rlxbsp.h>

#include <asm/rtl865x/rtl865xc_asicregs.h>

#include "bspchip.h"

static void shutdown_netdev(void)
{
	struct net_device *dev;
	
	printk("Shutdown network interface\n");
	read_lock(&dev_base_lock);

	for_each_netdev(&init_net, dev)
	{
		if(dev->flags &IFF_UP) 
		{
			printk("%s:===>\n",dev->name);			
			rtnl_lock();
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			if(dev->stop)
				dev->stop(dev);
#else
			if ((dev->netdev_ops)&&(dev->netdev_ops->ndo_stop))
				dev->netdev_ops->ndo_stop(dev);
#endif
			rtnl_unlock();
		}
      	}
#if defined(CONFIG_RTL8192CD)
	{
		extern void force_stop_wlan_hw(void);
		force_stop_wlan_hw();
	}
#endif
	read_unlock(&dev_base_lock);
}

static void bsp_machine_restart(char *command)
{
    static void (*back_to_prom)(void) = (void (*)(void)) 0xbfc00000;
	
    REG32(GIMR)=0;
	
    local_irq_disable();
    shutdown_netdev();
    REG32(BSP_WDTCNR) = 0; //enable watch dog
    while (1) ;
    /* Reboot */
    back_to_prom();
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
