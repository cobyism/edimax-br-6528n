/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * rtl8196b/bsp/bspchip.h:
 *   RTL8196B chip-level header file
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 07, 2006
 */

#ifndef _BSPCHIP_H_
#define _BSPCHIP_H_

#include <linux/autoconf.h>
#define RTL8196C 1
/*
 * Register access macro
 */
#define REG32(reg)	(*(volatile unsigned int   *)((unsigned int)reg))
#define REG16(reg)	(*(volatile unsigned short *)((unsigned int)reg))
#define REG08(reg)	(*(volatile unsigned char  *)((unsigned int)reg))
#define REG8(reg)   (*(volatile unsigned char  *)((unsigned int)reg))

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)

/*
 * IRQ Controller
 */
#define BSP_IRQ_CPU_BASE   0
#define BSP_IRQ_CPU_NUM    8

#define BSP_IRQ_LOPI_BASE   (BSP_IRQ_CPU_BASE + BSP_IRQ_CPU_NUM)
#define BSP_IRQ_LOPI_NUM   8

#define BSP_IRQ_ICTL_BASE  (BSP_IRQ_LOPI_BASE + BSP_IRQ_LOPI_NUM)
#define BSP_IRQ_ICTL_NUM   32

/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

#ifdef CONFIG_FPGA_PLATFORM
//#define BSP_SYS_CLK_RATE	  	(33860000)      //33.86MHz
#define BSP_SYS_CLK_RATE	  	(27000000)      //27MHz
#else
#define BSP_SYS_CLK_RATE	  	(200000000)     //HS1 clock : 200 MHz
#endif

#define BSP_BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

/*
 *   RTL8196b Interrupt Scheme (Subject to change)
 *
 *   Source     EXT_INT  CPU INT    LOPI    IRQ      
 *   --------   -------  -------  -------  ------   
 *   PCIB0TO    0           2        -      16+0     
 *   PCIB1TO    1           2        -      16+1     
 *   LBCTMOm0   2           2        -      16+2     
 *   LBCTMOm1   3           2        -      16+3     
 *   LBCTMOs    4           2        -      16+4     
 *   TIMER0     8           -        5       8+5 
 *   TIMER1     9           2        -      16+9     
 *   USB        10          -        1       8+1
 *   UART0      12          2        -      16+12
 *   UART1      13          2        -      16+13    
 *   VoIP       14          2        -      16+14    
 *   SWCORE     15          -        4       8+4        
 *   GPIO_ABCD  16          2        -      16+16    
 *   GPIO_EFGH  17          2        -      16+17    
 *   NFBI       18          2        -      16+18    
 *   PCM        19          2        -      16+19    
 *   CRYPTO     20          2        -      16+20
 *   PCIE0      21          -        2       8+2
 *   PCIE1      22          -        3       8+3
 */                               
/*                        
 * IRQ Mapping
 */
#define BSP_ICTL_IRQ            (BSP_IRQ_CPU_BASE + 2) //for irq cascade

#if defined(RTL8196C) || defined(CONFIG_RTL_8196C)
#define BSP_ICTL_IRQ			(BSP_IRQ_CPU_BASE + 2) //for irq cascade
#define BSP_TC0_IRQ             (13)	/* BSP_IRQ_LOPI_BASE + 5	*/
#define BSP_TC1_IRQ             (BSP_IRQ_ICTL_BASE + 15)
#define BSP_USB_IRQ             (10)	/* BSP_IRQ_LOPI_BASE + 2	*/
//#define BSP_UART0_IRQ           (BSP_IRQ_ICTL_BASE + 0)
#define BSP_UART0_IRQ           (BSP_IRQ_ICTL_BASE + 7)	/* BSP_IRQ_LOPI_BASE + 0	*/
#define BSP_SWCORE_IRQ          (12)	/* BSP_IRQ_LOPI_BASE + 4	*/
#define BSP_GPIO_ABCD_IRQ	(BSP_IRQ_ICTL_BASE + 9)
#define BSP_PCIE_IRQ            (11)	/* BSP_IRQ_LOPI_BASE + 3	*/
#define BSP_NFBI_IRQ            (15)	/* BSP_IRQ_LOPI_BASE + 7	*/
#else
#define BSP_PCIB0TMO_IRQ        (BSP_IRQ_ICTL_BASE + 0)
#define BSP_PCIB1TMO_IRQ        (BSP_IRQ_ICTL_BASE + 1)
#define BSP_LBCTMOm0_IRQ        (BSP_IRQ_ICTL_BASE + 2)
#define BSP_LBCTMOm1_IRQ        (BSP_IRQ_ICTL_BASE + 3)
#define BSP_LBCTMOs_IRQ         (BSP_IRQ_ICTL_BASE + 4)
#define BSP_TC0_IRQ             (5)
#define BSP_TC1_IRQ             (BSP_IRQ_ICTL_BASE + 9)
#define BSP_USB_IRQ             (BSP_IRQ_LOPI_BASE + 1)
//#define BSP_UART0_IRQ           (BSP_IRQ_LOPI_BASE + 0)
#define BSP_UART0_IRQ           (BSP_IRQ_ICTL_BASE + 12)
#define BSP_UART1_IRQ           (BSP_IRQ_ICTL_BASE + 13)
#define BSP_SWCORE_IRQ          (4)
#define BSP_GPIO_ABCD_IRQ       (BSP_IRQ_LOPI_BASE + 6)
#define BSP_GPIO_EFGH_IRQ       (BSP_IRQ_LOPI_BASE + 7)
#define BSP_NFBI_IRQ            (BSP_IRQ_ICTL_BASE + 18)
#define BSP_PCM_IRQ             (BSP_IRQ_ICTL_BASE + 19)
#define BSP_CRYPTO_IRQ          (BSP_IRQ_ICTL_BASE + 20)
#define BSP_GDMA_IRQ            (BSP_IRQ_ICTL_BASE + 23)
#define BSP_PCIE_IRQ            (3)  // shall be 22
#endif
/*
 * Interrupt Routing Selection
 */
