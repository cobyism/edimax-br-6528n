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

#include "../../../define/C_DEFINE.h" // EDX

#define AUTO_CONFIG

// 2009-0414
	#define READ_RF_SWITCH_GPIO

	#include <asm/rtl865x/rtl865xc_asicregs.h>
	#define RTL_GPIO_PABDIR		PABCD_DIR
	#define RTL_GPIO_PABDATA		PABCD_DAT
	#define RTL_GPIO_PABCNR		PABCD_CNR
	#define AUTOCFG_BTN_PIN		_HW_BUTTON_WPS_ //1
	#define AUTOCFG_LED_PIN		_HW_LED_WPS_    //20
#if 0 // EDX BSP, tommy
	#define RESET_LED_PIN		18
	#define RESET_BTN_PIN		0
	#define RTL_GPIO_MUX 0xB8000030
	#define RTL_GPIO_MUX_DATA 0x0FC00380//for WIFI ON/OFF and GPIO
	#define RTL_GPIO_WIFI_ONOFF	19
#else // EDX BSP, tommy
//	#define RESET_LED_PIN		(RTL_GPIO_WLAN_LED) /* tommy --> Edimax use this pin as WLAN LED */

	#define RESET_BTN_PIN		_HW_BUTTON_RESET_ //0
	#define RTL_GPIO_MUX		0xB8000040
	#ifdef _Logitec_
		#define RTL_GPIO_MUX_DATA	0x003C3C04
	#else
		#define RTL_GPIO_MUX_DATA	0x00300000 //for WIFI ON/OFF and GPIO
	#endif
	#define RTL_GPIO_WIFI_ONOFF	_HW_BUTTON_SWITCH_ //19

	#define RTL_GPIO_WLAN_LED	_HW_LED_WIRELESS_ //18 /* tommy --> Edimax WLAN_LED GPIOC[2] */
	#define RTL_GPIO_POWER_LED	_HW_LED_POWER_ //17 /* tommy --> Edimax POWER LED GPIOC[1] */

#endif

// 2009-0414
#ifdef DET_WPS_SPEC
	#define GPIO_IRQ_NUM		1
#endif
	#define PROBE_TIME		10	// EDX BSP. tommy add

#define PROBE_NULL			0
#define PROBE_ACTIVE			1
#define PROBE_RESET			2
#define PROBE_RELOAD			3
#define RTL_R32(addr)		(*(volatile unsigned long *)(addr))
#define RTL_W32(addr, l)	((*(volatile unsigned long*)(addr)) = (l))
#define RTL_R8(addr)		(*(volatile unsigned char*)(addr))
#define RTL_W8(addr, l)		((*(volatile unsigned char*)(addr)) = (l))

#define  GPIO_DEBUG
#ifdef GPIO_DEBUG
/* note: prints function name for you */
#  define DPRINTK(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define DPRINTK(fmt, args...)
#endif

static struct timer_list probe_timer;
static unsigned int    probe_counter;
static unsigned int    probe_state;

// Start, EDX BSP, tommy
static struct timer_list led_timer;	// tommy add
static struct timer_list rf_switch_timer;	// tommy add

static unsigned int    wlan_counter;	// tommy add
static unsigned int    power_counter;	// tommy add

//static int led_flash[30]={20,10,100,5,5,150,100,5,5,50,20,50,50,20,60,5,20,10,30,10,5,10,50,2,5,5,5,70,10,50}; // tommy add
static int led_flash[30]={2,10,1,70,2,15,1,5,50,5,20,5,20,2,6,5,2,10,30,10,5,10,50,2,5,30,5,17,10,50}; // tommy add
static char wlan_led_flag = '2'; /* tommy add */
static unsigned int wlan_LED_Blink; /* tommy add */
#ifdef _ESD_DETECT_
extern int is_fault;
static char esdFlag = '0'; /* Jeff add */
static int phyReady[5] = {0};
#endif

static unsigned int    rf_probe_counter;
static unsigned int    rf_probe_state;
static char rf_switch_flag = '0'; /* ReHua Modify, 1:push button pressed */
#if	defined _WPS_INDEPEND_
unsigned int wps_pressed = 0;
unsigned int reset_pressed = 0;
#endif
// End, EDX BSP, tommy

