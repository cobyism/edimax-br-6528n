
#ifndef _TEST_SLV_PCIE_H
#define _TEST_SLV_PCIE_H


//===================================================
#ifdef CONFIG_R8198EP_HOST
#define TEST_SlvPCIe_PROMPT		"<RealTek/HPCIe>"
#else
#define TEST_SlvPCIe_PROMPT		"<RealTek/SPCIe>"
#endif


//===================================================
#ifdef CONFIG_R8198EP_HOST
//Host region
#define PCIE0_RC_CFG_BASE (0xb8b00000)
#define PCIE0_RC_EXT_BASE (0xb8b01000)

#define PCIE0_EP_CFG_BASE (0xb8b10000)
//#define PCIE0_EP_CFG1_BASE (0xb8b11000)  //not use

#define PCIE0_EP_IO_BASE  (0xb8c00000)
#define PCIE0_EP_MEM_BASE (0xb9000000)
//--------------------------------------------------------------------
//RC Configuration layout
#define PCIE0_RC_CAP_BASE (PCIE0_RC_CFG_BASE + 0x40)
#define PCIE0_RC_ECFG_BASE (PCIE0_RC_CFG_BASE + 0x100)
#define PCIE0_RC_PL_BASE (PCIE0_RC_CFG_BASE + 0x700)
//--------------------------------------------------------------------
//RC Extended register
#define PCIE0_MDIO	(PCIE0_RC_EXT_BASE+0x00)
#define PCIE0_ISR	(PCIE0_RC_EXT_BASE+0x04)
#define PCIE0_PWRCR	(PCIE0_RC_EXT_BASE+0x08)
#define PCIE0_IPCFG	(PCIE0_RC_EXT_BASE+0x0c)
#define PCIE0_BISTFAIL	(PCIE0_RC_EXT_BASE+0x10)
#endif
//====================================================
//EP Configuration layout
#ifdef CONFIG_R8198EP_HOST
#define SPE_EP_CFG_BASE  (PCIE0_EP_CFG_BASE)
#define SPE_EP_EXT_BASE  (PCIE0_EP_MEM_BASE)
#define SPE_RC_EXT_SELF_BASE (PCIE0_RC_EXT_BASE)
#else //ifdef CONFIG_R8198EP_DEVICE
#define SPE_EP_CFG_BASE (0xb8b40000)
#define SPE_EP_EXT_BASE (0xb8b41000)
#define SPE_EP_EXT_SELF_BASE (SPE_EP_EXT_BASE+0xC0)
#endif
//--------------------------------------------------------------------
//EP config layout
#define SPE_EP_CAP_BASE (SPE_EP_CFG_BASE + 0x40)  //Capability
#define SPE_EP_ECFG_BASE (SPE_EP_CFG_BASE + 0x100)  //Extended Configuration
#define SPE_EP_PL_BASE (SPE_EP_CFG_BASE + 0x700)  //Port Logic
//====================================================
//DMA Tx/Rx Register
#define SPE_DMA_TXFDP	(SPE_EP_EXT_BASE+0x00)        
#define SPE_DMA_TXCDO	(SPE_EP_EXT_BASE+0x04)        
#define SPE_DMA_RXFDP 	(SPE_EP_EXT_BASE+0x08)
#define SPE_DMA_RXCDO 	(SPE_EP_EXT_BASE+0x0c)

#define SPE_DMA_TXOKCNT (SPE_EP_EXT_BASE+0x10)
#define SPE_DMA_RXOKCNT	(SPE_EP_EXT_BASE+0x14)

#define SPE_DMA_IOCMD 	(SPE_EP_EXT_BASE+0x20)
#define SPE_DMA_IM 	(SPE_EP_EXT_BASE+0x24)
#define SPE_DMA_IMR 	(SPE_EP_EXT_BASE+0x28)
#define SPE_DMA_ISR 	(SPE_EP_EXT_BASE+0x2c)

//---------------------------Max Mem Req Read Size
#ifdef CONFIG_R8198EP_HOST
#define SPE_DMA_SIZE 	(SPE_EP_EXT_BASE+0x30)
#endif

