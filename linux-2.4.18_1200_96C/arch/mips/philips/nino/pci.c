/*
 *  arch/mips/philips/nino/pci.c
 *
 *  Copyright (C) 2004 Hsin-I Wu (hiwu@realtek.com.tw)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * PCI service routines for RTL8181
 */ 


#include <linux/config.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/paccess.h>
#include <asm/pci_channel.h>

#ifdef CONFIG_PCI 

//#define PCI_SLOT0_CONFIG_BASE 0xbd700000 /* 512K bytes 0xbd77ffff */
// change to config addr to bd710000 for idsel pin fix (AD16)
#if 0
#ifdef CONFIG_RTL_EB8186
	#define PCI_SLOT0_CONFIG_BASE 0xbd710000 /* 512K bytes 0xbd77ffff */
#else
	#define PCI_SLOT0_CONFIG_BASE 0xbd700000 /* 512K bytes 0xbd77ffff */
#endif
#define PCI_SLOT1_CONFIG_BASE 0xbd780000 /* 512K bytes 0xbd7fffff */
#else
#define PCI_SLOT0_CONFIG_BASE 0xbd710000 /* 512K bytes 0xbd77ffff */
#define PCI_SLOT1_CONFIG_BASE 0xbd720000 /* 512K bytes 0xbd7fffff */
#define PCI_SLOT2_CONFIG_BASE 0xbd740000 /* 512K bytes 0xbd7fffff */
#define PCI_SLOT3_CONFIG_BASE 0xbd780000 /* 512K bytes 0xbd7fffff */
#endif

#define REG8(offset)	(*(volatile unsigned char *)(offset))
#define REG16(offset)	(*(volatile unsigned short *)(offset))
#define REG32(offset)	(*(volatile unsigned long *)(offset))

#define PCI_ILEV 6

#define PCI_SLOT_NUMBER CONFIG_PCI_EXTRA_DEVS
#if (PCI_SLOT_NUMBER < 1) || (PCI_SLOT_NUMBER > 4)
#error "wrong range of PCI_SLOT_NUMBER, rtl8l86 should between 1 and 4"
#endif

#define PCI_SLOT_MEM_BASE 0xbd600000
#define PCI_SLOT_IO_BASE  0xbd500000
#define PCI_MEM_SPACE_SIZE 0x00100000
#define PCI_IO_SPACE_SIZE  0x00100000

//PCI configuration space
#define PCI_CONFIG_VENDER_DEV 	0x0000
#define PCI_CONFIG_COMMAND 	0x0004
#define PCI_CONFIG_STATUS  	0x0006
#define PCI_CONFIG_CLASS_REVISION  	0x0008
#define PCI_CONFIG_CACHE  	0x000c
#define PCI_CONFIG_LATENCY 	0x000d
#define PCI_CONFIG_HEADER_TYPE 	0x000e
#define PCI_CONFIG_BASE0	0x0010
#define PCI_CONFIG_BASE1	0x0014
#define PCI_CONFIG_BASE2	0x0018
#define PCI_CONFIG_BASE3	0x001c
#define PCI_CONFIG_BASE4	0x0020
#define PCI_CONFIG_BASE5	0x0024
#define PCI_CONFIG_SUBSYSTEMVENDOR 	0x002c
#define PCI_CONFIG_INT_LINE	0x003c
#define PCI_CONFIG_INT_PIN	0x003d
#define PCI_CONFIG_MAXLAN 	0x003f
#define PCI_CONFIG_MINGNT 	0x003e

//PCI command register flag
#define CMD_FAST_BACK_TO_BACK	 	(1<<9)
#define CMD_SERR		 	(1<<8)
#define CMD_STEP_CONTROL	 	(1<<7)
#define CMD_PARITY_ERROR_RESPONSE 	(1<<6)
#define CMD_VGA_PALLETE_SNOOP	 	(1<<5)
#define CMD_WRITE_AND_INVALIDATE 	(1<<4)
#define CMD_SPECIAL_CYCLE	 	(1<<3)
#define CMD_BUS_MASTER			(1<<2)
#define CMD_MEM_SPACE			(1<<1)
#define CMD_IO_SPACE			(1<<0)


typedef struct BASE_ADDRESS_STRUCT{
  u32 Address;
  enum type_s {
    IO_SPACE=0x80,
    MEM_SPACE
  }Type;
  enum width_s {
    WIDTH32,
    WIDTH64
  }Width;
  u32 Prefetch;
  u32 Size;
}base_address_s;

