/*
 * init.c: early initialisation code for R39XX Class PDAs
 *
 * Copyright (C) 1999 Harald Koerfgen
 *
 * $Id: init.c,v 1.1 2009/11/13 13:22:47 jasonwang Exp $
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <linux/circ_buf.h>
#include <asm/io.h>


#if defined(RTL8196B)	
#include <asm/rtl8196.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif

#if defined(CONFIG_SERIAL_SC16IS7X0_CONSOLE)
#include <linux/delay.h>
#endif



//#include <linux/init.h>
//#include <linux/config.h>
//#include <linux/kernel.h>
//#include <asm/rtl8181.h>
//#include <asm/io.h>


#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
extern int early_sc16is7x0_init_i2c_and_check( void );
extern unsigned int sc16is7x0_serial_out_i2c(int offset, int value);
extern unsigned int sc16is7x0_serial_in_i2c(int offset);
extern int sc16is7x0_err;
#define UART_MEM2REG( m )	( ( m - UART_RBR ) / 4 )
#endif


void serial_outc(char c)
{
	int i;
	i=0;

#ifndef CONFIG_SERIAL_SC16IS7X0_CONSOLE

#if 1
    while (1)
	{
		i++;
		if (i >=3210)
			break;
		
		if 	(rtl_inb(UART_LSR) & 0x60)
			break;	
	}
#endif

	//for(i=0; i<0xff00;i++);
 	rtl_outb(UART_THR, c);  
		  
	if (c == 0x0a)
		rtl_outb(UART_THR, 0x0d);  
#endif
	// ----------------------------------------------------
	// above is UART0, and below is SC16IS7x0 
	// ----------------------------------------------------

#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
	if( sc16is7x0_err )
		return;
	
	while (1)
	{
		i++;
		if (i >=3210)
			break;
		
		if (sc16is7x0_serial_in_i2c( UART_MEM2REG( UART_LSR ) ) & 0x60)
			break;	
	}
	
	sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_THR ), c );
	
	if (c == 0x0a)
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_THR ), 0x0d);
#endif
}


char serial_inc()
{
	int i;
#ifndef CONFIG_SERIAL_SC16IS7X0_CONSOLE
    while (1)
	{
		if 	(rtl_inb(UART_LSR) & 0x1)
			break;		
	}	
	i=rtl_inb(UART_RBR);
	return (i & 0xff);
#endif
	// ----------------------------------------------------
	// above is UART0, and below is SC16IS7x0 
	// ----------------------------------------------------
	
#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
	unsigned long flags;
	unsigned int in;
	
	if( sc16is7x0_err )
		while( 1 );		// never return
	
	while (1)
	{
		save_and_cli(flags);
		
		in = sc16is7x0_serial_in_i2c( UART_MEM2REG( UART_LSR ) );
		
		restore_flags(flags);
		
		if( in & 0x1)
			break;
		
		// add delay to be interrupted by eth-driver
		for( i = 0; i < 10; i ++ )		// 10ms delay 
			udelay( 1000 );
	}
	i = sc16is7x0_serial_in_i2c( UART_MEM2REG( UART_RBR ) );
	return (i & 0xff);
#endif
}

int isspace(char ch)
{
    return (unsigned int)(ch - 9) < 5u  ||  ch == ' ';	
}


/*
 * Helpful for debugging :-)
 */
 #if 0
int prom_printf(const char * fmt, ...)
{

	static char buf[1024];
	va_list args;
	char c;
	int i = 0;

	/*
	 * Printing messages via serial port
	 */
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	//do_printf(buf, fmt, args);
	va_end(args);

	for (i = 0; buf[i] != '\0'; i++) {
		c = buf[i];
		if (c == '\n')
			serial_outc('\r');
		serial_outc(c);
	}

	return i;
}
#endif
