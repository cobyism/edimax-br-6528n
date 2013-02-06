#include "utility.h"
#include "rtk.h"

#include <asm/system.h>
#include <rtl8196x/asicregs.h>

//#define UTILITY_DEBUG 1
#define NEED_CHKSUM 1

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE	
#define BANK1_BOOT 1
#define BANK2_BOOT 2

#define GOOD_BANK_MARK_MASK 0x80000000  //goo abnk mark must set bit31 to 1

#define NO_IMAGE_BANK_MARK 0x80000000  
#define OLD_BURNADDR_BANK_MARK 0x80000001 
#define BASIC_BANK_MARK 0x80000002           
#define FORCEBOOT_BANK_MARK 0xFFFFFFF0  //means always boot/upgrade in this bank

#define IN_TFTP_MODE 0
#define IN_BOOTING_MODE 1


int boot_bank=0; 
unsigned long  bank_mark=0;

#endif

unsigned long  glexra_clock=200*1000*1000;
unsigned long mem2x_clksel_table[7]={ 131250000, 156250000, 250000000, 300000000, 312500000, 337500000, 387500000};
//------------------------------------------------------------------------------------------
int enable_10M_power_saving(int phyid , int regnum,int data)
{   
    	unsigned int uid,tmp;  
     	rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );
     	uid=tmp;
     	uid =data;
    	rtl8651_setAsicEthernetPHYReg( phyid, regnum, uid );
    	rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );
    	uid=tmp;
	return 0;
}
//------------------------------------------------------------------------------------------
//check img
unsigned int gCHKKEY_HIT=0;
unsigned int gCHKKEY_CNT=0;
#if defined(CONFIG_NFBI)
// return,  0: not found, 1: linux found, 2:linux with root found
int check_system_image(unsigned long addr,IMG_HEADER_Tp pHeader)
{
	// Read header, heck signature and checksum
	int i, ret=0;
	unsigned short sum=0, *word_ptr;
	unsigned short length=0;
	unsigned short temp16=0;

	if(gCHKKEY_HIT==1)	return 0;
	
    	/*check firmware image.*/
	word_ptr = (unsigned short *)pHeader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
		*word_ptr = *((unsigned short *)(addr + i));

	if (!memcmp(pHeader->signature, FW_SIGNATURE, SIG_LEN))
		ret=1;
//	else if  (!memcmp(pHeader->signature, FW_SIGNATURE_WITH_ROOT, SIG_LEN))
	else if  (!memcmp(pHeader->signature, FW_SIGNATURE_WITH_ROOT, 2))//EDX check "cr" only
		ret=2;
	else 
		dprintf("no sys signature at %X!\n",addr);
#if defined(NEED_CHKSUM)	
	if (ret) {
		for (i=0; i<pHeader->len; i+=2) {
			sum += *((unsigned short *)(addr + sizeof(IMG_HEADER_T) + i));
			//prom_printf("x=%x\n", (addr + sizeof(IMG_HEADER_T) + i));
		}

		if ( sum ) {
			//SYSSR: checksum done, but fail
			REG32(NFBI_SYSSR)= (REG32(NFBI_SYSSR)|0x8000) & (~0x4000);
			dprintf("sys checksum error at %X!\n",addr);
			ret=0;
		}
		else {
			//SYSSR: checksum done and OK
			REG32(NFBI_SYSSR)= REG32(NFBI_SYSSR) | 0xc000;
		}
	}
#else
	//SYSSR: checksum done and OK
	REG32(NFBI_SYSSR)= REG32(NFBI_SYSSR) | 0xc000;
#endif
	return (ret);
}
#else
// return,  0: not found, 1: linux found, 2:linux with root found
int check_system_image(unsigned long addr,IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp setting_header)
{
	// Read header, heck signature and checksum
	int i, ret=0;
	unsigned short sum=0, *word_ptr;
	unsigned short length=0;
	unsigned short temp16=0;
	char image_sig_check[1]={0};
	char image_sig[4]={0};
	char image_sig_root[4]={0};
	if(gCHKKEY_HIT==1)
		return 0;
        /*check firmware image.*/
	word_ptr = (unsigned short *)pHeader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
		*word_ptr = rtl_inw(addr + i);	

	memcpy(image_sig, FW_SIGNATURE, SIG_LEN);
	memcpy(image_sig_root, FW_SIGNATURE_WITH_ROOT, SIG_LEN);

	if (!memcmp(pHeader->signature, image_sig, SIG_LEN))
		ret=1;
//	else if  (!memcmp(pHeader->signature, image_sig_root, SIG_LEN))
	else if  (!memcmp(pHeader->signature, image_sig_root, 2)) //EDX check "cr" only
		ret=2;
	else{
		prom_printf("no sys signature at %X!\n",addr-FLASH_BASE);
	}		
	//prom_printf("ret=%d  sys signature at %X!\n",ret,addr-FLASH_BASE);
	
	if (ret) {
		for (i=0; i<pHeader->len; i+=2) {
#if 1  //slowly
			gCHKKEY_CNT++;
			if( gCHKKEY_CNT>ACCCNT_TOCHKKEY)
			{	gCHKKEY_CNT=0;
				if ( user_interrupt(0)==1 )  //return 1: got ESC Key
				{
					//prom_printf("ret=%d  ------> line %d!\n",ret,__LINE__);
					return 0;
				}
			}
#else  //speed-up, only support UART, not support GPIO
			if((Get_UART_Data()==ESC)  || (Get_GPIO_SW_IN()!=0))
			{	gCHKKEY_HIT=1; 
				return 0;
			}
#endif
#if defined(NEED_CHKSUM)	
			sum += rtl_inw(addr + sizeof(IMG_HEADER_T) + i);
#endif
		}	
#if defined(NEED_CHKSUM)			
		if ( sum ) {
			//prom_printf("ret=%d  ------> line %d!\n",ret,__LINE__);
			ret=0;
		}
#endif		
	}
	//prom_printf("ret=%d  sys signature at %X!\n",ret,addr-FLASH_BASE);

	return (ret);
}

