/*
 *  Routines to handle OS dependent jobs and interfaces
 *
 *  $Id: 8192cd_osdep.c,v 1.61.2.34 2011/04/29 09:01:39 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*-----------------------------------------------------------------------------
	This file is for OS related functions.
------------------------------------------------------------------------------*/
#define _8192CD_OSDEP_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#endif

#ifdef __DRAYTEK_OS__
#include <draytek/softimer.h>
#include <draytek/skbuff.h>
#include <draytek/wl_dev.h>
#endif

#include "./8192cd_cfg.h"

#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>
#endif
#if defined(RTK_BR_EXT) || defined(BR_SHORTCUT)
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#include <../net/bridge/br_private.h>
#endif
#else
#include "./sys-support.h"
#endif

#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_rx.h"
#include "./8192cd_debug.h"

#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif



#ifdef	CONFIG_RTK_MESH
#include "./mesh_ext/mesh_route.h"	// Note : Not include in  #ifdef CONFIG_RTK_MESH, Because use in wlan_device[]
#include "./mesh_ext/mesh_util.h"
#endif	// CONFIG_RTK_MESH

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif

#ifndef CONFIG_RTK_MESH	//move below from mesh_route.h ; plus 0119
#define MESH_SHIFT			0
#define MESH_NUM_CFG		0
#else
#define MESH_SHIFT			8 // ACCESS_MASK uses 2 bits, WDS_MASK use 4 bits
#define MESH_MASK			0x3
#define MESH_NUM_CFG		NUM_MESH
#endif

#ifdef CONFIG_RTL8672
#include <platform.h>
#else
#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196B)
#include <asm/rtl865x/platform.h>
#endif

#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196C)
#include <asm/rtl865x/platform.h>
#endif
#endif

#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL_8196C) && !defined(USE_RLX_BSP)
#include <platform.h>
#endif

#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL_819X) && defined(USE_RLX_BSP)
#include <bsp/bspchip.h>
#endif

#ifdef CONFIG_WIRELESS_LAN_MODULE
extern int (*wirelessnet_hook)(void);
#ifdef BR_SHORTCUT
extern struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da);
#endif
#ifdef PERF_DUMP
extern int rtl8651_romeperfEnterPoint(unsigned int index);
extern int rtl8651_romeperfExitPoint(unsigned int index);
extern int (*Fn_rtl8651_romeperfEnterPoint)(unsigned int index);
extern int (*Fn_rtl8651_romeperfExitPoint)(unsigned int index);
#endif
#ifdef CONFIG_RTL8190_PRIV_SKB
extern int (*wirelessnet_hook_is_priv_buf)(void);
extern void (*wirelessnet_hook_free_priv_buf)(unsigned char *head);
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE


struct _device_info_ {
	int type;
	unsigned long conf_addr;
	unsigned long base_addr;
	int irq;
	struct rtl8192cd_priv *priv;
};

static struct _device_info_ wlan_device[] =
{
#if defined(USE_RTL8186_SDK)
	//{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT), 0, 0xbd400000, 2, NULL},
	#ifdef CONFIG_NET_PCI
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	#else
		#ifdef IO_MAPPING
			{(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT1_CONFIG_ADDR, 0xb8C00000, 3, NULL},
		#else
			#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_819X)
				#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
					{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
				#else
					{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCIE0_D_CFG0, PCIE0_D_MEM, PCIE_IRQ, NULL},
				#endif
				//{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
			#else
				{(MESH_NUM_CFG<<MESH_SHIFT) | (WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT1_CONFIG_ADDR, 0xb9000000, 3, NULL},
			#endif
		#endif
	#endif
#elif defined(CONFIG_X86)
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT), 0, 0, 0, NULL}
#elif defined(CONFIG_RTL8671)
	#ifdef IO_MAPPING
	{(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT0_CONFIG_ADDR, 0xbd200000, 7, NULL},
	#else
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT0_CONFIG_ADDR, 0xbd300000, 7, NULL},
	#endif
#else
	#error No_System_Defined
#endif
};

static int wlan_index=0;

#ifdef CONFIG_RTL8672
// Added by Mason Yu
// MBSSID Port Mapping
struct port_map wlanDev[RTL8192CD_NUM_VWLAN+1];
static int wlanDevNum=0;
#endif

#ifdef PRIV_STA_BUF
static struct rtl8192cd_hw hw_info;
static struct priv_shared_info shared_info;
static struct wlan_hdr_poll hdr_pool;
static struct wlanllc_hdr_poll llc_pool;
static struct wlanbuf_poll buf_pool;
static struct wlanicv_poll icv_pool;
static struct wlanmic_poll mic_pool;
static unsigned char desc_buf[DESC_DMA_PAGE_SIZE];
#endif

// init and remove char device
#ifdef CONFIG_WIRELESS_LAN_MODULE
extern int rtl8192cd_chr_init(void);
extern void rtl8192cd_chr_exit(void);
#else
int rtl8192cd_chr_init(void);
void rtl8192cd_chr_exit(void);
#endif
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv);
void rtl8192cd_chr_unreg(unsigned int minor);
int rtl8192cd_fileopen(const char *filename, int flags, int mode);
struct net_device *get_shortcut_dev(unsigned char *da);

void force_stop_wlan_hw(void);

#if defined(_INCLUDE_PROC_FS_) && defined(PERF_DUMP)
static int read_perf_dump(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	unsigned long x;
	save_and_cli(x);

	rtl8651_romeperfDump(1, 1);

	restore_flags(x);
    return count;
}


static int flush_perf_dump(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	unsigned long x;
	save_and_cli(x);

	rtl8651_romeperfReset();

	restore_flags(x);
    return count;
}
#endif // _INCLUDE_PROC_FS_ && PERF_DUMP

#define GPIO_BASE			0xB8003500
#define PEFGHCNR_REG		(0x01C + GPIO_BASE)     /* Port EFGH control */
#define PEFGHPTYPE_REG		(0x020 + GPIO_BASE)     /* Port EFGH type */
#define PEFGHDIR_REG		(0x024 + GPIO_BASE)     /* Port EFGH direction */
#define PEFGHDAT_REG		(0x028 + GPIO_BASE)     /* Port EFGH data */
#ifndef REG32
	#define REG32(reg)	 	(*(volatile unsigned int *)(reg))
#endif

#if !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
#define MAX_PAYLOAD_SIZE_128B    0x00

#if !(defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C))
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int rtl8196b_pci_reset(unsigned long conf_addr)
{
   /* If PCI needs to be reset, put code here.
    * Note:
    *    Software may need to do hot reset for a period of time, say ~100us.
    *    Here we put 2ms.
    */
	//Modified for PCIE PHY parameter due to RD center suggestion by Jason 12252009
	WRITE_MEM32(0xb8000044, 0x9);//Enable PCIE PLL
	delay_ms(10);
	REG32(0xb8000010)=REG32(0xb8000010)|(0x500); //Active LX & PCIE Clock in 8196B system register
	delay_ms(10);
	WRITE_MEM32(0xb800003C, 0x1);//PORT0 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb800003C, 0x3);//PORT0 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8000040, 0x1);//PORT1 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8000040, 0x3);//PORT1 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8b01008, 0x1);// PCIE PHY Reset Close:Port 0
	delay_ms(10);
	WRITE_MEM32(0xb8b01008, 0x81);// PCIE PHY Reset On:Port 0
	delay_ms(10);
#ifdef PIN_208
	WRITE_MEM32(0xb8b21008, 0x1);// PCIE PHY Reset Close:Port 1
	delay_ms(10);
	WRITE_MEM32(0xb8b21008, 0x81);// PCIE PHY Reset On:Port 1
	delay_ms(10);
#endif
#ifdef OUT_CYSTALL
	WRITE_MEM32(0xb8b01000, 0xcc011901);// PCIE PHY Reset On:Port 0
	delay_ms(10);
#ifdef PIN_208
	WRITE_MEM32(0xb8b21000, 0xcc011901);// PCIE PHY Reset On:Port 1
	delay_ms(10);
#endif
#endif
	REG32(0xb8000010)=REG32(0xb8000010)|(0x01000000); //PCIE PHY Reset On:Port 0
	delay_ms(10);

#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef PIN_208
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   delay_ms(100);
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) | 0x00000080);
#ifdef PIN_208
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) | 0x00000080);
#endif
#else
   WRITE_MEM32(PCIE0_H_PWRCR, READ_MEM32(PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef PIN_208
   WRITE_MEM32(PCIE1_H_PWRCR, READ_MEM32(PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   delay_ms(100);
   WRITE_MEM32(PCIE0_H_PWRCR, READ_MEM32(PCIE0_H_PWRCR) | 0x00000080);
#ifdef PIN_208
   WRITE_MEM32(PCIE1_H_PWRCR, READ_MEM32(PCIE1_H_PWRCR) | 0x00000080);
#endif
#endif

   delay_ms(10);

	if ((READ_MEM32(0xb8b00728)&0x1f)!=0x11)
	{
		_DEBUG_INFO("PCIE LINK FAIL\n");
		return FAIL;
	}

   // Enable PCIE host
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
	WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
	WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
	return SUCCESS;
}
#endif // !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
#endif


#if !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C))
#define MAX_PAYLOAD_SIZE_128B    0x00

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C)
//HOST PCIE
#define PCIE0_RC_EXT_BASE (0xb8b01000)
//RC Extended register
#define PCIE0_MDIO (PCIE0_RC_EXT_BASE+0x00)
//MDIO
#define PCIE_MDIO_DATA_OFFSET (16)
#define PCIE_MDIO_DATA_MASK (0xffff <<PCIE_MDIO_DATA_OFFSET)
#define PCIE_MDIO_REG_OFFSET (8)
#define PCIE_MDIO_RDWR_OFFSET (0)


void HostPCIe_SetPhyMdioWrite(unsigned int port, unsigned int regaddr, unsigned short val)
{
	REG32(PCIE0_MDIO)= ( (regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET) | ((val&0xffff)<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ;
	//delay
	delay_ms(1);//mdelay(10);
}


#ifdef CONFIG_RTL8672
static int PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset,unsigned long conf_addr)
{
	//#define SYS_PCIE_PHY0   (0xb8000000 +0x50)
	#define SYS_PCIE_PHY0   0xb8003400
	#define Module_Enable   0xb800330c
	//PCIE Register
	#define CLK_MANAGE  0xb800350c
	#define PCIE_PHY0_REG  0xb8b01000
	//#define PCIE_PHY1_REG  0xb8b21000
	#define PCIE_PHY0  0xb8b01008
	// #define PCIE_PHY1  0xb8b21008
	#define PCIE_gpio_RST 5

    //1. PCIE phy mdio reset
    REG32(SYS_PCIE_PHY0) = 0x1d400000;
    REG32(SYS_PCIE_PHY0) = 0x1d500000;

    //2. PCIE MAC reset
    REG32(Module_Enable) &= ~(1<<9);
	REG32(Module_Enable) |= (1<<9);

	//3.Active LX & PCIE Clock
    gpioSet(PCIE_gpio_RST);
    delay_ms(100);

    //4. GPIO output
    REG32(0xb8003508) |= (1<<5);
    delay_ms(100);

#if 1
	if(mdio_reset)
	{
		//printk("Do MDIO_RESET\n");
		// 5.MDIO Reset
		REG32(SYS_PCIE_PHY0) = 0x1d500000;
	}
	//6. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(100);
#endif

	delay_ms(100);

   //----------------------------------------
   if(mdio_reset)
	{
#if 1
	int port=0;
		HostPCIe_SetPhyMdioWrite(port, 0, 0x5027);
		HostPCIe_SetPhyMdioWrite(port, 2, 0x6d18);
		//HostPCIe_SetPhyMdioWrite(port, 6, 0x8028);
		if( (IS_RLE0315 ||IS_6166) && (REG32(MISC_PINSR)&(0x200000))){
			HostPCIe_SetPhyMdioWrite(port, 6, 0x20c8); //35.328 clock source
			HostPCIe_SetPhyMdioWrite(port, 5, 0x0bcb);
			printk("Clock source is 35.328MHz\n");
		}else if( (IS_RLE0315 ||IS_6166) && ~(REG32(MISC_PINSR)&(0x200000))){
			HostPCIe_SetPhyMdioWrite(port, 6, 0xf848);  //25M clock source
			HostPCIe_SetPhyMdioWrite(port, 5, 0x08ab);
			printk("Clock source is 25MHz\n");
		}
		HostPCIe_SetPhyMdioWrite(port, 7, 0x30ff);
		//HostPCIe_SetPhyMdioWrite(port, 8, 0x18dd);
		HostPCIe_SetPhyMdioWrite(port, 8, 0x18d7);
		HostPCIe_SetPhyMdioWrite(port, 0xa, 0xe9);
		HostPCIe_SetPhyMdioWrite(port, 0xb, 0x0511);
		//HostPCIe_SetPhyMdioWrite(port, 0xd, 0x15b6);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x15a6);
		HostPCIe_SetPhyMdioWrite(port, 0xf, 0x0f0f);
		//HostPCIe_SetPhyMdioWrite(port,  0x9, 0xe950); //czyao adds to leave L0 mode
#if 0 // PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(port, 5, 0xbcb);    //[9:3]=1111001 (binary)   121 (10)
		HostPCIe_SetPhyMdioWrite(port, 6, 0x8128);  //[11]=0   [9:8]=01
#endif
		/*
		emdiow 0 5027
		emdiow 2 6d18
		emdiow 6 8828
		emdiow 7 30ff
		emdiow 8 18dd
		emdiow a e9
		emdiow b 0511
		emdiow d 15b6
		emdiow f 0f0f
		*/
#else  // #if 0

		HostPCIe_SetPhyMdioWrite(port, 0, 0xD187);
		HostPCIe_SetPhyMdioWrite(port, 1, 0x0003);
		HostPCIe_SetPhyMdioWrite(port, 2, 0x4d18);
#if  1//PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(port, 5, 0x0BCB);   //40M
#endif

#if  1//PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(port, 6, 0xF148);  //40M
#else
		HostPCIe_SetPhyMdioWrite(port, 6, 0xf048);  //25M
#endif

		HostPCIe_SetPhyMdioWrite(port, 7, 0x31ff);
		HostPCIe_SetPhyMdioWrite(port, 9, 0x531c);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x1766);
		HostPCIe_SetPhyMdioWrite(port, 0xf, 0x0a00);

#if HAVING_FIB
		HostPCIe_SetPhyMdioWrite(port,8, 0x18dd);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x1776);
#endif

		HostPCIe_SetPhyMdioWrite(port, 0x19, 0xFCE0);
		HostPCIe_SetPhyMdioWrite(port, 0x1e, 0xC280);
#endif
	}

	//---------------------------------------
    //delay_ms(10000);

	// 7. PCIE Device Reset
	gpioClear(PCIE_gpio_RST);
	delay_ms(100);

	gpioSet(PCIE_gpio_RST);

	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(100);

    //8. Set BAR
	REG32(0xb8b10010) = 0x18c00001;
	REG32(0xb8b10018) = 0x19000004;
	REG32(0xb8b10004) = 0x00180007;
	REG32(0xb8b00004) = 0x00100007;

	//8. Fine tune parameter, and depends on RD center's suggestion
	//REG32(0xb8003204) = 0x4cc3106d;    //This is added in bootcode
#if 1  //wait for LinkUP
	int i=100;
	while(--i)
	{
		if( (REG32(0xb8b00728)&0x1f)==0x11)
		break;
		delay_ms(100);
	}
	if(i==0)
	{
		printk("i=%x Cannot LinkUP \n",i);
		return FAIL;
	}
#endif
	// Enable PCIE host
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
	WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
	WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
	return SUCCESS;
}
#else
static int PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset,unsigned long conf_addr)
{
	int i=100;
	#define SYS_PCIE_PHY0   (0xb8000000 +0x50)
	//PCIE Register
	#define CLK_MANAGE  0xb8000010
	#define PCIE_PHY0_REG  0xb8b01000
	//#define PCIE_PHY1_REG  0xb8b21000
	#define PCIE_PHY0  0xb8b01008
	// #define PCIE_PHY1  0xb8b21008
	int port =0;

	//2.Active LX & PCIE Clock
    REG32(CLK_MANAGE) |=  (1<<11);        //enable active_pcie0
    delay_ms(100);

#if 1
	if(mdio_reset)
	{
		//printk("Do MDIO_RESET\n");
		// 3.MDIO Reset
		REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,
		REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1,
		REG32(SYS_PCIE_PHY0) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
	}
	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(100);
#endif

	delay_ms(100);

   //----------------------------------------
   if(mdio_reset)
	{
		if (REG32(REVR) == RTL8196C_REVISION_A)
		{
		HostPCIe_SetPhyMdioWrite(port, 0, 0x5027);
		HostPCIe_SetPhyMdioWrite(port, 2, 0x6d18);
		HostPCIe_SetPhyMdioWrite(port, 6, 0x8828);
		HostPCIe_SetPhyMdioWrite(port, 7, 0x30ff);
		HostPCIe_SetPhyMdioWrite(port, 8, 0x18dd);
		HostPCIe_SetPhyMdioWrite(port, 0xa, 0xe9);
		HostPCIe_SetPhyMdioWrite(port, 0xb, 0x0511);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x15b6);
		HostPCIe_SetPhyMdioWrite(port, 0xf, 0x0f0f);
#if 1 // PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(port, 5, 0xbcb);    //[9:3]=1111001 (binary)   121 (10)
		HostPCIe_SetPhyMdioWrite(port, 6, 0x8128);  //[11]=0   [9:8]=01
#endif
		/*
		emdiow 0 5027
		emdiow 2 6d18
		emdiow 6 8828
		emdiow 7 30ff
		emdiow 8 18dd
		emdiow a e9
		emdiow b 0511
		emdiow d 15b6
		emdiow f 0f0f
		*/
		}
		else
		{
				HostPCIe_SetPhyMdioWrite(port, 0, 0xD087);
		HostPCIe_SetPhyMdioWrite(port, 1, 0x0003);
		HostPCIe_SetPhyMdioWrite(port, 2, 0x4d18);

#ifdef HIGH_POWER_EXT_PA
		HostPCIe_SetPhyMdioWrite(port, 5, 0x0BF3);   //40M
#else
		HostPCIe_SetPhyMdioWrite(port, 5, 0x0BCB);   //40M
#endif

#if  1//PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(port, 6, 0xF148);  //40M
#else
		HostPCIe_SetPhyMdioWrite(port, 6, 0xf848);  //25M
#endif

		HostPCIe_SetPhyMdioWrite(port, 7, 0x31ff);
		HostPCIe_SetPhyMdioWrite(port, 8, 0x18d7);
		HostPCIe_SetPhyMdioWrite(port, 9, 0x539c);
		HostPCIe_SetPhyMdioWrite(port, 0xa, 0x20eb);
		HostPCIe_SetPhyMdioWrite(port, 0xb, 0x0511);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x1764);
		HostPCIe_SetPhyMdioWrite(port, 0xf, 0x0a00);

#ifdef HAVING_FIB
		HostPCIe_SetPhyMdioWrite(port,8, 0x18dd);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x1776);
#endif

		HostPCIe_SetPhyMdioWrite(port, 0x19, 0xFCE0);
		HostPCIe_SetPhyMdioWrite(port, 0x1e, 0xC280);
		}
	}

	//---------------------------------------
	// 6. PCIE Device Reset
	REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.
	delay_ms(300);

	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(300);

	REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
	//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));
	delay_ms(100);

#if 1  //wait for LinkUP
	while(--i)
	{
		if( (REG32(0xb8b00728)&0x1f)==0x11)
		break;
		delay_ms(100);
	}
	if(i==0)
	{
		printk("i=%x Cannot LinkUP \n",i);
		return FAIL;
	}
#endif
	// Enable PCIE host
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
	WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
	WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
	return SUCCESS;
}
#endif


void HostPCIe_Close(void)
{
	REG32(0xb8b10044) &= (~(3));
	REG32(0xb8b10044) |= (3);
	HostPCIe_SetPhyMdioWrite(0, 0xf, 0x0708);
	//.DeActive LX & PCIE Clock
	REG32(CLK_MANAGE) &=(0xFFFFFFFF-  (1<<11));        //enable active_pcie0
}

#endif

#endif // !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))


#if 0
static int is_giga_board(void)
{
	unsigned int tmp;
	int b1, b2, b3, b4;

	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG)  &(~(0x03<<24)) & (~(0x48<<8) ); //set (GP4 GP3 GP2 GP1)=(H1 H0 F3 F6)= gpio
	REG32(PEFGHPTYPE_REG)=REG32(PEFGHPTYPE_REG)&(~(0x03<<24)) & (~(0x48<<8) );  //change to GPIO mode
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG)    &(~(0x03<<24)) & (~(0x48<<8) );;  //0 input, 1 output, set  inpur
	tmp=REG32(PEFGHDAT_REG);
	b4 = (tmp&(0x02<<24))>>25;
	b3 = (tmp&(0x01<<24))>>24;
	b2 = (tmp&(0x08<<8))>>11;
	b1 = (tmp&(0x40<<8))>>14;
	tmp = (b1&0x1) | ((b2<<1)&0x2) |  ((b3<<2)&0x4) | ((b4<<2)&0x8);

	/*
	 * GP4 GP3 GP2 GP1   ||   Giga     || Type
	 * =========================================
	 *   0    0     0    0           Yes          10/100
	 *   0    0     0    1           No           10/100
	 *   0    0     1    0           Yes          10/100
	 *   0    0     1    1           No           10/100
	 *   0    1     0    0           Yes          10/100
	 *   0    1     0    1           No           10/100
	 *   0    1     1    0           Yes          10/100
	 *   0    1     1    1           No           10/100
	 *   1    0     0    0           Yes          Serdes
	 *   1    0     0    1                          Serdes
	 *   1    0     1    0           Yes          Serdes
	 *   1    0     1    1                          Serdes
	 *   1    1     0    0           Yes          Serdes
	 *   1    1     0    1                          Serdes
	 *   1    1     1    0           Yes          Serdes
	 *   1    1     1    1                          Serdes
	 */
	/* !!! use "priv->pshare->is_giga_exist != 0" to indicate "Serdes or 10/100 have Giga" */
	if (b4 == 1 && b1 == 0) /* Serdes has Giga */
		return 2;
	else if (b1 == 0 || b4 == 1) /* Serdes or 10/100 have Giga */
		return 1;
	else
		return 0;
}
#endif


static void rtl8192cd_bcnProc(struct rtl8192cd_priv *priv, unsigned int bcnInt,
				unsigned int bcnDmaOk, unsigned int bcnOk, unsigned int bcnErr, unsigned int status)
{
#ifdef MBSSID
	int i;
#endif

	/* ================================================================
						Process Beacon interrupt
	   ================================================================ */
	//
	// Update beacon content
	//
	if (bcnInt) {
		if (status & HIMR_BCNDMA0) {
#ifdef UNIVERSAL_REPEATER
			if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
				(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
				if (GET_VXD_PRIV(priv)->timoffset) {
					update_beacon(GET_VXD_PRIV(priv));
				}
			} else
#endif
			{
				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
		}
#ifdef MBSSID
		else {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))
					{
						if (priv->pvap_priv[i]->timoffset) {
							update_beacon(priv->pvap_priv[i]);
						}
					}
				}
			}
		}
#endif

		//
		// Polling highQ as there is multicast waiting for tx...
		//
#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		if ((OPMODE & WIFI_AP_STATE)) {
			unsigned char val8;

			if (status & HIMR_BCNDMA0) {
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
					if (RTL_R8(BCN_CTRL) & DIS_ATIM)
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
				} else {
					if (!(RTL_R8(BCN_CTRL) & DIS_ATIM))
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));
				}
#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
#endif
			}
#ifdef MBSSID
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1)))) {
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
						if (val8 & 0x01) {
							if (RTL_R8(BCN_CTRL) & DIS_ATIM)
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
						} else {
							if (!(RTL_R8(BCN_CTRL) & DIS_ATIM))
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));
						}
						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}
