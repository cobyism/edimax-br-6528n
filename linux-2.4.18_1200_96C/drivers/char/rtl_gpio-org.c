/*
 * FILE NAME rtl_gpio.c
 *
 * BRIEF MODULE DESCRIPTION
 *  GPIO For Flash Reload Default
 *
 *  Author: jimmylin@realtek.com.tw
 *
 * Copyright 2005 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE	LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */



#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/reboot.h>
#include <linux/kmod.h>
#include <linux/proc_fs.h>

#define AUTO_CONFIG

// 2009-0414
//#define	DET_WPS_SPEC
#ifdef CONFIG_RTL_EB8186
	#define READ_RF_SWITCH_GPIO
#endif

#ifdef CONFIG_RTL8196B
	#define READ_RF_SWITCH_GPIO
#endif

#if defined(CONFIG_RTL865X)
	#include <asm/rtl865x/rtl865xc_asicregs.h>

	#define RTL_GPIO_PABDIR		PABCD_DIR
	#define RTL_GPIO_PABDATA	PABCD_DAT
	#define RTL_GPIO_PABCNR		PABCD_CNR
	#define RTL_GPIO_PEFGHDIR		PEFGH_DIR
	#define RTL_GPIO_PEFGHDATA	PEFGH_DAT

	#define AUTOCFG_BTN_PIN		15
	#define AUTOCFG_LED_PIN		14
	#define RESET_LED_PIN		13
	#define RESET_BTN_PIN		15
	#define WEP_WPA_LED_PIN	25
	#define MAC_CTL_LED_PIN	24
	#define BRIDGE_REPEATER_LED_PIN	11
	#define ETHER_LED_REG 0xbb804300
	#define LEDCR0 (ETHER_LED_REG + 0x00)
	#define LEDCR1 (ETHER_LED_REG + 0x04)
	#define LEDBCR (ETHER_LED_REG + 0x0C)
#endif
#if defined(CONFIG_RTL8196B)
	#ifdef CONFIG_RTL8196C
	#include <asm/rtl865x/rtl865xc_asicregs.h>
	#define RTL_GPIO_PABDIR		PABCD_DIR
	#define RTL_GPIO_PABDATA	PABCD_DAT
	#define RTL_GPIO_PABCNR		PABCD_CNR
	#define AUTOCFG_BTN_PIN		3
	#define AUTOCFG_LED_PIN		4
	#define RESET_LED_PIN		6
	#define RESET_BTN_PIN		5
	#define RTL_GPIO_MUX 0xB8000040
	#define RTL_GPIO_MUX_DATA 0x00340C00//for WIFI ON/OFF and GPIO
	#define RTL_GPIO_WIFI_ONOFF	19

#ifdef CONFIG_POCKET_ROUTER_SUPPORT
	#define RTL_GPIO_MUX_GPIOA2	(3<<20)
	#define RTL_GPIO_MUX_GPIOB3	(3<<2)
	#define RTL_GPIO_MUX_GPIOB2	(3<<0)	
	#define RTL_GPIO_MUX_GPIOC0	(3<<12)
	#define RTL_GPIO_MUX_POCKETAP_DATA	(RTL_GPIO_MUX_GPIOA2 | RTL_GPIO_MUX_GPIOB3 | RTL_GPIO_MUX_GPIOB2 | RTL_GPIO_MUX_GPIOC0)

	#define RTL_GPIO_CNR_GPIOA2	(1<<2)
	#define RTL_GPIO_CNR_GPIOB3	(1<<11)
	#define RTL_GPIO_CNR_GPIOB2	(1<<10)	
	#define RTL_GPIO_CNR_GPIOC0	(1<<16)
	#define RTL_GPIO_CNR_POCKETAP_DATA	(RTL_GPIO_CNR_GPIOA2 | RTL_GPIO_CNR_GPIOB3 | RTL_GPIO_CNR_GPIOB2 | RTL_GPIO_CNR_GPIOC0)

	#define RTL_GPIO_DIR_GPIOA2	(1<<2) /* &- */
	#define RTL_GPIO_DIR_GPIOB3	(1<<11) /* &- */
	#define RTL_GPIO_DIR_GPIOB2	(1<<10) /* |*/	
	#define RTL_GPIO_DIR_GPIOC0	(1<<16) /* &- */

	#define RTL_GPIO_DAT_GPIOA2	(1<<2) 
	#define RTL_GPIO_DAT_GPIOB3	(1<<11) 
	#define RTL_GPIO_DAT_GPIOB2	(1<<10) 	
	#define RTL_GPIO_DAT_GPIOC0	(1<<16) 

	static int ap_cli_rou_time_state[2] = {0};
	static char ap_cli_rou_state = 0;
	static char ap_cli_rou_idx=0;
	static char pocketAP_hw_set_flag='0';

	static char dc_pwr_plugged_time_state = 0;
	static char dc_pwr_plugged_state = 0;
	static char dc_pwr_plugged_flag = '0';

	static int pwr_saving_state=0;
	static char pwr_saving_led_toggle = 0;
#endif
	#else
	#include <asm/rtl865x/rtl865xc_asicregs.h>
	#define RTL_GPIO_PABDIR		PABCD_DIR
	#define RTL_GPIO_PABDATA	PABCD_DAT
	#define RTL_GPIO_PABCNR		PABCD_CNR
	#define AUTOCFG_BTN_PIN		1
	#define AUTOCFG_LED_PIN		20
	#define RESET_LED_PIN		18
	#define RESET_BTN_PIN		0
	#define RTL_GPIO_MUX 0xB8000030
	#define RTL_GPIO_MUX_DATA 0x0FC00380//for WIFI ON/OFF and GPIO
	#define RTL_GPIO_WIFI_ONOFF	19
	#endif


#endif


#ifdef CONFIG_RTL_EB8186
	#define RTL_GPIO_PABDIR		0xbd010124
	#define RTL_GPIO_PABDATA	0xbd010120
	#define RESET_BTN_PIN		0
	#define AUTOCFG_BTN_PIN		1
	#define RESET_LED_PIN		2
	#define AUTOCFG_LED_PIN		3
	#define AUTOCFG_LED_OFF		0
	#define AUTOCFG_LED_ON		1
	#define AUTOCFG_LED_BLINK	2
#if 0
#else
	#define RTL_GPIO_PABDIR		0xbd010040
	#define RTL_GPIO_PABDATA	0xbd010044
	#define RTL_WLAN_CR9346		0xbd400050
	#define RTL_WLAN_CONFIG0	0xbd400051
	#define RTL_WLAN_PSR		0xbd40005e
	#define RESET_BTN_PIN		10
#endif
#endif

#ifdef CONFIG_RTL865X_KLD
	#define PS_LED_GREEN_PIN	13
	#define PS_LED_ORANGE_PIN	14
	#define INET_LED_GREEN_PIN	24
	#define INET_LED_ORANGE_PIN	25
	#define GPIO_IRQ_NUM		1	
#endif

#if defined(CONFIG_RTL8196B_KLD)
	#define PS_LED_GREEN_PIN	18
	#define PS_LED_ORANGE_PIN	3
	#define INET_LED_GREEN_PIN	8
	#define INET_LED_ORANGE_PIN	17
	#define GPIO_IRQ_NUM		1	
	static char test_mode_flag = '0';
	static unsigned int    rst_btn_flag;
	static unsigned int    wps_btn_flag;
#endif

#if defined(CONFIG_RTL8196C_KLD) 
	#define PS_LED_GREEN_PIN 6
	#define INET_LED_GREEN_PIN	14
	#define GPIO_IRQ_NUM		1	
	#define _GIMR_			0xb8003000 //keith	
	static char test_mode_flag = '0';
	static unsigned int    rst_btn_flag;
	static unsigned int    wps_btn_flag;
