/*
 *  linux/init/main.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  GK 2/5/95  -  Changed to support mounting root fs via NFS
 *  Added initrd & change_root: Werner Almesberger & Hans Lermen, Feb '96
 *  Moan early if gcc is old, avoiding bogus kernels - Paul Gortmaker, May '96
 *  Simplified starting of init:  Michael A. Griffith <grif@acm.org>
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <linux/string.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <linux/circ_buf.h>


#ifdef RTL8196B
#include <asm/rtl8196.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif


#if defined(RTL8196B) || defined(RTL8198)
#ifdef CONFIG_SPI_FLASH
extern void spi_probe(); //// spi_probe() is from spi_flash.c
#endif
#endif
#if defined(RTL8196B)
#include <asm/rtl8196x.h>
#endif

#include <asm/io.h>

#include "etherboot.h"
#include "./banner/mk_time"
#include "./rtk.h"
#include "./ver.h"
#include "../../autoconf.h"
#define __KERNEL_SYSCALLS__

#define SYS_STACK_SIZE		(4096 * 2)
#define SERIAL_XMIT_SIZE	4096



#define HS_IMAGE_OFFSET		(24*1024)	//0x6000
#define DS_IMAGE_OFFSET		(25*1024)	//0x6400
#define CS_IMAGE_OFFSET		(32*1024)	//0x8000

#define CODE_IMAGE_OFFSET	(64*1024)	//0x10000
#define CODE_IMAGE_OFFSET2	(128*1024)	//0x20000
#define CODE_IMAGE_OFFSET3	(192*1024)	//0x30000
#define CODE_IMAGE_OFFSET4	(0x8000)
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
#define LINUX_IMAGE_OFFSET_START 	CONFIG_LINUX_IMAGE_OFFSET_START
#define LINUX_IMAGE_OFFSET_END           CONFIG_LINUX_IMAGE_OFFSET_END 
#define LINUX_IMAGE_OFFSET_STEP CONFIG_LINUX_IMAGE_OFFSET_STEP
#endif
#define ROOT_FS_OFFSET		(0xA0000) // For EDX Image
#define ROOT_FS_OFFSET1   (0xB0000) // For EDX Image
#define ROOT_FS_OFFSET2   (0xC0000) // For EDX Image
#define ROOT_FS_OFFSET3   (0xD0000) // For EDX Image
#define ROOT_FS_OFFSET4   (0xE0000) // For EDX Image
#define ROOT_FS_OFFSET5   (0xF0000) // For EDX Image
#define ROOT_FS_OFFSET6   (0x100000) // For EDX Image				//cypress 100715 edimax30
#define ROOT_FS_OFFSET7   (0x110000) // For EDX Image				//cypress 100715 edimax30
#define ROOT_FS_OFFSET8   (0x120000) // For EDX Image				//cypress 100715 edimax30
#define ROOT_FS_OFFSET9   (0x130000) // For EDX Image				//cypress 100715 edimax30
#define ROOT_FS_OFFSET10  (0x140000) // For EDX Image				//cypress 100715 edimax30
#define ROOT_FS_OFFSET11  (0x150000) // For EDX Image				//cypress 100715 edimax30
#define ROOT_FS_OFFSET_OP1		(0x10000)
#define ROOT_FS_OFFSET_OP2		(0x40000)
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
#define ROOT_IMAGE_OFFSET_START CONFIG_ROOT_IMAGE_OFFSET_START
#define ROOT_IMAGE_OFFSET_END   CONFIG_ROOT_IMAGE_OFFSET_END
#define ROOT_IMAGE_OFFSET_STEP   CONFIG_ROOT_IMAGE_OFFSET_STEP
#endif
#define IS_32BIT			(rtl_inl(0x1000)&0x100000)

/*Cyrus Tsai*/
Int16  DSversion;
Int16  CSversion;
/*Cyrus Tsai*/

#define PIN_MUX_SEL 0xb8000040
#ifdef CONFIG_FPGA_PLATFORM
//#define CPU_CLOCK			(33860000)  /* 33.86 MHz */
#define CPU_CLOCK			(27000000)  /* 27MHz */
#else
//#define CPU_CLOCK			(22*1000*1000) //8186
#define CPU_CLOCK			(200*1000*1000)
#endif
#define WAIT_TIME_USER_INTERRUPT	(3*CPU_CLOCK)


/* Setting image header */
typedef struct _setting_header_ {
	Int8  Tag[2];
	Int8  Version[2];
	Int16 len;
} SETTING_HEADER_T, *SETTING_HEADER_Tp;

//-----------------------------------------------------------

#if defined(KLD)
int check_image_signature_tag(unsigned long addr, char *tag);
#define HW_SETTING_OFFSET 0x6000
#define HW_SETTING_11N_RESERVED7_OFFSET 0xB2
#define HW_SETTING_11N_RESERVED8_OFFSET 0xB3
#define HW_SETTING_11N_RESERVED9_OFFSET 0xB4
#define HW_SETTING_11N_RESERVED10_OFFSET 0xB5
#endif
#ifndef RTL8197B
static int check_system_image(unsigned long addr, IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp sHeader);
#endif
int user_interrupt(unsigned long time);

/*in flash.c*/
extern int flashread (unsigned long dst, unsigned int src, unsigned long length);
extern int flashinit();
#if defined(RTL8196B) || defined(RTL8198)
#ifdef CONFIG_SPI_FLASH
extern void spi_probe(); //// : from spi_flash.c
extern void check_spi_clk_div(void);
int check_dram_freq_reg(void) ;
int RTL_TIMEOUT();
#endif
#endif

unsigned char init_task_union[SYS_STACK_SIZE];

/*Cyrus Tsai*/
unsigned long kernelsp;

#if  defined(RTL8196B) || defined(RTL8198)
	#define GICR_BASE	0xB8003000
	#define SCCR				(0x200 + GICR_BASE)     /* System Clock Control Register */
	#define DPLCR0			(0x204 + GICR_BASE)     /* DPLL Clock Control Register 0 */
	#define DPLCR1			(0x208 + GICR_BASE)     /* DPLL Clock Control Register 1 */
	#define PCCR				(0x20C + GICR_BASE)     /* Peripheral Clock Control Register */
	#define tick_Delay10ms(x) { int i=x; while(i--) __delay(5000); }

	#define _SYSTEM_HEAP_SIZE	1024*64	//wei add
	char dl_heap[_SYSTEM_HEAP_SIZE];	//wei add
#endif
//------------------------------------------------------------------------------------------
extern void __init exception_init(void);
/*in trap.c*/
extern void flush_cache(void);
extern void flush_dcache(UINT32 start, UINT32 end);

/*in irq.c*/
extern void init_IRQ(void);
extern void eth_startup(int etherport);
extern void eth_listening(void);
/*in eth_tftpd.c*/
extern void tftpd_entry(void);
unsigned long self_test(void);

/*in monitor.c*/
extern int check_cpu_speed(void);
extern volatile int get_timer_jiffies(void);

int enable_10M_power_saving(int phyid , int regnum,int data);



