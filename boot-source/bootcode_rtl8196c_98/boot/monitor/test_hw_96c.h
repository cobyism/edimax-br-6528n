
/* ---------------------------------
	Test 8196C hardware
-----------------------------------*/
#ifndef _HW_TEST_H
#define _HW_TEST_H


#define SYS_BASE 0xb8000000
#define SYS_INT_STATUS (SYS_BASE +0x04)
#define SYS_HW_STRAP   (SYS_BASE +0x08)
#define SYS_BOND_OPTION   (SYS_BASE +0x0C)
#define SYS_CLKMANAGE (SYS_BASE +0x10)
#define SYS_BIST_CTRL (SYS_BASE +0x14)
#define SYS_CPU_EMA_CTRL (SYS_BASE +0x18)
#define SYS_BIST_DONE (SYS_BASE +0x20)
#define SYS_BIST_FAIL (SYS_BASE +0x24)
#define SYS_DRF_BIST_DONE (SYS_BASE +0x28)
#define SYS_DRF_BIST_FAIL (SYS_BASE +0x2C)
#define SYS_PLL (SYS_BASE +0x30)
#define SYS_DBG_SEL (SYS_BASE +0x3C)
#define SYS_PIN_MUX_SEL (SYS_BASE +0x40)
#define SYS_PAD_CTRL (SYS_BASE +0x48)
#define SYS_PCIE_PHY0 (SYS_BASE +0x50)

//hw strap
#define ST_CLKLX_FROM_CLKM_OFFSET 7
#define ST_SYNC_OCP_OFFSET 9
#define CK_M2X_FREQ_SEL_OFFSET 10
#define ST_CPU_FREQ_SEL_OFFSET 13
#define ST_CPU_FREQDIV_SEL_OFFSET 19

//#define ST_BOOTPINSEL (1<<0)
//#define ST_DRAMTYPE (1<<1)
#define ST_BOOTSEL (1<<2)
//#define ST_PHYID (0x3<<3) //2'b11 
#define ST_CLKLX_FROM_CLKM (1<<7)   //new, 8196C new 
#define ST_EN_EXT_RST (1<<8)
#define ST_SYNC_OCP (1<<9)   //ocp clock is come from clock lx
#define CK_M2X_FREQ_SEL (0x7 <<10)
#define ST_CPU_FREQ_SEL (0xf<<13)
#define ST_NRFRST_TYPE (1<<17)
//#define ST_SYNC_LX (1<<18)
#define ST_CPU_FREQDIV_SEL (0x1<<19)  //8196C, change to only one-bit
#define ST_SWAP_DBG_HALFWORD (0x1<<22) 
#define ST_EVER_REBOOT_ONCE (1<<23)
#define ST_SYS_DBG_SEL  (0x3f<<24)
#define ST_PINBUS_DBG_SEL (3<<30)

// clock manager
#define ACTIVE_GDMA (1<<8)
#define ACTIVE_SWCORE (1<<9)
#define ACTIVE_STI (1<<10)
#define ACTIVE_PCIE (1<<12)

#define ACTIVE_DEFAULT  (ACTIVE_GDMA|ACTIVE_SWCORE |ACTIVE_STI |ACTIVE_PCIE)  //(0x00FFFFD6)//(0x00FFFFD8)

//=================================================================
#define SPRS 0xb8000040 // Share Pin Register, pin mux


//============================================================================


//===========================================================================

//============================================================================
//HOST PCIE
#define PCIE0_RC_EXT_BASE (0xb8b01000)
//RC Extended register
#define PCIE0_MDIO	(PCIE0_RC_EXT_BASE+0x00)
//MDIO
#define PCIE_MDIO_DATA_OFFSET (16)
#define PCIE_MDIO_DATA_MASK (0xffff <<PCIE_MDIO_DATA_OFFSET)
#define PCIE_MDIO_REG_OFFSET (8)
#define PCIE_MDIO_RDWR_OFFSET (0)



//============================================================================

/*
#define SPEED_IRQ_NO 29
#define SPEED_IRR_NO 3
#define SPEED_IRR_OFFSET 20
*/
#define SPEED_IRQ_NO 13  //PA0
#define SPEED_IRR_NO (SPEED_IRQ_NO/8)   //IRR1
#define SPEED_IRR_OFFSET ((SPEED_IRQ_NO-SPEED_IRR_NO*8)*4)   //20
#endif