#endif

// 2009-0414
#ifdef DET_WPS_SPEC
	#define GPIO_IRQ_NUM		1	
#endif
#ifdef CONFIG_RTL8186_KB
	#define PROBE_TIME	10
#elif defined(CONFIG_RTL865X_SC)
	#define PROBE_TIME	10
#else
	#define PROBE_TIME	5
#endif

#define PROBE_NULL		0
#define PROBE_ACTIVE	1
#define PROBE_RESET		2
#define PROBE_RELOAD	3
#define RTL_R32(addr)		(*(volatile unsigned long *)(addr))
#define RTL_W32(addr, l)	((*(volatile unsigned long*)(addr)) = (l))
#define RTL_R8(addr)		(*(volatile unsigned char*)(addr))
#define RTL_W8(addr, l)		((*(volatile unsigned char*)(addr)) = (l))

//#define  GPIO_DEBUG
#ifdef GPIO_DEBUG
/* note: prints function name for you */
#  define DPRINTK(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define DPRINTK(fmt, args...)
#endif

static struct timer_list probe_timer;
static unsigned int    probe_counter;
static unsigned int    probe_state;

static char default_flag='0';
//Brad add for update flash check 20080711
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL8196C_KLD)  && !defined(CONFIG_RTL8196C_EC)
int start_count_time=0;
int Reboot_Wait=0;
#endif
#ifdef CONFIG_RTL865X_CMO
int	system_led_blink=0;
unsigned int sys_keep_alive = 0;
#endif
#ifndef CONFIG_RTL_EB8186
unsigned int led0enable;
#endif

#ifdef CONFIG_RTL8186_KB
int	disable_power_led_blink=0;
#endif

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
int	system_led_blink=0;
extern int flash_write_flag;
#if 0 //Brad disable, since the check is not precise for system status			
unsigned int sys_keep_alive = 0;
#endif
#endif

//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
int	system_led_blink=0;
extern int flash_write_flag;
#if 0 //Brad disable, since the check is not precise for system status			
unsigned int sys_keep_alive = 0;
#endif
int kld_model=0;
#endif

#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
static char ps_led_flag = 0;
static struct timer_list ps_led_timer;
static char ps_green_toggle = 0;
static char ps_orange_toggle = 0;

static char inet_led_flag = 0;
static struct timer_list inet_led_timer;
static char inet_green_toggle = 0;
static char inet_orange_toggle = 0;
static int wps_button_push = 0;

static struct timer_list wps_QuickBlink_timer; 
static void ps_led_process(void);
#endif

#ifdef CONFIG_POCKET_ROUTER_SUPPORT
static struct timer_list pocket_ap_timer;
#endif

#ifdef	DET_WPS_SPEC
static int wps_button_push = 0;

#endif

#ifdef AUTO_CONFIG
static unsigned int		AutoCfg_LED_Blink;
static unsigned int		AutoCfg_LED_Toggle;

#if defined(CONFIG_RTL865X_SC)
static unsigned int ResetToAutoCfgBtn = 0;
#endif

//#define DETECT_ESD
#if defined(DETECT_ESD)
static unsigned char phy_ready = 0;
#endif

#ifdef CONFIG_RTL865X_KLD
void ps_led_gpio_init(void)
{
	if(kld_model==1){
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_GREEN_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PEFGH_CNR, (RTL_R32(PEFGH_CNR) & (~((1 << PS_LED_ORANGE_PIN) | (1 << INET_LED_GREEN_PIN) | (1 << INET_LED_ORANGE_PIN)))));
	RTL_W32(PEFGH_DIR, (RTL_R32(PEFGH_DIR) | ((1 << PS_LED_ORANGE_PIN) | (1 << INET_LED_GREEN_PIN) | (1 << INET_LED_ORANGE_PIN))));
	RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
	RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | ((1 << INET_LED_GREEN_PIN) | (1 << INET_LED_ORANGE_PIN))));
	}else{

	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_GREEN_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
	}
	
}
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
void ps_led_gpio_init(void)
{
	#if defined(CONFIG_RTL8196B_KLD)
	RTL_W32(0xB8000030, (RTL_R32(0xB8000030) | 0x00F00F80 ));
	#else
	RTL_W32(0xB8000040, (RTL_R32(0xB8000040) | 0x00300300 ));
	#endif
	if(kld_model==1){
	/* ps_led init setting */
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_GREEN_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
	
	#if !defined(CONFIG_RTL8196C_KLD)
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_ORANGE_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_ORANGE_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
	#endif
	/* inet_led init setting */

	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << INET_LED_GREEN_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << INET_LED_GREEN_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));
	
	#if !defined(CONFIG_RTL8196C_KLD)
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << INET_LED_ORANGE_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << INET_LED_ORANGE_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
	#endif


	}
	
}
#endif
void autoconfig_gpio_init(void)
{
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	RTL_W32(PABCD_CNR,(RTL_R32(PABCD_CNR)&(~(1 << AUTOCFG_BTN_PIN))));
	RTL_W32(PABCD_CNR,(RTL_R32(PABCD_CNR)&(~(1 << AUTOCFG_LED_PIN))));
#endif

	// Set GPIOA pin 1 as input pin for auto config button
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) & (~(1 << AUTOCFG_BTN_PIN))));

	// Set GPIOA ping 3 as output pin for auto config led
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) | (1 << AUTOCFG_LED_PIN)));

	// turn off auto config led in the beginning
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
}
#ifdef CONFIG_RTL8196B_GW_MP

#if defined(CONFIG_RTL8196B_GW_MP_KLD)
	#define PS_LED_GREEN_PIN	18
	#define PS_LED_ORANGE_PIN	3
	#define INET_LED_GREEN_PIN	8
	#define INET_LED_ORANGE_PIN	17
	#define GPIO_IRQ_NUM		1	
	#define AUTOCFG_LED_PIN_MP        20
	#define RESET_LED_PIN_MP           18
#endif
void all_led_on(void)
{
	//printk("Into MP GPIO");
	 #ifdef CONFIG_RTL8196B_GW_MP_KLD
	RTL_W32(0xB8000030, (RTL_R32(0xB8000030) | 0x00F00F80 ));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_GREEN_PIN))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_ORANGE_PIN))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_ORANGE_PIN)));
	
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));

	/* inet_led init setting */
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << INET_LED_GREEN_PIN))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << INET_LED_ORANGE_PIN))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << INET_LED_GREEN_PIN)));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << INET_LED_ORANGE_PIN)));
	
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << AUTOCFG_LED_PIN_MP))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << AUTOCFG_LED_PIN_MP)));

	//RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
	//RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));	
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << AUTOCFG_LED_PIN_MP))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << RESET_LED_PIN_MP))));
	#endif

}
#endif
void autoconfig_gpio_off(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
	AutoCfg_LED_Blink = 0;
}


void autoconfig_gpio_on(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
	AutoCfg_LED_Blink = 0;
}