#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *) (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *) (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *) (addr))
#ifdef DDR_SDRAM
void DDR_cali_API7(void);
#endif
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
#if defined(RTL8196B) || defined(RTL8198)
#ifdef CONFIG_FPGA_PLATFORM
//#define SYS_CLK_RATE	  	(33860000)      //33.86 MHz
#define SYS_CLK_RATE	  	(27000000)      //27MHz
#else
//#define SYS_CLK_RATE	  	(0x00f42400)      //16MHz
//#define SYS_CLK_RATE	  	(0x01312d00)      //20MHz
//#define SYS_CLK_RATE	  	(25000000)      //25MHz
//#define SYS_CLK_RATE	  	(0x01ab3f00)      //28MHz
//#define SYS_CLK_RATE	  	(0x02625a00)      //40MHz
#define SYS_CLK_RATE	  	(200000000)      //200MHz
#endif
#define BAUD_RATE	  	(38400)  

#define ACCCNT_TOCHKKEY (128*1024)  //128K
unsigned int gCHKKEY_HIT=0;
unsigned int gCHKKEY_CNT=0;

void console_init(unsigned long lexea_clock)
{
	#define UART_BASE         0xB8000000
	#define UART_RBR_REG	(0x2000+UART_BASE)
	#define UART_THR_REG	(0x2000+UART_BASE)
	#define UART_DLL_REG	(0x2000+UART_BASE)
	#define	UART_IER_REG	(0x2004+UART_BASE)	
	#define	UART_DLM_REG	(0x2004+UART_BASE)
	#define	UART_IIR_REG	(0x2008+UART_BASE)
	#define	UART_FCR_REG	(0x2008+UART_BASE)
	#define UART_LCR_REG	(0x200c+UART_BASE)
	#define	UART_MCR_REG	(0x2010+UART_BASE)
	#define	UART_LSR_REG	(0x2014+UART_BASE)
	#define	UART_MSR_REG	(0x2018+UART_BASE)
	#define	UART_SCR_REG	(0x201c+UART_BASE)
	
        #define BAUD_RATE	  	(38400)  
  
	int i;
	unsigned long dl;
	unsigned long dll;     
	unsigned long dlm;       
           
  	REG32(UART_LCR_REG)=0x03000000;		//Line Control Register  8,n,1
  			
  	REG32( UART_FCR_REG)=0xc7000000;		//FIFO Ccontrol Register
  	REG32( UART_IER_REG)=0x00000000;
  	dl = (lexea_clock /16)/BAUD_RATE-1;
  	*(volatile unsigned long *)(0xa1000000) = dl ; 
  	dll = dl & 0xff;
  	dlm = dl / 0x100;
  	REG32( UART_LCR_REG)=0x83000000;		//Divisor latch access bit=1
  	REG32( UART_DLL_REG)=dll*0x1000000;
   	REG32( UART_DLM_REG)=dlm*0x1000000; 
    	REG32( UART_LCR_REG)=0x83000000& 0x7fffffff;	//Divisor latch access bit=0
   	//rtl_outl( UART_THR,0x41000000);	
#if defined(RTL8196B)	
  	REG32( 0xb8003000)=REG32( 0xb8003000) | 0x1000;
#endif
	//dprintf("\n\n-------------------------------------------");
	//dprintf("\nUART1 output test ok\n");
}
#endif
#if defined(CONFIG_POST_ENABLE)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))
int POSTRW_API(void)
{
  	unsigned int Test_Start_offset=0x0;  
#if defined(CONFIG_D8_16)  	
unsigned int Test_Size=0x00800000;  //8MB	
#else
unsigned int Test_Size=0x01000000;  //16MB	
#endif
	  unsigned short test_pattern_mode=0;//"0"=0x5a,"1"=0xa5,
	  unsigned int test_cnt=0;    
	  unsigned int test_result=1;  //"1":Pass,"0":fail

	  //unsigned int DRAM_ADR = 0xA0000000; //uncache address
    unsigned int DRAM_ADR = 0x80000000; //cache address
    unsigned int DRAM_Start_Test_ADR,DRAM_End_Test_ADR; 	  
    unsigned int DRAM_pattern1 = 0xA5A5A5A5;
	  unsigned int DRAM_pattern0 = 0x5A5A5A5A;
	  unsigned int DRAM_pattern;

	  DRAM_Start_Test_ADR= DRAM_ADR + Test_Start_offset; 
	  DRAM_End_Test_ADR=  DRAM_Start_Test_ADR+Test_Size; 
	
	for(test_pattern_mode=1 ;test_pattern_mode<=2;test_pattern_mode++)
	{
		
		if(test_pattern_mode%2==0)
		{
			DRAM_pattern=DRAM_pattern0;
		}
		else
		{
			DRAM_pattern=DRAM_pattern1;
		}
		prom_printf("\nPOST(%d),Pattern:0x%x=> ",test_pattern_mode,DRAM_pattern);
		

		/* Set Data Loop*/
		/* 	Test from 1~16MB ,except 7~8MB*/
		for (test_cnt= 0; test_cnt < Test_Size;test_cnt+=0x00100000 )
		{	 	
		     if ((test_cnt==0x0 )||(test_cnt==0x00400000 ))//skip DRAM size from 0~1MB and 4~5MB
		     {				     
					  continue;
		     }				  
			 memset((unsigned int *) (DRAM_Start_Test_ADR+ test_cnt),DRAM_pattern,(unsigned int)0x00100000 );
		}	 

		/*Verify Data Loop*/
		 for(test_cnt=0;test_cnt<Test_Size;test_cnt+=4)
		 {
		 	 if(((test_cnt >= 0x0 ) && (test_cnt <=0x00100000))||((test_cnt >= 0x00400000 ) && (test_cnt <=0x00500000)))
			 	 continue;

			 if (READ_MEM32(DRAM_Start_Test_ADR+test_cnt) != DRAM_pattern)//Compare FAIL
		 	  {											
						prom_printf("\nDRAM POST Fail at addr:0x%x!!!\n\n",(DRAM_Start_Test_ADR+test_cnt) );
						test_result=0;
						return 0;					
		 	  }
		 }//end of test_cnt

		   if (test_result)
			  	prom_printf("PASS\n");
			  else
			  	prom_printf("Fail\n");
		 
	}//end of test_pattern_mode
	 
	  prom_printf("\n\n");
	   return 1;
  }//end of POSTRW_API
#endif
//------------------------------------------------------------------------------------------
int RTL_TIMEOUT()
{
	//return 1 is timeout, 0 is not yet timeout 


#if  defined(RTL8196B)
	return (!(rtl_inl(TC1CNT)>>8));
#endif
}

//-----------------------------------------------------------


#if defined(RTL8196B)
#define RESET_LED_PIN 18
#endif
#if defined(RTL8196C)
#define RESET_LED_PIN 6
#endif
//EDX Jeff
#if defined(CONFIG_EDX_6X58GN)
	#define HW_BUTTON_RESET 3 
	#define HW_LED_WIRELESS 4 
	#define HW_LED_POWER 11
#elif defined(CONFIG_EDX_6X68GN)
        #define HW_BUTTON_RESET 3 
        #define HW_LED_WIRELESS 13 
        #define HW_LED_POWER 11
	#define HW_LED_WPS 4
#elif defined(CONFIG_EDX_6X28HPN)
        #define HW_BUTTON_RESET 5
        #define HW_LED_WIRELESS 6 
        #define HW_LED_POWER 16
#else
	#define HW_BUTTON_RESET 5
	#define HW_LED_WIRELESS 2
	#define HW_LED_POWER 6
#endif
//EDX Jeff

void Init_GPIO()
{

#if defined(RTL8196B)
#ifndef RTL8197B
  #ifndef CONFIG_RTL8198
	#ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<1) ); //set byte A GPIO1 = gpio
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<1) );  //0 input, 1 output, set F bit 1 input
	#endif
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<HW_BUTTON_RESET) ); //set byte F GPIO7 = gpio
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<HW_BUTTON_RESET) );  //0 input, 1 output, set F bit 7 input
	//modify for light reset led pin in output mode
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<RESET_LED_PIN) ); 
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | ((1<<RESET_LED_PIN) ); 
	REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) | ((1<<RESET_LED_PIN) );
  