int check_rootfs_image(unsigned long addr)
{
	// Read header, heck signature and checksum
	int i;
	unsigned short sum=0, *word_ptr;
	unsigned long length=0;
	unsigned char tmpbuf[16];	
	#define SIZE_OF_SQFS_SUPER_BLOCK 640
	#define SIZE_OF_CHECKSUM 2
	#define OFFSET_OF_LEN 2
	
	if(gCHKKEY_HIT==1)
		return 0;
	
	word_ptr = (unsigned short *)tmpbuf;
	for (i=0; i<16; i+=2, word_ptr++)
		*word_ptr = rtl_inw(addr + i);

	if ( memcmp(tmpbuf, SQSH_SIGNATURE, SIG_LEN) && memcmp(tmpbuf, SQSH_SIGNATURE_LE, SIG_LEN)) {
		prom_printf("no rootfs signature at %X!\n",addr-FLASH_BASE);
		return 0;
	}

	length = *(((unsigned long *)tmpbuf) + OFFSET_OF_LEN) + SIZE_OF_SQFS_SUPER_BLOCK + SIZE_OF_CHECKSUM;
	for (i=0; i<length; i+=2) {
#if 1  //slowly
			gCHKKEY_CNT++;
			if( gCHKKEY_CNT>ACCCNT_TOCHKKEY)
			{	gCHKKEY_CNT=0;
				if ( user_interrupt(0)==1 )  //return 1: got ESC Key
					return 0;
			}
#else  //speed-up, only support UART, not support GPIO.
			if((Get_UART_Data()==ESC)  || (Get_GPIO_SW_IN()!=0))
			{	gCHKKEY_HIT=1; 
				return 0;
			}
#endif			
#if defined(NEED_CHKSUM)	
		sum += rtl_inw(addr + i);
#endif
	}
#if 0 //EDX 
#if defined(NEED_CHKSUM)		
	if ( sum ) {
		prom_printf("rootfs checksum error at %X!\n",addr-FLASH_BASE);
		return 0;
	}	
#endif	
#endif
	return 1;
}
static int check_image_header(IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp psetting_header,unsigned long bank_offset)
{
	int i,ret=0;
	//flash mapping
	return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET+bank_offset;
	ret = check_system_image((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET+bank_offset,pHeader, psetting_header);

	if(ret==0) {
		return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET2+bank_offset;		
		ret=check_system_image((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET2+bank_offset,  pHeader, psetting_header);
	}
	if(ret==0) {
		return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET3+bank_offset;				
		ret=check_system_image((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET3+bank_offset,  pHeader, psetting_header);
	}			

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE	
	i=CONFIG_LINUX_IMAGE_OFFSET_START;	
	while(i<=CONFIG_LINUX_IMAGE_OFFSET_END && (0==ret))
	{
		return_addr=(unsigned long)FLASH_BASE+i+bank_offset; 
		if(CODE_IMAGE_OFFSET == i || CODE_IMAGE_OFFSET2 == i || CODE_IMAGE_OFFSET3 == i){
			i += CONFIG_LINUX_IMAGE_OFFSET_STEP; 
			continue;
		}
		ret = check_system_image((unsigned long)FLASH_BASE+i+bank_offset, pHeader, psetting_header);
		i += CONFIG_LINUX_IMAGE_OFFSET_STEP; 
	}
#endif

	if(ret==2)
        {
                ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET+bank_offset);
                if(ret==0)
                	ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET+ROOT_FS_OFFSET_OP1+bank_offset);
                if(ret==0)
                	ret=check_rootfs_image((unsigned long)FLASH_BASE+ROOT_FS_OFFSET+ROOT_FS_OFFSET_OP1+ROOT_FS_OFFSET_OP2+bank_offset);

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
		i = CONFIG_ROOT_IMAGE_OFFSET_START;
		while((i <= CONFIG_ROOT_IMAGE_OFFSET_END) && (0==ret))
		{
			if( ROOT_FS_OFFSET == i ||
			    (ROOT_FS_OFFSET + ROOT_FS_OFFSET_OP1) == i ||
		            (ROOT_FS_OFFSET + ROOT_FS_OFFSET_OP1 + ROOT_FS_OFFSET_OP2) == i){
				i += CONFIG_ROOT_IMAGE_OFFSET_STEP;
				continue;
			}
			ret = check_rootfs_image((unsigned long)FLASH_BASE+i+bank_offset);
			i += CONFIG_ROOT_IMAGE_OFFSET_STEP;
		}
#endif
	}
	return ret;
}

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE	

int check_system_image_1(unsigned long addr,IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp setting_header)
{
	// Read header, heck signature and checksum
	int i,ret=0;
	unsigned short sum=0, *word_ptr;
	unsigned short length=0;
	unsigned short temp16=0;
	char image_sig_check[1]={0};
	char image_sig[4]={0};
	char image_sig_root[4]={0};
	
        /*check firmware image.*/
	word_ptr = (unsigned short *)pHeader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
		*word_ptr = rtl_inw(addr + i);	

	memcpy(image_sig, FW_SIGNATURE, SIG_LEN);
	memcpy(image_sig_root, FW_SIGNATURE_WITH_ROOT, SIG_LEN);

	if (!memcmp(pHeader->signature, image_sig, SIG_LEN))
		ret=1;
	else if  (!memcmp(pHeader->signature, image_sig_root, SIG_LEN))
		ret=2;
	else{
		//prom_printf("no sys signature at %X!\n",addr-FLASH_BASE);
	}		
	//prom_printf("ret=%d  sys signature at %X!\n",ret,addr-FLASH_BASE);

	if (ret) {
		for (i=0; i<pHeader->len; i+=2) {
			
#if defined(NEED_CHKSUM)	
			sum += rtl_inw(addr + sizeof(IMG_HEADER_T) + i);
#endif
		}	
#if defined(NEED_CHKSUM)			
		if ( sum ) {
			//prom_printf("ret=%d  ------> line %d!\n",ret,__LINE__);
			ret=0;
		}
#endif		
	}
	//prom_printf("ret=%d  sys signature at %X!\n",ret,addr-FLASH_BASE);

	return (ret);
}

int check_rootfs_image_1(unsigned long addr)
{
	// Read header, heck signature and checksum
	int i;
	unsigned short sum=0, *word_ptr;
	unsigned long length=0;
	unsigned char tmpbuf[16];	
	#define SIZE_OF_SQFS_SUPER_BLOCK 640
	#define SIZE_OF_CHECKSUM 2
	#define OFFSET_OF_LEN 2	
	
	word_ptr = (unsigned short *)tmpbuf;
	for (i=0; i<16; i+=2, word_ptr++)
		*word_ptr = rtl_inw(addr + i);

	if ( memcmp(tmpbuf, SQSH_SIGNATURE, SIG_LEN) && memcmp(tmpbuf, SQSH_SIGNATURE_LE, SIG_LEN)) {
		//prom_printf("no rootfs signature at %X!\n",addr-FLASH_BASE);
		return 0;
	}
	length = *(((unsigned long *)tmpbuf) + OFFSET_OF_LEN) + SIZE_OF_SQFS_SUPER_BLOCK + SIZE_OF_CHECKSUM;
	for (i=0; i<length; i+=2) {			
#if defined(NEED_CHKSUM)	
		sum += rtl_inw(addr + i);
#endif
	}
#if defined(NEED_CHKSUM)		
	if ( sum ) {
		//prom_printf("rootfs checksum error at %X!\n",addr-FLASH_BASE);
		return 0;
	}	
#endif	
	return 1;
}

static int check_image_header_1(IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp psetting_header,unsigned long bank_offset)
{
	int i,ret=0;
//flash mapping
	return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET+bank_offset;
	ret = check_system_image_1((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET+bank_offset,pHeader, psetting_header);

	if(ret==0) {
		return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET2+bank_offset;		
		ret=check_system_image_1((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET2+bank_offset,  pHeader, psetting_header);
	}
	if(ret==0) {
		return_addr = (unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET3+bank_offset;				
		ret=check_system_image_1((unsigned long)FLASH_BASE+CODE_IMAGE_OFFSET3+bank_offset,  pHeader, psetting_header);
	}			

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE	
	i=CONFIG_LINUX_IMAGE_OFFSET_START;	
	while(i<=CONFIG_LINUX_IMAGE_OFFSET_END && (0==ret))
	{
		return_addr=(unsigned long)FLASH_BASE+i+bank_offset; 
		if(CODE_IMAGE_OFFSET == i || CODE_IMAGE_OFFSET2 == i || CODE_IMAGE_OFFSET3 == i){
			i += CONFIG_LINUX_IMAGE_OFFSET_STEP; 
			continue;
		}
		ret = check_system_image_1((unsigned long)FLASH_BASE+i+bank_offset, pHeader, psetting_header);
		i += CONFIG_LINUX_IMAGE_OFFSET_STEP; 
	}
#endif

	if(ret==2)
        {
                ret=check_rootfs_image_1((unsigned long)FLASH_BASE+ROOT_FS_OFFSET+bank_offset);
                if(ret==0)
                	ret=check_rootfs_image_1((unsigned long)FLASH_BASE+ROOT_FS_OFFSET+ROOT_FS_OFFSET_OP1+bank_offset);
                if(ret==0)
                	ret=check_rootfs_image_1((unsigned long)FLASH_BASE+ROOT_FS_OFFSET+ROOT_FS_OFFSET_OP1+ROOT_FS_OFFSET_OP2+bank_offset);

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
		i = CONFIG_ROOT_IMAGE_OFFSET_START;
		while((i <= CONFIG_ROOT_IMAGE_OFFSET_END) && (0==ret))
		{
			if( ROOT_FS_OFFSET == i ||
			    (ROOT_FS_OFFSET + ROOT_FS_OFFSET_OP1) == i ||
		            (ROOT_FS_OFFSET + ROOT_FS_OFFSET_OP1 + ROOT_FS_OFFSET_OP2) == i){
				i += CONFIG_ROOT_IMAGE_OFFSET_STEP;
				continue;
			}
			ret = check_rootfs_image_1((unsigned long)FLASH_BASE+i+bank_offset);
			i += CONFIG_ROOT_IMAGE_OFFSET_STEP;
		}
#endif
	}
	return ret;
}
unsigned long sel_burnbank_offset()
{
	unsigned long burn_offset=0;

	if( ((boot_bank == BANK1_BOOT) && ( bank_mark != FORCEBOOT_BANK_MARK)) ||
	     ((boot_bank == BANK2_BOOT) && ( bank_mark == FORCEBOOT_BANK_MARK))) //burn to bank2
		 burn_offset = CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET;

	return burn_offset;
}

unsigned long get_next_bank_mark()
{
	if( bank_mark < BASIC_BANK_MARK)
		return BASIC_BANK_MARK;
	else if( bank_mark ==  FORCEBOOT_BANK_MARK)	 	
		return bank_mark;
	else
		return bank_mark+1;  
}

unsigned long header_to_mark(int  flag, IMG_HEADER_Tp pHeader)
{
	unsigned long ret_mark=NO_IMAGE_BANK_MARK;
	//mark_dual ,  how to diff "no image" "image with no bank_mark(old)" , "boot with lowest priority"
	if(flag) //flag ==0 means ,header is illegal
	{
		if( (pHeader->burnAddr & GOOD_BANK_MARK_MASK) )
			ret_mark=pHeader->burnAddr;	
		else
			ret_mark = OLD_BURNADDR_BANK_MARK;
	}
	return ret_mark;
}
int check_dualbank_setting(int in_mode)
{	
	int ret1=0,ret2=0,ret=0;
	unsigned long tmp_returnaddr;	
	IMG_HEADER_T tmp_bank_Header,Header,*pHeader=&Header; //0 :bank1 , 1 : bank2
	SETTING_HEADER_T setting_header,*psetting_header=&setting_header;
	unsigned long  tmp_bank_mark1,tmp_bank_mark2; 

	if(in_mode == IN_TFTP_MODE)
		ret1=check_image_header_1(&tmp_bank_Header,psetting_header,0);
	else
		ret1=check_image_header(&tmp_bank_Header,psetting_header,0);

	tmp_returnaddr=return_addr; //record the bank1 addr 

	if(in_mode == IN_TFTP_MODE)
		ret2=check_image_header_1(pHeader,psetting_header,CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET);
	else
		ret2=check_image_header(pHeader,psetting_header,CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET);
	//return_addr has been change to bank2.
	tmp_bank_mark1= header_to_mark(ret1, &tmp_bank_Header);
	tmp_bank_mark2 = header_to_mark(ret2,pHeader);

	if(tmp_bank_mark2 > tmp_bank_mark1)
       {
 	       boot_bank = BANK2_BOOT;
		bank_mark = tmp_bank_mark2;
		ret = ret2;	
       }
      else
      {
		boot_bank = BANK1_BOOT;
		bank_mark = tmp_bank_mark1;
		return_addr = tmp_returnaddr; 
		memcpy(pHeader,&tmp_bank_Header,sizeof(IMG_HEADER_T));
		ret = ret1;
      }	
	//prom_printf("---check dualbank setting boot_bank=%d,bank_mark=%x---\n",boot_bank,bank_mark);  
	return ret;
}
#endif

#endif //mark_nfbi


int check_image(IMG_HEADER_Tp pHeader,SETTING_HEADER_Tp psetting_header)
{
	int ret=0;
#ifdef CONFIG_NFBI
	prom_printf("---NFBI---\n");
#else
 	//only one bank
 #ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE	
 	ret=check_image_header(pHeader,psetting_header,0); 
 #else
       ret = check_dualbank_setting(IN_BOOTING_MODE);
#endif
#endif //end of NFBI else
	return ret;
}

//------------------------------------------------------------------------------------------
//monitor user interrupt
int pollingDownModeKeyword(int key)
{
	int i;
	if  (Check_UART_DataReady() )
	{
		i=Get_UART_Data();
		Get_UART_Data();
		if( i == key )
		{ 	
#if defined(UTILITY_DEBUG)		
			dprintf("User Press ESC Break Key\r\n");
#endif			
			gCHKKEY_HIT=1;
			return 1;
		}
	}
	return 0;
}
#ifdef CONFIG_BOOT_RESET_ENABLE
//EDX start
#if defined(CONFIG_RTL8198)
#define WLAN_LED_PIN 0
#endif
//EDX end
int pollingPressedButton(int pressedFlag)
{
#ifndef CONFIG_NFBI
#ifndef CONFIG_FPGA_PLATFORM
		// polling if button is pressed --------------------------------------
    		if (pressedFlag == -1 ||  pressedFlag == 1) 
		{
#if defined(RTL8196C)
			REG32(RTL_GPIO_MUX) =  REG32(RTL_GPIO_MUX)|0x00300000;
			REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<5) ); //set byte F GPIO7 = gpio
             		REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<5) );  //0 input, 1 out
#endif
#if defined(RTL8198)
//EDX			REG32(RTL_GPIO_MUX) =  0x0c0f;
			REG32(RTL_GPIO_MUX) =  0x0c1f;
			REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG)& (~(1<<25) ); //set byte F GPIO7 = gpio
             		REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) & (~(1<<25) );  //0 input, 1 out
