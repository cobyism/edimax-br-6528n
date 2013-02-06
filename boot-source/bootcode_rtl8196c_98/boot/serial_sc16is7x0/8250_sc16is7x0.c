#ifndef __BOOTCODE__ 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serial_8250.h>
#include <platform.h>		// GPIO_ABCD_IRQ, PABCD_ISR
#endif

#ifdef __BOOTCODE__
#define ENODEV		19      /* No such device */
#define __init
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// I2C level function 

#ifndef __BOOTCODE__
#include "../../../rtk_voip/voip_drivers/i2c.h"
#else
#include "i2c.h"
#endif

#define I2C_GPIO_ID( port, pin )		( ( ( port - 'A' ) << 16 ) | ( pin ) )

#define SC16IS7X0_RESET 	I2C_GPIO_ID( 'D', 3)	// RESET = D3
#define SC16IS7X0_SCL 		I2C_GPIO_ID( 'D', 4)	// SCL = D4
#define SC16IS7X0_SDA 		I2C_GPIO_ID( 'D', 5)	// SDA = D5
#define SC16IS7X0_IRQ 		I2C_GPIO_ID( 'D', 6)	// IRQ = D6
#define SC16IS7X0_IRQ_NUM	GPIO_ABCD_IRQ			// GPIO_ABCD_IRQ=16
#define SC16IS7X0_IRQ_STATUS_BIT	( 1 << ( 24 + 6 ) )		// clean pending status of D6 
#define SC16IS7X0_IRQ_STATUS_REG	PABCD_ISR

#define SC16IS7X0_I2C_ADDR	0x9A
#define SC16IS7X0_I2C_WRITE	0x00
#define SC16IS7X0_I2C_READ	0x01

static i2c_dev_t sc16is7x0_i2c_dev = {
	.sclk	= SC16IS7X0_SCL,
	.sdio	= SC16IS7X0_SDA,
	.irq	= SC16IS7X0_IRQ,
	.reset	= SC16IS7X0_RESET,
};

// Register address byte
//  7  : not used
//  6~3: A[3:0] UART's internal register select
//  2~1: channel select: CH1 = 0?!, CH0 = 0
//  0  : not used 
#define MK_SC16IS7X0_REG_ADDR( uart_reg )	( ( uart_reg & 0x0F ) << 3 )

unsigned int serial_in_i2c(unsigned int addr, int offset)
{
	unsigned short int data;
	
	//printk( "serial_in_i2c(%X):%X\n", addr, offset );
	
	if( addr != SC16IS7X0_I2C_ADDR )
		return 0;
	
	// start 
	i2c_start_condition( &sc16is7x0_i2c_dev );
	
	// addr + write
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, SC16IS7X0_I2C_ADDR | 
											   SC16IS7X0_I2C_WRITE );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// write register address 
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, MK_SC16IS7X0_REG_ADDR( offset ) );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// start 
	i2c_start_condition( &sc16is7x0_i2c_dev );

	// addr + read
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, SC16IS7X0_I2C_ADDR | 
											   SC16IS7X0_I2C_READ );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
		
	// read data
	i2c_serial_read( &sc16is7x0_i2c_dev, &data );
	
	// write negative-ACK
	i2c_ACK_w( &sc16is7x0_i2c_dev, 1 );
	
	// stop
	i2c_stop_condition( &sc16is7x0_i2c_dev );
	
	//printk( "in[%X]\n", data );
	
	return data;
}

unsigned int serial_out_i2c(unsigned int addr, int offset, int value)
{
	//printk( "serial_out_i2c(%X):%X,%X\n", addr, offset, value );

	if( addr != SC16IS7X0_I2C_ADDR )
		return 0;
		
	// start 
	i2c_start_condition( &sc16is7x0_i2c_dev );
	
	// addr + write
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, SC16IS7X0_I2C_ADDR | 
											   SC16IS7X0_I2C_WRITE );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;

	// write register address 
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, MK_SC16IS7X0_REG_ADDR( offset ) );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// write data 
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, value );

	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// stop
	i2c_stop_condition( &sc16is7x0_i2c_dev );
	
	return 0;
}

static void __init sc16is7x0_init_i2c( void )
{
	// init SCL / SDA 
	i2c_init_SCL_SDA( &sc16is7x0_i2c_dev );
	
	serial_in_i2c( SC16IS7X0_I2C_ADDR, 1 );		// avoid NO ACK at first time access
	
	// Enable IRQ 
	i2c_enable_irq( &sc16is7x0_i2c_dev );
}

