/* SPI Flash driver
 *
 * Written by sam (sam@realtek.com)
 * 2010-05-01
 *
 */

//#define CONFIG_SPI_STD_MODE 1
//#define SPI_KERNEL 1

typedef unsigned int (*FUNC_ERASE)(unsigned char uiChip, unsigned int uiAddr);
typedef unsigned int (*FUNC_READ)(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
typedef unsigned int (*FUNC_WRITE)(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
typedef unsigned int (*FUNC_SETQEBIT)(unsigned char uiChip);
typedef unsigned int (*FUNC_PAGEWRITE)(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);

struct spi_flash_type
{
	unsigned int	chip_id;
	unsigned char	mfr_id;
	unsigned char	dev_id;

	unsigned char	capacity_id;
	unsigned char	size_shift;

	unsigned char	device_size;        // 2 ^ N (bytes)
	unsigned int	chip_size;

	unsigned int	block_size;
	unsigned int	block_cnt;

	unsigned int	sector_size;
	unsigned int	sector_cnt;

	unsigned int	page_size;
	unsigned int	page_cnt;
	unsigned int	chip_clk;
	char*			chip_name;

	unsigned int	uiClk;
	FUNC_ERASE		pfErase;
	FUNC_WRITE		pfWrite;
	FUNC_READ		pfRead;
	FUNC_SETQEBIT	pfQeBit;
	FUNC_PAGEWRITE	pfPageWrite;
};

struct spi_flash_known
{
	unsigned int	uiChipId;
	unsigned int	uiDistinguish;
	unsigned int	uiCapacityId;
	unsigned int	uiBlockSize;
	unsigned int	uiSectorSize;
	unsigned int	uiPageSize;
	char*			pcChipName;
	unsigned int	uiClk;
#ifndef CONFIG_SPI_STD_MODE
	FUNC_ERASE		pfErase;
	FUNC_READ		pfRead;
	FUNC_SETQEBIT	pfQeBit;
	FUNC_PAGEWRITE	pfPageWrite;
#endif
};

#define SIZEN_01M	0x14
#define SIZEN_02M	0x15
#define SIZEN_04M	0x16
#define SIZEN_08M	0x17
#define SIZEN_16M	0x18
#define SIZEN_32M	0x19
#define SIZEN_64M	0x20
#define SIZEN_CAL	0xff
#define SIZE_256B	0x100
#define SIZE_004K	0x1000
#define SIZE_064K	0x10000


/* SPI Flash Configuration Register(SFCR) (0xb800-1200) */
#define SFCR					0xb8001200			/*SPI Flash Configuration Register*/		
#define SFCR_SPI_CLK_DIV(val)	((val) << 29)
#define SFCR_RBO(val)			((val) << 28)
#define SFCR_WBO(val)			((val) << 27)
#define SFCR_SPI_TCS(val)		((val) << 23)			/*4 bit, 1111 */

/* SPI Flash Configuration Register(SFCR2) (0xb800-1204) */
#define SFCR2						0xb8001204
#define SFCR2_SFCMD(val)			((val) << 24)			/*8 bit, 1111_1111 */
#define SFCR2_SFSIZE(val)			((val) << 21)			/*3 bit, 111 */
#define SFCR2_RD_OPT(val)			((val) << 20)
#define SFCR2_CMD_IO(val)			((val) << 18)			/*2 bit, 11 */
#define SFCR2_ADDR_IO(val)			((val) << 16)			/*2 bit, 11 */
#define SFCR2_DUMMY_CYCLE(val)		((val) << 13)			/*3 bit, 111 */
#define SFCR2_DATA_IO(val)			((val) << 11)			/*2 bit, 11 */
#define SFCR2_HOLD_TILL_SFDR2(val)	((val) << 10)

/* SPI Flash Control and Status Register(SFCSR)(0xb800-1208) */
#define SFCSR					0xb8001208
#define SFCSR_SPI_CSB0(val)		((val) << 31)
#define SFCSR_SPI_CSB1(val)		((val) << 30)		
#define SFCSR_LEN(val)			((val) << 28)			/*2 bits*/
#define SFCSR_SPI_RDY(val)		((val) << 27)
#define SFCSR_IO_WIDTH(val)		((val) << 25)			/*2 bits*/
#define SFCSR_CHIP_SEL(val)		((val) << 24)
#define SFCSR_CMD_BYTE(val)		((val) << 16)			/*8 bit, 1111_1111 */

#define SFCSR_SPI_CSB(val)		((val) << 30)

/* SPI Flash Data Register(SFDR)(0xb800-120c) */
#define SFDR					0xb800120c

/* SPI Flash Data Register(SFDR2)(0xb8001210) */
#define SFDR2					0xb8001210


#define SPI_BLOCK_SIZE			0x10000				/* 64KB */
#define SPI_SECTOR_SIZE			0x1000				/* 4KB */
#define SPI_PAGE_SIZE			0x100				/* 256B */



#define SPICMD_WREN			0x06	/* 06 xx xx xx xx sets the (WEL) write enable latch bit */
#define SPICMD_WRDI			0x04	/* 04 xx xx xx xx resets the (WEL) write enable latch bit*/
#define SPICMD_RDID			0x9f	/* 9f xx xx xx xx outputs JEDEC ID: 1 byte manufacturer ID & 2 byte device ID */
#define SPICMD_RDSR			0x05	/* 05 xx xx xx xx to read out the values of the status register */
#define SPICMD_WRSR			0x01	/* 01 xx xx xx xx to write new values to the status register */
#define SPICMD_READ			0x03	/* 03 a1 a2 a3 xx n bytes read out until CS# goes high */
#define SPICMD_FASTREAD		0x0b	/* 0b a1 a2 a3 dd n bytes read out until CS# goes high */
#define SPICMD_2READ		0xbb	/* bb 12 3d xx xx n bytes read out by 2 I/O until CS# goes high */
#define SPICMD_4READ		0xeb	/* eb 3a 3d xx xx n bytes read out by 4 x I/O until CS# goes high */
#define SPICMD_SE			0x20	/* 20 a1 a2 a3 xx to erase the selected sector */
#define SPICMD_BE			0xd8	/* d8 a1 a2 a3 xx to erase the selected block */
#define SPICMD_CE			0x60	/* 60 xx xx xx xx to erase whole chip (cmd or 0xc7) */
#define SPICMD_PP			0x02	/* 02 a1 a2 a3 xx to program the selected page */
#define SPICMD_4PP			0x38	/* 38 3a 3d xx xx quad input to program the selected page */
#define SPICMD_CP			0xad	/* ad a1 a2 a3 xx continously program whole chip, the address is automaticlly increase */
#define SPICMD_DP			0xb9	/* b9 xx xx xx xx enters deep power down mode */
#define SPICMD_RDP			0xab	/* ab xx xx xx xx release from deep power down mode */
#define SPICMD_RES			0xab	/* ab ?? ?? ?? xx to read out 1 byte device ID */
#define SPICMD_REMS_90		0x90	/* 90 ?? ?? ?? xx output the manufacter ID & device ID */
#define SPICMD_REMS_EF		0xef	/* ef ?? ?? ?? xx output the manufacter ID & device ID */
#define SPICMD_REMS_DF		0xdf	/* df ?? ?? ?? ?? output the manufacture ID & device ID */
#define SPICMD_ENSO			0xb1	/* b1 xx xx xx xx to enter the 512 bit secured OTP mode */
#define SPICMD_EXSO			0xc1	/* c1 xx xx xx xx to exit the 512 bit secured OTP mode */
#define SPICMD_RDSCUR		0x2b	/* 2b xx xx xx xx to read value of secured register */
#define SPICMD_WRSCUR		0x2f	/* 2f xx xx xx xx to set the lock down bit as "1" (once lock down, can not be updated) */
#define SPICMD_ESRY			0x70	/* 70 xx xx xx xx to enable SO to output RY/BY# during CP mode */
#define SPICMD_DSRY			0x80	/* 80 xx xx xx xx to disable SO to output RY/BY# during CP mode */

#define SPI_STATUS_REG_SRWD		0x07	/* status register write protect */
#define SPI_STATUS_CP			0x06	/* continously program mode */
#define SPI_STATUS_QE			0x06	/* quad enable */
#define SPI_STATUS_BP3			0x05	/* level of protected block */
#define SPI_STATUS_BP2			0x04	/* level of protected block */
#define SPI_STATUS_BP1			0x03	/* level of protected block */
#define SPI_STATUS_BP0			0x02	/* level of protected block */
#define SPI_STATUS_WEL			0x01	/* write enable latch */
#define SPI_STATUS_WIP			0x00	/* write in process bit */

/****** EON ******/
#define SPICMD_EON_EQIO		0x38	/* enable the flash device for Quad SPI bus operation */
#define SPICMD_EON_RSTQIO	0xff	/* resets the device to 1-bit Standard SPI operation */

#define EON_STATUS_SRP		0x07	/* SRP Status Register P// write sector use malloc buffer */
#define EON_STATUS_WPDIS	0x06	/* (WP# disable) 1 = WP# disable 0 = WP# enable */
#define EON_STATUS_BP3		0x05	/* Block Protected bits */
#define EON_STATUS_BP2		0x04	/* Block Protected bits */
#define EON_STATUS_BP1		0x03	/* Block Protected bits */
#define EON_STATUS_BP0		0x02	/* Block Protected bits */
#define EON_STATUS_WEL		0x01	/* (Write Enable Latch) 1 = write enable 0 = not write enable */
#define EON_STATUS_WIP		0x00	/* WIP (Write In Progress bit) 1 = write operation 0 = not in write operation */

#define SPICMD_EON_EQIO		0x38	/* Enable Quad I/O (EQIO) (38h) */
#define SPICMD_EON_RSTQIO	0xff	/* Reset Quad I/O (RSTQIO) (FFh) */
#define SPICMD_EON_WREN		0x06	/* Write Enable (WREN) (06h) */
#define SPICMD_EON_WRDI		0x04	/* Write Disable (WRDI) (04h) */
#define SPICMD_EON_RDSR		0x05	/* Read Status Register (RDSR) (05h) */
#define SPICMD_EON_WRSR		0x01	/* Write Status Register (WRSR) (01h) */
#define SPICMD_EON_READ			0x03	/* Read Data Bytes (READ) (03h) */
#define SPICMD_EON_FASTREAD		0x0b	/* Read Data Bytes at Higher Speed (FAST_READ) (0Bh) */
#define SPICMD_EON_2READ		0x3b	/* Dual Output Fast Read (3Bh) */
#define SPICMD_EON_2FASTREAD	0xbb	/* Dual Input / Output FAST_READ (BBh) */
#define SPICMD_EON_4FASTREAD	0xeb	/* Quad Input / Output FAST_READ (EBh) */
#define SPICMD_EON_PP			0x02	/* Page Program (PP) (02h) */
#define SPICMD_EON_SE			0x20	/* Sector Erase (SE) (20h) */
#define SPICMD_EON_BE			0xd8	/* Block Erase (BE) (D8h) */
#define SPICMD_EON_CE			0x60	/* Chip Erase (CE) (C7h/60h) */
#define SPICMD_EON_DP			0xb9	/* Deep Power-down (DP) (B9h) */
#define SPICMD_EON_RDI			0xab	/* Release from Deep Power-down and Read Device ID (RDI) */
#define SPICMD_EON_DEVID		0x90	/* Read Manufacturer / Device ID (90h) */
#define SPICMD_EON_RDID			0x9f	/* Read Identification (RDID) (9Fh) */
#define SPICMD_EON_OTP			0x3a	/* Enter OTP Mode (3Ah) */
#define SPICMD_EON_EOTP			0x20	/* Erase OTP Command (20h) */


/****** SPANSION ******/
#define SPICMD_SPAN_READ		0x03	/* 3 0 0 (1 to ∞) Read Data bytes */
#define SPICMD_SPAN_FASTREAD	0x0b	/* 3 0 1 (1 to ∞) Read Data bytes at Fast Speed */
#define SPICMD_SPAN_DOR			0x3b	/* 3 0 1 (1 to ∞) Dual Output Read */
#define SPICMD_SPAN_QOR			0x6b	/* 3 0 1 (1 to ∞) Quad Ou// write sector use malloc buffer */
#define SPICMD_SPAN_DIOR		0xbb	/* 3 1 0 (1 to ∞) Dual I/O High Performance Read */
#define SPICMD_SPAN_QIOR		0xeb	/* 3 1 2 (1 to ∞) Quad I/O High Performance Read */
#define SPICMD_SPAN_RDID		0x9f	/* 0 0 0 (1 to 81) Read Identification */
#define SPICMD_SPAN_READ_ID		0x90	/* 3 0 0 (1 to ∞) Read Manufacturer and Device Identification */
#define SPICMD_SPAN_WREN		0x06	/* 0 0 0 (0) Write Enable */
#define SPICMD_SPAN_WRDI		0x04	/* 0 0 0 (0) Write Disable */
#define SPICMD_SPAN_P4E			0x20	/* 3 0 0 (0) 4 KB Parameter Sector Erase */
#define SPICMD_SPAN_P8E			0x40	/* 3 0 0 (0) 8 KB (two 4KB) Parameter Sector Erase */
#define SPICMD_SPAN_SE			0xd8	/* 3 0 0 (0) 64KB Sector Erase */
#define SPICMD_SPAN_BE			0x60	/* 0 0 0 (0) Bulk Erase or 0xc7 */
#define SPICMD_SPAN_PP			0x02	/* 3 0 0 (1 to 256) Page Programming */
#define SPICMD_SPAN_QPP			0x32	/* 3 0 0 (1 to 256) Quad Page Programming */
#define SPICMD_SPAN_RDSR		0x05	/* 0 0 0 (1 to ∞) Read Status Register */
#define SPICMD_SPAN_WRR			0x01	/* 0 0 0 (1 to 2) Write (Status & Configuration) Register */
#define SPICMD_SPAN_RCR			0x35	/* 0 0 0 (1 to ∞) Read Configuration Register (CFG) */
#define SPICMD_SPAN_CLSR		0x30	/* 0 0 0 (1) Reset the Erase and Program Fail Flag (SR5 and SR6) and restore normal operation */
#define SPICMD_SPAN_DP			0xb9	/* 0 0 0 (0) Deep Power-Down */
#define SPICMD_SPAN_RES			0xab	/* 0 0 0 (0) or ) Release from Deep Power-Down Mode */
										/* 0 0 3 (1 to ∞) Release from Deep Power-Down and Read Electronic Signature */
#define SPICMD_SPAN_OTPP		0x42	/* 3 0 0 (1) Program one byte of data in OTP memory space */
#define SPICMD_SPAN_OTPR		0x4b	/* 3 0 1 (1 to ∞) Read data in the OTP memory space */
/* 5 TBPROT Configures start of block protection, 1 = Bottom Array (low address), 0 = Top Array (high address) (Default) */
#define SPAN_CONF_TBPROT	0x05
/* 3  BPNV  Configures BP2-0 bits in the Status Register, 1 = Volatile, 0 = Non-volatile (Default) */
#define SPAN_CONF_BPNV		0x03
/* 2 TBPARM Configures Parameter sector location, 1 = Top Array (high address), 0 = Bottom Array (low address) (Default) */
#define SPAN_CONF_TBPARM	0x02
/* 1  QUAD  Puts the device into Quad I/O mode, 1 = Quad I/O, 0 = Dual or Serial I/O (Default) */
#define SPAN_CONF_QUAD		0x01
/* 0 FREEZE Locks BP2-0 bits in the Status Register, 1 = Enabled, 0 = Disabled (Default) */
#define SPAN_CONF_FREEZE	0x00
/* 7 SRWD Status Register Write Disable, 1 = Protects when W#/ACC is low, 0 = No protection, even when W#/ACC is low */
#define SPAN_STATUS_SRWD	0x07
/* 6 P_ERR Programming Error Occurred, 0 = No Error, 1 = Error occurred */
#define SPAN_STATUS_PERR	0x06
/* 5 E_ERR Erase Error Occurred, 0 = No Error, 1 = Error occurred */
#define SPAN_STATUS_EERR	0x05
/* 4 BP2 Block Protect Protects selected Block from Program or Erase */
#define SPAN_STATUS_BP2		0x04
/* 3 BP1 Block Protect Protects selected Block from Program or Erase */
#define SPAN_STATUS_BP1		0x03
/* 2 BP0 Block Protect Protects selected Block from Program or Erase */
#define SPAN_STATUS_BP0		0x02
/* 1 WEL Write Enable Latch, 1 = Device accepts Write Registers, program or erase commands, 0 = Ignores Write Registers, program or erase commands */
#define SPAN_STATUS_WEL		0x01
/* 0 WIP Write in Progress, 1 = Device Busy a Write Registers, program or erase operation is in progress, 0 = Ready. Device is in standby mode and can accept commands. */
#define SPANSON_STATUS_WIP	0x00

/****** WINBOND ******/

#define WB_STATUS_BUSY		0x00	/* ERASE/WRITE IN PROGRESS */
#define WB_STATUS_WEL		0x01	/* WRITE ENABLE LATCH */
#define WB_STATUS_BP0		0x02	/* BLOCK PROTECT BITS 0 (non-volatile) */
#define WB_STATUS_BP1		0x03	/* BLOCK PROTECT BITS 1 (non-volatile) */
#define WB_STATUS_BP2		0x04	/* BLOCK PROTECT BITS 2 (non-volatile) */
#define WB_STATUS_TB		0x05	/* TOP/BOTTOM PROTECT (non-volatile) */
#define WB_STATUS_SEC		0x06	/* SECTOR PROTECT (non-volatile) */
#define WB_STATUS_SRP0		0x07	/* STATUS REGISTER PROTECT 0 (non-volatile) */
#define WB_STATUS_SRP1		0x08	/* STATUS REGISTER PROTECT 1 (non-volatile) */
#define WB_STATUS_QE		0x09	/* QUAD ENABLE (non-volatile) */
#define WB_STATUS_S10		0x0a	/* RESERVED */
#define WB_STATUS_S11		0x0b	/* RESERVED */
#define WB_STATUS_S12		0x0c	/* RESERVED */
#define WB_STATUS_S13		0x0d	/* RESERVED */
#define WB_STATUS_S14		0x0e	/* RESERVED */
#define WB_STATUS_SUS		0x0f	/* SUSPEND STATUS */

/*                                     INSTRUCTION NAME        	BYTE 1		BYTE 2 		BYTE 3 		BYTE 4		BYTE 5		BYTE 6*/
#define SPICMD_WB_WREN		0x06	/* Write Enable				06h */
#define SPICMD_WB_WRDI		0x04	/* Write Disable				04h */
#define SPICMD_WB_RDSR		0x05	/* Read Status Register-1		05h			(S7–S0) */
#define SPICMD_WB_RDSR2		0x35	/* Read Status Register-2		35h			(S15-S8) */
#define SPICMD_WB_WRSR		0x01	/* Write Status Register		01h			(S7–S0)		(S15-S8) */
#define SPICMD_WB_PP		0x02	/* Page Program				02h			A23–A16		A15–A8		A7–A0		(D7–D0) */
#define SPICMD_WB_QPP		0x32	/* Quad Page Program			32h			A23–A16		A15–A8		A7–A0		(D7–D0, ...) */
#define SPICMD_WB_SE		0x20	/* Sector Erase (4KB)			20h			A23–A16		A15–A8		A7–A0 */
#define SPICMD_WB_BE		0x52	/* Block Erase (32KB)			52h			A23–A16		A15–A8		A7–A0 */
#define SPICMD_WB_BE64		0xd8	/* Block Erase (64KB)			D8h			A23–A16		A15–A8		A7–A0 */
#define SPICMD_WB_CE		0x60	/* Chip Erase					C7h/60h */
#define SPICMD_WB_ES		0x75	/* Erase Suspend				75h */
#define SPICMD_WB_ER		0x7a	/* Erase Resume				7Ah */
#define SPICMD_WB_PD		0xb9	/* Power-down					B9h */
#define SPICMD_WB_CRMR		0xff	/* Continuous Read Mode Reset	FFh			FFh */
#define SPICMD_WB_READ		0x03	/* Read Data					03h			A23-A16		A15-A8		A7-A0		(D7-D0) */
#define SPICMD_WB_FASTREAD	0x0b	/* Fast Read					0Bh			A23-A16		A15-A8		A7-A0		dummy		(D7-D0) */
#define SPICMD_WB_2READ		0x3b	/* Fast Read Dual Output		3Bh			A23-A16		A15-A8		A7-A0		dummy		(D7-D0, ...)*/
#define SPICMD_WB_2FASTREAD	0xbb	/* Fast Read Dual I/O			BBh			A23-A8		A7-A0		M7-M0		(D7-D0, ...) */
#define SPICMD_WB_4READ		0x6b	/* Fast Read Quad Output		6Bh			A23-A16		A15-A8		A7-A0		dummy		(D7-D0, ...) */
#define SPICMD_WB_4FASTREAD	0xeb	/* Fast Read Quad I/O			EBh			A23-A0		M7-M0		(x,x,x,x, D7-D0, ...)	(D7-D0, ...) */
#define SPICMD_WB_4WREAD	0xe7	/* Word Read Quad I/O			E7h			A23-A0		M7-M0		(x,x, D7-D0, ...)		(D7-D0, ...) */
#define SPICMD_WB_WREAD		0xe3	/* Octal Word Read				E3h			A23-A0		M7-M0		(D7-D0, ...) */
#define SPICMD_WB_DI		0xab	/* Device ID					ABh			dummy		dummy		dummy		(ID7-ID0) */
#define SPICMD_WB_MDI		0x90	/* Manufacturer/Device ID		90h			dummy		dummy		00h			(MF7-MF0)	(ID7-ID0) */
#define SPICMD_WB_2MDI		0x92	/* Manufacturer/Device ID		92h			A23-A8		A7-A0		M[7:0]		(MF[7:0], ID[7:0])		(by Dual I/O) */
#define SPICMD_WB_4MDI		0x94	/* Manufacture/Device ID		94h			A23-A0		M[7:0] xxxx	(MF[7:0]	ID[7:0])	(MF[7:0], ID[7:0], ...)	(by Quad I/O) */
#define SPICMD_WB_RDID		0x9f	/* JEDEC ID					9Fh			(MF7-MF0)	(ID15-ID8)	(ID7-ID0)	(Manufacturer Memory Type Capacity) */
#define SPICMD_WB_RUID		0x4b	/* Read Unique ID				4Bh			dummy		dummy		dummy		dummy		(ID63-ID0)	*/

/****** SST ******/
#define SST_STATUS_BUSY		0x00	/* 0 BUSY 1 = Internal Write operation is in progress 0  R */
#define SST_STATUS_WEL		0x01	/* 1 WEL  1 = Device is memory Write enabled */
#define SST_STATUS_BP0		0x02	/* 2 BP0  Indicate current level of block write protection (See Table 4) 1 R/W */
#define SST_STATUS_BP1		0x03	/* 3 BP1  Indicate current level of block write protection (See Table 4) 1 R/W */
#define SST_STATUS_BP2		0x04	/* 4 BP2  Indicate current level of block write protection (See Table 4) 1 R/W */
#define SST_STATUS_BP3		0x05	/* 5 BP3  Indicate current level of block write protection (See Table 4) 0 R/W */
#define SST_STATUS_AAI		0x06	/* 6 AAI  Auto Address Increment Programming status 0  R 1 = AAI programming mode 0 = Byte-Program mode */
#define SST_STATUS_BPL		0x07	/* 7 BPL  1 = BP3, BP2, BP1, BP0 are read-only bits 0 R/W 0 = BP3, BP2, BP1, BP0 are readable/writable */

#define SPICMD_SST_READ		0x03		/* Read Read Memory 0000 0011b (03H) 3 0 25 MHz */
#define SPICMD_SST_FASTREAD	0x0b		/* High-Speed Read Read Memory at higher speed 0000 1011b (0BH) 3 1 80 MHz */
#define SPICMD_SST_SE		0x20		/* KByte Sector-Erase3 Erase 4 KByte of 0010 0000b (20H) 3 0 0 80 MHz */
#define SPICMD_SST_BE32		0x52		/* 32 KByte Block-Erase4 Erase 32KByte block 0101 0010b (52H) 3 0 0 80 MHz */
#define SPICMD_SST_BE64		0xd8		/* 64 KByte Block-Erase5 Erase 64 KByte block 1101 1000b (D8H) 3 0 0 80 MHz */
#define SPICMD_SST_CE		0x60		/* Chip-Erase Erase Full Memory Array 0110 0000b (60H) or 1100 0111b (C7H) 0 0 0 80 MHz */
#define SPICMD_SST_BP		0x02		/* Byte-Program To Program One Data Byte 0000 0010b (02H) 3 0 1 80 MHz */
#define SPICMD_SST_AAI		0xad		/* AAI-Word-Program Auto Address Increment 1010 1101b (ADH) 3 0 80 MHz */
#define SPICMD_SST_RDSR		0x05		/* RDSR Read-Status-Register 0000 0101b (05H) 0 0 80 MHz */
#define SPICMD_SST_EWSR		0X50		/* Enable-Write-Status-Register 0101 0000b (50H) 0 0 0 80 MHz */
#define SPICMD_SST_WRSR		0x01		/* Write-Status-Register 0000 0001b (01H) 0 0 1 80 MHz */
#define SPICMD_SST_WREN		0x06		/* Write-Enable 0000 0110b (06H) 0 0 0 80 MHz */
#define SPICMD_SST_WRDI		0x04		/* Write-Disable 0000 0100b (04H) 0 0 0 80 MHz */
#define SPICMD_SST_RDID		0x90		/* Read-ID 1001 0000b (90H) or 1010 1011b (ABH) 3 0 80 MHz */
#define SPICMD_SST_JEDECID	0x9f		/* JEDEC-ID JEDEC ID read 1001 1111b (9FH) 0 0 80 MHz */
#define SPICMD_SST_EBSY		0x70		/* Enable SO as an output RY/BY# status during AAI programming 0111 0000b (70H) 0 0 0 80 MHz */
#define SPICMD_SST_DBSY		0x80		/* Disable SO as an output RY/BY# status during AAI programming 1000 0000b (80H) 0 0 0 80 MHz */

/****** GigaDevice ******/
#define GD_STATUS_WIP		0x00	/* 0 WIP 1 = Internal Write operation is in progress 0  R */
#define GD_STATUS_WEL		0x01	/* 1 WEL  1 = Device is memory Write enabled */
#define GD_STATUS_BP0		0x02	/* 2 BP0  Indicate current level of block write protection (See Table 4) 1 R/W */
#define GD_STATUS_BP1		0x03	/* 3 BP1  Indicate current level of block write protection (See Table 4) 1 R/W */
#define GD_STATUS_BP2		0x04	/* 4 BP2  Indicate current level of block write protection (See Table 4) 1 R/W */
#define GD_STATUS_BP3		0x05	/* 5 BP3  Indicate current level of block write protection (See Table 4) 0 R/W */
#define GD_STATUS_BP4		0x06	/* 6 BP4  Indicate current level of block write protection (See Table 4) 0 R/W */
#define GD_STATUS_SRP0		0x07	/* 7 The SRP bits control the m// write sector use malloc buffer */
#define GD_STATUS_SRP1		0x08	/* 8 The SRP bits control the method of write protection */
#define GD_STATUS_QE		0x09	/* 9 When the QE bit is set to 0 (Default) the WP# pin and HOLD# pin are enable */

#define SPICMD_GD_WREN		0x06	/* Write Enable           06H */
#define SPICMD_GD_WRDI		0x04	/* Write Disable          04H */
#define SPICMD_GD_RDSR		0x05	/* Read Status Register   05H     (S7-S0)                                               (continuous) */
#define SPICMD_GD_RDSR1		0x35	/* Read Status Register-1 35H     (S15-S8)                                              (continuous) */
#define SPICMD_GD_WRSR		0x01	/* Write Status Register  01H     (S7-S0)   (S15-S8) */
#define SPICMD_GD_READ		0x03	/* Read Data              03H     A23-A16   A15-A8    A7-A0       (D7-D0)   (Next byte) (continuous) */
#define SPICMD_GD_FASTREAD	0x0b	/* Fast Read              0BH     A23-A16   A15-A8    A7-A0       dummy     (D7-D0)     (continuous) */
#define SPICMD_GD_READ2		0x3b	/* Dual Output            3BH     A23-A16   A15-A8    A7-A0       dummy     (D7-D0)(1)  (continuous) */
#define SPICMD_GD_READ4		0x6b	/* Quad Output Fast Read  6BH     A23-A16   A15-A8    A7-A0       dummy     (D7-D0)(3)  (continuous) */
#define SPICMD_GD_FASTREAD2	0xbb	/* Dual I/O Fast Read     BBH     A23-A8(2) A7-A0                 dummy     (D7-D0)     (continuous) */
#define SPICMD_GD_FASTREAD4	0xeb    /* Quad I/O Fast Read     EBH     A23-A0(4)                       dummy     (D7-D0)(3)  (continuous) */
#define SPICMD_GD_FASTREADW4 0xe7	/* Quad I/O Word          E7H     A23-A0                          dummy     (D7-D0)(3)  (continuous) */

#define SPICMD_GD_CRMR		0xff	/* Continuous Read Reset  FFH */
#define SPICMD_GD_PP		0x02	/* Page Program           02H     A23-A16   A15-A8    A7-A0       D7-D0     Next byte */
#define SPICMD_GD_SE		0x20	/* Sector Erase           20H     A23-A16   A15-A8    A7-A0 */
#define SPICMD_GD_BE32		0x52	/* Block Erase(32K)       52H     A23-A16   A15-A8    A7-A0 */
#define SPICMD_GD_BE64		0xd8	/* Block Erase(64K)       D8H     A23-A16   A15-A8    A7-A0 */
#define SPICMD_GD_BE128		0xd2	/* Block Erase(128K)      D2H     A23-A16   A15-A8    A7-A0 */
#define SPICMD_GD_CE		0x60	/* Chip Erase             C7/60H */
#define SPICMD_GD_PES		0x75	/* Program/Erase Suspend  75H */
#define SPICMD_GD_PER		0x7a	/* Program/Erase Resume   7AH */
#define SPICMD_GD_DP		0xb9	/* Deep Power-Down        B9H */
#define SPICMD_GD_RDP		0xab	/* Release From Deep      ABH     dummy     dummy     dummy       (ID7-ID0)              (continuous) */
#define SPICMD_GD_REMS		0x90	/*  Manufacturer/Device ID */
#define SPICMD_GD_HPM		0xa3	/*  High Performance Mode A3H dummy   dummy      dummy */
#define SPICMD_GD_RDID		0x9f	/* Read Identification   9FH (M7-M0) (ID15-ID8) (ID7-ID0)                   (continuous) */


/****** ATMEL ******/
//                                                                                                Clock         Address    Dummy  Data
//Command                                                                     Opcode           Frequency         Bytes      Bytes Bytes
#define SPICMD_AT_FASTREAD1	0x1b	//Read Array							1Bh   0001 1011    Up to 100 MHz          3         2    1+
#define SPICMD_AT_FASTREAD	0x0b	//Read Array							0Bh   0000 1011    Up to 85 MHz           3         1    1+
#define SPICMD_AT_READ		0x03	//Read Array							03h   0000 0011    Up to 50 MHz           3         0    1+
#define SPICMD_AT_READ2		0x3b	//Dual-Output Read Array				3Bh   0011 1011    Up to 85 MHz           3         1    1+
#define SPICMD_AT_SE		0x20	//Block Erase (4 KBytes)				20h   0010 0000    Up to 100 MHz          3         0     0
#define SPICMD_AT_BE1		0x52	//Block Erase (32 KBytes)				52h   0101 0010    Up to 100 MHz          3         0     0
#define SPICMD_AT_BE		0xd8	//Block Erase (64 KBytes)				D8h   1101 1000    Up to 100 MHz          3         0     0
#define SPICMD_AT_CE		0x60	//Chip Erase							60h   0110 0000    Up to 100 MHz          0         0     0
//#define SPICMD_AT_CE		0xc7	//Chip Erase							C7h   1100 0111    Up to 100 MHz          0         0     0
#define SPICMD_AT_PP		0x02	//Byte/Page Program (1 to 256 Bytes)	02h   0000 0010    Up to 100 MHz          3         0    1+
#define SPICMD_AT_PP2		0xa2	//Dual-Input Byte/Page Program 			A2h   1010 0010    Up to 100 MHz          3         0    1+
#define SPICMD_AT_PES		0xb0	//Program/Erase Suspend					B0h   1011 0000    Up to 100 MHz          0         0     0
#define SPICMD_AT_PER		0xd0	//Program/Erase Resume					D0h   1101 0000    Up to 100 MHz          0         0     0
#define SPICMD_AT_WREN		0x06	//Write Enable							06h   0000 0110    Up to 100 MHz          0         0     0
#define SPICMD_AT_WRDI		0x04	//Write Disable							04h   0000 0100    Up to 100 MHz          0         0     0
#define SPICMD_AT_PS		0x36	//Protect Sector						36h   0011 0110    Up to 100 MHz          3         0     0
#define SPICMD_AT_UPS		0x39	//Unprotect Sector						39h   0011 1001    Up to 100 MHz          3         0     0
#define SPICMD_AT_RSPR		0x3c	//Read Sector Protection Registers		3Ch   0011 1100    Up to 100 MHz          3         0    1+
#define SPICMD_AT_SLD		0x33	//Sector Lockdown						33h   0011 0011    Up to 100 MHz          3         0     1
#define SPICMD_AT_FSLS		0x34	//Freeze Sector Lockdown State			34h   0011 0100    Up to 100 MHz          3         0     1
#define SPICMD_AT_RSLR		0x35	//Read Sector Lockdown Registers		35h   0011 0101    Up to 100 MHz          3         0    1+
#define SPICMD_AT_POSR		0x9b	//Program OTP Security Register			9Bh   1001 1011    Up to 100 MHz          3         0    1+
#define SPICMD_AT_ROSR		0x77	//Read OTP Security Register			77h   0111 0111    Up to 100 MHz          3         2    1+
#define SPICMD_AT_RSR		0x05	//Read Status Register					05h   0000 0101    Up to 100 MHz          0         0    1+
#define SPICMD_AT_WSR1		0x01	//Write Status Register Byte 1			01h   0000 0001    Up to 100 MHz          0         0     1
#define SPICMD_AT_WSR2		0x31	//Write Status Register Byte 2			31h   0011 0001    Up to 100 MHz          0         0     1
#define SPICMD_AT_RESET		0xf0	//Reset									F0h   1111 0000    Up to 100 MHz          0         0     1
#define SPICMD_AT_RDID		0x9f	//Read Manufacturer and Device ID		9Fh   1001 1111    Up to 85 MHz           0         0   1 to 4
#define SPICMD_AT_DP		0xb9	//Deep Power-Down						B9h   1011 1001    Up to 100 MHz          0         0     0
#define SPICMD_AT_RDP		0xab	//Resume from Deep Power-Down			ABh   1010 1011    Up to 100 MHz          0         0     0

#define AT_STATUS2_RES	5	//5:7RES   Reserved for future use  R  0 Reserved for future use.
#define AT_STATUS2_RSTE	4	//RSTE   Reset Enabled:0 Reset command is disabled (default);1 Reset command is enabled.           R/W
#define AT_STATUS2_SLE	3	//SLE   Sector Lockdown Enabled R/W
//0 Sector Lockdown and Freeze Sector Lockdown State commands are disabled (default).
//1 Sector Lockdown and Freeze Sector Lockdown State commands are enabled.
                                      
#define AT_STATUS2_PS	2	//PS   Program Suspend Status:0 No sectors are program suspended (default);1 A sector is program suspended.   R
#define AT_STATUS2_ES	1	//ES   Erase Suspend Status:0 No sectors are erase suspended (default);1 A sector is erase suspended.     R
#define AT_STATUS2_RDY	0	//RDY/BSY Ready/Busy Status:0 Device is ready;1 Device is busy with an internal operation.        R
#define AT_STATUS1_SPRL	7	//SPRL  Sector Protection Registers Locked R/W
//0 Sector Protection Registers are unlocked (default).
//1 Sector Protection Registers are locked.
#define AT_STATUS1_RES	6	//RES   Reserved for future use             R  0  Reserved for future use.
#define AT_STATUS1_EPE	5	//EPE   Erase/Program Error:0 Erase or program operation was successful;1 Erase or program error detected.R
#define AT_STATUS1_WPP	4	//WPP    Write Protect (WP) Pin Status:0 WP is asserted;1 WP is deasserted.       R
#define AT_STATUS1_SWP	2	//3:2  SWP    Software Protection Status          R 
//00 All sectors are software unprotected (all Sector Protection Registers are 0).
//01 Some sectors are software protected. Read individual Sector Protection Registers to determine which sectors are protected.
//10 Reserved for future use.
//11 All sectors are software protected (all Sector Protection Registers are 1 – default).
#define AT_STATUS1_WEL	1	//WEL   Write Enable Latch Status:0 Device is not write enabled (default);1 Device is write enabled.R
#define AT_STATUS1_RDY	0  	//RDY/BSY Ready/Busy Status:0 Device is ready;1 Device is busy with an internal operation.R

/* Spanson Flash */
#define SPANSION		0x00010000		/*factory id*/
#define SPANSION_F		0x00010200		/*memory_type*/
#define S25FL004A		0x00010212
#define S25FL016A		0x00010214
#define S25FL032A		0x00010215
#define S25FL064A		0x00010216		/*supposed support*/
#define S25FL128P		0x00012018		/*only S25FL128P0XMFI001, Uniform 64KB secotr*/
										/*not support S25FL128P0XMFI011, Uniform 256KB secotr*/
										/*because #define SPI_BLOCK_SIZE 65536 */
#define S25FL032P		0x00010215


/* MICRONIX Flash */
#define MACRONIX		0x00C20000		/*factory_id*/
#define MACRONIX_05		0x00C22000		/*memory_type*/
#define MX25L4005		0x00C22013
#define MX25L1605D		0x00C22015
#define MX25L3205D		0x00C22016		/*supposed support*/
#define MX25L6405D		0x00C22017
#define MX25L12805D		0x00C22018

#define MACRONIX_Q1		0x00C22400		/*memory_type*/
#define MACRONIX_Q2		0x00C25E00		/*memory_type*/
#define MX25L1635D		0x00C22415
#define MX25L3235D		0x00C25E16
#define MX25L6445E		0x00C22017
#define MX25L12845E		0x00C22018

/* SST Flash */
#define SST				0x00BF0000		/*factory_id*/
#define SST_25			0x00BF2500		/*memory_type*/
#define SST_26			0x00BF2600		/*memory_tyep*/
#define SST25VF032B		0x00BF254A		//4MB
#define SST26VF016		0x00BF2601
#define SST26VF032		0x00BF2602

/* WinBond Flash */
#define WINBOND			0X00EF0000		/*factory_id*/
#define WINBOND_Q		0x00EF4000		/*memory_type*/
#define W25Q80			0x00EF4014
#define W25Q16			0x00EF4015
#define W25Q32			0x00EF4016

/* Eon Flash */
#define EON				0x001c0000		/*factory_id*/
#define EON_F			0x001c3100		/*memory_type*/
#define EON_Q			0x001c3000		/*memory_type*/
#define EN25F32			0x001c3116
#define EN25F16			0x001c3115
#define EN25Q32			0x001c3016
#define EN25Q16			0x001c3015

/* GigaDevice Flash */
#define GIGADEVICE		0x00c80000		/*factory_id*/
#define GD_Q			0x00c84000		/*memory_type*/
#define GD25Q16			0x00c84015
#define GD25Q32			0x00c84016

/* Atmel Flash */
#define ATMEL			0x001f0000		/*factory_id*/
#define AT25DF161		0x001f4602

#define SZIE2N_128K	0x11
#define SZIE2N_256K	0x12
#define SZIE2N_512K	0x13
#define SZIE2N_01MB	0x14
#define SZIE2N_02MB	0x15
#define SZIE2N_04MB	0x16
#define SZIE2N_08MB	0x17
#define SZIE2N_16MB	0x18
#define SZIE2N_32MB	0x19
#define SZIE2N_64MB	0x20



#define SPI_REG_READ(reg)		*((volatile unsigned int *)(reg))
#define SPI_REG_LOAD(reg,val)	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0); *((volatile unsigned int *)(reg)) = (val)

#define IOWIDTH_SINGLE			0x00
#define IOWIDTH_DUAL			0x01
#define IOWIDTH_QUAD			0x02
#define ISFAST_NO				0x00
#define ISFAST_YES				0x01
#define ISFAST_ALL				0x02
#define DUMMYCOUNT_0			0x00
#define DUMMYCOUNT_1			0x01
#define DUMMYCOUNT_2			0x02
#define DUMMYCOUNT_3			0x03
#define DUMMYCOUNT_4			0x04
#define DUMMYCOUNT_5			0x05
#define DUMMYCOUNT_6			0x06
#define DUMMYCOUNT_7			0x07
#define DUMMYCOUNT_8			0x08
#define DUMMYCOUNT_9			0x09


/****************************** Common0 ******************************/

void spi_regist(unsigned char uiChip, unsigned int uiPrnFlag);
void set_flash_info(unsigned char uiChip, unsigned int chip_id, unsigned int device_cap, unsigned int block_size, unsigned int sector_size, unsigned int page_size, char* chip_name, FUNC_ERASE pfErase, FUNC_READ pfRead, FUNC_SETQEBIT pfQeBit, FUNC_PAGEWRITE pfPageWrite);

/****************************** Common ******************************/
// get Dram Frequence
unsigned int CheckDramFreq(void);                       //JSW:For 8196C
// Set FSCR register
void setFSCR(unsigned char uiChip, unsigned int uiClkMhz, unsigned int uiRBO, unsigned int uiWBO, unsigned int uiTCS);
// Calculate write address group
void calAddr(unsigned int uiStart, unsigned int uiLenth, unsigned int uiSectorSize, unsigned int* uiStartAddr, unsigned int*  uiStartLen, unsigned int* uiSectorAddr, unsigned int* uiSectorCount, unsigned int* uiEndAddr, unsigned int* uiEndLen);
// Calculate chip capacity shift bit 
unsigned char calShift(unsigned char ucCapacityId, unsigned char ucChipSize);
// Print spi_flash_type
void prnFlashInfo(unsigned char uiChip, struct spi_flash_type sftInfo);
// Print SPI Register
//void prnInterfaceInfo();
// pirnt when writing
void prnDispAddr(unsigned int uiAddr);
// Check WIP bit
unsigned int spiFlashReady(unsigned char uiChip);
//toggle CS
void rstSPIFlash(unsigned char uiChip);

/****************************** Layer 1 ******************************/
//set cs low
unsigned int SFCSR_CS_L(unsigned char uiChip, unsigned int uiLen, unsigned int uiIOWidth);
// set cs high
unsigned int SFCSR_CS_H(unsigned char uiChip, unsigned int uiLen, unsigned int uiIOWidth);
// Write Enable (WREN) Sequence (Command 06)
void ComSrlCmd_WREN(unsigned char uiChip);
// Write Disable (WRDI) Sequence (Command 04)
void ComSrlCmd_WRDI(unsigned char uiChip);
// Read Identification (RDID) Sequence (Command 9F)
unsigned int ComSrlCmd_RDID(unsigned char uiChip, unsigned int uiLen);
// Read Status Register (RDSR) Sequence (Command 05)
unsigned int ComSrlCmd_RDSR(unsigned char uiChip, unsigned int uiLen);
// Write Status Register (WRSR) Sequence (Command 01)
unsigned int ComSrlCmd_WRSR(unsigned char uiChip,unsigned char ucValue);
unsigned int ComSrlCmd_SE(unsigned char uiChip, unsigned int uiAddr);
// Block Erase (BE) Sequence (Command D8)
unsigned int ComSrlCmd_BE(unsigned char uiChip, unsigned int uiAddr);
// Chip Erase (CE) Sequence (Command 60 or C7)
unsigned int ComSrlCmd_CE(unsigned char uiChip);
/*
// Deep Power Down (DP) Sequence (Command B9)
unsigned int ComSrlCmd_DP(unsigned char uiChip);
// Release from Deep Power-down(RDP) Sequence (Command AB)
unsigned int ComSrlCmd_RDP(unsigned char uiChip);
// Release from Deep Power-down and Read Electronic Signature (RES) Sequece (Command AB)
unsigned int ComSrlCmd_RES(unsigned char uiChip);
// Read Electronic Manufacturer & Device ID (REMS) Sequece (Command 90 or EF or DF)
unsigned int ComSrlCmd_REMS(unsigned char uiChip);
// Enter Security OTP
unsigned int ComSrlCmd_ENSO(unsigned char uiChip);
// Exit Security OTP
unsigned int ComSrlCmd_EXSO(unsigned char uiChip);
*/

/****************************** Layer 2 ******************************/
// without QE bit
unsigned int ComSrlCmd_NoneQeBit(unsigned char uiChip);
// uiIsFast: = 0 cmd, address, dummy single IO ; =1 cmd single IO, address and dummy multi IO; =2 cmd, address and dummy multi IO;
void ComSrlCmd_InputCommand(unsigned char uiChip, unsigned int uiAddr, unsigned char ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount);
// Set SFCR2 for memery map read
unsigned int SetSFCR2(unsigned int ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount);
// read function template
unsigned int ComSrlCmd_ComRead(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer,unsigned char ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount);
// write template
unsigned int ComSrlCmd_ComWrite(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer, unsigned char ucCmd, unsigned int uiIsFast, unsigned int uiIOWidth, unsigned int uiDummyCount);
// write a sector once
unsigned int ComSrlCmd_ComWriteSector(unsigned char uiChip, unsigned int uiAddr, unsigned char* pucBuffer);
// write sector use malloc buffer
unsigned int ComSrlCmd_BufWriteSector(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
// write function
unsigned int ComSrlCmd_ComWriteData(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);

/****************************** Macronix ******************************/
// Set quad enable bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_spi_setQEBit(unsigned char uiChip);
#endif
// MX25L1605 MX25L3205 Read at High Speed (FAST_READ) Sequence (Command 0B)
unsigned int mxic_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
// MX25L1605 MX25L3205 Read at Dual IO Mode Sequence (Command BB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_cmd_read_d1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// MX25L1635 MX25L3235 4 x I/O Read Mode Sequence (Command EB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_cmd_read_q1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Page Program (PP) Sequence (Command 02)
unsigned int mxic_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
// 4 x I/O Page Program (4PP) Sequence (Command 38)
#ifndef CONFIG_SPI_STD_MODE
unsigned int mxic_cmd_write_q1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

/****************************** SST ******************************/
// Layer1 SST Auto Address Increment (AAI) Word-Program
//#ifndef CONFIG_SPI_STD_MODE
//void SstComSrlCmd_AAI_Start(unsigned char uiChip, unsigned int uiAddr, unsigned short usValue);
//#endif
// Layer1 SST Auto Address Increment (AAI) Word-Program
//#ifndef CONFIG_SPI_STD_MODE
//void SstComSrlCmd_AAI_Continue(unsigned char uiChip, unsigned short usValue);
//#endif
// Layer1 SST Byte-Program
#ifndef CONFIG_SPI_STD_MODE
void SstComSrlCmd_BP(unsigned char uiChip, unsigned int uiAddr, unsigned char ucValue);
#endif
// Read at High Speed (FAST_READ) Sequence (Command 0B)
#ifndef CONFIG_SPI_STD_MODE
unsigned int sst_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Layer2 Sector Write Use BP Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int sst_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

/****************************** Spansion ******************************/
//Layer1 Spansion S25FL016A Bulk(Chip) Erase (BE 0xC7)
#ifndef CONFIG_SPI_STD_MODE
unsigned int SpanComSrlCmd_SE(unsigned char uiChip, unsigned int uiAddr);
#endif
// Layer1 Spansion Write Registers (WRR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int SpanComSrlCmd_WRR(unsigned char uiChip, unsigned int uiLen, unsigned int uiValue);
#endif
// Layer1 Spansion Read Configuration Register (RCR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int SpanComSrlCmd_RCR(unsigned char uiChip, unsigned int uiLen);
#endif
// Layer2 Spansion Set QE bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_spi_setQEBit(unsigned char uiChip);
#endif
// S25FL016A Layer1 Spansion FASTREAD Read Mode (Single IO)
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// S25FL032A Layer1 Spansion Quad Output Read Mode (QOR) 
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_read_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Layer1 Spansion Single IO Program (PP)
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Layer1 Spansion QUAD Page Program (QPP)
#ifndef CONFIG_SPI_STD_MODE
unsigned int span_cmd_write_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

/****************************** Winbond ******************************/
// Layer3 Winbond Set QE Bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int wb_spi_setQEBit(unsigned char uiChip);
#endif
// W25Q80 W25Q16 W25Q32 4 x I/O Read Mode Sequence (Command EB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int wb_cmd_read_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// 4 x I/O Page Program (4PP) Sequence (Command 38)
#ifndef CONFIG_SPI_STD_MODE
unsigned int wb_cmd_write_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

/****************************** Eon ******************************/
// Read Status Register (RDSR) Sequence (Command 05) Quad IO Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int ComSrlCmd_RDSR4(unsigned char uiChip, unsigned int uiLen);
#endif
// check WIP bit Quad IO Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int spiFlashReady4(unsigned char uiChip);
#endif
// Layer1 Eon Enable Quad I/O (EQIO) (38h)
#ifndef CONFIG_SPI_STD_MODE
void EonComSrlCmd_EQIO(unsigned char uiChip);
#endif
// Layer1 Eon Reset Quad I/O (RSTQIO) (FFh)
#ifndef CONFIG_SPI_STD_MODE
void EonComSrlCmd_RSTQIO(unsigned char uiChip);
#endif
// Eon read Single IO
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_read_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Eon Read Quad IO
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_read_q1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Page Program (PP) Sequence (Command 02)
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Layer1 Eon Page Program (PP) (02h) under QIO Mode
#ifndef CONFIG_SPI_STD_MODE
unsigned int eon_cmd_write_q2(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

/****************************** Giga Device ******************************/
// Layer1 High Performance Mode (HPM) (A3H)
#ifndef CONFIG_SPI_STD_MODE
unsigned int GdComSrlCmd_HPM(unsigned char uiChip);
#endif
// Layer1 GigaDevice Write Registers (WRSR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int GdComSrlCmd_WRSR(unsigned char uiChip, unsigned int uiLen, unsigned int uiValue);
#endif
// Layer1 Read Configuration Register (RCR)
#ifndef CONFIG_SPI_STD_MODE
unsigned int GdComSrlCmd_RDSR(unsigned char uiChip, unsigned int uiLen);
#endif
// Set quad enable bit
#ifndef CONFIG_SPI_STD_MODE
unsigned int gd_spi_setQEBit(unsigned char uiChip);
#endif
// GD25Q16 Read at Fast read Quad Sequence (Command EB)
#ifndef CONFIG_SPI_STD_MODE
unsigned int gd_cmd_read_q0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// Page Program (PP) Sequence (Command 02)
#ifndef CONFIG_SPI_STD_MODE
unsigned int gd_cmd_write_s1(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

/****************************** ATMEL ******************************/
// AT25DF161 Dual-Output Read Array(Command 3B)
#ifndef CONFIG_SPI_STD_MODE
unsigned int at_cmd_read_d0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif
// AT25DF161 Dual-Input Byte/Page Program(Command A2)
#ifndef CONFIG_SPI_STD_MODE
unsigned int at_cmd_write_d0(unsigned char uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
#endif