#else
    //98 gpio init here
  #endif	

#endif
#endif

}


unsigned int read_gpio_hw_setting()
{
	unsigned int tmp;
	int b2;

	REG32(PEFGHCNR_REG)   = REG32(PEFGHCNR_REG)   & (~(0x8<<8));  //set (GP2)=(F3)= gpio
	REG32(PEFGHPTYPE_REG) = REG32(PEFGHPTYPE_REG) & (~(0x8<<8));  //change to GPIO mode
	REG32(PEFGHDIR_REG)   = REG32(PEFGHDIR_REG)   & (~(0x8<<8));  //0 input, 1 output, set inpur
	tmp = REG32(PEFGHDAT_REG);
//	dprintf("GPIO HGFE=%08x\r\n", tmp);
//	dprintf("GP2=%01x\r\n", (tmp&(0x08<<8))>>11);
	b2 = (tmp&(0x08<<8))>>11;
	tmp = (b2<<1)&0x2;	
//	dprintf("GPIO val=%08x\r\n",tmp);
	return tmp;	
}



#if  defined(RTL8196B)
#if  defined(RTL8196C)
#define Set_GPIO_LED_ON()	(REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  & (~(1<<RESET_LED_PIN)) )
#define Set_GPIO_LED_OFF()	(REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  | (1<<RESET_LED_PIN))

#else
#define Set_GPIO_LED_ON()       (REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  & (~(1<<RESET_LED_PIN)) )
#define Set_GPIO_LED_OFF()      (REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  | (1<<RESET_LED_PIN))
#endif
#endif

#if defined(RTL8196B)
	#if defined(RTL8196C)
	#define Get_GPIO_SW_IN() (!((REG32(PABCDDAT_REG) & (1<<HW_BUTTON_RESET))>>HW_BUTTON_RESET) )  //return 0 if non-press
	#else

	#define Get_GPIO_SW_IN() (!(REG32(PABCDDAT_REG) & (1<<0)) )  //return 0 if non-press
	#ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
	#define Get_GPIO_SW_IN_KERNEL() (!(REG32(PABCDDAT_REG) & (1<<1)) )  //return 0 if non-press
	#endif
	#endif
#endif


//return 1: data ready, 0 not yet ready

#if  defined(RTL8196B) || defined(RTL8198)
	#define  Check_UART_DataReady() (rtl_inl(UART_LSR) & (1<<24))
	#define 	Get_UART_Data() ((rtl_inl(UART_RBR) & 0xff000000)>>24)
	#define REG32(reg) (*(volatile unsigned int *)(reg))
	#define SYS_BASE 0xb8000000
	#define SYS_HW_STRAP (SYS_BASE +0x08)
#endif
#ifdef CONFIG_BOOT_TIME_MEASURE
static stage_cnt=0;
void cp3_count_print(void)
{
	unsigned long long temp64bit;

	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9, $12\n\t"
		"nop\n\t"
		"la $10, 0x80000000\n\t"
		"or $9, $10\n\t"
		"mtc0 $9, $12\n\t"
		"nop\n\t"
		"nop\n\t"
		"cfc3 $9,$0 \n\t"
		"ctc3 $0,$0\n\t"
		"mfc3 %M0,$9\n\t"
		"mfc3 %L0,$8\n\t"
		"ctc3 $9, $0\n\t"
		"nop\n\t"
		: "=r"(temp64bit)
		:
		: "$9", "$10");
	prom_printf("[stage:%d, boot0x%xM %xKcycle]", stage_cnt, (unsigned int)(temp64bit>>20), (unsigned int)((temp64bit>>10) &0x3ff));
	/* NOTE: 1M=1024*1024, 1K=1024 */
	stage_cnt++;
}
#endif

unsigned long glexra_clock=200*1000*1000; 
//------------------------------------------------------------------------------------------
#ifdef KLD
extern struct irqaction irq_timer;
#endif
void start_kernel(void)
{
	int i, ret;
	unsigned long flags;
	unsigned char* str[80];
	char* ptr;
	unsigned long start_addr;
	void	(*jump)(void);
	unsigned long error_code;
	unsigned long return_addr;
	unsigned short *word_ptr;
	volatile int cpu_speed = 0;

	IMG_HEADER_T header;
	SETTING_HEADER_T setting_header;
#if defined(CONFIG_POST_ENABLE)
	int post_test_result=1;
#endif
#ifdef KLD	
	int return_rootfs_status=0;
	int return_web_status=1; //it does not need to check web squashfs
	extern int ps_led_blink;
#endif
#if defined(EC)
	int return_rootfs_status=0;
#endif	

#ifndef CONFIG_RTL8196C
#if defined(RTL8196B)	
	REG32(SYS_HW_STRAP) = REG32(SYS_HW_STRAP);
#endif
#endif

REG32(0xb8001000)=REG32(0xb8001000)|0x8000000;
#ifndef CONFIG_RTL8196C
#ifdef RTL8197B
    //clear all bits of SYSSR except bit 5, 6, 7, 11
    REG32(0xb801900c)= REG32(0xb801900c) & 0x08e0;
#endif
	 console_init(200*1000*1000);
#endif

#ifdef CONFIG_RTL8196C
#define RTL_GPIO_MUX 0xB8000040
#define RTL_GPIO_MUX_DATA 0x00340000//for WIFI ON/OFF and GPIO
	REG32(RTL_GPIO_MUX)=RTL_GPIO_MUX_DATA; 
#ifndef CONFIG_RTL8196C_REVISION_B
	REG32(0xb8000010)=0xa79;
#endif
#if !defined(KLD)
	//REG32(PIN_MUX_SEL)=(REG32(PIN_MUX_SEL)&(0xFFFFFFFF-0xFFFF));//change pin mux to switch specific pin	
	#if defined(CONFIG_EDX_6X58GN) 
	REG32(PIN_MUX_SEL)=(REG32(PIN_MUX_SEL) | 0x0c);//change pin mux to switch specific pin	
	#elif defined(CONFIG_EDX_6X68GN)
	REG32(PIN_MUX_SEL)=(REG32(PIN_MUX_SEL) | 0xcc);//change pin mux to switch specific pin
	#elif defined(CONFIG_EDX_6X28HPN)
	REG32(PIN_MUX_SEL)=(REG32(PIN_MUX_SEL) | 0x303000);//change pin mux to switch specific pin
	#endif
	int	clklx_from_clkm=(REG32(SYS_HW_STRAP) & (ST_CLKLX_FROM_CLKM)) >> ST_CLKLX_FROM_CLKM_OFFSET;
	//unsigned long lexra_newval=200*1000*1000;
	//dprintf("clklx_from_clkm=%x \n", clklx_from_clkm);
	if(clklx_from_clkm==1)
	{
		unsigned long mem2x_clksel_table[7]={ 131250000, 156250000, 250000000, 300000000, 312500000, 337500000, 387500000 	};
		int	m2xsel=(REG32(SYS_HW_STRAP) & (CK_M2X_FREQ_SEL))>>CK_M2X_FREQ_SEL_OFFSET;
		glexra_clock=mem2x_clksel_table[m2xsel] /2;
		
	}
#else
	glexra_clock=200000000;
#endif	
	
 		console_init( glexra_clock);
#endif


#ifdef CONFIG_RTL8196C

	REG32(0xb8000010) = REG32(0xb8000010)| (1<<9) | (1<<11);   //clk_manger
	/* EDX Jeff */
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG) & (~(1<<HW_LED_WIRELESS) ); /* gpioc2 -> wlan led */
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | (1<<HW_LED_WIRELESS) ; /* output */
	REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) & (~(1<<HW_LED_WIRELESS) ); /* wlan led on */

	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG) & (~(1<<HW_LED_POWER) ); /* gpioc1 -> power led */
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | (1<<HW_LED_POWER); /* output */
	REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) & (~(1<<HW_LED_POWER) ); /* power led on */
	#if defined(CONFIG_EDX_6X68GN)
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG) & (~(1<<HW_LED_WPS) ); /* gpioc1 -> wps led */
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | (1<<HW_LED_WPS); /* output */
	REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) | (1<<HW_LED_WPS); /* wps led off */	
	#endif
	/* EDX Jeff */
