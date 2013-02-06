/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
* 
* Program : GPIO Driver
* Abstract :
* Author :
*/

#ifndef __BOOTCODE__
#include <linux/kernel.h>
#endif
#include "gpio.h"

/*==================== FOR RTL8972B Family ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY

enum GPIO_FUNC
{
	GPIO_FUNC_DEDICATE,

	GPIO_FUNC_DIRECTION,
	GPIO_FUNC_DATA,
	GPIO_FUNC_INTERRUPT_STATUS,
	GPIO_FUNC_INTERRUPT_ENABLE,
	GPIO_FUNC_MAX,
};

//******************************************* GPIO control
static uint32 regGpioControl[] =
{
	GPABCDCNR, /* Port A */
	GPABCDCNR, /* Port B */
	GPABCDCNR, /* Port C */
	GPABCDCNR, /* Port D */
	GPEFGHCNR, /* Port E */
	GPEFGHCNR, /* Port F */
	GPEFGHCNR, /* Port G */
	GPEFGHCNR, /* Port H */
};

static uint32 bitStartGpioControl[] =
{
	0,  /* Port A */
	8,  /* Port B */
	16, /* Port C */
	24, /* Port D */
	0,  /* Port E */
	8,  /* Port F */
	16, /* Port G */
	24, /* Port H */
};

//******************************************* GPIO Share pin config
#if 0
static uint32 GpioShare_data[] =
{
	0xFF1F0FFF,
	0x0000007f,
};
#endif
#define SHAREPIN_REGISTER 0xB8000030

#define SET_BIT(x) (1<<x)
static uint32 GpioShare_setting[] =
{
	(SET_BIT(9)|SET_BIT(8)|SET_BIT(7)),	/* Port A0 */
	(SET_BIT(9)|SET_BIT(8)|SET_BIT(7)),	/* Port A1 */
	(SET_BIT(9)|SET_BIT(8)|SET_BIT(7)),	/* Port A2 */
	(SET_BIT(9)|SET_BIT(8)|SET_BIT(7)),	/* Port A3 */
	(SET_BIT(9)|SET_BIT(8)|SET_BIT(7)),	/* Port A4 */
	(SET_BIT(6)|SET_BIT(5)),	/* Port A5 */
	(SET_BIT(6)|SET_BIT(5)),	/* Port A6 */
	(SET_BIT(3)|SET_BIT(2)),	/* Port A7 */
	(SET_BIT(11)|SET_BIT(10)),	/* Port B0 */
	(SET_BIT(13)|SET_BIT(12)),	/* Port B1 */
	(SET_BIT(15)|SET_BIT(14)),	/* Port B2 */
	(SET_BIT(17)|SET_BIT(16)),	/* Port B3 */
	0x00000000,	/* Port B4 */
	0x00000000,	/* Port B5 */
	0x00000000,	/* Port B6 */
	0x00000000,	/* Port B7 */
	(SET_BIT(19)|SET_BIT(18)),	/* Port C0 */
	(SET_BIT(21)|SET_BIT(20)),	/* Port C1 */
	(SET_BIT(23)|SET_BIT(22)),	/* Port C2 */
	(SET_BIT(25)|SET_BIT(24)),	/* Port C3 */
	(SET_BIT(27)|SET_BIT(26)),	/* Port C4 */
	0x00000000,	/* Port C5 */
	0x00000000,	/* Port C6 */
	0x00000000,	/* Port C7 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D0 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D1 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D2 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D3 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D4 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D5 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D6 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port D7 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E0 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E1 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E2 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E3 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E4 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E5 */
	(SET_BIT(1)|SET_BIT(0)),	/* Port E6 */
	0x00000000,	/* Port E7 */
	0x00000000,	/* Port F0 */
	0x00000000,	/* Port F1 */
	0x00000000,	/* Port F2 */
	0x00000000,	/* Port F3 */
	0x00000000,	/* Port F4 */
	0x00000000,	/* Port F5 */
	0x00000000,	/* Port F6 */
	0x00000000,	/* Port F7 */
	0x00000000,	/* Port G0 */
	0x00000000,	/* Port G1 */
	0x00000000,	/* Port G2 */
	0x00000000,	/* Port G3 */
	0x00000000,	/* Port G4 */
	0x00000000,	/* Port G5 */
	0x00000000,	/* Port G6 */
	0x00000000,	/* Port G7 */
	0x00000000,	/* Port H0 */
	0x00000000,	/* Port H1 */
	0x00000000,	/* Port H2 */
	0x00000000,	/* Port H3 */
	0x00000000,	/* Port H4 */
	0x00000000,	/* Port H5 */
	0x00000000,	/* Port H6 */
	0x00000000,	/* Port H7 */
};
#undef SET_BIT