#endif
		}

		if (priv->pshare->pkt_in_hiQ) {
			int pre_head = get_txhead(priv->pshare->phw, MCAST_QNUM);
			do {
				txdesc_rollback(&pre_head);
			} while ((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->type != _PRE_ALLOCLLCHDR_);
			if (get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN) {
				unsigned short *phdr = (unsigned short *)((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->pframe);
#ifdef __MIPSEB__
				phdr = (unsigned short *)KSEG1ADDR(phdr);
#endif
				ClearMData(phdr);
			}
			tx_poll(priv, MCAST_QNUM);
			priv->pshare->pkt_in_hiQ = 0;
		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif
	}

	/* ================================================================
					Process Beacon OK/ERROR interrupt
	   ================================================================ */
	if (bcnDmaOk || bcnOk || bcnErr)
	{
		//unsigned char val8;

#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		//
		// Polling highQ as there is multicast waiting for tx...
		//
		if ((OPMODE & WIFI_AP_STATE) && bcnDmaOk) {
#ifdef PCIE_POWER_SAVING
			if ((priv->offload_ctrl & 1) && (priv->offload_ctrl >> 7) && priv->pshare->rf_ft_var.power_save) {
				priv->offload_ctrl &= (~1);
				update_beacon(priv);
				RTL_W16(0x100 , RTL_R16(0x100) & ~BIT(8));		// disable sw beacon
				return;
			}
#endif

#if 0
			if (status & HIMR_BCNDOK0) {
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
				}
#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
#endif
			}
#ifdef MBSSID
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (status & (HIMR_BCNDOK1 << (priv->pvap_priv[i]->vap_init_seq-1)))) {
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
						if (val8 & 0x01) {
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
						}
						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}
#endif

			if (priv->pshare->pkt_in_hiQ) {
				int pre_head = get_txhead(priv->pshare->phw, MCAST_QNUM);
				do {
					txdesc_rollback(&pre_head);
				} while ((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->type != _PRE_ALLOCLLCHDR_);
				if (get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN) {
					unsigned short *phdr = (unsigned short *)((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->pframe);
#ifdef __MIPSEB__
					phdr = (unsigned short *)KSEG1ADDR(phdr);
#endif
					ClearMData(phdr);
				}
				tx_poll(priv, MCAST_QNUM);
				priv->pshare->pkt_in_hiQ = 0;
			}
#endif
		}

		//
		// Statistics and LED counting
		//
		if (bcnOk) {
			// for SW LED
			if (((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE <= LEDTYPE_SW_LINKTXRX)) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX)||(LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX))
				priv->pshare->LED_tx_cnt++;
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
#else
			priv->ext_stats.beacon_ok++;
#endif
			SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);
		} else if (bcnErr) {
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
#else
			priv->ext_stats.beacon_er++;
#endif
		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif
	}

#ifdef CLIENT_MODE
	//
	// Ad-hoc beacon status
	//
	if (OPMODE & WIFI_ADHOC_STATE) {
		if (bcnOk)
			priv->ibss_tx_beacon = TRUE;
		if (bcnErr)
			priv->ibss_tx_beacon = FALSE;
	}
#endif
}


void check_dma_error(struct rtl8192cd_priv *priv, unsigned int status, unsigned int status_ext)
{
	unsigned char reg_rxdma;
	unsigned int reg_txdma;
	int clear_isr=0;

	if ((status & HIMR_RXFOVW) | (status_ext & HIMRE_RXERR)) {
		reg_rxdma = RTL_R8(RXDMA_STATUS);
		if (reg_rxdma) {
			RTL_W8(RXDMA_STATUS, reg_rxdma);
			//panic_printk("RXDMA_STATUS %02x\n", reg_rxdma);
			clear_isr = 1;
		}
	}

	if ((status & HIMR_TXFOVW) | (status_ext & HIMRE_TXERR)) {
		reg_txdma = RTL_R32(TXDMA_STATUS);
		if (reg_txdma) {
			RTL_W32(TXDMA_STATUS, reg_txdma);
			//panic_printk("TXDMA_STATUS %08x\n", reg_txdma);
			clear_isr = 1;
		}
	}

	if (clear_isr) {
		RTL_W32(HISR, status);
		RTL_W32(HISRE, status_ext);
	}
}


#ifdef __LINUX_2_6__
__IRAM_IN_865X
#else
__MIPS16
__IRAM_IN_865X
#endif
__inline__ static int __rtl8192cd_interrupt(void *dev_instance)
{
	struct net_device *dev = NULL;
	struct rtl8192cd_priv *priv = NULL;
	struct rtl8192cd_hw *phw = NULL;

	unsigned int status, status_ext;
	unsigned int caseBcnInt, caseBcnDmaOK, caseBcnStatusOK, caseBcnStatusER;
	unsigned int caseRxStatus, caseRxRDU;
	unsigned int caseRxCmd, caseTimer1, caseTimer2;
#ifdef SUPPORT_TX_AMSDU
	unsigned long current_value, timeout;
#endif

	dev = (struct net_device *)dev_instance;
	priv = (struct rtl8192cd_priv *)dev->priv;
	phw = GET_HW(priv);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
		return  FAIL;
	}
#endif
	caseBcnInt = caseBcnDmaOK = caseBcnStatusOK = caseBcnStatusER = 0;
	caseRxStatus = caseRxRDU = 0;
	caseRxCmd = caseTimer1 = caseTimer2 = 0;

	status = RTL_R32(HISR);
	RTL_W32(HISR, status);
	status_ext = RTL_R32(HISRE);
	RTL_W32(HISRE, status_ext);
	if (status == 0 && status_ext == 0) {
		return FAIL;
	}

#ifdef TXREPORT
	if(status_ext & BIT(9) ) {
		C2H_isr(priv);
	}
#endif

	check_dma_error(priv, status, status_ext);

	if (status & (HIMR_BCNDMA0 | HIMR_BCNDMA1 | HIMR_BCNDMA2 | HIMR_BCNDMA3 | HIMR_BCNDMA4 | HIMR_BCNDMA5 | HIMR_BCNDMA6 | HIMR_BCNDMA7))
		caseBcnInt = 1;

	if (status & (HIMR_BCNDOK0 | HIMR_BCNDOK1 | HIMR_BCNDOK2 | HIMR_BCNDOK3 | HIMR_BCNDOK4 | HIMR_BCNDOK5 | HIMR_BCNDOK6 | HIMR_BCNDOK7))
		caseBcnDmaOK = 1;

	if (status & HIMR_TXBCNOK)
		caseBcnStatusOK = 1;

	if (status & HIMR_TXBCNERR)
		caseBcnStatusER = 1;

	if (status & (HIMR_ROK | HIMR_RDU))
		caseRxStatus = 1;

	if (status & HIMR_RDU) {
		priv->ext_stats.rx_rdu++;
		caseRxRDU = 1;
		priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
	}

	if (status & HIMR_RXFOVW) {
		priv->ext_stats.rx_fifoO++;
		priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
	}

	if (status & HIMR_TIMEOUT1)
		caseTimer1 = 1;

	if (status & HIMR_TIMEOUT2)
		caseTimer2 = 1;

	if (caseBcnInt || caseBcnDmaOK || caseBcnStatusOK || caseBcnStatusER){
		rtl8192cd_bcnProc(priv, caseBcnInt, caseBcnDmaOK, caseBcnStatusOK, caseBcnStatusER, status);
	}


	//
	// Rx interrupt
	//
	if (caseRxStatus)
	{
		// stop RX first
#ifdef __KERNEL__
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
#if defined(RX_TASKLET)
		if (!priv->pshare->has_triggered_rx_tasklet) {
			priv->pshare->has_triggered_rx_tasklet = 1;
			//RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_RDU | HIMR_ROK));
			RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
#else
		rtl8192cd_rx_isr(priv);
#endif

#else	// !(defined RTL8190_ISR_RX && RTL8190_DIRECT_RX)
		if (caseRxRDU) {
			rtl8192cd_rx_isr(priv);
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
		else {
			if (priv->pshare->rxInt_useTsklt)
				tasklet_hi_schedule(&priv->pshare->rx_tasklet);
			else
				rtl8192cd_rx_dsr((unsigned long)priv);
		}
#endif

#else	// !__KERNEL__
		rtl8192cd_rx_dsr((unsigned long)priv);
#endif
	}

	//
	// Tx interrupt
	//
#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE)
		rtl8192cd_tx_dsr((unsigned long)priv);
	else
#endif
	if ((CIRC_CNT_RTK(phw->txhead0, phw->txtail0, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead1, phw->txtail1, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead2, phw->txtail2, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead3, phw->txtail3, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead4, phw->txtail4, NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead5, phw->txtail5, NUM_TX_DESC) > 10)
	) {
#ifdef __KERNEL__
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
#else
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
	}

	if (caseTimer1) {
		RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT1);
		// process timer handler
	}

#ifdef SUPPORT_TX_AMSDU
	if (caseTimer2) {
		RTL_W32(IMR, RTL_R32(IMR) & ~IMR_TIMEOUT2);

		current_value = RTL_R32(TSFR) ;
		timeout = RTL_R32(TIMER1);
		if (TSF_LESS(current_value, timeout))
			setup_timer2(priv, timeout);
		else
			amsdu_timeout(priv, current_value);
	}
#endif

	return SUCCESS;
}


#ifdef __LINUX_2_6__
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
#else
__IRAM_IN_865X
#endif
irqreturn_t rtl8192cd_interrupt(int irq, void *dev_instance)
{
	int ret;

	ret = __rtl8192cd_interrupt(dev_instance);
	if (ret == FAIL)
		return IRQ_RETVAL(IRQ_NONE);
	else
		return IRQ_RETVAL(IRQ_HANDLED);
}
#else
__MIPS16
__IRAM_IN_865X
void rtl8192cd_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
	__rtl8192cd_interrupt(dev_instance);
	return;
}
#endif


static void rtl8192cd_set_rx_mode(struct net_device *dev)
{

}


static struct net_device_stats *rtl8192cd_get_stats(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;

#ifdef WDS
	int idx;
	struct stat_info *pstat;

	if (dev->base_addr == 0) {
		idx = getWdsIdxByDev(priv, dev);
		if (idx < 0) {
			memset(&priv->wds_stats[NUM_WDS-1], 0, sizeof(struct net_device_stats));
			return &priv->wds_stats[NUM_WDS-1];
		}

		if (netif_running(dev) && netif_running(priv->dev)) {
			pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[idx].macAddr);
			if (pstat == NULL) {
				DEBUG_ERR("%s: get_stainfo() wds fail!\n", (char *)__FUNCTION__);
				memset(&priv->wds_stats[idx], 0, sizeof(struct net_device_stats));
			}
			else {
				priv->wds_stats[idx].tx_packets = pstat->tx_pkts;
				priv->wds_stats[idx].tx_errors = pstat->tx_fail;
				priv->wds_stats[idx].tx_bytes = pstat->tx_bytes;
				priv->wds_stats[idx].rx_packets = pstat->rx_pkts;
				priv->wds_stats[idx].rx_bytes = pstat->rx_bytes;
			}
		}

		return &priv->wds_stats[idx];
	}
#endif

#ifdef CONFIG_RTK_MESH

	if (dev->base_addr == 1) {
		if(priv->mesh_dev != dev)
			return NULL;

		return &priv->mesh_stats;
	}
#endif // CONFIG_RTK_MESH
	return &(priv->net_stats);
}


static int rtl8192cd_init_sw(struct rtl8192cd_priv *priv)
{
	// All the index/counters should be reset to zero...
	struct rtl8192cd_hw *phw=NULL;
	unsigned long offset;
	unsigned int  i;
	struct sk_buff	*pskb;
	unsigned char	*page_ptr;
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll;
#endif
	unsigned long ring_virt_addr;
	unsigned long ring_dma_addr;
	unsigned int  ring_buf_len;
	unsigned char MIMO_TR_hw_support;
	unsigned int NumTotalRFPath;
#ifndef PRIV_STA_BUF
	unsigned long alloc_dma_buf;
#endif
#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	unsigned char *pbackup=NULL;
	unsigned long backup_len=0;
#endif
#ifdef _11s_TEST_MODE_
	struct Galileo_poll 	*pgalileo_poll;
#endif

#ifdef DFS
	// For JAPAN : prevent switching to channels 52, 56, 60, and 64 in adhoc mode
	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
	{
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			// block channels 52~64 and place them in NOP_chnl[4]
			if (!timer_pending(&priv->ch52_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
			if (!timer_pending(&priv->ch56_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
			if (!timer_pending(&priv->ch60_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
			if (!timer_pending(&priv->ch64_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
		}

		// if users select an illegal channel, the driver will switch to channel 36~48
		if (priv->pmib->dot11RFEntry.dot11channel >= 52) {
			PRINT_INFO("Channel %d is illegal in ad-hoc mode in Japan!\n", priv->pmib->dot11RFEntry.dot11channel);
			priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel();
			PRINT_INFO("Swiching to channel %d!\n", priv->pmib->dot11RFEntry.dot11channel);
		}
	}

	// if users select a blocked channel, the driver will switch to channel 36~48
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		((timer_pending(&priv->ch52_timer) && (priv->pmib->dot11RFEntry.dot11channel == 52)) ||
		 (timer_pending(&priv->ch56_timer) && (priv->pmib->dot11RFEntry.dot11channel == 56)) ||
		 (timer_pending(&priv->ch60_timer) && (priv->pmib->dot11RFEntry.dot11channel == 60)) ||
		 (timer_pending(&priv->ch64_timer) && (priv->pmib->dot11RFEntry.dot11channel == 64)))) {
		PRINT_INFO("Channel %d is still in none occupancy period!\n", priv->pmib->dot11RFEntry.dot11channel);
		priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel();
		PRINT_INFO("Swiching to channel %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	}

	// disable all of the transmissions during channel availability check
	priv->pmib->dot11DFSEntry.disable_tx = 0;
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		(priv->pmib->dot11RFEntry.dot11channel >= 52) &&
		(OPMODE & WIFI_AP_STATE))
		priv->pmib->dot11DFSEntry.disable_tx = 1;
#endif

#ifdef ENABLE_RTL_SKB_STATS
 	rtl_atomic_set(&priv->rtl_tx_skb_cnt, 0);
 	rtl_atomic_set(&priv->rtl_rx_skb_cnt, 0);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef __KERNEL__
#ifdef DFS
		// will not initialize the tasklet if the driver is rebooting due to the detection of radar
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
#ifdef PCIE_POWER_SAVING
				tasklet_init(&priv->pshare->ps_tasklet, PCIe_power_save_tasklet, (unsigned long)priv);
#endif
#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
				tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_dsr, (unsigned long)priv);
#else
#ifdef RX_TASKLET
				tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_tkl_isr, (unsigned long)priv);
#endif
#endif
				tasklet_init(&priv->pshare->tx_tasklet, rtl8192cd_tx_dsr, (unsigned long)priv);
				tasklet_init(&priv->pshare->oneSec_tasklet, rtl8192cd_expire_timer, (unsigned long)priv);
			}
		}
#endif // __KERNEL__

#ifdef DFS
		if (priv->pmib->dot11DFSEntry.DFS_detected)
			priv->pmib->dot11DFSEntry.DFS_detected = 0;
#endif

		phw = GET_HW(priv);

		// save descriptor virtual address before reset, david
		ring_virt_addr = phw->ring_virt_addr;
		ring_dma_addr = phw->ring_dma_addr;
		ring_buf_len = phw->ring_buf_len;
#ifndef PRIV_STA_BUF
		alloc_dma_buf = phw->alloc_dma_buf;
#endif

		// save RF related settings before reset
		MIMO_TR_hw_support = phw->MIMO_TR_hw_support;
		NumTotalRFPath = phw->NumTotalRFPath;

		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
		phw->ring_virt_addr = ring_virt_addr;
		phw->ring_buf_len = ring_buf_len;
#ifndef PRIV_STA_BUF
		phw->alloc_dma_buf = alloc_dma_buf;
#endif

#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			phw->ring_dma_addr = ring_dma_addr;
#endif
		phw->MIMO_TR_hw_support = MIMO_TR_hw_support;
		phw->NumTotalRFPath = NumTotalRFPath;
	}

#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	if (priv->reset_hangup &&
			(OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))) {
		backup_len = ((unsigned long)&((struct rtl8192cd_priv *)0)->br_ip) -
				 ((unsigned long)&((struct rtl8192cd_priv *)0)->join_res)+4;
		pbackup = kmalloc(backup_len, GFP_ATOMIC);
		if (pbackup)
			memcpy(pbackup, &priv->join_res, backup_len);
	}
#endif

	offset = (unsigned long)(&((struct rtl8192cd_priv *)0)->net_stats);
	// zero all data members below (including) stats
	memset((void *)((unsigned long)priv + offset), 0, sizeof(struct rtl8192cd_priv)-offset);

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
		priv->up_time = 0;
	}

#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	if (priv->reset_hangup && pbackup) {
		memcpy(&priv->join_res, pbackup, backup_len);
		kfree(pbackup);
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		// zero all data members below (including) LED_Timer of share_info
		offset = (unsigned long)(&((struct priv_shared_info*)0)->LED_Timer);
		memset((void *)((unsigned long)priv->pshare+ offset), 0, sizeof(struct priv_shared_info)-offset);

#ifdef CONFIG_RTK_MESH
		memset((void *)&priv->pshare->meshare, 0, sizeof(struct MESH_Share));
		get_random_bytes((void *)&priv->pshare->meshare.seq, sizeof(priv->pshare->meshare.seq));
#if (MESH_DBG_LV & MESH_DBG_COMPLEX)
		init_timer(&priv->pshare->meshare.mesh_test_sme_timer);
		priv->pshare->meshare.mesh_test_sme_timer.data = (unsigned long) priv;
		priv->pshare->meshare.mesh_test_sme_timer.function = mesh_test_sme_timer;
		mod_timer(&priv->pshare->meshare.mesh_test_sme_timer, jiffies + 200);
#endif // (MESH_DBG_LV & MESH_DBG_COMPLEX)

#if (MESH_DBG_LV & MESH_DBG_TEST)
		init_timer(&priv->pshare->meshare.mesh_test_sme_timer2);
		priv->pshare->meshare.mesh_test_sme_timer2.data = (unsigned long) priv;
		priv->pshare->meshare.mesh_test_sme_timer2.function = mesh_test_sme_timer2;
		mod_timer(&priv->pshare->meshare.mesh_test_sme_timer2, jiffies + 5000);
#endif // (MESH_DBG_LV & MESH_DBG_TEST)

#endif // CONFIG_RTK_MESH

		pwlan_hdr_poll = priv->pshare->pwlan_hdr_poll;
		pwlanllc_hdr_poll = priv->pshare->pwlanllc_hdr_poll;
		pwlanbuf_poll = priv->pshare->pwlanbuf_poll;
		pwlanicv_poll = priv->pshare->pwlanicv_poll;
		pwlanmic_poll = priv->pshare->pwlanmic_poll;

#ifdef _11s_TEST_MODE_
		pgalileo_poll = priv->pshare->galileo_poll;
		pgalileo_poll->count = AODV_RREQ_TABLE_SIZE;
#endif
		pwlan_hdr_poll->count = PRE_ALLOCATED_HDR;
		pwlanllc_hdr_poll->count = PRE_ALLOCATED_HDR;
		pwlanbuf_poll->count = PRE_ALLOCATED_MMPDU;
		pwlanicv_poll->count = PRE_ALLOCATED_HDR;
		pwlanmic_poll->count = PRE_ALLOCATED_HDR;

		// initialize all the hdr/buf node, and list to the poll_list
		INIT_LIST_HEAD(&priv->pshare->wlan_hdrlist);
		INIT_LIST_HEAD(&priv->pshare->wlanllc_hdrlist);
		INIT_LIST_HEAD(&priv->pshare->wlanbuf_list);
		INIT_LIST_HEAD(&priv->pshare->wlanicv_list);
		INIT_LIST_HEAD(&priv->pshare->wlanmic_list);

#ifdef _11s_TEST_MODE_	//Galileo

		memset(priv->rvTestPacket, 0, 3000);

		INIT_LIST_HEAD(&priv->pshare->galileo_list);
		INIT_LIST_HEAD(&priv->mtb_list);

		for(i=0; i< AODV_RREQ_TABLE_SIZE; i++)
		{
			INIT_LIST_HEAD(&(pgalileo_poll->node[i].list));
			list_add_tail(&(pgalileo_poll->node[i].list), &priv->pshare->galileo_list);
			init_timer(&pgalileo_poll->node[i].data.expire_timer);
			pgalileo_poll->node[i].data.priv = priv;
			pgalileo_poll->node[i].data.expire_timer.function = galileo_timer;
		}
#endif

		for(i=0; i< PRE_ALLOCATED_HDR; i++)
		{
			INIT_LIST_HEAD(&(pwlan_hdr_poll->hdrnode[i].list));
			list_add_tail(&(pwlan_hdr_poll->hdrnode[i].list), &priv->pshare->wlan_hdrlist);

			INIT_LIST_HEAD(&(pwlanllc_hdr_poll->hdrnode[i].list));
			list_add_tail( &(pwlanllc_hdr_poll->hdrnode[i].list), &priv->pshare->wlanllc_hdrlist);

			INIT_LIST_HEAD(&(pwlanicv_poll->hdrnode[i].list));
			list_add_tail( &(pwlanicv_poll->hdrnode[i].list), &priv->pshare->wlanicv_list);

			INIT_LIST_HEAD(&(pwlanmic_poll->hdrnode[i].list));
			list_add_tail( &(pwlanmic_poll->hdrnode[i].list), &priv->pshare->wlanmic_list);
		}

		for(i=0; i< PRE_ALLOCATED_MMPDU; i++)
		{
			INIT_LIST_HEAD(&(pwlanbuf_poll->hdrnode[i].list));
			list_add_tail( &(pwlanbuf_poll->hdrnode[i].list), &priv->pshare->wlanbuf_list);
		}

		DEBUG_INFO("hdrlist=%x, llc_hdrlist=%x, buf_list=%x, icv_list=%x, mic_list=%X\n",
			(UINT)&priv->pshare->wlan_hdrlist, (UINT)&priv->pshare->wlanllc_hdrlist, (UINT)&priv->pshare->wlanbuf_list,
			(UINT)&priv->pshare->wlanicv_list, (UINT)&priv->pshare->wlanmic_list);

		page_ptr = (unsigned char *)phw->ring_virt_addr;
		memset(page_ptr, 0, phw->ring_buf_len); // this is vital!

#ifndef __KERNEL__
		if ((unsigned int)page_ptr & (~(PAGE_SIZE - 1)))
			page_ptr = (unsigned char *)(((unsigned int)page_ptr & (~(PAGE_SIZE - 1))) + PAGE_SIZE);
#endif
#ifdef CONFIG_NET_PCI
		if (!IS_PCIBIOS_TYPE)
#endif
			phw->ring_dma_addr = virt_to_bus(page_ptr);

		phw->rx_ring_addr  = phw->ring_dma_addr;
		phw->tx_ring0_addr = phw->ring_dma_addr + NUM_RX_DESC * sizeof(struct rx_desc);
		phw->tx_ring1_addr = phw->tx_ring0_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring2_addr = phw->tx_ring1_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring3_addr = phw->tx_ring2_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring4_addr = phw->tx_ring3_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring5_addr = phw->tx_ring4_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->rxcmd_ring_addr = phw->tx_ring5_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->txcmd_ring_addr = phw->rxcmd_ring_addr + NUM_CMD_DESC * sizeof(struct rx_desc);
		phw->tx_ringB_addr = phw->txcmd_ring_addr + NUM_CMD_DESC * sizeof(struct tx_desc);

		phw->rx_descL = (struct rx_desc *)page_ptr;
		phw->tx_desc0 = (struct tx_desc *)(page_ptr + NUM_RX_DESC * sizeof(struct rx_desc));
		phw->tx_desc1 = (struct tx_desc *)((unsigned long)phw->tx_desc0 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc2 = (struct tx_desc *)((unsigned long)phw->tx_desc1 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc3 = (struct tx_desc *)((unsigned long)phw->tx_desc2 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc4 = (struct tx_desc *)((unsigned long)phw->tx_desc3 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc5 = (struct tx_desc *)((unsigned long)phw->tx_desc4 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->rxcmd_desc = (struct rx_desc *)((unsigned long)phw->tx_desc5 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->txcmd_desc = (struct tx_desc *)((unsigned long)phw->rxcmd_desc + NUM_CMD_DESC * sizeof(struct rx_desc));
		phw->tx_descB = (struct tx_desc *)((unsigned long)phw->txcmd_desc + NUM_CMD_DESC * sizeof(struct tx_desc));

		/* To set the DMA address for both RX/TX ring */
		{
#ifndef USE_RTL8186_SDK
			int txDescRingIdx;

			struct tx_desc *tx_desc_array[] = {
				phw->tx_desc0,
				phw->tx_desc1,
				phw->tx_desc2,
				phw->tx_desc3,
				phw->tx_desc4,
				phw->tx_desc5,
				phw->tx_descB,
				0
				};
#ifdef CONFIG_NET_PCI
			unsigned long *tx_desc_dma_array[] = {
				(unsigned long*)(phw->tx_desc0_dma_addr),
				(unsigned long*)(phw->tx_desc1_dma_addr),
				(unsigned long*)(phw->tx_desc2_dma_addr),
				(unsigned long*)(phw->tx_desc3_dma_addr),
				(unsigned long*)(phw->tx_desc4_dma_addr),
				(unsigned long*)(phw->tx_desc5_dma_addr),
				(unsigned long*)(phw->tx_descB_dma_addr),
				(unsigned long*)0
				};
#endif
#endif // !USE_RTL8186_SDK

			/* RX RING */
			for (i=0; i<NUM_RX_DESC; i++) {
				phw->rx_descL_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rx_descL[i]),
					sizeof(struct rx_desc), PCI_DMA_TODEVICE);
			}

#ifndef USE_RTL8186_SDK
			/* TX RING */
			txDescRingIdx = 0;

			while (tx_desc_array[txDescRingIdx] != 0) {
#ifdef CONFIG_NET_PCI
				unsigned long *tx_desc_dma_ptr = tx_desc_dma_array[txDescRingIdx];
#endif
				struct tx_desc *tx_desc_ptr = tx_desc_array[txDescRingIdx];

				for (i=0; i<NUM_TX_DESC; i++) {
					tx_desc_dma_ptr[i] = get_physical_addr(priv, (void *)(&(tx_desc_ptr[i])),
						sizeof(struct tx_desc), PCI_DMA_TODEVICE);
				}

				txDescRingIdx ++;
			}
#endif	// !USE_RTL8186_SDK

			for (i=0; i<NUM_CMD_DESC; i++) {
				phw->rxcmd_desc_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rxcmd_desc[i]),
					sizeof(struct rx_desc), PCI_DMA_TODEVICE);
				phw->txcmd_desc_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->txcmd_desc[i]),
					sizeof(struct tx_desc), PCI_DMA_TODEVICE);
			}
		}

		DEBUG_INFO("rx_descL=%08x tx_desc0=%08x, tx_desc1=%08x, tx_desc2=%08x, tx_desc3=%08x, tx_desc4=%08x, "
			"tx_desc5=%08x, rxcmd_desc=%08x, txcmd_desc=%08x, tx_descB=%08x\n",
			(UINT)phw->rx_descL, (UINT)phw->tx_desc0, (UINT)phw->tx_desc1, (UINT)phw->tx_desc2,
			(UINT)phw->tx_desc3, (UINT)phw->tx_desc4, (UINT)phw->tx_desc5,
			(UINT)phw->rxcmd_desc, (UINT)phw->txcmd_desc, (UINT)phw->tx_descB);
