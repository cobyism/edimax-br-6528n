/*
 * RTL8196B PCIE Host Controller Glue Driver
 * Author: ghhuang@realtek.com.tw
 *
 * Notes:
 * - Two host controllers available.
 * - Each host direcly connects to one device
 * - Supports PCI devices through PCIE-to-PCI bridges
 * - If no PCI devices are connected to RC. Timeout monitor shall be 
 *   enabled to prevent bus hanging.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/rlxregs.h>
#include "bspchip.h"

#define PCI_8BIT_ACCESS    1
#define PCI_16BIT_ACCESS   2
#define PCI_32BIT_ACCESS   4
#define PCI_ACCESS_READ    8
#define PCI_ACCESS_WRITE   16

#define MAX_NUM_DEV  4

#define DEBUG_PRINTK 0
//#define PIN_208

static int pci0_bus_number = 0xff;
static int pci1_bus_number = 0xff;

static struct resource rtl8196b_pci0_io_resource = {
   .name   = "RTL8196B PCI0 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(BSP_PCIE0_D_IO),
   .end    = PADDR(BSP_PCIE0_D_IO + 0x1FFFFF)
};

static struct resource rtl8196b_pci0_mem_resource = {
   .name   = "RTL8196B PCI0 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(BSP_PCIE0_D_MEM),
   .end    = PADDR(BSP_PCIE0_D_MEM + 0xFFFFFF)
};

#ifdef PIN_208
static struct resource rtl8196b_pci1_io_resource = {
   .name   = "RTL8196B PCI1 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(BSP_PCIE1_D_IO),
   .end    = PADDR(BSP_PCIE1_D_IO + 0x1FFFFF)
};

static struct resource rtl8196b_pci1_mem_resource = {
   .name   = "RTL8196B PCI1 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(BSP_PCIE1_D_MEM),
   .end    = PADDR(BSP_PCIE1_D_MEM + 0xFFFFFF)
};
#endif


 
 
//HOST PCIE
#define PCIE0_RC_EXT_BASE (0xb8b01000)
//RC Extended register
#define PCIE0_MDIO (PCIE0_RC_EXT_BASE+0x00)
//MDIO
#define PCIE_MDIO_DATA_OFFSET (16)
#define PCIE_MDIO_DATA_MASK (0xffff <<PCIE_MDIO_DATA_OFFSET)
#define PCIE_MDIO_REG_OFFSET (8)
#define PCIE_MDIO_RDWR_OFFSET (0)
 

void HostPCIe_SetPhyMdioWrite(unsigned int regaddr, unsigned short val)
{
 REG32(PCIE0_MDIO)= ( (regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET) | ((val&0xffff)<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ; 
 //delay 
mdelay(1);//mdelay(10);
}

void PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset)
{
        #define SYS_PCIE_PHY0   (0xb8000000 +0x50)
 //PCIE Register
  #define CLK_MANAGE  0xb8000010
 
  #define PCIE_PHY0_REG  0xb8b01000
  //#define PCIE_PHY1_REG  0xb8b21000
  #define PCIE_PHY0  0xb8b01008
 // #define PCIE_PHY1  0xb8b21008
 

  
//JSW:PCIE reset procedure
#if 0
        //1. "Enable internal PCIE PLL
//#if Use_External_PCIE_CLK
if(Use_External_PCIE_CLK)
    REG32(PIE_PLL) = 0x358;                        //Use External PCIE CLK (clock chip)
//#else  
else
    REG32(PCIE_PLL) = 0x9;                          //Use Internal PCIE CLK and PCIE fine-tune
//#endif
 

//prom_printf("\nPCIE_PLL(0x%x)=0x%x\n",PCIE_PLL,READ_MEM32(PCIE_PLL));
    mdelay(100);//mdelay(10);
#endif
 
        //2.Active LX & PCIE Clock
    REG32(CLK_MANAGE) |=  (1<<11);        //enable active_pcie0
    mdelay(100);
 
#if 1
 if(mdio_reset)
 {
  printk("Do MDIO_RESET\n");
        // 3.MDIO Reset  
     REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,          
     REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1,   
     REG32(SYS_PCIE_PHY0) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
 } 
        //4. PCIE PHY Reset       
    REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
    REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
      mdelay(100);
   
#endif
 
        mdelay(100);    
 
#if 0
 //#if Use_External_PCIE_CLK
 if(Use_External_PCIE_CLK)
 {
  //5. PCIE P0 PHY external clock
   mdelay(100); 
   REG32(PCIE_PHY0_REG) = 0xC3930301; //bit[3] must be "0" for external clock
   //REG32(PCIE_PHY0_REG) = 0xC39B0301; //20090304:RDC for for PCIE port 0 Refine-tune
    mdelay(100);       
   
       
          REG32(PCIE_PHY0_REG) = 0x3c011901; //close PHY 0 ,"0"=R,"1"=W
          mdelay(100);
 } 
 //#endif
 #endif
 
 
   //----------------------------------------
   if(mdio_reset)
    {
 //fix 8196C test chip pcie tx problem. 
/* 
 HostPCIe_SetPhyMdioWrite( 8, HostPCIe_SetPhyMdioRead(8) | (1<<3) );
 HostPCIe_SetPhyMdioWrite(0x0d, HostPCIe_SetPhyMdioRead(0x0d) | (5<<5) );
 HostPCIe_SetPhyMdioWrite(0x0d,  HostPCIe_SetPhyMdioRead(0x0d) | (1<<4) );
 HostPCIe_SetPhyMdioWrite(0x0f, HostPCIe_SetPhyMdioRead(0x0f) & ~(1<<4));
 HostPCIe_SetPhyMdioWrite(0x06, HostPCIe_SetPhyMdioRead(0x06) | (1<<11) ); 
*/ 
  HostPCIe_SetPhyMdioWrite(0, 0x5027);
  HostPCIe_SetPhyMdioWrite(2, 0x6d18);
  HostPCIe_SetPhyMdioWrite(6, 0x8828);
  HostPCIe_SetPhyMdioWrite(7, 0x30ff);
  HostPCIe_SetPhyMdioWrite(8, 0x18d7);
  HostPCIe_SetPhyMdioWrite(0xa, 0xe9);
  HostPCIe_SetPhyMdioWrite(0xb, 0x0511);
  HostPCIe_SetPhyMdioWrite(0xd, 0x15b6);  
  HostPCIe_SetPhyMdioWrite(0xf, 0x0f0f);    