//--------------------------Indirect Share Memory
#define SPE_ISM_LR 	(SPE_EP_EXT_BASE+0x40)
#ifdef CONFIG_R8198EP_HOST
#define SPE_ISM_OR 	(SPE_EP_EXT_BASE+0x44)
#define SPE_ISM_DR 	(SPE_EP_EXT_BASE+0x48)
#else //#ifdef CONFIG_R8198EP_DEVICE
#define SPE_ISM_BAR 	(SPE_EP_EXT_BASE+0x44)
#endif

//--------------------------NFBI
#ifdef CONFIG_R8198EP_HOST
#define SPE_NFBI_CMD 	(SPE_EP_EXT_BASE+0x80)
#define SPE_NFBI_ADDR 	(SPE_EP_EXT_BASE+0x84)
#define SPE_NFBI_DR 	(SPE_EP_EXT_BASE+0x88)
#endif

#define SPE_NFBI_SYSSR 	(SPE_EP_EXT_BASE+0x8c)
#define SPE_NFBI_SYSCR 	(SPE_EP_EXT_BASE+0x90)

//-------------------------Memory
#ifdef CONFIG_R8198EP_HOST
#define SPE_NFBI_DCR 	(SPE_EP_EXT_BASE+0xA4)
#define SPE_NFBI_DTR 	(SPE_EP_EXT_BASE+0xA8)
#define SPE_NFBI_DDCR 	(SPE_EP_EXT_BASE+0xAC)
#define SPE_NFBI_TRXDLY 	(SPE_EP_EXT_BASE+0xB0)
#endif
//---------------------------other self register
#ifdef CONFIG_R8198EP_HOST
#define SPE_MDIO	(SPE_RC_EXT_SELF_BASE+0x00)
#define SPE_PWRCR	(SPE_RC_EXT_SELF_BASE+0x08)
#endif
#ifdef CONFIG_R8198EP_DEVICE
#define SPE_MDIO	 (SPE_EP_EXT_SELF_BASE+0x00)     //(SPE_EP_EXT_BASE+0xc0)
#define SPE_PWRCR	 (SPE_EP_EXT_SELF_BASE+0x08)      //(SPE_EP_EXT_BASE+0xc8)
#define SPE_BIST_FAILCHK1	(SPE_EP_EXT_SELF_BASE+0x10)                    //(SPE_EP_EXT_BASE+0xd0)
#define SPE_BIST_FAILCHK2	(SPE_EP_EXT_SELF_BASE+0x14)               //(SPE_EP_EXT_BASE+0xd4)
#define SPE_SRAMEMA             (SPE_EP_EXT_SELF_BASE+0x20) 	                   //(SPE_EP_EXT_BASE+0xe0)
#endif

//====================================================
//----------------------------------IOCMD 
#define SPE_DMA_IOCMD_SWAP (1<<4)
#define SPE_DMA_IOCMD_SWINT (1<<3)
#define SPE_DMA_IOCMD_RXEN (1<<2)
#define SPE_DMA_IOCMD_TXPOLL (1<<1)
#define SPE_DMA_IOCMD_RST (1<<0)


//----------------------------------IMR
#define SPE_DMA_IMR_NEEDBTCODE (1<<31)
#define SPE_DMA_IMR_TXLEN0 (1<<24)
#define SPE_DMA_IMR_SWINT (1<<23)
#define SPE_DMA_IMR_TXDU (1<<22)
#define SPE_DMA_IMR_TXERR (1<<21)
#define SPE_DMA_IMR_TXOK (1<<20)
#define SPE_DMA_IMR_TXTMR (1<<19)
#define SPE_DMA_IMR_RXDU (1<<18)
#define SPE_DMA_IMR_RXERR (1<<17)
#define SPE_DMA_IMR_RXOK (1<<17)

#define SPE_DMA_IMR_DMAALL (SPE_DMA_IMR_SWINT |	SPE_DMA_IMR_TXDU |SPE_DMA_IMR_TXERR| SPE_DMA_IMR_TXOK |SPE_DMA_IMR_TXTMR | \
											SPE_DMA_IMR_RXDU |SPE_DMA_IMR_RXERR| SPE_DMA_IMR_RXOK)