#ifdef RTK_QUE
		rtk_queue_init(&priv->pshare->skb_queue);
#else
		skb_queue_head_init(&priv->pshare->skb_queue);
#endif

		// Now for Rx desc...
		for(i=0; i<NUM_RX_DESC; i++)
		{
			pskb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 1);
			if (pskb == NULL) {
				printk("can't allocate skbuff for RX, abort!\n");
				return 1;
			}
			init_rxdesc(pskb, i, priv);
		}

		// Nothing to do for Tx desc...
		for(i=0; i<NUM_TX_DESC; i++)
		{
			init_txdesc(priv, phw->tx_desc0, phw->tx_ring0_addr, i);
			init_txdesc(priv, phw->tx_desc1, phw->tx_ring1_addr, i);
			init_txdesc(priv, phw->tx_desc2, phw->tx_ring2_addr, i);
			init_txdesc(priv, phw->tx_desc3, phw->tx_ring3_addr, i);
			init_txdesc(priv, phw->tx_desc4, phw->tx_ring4_addr, i);
			init_txdesc(priv, phw->tx_desc5, phw->tx_ring5_addr, i);
		}

#ifdef MBSSID
		for(i=0; i<(RTL8192CD_NUM_VWLAN+1); i++) {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				if (i == RTL8192CD_NUM_VWLAN)
					(phw->tx_descB + i)->Dword10 = set_desc(phw->tx_ringB_addr);
					//(phw->tx_descB + i)->NextTxDescAddress = set_desc(phw->tx_ringB_addr);
				else
					(phw->tx_descB + i)->Dword10 = set_desc(phw->tx_ringB_addr + (i+1) * sizeof(struct tx_desc));
					//(phw->tx_descB + i)->NextTxDescAddress = set_desc(phw->tx_ringB_addr + (i+1) * sizeof(struct tx_desc));
			}
		}
#endif

		for(i=0; i<NUM_CMD_DESC; i++) {
			if (i == (NUM_CMD_DESC - 1))// set NextAddrs
				(phw->txcmd_desc + i)->Dword9 = set_desc(phw->txcmd_ring_addr);
			else
				(phw->txcmd_desc + i)->Dword9 = set_desc(phw->txcmd_ring_addr + (i+1) * sizeof(struct tx_desc));
		}

		priv->pshare->amsdu_timer_head = priv->pshare->amsdu_timer_tail = 0;

#ifdef RX_BUFFER_GATHER
		INIT_LIST_HEAD(&priv->pshare->gather_list);
#endif		
	}

	INIT_LIST_HEAD(&priv->wlan_acl_list);
	INIT_LIST_HEAD(&priv->wlan_aclpolllist);

	pwlan_acl_poll = priv->pwlan_acl_poll;
	for(i=0; i< NUM_ACL; i++)
	{
		INIT_LIST_HEAD(&(pwlan_acl_poll->aclnode[i].list));
		list_add_tail(&(pwlan_acl_poll->aclnode[i].list), &priv->wlan_aclpolllist);
	}

	// copy acl from mib to link list
	for (i=0; i<priv->pmib->dot11StationConfigEntry.dot11AclNum; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->wlan_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist,	struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot11StationConfigEntry.dot11AclAddr[i], 6);
		paclnode->mode = (unsigned char)priv->pmib->dot11StationConfigEntry.dot11AclMode;

		list_add_tail(pnewlist, &priv->wlan_acl_list);
	}

	for(i=0; i<NUM_STAT; i++)
		INIT_LIST_HEAD(&(priv->stat_hash[i]));

#ifdef	CONFIG_RTK_MESH
	/*
	 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
	*/
#ifdef	_MESH_ACL_ENABLE_	// copy acl from mib to link list (below code copy above ACL code)
	INIT_LIST_HEAD(&priv->mesh_acl_list);
	INIT_LIST_HEAD(&priv->mesh_aclpolllist);

	pmesh_acl_poll = priv->pmesh_acl_poll;
	for(i=0; i< NUM_MESH_ACL; i++)
	{
		INIT_LIST_HEAD(&(pmesh_acl_poll->meshaclnode[i].list));
		list_add_tail(&(pmesh_acl_poll->meshaclnode[i].list), &priv->mesh_aclpolllist);
	}

	for (i=0; i<priv->pmib->dot1180211sInfo.mesh_acl_num; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->mesh_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist, struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot1180211sInfo.mesh_acl_addr[i], MACADDRLEN);
		paclnode->mode = (unsigned char)priv->pmib->dot1180211sInfo.mesh_acl_mode;

		list_add_tail(pnewlist, &priv->mesh_acl_list);
	}
#endif

#ifdef MESH_BOOTSEQ_AUTH
	INIT_LIST_HEAD(&(priv->mesh_auth_hdr));
#endif

	INIT_LIST_HEAD(&(priv->mesh_unEstablish_hdr));
	INIT_LIST_HEAD(&(priv->mesh_mp_hdr));

	priv->mesh_profile[0].used = FALSE; // Configure by WEB in the future, Maybe delete, Preservation before delete
#endif

	INIT_LIST_HEAD(&(priv->asoc_list));
	INIT_LIST_HEAD(&(priv->auth_list));
	INIT_LIST_HEAD(&(priv->sleep_list));
	INIT_LIST_HEAD(&(priv->defrag_list));
	INIT_LIST_HEAD(&(priv->wakeup_list));
	INIT_LIST_HEAD(&(priv->rx_datalist));
	INIT_LIST_HEAD(&(priv->rx_mgtlist));
	INIT_LIST_HEAD(&(priv->rx_ctrllist));
	INIT_LIST_HEAD(&(priv->addRAtid_list));	// to avoid add RAtid fail
	INIT_LIST_HEAD(&(priv->addrssi_list));

#ifdef A4_STA
	INIT_LIST_HEAD(&(priv->a4_sta_list));
#endif

#ifdef CHECK_BEACON_HANGUP
	if (priv->reset_hangup)
	   	priv->pshare->beacon_wait_cnt = 1;
	else
	   	priv->pshare->beacon_wait_cnt = 2;
#endif

#ifdef CHECK_HANGUP
	if (priv->reset_hangup) {
		get_available_channel(priv);
		validate_oper_rate(priv);
		get_oper_rate(priv);
		DOT11_InitQueue(priv->pevent_queue);
		return 0;
	}
#endif

	// construct operation and basic rates set
	{
		// validate region domain
		if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain < DOMAIN_FCC) ||
				(priv->pmib->dot11StationConfigEntry.dot11RegDomain >= DOMAIN_MAX)) {
			PRINT_INFO("invalid region domain, use default value [DOMAIN_FCC]!\n");
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FCC;
		}

		// validate band
		if (priv->pmib->dot11BssType.net_work_type == 0) {
			PRINT_INFO("operation band is not set, use G+B as default!\n");
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G;
		}
		if ((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G))) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				priv->pmib->dot11BssType.net_work_type &= (WIRELESS_11B | WIRELESS_11G);
				PRINT_INFO("operation band not appropriate, use G/B as default!\n");
			}
		}

		if (should_forbid_Nmode(priv))
			priv->pmib->dot11BssType.net_work_type &= ~WIRELESS_11N;

		// validate channel number
		if (get_available_channel(priv) == FAIL) {
			PRINT_INFO("can't get operation channels, abort!\n");
			return 1;
		}
		if (priv->pmib->dot11RFEntry.dot11channel != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11channel == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];

				PRINT_INFO("invalid channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11channel);
			}
			priv->auto_channel = 0;
#ifdef SIMPLE_CH_UNI_PROTOCOL
			SET_PSEUDO_RANDOM_NUMBER(priv->mesh_ChannelPrecedence);
#endif
		}
		else {
#ifdef SIMPLE_CH_UNI_PROTOCOL
			if(GET_MIB(priv)->dot1180211sInfo.mesh_enable)
				priv->auto_channel = 16;
			else
#endif
			{
			if (OPMODE & WIFI_AP_STATE)
				priv->auto_channel = 1;
			else
				priv->auto_channel = 2;
			priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
			}
		}
		priv->auto_channel_backup = priv->auto_channel;

		// validate hi and low channel
		if (priv->pmib->dot11RFEntry.dot11ch_low != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11ch_low == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11ch_low = priv->available_chnl[0];

				PRINT_INFO("invalid low channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11ch_low);
			}
		}
		if (priv->pmib->dot11RFEntry.dot11ch_hi != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11ch_hi == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11ch_hi = priv->available_chnl[priv->available_chnl_num-1];

				PRINT_INFO("invalid hi channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11ch_hi);
			}
		}

// Mark the code to auto disable N mode in WEP encrypt
#if 0
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
				(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_))
			priv->pmib->dot11BssType.net_work_type &= ~WIRELESS_11N;
#endif
//------------------------------ david+2008-01-11

		// support cck only in channel 14
		if ((priv->pmib->dot11RFEntry.disable_ch14_ofdm) &&
			(priv->pmib->dot11RFEntry.dot11channel == 14)) {
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B;
			PRINT_INFO("support cck only in channel 14!\n");
		}

		// validate and get support and basic rates
		validate_oper_rate(priv);
		get_oper_rate(priv);

		if (priv->pmib->dot11nConfigEntry.dot11nUse40M &&
			(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))) {
			PRINT_INFO("enable 40M but not in N mode! back to 20M\n");
			priv->pmib->dot11nConfigEntry.dot11nUse40M = 0;
		}

		// check deny band
		if ((priv->pmib->dot11BssType.net_work_type & (~priv->pmib->dot11StationConfigEntry.legacySTADeny)) == 0) {
			PRINT_INFO("legacySTADeny %d not suitable! set to 0\n", priv->pmib->dot11StationConfigEntry.legacySTADeny);
			priv->pmib->dot11StationConfigEntry.legacySTADeny = 0;
		}
	}

	if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)) {
		if (AMSDU_ENABLE)
			AMSDU_ENABLE = 0;
		if (AMPDU_ENABLE)
			AMPDU_ENABLE = 0;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pshare->is_40m_bw = 0;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
		}
	}
	else {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
			if (priv->pshare->is_40m_bw == 0)
				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
			else {
				if ((priv->pmib->dot11RFEntry.dot11channel < 5) &&
						(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_BELOW))
					priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
				else if ((priv->pmib->dot11RFEntry.dot11channel > 9) &&
						(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_ABOVE))
					priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
				else
					priv->pshare->offset_2nd_chan = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
			}
		}

		// force wmm enabled if n mode
		QOS_ENABLE = 1;
	}

	// set wep key length
	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = 8;
	else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = 16;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		init_crc32_table();	// for sw encryption
	}

#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if ((OPMODE & WIFI_AP_STATE)
#ifdef CLIENT_MODE
			|| (OPMODE & WIFI_ADHOC_STATE)
#endif
			) {
			GET_EDCA_PARA_UPDATE = 0;
			//BK
			GET_STA_AC_BK_PARA.AIFSN = 7;
			GET_STA_AC_BK_PARA.TXOPlimit = 0;
			GET_STA_AC_BK_PARA.ACM = 0;
			GET_STA_AC_BK_PARA.ECWmin = 4;
			GET_STA_AC_BK_PARA.ECWmax = 10;
			//BE
			GET_STA_AC_BE_PARA.AIFSN = 3;
			GET_STA_AC_BE_PARA.TXOPlimit = 0;
			GET_STA_AC_BE_PARA.ACM = 0;
			GET_STA_AC_BE_PARA.ECWmin = 4;
			GET_STA_AC_BE_PARA.ECWmax = 10;
			//VI
			GET_STA_AC_VI_PARA.AIFSN = 2;
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				GET_STA_AC_VI_PARA.TXOPlimit = 94; // 3.008ms
			else
				GET_STA_AC_VI_PARA.TXOPlimit = 188; // 6.016ms
			GET_STA_AC_VI_PARA.ACM = 0;
			GET_STA_AC_VI_PARA.ECWmin = 3;
			GET_STA_AC_VI_PARA.ECWmax = 4;
			//VO
			GET_STA_AC_VO_PARA.AIFSN = 2;
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				GET_STA_AC_VO_PARA.TXOPlimit = 47; // 1.504ms
			else
				GET_STA_AC_VO_PARA.TXOPlimit = 102; // 3.264ms
			GET_STA_AC_VO_PARA.ACM = 0;
			GET_STA_AC_VO_PARA.ECWmin = 2;
			GET_STA_AC_VO_PARA.ECWmax = 3;

			//init WMM Para ie in beacon
			init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_PARA_IE);
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE)
			init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_IE);  //  WMM STA
#endif
	}
#endif

	i = priv->pmib->dot11ErpInfo.ctsToSelf;
	memset(&priv->pmib->dot11ErpInfo, '\0', sizeof(struct erp_mib)); // reset ERP mib
	priv->pmib->dot11ErpInfo.ctsToSelf = i;

	if ( (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
			(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) )
		priv->pmib->dot11ErpInfo.shortSlot = 1;
	else
		priv->pmib->dot11ErpInfo.shortSlot = 0;

	if (OPMODE & WIFI_AP_STATE) {
		memcpy(priv->pmib->dot11StationConfigEntry.dot11Bssid,
				priv->pmib->dot11OperationEntry.hwaddr, 6);
		//priv->oper_band = priv->pmib->dot11BssType.net_work_type;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if (!priv->pshare->ra40MLowerMinus && (priv->pshare->rf_ft_var.raGoUp40MLower > 5) &&
				(priv->pshare->rf_ft_var.raGoDown40MLower > 5)) {
				priv->pshare->rf_ft_var.raGoUp40MLower-=5;
				priv->pshare->rf_ft_var.raGoDown40MLower-=5;
				priv->pshare->ra40MLowerMinus++;
			}
#ifdef HIGH_POWER_EXT_PA
			if (!priv->pshare->raThdHP_Minus) {

				if( priv->pshare->rf_ft_var.use_ext_pa )  {
					if(priv->pshare->rf_ft_var.raGoDownUpper > RSSI_DIFF_PA) {
						priv->pshare->rf_ft_var.raGoDownUpper -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUpUpper -= RSSI_DIFF_PA;
					}
					if(priv->pshare->rf_ft_var.raGoDown20MLower > RSSI_DIFF_PA) {					
						priv->pshare->rf_ft_var.raGoDown20MLower -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUp20MLower -= RSSI_DIFF_PA;
					}
					if(priv->pshare->rf_ft_var.raGoDown40MLower > RSSI_DIFF_PA) {	
						priv->pshare->rf_ft_var.raGoDown40MLower -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUp40MLower -= RSSI_DIFF_PA;		
					}
				}
				++priv->pshare->raThdHP_Minus;
			}
#endif
		}
	}
#ifdef CLIENT_MODE
	else {
		if (priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen == 0) {
			priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen = 11;
			memcpy(priv->pmib->dot11StationConfigEntry.dot11DefaultSSID, "defaultSSID", 11);
		}
		memset(priv->pmib->dot11StationConfigEntry.dot11Bssid, 0, 6);
		priv->join_res = STATE_Sta_No_Bss;

// Add mac clone address manually ----------
#ifdef RTK_BR_EXT
		if (priv->pmib->ethBrExtInfo.macclone_enable == 2) {
			extern void mac_clone(struct rtl8192cd_priv *priv, unsigned char *addr);
			mac_clone(priv, priv->pmib->ethBrExtInfo.nat25_dmzMac);
			priv->macclone_completed = 1;
		}
#endif
//------------------------- david+2007-5-31

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if (priv->pshare->ra40MLowerMinus) {
				priv->pshare->rf_ft_var.raGoUp40MLower+=5;
				priv->pshare->rf_ft_var.raGoDown40MLower+=5;
				priv->pshare->ra40MLowerMinus = 0;
			}
		}
	}
#endif

	// initialize event queue
	DOT11_InitQueue(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_InitQueue(priv->wapiEvent_queue);
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv) && priv->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			DOT11_InitQueue(priv->pvap_priv[i]->wapiEvent_queue);
	}
#endif

#endif

#ifdef CONFIG_RTK_MESH
	if(GET_MIB(priv)->dot1180211sInfo.mesh_enable == 1)	// plus add 0217, not mesh mode should not do below function
	{
	DOT11_InitQueue2(priv->pathsel_queue, MAXQUEUESIZE2, MAXDATALEN2);
#ifdef	_11s_TEST_MODE_
	DOT11_InitQueue2(priv->receiver_queue, MAXQUEUESIZE2, MAXDATALEN2);
#endif
		//modify by Joule for SECURITY
		i = priv->pmib->dot11sKeysTable.dot11Privacy;
		memset(&priv->pmib->dot11sKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset key
		priv->pmib->dot11sKeysTable.dot11Privacy = i;
	 }	//
#endif

#ifdef __DRAYTEK_OS__
	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_ &&
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_ &&
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_) {
#ifdef UNIVERSAL_REPEATER
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
#ifdef UNIVERSAL_REPEATER
			if (GET_VXD_PRIV(priv))
				GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
#endif
		}
	}
#endif

	i = RC_ENTRY_NUM;
	for (;;) {
		if (priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz >= i) {
			priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz = i;
			break;
		}
		else if (i > 8)
			i = i / 2;
		else {
			priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz = 8;
			break;
		}
	}

	// Realtek proprietary IE
	memcpy(&(priv->pshare->rtk_ie_buf[0]), Realtek_OUI, 3);
	priv->pshare->rtk_ie_buf[3] = 2;
	priv->pshare->rtk_ie_buf[4] = 1;
	priv->pshare->rtk_ie_buf[5] = 0;
	priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_WLAN_88C92C | RTK_CAP_IE_WLAN_8192SE;
#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE)
		priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_AP_CLIENT;
#endif
	priv->pshare->rtk_ie_len = 6;


#ifdef WIFI_HAPD
	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 0)
		&& (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
		&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher || priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher))
			rsn_init(priv);
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
		psk_init(priv);

#ifdef WDS
#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef LAZY_WDS
		if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) {
			priv->pmib->dot11WdsInfo.wdsNum = 0;
			memset(priv->pmib->dot11WdsInfo.entry, '\0', sizeof(struct wdsEntry)*NUM_WDS);
		}
#endif
		if ((priv->pmib->dot11WdsInfo.wdsEnabled) && (priv->pmib->dot11WdsInfo.wdsNum > 0) &&
			((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_) ||
			 (priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_)))
			wds_psk_init(priv);
	}
#endif
#endif

#ifdef GBWC
	priv->GBWC_tx_queue.head = 0;
	priv->GBWC_tx_queue.tail = 0;
	priv->GBWC_rx_queue.head = 0;
	priv->GBWC_rx_queue.tail = 0;
	priv->GBWC_tx_count = 0;
	priv->GBWC_rx_count = 0;
	priv->GBWC_consuming_Q = 0;
#endif

	priv->release_mcast = 0;

#ifdef USB_PKT_RATE_CTRL_SUPPORT //mark_test
	priv->change_toggle = 0;
	priv->pre_pkt_cnt = 0;
	priv->pkt_nsec_diff = 0;
	priv->poll_usb_cnt = 0;
	priv->auto_rate_mask = 0;
#endif

#ifdef STA_EXT
	priv->pshare->fw_free_space = FW_NUM_STAT - 2; // One for MAGANEMENT_AID, one for other STAs
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (priv->pmib->vlan.global_vlan)
		priv->pmib->dot11OperationEntry.disable_brsc = 1;
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		/*	set NMK	*/
		GenerateRandomData(priv->wapiNMK, WAPI_KEY_LEN);
		priv->wapiMCastKeyId = 0;
		priv->wapiMCastKeyUpdate = 0;
		wapiInit(priv);
	}

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			if (priv->pvap_priv[i]->pmib->wapiInfo.wapiType!=wapiDisable) {
				/*  set NMK */
				GenerateRandomData(priv->pvap_priv[i]->wapiNMK, WAPI_KEY_LEN);
				priv->pvap_priv[i]->wapiMCastKeyId = 0;
				priv->pvap_priv[i]->wapiMCastKeyUpdate = 0;
				wapiInit(priv->pvap_priv[i]);
			}
	}
#endif
#endif

#ifdef MBSSID
	// if vap enabled, set beacon int to 100 at minimun
	if ((OPMODE & WIFI_AP_STATE) && GET_ROOT(priv)->pmib->miscEntry.vap_enable
		&& priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 100)
		priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = 100;
#endif

#ifdef DOT11D
	{
		extern COUNTRY_IE_ELEMENT countryIEArray[];
		int found = 0;
		char *CStringPtr =	priv->pmib->dot11dCountry.dot11CountryString ;

		if ((OPMODE & WIFI_AP_STATE) && COUNTRY_CODE_ENABLED) {
			for (i=0; i<COUNTRYNUMBER; i++) {
				if (!memcmp(CStringPtr, countryIEArray[i].countryA2, 2)) {
					priv->pshare->countryTabIdx = i;
					found = 1;
					break;
				}
			}

			if (found == 0) {
				priv->pshare->countryTabIdx = 1;
				printk("can't found country code(%s),use default region\n",CStringPtr);
			}

			i = priv->pshare->countryTabIdx;

			if (countryIEArray[i].A_Band_Region == 0) {
				priv->pshare->countryBandUsed=0; /*2.4G*/
			} else {
				/*5G*/
				if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
					priv->pshare->countryBandUsed = 1;	/*5G*/
					//printk("2.4G and 5G both enabled, select 5G \n");
				} else {
					priv->pshare->countryBandUsed = 0;	/*2.4G*/
					//printk("2.4G and 5G both enabled, select 2.4G \n");
				}
			}
		}
	}
#endif

#ifdef RTL_MANUAL_EDCA
	for (i=0; i<8; i++) {
		if ((priv->pmib->dot11QosEntry.TID_mapping[i] < 1) || (priv->pmib->dot11QosEntry.TID_mapping[i] > 4))
			priv->pmib->dot11QosEntry.TID_mapping[i] = 2;
	}
#endif

#ifdef TXREPORT
	priv->pshare->sta_query_idx=-1;

	// Init StaDetectInfo to detect disappearing STA. Added by Annie, 2010-08-10.
	priv->pmib->staDetectInfo.txRprDetectPeriod = 1;
#endif

#ifdef	INCLUDE_WPS
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
			wps_init(priv);
#endif

	return 0;
}


