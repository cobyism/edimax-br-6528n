#ifndef __BASE_I2C_CORE_H__
#define __BASE_I2C_CORE_H__


/*********************** I2C data struct ********************************************/
typedef struct i2c_dev_s
{
	//unsigned int i2c_reset;		//output
	unsigned int sclk;		//output
	unsigned int sdio;		//input or output	
	unsigned int irq;		// interrupt (optional)
	unsigned int reset;		// reset (optional)
} i2c_dev_t;	

extern i2c_dev_t i2c_dev;

/*********************** I2C API for base_ic2_xxx ********************************************/

// read / write 
extern void i2c_serial_write(i2c_dev_t* pI2C_Dev, unsigned char *data);
extern void i2c_serial_write_byte(i2c_dev_t* pI2C_Dev, unsigned char data);

extern unsigned char i2c_ACK(i2c_dev_t* pI2C_Dev);

extern void i2c_ACK_w(i2c_dev_t* pI2C_Dev, unsigned char data);

extern void i2c_serial_read(i2c_dev_t* pI2C_Dev, unsigned short int *data);

// start / stop condition 
extern void i2c_start_condition(i2c_dev_t* pI2C_Dev);

extern void i2c_stop_condition(i2c_dev_t* pI2C_Dev);

// GPIO pins manipulation 
extern void i2c_init_SCL_SDA(i2c_dev_t* pI2C_Dev);

extern void i2c_enable_irq(i2c_dev_t* pI2C_Dev);

extern void i2c_reset(i2c_dev_t* pI2C_Dev);

#endif /* __BASE_I2C_CORE_H__ */

