#ifndef _MTD_SPI_PROBE_H_
#define _MTD_SPI_PROBE_H_
#define ID_MASK         0xffff
//#define RTL8196C 1
#define SPANSION 0x01
#define SPI          0x02

#define SIZE_64KiB      0x10000
#define SIZE_2MiB       0x200000

struct spi_flash_info {
	const unsigned char mfr_id;
	const unsigned char dev_id;
	const unsigned short extra_id;
	const char *name;
	//const int DeviceSize;
	int DeviceSize;
	const int EraseSize;
};

struct spi_chip_info {
	struct spi_flash_info *flash;
	void (*destroy)(struct spi_chip_info *chip_info);

	unsigned int (*read)(unsigned int  from, unsigned int  to, unsigned int  size);
	unsigned int  (*write)(unsigned int  from, unsigned int  to, unsigned int  size);
	int (*erase)(unsigned int  addr);
};

#endif /* _MTD_SPI_PROBE_H_ */
#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

/*
 * Macro Definition
 */
#define SPI_CS(i)           ((i) << 30)   /* 0: CS0 & CS1   1: CS0   2: CS1   3: NONE */
#define SPI_LENGTH(i)       ((i) << 28)   /* 0 ~ 3 */
#define SPI_READY(i)        ((i) << 27)   /* 0: Busy  1: Ready */

#define SPI_CLK_DIV(i)      ((i) << 29)   /* 0: DIV_2  1: DIV_4  2: DIV_6 ... 7: DIV_16 */
#define SPI_RD_ORDER(i)     ((i) << 28)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_WR_ORDER(i)     ((i) << 27)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_RD_MODE(i)      ((i) << 26)   /* 0: Fast-Mode  1: Normal Mode */
#define SPI_SFSIZE(i)       ((i) << 23)   /* 0 ~ 7. 128KB * (i+1) */
#define SPI_TCS(i)          ((i) << 19)   /* 0 ~ 15 */
#define SPI_RD_OPT(i)       ((i) << 18)   /* 0: No-Optimization  1: Optimized for Sequential Access */


/*
 * Structure Declaration
 */
struct spi_flash_type
{
   unsigned char maker_id;
   unsigned char type_id;
   unsigned char capacity_id;
   unsigned char device_size;        // 2 ^ N (bytes)
   unsigned char sector_cnt;
};

struct spi_flash_db
{
   unsigned char maker_id;
   unsigned char type_id;
   signed char size_shift;
};


/*
 * Function Prototypes
 */
void spi_pio_init(void);

void spi_read(unsigned int chip, unsigned int address, unsigned int *data_out);
//void spi_write(unsigned int chip, unsigned int address, unsigned int data_in);

void spi_erase_chip(unsigned int chip);

void spi_cp_probe(void);
void spi_burn_image(unsigned int chip, unsigned char *image_addr, unsigned int image_size);
void spi_ready(void);