//EDX start
			REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG) & (~(1<<WLAN_LED_PIN) ); 
			REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | ((1<<WLAN_LED_PIN) ); 
			REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) & (~(1<<WLAN_LED_PIN) );  
//EDX end
#endif
		
			if ( Get_GPIO_SW_IN() )			
			{// button pressed
#if defined(UTILITY_DEBUG)			
	    			dprintf("User Press GPIO Break Key\r\n");
#endif	    			
				if (pressedFlag == -1) 
				{
					//SET_TIMER(1*CPU_CLOCK); // wait 1 sec
				}
				pressedFlag = 1;
				gCHKKEY_HIT=1;
#if defined(UTILITY_DEBUG)				
				dprintf("User Press Break Button\r\n",__LINE__);
#endif
				return 1;	//jasonwang//wei add				

			}
#ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
			else if(Get_GPIO_SW_IN_KERNEL())
			{
	      			if (pressedFlag == -1) 
				{
        				//SET_TIMER(1*CPU_CLOCK); // wait 1 sec
        			}
        			pressedFlag = 1;
				gCHKKEY_HIT=1;
				return 3;	//jasonwang//wei add	
			}
#endif
			else
		      		pressedFlag = 0;
		}
#if defined(UTILITY_DEBUG)
	dprintf("j=%x\r\n",get_timer_jiffies());