#define BSP_IRQ_CASCADE     2
#define BSP_PTM_RS          BSP_IRQ_CASCADE
#define BSP_LBCTMOs2_RS     BSP_IRQ_CASCADE
#define BSP_LBCTMOs1_RS     BSP_IRQ_CASCADE
#define BSP_PKT_RS          BSP_IRQ_CASCADE
#define BSP_SPI_RS          BSP_IRQ_CASCADE
#define BSP_NIC100_RS       BSP_IRQ_CASCADE
#define BSP_SAR_RS          BSP_IRQ_CASCADE
#define BSP_DMT_RS          BSP_IRQ_CASCADE
#define BSP_PCIE_RS		BSP_PCIE_IRQ
#define BSP_PCIE2_RS	BSP_PCIE2_IRQ
#define BSP_GDMA_RS         BSP_IRQ_CASCADE
#define BSP_SECURITY_RS     BSP_IRQ_CASCADE
#define BSP_PCM_RS          BSP_IRQ_CASCADE
#define BSP_NFBI_RS         BSP_NFBI_IRQ
#define BSP_GPIO_EFGH_RS    BSP_IRQ_CASCADE
#define BSP_GPIO_ABCD_RS    BSP_IRQ_CASCADE
#define BSP_SW_RS           BSP_SWCORE_IRQ
//#define BSP_UART0_RS      BSP_UART0_IRQ
#define BSP_UART0_RS        BSP_IRQ_CASCADE
#define BSP_UART1_RS        BSP_IRQ_CASCADE
#define BSP_USB_D_RS        BSP_IRQ_CASCADE
#define BSP_USB_H_RS        BSP_USB_IRQ
#define BSP_TC0_RS			BSP_TC0_IRQ//13
#define BSP_TC1_RS          		BSP_IRQ_CASCADE
#define BSP_LBCTMOm2_RS     BSP_IRQ_CASCADE
#define BSP_LBCTMOm1_RS     BSP_IRQ_CASCADE
#define BSP_SPEED_RS        BSP_IRQ_CASCADE
#define BSP_LBCTMOs0_RS     BSP_IRQ_CASCADE
#define BSP_LBCTMOm0_RS     BSP_IRQ_CASCADE
#define BSP_OCPTMO_RS       BSP_IRQ_CASCADE
#define BSP_PCIB0TO_RS      BSP_IRQ_CASCADE
#define BSP_NONE            BSP_IRQ_CASCADE
#define BSP_PCIB0TMO_RS     BSP_IRQ_CASCADE
#define BSP_PCIB1TMO_RS     BSP_IRQ_CASCADE
#define BSP_LBCTMOs_RS      BSP_IRQ_CASCADE
#define BSP_CRYPTO_RS       BSP_IRQ_CASCADE


#define BSP_DIVISOR         1000

#if BSP_DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 *  ==========================
 *  Platform Register Settings
 *  ==========================
 */

/*
 * CPU
 */
#define BSP_IMEM_BASE       0x00C00000
#define BSP_IMEM_TOP        0x00C03FFF

