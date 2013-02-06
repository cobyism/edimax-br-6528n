/*
 *  arch/mips/realtek/rtl865x/pci.c
 *
 *  Copyright (C) 2004 Hsin-I Wu (hiwu@realtek.com.tw)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */ 

#include <linux/config.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/paccess.h>
#include <asm/pci_channel.h>
#include <asm-mips/rtl865x/interrupt.h>
#include <asm-mips/rtl865x/pci.h>

#ifdef CONFIG_PCI 

#if 0
	#define DBG(x...) printk(x)
#else
	#define DBG(x...)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

int rtl_pci_big_endian=TRUE;//Default assume 8650B 

//#define DEBUG_PCI_IO

#ifdef CONFIG_RTL865XC
#define REGIRR 0xb8003008
#define REGISR 0xb8003004
#define REGIMR 0xb8003000
#else
#define REGIRR 0xbd012008
#define REGISR 0xbd012004
#define REGIMR 0xbd012000
#endif

#ifdef DEBUG_PCI_IO
#define PRINTIO(endian,flg,adr,val,ver) do{ \
											if(flg=='R') \
												printk("[%c]PCI ioread%d-->addr=%08x value=%02x GIMR-->%x GISR-->%x \n",endian,ver,adr,val,REG32(REGIMR),REG32(REGISR)); \
											else if(flg=='W') \
												printk("[%c]PCI iowrite%d-->addr=%08x value=%02x GIMR-->%x GISR-->%x \n",endian,ver,adr,val,REG32(REGIMR),REG32(REGISR)); \
											else break; \
										}while(0)
#else
#define PRINTIO(endian,flg,adr,val,ver) do{} while(0)
#endif

#define REG8(offset)	(*(volatile unsigned char *)(offset))
#define REG16(offset)	(*(volatile unsigned short *)(offset))
#define REG32(offset)	(*(volatile unsigned long *)(offset))

/* Registers defined in RTL865xB */
#define PABCPTCR	(0xbd012054)
#define PABCCNR		(0xbd01200c)
#define CRMR			(0xbc805104)

/* Chip version check */
#ifdef CONFIG_RTL865XC
#define IS_RTL865XC			1
#define IS_RTL865XB_CPLUS	0
#define IS_RTL865XB_AB		0
#else
#define IS_RTL865XC			0
#define IS_RTL865XB_CPLUS	((REG32(CRMR)&0x00ff0000)>=0x00020000)
#define IS_RTL865XB_AB		((REG32(CRMR)&0x00ff0000)<0x00020000)
#endif

#ifdef CONFIG_RTL865XC
#define PCI_ILEV 3
#else	
#define PCI_ILEV 5
#endif

#define PCI_SLOT_NUMBER 4
#if (PCI_SLOT_NUMBER < 1) || (PCI_SLOT_NUMBER > 4)
#error "wrong range of PCI_SLOT_NUMBER, rtl865xB should between 1 and 4"
#endif

static void __init quirk_pci_bridge(struct pci_dev *dev)
{
	return;
}

int rtl_pci_isLinuxCompliantEndianMode(void)
{
#ifdef CONFIG_RTL865XC
	if((*(volatile unsigned int *)(0xb8003400))&2) rtl_pci_big_endian=FALSE;

	return ((*(volatile unsigned int *)(0xb8003400))&2);
#else
	if((*(volatile unsigned int *)(0xbd012064))&2) rtl_pci_big_endian=FALSE; 
	return ((*(volatile unsigned int *)(0xbd012064))&2);
#endif
}

//struct pci_fixup pcibios_fixups[] = {	{ PCI_FIXUP_HEADER, PCI_ANY_ID, PCI_ANY_ID, quirk_pci_bridge },	{ 0 }};


