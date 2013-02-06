#ifndef _PLATFORM_H
#define _PLATFORM_H


/*
 *  =============
 *  Utilty Macros
 *  =============
 */
#define REG8(reg)    (*(volatile unsigned char  *)((unsigned int)reg))
#define REG16(reg)   (*(volatile unsigned short *)((unsigned int)reg))
#define REG32(reg)   (*(volatile unsigned int   *)((unsigned int)reg))

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)

/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

#define MHZ             30
//#define MHZ             200
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

/*
 * Interrupt IRQ Assignments
 */
#ifdef CONFIG_RTL8196C


#define PTM_IRQ         31
#define LBCTMOs2_IRQ    30
#define LBCTMOs1_IRQ    29
#define PKT_IRQ         28
#define SPI_IRQ         27
#define NIC100_IRQ      26
#define SAR_IRQ         25
#define DMT_IRQ         24
#define PCIE2_IRQ       22
#define PCIE_IRQ        10//21  // shall be 22
#define GDMA_IRQ        11
#define SECURITY_IRQ    20
#define PCM_IRQ         19
#define HCI_IRQ         18
#define GPIO_EFGH_IRQ   17
#define GPIO_ABCD_IRQ   9
#define SW_IRQ          8
#define PCI_IRQ         14
#define UART1_IRQ       13
#define UART0_IRQ       7
#define USB_D_IRQ       11
#define USB_H_IRQ       10
#define TC1_IRQ         15
#define TC0_IRQ         14
#define LBCTMOm2_IRQ    1
#define LBCTMOm1_IRQ    6
#define SPEED_IRQ       5
#define LBCTMOs0_IRQ    4
#define LBCTMOm0_IRQ    3
#define OCPTMO_IRQ      2
#define PCIB0TO_IRQ     0

#elif defined(CONFIG_RTL8198)

#define PTM_IRQ         31
#define LBCTMOs2_IRQ    30
#define LBCTMOs1_IRQ    29
#define PKT_IRQ         28
#define SPI_IRQ         27
#define NIC100_IRQ      26
#define SAR_IRQ         25
#define DMT_IRQ         24
#define PCIE2_IRQ       22
#define PCIE_IRQ        21  // shall be 22
#define GDMA_IRQ        23
#define SECURITY_IRQ    20
#define PCM_IRQ         19
#define HCI_IRQ         18
#define GPIO_EFGH_IRQ   17
#define GPIO_ABCD_IRQ   16
#define SW_IRQ          15
#define PCI_IRQ         14
#define UART1_IRQ       13
#define UART0_IRQ       12
#define USB_D_IRQ       11
#define USB_H_IRQ       10
#define TC1_IRQ         9
#define TC0_IRQ         8
#define LBCTMOm2_IRQ    7
#define LBCTMOm1_IRQ    6
#define SPEED_IRQ       5
#define LBCTMOs0_IRQ    4
#define LBCTMOm0_IRQ    3
#define OCPTMO_IRQ      2
#define PCIB0TO_IRQ     0

#else

#define PTM_IRQ         31
#define LBCTMOs2_IRQ    30
#define LBCTMOs1_IRQ    29
#define PKT_IRQ         28
#define SPI_IRQ         27
#define NIC100_IRQ      26
#define SAR_IRQ         25
#define DMT_IRQ         24
#define PCIE2_IRQ       22
#define PCIE_IRQ        21  // shall be 22
#define GDMA_IRQ        23
#define SECURITY_IRQ    20
#define PCM_IRQ         19
#define HCI_IRQ         18
#define GPIO_EFGH_IRQ   17
#define GPIO_ABCD_IRQ   16
#define SW_IRQ          15
#define PCI_IRQ         14
#define UART1_IRQ       13
#define UART0_IRQ       12
#define USB_D_IRQ       11
#define USB_H_IRQ       10
#define TC1_IRQ         9
#define TC0_IRQ         8
#define LBCTMOm2_IRQ    7
#define LBCTMOm1_IRQ    6
#define SPEED_IRQ       5
#define LBCTMOs0_IRQ    4
#define LBCTMOm0_IRQ    3
#define OCPTMO_IRQ      2
#define PCIB0TO_IRQ     0

#endif









/*
 * Interrupt Routing Selection
 */
