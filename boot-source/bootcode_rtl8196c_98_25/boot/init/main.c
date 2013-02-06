/*
 *  linux/init/main.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  GK 2/5/95  -  Changed to support mounting root fs via NFS
 *  Added initrd & change_root: Werner Almesberger & Hans Lermen, Feb '96
 *  Moan early if gcc is old, avoiding bogus kernels - Paul Gortmaker, May '96
 *  Simplified starting of init:  Michael A. Griffith <grif@acm.org>
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <linux/string.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <linux/circ_buf.h>

#include "main.h"

void start_kernel(void)
{
	int ret;

	IMG_HEADER_T header;
	SETTING_HEADER_T setting_header;
//-------------------------------------------------------
	setClkInitConsole();

	initHeap();
	
	initInterrupt();

	initFlash();

#if !defined(CONFIG_FPGA_PLATFORM)
	Init_GPIO();				
#endif	
	
#if defined(CONFIG_PCIE_MODULE)  //for matt test code.
	PCIE_reset_procedure(0, 0, 1);   //1 do mdio_reset
#endif

	showBoardInfo();

	return_addr=0;
	ret=check_image	(&header,&setting_header);

#if defined(CONFIG_POST_ENABLE)
	int post_test_result=1;
	if(POSTRW_API()==0) ret=0;
#endif	
	doBooting(ret, return_addr, &header);
}

//-------------------------------------------------------
//show board info
void showBoardInfo(void)
{
	volatile int cpu_speed = 0;
	cpu_speed = check_cpu_speed();	
	prom_printf("%s",((*(volatile unsigned int *)(0xb8000008)) & (0x1<<23))?"Reboot Result from Watchdog Timeout!\n":" ");
	
#if defined(CONFIG_BOOT_TIME_MEASURE)
	cp3_count_print();
#endif
#if defined(RTL8196B)
	prom_printf("\n---RealTek(RTL8196C)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, "16bit", cpu_speed);	
#elif defined(RTL8198)
    #ifdef CONFIG_RTL89xxC
	prom_printf("\n---RealTek(RTL89xxC)at %s %s [%s](%dMHz)\n",  BOOT_CODE_TIME,B_VERSION, "16bit", cpu_speed);
    #else
	prom_printf("\n---RealTek(RTL8198)at %s %s [%s](%dMHz)\n",		BOOT_CODE_TIME,B_VERSION, "16bit", cpu_speed);	
    #endif
#elif defined( RTL8196C)
	prom_printf("\n---RealTek(RTL8196B)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, (IS_32BIT ? "32bit" : "16bit"), cpu_speed);	
#endif
}
