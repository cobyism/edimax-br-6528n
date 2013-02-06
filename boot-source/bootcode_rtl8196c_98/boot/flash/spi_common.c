/* SPI Flash driver
 *
 * Written by sam (sam@realtek.com)
 * 2010-05-01
 *
 */

#define MTD_SPI_DEBUG 3 

#include "spi_common.h"

#ifdef SPI_KERNEL
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#define malloc	vmalloc
#define free	vfree
//0
#if (MTD_SPI_DEBUG == 0)
#define NDEBUG(args...) printk(args)
#define KDEBUG(args...) printk(args)
#define LDEBUG(args...) printk(args)
#endif
//1
#if (MTD_SPI_DEBUG == 1)
#define NDEBUG(args...) printk(args)
#define KDEBUG(args...) printk(args)
#define LDEBUG(args...)
#endif
//2
#if (MTD_SPI_DEBUG == 2)
#define NDEBUG(args...) printk(args)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif
//3
#if (MTD_SPI_DEBUG == 3)
#define NDEBUG(args...)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif


#else
#include <asm/rtl8196x.h>
#include <rtl_types.h>
#if (MTD_SPI_DEBUG == 0)
//0
#define NDEBUG(args...) printf(args)
#define KDEBUG(args...) printf(args)
#define LDEBUG(args...) printf(args)
#endif
//1
#if (MTD_SPI_DEBUG == 1)
#define NDEBUG(args...) printf(args)
#define KDEBUG(args...) printf(args)
#define LDEBUG(args...)
#endif
//2
#if (MTD_SPI_DEBUG == 2)
#define NDEBUG(args...) printf(args)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif
//3
#if (MTD_SPI_DEBUG == 3)
#define NDEBUG(args...)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif

#endif

struct spi_flash_type	spi_flash_info[2];
unsigned char uiDispCount = 0;
unsigned char uiSFCR2_Flag = 154;

struct spi_flash_known spi_flash_registed[] = {
/****************************************** Micronix Flash ******************************************/
//#define MX25L1605D		0x00C22015
{0x00C22015, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L1605D", 50
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1
#endif
},
//#define MX25L3205D		0x00C22016
{0x00C22016, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L3205D", 50
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1
#endif
},
//#define MX25L6405D		0x00C22017
{0x00C22017, 0x00, SIZEN_08M, SIZE_064K, SIZE_064K, SIZE_256B, "MX25L6405D", 50
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1
#endif
},
//#define MX25L12805D		0x00C22018
{0x00C22018, 0x00, SIZEN_16M, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L12805D", 50
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1
#endif
},
//#define MX25L1635D		0x00C22415
{0x00C22415, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L1635D", 75
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, mxic_cmd_read_q1, mxic_spi_setQEBit, mxic_cmd_write_q1
#endif
},
//#define MX25L3235D		0x00C25E16
{0x00C25E16, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L3235D", 75
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, mxic_cmd_read_q1, mxic_spi_setQEBit, mxic_cmd_write_q1
#endif
},
//#define MX25L6445E		0x00C22017
//#define MX25L12845E		0x00C22018
//#define MX25L4005			0x00C22013

/****************************************** Spanson Flash ******************************************/
//#define S25FL016A		0x00010214
{0x00010214, 0x00, SIZEN_02M, SIZE_064K, SIZE_064K, SIZE_256B, "S25FL016A", 50
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_BE, span_cmd_read_s1, ComSrlCmd_NoneQeBit, span_cmd_write_s1
#endif
},
//#define S25FL032A		0x00010215
//#define S25FL032P		0x00010215
{0x00010215, 0x00, SIZEN_04M, SIZE_064K, SIZE_064K, SIZE_256B, "S25FL032A", 50
#ifndef CONFIG_SPI_STD_MODE
, SpanComSrlCmd_SE, span_cmd_read_q0, span_spi_setQEBit, span_cmd_write_q0
#endif
},
//#define S25FL004A		0x00010212
//#define S25FL064A		0x00010216
{0x00010216, 0x00, SIZEN_08M, SIZE_064K, SIZE_004K, SIZE_256B, "S25FL064P", 80
#ifndef CONFIG_SPI_STD_MODE
, SpanComSrlCmd_SE, span_cmd_read_q0, span_spi_setQEBit, span_cmd_write_q0
#endif
},
//#define S25FL128P		0x00012018

/****************************************** Eon Flash ******************************************/
//#define EN25F16			0x001c3115
{0x001c3115, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "EN25F16", 50
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, eon_cmd_read_s1, ComSrlCmd_NoneQeBit, eon_cmd_write_s1
#endif
},
//#define EN25F32			0x001c3116
{0x001c3116, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "EN25F32", 104
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, eon_cmd_read_s1, ComSrlCmd_NoneQeBit, eon_cmd_write_s1
#endif
},
//#define EN25Q16			0x001c3015
{0x001c3015, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "EN25Q16", 80
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, eon_cmd_read_q1, ComSrlCmd_NoneQeBit, eon_cmd_write_s1
#endif
},
//#define EN25Q32			0x001c3016
{0x001c3016, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "EN25Q32", 80
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, eon_cmd_read_q1, ComSrlCmd_NoneQeBit, eon_cmd_write_s1
#endif
},

#ifndef CONFIG_SPI_STD_MODE
/****************************************** SST Flash ******************************************/
//#define SST25VF032B		0x00BF254A
{0x00BF254A, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "SST25VF032B", 40
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, sst_cmd_read_s1, ComSrlCmd_NoneQeBit, sst_cmd_write_s1
#endif
},
//#define SST26VF032		0x00BF2602
//#define SST26VF016		0x00BF2601
#endif

/****************************************** GigaDevice Flash ******************************************/
//#define GD25Q16			0x00c84015
{0x00c84015, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q16", 120
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, gd_cmd_read_q0, gd_spi_setQEBit, gd_cmd_write_s1
#endif
},
//#define GD25Q32			0x00c84016
{0x00c84016, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q32", 120
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, gd_cmd_read_q0, gd_spi_setQEBit, gd_cmd_write_s1
#endif
},

/****************************************** WinBond Flash ******************************************/
//#define W25Q16			0x00EF4015
{0x00EF4015, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "W25Q16", 104
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, wb_cmd_read_q0, wb_spi_setQEBit, wb_cmd_write_q0
#endif
},
//#define W25Q32			0x00EF4016
{0x00EF4016, 0x00, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "W25Q32", 104
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, wb_cmd_read_q0, wb_spi_setQEBit, wb_cmd_write_q0
#endif
},
/****************************************** ATMEL Flash ******************************************/
//#define AT25DF161		0x001f4602
{0x001f4602, 0x00, SIZEN_02M, SIZE_064K, SIZE_004K, SIZE_256B, "AT25DF161", 85
#ifndef CONFIG_SPI_STD_MODE
, ComSrlCmd_SE, at_cmd_read_d0, ComSrlCmd_NoneQeBit, at_cmd_write_d0
#endif
}
};
// spi flash probe
void spi_regist(unsigned char uiChip, unsigned int uiPrnFlag)
{
	unsigned int ui, i, uiCount;
	unsigned char pucBuffer[4];

	ui = ComSrlCmd_RDID(0, 4);
	ui = ComSrlCmd_RDID(0, 4);
	ui = ui >> 8;

	uiCount = sizeof(spi_flash_registed) / sizeof(struct spi_flash_known);

	for (i = 0; i < uiCount; i++)
	{
		if((spi_flash_registed[i].uiChipId == ui) && (spi_flash_registed[i].uiDistinguish == 0x00))
		{
			break;
		}
	}
	if(i == uiCount)
	{
		// default setting
		setFSCR(uiChip, 40, 1, 1, 15);
		set_flash_info(uiChip, ui, SIZEN_04M, SIZE_064K, SIZE_004K, SIZE_256B, "UNKNOWN", ComSrlCmd_SE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1);
	}
	else
	{
		// have registed
		setFSCR(uiChip, spi_flash_registed[i].uiClk, 1, 1, 15);
#ifndef CONFIG_SPI_STD_MODE
		set_flash_info(uiChip, ui, spi_flash_registed[i].uiCapacityId, spi_flash_registed[i].uiBlockSize, spi_flash_registed[i].uiSectorSize, spi_flash_registed[i].uiPageSize, spi_flash_registed[i].pcChipName, spi_flash_registed[i].pfErase, spi_flash_registed[i].pfRead, spi_flash_registed[i].pfQeBit, spi_flash_registed[i].pfPageWrite);
#else
		if((ui & 0x00ffff00) == SPANSION_F)
		{
			set_flash_info(uiChip, ui, spi_flash_registed[i].uiCapacityId, spi_flash_registed[i].uiBlockSize, spi_flash_registed[i].uiSectorSize, spi_flash_registed[i].uiPageSize, spi_flash_registed[i].pcChipName, ComSrlCmd_BE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1);
		}
		else
		{
			set_flash_info(uiChip, ui, spi_flash_registed[i].uiCapacityId, spi_flash_registed[i].uiBlockSize, spi_flash_registed[i].uiSectorSize, spi_flash_registed[i].uiPageSize, spi_flash_registed[i].pcChipName, ComSrlCmd_SE, mxic_cmd_read_s1, ComSrlCmd_NoneQeBit, mxic_cmd_write_s1);
		}
#endif
	}
	spi_flash_info[uiChip].pfQeBit(uiChip);
	if(uiPrnFlag > 0)
	{
		prnFlashInfo(uiChip, spi_flash_info[uiChip]);
		//prnInterfaceInfo();
	}
	ui = spi_flash_info[uiChip].pfRead(uiChip, 0x00, 4, pucBuffer);
	LDEBUG("spi_regist: uiChip=%x; uiPrnFlag=%x; i=%x; uiCount=%x\n", uiChip, uiPrnFlag, i, uiCount);
}
// set spi_flash_info struction content
void set_flash_info(unsigned char uiChip, unsigned int chip_id, unsigned int device_cap, unsigned int block_size, unsigned int sector_size, unsigned int page_size, char* chip_name, FUNC_ERASE pfErase, FUNC_READ pfRead, FUNC_SETQEBIT pfQeBit, FUNC_PAGEWRITE pfPageWrite)
{
	unsigned int ui = 1 << device_cap;
	spi_flash_info[uiChip].chip_id = chip_id;
	spi_flash_info[uiChip].mfr_id = (chip_id >> 16) & 0xff;
	spi_flash_info[uiChip].dev_id = (chip_id >> 8) & 0xff;
	spi_flash_info[uiChip].capacity_id = (chip_id) & 0xff;
	spi_flash_info[uiChip].size_shift = calShift(spi_flash_info[uiChip].capacity_id, device_cap);
	spi_flash_info[uiChip].device_size = device_cap;			// 2 ^ N (bytes)
	spi_flash_info[uiChip].chip_size =  ui;
	spi_flash_info[uiChip].block_size = block_size;
	spi_flash_info[uiChip].block_cnt = ui / block_size;
	spi_flash_info[uiChip].sector_size = sector_size;
	spi_flash_info[uiChip].sector_cnt = ui / sector_size;
	spi_flash_info[uiChip].page_size = page_size;
	spi_flash_info[uiChip].page_cnt = sector_size / page_size;
	spi_flash_info[uiChip].chip_name = chip_name;
	spi_flash_info[uiChip].pfErase = pfErase;
	spi_flash_info[uiChip].pfWrite = ComSrlCmd_ComWriteData;
	spi_flash_info[uiChip].pfRead = pfRead;
	spi_flash_info[uiChip].pfQeBit = pfQeBit;
	spi_flash_info[uiChip].pfPageWrite = pfPageWrite;
	//SPI_REG_LOAD(SFCR2, 0x0bb08000);
	LDEBUG("set_flash_info: uiChip=%x; chip_id=%x; device_cap=%x; block_size=%x; sector_size=%x; page_size=%x; chip_name=%s\n", uiChip, chip_id, device_cap, block_size, sector_size, page_size, chip_name);
}