#define PTM_RS          2
#define LBCTMOs2_RS     2
#define LBCTMOs1_RS     2
#define PKT_RS          2
#define SPI_RS          2
#define NIC100_RS       2
#define SAR_RS          2
#define DMT_RS          2
#define PCIE2_RS        4
#define PCIE_RS         6
#define GDMA_RS         2
#define SECURITY_RS     2
#define PCM_RS          2
#define NFBI_RS         2
#define GPIO_EFGH_RS    2
#define GPIO_ABCD_RS    5
#define SW_RS           6
#define PCI_RS          5
#define UART1_RS        2
#define UART0_RS        3
#define USB_D_RS        2
#define USB_H_RS        4
#define TC1_RS          2
#define TC0_RS          7
#define LBCTMOm2_RS     2
#define LBCTMOm1_RS     2
#define SPEED_RS        2
#define LBCTMOs0_RS     2
#define LBCTMOm0_RS     2
#define OCPTMO_RS       2
#define PCIB0TO_RS      2
#define NONE            2
#define HCI_RS			3

#define DIVISOR         1000

#if DIVISOR > (1 << 16)
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
#define IMEM_BASE       0x00C00000
#define IMEM_TOP        0x00C03FFF

#define DMEM_BASE       0x00C04000
#define DMEM_TOP        0x00C05FFF

/*
 * Memory Controller
 */
#define MC_MCR          0xB8001000
   #define MC_MCR_VAL      0x92A28000

#define MC_MTCR0        0xB8001004
   #define MC_MTCR0_VAL    0x12120000

#define MC_MTCR1        0xB8001008
   #define MC_MTCR1_VAL    0x00000FEB

#define MC_PFCR         0xB8001010
   #define MC_PFCR_VAL     0x00000101


#define MC_BASE         0xB8001000
#define NCR             (MC_BASE + 0x100)
#define NSR             (MC_BASE + 0x104)
#define NCAR            (MC_BASE + 0x108)
#define NADDR           (MC_BASE + 0x10C)
#define NDR             (MC_BASE + 0x110)

#define SFCR            (MC_BASE + 0x200)
#define SFDR            (MC_BASE + 0x204)

/*
 * UART
 */
#define UART0_BASE      0xB8002000
#define UART0_RBR       (UART0_BASE + 0x000)
#define UART0_THR       (UART0_BASE + 0x000)
#define UART0_DLL       (UART0_BASE + 0x000)
#define UART0_IER       (UART0_BASE + 0x004)
#define UART0_DLM       (UART0_BASE + 0x004)
#define UART0_IIR       (UART0_BASE + 0x008)
#define UART0_FCR       (UART0_BASE + 0x008)
#define UART0_LCR       (UART0_BASE + 0x00C)
#define UART0_MCR       (UART0_BASE + 0x010)
#define UART0_LSR       (UART0_BASE + 0x014)

#define UART1_BASE      0xB8002100
#define UART1_RBR       (UART1_BASE + 0x000)
#define UART1_THR       (UART1_BASE + 0x000)
#define UART1_DLL       (UART1_BASE + 0x000)
#define UART1_IER       (UART1_BASE + 0x004)
#define UART1_DLM       (UART1_BASE + 0x004)
#define UART1_IIR       (UART1_BASE + 0x008)
#define UART1_FCR       (UART1_BASE + 0x008)
   #define FCR_EN          0x01
   #define FCR_RXRST       0x02
   #define     RXRST             0x02
   #define FCR_TXRST       0x04
   #define     TXRST             0x04
   #define FCR_DMA         0x08
   #define FCR_RTRG        0xC0
   #define     CHAR_TRIGGER_01   0x00
   #define     CHAR_TRIGGER_04   0x40
   #define     CHAR_TRIGGER_08   0x80
   #define     CHAR_TRIGGER_14   0xC0
#define UART1_LCR       (UART1_BASE + 0x00C)
   #define LCR_WLN         0x03
   #define     CHAR_LEN_5        0x00
   #define     CHAR_LEN_6        0x01
   #define     CHAR_LEN_7        0x02
   #define     CHAR_LEN_8        0x03
   #define LCR_STB         0x04
   #define     ONE_STOP          0x00
   #define     TWO_STOP          0x04
   #define LCR_PEN         0x08
   #define     PARITY_ENABLE     0x01
   #define     PARITY_DISABLE    0x00
   #define LCR_EPS         0x30
   #define     PARITY_ODD        0x00
   #define     PARITY_EVEN       0x10
   #define     PARITY_MARK       0x20
   #define     PARITY_SPACE      0x30
   #define LCR_BRK         0x40
   #define LCR_DLAB        0x80
   #define     DLAB              0x80
