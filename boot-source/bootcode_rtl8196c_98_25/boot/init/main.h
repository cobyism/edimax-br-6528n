/*
 *  linux/init/main.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  GK 2/5/95  -  Changed to support mounting root fs via NFS
 *  Added initrd & change_root: Werner Almesberger & Hans Lermen, Feb '96
 *  Moan early if gcc is old, avoiding bogus kernels - Paul Gortmaker, May '96
 *  Simplified starting of init:  Michael A. Griffith <grif@acm.org>
 */
#include "../include/linux/autoconf.h"
#include <asm/io.h>
#include "./banner/mk_time"
#include "./rtk.h"
#include "./ver.h"
#include "./utility.h"


//------------------------------------------------------------------------------------------
//
#define __KERNEL_SYSCALLS__
#define IS_32BIT			(rtl_inl(0x1000)&0x100000)

#define SYS_STACK_SIZE		(4096 * 2)
unsigned char init_task_union[SYS_STACK_SIZE];

#ifdef CONFIG_FPGA_PLATFORM
#define CPU_CLOCK			(27000000)  /* 27MHz */
#else
#define CPU_CLOCK			(200*1000*1000)
#endif

//------------------------------------------------------------------------------------------
//extern 
/*in trap.c*/
extern void flush_cache(void);
extern void flush_dcache(UINT32 start, UINT32 end);

/*in irq.c*/
extern void __init exception_init(void);
extern void init_IRQ(void);
extern void eth_startup(int etherport);
extern void eth_listening(void);

/*in eth_tftpd.c*/
extern void tftpd_entry(void);

/*in monitor.c*/
extern int check_cpu_speed(void);
extern volatile int get_timer_jiffies(void);


#ifdef CONFIG_SPI_FLASH
/*in spi_flash.c*/
extern void spi_probe(); 
extern void check_spi_clk_div(void);
#else 
/*in flash.c*/
extern int flashread (unsigned long dst, unsigned int src, unsigned long length);
extern int flashinit();	
#endif

/*in utility.c*/
extern unsigned long glexra_clock;

#if defined(CONFIG_PCIE_MODULE)  
/*in test_hw_96c.c[for matt test code.]*/
	extern void PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset);
#endif

//-----------------------------------------------------------
//
void showBoardInfo(void);