typedef struct BASE_ADDRESS_STRUCT
{
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

typedef struct PCI_CONFIG_SPACE_STRUCT
{
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

u32 rtlpci_read_config_endian_free(u32 address)
{		
	if(!rtl_pci_big_endian)
	{
		return le32_to_cpu(REG32(address));		
	}
	return REG32(address);
}

void rtlpci_write_config_endian_free(u32 address,u32 value)
{	

	if(!rtl_pci_big_endian)
		REG32(address)=le32_to_cpu(value);	
	else
		REG32(address)=value;
}

/* scan the resource needed by PCI device */
void scan_resource(pci_config_s *pci_config, int slot_num, int dev_function, u32 config_base)
{
	int i;
	u32 BaseAddr, data;
		
	BaseAddr = config_base+PCI_CONFIG_BASE0;	

    for (i=0;i<6;i++) {  //detect resource usage
    	rtlpci_write_config_endian_free(BaseAddr,0xffffffff);
    	data = rtlpci_read_config_endian_free(BaseAddr);
    	    	
    	if (data!=0) {  //resource request exist
    	    int j;
    	    if (data&1) {  //IO space
    	    
    	    	pci_config->BAR[i].Type = IO_SPACE;
    	    	    	    	
  	        //scan resource size
#if 0  	        
  	        for (j=2;j<32;j++)
  	            if (data&(1<<j)) break;
  	        if (j<32) pci_config->BAR[i].Size = 1<<j;
  	              else  pci_config->BAR[i].Size = 0;
#else
			pci_config->BAR[i].Size=((~(data&PCI_BASE_ADDRESS_IO_MASK))+1)&0xffff;
#endif
  	        DBG("IO Space %i, data=0x%x size=0x%x \n",i,data,pci_config->BAR[i].Size);
  	  
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
  	          {
  	          	pci_config->BAR[i].Size = 0;
  	          }
  	        DBG("Memory Space %i data=0x%x size=0x%x\n",i,data,pci_config->BAR[i].Size);	
    	    }
    	} else {  //no resource
    	    memset(&(pci_config->BAR[i]), 0, sizeof(base_address_s));    		
    	}
	BaseAddr += 4;  //next base address
    }
    
}

/* scan the PCI bus, save config information into pci_slot,
   return the number of PCI functions */
int rtl_pci_scan_slot(void)
{
	u32 config_base, data, vendor_device;
	int i, dev_function;
	int dev_num = 0;
	u16 device,vendor,last_ok_func;
  	int setAsicRegisterForPciSlot = 0;
		
	REG32(0xb8003004)=REG32(0xb8003004); //clear the timeout register, david
	
	for ( i=0 ; i<PCI_SLOT_NUMBER ; i++ )
	{  //probe 4 pci slots

		if (IS_RTL865XB_AB)
		{
DBG("is RTLl865XB A,B Cut\n");		
#ifndef CONFIG_RTL865XB_PCI_SLOT0
			if(i==0) continue;
#endif
#ifndef CONFIG_RTL865XB_PCI_SLOT1
			if(i==1) continue;
#endif
#ifndef CONFIG_RTL865XB_PCI_SLOT2
			if(i==2) continue;
#endif
#ifndef CONFIG_RTL865XB_PCI_SLOT3
			if(i==3) continue;
#endif
		}

   		// this delay is necessary to 
   		// work around pci configuration timing issue       		
   	   	config_base = 0;

   	   	while(config_base < 3000000)
		{
   	   		config_base++;
   	   	}   

		switch(i)
		{
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
				return 0;
		}

		dev_function=0;

#ifdef CONFIG_RTL865XC        
  	  DBG("timeout=%x base=%x\n",REG32(0xb8003004),(u32)(config_base+PCI_CONFIG_VENDER_DEV));	
#else
// 	  printk("timeout=%x base=%x\n",REG32(0xbd012004),(u32)(config_base+PCI_CONFIG_VENDER_DEV));
//	  printk("timeout=%x\n",REG32(0xbd012004));
#endif

		vendor_device = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV);



		vendor=vendor_device&0xffff;
		device=vendor_device>>16;

		last_ok_func=0;

		while (dev_function<8)
		{  //pci device exist

			if ((IS_RTL865XB_CPLUS)||(IS_RTL865XC))
			{
				int match=1;

#ifdef CONFIG_RTL865XC
				/* set Lexra Bus Time Out Limit = 2^7 Clock */
/*
	chenyl:
		After verifying on RTL8652 QA Board 14/13/17,
		decreasing LBT limit would cause SwCore processing FAILED.
		So we don't modify it here ( Default value is MAX LBT ).
*/
#if 0
				REG32(0xb8003210)&=0xfffffff8;
#endif
#else
				/* set Lexra Bus Time Out Limit = 2^7 Clock */
				REG32(0xbd012064)&=0x8fffffff;
#endif
				
				rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV);

				if(vendor==0x13f6) // C-media
				{
#ifdef CONFIG_RTL865XC
//					REG32(0xb8003200)
//					FIXME: RTL865XC support different 16bits mode.
#else
					REG32(0xbc805038)|=0x80000000;	//enable 16bit mode for Slot3
#endif					
					
				}
				
				if(dev_function>0)
				{
					int j;

					for( j=0 ; j<64 ; j+=4 )
					{
						if(rtlpci_read_config_endian_free(config_base+j)!=rtlpci_read_config_endian_free(config_base+j-(dev_function-last_ok_func)*0x100))
						{
							match=0;
						}
					}
				}
				else
				{
					match=0;
				}

#ifdef CONFIG_RTL865XC		   

				if(((REG32(0xb8003004)&0x0000000b)!=0)||(match==1))
				{	  		
					REG32(0xb8003004)=REG32(0xb8003004); //clear the timeout register
					DBG("BusTimeout Clear Register 0xb8003004=%x fun=%d\n",REG32(0xb8003004),dev_function);
					dev_function++;
					config_base += 256;			
					continue;
				}
				else
				{
					DBG("using SLOT:%d, FUNCTION:%d\n",i,dev_function);
					last_ok_func=dev_function;
				}


#else
				if(((REG32(0xbd012004)&0x00210000)!=0)||(match==1))
				{	  		
					REG32(0xbd012004)=REG32(0xbd012004); //clear the timeout register
					DBG("BusTimeout Clear Register 0xbd012004=%x fun=%d\n",REG32(0xbd012004),dev_function);
					dev_function++;
					config_base += 256;			
					continue;
				}
				else
				{
					DBG("using SLOT:%d, FUNCTION:%d\n",i,dev_function);
					last_ok_func=dev_function;
				}
#endif				
			}
			else // RTL865XB verA,verB
			{
				switch(vendor)
				{
					case 0x17a0: //Genesys
						if(!((dev_function==0)||(dev_function==3)) )
						{
							goto next_func;
						}
						DBG("Found Genesys USB 2.0 PCI Card!, function=%d!\n",dev_function);
						//REG32(config_base+0x50)&=(0xffffffef);
						break;

					case 0x13f6: // C_MEDIA
						REG32(0xbc805038)|=0x80000000;	//enable 16bit mode
						if(dev_function>0) //assume atheros's chip only have one function.
		                            {
							goto next_func;
						}
						break;
	      	
					case PCI_VENDOR_ID_AL: //ALi
						if(dev_function>3)
						{
							goto next_func;
						}
						DBG("Found ALi USB 2.0 PCI Card, function=%d!\n",dev_function);			      	
						break;
		
					case PCI_VENDOR_ID_VIA: //0x1106
						if(dev_function>2)
						{
							goto next_func;
						}
						DBG("Found VIA USB 2.0 PCI Card!, function=%d!\n",dev_function);			      	
						break;

					case PCI_VENDOR_ID_NEC: //0x1033
						if(dev_function>2)
						{
							goto next_func;
						}
						DBG("Found NEC USB 2.0 PCI Card, function=%d!\n",dev_function);			      	
						break;

					case PCI_VENDOR_ID_REALTEK:
						if(dev_function>0) ////assume realtek's chip only have one function.
						{
							if((device==0x8139)||(device==0x8185))
							{
								goto next_func;
							}
						}

						if(device==0x8139)
							DBG("Found Realtek 8139 PCI Card, function=%d!\n",dev_function);
						else if(device==0x8185)
							DBG("Found Realtek 8185 PCI Card, function=%d!\n",dev_function);						
						break;

					case 0x168c: // Atheros
						if(dev_function>0) //assume atheros's chip only have one function.
						{
							//if(device==0x0013)
							{
								goto next_func;
							}
						}
						if(device==0x0013)			
							DBG("Found Atheros AR5212 802.11a/b/g PCI Card, function=%d!\n",dev_function);
						else if(device==0x0012)
							DBG("Found Atheros AR5211 802.11a/b PCI Card, function=%d!\n",dev_function);				
						else
							DBG("Found Atheros unknow PCI Card, function=%d!\n",dev_function);				
						break;
					case 0x17cb: // Airgo
						if(dev_function>0) 
						{
							goto next_func;
						}
						break;

					case PCI_VENDOR_ID_PROMISE: //0x105a
						if(dev_function>0)
						{
next_func:
							dev_function++;
							config_base += 256;
							continue;
						}
						DBG("Found Promise IDE PCI Card, function=%d!\n",dev_function);
						break;

					default:
						break;
				}
			}

			if (	(0==rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV)) ||
				((rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV)&0xffff)==0xffff))
			{
				dev_function++;
				config_base += 256;  //next function's config base
				continue;
			}


			/*
				We check if there is any device at slot 2/3.
				If the answer is YES, we must modify PABCCNR and PABCPTCR for that PCI device.
																						*/
			if ( i >= 2 )	/* slot 2/3 */  
			{
				DBG("PCI: PCI slot %d inuse. Config GPIO-A pins for PCI use\n", i);

				/* We need to set these registers if they are not turned on yet. */
				if (setAsicRegisterForPciSlot == 0)
				{
 					REG32(PABCPTCR)|=0xf0000000;	/* GPIO Port A,B,C Peripheral Type Control Register (PABCPTCR) */
					REG32(PABCCNR)|=0xf0000000;	/* Port A,B,C Control Register (PABCCNR) */

					/* Indicate that we have set RTL865xB's PABCPTCR and PABCCNR */
					setAsicRegisterForPciSlot = 1;
				}
			}

			pci_slot[i][dev_function] = kmalloc(sizeof(pci_config_s),GFP_KERNEL);
			if(pci_slot[i][dev_function]==NULL)
			{
				DBG("%s: Out of Memory!\n",__FUNCTION__);
				return 0;

			}
				
			pci_slot[i][dev_function]->Config_Base = config_base;
			pci_slot[i][dev_function]->Status = DISABLE;
			pci_slot[i][dev_function]->Vendor_Device_ID = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV);
			data = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_CLASS_REVISION);
			pci_slot[i][dev_function]->Revision_ID = data&0x00FF;
			pci_slot[i][dev_function]->Class_Code = data>>8;
			pci_slot[i][dev_function]->Header_Type = (rtlpci_read_config_endian_free(config_base+PCI_CONFIG_CACHE)>>16)&0x000000FF;
			pci_slot[i][dev_function]->SubSystemVendor_ID = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_SUBSYSTEMVENDOR);
			scan_resource(pci_slot[i][dev_function], i, dev_function, config_base);  //probe resource request

			DBG("PCI device exists: slot %d function %d VendorID %x DeviceID %x %x\n", i, dev_function,           	
			pci_slot[i][dev_function]->Vendor_Device_ID&0x0000FFFF,
			pci_slot[i][dev_function]->Vendor_Device_ID>>16,config_base);
			dev_num++;
			dev_function++;
			////config_base += dev_function*64*4;  //next function's config base
			config_base += 256;  //next function's config base