#define UART1_MCR       (UART1_BASE + 0x010)
#define UART1_LSR       (UART1_BASE + 0x014)
   #define LSR_DR          0x01
   #define     RxCHAR_AVAIL      0x01
   #define LSR_OE          0x02
   #define LSR_PE          0x04
   #define LSR_FE          0x08
   #define LSR_BI          0x10
   #define LSR_THRE        0x20
   #define     TxCHAR_AVAIL      0x00
   #define     TxCHAR_EMPTY      0x20
   #define LSR_TEMT        0x40
   #define LSR_RFE         0x80


/*
 * Interrupt Controller
 */
#define GIMR            0xB8003000
#if defined(CONFIG_RTL8196C)
   #define PTM_IE          (1 << 31)
   #define LBCTMOs2_IE     (1 << 30)
   #define LBCTMOs1_IE     (1 << 29)
   #define PKT_IE          (1 << 28)
   #define SPI_IE          (1 << 27)
   #define NIC100_IE       (1 << 26)
   #define SAR_IE          (1 << 25)
   #define DMT_IE          (1 << 24)
   #define PCIE2_IE        (1 << 22)
   #define PCIE_IE         (1 << 10)  // shall be 22
   #define GDMA_IE         (1 << 23)
   #define SECURITY_IE     (1 << 20)
   #define PCM_IE          (1 << 19)
   #define HCI_IE          (1 << 18)
   #define GPIO_EFGH_IE    (1 << 17)
   #define GPIO_ABCD_IE    (1 << 9)
   #define SW_IE           (1 << 8)
   #define PCI_IE          (1 << 14)
   #define UART1_IE        (1 << 13)
   #define UART0_IE        (1 << 7)
   #define USB_D_IE        (1 << 11)
   #define USB_H_IE        (1 << 10)
   #define TC1_IE          (1 << 15)
   #define TC0_IE          (1 << 14)
   #define LBCTMOm2_IE     (1 << 7)
   #define LBCTMOm1_IE     (1 << 6)
   #define SPEED_IE        (1 << 5)
   #define LBCTMOs0_IE     (1 << 4)
   #define LBCTMOm0_IE     (1 << 3)
   #define OCPTMO_IE       (1 << 2)
//   #define PCIB1TO_IE      (1 << 1)
   #define PCIB0TO_IE      (1 << 0)

#define GISR            0xB8003004
   #define PTM_IP          (1 << 31)
   #define LBCTMOs2_IP     (1 << 30)
   #define LBCTMOs1_IP     (1 << 29)
   #define PKT_IP          (1 << 28)
   #define SPI_IP          (1 << 27)
   #define NIC100_IP       (1 << 26)
   #define SAR_IP          (1 << 25)
   #define DMT_IP          (1 << 24)
   #define PCIE2_IP        (1 << 22)
   #define PCIE_IP         (1 << 10) // shall be 22
   #define GDMA_IP         (1 << 23)
   #define SECURITY_IP     (1 << 20)
   #define PCM_IP          (1 << 19)
//   #define HCI_IP          (1 << 18)
   #define GPIO_EFGH_IP    (1 << 17)
   #define GPIO_ABCD_IP    (1 << 9)
   #define SW_IP           (1 << 8)
   #define PCI_IP          (1 << 14)
   #define UART1_IP        (1 << 13)
   #define UART0_IP        (1 << 7)
   #define USB_D_IP        (1 << 11)
   #define USB_H_IP        (1 << 10)
   #define TC1_IP          (1 << 15)
   #define TC0_IP          (1 << 14)
   #define LBCTMOm2_IP     (1 << 7)
   #define LBCTMOm1_IP     (1 << 6)
   #define SPEED_IP        (1 << 5)
   #define LBCTMOs0_IP     (1 << 4)
   #define LBCTMOm0_IP     (1 << 3)
   #define OCPTMO_IP       (1 << 2)
//   #define PCIB1TO_IP      (1 << 1)
   #define PCIB0TO_IP      (1 << 0)