#ifdef CONFIG_RTL865X_CMO
void extra_led_gpio_init(void)
{
	RTL_W32(PABCD_CNR,(RTL_R32(PABCD_CNR)&(~(1 << RESET_LED_PIN))));
	RTL_W32(PEFGH_CNR,(RTL_R32(PEFGH_CNR)&(~(1 << WEP_WPA_LED_PIN))));
	RTL_W32(PEFGH_CNR,(RTL_R32(PEFGH_CNR)&(~(1 << MAC_CTL_LED_PIN))));
	RTL_W32(PEFGH_CNR,(RTL_R32(PEFGH_CNR)&(~(1 << BRIDGE_REPEATER_LED_PIN))));

	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) | (1 << RESET_LED_PIN)));
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));

	// Set GPIOH pin 7 as output pin for WEP WPA led
	RTL_W32(RTL_GPIO_PEFGHDIR, (RTL_R32(RTL_GPIO_PEFGHDIR) | (1 << WEP_WPA_LED_PIN)));
	// Set GPIOH pin 8 as output pin for MAC control led
	RTL_W32(RTL_GPIO_PEFGHDIR, (RTL_R32(RTL_GPIO_PEFGHDIR) | (1 << MAC_CTL_LED_PIN)));
	// Set GPIOF pin 5 as output pin for BRIDGE REPEATER led
	RTL_W32(RTL_GPIO_PEFGHDIR, (RTL_R32(RTL_GPIO_PEFGHDIR) | (1 << BRIDGE_REPEATER_LED_PIN)));
	
	//turn off WEP WPA led in the beginning
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) | (1 << WEP_WPA_LED_PIN)));
	//turn off MAC control led in the beginning
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) | (1 << MAC_CTL_LED_PIN)));
	//turn off BRIDGE REPEATER led in the beginning
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) | (1 << BRIDGE_REPEATER_LED_PIN)));
}

void wep_wpa_led_on(void)
{
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) & (~(1 << WEP_WPA_LED_PIN))));
}

void wep_wpa_led_off(void)
{
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) | (1 << WEP_WPA_LED_PIN)));
}

void mac_ctl_led_on(void)
{
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) & (~(1 << MAC_CTL_LED_PIN))));
}

void mac_ctl_led_off(void)
{
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) | (1 << MAC_CTL_LED_PIN)));
}

void bridge_repeater_led_on(void)
{
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) & (~(1 << BRIDGE_REPEATER_LED_PIN))));
}

void bridge_repeater_led_off(void)
{
	RTL_W32(RTL_GPIO_PEFGHDATA, (RTL_R32(RTL_GPIO_PEFGHDATA) | (1 << BRIDGE_REPEATER_LED_PIN)));
}

void system_led_on(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));
}

//void system_led_off(void)
//{
//	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << RESET_LED_PIN)));
//}

void lan_led_on(void)
{
	printk("lan_led_on!!!\n");
	RTL_W32(LEDCR0, (RTL_R32(LEDCR0) |(0x10009F)));
	RTL_W32(LEDCR1, 0x0);
	RTL_W32(LEDBCR, 0x0);
}

void lan_led_off(void)
{
	printk("lan_led_off\n");
	RTL_W32(LEDCR0, (RTL_R32(LEDCR0) |(0x10009F)));
	RTL_W32(LEDCR1, 0x1FFFF8);
	RTL_W32(LEDBCR, 0x0);
}

#endif

#ifdef CONFIG_RTL819X_ONOFF_TEST
void system_led_on(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));
}

void system_led_off(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << RESET_LED_PIN)));
}
#endif

void autoconfig_gpio_blink(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
#if  defined(CONFIG_RTL8196B_KLD)	|| defined(CONFIG_RTL8196C_KLD)
	// need  fast blink (0.25 sec on and 0.25 sec off)
	AutoCfg_LED_Blink = 3;
	AutoCfg_LED_Toggle = 1;
	mod_timer(&wps_QuickBlink_timer, jiffies + 25);
#else
	AutoCfg_LED_Blink = 1;
	AutoCfg_LED_Toggle = 1;
#endif
#if defined(CONFIG_RTL865X_SC)
	ResetToAutoCfgBtn = 0;
#endif
}


#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
void autoconfig_gpio_blink_quick(void)
{
	//Brad Modify for tr 11n
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));

	AutoCfg_LED_Blink = 2;
	AutoCfg_LED_Toggle = 1;
}
#endif

#ifdef CONFIG_RTL865X_AC
void autoconfig_gpio_blink_quick(void)
{
	//Brad Modify for tr 11n
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));

	AutoCfg_LED_Blink = 2;
	AutoCfg_LED_Toggle = 1;
}
#endif
#endif // AUTO_CONFIG


#if 0
static int reset_flash_default(void *data)
{
	char *argv[3], *envp[1] = {NULL};
	int i = 0;
	int reset_default=(int)data;

	argv[i++] = "/bin/flash";
	argv[i++] = "reset";
	argv[i] = NULL;

	if(reset_default)
	{
		printk("Going to Reload Default\n");
		if (call_usermodehelper(argv[0], argv, envp,1))
			printk("failed to Reset to default\n");
	}
	machine_restart(0);
	return 0;
}
#endif


static void rtl_gpio_timer(unsigned long data)
{
	unsigned int pressed=1;

#if (defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)) && defined(AUTO_CONFIG)
	static int cnt=0;

	if (AutoCfg_LED_Blink == 2)
	{
		if (AutoCfg_LED_Toggle) {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
		}
		else {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
		}
		AutoCfg_LED_Toggle = AutoCfg_LED_Toggle? 0 : 1;
	}

	if (cnt++%4)
		goto ret_timer;
#endif

#if ( defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)) && defined(AUTO_CONFIG)
	static int cnt=0;

	if (AutoCfg_LED_Blink == 2)
	{
		if (AutoCfg_LED_Toggle) {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
		}
		else {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
		}
		AutoCfg_LED_Toggle = AutoCfg_LED_Toggle? 0 : 1;
	}

	if (cnt++%4)
		goto ret_timer;
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if(test_mode_flag == '1')
	{
		if(RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_BTN_PIN))
		{
			wps_btn_flag = 0;
		}
		else
		{
			wps_btn_flag = 1;
		}
	}
#endif	
#ifdef CONFIG_RTL_EB8186
	if (RTL_R32(RTL_GPIO_PABDATA) & (1 << RESET_BTN_PIN))
#endif
#if defined(CONFIG_RTL865X)
	if (RTL_R32(PEFGH_DAT) & (1 << RESET_BTN_PIN))
#elif defined(CONFIG_RTL8196B)	
	if (RTL_R32(PABCD_DAT) & (1 << RESET_BTN_PIN))
#endif
	{
		pressed = 0;
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)			
		rst_btn_flag = 0;
#endif
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL865X_CMO) && !defined(CONFIG_RTL8196C_KLD) && !defined(CONFIG_RTL8196C_EC)
		//turn off LED0
		#ifndef CONFIG_RTL8196B_GW_MP
		RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | ((1 << RESET_LED_PIN))));
		#endif
#endif
	}
	else
	{
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		rst_btn_flag = 1;
#endif
		DPRINTK("Key pressed %d!\n", probe_counter+1);
	}

	if (probe_state == PROBE_NULL)
	{
		if (pressed)
		{
			probe_state = PROBE_ACTIVE;
			probe_counter++;
		}
		else
			probe_counter = 0;
	}
	else if (probe_state == PROBE_ACTIVE)
	{
		if (pressed)
		{
			probe_counter++;
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
			if ((probe_counter >=1 ) && (probe_counter <=PROBE_TIME))
#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
			if ((probe_counter >=1 ) && (probe_counter <=PROBE_TIME))
#else
			if ((probe_counter >=2 ) && (probe_counter <=PROBE_TIME))
#endif
			{
#if 0
//#ifndef CONFIG_RTL_EB8186
				led0enable=1;
				// turn on LED0
				RTL_W8(RTL_WLAN_CR9346, 0xc0);
				RTL_W8(RTL_WLAN_CONFIG0, 0x10);
				RTL_W8(RTL_WLAN_PSR,(RTL_R8(RTL_WLAN_PSR)&0xEF));
#else
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL8196C_KLD) && !defined(CONFIG_RTL8196C_EC)
				DPRINTK("2-5 turn on led\n");
				//turn on LED0
				RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));