typedef struct PCI_CONFIG_SPACE_STRUCT{
  u32 Config_Base;  //config space base address
  enum status_s{
    ENABLE,
    DISABLE
  }Status;  //device is enable or disable
  u32 Vendor_Device_ID;
  u8  Revision_ID;
  u32 Class_Code;
  u8  Header_Type;
  base_address_s BAR[6];
  u32 SubSystemVendor_ID;
}pci_config_s;


static pci_config_s *pci_slot[PCI_SLOT_NUMBER][8];

/* scan the resource needed by PCI device */
void scan_resource(pci_config_s *pci_config, int slot_num, int dev_function, u32 config_base)
{
	int i;
	u32 BaseAddr, data;
		
	BaseAddr = config_base+PCI_CONFIG_BASE0;	

    for (i=0;i<6;i++) {  //detect resource usage
    	REG32(BaseAddr) = 0xFFFFFFFF;
    	data = REG32(BaseAddr);
    	if (data!=0) {  //resource request exist
    	    int j;
    	    if (data&1) {  //IO space
    	    	pci_config->BAR[i].Type = IO_SPACE;
  	        //scan resource size
  	        for (j=2;j<32;j++)
  	            if (data&(1<<j)) break;
  	        if (j<32) pci_config->BAR[i].Size = 1<<j;
  	        else  pci_config->BAR[i].Size = 0;
    	    } else {  //Memory space
    	    	pci_config->BAR[i].Type = MEM_SPACE;
    	    	//bus width
    	    	if ((data&0x0006)==4) pci_config->BAR[i].Width = WIDTH64; //bus width 64
    	    	else pci_config->BAR[i].Width = WIDTH32;  //bus width 32
    	    	//prefetchable
    	    	if (data&0x0008) pci_config->BAR[i].Prefetch = 1; //prefetchable
    	    	else pci_config->BAR[i].Prefetch = 0;  //no prefetch
  	        //scan resource size
  	        if (pci_config->BAR[i].Width==WIDTH32) {
  	          for (j=4;j<32;j++)
  	            if (data&(1<<j)) break;
  	          if (j<32) pci_config->BAR[i].Size = 1<<j;
  	          else  pci_config->BAR[i].Size = 0;
  	        } else //width64 is not support
  	          pci_config->BAR[i].Size = 0;
    	    };
    	} else {  //no resource
    	    memset(&(pci_config->BAR[i]), 0, sizeof(base_address_s));    		
    	};
	BaseAddr += 4;  //next base address
    };
}

/* scan the PCI bus, save config information into pci_slot,
   return the number of PCI functions */
int pci_scan(void)
{
    u32 config_base, data;
    int i, dev_function;
    int dev_num = 0;
  
	for (i=0;i<PCI_SLOT_NUMBER;i++) {  //probe 2 pci slots
    	switch(i){	
       	case 0:      	   	       	   	
           	config_base = PCI_SLOT0_CONFIG_BASE;           
           	break;
       	case 1:     
       		// this delay is necessary to 
       		// work around pci configuration timing issue
       		/*
       	   	config_base = 0;	       	
       	   	while(config_base < 1000){
       	   		config_base++;
       	   	}       	   	
       	   	*/
           	config_base = PCI_SLOT1_CONFIG_BASE;
           	break;
       	case 2:      	   	       	   	
           	config_base = PCI_SLOT2_CONFIG_BASE;           
           	break;
       	case 3:      	   	       	   	
           	config_base = PCI_SLOT3_CONFIG_BASE;           
           	break;
       	default:
       		return 0;
		}
      
      dev_function=0;
      while (0!=REG32(config_base+PCI_CONFIG_VENDER_DEV)) {  //pci device exist
          pci_slot[i][dev_function] = kmalloc(sizeof(pci_config_s),GFP_KERNEL);
          pci_slot[i][dev_function]->Config_Base = config_base;
          pci_slot[i][dev_function]->Status = DISABLE;
          pci_slot[i][dev_function]->Vendor_Device_ID = REG32(config_base+PCI_CONFIG_VENDER_DEV);
          data = REG32(config_base+PCI_CONFIG_CLASS_REVISION);
          pci_slot[i][dev_function]->Revision_ID = data&0x00FF;
          pci_slot[i][dev_function]->Class_Code = data>>8;
          pci_slot[i][dev_function]->Header_Type = (REG32(config_base+PCI_CONFIG_CACHE)>>16)&0x000000FF;
          pci_slot[i][dev_function]->SubSystemVendor_ID = REG32(config_base+PCI_CONFIG_SUBSYSTEMVENDOR);
          scan_resource(pci_slot[i][dev_function], i, dev_function, config_base);  //probe resource request
          printk("PCI device exists: slot %d function %d VendorID %x DeviceID %x %x\n", i, dev_function, 
              pci_slot[i][dev_function]->Vendor_Device_ID&0x0000FFFF,
              pci_slot[i][dev_function]->Vendor_Device_ID>>16,config_base);
          dev_num++;
          dev_function++;
          ////config_base += dev_function*64*4;  //next function's config base
          config_base += 256;  //next function's config base
          if (!(pci_slot[i][0]->Header_Type&0x80)) break;  //single function card
          if (dev_function>=8) break;  //only 8 functions allow in a PCI card
      };
      
    };
    return dev_num;
}