#endif
#endif
#endif //CONFIG_NFBI

	return pressedFlag;
}
#endif
//return 0: do nothing; 1: jump to down load mode; 3 jump to debug down load mode
int user_interrupt(unsigned long time)
{
	int i,ret;
	int tickStart=0;
	
#ifdef CONFIG_BOOT_RESET_ENABLE
	int button_press_detected=-1;
#endif
	
	tickStart=get_timer_jiffies();
	
	do {
		ret=pollingDownModeKeyword(ESC);
		if(ret == 1) return 1;
#ifdef CONFIG_BOOT_RESET_ENABLE		
		ret=pollingPressedButton(button_press_detected);
		button_press_detected=ret;
		if(ret > 0) return ret;
#endif		
	}while ( (get_timer_jiffies() - tickStart) < 100  );  // 1 sec
#if defined(UTILITY_DEBUG)
	dprintf("timeout\r\n");
#endif	
#ifdef CONFIG_BOOT_RESET_ENABLE
	if (button_press_detected>0)
	{   
		gCHKKEY_HIT=1;    
		return 1;
	}
#endif	
	return 0;
}
//------------------------------------------------------------------------------------------
int RTL_TIMEOUT()
{
	//return 1 is timeout, 0 is not yet timeout 
#if  defined(RTL8196B)
	return (!(rtl_inl(TC1CNT)>>8));
#endif
}