//******************************************* Direction
static uint32 regGpioDirection[] =
{
	GPABCDDIR, /* Port A */
	GPABCDDIR, /* Port B */
	GPABCDDIR, /* Port C */
	GPABCDDIR, /* Port D */
	GPEFGHDIR, /* Port E */
	GPEFGHDIR, /* Port F */
	GPEFGHDIR, /* Port G */
	GPEFGHDIR, /* Port H */
};

static uint32 bitStartGpioDirection[] =
{
	0,  /* Port A */
	8,  /* Port B */
	16, /* Port C */
	24, /* Port D */
	0,  /* Port E */
	8,  /* Port F */
	16, /* Port G */
	24, /* Port H */
};

//******************************************* Data
static uint32 regGpioData[] =
{
	GPABCDDATA, /* Port A */
	GPABCDDATA, /* Port B */
	GPABCDDATA, /* Port C */
	GPABCDDATA, /* Port D */
	GPEFGHDATA, /* Port E */
	GPEFGHDATA, /* Port F */
	GPEFGHDATA, /* Port G */
	GPEFGHDATA, /* Port H */
	GPEFGHDATA, /* Port I */
};

static uint32 bitStartGpioData[] =
{
	0,  /* Port A */
	8,  /* Port B */
	16, /* Port C */
	24, /* Port D */
	0,  /* Port E */
	8,  /* Port F */
	16, /* Port G */
	24, /* Port H */
};

//******************************************* ISR
static uint32 regGpioInterruptStatus[] =
{
	GPABCDISR, /* Port A */
	GPABCDISR, /* Port B */
	GPABCDISR, /* Port C */
	GPABCDISR, /* Port D */
	GPEFGHISR, /* Port E */
	GPEFGHISR, /* Port F */
	GPEFGHISR, /* Port G */
	GPEFGHISR, /* Port H */
};

static uint32 bitStartGpioInterruptStatus[] =
{
	0,  /* Port A */
	8,  /* Port B */
	16, /* Port C */
	24, /* Port D */
	0,  /* Port E */
	8,  /* Port F */
	16, /* Port G */
	24, /* Port H */
};

//******************************************* IMR
static uint32 regGpioInterruptEnable[] =
{
	GPABIMR, /* Port A */
	GPABIMR, /* Port B */
	GPCDIMR, /* Port C */
	GPCDIMR, /* Port D */
	GPEFIMR, /* Port E */
	GPEFIMR, /* Port F */
	GPGHIMR, /* Port G */
	GPGHIMR, /* Port H */
};

static uint32 bitStartGpioInterruptEnable[] =
{
	0,  /* Port A */
	16, /* Port B */
	0,  /* Port C */
	16, /* Port D */
	0,  /* Port E */
	16, /* Port F */
	0,  /* Port G */
	16, /* Port H */
};

int gpio_debug = 0;

