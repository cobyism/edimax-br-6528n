#ifndef __RTL_START_H__
#define __RTL_START_H__

//----- System Registers ------------------------------------------------
#define REG_REVISION	        0xbd01010c
#define	REG_PLLMNR		0xbd010104
#define	REG_SYSCLKR		0xbd010108
#define	REG_GPEFDIR		0xbd010144
#define	REG_GPEFDATA	        0xbd010140
#define	REG_MTCR0		0xbd011004
#define	REG_MCR			0xbd011000
#define	REG_MTCR1		0xbd011008
#define	REG_TKNR		0xbd010110
#define	REG_GISR		0xbd010004
#define	REG_WDTCNR		0xbd01005c
#define	REG_CDBR		0xbd010058

//----- Revision bit ----------------------------------------------------
#define VERSION_B		0x0		//0000
#define	VERSION_C		0x8		//1000
#define VERSION_D		0x4		//0100

//----- DPLL and Clock Setting ------------------------------------------
#define VAL_PLLMNR_8186B	0x36704		// 40*40/5=320 MHz
#define VAL_SYSCLKR_8186B	0xa0b		// CPU=320/2 MEM=320/2.5

//#define VAL_PLLMNR_8186C	0x35a03		// 180/108
//#define VAL_SYSCLKR_8186C	0x090b		// CPU=270/1.5 MEM=270/2.5

#define VAL_PLLMNR_8186C	0x36303		// 360MHz
#define VAL_SYSCLKR_8186C	0x0a0c		// CPU=360/2 MEM=360/3


// #define VAL_PLLMNR_8186C	0x36703		// 40*40/4=400 MHz
// #define VAL_SYSCLKR_8186C	0x0d0d		// CPU=100 MEM=100

// #define VAL_PLLMNR_8186C	0x35f03		// 40*320/4=320 MHz
// #define VAL_PLLMNR_8186C	0x36303		// 40*360/4=360 MHz
// #define VAL_SYSCLKR_8186C	0x0d0d		// CPU=360/4=90 MEM=360/4=90
// #define VAL_SYSCLKR_8186C	0x0e0e		// CPU=360/6=60 MEM=360/6=60
// #define VAL_SYSCLKR_8186C	0x0f0f		// CPU=360/6=60 MEM=360/6=45

//----- Memory Setting --------------------------------------------------
#define	VAL_FLASH_TIMING	0x88880000
//#define	VAL_FLASH_TIMING	0x33330000	//high speed.


//----- Others ----------------------------------------------------------
#define PCI_LOW_COUNTER	0xFFFF
#define	BOOT_ADDR		0x80100000	//compress
//#define	BOOT_ADDR		0x80000000    // no compress






//-------------------------------------------------
// Using register: t6, t7
#define REG32_W(addr,v) li t6,v;\
						li t7,addr;\
						sw t6, 0(t7);\
						nop;

// Using register: t5, t6, t7
#define UART_PRINT(addr,msg)		la   t5,msg;\
								1:	lb  t6,0(t5);\		
									beqz		t6,  2f;\
									addu	t5, 1;\	
									              ;\
									sll t6,t6,24;\
									li t7,addr;\
									sw t6, 0(t7);\
									nop	;\
									nop	;\									
									j 1b;\
									nop;\
									nop;\									
								2:	



//uart register
#define IO_BASE         0xB8000000
#define UART_RBR	(0x2000+IO_BASE)
#define UART_THR	(0x2000+IO_BASE)
#define UART_DLL	(0x2000+IO_BASE)
#define	UART_IER	(0x2004+IO_BASE)
#define	UART_DLM	(0x2004+IO_BASE)
#define	UART_IIR	(0x2008+IO_BASE)
#define	UART_FCR	(0x2008+IO_BASE)
#define UART_LCR	(0x200c+IO_BASE)
#define	UART_MCR	(0x2010+IO_BASE)
#define	UART_LSR	(0x2014+IO_BASE)
#define	UART_MSR	(0x2018+IO_BASE)
#define	UART_SCR	(0x201c+IO_BASE)



//---------------------------------------
#define SYS_CLK_RATE	  	(200000000) 	
//#define SYS_CLK_RATE	  	(  33868800)      //33.8688MHz
#define BAUD_RATE	  	(38400) 


//----------------------------------------------------






#endif