#ifdef DDR_SDRAM
__IRAM_FASTEXTDEV
void DDR_cali_API7(void)
{
   
       
        int i,j,k;
	  
        int L0 = 0, R0 = 33, L1 = 0, R1 = 33;
        int  DRAM_ADR = 0xA0a00000;
        int  DRAM_VAL = 0x5A5AA5A5;
        int  DDCR_ADR = DDCR_REG;
        int  DDCR_VAL = 0x80000000; //Digital
        //int  DDCR_VAL = 0x0; //Analog ,JSW:Runtime will dead even in IMEM
 

        *((volatile unsigned int *)(DRAM_ADR)) = DRAM_VAL;
 
        while( (*((volatile unsigned int *)(DDCR_REG))& 0x40000000) != 0x40000000);
        while( (*((volatile unsigned int *)(DDCR_REG))& 0x40000000) != 0x40000000);

    for(k=1;k<=IMEM_DDR_CALI_LIMITS;k++)  //Calibration times
    //while(1)
    {
	 
         // Calibrate for DQS0
         for (i = 1; i <= 31; i++)
         {
       #if DDR_DBG
            __delay(10);
             dprintf("\nDQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
	#endif

	#if PROMOS_DDR_CHIP
		__delay(100);  //__delay(1)=5ns
	#endif
	
            *((volatile unsigned int *)(DDCR_ADR)) = (DDCR_VAL & 0x80000000) | ((i-1) << 25);
 
            if (L0 == 0)
            {
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0x00FF00FF) == 0x005A00A5)
               {
                  L0 = i;
               }
            }
            else
            {       
            #if DDR_DBG
	        dprintf("\nDRAM(0x%x)=%x\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
	     #endif
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0x00FF00FF) != 0x005A00A5)
               {
                  //dprintf("\n\n\nError!DQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
             #if DDR_DBG
	           dprintf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
		#endif
                  R0 = i - 1;
                  //R0 = i - 3;  //JSW
                  break;
               }
            }
         }
         DDCR_VAL = (DDCR_VAL & 0xC0000000) | (((L0 + R0) >> 1) << 25); // ASIC
         *(volatile unsigned int *)(DDCR_ADR) = DDCR_VAL;
 
         // Calibrate for DQS1
         for (i = 1; i <= 31; i++)
         {
          #if DDR_DBG
            __delay(10);
             dprintf("\nDQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
	#endif

	#if PROMOS_DDR_CHIP
		__delay(100);  //__delay(1)=5ns
	#endif
	       
             *(volatile unsigned int *)(DDCR_ADR) = (DDCR_VAL & 0xFE000000) | ((i-1) << 20);
 
            if (L1 == 0)
            {
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0xFF00FF00) == 0x5A00A500)
               {
                  L1 = i;
               }
            }
            else
            {
            #if DDR_DBG
               dprintf("\nDRAM(0x%x)=%x\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
		#endif
               if ((*(volatile unsigned int *)(DRAM_ADR) & 0xFF00FF00) != 0x5A00A500)
               {
                 //dprintf("\n\n\nError!DQS1(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
	          // dprintf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
                  R1 = i - 1;
		    //R1 = i - 3;
                  break;
               }
            }
         }
 
         DDCR_VAL = (DDCR_VAL & 0xFE000000) | (((L1 + R1) >> 1) << 20); // ASIC
         *(volatile unsigned int *)(DDCR_ADR) = DDCR_VAL;
 
        /* wait a little bit time, necessary */
       // for(i=0; i < 10000000; i++);
       __delay(100);
 	#if 1  
		#if DDR_DBG
		        dprintf("\nR0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
		        dprintf("\nR1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
		#endif
                	
            

	//Over DDR 200MHZ ,modify DDCR DQS_TAP
	unsigned int ck_m2x_freq_sel=READ_MEM32(HW_STRAP_REG) & 0x1c00;
	//if ((k==IMEM_DDR_CALI_LIMITS) && (ck_m2x_freq_sel==0x1c00))//only for 200MHZ
	if ((k==IMEM_DDR_CALI_LIMITS) )
	{
              #if DDR_DBG
		  dprintf("\nR0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
		  dprintf("\nR1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
		  dprintf("\n=>After IMEM Cali,DDCR(%d)=0x%x\n\n",k ,READ_MEM32(DDCR_REG));
              #endif
		  unsigned short DQS_TAP_C0=(L0 + R0) >> 1;
		  unsigned short DQS_TAP_C1=(L1 + R1) >> 1;
		  //anson add
		  if(ck_m2x_freq_sel==0x1c00)
		  {
		  	if(DQS_TAP_C0 >= 9)
		  		DQS_TAP_C0 = DQS_TAP_C0-3;
		  	else if(DQS_TAP_C0 >= 6)
		  		DQS_TAP_C0 = DQS_TAP_C0-2;
		  	else if(DQS_TAP_C0 >= 5)
		  		DQS_TAP_C0 = DQS_TAP_C0-1;
		  	else
		  		DQS_TAP_C0 = DQS_TAP_C0;
		  //********************************************//
		  	if(DQS_TAP_C1 >= 9)
		  		DQS_TAP_C1 = DQS_TAP_C1-3;
		  	else if(DQS_TAP_C1 >= 6)
		  		DQS_TAP_C1 = DQS_TAP_C1-2;
		  	else if(DQS_TAP_C1 >= 5)
		  		DQS_TAP_C1 = DQS_TAP_C1-1;
		  	else
		  		DQS_TAP_C1 = DQS_TAP_C1;
		  }
		  //anson add end

		  //DDCR_VAL = (DDCR_VAL & 0x80000000) | (DQS_TAP_C1 << 20) | (DQS_TAP_C0 << 25); // Digital
		     DDCR_VAL = (DDCR_VAL & 0x0) | (DQS_TAP_C1 << 20) | (DQS_TAP_C0 << 25); // Analog
		   *(volatile unsigned int *)(DDCR_ADR) = DDCR_VAL;
		 __delay(100);
		 #if DDR_DBG
		  dprintf("\n=>After DQS_TAP Modified,DDCR=0x%x,C0=%d,C1=%d\n\n",READ_MEM32(DDCR_REG),DQS_TAP_C0,DQS_TAP_C1);
		 #endif
	}
	//dprintf("\n=================================\n");
	#endif
	 
	
    	}//end of while(1)	

}
#endif

#ifdef CONFIG_BOOT_TIME_MEASURE
static stage_cnt=0;
void cp3_count_print(void)
{
	unsigned long long temp64bit;

	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9, $12\n\t"
		"nop\n\t"
		"la $10, 0x80000000\n\t"
		"or $9, $10\n\t"
		"mtc0 $9, $12\n\t"
		"nop\n\t"
		"nop\n\t"
		"cfc3 $9,$0 \n\t"
		"ctc3 $0,$0\n\t"
		"mfc3 %M0,$9\n\t"
		"mfc3 %L0,$8\n\t"
		"ctc3 $9, $0\n\t"
		"nop\n\t"
		: "=r"(temp64bit)
		:
		: "$9", "$10");
	prom_printf("[stage:%d, boot0x%xM %xKcycle]", stage_cnt, (unsigned int)(temp64bit>>20), (unsigned int)((temp64bit>>10) &0x3ff));
	/* NOTE: 1M=1024*1024, 1K=1024 */
	stage_cnt++;
}
#endif

//------------------------------------------------------------------------------------------
//init gpio[96c not fix gpio, so close first. fix CPU 390MHz cannot boot from flash.]
void Init_GPIO()
{
#if defined(RTL8196B)||defined(CONFIG_RTL8198)
#ifndef CONFIG_NFBI
#ifndef CONFIG_RTL8198
	#ifdef CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<1) ); //set byte A GPIO1 = gpio
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<1) );  //0 input, 1 output, set F bit 1 input
	#endif
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<5) ); //set byte F GPIO7 = gpio
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<5) );  //0 input, 1 output, set F bit 7 input
	//modify for light reset led pin in output mode
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<RESET_LED_PIN) ); 
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | ((1<<RESET_LED_PIN) ); 
	REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) | ((1<<RESET_LED_PIN) );  
 #else
  #if CONFIG_RTL8198
	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG)& (~(1<<25) ); //set byte F GPIO7 = gpio
        REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) & (~(1<<25) );  //0 input, 1 output, set F bit 7 input

 
  #else
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<5) ); //set byte F GPIO7 = gpio
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~(1<<5) );  //0 input, 1 output, set F bit 7 input
	//modify for light reset led pin in output mode
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(1<<RESET_LED_PIN) ); 
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | ((1<<RESET_LED_PIN) ); 
	REG32(PABCDDAT_REG) = REG32(PABCDDAT_REG) | ((1<<RESET_LED_PIN) ); 
   #endif