/*
@func int32 | _getGpio | abstract GPIO registers
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@rvalue uint32 | value
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
{
	
	GPIO_PRINT(4, "[%s():%d] func=%d port=%d pin=%d\n", __FUNCTION__, __LINE__, func, port, pin );
	switch( func )
	{
		case GPIO_FUNC_DEDICATE:
			GPIO_PRINT(5, "[%s():%d] regGpioControl[port]=0x%08x  bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__, regGpioControl[port], bitStartGpioControl[port] );

			if ( REG32(regGpioControl[port]) & ( (uint32)1 << (pin+bitStartGpioControl[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DIRECTION:
			GPIO_PRINT(5, "[%s():%d] regGpioDirection[port]=0x%08x  bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirection[port], bitStartGpioDirection[port] );

			if ( REG32(regGpioDirection[port]) & ( (uint32)1 << (pin+bitStartGpioDirection[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DATA:
			GPIO_PRINT(5, "[%s():%d] regGpioData[port]=0x%08x  bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__, regGpioData[port], bitStartGpioData[port] );

			if ( REG32(regGpioData[port]) & ( (uint32)1 << (pin+bitStartGpioData[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
			GPIO_PRINT(5, "[%s():%d] regGpioInterruptEnable[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );

			return ( REG32(regGpioInterruptEnable[port]) >> (pin*2+bitStartGpioInterruptEnable[port]) ) & (uint32)0x3;
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
			GPIO_PRINT(5, "[%s():%d] regGpioInterruptStatus[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );

			if ( REG32(regGpioInterruptStatus[port]) & ( (uint32)1 << (pin+bitStartGpioInterruptStatus[port]) ) )
				return 1;
			else
				return 0;
			break;

		case GPIO_FUNC_MAX:
			printk("Wrong GPIO function\n");
			break;
	}
	return 0xffffffff;
}


/*
@func int32 | _setGpio | abstract GPIO registers
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@parm uint32 | data | value
@rvalue NONE
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
{
	
	GPIO_PRINT(4, "[%s():%d] func=%d port=%d pin=%d data=%d\n", __FUNCTION__, __LINE__, func, port, pin, data );
	switch( func )
	{
		case GPIO_FUNC_DEDICATE:
			GPIO_PRINT(5, "[%s():%d] regGpioControl[port]=0x%08x  bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__, regGpioControl[port], bitStartGpioControl[port] );

			if ( data )
				REG32(regGpioControl[port]) |= (uint32)1 << (pin+bitStartGpioControl[port]);
			else {
				REG32(regGpioControl[port]) &= ~((uint32)1 << (pin+bitStartGpioControl[port]));
				//printk("oldsharepin_register=%x", REG32(SHAREPIN_REGISTER));
				REG32(SHAREPIN_REGISTER) |= GpioShare_setting[(port<<3)|pin];
				//printk("newsharepin_register=%x", REG32(SHAREPIN_REGISTER));
				//GPIO_PRINT(5,"GPIOSET(%x)",(port<<3)|pin);
				//printk("[%d,%d]GPIOSET(%x)", port, pin,(port<<3)|pin);
			}
			break;
			
		case GPIO_FUNC_DIRECTION:
			GPIO_PRINT(5, "[%s():%d] regGpioDirection[port]=0x%08x  bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirection[port], bitStartGpioDirection[port] );

			if ( data )
				REG32(regGpioDirection[port]) |= (uint32)1 << (pin+bitStartGpioDirection[port]);
			else
				REG32(regGpioDirection[port]) &= ~((uint32)1 << (pin+bitStartGpioDirection[port]));
			break;

		case GPIO_FUNC_DATA:
			GPIO_PRINT(5, "[%s():%d] regGpioData[port]=0x%08x  bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__, regGpioData[port], bitStartGpioData[port] );

			if ( data )
				REG32(regGpioData[port]) |= (uint32)1 << (pin+bitStartGpioData[port]);
			else
				REG32(regGpioData[port]) &= ~((uint32)1 << (pin+bitStartGpioData[port]));
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
			GPIO_PRINT(5, "[%s():%d] regGpioInterruptEnable[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );

			REG32(regGpioInterruptEnable[port]) &= ~((uint32)0x3 << (pin*2+bitStartGpioInterruptEnable[port]));
			REG32(regGpioInterruptEnable[port]) |= (uint32)data << (pin*2+bitStartGpioInterruptEnable[port]);
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
			GPIO_PRINT(5, "[%s():%d] regGpioInterruptStatus[port]=0x%08x  bitStartGpioInterruptStatus[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );

			if ( data )
				REG32(regGpioInterruptStatus[port]) |= (uint32)1 << (pin+bitStartGpioInterruptStatus[port]);
			else
				REG32(regGpioInterruptStatus[port]) &= ~((uint32)1 << (pin+bitStartGpioInterruptStatus[port]));
			break;

		case GPIO_FUNC_MAX:
			printk("Wrong GPIO function\n");
			break;
	}
}

/*
@func int32 | _rtl8972B_initGpioPin | Initiate a specifed GPIO port.
@parm uint32 | gpioId | The GPIO port that will be configured
@parm enum GPIO_PERIPHERAL | dedicate | Dedicated peripheral type
@parm enum GPIO_DIRECTION | direction | Data direction, in or out
@parm enum GPIO_INTERRUPT_TYPE | interruptEnable | Interrupt mode
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
This function is used to initialize GPIO port.
*/
int32 _rtl8972B_initGpioPin( uint32 gpioId, enum GPIO_CONTROL dedicate,
                                           enum GPIO_DIRECTION direction,
                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;

	switch ( dedicate )
	{
		case GPIO_CONT_GPIO:
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 0 );
			break;
		case GPIO_CONT_PERI:
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 1 );
			break;
	}

	_setGpio( GPIO_FUNC_DIRECTION, port, pin, direction );

	_setGpio( GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable );

	return SUCCESS;
}

/*
@func int32 | _rtl8972B_getGpioDataBit | Get the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32* | data | Pointer to store return value
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl8972B_getGpioDataBit( uint32 gpioId, uint32* pData )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;
	if ( pData == NULL ) return FAILED;

	*pData = _getGpio( GPIO_FUNC_DATA, port, pin );
	GPIO_PRINT(3, "[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, *pData );
	return SUCCESS;
}

/*
@func int32 | _rtl8972B_setGpioDataBit | Set the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32 | data | Data to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl8972B_setGpioDataBit( uint32 gpioId, uint32 data )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;

	GPIO_PRINT(3, "[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, data );
	_setGpio( GPIO_FUNC_DATA, port, pin, data );
	return SUCCESS;
}

#endif //CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY

