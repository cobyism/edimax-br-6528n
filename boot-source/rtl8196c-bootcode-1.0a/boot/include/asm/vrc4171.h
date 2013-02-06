/*
 * linux/include/asm-mips/vrc4171.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999 by Steve K. Longerbeam
 * Copyright (C) 2000 Michael Klar <wyldfier@iname.com>
 */

#ifndef __ASM_MIPS_VRC4171_H_
#define __ASM_MIPS_VRC4171_H_

#include <asm/vr41xx.h>

//
// LCD display and PCMCIA controller base addresses
//
#define VRC4171_LCD_BASE    (KSEG1 + VR41XX_LCD)
#define VRC4171_LCDMEM_BASE VRC4171_LCD_BASE
#define VRC4171_LCDREG_BASE (VRC4171_LCD_BASE + 0x00800000)
#define VRC4171_PCMCIA_BASE (KSEG1 + VR41XX_ISA_IO + 0x01000000)
 
#define VRC4171_Config1          __preg16(VRC4171_PCMCIA_BASE + 0x05fe)
#define VRC4171_Config2          __preg16(VRC4171_PCMCIA_BASE + 0x05fc)
#define VRC4171_IntrStat         __preg16(VRC4171_PCMCIA_BASE + 0x05fa)
#define VRC4171_GPIOControlData  __preg16(VRC4171_PCMCIA_BASE + 0x05ee)

// NOTE: These reg defs are unconfirmed by documentation.  In fact, I'm
// not real sure where they came from.  This may be a VRC4171 vs VRC4171A
// issue, since the only documentation available is for VRC4171A.  MFK
#define VRC4171_PCS0UpperStrtStp __preg16(VRC4171_PCMCIA_BASE + 0x05ec)
#define VRC4171_PCS0LowerStart   __preg16(VRC4171_PCMCIA_BASE + 0x05ea)
#define VRC4171_PCS0LowerStop    __preg16(VRC4171_PCMCIA_BASE + 0x05e8)
#define VRC4171_PCS1UpperStrtStp __preg16(VRC4171_PCMCIA_BASE + 0x05e6)
#define VRC4171_PCS1LowerStart   __preg16(VRC4171_PCMCIA_BASE + 0x05e4)
#define VRC4171_PCS1LowerStop    __preg16(VRC4171_PCMCIA_BASE + 0x05e2)

// These reg defs are per the VRC4171A documentation:
#define VRC4171A_PCS0UpperStart  __preg16(VRC4171_PCMCIA_BASE + 0x05ec)
#define VRC4171A_PCS0LowerStart  __preg16(VRC4171_PCMCIA_BASE + 0x05ea)
#define VRC4171A_PCS0UpperStop   __preg16(VRC4171_PCMCIA_BASE + 0x05e8)
#define VRC4171A_PCS0LowerStop   __preg16(VRC4171_PCMCIA_BASE + 0x05e6)
#define VRC4171A_PCS1UpperStart  __preg16(VRC4171_PCMCIA_BASE + 0x05e4)
#define VRC4171A_PCS1LowerStart  __preg16(VRC4171_PCMCIA_BASE + 0x05e2)
#define VRC4171A_PCS1UpperStop   __preg16(VRC4171_PCMCIA_BASE + 0x05de)
#define VRC4171A_PCS1LowerStop   __preg16(VRC4171_PCMCIA_BASE + 0x05dc)

// The PCIC Index and Data registers
#define VRC4171_PCICINDEX        __preg8(VRC4171_PCMCIA_BASE + 0x03e0)
#define VRC4171_PCICDATA         __preg8(VRC4171_PCMCIA_BASE + 0x03e1)