/****************************** Common function ******************************/
// get Dram Frequence
unsigned int CheckDramFreq(void)                       //JSW:For 8196C
{
	unsigned short usFreqBit, usFreqVal;
	usFreqBit= (0x00001C00 & (*(unsigned int*)0xb8000008)) >> 10 ;
#ifdef CONFIG_RTL8198
	switch(usFreqBit)		// RTL8198
	{
		case 0:
		{
			usFreqVal = 65;
			break;
		}
		case 1:
		{
			usFreqVal = 78;
			break;
		}
		case 2:
		{
			usFreqVal = 150;
			break;
		}
		case 3:
		{
			usFreqVal = 125;
			break;
		}
		case 4:
		{
			usFreqVal = 156;
			break;
		}
		case 5:
		{
			usFreqVal = 168;
			break;
		}
		case 6:
		{
			usFreqVal = 237;
			break;
		}
		case 7:
		{
			usFreqVal = 193;
			break;
		}
		default :
		{
			usFreqVal = 237;
			KDEBUG("8198:Error ck_m2x_freq_sel number,should be 0~7");
			break;
		}
	}
	KDEBUG("CheckDramFreq:(8198)usFreqVal=%dMHZ; usFreqBit=%x; B8000008=%x;\n", usFreqVal, usFreqBit, (*(unsigned int*)0xb8000008));
#else
	switch(usFreqBit)	//RTL8196C
	{
		case 0:
		{
			usFreqVal = 65;
			break;
		}
		case 1:
		{
			usFreqVal = 78;
			break;
		}
		case 2:
		{
			usFreqVal = 125;
			break;
		}
		case 3:
		{
			usFreqVal = 150;
			break;
		}
		case 4:
		{
			usFreqVal = 156;
			break;
		}
		case 5:
		{
			usFreqVal = 168;
			break;
		}
		case 6:
		{
			usFreqVal = 193;
			break;
		}
		default :
		{
			usFreqVal = 193;
			KDEBUG("8196C:Error ck_m2x_freq_sel number,should be 0~6");
			break;
		}
	}
	KDEBUG("CheckDramFreq:(8196C)usFreqVal=%dMHZ; usFreqBit=%x; B8000008=%x;\n", usFreqVal, usFreqBit, (*(unsigned int*)0xb8000008));
#endif	
	return usFreqVal;
}
// Set FSCR register
void setFSCR(unsigned char uiChip, unsigned int uiClkMhz, unsigned int uiRBO, unsigned int uiWBO, unsigned int uiTCS)
{
	unsigned int ui, uiClk;
	uiClk = CheckDramFreq();
	ui = uiClk / uiClkMhz;
	if((uiClk % uiClkMhz) > 0)
	{
		ui = ui + 1;
	}
	if((ui % 2) > 0)
	{
		ui = ui + 1;
	}
	spi_flash_info[uiChip].chip_clk = uiClk / ui;
	SPI_REG_LOAD(SFCR, SFCR_SPI_CLK_DIV((ui-2)/2) | SFCR_RBO(uiRBO) | SFCR_WBO(uiWBO) | SFCR_SPI_TCS(uiTCS));
	KDEBUG("setFSCR:uiClkMhz=%d, uiRBO=%d, uiWBO=%d, uiTCS=%d, resMhz=%d, vale=%8x\n", uiClkMhz, uiRBO, uiWBO, uiTCS, spi_flash_info[uiChip].chip_clk, SPI_REG_READ(SFCR));
}
// Calculate write address group
void calAddr(unsigned int uiStart, unsigned int uiLenth, unsigned int uiSectorSize, unsigned int* uiStartAddr, unsigned int*  uiStartLen, unsigned int* uiSectorAddr, unsigned int* uiSectorCount, unsigned int* uiEndAddr, unsigned int* uiEndLen)
{
	KDEBUG("calAddr: uiStart=%x; uiLenth=%x; uiSectorSize=%x;\n", uiStart, uiLenth, uiSectorSize);	
	unsigned int ui;
	// only one sector
	if ((uiStart + uiLenth) < ((uiStart / uiSectorSize + 1) * uiSectorSize))
	{	// start	
		*uiStartAddr = uiStart;
		*uiStartLen = uiLenth;
		//middle
		*uiSectorAddr = 0x00;
		*uiSectorCount = 0x00;
		// end
		*uiEndAddr = 0x00;
		*uiEndLen = 0x00;
	}
	//more then one sector
	else
	{
		// start
		*uiStartAddr = uiStart;
		*uiStartLen = uiSectorSize - (uiStart % uiSectorSize);
		if(*uiStartLen == uiSectorSize)
		{
			*uiStartLen = 0x00;
		}
		// middle
		ui = uiLenth - *uiStartLen;
		*uiSectorAddr = *uiStartAddr + *uiStartLen;
		*uiSectorCount = ui / uiSectorSize;
		//end
		*uiEndAddr = *uiSectorAddr + (*uiSectorCount * uiSectorSize);
		*uiEndLen = ui % uiSectorSize;
	}
	KDEBUG("calAddr: uiStartAddr = %x; uiStartLen = %x; uiSectorAddr = %x; uiSectorCount = %x; uiEndAddr= %x; uiEndLen = %x;\n", *uiStartAddr, *uiStartLen, *uiSectorAddr, *uiSectorCount, *uiEndAddr, *uiEndLen);	
}
// Calculate chip capacity shift bit 
unsigned char calShift(unsigned char ucCapacityId, unsigned char ucChipSize)
{
	unsigned int ui;
	if(ucChipSize > ucCapacityId)
	{
		ui = ucChipSize - ucCapacityId;
	}
	else
	{
		ui = ucChipSize + 0x100 -ucCapacityId;
	}
	KDEBUG("calShift: ucCapacityId=%x; ucChipSize=%x; ucReturnVal=%x\n", ucCapacityId, ucChipSize, ui);
	return (unsigned char)ui;	
}
// Print spi_flash_type
void prnFlashInfo(unsigned char uiChip, struct spi_flash_type sftInfo)
{
#ifndef CONFIG_SPI_STD_MODE
	NDEBUG("\n********************************************************************************\n");
	NDEBUG("*\n");
	NDEBUG("* chip__no chip__id mfr___id dev___id cap___id size_sft dev_size chipSize\n");
	NDEBUG("* %8x %8x %8x %8x %8x %8x %8x %8x\n", uiChip, sftInfo.chip_id, sftInfo.mfr_id, sftInfo.dev_id, sftInfo.capacity_id, sftInfo.size_shift, sftInfo.device_size, sftInfo.chip_size);
	NDEBUG("* blk_size blk__cnt sec_size sec__cnt pageSize page_cnt chip_clk chipName\n");
	NDEBUG("* %8x %8x %8x %8x %8x %8x %8x %s\n", sftInfo.block_size, sftInfo.block_cnt, sftInfo.sector_size, sftInfo.sector_cnt, sftInfo.page_size, sftInfo.page_cnt, sftInfo.chip_clk, sftInfo.chip_name);
	NDEBUG("* \n");
	NDEBUG("********************************************************************************\n");
#else
	NDEBUG("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	NDEBUG("@\n");
	NDEBUG("@ chip__no chip__id mfr___id dev___id cap___id size_sft dev_size chipSize\n");
	NDEBUG("@ %8x %8x %8x %8x %8x %8x %8x %8x\n", uiChip, sftInfo.chip_id, sftInfo.mfr_id, sftInfo.dev_id, sftInfo.capacity_id, sftInfo.size_shift, sftInfo.device_size, sftInfo.chip_size);
	NDEBUG("@ blk_size blk__cnt sec_size sec__cnt pageSize page_cnt chip_clk chipName\n");
	NDEBUG("@ %8x %8x %8x %8x %8x %8x %8x %s\n", sftInfo.block_size, sftInfo.block_cnt, sftInfo.sector_size, sftInfo.sector_cnt, sftInfo.page_size, sftInfo.page_cnt, sftInfo.chip_clk, sftInfo.chip_name);
	NDEBUG("@ \n");
	NDEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
#endif
}

/*
// Print SPI Register
void prnInterfaceInfo()
{
	NDEBUG("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	NDEBUG("@ \n");
	NDEBUG("@ SFCR (0xb800_1200) = 0x%8x;  ", SPI_REG_READ(SFCR));
	NDEBUG("  SFCR2(0xb800_1204) = 0x%8x\n", SPI_REG_READ(SFCR2));
	NDEBUG("@ SFCSR(0xb800_1208) = 0x%8x\n", SPI_REG_READ(SFCSR));
//	NDEBUG("@ SFDR (0xb800_120c) = 0x%8x\n", SPI_REG_READ(SFDR));
//	NDEBUG("@ SFDR2(0xb800_1210) = 0x%8x\n", SPI_REG_READ(SFDR2));
	NDEBUG("@ \n");
	NDEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}
*/
// print when writing
void prnDispAddr(unsigned int uiAddr)
{
#if 0	
	if((uiDispCount % 0x0a) == 0)
	{
		NDEBUG("\n%8x", uiAddr);
	}
	else
	{
		NDEBUG("%8x", uiAddr);
	}
#endif
	prom_printf(".");
	uiDispCount++;
}
// Check WIP bit
unsigned int spiFlashReady(unsigned char uiChip)
{
	unsigned int uiCount, ui;	
	uiCount = 0;
	while (1)
	{
		uiCount++;
		ui = ComSrlCmd_RDSR(uiChip, 1);
		if ((ui & (1 << SPI_STATUS_WIP)) == 0)
		{
			break;
		}
	}
	KDEBUG("spiFlashReady: uiCount=%x\n", uiCount);	
	return uiCount;
}
//toggle CS
void rstSPIFlash(unsigned char uiChip)
{
	SFCSR_CS_L(uiChip, 0, 0);
	SFCSR_CS_H(uiChip, 0, 0);
	SFCSR_CS_L(uiChip, 0, 0);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("rstFPIFlash: uiChip=%x;\n", uiChip);	
}

/****************************** Layer 1 ******************************/
// set cs high
unsigned int SFCSR_CS_L(unsigned char uiChip, unsigned int uiLen, unsigned int uiIOWidth)
{
	unsigned int ui = 0;
	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0)
	{
		ui++;
	}
	//*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(1 + (uiChip)) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(uiIOWidth) | SFCSR_CHIP_SEL(0) | SFCSR_CMD_BYTE(0);
	*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(1 + (uiChip)) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(uiIOWidth);
	LDEBUG("SFCSR_CS_L: uiChip=%x; uiLen=%x; uiIOWidth=%x; ui=%x; \n", uiChip, uiLen, uiIOWidth, ui);
	return ui;
}
// set cs low
unsigned int SFCSR_CS_H(unsigned char uiChip, unsigned int uiLen, unsigned int uiIOWidth)
{
	unsigned int ui = 0;
	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0)
	{
		ui++;
	}
	//*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(3) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(uiIOWidth) | SFCSR_CHIP_SEL(0) | SFCSR_CMD_BYTE(0);
	*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(3) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(uiIOWidth);
	LDEBUG("SFCSR_CS_H: uiChip=%x; uiLen=%x; uiIOWidth=%x; ui=%x; \n", uiChip, uiLen, uiIOWidth, ui);
	return ui;
}
// Write Enable (WREN) Sequence (Command 06)
void ComSrlCmd_WREN(unsigned char uiChip)
{
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, SPICMD_WREN << 24);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_WREN: uiChip=%x; SPICMD_WREN=%x;\n", uiChip, SPICMD_WREN);
}
// Write Disable (WRDI) Sequence (Command 04)
void ComSrlCmd_WRDI(unsigned char uiChip)
{
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, SPICMD_WRDI << 24);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_WRDI: uiChip=%x; SPICMD_WRDI=%x;\n", uiChip, SPICMD_WRDI);
}
// Read Identification (RDID) Sequence (Command 9F)
unsigned int ComSrlCmd_RDID(unsigned char uiChip, unsigned int uiLen)
{
	unsigned int ui;

	SPI_REG_LOAD(SFCR, (SFCR_SPI_CLK_DIV(7) | SFCR_RBO(1) | SFCR_WBO(1) | SFCR_SPI_TCS(15)));		//SFCR default setting

	rstSPIFlash(uiChip);
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_RDID << 24));
	SFCSR_CS_L(uiChip, (uiLen - 1), 0);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_RDID: uiChip=%x; uiLen=%x; returnValue=%x; SPICMD_RDID=%x;\n", uiChip, uiLen, ui, SPICMD_RDID);
	return ui;
}
// Read Status Register (RDSR) Sequence (Command 05)
unsigned int ComSrlCmd_RDSR(unsigned char uiChip, unsigned int uiLen)
{
	unsigned int ui;
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_RDSR << 24));
	SFCSR_CS_L(uiChip, uiLen-1, 0);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(uiChip, 0, 0);
	ui = ui >> ((4 - uiLen) * 8);
	LDEBUG("ComSrlCmd_RDSR: uiChip=%x; uiLen=%x; returnValue=%x; SPICMD_RDSR=%x;\n", uiChip, uiLen, ui, SPICMD_RDSR);
	return ui;
}
// Write Status Register (WRSR) Sequence (Command 01)
unsigned int ComSrlCmd_WRSR(unsigned char uiChip,unsigned char ucValue)
{
	unsigned int ui = (SPICMD_WRSR << 24) | (ucValue << 16);
	SFCSR_CS_L(uiChip, 1, 0);
	SPI_REG_LOAD(SFDR,ui);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_WRSR: uiChip=%x; uiValue=%x; Status=%x; SPICMD_WRSR=%x;\n", uiChip, ucValue, ComSrlCmd_RDSR(uiChip, 4), SPICMD_WRSR);
	return ui;
}
// Sector Erase (SE) Sequence (Command 20)
unsigned int ComSrlCmd_SE(unsigned char uiChip, unsigned int uiAddr)
{
	ComSrlCmd_WREN(uiChip);	
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR, ((SPICMD_SE << 24) | (uiAddr & 0xffffff)));
	SFCSR_CS_H(uiChip, 0, 0);
	KDEBUG("ComSrlCmd_SE: uiChip=%x; uiSector=%x; uiSectorSize=%x; SPICMD_SE=%x\n", uiChip, uiAddr, spi_flash_info[uiChip].sector_size, SPICMD_SE);
	return spiFlashReady(uiChip);
}
// Block Erase (BE) Sequence (Command D8)
unsigned int ComSrlCmd_BE(unsigned char uiChip, unsigned int uiAddr)
{
	ComSrlCmd_WREN(uiChip);	
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR, ((SPICMD_BE << 24) | (uiAddr & 0xffffff)));
	SFCSR_CS_H(uiChip, 0, 0);
	KDEBUG("ComSrlCmd_BE: uiChip=%x; uiBlock=%x; uiBlockSize=%x; SPICMD_BE=%x\n", uiChip, uiAddr, spi_flash_info[uiChip].block_size, SPICMD_BE);
	return spiFlashReady(uiChip);
}
// Chip Erase (CE) Sequence (Command 60 or C7)
unsigned int ComSrlCmd_CE(unsigned char uiChip)
{
	ComSrlCmd_WREN(uiChip);	
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_CE << 24));
	SFCSR_CS_H(uiChip, 0, 0);
	KDEBUG("ComSrlCmd_CE: uiChip=%x; SPICMD_CE=%x\n", uiChip, SPICMD_CE);
	return spiFlashReady(uiChip);
}
/*
// Deep Power Down (DP) Sequence (Command B9)
unsigned int ComSrlCmd_DP(unsigned char uiChip)
{
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_DP << 24));
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_DP: uiChip=%x; SPICMD_DP=%x;\n", uiChip, SPICMD_DP);
	return 0;
}
// Release from Deep Power-down(RDP) Sequence (Command AB)
unsigned int ComSrlCmd_RDP(unsigned char uiChip)
{
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_RDP << 24));
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_RDP: uiChip=%x; SPICMD_RDP=%x;\n", uiChip, SPICMD_RDP);
	return 0;
}
// Release from Deep Power-down and Read Electronic Signature (RES) Sequece (Command AB)
unsigned int ComSrlCmd_RES(unsigned char uiChip)
{
	unsigned int ui;
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_RES << 24));		// Command
	SFCSR_CS_L(uiChip, 0, 0);
	ui = SPI_REG_READ(SFDR);					// Get Electronic Signature
	SFCSR_CS_H(uiChip, 0, 0);
	ui = ui >> 24;
	LDEBUG("ComSrlCmd_RES: uiChip=%x; returnValue=%x; SPICMD_RES=%x;\n", uiChip, ui, SPICMD_RES);
	return ui;
}
// Read Electronic Manufacturer & Device ID (REMS) Sequece (Command 90 or EF or DF)
unsigned int ComSrlCmd_REMS(unsigned char uiChip)
{
	unsigned int ui;

	ui = (SPICMD_REMS_90 << 24) | (0 << 16) | (1);		// Device ID First
	//ui = (SPICMD_REMS_DF << 24) | (0 << 16) | (0);		// Manufacturer ID First
	//ui = (SPICMD_REMS_EF << 24) | (0 << 16) | (0);		// Manufacturer ID First
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_L(uiChip, 0, 0);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_REMS: uiChip=%x; returnValue=%x; SPICMD_REMS_90=%x;\n", uiChip, ui, SPICMD_REMS_90);
	return ui;
}
// Enter Security OTP
unsigned int ComSrlCmd_ENSO(unsigned char uiChip)
{
	unsigned int ui;
	ui = SPICMD_ENSO << 24;
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_ENSO: uiChip=%x; returnValue=%x; SPICMD_ENSO=%x;\n", uiChip, ui, SPICMD_ENSO);
	return ui;
}
// Exit Security OTP
unsigned int ComSrlCmd_EXSO(unsigned char uiChip)
{
	unsigned int ui;
	ui = SPICMD_EXSO << 24;
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("ComSrlCmd_EXSO: uiChip=%x; returnValue=%x; SPICMD_EXSO=%x;\n", uiChip, ui, SPICMD_EXSO);
	return ui;
}
*/
/****************************** Layer 2 ******************************/
// without QE bit
unsigned int ComSrlCmd_NoneQeBit(unsigned char uiChip)
{
	KDEBUG("ComSrlCmd_NoneQeBit: uiChip=%x;\n", uiChip);
	return 0;
}
// uiIsFast: = 0 cmd, address, dummy single IO ; =1 cmd single IO, address and dummy multi IO; =2 cmd, address and dummy multi IO;
void ComSrlCmd_InputCommand(unsigned char uiChip, unsigned int uiAddr, unsigned char ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount)
{
	int i;
	LDEBUG("ComSrlCmd_InputCommand: uiChip=%x; uiAddr=%x; ucCmd=%x; uiIsfast=%x; uiIOWidth=%x; uiDummyCount=%x\n", uiChip, uiAddr, ucCmd, uiIsFast, uiIOWidth, uiDummyCount);

	// input command
	if(uiIsFast == 2)
	{
		SFCSR_CS_L(uiChip, 0, uiIOWidth);
	}
	else
	{
		SFCSR_CS_L(uiChip, 0, 0);
	}
	SPI_REG_LOAD(SFDR, (ucCmd << 24));				// Read Command

	// input 3 bytes address
	if(uiIsFast == 0)
	{
		SFCSR_CS_L(uiChip, 0, IOWIDTH_SINGLE);
	}
	else
	{
		SFCSR_CS_L(uiChip, 0, uiIOWidth);
	}
	SPI_REG_LOAD(SFDR,(uiAddr << 8));
	SPI_REG_LOAD(SFDR,(uiAddr << 16));
	SPI_REG_LOAD(SFDR,(uiAddr << 24));

	//input dummy cycle
	for (i = 0; i < uiDummyCount; i++)
	{
		SPI_REG_LOAD(SFDR, 0);
	}
	
	SFCSR_CS_L(uiChip, 3, uiIOWidth);
}
// Set SFCR2 for memery map read
unsigned int SetSFCR2(unsigned int ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount)
{
	unsigned int ui, uiDy;
	uiSFCR2_Flag = 0;
	ui = SFCR2_SFCMD(ucCmd) | SFCR2_SFSIZE(spi_flash_info[0].device_size - 0x11) | SFCR2_RD_OPT(0) | SFCR2_HOLD_TILL_SFDR2(0);
	switch (uiIsFast)
	{
		case ISFAST_NO:
		{
			ui = ui | SFCR2_CMD_IO(IOWIDTH_SINGLE) | SFCR2_ADDR_IO(IOWIDTH_SINGLE) | SFCR2_DATA_IO(uiIOWidth);
			uiDy = 1;
			break;
		}
		case ISFAST_YES:
		{
			ui = ui | SFCR2_CMD_IO(IOWIDTH_SINGLE) | SFCR2_ADDR_IO(uiIOWidth) | SFCR2_DATA_IO(uiIOWidth);
			uiDy = uiIOWidth * 2;
			break;
		}
		case ISFAST_ALL:
		{
			ui = ui | SFCR2_CMD_IO(uiIOWidth) | SFCR2_ADDR_IO(uiIOWidth) | SFCR2_DATA_IO(uiIOWidth);
			uiDy = uiIOWidth * 2;
			break;
		}
		default:
		{
			ui = ui | SFCR2_CMD_IO(IOWIDTH_SINGLE) | SFCR2_ADDR_IO(IOWIDTH_SINGLE) | SFCR2_DATA_IO(uiIOWidth);
			uiDy = 1;
			break;
		}
	}
	if (uiDy == 0)
	{
		uiDy = 1;
	}
	KDEBUG("SetSFCR2:ui=%x; uiDy=%x; uiDummyCount=%x\n", ui, uiDy, uiDummyCount);
	ui = ui | SFCR2_DUMMY_CYCLE((uiDummyCount * 4 / uiDy));		// uiDummyCount is Byte Count uiDummyCount*8 / (uiDy*2)
	SPI_REG_LOAD(SFCR2, ui);
	KDEBUG("SetSFCR2: ucCmd=%x; uiIsFast=%; uiIOWidth=%x; uiDummyCount=%x; uiSFCR2_Flag=%x; SFCR2=%x\n;", ucCmd, uiIsFast, uiIOWidth, uiDummyCount, uiSFCR2_Flag, ui);
	return ui;	
}
// read template
unsigned int ComSrlCmd_ComRead(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer,unsigned char ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount)
{

	unsigned int ui, uiCount, i;
	unsigned char* puc = pucBuffer;
	LDEBUG("ComSrlCmd_ComRead: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; ucCmd=%x; uiIsfast=%x; uiIOWidth=%x; uiDummyCount=%x\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, ucCmd, uiIsFast, uiIOWidth, uiDummyCount);
	ComSrlCmd_InputCommand(uiChip, uiAddr, ucCmd, uiIsFast, uiIOWidth, uiDummyCount);
	if(uiSFCR2_Flag != 0)	// set SFCR2
	{
		ui = SetSFCR2(ucCmd, uiIsFast, uiIOWidth, uiDummyCount);
	}

	uiCount = uiLen / 4;							
	for( i = 0; i< uiCount; i++)					// Read 4 bytes every time.
	{
		ui = SPI_REG_READ(SFDR);
		memcpy(puc, &ui, 4);
		puc += 4;
	}

	i = uiLen % 4;
	if(i > 0)
	{
		ui = SPI_REG_READ(SFDR);					// another bytes.
		memcpy(puc, &ui, i);
		puc += i;
	}
	SFCSR_CS_H(uiChip, 0, 0);
	return uiLen;
	
}
// write template
unsigned int ComSrlCmd_ComWrite(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer, unsigned char ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount)
{
	unsigned int ui, uiCount, i;
	unsigned char* puc = pucBuffer;
	KDEBUG("ComSrlCmd_ComWrite: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; ucCmd=%x; uiIsfast=%x; uiIOWidth=%x; uiDummyCount=%x\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, ucCmd, uiIsFast, uiIOWidth, uiDummyCount);
	ComSrlCmd_WREN(uiChip);

	ComSrlCmd_InputCommand(uiChip, uiAddr, ucCmd, uiIsFast, uiIOWidth, uiDummyCount);

	uiCount = uiLen / 4;
	for (i = 0; i <  uiCount; i++)
	{
		memcpy(&ui, puc, 4);
		puc += 4;
		SPI_REG_LOAD(SFDR, ui);
	}

	i = uiLen % 4;
	if(i > 0)
	{
		memcpy(&ui, puc, i);
		puc += i;
		SFCSR_CS_L(uiChip, i-1, uiIOWidth);
		SPI_REG_LOAD(SFDR, ui);
	}
	SFCSR_CS_H(uiChip, 0, 0);
	ui = spiFlashReady(uiChip);
	return uiLen;
}
// write a whole sector once
unsigned int ComSrlCmd_ComWriteSector(unsigned char uiChip, unsigned int uiAddr, unsigned char* pucBuffer)
{
	unsigned int i, ui;
	unsigned char* puc = pucBuffer;
	KDEBUG("ComSrlCmd_ComWriteSector: uiChip=%x; uiAddr=%x; pucBuffer=%x; returnValue=%x;\n", uiChip, uiAddr, (unsigned int)pucBuffer, spi_flash_info[uiChip].sector_size);
	prnDispAddr(uiAddr);
	ui = spi_flash_info[uiChip].pfErase(uiChip, uiAddr);
	for (i = 0; i < spi_flash_info[uiChip].page_cnt; i++)
	{
		ui = spi_flash_info[uiChip].pfPageWrite(uiChip, uiAddr, spi_flash_info[uiChip].page_size, puc);
		uiAddr += spi_flash_info[uiChip].page_size;
		puc += spi_flash_info[uiChip].page_size;
	}
	return spi_flash_info[uiChip].sector_size;
}

// write sector use malloc buffer
unsigned int ComSrlCmd_BufWriteSector(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned char pucSector[spi_flash_info[uiChip].sector_size];
	unsigned int ui, uiStartAddr, uiOffset;
	KDEBUG("ComSrlCmd_BufWriteSector:uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x;\n", uiChip, uiAddr, uiLen, pucBuffer);
	uiOffset = uiAddr % spi_flash_info[uiChip].sector_size;
	uiStartAddr = uiAddr - uiOffset;
	// get
	ui = spi_flash_info[uiChip].pfRead(uiChip, uiStartAddr, spi_flash_info[uiChip].sector_size, pucSector);
	// modify
	memcpy(pucSector + uiOffset, pucBuffer, uiLen);
	//write back
	ui = ComSrlCmd_ComWriteSector(uiChip, uiStartAddr, pucSector);
	return ui;
}

// write data, any address any lenth
unsigned int ComSrlCmd_ComWriteData(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned int uiStartAddr, uiStartLen, uiSectorAddr, uiSectorCount, uiEndAddr, uiEndLen, ui, i;
	unsigned char* puc = pucBuffer;
	KDEBUG("ComSrlCmd_ComWriteData:uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer);
	calAddr(uiAddr, uiLen, spi_flash_info[uiChip].sector_size, &uiStartAddr, &uiStartLen, &uiSectorAddr, &uiSectorCount, &uiEndAddr, &uiEndLen);
	if((uiSectorCount == 0x00) && (uiEndLen == 0x00))	// all data in the same sector
	{
		ComSrlCmd_BufWriteSector(uiChip, uiStartAddr, uiStartLen, puc);
	}
	else
	{
		if(uiStartLen > 0)
		{
			ComSrlCmd_BufWriteSector(uiChip, uiStartAddr, uiStartLen, puc);
			puc += uiStartLen;
		}
		for(i = 0; i < uiSectorCount; i++)
		{
			ComSrlCmd_ComWriteSector(uiChip, uiSectorAddr, puc);
			puc += spi_flash_info[uiChip].sector_size;
			uiSectorAddr += spi_flash_info[uiChip].sector_size;
		}
		if(uiEndLen > 0)
		{
			ComSrlCmd_BufWriteSector(uiChip, uiEndAddr, uiEndLen, puc);
		}
	}
	ComSrlCmd_WRDI(uiChip);
	return uiLen;
}

/****************************** Macronix ******************************/
// Set quad enable bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_spi_setQEBit(unsigned char uiChip)
{
	unsigned int ui;
	ComSrlCmd_WREN(uiChip);
	//ui = (0 << SPI_STATUS_REG_SRWD) | (1 << SPI_STATUS_QE) | (0 << SPI_STATUS_BP3) | (0 << SPI_STATUS_BP2) | (0 << SPI_STATUS_BP1) | (0 << SPI_STATUS_BP0) | (0 << SPI_STATUS_WEL) | (0 << SPI_STATUS_WIP);
	ui = 1 << SPI_STATUS_QE;
	ComSrlCmd_WRSR(uiChip, ui);			// set Micronix QE bit
	KDEBUG("MxicSetQEBit: uiChip=%d; statusRegister=%x; returnValue=%x\n", uiChip, ComSrlCmd_RDSR(uiChip, 4), ui);
	return ui;
}
#endif
// MX25L1605 MX25L3205 Read at High Speed (FAST_READ) Sequence (Command 0B)
unsigned int mxic_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("mxic_cmd_read_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_FASTREAD=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_FASTREAD);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_FASTREAD, ISFAST_YES, IOWIDTH_SINGLE, DUMMYCOUNT_1);
}
// MX25L1605 MX25L3205 Read at Dual IO Mode Sequence (Command BB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_cmd_read_d1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("mxic_cmd_read_d1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_2READ=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_2READ);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_2READ, ISFAST_YES, IOWIDTH_DUAL, DUMMYCOUNT_1);
}
#endif
// MX25L1635 MX25L3235 4 x I/O Read Mode Sequence (Command EB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_cmd_read_q1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("mxic_cmd_read_q1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_4READ=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_4READ);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_4READ, ISFAST_YES, IOWIDTH_QUAD, DUMMYCOUNT_3);
}
#endif
// Page Program (PP) Sequence (Command 02)
unsigned int mxic_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("mxic_cmd_write_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_PP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_PP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_PP, ISFAST_NO, IOWIDTH_SINGLE, DUMMYCOUNT_0);
}
// 4 x I/O Page Program (4PP) Sequence (Command 38)
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_cmd_write_q1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("mxic_cmd_write_q1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_4PP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_4PP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_4PP, ISFAST_YES, IOWIDTH_QUAD, DUMMYCOUNT_0);
}
#endif

/****************************** SST ******************************/
/*
// Layer1 SST Auto Address Increment (AAI) Word-Program
#ifndef CONFIG_SPI_STD_MODE
void SstComSrlCmd_AAI_Start(unsigned char uiChip, unsigned int uiAddr, unsigned short usValue)
{
	unsigned int ui;
	ui = (SPICMD_SST_AAI << 24) | uiAddr;
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_L(uiChip, 1, 0);
	SPI_REG_LOAD(SFDR, usValue << 16);
	SFCSR_CS_H(uiChip, 0, 0);
	//ComSrlCmd_WRDI(uiChip);
	//spiFlashReady(uiChip);
	LDEBUG("SstComSrlCmd_AAI_Start: uiChip=%x; uiAddr=%x; usValue=%x; SPICMD_SST_AAI=%x;\n", uiChip, uiAddr, usValue, SPICMD_SST_AAI);
}
#endif
// Layer1 SST Auto Address Increment (AAI) Word-Program
#ifndef CONFIG_SPI_STD_MODE
void SstComSrlCmd_AAI_Continue(unsigned char uiChip, unsigned short usValue)
{
	unsigned int ui;
	ui = usValue;
	ui = (SPICMD_SST_AAI << 24) | (ui << 8);
	SFCSR_CS_L(uiChip, 2, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_H(uiChip, 0, 0);
	//ComSrlCmd_WRDI(uiChip);
	//spiFlashReady(uiChip);
	LDEBUG("SstComSrlCmd_AAI_Continue: uiChip=%x; usValue=%x; SPICMD_SST_AAI=%x;\n", uiChip, usValue, SPICMD_SST_AAI);
}
#endif
*/
// Layer1 SST Byte-Program
#ifndef CONFIG_SPI_STD_MODE
void SstComSrlCmd_BP(unsigned char uiChip, unsigned int uiAddr, unsigned char ucValue)
{
	unsigned int ui;
	ui = (SPICMD_SST_BP << 24) | (uiAddr & 0x00ffffff);
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (ucValue<< 24));
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("SstComSrlCmd_BP: uiChip=%x; uiAddr=%x; ucValue=%x; SPICMD_SST_BP=%x;\n", uiChip, uiAddr, ucValue, SPICMD_SST_BP);
}
#endif
// Read at High Speed (FAST_READ) Sequence (Command 0B)
#ifndef CONFIG_SPI_STD_MODE
unsigned int sst_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("sst_cmd_read_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_FASTREAD=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_FASTREAD);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_FASTREAD, ISFAST_YES, IOWIDTH_SINGLE, DUMMYCOUNT_1);
}
#endif
// Layer2 Sector Write Use BP Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int sst_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned int i, ui;
	unsigned char* puc = pucBuffer;
	LDEBUG("sst_cmd_write_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; returnValue=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, uiLen);
	for (i = 0; i < uiLen; i++)
	{
		ComSrlCmd_WREN(uiChip);
		SstComSrlCmd_BP(uiChip, uiAddr, *puc);
		ui = spiFlashReady(uiChip);
		puc += 1;
		uiAddr = uiAddr + 1;
	}
	return uiLen;
}
#endif