#endif

#ifdef DDR_SDRAM
#ifndef CONFIG_RTL8198
     cli();  		
     flush_cache(); 	
     REG32(GIMR_REG)=0x0;
     REG32(BIST_CONTROL_REG)|=0x0000001C ;           //Lock LX Bus 0/1/2
     while(1)
     {
         if(READ_MEM32(BIST_DONE_REG) & 0x00000007)
         {
             break;
         }
     }//end of while(1)
     DDR_cali_API7();
     /* wait a little bit time, necessary */
     int i2;
     for( i2=0; i2 < 1000000; i2++);
     //Free LX Bus 0/1/2
     REG32(BIST_CONTROL_REG)&=0xFFFFFFE3 ;
     sti();    
#endif
#endif
#if defined(CONFIG_POST_ENABLE)
	if(POSTRW_API()==0)
		post_test_result=0;
#endif	 


#if  defined(RTL8196B) || defined(RTL8198)
	/* Initialize malloc mechanism */
	UINT32 heap_addr=((UINT32)dl_heap&(~7))+8 ;
	UINT32 heap_end=heap_addr+sizeof(dl_heap)-8;
  	i_alloc((void *)heap_addr, heap_end);
	
//	dprintf("heap ptr=%x size=%x \n", dl_heap, sizeof(dl_heap));	
//	dprintf("Heap: [%x - %x] size=%x Init\r\n", heap_addr, heap_end, sizeof(dl_heap)-8);
#endif
	
#ifdef KLD
	ps_led_blink = 0;
#endif

	cli();  	
	flush_cache(); // david

#ifndef RTL8197B
#ifdef RTL8196B //jason 0829
	//REG32(0xB8000030)= 0x00000300;   //wei add, ic test not turn-off jtag
#endif
#endif

	// mask all interrupt
	rtl_outl(GIMR0,0x00);

	setup_arch();    /*setup the BEV0,and IRQ */
	exception_init();/*Copy handler to 0x80000080*/
	init_IRQ();      /*Allocate IRQfinder to Exception 0*/

	sti();

	cpu_speed = check_cpu_speed();
	//set_bridge_clock(cpu_speed);
#ifdef CONFIG_SPI_FLASH
   //prom_printf("========== SPI =============\n");    	
   		 spi_probe();                                    //JSW : SPI flash init		
#else
#ifndef RTL8197B
	flashinit();
#endif
#endif

#ifdef CONFIG_PCIE_MODULE  //for matt test code.
	extern void PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset);
	PCIE_reset_procedure(0, 0, 1);   //1 do mdio_reset
#endif

#ifdef CONFIG_BOOT_TIME_MEASURE
	cp3_count_print();
#endif
#ifndef RTL8196C
#if defined(RTL8196B)
#if defined(RTL8197B)
	return_addr=0;
	prom_printf("\n---RealTek(RTL8197B)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, (IS_32BIT ? "32bit" : "16bit"), cpu_speed);	
#else
	return_addr=0;
	prom_printf("\n---RealTek(RTL8196B)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, (IS_32BIT ? "32bit" : "16bit"), cpu_speed);	
#endif
#endif
  #if defined(RTL8198)
	return_addr=0;
	prom_printf("\n---RealTek(RTL8198)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, "16bit", cpu_speed);	
  #endif
#endif

#ifdef RTL8196C
	return_addr=0;
#if defined(KLD)
	prom_printf("\n---RealTek(RTL8196C-kld)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, (IS_32BIT ? "32bit" : "16bit"), cpu_speed);			
#elif defined(EC)
	prom_printf("\n---RealTek(RTL8196C-ec) at %s %s [%s](%dMHz)\n", BOOT_CODE_TIME,B_VERSION, (IS_32BIT ? "32bit" : "16bit"),cpu_speed);
#else 
        prom_printf("\n---RealTek(RTL8196C)at %s %s [%s](%dMHz)\n",	BOOT_CODE_TIME,B_VERSION, "16bit", cpu_speed);	
#endif
#endif


#ifndef RTL8197B
	return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET;
	ret = check_system_image((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET,&header, &setting_header);
#if !defined(KLD)
	if(ret==0) {
		return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET2;		
		ret=check_system_image((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET2,  &header, &setting_header);
	}
	if(ret==0) {
		return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET3;				
		ret=check_system_image((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET3,  &header, &setting_header);
	}			
#endif
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE	
	i=LINUX_IMAGE_OFFSET_START;	
	while(i<=LINUX_IMAGE_OFFSET_END && (0==ret))
	{
		return_addr=(unsigned long)FLASH_BASE+i; 
		if(CODE_IMAGE_OFFSET == i || CODE_IMAGE_OFFSET2 == i || CODE_IMAGE_OFFSET3 == i){
			i += LINUX_IMAGE_OFFSET_STEP; 
			continue;
		}
		ret = check_system_image((unsigned long)FLASH_BASE+i, &header, &setting_header);
		i += LINUX_IMAGE_OFFSET_STEP; 
	}
#endif

#endif //RTL8197B
	if(ret==2)
        {
                ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET);
		// for EDX image
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET1);
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET2);
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET3);
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET4);
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET5);
//cypress 100715 edimax30
		  if(ret==0)
               ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET6);		
           if(ret==0)
               ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET7);
		  if(ret==0)
               ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET8);
           if(ret==0)
               ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET9);
		  if(ret==0)
               ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET10);
           if(ret==0)
               ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET11);
		  //cypress 100715 edimax30
		  if(ret==0)		//EDX Walter For Logitec linux too large
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+0x100000);
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+0x110000);
		  if(ret==0)
			  ret=check_rootfs_image((unsigned long)FLASH_BASE+0x120000);
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
		i = ROOT_IMAGE_OFFSET_START;
		while((i <= ROOT_IMAGE_OFFSET_END) && (0==ret))
		{
			if( ROOT_FS_OFFSET == i ){
				i += ROOT_IMAGE_OFFSET_STEP;
				continue;
			}
			ret = check_rootfs_image((unsigned long)FLASH_BASE+i);
			i += ROOT_IMAGE_OFFSET_STEP;
		}
#endif
	}
#if  defined(RTL8196B) || defined(RTL8198)
#ifndef CONFIG_FPGA_PLATFORM
#ifdef RTL8196C     //96c not fix gpio, so close first. fix CPU 390MHz cannot boot from flash.
	Init_GPIO();
#endif	
#endif
#endif
#ifndef RTL8197B

