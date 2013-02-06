// RTL8168 NIC Support, Copied from Linux's r8168.h
#ifndef _8168n_H
#define _8168n_H



// PCIE RC1
#define PCIE1_RC_CFG  0xB8B00000
#define PCIE1_EP_CFG  0xB8B10000
#define PCIE1_EP_MEM  0xB9030000
#define PCIE1_EP_IO   0xB8C00000

#define TX1_DESC_ADDR 0xA0220000
#define RX1_DESC_ADDR 0xA0230000

#define TX1_BUFF_ADDR 0xA0420000
//#define TX1_BUFF_ADDR 0xA003c500
#define RX1_BUFF_ADDR 0xA0630000

#define TX1_DESC_SIZE 16
#define RX1_DESC_SIZE 16

#define TX1_BUFF_SIZE 4096
#define RX1_BUFF_SIZE 4096

// PCIE RC2
#define PCIE2_RC_CFG  0xB8B20000
#define PCIE2_EP_CFG  0xB8B30000
#define PCIE2_EP_MEM  0xBA030000
#define PCIE2_EP_IO   0xB8E00000

#define TX2_DESC_ADDR 0xA0320000
#define RX2_DESC_ADDR 0xA0330000

#define TX2_BUFF_ADDR 0xA0820000
#define RX2_BUFF_ADDR 0xA0A30000

#define TX2_DESC_SIZE 16
#define RX2_DESC_SIZE 16

#define TX2_BUFF_SIZE 4096
#define RX2_BUFF_SIZE 4096

#define MIN(i, j)                ((i) < (j) ? (i) : (j))
#define MAX(i, j)                ((i) > (j) ? (i) : (j))

#define PADDR(i)                 ((i) & 0x1FFFFFFF)





void rtl8168_init(int no);
void rtl8168_tx(unsigned char *content, unsigned int size, int no);
int  rtl8168_rx(unsigned char *content, unsigned int size, int no);
void rtl8168_tx_trigger(int no);

#define NUM_TX_DESC	64
#define NUM_RX_DESC	64

#define MAX_READ_REQSIZE_128B    0x00
#define MAX_READ_REQSIZE_256B    0x10
#define MAX_READ_REQSIZE_512B    0x20
#define MAX_READ_REQSIZE_1KB     0x30
#define MAX_READ_REQSIZE_2KB     0x40
#define MAX_READ_REQSIZE_4KB     0x50

#define MAX_PAYLOAD_SIZE_128B    0x00
#define MAX_PAYLOAD_SIZE_256B    0x20
#define MAX_PAYLOAD_SIZE_512B    0x40
#define MAX_PAYLOAD_SIZE_1KB     0x60
#define MAX_PAYLOAD_SIZE_2KB     0x80
#define MAX_PAYLOAD_SIZE_4KB     0xA0


enum RTL8168_registers {
	MAC0 = 0,		/* Ethernet hardware address. */
	MAC4 = 0x04,
	MAR0 = 8,		/* Multicast filter. */
	CounterAddrLow = 0x10,
	CounterAddrHigh = 0x14,
	TxDescStartAddrLow = 0x20,
	TxDescStartAddrHigh = 0x24,
	TxHDescStartAddrLow = 0x28,
	TxHDescStartAddrHigh = 0x2c,
	FLASH = 0x30,
	ERSR = 0x36,
	ChipCmd = 0x37,
	TxPoll = 0x38,
	IntrMask = 0x3C,
	IntrStatus = 0x3E,
	TxConfig = 0x40,
	RxConfig = 0x44,
	TCTR = 0x48,
	Cfg9346 = 0x50,
	Config0 = 0x51,
	Config1 = 0x52,
	Config2 = 0x53,
	Config3 = 0x54,
	Config4 = 0x55,
	Config5 = 0x56,
	TimeIntr = 0x58,
	PHYAR = 0x60,
	CSIDR = 0x64,
	CSIAR = 0x68,
	PHYstatus = 0x6C,
	EPHYAR = 0x80,
	DBG_reg = 0xD1,
	RxMaxSize = 0xDA,
	CPlusCmd = 0xE0,
	IntrMitigate = 0xE2,
	RxDescAddrLow = 0xE4,
	RxDescAddrHigh = 0xE8,
	EarlyTxThres = 0xEC,
	FuncEvent = 0xF0,
	FuncEventMask = 0xF4,
	FuncPresetState = 0xF8,
	FuncForceEvent = 0xFC,
};