//------------------------------------ISR
#define SPE_DMA_ISR_NEEDBTCODE (1<<31)
#define SPE_DMA_ISR_TXLEN0 (1<<24)
#define SPE_DMA_ISR_SWINT (1<<23)
#define SPE_DMA_ISR_TXDU (1<<22)
#define SPE_DMA_ISR_TXERR (1<<21)
#define SPE_DMA_ISR_TXOK (1<<20)
#define SPE_DMA_ISR_TXTMR (1<<19)
#define SPE_DMA_ISR_RXDU (1<<18)
#define SPE_DMA_ISR_RXERR (1<<17)
#define SPE_DMA_ISR_RXOK (1<<16)
#ifdef CONFIG_R8198EP_HOST
#define SPE_DMA_ISR_FWUSE15 (1<<15)
#define SPE_DMA_ISR_FWUSE14 (1<<14)
#define SPE_DMA_ISR_FWUSE13 (1<<13)
#define SPE_DMA_ISR_FWUSE12 (1<<12)
#define SPE_DMA_ISR_FWUSE11 (1<<11)
#define SPE_DMA_ISR_FWUSE10 (1<<10)
#define SPE_DMA_ISR_FWUSE09 (1<<09)
#define SPE_DMA_ISR_FWUSE08 (1<<08)
#define SPE_DMA_ISR_FWUSE07 (1<<07)
#define SPE_DMA_ISR_FWUSE06 (1<<06)
#define SPE_DMA_ISR_FWUSE05 (1<<05)
#define SPE_DMA_ISR_FWUSE04 (1<<04)
#define SPE_DMA_ISR_FWUSE03 (1<<03)
#define SPE_DMA_ISR_FWUSE02 (1<<02)
#define SPE_DMA_ISR_FWUSE01 (1<<01)
#define SPE_DMA_ISR_FWUSE00 (1<<00)

#define SPE_DMA_ISR_FWUSEALL (0xffff)

#else  //ifdef CONFIG_R8198EP_DEVICE
#define SPE_DMA_ISR_CUSTUSE15 (1<<15)
#define SPE_DMA_ISR_CUSTUSE14 (1<<14)
#define SPE_DMA_ISR_CUSTUSE13 (1<<13)
#define SPE_DMA_ISR_CUSTUSE12 (1<<12)
#define SPE_DMA_ISR_CUSTUSE11 (1<<11)
#define SPE_DMA_ISR_CUSTUSE10 (1<<10)
#define SPE_DMA_ISR_CUSTUSE09 (1<<09)
#define SPE_DMA_ISR_CUSTUSE08 (1<<08)
#define SPE_DMA_ISR_CUSTUSE07 (1<<07)
#define SPE_DMA_ISR_CUSTUSE06 (1<<06)
#define SPE_DMA_ISR_CUSTUSE05 (1<<05)
#define SPE_DMA_ISR_CUSTUSE04 (1<<04)
#define SPE_DMA_ISR_CUSTUSE03 (1<<03)
#define SPE_DMA_ISR_CUSTUSE02 (1<<02)
#define SPE_DMA_ISR_CUSTUSE01 (1<<01)
#define SPE_DMA_ISR_CUSTUSE00 (1<<00)

#define SPE_DMA_ISR_CUSTUSEALL (0xffff)
#endif

#define SPE_DMA_ISR_DMAALL (SPE_DMA_ISR_SWINT |	SPE_DMA_ISR_TXDU |SPE_DMA_ISR_TXERR| SPE_DMA_ISR_TXOK |SPE_DMA_ISR_TXTMR | \
				SPE_DMA_ISR_RXDU |SPE_DMA_ISR_RXERR| SPE_DMA_ISR_RXOK)