/****************************** Spansion ******************************/
//Layer1 Spansion S25FL032A S25FL064P Sector or Block Erase
#ifndef CONFIG_SPI_STD_MODE
unsigned int SpanComSrlCmd_SE(unsigned char uiChip, unsigned int uiAddr)
{
	unsigned int ui;
	if(uiAddr < 0x20000)		// the first two block use 4k erase cmd=0x20
	{
		return ComSrlCmd_SE(uiChip, uiAddr);
	}
	else
	{
		ui = uiAddr % 0x10000;
		if(ui < 0x1000)			// use block erase 0xd8, the whole bloce erase only once.
		{
			return ComSrlCmd_BE(uiChip, uiAddr);
		}
		else
		{
			return spiFlashReady(uiChip);
		}
	}	
}
#endif
// Layer1 Spansion Write Registers (WRR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int SpanComSrlCmd_WRR(unsigned char uiChip, unsigned int uiLen, unsigned int uiValue)
{
	unsigned int ui = (SPICMD_SPAN_WRR << 24) | (uiValue << ((uiLen == 1) ? 16 : 8));
	SFCSR_CS_L(uiChip, uiLen, 0);
	SPI_REG_LOAD(SFDR,ui);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("SpanComSrlCmd_WRR: uiChip=%x; uiValue=%x; Configure=%x; SPICMD_SPAN_WRR=%x;\n", uiChip, uiValue, SpanComSrlCmd_RCR(uiChip, 4), SPICMD_SPAN_WRR);
	return ui;
}
#endif
// Layer1 Spansion Read Configuration Register (RCR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int SpanComSrlCmd_RCR(unsigned char uiChip, unsigned int uiLen)
{
	unsigned int ui;
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_SPAN_RCR << 24));
	SFCSR_CS_L(uiChip, uiLen-1, 0);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(uiChip, 0, 0);
	ui = ui >> ((4 - uiLen) * 8);
	LDEBUG("SpanComSrlCmd_RCR: uiChip=%x; uiLen=%x; returnValue=%x; SPICMD_SPAN_RCR=%x;\n", uiChip, uiLen, ui, SPICMD_SPAN_RCR);
	return ui;
}
#endif
// Layer2 Spansion Set QE bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_spi_setQEBit(unsigned char uiChip)
{
	unsigned int ui;
	ComSrlCmd_WREN(uiChip);
	// Status
	//ui = (0 << SPAN_STATUS_SRWD) | (0 << SPAN_STATUS_PERR) | (0 << SPAN_STATUS_EERR) | (0 << SPAN_STATUS_BP2) | (0 << SPAN_STATUS_BP1) | (0 << SPAN_STATUS_BP0) | (0 << SPAN_STATUS_WEL) | (0 << SPANSON_STATUS_WIP);
	// Configure
	//ui = (ui << 8) | (0 << SPAN_CONF_TBPROT) | (0 << SPAN_CONF_BPNV) |(0 << SPAN_CONF_TBPARM) | (1 << SPAN_CONF_QUAD ) | (0 << SPAN_CONF_FREEZE);
	ui = 1 << SPAN_CONF_QUAD;
	SpanComSrlCmd_WRR(uiChip, 2, ui);
	KDEBUG("SpanSetQEBit: uiChip=%d; statusRegister=%x; returnValue=%x\n", uiChip, SpanComSrlCmd_RCR(uiChip, 2), ui);
	return spiFlashReady(uiChip);
}
#endif
// S25FL016A Layer1 Spansion FASTREAD Read Mode (Single IO)
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("span_cmd_read_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_SPAN_FASTREAD=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_SPAN_FASTREAD);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer,SPICMD_SPAN_FASTREAD, ISFAST_YES, IOWIDTH_SINGLE, DUMMYCOUNT_1);
}
#endif
// S25FL032A Layer1 Spansion Quad Output Read Mode (QOR) 
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_read_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("span_cmd_read_q0: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_SPAN_QOR=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_SPAN_QOR);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer,SPICMD_SPAN_QOR, ISFAST_NO, IOWIDTH_QUAD, DUMMYCOUNT_1);
}
#endif
// Layer1 Spansion Single IO Program (PP)
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("span_cmd_write_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_SPAN_PP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_SPAN_PP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_SPAN_PP, ISFAST_YES, IOWIDTH_SINGLE, DUMMYCOUNT_0);
}
#endif
// Layer1 Spansion QUAD Page Program (QPP)
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_write_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("span_cmd_write_q0: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_SPAN_QPP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_SPAN_QPP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_SPAN_QPP, ISFAST_NO, IOWIDTH_QUAD, DUMMYCOUNT_0);
}
#endif

