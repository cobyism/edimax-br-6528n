

#define mips_io_port_base	0xB4000000

#define wpreg_wwrite(value, address)	\
do {	\
	*(volatile unsigned long *)(address + mips_io_port_base) = (unsigned long)(value);\
} while(0);\

#define wpreg_wread(buf , address)		\
do {	\
	(unsigned long)(buf) = *(volatile unsigned long *)(address + mips_io_port_base);	\
} while(0);\


#define UART1_REG1	0x80
#define UART1_REG2	0x84
#define UART1_INTENB	0x88
#define UART1_BAUD	0x8C
#define UART1_TXREG	0x90
#define UART1_RXREG	0x94
#define UART1_TXBF	0x98
#define UART1_RXBF	0x9C
#define UART1_INSTA	0xA0
#define UART1_MSREG	0xA4
#define UART1_MCREG	0xA8

#define UART2_REG1	0xc0
#define UART2_REG2	0xc4
#define UART2_INTENB	0xc8
#define UART2_BAUD	0xcC
#define UART2_TXREG	0xd0
#define UART2_RXREG	0xd4
#define UART2_TXBF	0xd8
#define UART2_RXBF	0xdC
#define UART2_INSTA	0xe0
#define UART2_MSREG	0xe4
#define UART2_MCREG	0xA8

#define B921600	0
#define	B115200	4
#define B57600	8
#define	B38400	12
#define	B9600	48
#define B4800	96

#define UART_ITR_TXEMPTY	0x80000000
#define UART_ITR_RXFULL		0x40000000
#define UART_ITR_RXOLD		0x20000000
#define UART_ITR_RXOVR		0x10000000



#define TL1	0x440
#define TL2	0x444
#define TL3	0x448


#define TH1	0x44C
#define TH2	0x450
#define TH3	0x454


#define TP	0x458
#define TCS	0x45c
#define TM1	0x460


#define TM2	0x464
#define TM3	0x468

#define WM	0x46c
#define WS	0x470
#define TIS	0x474

#define TIM	0x478
#define TCR	0x47c


#define IMR	0x0400
#define IRR	0x040C