enum RTL8168_register_content {
	/* InterruptStatusBits */
	SYSErr		= 0x8000,
	PCSTimeout	= 0x4000,
	SWInt		= 0x0100,
	TxDescUnavail	= 0x0080,
	RxFIFOOver	= 0x0040,
	LinkChg		= 0x0020,
	RxDescUnavail	= 0x0010,
	TxErr		= 0x0008,
	TxOK		= 0x0004,
	RxErr		= 0x0002,
	RxOK		= 0x0001,

	/* RxStatusDesc */
	RxRWT = (1 << 22),
	RxRES = (1 << 21),
	RxRUNT = (1 << 20),
	RxCRC = (1 << 19),

	/* ChipCmdBits */
	StopReq  = 0x80,
	CmdReset = 0x10,
	CmdRxEnb = 0x08,
	CmdTxEnb = 0x04,
	RxBufEmpty = 0x01,

	/* Cfg9346Bits */
	Cfg9346_Lock = 0x00,
	Cfg9346_Unlock = 0xC0,

	/* rx_mode_bits */
	AcceptErr = 0x20,
	AcceptRunt = 0x10,
	AcceptBroadcast = 0x08,
	AcceptMulticast = 0x04,
	AcceptMyPhys = 0x02,
	AcceptAllPhys = 0x01,

	/* Transmit Priority Polling*/
	HPQ = 0x80,
	NPQ = 0x40,
	FSWInt = 0x01,

	/* RxConfigBits */
	RxCfgFIFOShift = 13,
	RxCfgDMAShift = 8,

   /* ghhuang */
   RxDMA16B  = 0x000,
   RxDMA32B  = 0x100,
   RxDMA64B  = 0x200,
   RxDMA128B = 0x300,
   RxDMA256B = 0x400,
   RxDMA512B = 0x500,
   RxDMA1KB  = 0x600,
   RxDMAall  = 0x700,

	/* TxConfigBits */
	TxInterFrameGapShift = 24,
	TxDMAShift = 8,	/* DMA burst value (0-7) is shift this many bits */
	TxMACLoopBack = (1 << 17),	/* MAC loopback */

   TxDMA16B  = 0x000,
   TxDMA32B  = 0x100,
   TxDMA64B  = 0x200,
   TxDMA128B = 0x300,
   TxDMA256B = 0x400,
   TxDMA512B = 0x500,
   TxDMA1KB  = 0x600,
   TxDMAall  = 0x700,

	/* Config1 register p.24 */
	LEDS1		= (1 << 7),
	LEDS0		= (1 << 6),
	Speed_down	= (1 << 4),
	MEMMAP		= (1 << 3),
	IOMAP		= (1 << 2),
	VPD		= (1 << 1),
	PMEnable	= (1 << 0),	/* Power Management Enable */

	/* Config3 register */
	MagicPacket	= (1 << 5),	/* Wake up when receives a Magic Packet */
	LinkUp		= (1 << 4),	/* This bit is reserved in RTL8168B.*/
					/* Wake up when the cable connection is re-established */
	ECRCEN		= (1 << 3),	/* This bit is reserved in RTL8168B*/
	Jumbo_En	= (1 << 2),	/* This bit is reserved in RTL8168B*/
	RDY_TO_L23	= (1 << 1),	/* This bit is reserved in RTL8168B*/
	Beacon_en	= (1 << 0),	/* This bit is reserved in RTL8168B*/