#if 1 // PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(5, 0xbcb);    //[9:3]=1111001 (binary)   121 (10)
		HostPCIe_SetPhyMdioWrite(6, 0x8128);  //[11]=0   [9:8]=01
#endif  
/* 
emdiow 0 5027
emdiow 2 6d18
emdiow 6 8828
emdiow 7 30ff
emdiow 8 18dd
emdiow a e9
emdiow b 0511
emdiow d 15b6
emdiow f 0f0f
*/
    }
 
 //---------------------------------------
         // 6. PCIE Device Reset
     REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.
        mdelay(100);   
        mdelay(100);   
        mdelay(100);   
  
    REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
    //prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));
 
 
        //4. PCIE PHY Reset       
    REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
    REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
      mdelay(100);
  mdelay(1000);
  #if 1  //wait for LinkUP
 int i=100;
 while(--i)
 {
       if( (REG32(0xb8b00728)&0x1f)==0x11)
     break;
          mdelay(100);
 }
 if(i==0)
  printk("i=%x Cannot LinkUP \n",i);
#endif
 
printk("devid=%x\n",REG32(0xb8b10000));
   
 
}



static int rtl8196b_pcibios_config_access(unsigned char access_type,
       unsigned int addr, unsigned int *data)
{
   /* Do 8bit/16bit/32bit access */
   if (access_type & PCI_ACCESS_WRITE)
   {
      if (access_type & PCI_8BIT_ACCESS)
         WRITE_MEM8(addr, *data);
      else if (access_type & PCI_16BIT_ACCESS)
         WRITE_MEM16(addr, *data);
      else
         WRITE_MEM32(addr, *data);
   }
   else if (access_type & PCI_ACCESS_READ)
   {
      if (access_type & PCI_8BIT_ACCESS)
         *data = READ_MEM8(addr);
      else if (access_type & PCI_16BIT_ACCESS)
         *data = READ_MEM16(addr);
      else
         *data = READ_MEM32(addr);
   }

   /* If need to check for PCIE access timeout, put code here */
   /* ... */

   return 0;
}