//rtk bootcode and enable post
#if defined(CONFIG_POST_ENABLE)
	if(ret && post_test_result)
#else
	if(ret)
#endif	
	

	{


		switch(user_interrupt(WAIT_TIME_USER_INTERRUPT))
		{
		case 0:
		default:
			//prom_printf("\n---%X\n",return_addr);
			word_ptr = (unsigned short *)&header;
			for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
				*word_ptr = rtl_inw(return_addr + i);
			
			// move image to SDRAM
			flashread( header.startAddr,	(unsigned int)(return_addr-FLASH_BASE+sizeof(header)), 	header.len-2);
			
			if ( !user_interrupt(0) )  // See if user escape during copy image
			{
				outl(0,GIMR0); // mask all interrupt
#if defined(CONFIG_BOOT_RESET_ENABLE) && !defined(KLD)
				Set_GPIO_LED_OFF();
#endif
#if defined(RTL8196B)
				REG32(0xb8010000)=0xe4000000; //speedup lexra access
#endif
#ifdef CONFIG_BOOT_TIME_MEASURE
			cp3_count_print();
#endif
				prom_printf("Jump to image start=0x%x...\n", header.startAddr);
				jump = (void *)(header.startAddr);
				
				cli();
				flush_cache(); 
				jump();				 // jump to start
			}
			/*i think i can comment this break*/
			/*if user_interrupt, then go to case 1 is ok too*/
			/*if user_not interrupt, then jump() to start linux image.*/
			//break;
		#ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
		case 3:
			//prom_printf("\n---%X\n",return_addr);
			word_ptr = (unsigned short *)&header;
			for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
				*word_ptr = rtl_inw(return_addr + i);
			
			// move image to SDRAM
			flashread( header.startAddr,	(unsigned int)(return_addr-FLASH_BASE+sizeof(header)), 	header.len-2);
			
			if ( !user_interrupt(0) )  // See if user escape during copy image
			{
				outl(0,GIMR0); // mask all interrupt
#ifdef CONFIG_BOOT_RESET_ENABLE
				Set_GPIO_LED_OFF();
#endif
#if defined(RTL8196B)
				REG32(0xb8010000)=0xe4000000; //speedup lexra access
#endif

				REG32(0xb8019004)=0xFE;
				int count=500;
				while(count--)
				{}
				if(REG32(0xb8019004)!=0xFE)
				{
					prom_printf("fail debug-Jump to image start=0x%x...\n", header.startAddr);

				}

				prom_printf("Debug-Jump to image start=0x%x...\n", header.startAddr);
				jump = (void *)(header.startAddr);
				
				cli();
				flush_cache(); 
				jump();				 // jump to start
			}
			/*i think i can comment this break*/
			/*if user_interrupt, then go to case 1 is ok too*/
			/*if user_not interrupt, then jump() to start linux image.*/
			//break;

		//break;
		#endif
		case 1:
#ifdef CONFIG_BOOT_TIME_MEASURE
			cp3_count_print();
#endif
			prom_printf("\n---Escape booting by user\n");

	
			cli();
#if defined(CONFIG_BOOT_RESET_ENABLE) && !defined(KLD)
			Set_GPIO_LED_ON();
#endif
#ifndef CONFIG_FPGA_PLATFORM
			eth_startup(0);

#endif
			sti();

			tftpd_entry();

#ifdef DHCP_SERVER
			//prom_printf("\nStart Dhcpd\n");		
			dhcps_entry();
#endif

#ifdef HTTP_SERVER
			httpd_entry();
#endif

#ifdef KLD
			ps_led_blink = 1;
			request_IRQ(14, &irq_timer, NULL);
#endif
#if !defined(KLD)
			REG32(0xb8000040)=REG32(0xb8000040)&(0xFFFFFFFF-0x00300000);
#endif

			#if defined (SW_8366GIGA)
			REG32(0xbb80414c) = 0x00037d16;  //P0GMII 
			REG32(0xbb804100) = 0x1;    //PITCR
			REG32(0xbb804104) = 0x00E80367;   //P0 Config Reg
			#endif
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (REG32(REVR) == RTL8196C_REVISION_B) //green ethernet initialization
			{
				unsigned short PHYID_Count; 
				for(PHYID_Count=0;PHYID_Count<=4;PHYID_Count++)
				enable_10M_power_saving(PHYID_Count , 0x18,0x0310);
			}
			#endif
			monitor();
			break;
		}/*switch case */
	}/*if image correct*/
	else
#endif //RTL8197B
	{
	
#ifndef CONFIG_FPGA_PLATFORM
  #ifndef CONFIG_RTL8198
  #if !defined(KLD)
		REG32(0xb8000040)=REG32(0xb8000040)&(0xFFFFFFFF-0x00300000);
	#endif
  #endif	
#endif


#ifndef CONFIG_FPGA_PLATFORM
		eth_startup(0);	
#endif

		#if defined (SW_8366GIGA)
			REG32(0xbb80414c) = 0x00037d16;
			REG32(0xbb804100) = 0x1;
			REG32(0xbb804104) = 0x00E80367;
		#endif

#ifdef CONFIG_BOOT_TIME_MEASURE
		cp3_count_print();
#endif
		
		prom_printf("\n---Ethernet init Okay!\n");

		sti();
#ifdef KLD
ps_led_blink = 1;
#endif

		tftpd_entry();

#ifdef DHCP_SERVER			
		dhcps_entry();
#endif

#ifdef HTTP_SERVER
		httpd_entry();
#endif
	#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
	{
		unsigned short PHYID_Count; 
        	for(PHYID_Count=0;PHYID_Count<=4;PHYID_Count++)
			enable_10M_power_saving(PHYID_Count , 0x18,0x0310);
	}
	#endif
		monitor();
	}
}

unsigned char tempBuffer[8*1024];