#if defined(CONFIG_RTL8196C) ||defined(CONFIG_RTL8198)
	#define SPI_REG(reg) *((volatile unsigned int *)(reg))
	#define SPI_BLOCK_SIZE 65536  /* 64KB */
	#define SPI_SECTOR_SIZE 4096  /*  4KB */
	#define ENABLE_SPI_FLASH_FORMAL_READ
	

	#define SFRB_8198   0xB8001200       /*SPI Flash Register Base*/	
	#define SFCR_8198   (SFRB_8198)           /*SPI Flash Configuration Register*/		
		#define SFCR_CLK_DIV(val)   ((val)<<29)		
		#define SFCR_EnableRBO      (1<<28)		
		#define SFCR_EnableWBO      (1<<27)		
		#define SFCR_SPI_TCS(val)   ((val)<<23) /*4 bit, 1111 */
	#define SFCR2_8198  (SFRB_8198+0x04)      /*For memory mapped I/O */
		#define SFCR2_SFCMD(val)    ((val)<<24) /*8 bit, 1111_1111 */
		#define SFCR2_SIZE(val)     ((val)<<21) /*3 bit, 111 */
		#define SFCR2_RDOPT         (1<<20)
		#define SFCR2_CMDIO(val)    ((val)<<18) /*2 bit, 11 */
		#define SFCR2_ADDRIO(val)   ((val)<<16) /*2 bit, 11 */
		#define SFCR2_DUMMYCYCLE(val)   ((val)<<13) /*3 bit, 111 */
		#define SFCR2_DATAIO(val)   ((val)<<11) /*2 bit, 11 */
		#define SFCR2_HOLD_TILL_SFDR2  (1<<10)
		#define SFCR2_GETSIZE(x)    (((x)&0x00E00000)>>21)			
	#define SFCSR_8198  (SFRB_8198+0x08)   /*SPI Flash Control&Status Register*/		
		#define SFCSR_SPI_CSB0      (1<<31)
		#define SFCSR_SPI_CSB1      (1<<30)		
		#define SFCSR_LEN(val)      ((val)<<28)  /*2 bits*/
		#define SFCSR_SPI_RDY       (1<<27)		
		#define SFCSR_IO_WIDTH(val) ((val)<<25)  /*2 bits*/
		#define SFCSR_CHIP_SEL      (1<<24)
		#define SFCSR_CMD_BYTE(val) ((val)<<16)  /*8 bit, 1111_1111 */		
	#define SFDR_8198   (SFRB_8198+0x0C) /*SPI Flash Data Register*/	
	#define SFDR2_8198  (SFRB_8198+0x10) /*SPI Flash Data Register - for post SPI bootup setting*/	
	#define SPI_CS_INIT    (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SPI_LEN1 | SFCSR_SPI_RDY)
	#define SPI_CS0    SFCSR_SPI_CSB0	
	#define SPI_CS1    SFCSR_SPI_CSB1	
	#define SPI_eCS0  ((SFCSR_SPI_CSB1) | SFCSR_SPI_RDY) /*and SFCSR to active CS0*/
	#define SPI_eCS1  ((SFCSR_SPI_CSB0) | SFCSR_SPI_RDY) /*and SFCSR to active CS1*/	
	#define SPI_WIP (1)   /* Write In Progress */	
	#define SPI_WEL (1<<1)   /* Write Enable Latch*/
	#define SPI_SST_QIO_WIP (1<<7)   /* SST QIO Flash Write In Progress */
	#define SPI_LEN_INIT 0xCFFFFFFF /* and SFCSR to init   */	
	#define SPI_LEN4    0x30000000     /* or SFCSR to set */	
	#define SPI_LEN3    0x20000000     /* or SFCSR to set */	
	#define SPI_LEN2    0x10000000     /* or SFCSR to set */	
	#define SPI_LEN1    0x00000000     /* or SFCSR to set */		
	#define SPI_SETLEN(val) do {		\
		SPI_REG(SFCSR_8198) &= 0xCFFFFFFF;   \
		SPI_REG(SFCSR_8198) |= (val-1)<<28;	\
		}while(0)		


#define SPI_MAX_TRANSFER_SIZE 256
#define CHECK_READY while( !(SPI_REG(SFCSR_8198)&SFCSR_SPI_RDY) );
#if 1//def SUPPORT_SPI_MIO_8198_8196C
#define SPI_SFSIZE_8198(i)       ((i) << 21)   /* 0 ~ 7. 128KB * (i+1) */
#define SPI_TCS_8198(i)          ((i) << 23)   /* 0 ~ 15 */
#define SPI_RD_OPT_8198(i)       ((i) << 20)   /* 0: No-Optimization  1: Optimized for Sequential Access */
#define SPI_CS_8198(i)           ((i) << 24)   /* 0: CS0   1: CS1   */ //MMIO
#endif
#define CS_8198(i)           SPI_CS_8198(i)
#endif

#endif /* _SPI_FLASH_H_ */