#define BSP_DMEM_BASE       0x00C04000
#define BSP_DMEM_TOP        0x00C05FFF

/*
 * Memory Controller
 */
#define BSP_MC_MCR          0xB8001000
   #define BSP_MC_MCR_VAL      0x92A28000

#define BSP_MC_MTCR0        0xB8001004
   #define BSP_MC_MTCR0_VAL    0x12120000

#define BSP_MC_MTCR1        0xB8001008
   #define BSP_MC_MTCR1_VAL    0x00000FEB

#define BSP_MC_PFCR         0xB8001010
   #define BSP_MC_PFCR_VAL     0x00000101


#define BSP_MC_BASE         0xB8001000
#define BSP_NCR             (BSP_MC_BASE + 0x100)
#define BSP_NSR             (BSP_MC_BASE + 0x104)
#define BSP_NCAR            (BSP_MC_BASE + 0x108)
#define BSP_NADDR           (BSP_MC_BASE + 0x10C)
#define BSP_NDR             (BSP_MC_BASE + 0x110)

#define BSP_SFCR            (BSP_MC_BASE + 0x200)
#define BSP_SFDR            (BSP_MC_BASE + 0x204)

/*
 * UART
 */
#define BSP_UART0_BASE      0xB8002000
#define BSP_UART0_MAP_BASE  0x18002000
#define BSP_UART0_RBR       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_THR       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_DLL       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_IER       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_DLM       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_IIR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_FCR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_LCR       (BSP_UART0_BASE + 0x00C)
#define BSP_UART0_MCR       (BSP_UART0_BASE + 0x010)
#define BSP_UART0_LSR       (BSP_UART0_BASE + 0x014)

#define BSP_UART1_BASE      0xB8002100
#define BSP_UART1_RBR       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_THR       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_DLL       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_IER       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_DLM       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_IIR       (BSP_UART1_BASE + 0x008)
#define BSP_UART1_FCR       (BSP_UART1_BASE + 0x008)
   #define BSP_FCR_EN          0x01
   #define BSP_FCR_RXRST       0x02
   #define     BSP_RXRST             0x02
   #define BSP_FCR_TXRST       0x04
   #define     BSP_TXRST             0x04
   #define BSP_FCR_DMA         0x08
   #define BSP_FCR_RTRG        0xC0
   #define     BSP_CHAR_TRIGGER_01   0x00
   #define     BSP_CHAR_TRIGGER_04   0x40
   #define     BSP_CHAR_TRIGGER_08   0x80
   #define     BSP_CHAR_TRIGGER_14   0xC0
#define BSP_UART1_LCR       (BSP_UART1_BASE + 0x00C)
   #define BSP_LCR_WLN         0x03
   #define     BSP_CHAR_LEN_5        0x00
   #define     BSP_CHAR_LEN_6        0x01
   #define     BSP_CHAR_LEN_7        0x02
   #define     BSP_CHAR_LEN_8        0x03
   #define BSP_LCR_STB         0x04
   #define     BSP_ONE_STOP          0x00
   #define     BSP_TWO_STOP          0x04
   #define BSP_LCR_PEN         0x08
   #define     BSP_PARITY_ENABLE     0x01
   #define     BSP_PARITY_DISABLE    0x00
   #define BSP_LCR_EPS         0x30
   #define     BSP_PARITY_ODD        0x00
   #define     BSP_PARITY_EVEN       0x10
   #define     BSP_PARITY_MARK       0x20
   #define     BSP_PARITY_SPACE      0x30
   #define BSP_LCR_BRK         0x40
   #define BSP_LCR_DLAB        0x80
   #define     BSP_DLAB              0x80
#define BSP_UART1_MCR       (BSP_UART1_BASE + 0x010)
#define BSP_UART1_LSR       (BSP_UART1_BASE + 0x014)
   #define BSP_LSR_DR          0x01
   #define     BSP_RxCHAR_AVAIL      0x01
   #define BSP_LSR_OE          0x02
   #define BSP_LSR_PE          0x04
   #define BSP_LSR_FE          0x08
   #define BSP_LSR_BI          0x10
   #define BSP_LSR_THRE        0x20
   #define     BSP_TxCHAR_AVAIL      0x00
   #define     BSP_TxCHAR_EMPTY      0x20
   #define BSP_LSR_TEMT        0x40
   #define BSP_LSR_RFE         0x80


/*
 * Interrupt Controller
 */