#endif
#endif
			}
			else if (probe_counter >= PROBE_TIME)
			{
#if 0
//#ifndef CONFIG_RTL_EB8186
				led0enable=1;
				RTL_W8(RTL_WLAN_CR9346, 0xc0);
				RTL_W8(RTL_WLAN_CONFIG0, 0x10);
				// sparkling LED0
				if (probe_counter & 1)
					RTL_W8(RTL_WLAN_PSR, (RTL_R8(RTL_WLAN_PSR)|0x10));
				else
					RTL_W8(RTL_WLAN_PSR, (RTL_R8(RTL_WLAN_PSR)&0xEF));
#else
				// sparkling LED0
				DPRINTK(">5 \n");

#ifdef CONFIG_RTL8186_KB
				disable_power_led_blink=0;
#elif !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL8196C_KLD) && !defined(CONFIG_RTL8196C_EC)
				if (probe_counter & 1)
					RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | ((1 << RESET_LED_PIN))));
				else
					RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RESET_LED_PIN))));
#endif // CONFIG_RTL8186_KB
#endif // CONFIG_RTL_EB8186
			}
		}
		else
		{
			#if defined(CONFIG_RTL865X_SC)
			if (probe_counter < 5)
			#else
			if (probe_counter < 2)
			#endif
			{
				probe_state = PROBE_NULL;
				probe_counter = 0;
				DPRINTK("<2 \n");
				#if defined(CONFIG_RTL865X_SC)
				ResetToAutoCfgBtn = 1;
				#endif
			}
			else if (probe_counter >= PROBE_TIME)
			{
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)		
				if(test_mode_flag == '1')
				{					
				}
				else
				{
#endif				
#if 0
//#ifndef CONFIG_RTL_EB8186
				led0enable=1;
#endif

#ifdef CONFIG_RTL8186_KB
				disable_power_led_blink=0;
#endif

				//reload default
				default_flag = '1';
#ifdef CONFIG_RTL865X_KLD
			if(kld_model==1){
				ps_led_flag = '0'; 
				ps_led_process(); 
			}
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
			if(kld_model==1){
				ps_led_flag = '0'; 
				ps_led_process(); 
			}
#endif

				//kernel_thread(reset_flash_default, (void *)1, SIGCHLD);
				return;
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)			
				}
#endif	

			}
			else
			{
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)			
				if(test_mode_flag == '1')
				{					
				}
				else
				{
#endif	
#if 0
//#ifndef CONFIG_RTL_EB8186
				RTL_W8(RTL_WLAN_PSR , (RTL_R8(RTL_WLAN_PSR)|0x10));
				RTL_W8(RTL_WLAN_CONFIG0, (RTL_R8(RTL_WLAN_CONFIG0)&(~0x10)));
#endif
				DPRINTK("2-5 reset\n");
#ifdef CONFIG_RTL865X_KLD
			if(kld_model==1){
				ps_led_flag = '0';//brad
				ps_led_process();//brad
			}
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
			if(kld_model==1){
				ps_led_flag = '0';//brad
				ps_led_process();//brad
			}
#endif

				kill_proc(1,SIGTERM,1);
				//kernel_thread(reset_flash_default, 0, SIGCHLD);
				return;
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)			
				}
#endif						
			}
		}
	}

#ifdef AUTO_CONFIG
	if (AutoCfg_LED_Blink==1)
	{
		if (AutoCfg_LED_Toggle) {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
		}
		else {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
		}
		AutoCfg_LED_Toggle = AutoCfg_LED_Toggle? 0 : 1;
	}
#endif

#if defined(DETECT_ESD)
	extern int is_fault;
	//panic_printk("\r\n RTL_R32(0xBB804114)=[0x%x],__[%s-%u]",RTL_R32(0xBB804114),__FILE__,__LINE__);		
	if(RTL_R32(0xBB804114) & (0x01))
	{
		phy_ready = 1;
	}
	else
	{
		if(phy_ready == 1)
			is_fault=1;
	}

#endif

#if (defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)) && defined(AUTO_CONFIG)
ret_timer:
	mod_timer(&probe_timer, jiffies + 25);
#elif defined(CONFIG_RTL865X_AC) && defined(AUTO_CONFIG)
ret_timer:
	mod_timer(&probe_timer, jiffies + 25);
#elif defined(CONFIG_RTL865X_KLD) && defined(AUTO_CONFIG)
ret_timer:
	mod_timer(&probe_timer, jiffies + 25);
#elif (defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)) && defined(AUTO_CONFIG)
ret_timer:
	mod_timer(&probe_timer, jiffies + 25);	
#else
	mod_timer(&probe_timer, jiffies + 100);
#endif
}

#ifdef AUTO_CONFIG
#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(DET_WPS_SPEC) || defined(CONFIG_RTL8196C_KLD)
static void gpio_interrupt_isr(int irq, void *dev_instance, struct pt_regs *regs)
{
	wps_button_push = 1;   	  	
  	RTL_W32(PABCD_ISR, RTL_R32(PABCD_ISR)); 	
}
#endif

static int read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (wps_button_push) {
		flag = '1';
		wps_button_push = 0;		
	}
	else
		flag = '0';
		
#elif defined(DET_WPS_SPEC)
// 2009-0414		
	if (wps_button_push) {
		flag = '1';
		wps_button_push = 0;		
	}
	else{
		if (RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_BTN_PIN)){
			flag = '0';
		}else{
			//panic_printk("wps button be held \n");
			flag = '1';
		}

	}
// 2009-0414		
#else
#if defined(CONFIG_RTL865X_SC)
	if ((RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_BTN_PIN)) && ResetToAutoCfgBtn == 0)
#else
	if (RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_BTN_PIN))
#endif
		flag = '0';
	else {
		#ifdef CONFIG_RTL865X_CMO
		printk("!!read_proc AUTOCFG_BTN_PIN 1\n");
		#endif
		flag = '1';
	}
#endif // CONFIG_RTL865X_KLD		
		
	len = sprintf(page, "%c\n", flag);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_CMO) || defined(CONFIG_RTL8196C_EC)
static int read_wps_led_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_LED_PIN))
		flag = '0';
	else
		flag = '1';
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#endif

#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
static int read_wps_led_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_LED_PIN))
		flag = '0';
	else
		flag = '1';
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
static int read_rst_btn_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (rst_btn_flag == 0)
		flag = '0';
	else
		flag = '1';
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

static int read_wps_btn_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (wps_btn_flag == 0)
		flag = '0';
	else
		flag = '1';
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#endif
#endif

#ifdef CONFIG_RTL_KERNEL_MIPS16_CHAR
__NOMIPS16
#endif 
static int write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char flag[20];
//Brad add for update flash check 20080711
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL8196C_KLD) && !defined(CONFIG_RTL8196C_EC)
	char start_count[10], wait[10];
#endif
	if (count < 2)
		return -EFAULT;

	DPRINTK("file: %08x, buffer: %s, count: %lu, data: %08x\n",
		(unsigned int)file, buffer, count, (unsigned int)data);

	if (buffer && !copy_from_user(&flag, buffer, 1)) {
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)	
		if(test_mode_flag == '1' && flag[0] != '0' && flag[0] != '9' )
		{
			return count;
		}
#endif		
		if (flag[0] == 'E') {
			autoconfig_gpio_init();
			#ifdef CONFIG_RTL865X_CMO
			extra_led_gpio_init();
			#endif
		}
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		else if (flag[0] == '9' && test_mode_flag == '1')
			autoconfig_gpio_on();