static char default_flag='0';
static char wps_flag='0';	// EDX BSP, RexHua
//Brad add for update flash check 20080711
int start_count_time=0;
int Reboot_Wait=0;

unsigned int led0enable;

#ifdef	DET_WPS_SPEC
static int wps_button_push = 0;
#endif

#ifdef AUTO_CONFIG
static unsigned int		AutoCfg_LED_Blink;
static unsigned int		AutoCfg_LED_Toggle;

// Start, EDX BSP, tommy
void wlan_gpio_off(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1<<RTL_GPIO_WLAN_LED)));
	wlan_LED_Blink = 0;
}

void wlan_gpio_on(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_WLAN_LED))));
	wlan_LED_Blink = 0;
}

void wlan_gpio_blink(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_WLAN_LED))));
	wlan_LED_Blink = 1;
}
/* End - EDX BSP, tommy add */

void autoconfig_gpio_init(void)
{
	RTL_W32(PABCD_CNR,(RTL_R32(PABCD_CNR)&(~(1 << AUTOCFG_BTN_PIN))));
	RTL_W32(PABCD_CNR,(RTL_R32(PABCD_CNR)&(~(1 << AUTOCFG_LED_PIN))));

	// Set GPIOA pin 1 as input pin for auto config button
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) & (~(1 << AUTOCFG_BTN_PIN))));

	// Set GPIOA ping 3 as output pin for auto config led
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) | (1 << AUTOCFG_LED_PIN)));

	// turn off auto config led in the beginning
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
}
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


void autoconfig_gpio_blink(void)
{
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
	AutoCfg_LED_Blink = 1;
	AutoCfg_LED_Toggle = 1;
}

#endif // AUTO_CONFIG

// Start, EDX BSP, tommy
static void wlan_led_flash(unsigned long data)
{
	if (wlan_LED_Blink) {
		if ((wlan_counter%2) == 1)	{
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_WLAN_LED))));
		}
		else {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1<<RTL_GPIO_WLAN_LED)));
		}
	}

	wlan_counter++;
	mod_timer(&led_timer, jiffies + led_flash[wlan_counter%30]);
}

static void rf_switch_on_off(unsigned long data)
{
#if 1 // RF ON/OFF SWITCH
	if (RTL_R32(RTL_GPIO_PABDATA) & (1 << RTL_GPIO_WIFI_ONOFF))
		rf_switch_flag = '0';
	else
		rf_switch_flag = '1';
//printk("rf_switch_on_off dbg %d\n", rf_switch_flag);

	mod_timer(&rf_switch_timer, jiffies + 100);

#else // RF ON/OFF Button
	if (RTL_R32(RTL_GPIO_PABDATA) & (1 << RTL_GPIO_WIFI_ONOFF)) {
		rf_switch_pressed = 0;
	}
	else{
		DPRINTK("rf_switch is pressed %d!\n", rf_probe_counter+1);
	}

	if (rf_probe_state == PROBE_NULL) {
		if (rf_switch_pressed) {
			rf_probe_state = PROBE_ACTIVE;
			rf_probe_counter++;
		}
		else
			rf_probe_counter = 0;
	}
	else if (rf_probe_state == PROBE_ACTIVE) {
		if (rf_switch_pressed) {
			rf_probe_counter++;
		}
		else {
			rf_probe_state = PROBE_NULL;

			if ( rf_switch_flag == '0' )
			{
				rf_switch_flag = '1';
				RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) ^ (1<<RTL_GPIO_WLAN_LED)));
				// wlan_gpio_off();		// EDX BSP, RexHua
			}
		}
	}
	mod_timer(&rf_switch_timer, jiffies + 30);
#endif
}
/* End - EDX BSP, tommy add */