/* sort resource by its size */
void bubble_sort(int space_size[PCI_SLOT_NUMBER*8*6][2], int num)
{
  int i, j;
  int tmp_swap[2];

  if(num < 2)
	return;
#if 0 // HIWU: new bobble sort algorithm; old sorting method is not correct
  
    for (i=0;i<num-1;i++) {
    	for (j=i;j<num-1;j++) {
          if (space_size[j][1]<space_size[j+1][1]) {
            tmp_swap[0] = space_size[j][0];
            tmp_swap[1] = space_size[j][1];
            space_size[j][0] = space_size[j+1][0];
            space_size[j][1] = space_size[j+1][1];
            space_size[j+1][0] = tmp_swap[0];
            space_size[j+1][1] = tmp_swap[1];
          };
        };
    };
#else // new bobble sort algorithm
    for (i=0;i<num-1;i++) {
        for (j=i+1;j<num;j++) {
          if (space_size[i][1]>space_size[j][1]) {
            tmp_swap[0] = space_size[i][0];
            tmp_swap[1] = space_size[i][1];
            space_size[i][0] = space_size[j][0];
            space_size[i][1] = space_size[j][1];
            space_size[j][0] = tmp_swap[0];
            space_size[j][1] = tmp_swap[1];
          };
        };
    };
#endif  // new bobble sort algorithm
}

/*	assign memory location to MEM & IO space
	return 0:OK, else:fail */
