/**
 *  SPI Flash common control code.
 *  (C) 2006 Atmark Techno, Inc.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

#include "spi_flash.h"

//#define MTD_SPI_DEBUG

#if defined(MTD_SPI_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

// eric modified
#if 1 
#else 
extern int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr);
extern int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
extern int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
extern void mtd_spi_sync(struct mtd_info *mtd);
extern int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_suspend(struct mtd_info *mtd);
extern void mtd_spi_resume(struct mtd_info *mtd);
#endif

int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	unsigned long adr, len;
	int ret = 0;

	if (!chip_info->erase)
		return -EOPNOTSUPP;

	// skip 1st block erase
	if (instr->addr < (mtd->erasesize ))
	{
		instr->state = MTD_ERASE_DONE;
		return 0;
	}
	if (instr->addr & (mtd->erasesize - 1))
		return -EINVAL;
/*
	if (instr->len & (mtd->erasesize -1))
		return -EINVAL;
*/

	if ((instr->len + instr->addr) > mtd->size)
		return -EINVAL;

	adr = instr->addr;
	len = instr->len;

	KDEBUG("mtd_spi_erase():: adr: 0x%08lx, len: 0x%08lx\n", adr, len);


	if (len & (mtd->erasesize-1))
	{
		len = len - (len & (mtd->erasesize-1)) + mtd->erasesize;		
	}

	if (len < mtd->erasesize)
		len = mtd->erasesize;

	while (len) {
		ret = chip_info->erase(adr);
		if (ret)
			return ret;
		adr += mtd->erasesize;
		len -= mtd->erasesize;
	}

	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);

	return 0;
}

int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	int ret = 0;

	if (!chip_info->read) 
		return -EOPNOTSUPP;

	KDEBUG("mtd_spi_read():: adr: 0x%08x, len: %08x\n", (u32)from, len);

	ret = chip_info->read(from, (u32)buf, len);
	if (ret)
		return ret;

	if(retlen)
		(*retlen) = len;

	return 0;
}

int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	int ret = 0;
	
	if (!chip_info->write)
		return -EOPNOTSUPP;

	KDEBUG("mtd_spi_write():: adr: 0x%08x, len: 0x%08x\n", (u32)to, len);

	ret = chip_info->write((u32)buf, to, len);
	if (ret)
		return ret;

	if (retlen)
		(*retlen) = len;

	return 0;
}

void mtd_spi_sync(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
}

int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

int mtd_spi_suspend(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

void mtd_spi_resume(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
}

EXPORT_SYMBOL(mtd_spi_erase);
EXPORT_SYMBOL(mtd_spi_read);
EXPORT_SYMBOL(mtd_spi_write);
EXPORT_SYMBOL(mtd_spi_sync);
EXPORT_SYMBOL(mtd_spi_lock);
EXPORT_SYMBOL(mtd_spi_unlock);
EXPORT_SYMBOL(mtd_spi_suspend);
EXPORT_SYMBOL(mtd_spi_resume);