#define IRR0            0xB8003008
#define IRR0_SETTING    ((UART0_RS << 28) | \
                         (LBCTMOm1_RS << 24) | \
                         (SPEED_RS    << 20) | \
                         (LBCTMOs0_RS << 16) | \
                         (LBCTMOm0_RS << 12) | \
                         (OCPTMO_RS   << 8)  | \
                         (NONE        << 4)  | \
                         (PCIB0TO_RS  << 0)    \
                        )

#define IRR1            0xB800300C
#define IRR1_SETTING    ((SW_RS    << 0) | \
                         (PCIE_RS  << 8) | \
                         (UART1_RS << 20) | \
                         (OCPTMO_RS << 16) | \
                         (GDMA_RS << 12) | \
                         (GPIO_ABCD_RS<< 4)  | \
                         (TC1_RS   << 18)  | \
                         (TC0_RS   << 24)    \
                        )
#elif defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8198) 
   #define PTM_IE          (1 << 31)
   #define LBCTMOs2_IE     (1 << 30)
   #define LBCTMOs1_IE     (1 << 29)
   #define PKT_IE          (1 << 28)
   #define SPI_IE          (1 << 27)
   #define NIC100_IE       (1 << 26)
   #define SAR_IE          (1 << 25)
   #define DMT_IE          (1 << 24)
   #define PCIE2_IE        (1 << 22)
   #define PCIE_IE         (1 << 21)  // shall be 22
   #define GDMA_IE         (1 << 23)
   #define SECURITY_IE     (1 << 20)
   #define PCM_IE          (1 << 19)
   #define HCI_IE          (1 << 18)
   #define GPIO_EFGH_IE    (1 << 17)
   #define GPIO_ABCD_IE    (1 << 16)
   #define SW_IE           (1 << 15)
   #define PCI_IE          (1 << 14)
   #define UART1_IE        (1 << 13)
   #define UART0_IE        (1 << 12)
   #define USB_D_IE        (1 << 11)
   #define USB_H_IE        (1 << 10)
   #define TC1_IE          (1 << 9)
   #define TC0_IE          (1 << 8)
   #define LBCTMOm2_IE     (1 << 7)
   #define LBCTMOm1_IE     (1 << 6)
   #define SPEED_IE        (1 << 5)
   #define LBCTMOs0_IE     (1 << 4)
   #define LBCTMOm0_IE     (1 << 3)
   #define OCPTMO_IE       (1 << 2)
//   #define PCIB1TO_IE      (1 << 1)
   #define PCIB0TO_IE      (1 << 0)

#define GISR            0xB8003004
   #define PTM_IP          (1 << 31)
   #define LBCTMOs2_IP     (1 << 30)
   #define LBCTMOs1_IP     (1 << 29)
   #define PKT_IP          (1 << 28)
   #define SPI_IP          (1 << 27)
   #define NIC100_IP       (1 << 26)
   #define SAR_IP          (1 << 25)
   #define DMT_IP          (1 << 24)
   #define PCIE2_IP        (1 << 22)
   #define PCIE_IP         (1 << 21) // shall be 22
   #define GDMA_IP         (1 << 23)
   #define SECURITY_IP     (1 << 20)
   #define PCM_IP          (1 << 19)
//   #define HCI_IP          (1 << 18)
   #define GPIO_EFGH_IP    (1 << 17)
   #define GPIO_ABCD_IP    (1 << 16)
   #define SW_IP           (1 << 15)
   #define PCI_IP          (1 << 14)
   #define UART1_IP        (1 << 13)
   #define UART0_IP        (1 << 12)
   #define USB_D_IP        (1 << 11)
   #define USB_H_IP        (1 << 10)
   #define TC1_IP          (1 << 9)
   #define TC0_IP          (1 << 8)
   #define LBCTMOm2_IP     (1 << 7)
   #define LBCTMOm1_IP     (1 << 6)
   #define SPEED_IP        (1 << 5)
   #define LBCTMOs0_IP     (1 << 4)
   #define LBCTMOm0_IP     (1 << 3)
   #define OCPTMO_IP       (1 << 2)
//   #define PCIB1TO_IP      (1 << 1)
   #define PCIB0TO_IP      (1 << 0)

#define IRR0            0xB8003008
#define IRR0_SETTING    ((LBCTMOm2_RS << 28) | \
                         (LBCTMOm1_RS << 24) | \
                         (SPEED_RS    << 20) | \
                         (LBCTMOs0_RS << 16) | \
                         (LBCTMOm0_RS << 12) | \
                         (OCPTMO_RS   << 8)  | \
                         (NONE        << 4)  | \
                         (PCIB0TO_RS  << 0)    \
                        )

