/*
 * Flash mapping for rtl8196 board
 *
 * Copyright (C) 2008 Realtek Corporation
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
//#include <linux/root_dev.h>
#include <linux/mtd/partitions.h>
#include <linux/config.h>
#include <linux/delay.h>
//#include "../../../../config/autoconf.h"


#define WINDOW_ADDR 0xbd000000
#ifdef CONFIG_SPANSION_16M_FLASH	
#define WINDOW_SIZE 0x1000000
#define FLASH_BANK_SIZE 0x400000
#else 
#define WINDOW_SIZE 0x400000
#endif 
#define BUSWIDTH 2
#define WINDOW_SIZE 0x200000
static struct mtd_info *rtl8196_mtd;

__u8 rtl8196_map_read8(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readb(map->map_priv_1 + ofs);
}

__u16 rtl8196_map_read16(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readw(map->map_priv_1 + ofs);
}

__u32 rtl8196_map_read32(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readl(map->map_priv_1 + ofs);
}

void rtl8196_map_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	//printk("enter to %x from  %x len %d\n",to, map->map_priv_1+from , len);
	//11/15/05' hrchen, change the size to fit file systems block size if use different fs
	//4096 for cramfs, 1024 for squashfs
	if (from>0x10000)
	    memcpy(to, map->map_priv_1 + from, (len<=1024)?len:1024);//len);
	else
	    memcpy(to, map->map_priv_1 + from, (len<=4096)?len:4096);//len);
	//printk("enter %s %d\n", __FILE__,__LINE__);

}

void rtl8196_map_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	memcpy_toio(map->map_priv_1 + to, from, len);
}

struct map_info rtl8196_map = {
	name: "Physically mapped flash",
	size: WINDOW_SIZE,
	buswidth: BUSWIDTH,
	//phys: WINDOW_ADDR,
//jasonwang unlock 0227

	read8: rtl8196_map_read8,
	read16: rtl8196_map_read16,
	read32: rtl8196_map_read32,
	copy_from: rtl8196_map_copy_from,
	write8: rtl8196_map_write8,
	write16: rtl8196_map_write16,
	write32: rtl8196_map_write32,
	copy_to: rtl8196_map_copy_to

//end of jasonwang unlock 0227
};



/////////////////////////////////////////////////////////////////////////////

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)


	#if defined(CONFIG_RTL8196C_KLD)
	static struct mtd_partition rtl8196_parts[] = {
        {
                name: "boot+cfg+linux",
                size:  (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:0x00000000,
        },
        {
                name:           "root fs",                
                size:           (CONFIG_RTL_WEB_PAGES_OFFSET-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),   
	},
	 {
                name:           "webpage",                
		   size:           (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_WEB_PAGES_OFFSET),
		   offset:         (CONFIG_RTL_WEB_PAGES_OFFSET),
		}
	};
	#else
	static struct mtd_partition rtl8196_parts[] = {
		#ifdef _FLASH4M_
			{
					name:	"flash 4M",
					#ifdef _LANGUAGE_PACK_
						size:	0x400000 - 0x10000*_LANGUAGE_PACK_,
					#else
						size:	0x400000,
					#endif
					offset:	0x00000000,
			},
			{
					name:	"root fs",

					#ifdef _Logitec_
						#ifdef _LANGUAGE_PACK_
							size:	0x2F0000 - 0x10000*_LANGUAGE_PACK_,
						#else
							size:	0x2F0000,
						#endif
						offset:	0x110000,
					#else
						#ifdef _LANGUAGE_PACK_
							size:	0x350000 - 0x10000*_LANGUAGE_PACK_,
						#else
							size:	0x350000,
						#endif
						offset:	0x0b0000,
					#endif
			}

			#ifdef _LANGUAGE_PACK_
			,{
					name:	"Language Pack",
					size:	0x10000*_LANGUAGE_PACK_,
					offset:	0x400000 - 0x10000*_LANGUAGE_PACK_,
			}
			#endif
		#else
	/*
			{
					name: "boot+cfg+linux",
					size:  (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
					offset:0x00000000,
			},
			{
					name:           "root fs",
					size:           (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
					offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
			}
	*/
			{
					name: 	"flash 2M",
					#ifdef _LANGUAGE_PACK_
					size: 	0x200000 - 0x10000*_LANGUAGE_PACK_,
					#else
					size: 	0x200000,
					#endif
					offset:	0x00000000,
			},
			{
					name:	"root fs",
					#ifdef _LANGUAGE_PACK_
					size:   0x150000 - 0x10000*_LANGUAGE_PACK_,
					#else
					size:   0x150000,
					#endif
					offset: 0xb0000,
			}
			#ifdef _LANGUAGE_PACK_
			,{
					name:	"Language Pack",
					size:	0x10000*_LANGUAGE_PACK_,
					offset:	0x200000 - 0x10000*_LANGUAGE_PACK_,
			}
			#endif
		#endif
	};
	#endif
#else
static struct mtd_partition rtl8196_parts[] = {
        {
                name: "boot+cfg+linux",
                size:  0x000E0000,
                offset:0x00000000,
        },
        {
                name:           "root fs",                
	#ifdef CONFIG_RTL_WAPI_SUPPORT
                size:           0x00300000,
                offset:         0xE0000,
	#else
	   	size:        	0x00110000,
                offset:         0x000E0000,
        #endif
	}
};

#endif // #if defined(CONFIG_RTL8196C)








#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_rtl8196_map init_module
#define cleanup_rtl8196_map cleanup_module
#endif

#define mod_init_t  static int __init
#define mod_exit_t  static void __exit

mod_init_t init_rtl8196_map(void)
{
    printk(KERN_NOTICE "flash device: 0x%x at 0x%x\n", WINDOW_SIZE, WINDOW_ADDR);

	rtl8196_map.map_priv_1 =WINDOW_ADDR;

	if (!rtl8196_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	rtl8196_mtd = do_map_probe("cfi_probe", &rtl8196_map);
	if (!rtl8196_mtd)
	{
		printk("probe realtek\n");
		rtl8196_mtd = do_map_probe("spi_probe", &rtl8196_map);
	}
	printk("probe ends\n");
	if (rtl8196_mtd) {
		printk("probe realtek2\n");
		rtl8196_mtd->module = THIS_MODULE;
		add_mtd_partitions(rtl8196_mtd, rtl8196_parts, sizeof(rtl8196_parts)/sizeof(rtl8196_parts[0]));
		return 0;
	}

	iounmap((void *)rtl8196_map.map_priv_1);
	return -ENXIO;
}

mod_exit_t cleanup_rtl8196_map(void)
{
	if (rtl8196_mtd) {
		del_mtd_partitions(rtl8196_mtd);
		map_destroy(rtl8196_mtd);
	}
	//jasonwang 12112009 for linux 2.4 SPI driver
	if(rtl8196_map.map_priv_1)
	{
		iounmap((void *)rtl8196_map.map_priv_1);
		rtl8196_map.map_priv_1 = 0;
	}

}


MODULE_LICENSE("GPL");
module_init(init_rtl8196_map);
module_exit(cleanup_rtl8196_map);