/****************************** Winbond ******************************/
// Layer3 Winbond Set QE Bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int wb_spi_setQEBit(unsigned char uiChip)
{
	//unsigned int ui, uiA, uiB;
	unsigned int ui;
	ComSrlCmd_WREN(uiChip);
	//uiA = (0 << WB_STATUS_BUSY) | (0 << WB_STATUS_WEL) | (0 << WB_STATUS_BP0) | (0 << WB_STATUS_BP1) | (0 << WB_STATUS_BP2) | (0 << WB_STATUS_TB) | (0 << WB_STATUS_SEC) | (0 << WB_STATUS_SRP0);
 	//uiB = (0 << WB_STATUS_SRP1) | (1 << WB_STATUS_QE) | (0 << WB_STATUS_S10) | (0 << WB_STATUS_S11) | (0 << WB_STATUS_S12) | (0 << WB_STATUS_S13) | (0 << WB_STATUS_S14) | (0 << WB_STATUS_SUS);
	//ui = (uiA << 8) | (uiB >> 8);
	ui = (1 << WB_STATUS_QE) >> 8;
	SpanComSrlCmd_WRR(uiChip, 2, ui);
	KDEBUG("WBSetQEBit: uiChip=%d; statusRegister=%x; returnValue=%x\n", uiChip, SpanComSrlCmd_RCR(uiChip, 2), ui);
	return spiFlashReady(uiChip);
}
#endif