#endif	

#endif
#endif
}
unsigned int read_gpio_hw_setting()
{
	unsigned int tmp;
	int b2;

	REG32(PEFGHCNR_REG)   = REG32(PEFGHCNR_REG)   & (~(0x8<<8));  //set (GP2)=(F3)= gpio
	REG32(PEFGHPTYPE_REG) = REG32(PEFGHPTYPE_REG) & (~(0x8<<8));  //change to GPIO mode
	REG32(PEFGHDIR_REG)   = REG32(PEFGHDIR_REG)   & (~(0x8<<8));  //0 input, 1 output, set inpur
	tmp = REG32(PEFGHDAT_REG);
//	dprintf("GPIO HGFE=%08x\r\n", tmp);
//	dprintf("GP2=%01x\r\n", (tmp&(0x08<<8))>>11);
	b2 = (tmp&(0x08<<8))>>11;
	tmp = (b2<<1)&0x2;	
//	dprintf("GPIO val=%08x\r\n",tmp);
	return tmp;	
}
//------------------------------------------------------------------------------------------
#if defined(CONFIG_POST_ENABLE)
int POSTRW_API(void)
{
  	unsigned int Test_Start_offset=0x0;  
#if defined(CONFIG_D8_16)  	
unsigned int Test_Size=0x00800000;  //8MB	
#else
unsigned int Test_Size=0x01000000;  //16MB	
#endif
	  unsigned short test_pattern_mode=0;//"0"=0x5a,"1"=0xa5,
	  unsigned int test_cnt=0;    
	  unsigned int test_result=1;  //"1":Pass,"0":fail

	  //unsigned int DRAM_ADR = 0xA0000000; //uncache address
    unsigned int DRAM_ADR = 0x80000000; //cache address
    unsigned int DRAM_Start_Test_ADR,DRAM_End_Test_ADR; 	  
    unsigned int DRAM_pattern1 = 0xA5A5A5A5;
	  unsigned int DRAM_pattern0 = 0x5A5A5A5A;
	  unsigned int DRAM_pattern;

	  DRAM_Start_Test_ADR= DRAM_ADR + Test_Start_offset; 
	  DRAM_End_Test_ADR=  DRAM_Start_Test_ADR+Test_Size; 
	
	for(test_pattern_mode=1 ;test_pattern_mode<=2;test_pattern_mode++)
	{
		
		if(test_pattern_mode%2==0)
		{
			DRAM_pattern=DRAM_pattern0;
		}
		else
		{
			DRAM_pattern=DRAM_pattern1;
		}
		prom_printf("\nPOST(%d),Pattern:0x%x => ",test_pattern_mode,DRAM_pattern);
		

		/* Set Data Loop*/
		/* 	Test from 1~16MB ,except 7~8MB*/
		for (test_cnt= 0; test_cnt < Test_Size;test_cnt+=0x00100000 )
		{	 	
		     if ((test_cnt==0x0 )||(test_cnt==0x00700000 ) )//skip DRAM size from 0~1MB and 7~8MB
		     {				     
					  continue;
		     }				  
			 memset((unsigned int *) (DRAM_Start_Test_ADR+ test_cnt),DRAM_pattern,(unsigned int)0x00100000 );
		}	 

		/*Verify Data Loop*/
		 for(test_cnt=0;test_cnt<Test_Size;test_cnt+=4)
		 {
		 	 if(((test_cnt >= 0x0 ) && (test_cnt <=0x00100000))||((test_cnt >= 0x00700000 ) && (test_cnt <=0x00800000)))
			 	 continue;

			 if (READ_MEM32(DRAM_Start_Test_ADR+test_cnt) != DRAM_pattern)//Compare FAIL
		 	  {											
						prom_printf("\nDRAM POST Fail at addr:0x%x!!!\n\n",(DRAM_Start_Test_ADR+test_cnt) );
						test_result=0;
						return 0;					
		 	  }
		 }//end of test_cnt

		   if (test_result)
			  	prom_printf("PASS\n");
			  else
			  	prom_printf("Fail\n");
		 
	}//end of test_pattern_mode
	 
	  prom_printf("\n\n");
	   return 1;
  }//end of POSTRW_API
#endif
//-------------------------------------------------------
#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
extern int early_sc16is7x0_init_i2c_and_check( void );
extern unsigned int sc16is7x0_serial_out_i2c(int offset, int value);
int sc16is7x0_err = -1;
#define UART_MEM2REG( m )	( ( m - UART_RBR ) / 4 )
#endif

