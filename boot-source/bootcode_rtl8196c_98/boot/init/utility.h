#include <linux/types.h>
#include "rtk.h"
#if defined(RTL8196B)
#include <asm/rtl8196.h>
#include <asm/rtl8196x.h>
#elif defined(RTL8198)
#include <asm/rtl8198.h>
#endif
#include "etherboot.h"


/* Setting image header */
typedef struct _setting_header_ {
	Int8  Tag[2];
	Int8  Version[2];
	Int16 len;
} SETTING_HEADER_T, *SETTING_HEADER_Tp;

//------------------------------------------------------------------------------------------
#define BAUD_RATE	  		(38400)  
#define ACCCNT_TOCHKKEY 	(128*1024)  //128K
unsigned long return_addr;
#define WAIT_TIME_USER_INTERRUPT	(3*CPU_CLOCK)

#define LOCALSTART_MODE 0
#define DOWN_MODE 1
#define DEBUG_LOCALSTART_MODE 3

/*Cyrus Tsai*/
unsigned long kernelsp;
#if  defined(RTL8196B) || defined(RTL8198)
	#define _SYSTEM_HEAP_SIZE	1024*64	//wei add
	char dl_heap[_SYSTEM_HEAP_SIZE];	//wei add
#endif


//check
#define HS_IMAGE_OFFSET		(24*1024)	//0x6000
#define DS_IMAGE_OFFSET		(25*1024)	//0x6400
#define CS_IMAGE_OFFSET		(32*1024)	//0x8000

#define CODE_IMAGE_OFFSET		(64*1024)	//0x10000
#define CODE_IMAGE_OFFSET2	(128*1024)	//0x20000
#define CODE_IMAGE_OFFSET3	(192*1024)	//0x30000
#define CODE_IMAGE_OFFSET4	(0x8000)

//flash mapping
#define ROOT_FS_OFFSET		(0xE0000)
#define ROOT_FS_OFFSET_OP1		(0x10000)
#define ROOT_FS_OFFSET_OP2		(0x40000)



int enable_10M_power_saving(int phyid , int regnum,int data);
int user_interrupt(unsigned long time);

#ifdef CONFIG_NFBI
// return,  0: not found, 1: linux found, 2:linux with root found
int check_system_image(unsigned long addr,IMG_HEADER_Tp pHeader);
#else
int check_system_image(unsigned long addr,IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp setting_header);
int check_rootfs_image(unsigned long addr);
#endif
void cp3_count_print(void);
//------------------------------------------------------------------------------------------
#ifdef CONFIG_SPI_FLASH
	//int check_dram_freq_reg(void) ;
	int RTL_TIMEOUT();
#endif
//ddr
#define DDR_DBG 0
#define PROMOS_DDR_CHIP 1
#define IMEM_DDR_CALI_LIMITS 60 //1sec=30 times
#define __IRAM_IN_865X      __attribute__ ((section(".iram-rtkwlan")))
#define __IRAM_FASTEXTDEV        __IRAM_IN_865X

#ifdef DDR_SDRAM
void DDR_cali_API7(void);
#endif

#if defined(CONFIG_POST_ENABLE)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))
int POSTRW_API(void)
#endif

//------------------------------------------------------------------------------------------
//need move to regs.h
#if defined(CONFIG_RTL8196C)||defined(RTL8198)
#if defined(CONFIG_RTL8196C_REVISION_B)
       #define RTL_GPIO_MUX 0xb8000040
#else
	#define RTL_GPIO_MUX 0xb8000040
#endif
#endif
#define RTL_GPIO_MUX_DATA 			0x00340000//for WIFI ON/OFF and GPIO

#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
extern  int sc16is7x0_err;

  #define Check_UART_DataReady() ( !sc16is7x0_err && \
			  ( sc16is7x0_serial_in_i2c( UART_MEM2REG( UART_LSR ) ) & 0x01 ) )
  #define Get_UART_Data() ( sc16is7x0_serial_in_i2c( UART_MEM2REG( UART_RBR ) ) )
#else
  #define Check_UART_DataReady() (rtl_inl(UART_LSR) & (1<<24))
  #define Get_UART_Data() ((rtl_inl(UART_RBR) & 0xff000000)>>24)
#endif

#if defined(RTL8196B) || defined(RTL8198)
	#if defined(RTL8196C)
		#define Get_GPIO_SW_IN() (!((REG32(PABCDDAT_REG) & (1<<5))>>5) )  //return 0 if non-press
	#elif defined(RTL8198)
		#define Get_GPIO_SW_IN() (!((REG32(PEFGHDAT_REG) & (1<<25))>>25) )  //return 0 if non-press
	#else
		#define Get_GPIO_SW_IN() (!(REG32(PABCDDAT_REG) & (1<<0)) )  //return 0 if non-press
		#ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
			#define Get_GPIO_SW_IN_KERNEL() (!(REG32(PABCDDAT_REG) & (1<<1)) )  //return 0 if non-press
		#endif
	#endif
#endif

#if  defined(RTL8196B) || defined(RTL8198)	
	//#define REG32(reg) (*(volatile unsigned int *)(reg))
	#define SYS_BASE 0xb8000000
	#define SYS_HW_STRAP (SYS_BASE +0x08)
#endif
//------------------------------------------------------------------------------------------
//gpio
#if defined(RTL8196B)|| defined(RTL8198)
#define RESET_LED_PIN 24
#endif
#if defined(RTL8196C)
#define RESET_LED_PIN 6
#endif


#if  defined(RTL8196B)|| defined(RTL8198)
#if  defined(RTL8196C)
#define Set_GPIO_LED_ON()	(REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  & (~(1<<RESET_LED_PIN)) )
#define Set_GPIO_LED_OFF()	(REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  | (1<<RESET_LED_PIN))
#else
#define Set_GPIO_LED_ON()       (REG32(PEFGHDAT_REG) =  REG32(PEFGHDAT_REG)  & (~(1<<RESET_LED_PIN)) )
#define Set_GPIO_LED_OFF()      (REG32(PEFGHDAT_REG) =  REG32(PEFGHDAT_REG)  | (1<<RESET_LED_PIN))
#endif
#endif
void Init_GPIO();
unsigned int read_gpio_hw_setting();

