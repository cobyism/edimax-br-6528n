/* SPI Flash driver
 *
 * Written by sam (sam@realtek.com)
 * 2010-05-01
 *
 */
#include <asm/rtl8196.h>   
#ifdef CONFIG_RTL8198
#include <asm/rtl8198.h>   
#endif
#include <linux/autoconf2.h>

//#define MTD_SPI_DEBUG 1
#if MTD_SPI_DEBUG
	#define KDEBUG(args...) printf(args)
#else
	#define KDEBUG(args...)
#endif

#define NDEBUG(args...) printf(args)

#include "spi_flash.h"
#include "spi_common.h"

extern struct spi_flash_type	spi_flash_info[2];
//extern struct spi_flash_known spi_flash_registed[];

// SPI Flash Init
void spi_pio_init_8198(void)
{
	KDEBUG("spi_pio_init: rstSPIFlash(0)");
	rstSPIFlash(0);
}
void spi_pio_init(void)
{
	KDEBUG("spi_pio_init: rstSPIFlash(0)");
	rstSPIFlash(0);
}

// SPI Flash Probe
void spi_probe()
{
	int i;
//	unsigned int uiCount;
//	uiCount = sizeof(spi_flash_registed) / sizeof(struct spi_flash_known);
	KDEBUG("spi_probe: spi_regist(0, 1)\n");
	for(i=0;i<CONFIG_FLASH_NUMBER;i++)
	{
		spi_regist(i);
	}
}

//SPI Flash Erase Sector
unsigned int spi_sector_erase(unsigned int uiChip, unsigned int uiAddr)
{
	KDEBUG("spi_sector_erase: uiChip=%x; uiAddr=%x\n", uiChip, uiAddr);
	return spi_flash_info[uiChip].pfErase(uiChip, uiAddr);
}

// SPI Flash Erase Block
unsigned int spi_block_erase(unsigned int uiChip, unsigned int uiAddr)
{
	KDEBUG("spi_block_erase: uiChip=%x; uiAddr=%x\n", uiChip, uiAddr);
	unsigned int uiRet;
	uiRet = ComSrlCmd_BE(uiChip, uiAddr);
	return uiRet;
}

// Erase whole chip
unsigned int spi_erase_chip(unsigned int uiChip)
{
	unsigned int uiRet;
	// Spansion
	KDEBUG("spi_erase_chip: uiChip=%x\n", uiChip);
	uiRet = ComSrlCmd_CE(uiChip);
	return uiRet;
}

// print unsigned char
/*
void prnUChar(char* pcName, unsigned char* pucBuffer, unsigned int uiLen)
{
	int i;
	unsigned char* puc;
	puc = pucBuffer;
	NDEBUG("%s", pcName);
	for (i = 0; i< uiLen; i++)
	{
		NDEBUG("%2x ",*puc);
		puc+=1;
	}
	NDEBUG("\n");
}
*/
/************************************ for old interface ************************************/
unsigned int spi_read(unsigned int uiChip, unsigned int uiAddr, unsigned int* puiDataOut)
{
	KDEBUG("spi_read: uiChip=%x; uiAddr=%x; uiLen=4; puiDataOut=%x\n", uiChip, uiAddr, (unsigned long)puiDataOut);
	return spi_flash_info[uiChip].pfRead(uiChip, uiAddr, 4, (unsigned char*)puiDataOut);
}

int flashread (unsigned long dst, unsigned int src, unsigned long length)
{

	KDEBUG("flashread: chip(uiChip)=%d; dst(pucBuffer)=%x; src(uiAddr)=%x; length=%x\n", uiChip, dst, src, length);
	return spi_flash_info[0].pfRead(0, src, length, (unsigned char*)dst);
}

int flashwrite(unsigned long dst, unsigned long src, unsigned long length)
{

	KDEBUG("flashwrite: dst(uiAddr)=%x; src(pucBuffer)=%x; length=%x; \n", dst, src, length);
	return spi_flash_info[0].pfWrite(0, dst, length, (unsigned char*)src);
}

int spi_flw_image(unsigned int chip, unsigned int flash_addr_offset ,unsigned char *image_addr, unsigned int image_size)
{
	KDEBUG("spi_flw_image: chip=%x; flash_addr_offset=%x; image_addr=%x; image_size=%x\n", chip, flash_addr_offset, (unsigned int)image_addr, image_size);
	return spi_flash_info[chip].pfWrite(chip, flash_addr_offset, image_size, image_addr);
}
int spi_flw_image_mio_8198(unsigned int cnt, unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size)
{
	KDEBUG("spi_flw_image_mio_8198: cnt=%x; flash_addr_offset=%x; image_addr=%x; image_size=%x\n", cnt, flash_addr_offset, (unsigned int)image_addr, 	image_size);
	return spi_flash_info[cnt].pfWrite(cnt, flash_addr_offset, image_size, image_addr);
}