#ifdef CONFIG_R8198EP_HOST
//------------------------------------------NFBI_CMD
#define NFBI_CMD_DRAMTYPE_DDR (1<<3)
#define NFBI_CMD_SWAP (1<<2)
//------------------------------------------NFBI SYSSR
#define SPE_NFBI_FWUSR15 (1<<15)
#define SPE_NFBI_FWUSR14 (1<<14)
#define SPE_NFBI_FWUSR13 (1<<13)
#define SPE_NFBI_FWUSR12 (1<<12)
#define SPE_NFBI_FWUSR11 (1<<11)
#define SPE_NFBI_FWUSR10 (1<<10)
#define SPE_NFBI_FWUSR09 (1<<09)
#define SPE_NFBI_FWUSR08 (1<<08)
#define SPE_NFBI_FWUSR07 (1<<07)
#define SPE_NFBI_FWUSR06 (1<<06)
#define SPE_NFBI_FWUSR05 (1<<05)
#define SPE_NFBI_FWUSR04 (1<<04)
#define SPE_NFBI_FWUSR03 (1<<03)
#define SPE_NFBI_FWUSR02 (1<<02)
#define SPE_NFBI_FWUSR01 (1<<01)
#define SPE_NFBI_FWUSR00 (1<<00)
//------------------------------------------NFBI SYSCR
#endif
//====================================================
//MDIO
#define SLVPE_MDIO_DATA_OFFSET (16)
#define SLVPE_MDIO_DATA_MASK (0xffff <<SLVPE_MDIO_DATA_OFFSET)
#define SLVPE_MDIO_REG_OFFSET (8)
#define SLVPE_MDIO_RDWR_OFFSET (0)



#ifdef CONFIG_R8198EP_DEVICE
//PWRCR
#define SPE_PWRCR_ENTERL1 	(1<<1)
#define SPE_PWRCR_EXITL1 	(1<<2)
#define SPE_PWRCR_ENTERL23 (1<<3)
#define SPE_PWRCR_AUXPWRDET (1<<4)
#define SPE_PWRCR_LOWPWREN (1<<5)
#define SPE_PWRCR_CLKREQ (1<<6)
#endif
//==================================================================
unsigned char  PCIE_MEM8_READ(unsigned int addr)
{
	return REG8(addr);
}

void  PCIE_MEM8_WRITE(unsigned int addr, unsigned char val)
{
	REG8(addr)=val;
}
//------------------------------------------------------------------
unsigned char  PCIE_MEM16_READ(unsigned int addr)
{
	return REG16(addr);
}

void  PCIE_MEM16_WRITE(unsigned int addr, unsigned char val)
{
	REG16(addr)=val;
}
//------------------------------------------------------------------
unsigned int  PCIE_MEM32_READ(unsigned int addr)
{
	return REG32(addr);
}

void  PCIE_MEM32_WRITE(unsigned int addr, unsigned int val)
{
	REG32(addr)=val;
}


//------------------------------------------------------------------
inline unsigned int  EP_REG32_READ(unsigned int addr)
{
	return REG32(addr);
}

inline void  EP_REG32_WRITE(unsigned int addr, unsigned int val)
{
	REG32(addr)=val;
}
//====================================================


//--------------------------------------------------------------------------
//Descriptor Ring
#define SPE_DESC_OWN_NIC (1<<31)
#define SPE_DESC_OWN_CPU (0<<31)

#define SPE_DESC_EOR (1<<30)
#define SPE_DESC_FS (1<<29)
#define SPE_DESC_LS (1<<28)


#define SPE_DESC_DROP_OFFSET (29)
#define SPE_DESC_DROP_MASK (1<<29)


#define SPE_DESC_INFO1_OFFSET (16)
#define SPE_DESC_INFO1_MASK (0x0fff0000)

#define SPE_DESC_BUFFLEN_OFFSET (0)
#define SPE_DESC_BUFFLEN_MASK (0x0000ffff)
//------------------------------------


//-----------------------------------

#define TX_DESC_NUM	8  
#define RX_DESC_NUM	8
#define BUF_SIZE	 ((1<<15))	// Byte Counts

unsigned char DMA_buff[BUF_SIZE];
unsigned int DMA_len;
#ifdef CONFIG_R8198EP_HOST
unsigned char DMA_buff2[BUF_SIZE];
unsigned int DMA_len2;
#endif
#ifdef CONFIG_R8198EP_DEVICE
unsigned int ISM_len=BUF_SIZE;
unsigned char ISM_buff[BUF_SIZE];
#endif
//------------------------------------------
//buffer
unsigned char SlvPE_tx_buf[TX_DESC_NUM][BUF_SIZE];
unsigned char SlvPE_rx_buf[RX_DESC_NUM][BUF_SIZE];


struct slvpcie_buff_t
{	unsigned char buff[BUF_SIZE];  
};