int __init early_sc16is7x0_init_i2c_and_check( void )
{
	unsigned int bak;
	
	// init i2c 
	sc16is7x0_init_i2c();
	
	// do simple test (write non-zero, because NO ACK return zero)
	bak = serial_in_i2c( SC16IS7X0_I2C_ADDR, 1 );
	
	serial_out_i2c( SC16IS7X0_I2C_ADDR, 1, 0x0F );
	if( serial_in_i2c( SC16IS7X0_I2C_ADDR, 1 ) != 0x0F )
		goto label_out;
	
	// restore 
	serial_out_i2c( SC16IS7X0_I2C_ADDR, 1, bak );
	
	return 0;

label_out:
	return -ENODEV;
}

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// Bootcode wrapper 
#ifdef __BOOTCODE__
unsigned int sc16is7x0_serial_out_i2c(int offset, int value)
{
	return serial_out_i2c( SC16IS7X0_I2C_ADDR, offset, value );
}

unsigned int sc16is7x0_serial_in_i2c(int offset)
{
	return serial_in_i2c( SC16IS7X0_I2C_ADDR, offset );
}
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// Platform level function 
#ifndef __BOOTCODE__
void sc16is7x0_clean_interrupt( int irq )
{
	if( irq != SC16IS7X0_IRQ_NUM )
		return;
	
	// clean my bit only 
	if( REG32( SC16IS7X0_IRQ_STATUS_REG ) & SC16IS7X0_IRQ_STATUS_BIT )
		REG32( SC16IS7X0_IRQ_STATUS_REG ) = SC16IS7X0_IRQ_STATUS_BIT;	
}
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// UART level function 
#ifndef __BOOTCODE__

#define SC16IS7X0_BAUDRATE	38400

#ifdef CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_1843200
#define XTAL1_CLK	( SC16IS7X0_BAUDRATE * 3 * 16 )		// 1.8432 MHZ
#elif defined( CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_14746500 )
#define XTAL1_CLK	( SC16IS7X0_BAUDRATE * 24 * 16 )	// 14.7465 MHZ (NXP demoboard)
#endif

#define PORT(_base,_irq)				\
	{						\
		.iobase		= _base,			\
		.irq		= _irq,			\
		.uartclk	= XTAL1_CLK,	\
		.iotype		= UPIO_I2C,		\
		.flags		= /*UPF_BOOT_AUTOCONF |*/ UPF_SKIP_TEST,	\
	}

static struct plat_serial8250_port sc16is7x0_data[] = {
	PORT(SC16IS7X0_I2C_ADDR, SC16IS7X0_IRQ_NUM),	
	{ },
};

static struct platform_device sc16is7x0_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_SC16IS7X0,
	.dev			= {
		.platform_data	= sc16is7x0_data,
	},
};

void __init sc16is7x0_get_port( struct uart_port *port )
{
	// call by static void __init serial_init(void)
	// in linux-2.6.x/arch/mips/realtek/rtl8196b/setup.c 
	memset(port, 0, sizeof(*port));
	
	port ->type = PORT_16654;
	port ->iobase = SC16IS7X0_I2C_ADDR;
	port ->irq = SC16IS7X0_IRQ_NUM;
	port ->uartclk = XTAL1_CLK;
	//port ->flags = UPF_SKIP_TEST | UPF_LOW_LATENCY | UPF_SPD_CUST;
	port ->flags = UPF_SKIP_TEST | UPF_SPD_CUST;
	//port ->flags = UPF_SKIP_TEST | UPF_SPD_CUST | UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP
	port ->iotype = UPIO_I2C;
	port ->fifosize = 1;
	port ->custom_divisor = XTAL1_CLK / (SC16IS7X0_BAUDRATE * 16);
}

static int __init sc16is7x0_init(void)
{
	int ret;
	
	sc16is7x0_init_i2c();
	
	ret = platform_device_register(&sc16is7x0_device);
	
	return ret;
}

module_init(sc16is7x0_init);

MODULE_AUTHOR("Realtek");
MODULE_DESCRIPTION("8250 serial probe module for SC16IS7x0");
MODULE_LICENSE("GPL");
#endif