#endif		
		else if (flag[0] == '0')
			autoconfig_gpio_off();
		else if (flag[0] == '1')
			autoconfig_gpio_on();
		else if (flag[0] == '2')
			autoconfig_gpio_blink();
#ifdef CONFIG_RTL8196B_GW_MP
		else if (flag[0] == '9') // disable system led
                {
			all_led_on();
		}
#endif	

#ifdef CONFIG_RTL865X_CMO
		else if (flag[0] == '3')
			wep_wpa_led_on();
		else if (flag[0] == '5')
			wep_wpa_led_off();
		else if (flag[0] == '6')
			mac_ctl_led_on();
		else if (flag[0] == '7')
			mac_ctl_led_off();
		else if (flag[0] == '8')
			bridge_repeater_led_on();
		else if (flag[0] == '9')
			bridge_repeater_led_off();
		else if (flag[0] == 'A')
			system_led_on();
//		else if (flag[0] == 'B')
//			system_led_off();
		else if (flag[0] == 'C')
			lan_led_on();
		else if (flag[0] == 'D')
			lan_led_off();
		else if (flag[0] == 'Z')
			printk("gpio test test\n");
//		else if (flag[0] == '9')
//			system_led_blink = 2;
#endif

#ifdef CONFIG_RTL819X_ONOFF_TEST
		else if (flag[0] == 'A')
			system_led_on();
		else if (flag[0] == 'B')
			system_led_off();
#endif

//Brad add for update flash check 20080711
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL8196C_KLD) && !defined(CONFIG_RTL8196C_EC)
		else if (flag[0] == '4'){
			start_count_time= 1;
			sscanf(buffer, "%s %s", start_count, wait);
			Reboot_Wait = (simple_strtol(wait,NULL,0))*100;
		}
#endif
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
		else if (flag[0] == '3') // update flash in special case
			flash_write_flag = 0x8000;
		else if (flag[0] == '4') // enable system led blinking
			system_led_blink = 1;
		else if (flag[0] == '5') // disable system led blinking
			system_led_blink = 0;
		else if (flag[0] == '6')
			autoconfig_gpio_blink_quick();
		else if (flag[0] == '7') // disable system led
			system_led_blink = 2;
#if 0 //Brad disable, since the check is not precise for system status			
		else if(flag[0] =='8'){ //update keep alive flag
			//printk("update keep alive flag\n");
			sys_keep_alive++;
		}
#endif
#endif

#ifdef CONFIG_RTL865X_AC
		else if (flag[0] == '3') // update flash in special case
			flash_write_flag = 0x8000;
		else if (flag[0] == '4') // enable system led blinking
			system_led_blink = 1;
		else if (flag[0] == '5') // disable system led blinking
			system_led_blink = 0;
		else if (flag[0] == '6')
			autoconfig_gpio_blink_quick();
		else if (flag[0] == '7') // disable system led
			system_led_blink = 2;
#if 0 //Brad disable, since the check is not precise for system status
		else if(flag[0] =='8'){ //update keep alive flag
			//printk("update keep alive flag\n");
			sys_keep_alive++;
		}
#endif
#endif
#ifdef CONFIG_RTL865X_KLD
		else if (flag[0] == '3') // update flash in special case
			flash_write_flag = 0x8000;
		else if (flag[0] == '4') // enable system led blinking
			system_led_blink = 1;
		else if (flag[0] == '5') // disable system led blinking
			system_led_blink = 0;
//		else if (flag[0] == '6')
//			autoconfig_gpio_blink_quick();
		else if (flag[0] == '7') // disable system led
			system_led_blink = 2;
#if 0 //Brad disable, since the check is not precise for system status
		else if(flag[0] =='8'){ //update keep alive flag
			//printk("update keep alive flag\n");
			sys_keep_alive++;
		}
#endif
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		else if (flag[0] == '3') // update flash in special case
			flash_write_flag = 0x8000;
		else if (flag[0] == '4') // enable system led blinking
			system_led_blink = 1;
		else if (flag[0] == '5') // disable system led blinking
			system_led_blink = 0;
//		else if (flag[0] == '6')
//			autoconfig_gpio_blink_quick();
		else if (flag[0] == '7') // disable system led
			system_led_blink = 2;
#if 0 //Brad disable, since the check is not precise for system status
		else if(flag[0] =='8'){ //update keep alive flag
			//printk("update keep alive flag\n");
			sys_keep_alive++;
		}
#endif
#endif
#ifdef CONFIG_RTL8186_KB
		else if (flag[0] == '3')
			disable_power_led_blink=1;
		else if (flag[0] == '4')
			disable_power_led_blink=0;
#ifdef CONFIG_SQUASHFS
		else if (flag[0] == '5') {
			disable_power_led_blink=3; // set power-led off
#if 0
			extern int init_squashfs_fs(void);
			extern void exit_squashfs_fs(void);
			exit_squashfs_fs();
			init_squashfs_fs();
#endif
		}
#endif
#endif
		else
			{}

		return count;
	}
	else
		return -EFAULT;
}
#endif // AUTO_CONFIG

static int default_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%c\n", default_flag);
	if (len <= off+count) *eof = 1;
	  *start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	  return len;
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_CHAR
__NOMIPS16
#endif 
static int default_write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&default_flag, buffer, 1)) {
		return count;
	}
	return -EFAULT;
}

#ifdef READ_RF_SWITCH_GPIO
static int rf_switch_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RTL_R32(RTL_GPIO_PABDATA) & (0x00080000)){
		flag = '1';
	}else{
		flag = '0';
	}
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#endif

#ifdef CONFIG_POCKET_ROUTER_SUPPORT
static int write_pocketAP_hw_set_flag_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	unsigned int reg_cnr, reg_dir;

	if (count != 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&pocketAP_hw_set_flag, buffer, 1)) {
		
	}
	return -EFAULT;
}

static int read_pocketAP_hw_set_flag_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	
	len = sprintf(page, "%c\n", pocketAP_hw_set_flag);
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;

//panic_printk("\r\n __[%s-%u]",__FILE__,__LINE__);	
	return len;

}



static int read_ap_client_rou_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;
	int gpioA2_flag,gpioB3_flag;
	
	
	if(ap_cli_rou_state == 2)
	{
		len = sprintf(page, "%c\n", '2'); // AP
	}
	else if(ap_cli_rou_state == 1)
	{
		len = sprintf(page, "%c\n", '1'); // Client
	}
	else if(ap_cli_rou_state == 3)
	{
		len = sprintf(page, "%c\n", '3'); // Router
	}
	else
	{
		len = sprintf(page, "%c\n", '0'); 
	}
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;

//panic_printk("\r\n __[%s-%u]",__FILE__,__LINE__);	
	return len;
}

static int read_dc_pwr_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;
	int pluged_state=0;
//panic_printk("\r\n 0x%x__[%s-%u]",RTL_R32(RTL_GPIO_PABDATA),__FILE__,__LINE__);		

	pluged_state = get_dc_pwr_plugged_state();
	if(pluged_state == 1)
	{
		len = sprintf(page, "%c\n", '1');
	}
	else if(pluged_state == 2)
	{
		len = sprintf(page, "%c\n", '2');
	}
	else
	{
		len = sprintf(page, "%c\n", '0');
	}		

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;

//panic_printk("\r\n len=[%u]__[%s-%u]",len,__FILE__,__LINE__);	
	return len;
}

static int read_dc_pwr_plugged_flag_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	
	len = sprintf(page, "%c\n", dc_pwr_plugged_flag);
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;