// The LCD registers
#define VRC4171_PanelSelect      __preg16(VRC4171_LCDREG_BASE + 0x0000)
#define VRC4171_LCDPanelCtl      __preg16(VRC4171_LCDREG_BASE + 0x0002)
#define VRC4171_PowerModeCtl     __preg16(VRC4171_LCDREG_BASE + 0x0004)
#define VRC4171_MCLKEnable       __preg16(VRC4171_LCDREG_BASE + 0x0006)
#define VRC4171_VCLKEnable       __preg16(VRC4171_LCDREG_BASE + 0x0008)
#define VRC4171_VidFIFOMemIntCtl __preg16(VRC4171_LCDREG_BASE + 0x000A)
#define VRC4171_PixelAdjVertHalf __preg16(VRC4171_LCDREG_BASE + 0x000C)
#define VRC4171_HorzDisplayCtl   __preg16(VRC4171_LCDREG_BASE + 0x0010)
#define VRC4171_HorzRetrCtl      __preg16(VRC4171_LCDREG_BASE + 0x0012)
#define VRC4171_VertDispEndCtl   __preg16(VRC4171_LCDREG_BASE + 0x0014)
#define VRC4171_VertDispCtl      __preg16(VRC4171_LCDREG_BASE + 0x0016)
#define VRC4171_VertRetrStartCtl __preg16(VRC4171_LCDREG_BASE + 0x0018)
#define VRC4171_VertRetrEndCtl   __preg16(VRC4171_LCDREG_BASE + 0x001A)
#define VRC4171_StartingAddress  __preg16(VRC4171_LCDREG_BASE + 0x001C)
#define VRC4171_Offset           __preg16(VRC4171_LCDREG_BASE + 0x001E)
#define VRC4171_HardwareCurCtl   __preg16(VRC4171_LCDREG_BASE + 0x0020)
#define VRC4171_HardwareCurXPos  __preg16(VRC4171_LCDREG_BASE + 0x0022)
#define VRC4171_HardwareCurYPos  __preg16(VRC4171_LCDREG_BASE + 0x0024)
#define VRC4171_HardwareCurClr0A __preg16(VRC4171_LCDREG_BASE + 0x0026)
#define VRC4171_HardwareCurClr0B __preg16(VRC4171_LCDREG_BASE + 0x0028)
#define VRC4171_HardwareCurClr1A __preg16(VRC4171_LCDREG_BASE + 0x002A)
#define VRC4171_HardwareCurClr1B __preg16(VRC4171_LCDREG_BASE + 0x002C)
#define VRC4171_HardwareCurOrign __preg16(VRC4171_LCDREG_BASE + 0x002E)
#define VRC4171_RAMWriteAddress  __preg16(VRC4171_LCDREG_BASE + 0x0040)
#define VRC4171_RAMWritePort0    __preg16(VRC4171_LCDREG_BASE + 0x0042)
#define VRC4171_RAMWritePort1    __preg16(VRC4171_LCDREG_BASE + 0x0044)
// Note: RAMRead doesn't seem to work on VRC4171, maybe another VRC4171A diff
#define VRC4171_RAMReadAddress   __preg16(VRC4171_LCDREG_BASE + 0x0046)
#define VRC4171_RAMReadPort0     __preg16(VRC4171_LCDREG_BASE + 0x0048)
#define VRC4171_RAMReadPort1     __preg16(VRC4171_LCDREG_BASE + 0x004A)
#define VRC4171_ScratchPadReg0   __preg16(VRC4171_LCDREG_BASE + 0x0050)
#define VRC4171_ScratchPadReg1   __preg16(VRC4171_LCDREG_BASE + 0x0052)

//-----------------------------------------------------------
// Configuration I Register (VRC4171_Config1) Bit Definitions
//-----------------------------------------------------------

#define SLOT_B_NONE                        0x0000
#define SLOT_B_PCMCIA                      0x4000
#define SLOT_B_COMP_FLASH                  0x8000
#define SLOT_B_MINI                        0xc000
#define SLOT_B_MASK                        0xc000
#define EN_OR                              0x2000
#define EN_VCLK                            0x1000
#define EN_BUSCLK                          0x0800
#define IO_UADEC                           0x0200
#define VMCLK_DIV8                         0x01c0
#define VMCLK_DIV7                         0x0180
#define VMCLK_DIV6                         0x0140
#define VMCLK_DIV5                         0x0100
#define VMCLK_DIV4                         0x00c0
#define VMCLK_DIV3                         0x0080
#define VMCLK_DIV2                         0x0040
#define VMCLK_DIV1                         0x0000
#define VCLK_DIV8                          0x0038
#define VCLK_DIV7                          0x0030
#define VCLK_DIV6                          0x0028
#define VCLK_DIV5                          0x0020
#define VCLK_DIV4                          0x0018
#define VCLK_DIV3                          0x0010
#define VCLK_DIV2                          0x0008
#define VCLK_DIV1                          0x0000

//------------------------------------------------------------
// Configuration II Register (VRC4171_Config2) Bit Definitions
//------------------------------------------------------------
#define SIL_REV_MASK         0xf000
#define  REV_VRC4171A        0x2000
#define  REV_VRC4171         0x1000
#define MCLK_EN              0x0400
#define READ_HOLD_DELAY_NONE 0x0000
#define READ_HOLD_DELAY_7    0x0004
#define READ_HOLD_DELAY_X1   0x0008
#define READ_HOLD_DELAY_14   0x000c
#define IOCH_R1              0x0002