#define BSP_GIMR            0xB8003000
#if defined(RTL8196C) || defined(CONFIG_RTL_8196C)
   #define BSP_USB_H_IE        (1 << 16)
   #define BSP_TC1_IE          (1 << 15)
   #define BSP_TC0_IE          (1 << 14)
   #define BSP_GDMA_IE         (1 << 11)
   #define BSP_PCIE_IE         (1 << 10)  // shall be 22
   #define BSP_PCI_IE          (1 << 10)
   #define BSP_GPIO_ABCD_IE    (1 << 9)
   #define BSP_SW_IE           (1 << 8)
   #define BSP_UART0_IE        (1 << 7)
#else   
   #define BSP_GDMA_IE         (1 << 23)
   #define BSP_PCIE2_IE        (1 << 22)
   #define BSP_PCIE_IE         (1 << 21)  // shall be 22
   #define BSP_CRYPTO_IE       (1 << 20)
   #define BSP_PCM_IE          (1 << 19)
   #define BSP_NFBI_IE         (1 << 18)
   #define BSP_GPIO_EFGH_IE    (1 << 17)
   #define BSP_GPIO_ABCD_IE    (1 << 16)
   #define BSP_SW_IE           (1 << 15)
   #define BSP_PCI_IE          (1 << 14)
   #define BSP_UART1_IE        (1 << 13)
   #define BSP_UART0_IE        (1 << 12)
   #define BSP_USB_H_IE        (1 << 16)
   #define BSP_TC1_IE          (1 << 9)
   #define BSP_TC0_IE          (1 << 8)
   #define BSP_LBCTMOs_IE      (1 << 4)
   #define BSP_LBCTMOm1_IE     (1 << 3)
   #define BSP_LBCTMOm0_IE     (1 << 2)
   #define BSP_PCIB1TO_IE      (1 << 1)
   #define BSP_PCIB0TO_IE      (1 << 0)
#endif
#define BSP_GISR            0xB8003004
#if defined(RTL8196C) || defined(CONFIG_RTL_8196C)
   #define BSP_USB_H_IP        (1 << 16)
   #define BSP_TC1_IP          (1 << 15)
   #define BSP_TC0_IP          (1 << 14)
   #define BSP_GDMA_IP         (1 << 11)
   #define BSP_PCIE_IP         (1 << 10)  
   #define BSP_PCI_IP          (1 << 10)
   #define BSP_GPIO_ABCD_IP    (1 << 9)
   #define BSP_SW_IP           (1 << 8)
   #define BSP_UART0_IP        (1 << 7)
   #else
   	#define BSP_GDMA_IP         (1 << 23)
   	#define BSP_PCIE2_IP        (1 << 22)
  	#define BSP_PCIE_IP         (1 << 21) // shall be 22
   	#define BSP_CRYPTO_IP       (1 << 20)
   	#define BSP_PCM_IP          (1 << 19)
   	#define BSP_NFBI_IP         (1 << 18)
   	#define BSP_GPIO_EFGH_IP    (1 << 17)
   	//#define BSP_GPIO_ABCD_IP    (1 << 16)
   	#define BSP_SW_IP           (1 << 15)
   	#define BSP_PCI_IP          (1 << 14)
   	#define BSP_UART1_IP        (1 << 13)
   	#define BSP_UART0_IP        (1 << 12)
   	#define BSP_USB_H_IP        (1 << 16)
   	#define BSP_TC1_IP          (1 << 9)
   	#define BSP_TC0_IP          (1 << 8)
   	#define BSP_LBCTMOs_IP      (1 << 4)
   	#define BSP_LBCTMOm1_IP     (1 << 3)
   	#define BSP_LBCTMOm0_IP     (1 << 2)
   	#define BSP_PCIB1TO_IP      (1 << 1)
   	#define BSP_PCIB0TO_IP      (1 << 0)
   #endif
#define BSP_IRR0            0xB8003008
#define BSP_IRR0_SETTING    ((BSP_UART0_RS << 28) | \
                         (BSP_NONE << 24) | \
                         (BSP_NONE    << 20) | \
                         (BSP_NONE << 16) | \
                         (BSP_NONE << 12) | \
                         (BSP_NONE   << 8)  | \
                         (BSP_NONE        << 4)  | \
                         (BSP_NONE  << 0)    \
                        )

#define BSP_IRR1            0xB800300C
#define BSP_IRR1_SETTING    ((BSP_TC1_RS<<28)|\
			(BSP_TC0_RS<<24)|\
			(BSP_NONE<<20)|\
			(BSP_OCPTMO_RS<<16)|\
			(BSP_GDMA_RS<<12)|\
			(BSP_PCIE_RS<<8)|\
			(BSP_GPIO_ABCD_RS<<4)|\
			(BSP_SW_RS<<0))