int assign_resource(void)
{
  	int i, slot, func, BARnum;
  	int mem_space_size[PCI_SLOT_NUMBER*8*6][2]; //[0]:store device index, [1]:store resource size
  	int io_space_size[PCI_SLOT_NUMBER*8*6][2]; //[0]:store device index, [1]:store resource size
  	int mem_idx, io_idx, total_size, tmp;
  	u16 config_command[PCI_SLOT_NUMBER][8];  	//record config space resource usage, 
  												// 1:use IO, 2:use memory, 3:both
  
    memset(mem_space_size, 0, sizeof(mem_space_size));
    memset(io_space_size, 0, sizeof(io_space_size));
    memset(config_command, 0, sizeof(config_command));
    //collect resource
    mem_idx = io_idx =0;
    for (slot=0;slot<PCI_SLOT_NUMBER;slot++) {
      if (pci_slot[slot][0]==0) continue;  //this slot is null      
      if (pci_slot[slot][0]->Vendor_Device_ID==0) continue;  //this slot is null      
      for (func=0;func<8;func++) {
        if (pci_slot[slot][func]==0) break;
        pci_slot[slot][func]->Status = ENABLE;
        for (BARnum=0;BARnum<6;BARnum++) {
      		if (pci_slot[slot][func]->BAR[BARnum].Type==MEM_SPACE){  //memory space
      		    config_command[slot][func] |= CMD_MEM_SPACE;  //this device use Memory
      		    mem_space_size[mem_idx][0] = (slot<<16)|(func<<8)|(BARnum<<0);
      		    mem_space_size[mem_idx][1] = pci_slot[slot][func]->BAR[BARnum].Size;
      		    mem_idx++;
      		} else if (pci_slot[slot][func]->BAR[BARnum].Type==IO_SPACE){  //io space
      		    config_command[slot][func] |= CMD_IO_SPACE;  //this device use IO
      		    io_space_size[io_idx][0] = (slot<<16)|(func<<8)|(BARnum<<0);
      		    io_space_size[io_idx][1] = pci_slot[slot][func]->BAR[BARnum].Size;
      		    io_idx++;
      		};
	    };  //for (BARnum=0;BARnum<6;BARnum++) 
      };  //for (func=0;func<8;func++)
    };  //for (slot=0;slot<PCI_SLOT_NUMBER;slot++)
      
    //sort by size
    if (mem_idx>1) bubble_sort(mem_space_size, mem_idx);
    if (io_idx>1)  bubble_sort(io_space_size, io_idx);   
    
    //check mem total size
    total_size = 0;
    for (i=0;i<mem_idx;i++) {  	
    	tmp = mem_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);     
        total_size = total_size + mem_space_size[i][1];
    };
    if (total_size>PCI_MEM_SPACE_SIZE) {
	return -1;  //lack of memory space resource
    }

    //check io total size
    total_size = 0;
    for (i=0;i<io_idx;i++) {     	
    	tmp = io_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);     
        total_size = total_size + io_space_size[i][1];
    };
    if (total_size>PCI_IO_SPACE_SIZE) 
    {
		return -2;  //lack of io space resource
    }
    
    //assign memory space
    total_size = 0;
    for (i=0;i<mem_idx;i++) {
    	unsigned int config_base;     	
    	tmp = mem_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);        
        tmp = mem_space_size[i][0];
        //assign to struct
        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_MEM_BASE+total_size;
     	switch((tmp>>16)){
     	case 0:
     		config_base = PCI_SLOT0_CONFIG_BASE;
     		break;
     	case 1:
     		config_base = PCI_SLOT1_CONFIG_BASE;
     		break;     	
     	case 2:
     		config_base = PCI_SLOT2_CONFIG_BASE;
     		break;     	
     	case 3:
     		config_base = PCI_SLOT3_CONFIG_BASE;
     		break;     	
     	default:
     		panic("PCI slot assign error");
    	}        
        //get BAR address and assign to PCI device
        tmp = config_base   //SLOT
              +((tmp>>8)&0x00FF)*64*4					//function
              +((tmp&0x00FF)*4+PCI_CONFIG_BASE0);			//BAR bumber
        REG32(tmp) = (PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF;  //map to physical address
        total_size = total_size + mem_space_size[i][1];  //next address
    };
    //assign IO space
    total_size = 0;
    for (i=0;i<io_idx;i++) {
    	unsigned int config_base;    	
    	tmp = io_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);        
        tmp = io_space_size[i][0];
        //assign to struct
        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_IO_BASE+total_size;
        //get BAR address and assign to PCI device
     	switch((tmp>>16)){
     	case 0:
     		config_base = PCI_SLOT0_CONFIG_BASE;
     		break;
     	case 1:
     		config_base = PCI_SLOT1_CONFIG_BASE;
     		break;     	
     	case 2:
     		config_base = PCI_SLOT2_CONFIG_BASE;
     		break;     	
     	case 3:
     		config_base = PCI_SLOT3_CONFIG_BASE;
     		break;     	
     	default:
     		panic("PCI slot assign error");
    	}
        tmp = config_base   //SLOT
              +((tmp>>8)&0x00FF)*64*4					//function
              +((tmp&0x00FF)*4+PCI_CONFIG_BASE0);			//BAR bumber
        
        REG32(tmp) = (PCI_SLOT_IO_BASE+total_size)&0xFFFF;  //HIWU: map to physical address < 64K.(work around PCI-IO mask)
        
        total_size = total_size + io_space_size[i][1];  //next address
    };
    //enable device
    for (slot=0;slot<PCI_SLOT_NUMBER;slot++) {
      if (pci_slot[slot][0]==0) continue;  //this slot is null      
      if (pci_slot[slot][0]->Vendor_Device_ID==0) continue;  //this slot is null      
      for (func=0;func<8;func++) {
      	unsigned int config_base;
        if (pci_slot[slot][func]==0) continue;  //this slot:function is null      
        if (pci_slot[slot][func]->Vendor_Device_ID==0) continue;  //this slot:function is null      
        //get config base address
     	switch(slot){
     	case 0:
     		config_base = PCI_SLOT0_CONFIG_BASE;
     		break;
     	case 1:
     		config_base = PCI_SLOT1_CONFIG_BASE;
     		break;     	
     	case 2:
     		config_base = PCI_SLOT2_CONFIG_BASE;
     		break;     	
     	case 3:
     		config_base = PCI_SLOT3_CONFIG_BASE;
     		break;     	
     	default:
     		panic("PCI slot assign error");
    	}        
        
        tmp = config_base   //SLOT
              +func*64*4;					   //function
        //set Max_Lat & Min_Gnt, irq
        //printk("1 REG32(tmp+PCI_CONFIG_INT_LINE) = %x\n",REG32(tmp+PCI_CONFIG_INT_LINE));
		REG32(tmp+PCI_CONFIG_INT_LINE) = 0x40200100|(PCI_ILEV); //IRQ level
		//printk("2 REG32(tmp+PCI_CONFIG_INT_LINE) = %x\n",REG32(tmp+PCI_CONFIG_INT_LINE));

		//enable cache line size, lantancy
		REG32(tmp+PCI_CONFIG_CACHE) = (REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004;  //32 byte cache, 20 latency
		
        //set command register
        //REG32(tmp+PCI_CONFIG_COMMAND) = (REG32(tmp+PCI_CONFIG_COMMAND)&0xFFFF0000)|config_command[slot][func]
        //					|CMD_BUS_MASTER|CMD_WRITE_AND_INVALIDATE|CMD_PARITY_ERROR_RESPONSE;
	// this delay is necessary to 
       	// work around pci configuration timing issue
       	config_base = 0;	       	
       	while(config_base < 1000){
       	config_base++;
       	} 

        REG16(tmp+PCI_CONFIG_COMMAND) = config_command[slot][func]
        					|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE;//|CMD_WRITE_AND_INVALIDATE;
      };
    };
    
    return 0;
}


