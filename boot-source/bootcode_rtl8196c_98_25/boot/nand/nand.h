/*
 *  linux/include/linux/mtd/nand.h
 *
 */
#ifndef __LINUX_MTD_NAND_H
#define __LINUX_MTD_NAND_H

#define u_char unsigned char
//#define  FIXED_MEM

#ifdef	 FIXED_MEM

#define  SDRAMBUF	
#define  CHIP0BUF 
#define  CHIP1BUF

#endif

/*
 * Searches for a NAND device
 */
//extern int nand_scan (struct nand_chip *this);

// specific Samsung 8 bit NAND flash configuration
#define  PAGESIZE	528		// 528 bytes per page
#define  PAGENUM	32
#define  BLOCKSIZE      (32*528)	// 16.5K bytes per block
#define  ECCSIZE	256
#define  ECCBYTES	6
#define	 OOBSIZE	16
#define  BERASESIZE	BLOCKSIZE // block erase
#define  FIRSTBLOCK 	0
#define  SECONDBLOCK	1

// NAND flash address mapping
#define  COLUMN		0x000000ff	// BIT(0)~BIT(7)
#define	 ROW0		0x0001fe00	// BIT(9)~BIT(16)
#define  ROW1		0x01fe0000	// BIT(17)~BIT(24)
// Samsung K9F5608 status bitmap
#define  WRITEPROTECT	BIT(7)
#define  DEVREADY	BIT(6)
#define	 STATUSOK	BIT(0)

// RTL8186 NAND flash controller
#define  MCR		0x1000
#define  MTCR0		0x1004
#define  MTCR1		0x1008
#define  NCR		0x100C
#define  NCAR		0x1010
#define  NADDR   	0x1014
#define  NDR		0x1018

// NCAR
#define  CE4_EN  	BIT(31)
#define  CE5_EN  	BIT(30)
#define  CE4_CMD 	0x000000ff
// we should write CE4/CE5 at the beginning
// so it is not necessary to enable again
#define  CE4_CMD_EN     CE4_EN | CE4_CMD
#define  CE5_CMD_EN	CE5_EN | CE4_CMD
// NCR
#define  CE4_RDY 	BIT(31)
#define  CE4_TWP 	0x00f00000
#define  CE4_TWB 	0x000f0000
#define  CE4_TRR 	0x0000f000
#define  CE4_TREA	0x00000f00
#define  CE4_TH		0x000000f0
#define  CE4_TS		0x0000000f
// NADDR
#define  CE4_ADEN2	BIT(26)
#define  CE4_ADEN1	BIT(25)
#define	 CE4_ADEN0	BIT(24)
#define  CE4_ADD2	0x00ff0000
#define	 CE4_ADD1	0x0000ff00
#define  CE4_ADD0	0x000000ff
#define  CE4_CYC2	CE4_ADEN2 | CE4_ADD2
#define  CE4_CYC1	CE4_ADEN1 | CE4_ADD1
#define  CE4_CYC0	CE4_ADEN0 | CE4_ADD0
// NDR
#define  CE4_DATA3	0xff000000
#define  CE4_DATA2	0x00ff0000
#define  CE4_DATA1	0x0000ff00
#define  CE4_DATA0	0x000000ff

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0(x)	0 | (x ? CE5_EN:CE4_EN)
#define NAND_CMD_READ1(x)	1 | (x ? CE5_EN:CE4_EN)  // only for K9F5608
#define NAND_CMD_PAGEPROG(x)	0x10 | (x ? CE5_EN:CE4_EN)
#define NAND_CMD_READOOB(x)	0x50 | (x ? CE5_EN:CE4_EN)  // "Read 2" in datasheet
#define NAND_CMD_ERASE1(x)	0x60 | (x ? CE5_EN:CE4_EN)  // "Block erase" in datasheet
#define NAND_CMD_STATUS(x)	0x70 | (x ? CE5_EN:CE4_EN)
#define NAND_CMD_SEQIN(x)	0x80 | (x ? CE5_EN:CE4_EN)
#define NAND_CMD_READID(x)	0x90 | (x ? CE5_EN:CE4_EN)
#define NAND_CMD_ERASE2(x)	0xd0 | (x ? CE5_EN:CE4_EN)
#define NAND_CMD_RESET(x)	0xff | (x ? CE5_EN:CE4_EN)

// more command from datasheet K9F56XXX0C.pdf
#define NAND_CMD_LOCK(x)        0x2a & (x ?CE5_CMD_EN:CE4_CMD_EN)
#define NAND_CMD_UNLOCK(x)      0x23 & (x ?CE5_CMD_EN:CE4_CMD_EN)
#define NAND_CMD_LOCK_TIGHT(x)  0x2c & (x ?CE5_CMD_EN:CE4_CMD_EN)
#define NAND_CMD_LOCK_STS(x)    0x7a & (x ?CE5_CMD_EN:CE4_CMD_EN)


/*
 * Enumeration for NAND flash chip state
 */
typedef enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING
} nand_state_t;

/*
 * NAND Private Flash Chip Data
 *
 * Structure overview:
 *
 *  IO_ADDR - address to access the 8 I/O lines to the flash device
 *
 *  CTRL_ADDR - address where ALE, CLE and CE control bits are accessed
 *
 *  CLE - location in control word for Command Latch Enable bit
 *
 *  ALE - location in control word for Address Latch Enable bit
 *
 *  NCE - location in control word for nChip Enable bit
 *
 *  chip_lock - spinlock used to protect access to this structure
 *
 *  wq - wait queue to sleep on if a NAND operation is in progress
 *
 *  state - give the current state of the NAND device
 *
 *  page_shift - number of address bits in a page (column address bits)
 *
 *  data_buf - data buffer passed to/from MTD user modules
 *
 *  ecc_code_buf - used only for holding calculated or read ECCs for
 *                 a page read or written when ECC is in use
 *
 *  reserved - padding to make structure fall on word boundary if
 *             when ECC is in use
 */
struct nand_chip {
	int 			id;
	unsigned long 		IO_ADDR;
	unsigned long 		CTRL_ADDR;
	int 			page_shift;
	u_char 			*data_buf;
#ifdef CONFIG_MTD_NAND_ECC
	u_char 			ecc_code_buf[6];
#endif
};

/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SAMSUNG	0xec

/*
 * NAND Flash Device ID Structure
 *
 * Structure overview:
 *
 *  name - Complete name of device
 *
 *  manufacture_id - manufacturer ID code of device.
 *
 *  model_id - model ID code of device.
 *
 *  chipshift - total number of address bits for the device which
 *              is used to calculate address offsets and the total
 *              number of bytes the device is capable of.
 *
 *  page256 - denotes if flash device has 256 byte pages or not.
 *
 *  pageadrlen - number of bytes minus one needed to hold the
 *               complete address into the flash array. Keep in
 *               mind that when a read or write is done to a
 *               specific address, the address is input serially
 *               8 bits at a time. This structure member is used
 *               by the read/write routines as a loop index for
 *               shifting the address out 8 bits at a time.
 *
 *  erasesize - size of an erase block in the flash device.
 */
struct nand_flash_dev {
	char 		*name;
	int 		manufacture_id;
	int 		model_id;
	int 		chipshift;
	char 		page256;
	char 		pageadrlen;
	unsigned long   erasesize;
};


#endif /* __LINUX_MTD_NAND_H */