static int rtl8192cd_stop_sw(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw;
	unsigned long	flags;
	int	i;

	// we hope all this can be done in critical section
	SAVE_INT_AND_CLI(flags);

#ifdef INCLUDE_WPS
   	priv->pshare->WSC_CONT_S.wait_reinit = 1 ;
#endif

	if (timer_pending(&priv->frag_to_filter))
		del_timer_sync(&priv->frag_to_filter);

#ifdef DETECT_STA_EXISTANCE
	// Added by Annie for Retry Limit Recovery Timer, 2010-08-10.
	if (timer_pending(&priv->pshare->rl_recover_timer))
		del_timer_sync (&priv->pshare->rl_recover_timer);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		if (timer_pending(&priv->expire_timer))
			del_timer_sync(&priv->expire_timer);
#ifdef 	SW_ANT_SWITCH
		if (timer_pending(&priv->pshare->swAntennaSwitchTimer))
			del_timer_sync(&priv->pshare->swAntennaSwitchTimer);
#endif		
		if (timer_pending(&priv->pshare->rc_sys_timer))
			del_timer_sync(&priv->pshare->rc_sys_timer);
#if 0
		if (timer_pending(&priv->pshare->phw->tpt_timer))
			del_timer_sync(&priv->pshare->phw->tpt_timer);
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (priv->pmib->wapiInfo.wapiType!=wapiDisable
#ifdef CHECK_HANGUP
		&&(!priv->reset_hangup)
#endif
		)
		{
			wapiExit(priv);
		}


#ifdef MBSSID
	if( priv->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (priv->pvap_priv[i]->pmib->wapiInfo.wapiType!=wapiDisable
#ifdef CHECK_HANGUP
				&&(!priv->pvap_priv[i]->reset_hangup)
#endif
			) {
				wapiExit(priv->pvap_priv[i]);
			}
		}
	}
#endif

#ifdef PCIE_POWER_SAVING
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);
#endif

#endif
#ifdef CONFIG_RTK_MESH
		/*
		 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
		 */
		if (timer_pending(&priv->mesh_peer_link_timer))
			del_timer_sync(&priv->mesh_peer_link_timer);

#ifdef MESH_BOOTSEQ_AUTH
		if (timer_pending(&priv->mesh_auth_timer))
			del_timer_sync(&priv->mesh_auth_timer);
#endif
#endif
	}
	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);
	if (timer_pending(&priv->MIC_check_timer))
		del_timer_sync(&priv->MIC_check_timer);
	if (timer_pending(&priv->assoc_reject_timer))
		del_timer_sync(&priv->assoc_reject_timer);
	// to avoid add RAtid fail
	if (timer_pending(&priv->add_RATid_timer))
		del_timer_sync(&priv->add_RATid_timer);
	if (timer_pending(&priv->add_rssi_timer))
		del_timer_sync(&priv->add_rssi_timer);
	if (timer_pending(&priv->add_ps_timer))
		del_timer_sync(&priv->add_ps_timer);

#ifdef CLIENT_MODE
	if (timer_pending(&priv->reauth_timer))
		del_timer_sync(&priv->reauth_timer);
	if (timer_pending(&priv->reassoc_timer))
		del_timer_sync(&priv->reassoc_timer);
	if (timer_pending(&priv->idle_timer))
		del_timer_sync(&priv->idle_timer);
#endif

#ifdef GBWC
	if (timer_pending(&priv->GBWC_timer))
		del_timer_sync(&priv->GBWC_timer);
	while (CIRC_CNT(priv->GBWC_tx_queue.head, priv->GBWC_tx_queue.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = priv->GBWC_tx_queue.pSkb[priv->GBWC_tx_queue.tail];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		priv->GBWC_tx_queue.tail++;
		priv->GBWC_tx_queue.tail = priv->GBWC_tx_queue.tail & (NUM_TXPKT_QUEUE - 1);
	}
	while (CIRC_CNT(priv->GBWC_rx_queue.head, priv->GBWC_rx_queue.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = priv->GBWC_rx_queue.pSkb[priv->GBWC_rx_queue.tail];
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		priv->GBWC_rx_queue.tail++;
		priv->GBWC_rx_queue.tail = priv->GBWC_rx_queue.tail & (NUM_TXPKT_QUEUE - 1);
	}
#endif

#ifdef INCLUDE_WPA_PSK
	if (timer_pending(&priv->wpa_global_info->GKRekeyTimer))
		del_timer_sync(&priv->wpa_global_info->GKRekeyTimer);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef DFS
		if (timer_pending(&priv->DFS_timer))
			del_timer_sync(&priv->DFS_timer);

		if (timer_pending(&priv->ch_avail_chk_timer))
			del_timer_sync(&priv->ch_avail_chk_timer);

		// when we disable the DFS function dynamically, we also remove the channel
		// from NOP_chnl[4] while the driver is rebooting
		if (timer_pending(&priv->ch52_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch52_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
		}

		if (timer_pending(&priv->ch56_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch56_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
		}

		if (timer_pending(&priv->ch60_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch60_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
		}

		if (timer_pending(&priv->ch64_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch64_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
		}

		// For JAPAN in adhoc mode
		if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK)	||
			 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
			 (OPMODE & WIFI_ADHOC_STATE)) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				if (!timer_pending(&priv->ch52_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
				if (!timer_pending(&priv->ch56_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
				if (!timer_pending(&priv->ch60_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
				if (!timer_pending(&priv->ch64_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
			}
		}
#endif // DFS

		// for SW LED
		if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
			disable_sw_LED(priv);

#ifdef __KERNEL__
#ifdef DFS
		// prevent killing tasklet issue in interrupt
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
				tasklet_kill(&priv->pshare->rx_tasklet);
				tasklet_kill(&priv->pshare->tx_tasklet);
				tasklet_kill(&priv->pshare->oneSec_tasklet);
			}
		}
#endif // __KERNEL__

		phw = GET_HW(priv);

#ifdef DELAY_REFILL_RX_BUF
                priv->pshare->phw->cur_rx_refill = priv->pshare->phw->cur_rx = 0;       // avoid refill to rx ring
#endif
		for (i=0; i<NUM_RX_DESC; i++)
		{
			if (phw->rx_infoL[i].pbuf != NULL) {
#ifdef CONFIG_NET_PCI
				// if pci bios, then pci_unmap_single and dev_kfree_skb
				if (IS_PCIBIOS_TYPE)
					pci_unmap_single(priv->pshare->pdev, phw->rx_infoL[i].paddr, (RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
#endif
				rtl_kfree_skb(priv, (struct sk_buff*)(phw->rx_infoL[i].pbuf), _SKB_RX_);
			}
		}

		// free the skb buffer in Low and Hi queue
		DEBUG_INFO("free tx Q0 head %d tail %d\n", phw->txhead0, phw->txtail0);
		DEBUG_INFO("free tx Q1 head %d tail %d\n", phw->txhead1, phw->txtail1);
		DEBUG_INFO("free tx Q2 head %d tail %d\n", phw->txhead2, phw->txtail2);
		DEBUG_INFO("free tx Q3 head %d tail %d\n", phw->txhead3, phw->txtail3);
		DEBUG_INFO("free tx Q4 head %d tail %d\n", phw->txhead4, phw->txtail4);
		DEBUG_INFO("free tx Q5 head %d tail %d\n", phw->txhead5, phw->txtail5);

		for (i=0; i<NUM_TX_DESC; i++)
		{
			// free tx queue skb
			struct tx_desc_info *tx_info;
			int j;
			int	head, tail;

			for (j=0; j<=HIGH_QUEUE; j++) {
				head = get_txhead(phw, j);
				tail = get_txtail(phw, j);
//				if (i <tail || i >= head)
				if( (tail < head) ? (i <tail || i >= head) :(i <tail && i >= head))
					continue;

				tx_info = get_txdesc_info(priv->pshare->pdesc_info, j);

				if (tx_info[i].pframe &&
					(tx_info[i].type == _SKB_FRAME_TYPE_)) {
#ifdef CONFIG_NET_PCI
					if (IS_PCIBIOS_TYPE)
						pci_unmap_single(priv->pshare->pdev, tx_info[i].paddr, (tx_info[i].len), PCI_DMA_TODEVICE);
#endif
					rtl_kfree_skb(priv, tx_info[i].pframe, _SKB_TX_);
					DEBUG_INFO("free skb in queue %d\n", j);
				}
			}
		} // TX descriptor Free

#ifdef CONFIG_NET_PCI
		// unmap  beacon buffer
		if (IS_PCIBIOS_TYPE) {
			pci_unmap_single(priv->pshare->pdev, get_desc(phw->tx_descB->Dword8),
				128*sizeof(unsigned int), PCI_DMA_TODEVICE);
		}
#endif

#ifdef RX_BUFFER_GATHER
		flush_rx_list(priv);
#endif
	} // if (IS_ROOT_INTERFACE(priv))

	for (i=0; i<NUM_STAT; i++)
	{
		if (priv->pshare->aidarray[i]) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv != priv->pshare->aidarray[i]->priv)
				continue;
			else
#endif
			{
				if (priv->pshare->aidarray[i]->used == TRUE)
					if (free_stainfo(priv, &(priv->pshare->aidarray[i]->station)) == FALSE)
					DEBUG_ERR("free station %d fails\n", i);

#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
				free_sta_que(priv, priv->pshare->aidarray[i]->station.VO_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.VI_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.BE_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.BK_dz_queue);

#else
				kfree(priv->pshare->aidarray[i]->station.VO_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.VI_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BE_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BK_dz_queue);
#endif
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
				free_sta_mgt_que(priv, priv->pshare->aidarray[i]->station.MGT_dz_queue);
#else
				kfree(priv->pshare->aidarray[i]->station.MGT_dz_queue);
#endif
#endif

#ifdef INCLUDE_WPA_PSK
#ifdef PRIV_STA_BUF
				free_wpa_buf(priv, priv->pshare->aidarray[i]->station.wpa_sta_info);
#else
				kfree(priv->pshare->aidarray[i]->station.wpa_sta_info);
#endif
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
			{
				unsigned int index = (unsigned int)i;
				rtl8192cd_dmem_free(AID_OBJ, &index);
			}
#else
#ifdef PRIV_STA_BUF
				free_sta_obj(priv, priv->pshare->aidarray[i]);
#else
				kfree(priv->pshare->aidarray[i]);
#endif
#endif
				priv->pshare->aidarray[i] = NULL;
			}
		}
	}

#ifndef __DRAYTEK_OS__
	// reset rsnie and group key from open to here, david
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef WIFI_SIMPLE_CONFIG
		if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->wscEntry.beacon_ielen = 0;
			priv->pmib->wscEntry.probe_rsp_ielen = 0;
			priv->pmib->wscEntry.probe_req_ielen = 0;
			priv->pmib->wscEntry.assoc_ielen = 0;
		}

//		if (!(OPMODE & WIFI_AP_STATE))
//			priv->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif
	}

	if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
		priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
		memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
#ifdef UNIVERSAL_REPEATER
		if (GET_VXD_PRIV(priv))
			GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
#endif
		priv->auto_channel_backup = 0;
	}
	else {
		// When wlan scheduling and auto-chan case, it will disable/enable
		// wlan interface directly w/o re-set mib. Therefore, we need use
		// "keep_rsnie" flag to keep auto-chan value

		if (
#ifdef CHECK_HANGUP
			!priv->reset_hangup &&
#endif
			priv->auto_channel_backup)
			priv->pmib->dot11RFEntry.dot11channel = 0;
	}
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv) && !priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
		}
	}
#endif

#ifdef RTK_BR_EXT
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
			nat25_db_cleanup(priv);
	}
#endif

#ifdef A4_STA
	if (OPMODE & WIFI_AP_STATE) {
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
			a4_sta_cleanup(priv);
	}
#endif

	{
		int				hd, tl;
		struct sk_buff	*pskb;

		hd = priv->dz_queue.head;
		tl = priv->dz_queue.tail;
		while (CIRC_CNT(hd, tl, NUM_TXPKT_QUEUE))
		{
			pskb = priv->dz_queue.pSkb[tl];
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
			tl++;
			tl = tl & (NUM_TXPKT_QUEUE - 1);
		}
		priv->dz_queue.head = 0;
		priv->dz_queue.tail = 0;
	}

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	priv->pshare->ThermalValue = 0;

	RESTORE_INT(flags);
	return 0;
}


#ifdef MBSSID
static void rtl8192cd_init_vap_mib(struct rtl8192cd_priv *priv)
{
#if 0
	unsigned char tmpbuf[36], hwaddr[6];
	int len;
	//unsigned int AclMode;
	//unsigned char AclAddr[NUM_ACL][MACADDRLEN];
	//unsigned int AclNum;
	struct Dot1180211AuthEntry dot1180211AuthEntry;
	struct Dot118021xAuthEntry dot118021xAuthEntry;
	struct Dot11DefaultKeysTable dot11DefaultKeysTable;
	struct Dot11RsnIE dot11RsnIE;

	// backup mib that can differ from root interface
	memcpy(hwaddr, GET_MY_HWADDR, 6);
	len = SSID_LEN;
	memcpy(tmpbuf, SSID, len);
	//AclMode = priv->pmib->dot11StationConfigEntry.dot11AclMode;
	//memcpy(AclAddr, priv->pmib->dot11StationConfigEntry.dot11AclAddr, sizeof(AclAddr));
	//AclNum = priv->pmib->dot11StationConfigEntry.dot11AclNum;
	memcpy(&dot1180211AuthEntry, &priv->pmib->dot1180211AuthEntry, sizeof(struct Dot1180211AuthEntry));
	memcpy(&dot118021xAuthEntry, &priv->pmib->dot118021xAuthEntry, sizeof(struct Dot118021xAuthEntry));
	memcpy(&dot11DefaultKeysTable, &priv->pmib->dot11DefaultKeysTable, sizeof(struct Dot11DefaultKeysTable));
	memcpy(&dot11RsnIE, &priv->pmib->dot11RsnIE, sizeof(struct Dot11RsnIE));

	// copy mib from root interface
	memcpy(priv->pmib, GET_ROOT_PRIV(priv)->pmib, sizeof(struct wifi_mib));

	// restore the different part
	memcpy(GET_MY_HWADDR, hwaddr, 6);
	SSID_LEN = len;
	memcpy(SSID, tmpbuf, len);
	SSID2SCAN_LEN = len;
	memcpy(SSID2SCAN, SSID, len);
	//priv->pmib->dot11StationConfigEntry.dot11AclMode = AclMode;
	//memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr, AclAddr, sizeof(AclAddr));
	//priv->pmib->dot11StationConfigEntry.dot11AclNum = AclNum;
	memcpy(&priv->pmib->dot1180211AuthEntry, &dot1180211AuthEntry, sizeof(struct Dot1180211AuthEntry));
	memcpy(&priv->pmib->dot118021xAuthEntry, &dot118021xAuthEntry, sizeof(struct Dot118021xAuthEntry));
	memcpy(&priv->pmib->dot11DefaultKeysTable, &dot11DefaultKeysTable, sizeof(struct Dot11DefaultKeysTable));
	memcpy(&priv->pmib->dot11RsnIE, &dot11RsnIE, sizeof(struct Dot11RsnIE));
#endif

	// copy mib_rf from root interface
	memcpy(&priv->pmib->dot11RFEntry, &GET_ROOT_PRIV(priv)->pmib->dot11RFEntry, sizeof(struct Dot11RFEntry));

	// special mib that need to set
#ifdef WIFI_WMM
	//QOS_ENABLE = 0;
#ifdef WMM_APSD
	APSD_ENABLE = 0;
#endif
#endif

#ifdef WDS
	// always disable wds in vap
	priv->pmib->dot11WdsInfo.wdsEnabled = 0;
	priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif
#ifdef CONFIG_RTK_MESH
	// in current release, mesh can be only run upon wlan0, so we disable the following flag in vap
	priv->pmib->dot1180211sInfo.mesh_enable = 0;
#endif
}


static void rtl8192cd_init_mbssid(struct rtl8192cd_priv *priv)
{
	int i, j;
	unsigned int camData[2];
	unsigned char *macAddr = GET_MY_HWADDR;

	if (IS_ROOT_INTERFACE(priv))
	{
		//camData[0] = 0x00800000 | (macAddr[5] << 8) | macAddr[4];
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID | (macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
//		for (j=0; j<2; j++) {
		for (j=1; j>=0; j--) {
			//RTL_W32((_MBIDCAMCONTENT_+4)-4*j, camData[j]);
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
		//RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6));

		// clear the rest area of CAM
		//camData[0] = 0;
		camData[1] = 0;
		for (i=1; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
//			for (j=0; j<2; j++) {
			for (j=1; j>=0; j--) {
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
			}
//			RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | (unsigned char)i);
		}

		// set MBIDCTRL & MBID_BCN_SPACE by cmd
//		set_fw_reg(priv, 0xf1000101, 0, 0);
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION |EN_MBSSID| DIS_SUB_STATE | DIS_TSF_UPDATE|EN_TXBCN_RPT);
		else
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
/*
#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) || (OPMODE & WIFI_ADHOC_STATE))
			RTL_W32(RCR, RTL_R32(RCR) | RCR_CBSSID);
#endif
*/
	}
	else if (IS_VAP_INTERFACE(priv))
	{
//		priv->vap_init_seq = (RTL_R8(_MBIDCTRL_) & (BIT(4) | BIT(5) | BIT(6))) >> 4;
//		priv->vap_init_seq++;
//		set_fw_reg(priv, 0xf1000001 | ((priv->vap_init_seq + 1)&0xffff)<<8, 0, 0);

		priv->vap_init_seq = RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask;
		priv->vap_init_seq++;
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
				(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
				(macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));
		RTL_W32(MBSSID_BCN_SPACE,
			((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
			((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq))
			& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1)) & BCN_SPACE1_Mask)
			<<BCN_SPACE1_SHIFT);
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION |EN_MBSSID| DIS_SUB_STATE | DIS_TSF_UPDATE|EN_TXBCN_RPT);
		else
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);


		RTL_W8(MBID_NUM, priv->vap_init_seq & MBID_BCN_NUM_Mask);
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
	}
}


static void rtl8192cd_stop_mbssid(struct rtl8192cd_priv *priv)
{
	int i, j;
	unsigned int camData[2];
	camData[1] = 0;

	if (IS_ROOT_INTERFACE(priv))
	{
		// clear the rest area of CAM
		for (i=0; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
			for (j=1; j>=0; j--) {
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
			}
//			RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | (unsigned char)i);
		}

//		set_fw_reg(priv, 0xf1000001, 0, 0);
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);	// MBSSID disable
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
		else
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

	}
	else if (IS_VAP_INTERFACE(priv) && (priv->vap_init_seq >= 0))
	{
//		set_fw_reg(priv, 0xf1000001 | (((RTL_R8(_MBIDCTRL_) & (BIT(4) | BIT(5) | BIT(6))) >> 4)&0xffff)<<8, 0, 0);
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN |
			(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));

		if (RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask) {
			RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM)-1) & MBID_BCN_NUM_Mask);
			RTL_W32(MBSSID_BCN_SPACE,
			((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
			((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1))*RTL_R8(MBID_NUM)))
			& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1)) & BCN_SPACE1_Mask)
			<<BCN_SPACE1_SHIFT);
			RTL_W8(BCN_CTRL, 0);
			RTL_W8(0x553, 1);
			if (IS_TEST_CHIP(priv))
				RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
			else
				RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

		}
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);
		priv->vap_init_seq = -1;
	}
}
#endif


#ifdef WDS
#ifdef LAZY_WDS
void delete_wds_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i;

	for (i=0; i<NUM_WDS; i++) {
		if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, pstat->hwaddr, MACADDRLEN)) {
			memcpy(priv->pmib->dot11WdsInfo.entry[i].macAddr,
							NULL_MAC_ADDR, MACADDRLEN);
			free_stainfo(priv, pstat);
			priv->pmib->dot11WdsInfo.wdsNum--;
			break;
		}
	}
}
#endif

struct stat_info *add_wds_entry(struct rtl8192cd_priv *priv, int idx, unsigned char *mac)
{
	struct stat_info *pstat;
	DOT11_SET_KEY Set_Key;
#ifdef LAZY_WDS
	int i;

	if (mac != NULL) {
		for (i=0; i<NUM_WDS; i++) {
			if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr,
						NULL_MAC_ADDR, MACADDRLEN)) {
				memcpy(	priv->pmib->dot11WdsInfo.entry[i].macAddr, mac, MACADDRLEN);
				idx = i;
				priv->pmib->dot11WdsInfo.wdsNum++;
				break;
			}
		}
		if (i == NUM_WDS) {
			DEBUG_ERR("WDS table is full!!!\n");
			return NULL;
		}
	}
#endif

	pstat = alloc_stainfo(priv, priv->pmib->dot11WdsInfo.entry[idx].macAddr, -1);
	if (pstat == NULL) {
		DEBUG_ERR("alloc_stainfo() fail!\n");
		return NULL;
	}

	// use self supported rate for wds
	memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
	pstat->bssratelen = AP_BSSRATE_LEN;

	if (priv->pmib->dot11WdsInfo.entry[idx].txRate) {
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0x0000fff;	// mask HT rates

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
					!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0xffff00f;	// mask OFDM rates

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0xffffff0;	// mask CCK rates
	}

	pstat->state = WIFI_WDS;

#ifdef LAZY_WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE)
		pstat->state |= WIFI_WDS_LAZY;

	if ((priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) &&
		(priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
			priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_))
		wds_psk_set(priv, idx, NULL);
#endif

	if (priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_40_PRIVACY_ ||
				priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_104_PRIVACY_ ) {
#ifndef CONFIG_RTL8186_KB
		memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[idx].macAddr, 6);
		Set_Key.KeyType = DOT11_KeyType_Pairwise;
		Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
		Set_Key.KeyIndex = 0;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, priv->pmib->dot11WdsInfo.wdsWepKey);
#endif
	}
	else if ((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
						priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_) &&
					(priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx]&0x80000000) ) {
		priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] &= ~0x80000000;
		memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[idx].macAddr, 6);
				Set_Key.KeyType = DOT11_KeyType_Pairwise;
				Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
				Set_Key.KeyIndex = priv->pmib->dot11WdsInfo.wdsKeyId;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, priv->pmib->dot11WdsInfo.wdsMapingKey[idx]);
	}

	add_update_RATid(priv, pstat);
	pstat->wds_idx = idx;
	assign_tx_rate(priv, pstat, NULL);
	assign_aggre_mthod(priv, pstat);
	assign_aggre_size(priv, pstat);

	list_add_tail(&pstat->asoc_list, &priv->asoc_list);

	return pstat;
}

static void create_wds_tbl(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat;

#ifdef FAST_RECOVERY
	if (priv->reset_hangup)
		return;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv))
		return;
#endif

	if (priv->pmib->dot11WdsInfo.wdsEnabled) {
		for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
			pstat = add_wds_entry(priv, i, NULL);
			if (pstat == NULL)
				break;
		}
	}
}
#endif //  WDS


void validate_fixed_tx_rate(struct rtl8192cd_priv *priv)
{
	if (!priv->pmib->dot11StationConfigEntry.autoRate) {
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))
			priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0x0000fff;	// mask HT rates

		if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
			((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)))
			priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0x00fffff;	// mask MCS8 - MCS15

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
			!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
			priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0xffff00f;	// mask OFDM rates

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))
			priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0xffffff0;	// mask CCK rates

		if (priv->pmib->dot11StationConfigEntry.fixedTxRate==0) {
			priv->pmib->dot11StationConfigEntry.autoRate=1;
			PRINT_INFO("invalid fixed tx rate, use auto rate!\n");
		}
		else
			priv->pshare->current_tx_rate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate);
	}
}


#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
void recalc_txdesc_limit(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *root_priv = NULL;
	int i, num, total_if = 0;

	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT_PRIV(priv);

	if (IS_DRV_OPEN(root_priv))
		total_if++;

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(root_priv->pvxd_priv))
		total_if++;
#endif

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i]))
			total_if++;
	}
#endif

	if (total_if <= 1) {
		root_priv->pshare->num_txdesc_cnt = NUM_TX_DESC - 2;  // 2 for space...
		root_priv->pshare->num_txdesc_upper_limit = NUM_TX_DESC - 2;
		root_priv->pshare->num_txdesc_lower_limit = 0;
		return;
	}
	
	num = (NUM_TX_DESC * IF_TXDESC_UPPER_LIMIT) / 100;
	root_priv->pshare->num_txdesc_upper_limit = num;
	
	num = ((NUM_TX_DESC - 2) - num) / (total_if - 1);
	root_priv->pshare->num_txdesc_lower_limit = num;

	num = root_priv->pshare->num_txdesc_upper_limit + 
			root_priv->pshare->num_txdesc_lower_limit * (total_if - 1);
	root_priv->pshare->num_txdesc_cnt = num;
}
#endif