#define BSP_IRR2            0xB8003010
#define BSP_IRR2_SETTING    ((BSP_NONE      << 28) | \
                         (BSP_NONE     << 24) | \
                         (BSP_NONE      << 20) | \
                         (BSP_NONE  << 16) | \
                         (BSP_NONE       << 12) | \
                         (BSP_NONE      << 8)  | \
                         (BSP_PCM_RS << 4)  | \
                         (BSP_USB_H_RS << 0)    \
                        )

#define BSP_IRR3            0xB8003014
#define BSP_IRR3_SETTING    ((BSP_NONE      << 28) | \
                         (BSP_NONE << 24) | \
                         (BSP_NONE << 20) | \
                         (BSP_NONE      << 16) | \
                         (BSP_NONE      << 12) | \
                         (BSP_NONE   << 8)  | \
                         (BSP_NONE      << 4)  | \
                         (BSP_NONE      << 0)    \
                        )
/*
 * Timer/Counter
 */
#define BSP_TC_BASE         0xB8003100
#define BSP_TC0DATA         (BSP_TC_BASE + 0x00)
#define BSP_TC1DATA         (BSP_TC_BASE + 0x04)
   #define BSP_TCD_OFFSET      8
#define BSP_TC0CNT          (BSP_TC_BASE + 0x08)
#define BSP_TC1CNT          (BSP_TC_BASE + 0x0C)
#define BSP_TCCNR           (BSP_TC_BASE + 0x10)
   #define BSP_TC0EN           (1 << 31)
   #define BSP_TC0MODE_TIMER   (1 << 30)
   #define BSP_TC1EN           (1 << 29)
   #define BSP_TC1MODE_TIMER   (1 << 28)
#define BSP_TCIR            (BSP_TC_BASE + 0x14)
   #define BSP_TC0IE           (1 << 31)
   #define BSP_TC1IE           (1 << 30)
   #define BSP_TC0IP           (1 << 29)
   #define BSP_TC1IP           (1 << 28)
#define BSP_CDBR            (BSP_TC_BASE + 0x18)
   #define BSP_DIVF_OFFSET     16
#define BSP_WDTCNR          (BSP_TC_BASE + 0x1C)

/*
 * PCIE Host Controller
 */
#define BSP_PCIE0_H_CFG     0xB8B00000
#define BSP_PCIE0_H_EXT     0xB8B01000
#define BSP_PCIE0_H_MDIO    (BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR  (BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR   (BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG   (BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC    (BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE0_D_CFG0    0xB8B10000
#define BSP_PCIE0_D_CFG1    0xB8B11000
#define BSP_PCIE0_D_MSG     0xB8B12000

#define BSP_PCIE1_H_CFG     0xB8B20000
#define BSP_PCIE1_H_EXT     0xB8B21000
#define BSP_PCIE1_H_MDIO    (BSP_PCIE1_H_EXT + 0x00)
#define BSP_PCIE1_H_INTSTR  (BSP_PCIE1_H_EXT + 0x04)
#define BSP_PCIE1_H_PWRCR   (BSP_PCIE1_H_EXT + 0x08)
#define BSP_PCIE1_H_IPCFG   (BSP_PCIE1_H_EXT + 0x0C)
#define BSP_PCIE1_H_MISC    (BSP_PCIE1_H_EXT + 0x10)
#define BSP_PCIE1_D_CFG0    0xB8B30000
#define BSP_PCIE1_D_CFG1    0xB8B31000
#define BSP_PCIE1_D_MSG     0xB8B32000

#define BSP_PCIE0_D_IO      0xB8C00000
#define BSP_PCIE1_D_IO      0xB8E00000
#define BSP_PCIE_FUN_OFS      0xC00000
#define BSP_PCIE0_D_MEM     0xB9000000
#define BSP_PCIE0_F1_D_MEM  (BSP_PCIE0_D_MEM + BSP_PCIE_FUN_OFS)
#define BSP_PCIE1_D_MEM     0xBA000000
#define BSP_PCIE1_F1_D_MEM  (BSP_PCIE1_D_MEM + BSP_PCIE_FUN_OFS)

#ifndef REVR
#define REVR                                    0xB8000000
#endif
 
#ifndef RTL8196C_REVISION_A
#define RTL8196C_REVISION_A     0x80000001
#endif
 
#ifndef RTL8196C_REVISION_B
#define RTL8196C_REVISION_B     0x80000002
#endif

#endif   /* _BSPCHIP_H */