#ifdef RTL8197B
// return,  0: not found, 1: linux found, 2:linux with root found
int check_system_image(unsigned long addr,IMG_HEADER_Tp pHeader)
{
	// Read header, heck signature and checksum
	int i, ret=0;
	unsigned short sum=0, *word_ptr;
	unsigned short length=0;
	unsigned short temp16=0;

	if(gCHKKEY_HIT==1)
		return 0;
	
    /*check firmware image.*/
	word_ptr = (unsigned short *)pHeader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
		*word_ptr = *((unsigned short *)(addr + i));

	if (!memcmp(pHeader->signature, FW_SIGNATURE, SIG_LEN))
		ret=1;
//	else if  (!memcmp(pHeader->signature, FW_SIGNATURE_WITH_ROOT, SIG_LEN))
	else if  (!memcmp(pHeader->signature, FW_SIGNATURE_WITH_ROOT, 2)) //EDX check "cr" only
		ret=2;
	else 
		prom_printf("no sys signature at %X!\n",addr);
	
	if (ret) {
		for (i=0; i<pHeader->len; i+=2) {
			
#if 1 //slowly
			gCHKKEY_CNT++;
			if( gCHKKEY_CNT>ACCCNT_TOCHKKEY)
			{	gCHKKEY_CNT=0;
				if ( user_interrupt(0)==1 )  //return 1: got ESC Key
					return 0;
			}
#else  //speed-up, only support UART, not support GPIO
			if((Get_UART_Data()==ESC)  || (Get_GPIO_SW_IN()!=0))
			{	gCHKKEY_HIT=1; 
				return 0;
			}
#endif
			
			sum += rtl_inw(addr + sizeof(IMG_HEADER_T) + i);
		}	
		if ( sum ) {
			prom_printf("sys checksum error at %X!\n",addr-FLASH_BASE);



			ret=0;
		}


	}
	return (ret);
}
#else
// return,  0: not found, 1: linux found, 2:linux with root found
static int check_system_image(unsigned long addr,IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp setting_header)
{
	// Read header, heck signature and checksum
	int i, ret=0;
	unsigned short sum=0, *word_ptr;
	unsigned short length=0;
	unsigned short temp16=0;
	char image_sig_check[1]={0};
	char image_sig[4]={0};
	char image_sig_root[4]={0};
	if(gCHKKEY_HIT==1)
		return 0;
        /*check firmware image.*/
	word_ptr = (unsigned short *)pHeader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
		*word_ptr = rtl_inw(addr + i);
	
#if defined(KLD)
memcpy(image_sig, FW_SIGNATURE_WITH_ROOT_615, SIG_LEN);
#else
memcpy(image_sig, FW_SIGNATURE, SIG_LEN);
memcpy(image_sig_root, FW_SIGNATURE_WITH_ROOT, SIG_LEN);
#endif
#if defined(KLD)	
check_image_signature_tag(HW_SETTING_OFFSET+HW_SETTING_11N_RESERVED7_OFFSET, (&image_sig_check[0]));

if(image_sig_check[0] !=0x00){
	image_sig[3]=image_sig_check[0];
}
#endif
//prom_printf("image sig =%X-%X-%X-%X!\n",image_sig[0], image_sig[1], image_sig[2],image_sig[3]);
	#if defined(KLD)
		if  (!memcmp(pHeader->signature, image_sig, SIG_LEN))
			ret=2;
		else{
			prom_printf("no sys signature at %X!\n",addr-FLASH_BASE);
		}
	#else
		if (!memcmp(pHeader->signature, image_sig, SIG_LEN))
			ret=1;
//		else if  (!memcmp(pHeader->signature, image_sig_root, SIG_LEN))
		else if  (!memcmp(pHeader->signature, image_sig_root, 2)) //EDX check "cr" only
				ret=2;
		else{
				prom_printf("no sys signature at %X!\n",addr-FLASH_BASE);
			}
		
	#endif	
	
	if (ret) {
		for (i=0; i<pHeader->len; i+=2) {
/*	cypress 100723
#if 1  //slowly
			gCHKKEY_CNT++;
			if( gCHKKEY_CNT>ACCCNT_TOCHKKEY)
			{	gCHKKEY_CNT=0;
				if ( user_interrupt(0)==1 )  //return 1: got ESC Key
					return 0;
			}
#else  //speed-up, only support UART, not support GPIO
			if((Get_UART_Data()==ESC)  || (Get_GPIO_SW_IN()!=0))
			{	gCHKKEY_HIT=1; 
				return 0;
			}
#endif*/
			sum += rtl_inw(addr + sizeof(IMG_HEADER_T) + i);
		}	
		if ( sum ) {
			prom_printf("sys checksum error at %X!\n",addr-FLASH_BASE);
			ret=0;
		}
	}
	return (ret);
}

static int check_rootfs_image(unsigned long addr)
{
	// Read header, heck signature and checksum
	int i;
	unsigned short sum=0, *word_ptr;
	unsigned long length=0;
	unsigned char tmpbuf[16];	
	#define SIZE_OF_SQFS_SUPER_BLOCK 640
	#define SIZE_OF_CHECKSUM 2
	#define OFFSET_OF_LEN 2
	
	if(gCHKKEY_HIT==1)
		return 0;
	
	word_ptr = (unsigned short *)tmpbuf;
	for (i=0; i<16; i+=2, word_ptr++)
		*word_ptr = rtl_inw(addr + i);

	if ( memcmp(tmpbuf, SQSH_SIGNATURE, SIG_LEN) && memcmp(tmpbuf, SQSH_SIGNATURE_LE, SIG_LEN)) {
		prom_printf("no rootfs signature at %X!\n",addr-FLASH_BASE);
		return 0;
	}

	length = *(((unsigned long *)tmpbuf) + OFFSET_OF_LEN) + SIZE_OF_SQFS_SUPER_BLOCK + SIZE_OF_CHECKSUM;
	for (i=0; i<length; i+=2) {
#if 1  //slowly
			gCHKKEY_CNT++;
			if( gCHKKEY_CNT>ACCCNT_TOCHKKEY)
			{	gCHKKEY_CNT=0;
				if ( user_interrupt(0)==1 )  //return 1: got ESC Key
					return 0;
			}
#else  //speed-up, only support UART, not support GPIO.
			if((Get_UART_Data()==ESC)  || (Get_GPIO_SW_IN()!=0))
			{	gCHKKEY_HIT=1; 
				return 0;
			}
#endif			
		sum += rtl_inw(addr + i);
	}
#if 0 // EDX
	if ( sum ) {
		prom_printf("rootfs checksum error at %X!\n",addr-FLASH_BASE);
		return 0;
	}
#endif
	return 1;
}
#endif //RTL8197B

//------------------------------------------------------------------------------------------








//------------------------------------------------------------------------------------------
#if  defined(RTL8196B) || defined(RTL8198)
int user_interrupt(unsigned long time)
{
	int i;

	int button_press_detected=-1;
	int tickStart=0;
	tickStart=get_timer_jiffies();
	
	if(gCHKKEY_HIT==1)
		return 1;
	
	//SET_TIMER(time);	
	//while(1)
	//{
	//	dprintf("rx=%x\r\n",    );
	//}
	#if defined(KLD)	
	char image_sig_check[1]={0};
	check_image_signature_tag(HW_SETTING_OFFSET+HW_SETTING_11N_RESERVED10_OFFSET, (&image_sig_check[0]));

	if(image_sig_check[0] ==0x1){
			return 1;
	}
        #endif
	

	do {
		if  (Check_UART_DataReady() )
		{
	   		i=Get_UART_Data();
			Get_UART_Data();
            		if( i == ESC )
		       { 	//dprintf("User Press ESC Break Key\r\n");
		       	gCHKKEY_HIT=1;
		       	return 1;
            		}
		}
#ifndef RTL8197B
#ifndef CONFIG_FPGA_PLATFORM
		// polling if button is pressed --------------------------------------
		if (button_press_detected == -1 ||  button_press_detected == 1) {
#if defined(RTL8196B)

			//REG32(RTL_GPIO_MUX) = 0x300;
			//		REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<0) ); //set byte F GPIO7 = gpio
			//		REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<0) );  //0 input, 1 out
#endif
#if defined(RTL8196C)
/* EDX Jeff
			REG32(RTL_GPIO_MUX) =  REG32(RTL_GPIO_MUX)|0x00300000;
			REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<5) ); //set byte F GPIO7 = gpio
                        REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<5) );  //0 input, 1 out
 EDX Jeff */
	
#endif
			
#ifdef CONFIG_BOOT_RESET_ENABLE
			if ( Get_GPIO_SW_IN() )
#else
			if ( 0 )
#endif				
			{// button pressed
	    			//dprintf("User Press GPIO Break Key\r\n");
				if (button_press_detected == -1) 
				{
					//SET_TIMER(1*CPU_CLOCK); // wait 1 sec
				}
				button_press_detected = 1;
				gCHKKEY_HIT=1;
				return 1;	//jasonwang//wei add				

			}
      #ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
			else if(Get_GPIO_SW_IN_KERNEL())
			{
	      			if (button_press_detected == -1) 
				{
        				//SET_TIMER(1*CPU_CLOCK); // wait 1 sec
        			}
        			button_press_detected = 1;
				gCHKKEY_HIT=1;
				return 3;	//jasonwang//wei add					
			}
       #endif
			else
		      	button_press_detected = 0;
		}
                //---------------------------------------------------
                //dprintf("j=%x\r\n",get_timer_jiffies());
#endif
#endif //RTL8197B
	}
	//while ( !RTL_TIMEOUT());

//	while ( (get_timer_jiffies() - tickStart) < 300  );  // 3 sec
	while ( (get_timer_jiffies() - tickStart) < 100  );  // 1 sec

	//dprintf("timeout\r\n");
        if (button_press_detected>0)
        {    gCHKKEY_HIT=1;    
                return 1;
        }	

	return 0;

}
#endif

