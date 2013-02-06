/*
 * include/linux/serial_reg.h
 *
 * Copyright (C) 1992, 1994 by Theodore Ts'o.
 *
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL)
 *
 * These are the UART port assignments, expressed as offsets from the base
 * register.  These assignments should hold for any serial port based on
 * a 8250, 16450, or 16550(A).
 */

#ifndef _LINUX_SERIAL_REG_H
#define _LINUX_SERIAL_REG_H

/* Register List */

#define UART_UCR		0x04	/* Out: Uart Control Register */
#define UART_IER		0x08	/* Out: Interrupt Enable Register */
#define UART_BAUDREG	0x0C	/* Out: Baud Rate Control Register */
#define UART_MSR		0x24	/* In:  Modem Status Register */
#define UART_TXREG      0x10	/* In:  Tx Reg  Register */
#define UART_TXBUF      0x18	/* In:  Tx Buf Status Register */
#define UART_RXBUF      0x1C	/* In:  RxBuf Status Register */
#define UART_RXREG      0x14	/* In:  RxReg  Register */
#define UART_ITR	   	0x20    /* In:  Interrupt Status Register */

/* definition for Uar Control Register */
#define UART_UCR_PARITY	0x80000000	/* Parity Enable */
#define UART_UCR_EPAR	0x40000000	/* Even parity select */
#define UART_UCR_STOP	0x20000000	/* Stop bits: 0=1 stop bit, 1= 2 stop bits */
#define UART_UCR_WLEN7  0x10000000	/* Wordlength: 7 bits */
#define UART_UCR_SBC    0x02000000

/* definition for Uart Interrupt Enable Register */
#define UART_IER_TXEMP 0x80000000
#define UART_IER_RXFUL 0x40000000
#define UART_IER_RXOLD 0x20000000
#define UART_IER_RXOE  0x10000000
#define UART_IER_RXBRK 0x08000000
#define UART_IER_MSC   0x02000000


/* definition for Uart TxReg Register */

/* definition for Uart RxReg Register */
#define UART_LSR_BI	0x00002000
#define UART_LSR_FE	0x00008000
#define UART_LSR_DR	0x00000800
#define UART_LSR_PE	0x00004000
#define UART_LSR_OE	0x00001000

/* defintion for Uart TxBuf register */
#define UART_TXBUF_EMPTY 0x80000000   	
#define UART_TXBUF_RST 	 0x00008000	
#define UART_TXBUF_LEN 	 0x3F	


/* defintion for Uart TxBuf register */
#define UART_RXBUF_FULL  0x80000000                                           
#define UART_RXBUF_RST   0x00008000
#define UART_RXBUF_LEN   0x3F


/* definition for Interrupt Status Register */
#define UART_ITR_TXEMP 0x80000000
#define UART_ITR_RXFUL 0x40000000
#define UART_ITR_RXOLD 0x20000000
#define UART_ITR_RXOE  0x10000000
#define UART_ITR_MSC   0x02000000

/*
 * These are the definitions for the Modem Status/Control Register
 */
#define UART_MSR_DCD	0x80000000	/* Data Carrier Detect */
#define UART_MSR_RI	    0x40000000	/* Ring Indicator */
#define UART_MSR_DSR	0x20000000	/* Data Set Ready */
#define UART_MSR_CTS	0x10000000	/* Clear to Send */
#define UART_MSR_DDCD	0x08000000	/* Delta DCD */
#define UART_MSR_TERI	0x04000000	/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02000000	/* Delta DSR */
#define UART_MSR_DCTS	0x01000000	/* Delta CTS */
#define UART_MSR_DTR	0x00800000	/* Delta CTS */
#define UART_MSR_RTS	0x00400000	/* Delta CTS */
#define UART_MSR_ANY_DELTA 0x0F000000	/* Any of the delta bits! */


#endif /* _LINUX_SERIAL_REG_H */