static void rtl_gpio_timer(unsigned long data)
{
	unsigned int pressed=1;

#ifdef _ESD_DETECT_
	int port;
	int offset = 0xBB804104;
	for(port = 0; port < 5; port++)
	{
		if(RTL_R32(offset) & (0x01))
			phyReady[port] = 1;
		else
		{
			if(phyReady[port] == 1)
			{
				esdFlag = '1';
				is_fault = 1;
			}
		}
		offset += 4;
	}

#endif

#if	defined _WPS_INDEPEND_
	unsigned int isresetbutton=0;
	if ((RTL_R32(PABCD_DAT) & (1 << RESET_BTN_PIN)) && (RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_BTN_PIN)))	//EDX
#else
	if (RTL_R32(PABCD_DAT) & (1 << RESET_BTN_PIN))
#endif
	{
		pressed = 0;
	}
	else
	{
		DPRINTK("RESET_BTN_PIN  pressed %d!\n", probe_counter+1);
	}
	if (power_counter<=16) {
		if ((power_counter%2) == 1)     {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_POWER_LED))));
		}
		else {
			RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1<<RTL_GPIO_POWER_LED)));
		}
		power_counter++;
	}
	else
		RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_POWER_LED))));
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
			if ((probe_counter >=2 ) && (probe_counter <=PROBE_TIME))
			{
				DPRINTK("2-5 turn on led\n");
				//EDX BSP, RexHua RTL_GPIO_WLAN_LED flash
				if(rf_switch_flag == '0')
				{
					if (probe_counter & 1)
						RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | ((1 << RTL_GPIO_WLAN_LED))));
					else
						RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << RTL_GPIO_WLAN_LED))));
				}
#if	defined _WPS_INDEPEND_
				if ((reset_pressed == 0) && (wps_pressed == 0)) {
					if ((RTL_R32(PABCD_DAT) & (1 << RESET_BTN_PIN)) == 0) {
						reset_pressed = 1;
					}
					if ((RTL_R32(PABCD_DAT) & (1 << AUTOCFG_BTN_PIN)) == 0) {
						wps_pressed = 1;
					}
				}
#endif
			}
			else if (probe_counter >= PROBE_TIME)
			{
				// sparkling LED0
				DPRINTK(">5 \n");
				wlan_gpio_on();		// EDX BSP, RexHua
#if	defined _WPS_INDEPEND_	//Power Led blinding
				if (reset_pressed) {
					if ((probe_counter%2) == 1) {
						RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_POWER_LED))));
					}
					else {
						RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1<<RTL_GPIO_POWER_LED)));
					}
				}
#else
				if ((probe_counter%2) == 1)
					RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_POWER_LED))));
				else
					RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1<<RTL_GPIO_POWER_LED)));