//			if (!(pci_slot[i][0]->Header_Type&0x80)) break;  //single function card
			if (dev_function>=8) break;  //only 8 functions allow in a PCI card  
		}      
	}	

    return dev_num;
}

/* sort resource by its size */
#ifdef CONFIG_RTL865XC
void bubble_sort(int space_size[2][(PCI_SLOT_NUMBER/2)*8*6][2], int num[2])
{
  int i, j,k;
  int tmp_swap[2];
  for(k=0;k<2;k++)
  {
    for (i=0;i<num[k]-1;i++) {
    	/*for (j=i;j<num[k]-1;j++) {*/
	for (j=0;j<num[k]-i-1;j++) {
          if (space_size[k][j][1]<space_size[k][j+1][1]) {
            tmp_swap[0] = space_size[k][j][0];
            tmp_swap[1] = space_size[k][j][1];
            space_size[k][j][0] = space_size[k][j+1][0];
            space_size[k][j][1] = space_size[k][j+1][1];
            space_size[k][j+1][0] = tmp_swap[0];
            space_size[k][j+1][1] = tmp_swap[1];
          };
        };
    }
  }
}

#else

void bubble_sort(int space_size[PCI_SLOT_NUMBER*8*6][2], int num)
{
  int i, j;
  int tmp_swap[2];
  
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
}
#endif