//panic_printk("\r\n __[%s-%u]",__FILE__,__LINE__);
	dc_pwr_plugged_flag = '0';
	return len;

}
static int read_EnablePHYIf_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

//panic_printk("\r\n 0x%x__[%s-%u]",RTL_R32(RTL_GPIO_PABDATA),__FILE__,__LINE__);		

	if(RTL_R32(0xBB804114) & (0x01))
	{
		flag = '1';
	}
	else
	{
		flag = '0';
	}
		
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;

//panic_printk("\r\n len=[%u]__[%s-%u]",len,__FILE__,__LINE__);	
	return len;
}

static int get_pocketAP_ap_cli_rou_state()
{
	int gpioA2_flag,gpioB3_flag;
	
	if(RTL_R32(RTL_GPIO_PABDATA) & (RTL_GPIO_DAT_GPIOA2))
	{
		gpioA2_flag = 1;
	}
	else
	{
		gpioA2_flag = 0;
	}

	if(RTL_R32(RTL_GPIO_PABDATA) & (RTL_GPIO_DAT_GPIOB3))
	{
		gpioB3_flag = 1;
	}
	else
	{
		gpioB3_flag = 0;
	}

	return ((1<<gpioA2_flag)|gpioB3_flag);
}

static int get_dc_pwr_plugged_state()
{
	
	if(RTL_R32(RTL_GPIO_PABDATA) & (RTL_GPIO_DAT_GPIOC0))
	{
		return 1; //plugged
	}
	else
	{
		return 2; //unplugged
	}

}

static int check_EnablePHYIf()
{
	if(RTL_R32(0xBB804114) & (0x01))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

static void pocket_ap_timer_func(unsigned long data)
{
//panic_printk("\r\n __[%s-%u]",__FILE__,__LINE__);

	if(ap_cli_rou_idx >= 1)
		ap_cli_rou_idx = 0;
	else
		ap_cli_rou_idx++;

	ap_cli_rou_time_state[ap_cli_rou_idx]=get_pocketAP_ap_cli_rou_state();
	dc_pwr_plugged_time_state = get_dc_pwr_plugged_state();

	if(ap_cli_rou_time_state[0] == ap_cli_rou_time_state[1] )
	{
		if(ap_cli_rou_state != ap_cli_rou_time_state[0])
		{
			ap_cli_rou_state = ap_cli_rou_time_state[0];
			pocketAP_hw_set_flag = '0';
		}
	}

	if(dc_pwr_plugged_state == 0)
	{
		dc_pwr_plugged_state = dc_pwr_plugged_time_state;
	}
	else if(dc_pwr_plugged_state != dc_pwr_plugged_time_state)
	{
		dc_pwr_plugged_state = dc_pwr_plugged_time_state;
		dc_pwr_plugged_flag = '1';
	}
	
//B8b00728 & 0x1F 0x11:L0 0x14:L1  
//panic_printk("\r\n [%d-%d-%d-%d],__[%s-%u]",ap_cli_rou_time_state[0],ap_cli_rou_time_state[1],ap_cli_rou_state,__FILE__,__LINE__);		

//panic_printk("\r\n [0x%x]",RTL_R32(0xB8b00728) & (0x1F));
	pwr_saving_state=(RTL_R32(0xB8b00728) & (0x1F));
//panic_printk("\r\n pwr_saving_state = [0x%x]",pwr_saving_state);

	if(pwr_saving_state == 0x14) // L1 state, in low speed
	{
		if (pwr_saving_led_toggle < 2) {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (RTL_GPIO_DAT_GPIOB2)));
			pwr_saving_led_toggle++;
		}
		else if (pwr_saving_led_toggle < 4){
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(RTL_GPIO_DAT_GPIOB2))));
			pwr_saving_led_toggle++;
			if(pwr_saving_led_toggle == 4)
				pwr_saving_led_toggle = 0;
		}
		else
		{
			pwr_saving_led_toggle = 0;
		}
	}
	else // L0 state, always on
	{
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(RTL_GPIO_DAT_GPIOB2))));
	}


mod_timer(&pocket_ap_timer, jiffies + 50);
}
#endif

#ifdef CONFIG_RTL865X_KLD
static void ps_led_process(void)
{
	switch (ps_led_flag)
	{
	case '0':	// solid orange
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
		break;
	case '1':	// green flash once then solid green
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << PS_LED_ORANGE_PIN)));
		ps_green_toggle = 1;
		mod_timer(&ps_led_timer, jiffies + 50);
		break;
	case '2':	// orange in low speed
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
		ps_orange_toggle = 1;
		mod_timer(&ps_led_timer, jiffies + 100);
		break;
	default:
		break;
	}
}

static void inet_led_process(void)
{
	switch (inet_led_flag)
	{
	case '0':	// all off
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_GREEN_PIN)));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_ORANGE_PIN)));
		break;
	case '1':	// solid green
	case '5':
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << INET_LED_GREEN_PIN))));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_ORANGE_PIN)));
		break;
	case '2':	// solid orange
	case '4':
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_GREEN_PIN)));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
		break;
	case '3':	// green in high speed
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << INET_LED_GREEN_PIN))));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_ORANGE_PIN)));
		inet_green_toggle = 1;
		mod_timer(&inet_led_timer, jiffies + 25);
		break;
	case '6':	// orange in low speed
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_GREEN_PIN)));
		RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
		inet_orange_toggle = 1;
		mod_timer(&inet_led_timer, jiffies + 100);
		break;
	default:
		break;
	}
}

static int write_ps_led_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	unsigned int reg_cnr, reg_dir;

	if (count != 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&ps_led_flag, buffer, 1)) {
		//printk("ps led flag '%c'\n", ps_led_flag);
		if(ps_led_flag=='E'){
			kld_model = 0;
			ps_led_gpio_init();
		}else if(ps_led_flag=='F'){
			kld_model = 1;
			ps_led_gpio_init();
		}else
			ps_led_process();
		return count;
	}
	return -EFAULT;
}

static int write_inet_led_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	unsigned int reg_cnr, reg_dir;

	if (count != 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&inet_led_flag, buffer, 1)) {
		//printk("inet led flag '%c'\n", inet_led_flag);
		inet_led_process();
		return count;
	}
	return -EFAULT;
}

static void ps_led_timer_func(unsigned long data)
{
	switch (ps_led_flag)
	{
	case '1':	// green flash once then solid green
		if (ps_green_toggle) {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
			ps_green_toggle = 0;
			mod_timer(&ps_led_timer, jiffies + 50);
		}
		else
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));
		break;
	case '2':	// orange in low speed
		if (ps_orange_toggle) {
			RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << PS_LED_ORANGE_PIN)));
			ps_orange_toggle = 0;
			mod_timer(&ps_led_timer, jiffies + 200);
		}
		else {
			RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
			ps_orange_toggle = 1;
			mod_timer(&ps_led_timer, jiffies + 100);
		}
		break;
	default:
		break;
	}
}