/*
 * RTL8196b supports config word read access for 8/16/32 bit
 *
 * FIXME: currently only utilize 32bit access
 */
static int rtl8196b_pcibios0_read(struct pci_bus *bus, unsigned int devfn,
                                  int where, int size, unsigned int *val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   if (pci0_bus_number == 0xff)
      pci0_bus_number = bus->number;
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci0_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE0_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            *val = (data >> ((where & 3) << 3)) & 0xff;
         else if (size == 2)
            *val = (data >> ((where & 3) << 3)) & 0xffff;
         else
            *val = data;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci0_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE0_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(BSP_PCIE0_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = BSP_PCIE0_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Read Value: 0x%08X\n", *val);
   #endif

   return PCIBIOS_SUCCESSFUL;
}


static int rtl8196b_pcibios0_write(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, unsigned int val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   static int pci0_bus_number = 0xff;
   if (pci0_bus_number == 0xff)
      pci0_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci0_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE0_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            data = (data & ~(0xff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else if (size == 2)
            data = (data & ~(0xffff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else
            data = val;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci0_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE0_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(BSP_PCIE0_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = BSP_PCIE0_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   return PCIBIOS_SUCCESSFUL;
}


/*
 * RTL8196b supports config word read access for 8/16/32 bit
 *
 * FIXME: currently only utilize 32bit access
 */
#ifdef PIN_208
static int rtl8196b_pcibios1_read(struct pci_bus *bus, unsigned int devfn,
                                  int where, int size, unsigned int *val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   if (pci1_bus_number == 0xff)
      pci1_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci1_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE1_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            *val = (data >> ((where & 3) << 3)) & 0xff;
         else if (size == 2)
            *val = (data >> ((where & 3) << 3)) & 0xffff;
         else
            *val = data;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci1_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE1_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(BSP_PCIE1_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = BSP_PCIE1_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Read Value: 0x%08X\n", *val);
   #endif

   return PCIBIOS_SUCCESSFUL;
}


static int rtl8196b_pcibios1_write(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, unsigned int val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   static int pci1_bus_number = 0xff;

   if (pci1_bus_number == 0xff)
      pci1_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif


   if (bus->number == pci1_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE1_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            data = (data & ~(0xff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else if (size == 2)
            data = (data & ~(0xffff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else
            data = val;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci1_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE1_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(BSP_PCIE1_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = BSP_PCIE1_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   return PCIBIOS_SUCCESSFUL;
}
#endif

struct pci_ops rtl8196b_pci0_ops = {
   .read = rtl8196b_pcibios0_read,
   .write = rtl8196b_pcibios0_write
};

#ifdef PIN_208
struct pci_ops rtl8196b_pci1_ops = {
   .read = rtl8196b_pcibios1_read,
   .write = rtl8196b_pcibios1_write
};
#endif

static struct pci_controller rtl8196b_pci0_controller = {
   .pci_ops        = &rtl8196b_pci0_ops,
   .mem_resource   = &rtl8196b_pci0_mem_resource,
   .io_resource    = &rtl8196b_pci0_io_resource,
};

#ifdef PIN_208
static struct pci_controller rtl8196b_pci1_controller = {
   .pci_ops        = &rtl8196b_pci1_ops,
   .mem_resource   = &rtl8196b_pci1_mem_resource,
   .io_resource    = &rtl8196b_pci1_io_resource,
};
#endif

int pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("**Slot: %d\n", slot);
   printk("**Pin: %d\n", pin);
   printk("**Dev->BUS->Number: %d\n", dev->bus->number);
   #endif

   if (dev->bus->number < pci1_bus_number)
      return BSP_PCIE_IRQ;
   else
      return BSP_PCIE2_IRQ;
}

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   #endif

   return 0;
}

static __init int bsp_pcie_init(void)
{
   //rtl8196b_pci_reset();
   PCIE_reset_procedure(0,0,1);

#if DEBUG_PRINTK
   printk("<<<<<Register 1st PCI Controller>>>>>\n");
#ifdef PIN_208
   printk("<<<<<Register 2nd PCI Controller>>>>>\n");
#endif
#endif

   register_pci_controller(&rtl8196b_pci0_controller);
#ifdef PIN_208
   register_pci_controller(&rtl8196b_pci1_controller);
#endif
   return 0;
}

arch_initcall(bsp_pcie_init);