void console_init(unsigned long lexea_clock)
{
	int i;
	unsigned long dl;
	unsigned long dll;     
	unsigned long dlm;       
#if !defined(CONFIG_SERIAL_SC16IS7X0_CONSOLE)           
  	REG32(UART_LCR_REG)=0x03000000;		//Line Control Register  8,n,1
  			
  	REG32( UART_FCR_REG)=0xc7000000;		//FIFO Ccontrol Register
  	REG32( UART_IER_REG)=0x00000000;
  	dl = (lexea_clock /16)/BAUD_RATE-1;
  	*(volatile unsigned long *)(0xa1000000) = dl ; 
  	dll = dl & 0xff;
  	dlm = dl / 0x100;
  	REG32( UART_LCR_REG)=0x83000000;		//Divisor latch access bit=1
  	REG32( UART_DLL_REG)=dll*0x1000000;
   	REG32( UART_DLM_REG)=dlm*0x1000000; 
    	REG32( UART_LCR_REG)=0x83000000& 0x7fffffff;	//Divisor latch access bit=0
   	//rtl_outl( UART_THR,0x41000000);	
#if defined(RTL8196B)	
  	REG32(GIMR_REG)=REG32(GIMR_REG) | 0x1000;
#endif
#endif

		// ----------------------------------------------------
		// above is UART0, and below is SC16IS7x0 
		// ----------------------------------------------------
	
#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
		sc16is7x0_err = early_sc16is7x0_init_i2c_and_check();
		
		//rtl_outl( UART_LCR,0x03000000);		//Line Control Register  8,n,1
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_LCR ), 0x03 );
		
		//rtl_outl( UART_FCR,0xc7000000);		//FIFO Ccontrol Register
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_FCR ), 0xc7 );
		//rtl_outl( UART_IER,0x00000000);
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_IER ), 0x00 );
		
  #ifdef CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_1843200
		dl = (1843200 /16)/BAUD_RATE;
  #elif defined( CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_14746500 )
		dl = (14746500 /16)/BAUD_RATE;
  #else
		???
  #endif
		//*(volatile unsigned long *)(0xa1000000) = dl ; 
		dll = dl & 0xff;
		dlm = dl >> 8;
		//rtl_outl( UART_LCR,0x83000000);		//Divisor latch access bit=1
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_LCR ), 0x83 );
		//rtl_outl( UART_DLL,dll*0x1000000);
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_DLL ), dll );
		//rtl_outl( UART_DLM,dlm*0x1000000); 
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_DLM ), dlm );
		//rtl_outl( UART_LCR,0x83000000& 0x7fffffff);	//Divisor latch access bit=0
		sc16is7x0_serial_out_i2c( UART_MEM2REG( UART_LCR ), 0x83 & 0x7F );	
#endif

	//dprintf("\n\n-------------------------------------------");
	//dprintf("\nUART1 output test ok\n");
}
//-------------------------------------------------------
void goToDownMode()
{
#ifndef CONFIG_FPGA_PLATFORM
#ifndef CONFIG_RTL8198
		REG32(PIN_MUX_SEL)=REG32(PIN_MUX_SEL)&(0xFFFFFFFF-0x00300000);	
#else
	REG32(PIN_MUX_SEL)=(0x0c0f);  
#endif	
#endif

#ifndef CONFIG_FPGA_PLATFORM
		eth_startup(0);	
#endif

#if defined (SW_8366GIGA)
		REG32(P0GMIICR) = 0x00037d16;
		REG32(PITCR) = 0x1;
		REG32(PCRP0) = 0x00E80367;
#endif

#ifdef CONFIG_BOOT_TIME_MEASURE
		cp3_count_print();
#endif		
		dprintf("\n---Ethernet init Okay!\n");
		sti();
		tftpd_entry();
#ifdef DHCP_SERVER			
		dhcps_entry();
#endif
#ifdef HTTP_SERVER
		httpd_entry();
#endif

#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
	{
	unsigned short PHYID_Count; 
	for(PHYID_Count=0;PHYID_Count<=4;PHYID_Count++)
			enable_10M_power_saving(PHYID_Count , 0x18,0x0310);
	}
#endif
	monitor();
	return ;
}

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE		
void set_bankinfo_register()  //in order to notify kernel
{
#define SYSTEM_CONTRL_DUMMY_REG 0xb8000068
	prom_printf("return_addr = %x ,boot bank=%d, bank_mark=0x%x...\n",return_addr,boot_bank,bank_mark);	
	if(boot_bank == BANK2_BOOT)
		REG32(SYSTEM_CONTRL_DUMMY_REG) = (REG32(SYSTEM_CONTRL_DUMMY_REG) | 0x00000001); //mark_dul, issue use function is better
	//prom_printf("2SYSTEM_CONTRL_DUMMY_REG = %x",REG32(SYSTEM_CONTRL_DUMMY_REG));	
}			
#endif		