#endif

			}
		}
		else
		{
			if (probe_counter < 2)
			{
				probe_state = PROBE_NULL;
				probe_counter = 0;
				DPRINTK("<2 \n");
#if	defined _WPS_INDEPEND_
				reset_pressed = 0;
				wps_pressed = 0;
#endif
			}
			else if (probe_counter >= PROBE_TIME)
			{
#if	defined _WPS_INDEPEND_
				if (wps_pressed){
					wps_pressed = 0;
					wps_flag = '1'; // EDX
				probe_state = PROBE_NULL; // EDX
				probe_counter = 0;	  // EDX
				}
				if (reset_pressed){
				reset_pressed = 0;
				default_flag='1';
				return;
				}
#else
				//reload default
				default_flag = '1';
				//kernel_thread(reset_flash_default, (void *)1, SIGCHLD);
				return;
#endif

			}
			else
			{
#if	defined _WPS_INDEPEND_
				if (reset_pressed) {
					reset_pressed = 0;
				}
				if (wps_pressed) {
					wps_pressed = 0;
					DPRINTK("2-5 reset\n");
					wps_flag = '1';
					wlan_gpio_on();
				}
				probe_state = PROBE_NULL; // EDX
				probe_counter = 0;	  // EDX
#else
				DPRINTK("2-5 reset\n");
				if (rf_switch_flag == '0') {
					wps_flag = '1'; // EDX BSP, RexHua
					wlan_gpio_on();
				}
				probe_state = PROBE_NULL; // EDX BSP, RexHua
				probe_counter = 0;	  // EDX BSP, RexHua
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

	mod_timer(&probe_timer, jiffies + 100);
}

#ifdef AUTO_CONFIG
static int read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	//if (RTL_R32(RTL_GPIO_PABDATA) & (1 << AUTOCFG_BTN_PIN)) // EDX BSP, tommy removed it for wps

	len = sprintf(page, "%c\n", wps_flag);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_CHAR
__NOMIPS16
#endif
static int write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char flag[20];
        char start_count[10], wait[10];

//Brad add for update flash check 20080711
	if (count < 2)
		return -EFAULT;

	DPRINTK("file: %08x, buffer: %s, count: %lu, data: %08x\n",
		(unsigned int)file, buffer, count, (unsigned int)data);

	if (buffer && !copy_from_user(&flag, buffer, 1)) {
		if (flag[0] == '2') // Call by WPSd
		{
			DPRINTK("write 2, wps_flag=%d\n",wps_flag); // EDX BSP, RexHua
		}
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

static int rf_switch_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	len = sprintf(page, "%c\n", rf_switch_flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
// { EDX BSP, RexHua Add
static int rf_switch_write_proc(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
        if (count < 2)
                return -EFAULT;

        if (buffer && !copy_from_user(&rf_switch_flag, buffer, 1))
		return count;

        return -EFAULT;
}

#if defined(_KREBOOT_) && !defined (_NO_KREBOOT_)
static int reset_but_write_proc(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
	char tmp;
        if (count < 2)
                return -EFAULT;

        if (buffer && !copy_from_user(&tmp, buffer, 1))
	{
		machine_restart(0);
		return count;
	}

        return -EFAULT;
}
#endif

#if defined(_PCI_THREEWAY_SWITCH_) && defined(_BR6428GNL_)
static int pci_threeway_switch_read_proc(char *page, char **start, off_t off,
                                int count, int *eof, void *data)
{
	int len, result=0;
	if ( !(RTL_R32(PABCD_DAT) & (1 << 11)) ) // Converter
		result=2;
	else if ( !(RTL_R32(PABCD_DAT) & (1 << 12)) ) // AP
		result=1;
	else if ( !(RTL_R32(PABCD_DAT) & (1 << 13)) ) // Auto
		result=0;

	len = sprintf(page, "%d\n", result);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
static int pci_threeway_switch_write_proc(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
        return -EFAULT;
}
#endif

static int wps_but_read_proc(char *page, char **start, off_t off,
                                int count, int *eof, void *data)
{
        int len;
        len = sprintf(page, "%c\n", wps_flag);

        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len > count) len = count;
        if (len < 0) len = 0;
        return len;
}
static int wps_but_write_proc(struct file *file, const char *buffer,
                                unsigned long count, void *data)
{
        if (count < 2)
                return -EFAULT;

        if (buffer && !copy_from_user(&wps_flag, buffer, 1))
                return count;

        return -EFAULT;
}


// } EDX BSP, RexHua Add

/* EDX BSP, tommy add */
static int write_wlan_led_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;

	DPRINTK("file: %08x, buffer: %s, count: %lu, data: %08x\n",
		(unsigned int)file, buffer, count, (unsigned int)data);

	if (buffer && !copy_from_user(&wlan_led_flag, buffer, 1)) {
		if (wlan_led_flag == '0') {
			//printk("Running: echo 0 > /proc/wlan_led\n");
			//wlan_gpio_blink();
			wlan_gpio_on();
		}
		else if (wlan_led_flag == '1') {
			//printk("Running: echo 1 > /proc/wlan_led\n");
			wlan_gpio_on();
		}
		else if (wlan_led_flag == '2') {
			//printk("Running: echo 2 > /proc/wlan_led\n");
			//wlan_gpio_on();
			wlan_gpio_blink();
		}
		else if (wlan_led_flag == '3') {
			//printk("Running: echo 3 > /proc/wlan_led\n");
			wlan_gpio_off();
		}
		else
			{}

		return count;
	}
	else
		return -EFAULT;
}
/* end - EDX BSP, tommy add */

/* EDX BSP,  JeffYu add */
#ifdef _WPS_LED_
static int write_wps_led_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;

	DPRINTK("file: %08x, buffer: %s, count: %lu, data: %08x\n",
		(unsigned int)file, buffer, count, (unsigned int)data);

	if (buffer && !copy_from_user(&wlan_led_flag, buffer, 1)) {
		if (wlan_led_flag == '0') {
			//printk("Running: echo 0 > /proc/wlan_led\n");
			//wlan_gpio_blink();
			autoconfig_gpio_on();
		}
		else if (wlan_led_flag == '1') {
			//printk("Running: echo 1 > /proc/wlan_led\n");
			autoconfig_gpio_on();
		}
		else if (wlan_led_flag == '2') {
			//printk("Running: echo 2 > /proc/wlan_led\n");
			//wlan_gpio_on();
			autoconfig_gpio_blink();
		}
		else if (wlan_led_flag == '3') {
			//printk("Running: echo 3 > /proc/wlan_led\n");
			autoconfig_gpio_off();
		}
		else
			{}

		return count;
	}
	else
		return -EFAULT;
}
#endif

#ifdef _ESD_DETECT_
static int write_esdFlag_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;
        if (buffer && !copy_from_user(&esdFlag, buffer, 1))
                return count;

        return -EFAULT;

}