/* scan pci bus, assign Memory & IO space to PCI card, 
	and call init_XXXX to enable & register PCI device
	rtn 0:ok, else:fail */
int init_rtl8181pci(void)
{
  	int function_num;
    printk("Probe PCI Bus : There must be one device at the slot.\n");
    
    if(PCI_SLOT_NUMBER > 1){
    	 u32 config_base;
         config_base=inl(0x0100);//read Bridge Control Register (BRIDGE_REG)
    	 //// printk("(1)BRIDGE_REG = %8.8x\n",config_base);         
         //The second PCI bus enable. 0 enable second
         //PCI device,1 disable second PCI device                
         config_base &= 0xffdfffff;
         outl(config_base,0x0100);    
         config_base=inl(0x0100);
         //// printk("(2)BRIDGE_REG = %8.8x\n",config_base);
    }

    memset(pci_slot, 0, 4*PCI_SLOT_NUMBER*8);
    function_num = pci_scan();
    if (function_num==0) {
    	printk("No PCI device exist!!\n");
    	return -1;
    };
    
    //auto assign resource
    if (assign_resource()) {
    	printk("PCI Resource assignment failed!\n");
    	return -2;
    };
    
    printk("Find Total %d PCI function\n", function_num);
    return 0;
      
}


static int rtlpci_read_config_byte(struct pci_dev *dev, int where, u8 *value)
{
	u32 tmp;
	u32 addr;
	u32 shift=3-(where & 0x3);	
	
	
	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
			
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if (dev->bus->number != 0)
	{
		*value = 0;
		return PCIBIOS_SUCCESSFUL;
	}

	addr |= (where&~0x3);
	

	tmp = REG32(addr);
	tmp = cpu_to_le32(tmp);
	for (addr=0;addr<shift;addr++)
		tmp = tmp >>8;

	*value = (u8)tmp;
	
	return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_read_config_word(struct pci_dev *dev, int where, u16 *value)
{
	
	u32 tmp,addr;

	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL) 
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if (dev->bus->number != 0)
	{
		*value = 0;
		return PCIBIOS_SUCCESSFUL;
	}

	addr |= (where&~0x3);
	

	tmp = REG32(addr);
	if (where&0x2)
		*value = (u16)tmp>>16 ;
	else
		*value = (u16)(tmp);

	return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_read_config_dword(struct pci_dev *dev, int where, u32 *value)
{
	u32 addr;
	
	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;		

	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
		
	if (dev->bus->number != 0)
	{
		*value=0;
		return PCIBIOS_SUCCESSFUL;
	}

	*value = REG32(addr+where);
	return PCIBIOS_SUCCESSFUL;
}


static int rtlpci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
	u32 addr;
	
	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
				
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;				
				
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if (dev->bus->number != 0)		
		return PCIBIOS_SUCCESSFUL;
	REG8(addr+where)=value;
		return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_write_config_word(struct pci_dev *dev, int where, u16 value)
{
	u32 addr;
	
	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;	
	
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
				
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if (dev->bus->number != 0)		
		return PCIBIOS_SUCCESSFUL;
	REG16(addr+where)=value;
		return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_write_config_dword(struct pci_dev *dev, int where, u32 value)
{
	u32 addr;

	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;		
				
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if (dev->bus->number != 0)		
		return PCIBIOS_SUCCESSFUL;
	REG32(addr+where)=value;
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops pcibios_ops = {
	rtlpci_read_config_byte,
	rtlpci_read_config_word,
	rtlpci_read_config_dword,
	rtlpci_write_config_byte,
	rtlpci_write_config_word,
	rtlpci_write_config_dword
};

/*
#define PCI_IO_START      0x1d500000
#define PCI_IO_END        0x1d5fffff
#define PCI_MEM_START     0x1d600000
#define PCI_MEM_END       0x1d6fffff
*/
#define PCI_IO_START      (0xbd500000 - 0xbd010000)
#define PCI_IO_END        (0xbdfffff - 0xbd010000)
#define PCI_MEM_START     0xbd600000
#define PCI_MEM_END       0xbd6fffff

static struct resource pci_io_resource = {
	"pci IO space", 
	PCI_IO_START,
	PCI_IO_END,
	IORESOURCE_IO
};

static struct resource pci_mem_resource = {
	"pci memory space", 
	PCI_MEM_START,
	PCI_MEM_END,
	IORESOURCE_MEM
};


struct pci_channel mips_pci_channels[] = {
	{&pcibios_ops, &pci_io_resource, &pci_mem_resource, 0, 1},
	{(struct pci_ops *) NULL, (struct resource *) NULL,
	 (struct resource *) NULL, (int) NULL, (int) NULL}
};


int pcibios_enable_resources(struct pci_dev *dev)
{
	return 0;
}

#if 0
static void memdump(unsigned address, unsigned length) {
	unsigned data, currentaddress, i, j;
	char c;

	i=0;
	printk("Memory dump: start address 0x%8.8x , length %d bytes\n\n\r",address,length);
	printk("Address        Data\n\r");

	while(i<length) {
		currentaddress = address + i;
		data = *(unsigned long*)(currentaddress);
		printk("0x%8.8x:   %8.8x    ",currentaddress,data);
		for(j=0;j<4;j++) {
			c = (data >> 8*j) & 0xFF;
			if(c<0x20 || c>0x7e) {
				printk("\\%3.3d ",(int)c);
			} else {
				printk(" %c   ",c);
			}
		}
		printk("\n\r");
		i += 4;
	}
	printk("\n\rEnd of memory dump.\n\n\r");
}
#endif /* if 0 */

/*
 *  If we set up a device for bus mastering, we need to check the latency
 *  timer as certain crappy BIOSes forget to set it properly.
 *
 *  
 */


unsigned int pcibios_assign_all_busses(void)
{
	return 0;
}


void __init pcibios_fixup_resources(struct pci_dev *dev)
{	
#if 1	
	int i;
	/* Search for the IO base address.. */
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		unsigned int io_addr = pci_resource_start(dev, i);
		unsigned int io_size = pci_resource_len(dev, i);
		unsigned int new_io_addr ;
		if (!(pci_resource_flags(dev,i) & IORESOURCE_IO))
			continue;
		// to work around pci card with IO mappping <64K limitation
		io_addr &= 0x000fffff;
		io_addr |= 0x1d500000;

		pci_resource_start(dev, i) = (io_addr - 0x1D010000);
		pci_resource_end(dev, i) = (io_addr + io_size - 0x1D010000 -1 );
		new_io_addr = pci_resource_start(dev, i);
		printk("pcibios_fixup_resources IO form %x to %x\n",io_addr,new_io_addr);
		
		/* Is it already in use? */
		if (check_region (new_io_addr, io_size))
			break;		
	}
#endif 		
}


void __init pcibios_fixup(void)
{
	/* nothing to do here */
}

void __init pcibios_fixup_irqs(void)
{
    struct pci_dev *dev;

	pci_for_each_dev(dev) {
		dev->irq = 6; // fix irq
	}
}

#endif /* CONFIG_PCI */

