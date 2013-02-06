/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
* 
* Program : GPIO Header File
* Abstract :
* Author :
* 
*/

 
#ifndef __GPIO_8972B_H
#define __GPIO_8972B_H

////// For 8982B QA Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8982B_QA

#define GPIO "E"
/* Slic used */
#define PIN_RESET1	GPIO_ID(GPIO_PORT_E,0)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_E,1)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_E,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_E,2) 	//input
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)
#define PIN_DO		GPIO_ID(GPIO_PORT_E,4)	//output  for Si3215 daugher board
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
#define PIN_DO		GPIO_ID(GPIO_PORT_E,4)	//output  for Le88221 daughter board
#endif

/* DAA used (Reserved)*/ 
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,1)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,2)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,3) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,4)	//output

#endif

////// For 8972B EV Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8972B
//#define GPIO_OC

#define GPIO "E"
/* SLIC used */
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116	// for RTL89116 SLIC card
#define PIN_RESET1		GPIO_ID(GPIO_PORT_D,7)	//output
#define PIN_CS1			GPIO_ID(GPIO_PORT_D,4)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_D,3)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_D,5) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_D,6) 	//output
#else	// for Silab SLIC
#ifdef GPIO_OC 
#define PIN_RESET1		GPIO_ID(GPIO_PORT_E,6)	//output
#define PIN_CS1			GPIO_ID(GPIO_PORT_E,0)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_E,5)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_D,0) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_E,3) 	//output
#else
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
//#define PIN_RESET1		GPIO_ID(GPIO_PORT_E,0)	//output
#define PIN_CS1			GPIO_ID(GPIO_PORT_D,3)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_D,4)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_D,5) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_D,6) 	//output
#define PIN_INT			GPIO_ID(GPIO_PORT_D,7) 	//intput
#else
#define PIN_RESET1		GPIO_ID(GPIO_PORT_E,0)	//output
#define PIN_CS1			GPIO_ID(GPIO_PORT_E,1)	//output
#ifdef CONFIG_RTK_VOIP_MULTIPLE_SI32178
#define PIN_CS2			GPIO_ID(GPIO_PORT_D,5)	//output
#define PIN_CS3			GPIO_ID(GPIO_PORT_F,2)	//output
#define PIN_CS4			GPIO_ID(GPIO_PORT_D,0)	//output
#endif
#define PIN_CLK			GPIO_ID(GPIO_PORT_E,3)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_E,4) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_E,2) 	//output
#endif	//!GPIO_DECT
#endif	//!GPIO_OC
#endif	//!CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116
//#define PIN_INT1		GPIO_ID(GPIO_PORT_E,2)  //input

/* DAA used */ 
#define PIN_RESET3_DAA	GPIO_ID(GPIO_PORT_E,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_G,0)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_G,1)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_G,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_G,6)	//output

/* LED */ 
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_D,1)  //output
#define PIN_VOIP2_LED	GPIO_ID(GPIO_PORT_D,2)	//output
#define PIN_VOIP3_LED	GPIO_ID(GPIO_PORT_D,3)  //output
#define PIN_VOIP4_LED	GPIO_ID(GPIO_PORT_D,4)	//output
#define PIN_PSTN_LED	GPIO_ID(GPIO_PORT_F,5)	//output

#endif

////// For IP Phone /////
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
/* To be modified!! */
#define GPIO "A"
/* SLIC Si3226 */
#define PIN_RESET1		GPIO_ID(GPIO_PORT_H,7)	//output
#define PIN_INT1		GPIO_ID(GPIO_PORT_E,2)  //input
#define PIN_CS1			GPIO_ID(GPIO_PORT_H,6)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_E,1)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_G,3) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_G,2) 	//output

/* DAA used */ 
#define PIN_RESET3_DAA	GPIO_ID(GPIO_PORT_E,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_G,0)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_G,1)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_G,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_G,6)	//output

/* LED */ 
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_F,4)  //output
#define PIN_VOIP2_LED	GPIO_ID(GPIO_PORT_F,2)	//output
#define PIN_PSTN_LED	GPIO_ID(GPIO_PORT_F,5)	//output
#endif 

/*==================== FOR RTL8972B Family ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY

/******** GPIO define ********/

/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_MAX,
};

/* define GPIO control pin */
enum GPIO_CONTROL
{
	GPIO_CONT_GPIO = 0,
	GPIO_CONT_PERI = 0x1,
};

/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT =1,
};

/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_FALLING_EDGE,
	GPIO_INT_RISING_EDGE,
	GPIO_INT_BOTH_EDGE,
};

/*************** Define RTL8972B Family GPIO Register Set ************************/
#define GPABCDCNR		0xB8003500

#define	GPABCDDIR		0xB8003508
#define	GPABCDDATA		0xB800350C
#define	GPABCDISR		0xB8003510
#define	GPABIMR			0xB8003514
#define	GPCDIMR			0xB8003518
#define GPEFGHCNR		0xB800351C

#define	GPEFGHDIR		0xB8003524
#define	GPEFGHDATA		0xB8003528
#define	GPEFGHISR		0xB800352C
#define	GPEFIMR			0xB8003530
#define	GPGHIMR			0xB8003534
/**************************************************************************/
/* Register access macro (REG*()).*/
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 			(*((volatile uint8 *)(reg)))

/*********************  Function Prototype in gpio.c  ***********************/
int32 _rtl8972B_initGpioPin(uint32 gpioId, enum GPIO_CONTROL dedicate, 
                                           enum GPIO_DIRECTION, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl8972B_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl8972B_setGpioDataBit( uint32 gpioId, uint32 data );

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY

#endif /* __GPIO_8972B_H */