int rtl8192cd_open(struct net_device *dev)
{
	struct rtl8192cd_priv *priv;	// recuresively used, can't be static
	int rc;
#ifdef MBSSID	
	int i;
#endif
	unsigned long x;
#ifdef CONFIG_RTL865X_WTDOG
#if !(defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
	unsigned long wtval;
#endif
#endif
#ifdef CHECK_HANGUP
	int is_reset;
#endif
	int init_hw_cnt = 0;

	DBFENTER;

	priv = dev->priv;
#if 0
//#ifdef PCIE_POWER_SAVING
	if (((REG32(CLK_MANAGE) & BIT(11)) == 0)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		&& IS_ROOT_INTERFACE(priv)
#endif
		)
	{
		extern void setBaseAddressRegister(void);
		REG32(CLK_MANAGE) |= BIT(11);
		delay_ms(10);
		PCIE_reset_procedure(0, 0, 1, priv->dev->base_addr);
		setBaseAddressRegister();
	}
#endif

#ifdef CHECK_HANGUP
	is_reset = priv->reset_hangup;
#endif

// init mib from cfg file, we only need to load cfg file once - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_CFGFILE
#ifdef WDS
	if (dev->base_addr) //root
#endif
	{
		//printk(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>dev %s set_default\n", dev->name);
		//memset(priv->pmib, 0, sizeof(struct wifi_mib));
		//set_mib_default(priv);

		CfgFileProc(dev);
	}
#endif

// register iw_handler - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	dev->wireless_handlers = (struct iw_handler_def *) &rtl8192cd_iw_handler_def;
#endif

	memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

#ifdef WDS
	if (dev->base_addr == 0)
	{
#ifdef BR_SHORTCUT
		extern struct net_device *cached_wds_dev;
		cached_wds_dev = NULL;
#endif

		netif_start_queue(dev);
		return 0;
	}
#endif

#ifdef CONFIG_RTK_MESH
	if (dev->base_addr == 1) {
		netif_start_queue(dev);
		return 0;
	}
#endif // CONFIG_RTK_MESH

	// stop h/w in the very beginning
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{			
		if ( CheckNoResetHwExceptionCase(priv) ) {
			rtl8192cd_stop_hw(priv);
		}
	}

#ifdef UNIVERSAL_REPEATER
	// If vxd interface, see if some mandatory mib is set. If ok, backup these
	// mib, and copy all mib from root interface. Then, restore the backup mib
	// to current.

	if (IS_VXD_INTERFACE(priv)) {
		DEBUG_INFO("Open request from vxd\n");
		if (!IS_DRV_OPEN(GET_ROOT_PRIV(priv))) {
			printk("Open vxd error! Root interface should be opened in advanced.\n");
			return 0;
		}

		if (!(priv->drv_state & DRV_STATE_VXD_INIT)) {
// Mark following code. MIB copy will be executed through ioctl -------------
#if 0
			unsigned char tmpbuf[36];
			int len, encyption, is_1x, mac_clone, nat25;
			struct Dot11RsnIE rsnie;

			len = SSID_LEN;
			memcpy(tmpbuf, SSID, len);
			encyption = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
			is_1x = IEEE8021X_FUN;
			mac_clone = priv->pmib->ethBrExtInfo.macclone_enable;
			nat25 = priv->pmib->ethBrExtInfo.nat25_disable;
			memcpy((char *)&rsnie, (char *)&priv->pmib->dot11RsnIE, sizeof(rsnie));

			memcpy(priv->pmib, GET_ROOT_PRIV(priv)->pmib, sizeof(struct wifi_mib));

			SSID_LEN = len;
			memcpy(SSID, tmpbuf, len);
			SSID2SCAN_LEN = len;
			memcpy(SSID2SCAN, SSID, len);
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = encyption;
			IEEE8021X_FUN = is_1x;
			priv->pmib->ethBrExtInfo.macclone_enable = mac_clone;
			priv->pmib->ethBrExtInfo.nat25_disable = nat25;
			memcpy((char *)&priv->pmib->dot11RsnIE, (char *)&rsnie, sizeof(rsnie));
#ifdef WDS
			// always disable wds in vxd
			priv->pmib->dot11WdsInfo.wdsEnabled = 0;
			priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif

			// if root interface is AP mode, set infra-client in vxd
			// if root interfeace is a infra-client, set AP in vxd
			if (OPMODE & WIFI_AP_STATE) {
				OPMODE = WIFI_STATION_STATE;
#if defined(WIFI_WMM) && defined(WMM_APSD)
				APSD_ENABLE = 0;
#endif
				DEBUG_INFO("Set vxd as an infra-client\n");
			}
			else if (OPMODE & WIFI_STATION_STATE) {
				OPMODE = WIFI_AP_STATE;
				priv->auto_channel = 0;
				DEBUG_INFO("Set vxd as an AP\n");
			}
			else {
				DEBUG_ERR("Invalid opmode for vxd!\n");
				return 0;
			}
#endif
//---------------------------------------------------------- david+2008-03-17

			// correct RSN IE will be set later for WPA/WPA2
#ifdef CHECK_HANGUP
			if (!is_reset)
#endif
				memset(&priv->pmib->dot11RsnIE, 0, sizeof(struct Dot11RsnIE));

#ifdef WDS
			// always disable wds in vxd
			priv->pmib->dot11WdsInfo.wdsEnabled = 0;
			priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif

#ifdef CONFIG_RTK_MESH
			// always disable mesh in vxd (for dev)
			GET_MIB(priv)->dot1180211sInfo.mesh_enable = 0;
#endif // CONFIG_RTK_MESH
			priv->drv_state |= DRV_STATE_VXD_INIT;	// indicate the mib of vxd driver has been initialized
		}
	}
#endif // UNIVERSAL_REPEATER

#ifdef CHECK_HANGUP
	if (!is_reset)
#endif
	{
		if (OPMODE & WIFI_AP_STATE)
			OPMODE = WIFI_AP_STATE;
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			OPMODE = WIFI_STATION_STATE;
#if defined(WIFI_WMM) && defined(WMM_APSD)
			APSD_ENABLE = 0;
#endif
		}
		else if (OPMODE & WIFI_ADHOC_STATE) {
			OPMODE = WIFI_ADHOC_STATE;
#ifdef WIFI_WMM
#ifdef WMM_APSD
			APSD_ENABLE = 0;
#endif
#endif
		}
#endif
		else {
			printk("Undefined state... using AP mode as default\n");
			OPMODE = WIFI_AP_STATE;
		}
	}

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			if (!IS_DRV_OPEN(GET_ROOT_PRIV(priv))) {
				printk("Open vap error! Root interface should be opened in advanced.\n");
				return -1;
			}

			if ((GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) == 0) {
				printk("Fail to open VAP under non-AP mode!\n");
				return -1;
			}
			else {
				rtl8192cd_init_vap_mib(priv);
			}
		}
	}
#endif

#if defined(BR_SHORTCUT)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if(IS_ROOT_INTERFACE(priv))
#endif
		clear_shortcut_cache();
#endif

	rc = rtl8192cd_init_sw(priv);
    if (rc)
        return rc;

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif

	validate_fixed_tx_rate(priv);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef __KERNEL__
#ifdef CHECK_HANGUP
		if (!is_reset)
#endif
		{
#ifndef CONFIG_RTL8671
#ifdef __LINUX_2_6__
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_SHARED, dev->name, dev);
#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, SA_SHIRQ, dev->name, dev);
#endif
#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, SA_INTERRUPT, dev->name, dev);
#endif

#if defined(PCIE_POWER_SAVING) && defined(GPIO_WAKEPIN)
			rc |= request_irq_for_wakeup_pin(dev);
#endif

			if (rc) {
				DEBUG_ERR("some issue in request_irq, rc=%d\n", rc);
			}
		}
#endif

#ifndef	CONFIG_X86
		SAVE_INT_AND_CLI(x);
#endif

#ifdef CONFIG_RTL865X_WTDOG
#if !(defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
		wtval = *((volatile unsigned long *)0xB800311C);
		*((volatile unsigned long *)0xB800311C) = 0xA5f00000;
#endif
#endif

do_hw_init:

#ifdef EN_EFUSE
		{
			int i, readEfuse=0;
			for(i=0;i<MAX_2G_CHANNEL_NUM;i++)	{
				if(	priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]==0
				||	priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] ==0) {
					readEfuse = 1;
					break;
				}
			}
			if(readEfuse)
				ReadTxPowerInfoFromHWPG(priv);

			if(priv->pmib->dot11RFEntry.ther==0)
				ReadThermalMeterFromEfuse(priv);
		}
#endif
		rc = rtl8192cd_init_hw_PCI(priv);
		delay_ms(200);		// TODO: need refinement

		// write IDR0, IDR4 here
		{
			unsigned long reg = 0;
			reg = *(unsigned long *)(dev->dev_addr);
//			RTL_W32(IDR0, (cpu_to_le32(reg)));
			RTL_W32(MACID, (cpu_to_le32(reg)));
			reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
//			RTL_W32(IDR4, (cpu_to_le32(reg)));
			RTL_W16(MACID+4, (cpu_to_le16(reg)));
		}

		if (rc && ++init_hw_cnt < 5) {
#ifdef CONFIG_RTL865X_WTDOG
			*((volatile unsigned long *)0xB800311C) |=  1 << 23;
#endif
			goto do_hw_init;
		}

#ifdef CONFIG_RTL865X_WTDOG
#if !(defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
		*((volatile unsigned long *)0xB800311C) |=  1 << 23;
		*((volatile unsigned long *)0xB800311C) = wtval;
#endif
#endif

#ifndef	CONFIG_X86
		RESTORE_INT(x);
#endif

		if (rc) {
			DEBUG_ERR("init hw failed!\n");
			force_stop_wlan_hw();
#ifdef LINUX_2_6_22_
			local_irq_disable();
#else
			cli();
#endif
			*(volatile unsigned long *)(0xB800311c) = 0; /* enable watchdog reset now */
			for(;;)
				;
			return rc;
		}
	}
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	else {
		if (get_rf_mimo_mode(priv) == MIMO_1T1R)
			GET_MIB(priv)->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;
	}
#endif

#ifdef MBSSID
	if ((OPMODE & WIFI_AP_STATE) && (GET_ROOT(priv)->pmib->miscEntry.vap_enable)) {
		rtl8192cd_init_mbssid(priv);

		if (IS_VAP_INTERFACE(priv)) {
			// set BcnDmaInt & BcnOk of different VAP in IMR
			priv->pshare->InterruptMask |= (HIMR_BCNDOK1 << (priv->vap_init_seq-1)) |
				(HIMR_BCNDMA1 << (priv->vap_init_seq-1));
			RTL_W32(HIMR, priv->pshare->InterruptMask);

			if (GET_ROOT_PRIV(priv)->auto_channel == 0) {
				priv->pmib->dot11RFEntry.dot11channel = GET_ROOT_PRIV(priv)->pmib->dot11RFEntry.dot11channel;
				priv->ht_cap_len = 0;	// re-construct HT IE

				init_beacon(priv);
			}
		}
	}
#endif

// new added to reset keep_rsnie flag
	if (priv->pmib->dot11OperationEntry.keep_rsnie)
		priv->pmib->dot11OperationEntry.keep_rsnie = 0;
//------------------- david+2006-06-30

	priv->drv_state |= DRV_STATE_OPEN;      // set driver as has been opened, david

#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	recalc_txdesc_limit(priv);
#endif

	//memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

	// below is for site_survey timer
	init_timer(&priv->ss_timer);
	priv->ss_timer.data = (unsigned long) priv;
	priv->ss_timer.function = rtl8192cd_ss_timer;

#ifdef CLIENT_MODE
	init_timer(&priv->reauth_timer);
	priv->reauth_timer.data = (unsigned long) priv;
	priv->reauth_timer.function = rtl8192cd_reauth_timer;

	init_timer(&priv->reassoc_timer);
	priv->reassoc_timer.data = (unsigned long) priv;
	priv->reassoc_timer.function = rtl8192cd_reassoc_timer;

	init_timer(&priv->idle_timer);
	priv->idle_timer.data = (unsigned long) priv;
	priv->idle_timer.function = rtl8192cd_idle_timer;
#endif

	priv->frag_to = 0;
	init_timer(&priv->frag_to_filter);
	priv->frag_to_filter.expires = jiffies + FRAG_TO;
	priv->frag_to_filter.data = (unsigned long) priv;
	priv->frag_to_filter.function = rtl8192cd_frag_timer;
	mod_timer(&priv->frag_to_filter, jiffies + FRAG_TO);

#ifdef DETECT_STA_EXISTANCE
	// Added by Annie for Retry Limit Recovery Timer, 2010-08-10.
	init_timer(&priv->pshare->rl_recover_timer);
	priv->pshare->rl_recover_timer.data = (unsigned long) priv;
	priv->pshare->rl_recover_timer.function = RetryLimitRecovery;
	priv->pshare->bRLShortened = FALSE;
#endif

	priv->auth_to = AUTH_TO / 100;
	priv->assoc_to = ASSOC_TO / 100;

#ifdef PCIE_POWER_SAVING_DEBUG
	priv->expire_to = 60;
#else
	priv->expire_to = (EXPIRETIME > 100)? (EXPIRETIME / 100) : 86400;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef PCIE_POWER_SAVING
		init_timer(&priv->ps_timer);
		priv->ps_timer.expires = jiffies + POWER_DOWN_T0;
		priv->ps_timer.data = (unsigned long) priv;
		priv->ps_timer.function = PCIe_power_save_timer;
		mod_timer(&priv->ps_timer, jiffies + POWER_DOWN_T0);
//		priv->ps_ctrl = 0x11;
#endif
		init_timer(&priv->expire_timer);
		priv->expire_timer.expires = jiffies + EXPIRE_TO;
		priv->expire_timer.data = (unsigned long) priv;
		priv->expire_timer.function = rtl8192cd_1sec_timer;
#ifdef 	SW_ANT_SWITCH
		init_timer(&priv->pshare->swAntennaSwitchTimer);
		priv->pshare->swAntennaSwitchTimer.data = (unsigned long) priv;
		priv->pshare->swAntennaSwitchTimer.function = dm_SW_AntennaSwitchCallback;
#endif
		mod_timer(&priv->expire_timer, jiffies + EXPIRE_TO);

		init_timer(&priv->pshare->rc_sys_timer);
		priv->pshare->rc_sys_timer.data = (unsigned long) priv;
		priv->pshare->rc_sys_timer.function = reorder_ctrl_timeout;

#if 0
		init_timer(&priv->pshare->phw->tpt_timer);
		priv->pshare->phw->tpt_timer.data = (unsigned long)priv;
		priv->pshare->phw->tpt_timer.function = rtl8192cd_tpt_timer;
#endif
	}

	// for MIC check
	init_timer(&priv->MIC_check_timer);
	priv->MIC_check_timer.data = (unsigned long) priv;
	priv->MIC_check_timer.function = DOT11_Process_MIC_Timerup;
	init_timer(&priv->assoc_reject_timer);
	priv->assoc_reject_timer.data = (unsigned long) priv;
	priv->assoc_reject_timer.function = DOT11_Process_Reject_Assoc_Timerup;
	priv->MIC_timer_on = FALSE;
	priv->assoc_reject_on = FALSE;

#ifdef GBWC
	init_timer(&priv->GBWC_timer);
	priv->GBWC_timer.data = (unsigned long) priv;
	priv->GBWC_timer.function = rtl8192cd_GBWC_timer;
	mod_timer(&priv->GBWC_timer, jiffies + GBWC_TO);
#endif

	// to avoid add RAtid fail
	init_timer(&priv->add_RATid_timer);
	priv->add_RATid_timer.data = (unsigned long) priv;
	priv->add_RATid_timer.function = add_RATid_timer;

	init_timer(&priv->add_rssi_timer);
	priv->add_rssi_timer.data = (unsigned long) priv;
	priv->add_rssi_timer.function = add_rssi_timer;

	init_timer(&priv->add_ps_timer);
	priv->add_ps_timer.data = (unsigned long) priv;
	priv->add_ps_timer.function = add_ps_timer;

#ifdef CONFIG_RTK_MESH
	/*
	 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
	 * These statment is initial information, (If "ZERO" no need set it, because all cleared to ZERO)
	 */
	init_timer(&priv->mesh_peer_link_timer);
	priv->mesh_peer_link_timer.data = (unsigned long) priv;
	priv->mesh_peer_link_timer.function = mesh_peer_link_timer;

#ifdef MESH_BOOTSEQ_AUTH
	init_timer(&priv->mesh_auth_timer);
	priv->mesh_auth_timer.data = (unsigned long) priv;
	priv->mesh_auth_timer.function = mesh_auth_timer;
#endif

	priv->mesh_Version = 1;
#ifdef	MESH_ESTABLISH_RSSI_THRESHOLD
	priv->mesh_fake_mib.establish_rssi_threshold = DEFAULT_ESTABLISH_RSSI_THRESHOLD;
#endif

#ifdef MESH_USE_METRICOP
	// in next version, the fake_mib related values will be actually recorded in MIB
	priv->mesh_fake_mib.metricID = 1; // 0: very old version,  1: version before 2009/3/10,  2: 11s
	priv->mesh_fake_mib.isPure11s = 0;
	priv->mesh_fake_mib.intervalMetricAuto = 60 * HZ; // 1 Mins
	priv->mesh_fake_mib.spec11kv.defPktTO = 20; // 2 * 100 = 2 secs
	priv->mesh_fake_mib.spec11kv.defPktLen = 1024; // bt=8196 bits
	priv->mesh_fake_mib.spec11kv.defPktCnt = 2;
	priv->mesh_fake_mib.spec11kv.defPktPri = 5;

	// once driver starts up, toMeshMetricAuto will be updated
	// (I think it might be put in "init one" to match our original concept...)
	priv->toMeshMetricAuto = jiffies + priv->mesh_fake_mib.intervalMetricAuto;
#endif // MESH_USE_METRICOP

	mesh_set_PeerLink_CAP(priv, GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor);

	priv->mesh_PeerCAP_flags = 0x80;		// Bit15(Operation as MP) shall be "1"
	priv->mesh_HeaderFlags = 0;				// NO Address Extension

	// The following info can be saved by FLASH in the future
	priv->mesh_profile[0].used = TRUE;
	priv->mesh_profile[0].PathSelectMetricID.value = 0;
	priv->mesh_profile[0].PathSelectMetricID.OUI[0] = 0x00;
	priv->mesh_profile[0].PathSelectMetricID.OUI[1] = 0x0f;
	priv->mesh_profile[0].PathSelectMetricID.OUI[2] = 0xac;
	priv->mesh_profile[0].PathSelectProtocolID.value = 0;
	priv->mesh_profile[0].PathSelectProtocolID.OUI[0] = 0x00;
	priv->mesh_profile[0].PathSelectProtocolID.OUI[1] = 0x0f;
	priv->mesh_profile[0].PathSelectProtocolID.OUI[2] = 0xac;
#endif


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		// for HW/SW LED
		if ((LED_TYPE >= LEDTYPE_HW_TX_RX) && (LED_TYPE <= LEDTYPE_HW_LINKACT_INFRA))
			enable_hw_LED(priv, LED_TYPE);
		else if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
			enable_sw_LED(priv, 1);
#ifdef SW_ANT_SWITCH
		dm_SW_AntennaSwitchInit(priv);	// SW Ant Switch use LED pin to control TRX Antenna
#endif
#if defined(HW_ANT_SWITCH)
		dm_HW_AntennaSwitchInit(priv);
#endif

#ifdef DFS
		if (!priv->pmib->dot11DFSEntry.disable_DFS &&
			(OPMODE & WIFI_AP_STATE) &&
			(priv->pmib->dot11RFEntry.dot11channel >= 52)) {
			init_timer(&priv->ch_avail_chk_timer);
			priv->ch_avail_chk_timer.data = (unsigned long) priv;
			priv->ch_avail_chk_timer.function = rtl8192cd_ch_avail_chk_timer;
			mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO);

			init_timer(&priv->DFS_timer);
			priv->DFS_timer.data = (unsigned long) priv;
			priv->DFS_timer.function = rtl8192cd_DFS_timer;
			mod_timer(&priv->DFS_timer, jiffies + 500); // DFS activated after 5 sec; prevent switching channel due to DFS false alarm
		}
#endif

#ifdef SUPPORT_SNMP_MIB
		mib_init(priv);
#endif

	}

#if defined(BR_SHORTCUT) && defined(CLIENT_MODE)
	if (OPMODE & WIFI_STATION_STATE) {
		extern struct net_device *cached_sta_dev;
		cached_sta_dev = NULL;
	}
#endif

#if	defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
	{
		extern unsigned char cached_br_sta_mac[MACADDRLEN];
		extern struct net_device *cached_br_sta_dev;
		memset(cached_br_sta_mac, 0, MACADDRLEN);
		cached_br_sta_dev = NULL;
	}
#endif

	//if (OPMODE & WIFI_AP_STATE)  //in case of station mode, queue will start only after assoc.
		netif_start_queue(dev);		// start queue always

#ifdef WDS
	create_wds_tbl(priv);
#endif

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->reset_hangup = 0;
#endif

#if defined(INCLUDE_WPA_PSK) && defined(CLIENT_MODE)
	if (OPMODE & WIFI_ADHOC_STATE)
		if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
			ToDrv_SetGTK(priv);
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv))
#endif
	if ((OPMODE & WIFI_AP_STATE) && priv->auto_channel) {
		if (((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_)) ||
			 (priv->pmib->dot11RsnIE.rsnielen > 0)) {
			priv->ss_ssidlen = 0;
			DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
			start_clnt_ss(priv);
		}
	}

#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
#ifdef RTK_BR_EXT
		if (priv->dev->br_port) {
#ifdef __LINUX_2_6__
			memcpy(priv->br_mac, priv->dev->br_port->br->dev->dev_addr, MACADDRLEN);
#else
			memcpy(priv->br_mac, priv->dev->br_port->br->dev.dev_addr, MACADDRLEN);
#endif
		}
#endif

		if (!IEEE8021X_FUN || (IEEE8021X_FUN && (priv->pmib->dot11RsnIE.rsnielen > 0))) {
#ifdef CHECK_HANGUP
			if (!is_reset || priv->join_res == STATE_Sta_No_Bss ||
					priv->join_res == STATE_Sta_Roaming_Scan || priv->join_res == 0)
#endif
			{
#ifdef CHECK_HANGUP
				if (is_reset)
					OPMODE &= ~WIFI_SITE_MONITOR;
#endif
				start_clnt_lookup(priv, 1);
			}
		}
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) &&
		netif_running(GET_VXD_PRIV(priv)->dev)) {
		SAVE_INT_AND_CLI(x);
		rtl8192cd_open(GET_VXD_PRIV(priv)->dev);
		RESTORE_INT(x);
	}
	if (IS_VXD_INTERFACE(priv) &&
		(GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode&WIFI_STATION_STATE) &&
		(GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode&WIFI_ASOC_STATE) &&
#ifdef RTK_BR_EXT
		!(GET_ROOT_PRIV(priv)->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed) &&
#endif
		!(priv->drv_state & DRV_STATE_VXD_AP_STARTED) )
		enable_vxd_ap(priv);
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		if (priv->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (netif_running(priv->pvap_priv[i]->dev))
					rtl8192cd_open(priv->pvap_priv[i]->dev);
			}
		}
	}
#endif

#ifdef PCIE_POWER_SAVING
#ifdef CHECK_HANGUP
	if(!is_reset)
#endif
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		if (REG32(REVR) == RTL8196C_REVISION_B)
			init_pcie_power_saving(priv);
		else
			priv->pshare->rf_ft_var.power_save = 0;
	}
#endif

	DBFEXIT;
	return 0;
}


int  rtl8192cd_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	unsigned long reg;
	unsigned char *p;
#ifdef WDS
	int i;
#endif

#ifdef __KERNEL__
	p = ((struct sockaddr *)addr)->sa_data;
#else
	p = (unsigned char *)addr;
#endif

	SAVE_INT_AND_CLI(flags);

	memcpy(priv->dev->dev_addr, p, 6);
	memcpy(GET_MY_HWADDR, p, 6);

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			RESTORE_INT(flags);
			return 0;
		}
	}
#endif

#ifdef WDS
	for (i=0; i<NUM_WDS; i++)
		if (priv->wds_dev[i])
			memcpy(priv->wds_dev[i]->dev_addr, p, 6);
