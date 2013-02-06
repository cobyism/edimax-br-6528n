/**
 *  SPI Flash probe code.
 *  (C) 2006 Atmark Techno, Inc.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include "spi_flash.h"

#include <linux/mtd/mtd.h>
#define MTD_SPI_DEBUG

#if defined(MTD_SPI_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

typedef struct spi_chip_info *(spi_probe_func)(struct map_info *, struct chip_probe *);

//extern spi_probe_func spi_probe_suzaku;
struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp);
static spi_probe_func *probe_func[] = {
//#if defined(CONFIG_MTD_SPI_SUZAKU) || defined(CONFIG_MTD_SPI_SUZAKU_MODULE)
	spi_probe_flash_chip,  
//#endif
	NULL
};

extern int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr);
extern int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
extern int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
extern void mtd_spi_sync(struct mtd_info *mtd);
extern int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_suspend(struct mtd_info *mtd);
extern void mtd_spi_resume(struct mtd_info *mtd);

static struct mtd_info *spi_chip_setup(struct map_info *map, struct spi_chip_info *chip_info);
static struct mtd_info *spi_probe_chip(struct map_info *map, struct chip_probe *cp);
struct mtd_info *spi_probe(struct map_info *map);
static void spi_destroy(struct mtd_info *mtd);

static struct mtd_chip_driver spi_chipdrv = {
	probe:   spi_probe,
	destroy: spi_destroy,
	name:    "spi_probe",
	module:  THIS_MODULE,
};

static struct mtd_info *spi_chip_setup(struct map_info *map, struct spi_chip_info *chip_info)
{
	struct mtd_info *mtd;

	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if (!mtd) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}

	memset(mtd, 0, sizeof(struct mtd_info));
	
	mtd->type                = MTD_OTHER;
	mtd->flags               = MTD_CAP_NORFLASH;

	mtd->name                = map->name;

	mtd->size                = chip_info->flash->DeviceSize;
	mtd->erasesize           = chip_info->flash->EraseSize;

	mtd->erase               = mtd_spi_erase;
	mtd->read                = mtd_spi_read;
	mtd->write               = mtd_spi_write;
	mtd->sync                = mtd_spi_sync;
	mtd->lock                = mtd_spi_lock;
	mtd->unlock              = mtd_spi_unlock;
	mtd->suspend             = mtd_spi_suspend;
	mtd->resume              = mtd_spi_resume;

	mtd->priv                = (void *)map;

	map->fldrv               = &spi_chipdrv;
	map->fldrv_priv          = chip_info;

	
	printk(KERN_INFO "%s: Found an alies 0x%x for the chip at 0x%x, ", map->name, chip_info->flash->DeviceSize, 0);
	printk("%s device detect.\n", chip_info->flash->name);

 	//mtd->writesize = 1;
	//MOD_INC_USE_COUNT;

	return mtd;
}

static struct mtd_info *spi_probe_chip(struct map_info *map, struct chip_probe *cp)
{
	struct mtd_info *mtd = NULL;
	struct spi_chip_info *chip_info = NULL;
	int i;
	
	for (i = 0; probe_func[i]; i++) {
		chip_info = probe_func[i](map, cp);
		if (!chip_info) continue;

 		mtd = spi_chip_setup(map, chip_info);
	
		if (!mtd) {
			kfree(chip_info);
			continue;
		}

		return mtd;
	}
	return NULL;
}

static struct chip_probe spi_chip_probe = {
	name: "SPI",
};

struct mtd_info *spi_probe(struct map_info *map)
{
	return spi_probe_chip(map, &spi_chip_probe);
}

static void spi_destroy(struct mtd_info *mtd)
{
	struct map_info *map = (struct map_info *)mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;

	if (chip_info->destroy) {
		chip_info->destroy(chip_info);
	}
}

int __init spi_probe_init(void)
{
	printk("SPI INIT\n");
	register_mtd_chip_driver(&spi_chipdrv);
	return 0;
}

void __exit spi_probe_exit(void)
{
	unregister_mtd_chip_driver(&spi_chipdrv);
}

module_init(spi_probe_init);
module_exit(spi_probe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masahiro Nakai <nakai@atmark-techno.com> et al.");
MODULE_DESCRIPTION("Probe code for SPI flash chips");