//------------------------------------------------------------------------------------------
 /*Boot Code*/
 /******region 0x0000******/
 /**********************/
 /*16k                 */
 /*            0x4000  */
 /**********************/
 /* 8k                 */
 /*            0x6000  */
 /**********************/
 /******region 0x6000******/

 /**********************/
 /* 8k                 */
 /*            0x8000  */
 /**********************/


 /*            from   0x6000*/
 /*First 1K      to   0x6400*/
 /***************************/
 /*            from   0x6400*/
 /*Next  7K      to   0x8000*/
 /***************************/


 /***************************/
 /* 32k        from   0x8000*/
 /*              to   0x10000*/
 /****************************/

// Cyrus: now check system image will become 3 steps...
// Cyrus: first: first sector, second second sector.
// Cyrus --------------------------------------------------------------------
/*this 8K sector will be Hardware and Default Software setting*/
#if 0 //UNUSED
#ifndef RTL8197B
int check_setting_image(unsigned long addr, SETTING_HEADER_Tp pHeader,char * type)
{
	int i;
	unsigned short *word_ptr;

	unsigned char TARGET;
	unsigned char sum=0;
	word_ptr = (unsigned short *)pHeader;
        /*Get the Header back from FLASH, and we have to use 16-bit access*/
	for (i=0; i<sizeof(SETTING_HEADER_T); i+=2, word_ptr++)
		{
		 *word_ptr = rtl_inw(addr + i);
		 //prom_printf("Read %X\n",*word_ptr);
                 }

	/*Check the Header signature */
	if (memcmp(pHeader->Tag, type,2)) {
		prom_printf("no tag! type %X,%X\n",*type,*(type+1));
		return 0;
	}

	/*Check the setting checksum*/
	for(i=0;i<pHeader->len;i++)/*plus the last byte checksum*/
	   {
	    if ( flashread((unsigned long)&TARGET,addr-(unsigned long)FLASH_BASE+sizeof(SETTING_HEADER_T)+i,1) )
	        {
	         sum=sum+TARGET;
	        //prom_printf("%X,%X,%X\n",TARGET,i,sum);
	        }

	    else
	        return 0;
	   }

            //prom_printf("%X\n",sum);

	if (sum) {
		prom_printf("data checksum error! type %X,%X\n",*type,*(type+1));
		return 0;
	}
        //prom_printf("OK\n");
        if( type == SW_SIGNATURE_C)
          {
           /*Go get the version number.*/
           CSversion=rtl_inw((unsigned long)FLASH_BASE+CS_IMAGE_OFFSET+2);
           //prom_printf("additional check for CSversion %X!\n",CSversion);
           if(DSversion!=CSversion)
              {
               prom_printf("current version error, restore default setting\n");
               return 0;
              }
          }
	return 1;
}
#endif
#endif //RTL8197B