#endif
#ifdef CONFIG_RTK_MESH
	if(NUM_MESH>0)
		if (priv->mesh_dev)
			memcpy(priv->mesh_dev->dev_addr, p, 6);
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_VXD_PRIV(priv)) {
			memcpy(GET_VXD_PRIV(priv)->dev->dev_addr, p, 6);
			memcpy(GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.hwaddr, p, 6);
		}
	}
	else if (IS_VXD_INTERFACE(priv)) {
		memcpy(GET_ROOT_PRIV(priv)->dev->dev_addr, p, 6);
		memcpy(GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.hwaddr, p, 6);
	}
#endif
	reg = *(unsigned long *)(dev->dev_addr);
	RTL_W32(MACID, (cpu_to_le32(reg)));
	reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	RTL_W16(MACID+4, (cpu_to_le16(reg)));
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (OPMODE & WIFI_AP_STATE)
			rtl8192cd_init_mbssid(priv);
	}
#endif

	RESTORE_INT(flags);

#ifdef CLIENT_MODE
	if (!(OPMODE & WIFI_AP_STATE) && netif_running(priv->dev)) {
		int link_status = chklink_wkstaQ(priv);
		if (link_status)
			start_clnt_join(priv);
	}
#endif

	return 0;
}


int rtl8192cd_close(struct net_device *dev)
{
    struct rtl8192cd_priv *priv = dev->priv;
#ifdef UNIVERSAL_REPEATER
	struct rtl8192cd_priv *priv_vxd;
#endif

#ifndef CONFIG_X86
	unsigned long flags;
#endif

	DBFENTER;

	if (!(priv->drv_state & DRV_STATE_OPEN)
#ifdef WDS
		&& dev->base_addr
#endif
		) {
		DBFEXIT;
		return 0;
	}

#ifndef CONFIG_X86
	SAVE_INT_AND_CLI(flags);
#endif

#ifdef WDS
	if (dev->base_addr)
#endif
	priv->drv_state &= ~DRV_STATE_OPEN;     // set driver as has been closed, david

#ifdef PCIE_POWER_SAVING
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);
	if((priv->pwr_state == L1) || (priv->pwr_state == L2)) {
		priv->ps_ctrl = 0x82 | (priv->pwr_state<<4);
		PCIe_power_save_tasklet((unsigned long)priv);
	}
#endif

#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	recalc_txdesc_limit(priv);
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv)) {
		priv_vxd = GET_VXD_PRIV(priv);

		// if vxd interface is opened, close it first
		if (IS_DRV_OPEN(priv_vxd))
			rtl8192cd_close(priv_vxd->dev);
	}
	else {
#ifdef MBSSID
/*
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
*/
		if (priv->vap_id < 0)
#endif
		disable_vxd_ap(priv);
	}
#endif

    netif_stop_queue(dev);

#ifdef WDS
	if (dev->base_addr == 0)
	{
#ifndef CONFIG_X86
		RESTORE_INT(flags);
#endif
		DBFEXIT;
		return 0;
	}
#endif

#ifdef CONFIG_RTK_MESH
	if (dev->base_addr == 1)
	{
#ifndef CONFIG_X86
		RESTORE_INT(flags);
#endif
		return 0;
	}
#endif // CONFIG_RTK_MESH

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	if (OPMODE & WIFI_AP_STATE) {
		int i;
		for(i=0; i<NUM_STAT; i++)
		{
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)
#ifdef WDS
				&& !(priv->pshare->aidarray[i]->station.state & WIFI_WDS)
#endif
			) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
				if (priv != priv->pshare->aidarray[i]->priv)
					continue;
#endif
				issue_deauth(priv, priv->pshare->aidarray[i]->station.hwaddr, _RSON_DEAUTH_STA_LEAVING_);
			}
		}

		delay_ms(10);
	}

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		int i;
		if (priv->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					rtl8192cd_close(priv->pvap_priv[i]->dev);
			}
		}
	}

	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
		rtl8192cd_stop_mbssid(priv);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef RTK_QUE
		free_rtk_queue(priv, &priv->pshare->skb_queue);
#else
		free_skb_queue(priv, &priv->pshare->skb_queue);
#endif

		rtl8192cd_stop_hw(priv);

#ifdef __KERNEL__
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
		{
			free_irq(dev->irq, dev);
#ifdef PCIE_POWER_SAVING
			if (REG32(REVR) == RTL8196C_REVISION_B) 	{
#ifdef RTL8198_WAKE
				REG32(0xb8003000) &= ~ BIT(16);		// GIMR
#else
				REG32(0xb8003000) &= ~ BIT(9);		// GIMR
#endif
				free_irq(1, dev);
			}
#endif
		}
#endif



#ifdef UNIVERSAL_REPEATER
		GET_VXD_PRIV(priv)->drv_state &= ~DRV_STATE_VXD_INIT;
#endif
	}

	rtl8192cd_stop_sw(priv);

#ifdef ENABLE_RTL_SKB_STATS
	DEBUG_INFO("skb_tx_cnt =%d\n", rtl_atomic_read(&priv->rtl_tx_skb_cnt));
	DEBUG_INFO("skb_rx_cnt =%d\n", rtl_atomic_read(&priv->rtl_rx_skb_cnt));
#endif

#ifndef CONFIG_X86
	RESTORE_INT(flags);
#endif

#if 0
//#ifdef PCIE_POWER_SAVING
	if(!IS_UMC_A_CUT_88C(priv))
#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
		HostPCIe_Close();
#endif

	DBFEXIT;
    return 0;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static void MDL_DEVINIT set_mib_default(struct rtl8192cd_priv *priv)
{
	unsigned char *p;
#ifdef __KERNEL__
	struct sockaddr addr;
	p = addr.sa_data;
#else
	unsigned char tmpbuf[10];
	p = (unsigned char *)tmpbuf;
#endif

	priv->pmib->mib_version = MIB_VERSION;
	set_mib_default_tbl(priv);

	// others that are not types of byte and int
	strcpy(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, "RTL8186-default");
	priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen("RTL8186-default");
	memcpy(p, "\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);

#ifdef	DOT11D
	// set countryCode for 11d
	strcpy(priv->pmib->dot11dCountry.dot11CountryString, "US");
#endif

#ifdef 	__KERNEL__
	rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
	rtl8192cd_set_hwaddr(priv->dev, (void *)p);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
	{
#ifdef DFS
		init_timer(&priv->ch52_timer);
		priv->ch52_timer.data = (unsigned long) priv;
		priv->ch52_timer.function = rtl8192cd_ch52_timer;

		init_timer(&priv->ch56_timer);
		priv->ch56_timer.data = (unsigned long) priv;
		priv->ch56_timer.function = rtl8192cd_ch56_timer;

		init_timer(&priv->ch60_timer);
		priv->ch60_timer.data = (unsigned long) priv;
		priv->ch60_timer.function = rtl8192cd_ch60_timer;

		init_timer(&priv->ch64_timer);
		priv->ch64_timer.data = (unsigned long) priv;
		priv->ch64_timer.function = rtl8192cd_ch64_timer;
#endif

		if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_EMBEDDED) {
			// not implement yet
		}
		else {
#ifdef IO_MAPPING
			priv->pshare->io_mapping = 1;
#endif
		}
	}
}

#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_netdev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl8192cd_set_hwaddr,
        .ndo_set_multicast_list = rtl8192cd_set_rx_mode,
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};
#endif


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int MDL_DEVINIT rtl8192cd_init_one(struct pci_dev *pdev,
                  const struct pci_device_id *ent, struct _device_info_ *wdev, int vap_idx)
{
    struct net_device *dev;
    struct rtl8192cd_priv *priv;
    void *regs;
	struct wifi_mib 		*pmib;
	DOT11_QUEUE				*pevent_queue;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_QUEUE				*wapiEvent_queue;
#if defined(MBSSID)
	DOT11_QUEUE 			*wapiVapEvent_queue;
#endif

#endif
	struct rtl8192cd_hw		*phw;
	struct rtl8192cd_tx_desc_info		*ptxdesc;
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
	DOT11_EAP_PACKET		*Eap_packet;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	WPA_GLOBAL_INFO			*wpa_global_info;
#endif
#ifdef  CONFIG_RTK_MESH
#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll = NULL;
#endif
#ifdef _11s_TEST_MODE_
	struct Galileo_poll		*pgalileo_poll = NULL;
#endif
#ifndef WDS
	char baseDevName[8];
#endif
	int mesh_num;

	struct hash_table		*proxy_table;
#ifdef PU_STANDARD
	//pepsi
	struct hash_table		*proxyupdate_table;
#endif
	struct mpp_tb			*pann_mpp_tb;
	struct hash_table		*mesh_rreq_retry_queue;
	// add by chuangch 2007.09.13
	struct hash_table		*pathsel_table;

	DOT11_QUEUE2			*pathsel_queue ;
#ifdef	_11s_TEST_MODE_
	DOT11_QUEUE2			*receiver_queue = NULL;
#endif
#endif	// CONFIG_RTK_MESH



#ifdef CONFIG_NET_PCI
    u8 cache_size;
    u16 pci_command;
#ifndef USE_IO_OPS
    u32 pciaddr;
#endif
	u32 pmem_len;
#endif

#ifdef WDS
	int wds_num;
	char baseDevName[8];
#endif
#if defined(CONFIG_RTK_MESH) || defined(WDS)
	int i;
#endif

	unsigned char *page_ptr;
	struct priv_shared_info *pshare;	// david

	int rc=0;

    priv = NULL;
    regs = NULL;
	pmib = NULL;
	pevent_queue = NULL;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiEvent_queue = NULL;
#if defined(MBSSID)
	wapiVapEvent_queue = NULL;
#endif

#endif
	phw = NULL;
	ptxdesc = NULL;
	pwlan_hdr_poll = NULL;
	pwlanllc_hdr_poll = NULL;
	pwlanbuf_poll = NULL;
	pwlanicv_poll = NULL;
	pwlanmic_poll = NULL;
	pwlan_acl_poll = NULL;
	Eap_packet = NULL;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	wpa_global_info = NULL;
#endif




#ifdef CONFIG_NET_PCI
    cache_size=0;
#ifndef USE_IO_OPS
    pciaddr=0;
#endif
	pmem_len=0;
#endif


	pshare = NULL;	// david

	printk("=====>>INSIDE rtl8192cd_init_one <<=====\n");
	dev = alloc_etherdev(sizeof(struct rtl8192cd_priv));
	if (!dev) {
		printk(KERN_ERR "alloc_etherdev() error!\n");
       	return -ENOMEM;
	}

	// now, allocating memory for pmib
#ifdef RTL8192CD_VARIABLE_USED_DMEM
	pmib = (struct wifi_mib *)rtl8192cd_dmem_alloc(PMIB, NULL);
#else
	pmib = (struct wifi_mib *)kmalloc((sizeof(struct wifi_mib)), GFP_KERNEL);
#endif
	if (!pmib) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wifi_mib (size %d)\n", sizeof(struct wifi_mib));
		goto err_out_free;
	}
	memset(pmib, 0, sizeof(struct wifi_mib));

	pevent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_KERNEL);
	if (!pevent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)pevent_queue, 0, sizeof(DOT11_QUEUE));
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiEvent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_KERNEL);
	if (!wapiEvent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)wapiEvent_queue, 0, sizeof(DOT11_QUEUE));
#ifdef MBSSID
	wapiVapEvent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN), GFP_KERNEL);
	if (!wapiVapEvent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN);
		goto err_out_free;
	}
	memset((void *)wapiVapEvent_queue, 0, sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN);
#endif

#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv == NULL) // root interface
#endif
	{
#ifdef PRIV_STA_BUF
		phw = &hw_info;
#else
		phw = (struct rtl8192cd_hw *)kmalloc((sizeof(struct rtl8192cd_hw)), GFP_KERNEL);
		if (!phw) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for rtl8192cd_hw (size %d)\n", sizeof(struct rtl8192cd_hw));
			goto err_out_free;
		}
#endif
		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
		ptxdesc = &phw->tx_info;

#ifdef PRIV_STA_BUF
		pshare = &shared_info;
#else
		pshare = (struct priv_shared_info *)kmalloc(sizeof(struct priv_shared_info), GFP_KERNEL);
		if (!pshare) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for priv_shared_info (size %d)\n", sizeof(struct priv_shared_info));
			goto err_out_free;
		}
#endif
		memset((void *)pshare, 0, sizeof(struct priv_shared_info));

#ifdef PRIV_STA_BUF
		pwlan_hdr_poll = &hdr_pool;
#else
		pwlan_hdr_poll = (struct wlan_hdr_poll *)
						kmalloc((sizeof(struct wlan_hdr_poll)), GFP_KERNEL);
		if (!pwlan_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlan_hdr_poll (size %d)\n", sizeof(struct wlan_hdr_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
		pwlanllc_hdr_poll = &llc_pool;
#else
		pwlanllc_hdr_poll = (struct wlanllc_hdr_poll *)
						kmalloc((sizeof(struct wlanllc_hdr_poll)), GFP_KERNEL);
		if (!pwlanllc_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanllc_hdr_poll (size %d)\n", sizeof(struct wlanllc_hdr_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
		pwlanbuf_poll = &buf_pool;
#else
		pwlanbuf_poll = (struct	wlanbuf_poll *)
						kmalloc((sizeof(struct	wlanbuf_poll)), GFP_KERNEL);
		if (!pwlanbuf_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanbuf_poll (size %d)\n", sizeof(struct wlanbuf_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
		pwlanicv_poll = &icv_pool;

#else
		pwlanicv_poll = (struct	wlanicv_poll *)
						kmalloc((sizeof(struct	wlanicv_poll)), GFP_KERNEL);
		if (!pwlanicv_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanicv_poll (size %d)\n", sizeof(struct wlanicv_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
		pwlanmic_poll = &mic_pool;
#else
		pwlanmic_poll = (struct	wlanmic_poll *)
						kmalloc((sizeof(struct	wlanmic_poll)), GFP_KERNEL);
		if (!pwlanmic_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanmic_poll (size %d)\n", sizeof(struct wlanmic_poll));
			goto err_out_free;
		}
#endif
	}

	pwlan_acl_poll = (struct wlan_acl_poll *)
					kmalloc((sizeof(struct wlan_acl_poll)), GFP_KERNEL);
	if (!pwlan_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wlan_acl_poll (size %d)\n", sizeof(struct wlan_acl_poll));
		goto err_out_free;
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	pmesh_acl_poll = (struct mesh_acl_poll *)
					kmalloc((sizeof(struct mesh_acl_poll)), GFP_KERNEL);
	if (!pmesh_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Mesh wlan_acl_poll (size %d)\n", sizeof(struct mesh_acl_poll));
		goto err_out_free;
	}
#endif

	Eap_packet = (DOT11_EAP_PACKET *)
					kmalloc((sizeof(DOT11_EAP_PACKET)), GFP_KERNEL);
	if (!Eap_packet) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Eap_packet (size %d)\n", sizeof(DOT11_EAP_PACKET));
		goto err_out_free;
	}
	memset((void *)Eap_packet, 0, sizeof(DOT11_EAP_PACKET));

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	wpa_global_info = (WPA_GLOBAL_INFO *)
					kmalloc((sizeof(WPA_GLOBAL_INFO)), GFP_KERNEL);
	if (!wpa_global_info) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wpa_global_info (size %d)\n", sizeof(WPA_GLOBAL_INFO));
		goto err_out_free;
	}
	memset((void *)wpa_global_info, 0, sizeof(WPA_GLOBAL_INFO));
#endif

#ifndef __DRAYTEK_OS__
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv) {
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0)
			sprintf(dev->name, "%s-vxd", wdev->priv->dev->name);
#endif
#ifdef MBSSID
		if (vap_idx >= 0)
#ifdef CONFIG_RTL8672
			sprintf(dev->name, "%s-vap%d", wdev->priv->dev->name, vap_idx);
#else
			sprintf(dev->name, "%s-va%d", wdev->priv->dev->name, vap_idx);
#endif
#endif
	}
	else
#endif
		strcpy(dev->name, "wlan%d");
#endif

#ifdef LINUX_2_6_24_
	/*SET_MODULE_OWNER is obsolete from 2.6.24*/
#else
	SET_MODULE_OWNER(dev);
#endif
	priv = dev->priv;
	priv->pmib = pmib;
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	/*	only for test	*/
	priv->pmib->wapiInfo.wapiType = wapiDisable;
//	priv->pmib->wapiInfo.wapiUpdateUCastKeyType =
		priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
//	priv->pmib->wapiInfo.wapiUpdateMCastKeyTimeout =
		priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
//	priv->pmib->wapiInfo.wapiUpdateMCastKeyPktNum =
		priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;

#endif
	priv->pevent_queue = pevent_queue;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	priv->wapiEvent_queue= wapiEvent_queue;
#endif
	priv->pwlan_acl_poll = pwlan_acl_poll;
	priv->Eap_packet = Eap_packet;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	priv->wpa_global_info = wpa_global_info;
#endif
#ifdef MBSSID
	priv->vap_id = -1;
#endif
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	priv->pmesh_acl_poll = pmesh_acl_poll;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv) {
		priv->pshare = wdev->priv->pshare;
		GET_ROOT_PRIV(priv) = wdev->priv;
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0) // create for vxd
			GET_VXD_PRIV(wdev->priv) = priv;
#endif
#ifdef MBSSID
		if (vap_idx >= 0)  { // create for vap
			GET_ROOT_PRIV(priv)->pvap_priv[vap_idx] = priv;
			priv->vap_id = vap_idx;
			priv->vap_init_seq = -1;

#if defined(CONFIG_RTL_WAPI_SUPPORT)
			priv->pmib->wapiInfo.wapiType = wapiDisable;
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
			priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
			priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;

			priv->wapiEvent_queue= &wapiVapEvent_queue[vap_idx];

			printk("dev[%s]:wapiType[%d] UCastKeyType[%d] psk[%s] len[%d]\n",
			priv->dev->name, priv->pmib->wapiInfo.wapiType,
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType,
			priv->pmib->wapiInfo.wapiPsk.octet,
			priv->pmib->wapiInfo.wapiPsk.len);
#endif
		}
#endif
	}
	else
#endif
	{
		priv->pshare = pshare;	// david
		priv->pshare->phw = phw;
		priv->pshare->pdesc_info = ptxdesc;
		priv->pshare->pwlan_hdr_poll = pwlan_hdr_poll;
		priv->pshare->pwlanllc_hdr_poll = pwlanllc_hdr_poll;
		priv->pshare->pwlanbuf_poll = pwlanbuf_poll;
		priv->pshare->pwlanicv_poll = pwlanicv_poll;
		priv->pshare->pwlanmic_poll = pwlanmic_poll;
		wdev->priv = priv;
#ifdef __KERNEL__
		spin_lock_init(&priv->pshare->lock);
#endif

#ifdef CONFIG_RTK_MESH
		spin_lock_init(&priv->pshare->lock_queue);
		spin_lock_init(&priv->pshare->lock_Rreq);
#endif
#ifdef CONFIG_NET_PCI
		if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
			priv->pshare->pdev = pdev;
#endif
		priv->pshare->type = wdev->type;
#ifdef USE_RTL8186_SDK
#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
		priv->pshare->have_hw_mic = 1;
#else
		priv->pshare->have_hw_mic = 0;
#endif
#else
		priv->pshare->have_hw_mic = 0;
#endif
//		priv->pshare->is_giga_exist  = is_giga_board();
	}

	priv->dev = dev;
#ifndef __DRAYTEK_OS__
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv)) { // is a vxd or vap
		dev->base_addr = GET_ROOT_PRIV(priv)->dev->base_addr;
		goto register_driver;
	}
#endif
#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
	{
		rc = pci_enable_device(pdev);
		if (rc)
			goto err_out_free;
#ifndef USE_IO_OPS
		rc = pci_request_regions(pdev, DRV_NAME);
#endif
		if (rc)
			goto err_out_disable;

		if (pdev->irq < 2) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "invalid irq (%d) for pci dev\n", pdev->irq);
#else
			printk(KERN_ERR "invalid irq (%d) for pci dev %s\n", pdev->irq, pdev->slot_name);
#endif
			goto err_out_res;
		}
#ifdef USE_IO_OPS
		{
			unsigned long pio_start, pio_len, pio_flags;

			pio_start = (unsigned long)pci_resource_start(pdev, 0);
			pio_len = (unsigned long)pci_resource_len(pdev, 0);
			pio_flags = (unsigned long)pci_resource_flags(pdev, 0);
////			pio_start = (unsigned long)pci_resource_start(pdev, 2);
////			pio_len = (unsigned long)pci_resource_len(pdev, 2);
////			pio_flags = (unsigned long)pci_resource_flags(pdev, 2);


			if (!(pio_flags & IORESOURCE_IO)) {
				rc = -EIO;
#ifdef __LINUX_2_6__
				printk(KERN_ERR "pci: region #0 not a PIO resource, aborting\n");
#else
				printk(KERN_ERR "%s: region #0 not a PIO resource, aborting\n", pdev->slot_name);
#endif
				goto err_out_res;
			}

			if (!request_region(pio_start, pio_len, DRV_NAME)) {
				rc = -EIO;
				printk(KERN_ERR "request_region failed!\n");
				goto err_out_res;
			}

			if (pio_len < RTL8192CD_REGS_SIZE) {
				rc = -EIO;
#ifdef __LINUX_2_6__
				printk(KERN_ERR "PIO resource (%lx) too small on pci dev\n", pio_len);
#else
				printk(KERN_ERR "PIO resource (%lx) too small on pci dev %s\n", pio_len, pdev->slot_name);
#endif
				goto err_out_res;
			}

			dev->base_addr = pio_start;
			priv->pshare->ioaddr = pio_start; // device I/O address
		}
#else
#ifdef IO_MAPPING
		pciaddr = pci_resource_start(pdev, 0);
////		pciaddr = pci_resource_start(pdev, 2);
#else
//		pciaddr = pci_resource_start(pdev, 1);
		pciaddr = pci_resource_start(pdev, 2);
#endif
		if (!pciaddr) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "no MMIO resource for pci dev");
#else
			printk(KERN_ERR "no MMIO resource for pci dev %s\n", pdev->slot_name);
#endif
			goto err_out_res;
		}

//		if ((pmem_len = pci_resource_len(pdev, 1)) < RTL8192CD_REGS_SIZE) {
		if ((pmem_len = pci_resource_len(pdev, 2)) < RTL8192CD_REGS_SIZE) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "MMIO resource () too small on pci dev\n");
#else
			printk(KERN_ERR "MMIO resource (%lx) too small on pci dev %s\n", (unsigned long)pmem_len, pdev->slot_name);
#endif
			goto err_out_res;
		}

		regs = ioremap_nocache(pciaddr, pmem_len);
		if (!regs) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "Cannot map PCI MMIO () on pci dev \n");
#else
			printk(KERN_ERR "Cannot map PCI MMIO (%lx@%lx) on pci dev %s\n", (unsigned long)pmem_len, (long)pciaddr, pdev->slot_name);
#endif
			goto err_out_res;
		}

		dev->base_addr = (unsigned long)regs;
		priv->pshare->ioaddr = (UINT)regs;		
		check_chipID_MIMO(priv);
#endif // USE_IO_OPS
	}
	else