//--------------------------------------------------------//
// Register Index Values
//--------------------------------------------------------//
typedef enum {
    IndexChipRevision = 0,         // 0x00
    IndexInterfaceStatus,          // 0x01
    IndexPowerControl,             // 0x02
    IndexIntrAndGeneralControl,    // 0x03
    IndexCardStatusChange,         // 0x04
    IndexCardStatusIntrConfig,     // 0x05
    IndexMappingEnable,            // 0x06
    IndexIoWindowControl,          // 0x07
    IndexIomap0StartAddrLow,       // 0x08
    IndexIomap0StartAddrHi,        // 0x09
    IndexIomap0EndAddrLow,         // 0x0a
    IndexIomap0EndAddrHi,          // 0x0b
    IndexIomap1StartAddrLow,       // 0x0c
    IndexIomap1StartAddrHi,        // 0x0d
    IndexIomap1EndAddrLow,         // 0x0e
    IndexIomap1EndAddrHi,          // 0x0f
    IndexMemmap0StartAddrLow,      // 0x10
    IndexMemmap0StartAddrHi,       // 0x11
    IndexMemmap0EndAddrLow,        // 0x12
    IndexMemmap0EndAddrHi,         // 0x13
    IndexMemmap0AddrOffsetLow,     // 0x14
    IndexMemmap0AddrOffsetHi,      // 0x15
    IndexMiscControl,              // 0x16
    IndexFifoControl,              // 0x17
    IndexMemmap1StartAddrLow,      // 0x18
    IndexMemmap1StartAddrHi,       // 0x19
    IndexMemmap1EndAddrLow,        // 0x1a
    IndexMemmap1EndAddrHi,         // 0x1b
    IndexMemmap1AddrOffsetLow,     // 0x1c
    IndexMemmap1AddrOffsetHi,      // 0x1d
    IndexGlobalControl,            // 0x1e
    IndexVoltageSense,             // 0x1f
    IndexMemmap2StartAddrLow,      // 0x20
    IndexMemmap2StartAddrHi,       // 0x21
    IndexMemmap2EndAddrLow,        // 0x22
    IndexMemmap2EndAddrHi,         // 0x23
    IndexMemmap2AddrOffsetLow,     // 0x24
    IndexMemmap2AddrOffsetHi,      // 0x25
    IndexAtaControl,               // 0x26
    IndexReserved0,                // 0x27
    IndexMemmap3StartAddrLow,      // 0x28
    IndexMemmap3StartAddrHi,       // 0x29
    IndexMemmap3EndAddrLow,        // 0x2a
    IndexMemmap3EndAddrHi,         // 0x2b
    IndexMemmap3AddrOffsetLow,     // 0x2c
    IndexMemmap3AddrOffsetHi,      // 0x2d
    IndexReserved1,                // 0x2e
    IndexVoltageSelect,            // 0x2f
    IndexMemmap4StartAddrLow,      // 0x30
    IndexMemmap4StartAddrHi,       // 0x31
    IndexMemmap4EndAddrLow,        // 0x32
    IndexMemmap4EndAddrHi,         // 0x33
    IndexMemmap4AddrOffsetLow,     // 0x34
    IndexMemmap4AddrOffsetHi,      // 0x35
    IndexIomap0AddrOffsetLow,      // 0x36
    IndexIomap0AddrOffsetHi,       // 0x37
    IndexIomap1AddrOffsetLow,      // 0x38
    IndexIomap1AddrOffsetHi,       // 0x39
    IndexSetupTiming0,             // 0x3a
    IndexCommandTiming0,           // 0x3b
    IndexRecoveryTiming0,          // 0x3c
    IndexSetupTiming1,             // 0x3d
    IndexCommandTiming1,           // 0x3e
    IndexRecoveryTiming1           // 0x3f
} vrc4171pcm_index_t;


// To access Slot B PCIC data registers, add the following
// to the index values above.
#define SLOT_B_INDEX_OFFSET           0x40

//--------------------------------------------------------//
// SOCKET                                                 //
// Data Register                                          //

//--------------------------------------------------------//
// REGISTER INDEX No.00 Bit Definitions
//--------------------------------------------------------//
#define SLOT_REV_MASK     0x0f
#define IF_TYPE_MASK      0xc0
#define   IF_TYPE_IO_MEM  0x80