#define DDR_DBG 0
#define PROMOS_DDR_CHIP 1
#define IMEM_DDR_CALI_LIMITS 60 //1sec=30 times
#define __IRAM_IN_865X      __attribute__ ((section(".iram-rtkwlan")))
#define __IRAM_FASTEXTDEV        __IRAM_IN_865X
#ifdef DDR_SDRAM
__IRAM_FASTEXTDEV
void DDR_cali_API7(void)
{
   
       
        int i,j,k;
	  
        int L0 = 0, R0 = 33, L1 = 0, R1 = 33;
        int  DRAM_ADR = 0xA0a00000;
        int  DRAM_VAL = 0x5A5AA5A5;
        int  DDCR_ADR = 0xB8001050;
        int  DDCR_VAL = 0x80000000; //Digital
        //int  DDCR_VAL = 0x0; //Analog ,JSW:Runtime will dead even in IMEM
 

        *((volatile unsigned int *)(DRAM_ADR)) = DRAM_VAL;
 
        while( (*((volatile unsigned int *)(0xb8001050))& 0x40000000) != 0x40000000);
        while( (*((volatile unsigned int *)(0xb8001050))& 0x40000000) != 0x40000000);

    for(k=1;k<=IMEM_DDR_CALI_LIMITS;k++)  //Calibration times
    //while(1)
    {
	 
         // Calibrate for DQS0
         for (i = 1; i <= 31; i++)
         {
       #if DDR_DBG
            __delay(10);
             dprintf("\nDQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
	#endif

	#if PROMOS_DDR_CHIP
		__delay(100);  //__delay(1)=5ns
	#endif
	
            *((volatile unsigned int *)(DDCR_ADR)) = (DDCR_VAL & 0x80000000) | ((i-1) << 25);
 
            if (L0 == 0)
            {
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0x00FF00FF) == 0x005A00A5)
               {
                  L0 = i;
               }
            }
            else
            {       
            #if DDR_DBG
	        dprintf("\nDRAM(0x%x)=%x\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
	     #endif
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0x00FF00FF) != 0x005A00A5)
               {
                  //dprintf("\n\n\nError!DQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
             #if DDR_DBG
	           dprintf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
		#endif
                  R0 = i - 1;
                  //R0 = i - 3;  //JSW
                  break;
               }
            }
         }
         DDCR_VAL = (DDCR_VAL & 0xC0000000) | (((L0 + R0) >> 1) << 25); // ASIC
         *(volatile unsigned int *)(DDCR_ADR) = DDCR_VAL;
 
         // Calibrate for DQS1
         for (i = 1; i <= 31; i++)
         {
          #if DDR_DBG
            __delay(10);
             dprintf("\nDQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
	#endif

	#if PROMOS_DDR_CHIP
		__delay(100);  //__delay(1)=5ns
	#endif
	       
             *(volatile unsigned int *)(DDCR_ADR) = (DDCR_VAL & 0xFE000000) | ((i-1) << 20);
 
            if (L1 == 0)
            {
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0xFF00FF00) == 0x5A00A500)
               {
                  L1 = i;
               }
            }
            else
            {
            #if DDR_DBG
               dprintf("\nDRAM(0x%x)=%x\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
		#endif
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0xFF00FF00) != 0x5A00A500)
               {
                 //dprintf("\n\n\nError!DQS1(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
	          // dprintf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
                  R1 = i - 1;
		    //R1 = i - 3;
                  break;
               }
            }
         }
 
         DDCR_VAL = (DDCR_VAL & 0xFE000000) | (((L1 + R1) >> 1) << 20); // ASIC
         *(volatile unsigned int *)(DDCR_ADR) = DDCR_VAL;
 
        /* wait a little bit time, necessary */
       // for(i=0; i < 10000000; i++);
       __delay(100);
 	#if 1  
		#if DDR_DBG
		        dprintf("\nR0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
		        dprintf("\nR1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
		#endif
                	
            

	//Over DDR 200MHZ ,modify DDCR DQS_TAP
	unsigned int ck_m2x_freq_sel=READ_MEM32(HW_STRAP_REG) & 0x1c00;
	//if ((k==IMEM_DDR_CALI_LIMITS) && (ck_m2x_freq_sel==0x1c00))//only for 200MHZ
	if ((k==IMEM_DDR_CALI_LIMITS) )
	{
              #if DDR_DBG
		  dprintf("\nR0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
		  dprintf("\nR1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
		  dprintf("\n=>After IMEM Cali,DDCR(%d)=0x%x\n\n",k ,READ_MEM32(DDCR_REG));
              #endif
		  unsigned short DQS_TAP_C0=(L0 + R0) >> 1;
		  unsigned short DQS_TAP_C1=(L1 + R1) >> 1;
		  //anson add
		  if(ck_m2x_freq_sel==0x1c00)
		  {
		  	if(DQS_TAP_C0 >= 9)
		  		DQS_TAP_C0 = DQS_TAP_C0-3;
		  	else if(DQS_TAP_C0 >= 6)
		  		DQS_TAP_C0 = DQS_TAP_C0-2;
		  	else if(DQS_TAP_C0 >= 5)
		  		DQS_TAP_C0 = DQS_TAP_C0-1;
		  	else
		  		DQS_TAP_C0 = DQS_TAP_C0;
		  //********************************************//
		  	if(DQS_TAP_C1 >= 9)
		  		DQS_TAP_C1 = DQS_TAP_C1-3;
		  	else if(DQS_TAP_C1 >= 6)
		  		DQS_TAP_C1 = DQS_TAP_C1-2;
		  	else if(DQS_TAP_C1 >= 5)
		  		DQS_TAP_C1 = DQS_TAP_C1-1;
		  	else
		  		DQS_TAP_C1 = DQS_TAP_C1;
		  }
		  //anson add end

		  //DDCR_VAL = (DDCR_VAL & 0x80000000) | (DQS_TAP_C1 << 20) | (DQS_TAP_C0 << 25); // Digital
		     DDCR_VAL = (DDCR_VAL & 0x0) | (DQS_TAP_C1 << 20) | (DQS_TAP_C0 << 25); // Analog
		   *(volatile unsigned int *)(DDCR_ADR) = DDCR_VAL;
		 __delay(100);
		 #if DDR_DBG
		  dprintf("\n=>After DQS_TAP Modified,DDCR=0x%x,C0=%d,C1=%d\n\n",READ_MEM32(DDCR_REG),DQS_TAP_C0,DQS_TAP_C1);
		 #endif
	}
	//dprintf("\n=================================\n");
	#endif
	 
	
    	}//end of while(1)	

}
#endif


#if 0
int flashwrite(unsigned long dst, unsigned long src, unsigned long length)
{
 	spi_flw_image(0,dst,src,length);
}
 
 

int flashinit()
{
 
 	dprintf("SPI Probe \n");
	 spi_probe();
}
#endif
#if 0 //UNUSED
int check_dram_freq_reg(void)                       //JSW:For 8196C
{
    unsigned short dram_freq_reg_bit,dram_freq;
    dram_freq_reg_bit= ((0x00001C00) & (REG32(HW_STRAP_REG)))>>10 ;
    switch(dram_freq_reg_bit)
    {
        case 0:
            //printf("\nDefault 1x DRAM clock : 65 MHZ\n");
            dram_freq=65;
            break;

        case 1:
            //dprintf("\nDefault 1x DRAM clock : 78 MHZ\n");
            dram_freq=78;
            break;

        case 2:
            //dprintf("\nDefault 1x DRAM clock : 125 MHZ\n");
            dram_freq=125;
            break;

        case 3:
            //dprintf("\nDefault 1x DRAM clock : 150 MHZ\n");
            dram_freq=150;
#if 0
            if(READ_MEM32(MCR_REG)&(0x80000000))
            {                                       //Adapt DDR setting here
                REG32(DTR_REG)=0x48A90840;          //safe parameter for DDR PCIE test,from DanielWu @20090213
                __delay(10);
            }
            else
            {                                       //Adapt SDR setting here
                __delay(10);
            }
#endif
            break;

        case 4:
            //dprintf("\nDefault 1x DRAM clock : 156 MHZ\n");
            dram_freq=156;
#if 0
            if(READ_MEM32(MCR_REG)&(0x80000000))
            {                                       //Adapt DDR setting here
                REG32(DTR_REG)=0x48A90840;          //Not confirm yet
                __delay(10);

            }
            else
            {                                       //Adapt SDR setting here
                REG32(DTR_REG)=0x48EA0C80;          //SDR 148MHZ- (from DanielWu)
                __delay(100);

            }
#endif
            break;

        case 5:
            dprintf("\nDefault 1x DRAM clock : 168 MHZ\n");
            dram_freq=168;
#if 0
            if(READ_MEM32(MCR_REG)&(0x80000000))
            {                                       //Adapt DDR setting here
                REG32(DTR_REG)=0x48EB0A80;          // DDR166MHZ from DanielWu
        //REG32(DTR_REG)=0xffff05c0;  // DDR166MHZ from DanielWu
                dprintf("\nDDR's MCR(0xb8001000)=%x\n",READ_MEM32(MCR_REG));

            }
            else
            {                                       //Adapt SDR setting here
                REG32(DTR_REG)=0x6CEA0A80;          //SDR 166MHZ- (from ghHuang)
                dprintf("\nSDR's MCR(0xb8001000)=%x\n",READ_MEM32(MCR_REG));

            }
#endif
            break;

        case 6:
            dprintf("\nDefault 1x DRAM clock : 193 MHZ\n");
            dram_freq=193;
#if 0
            if(READ_MEM32(MCR_REG)&(0x80000000))
            {                                       //Adapt DDR setting here
                REG32(DTR_REG)=0x48EB0A80;          //Not confirm yet

            }
            else
            {                                       //Adapt SDR setting here
                REG32(DTR_REG)=0x6CEA0A80;          //Not confirm yet

            }
#endif
            break;


        default :
            dprintf("Error ck_m2x_freq_sel number,should be 0~6");
            break;

    }

        /*set DTR(0xb8001008) , and remember DCR(0xb8001004) should be set after DTR*/
    REG32(0xb8001004)|=READ_MEM32(0xb8001004);
    return dram_freq;
}
#endif
int enable_10M_power_saving(int phyid , int regnum,int data)
{
   
    unsigned int uid,tmp;  
     rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );
     uid=tmp;
     uid =data;
     //dprintf("\nBefore or,uid =%x",tmp);
     //dprintf("\nAfter or,uid =%x\n",uid);
    //dprintf("\nSet enable_10M_power_saving00!\n");
    rtl8651_setAsicEthernetPHYReg( phyid, regnum, uid );
    //dprintf("\nSet enable_10M_power_saving01!\n");
    rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );
    //dprintf("\nSet enable_10M_power_saving02!\n");
    uid=tmp;
    //dprintf("After setting,PHYID=0x%x ,regID=0x%x, Find PHY Chip! UID=0x%x\r\n", phyid, regnum, uid);

}

#if defined(KLD)

int check_image_signature_tag(unsigned long addr, char *tag)
{
	int i;

	unsigned char TARGET[1]={0};

	TARGET[0]=rtl_inb(FLASH_BASE+addr);
	if(TARGET[0]==0x00 || TARGET[0]==0xFF)
		return 0;
	else{
		*tag=TARGET[0];
	}
	return 1;
}
#endif