#endif
	{
		regs = (void *)wdev->base_addr;
		dev->base_addr = (unsigned long)wdev->base_addr;
		priv->pshare->ioaddr = (UINT)regs;

		if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_DIRECT)
		{
			int i ;

			_DEBUG_INFO("INIT PCI config space directly\n");
#if !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C)
			if (PCIE_reset_procedure(0, 0, 1, wdev->conf_addr) == FAIL)
				goto err_out_free;
#else
			if (rtl8196b_pci_reset(wdev->conf_addr) == FAIL)
				goto err_out_free;
#endif
#endif


			{
				u32 vendor_deivce_id, config_base;
				config_base = wdev->conf_addr;
				vendor_deivce_id = *((volatile unsigned long *)(config_base+0));
				printk("vendor_deivce_id=%x\n", vendor_deivce_id);
				if ((vendor_deivce_id != ((unsigned long)((0x8191<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8171<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8178<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8174<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8176<<16)|PCI_VENDOR_ID_REALTEK))))
				{
					_DEBUG_ERR("vendor_deivce_id=%x not match\n", vendor_deivce_id);
					rc = -EIO;
					goto err_out_free;
				}
			}

			*((volatile unsigned long *)PCI_CONFIG_BASE1) = virt_to_bus((void *)dev->base_addr);
			//DEBUG_INFO("...config_base1 = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_BASE1));
			for(i=0; i<1000000; i++);
			*((volatile unsigned char *)PCI_CONFIG_COMMAND) = 0x07;
			//DEBUG_INFO("...command = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_COMMAND));
			for(i=0; i<1000000; i++);
			*((volatile unsigned short *)PCI_CONFIG_LATENCY) = 0x2000;
			for(i=0; i<1000000; i++);
			//DEBUG_INFO("...latency = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_LATENCY));

#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8196C)
			#if defined(CONFIG_NET_PCI)
				#define REVR                                    0xB8000000
				#define RTL8196C_REVISION_A     0x80000001
				#define RTL8196C_REVISION_B     0x80000002
			#endif
			if (REG32(REVR) == RTL8196C_REVISION_B) {
				REG32(0xb9000354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
            			REG32(0xb9000358)=0x24;
				for(i=0; i<1000000; i++);
                		REG32(0xb9000354)=0x4270;
                		REG32(0xb9000358)=0x25;
				for(i=0; i<1000000; i++);
				REG32(0xb9000354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
            			REG32(0xb9000358)=0x23;
			}
#endif
			check_chipID_MIMO(priv);
			if ( CheckNoResetHwExceptionCase(priv) ) {
				rtl8192cd_stop_hw(priv);
			}
		}
	}
/*	==========>> maybe later
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
		rtl8192cd_ePhyInit(priv);
*/


#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
		dev->irq = pdev->irq;
		pci_set_drvdata(pdev, dev);
	}
	else
#endif
	{
		dev->irq = wdev->irq;
	}

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
register_driver:
#endif

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
	dev->open = rtl8192cd_open;
	dev->stop = rtl8192cd_close;
	dev->set_multicast_list = rtl8192cd_set_rx_mode;
	dev->hard_start_xmit = rtl8192cd_start_xmit;
	dev->get_stats = rtl8192cd_get_stats;
	dev->do_ioctl = rtl8192cd_ioctl;
	dev->set_mac_address = rtl8192cd_set_hwaddr;
#else
	dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#ifdef CONFIG_RTL8672
	dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;

#ifdef CONFIG_RTL8672
	// Added by Mason Yu
	// MBSSID Port Mapping
	wlanDev[wlanDevNum].dev_pointer = dev;
	wlanDev[wlanDevNum].dev_ifgrp_member = 0;
	wlanDevNum++;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
		DEBUG_INFO("Init %s, base_addr=%08x, irq=%d\n",
			dev->name, (UINT)dev->base_addr,  dev->irq);

#else //  __DRAYTEK_OS__
	regs = (void *)wdev->base_addr;
	dev->base_addr = (unsigned long)wdev->base_addr;
	priv->pshare->ioaddr = (UINT)regs;

#ifdef UNIVERSAL_REPEATER
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
		DEBUG_INFO("Init %s, base_addr=%08x\n",
			dev->name, (UINT)dev->base_addr);

#endif // __DRAYTEK_OS__


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
	{
#ifdef CONFIG_NET_PCI
		if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
			if (cache_size != SMP_CACHE_BYTES) {
//	        	printk(KERN_INFO "%s: PCI cache line size set incorrectly "
//	               "(%i bytes) by BIOS/FW, ", dev->name, cache_size);
		        if (cache_size > SMP_CACHE_BYTES)
	    	        printk("expecting %i\n", SMP_CACHE_BYTES);
	        	else {
	            	printk("correcting to %i\n", SMP_CACHE_BYTES);
		            pci_write_config_byte(pdev, PCI_CACHE_LINE_SIZE,
	                          SMP_CACHE_BYTES >> 2);
	    	    }
		    }

	    	/* enable busmastering and memory-write-invalidate */
		    pci_read_config_word(pdev, PCI_COMMAND, &pci_command);
	    	if (!(pci_command & PCI_COMMAND_INVALIDATE)) {
	        	pci_command |= PCI_COMMAND_INVALIDATE;
		        pci_write_config_word(pdev, PCI_COMMAND, pci_command);
	    	}
		    pci_set_master(pdev);
		}
#endif
	}


#ifdef _INCLUDE_PROC_FS_
#ifdef __KERNEL__
	rtl8192cd_proc_init(dev);
#ifdef PERF_DUMP
	{
		#include <linux/proc_fs.h>

		struct proc_dir_entry *res;
	    res = create_proc_entry("perf_dump", 0, NULL);
	    if (res) {
    	    res->read_proc = read_perf_dump;
    	    res->write_proc = flush_perf_dump;
	    }
	}
#endif
#endif
#endif

	// set some default value of mib
	set_mib_default(priv);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
	{
#ifndef __DRAYTEK_OS__
#ifdef WDS
		wds_num = (priv->pshare->type>>WDS_SHIFT) & WDS_MASK;
		strcpy(baseDevName, dev->name);

		for (i=0; i<wds_num; i++) {
			dev = alloc_etherdev(0);
			if (!dev) {
				printk(KERN_ERR "alloc_etherdev() wds error!\n");
				rc = -ENOMEM;
	    	   	goto err_out_dev;
			}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
			dev->open = rtl8192cd_open;
		    dev->stop = rtl8192cd_close;
		    dev->hard_start_xmit = rtl8192cd_start_xmit;
		    dev->get_stats = rtl8192cd_get_stats;
		    dev->set_mac_address = rtl8192cd_set_hwaddr;
#else
		    dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#ifdef CONFIG_RTL8672
			dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

			priv->wds_dev[i] = dev;
			strcpy(dev->name, baseDevName);
			strcat(dev->name, "-wds%d");
			dev->priv = priv;
		    rc = register_netdev(dev);
			if (rc) {
				printk(KERN_ERR "register_netdev() wds error!\n");
				goto err_out_dev;
			}
		}
#endif // WDS

#ifdef CONFIG_RTK_MESH
		mesh_num = (priv->pshare->type>>MESH_SHIFT) & MESH_MASK;

#ifndef WDS
		strcpy(baseDevName, dev->name);
#endif
		if(mesh_num>0) {
			GET_MIB(priv)->dot1180211sInfo.mesh_enable = 1;
			dev = alloc_etherdev(0);	// mesh allocate ethernet device BUT don't have priv memory (Because share root priv)
			if (!dev) {
				printk(KERN_ERR "alloc_etherdev() mesh error!\n");
				rc = -ENOMEM;
				goto err_out_iomap;
			}
			dev->base_addr = 1;
#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
			dev->open = rtl8192cd_open;
			dev->stop = rtl8192cd_close;
			dev->hard_start_xmit = rtl8192cd_start_xmit;
			dev->get_stats = rtl8192cd_get_stats;
			dev->set_mac_address = rtl8192cd_set_hwaddr;
			dev->do_ioctl = rtl8192cd_ioctl;
#else
			dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#ifdef CONFIG_RTL8672
			dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

			priv->mesh_dev = dev; // NO priv zone dev
			strcpy(dev->name, baseDevName);
			strcat(dev->name, "-msh%d");
			dev->priv = priv;		// mesh priv pointer to root's priv
			rc = register_netdev(dev);
			if (rc) {
				printk(KERN_ERR "register_netdev() mesh error!\n");
				goto err_out_iomap;
			}
		} // end of if(mesh_num>0)

			priv->RreqEnd = 0;
			priv->RreqBegin = 0;

			pann_mpp_tb = (struct mpp_tb*)kmalloc(sizeof(struct mpp_tb), GFP_KERNEL);
			if(!pann_mpp_tb)
			{
				rc = -ENOMEM;
				printk("allocate pann_mpp_tb error!!\n");
				goto err_out_free;
			}
			init_mpp_pool(pann_mpp_tb);
			proxy_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_KERNEL);
			if(!proxy_table)
			{
				rc = -ENOMEM;
				printk("allocate proxy_table error!!\n");
				goto err_out_free;
			}
			memset((void*)proxy_table, 0, sizeof(struct hash_table));

#ifdef PU_STANDARD
			//pepsi
			proxyupdate_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_KERNEL);
			if(!proxyupdate_table)
			{
				rc = -ENOMEM;
				printk("allocate proxyupdate_table error!!\n");
				goto err_out_free;
			}
			memset((void*)proxyupdate_table, 0, sizeof(struct hash_table));
#endif

			pathsel_queue = (DOT11_QUEUE2 *)kmalloc((sizeof(DOT11_QUEUE2)), GFP_KERNEL);
			if (!pathsel_queue) {
				rc = -ENOMEM;
				printk(KERN_ERR "Can't kmalloc for PATHSELECTION_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
				goto err_out_free;
			}
			memset((void *)pathsel_queue, 0, sizeof (DOT11_QUEUE2));
#ifdef _11s_TEST_MODE_
			receiver_queue = (DOT11_QUEUE2 *)kmalloc((sizeof(DOT11_QUEUE2)), GFP_KERNEL);
			if (!receiver_queue) {
				rc = -ENOMEM;
				printk(KERN_ERR "Can't kmalloc for receiver_queue (size %d)\n", sizeof(DOT11_QUEUE));
				goto err_out_free;
			}
			memset((void *)receiver_queue, 0, sizeof (DOT11_QUEUE2));
			pgalileo_poll = (struct Galileo_poll *)	kmalloc((sizeof( struct Galileo_poll)), GFP_KERNEL);
			if (!pgalileo_poll) {
				rc = -ENOMEM;
				printk(KERN_ERR "Can't kmalloc for pgalileo_poll (size %d)\n", sizeof(struct Galileo_poll));
				goto err_out_free;
			}
#endif
			pathsel_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_KERNEL);
			if(!pathsel_table)
			{
				rc = -ENOMEM;
				printk("allocate pathsel_table error!!\n");
				goto err_out_free;
			}
			memset((void*)pathsel_table, 0, sizeof(struct hash_table));

			mesh_rreq_retry_queue = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_KERNEL);
			if(!mesh_rreq_retry_queue)
			{
				rc = -ENOMEM;
				printk("allocate mesh_rreq_retry_queue error!!\n");
				goto err_out_free;
			}
			memset((void*)mesh_rreq_retry_queue, 0, sizeof(struct hash_table));

			rc = init_hash_table(proxy_table, PROXY_TABLE_SIZE, MACADDRLEN, sizeof(struct proxy_table_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"proxy_table\" error!!\n");
			}

#ifdef PU_STANDARD
			//pepsi
			rc = init_hash_table(proxyupdate_table, 8, sizeof(UINT8), sizeof(struct proxyupdate_table_entry), PU_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"proxyupdate_table\" error!!\n");
			}
#endif
			rc = init_hash_table(pathsel_table, 8, MACADDRLEN, sizeof(struct path_sel_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"pathsel_table\" error!!\n");
			}

			rc = init_hash_table(mesh_rreq_retry_queue, DATA_SKB_BUFFER_SIZE, MACADDRLEN, sizeof(struct mesh_rreq_retry_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"mesh_rreq_retry_queue\" error!!\n");
			}

			for(i = 0; i < (1 << mesh_rreq_retry_queue->table_size_power); i++)
			{
				(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr) = (struct pkt_queue*)kmalloc(sizeof(struct pkt_queue), GFP_KERNEL);
				if (!(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr)) {
					rc = -ENOMEM;
					printk(KERN_ERR "Can't kmalloc for mesh_rreq_retry_entry (size %d)\n", sizeof(struct pkt_queue));
					goto err_out_free;
				}
				memset((void *)((((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr)), 0, sizeof (struct pkt_queue));
			}

#ifdef PU_STANDARD
			priv->proxyupdate_table = proxyupdate_table;
#endif
#ifdef _11s_TEST_MODE_
			priv->receiver_queue = receiver_queue;
			priv->pshare->galileo_poll = pgalileo_poll ;
#endif
			priv->proxy_table = proxy_table;
			priv->pathsel_queue = pathsel_queue;
			priv->pann_mpp_tb = pann_mpp_tb;
			priv->pathsel_table = pathsel_table;
			priv->mesh_rreq_retry_queue = mesh_rreq_retry_queue;
			//=========================================================
#endif // CONFIG_RTK_MESH


#endif  // __DRAYTEK_OS__

#ifdef PRIV_STA_BUF
		page_ptr = (unsigned char *)
			(((unsigned long)desc_buf) + (PAGE_SIZE - (((unsigned long)desc_buf) & (PAGE_SIZE-1))));
		phw->ring_buf_len = (unsigned long)desc_buf + sizeof(desc_buf) - (unsigned long)page_ptr;
		phw->ring_dma_addr = virt_to_bus(page_ptr);
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#else
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			page_ptr = pci_alloc_consistent(priv->pshare->pdev, DESC_DMA_PAGE_SIZE, (dma_addr_t *)&phw->ring_dma_addr);
		else
#endif
		{
#ifdef __DRAYTEK_OS__
		page_ptr = rtl8185_malloc(DESC_DMA_PAGE_SIZE, 1);	// allocate non-cache buffer
#else
		page_ptr = kmalloc(DESC_DMA_PAGE_SIZE, GFP_KERNEL);
#endif
		}

		if (page_ptr == NULL) {
			printk(KERN_ERR "can't allocate descriptior page, abort!\n");
			goto err_out_dev;
		}

		phw->alloc_dma_buf = (unsigned long)page_ptr;
		page_ptr = (unsigned char *)
			(((unsigned long)page_ptr) + (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1))));
		phw->ring_buf_len = phw->alloc_dma_buf + DESC_DMA_PAGE_SIZE - ((unsigned long)page_ptr);
		phw->ring_dma_addr = virt_to_bus(page_ptr);
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#endif

		DEBUG_INFO("page_ptr=%lx, size=%ld\n",  (unsigned long)page_ptr, (unsigned long)DESC_DMA_PAGE_SIZE);
		phw->ring_virt_addr = (unsigned long)page_ptr;

#ifdef CONFIG_RTL8190_PRIV_SKB
		init_priv_skb_buf(priv);
#endif

#ifdef PRIV_STA_BUF
		init_priv_sta_buf(priv);
#endif

	}

	INIT_LIST_HEAD(&priv->asoc_list); // init assoc_list first because webs may get sta_num even it is not open,
																// and it will cause exception if it is not init, david+2008-03-05
#ifdef EN_EFUSE
		ReadAdapterInfo8192CE(priv);
#endif
	printk("=====>>EXIT rtl8192cd_init_one <<=====\n");
	return 0;

err_out_dev:

	unregister_netdev(dev);

err_out_iomap:

#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifndef USE_IO_OPS
	    iounmap(regs);
#endif
	}

err_out_res:

	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifdef USE_IO_OPS
		release_region(dev->base_addr, pci_resource_len(pdev, 0));
////	release_region(dev->base_addr, pci_resource_len(pdev, 2));
#else
	    pci_release_regions(pdev);
#endif
	}

err_out_disable:

	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
	    pci_disable_device(pdev);
#endif // CONFIG_NET_PCI

err_out_free:

	if (pmib){
#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(PMIB, pmib);
#else
		kfree(pmib);
#endif
	}
#ifdef  CONFIG_RTK_MESH
	if(proxy_table)
	{
		remove_hash_table(proxy_table);
		kfree(proxy_table);
	}
	if(mesh_rreq_retry_queue)
	{
		for (i=0; i< (1 << mesh_rreq_retry_queue->table_size_power); i++)
		{
			if(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr)
			{
				kfree(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr);
			}
		}
		remove_hash_table(mesh_rreq_retry_queue);
		kfree(mesh_rreq_retry_queue);
	}

	// add by chuangch 2007.09.13
	if(pathsel_table)
	{
		remove_hash_table(pathsel_table);
		kfree(pathsel_table);
	}

	if(pann_mpp_tb)
		kfree(pann_mpp_tb);

	if (pathsel_queue)
		kfree(pathsel_queue);
#ifdef	_11s_TEST_MODE_
	if (receiver_queue)
		kfree(receiver_queue);
#endif
#endif	// CONFIG_RTK_MESH
	if (pevent_queue)
		kfree(pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (wapiEvent_queue)
		kfree(wapiEvent_queue);
#if defined(MBSSID)
	if (wapiVapEvent_queue)
		kfree(wapiVapEvent_queue);
#endif

#endif
#ifndef PRIV_STA_BUF
	if (phw)
		kfree(phw);
	if (pshare)	// david
		kfree(pshare);
	if (pwlan_hdr_poll)
		kfree(pwlan_hdr_poll);
	if (pwlanllc_hdr_poll)
		kfree(pwlanllc_hdr_poll);
	if (pwlanbuf_poll)
		kfree(pwlanbuf_poll);
	if (pwlanicv_poll)
		kfree(pwlanicv_poll);
	if (pwlanmic_poll)
		kfree(pwlanmic_poll);
#endif
	if (pwlan_acl_poll)
		kfree(pwlan_acl_poll);

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	if (pmesh_acl_poll)
		kfree(pmesh_acl_poll);
#endif

	if (Eap_packet)
		kfree(Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	if (wpa_global_info)
		kfree(wpa_global_info);
#endif

#ifdef __LINUX_2_6__
	free_netdev(dev);
#else
    kfree(dev);
#endif
    wdev->priv = NULL;
	printk("=====>>EXIT rtl8192cd_init_one2 <<=====\n");
    return rc;
}


#ifdef CONFIG_RTL_STP
static int rtl_pseudo_dev_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);
	for (i = 0; i<MACADDRLEN; ++i) {
		dev->dev_addr[i] = p[i];
	}
	local_irq_restore(flags);
	return SUCCESS;
}


void rtl_pseudo_dev_init(void* priv)
{
	struct net_device *dev;

/*	printk("[%s][%d] priv of %s\n", __FUNCTION__, __LINE__, ((struct rtl8192cd_priv*)priv)->dev->name);*/
	dev = alloc_etherdev(0);
	if (dev == NULL) {
		printk("alloc_etherdev() pseudo port5 error!\n");
		return;
	}

	dev->open = rtl8192cd_open;
	dev->stop = rtl8192cd_close;
	dev->hard_start_xmit = rtl8192cd_start_xmit;
	dev->get_stats = rtl8192cd_get_stats;
	dev->set_mac_address = rtl_pseudo_dev_set_hwaddr;
	dev->priv = priv;
	strcpy(dev->name, "port5");
	memcpy((char*)dev->dev_addr,"\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);
	if (register_netdev(dev)) {
		printk(KERN_ERR "register_netdev() wds error!\n");
	}
	rtl865x_wlanIF_Init(dev);
}
#endif


#if defined(__DRAYTEK_OS__) && defined(WDS)
int rtl8192cd_add_wds(struct net_device *dev, struct net_device *wds_dev, unsigned char *addr)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int wds_num=priv->pmib->dot11WdsInfo.wdsNum;

	priv->pmib->dot11WdsInfo.dev[wds_num] = wds_dev;
	memcpy(priv->pmib->dot11WdsInfo.entry[wds_num].macAddr, addr, 6);
	wds_dev->priv = priv;
	wds_dev->base_addr = 0;
	priv->pmib->dot11WdsInfo.wdsNum++;

	if (!priv->pmib->dot11WdsInfo.wdsEnabled)
		priv->pmib->dot11WdsInfo.wdsEnabled = 1;

	if (netif_running(priv->dev))
		create_wds_tbl(priv);

	DEBUG_INFO("\r\nAdd WDS: %02x%02x	%02x%02x%02x%02x\n",
		addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
}
#endif


#ifdef CONFIG_NET_PCI
static int MDL_DEVINIT rtl8192cd_init_pci(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int dev_num=0;
	int ret;
#ifdef MBSSID
	int i;
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
	/* For D-MEM allocation system's initialization : It would before ALL processes */
	rtl8192cd_dmem_init();
#endif

	if(dev_num >= (sizeof(wlan_device)/sizeof(struct _device_info_))){
		printk("PCI device %d can't be support\n", dev_num);
		return -1;
	}
	else {
		ret = rtl8192cd_init_one(pdev, ent, &wlan_device[dev_num++], -1);
#ifdef UNIVERSAL_REPEATER
		if (ret == 0) {
			ret = rtl8192cd_init_one(pdev, ent, &wlan_device[--dev_num], -1);
			dev_num++;
		}
#endif
#ifdef MBSSID
		if (ret == 0) {
			dev_num--;
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				ret = rtl8192cd_init_one(pdev, ent, &wlan_device[dev_num], i);
				if (ret != 0) {
					printk("Init fail!\n");
					return ret;
				}
			}
			dev_num++;
		}
#endif
		return ret;
	}
}


static void MDL_DEVEXIT rtk_remove_one(struct pci_dev *pdev)
{
    struct net_device *dev = pci_get_drvdata(pdev);
    struct rtl8192cd_priv *priv = dev->priv;

    if (!dev)
        BUG();

    iounmap((void *)priv->dev->base_addr);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    pci_set_drvdata(pdev, NULL);
}


static struct pci_device_id MDL_DEVINITDATA rtl8192cd_pci_tbl[] =
{
/*
	{ PCI_VENDOR_ID_REALTEK, 0x8190,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
*/
	{ PCI_VENDOR_ID_REALTEK, 0x8191,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8171,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8178,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8176,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8174,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ },
};


MODULE_DEVICE_TABLE(pci, rtl8192cd_pci_tbl);

static struct pci_driver rtl8192cd_driver = {
	name:		DRV_NAME,
	id_table:	rtl8192cd_pci_tbl,
	probe:		rtl8192cd_init_pci,
	remove:		__devexit_p(rtk_remove_one),
};
#endif // CONFIG_NET_PCI


#ifdef CONFIG_WIRELESS_LAN_MODULE
int GetCpuCanSuspend(void)
{
	extern int gCpuCanSuspend;
	return gCpuCanSuspend;
}
#endif


#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
//System identification for CHIP
#define CHIP_OEM_ID	0xb8000000
#define DDR_SELECT	0xb8000008
#define C_CUT		2
#define DDR_BOOT	2
int no_ddr_patch;
#endif


int MDL_INIT __rtl8192cd_init(unsigned long base_addr)
{
#ifdef CONFIG_NET_PCI
	int pci_reg=0;
#endif
	int rc;
#ifdef MBSSID
	int i;
#endif

#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
	//System identification for CHIP
	no_ddr_patch = !((REG32(CHIP_OEM_ID)<C_CUT)&(REG32(DDR_SELECT)&&DDR_BOOT));
#endif

#ifdef __KERNEL__
#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif
#ifdef CONFIG_WIRELESS_LAN_MODULE
	wirelessnet_hook = GetCpuCanSuspend;
#ifdef BR_SHORTCUT
	wirelessnet_hook_shortcut = get_shortcut_dev;
#endif
#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = rtl8651_romeperfEnterPoint;
	Fn_rtl8651_romeperfExitPoint = rtl8651_romeperfExitPoint;
#endif
#ifdef CONFIG_RTL8190_PRIV_SKB
	wirelessnet_hook_is_priv_buf = is_rtl8190_priv_buf;
	wirelessnet_hook_free_priv_buf = free_rtl8190_priv_buf;
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE
#endif // __KERNEL__

#ifndef GREEN_HILL
#ifdef CONFIG_RTL8671
	printk("%s driver version %d.%d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_VERSION_SUBL, DRV_RELDATE);
#else
	panic_printk("%s driver version %d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_RELDATE);
#endif
#endif


#ifdef __KERNEL__
	for (wlan_index=0; wlan_index<sizeof(wlan_device)/sizeof(struct _device_info_); wlan_index++)
#else
	if (wlan_index<sizeof(wlan_device)/sizeof(struct _device_info_))
#endif
	{
		if (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifdef CONFIG_NET_PCI
			if(!pci_reg){
				pci_reg=1;
#ifdef LINUX_2_6_22_
				pci_register_driver(&rtl8192cd_driver);
#else
				pci_module_init(&rtl8192cd_driver);
#endif
			}
#endif
		}
		else {

#ifdef __DRAYTEK_OS__
			wlan_device[wlan_index].base_addr = base_addr;
			wlan_device[wlan_index].type = (TYPE_PCI_DIRECT<<TYPE_SHIFT);
#endif

			rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);

			// victoryman debug
			if (rc)
				printk("init_one fail!!!   rc=%d\n",rc);

#ifdef UNIVERSAL_REPEATER
			if (rc == 0)
				rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);
#endif
#ifdef MBSSID
			if (rc == 0) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], i);
					if (rc != 0) {
						printk("Init fail! rc=%d\n", rc);
						break;
					}
				}
			}
#endif
		}

#ifdef __DRAYTEK_OS__
		if (rc != 0)
			return rc;
#endif

#ifndef __KERNEL__
		wlan_index++;
#endif
	}

#if 0
//#ifdef PCIE_POWER_SAVING
	HostPCIe_Close();
#endif


#ifdef CONFIG_RTL_STP
	rtl_pseudo_dev_init(wlan_device[0].priv);
#endif

