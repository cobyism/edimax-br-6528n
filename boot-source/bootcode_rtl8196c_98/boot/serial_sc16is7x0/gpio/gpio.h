/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO__
#define __GPIO__

#ifdef __UBOOT__
#include <config.h>
#endif
#ifndef __BOOTCODE__
#include <linux/config.h>
#endif
#include "voip_types.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#include "gpio_8186.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#include "gpio_8651bv.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#include "gpio_8972.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672)
#include "gpio_8672.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
#include "gpio_8952.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
#include "gpio_8972b.h"
#else
#error "unknown gpio driver"
#endif

#define _GPIO_DEBUG_

#ifdef _GPIO_DEBUG_ 
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#define GPIO_PRINT(level, fmt, args...) do { if (gpio_debug >= level) rtlglue_printf(fmt, ## args); } while (0)
#else
#define GPIO_PRINT(level, fmt, args...) do { if (gpio_debug >= level) printk(fmt, ## args); } while (0)
#endif
#else
#define GPIO_PRINT(fmt, args...)
#endif

/*
 * Every pin of GPIO port can be mapped to a unique ID. All the access to a GPIO pin must use the ID.
 * This macro is used to map the port and pin into the ID.
 */
#define GPIO_ID(port,pin) ((uint32)port<<16|(uint32)pin)

/* This is reversed macro. */
#define GPIO_PORT(id) (id>>16)
#define GPIO_PIN(id) (id&0xffff)

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define RTK_GPIO_INIT(pid, dedicate, dir, interrupt) _rtl8186_initGpioPin(pid, dir, interrupt)
#define RTK_GPIO_GET(pid, pData) _rtl8186_getGpioDataBit(pid, pData) 
#define RTK_GPIO_SET(pid, data) _rtl8186_setGpioDataBit(pid, data)

#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#define RTK_GPIO_INIT(pid, dedicate, dir, interrupt)  _rtl865x_initGpioPin(pid, dedicate, dir, interrupt)
#define RTK_GPIO_GET(pid, pData) _rtl865x_getGpioDataBit(pid, pData) 
#define RTK_GPIO_SET(pid, data) _rtl865x_setGpioDataBit(pid, data)

#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#define RTK_GPIO_INIT(pid, dedicate, dir, interrupt)  _rtl867x_initGpioPin(pid, dir, interrupt)
#define RTK_GPIO_GET(pid, pData) _rtl867x_getGpioDataBit(pid, pData) 
#define RTK_GPIO_SET(pid, data) _rtl867x_setGpioDataBit(pid, data)

#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672)
#define RTK_GPIO_INIT(pid, dedicate, dir, interrupt)  _rtl867x_initGpioPin(pid, dir)
#define RTK_GPIO_GET(pid, pData) _rtl867x_getGpioDataBit(pid, pData) 
#define RTK_GPIO_SET(pid, data) _rtl867x_setGpioDataBit(pid, data)

#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
#define RTK_GPIO_INIT(pid, dedicate, dir, interrupt)  _rtl865xC_initGpioPin(pid, dedicate, dir, interrupt)
#define RTK_GPIO_GET(pid, pData) _rtl865xC_getGpioDataBit(pid, pData) 
#define RTK_GPIO_SET(pid, data) _rtl865xC_setGpioDataBit(pid, data)

#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
#define RTK_GPIO_INIT(pid, dedicate, dir, interrupt)  _rtl8972B_initGpioPin(pid, dedicate, dir, interrupt)
#define RTK_GPIO_GET(pid, pData) _rtl8972B_getGpioDataBit(pid, pData) 
#define RTK_GPIO_SET(pid, data) _rtl8972B_setGpioDataBit(pid, data)

#else
#error "unknown gpio driver"
#endif

extern int gpio_debug;

#endif/*__GPIO__*/