static void inet_led_timer_func(unsigned long data)
{
	switch (inet_led_flag)
	{
	case '3':	// green in high speed
		if (inet_green_toggle) {
			RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_GREEN_PIN)));
			inet_green_toggle = 0;
		}
		else {
			RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << INET_LED_GREEN_PIN))));
			inet_green_toggle = 1;
		}
		mod_timer(&inet_led_timer, jiffies + 25);
		break;
	case '6':	// orange in low speed
		if (inet_orange_toggle) {
			RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) | (1 << INET_LED_ORANGE_PIN)));
			inet_orange_toggle = 0;
			mod_timer(&inet_led_timer, jiffies + 200);
		}
		else {
			RTL_W32(PEFGH_DAT, (RTL_R32(PEFGH_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
			inet_orange_toggle = 1;
			mod_timer(&inet_led_timer, jiffies + 100);
		}
		break;
		default:
			break;
	}
}
#endif //CONFIG_RTL865X_KLD

#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
static void ps_led_process(void)
{
	switch (ps_led_flag)
	{
	case '0':	// solid orange
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
#if !defined(CONFIG_RTL8196C_KLD)		
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
#endif
		break;
	case '1':	// green flash once then solid green
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));
#if !defined(CONFIG_RTL8196C_KLD)		
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_ORANGE_PIN)));
#endif
		ps_green_toggle = 1;
		mod_timer(&ps_led_timer, jiffies + 50);
		break;
	case '2':	// orange in low speed
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
#if !defined(CONFIG_RTL8196C_KLD)		
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
#endif
		ps_orange_toggle = 1;
		mod_timer(&ps_led_timer, jiffies + 100);
		break;
	case '3':	// solid green
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));
#if !defined(CONFIG_RTL8196C_KLD)		
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_ORANGE_PIN)));
#endif
		break;
	case '4':	// all turn off
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_ORANGE_PIN)));
#endif
		break;
	default:
		break;
	}
}

static void inet_led_process(void)
{
	switch (inet_led_flag)
	{
	case '0':	// all off
	case '5':
		if(test_mode_flag == '1')
			break;
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
#endif
		break;
	case '1':	// solid green
		if(test_mode_flag == '1')
			break;
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
#endif
		break;

	case '2':	// solid orange
	case '4':
		if(test_mode_flag == '1')
			break;
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));
#if !defined(CONFIG_RTL8196C_KLD)		
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
#endif
		break;


	case '3':	// green in high speed
		if(test_mode_flag == '1')
			break;
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
#endif
		inet_green_toggle = 1;
		mod_timer(&inet_led_timer, jiffies + 25);
		break;
		
	case '6':	// orange in low speed
		if(test_mode_flag == '1')
			break;
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));	
		
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
		inet_orange_toggle = 1;
		mod_timer(&inet_led_timer, jiffies + 100);
#endif
		break;
	case '7':	// all off
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));
		
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
#endif
		break;
		
	case '8':	// solid green
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
		
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
#endif
		break;
		
	case '9':	// solid orange
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));
		
#if !defined(CONFIG_RTL8196C_KLD)
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
#endif
		break;
	default:
		break;
	}
}

static int write_test_mode_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	unsigned int reg_cnr, reg_dir;

	if (count != 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&test_mode_flag, buffer, 1)) {
		
	}
	return -EFAULT;
}
static int write_ps_led_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	unsigned int reg_cnr, reg_dir;

	if (count != 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&ps_led_flag, buffer, 1)) {
		//printk("ps led flag '%c'\n", ps_led_flag);
		if(ps_led_flag=='E'){
			kld_model = 0;
			ps_led_gpio_init();
		}else if(ps_led_flag=='F'){
			kld_model = 1;
			ps_led_gpio_init();
		}else
			ps_led_process();
		return count;
	}
	return -EFAULT;
}

static int write_inet_led_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	unsigned int reg_cnr, reg_dir;

	if (count != 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&inet_led_flag, buffer, 1)) {
		//panic_printk("inet led flag '%c'\n", inet_led_flag);
		inet_led_process();
		return count;
	}
	return -EFAULT;
}

static void ps_led_timer_func(unsigned long data)
{
	switch (ps_led_flag)
	{
	case '1':	// green flash once then solid green
		if (ps_green_toggle) {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
			ps_green_toggle = 0;
			mod_timer(&ps_led_timer, jiffies + 50);
		}
		else
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));
		break;
#if !defined(CONFIG_RTL8196C_KLD)		
	case '2':	// orange in low speed
		if (ps_orange_toggle) {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_ORANGE_PIN)));
			ps_orange_toggle = 0;
			mod_timer(&ps_led_timer, jiffies + 200);
		}
		else {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
			ps_orange_toggle = 1;
			mod_timer(&ps_led_timer, jiffies + 100);
		}
		break;
#endif
	default:
		break;
	}
}

static void wps_QuickBlink_func(unsigned long data)
{

	if(AutoCfg_LED_Blink == 3){
		if (AutoCfg_LED_Toggle) {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));		
			AutoCfg_LED_Toggle = 0;
		}
		else {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));				
			AutoCfg_LED_Toggle = 1;
		}
		mod_timer(&wps_QuickBlink_timer, jiffies + 25);	
	}
}
static void inet_led_timer_func(unsigned long data)
{
	switch (inet_led_flag)
	{
	case '3':	// green in high speed
		if (inet_green_toggle) {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_GREEN_PIN)));
			inet_green_toggle = 0;
		}
		else {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
			inet_green_toggle = 1;
		}
		mod_timer(&inet_led_timer, jiffies + 25);
		break;
		
#if !defined(CONFIG_RTL8196C_KLD)		
	case '6':	// orange in low speed
		if (inet_orange_toggle) {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
			inet_orange_toggle = 0;
			mod_timer(&inet_led_timer, jiffies + 200);
		}
		else {
			RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
			inet_orange_toggle = 1;
			mod_timer(&inet_led_timer, jiffies + 100);
		}
		break;
#endif
		default:
			break;
	}
}
#endif //CONFIG_RTL8196B_KLD || defined(CONFIG_RTL8196C_KLD)

int __init rtl_gpio_init(void)
{
	struct proc_dir_entry *res=NULL;

	printk("Realtek GPIO Driver for Flash Reload Default\n");

	// Set GPIOA pin 10(8181)/0(8186) as input pin for reset button
#ifdef CONFIG_RTL_EB8186
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) & (~(1 << RESET_BTN_PIN))));
#endif

#if defined(CONFIG_RTL865X)
	RTL_W32(PEFGH_CNR, (RTL_R32(PEFGH_CNR) & (~(1 << RESET_BTN_PIN))));
	RTL_W32(PEFGH_DIR, (RTL_R32(PEFGH_DIR) & (~(1 << RESET_BTN_PIN))));

#elif defined(CONFIG_RTL8196B)

#ifdef CONFIG_POCKET_ROUTER_SUPPORT

//panic_printk("\r\n 0x%x__[%s-%u]",RTL_R32(RTL_GPIO_MUX),__FILE__,__LINE__);	
	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | (RTL_GPIO_MUX_POCKETAP_DATA)));
//panic_printk("\r\n 0x%x__[%s-%u]",RTL_R32(RTL_GPIO_MUX),__FILE__,__LINE__);	
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(RTL_GPIO_CNR_POCKETAP_DATA))));





	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA2))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOB3))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | ((RTL_GPIO_DIR_GPIOB2))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOC0))));	

#endif	


    	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | (RTL_GPIO_MUX_DATA)));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << RESET_BTN_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(1 << RESET_BTN_PIN))));

#if defined(READ_RF_SWITCH_GPIO)
	RTL_W32(RTL_GPIO_PABCNR, (RTL_R32(RTL_GPIO_PABCNR) & ( ~(1<<RTL_GPIO_WIFI_ONOFF))));
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) & (~(1<<RTL_GPIO_WIFI_ONOFF))));
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_WIFI_ONOFF))));

#endif // #if defined(READ_RF_SWITCH_GPIO)
#endif // #if defined(CONFIG_RTL865X)

#if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL8196C_KLD)
	// Set GPIOA ping 2 as output pin for reset led
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) | ((1 << RESET_LED_PIN))));
#endif

