/*
 *  Copyright © 2001 Flaga hf. Medical Devices, Kári Davíðsson <kd@flaga.is>
 *
 *  $Id: mx29lv640b.c,v 1.1.1.1 2007/08/06 10:04:50 root Exp $
 *  
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
                                                                                                                             
#define FLASH_PHYS_ADDR 0x1e400000
#define FLASH_SIZE              0x400000
                                                                                                                             
#define FLASH_PARTITION1_ADDR 0x00000000
#define FLASH_PARTITION1_SIZE 0x00400000

/*                                                                                                                             
struct map_info mx29lv640b_map = {
                .name =         "MX29LV640B flash device",
                .size =         FLASH_SIZE,
                .buswidth =     2,
};
*/
                                                                                                                             
struct mtd_partition mx29lv640b_parts[] = {
        {
                .name =         "Flash Disk (4MB)",
                .offset =       FLASH_PARTITION1_ADDR,
                .size =         FLASH_PARTITION1_SIZE
        }
};
                                                                                                                             
#define PARTITION_COUNT (sizeof(mx29lv640b_parts)/sizeof(struct mtd_partition))



static struct mtd_info *mymtd;

__u8 physmap_read8(struct map_info *map, unsigned long ofs)
{
        return __raw_readb(map->map_priv_1 + ofs);
}
                                                                                                                             
__u16 physmap_read16(struct map_info *map, unsigned long ofs)
{
        return __raw_readw(map->map_priv_1 + ofs);
}
                                                                                                                             
__u32 physmap_read32(struct map_info *map, unsigned long ofs)
{
        return __raw_readl(map->map_priv_1 + ofs);
}
                                                                                                                             
void physmap_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
        memcpy_fromio(to, map->map_priv_1 + from, len);
}
                                                                                                                             
void physmap_write8(struct map_info *map, __u8 d, unsigned long adr)
{
        __raw_writeb(d, map->map_priv_1 + adr);
        mb();
}
                                                                                                                             
void physmap_write16(struct map_info *map, __u16 d, unsigned long adr)
{
        __raw_writew(d, map->map_priv_1 + adr);
        mb();
}

void physmap_write32(struct map_info *map, __u32 d, unsigned long adr)
{
        __raw_writel(d, map->map_priv_1 + adr);
        mb();
}
                                                                                                                             
void physmap_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
        memcpy_toio(map->map_priv_1 + to, from, len);
}

struct map_info mx29lv640b_map = {
        name: "MX29LV640B flash map",
        size: FLASH_SIZE,
        buswidth: 2,
        read8: physmap_read8,
        read16: physmap_read16,
        read32: physmap_read32,
        copy_from: physmap_copy_from,
        write8: physmap_write8,
        write16: physmap_write16,
        write32: physmap_write32,
        copy_to: physmap_copy_to
};



int __init init_mx29lv640b(void)
{	
	printk(KERN_NOTICE "MX29LV640B flash device: %x at %x\n",
			FLASH_SIZE, FLASH_PHYS_ADDR);
	
	////mx29lv640b_map.phys = FLASH_PHYS_ADDR;
	mx29lv640b_map.map_priv_1 = (unsigned long)ioremap_nocache(FLASH_PHYS_ADDR,
					FLASH_SIZE);

	if (!mx29lv640b_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}

////	simple_map_init(&mx29lv640b_map);

	mymtd = do_map_probe("cfi_probe", &mx29lv640b_map);
	if (mymtd) {
		mymtd->module = THIS_MODULE;
		add_mtd_partitions(mymtd, mx29lv640b_parts, PARTITION_COUNT);
		printk(KERN_NOTICE "MX29LV640b flash device initialized\n");
		return 0;
	}

	iounmap((void *)mx29lv640b_map.map_priv_1);
	return -ENXIO;
}

static void __exit cleanup_mx29lv640b(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
	}
	if (mx29lv640b_map.map_priv_1) {
		iounmap((void *)mx29lv640b_map.map_priv_1);
		mx29lv640b_map.map_priv_1 = 0;
	}
}

module_init(init_mx29lv640b);
module_exit(cleanup_mx29lv640b);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIWU");
MODULE_DESCRIPTION("MTD map driver for MX29Lv640B");