static int read_esdFlag_proc(char *page, char **start, off_t off,
                                int count, int *eof, void *data)
{
        int len;
        len = sprintf(page, "%c\n", esdFlag);

        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len > count) len = count;
        if (len < 0) len = 0;
//is_fault = 1;
        return len;
}
#endif
/* end - EDX BSP, JeffYu add */

int __init rtl_gpio_init(void)
{
	struct proc_dir_entry *res=NULL;

	// Set GPIOA pin 10(8181)/0(8186) as input pin for reset button

// EDX	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | (RTL_GPIO_MUX_DATA)));
#ifdef _Logitec_
			RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | 0x00003000));
#endif //#ifdef _Logitec_ 13:12 GPIOC0
#if defined(_BR6268GN_) 
	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | 0x003000FC));
#endif
#if defined(_BR6428HPN_) 
	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | 0x00303000));
#endif

#if defined(_PCI_THREEWAY_SWITCH_) && defined(_BR6428GNL_)
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << 11))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << 12))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << 13))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << 16))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(1 << 11))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(1 << 12))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(1 << 13))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << 16)));

	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | 0x000030FC));

#endif
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << RESET_BTN_PIN))));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) & (~(1 << RESET_BTN_PIN))));

	/* EDX BSP, tommy add */
	RTL_W32(RTL_GPIO_PABCNR, (RTL_R32(RTL_GPIO_PABCNR) & ( ~(1<<RTL_GPIO_POWER_LED))));
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) | (1 << RTL_GPIO_POWER_LED)));
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_POWER_LED))));

	RTL_W32(RTL_GPIO_PABCNR, (RTL_R32(RTL_GPIO_PABCNR) & ( ~(1<<RTL_GPIO_WLAN_LED))));
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) | (1 << RTL_GPIO_WLAN_LED))); /* dir -> output */
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_WLAN_LED)))); /* turn on */
	/* end - EDX BSP tommy add */
#if defined(READ_RF_SWITCH_GPIO)
	RTL_W32(RTL_GPIO_PABCNR, (RTL_R32(RTL_GPIO_PABCNR) & ( ~(1<<RTL_GPIO_WIFI_ONOFF))));
	RTL_W32(RTL_GPIO_PABDIR, (RTL_R32(RTL_GPIO_PABDIR) & (~(1<<RTL_GPIO_WIFI_ONOFF))));
	RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1<<RTL_GPIO_WIFI_ONOFF))));

#endif // #if defined(READ_RF_SWITCH_GPIO)

#ifdef AUTO_CONFIG
	res = create_proc_entry("gpio", 0, NULL);
	if (res) {
		res->read_proc = read_proc;
		res->write_proc = write_proc;
	}
	else {
		printk("Realtek GPIO Driver, create proc failed!\n");
	}