#ifdef CONFIG_RTL865XC

int rtl_pci_reset(void)
{

	//REG32(0xb8003000)|=0x4000; /* enable PCI interrupt */
#if 0
	REG32(0xb800320c)=6; //PCI2LEXBCR = 1, set CLK to 28M
	REG32(0xb8003310)=0xe0; //enlarge PCI driving current
#endif	
	REG32(0xb800320c)=0x5; //PCI2LEXBCR = 1, set CLK to 33M
	REG32(0xb8003210)=0xf; //Peripheral Timeout&Timing Enable

	
	DBG("REG32(0xb8003400)=%x func=%s line=%d\n",REG32(0xb8003400),__FUNCTION__,__LINE__);      	
	REG32(0xb8003400) |= 0x1; // PCI RESET ACTIVE
	DBG("REG32(0xb8003400)=%x func=%s line=%d\n",REG32(0xb8003400),__FUNCTION__,__LINE__);      	
	mdelay(10);
	REG32(0xb8003400) &= ~0x1; // PCI RESET RELEASE
	DBG("REG32(0xb8003400)=%x func=%s line=%d\n",REG32(0xb8003400),__FUNCTION__,__LINE__);      	
	return 1;
}

#else //RTL865

#define MACCR                               (0xbc803000)
#define MACMR                               (0xbc803004)
#define SYS_CLK_MASK                        (0x7 << 16)


int rtl_pci_reset(void){
	u32 tmp, divisor, scr;	
	u32 ratio[]={5,5,4,5,4,4,3,2};	/*check Datasheet SCR and MACCR. */

	tmp = REG32(MACCR)&0xFCFF8FFF; 
	REG32(MACMR) = REG32(MACMR) & ~SYS_CLK_MASK;

	scr=(REG32(MACMR)>>16)&7;	

	divisor=ratio[scr];    
	{
		volatile unsigned int i;
		DBG("[PCI] Reset Bridge ..... ");

		/*
			RyanYeh (2005/6/15):

				In PCI reset test,
					50000 loops:	about 5ms, bouncing found (might timer interrupt re-trigger this loop).
					40000 loops:	about 4ms, bouncing found (might timer interrupt re-trigger this loop).
					30000 loops:	about 3ms, no bouncing found.
					20000 loops:	about 2ms, no bouncing found.
					10000 loops:	about 1ms, no bouncing found.
					5000 loops:	about 0.5ms, no bouncing found.
					1 loop:		3.82 us

					Summary:
						(1) PCI spec. require 1 ms for PCI reset signal.
						(2) In RTL865xB, 10000 loops for PCI_RST is needed.
						(3) In RTL865xC, it would be modified by JB.
 		*/
 		#define RTL865XB_PCI_RESET_LOOPCNT	10000

		for ( i = 0 ; i < RTL865XB_PCI_RESET_LOOPCNT ; i ++ )
		{
			REG32(MACCR) = tmp|(divisor<<12)|(1<<15);	/* reset PCI bridge */
		}
		
		DBG("Finish!\n");
	}

	/* patch: set PCI clk = /6 */
	/* divisor=5; */
	REG32(MACCR) = tmp|(divisor<<12);
	return divisor;
}
#endif

/* scan pci bus, assign Memory & IO space to PCI card, 
	and call init_XXXX to enable & register PCI device
	rtn 0:ok, else:fail */

int init_rtl8181pci(void)
{
  	int function_num;
	u32 tmp;
//	REG32(0xbc805038)|=0x80000000;
//	REG32(0xb8003400) = 0x2;  /*little endian*/
	tmp=rtl_pci_isLinuxCompliantEndianMode();
	DBG("NEW PCI Driver...isLinuxCompliantEndianMode=%s\n",(tmp==2)?"True(Little Endian)":"False(Big Endian)");
	rtl_pci_reset();
	

#if 0
	printk("SLOT 0: %x\n", REG32(PCI_SLOT0_CONFIG_BASE));
	printk("SLOT 1: %x\n", REG32(PCI_SLOT1_CONFIG_BASE));
	printk("SLOT 2: %x\n", REG32(PCI_SLOT2_CONFIG_BASE));
	printk("SLOT 3: %x\n", REG32(PCI_SLOT3_CONFIG_BASE));
//	printk("REG32(0xbd012004)&0x10000=%x\n",REG32(0xbd012004)&0x10000);
#endif	

  	
    //printk("Probe PCI Bus : There must be one device at the slot.\n");
    
    memset(pci_slot, 0, 4*PCI_SLOT_NUMBER*8);

    function_num = rtl_pci_scan_slot();

    if (function_num==0) {
    	DBG("No PCI device exist!!\n");
    	return -1;
    };
	
    //auto assign resource
    if (rtl_pci_assign_resource()) {
    	DBG("PCI Resource assignment failed!\n");
    	return -2;
    };

    DBG("Find Total %d PCI functions\n", function_num);
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

	

	if(rtl_pci_big_endian)
	{
		addr |= (where&~0x3);
		tmp = REG32(addr);
		for (addr=0;addr<(3-shift);addr++)
			tmp = tmp >>8;
		*value = (u8)tmp;
	}
	else
	{
		//*value = REG8(addr+where);
		addr |= (where&0xfffffffc);
		tmp = REG32(addr);
		for (addr=0;addr<shift;addr++)
			tmp = tmp >>8;
		*value = (u8)tmp;		
	}

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

	if(rtl_pci_big_endian)
	{
		tmp=REG32(addr);
		if (where&0x2)
			*value = (u16)(tmp>>16);
		else
			*value = (u16)(tmp);
	}
	else
	{
		tmp=REG32(addr);
		if(where&0x2)
			*value=le16_to_cpu((u16)(tmp));
		else
			*value=le16_to_cpu((u16)(tmp>>16));
	}
	
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

	if(rtl_pci_big_endian)
	{
		*value = REG32(addr+where);		
	}
	else
	{
		*value = le32_to_cpu(REG32(addr+where));		
	}
	

//	*value = REG32(addr+where);
	
	return PCIBIOS_SUCCESSFUL;
}