	/* Config4 register */
	LANWake		= (1 << 1),	/* This bit is reserved in RTL8168B*/

	/* Config5 register */
	BWF		= (1 << 6),	/* Accept Broadcast wakeup frame */
	MWF		= (1 << 5),	/* Accept Multicast wakeup frame */
	UWF		= (1 << 4),	/* Accept Unicast wakeup frame */
	LanWake		= (1 << 1),	/* LanWake enable/disable */
	PMEStatus	= (1 << 0),	/* PME status can be reset by PCI RST# */

	/* CPlusCmd */
	EnableBist	= (1 << 15),
	Macdbgo_oe	= (1 << 14),
	Normal_mode	= (1 << 13),
	Force_halpdup	= (1 << 12),
	Force_rxflow_en	= (1 << 11),
	Force_txflow_en	= (1 << 10),
	Cxpl_dbg_sel	= (1 << 9),//This bit is reserved in RTL8168B
	ASF		= (1 << 8),//This bit is reserved in RTL8168C
	PktCntrDisable	= (1 << 7),
	RxVlan		= (1 << 6),
	RxChkSum	= (1 << 5),
	Macdbgo_sel	= 0x001C,
	INTT_0		= 0x0000,
	INTT_1		= 0x0001,
	INTT_2		= 0x0002,
	INTT_3		= 0x0003,
	
	/* rtl8168_PHYstatus */
	TxFlowCtrl = 0x40,
	RxFlowCtrl = 0x20,
	//_1000bpsF = 0x10,
	_100bps = 0x08,
	_10bps = 0x04,
	LinkStatus = 0x02,
	FullDup = 0x01,

	/* DBG_reg */
	Fix_Nak_1 = (1 << 4),
	Fix_Nak_2 = (1 << 3),
	DBGPIN_E2 = (1 << 0),

	/* DumpCounterCommand */
	CounterDump = 0x8,

	/* PHY access */
	PHYAR_Flag = 0x80000000,
	PHYAR_Write = 0x80000000,
	PHYAR_Read = 0x00000000,
	PHYAR_Reg_Mask = 0x1f,
	PHYAR_Reg_shift = 16,
	PHYAR_Data_Mask = 0xffff,

	/* EPHY access */
	EPHYAR_Flag = 0x80000000,
	EPHYAR_Write = 0x80000000,
	EPHYAR_Read = 0x00000000,
	EPHYAR_Reg_Mask = 0x1f,
	EPHYAR_Reg_shift = 16,
	EPHYAR_Data_Mask = 0xffff,

	/* CSI access */	
	CSIAR_Flag = 0x80000000,
	CSIAR_Write = 0x80000000,
	CSIAR_Read = 0x00000000,
	CSIAR_ByteEn = 0x0f,
	CSIAR_ByteEn_shift = 12,
	CSIAR_Addr_Mask = 0x0fff,
};

enum _DescStatusBit {
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */

	/* Tx private */
	/*------ offset 0 of tx descriptor ------*/
	LargeSend	= (1 << 27), /* TCP Large Send Offload (TSO) */
	MSSShift	= 16,        /* MSS value position */
	MSSMask		= 0xfff,     /* MSS value + LargeSend bit: 12 bits */
	TxIPCS		= (1 << 18), /* Calculate IP checksum */
	TxUDPCS		= (1 << 17), /* Calculate UDP/IP checksum */
	TxTCPCS		= (1 << 16), /* Calculate TCP/IP checksum */
	TxVlanTag	= (1 << 17), /* Add VLAN tag */