#ifdef AUTO_CONFIG
	res = create_proc_entry("gpio", 0, NULL);
	if (res) {
		res->read_proc = read_proc;
		res->write_proc = write_proc;
	}
	else {
		printk("Realtek GPIO Driver, create proc failed!\n");
	}

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_CMO) || defined(CONFIG_RTL8196C_EC)
	res = create_proc_entry("wps_led", 0, NULL);
	if (res) {
		#ifdef CONFIG_RTL865X_CMO
		printk("create wps led proc!\n");
		#endif
		res->read_proc = read_wps_led_proc;
	}
	else
		printk("create wps led proc failed!\n");
#endif
#ifdef CONFIG_RTL865X_AC
	res = create_proc_entry("wps_led", 0, NULL);
	if (res)
		res->read_proc = read_wps_led_proc;
	else
		printk("create wps led proc failed!\n");
#endif
#ifdef CONFIG_RTL865X_KLD
	res = create_proc_entry("wps_led", 0, NULL);
	if (res)
		res->read_proc = read_wps_led_proc;
	else
		printk("create wps led proc failed!\n");
	
	RTL_R32(PAB_IMR) |= (0x01 << AUTOCFG_BTN_PIN*2); // enable interrupt in falling-edge		
	if (request_irq(GPIO_IRQ_NUM, gpio_interrupt_isr, SA_INTERRUPT, "rtl_gpio", NULL)) {	  
		printk("request_irq(%d) error!\n", GPIO_IRQ_NUM);		
   	}	
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	res = create_proc_entry("wps_led", 0, NULL);
	if (res)
		res->read_proc = read_wps_led_proc;
	else
		printk("create wps led proc failed!\n");
	
#if defined(CONFIG_RTL8196C_KLD)		
	RTL_R32(_GIMR_) |= (0x01 << 9);		// GIMR keith. (=| 9 bit) to enable wps button work in 96c.
#endif

	RTL_R32(PAB_IMR) |= (0x01 << AUTOCFG_BTN_PIN*2); // enable interrupt in falling-edge	
#if !defined(CONFIG_RTL8196C_KLD)			
	if (request_irq(GPIO_IRQ_NUM, gpio_interrupt_isr, SA_INTERRUPT, "rtl_gpio", NULL)) {	  
		printk("request_irq(%d) error!\n", GPIO_IRQ_NUM);		
   	}	
#endif   	
#endif
// 2009-0414		
#if defined(DET_WPS_SPEC)
	RTL_R32(PAB_IMR) |= (0x01 << AUTOCFG_BTN_PIN*2); // enable interrupt in falling-edge		
	if (request_irq(GPIO_IRQ_NUM, gpio_interrupt_isr, SA_INTERRUPT, "rtl_gpio", NULL)) {	  
		printk("gpio request_irq(%d) error!\n", GPIO_IRQ_NUM);		
   	}
#endif
// 2009-0414		
#endif

	res = create_proc_entry("load_default", 0, NULL);
	if (res) {
		res->read_proc = default_read_proc;
		res->write_proc = default_write_proc;
	}

#ifdef READ_RF_SWITCH_GPIO
	res = create_proc_entry("rf_switch", 0, NULL);
	if (res) {
		res->read_proc = rf_switch_read_proc;
		res->write_proc = NULL;
	}
#endif

#ifdef CONFIG_RTL865X_KLD
	res = create_proc_entry("ps_led", 0, NULL);
	if (res)
		res->write_proc = write_ps_led_proc;
	else
		printk("create ps led proc failed!\n");

	res = create_proc_entry("inet_led", 0, NULL);
	if (res)
		res->write_proc = write_inet_led_proc;
	else
		printk("create inet led proc failed!\n");

	init_timer(&ps_led_timer);
	ps_led_timer.data = (unsigned long)NULL;
	ps_led_timer.function = &ps_led_timer_func;

	init_timer(&inet_led_timer);
	inet_led_timer.data = (unsigned long)NULL;
	inet_led_timer.function = &inet_led_timer_func;
#endif

#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	res = create_proc_entry("test_mode", 0, NULL);
	if (res)
		res->write_proc = write_test_mode_proc;
	else
		printk("create test_mode proc failed!\n");

	res = create_proc_entry("rst_btn", 0, NULL);
	if (res)
		res->read_proc = read_rst_btn_proc;
	else
		printk("create read_rst_btn_proc failed!\n");

	res = create_proc_entry("wps_btn", 0, NULL);
	if (res)
		res->read_proc = read_wps_btn_proc;
	else
		printk("create read_wps_btn_proc failed!\n");
	
	
	res = create_proc_entry("ps_led", 0, NULL);
	if (res)
		res->write_proc = write_ps_led_proc;
	else
		printk("create ps led proc failed!\n");

	res = create_proc_entry("inet_led", 0, NULL);
	if (res)
		res->write_proc = write_inet_led_proc;
	else
		printk("create inet led proc failed!\n");

	init_timer(&ps_led_timer);
	ps_led_timer.data = (unsigned long)NULL;
	ps_led_timer.function = &ps_led_timer_func;

	init_timer(&inet_led_timer);
	inet_led_timer.data = (unsigned long)NULL;
	inet_led_timer.function = &inet_led_timer_func;
	init_timer(&wps_QuickBlink_timer);
	wps_QuickBlink_timer.data = (unsigned long)NULL;
	wps_QuickBlink_timer.function = &wps_QuickBlink_func;
#endif

#ifdef CONFIG_POCKET_ROUTER_SUPPORT

	res = create_proc_entry("dc_pwr", 0, NULL);
	if (res)
		res->read_proc = read_dc_pwr_proc;
	else
		printk("create read_dc_pwr_proc failed!\n");

	res = create_proc_entry("dc_pwr_plugged_flag", 0, NULL);
	if (res)
	{
		res->read_proc = read_dc_pwr_plugged_flag_proc;
	}
	else
		printk("create read_pocketAP_hw_set_flag_proc failed!\n");
	
	res = create_proc_entry("ap_client_rou", 0, NULL);
	if (res)
		res->read_proc = read_ap_client_rou_proc;
	else
		printk("create read_ap_client_rou_proc failed!\n");

	res = create_proc_entry("pocketAP_hw_set_flag", 0, NULL);
	if (res)
	{
		res->read_proc = read_pocketAP_hw_set_flag_proc;
		res->write_proc = write_pocketAP_hw_set_flag_proc;
	}
	else
		printk("create read_pocketAP_hw_set_flag_proc failed!\n");

	res = create_proc_entry("phyif", 0, NULL);
	if (res)
		res->read_proc = read_EnablePHYIf_proc;
	else
		printk("create read_EnablePHYIf_proc failed!\n");
				
	init_timer(&pocket_ap_timer);
	pocket_ap_timer.data = (unsigned long)NULL;
	pocket_ap_timer.function = &pocket_ap_timer_func;
	mod_timer(&pocket_ap_timer, jiffies + 100);
#endif

#ifndef CONFIG_RTL819X_ONOFF_TEST
	init_timer(&probe_timer);
	probe_counter = 0;
	probe_state = PROBE_NULL;
	probe_timer.expires = jiffies + 100;
	probe_timer.data = (unsigned long)NULL;
	probe_timer.function = &rtl_gpio_timer;
	mod_timer(&probe_timer, jiffies + 100);
#endif	

#ifdef CONFIG_RTL865X_CMO
	extra_led_gpio_init();
#endif
	return 0;
}


static void __exit rtl_gpio_exit(void)
{
	printk("Unload Realtek GPIO Driver \n");
	del_timer_sync(&probe_timer);
}


module_exit(rtl_gpio_exit);
module_init(rtl_gpio_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");