static int rtlpci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
	u32 tmp;
	u32 addr;
	u32 shift=(where & 0x3);	
	
	
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
	

	
	if(rtl_pci_big_endian)
	{
		u32 newval=(u32)value,mask=0xff;
		addr |= (where&0xfffffffc);
		tmp = REG32(addr);
		newval <<= (8*(shift));
		mask <<= (8*(shift));
		REG32(addr)=(tmp&(~mask))|newval;	

	}
	else
	{
		//*value = REG8(addr+where);
		u32 newval=(u32)value,mask=0xff;
		addr |= (where&0xfffffffc);
		tmp = REG32(addr);
		newval <<= (8*(3-shift));
		mask <<= (8*(3-shift));
		REG32(addr)=(tmp&(~mask))|newval;
	}
	
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

	if(rtl_pci_big_endian)
	{
		u32 tmp;
		addr |= (where&0xfffffffc);
		tmp = REG32(addr);
		if(where&2)			
			REG32(addr)=(tmp&0xffff)|(((u32)(value)<<16));			
		else
			REG32(addr)=(tmp&0xffff0000)|((u32)(value));
	}
	else
	{
		u32 tmp;
		addr |= (where&0xfffffffc);
		tmp = REG32(addr);
		if(where&2)			
			REG32(addr)=(tmp&0xffff0000)|((u32)(le16_to_cpu(value)));			
		else
			REG32(addr)=(tmp&0xffff)|((u32)((le16_to_cpu(value))<<16));
			
	}	

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


	if(rtl_pci_big_endian)
	{
		REG32(addr+where)=value;		
	}
	else
	{
		REG32(addr+where)=le32_to_cpu(value);
	}
		
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


#define PCI_IO_START      0x1be00000
#define PCI_IO_END        0x1befffff
#define PCI_MEM_START     0x1bf00000
#define PCI_MEM_END       0x1bffffff





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
#if 0	
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
		DBG("pcibios_fixup_resources IO form %x to %x\n",io_addr,new_io_addr);
		
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
		dev->irq = PCI_ILEV; // fix irq
	}
}

/*	assign memory location to MEM & IO space
	return 0:OK, else:fail */

#ifdef CONFIG_RTL865XC