void goToLocalStartMode(unsigned long addr,IMG_HEADER_Tp pheader)
{
	unsigned short *word_ptr;
	void	(*jump)(void);
	int i;
	
	//prom_printf("\n---%X\n",return_addr);
	word_ptr = (unsigned short *)pheader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
	*word_ptr = rtl_inw(addr + i);
			
	// move image to SDRAM
	flashread( pheader->startAddr,	(unsigned int)(addr-FLASH_BASE+sizeof(IMG_HEADER_T)), 	pheader->len-2);
			
	if ( !user_interrupt(0) )  // See if user escape during copy image
	{
		outl(0,GIMR0); // mask all interrupt
#if defined(CONFIG_BOOT_RESET_ENABLE)
		Set_GPIO_LED_OFF();
#endif
#if defined(RTL8196B)
		REG32(CPUICR)=0xe4000000; //speedup lexra access
#endif
#ifdef CONFIG_BOOT_TIME_MEASURE
		cp3_count_print();
#endif
		prom_printf("Jump to image start=0x%x...\n", pheader->startAddr);
		
#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
		set_bankinfo_register();
#endif
		jump = (void *)(pheader->startAddr);
				
		cli();
		flush_cache(); 
		jump();				 // jump to start
		return ;
	}
	return;
}
void debugGoToLocalStartMode(unsigned long addr,IMG_HEADER_Tp pheader)
{
	unsigned short *word_ptr;
	void	(*jump)(void);
	int i, count=500;

	//prom_printf("\n---%X\n",return_addr);
	word_ptr = (unsigned short *)pheader;
	for (i=0; i<sizeof(IMG_HEADER_T); i+=2, word_ptr++)
	*word_ptr = rtl_inw(addr + i);
			
	// move image to SDRAM
	flashread( pheader->startAddr,	(unsigned int)(addr-FLASH_BASE+sizeof(IMG_HEADER_T)), 	pheader->len-2);
			
	if ( !user_interrupt(0) )  // See if user escape during copy image
	{
		outl(0,GIMR0); // mask all interrupt
#ifdef CONFIG_BOOT_RESET_ENABLE
		Set_GPIO_LED_OFF();
#endif
#if defined(RTL8196B)
		REG32(CPUICR)=0xe4000000; //speedup lexra access
#endif

		REG32(0xb8019004)=0xFE;
		while(count--)
		{continue;}
		
		if(REG32(0xb8019004)!=0xFE)
			prom_printf("fail debug-Jump to image start=0x%x...\n", pheader->startAddr);
		prom_printf("Debug-Jump to image start=0x%x...\n", pheader->startAddr);
		jump = (void *)(pheader->startAddr);
				
		cli();
		flush_cache(); 
		jump();				 // jump to start
	}
}

//-------------------------------------------------------
//set clk and init console	
void setClkInitConsole(void)
{
	#if !defined(CONFIG_RTL8196C) &&  defined(RTL8196B)	
	REG32(SYS_HW_STRAP) = REG32(SYS_HW_STRAP);
#endif

	REG32(MCR_REG)=REG32(MCR_REG)|0x8000000;
#if defined(DDR_SDRAM) &&  defined(RTL8198)
	REG32(0xb8000044)= 0;
#endif

#if defined(CONFIG_RTL8196C)
	REG32(RTL_GPIO_MUX)=RTL_GPIO_MUX_DATA; 
	#if !defined(CONFIG_RTL8196C_REVISION_B)
	REG32(CLK_MANAGE)=0xa79;
	#endif
	int	clklx_from_clkm=(REG32(SYS_HW_STRAP) & (ST_CLKLX_FROM_CLKM)) >> ST_CLKLX_FROM_CLKM_OFFSET;
	if(clklx_from_clkm==1)
	{
		int	m2xsel=(REG32(SYS_HW_STRAP) & (CK_M2X_FREQ_SEL))>>CK_M2X_FREQ_SEL_OFFSET;
		glexra_clock=mem2x_clksel_table[m2xsel] /2;		
	}	
#else
	#if defined(CONFIG_NFBI)
    	//clear all bits of SYSSR except bit 5, 6, 7, 11
    	REG32(NFBI_SYSSR)= REG32(NFBI_SYSSR) & 0x08e0;
	#endif
#endif

	console_init( glexra_clock);

#if defined(CONFIG_RTL8196C)
	REG32(CLK_MANAGE) = REG32(CLK_MANAGE)| (1<<9) | (1<<11);   //clk_manger
#endif

#if !defined(CONFIG_RTL8198) && defined(DDR_SDRAM)
     cli();  		
     flush_cache(); 	
     REG32(GIMR_REG)=0x0;
     REG32(BIST_CONTROL_REG)|=0x0000001C ;           //Lock LX Bus 0/1/2
     while(1)
     {
         if(READ_MEM32(BIST_DONE_REG) & 0x00000007)
         {
             break;
         }
     }//end of while(1)
     
     DDR_cali_API7();
/* wait a little bit time, necessary */
     int i2;
     for( i2=0; i2 < 1000000; i2++);     
	     REG32(BIST_CONTROL_REG)&=0xFFFFFFE3 ;//Free LX Bus 0/1/2
     sti();    
#endif
}
//-------------------------------------------------------
//init heap	
void initHeap(void)
{
#if  defined(RTL8196B) || defined(RTL8198)
	/* Initialize malloc mechanism */
	UINT32 heap_addr=((UINT32)dl_heap&(~7))+8 ;
	UINT32 heap_end=heap_addr+sizeof(dl_heap)-8;
  	i_alloc((void *)heap_addr, heap_end);
#endif
	cli();  	
	flush_cache(); // david
}
//-------------------------------------------------------
// init interrupt 
void initInterrupt(void)
{
	rtl_outl(GIMR0,0x00);/*mask all interrupt*/
	setup_arch();    /*setup the BEV0,and IRQ */
	exception_init();/*Copy handler to 0x80000080*/
	init_IRQ();      /*Allocate IRQfinder to Exception 0*/
	sti();
}
//-------------------------------------------------------
// init flash 
void initFlash(void)
{
#if !defined(CONFIG_NONE_FLASH) && defined(CONFIG_SPI_FLASH)
   	prom_printf("\n========== SPI =============\n");    	
   	spi_probe();                                    //JSW : SPI flash init		
#elif !defined(CONFIG_NONE_FLASH)
	flashinit();
#endif
}
//-------------------------------------------------------
//rtk bootcode and enable post
//copy img to sdram and monitor ESC interrupt

void doBooting(int flag, unsigned long addr, IMG_HEADER_Tp pheader)
{
#if !defined(CONFIG_NFBI)
	if(flag)
	{

		switch(user_interrupt(WAIT_TIME_USER_INTERRUPT))
		{
		case LOCALSTART_MODE:
		default:
			goToLocalStartMode(addr,pheader);			
		#if defined(CONFIG_RTL8196BU_UART_DISABLE_TO_GPIO)
		case DEBUG_LOCALSTART_MODE:
			debugGoToLocalStartMode(addr,pheader);	
		#endif
		case DOWN_MODE:
#if defined(CONFIG_BOOT_TIME_MEASURE)
			cp3_count_print();
#endif
			dprintf("\n---Escape booting by user\n");	
			cli();
#if defined(CONFIG_BOOT_RESET_ENABLE)
			Set_GPIO_LED_ON();
#endif
			goToDownMode();	
			break;
		}/*switch case */
	}/*if image correct*/
	else
#endif //CONFIG_NFBI
	{

		goToDownMode();		
	}
	return;
}