#ifdef _USE_DRAM_
	{
	extern unsigned char *en_cipherstream;
	extern unsigned char *tx_cipherstream;
	extern char *rc4sbox, *rc4kbox;
	extern unsigned char *pTkip_Sbox_Lower, *pTkip_Sbox_Upper;
	extern unsigned char Tkip_Sbox_Lower[256], Tkip_Sbox_Upper[256];

#ifdef CONFIG_RTL8671
	extern void r3k_enable_DRAM(void);    //6/7/04' hrchen, for 8671 DRAM init
	r3k_enable_DRAM();    //6/7/04' hrchen, for 8671 DRAM init
#endif

	en_cipherstream = (unsigned char *)(DRAM_START_ADDR);
	tx_cipherstream = en_cipherstream;

	rc4sbox = (char *)(DRAM_START_ADDR + 2048);
	rc4kbox = (char *)(DRAM_START_ADDR + 2048 + 256);
	pTkip_Sbox_Lower = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*2);
	pTkip_Sbox_Upper = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*3);

	memcpy(pTkip_Sbox_Lower, Tkip_Sbox_Lower, 256);
	memcpy(pTkip_Sbox_Upper, Tkip_Sbox_Upper, 256);
	}
#endif

#ifdef __KERNEL__
#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	rtl8192cd_chr_reg_hook = rtl8192cd_chr_reg;
	rtl8192cd_chr_unreg_hook = rtl8192cd_chr_unreg;
//------------------------------------
	rtl8192cd_chr_init();
#endif
#endif

#ifdef CONFIG_RTL8671
	//turn off AP LED
	{
		unsigned char wlanreg = *(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e = (wlanreg | ((1<<5)));
	}
#endif

#ifdef PERF_DUMP
	rtl8651_romeperfInit();
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	register_usb_hook = (register_usb_pkt_cnt_fn)(register_usb_pkt_cnt_f);
#endif

	return 0;
}


#ifdef __DRAYTEK_OS__
int rtl8192cd_init(unsigned long base_addr)
{
	return __rtl8192cd_init(base_addr);
}
#else // not __DRAYTEK_OS__
int MDL_INIT rtl8192cd_init(void)
{
#ifdef CONFIG_RTL8671
	gpioConfig(10,2);
	gpioClear(10);
	delay_ms(10);
	gpioSet(10);
#endif

	return __rtl8192cd_init(0);
}
#endif


#ifdef __KERNEL__
static void MDL_EXIT rtl8192cd_exit (void)
{
	struct net_device *dev;
	struct rtl8192cd_priv *priv;
	int idx;
#if defined(WDS) || defined(MBSSID)
	int i;
#endif

#ifdef CONFIG_WIRELESS_LAN_MODULE
	wirelessnet_hook = NULL;
#ifdef BR_SHORTCUT
	wirelessnet_hook_shortcut = NULL;
#endif
#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = NULL;
	Fn_rtl8651_romeperfExitPoint = NULL;
 #endif
#ifdef CONFIG_RTL8190_PRIV_SKB
	wirelessnet_hook_is_priv_buf = NULL;
	wirelessnet_hook_free_priv_buf = NULL;
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif

#ifdef WDS
	int num;

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		if (wlan_device[idx].priv) {
			num = (wlan_device[idx].type >> WDS_SHIFT) & WDS_MASK;
			for (i=0; i<num; i++) {
				unregister_netdev(wlan_device[idx].priv->wds_dev[i]);
				wlan_device[idx].priv->wds_dev[i]->priv = NULL;
#ifdef __LINUX_2_6__
				free_netdev(wlan_device[idx].priv->wds_dev[i]);
#else
				kfree(wlan_device[idx].priv->wds_dev[i]);
#endif
			}
		}
	}
#endif

#ifdef CONFIG_RTK_MESH
#ifndef WDS
	int num;
#endif

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		num = (wlan_device[idx].type >> MESH_SHIFT) & MESH_MASK;
		if(num > 0) { // num is always 0 or 1 in this time
		// for (i=0; i<num; i++) {
			if (wlan_device[idx].priv) {
				wlan_device[idx].priv->mesh_dev->priv = NULL;
				unregister_netdev(wlan_device[idx].priv->mesh_dev);
				kfree(wlan_device[idx].priv->mesh_dev);
			}
		} // end of if(num > 0)
	}

#endif // CONFIG_RTK_MESH


#ifdef UNIVERSAL_REPEATER
	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		if (wlan_device[idx].priv) {
			struct rtl8192cd_priv *vxd_priv = GET_VXD_PRIV(wlan_device[idx].priv);
			if (vxd_priv) {
				unregister_netdev(vxd_priv->dev);
#ifdef RTL8192CD_VARIABLE_USED_DMEM
				rtl8192cd_dmem_free(PMIB, vxd_priv->pmib);
#else
				kfree(vxd_priv->pmib);
#endif
				kfree(vxd_priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
				kfree(vxd_priv->wapiEvent_queue);
#endif
				kfree(vxd_priv->pwlan_acl_poll);
				kfree(vxd_priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
				kfree(vxd_priv->wpa_global_info);
#endif
#ifdef __LINUX_2_6__
				free_netdev(vxd_priv->dev);
#else
				kfree(vxd_priv->dev);
#endif
				wlan_device[idx].priv->pvxd_priv = NULL;
			}
		}
	}
#endif

#ifdef MBSSID
	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		if (wlan_device[idx].priv) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				struct rtl8192cd_priv *vap_priv = wlan_device[idx].priv->pvap_priv[i];
				if (vap_priv) {
					unregister_netdev(vap_priv->dev);
#ifdef RTL8192CD_VARIABLE_USED_DMEM
					rtl8192cd_dmem_free(PMIB, vap_priv->pmib);
#else
					kfree(vap_priv->pmib);
#endif
					kfree(vap_priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
					kfree(vap_priv->wapiEvent_queue);
#endif
					kfree(vap_priv->pwlan_acl_poll);
					kfree(vap_priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
					kfree(vap_priv->wpa_global_info);
#endif
#ifdef __LINUX_2_6__
					free_netdev(vap_priv->dev);
#else
					kfree(vap_priv->dev);
#endif
					wlan_device[idx].priv->pvap_priv[i] = NULL;
				}
			}
		}
	}
#endif

#ifdef CONFIG_NET_PCI
	pci_unregister_driver (&rtl8192cd_driver);
#endif

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++)
	{
		if (wlan_device[idx].priv == NULL)
			continue;
		priv = wlan_device[idx].priv;
		dev = priv->dev;

#ifdef _INCLUDE_PROC_FS_
		rtl8192cd_proc_remove(dev);
#endif

		unregister_netdev(dev);

#ifndef PRIV_STA_BUF
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			pci_free_consistent(priv->pshare->pdev, DESC_DMA_PAGE_SIZE, (void *)priv->pshare->phw->ring_virt_addr,
				priv->pshare->phw->alloc_dma_buf);
		else
#endif
			kfree((void *)priv->pshare->phw->alloc_dma_buf);
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(PMIB, priv->pmib);
#else
		kfree(priv->pmib);
#endif
		kfree(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
		kfree(priv->wapiEvent_queue);
#ifdef MBSSID
		if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable){
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
				kfree(priv->pvap_priv[i]->wapiEvent_queue);
		}
#endif

#endif

#ifdef CONFIG_RTK_MESH
		kfree(priv->pathsel_queue);
#ifdef _11s_TEST_MODE_
		kfree(priv->receiver_queue);

		for(i=0; i< AODV_RREQ_TABLE_SIZE; i++)
			del_timer(&priv->pshare->galileo_poll->node[i].data.expire_timer);

		kfree(priv->pshare->galileo_poll);
#endif
#ifdef	_MESH_ACL_ENABLE_
		kfree(priv->pmesh_acl_poll);
#endif
#endif

#ifndef PRIV_STA_BUF
		kfree(priv->pshare->phw);
		kfree(priv->pshare->pwlan_hdr_poll);
		kfree(priv->pshare->pwlanllc_hdr_poll);
		kfree(priv->pshare->pwlanbuf_poll);
		kfree(priv->pshare->pwlanicv_poll);
		kfree(priv->pshare->pwlanmic_poll);
#endif
		kfree(priv->pwlan_acl_poll);
		kfree(priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
		kfree(priv->wpa_global_info);
#endif
#ifndef PRIV_STA_BUF
		kfree(priv->pshare);	// david
#endif
#ifdef __LINUX_2_6__
		free_netdev(dev);
#else
		kfree(dev);
#endif
		wlan_device[idx].priv = NULL;
	}

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	rtl8192cd_chr_reg_hook = NULL;
	rtl8192cd_chr_unreg_hook = NULL;
//------------------------------------

	rtl8192cd_chr_exit();
#endif
}
#else // not __KERNEL__
void MDL_EXIT rtl8192cd_exit(void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = dev->priv;
	int idx, i;

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++)
		if (wlan_device[idx].priv == priv)
			break;

	if (idx == sizeof(wlan_device)/sizeof(struct _device_info_))
		return;		// wrong argument!!

#ifdef WDS
	{
		int num;

		num = (wlan_device[idx].type >> WDS_SHIFT) & WDS_MASK;
		for (i=0; i<num; i++) {
			wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]->priv = NULL;
			unregister_netdev(wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]);
			kfree(wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]);
		}
	}
#endif

	unregister_netdev(dev);

	kfree(priv->pmib);

#ifdef	CONFIG_RTK_MESH

	if(priv->proxy_table)
	{
		remove_hash_table(priv->proxy_table);
		kfree(priv->proxy_table);
	}
	if(priv->mesh_rreq_retry_queue)
	{
		for (i = 0; i < (1 << priv->mesh_rreq_retry_queue->table_size_power); i++) {
			if(((struct mesh_rreq_retry_entry*)(priv->mesh_rreq_retry_queue->entry_array[i].data))->ptr)
				kfree(((struct mesh_rreq_retry_entry*)(priv->mesh_rreq_retry_queue->entry_array[i].data))->ptr);
		}
		remove_hash_table(priv->mesh_rreq_retry_queue);
		kfree(priv->mesh_rreq_retry_queue);
	}

	// add by chuangch 2007.09.13
	if(priv->pathsel_table)
	{
		remove_hash_table(priv->pathsel_table);
		kfree(priv->pathsel_table);
	}

	if(priv->pann_mpp_tb)
		kfree(priv->pann_mpp_tb);

	kfree(priv->pathsel_queue);
#ifdef _11s_TEST_MODE_
	kfree(priv->receiver_queue);
	for(i=0; i< AODV_RREQ_TABLE_SIZE; i++)
		del_timer(&priv->pshare->galileo_poll->node[i].data.expire_timer);

	kfree(priv->pshare->galileo_poll);
#endif

#ifdef	_MESH_ACL_ENABLE_
	kfree(priv->pmesh_acl_poll);
#endif
#endif	// CONFIG_RTK_MESH

	kfree(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
//	kfree(vxd_priv->wapiEvent_queue);
	kfree(priv->wapiEvent_queue);
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)	{
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			kfree(priv->pvap_priv[i]->wapiEvent_queue);
	}
#endif

#endif
#ifndef PRIV_STA_BUF
	kfree((void *)priv->pshare->phw->alloc_dma_buf);

	kfree(priv->pshare->phw);
	kfree(priv->pshare->pwlan_hdr_poll);
	kfree(priv->pshare->pwlanllc_hdr_poll);
	kfree(priv->pshare->pwlanbuf_poll);
	kfree(priv->pshare->pwlanicv_poll);
	kfree(priv->pshare->pwlanmic_poll);
#endif
	kfree(priv->pwlan_acl_poll);
	kfree(priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	kfree(priv->wpa_global_info);
#endif
#ifndef PRIV_STA_BUF
	kfree(priv->pshare);	// david
#endif
	kfree(dev);
	wlan_device[idx].priv = NULL;

	wlan_index--;
}
#endif


#ifdef __KERNEL__
#ifdef USE_CHAR_DEV
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv)
{
	if (wlan_device[minor].priv)
		wlan_device[minor].priv->pshare->chr_priv = priv;
	return wlan_device[minor].priv;
}


void rtl8192cd_chr_unreg(unsigned int minor)
{
	if (wlan_device[minor].priv)
		wlan_device[minor].priv->pshare->chr_priv = NULL;
}
#endif


#ifdef RTL_WPA2_PREAUTH
void wpa2_kill_fasync(void)
{
	int wlan_index = 0;
	struct _device_info_ *wdev = &wlan_device[wlan_index];
	struct rtl8192cd_priv *priv = wdev->priv;
	event_indicate(priv, NULL, -1);
}


void wpa2_preauth_packet(struct sk_buff	*pskb)
{
	// ****** NOTICE **********
	int wlan_index = 0;
	struct _device_info_ *wdev = &wlan_device[wlan_index];
	// ****** NOTICE **********

	struct rtl8192cd_priv *priv = wdev->priv;

	unsigned char		szEAPOL[] = {0x02, 0x01, 0x00, 0x00};
	DOT11_EAPOL_START	Eapol_Start;

	if (priv == NULL) {
		PRINT_INFO("%s: priv == NULL\n", (char *)__FUNCTION__);
		return;
	}

#ifndef WITHOUT_ENQUEUE
	if (!memcmp(pskb->data, szEAPOL, sizeof(szEAPOL)))
	{
		Eapol_Start.EventId = DOT11_EVENT_EAPOLSTART_PREAUTH;
		Eapol_Start.IsMoreEvent = FALSE;
#ifdef LINUX_2_6_22_
		memcpy(&Eapol_Start.MACAddr, pskb->mac_header + MACADDRLEN, WLAN_ETHHDR_LEN);
#else
		memcpy(&Eapol_Start.MACAddr, pskb->mac.raw + MACADDRLEN, WLAN_ETHHDR_LEN);
#endif
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&Eapol_Start, sizeof(DOT11_EAPOL_START));
	}
	else
	{
		unsigned short		pkt_len;

		pkt_len = WLAN_ETHHDR_LEN + pskb->len;
		priv->Eap_packet->EventId = DOT11_EVENT_EAP_PACKET_PREAUTH;
		priv->Eap_packet->IsMoreEvent = FALSE;
		memcpy(&(priv->Eap_packet->packet_len), &pkt_len, sizeof(unsigned short));
#ifdef LINUX_2_6_22_
		memcpy(&(priv->Eap_packet->packet[0]), pskb->mac_header, WLAN_ETHHDR_LEN);
#else
		memcpy(&(priv->Eap_packet->packet[0]), pskb->mac.raw, WLAN_ETHHDR_LEN);
#endif
		memcpy(&(priv->Eap_packet->packet[WLAN_ETHHDR_LEN]), pskb->data, pskb->len);
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)priv->Eap_packet, sizeof(DOT11_EAP_PACKET));
	}
#endif // WITHOUT_ENQUEUE

	event_indicate(priv, NULL, -1);

	// let dsr to free this skb
}
#endif // RTL_WPA2_PREAUTH
#endif // __KERNEL__


#ifdef BR_SHORTCUT
#ifdef __LINUX_2_6__
__IRAM_IN_865X
#else
__MIPS16
__IRAM_IN_865X
#endif
struct net_device *get_shortcut_dev(unsigned char *da)
{
	int i;
	struct stat_info *pstat;
	struct rtl8192cd_priv *priv;
	struct net_device *dev = NULL;

#ifdef CONFIG_RTK_MESH	//11 mesh no support shortcut now
	{
		extern unsigned char cached_mesh_mac[MACADDRLEN];
		extern struct net_device *cached_mesh_dev;
		if (cached_mesh_dev && !memcmp(da, cached_mesh_mac, MACADDRLEN))
			return cached_mesh_dev;
	}
#endif

#ifdef WDS
	{
		extern unsigned char cached_wds_mac[MACADDRLEN];
		extern struct net_device *cached_wds_dev;
		if (cached_wds_dev && !memcmp(da, cached_wds_mac, MACADDRLEN)) {
			priv = cached_wds_dev->priv;
			if (!priv->pmib->dot11OperationEntry.disable_brsc)
				return cached_wds_dev;
		}
	}
#endif

#ifdef CLIENT_MODE
	{
		extern unsigned char cached_sta_mac[MACADDRLEN];
		extern struct net_device *cached_sta_dev;
		if (cached_sta_dev && !memcmp(da, cached_sta_mac, MACADDRLEN)) {
			priv = cached_sta_dev->priv;
			if (!priv->pmib->dot11OperationEntry.disable_brsc)
				return cached_sta_dev;
		}
	}
#endif

#if	defined(RTL_CACHED_BR_STA)
	{
		extern unsigned char cached_br_sta_mac[MACADDRLEN];
		extern struct net_device *cached_br_sta_dev;
		if (!memcmp(da, cached_br_sta_mac, MACADDRLEN))
			return cached_br_sta_dev;
	}
#endif

	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)) && (dev==NULL); i++)
	{
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev))
		{
			priv = wlan_device[i].priv;

			if (
#ifndef _SINUX_ 
                // if sinux, no linux bridge, so should don't depend on br_port if use br_shortcut (John Qian 2010/6/24) 
                (priv->dev->br_port) &&
				!(priv->dev->br_port->br->stp_enabled) &&
#endif
				!(priv->pmib->dot11OperationEntry.disable_brsc))
			{
				pstat = get_stainfo(priv, da);
				if (pstat) {
					if (pstat->tx_pkts > 10) {	/* Make sure it must have some packets go theough bridge module before shortcut */
#ifdef WDS
						if (!(pstat->state & WIFI_WDS))	// if WDS peer
#endif
					    {
#ifdef CONFIG_RTK_MESH
							if (isMeshPoint(pstat))
								dev = priv->mesh_dev;
							else
#endif
								dev = priv->dev;
						}
					}
				}
#ifdef MBSSID
				else if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) {
					int j;
					for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
						if (IS_DRV_OPEN(priv->pvap_priv[j])) {
							pstat = get_stainfo(priv->pvap_priv[j], da);
							if (pstat) {
								if (pstat->tx_pkts > 10) {
									dev = priv->pvap_priv[j]->dev;
									break;
								}
							}
						}
					}
				}
#endif
			}
		}
	}

#if	defined(RTL_CACHED_BR_STA)
	memcpy(cached_br_sta_mac, da, MACADDRLEN);
	cached_br_sta_dev = dev;
#endif

	return dev;
}


void clear_shortcut_cache(void)
{
	extern 	 unsigned char cached_eth_addr[MACADDRLEN];
	extern struct net_device *cached_dev;
#ifdef CLIENT_MODE
	extern struct net_device *cached_sta_dev;
	extern unsigned char cached_sta_mac[MACADDRLEN];
#endif
#ifdef CONFIG_RTK_MESH
	extern struct net_device *cached_mesh_dev;
	extern unsigned char cached_mesh_mac[MACADDRLEN];
	cached_mesh_dev	= NULL;
	memset(cached_mesh_mac,0,MACADDRLEN);
#endif

#ifdef WDS
	extern struct net_device *cached_wds_dev;
	extern unsigned char cached_wds_mac[MACADDRLEN];
	cached_wds_dev= NULL;
	memset(cached_wds_mac,0,MACADDRLEN);
#endif

#ifdef CLIENT_MODE
	cached_sta_dev = NULL;
	memset(cached_sta_mac,0,MACADDRLEN);
#endif

#if	defined(RTL_CACHED_BR_STA)
	extern struct net_device *cached_br_sta_dev;
	extern unsigned char cached_br_sta_mac[MACADDRLEN];
	cached_br_sta_dev = NULL;
	memset(cached_br_sta_mac,0,MACADDRLEN);
#endif
	cached_dev = NULL;
	memset(cached_eth_addr,0,MACADDRLEN);
#ifdef CONFIG_RTL8672	
	extern void clear_cached_eth_mac_addr(void);
	clear_cached_eth_mac_addr();
#endif
}
#endif // BR_SHORTCUT


void update_fwtbl_asoclst(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];
	int i;

#ifdef __KERNEL__
	struct sk_buff *skb = NULL;
	struct wlan_ethhdr_t *e_hdr;
	unsigned char xid_cmd[] = {0, 0, 0xaf, 0x81, 1, 2};

	// update forwarding table of bridge module
	if (priv->dev->br_port) {
		skb = dev_alloc_skb(64);
		if (skb != NULL) {
			skb->dev = priv->dev;
			skb_put(skb, 60);
			e_hdr = (struct wlan_ethhdr_t *)skb->data;
			memset(e_hdr, 0, 64);
			memcpy(e_hdr->daddr, priv->dev->dev_addr, MACADDRLEN);
			memcpy(e_hdr->saddr, pstat->hwaddr, MACADDRLEN);
			e_hdr->type = 8;
			memcpy(&skb->data[14], xid_cmd, sizeof(xid_cmd));
			skb->protocol = eth_type_trans(skb, priv->dev);
#if defined(__LINUX_2_6__) && defined(RX_TASKLET)
			netif_receive_skb(skb);
#else
			netif_rx(skb);
#endif
		}
	}
#endif

	// update association lists of the other WLAN interfaces
	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) {
		if (wlan_device[i].priv && (wlan_device[i].priv != priv)) {
			if (wlan_device[i].priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
				sprintf(tmpbuf, "%02x%02x%02x%02x%02x%02x",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
				del_sta(wlan_device[i].priv, tmpbuf);
			}
		}
	}

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (priv->pvap_priv[i] && IS_DRV_OPEN(priv->pvap_priv[i]) && (priv->pvap_priv[i] != priv) &&
				(priv->vap_init_seq >= 0)) {
				if (priv->pvap_priv[i]->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
					sprintf(tmpbuf, "%02x%02x%02x%02x%02x%02x",
						pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
					del_sta(priv->pvap_priv[i], tmpbuf);
				}
			}
		}
	}
#endif
}


// quick fix for warn reboot fail issue
void force_stop_wlan_hw(void)
{
	int i;

	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) {
		if (wlan_device[i].priv) {
			struct rtl8192cd_priv *priv = wlan_device[i].priv;
#ifdef PCIE_POWER_SAVING
			if ((REG32(CLK_MANAGE) & BIT(11)) == 0)
			{
				extern void setBaseAddressRegister(void);
				REG32(CLK_MANAGE) |= BIT(11);
				delay_ms(10);
				PCIE_reset_procedure(0, 0, 1, wlan_device[i].base_addr);
				setBaseAddressRegister();
			}
#endif
			if ( CheckNoResetHwExceptionCase(priv) ) {
				rtl8192cd_stop_hw(priv);
			}
#if 0
//#ifdef PCIE_POWER_SAVING
			HostPCIe_Close();
#endif
		}
	}
}


#ifdef CONFIG_RTL8671
/*6/7/04 hrchen, invalidate the dcache with a 0->1 transition*/

#ifdef CONFIG_CPU_RLX4181
int r3k_flush_dcache_range(int a, int b)
{
  int garbage_tmp;
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
		"mfc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"andi %0, 0xFDFF\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"ori %0, 0x200\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                : "=r" (garbage_tmp));
        return 0;
}
#else
int r3k_flush_dcache_range(int a, int b)
{
  int garbage_tmp;
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
		"mfc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"andi %0, 0xFFFE\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"ori %0, 1\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                : "=r" (garbage_tmp));
        return 0;
}
#endif

#ifdef _USE_DRAM_
//init DRAM
void r3k_enable_DRAM(void)
{
  int tmp, tmp1;
	//--- initialize and start COP3
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
		"mfc0	%0,$12\n\t"
		"nop\n\t"
		"lui	%1,0x8000\n\t"
		"or	%1,%0\n\t"
		"mtc0	%1,$12\n\t"
		"nop\n\t"
		"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
		: "=r" (tmp), "=r" (tmp1));

	//set base
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
	 	"mtc3 %0, $4	# $4: d-ram base\n\t"
 	 	"nop\n\t"
	 	"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
		:
		: "r" (DRAM_START_ADDR&0x0fffffff));

	//set size
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
	 	"mtc3 %0, $5    # $5: d-ram top\n\t"
	 	"nop\n\t"
	 	"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
		:
		: "r" (R3K_DRAM_SIZE-1));

	//clear DRAM
	__asm__ __volatile__(
"1:\n\t"
	 	"sw	$0,0(%1)\n\t"
	 	"addiu	%1,4\n\t"
	 	"bne	%0,%1,1b\n\t"
	 	"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                :
                : "r" (DRAM_START_ADDR+R3K_DRAM_SIZE), "r" (DRAM_START_ADDR));
}
#endif // _USE_DRAM_
#endif // CONFIG_RTL8671


#ifdef __KERNEL__
#ifdef CONFIG_X86
MODULE_LICENSE("GPL");
#endif
module_init(rtl8192cd_init);
module_exit(rtl8192cd_exit);
#endif