int rtl_pci_assign_resource(void)
{
  	int i,j, slot, func, BARnum;
 	int mem_space_size_array[2][(PCI_SLOT_NUMBER/2)*8*6][2]; //[0]:store device index, [1]:store resource size
 	int io_space_size_array[2][(PCI_SLOT_NUMBER/2)*8*6][2]; //[0]:store device index, [1]:store resource size
  	int mem_idx[2]={0};
	int io_idx[2]={0};
	int total_size[2]={0};
	int tmp;
  	u16 config_command[PCI_SLOT_NUMBER][8];  	//record config space resource usage, 


    memset(mem_space_size_array, 0, sizeof(mem_space_size_array));
    memset(io_space_size_array, 0, sizeof(io_space_size_array));
    memset(config_command, 0, sizeof(config_command));
    
    //collect resource

    for (slot=0;slot<PCI_SLOT_NUMBER;slot++) {

    	
      if (pci_slot[slot][0]==0) continue;  //this slot is null      
      if (pci_slot[slot][0]->Vendor_Device_ID==0) continue;  //this slot is null      
      for (func=0;func<8;func++) {
        if (pci_slot[slot][func]==0) continue;
        pci_slot[slot][func]->Status = ENABLE;
        for (BARnum=0;BARnum<6;BARnum++) {
		int bus=slot>>1;
      		if (pci_slot[slot][func]->BAR[BARnum].Type==MEM_SPACE){  //memory space
      			DBG("memory mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
      		    config_command[slot][func] |= CMD_MEM_SPACE;  //this device use Memory
      		    mem_space_size_array[bus][mem_idx[bus]][0] = (slot<<16)|(func<<8)|(BARnum<<0);
      		    mem_space_size_array[bus][mem_idx[bus]][1] = pci_slot[slot][func]->BAR[BARnum].Size;
      		    mem_idx[bus]++;
      		} else if (pci_slot[slot][func]->BAR[BARnum].Type==IO_SPACE){  //io space
	      		DBG("io mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
      		    config_command[slot][func] |= CMD_IO_SPACE;  //this device use IO
      		    io_space_size_array[bus][io_idx[bus]][0] = (slot<<16)|(func<<8)|(BARnum<<0);
      		    io_space_size_array[bus][io_idx[bus]][1] = pci_slot[slot][func]->BAR[BARnum].Size;
      		    io_idx[bus]++;
      		}
      		else
      		{
//      			printk("unknow mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
      		}
	    }  //for (BARnum=0;BARnum<6;BARnum++) 
      }  //for (func=0;func<8;func++)
    } //for (slot=0;slot<PCI_SLOT_NUMBER;slot++)

    //sort by size
	bubble_sort(mem_space_size_array, mem_idx);
	bubble_sort(io_space_size_array, io_idx);   


    //check mem total size
	for(j=0;j<2;j++)
	{
	    for (i=0;i<mem_idx[j];i++) {  	
	    	tmp = mem_space_size_array[j][i][1]-1;
	        total_size[j] = (total_size[j]+tmp)&(~tmp);     
	        total_size[j] = total_size[j] + mem_space_size_array[j][i][1];
	    }
	}

    if ((total_size[0]>PCI_MEM_SPACE_SIZE0)||(total_size[1]>PCI_MEM_SPACE_SIZE1) ) {
	DBG("lack of memory map space resource \n");
	return -1;  //lack of memory space resource
    }

    //check io total size
	for(j=0;j<2;j++)
	{
	    total_size[j]=0;
	    for (i=0;i<io_idx[j];i++) {     	
	    	tmp = io_space_size_array[j][i][1]-1;
	        total_size[j] = (total_size[j]+tmp)&(~tmp);     
	        total_size[j] = total_size[j] + io_space_size_array[j][i][1];
	    }
   	} 

    if ((total_size[0]>PCI_IO_SPACE_SIZE0)||(total_size[1]>PCI_IO_SPACE_SIZE1) ) 
    {
	DBG("lack of io map space resource\n");
		return -2;  //lack of io space resource
    }
    
    //assign memory space
	for(j=0;j<2;j++)
	{
	    total_size[j] = 0;
	    for (i=0;i<mem_idx[j];i++) {
	    	unsigned int config_base;     	
	    	tmp = mem_space_size_array[j][i][1]-1;
	        total_size[j] = (total_size[j]+tmp)&(~tmp);        
	        tmp = mem_space_size_array[j][i][0];
	        //assign to struct
	        if (pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Type!= MEM_SPACE)
	        	continue;
	   
		if(j==0)
		{
		        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_MEM_BASE0+total_size[j];
		 }
		else
		{
		        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_MEM_BASE1+total_size[j];
		}	
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
	//        REG32(tmp) = (PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF;  //map to physical address
		if(j==0)
		{
		        rtlpci_write_config_endian_free(tmp,(PCI_SLOT_MEM_BASE0+total_size[j])&0x1FFFFFFF);
		        DBG("assign mem base %x at %x size=%d\n",(PCI_SLOT_MEM_BASE0+total_size[j])&0x1FFFFFFF,tmp,mem_space_size_array[j][i][1]);
		}
		else
		{
		        rtlpci_write_config_endian_free(tmp,(PCI_SLOT_MEM_BASE1+total_size[j])&0x1FFFFFFF);
		        DBG("assign mem base %x at %x size=%d\n",(PCI_SLOT_MEM_BASE1+total_size[j])&0x1FFFFFFF,tmp,mem_space_size_array[j][i][1]);
		}
	        total_size[j] = total_size[j] + mem_space_size_array[j][i][1];  //next address

	    };
	    //assign IO space
	    total_size[j] = 0;
	    for (i=0;i<io_idx[j];i++) {
	    	unsigned int config_base;    	
	    	tmp = io_space_size_array[j][i][1]-1;
	        total_size[j] = (total_size[j]+tmp)&(~tmp);        
	        tmp = io_space_size_array[j][i][0];
	        //assign to struct
	        if (pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Type!= IO_SPACE)
	        	continue;

		if(j==0)
		{
		       pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_IO_BASE0+total_size[j];
		}
		else
		{
		        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_IO_BASE1+total_size[j];
		}

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
	        
	//        REG32(tmp) = (PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF;  //map to physical address
		if(j==0)
		{
			 rtlpci_write_config_endian_free(tmp,(PCI_SLOT_IO_BASE0+total_size[j])&0x1FFFFFFF);	 
			DBG("assign I/O base %x~%x at %x size=%d\n",(PCI_SLOT_IO_BASE0+total_size[j])&0x1FFFFFFF,((PCI_SLOT_IO_BASE0+total_size[j])&0x1FFFFFFF)+io_space_size_array[j][i][1]-1,tmp,io_space_size_array[j][i][1]);
		}
		else
		{
			 rtlpci_write_config_endian_free(tmp,(PCI_SLOT_IO_BASE1+total_size[j])&0x1FFFFFFF);	 
			DBG("assign I/O base %x~%x at %x size=%d\n",(PCI_SLOT_IO_BASE1+total_size[j])&0x1FFFFFFF,((PCI_SLOT_IO_BASE1+total_size[j])&0x1FFFFFFF)+io_space_size_array[j][i][1]-1,tmp,io_space_size_array[j][i][1]);
		}
		total_size[j] += io_space_size_array[j][i][1];  //next address	
	       
	        
	    }
	}
DBG("%s %d\n",__FUNCTION__,__LINE__);	    
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
              +func*64*4;		


			   //function
        //set Max_Lat & Min_Gnt, irq
        //printk("1 REG32(tmp+PCI_CONFIG_INT_LINE) = %x\n",REG32(tmp+PCI_CONFIG_INT_LINE));
		//REG32(tmp+PCI_CONFIG_INT_LINE) = 0x40200100|(PCI_ILEV); //IRQ level
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_INT_LINE,0x40200100|(PCI_ILEV));
		//printk("2 REG32(tmp+PCI_CONFIG_INT_LINE) = %x\n",REG32(tmp+PCI_CONFIG_INT_LINE));

		//enable cache line size, lantancy
		//REG32(tmp+PCI_CONFIG_CACHE) = (REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004;  //32 byte cache, 20 latency
		//printk("(REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004=%x\n",(REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004);
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_CACHE,(rtlpci_read_config_endian_free(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004);
	

		
        //set command register
        //REG32(tmp+PCI_CONFIG_COMMAND) = (REG32(tmp+PCI_CONFIG_COMMAND)&0xFFFF0000)|config_command[slot][func]
        //					|CMD_BUS_MASTER|CMD_WRITE_AND_INVALIDATE|CMD_PARITY_ERROR_RESPONSE;
	// this delay is necessary to 
       	// work around pci configuration timing issue
       	config_base = 0;	       	
       	while(config_base < 1000){
       	config_base++;
       	} 


	if(!rtl_pci_big_endian){

		u32 command_status=rtlpci_read_config_endian_free(tmp+PCI_CONFIG_COMMAND);
		command_status=(config_command[slot][func]|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE)|(command_status&0xffff0000);
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_COMMAND,command_status);		

	}
	else
	{

       	REG16(tmp+PCI_CONFIG_COMMAND) = config_command[slot][func]
        	|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE;//|CMD_WRITE_AND_INVALIDATE;

	}


      };
   }

    return 0;
}