#define IRR1            0xB800300C
#define IRR1_SETTING    ((SW_RS    << 28) | \
                         (PCIE_RS  << 24) | \
                         (UART1_RS << 20) | \
                         (UART0_RS << 16) | \
                         (USB_D_RS << 12) | \
                         (USB_H_RS << 8)  | \
                         (TC1_RS   << 4)  | \
                         (TC0_RS   << 0)    \
                        )
#endif

#define IRR2            0xB8003010
#define IRR2_SETTING    ((GDMA_RS      << 28) | \
                         (PCIE2_RS     << 24) | \
                         (PCIE_RS      << 20) | \
                         (SECURITY_RS  << 16) | \
                         (PCM_RS       << 12) | \
                         (NFBI_RS      << 8)  | \
                         (GPIO_EFGH_RS << 4)  | \
                         (GPIO_ABCD_RS << 0)    \
                        )

#define IRR3            0xB8003014
#define IRR3_SETTING    ((PTM_RS      << 28) | \
                         (LBCTMOs2_RS << 24) | \
                         (LBCTMOs1_RS << 20) | \
                         (PKT_RS      << 16) | \
                         (SPI_RS      << 12) | \
                         (NIC100_RS   << 8)  | \
                         (SAR_RS      << 4)  | \
                         (DMT_RS      << 0)    \
                        )

/*
 * Timer/Counter
 */
#define TC_BASE         0xB8003100
#define TC0DATA         (TC_BASE + 0x00)
#define TC1DATA         (TC_BASE + 0x04)
   #define TCD_OFFSET      8
#define TC0CNT          (TC_BASE + 0x08)
#define TC1CNT          (TC_BASE + 0x0C)
#define TCCNR           (TC_BASE + 0x10)
   #define TC0EN           (1 << 31)
   #define TC0MODE_TIMER   (1 << 30)
   #define TC1EN           (1 << 29)
   #define TC1MODE_TIMER   (1 << 28)
#define TCIR            (TC_BASE + 0x14)
   #define TC0IE           (1 << 31)
   #define TC1IE           (1 << 30)
   #define TC0IP           (1 << 29)
   #define TC1IP           (1 << 28)
#define CDBR            (TC_BASE + 0x18)
   #define DIVF_OFFSET     16
#define WDTCNR          (TC_BASE + 0x1C)

/*
 * PCIE Host Controller
 */
#define PCIE0_H_CFG     0xB8B00000
#define PCIE0_H_EXT     0xB8B01000
#define PCIE0_H_MDIO    (PCIE0_H_EXT + 0x00)
#define PCIE0_H_INTSTR  (PCIE0_H_EXT + 0x04)
#define PCIE0_H_PWRCR   (PCIE0_H_EXT + 0x08)
#define PCIE0_H_IPCFG   (PCIE0_H_EXT + 0x0C)
#define PCIE0_H_MISC    (PCIE0_H_EXT + 0x10)
#define PCIE0_D_CFG0    0xB8B10000
#define PCIE0_D_CFG1    0xB8B11000
#define PCIE0_D_MSG     0xB8B12000

#define PCIE1_H_CFG     0xB8B20000
#define PCIE1_H_EXT     0xB8B21000
#define PCIE1_H_MDIO    (PCIE1_H_EXT + 0x00)
#define PCIE1_H_INTSTR  (PCIE1_H_EXT + 0x04)
#define PCIE1_H_PWRCR   (PCIE1_H_EXT + 0x08)
#define PCIE1_H_IPCFG   (PCIE1_H_EXT + 0x0C)
#define PCIE1_H_MISC    (PCIE1_H_EXT + 0x10)
#define PCIE1_D_CFG0    0xB8B30000
#define PCIE1_D_CFG1    0xB8B31000
#define PCIE1_D_MSG     0xB8B32000

#define PCIE0_D_IO      0xB8C00000
#define PCIE1_D_IO      0xB8E00000
#define PCIE0_D_MEM     0xB9000000
#define PCIE1_D_MEM     0xBA000000

#define REVR                 			0xB8000000
#define RTL8196C_REVISION_A	0x80000001
#define RTL8196C_REVISION_B	0x80000002

#endif /* _PLATFORM_H */