// W25Q80 W25Q16 W25Q32 4 x I/O Read Mode Sequence (Command EB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int wb_cmd_read_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("wb_cmd_read_q1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_WB_4READ=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_WB_4READ);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_WB_4READ, ISFAST_NO, IOWIDTH_QUAD, DUMMYCOUNT_1);
}
#endif
// 4 x I/O Page Program (4PP) Sequence (Command 38)
#ifndef CONFIG_SPI_STD_MODE
unsigned int wb_cmd_write_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("wb_cmd_write_q1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_WB_QPP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_WB_QPP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_WB_QPP, ISFAST_NO, IOWIDTH_QUAD, DUMMYCOUNT_0);
}
#endif
/****************************** Eon ******************************/
/*
// Read Status Register (RDSR) Sequence (Command 05) Quad IO Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int ComSrlCmd_RDSR4(unsigned char uiChip, unsigned int uiLen)
{
	unsigned int ui;
	SFCSR_CS_L(uiChip, 0, 2);
	SPI_REG_LOAD(SFDR, (SPICMD_RDSR << 24));
	SFCSR_CS_L(uiChip, uiLen-1, 2);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(uiChip, 0, 2);
	ui = ui >> ((4 - uiLen) * 8);
	LDEBUG("ComSrlCmd_RDSR4: uiChip=%x; uiLen=%x; returnValue=%x; SPICMD_RDSR=%x;\n", uiChip, uiLen, ui, SPICMD_RDSR);
	return ui;
}
#endif
// check WIP bit Quad IO Mode
unsigned int spiFlashReady4(unsigned char uiChip)
#ifndef CONFIG_SPI_STD_MODE
{
	unsigned int uiCount, ui;	
	uiCount = 0;
	while (1)
	{
		uiCount++;
		ui = ComSrlCmd_RDSR4(uiChip, 1);
		if ((ui & (1 << SPI_STATUS_WIP)) == 0)
		{
			break;
		}
	}
	LDEBUG("spiFlashReady4: uiCount=%x\n", uiCount);	
	return uiCount;
}
#endif
// Layer1 Eon Enable Quad I/O (EQIO) (38h)
#ifndef CONFIG_SPI_STD_MODE
void EonComSrlCmd_EQIO(unsigned char uiChip)
{
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, SPICMD_EON_EQIO << 24);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("EonComSrlCmd_EQIO: uiChip=%x; SPICMD_EON_EQIO=%x;\n", uiChip, SPICMD_EON_EQIO);
}
#endif
// Layer1 Eon Reset Quad I/O (RSTQIO) (FFh)
#ifndef CONFIG_SPI_STD_MODE
void EonComSrlCmd_RSTQIO(unsigned char uiChip)
{
	//SFCSR_CS_L(uiChip, 0, 2);
	//SPI_REG_LOAD(SFDR, SPICMD_EON_RSTQIO << 24);
	//SFCSR_CS_H(uiChip, 0, 2);
	SFCSR_CS_L(uiChip, 3, 2);
	SPI_REG_LOAD(SFDR, 0xffffffff);
	SFCSR_CS_H(uiChip, 3, 2);
	LDEBUG("EonComSrlCmd_RSTQIO: uiChip=%x; SPICMD_EON_RSTQIO=%x;\n", uiChip, SPICMD_EON_RSTQIO);
}
#endif
*/
// Eon read Single IO
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("eon_cmd_read_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_EON_FASTREAD=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_EON_FASTREAD);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_EON_FASTREAD, ISFAST_YES, IOWIDTH_SINGLE, DUMMYCOUNT_1);
}
#endif
// Eon Read Quad IO
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_read_q1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("eon_cmd_read_q1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_EON_4FASTREAD=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_EON_4FASTREAD);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_EON_4FASTREAD, ISFAST_YES, IOWIDTH_QUAD, DUMMYCOUNT_3);
}
#endif
// Page Program (PP) Sequence (Command 02)
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("eon_cmd_write_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_EON_PP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_EON_PP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_EON_PP, ISFAST_NO, IOWIDTH_SINGLE, DUMMYCOUNT_0);
}
#endif
/*
// Layer1 Eon Page Program (PP) (02h) under QIO Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_write_q2(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned int ui, uiCount, i;
	unsigned char* puc = pucBuffer;
	LDEBUG("EonComSrlCmd_4PP: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_EON_PP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_EON_PP);
	ComSrlCmd_WREN(uiChip);
	EonComSrlCmd_EQIO(uiChip);

	SFCSR_CS_L(uiChip, 0, 2);
	SPI_REG_LOAD(SFDR, (SPICMD_EON_PP << 24));			// Command
	SFCSR_CS_L(uiChip, 2, 2);
	SPI_REG_LOAD(SFDR, (uiAddr << 8));					// Address
	SFCSR_CS_L(uiChip, 3, 2);

	uiCount = uiLen / 4;
	for (i = 0; i <  uiCount; i++)
	{
		memcpy(&ui, puc, 4);
		puc += 4;
		SPI_REG_LOAD(SFDR, ui);
	}

	i = uiLen % 4;
	if(i > 0)
	{
		SFCSR_CS_L(uiChip, (i - 1), 2);
		memcpy(&ui, puc, i);
		puc += i;
		SPI_REG_LOAD(SFDR, ui);
	}
	SFCSR_CS_H(uiChip, 0, 2);
	EonComSrlCmd_RSTQIO(uiChip);
	ui = spiFlashReady(uiChip);
	return uiLen;
}
#endif
*/
/****************************** Giga Device ******************************/
// Layer1 High Performance Mode (HPM) (A3H)
#ifndef CONFIG_SPI_STD_MODE
unsigned int GdComSrlCmd_HPM(unsigned char uiChip)
{
	unsigned int ui = (SPICMD_GD_HPM << 24);	// command adn 3 dummy
	SFCSR_CS_L(uiChip, 3, 0);
	SPI_REG_LOAD(SFDR,ui);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("GdComSrlCmd_HPM: uiChip=%x; SPICMD_GD_HPM=%x;\n", uiChip, SPICMD_GD_HPM);
	return ui;
}
#endif
// Layer1 GigaDevice Write Registers (WRSR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int GdComSrlCmd_WRSR(unsigned char uiChip, unsigned int uiLen, unsigned int uiValue)
{
	unsigned int ui = (SPICMD_GD_WRSR << 24) | (uiValue << ((uiLen == 1) ? 16 : 8));
	SFCSR_CS_L(uiChip, uiLen, 0);
	SPI_REG_LOAD(SFDR,ui);
	SFCSR_CS_H(uiChip, 0, 0);
	LDEBUG("GdComSrlCmd_WRSR: uiChip=%x; uiValue=%x; Configure=%x; SPICMD_GD_WRSR=%x;\n", uiChip, uiValue, GdComSrlCmd_RDSR(uiChip, 2), SPICMD_GD_WRSR);
	return ui;
}
#endif
// Layer1 Read Configuration Register (RCR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int GdComSrlCmd_RDSR(unsigned char uiChip, unsigned int uiLen)
{
	unsigned int ui;
	SFCSR_CS_L(uiChip, 0, 0);
	SPI_REG_LOAD(SFDR, (SPICMD_GD_RDSR1 << 24));
	SFCSR_CS_L(uiChip, uiLen-1, 0);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(uiChip, 0, 0);
	ui = ui >> ((4 - uiLen) * 8);
	LDEBUG("GdComSrlCmd_RDSR: uiChip=%x; uiLen=%x; returnValue=%x; SPICMD_GD_RDSR1=%x;\n", uiChip, uiLen, ui, SPICMD_GD_RDSR1);
	return ui;
}
#endif
// Set quad enable bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int gd_spi_setQEBit(unsigned char uiChip)
{
	unsigned int ui;
	ComSrlCmd_WREN(uiChip);

	//ui = (0 << GD_STATUS_WIP) | (0 << GD_STATUS_WEL) | (0 << GD_STATUS_BP0) | (0 << GD_STATUS_BP1) | (0 << GD_STATUS_BP2) | (0 << GD_STATUS_BP3) | ( 0 << GD_STATUS_BP4) | (0 << GD_STATUS_SRP0);
	//ui = (ui << 8) | (0 << (GD_STATUS_SRP1 - 8)) | 
	ui = 1 << (GD_STATUS_QE - 8);
	GdComSrlCmd_WRSR(uiChip, 2, ui);			// set Giga Devcie QE bit
	KDEBUG("gd_spi_setQEBit: uiChip=%d; statusRegister=%x; returnValue=%x\n", uiChip, GdComSrlCmd_RDSR(uiChip, 1), ui);
	return ui;
}
#endif
// GD25Q16 Read at Fast read Quad Sequence (Command EB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int gd_cmd_read_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	GdComSrlCmd_HPM(uiChip);
	KDEBUG("gd_cmd_read_q0: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_GD_READ4=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_GD_READ4);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_GD_READ4, ISFAST_NO, IOWIDTH_QUAD, DUMMYCOUNT_1);
}
#endif
// Page Program (PP) Sequence (Command 02)
#ifndef CONFIG_SPI_STD_MODE
unsigned int gd_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("gd_cmd_write_s1: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_GD_PP=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_PP);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_GD_PP, ISFAST_NO, IOWIDTH_SINGLE, DUMMYCOUNT_0);
}
#endif

/****************************** ATMEL ******************************/
// AT25DF161 Dual-Output Read Array(Command 3B)
#ifndef CONFIG_SPI_STD_MODE
unsigned int at_cmd_read_d0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("at_cmd_read_d0: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_AT_READ2=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_AT_READ2);
	return ComSrlCmd_ComRead(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_AT_READ2, ISFAST_NO, IOWIDTH_DUAL, DUMMYCOUNT_1);
}
#endif
// AT25DF161 Dual-Input Byte/Page Program(Command A2)
#ifndef CONFIG_SPI_STD_MODE
unsigned int at_cmd_write_d0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	LDEBUG("at_cmd_write_s0: uiChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_AT_PP2=%x;\n", uiChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_AT_PP2);
	return ComSrlCmd_ComWrite(uiChip, uiAddr, uiLen, pucBuffer, SPICMD_AT_PP2, ISFAST_NO, IOWIDTH_DUAL, DUMMYCOUNT_0);
}
#endif