struct slvpcie_buff_t *pTxBuffPtr[TX_DESC_NUM];
struct slvpcie_buff_t *pRxBuffPtr[RX_DESC_NUM];


//------------------------------------------
//desc
volatile struct slvpcie_desc_t
{
	unsigned int flag;
	unsigned int DataPtr;
	unsigned int info2;
	unsigned int info3;		
};


unsigned char SlvPE_tx_desc[TX_DESC_NUM*sizeof(struct slvpcie_desc_t)+4];  //dont direct use this array, because alignment issue
unsigned char SlvPE_rx_desc[RX_DESC_NUM*sizeof(struct slvpcie_desc_t)+4];


volatile struct slvpcie_desc_t *pTxDescPtr;
volatile struct slvpcie_desc_t *pRxDescPtr;

//------------------------------------------
volatile unsigned int	SPE_txhead = 0;	// index for system to release buffer
volatile unsigned int	SPE_txtail = 0;	// index for system to set buf to BD
volatile unsigned int	SPE_rxtail = 0;	// index for system to set buf to BD

unsigned int SPE_IntNum =0;
unsigned int SPE_IntFlag =0;
unsigned int SPE_TxOKNum =0;
unsigned int SPE_RxOKNum =0;



//-----------------------------------------------------------------------
//IRQ=> IRR ::  [31-24] => IRR3,  [23-16] => IRR2, [15-8] => IRR1, [7-0] => IRR0 
//IRR= 3-(IRQ /8)

//  bit7 => [31-28] , bit6=>[27-24], bit5=>[23-20], bit4=>[19-16], bit3=>[15-12], bit2=>[11-8], bit1=>[7-4], bit0=>[3-0], 
//IRR_OFFSET= (IRQ-IRR*8)*4

#ifdef CONFIG_R8198EP_HOST
#define SPE_IRQ_NO 21 //Host PCIE0 bit 21
#else
#define SPE_IRQ_NO 22 
#endif
#define SPE_IRR_NO (SPE_IRQ_NO/8)  // 2
#define SPE_IRR_OFFSET ((SPE_IRQ_NO-SPE_IRR_NO*8)*4)   //bit 6 [27-24]

#ifdef CONFIG_R8198EP_DEVICE
#define GPIO_IRQ_NO 16  //PA0
#define GPIO_IRR_NO (GPIO_IRQ_NO/8)  
#define GPIO_IRR_OFFSET ((GPIO_IRQ_NO-GPIO_IRR_NO*8)*4)   
#endif

#ifdef CONFIG_R8198EP_HOST
//=================================================================
//ew 0xb8001004 = 0x54880000 // DCR ,SDR=64MB-Micron
//ew 0xb8001004 = 0x54480000 // DCR ,SDR=32MB
//ew 0xb8001004 = 0x52480000 // DCR ,SDR=16MB
//ew 0xb8001004 = 0x52080000 // DCR ,SDR=8MB


//DTR 0x6CEA0A80 , come from jason tune-performance
//DTR 0xffff05c0  //default safed value
#define DRAM_CONFIG_VAL 0x54480000  //(32M)  //0x52080000 //(8M)     //default: 0x58080000 
#define DRAM_TIMING_VAL 0xffff05c0  //default:  FFFF0FC0
#define DRAM_TRXDLY_VAL ((0xf<<5) | (0))

#define DRAM_CONFIG_VALH (DRAM_CONFIG_VAL>>16)

#define DRAM_CONFIG_VALL (DRAM_CONFIG_VAL&0x0000ffff)
#define DRAM_TIMING_VALH (DRAM_TIMING_VAL>>16)
#define DRAM_TIMING_VALL (DRAM_TIMING_VAL&0x0000ffff)

#define NFBI_BOOTADDR 0x00008000
#define NFBI_KERNADDR 0x00700000


//==============================================
#endif
//Customer ISM
#define SPE_ISM_MAGNUMI (('w'<<24)|('e' <<16) | ('i'<<8))
#define SPE_ISM_MAGNUMO (('w'<<24)|('e' <<16) | ('o'<<8))
#define SPE_ISM_MAG_REGR 0x01
#define SPE_ISM_MAG_REGW 0x02


#endif


