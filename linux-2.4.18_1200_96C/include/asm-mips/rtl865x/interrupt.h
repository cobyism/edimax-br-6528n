#include <linux/autoconf.h>

#ifdef CONFIG_RTL865XC

	#if 1 /*NEW_SPEC*/

#define CPU_CLOCK_RATE	16000000	/* CPU clock @ 14 MHz */
#define ICU_GPIO	       0
#define ICU_TMR		9
#define ICU_UART0	5
#define ICU_UART1 	4
//#define ICU_PCMCIA 	0
#define ICU_PCI		3
#define ICU_NIC		2
#define ICU_PCM		6
#define ICU_USB		7//	
//#define ICU_EXT		0//
//#define ICU_LBCT		0//

	#else

#define CPU_CLOCK_RATE	96000000	/* CPU clock @ 14 MHz */
#define ICU_GPIO	       0
#define ICU_TMR		2
#define ICU_UART0	4
#define ICU_UART1 	5
#define ICU_PCMCIA 	6
#define ICU_PCI		7
#define ICU_NIC		8
#define ICU_USB		1
#define ICU_EXT		8
#define ICU_LBCT		10

	#endif

#else

#define CPU_CLOCK_RATE	96000000	/* CPU clock @ 14 MHz */
#define ICU_TMR		0
#define ICU_USB		1	
#define ICU_PCMCIA 	2
#define ICU_UART0	4
#define ICU_UART1 	3	
#define ICU_PCI		5
#define ICU_NIC		6
#define ICU_GPIO	7
#define ICU_EXT		8
#define ICU_LBCT	10
#define ICU_CRYPTO	11
#define ICU_AUTH	12
#define ICU_PCM		13
#define ICU_PDE		14
#define ICU_PCIBTO	15

#endif
