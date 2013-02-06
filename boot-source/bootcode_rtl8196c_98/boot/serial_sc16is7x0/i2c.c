#ifdef __UBOOT__
#include <config.h>
#endif
#ifndef __BOOTCODE__
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#endif
#ifndef __UBOOT__
#include <linux/delay.h>	// linux and realtek bootcode use 
#endif

#include "gpio/gpio.h"
#include "i2c.h"

#define __i2c_initGpioPin(pid, dir, interrupt)		RTK_GPIO_INIT(pid, GPIO_CONT_GPIO, dir, interrupt)
#define __i2c_getGpioDataBit(pid, pData)			RTK_GPIO_GET(pid, pData)
#define __i2c_setGpioDataBit(pid, data)				RTK_GPIO_SET(pid, data)

//-----------------------------
#if 0	//0: kernel used(udelay), 1: test program used(for(;;)) 
#define __test_program_used__
#else
#define __kernel_used__
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
//----------------------- WM8510 codec ----------------------------------------------
#define I2C_RATING_FACTOR	10	//Adjust I2C read and write rating.
//There is an I2C protocal.
#define _I2C_WM8510_ 	//GPIO pin
//cli() protection for kernel used. Don't define MACRO _WM8510_CLI_PROTECT in test program.
//#define _WM8510_CLI_PROTECT

#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
//----------------------- ALC5621 codec ----------------------------------------------
#define I2C_RATING_FACTOR	10	//Adjust I2C read and write rating.
//There is an I2C protocal.
#define _I2C_ALC5621_ 	//GPIO pin
//cli() protection for kernel used. Don't define MACRO _WM8510_CLI_PROTECT in test program.
  #if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99 )
    #define _ALC5621_CLI_PROTECT	// 8972 share ALC PIN to other 
  #endif
#elif defined(CONFIG_SERIAL_SC16IS7X0)
#define I2C_RATING_FACTOR	1
#endif

#ifdef __kernel_used__		// pkshih: If we use udelay, factor 1 should be work, doesn't it? 
#undef I2C_RATING_FACTOR
#define I2C_RATING_FACTOR	1
#endif

/************************* I2C read/write function ************************/
void i2c_serial_write_byte(i2c_dev_t* pI2C_Dev, unsigned char data)
{
#ifdef __test_program_used__
	int i;
#endif
	char j;
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	for (j=7;j>=0;j--) {
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		__i2c_setGpioDataBit( pI2C_Dev->sdio, ((data)>>j)&0x00000001);//write data,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
	}	
	return;
}

void i2c_serial_write(i2c_dev_t* pI2C_Dev, unsigned char *data)
{
	i2c_serial_write_byte( pI2C_Dev, *data );
}

unsigned char i2c_ACK(i2c_dev_t* pI2C_Dev)	
{
#ifdef __test_program_used__
	int i;
#endif
	unsigned int buf;
#if 0
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
#endif
	//__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
		
	__i2c_getGpioDataBit(pI2C_Dev->sdio,&buf);
	if (buf != 0) 
		printk("NO ACK\n");
	//__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	#if 0
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	#endif
	return buf;
}

void i2c_ACK_w(i2c_dev_t* pI2C_Dev, unsigned char data)	
{
#ifdef __test_program_used__
	int i;
#endif

#if 0
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, data); /* sdio send 0 for ACK, 1 for NACK*/
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
#endif
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//for (i=0;i<500*I2C_RATING_FACTOR;i++);
	
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, data); /* sdio send 0 for ACK, 1 for NACK*/

	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif

	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	#if 0
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	#endif
	return;
}

void i2c_serial_read(i2c_dev_t* pI2C_Dev, unsigned short int *data)
{
#ifdef __test_program_used__
	int i;
#endif
	char j;
	unsigned int buf;
	
	*data = 0;
	
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	for (j=7;j>=0;j--) {
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		__i2c_getGpioDataBit( pI2C_Dev->sdio, &buf);//read data,from MSB to LSB
		*data |= (buf<<j); 
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
	}
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output	
	return;
}	

void i2c_start_condition(i2c_dev_t* pI2C_Dev)
{
#ifdef __test_program_used__
	int i;
#endif

	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
}

void i2c_stop_condition(i2c_dev_t* pI2C_Dev)
{
#ifdef __test_program_used__
	int i;
#endif

	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/
}

void i2c_init_SCL_SDA(i2c_dev_t* pI2C_Dev)
{	
	__i2c_initGpioPin(pI2C_Dev->sclk, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1 );
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1 );
}

void i2c_enable_irq(i2c_dev_t* pI2C_Dev)
{
	__i2c_initGpioPin(pI2C_Dev->irq, GPIO_DIR_IN, GPIO_INT_BOTH_EDGE);
}

void i2c_reset(i2c_dev_t* pI2C_Dev)
{
	__i2c_initGpioPin(pI2C_Dev->reset, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__i2c_setGpioDataBit( pI2C_Dev->reset, 1);
	__i2c_setGpioDataBit( pI2C_Dev->reset, 0);
	udelay( 3 );	// delay 3 us
	__i2c_setGpioDataBit( pI2C_Dev->reset, 1);
	udelay( 3 );	// delay 3us
}