//--------------------------------------------------------//
// REGISTER INDEX No.01 Bit Definitions
//--------------------------------------------------------//
#define CARD_POWER_ON                          (1<<6)
#define READY_BUSY                             (1<<5)
#define WRITE_PROTECT                          (1<<4)
#define CARD_DETECT_CD2                        (1<<3)
#define CARD_DETECT_CD1                        (1<<2)
#define BATTERY_VOLTAGE_DETECT_BVD2            (1<<1)
#define BATTERY_VOLTAGE_DETECT_BVD1            (1<<0)

//--------------------------------------------------------//
// REGISTER INDEX No.02 Bit Definitions
//--------------------------------------------------------//
#define OUTPUT_ENABLE                                0x80
#define POWER_ENABLE                                 0x10
#define VPP_SELECT_MASK                              0x03

//--------------------------------------------------------//
// REGISTER INDEX No.03 Bit Definitions
//--------------------------------------------------------//

#define RING_INDICATE_ENABLE                         0x80 // 
#define CARD_RESET                                   0x40 // 
#define CARD_IS_IO                                   0x20 // 
#define ENABLE_MANAGE_INT                            0x10 // 
#define IO_IRQ_BIT_OFFSET                            0

//--------------------------------------------------------//
// REGISTER INDEX No.04 Bit Definitions
//--------------------------------------------------------//
#define DETECT_CHANGE                                0x08 // 
#define READY_CHANGE                                 0x04 // 
#define BATTERY_WARNING                              0x02 // 
#define BATTERY_DEAD_OR_STS_CHG                      0x01 // 

//--------------------------------------------------------//
// REGISTER INDEX No.05 Bit Definitions
//--------------------------------------------------------//

#define CSC_IRQ_BIT_OFFSET                           4
#define CARD_DETECT_ENABLE                           0x08 // 
#define READY_ENABLE                                 0x04 // 
#define BATTERY_WARNING_ENABLE                       0x02 // 
#define BATTERY_DEAD_STSCHG_ENABLE                   0x01 // 

//--------------------------------------------------------//
// REGISTER INDEX No.06 Bit Definitions
//--------------------------------------------------------//

#define IOMAP1_ENABLE                                0x80 // 
#define IOMAP0_ENABLE                                0x40 // 
#define MEMCS16_DECODE                               0x20 // 
#define MEMMAP4_ENABLE                               0x10 // 
#define MEMMAP3_ENABLE                               0x08 // 
#define MEMMAP2_ENABLE                               0x04 // 
#define MEMMAP1_ENABLE                               0x02 // 
#define MEMMAP0_ENABLE                               0x01 // 

//--------------------------------------------------------//
// REGISTER INDEX No.07 Bit Definitions
//--------------------------------------------------------//

#define TIMING_REGISTER_SELECT1                      0x80 // 
//                                                   0x40 // 
#define AUTO_SIZE_IO_WINDOW1                         0x20 // 
#define IO_WINDOW1_SIZE                              0x10 // 
#define TIMING_REGISTER_SELECT0                      0x08 // 
//                                                   0x04 // 
#define AUTO_SIZE_IO_WINDOW0                         0x02 // 
#define IO_WINDOW0_SIZE                              0x01 // 

#define IO_WINDOW0_WS                                0x04
#define IO_WINDOW1_WS                                0x40

//--------------------------------------------------------//
// REGISTER INDEX No.08 ,0C Bit Definitions
//                                                        //
// I/O MAP0,1 START ADDRESS LOW : START ADDRESS Bit 7-0   //
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.09 ,0D Bit Definitions
//                                                        //
// I/O MAP0,1 START ADDRESS HIGH : START ADDRESS Bit 15-8 //
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.0A ,0E Bit Definitions
//                                                        //
// I/O MAP0,1 END ADDRESS LOW : END ADDRESS Bit 7-0       //
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.0B ,0F Bit Definitions
//                                                        //
// I/O MAP0,1 END ADDRESS HIGH : END ADDRESS Bit 15-8     //
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.10,18,20,28,30 Bit Definitions
//                                                        //
// MEMORY MAP0,1,2,3,4 START ADDRESS LOW                  //
//                               : START ADDRESS Bit 19-12//
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.11,19,21,29,31 Bit Definitions
//                                                        //
// MEMORY MAP0,1,2,3,4 START ADDRESS HIGH                 //
//                               : START ADDRESS Bit 23-20//
//--------------------------------------------------------//

#define WINDOW_DATA_SIZE                             0x80 // 

//--------------------------------------------------------//
// REGISTER INDEX No.12,1A,22,2A,32 Bit Definitions
//                                                        //
// MEMORY MAP0,1,2,3,4 END ADDRESS LOW                    //
//                             : END ADDRESS Bit 19-12    //
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.13,1B,23,2B,33 Bit Definitions
//                                                        //
// MEMORY MAP0,1,2,3,4 END ADDRESS HIGH                   //
//                             : END ADDRESS Bit 23-20    //
//--------------------------------------------------------//