#endif


#if defined(_PCI_THREEWAY_SWITCH_) && defined(_BR6428GNL_)
	res = create_proc_entry("pci_threeway_switch", 0, NULL);
	if (res) {
		res->read_proc = pci_threeway_switch_read_proc;
		res->write_proc = pci_threeway_switch_write_proc;
	}
#endif

// 2009-0414
#if defined(DET_WPS_SPEC)
	RTL_R32(PAB_IMR) |= (0x01 << AUTOCFG_BTN_PIN*2); // enable interrupt in falling-edge
	if (request_irq(GPIO_IRQ_NUM, gpio_interrupt_isr, SA_INTERRUPT, "rtl_gpio", NULL)) {
		panic_printk("gpio request_irq(%d) error!\n", GPIO_IRQ_NUM);
   	}
#endif
// 2009-0414

	res = create_proc_entry("load_default", 0, NULL);
	if (res) {
		res->read_proc = default_read_proc;
		res->write_proc = default_write_proc;
	}
/* EDX BSP, tommy add */
	res = create_proc_entry("wlan_led", 0, NULL);
	if (res) {
		res->read_proc = NULL;
		res->write_proc = write_wlan_led_proc;
	}
/* end - EDX BSP, tommy add */

	res = create_proc_entry("rf_switch", 0, NULL);
	if (res) {
		res->read_proc = rf_switch_read_proc;
		res->write_proc = rf_switch_write_proc; // EDX BSP, RexHua
	}

#if defined(_KREBOOT_) && !defined (_NO_KREBOOT_)
	res = create_proc_entry("reset_but", 0, NULL);
	if (res) {
		res->write_proc = reset_but_write_proc; // EDX BSP, RexHua
	}
#endif
	res = create_proc_entry("wps_but", 0, NULL);
	if (res) {
		res->read_proc = wps_but_read_proc; 	// EDX BSP, RexHua
		res->write_proc = wps_but_write_proc; 	// EDX BSP, RexHua
	}

#ifdef _WPS_LED_
	autoconfig_gpio_init();
	res = create_proc_entry("wps_led", 0, NULL);
	if (res) {
		res->read_proc = NULL; 	// EDX BSP, JeffYu
		res->write_proc = write_wps_led_proc; 	// EDX BSP, JeffYu
	}
#endif

#ifdef _ESD_DETECT_
	res = create_proc_entry("esd_detect", 0, NULL);
	if (res) {
		res->read_proc = read_esdFlag_proc; 	// EDX BSP, JeffYu
		res->write_proc = write_esdFlag_proc; 	// EDX BSP, JeffYu
	}

#endif
	init_timer(&probe_timer);
	probe_counter = 0;
	probe_state = PROBE_NULL;
	probe_timer.expires = jiffies + 100;
	probe_timer.data = (unsigned long)NULL;
	probe_timer.function = &rtl_gpio_timer;
	mod_timer(&probe_timer, jiffies + 100);

/* EDX BSP, tommy add */
	init_timer(&led_timer);
	wlan_LED_Blink = 1;
	led_timer.expires=jiffies+10;
	led_timer.data =(unsigned long) NULL ;
	led_timer.function = &wlan_led_flash;
	mod_timer(&led_timer, jiffies+10);

	init_timer(&rf_switch_timer);
	rf_probe_counter = 0; // tommy
	rf_probe_state = PROBE_NULL; // tommy
	rf_switch_timer.expires=jiffies+100;
	rf_switch_timer.data =(unsigned long) NULL ;
	rf_switch_timer.function = &rf_switch_on_off;
	mod_timer(&rf_switch_timer, jiffies+100);
/* end - EDX BSP, tommy add */

	return 0;
}

static void __exit rtl_gpio_exit(void)
{
	printk("Unload Realtek GPIO Driver \n");
	del_timer_sync(&probe_timer);
	del_timer_sync(&led_timer); /* EDX BSP, tommy add */
	del_timer_sync(&rf_switch_timer); /* EDX BSP, tommy add */
}

module_exit(rtl_gpio_exit);
module_init(rtl_gpio_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");
