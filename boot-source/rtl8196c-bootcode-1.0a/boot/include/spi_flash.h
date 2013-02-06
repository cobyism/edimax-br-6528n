#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

/* 20090805 JSW:Support SPI List 
//MXIC Single-I/O with 50-MHz SPI
MX25L1605D  //2MB
MX25L3205D  //4MB
MX25L6405D  //8MB
MX25L12805D //16MB

//MXIC Single-I/O with 104-MHz SPI
MX25L1635D   //2MB
MX25L3235D   //4MB
 
//Spansion Single-I/O with 50-MHz SPI
S25FL004A  //0.5MB
S25FL008A  //1MB
S25FL016A  //2MB
S25FL032A  //4MB
s25fl064a  //8MB

//Spansion Single-I/O with 104-MHz SPI
S25FL032P  //4MB
S25FL128p  //16MB

 
//SST Single-I/O with 50-MHz SPI
SST25VF016B //2MB
SST25VF032B //4MB


//Winbond Multi-I/O with 80-MHz SPI
W25Q80 //1MB
W25Q16 //2MB
W25Q32 //4MB
W25Q64 //8MB


*/



#include <asm/rtl8196.h> //define in boot's MAKEFILE

//Show Debug message and test_program
#define SPI_DBG 0

#define SE_code 0x20

/*JSW@20090714 Note:
         Instruction code :
   =======================
         For Old 50MHZ SPI:         
         Sector Erase(20h)~
         1.MXIC: 4KB
         2.Spansion : Not support
         3.SST: 4KB
         
         SE(D8h)~
         1.MXIC: 64KB
         2.Spansion : 64KB
         3.SST:  64KB       

  ========================
   For New 80 MHZ SPI:         
         Sector Erase(20h)~
         1.Winbond: 4KB (W25Q80/W25Q16/W25Q32//W25Q64) 
         
         SE(D8h)~
         1.Winbond: 4KB (W25Q80/W25Q16/W25Q32) 

   ========================
   For New 104MHZ SPI:         
         Sector Erase(20h)~
         1.MXIC: 4KB
         2.Spansion : 4KB (only S25FL032P/S25FL064P)
         3.SST:  4KB

 

         SE(D8h)~
         1.MXIC: 64KB
         2.Spansion : 64KB
         3.SST: 64KB
        
         */



/*Erase sector= 4KB or 64KB*/
#define Set_SECTOR_ERASE_4KB
//#define Set_SECTOR_ERASE_64KB  



#define OS_PRINTF printf    


/* 8196 SPI Flash Controller,NOT compliant with 8198 */
#define SFCR         0xB8001200
#define SFCSR        0xB8001204
#define SFDR         0xB8001208
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


#if 1//def SUPPORT_SPI_MIO_8198_8196C
#define SPI_SFSIZE_8198(i)       ((i) << 21)   /* 0 ~ 7. 128KB * (i+1) */
#define SPI_TCS_8198(i)          ((i) << 23)   /* 0 ~ 15 */
#define SPI_RD_OPT_8198(i)       ((i) << 20)   /* 0: No-Optimization  1: Optimized for Sequential Access */
#define SPI_CS_8198(i)           ((i) << 24)   /* 0: CS0   1: CS1   */ //MMIO
#endif

/*
 * Structure Declaration
 */
struct spi_flash_type
{
   unsigned char maker_id;
   unsigned char type_id;
   unsigned char capacity_id;
   unsigned char device_size;        // 2 ^ N (bytes)
   int sector_cnt;
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
void spi_write(unsigned int chip, unsigned int address, unsigned int data_in);
void spi_erase_chip(unsigned int chip);
void spi_probe(void);
void spi_burn_image(unsigned int chip, unsigned char *image_addr, unsigned int image_size);
void spi_ready(void);
//int spi_flw_image(unsigned int chip, unsigned int flash_addr_offset ,unsigned char *image_addr, unsigned int image_size);


/* 20090303: JSW
  * Add code For SPI Dual/Qual IO  
  */
#if 1//def SUPPORT_SPI_MIO_8198_8196C  
       /*   List of supported single I/O chip    */
       /*  Spanson Flash  */
       #define S25FL004A 0x00010212
       #define S25FL016A 0x00010214
       #define S25FL064A 0x00010216  /*supposed support*/
       #define S25FL128P 0x00012018  /*only S25FL128P0XMFI001, Uniform  64KB secotr*/
                              /*not support S25FL128P0XMFI011, Uniform 256KB secotr*/
                              /*because #define SPI_BLOCK_SIZE 65536  */
       
       /*  MXIC Flash  */
       #define MX25L4005   0x00C22013
       #define MX25L1605D  0x00C22015
       #define MX25L3205D  0x00C22016  /*supposed support*/
       #define MX25L6405D  0x00C22017
       #define MX25L12805D 0x00C22018
       
       
       /*  SST Flash  */
       #define SST25VF032B 0x00BF254A //4MB
       
       
      /*   List of supported Multi I/O chip    */
	/*  Spanson Flash   */
	/*  MXIC Flash      */	
	#define MX25L1635D  0x00C22415  /*supposed support*/
	#define MX25L3235D  0x00C25E16  /*supposed support*/
	#define MX25L6445E 0x00C22017  /*JSW@20091011:OK*/
	#define MX25L12845E 0x00C22018  /*JSW@20091011:OK*/
	/*  SST Flash       */
	#define SST26VF016  0x00BF2601
	#define SST26VF032  0x00BF2602
	/*  WindBond Flash  */
	#define W25Q80      0x00EF4014
	#define W25Q16      0x00EF4015
	#define W25Q32      0x00EF4016
	/* Spansion Flash*/
	#define S25FL032P   0x00010215

	

	#define SPI_REG(reg) *((volatile uint32 *)(reg))
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

#define FLASHBASE CFG_FLASH_BASE
#define MAX_SPI_FLASH_CHIPS 2

//#define ENABLE_SPI_FLASH_READ
//#define ENABLE_SPI_FLASH_PIO_READ

#ifndef CONFIG_SPI_WBO
	#define CONFIG_SPI_WBO 1
#endif

#ifndef CONFIG_SPI_RBO
	#define CONFIG_SPI_RBO 1
#endif	
	

#endif //end of SUPPORT_SPI_MIO


#endif /* _SPI_FLASH_H_ */