#define CARD_TIMER_SELECT_HI                         0x80 // 
#define CARD_TIMER_SELECT_LOW                        0x40 // 

//--------------------------------------------------------//
// REGISTER INDEX No.14,1C,24,2C,34 Bit Definitions
//                                                        //
// MEMORY MAP0,1,2,3,4 ADDRESS OFFSET LOW                 //
//                             : OFFSET ADDRESS Bit 19-12 //
//--------------------------------------------------------//

//--------------------------------------------------------//
// REGISTER INDEX No.15,1D,25,2D,35 Bit Definitions
//                                                        //
// MEMORY MAP0,1,2,3,4 ADDRESS OFFSET HIGH                //
//                             : OFFSET ADDRESS Bit 25-20 //
//--------------------------------------------------------//

#define WINDOW_WRITE_PROTECT                         0x80 // 
#define REG_SETTING                                  0x40 // 

//--------------------------------------------------------//
// REGISTER INDEX No.16 Bit Definitions
//--------------------------------------------------------//

#define INPACK_ENABLE                                0x80 // 
//                                                   0x40 // 
//                                                   0x20 // 
#define SPEAKER_ENABLE                               0x10 // 
#define PULSE_SYSTEM_IRQ                             0x08 // 
#define PLUSE_MANAGEMENT_INTERUPT                    0x04 // 
#define VCC3_3                                       0x02 // 
#define DETECT_5V                                    0x01 // 
#define DLY16INH                                     0x01

//--------------------------------------------------------//
// REGISTER INDEX No.17 Bit Definitions
//--------------------------------------------------------//

#define EMPTY_WRITE_FIFO                             0x80 // 
//                                                   0x40 // 
//                                                   0x20 // 
//                                                   0x10 // 
//                                                   0x08 // 
//                                                   0x04 // 
//                                                   0x02 // 
//                                                   0x01 // 

//--------------------------------------------------------//
// REGISTER INDEX No.1E Bit Definitions
//--------------------------------------------------------//
#define CLRPMIRQ                  0x10
#define IRQPM_EN                  0x08
#define EXWRBK                    0x04

//--------------------------------------------------------//
// REGISTER INDEX No.1F Bit Definitions
//--------------------------------------------------------//

#define CHIP_IDENTIFICATION_HI                       0x80 // 
#define CHIP_IDENTIFICATION_LOW                      0x40 // 
#define DUAL_SIGLE_SLOT                              0x20 // 
#define CL_PD67_REV_LEVEL_BIT2                       0x10 // 
#define CL_PD67_REV_LEVEL_BIT1                       0x08 // 
#define CL_PD67_REV_LEVEL_BIT0                       0x04 // 
#define VS2                                          0x02
#define VS1                                          0x01
#define VOLTAGE_SENSE_B_SHIFT                        2

//--------------------------------------------------------//
// REGISTER INDEX No.26 Bit Definitions
//--------------------------------------------------------//

#define A25_CSEL                                     0x80 // 
#define A24_M_S                                      0x40 // 
#define A23_VU                                       0x20 // 
#define A22                                          0x10 // 
#define A21                                          0x08 // 
//                                                   0x04 // 
#define SPEAKER_IS_LED_INPUT                         0x02 // 
#define ATA_MORD                                     0x01 // 

//--------------------------------------------------------//
// REGISTER INDEX No.2F Bit Definitions
//--------------------------------------------------------//
#define VCC_SELECT_MASK              0x03

//
// Mode Control Register 2
//
#define DIRECT_VOLTAGE_SWITCH_ENABLE       (1<<3)
#define INPUT_ACKNOWLIEDGE_ENABLE          (1<<2)
#define IREQ_SENSE_SELECTION               (1<<1)
#define VOLTAGE_SELECTION                  (1<<0)
#define VSELECT2                           (1<<1)
#define VSELECT1                           (1<<0)
#define VOLTAGE_LIMIT_SHIFT                2
#define VOLTAGE_SELECT_MASK                0x3

//--------------------------------------------------------//
// REGISTER INDEX No.3c Bit Definitions
//--------------------------------------------------------//
#define INTERNAL_VOLTAGE_SENSE     0x4

//--------------------------------------------------------//
// REGISTER INDEX No.3b & 7b Bit Definitions
//--------------------------------------------------------//
#define GP_OUTPUT                  0x30


#endif // __ASM_MIPS_VRC4171_H_
