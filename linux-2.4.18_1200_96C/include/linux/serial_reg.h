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

#define UART_UCR		(0xCF-0xc3)	/* Out: Uart Control Register */
#define UART_IER		(0xC7-0xc3)	/* Out: Interrupt Enable Register */
#define UART_ITR	   	(0xD7-0xc3)    /* In:  Interrupt Status Register */
#define UART_IIR	   	(0xCB-0xc3)    /* In:  Interrupt Status Register */
#define UART_LSR	   	(0xD7-0xc3)    /* In:  Interrupt Status Register */
#define UART_BAUDREG		(0xc3-0xc3)	/* Out: Baud Rate Control Register */
#define UART_TX     		(0xc3-0xc3)	/* In:  Tx Reg  Register */
#define UART_RX     		(0xc3-0xc3)	/* In:  Tx Buf Status Register */


/* definition for Uart Interrupt Enable Register */
#define UART_IER_THRI    ( 1 << 1)	
#define UART_IER_RX      (1 << 0)	


/* definition for Uart TxReg Register */

/* definition for Uart RxReg Register */
#define UART_LSR_BI	0x10
#define UART_LSR_FE	0x8
#define UART_LSR_DR	0x1
#define UART_LSR_PE	0x4
#define UART_LSR_OE	0x2
#define UART_LSR_TEMP	0x60

/* definition for Interrupt Status Register */
#define UART_ITR_TXEMP ((1 << 6) | (1 << 5)) 
#define UART_ITR_RXFUL (1 << 0) 


#define UART_MCR_LOOP	0x10	/* Enable loopback test mode */
#define UART_MCR_OUT2	0x08	/* Out2 complement */
#define UART_MCR_OUT1	0x04	/* Out1 complement */
#define UART_MCR_RTS	0x02	/* RTS complement */
#define UART_MCR_DTR	0x01	/* DTR complement */

#define UART_MSR_DCD	0x80	/* Data Carrier Detect */
#define UART_MSR_RI	0x40	/* Ring Indicator */
#define UART_MSR_DSR	0x20	/* Data Set Ready */
#define UART_MSR_CTS	0x10	/* Clear to Send */
#define UART_MSR_DDCD	0x08	/* Delta DCD */
#define UART_MSR_TERI	0x04	/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02	/* Delta DSR */
#define UART_MSR_DCTS	0x01	/* Delta CTS */
#define UART_MSR_ANY_DELTA 0x0F	/* Any of the delta bits! */


#define UART_LCR_DLAB	0x80	/* Divisor latch access bit */
#define UART_LCR_SBC	0x40	/* Set break control */
#define UART_LCR_SPAR	0x20	/* Stick parity (?) */
#define UART_LCR_EPAR	0x10	/* Even parity select */
#define UART_LCR_PARITY	0x08	/* Parity Enable */
#define UART_LCR_STOP	0x04	/* Stop bits: 0=1 stop bit, 1= 2 stop bits */
#define UART_LCR_WLEN5  0x00	/* Wordlength: 5 bits */
#define UART_LCR_WLEN6  0x01	/* Wordlength: 6 bits */
#define UART_LCR_WLEN7  0x02	/* Wordlength: 7 bits */
#define UART_LCR_WLEN8  0x03	/* Wordlength: 8 bits */
#endif /* _LINUX_SERIAL_REG_H */