#else

int rtl_pci_assign_resource(void)
{
  	int i, slot, func, BARnum;
  	int mem_space_size[PCI_SLOT_NUMBER*8*6][2]; //[0]:store device index, [1]:store resource size
  	int io_space_size[PCI_SLOT_NUMBER*8*6][2]; //[0]:store device index, [1]:store resource size
  	int mem_idx, io_idx, total_size, tmp;
  	u16 config_command[PCI_SLOT_NUMBER][8];  	//record config space resource usage, 
  
	memset(mem_space_size, 0, sizeof(mem_space_size));
	memset(io_space_size, 0, sizeof(io_space_size));
	memset(config_command, 0, sizeof(config_command));
    
	//collect resource
	mem_idx = io_idx =0;
	for (slot=0;slot<PCI_SLOT_NUMBER;slot++)
	{
		if (IS_RTL865XB_AB)
		{			
#ifndef CONFIG_RTL865XB_PCI_SLOT0
			if(slot==0) continue;
#endif
#ifndef CONFIG_RTL865XB_PCI_SLOT1
			if(slot==1) continue;
#endif
#ifndef CONFIG_RTL865XB_PCI_SLOT2
			if(slot==2) continue;
#endif
#ifndef CONFIG_RTL865XB_PCI_SLOT3
			if(slot==3) continue;
#endif
		}
    	
		if (pci_slot[slot][0]==0) continue;  //this slot is null      
		if (pci_slot[slot][0]->Vendor_Device_ID==0) continue;  //this slot is null      

		for (func=0;func<8;func++)
		{
			if (pci_slot[slot][func]==0) continue;

			pci_slot[slot][func]->Status = ENABLE;

			for (BARnum=0;BARnum<6;BARnum++)
			{
				if (pci_slot[slot][func]->BAR[BARnum].Type==MEM_SPACE)
				{  //memory space
					DBG("memory mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
					config_command[slot][func] |= CMD_MEM_SPACE;  //this device use Memory
					mem_space_size[mem_idx][0] = (slot<<16)|(func<<8)|(BARnum<<0);
					mem_space_size[mem_idx][1] = pci_slot[slot][func]->BAR[BARnum].Size;
					mem_idx++;
				} else if (pci_slot[slot][func]->BAR[BARnum].Type==IO_SPACE)
				{  //io space
					DBG("io mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
					config_command[slot][func] |= CMD_IO_SPACE;  //this device use IO
					io_space_size[io_idx][0] = (slot<<16)|(func<<8)|(BARnum<<0);
					io_space_size[io_idx][1] = pci_slot[slot][func]->BAR[BARnum].Size;
					io_idx++;
				}
	      			else
	      			{
//					printk("unknow mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
				}
			}  //for (BARnum=0;BARnum<6;BARnum++) 
		}  //for (func=0;func<8;func++)
	} //for (slot=0;slot<PCI_SLOT_NUMBER;slot++)

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
	DBG("lack of memory map space resource \n");
	return -1;  //lack of memory space resource
    }

    //check io total size
    total_size = 0;
    for (i=0;i<io_idx;i++) {     	
    	tmp = io_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);     
        total_size = total_size + io_space_size[i][1];
    }
    
    if (total_size>PCI_IO_SPACE_SIZE) 
    {
	DBG("lack of io map space resource\n");
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
        if (pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Type!= MEM_SPACE)
        	continue;
        
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
//        REG32(tmp) = (PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF;  //map to physical address
        rtlpci_write_config_endian_free(tmp,(PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF);
        DBG("assign mem base %x~%x at %x size=%d\n",(PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF,((PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF)+mem_space_size[i][1]-1,tmp,mem_space_size[i][1]);
	
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
        if (pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Type!= IO_SPACE)
        	continue;

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
        
//        REG32(tmp) = (PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF;  //map to physical address
	 rtlpci_write_config_endian_free(tmp,(PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF);
	 
	printk("assign I/O base %x~%x at %x size=%d\n",(PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF,((PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF)+io_space_size[i][1]-1,tmp,io_space_size[i][1]);
	total_size += io_space_size[i][1];  //next address	
       
        
    }
    
    //enable device
    for (slot=0;slot<PCI_SLOT_NUMBER;slot++) {


	if (IS_RTL865XB_AB)
	{	
	#ifndef CONFIG_RTL865XB_PCI_SLOT0
			if(slot==0) continue;
	#endif
	#ifndef CONFIG_RTL865XB_PCI_SLOT1
			if(slot==1) continue;
	#endif
	#ifndef CONFIG_RTL865XB_PCI_SLOT2
			if(slot==2) continue;
	#endif
	#ifndef CONFIG_RTL865XB_PCI_SLOT3
			if(slot==3) continue;
	#endif
	}
    	
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
		//REG32(tmp+PCI_CONFIG_INT_LINE) = 0x40200100|(PCI_ILEV); //IRQ level
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_INT_LINE,0x40200100|(PCI_ILEV));
		//printk("2 REG32(tmp+PCI_CONFIG_INT_LINE) = %x\n",REG32(tmp+PCI_CONFIG_INT_LINE));

		//enable cache line size, lantancy
		//REG32(tmp+PCI_CONFIG_CACHE) = (REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004;  //32 byte cache, 20 latency
		//printk("(REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004=%x\n",(REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004);
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_CACHE,(rtlpci_read_config_endian_free(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004);
	
		
		
        //set command register
        //REG32(tmp+PCI_CONFIG_COMMAND) = (REG32(tmp+PCI_CONFIG_COMMAND)&0xFFFF0000)|config_command[slot][func]
        //					|CMD_BUS_MASTER|CMD_WRITE_AND_INVALIDATE|CMD_PARITY_ERROR_RESPONSE;
	// this delay is necessary to 
       	// work around pci configuration timing issue
       	config_base = 0;	       	
       	while(config_base < 1000){
       	config_base++;
       	} 


	if(!rtl_pci_big_endian){
		u32 command_status=rtlpci_read_config_endian_free(tmp+PCI_CONFIG_COMMAND);
		command_status=(config_command[slot][func]|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE)|(command_status&0xffff0000);
		printk("%s %d command_status=%x\n",__FUNCTION__,__LINE__,command_status);
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_COMMAND,command_status);		
	}
	else
	{
       	REG16(tmp+PCI_CONFIG_COMMAND) = config_command[slot][func]
        	|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE;//|CMD_WRITE_AND_INVALIDATE;
		DBG("%s %d command_status=%x\n",__FUNCTION__,__LINE__,config_command[slot][func]);        	
	}


      };
    };

    return 0;
}

#endif
u8 rtl865x_pci_ioread8(u32 addr){ 	
 	//For legacy 8650's linux incompliant PCI access endian mode 	
	if(rtl_pci_big_endian){
		u8 tmp;
		u8 swap[4]={3,2,1,0};
		int diff = swap[addr&0x3];
#ifdef RTL865X_PCI_DEBUG		
		printk("R8 0x%08x->", addr);
#endif
		addr=addr&(~0x3);
#ifdef RTL865X_PCI_DEBUG
		printk("0x%08x=", addr);
#endif
		tmp= *(volatile u8*)(addr+diff);
#ifdef RTL865X_PCI_DEBUG
		printk("0x%02x\n", tmp);
#endif
		return tmp;
	}
	return (*(volatile unsigned char *)(addr));
}

 u16 rtl865x_pci_ioread16(u32 addr){
 	//For legacy 8650's linux incompliant PCI access endian mode 	
	if(rtl_pci_big_endian){
		u16 tmp;
		int diff=2-(addr&0x3);
#ifdef RTL865X_PCI_DEBUG
		printk("R16 0x%08x->", addr);
#endif
		addr=addr&(~0x3);
#ifdef RTL865X_PCI_DEBUG
		printk("0x%08x=", addr);
#endif
		tmp= *(volatile u16*)(addr+diff);
#ifdef RTL865X_PCI_DEBUG
		printk("0x%04x\n", tmp);
#endif
		return tmp;
	}
	return cpu_to_le16((*(volatile unsigned short *)(addr)));
}

u32 rtl865x_pci_ioread32(u32 addr){

 	//For legacy 8650's linux incompliant PCI access endian mode 	
	if(rtl_pci_big_endian){
		u32 tmp;
		tmp= *(volatile u32*)(addr);		
//		printk("[B]read32 from 0x%x return value=0x%x\n",addr,tmp);		
		return tmp;
	}
#if 0
	{
		u32 temp;
		printk("[L]read32 from 0x%x\n",addr);		
		temp=cpu_to_le32((*(volatile unsigned int *)(addr)));
		printk("[L]read32 from 0x%x return value=0x%x\n",addr,temp);
		return temp;
	
	}
#else	
	return cpu_to_le32((*(volatile unsigned int *)(addr)));
#endif
}


void rtl865x_pci_iowrite8(u32 addr, u8 val){
	if(rtl_pci_big_endian)
		(*(volatile unsigned char*)(addr|0xa0000000))=val;
	else
		(*(volatile unsigned char *)(addr|0xa0000000)) = (val);
}

 void rtl865x_pci_iowrite16(u32 addr, u16 val){
	if(rtl_pci_big_endian)
		(*(volatile unsigned short*)(addr|0xa0000000))=val;
	else
		(*(volatile unsigned short *)(addr|0xa0000000)) = cpu_to_le16(val);
}

 void rtl865x_pci_iowrite32(u32 addr, u32 val){
	if(rtl_pci_big_endian)
	{
		(*(volatile unsigned long*)(addr|0xa0000000))=val;
//		printk("[B]write32 to 0x%x set value = 0x%x\n",addr,val);		
	}	
	else
	{
//		printk("[L]write32 to 0x%x set value = 0x%x\n",addr,val);
		(*(volatile unsigned long *)(addr|0xa0000000)) = cpu_to_le32(val);
	}

}
#ifndef CONFIG_WIRELESS_LAN_MODULE
EXPORT_SYMBOL(rtl865x_pci_iowrite32);
EXPORT_SYMBOL(rtl865x_pci_iowrite16);
EXPORT_SYMBOL(rtl865x_pci_iowrite8);
EXPORT_SYMBOL(rtl865x_pci_ioread32);
EXPORT_SYMBOL(rtl865x_pci_ioread16);
EXPORT_SYMBOL(rtl865x_pci_ioread8);
#endif
#endif /* CONFIG_PCI */