	/*@@@@@@ offset 4 of tx descriptor => bits for RTL8168C/CP only		begin @@@@@@*/
	TxUDPCS_C	= (1 << 31), /* Calculate UDP/IP checksum */
	TxTCPCS_C	= (1 << 30), /* Calculate TCP/IP checksum */
	TxIPCS_C	= (1 << 29), /* Calculate IP checksum */
	/*@@@@@@ offset 4 of tx descriptor => bits for RTL8168C/CP only		end @@@@@@*/


	/* Rx private */
	/*------ offset 0 of rx descriptor ------*/
	PID1		= (1 << 18), /* Protocol ID bit 1/2 */
	PID0		= (1 << 17), /* Protocol ID bit 2/2 */

#define RxProtoUDP	(PID1)
#define RxProtoTCP	(PID0)
#define RxProtoIP	(PID1 | PID0)
#define RxProtoMask	RxProtoIP

	RxIPF		= (1 << 16), /* IP checksum failed */
	RxUDPF		= (1 << 15), /* UDP/IP checksum failed */
	RxTCPF		= (1 << 14), /* TCP/IP checksum failed */
	RxVlanTag	= (1 << 16), /* VLAN tag available */

	/*@@@@@@ offset 0 of rx descriptor => bits for RTL8168C/CP only		begin @@@@@@*/
	RxUDPT		= (1 << 18),
	RxTCPT		= (1 << 17),
	/*@@@@@@ offset 0 of rx descriptor => bits for RTL8168C/CP only		end @@@@@@*/

	/*@@@@@@ offset 4 of rx descriptor => bits for RTL8168C/CP only		begin @@@@@@*/
	RxV6F		= (1 << 31),
	RxV4F		= (1 << 30),
	/*@@@@@@ offset 4 of rx descriptor => bits for RTL8168C/CP only		end @@@@@@*/
};

//==============================================================
/*   System Clock Configuration   */
#if 1
	#define ASIC_8196 1
#else
	#define FPGA_8196 1
#endif	

/*    PCIE Testing Configuration   */
	#define PCIE_Test_With_8102E 0  //"0 "=8111C /8111D  ,"1" =8102E
	#define PCIE_RESET_PROCEDURE 1
	#define PCIE_External_Loopback_test 1

	/*JSW:20090205:P1 will be tested by recognize BOND_OPTION_REG(0XB8000030),but still set it to 2  */
	//#define PCIE_Port0and1_8196B_208pin 0

	/*JSW:20090507 : Forced Enable PCIE_P1 test for 8196_208pin ,"0":Don't test PCIE PCIE_P1 , "1":Both PCIE_P0 and PCIE_P1*/
	#define TEST_8196_PCIE_P1 0

	#define Test_with_GPIO_C1_trigger 0       //Add for 8102 FT1 test 
	#define READ_MINI_PCIE 0


/*    Set Shared pin to LED Scan mode   */
	
	#define SHARED_PIN_LED_MODE 1


/*    More HW Testing Configuration   */
	#define CONFIG_TEST_HW 1
	#define ENABLE_DRAM_SELF_TEST 0   //JSW:20090331 : For Alpha


//End of RTL8196B_ASIC_FT2 setting
//===============================================


	//Chip ID Code
	#define ChipIDCode_RTL8196B_128pin 0x2A   //
	#define ChipIDCode_RTL8196BU_128pin 0x3B   //JSW:20090710
	#define ChipIDCode_RTL8196_208pin 0x78     //
	#define ChipIDCode_RTL8197B_208pin 0x39   //

	//Chip ID Code (VoIP)	
	#define ChipIDCode_RTL8972B_208pin     0x30   
	#define ChipIDCode_RTL8982B_208pin     0x31
	#define ChipIDCode_RTL8984B_208pin     0x33
	#define ChipIDCode_RTL8981B_208pin     0x32 	 //JSW:20090508     	
	

	//For system register
	#define PLL_REG 0xb8000020
	#define HW_STRAP_REG 0xb8000008
	#define PIN_MUX_SEL 0xb8000030
	#define Bond_Option_REG 0xb800000c





	
#endif


