



#ifndef _RTL8196_H
#define _RTL8196_H


/*Cyrus Tsai*/
#ifndef __ASSEMBLY__
typedef unsigned long Int32;  
typedef unsigned short Int16;  
typedef unsigned char Int8;  
//
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;
typedef char CHAR8;
typedef unsigned char UCHAR8;




#define Virtual2Physical(x)		(((Int32)x) & 0x1fffffff)
#define Physical2Virtual(x)		(((Int32)x) | 0x80000000)
#define Virtual2NonCache(x)		(((Int32)x) | 0x20000000)
#define Physical2NonCache(x)		(((Int32)x) | 0xa0000000)
#define UNCACHE_MASK			0x20000000  //wei add

#endif

#ifndef TRUE
	#define TRUE		0x01
	#define FALSE	0x0
#endif	

#define BIT(x)	(1 << (x))

#define rtl_inb(offset)		(*(volatile unsigned char *)(mips_io_port_base + offset))
#define rtl_inw(offset)	(*(volatile unsigned short *)(mips_io_port_base + offset))
#define rtl_inl(offset)		(*(volatile unsigned long *)(mips_io_port_base + offset))

#define rtl_outb(offset,val)	(*(volatile unsigned char *)(mips_io_port_base + offset) = val)
#define rtl_outw(offset,val)	(*(volatile unsigned short *)(mips_io_port_base + offset) = val)
#define rtl_outl(offset,val)	(*(volatile unsigned long *)(mips_io_port_base + offset) = val)



#define mips_io_port_base (0xB8000000)



/*mips_io_port_base + FLASH_BASE =0xbfc0 0000   */
#ifdef RTL8196B
/*Replace the 0xbfc0 0000 with 0xbd00 0000      */
#define FLASH_BASE 		0x05000000 //JSW : 8672/8196 OCP
#else
/*Replace the 0xbfc0 0000 with 0xbe00 0000      */
#define FLASH_BASE 		0x06000000
#endif






// For BUS_MASTER

// For Uart1 Controller 865x
#define UART_RBR	0x2000
#define UART_THR	0x2000
#define UART_DLL	0x2000
#define	UART_IER	0x2004
#define	UART_DLM	0x2004
#define	UART_IIR	0x2008
#define	UART_FCR	0x2008
#define UART_LCR	0x200c
#define	UART_MCR	0x2010
#define	UART_LSR	0x2014
#define	UART_MSR	0x2018
#define	UART_SCR	0x201c


// For Uart1 Flags
#define UART_RXFULL	BIT(0)
#define UART_TXEMPTY	(BIT(6) | BIT(5))
#define UART_RXFULL_MASK	BIT(0)
#define UART_TXEMPTY_MASK	BIT(1)



// For Interrupt Controller
#define GIMR0	0x3000
#define GISR		0x3004
#define IRR0		0x3008
#define IRR1		0x300c
#define IRR2		0x3010
#define IRR3		0x3014


// For General Purpose Timer/Counter
#define TC0DATA		0x3100
#define TC1DATA		0x3104
#define TC0CNT		0x3108
#define TC1CNT		0x310c
#define TCCNR		0x3110
#define TCIR			0x3114
#define BTDATA		0x3118
#define WDTCNR		0x311c




//-----------------------------------------------------------------------
/* Register access macro
*/
#if 0
#define REG32(reg) (*(volatile unsigned int *)(reg))
#define REG8(reg) (*(volatile unsigned char *)(reg))
#endif

#define MEM_CONTROLLER_REG	0xB8001000	// memory contoller register

/*GPIO register */
#define GPIO_BASE                           0xB8003500
#define PABCDCNR_REG                            (0x000 + GPIO_BASE)     /* Port ABCD control */
#define PABCDPTYPE_REG                          (0x004 + GPIO_BASE)     /* Port ABCD type */
#define PABCDDIR_REG                             (0x008 + GPIO_BASE)     /* Port ABCD direction */
#define PABCDDAT_REG                             (0x00C + GPIO_BASE)     /* Port ABCD data */
#define PABCDISR_REG                             (0x010 + GPIO_BASE)     /* Port ABCD interrupt status */
#define PABIMR_REG                               (0x014 + GPIO_BASE)     /* Port AB interrupt mask */
#define PCDIMR_REG                               (0x018 + GPIO_BASE)     /* Port CD interrupt mask */
#define PEFGHCNR_REG                             (0x01C + GPIO_BASE)     /* Port ABCD control */
#define PEFGHPTYPE_REG                           (0x020 + GPIO_BASE)     /* Port ABCD type */
#define PEFGHDIR_REG                             (0x024 + GPIO_BASE)     /* Port ABCD direction */
#define PEFGHDAT_REG                             (0x028 + GPIO_BASE)     /* Port ABCD data */
#define PEFGHISR_REG                             (0x02C + GPIO_BASE)     /* Port ABCD interrupt status */
#define PEFIMR_REG                               (0x030 + GPIO_BASE)     /* Port AB interrupt mask */
#define PGHIMR_REG                               (0x034 + GPIO_BASE)     /* Port CD interrupt mask */

/* Timer control registers 
*/
#define GICR_BASE                           0xB8003000
#define TC0DATA_REG                             (0x100 + GICR_BASE)       /* Timer/Counter 0 data */
#define TC1DATA_REG                             (0x104 + GICR_BASE)       /* Timer/Counter 1 data */
#define TC0CNT_REG                              (0x108 + GICR_BASE)       /* Timer/Counter 0 count */
#define TC1CNT_REG                              (0x10C + GICR_BASE)       /* Timer/Counter 1 count */
#define TCCNR_REG                               (0x110 + GICR_BASE)       /* Timer/Counter control */
#define TCIR_REG                                (0x114 + GICR_BASE)       /* Timer/Counter intertupt */
#define CDBR_REG                                (0x118 + GICR_BASE)       /* Clock division base */
#define WDTCNR_REG                              (0x11C + GICR_BASE)       /* Watchdog timer control */


/* Global interrupt control registers 
*/
#define GICR_BASE                           0xB8003000
#define GIMR_REG                                (0x000 + GICR_BASE)       /* Global interrupt mask */
#define GISR_REG                                (0x004 + GICR_BASE)       /* Global interrupt status */
#define IRR_REG                                 (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR1_REG                                (0x00C + GICR_BASE)       /* Interrupt routing */
#define IRR2_REG                                (0x010 + GICR_BASE)       /* Interrupt routing */
#define IRR3_REG                                (0x014 + GICR_BASE)       /* Interrupt routing */


#define HW_STRAP_REG 0xb8000008
#define BIST_CONTROL_REG 0xb8000014
#define BIST_DONE_REG 0xb8000018
#define BIST_FAIL_REG 0xb800001C
#define DDCR_REG 0xb8001050



#define ALL_PORT_MASK	0x3F


//-----------------------------------------------------------------------
extern int dprintf(char *fmt, ...);
#define printf dprintf
#define prom_printf dprintf

#define CONFIG_RTL865XC 1


#endif
