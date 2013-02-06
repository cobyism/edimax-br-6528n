
#include <linux/interrupt.h>
#include <asm/system.h>

#include "monitor.h"
#include "etherboot.h"
#include "nic.h"


#include <asm/mipsregs.h>	//wei add
#if defined(CONFIG_SPI_FLASH) || defined(CONFIG_NFBI) //NFBI is for compile issue
#include "spi_flash.h"
#endif

#include <rtl8196x/asicregs.h>        
#if defined(RTL8196B)
#include <asm/rtl8196x.h>
#endif

#if defined(RTL8198)
#include <asm/rtl8198.h>
#define  REVR  0xB8000000
#define  RTL8198_REVISION_A  0xc0000000
#define  RTL8198_REVISION_B  0xc0000001
#endif

/*
#ifdef RTL8196B
#include <asm/rtl8196.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif
*/

#define SYS_BASE 0xb8000000
#define SYS_INI_STATUS (SYS_BASE +0x04)
#define SYS_HW_STRAP (SYS_BASE +0x08)
#define SYS_CLKMANAGE (SYS_BASE +0x10)
//hw strap
#define ST_SYNC_OCP_OFFSET 9
#define CK_M2X_FREQ_SEL_OFFSET 10
#define ST_CPU_FREQ_SEL_OFFSET 13
#define ST_CPU_FREQDIV_SEL_OFFSET 19
#define ST_BOOTPINSEL (1<<0)
#define ST_DRAMTYPE (1<<1)
#define ST_BOOTSEL (1<<2)
#define ST_PHYID (0x3<<3) //11b 
#define ST_EN_EXT_RST (1<<8)
#define ST_SYNC_OCP (1<<9)
#define CK_M2X_FREQ_SEL (0x7 <<10)
#define ST_CPU_FREQ_SEL (0xf<<13)
#define ST_NRFRST_TYPE (1<<17)
#define SYNC_LX (1<<18)
#define ST_CPU_FREQDIV_SEL (0x7<<19)
#define ST_EVER_REBOOT_ONCE (1<<23)
#define ST_SYS_DBG_SEL  (0x3f<<24)
#define ST_PINBUS_DBG_SEL (3<<30)
#define SPEED_IRQ_NO 29
#define SPEED_IRR_NO 3
#define SPEED_IRR_OFFSET 20
extern unsigned int	_end;

extern unsigned char 	ethfile[20];
extern struct arptable_t	arptable[MAX_ARP];
#define MAIN_PROMPT						"<RealTek>"
#define putchar(x)	serial_outc(x)
#define IPTOUL(a,b,c,d)	((a << 24)| (b << 16) | (c << 8) | d )

int YesOrNo(void);
int CmdHelp( int argc, char* argv[] );


#if defined(CONFIG_BOOT_DEBUG_ENABLE)
int CmdDumpWord( int argc, char* argv[] );
int CmdDumpByte( int argc, char* argv[] ); //wei add
int CmdWriteWord( int argc, char* argv[] );
int CmdWriteByte( int argc, char* argv[] );
int CmdWriteHword( int argc, char* argv[] );
int CmdWriteAll( int argc, char* argv[] );
int CmdCmp(int argc, char* argv[]);
int CmdIp(int argc, char* argv[]);
int CmdAuto(int argc, char* argv[]);
int CmdLoad(int argc, char* argv[]);
#endif
//int CmdEDl(int argc, char* argv[]);
//int CmdEUl(int argc, char* argv[]);
int CmdCfn(int argc, char* argv[]);
//int CmdFle(int argc, char* argv[]);
#ifndef CONFIG_SPI_FLASH
int CmdFlw(int argc, char* argv[]);
int CmdFlr(int argc, char* argv[]);
#endif
int CmdNFlr(int argc, char* argv[]);
int CmdNFlw(int argc, char* argv[]);

//int CmdTimer(int argc, char* argv[]);
//int CmdMTC0SR(int argc, char* argv[]);  //wei add
//int CmdMFC0SR(int argc, char* argv[]);  //wei add
//int CmdTFTP(int argc, char* argv[]);  //wei add
#if defined(CONFIG_BOOT_DEBUG_ENABLE)
#ifdef REMOVED_UNUSED
int CmdTFTP_TX(int argc, char* argv[]);  //wei add
#endif
#endif
#ifdef CONFIG_RTL8198_TAROKO
int CmdIMEM98TEST(int argc, char* argv[]);
int CmdWBMG(int argc, char* argv[]);
#endif

#ifdef RTL8198
int CmdEEEPatch(int argc, char* argv[]); 
#endif

//Ziv
#ifdef WRAPPER
#ifndef CONFIG_SPI_FLASH
//write bootcode to flash from my content
int CmdWB(int argc, char* argv[]);

#endif
#ifdef CONFIG_SPI_FLASH
int CmdSWB(int argc, char* argv[]);
#endif
extern char _bootimg_start, _bootimg_end;
#endif

#ifdef CONFIG_SPI_FLASH
int CmdSFlw(int argc, char* argv[]);
     //JSW: Auto-memory test program @20070916 for SIO/MIO

extern void auto_spi_memtest_8198(unsigned long DRAM_starting_addr, unsigned int spi_clock_div_num);
#endif


#ifdef  CONFIG_DRAM_TEST
	void Dram_test(int argc, char* argv[]);
#endif

#ifdef  CONFIG_NOR_TEST
	int CmdNTEST(int argc, char* argv[]);	
	extern void auto_nor_memtest( unsigned long dram_test_starting_addr   );
#endif


#ifdef  CONFIG_SPI_TEST
	int CmdSTEST(int argc, char* argv[]);               //JSW: add for SPI/SDRAM auto-memory-test program
#endif


#ifdef CONFIG_CPUsleep_PowerManagement_TEST
	int CmdCPUSleep(int argc, char* argv[]);
	void CmdCPUSleepIMEM(void);
#endif


#if defined(CONFIG_PCIE_MODULE) 
int CmdTestPCIE(int argc, char* argv[]);
#endif
#if defined(CONFIG_R8198EP_HOST) || defined(CONFIG_R8198EP_DEVICE)
int CmdTestSlavePCIE(int argc, char* argv[]);
#endif



#ifdef CMD_PHY_RW
int CmdPHYregR(int argc, char* argv[]);
int CmdPHYregW(int argc, char* argv[]);
#endif

/*Cyrus Tsai*/
/*move to ehterboot.h
#define TFTP_SERVER 0
#define TFTP_CLIENT 1
*/
extern struct arptable_t  arptable_tftp[3];
/*Cyrus Tsai*/

//extern int flasherase(unsigned long src, unsigned int length);
//extern int flashwrite(unsigned long dst, unsigned long src, unsigned long length);
//extern int flashread (unsigned long dst, unsigned long src, unsigned long length);

extern int write_data(unsigned long dst, unsigned long length, unsigned char *target);
extern int read_data (unsigned long src, unsigned long length, unsigned char *target);

/*Cyrus Tsai*/
extern unsigned long file_length_to_server;
extern unsigned long file_length_to_client;
extern unsigned long image_address; 
/*this is the file length, should extern to flash driver*/
/*Cyrus Tsai*/

#if defined(RTL8196B) || defined(RTL8198)
#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *) (addr)) = (val)
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))

#define SWCORE_BASE      0xBB800000        
#define PCRAM_BASE       (0x4100+SWCORE_BASE)
#define PITCR                  (0x000+PCRAM_BASE)       /* Port Interface Type Control Register */
#define PCRP0                 (0x004+PCRAM_BASE)       /* Port Configuration Register of Port 0 */
#define PCRP1                 (0x008+PCRAM_BASE)       /* Port Configuration Register of Port 1 */
#define PCRP2                 (0x00C+PCRAM_BASE)       /* Port Configuration Register of Port 2 */
#define PCRP3                 (0x010+PCRAM_BASE)       /* Port Configuration Register of Port 3 */
#define PCRP4                 (0x014+PCRAM_BASE)       /* Port Configuration Register of Port 4 */
#define EnablePHYIf        (1<<0)                           /* Enable PHY interface.                    */
#endif
 
COMMAND_TABLE	MainCmdTable[] =
{
	{ "?"	  ,0, CmdHelp			, "HELP (?)				    : Print this help message"					},
	{ "HELP"  ,0, CmdHelp			, NULL					},		
#if defined(CONFIG_BOOT_DEBUG_ENABLE)												
	{ "D"	  ,2, CmdDumpWord		, "D <Address> <Len>"},    
	{ "DB"	  ,2, CmdDumpByte		, "DB <Address> <Len>"}, //wei add	
	{ "DW"	  ,2, CmdDumpWord		, "DW <Address> <Len>"},  //same command with ICE, easy use
	{ "EW",2, CmdWriteWord, "EW <Address> <Value1> <Value2>..."},
#ifdef REMOVED_UNUSED
	{ "EH",2, CmdWriteHword, "EH <Address> <Value1> <Value2>..."},
#endif
	{ "EB",2, CmdWriteByte, "EB <Address> <Value1> <Value2>..."},
#ifdef REMOVED_UNUSED
	{ "EC",2, CmdWriteAll, "EC <Address> <Value1> <Length>..."},
#endif
	{ "CMP",3, CmdCmp, "CMP: CMP <dst><src><length>"},
	{ "IPCONFIG",2, CmdIp, "IPCONFIG:<TargetAddress>"},
#ifndef CONFIG_NONE_FLASH
	{ "AUTOBURN"   ,1, CmdAuto			, "AUTOBURN: 0/1" },
#endif
	{ "LOADADDR"   ,1, CmdLoad			, "LOADADDR: <Load Address>"					},
#endif
	{ "J"  ,1, CmdCfn			, "J: Jump to <TargetAddress>"											},
#ifndef CONFIG_NONE_FLASH
#ifndef CONFIG_SPI_FLASH
	{ "FLW"   ,3, CmdFlw			, "FLW: FLW <dst><src><length>"					},
	{ "FLR"   ,3, CmdFlr			, "FLR: FLR <dst><src><length>"					},	
#endif
#endif

#if defined(CONFIG_BOOT_DEBUG_ENABLE)
#ifdef REMOVED_UNUSED
	{ "TFTPTX"   ,0, CmdTFTP_TX			, "TFTPTX: Start TFTP Send "					}, 	//wei add
#endif
#endif
#ifdef CONFIG_RTL8198_TAROKO
	{ "IMEMTEST"   ,0, CmdIMEM98TEST			, "IMEMTEST "					},
	{ "WBMG"	,0, CmdWBMG			, "WBMF: write buffer merge"				},
#endif

	//{ "TIMER"   ,1, CmdTimer			, "TIMER: Timer Test" },	
//	{ "MTC0SR"   ,1, CmdMTC0SR			, "MTC0SR: MTC0SR <Value>>"					}, //wei add
//	{ "MFC0SR"   ,0, CmdMFC0SR			, "MFC0SR: MFC0SR "					}, //wei add	

#ifdef SUPPORT_NAND
	{ "NFLR",3, CmdNFlr, "NFLR: NFLR <dst><src><length>"},
	{ "NFLW",3, CmdNFlw, "NFLW: NFLW <dst><src><length>"},
#endif

#ifdef WRAPPER
#ifndef CONFIG_NONE_FLASH
#ifndef CONFIG_SPI_FLASH
	{ "WB", 0, CmdWB, "WB: WB"},
#endif
#endif
#ifdef CONFIG_SPI_FLASH
	{ "SWB", 1, CmdSWB, "SWB <SPI cnt#> (<0>=1st_chip,<1>=2nd_chip): SPI Flash WriteBack (for MXIC/Spansion)"}, 	//JSW	
	
#endif
#endif

#ifdef CONFIG_SPI_FLASH
	{ "FLW",4, CmdSFlw, "FLW <dst_ROM_offset><src_RAM_addr><length_Byte> <SPI cnt#>: Write offset-data to SPI from RAM"},	 //JSW
#endif


#ifdef CONFIG_DRAM_TEST
	{ "DRAMTEST",2,Dram_test , "DRAMTEST <R/W> <enable_random_delay> <PowerManagementMode>" },
#endif

#ifdef CONFIG_NOR_TEST
	{ "NTEST",2, CmdNTEST, "NTEST <test_count> <HEX_DRAM_test_starting_addr>: Auto test NOR and DRAM"    }, //JSW
#endif

#ifdef CONFIG_SPI_TEST
	{ "STEST",3, CmdSTEST, "STEST <test_count><HEX_DRAM_test_starting_addr><spi_clock_div_num>: Auto test SPI and DRAM "},	//JSW
#endif

#ifdef CONFIG_CPUsleep_PowerManagement_TEST
  { "SLEEP"   ,0, CmdCPUSleep          , "Sleep  : Test CPU sleep "                 },
#endif

#if defined(CONFIG_PCIE_MODULE) 
	{ "PCIE",0, CmdTestPCIE, "PCIE: Test Host PCI-E"},
	
#endif
#if defined(CONFIG_R8198EP_HOST) || defined(CONFIG_R8198EP_DEVICE)
	{ "SPE",0, CmdTestSlavePCIE, "SPE: Test Slave PCI-E"},	
#endif
#ifdef CMD_PHY_RW
  { "PHYR",2, CmdPHYregR, "PHYR: PHYR <PHYID><reg>"},
  { "PHYW",3, CmdPHYregW, "PHYW: PHYW <PHYID><reg><data>"},
#endif
#ifdef RTL8198
	{ "EEE"   ,1, CmdEEEPatch			, "EEE :Set EEE Pathch "}, 
#endif 
};



static unsigned long	CurrentDumpAddress;
static unsigned long   sys_ipaddress;
//------------------------------------------------------------------------------
/********   caculate CPU clock   ************/
int check_cpu_speed(void);
void timer_init(unsigned long lexra_clock);
void timer_stop(void);
static void timer_interrupt(int num, void *ptr, struct pt_regs * reg);
struct irqaction irq_timer = {timer_interrupt, 0, 8, "timer", NULL, NULL};                                   
static volatile unsigned int jiffies=0;
static void timer_interrupt(int num, void *ptr, struct pt_regs * reg)
{
	//dprintf("jiffies=%x\r\n",jiffies);
	//flush_WBcache();
	rtl_outl(TCIR,rtl_inl(TCIR));
	jiffies++;


}
volatile int get_timer_jiffies(void)
{

	return jiffies;
};



//------------------------------------------------------------------------------
void timer_init(unsigned long lexra_clock)
{
    /* Set timer mode and Enable timer */
    REG32(TCCNR_REG) = (0<<31) | (0<<30);	//using time0
    //REG32(TCCNR_REG) = (1<<31) | (0<<30);	//using counter0

	#define DIVISOR     0xE
	#define DIVF_OFFSET                         16		
    REG32(CDBR_REG) = (DIVISOR) << DIVF_OFFSET;
    
    /* Set timeout per msec */
#ifdef CONFIG_FPGA_PLATFORM
    //int SysClkRate = 33860000;	 /* 33.86 MHz */
    int SysClkRate = 27000000;	 /* 27MHz */
#else
	int SysClkRate = lexra_clock;	 /* CPU 200MHz */
#endif

	#define TICK_10MS_FREQ  100 /* 100 Hz */
	#define TICK_100MS_FREQ 1000 /* 1000 Hz */
	#define TICK_FREQ       TICK_10MS_FREQ	
    #ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
    		REG32(TC0DATA_REG) = (((SysClkRate / DIVISOR) / TICK_FREQ) + 1) <<4;
    	else
    #endif 
    #ifdef CONFIG_RTL8198_REVISION_B
   	if (REG32(REVR) >= RTL8198_REVISION_B)
                REG32(TC0DATA_REG) = (((SysClkRate / DIVISOR) / TICK_FREQ) + 1) <<4;
        else 
   #endif
    REG32(TC0DATA_REG) = (((SysClkRate / DIVISOR) / TICK_FREQ) + 1) <<8;		//set 10msec

       
    /* Set timer mode and Enable timer */
    REG32(TCCNR_REG) = (1<<31) | (1<<30);	//using time0
    /* We must wait n cycles for timer to re-latch the new value of TC1DATA. */
	int c;	
	for( c = 0; c < DIVISOR; c++ );
	

      /* Set interrupt mask register */
    //REG32(GIMR_REG) |= (1<<8);	//request_irq() will set 

    /* Set interrupt routing register */
#ifdef RTL8196C
    REG32(IRR1_REG) = (4<<24);  // time0:IRQ4
#else  //RTL8196B, RTL8198
    REG32(IRR1_REG) = 0x00050004;  //uart:IRQ5,  time0:IRQ4
#endif    
    
    /* Enable timer interrupt */
    REG32(TCIR_REG) = (1<<31);
}
//------------------------------------------------------------------------------
void timer_stop(void)
{
	// disable timer interrupt
	rtl_outl(TCCNR,0);
	rtl_outl(TCIR,0);
}


//------------------------------------------------------------------------------

__inline__ void
__delay(unsigned long loops)
{
	__asm__ __volatile__ (
		".set\tnoreorder\n"
		"1:\tbnez\t%0,1b\n\t"
		"subu\t%0,1\n\t"
		".set\treorder"
		:"=r" (loops)
		:"0" (loops));
}

/*
80007988 <__delay>:                                             
80007988:	1480ffff 	bnez	a0,80007988 <__delay>           
8000798c:	2484ffff 	addiu	a0,a0,-1                        
80007990:	03e00008 	jr	ra                                  
*/

//---------------------------------------------------------------------------
static unsigned long loops_per_jiffy = (1<<12);
#define LPS_PREC 8
#define HZ 100
#ifdef CONFIG_SERIAL_SC16IS7X0
#ifdef RTL8198
unsigned long loops_per_sec = 2490368 * HZ;	// @CPU 500MHz (this will be update in check_cpu_speed())
#else
unsigned long loops_per_sec = 0x1db000 * HZ;	// @CPU 390MHz, DDR 195 MHz (this will be update in check_cpu_speed())
#endif
#endif
int check_cpu_speed(void)
{
	unsigned long ticks, loopbit;
	int lps_precision = LPS_PREC;
      
#ifdef RTL8196C
  	request_IRQ(14, &irq_timer, NULL); 
#else  //RTL8196B, RTL8198
  	request_IRQ(8, &irq_timer, NULL); 
#endif
	extern long glexra_clock;
       timer_init(glexra_clock);	

	loops_per_jiffy = (1<<12);
	while (loops_per_jiffy <<= 1) {
		/* wait for "start of" clock tick */
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */;
		/* Go .. */
		ticks = jiffies;
		__delay(loops_per_jiffy);
		ticks = jiffies - ticks;
		if (ticks)
			break;
	}
/* Do a binary approximation to get loops_per_jiffy set to equal one clock
   (up to lps_precision bits) */
	loops_per_jiffy >>= 1;
	loopbit = loops_per_jiffy;
	while ( lps_precision-- && (loopbit >>= 1) ) 
	{
		loops_per_jiffy |= loopbit;
		ticks = jiffies;
		while (ticks == jiffies);
		ticks = jiffies;
		__delay(loops_per_jiffy);
		if (jiffies != ticks)	/* longer than 1 tick */
			loops_per_jiffy &= ~loopbit;
	}

#ifdef CONFIG_SERIAL_SC16IS7X0
	loops_per_sec = loops_per_jiffy * HZ;
#endif
	
	//timer_stop();	//wei del, because not close timer
	//free_IRQ(8);
/* Round the value and print it */	
	//prom_printf("cpu run %d.%d MIPS\n", loops_per_jiffy/(500000/HZ),      (loops_per_jiffy/(5000/HZ)) % 100);
	return ((loops_per_jiffy/(500000/HZ))+1);
	
}
//---------------------------------------------------------------------------


/*
---------------------------------------------------------------------------
;				Monitor
---------------------------------------------------------------------------
*/
extern char** GetArgv(const char* string);

void monitor(void)
{
	char		buffer[ MAX_MONITOR_BUFFER +1 ];
	int		argc ;
	char**		argv ;
	int		i, retval ;
	
//	i = &_end;
//	i = (i & (~4095)) + 4096;
	//printf("Free Mem Start=%X\n", i);
	while(1)
	{	
		printf( "%s", MAIN_PROMPT );
		memset( buffer, 0, MAX_MONITOR_BUFFER );
		GetLine( buffer, MAX_MONITOR_BUFFER,1);
		printf( "\n" );
		argc = GetArgc( (const char *)buffer );
		argv = GetArgv( (const char *)buffer );
		if( argc < 1 ) continue ;
		StrUpr( argv[0] );
		for( i=0 ; i < (sizeof(MainCmdTable) / sizeof(COMMAND_TABLE)) ; i++ )
		{
			
			if( ! strcmp( argv[0], MainCmdTable[i].cmd ) )
			{
#if 0
				if (MainCmdTable[i].n_arg != (argc - 1))
					printf("%s\n", MainCmdTable[i].msg);
				else
					retval = MainCmdTable[i].func( argc - 1 , argv+1 );
#endif
				retval = MainCmdTable[i].func( argc - 1 , argv+1 );
				memset(argv[0],0,sizeof(argv[0]));
				break;
			}
		}
		if(i==sizeof(MainCmdTable) / sizeof(COMMAND_TABLE)) printf("Unknown command !\r\n");
	}
}


//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------


#ifdef WRAPPER
#ifndef CONFIG_NONE_FLASH
#ifndef CONFIG_SPI_FLASH
int CmdWB(int argc, char* argv[])
{
       char* start = &_bootimg_start;
	char* end  = &_bootimg_end;
        
	   
	unsigned int length = end - start;
	
	printf("Flash wille write %X length of embedded boot code at %X to %X\n", length, start, end);
	printf("(Y)es, (N)o->");
	if (YesOrNo())
		if (flashwrite(0, (unsigned long)start, length))
			printf("Flash Write Successed!\n");
		else
			printf("Flash Write Failed!\n");
	else
		printf("Abort!\n");

}
#endif
#endif

#ifdef CONFIG_SPI_FLASH
extern char _bootimg_start, _bootimg_end;
//SPI Write-Back
int CmdSWB(int argc, char* argv[])
{
	unsigned short auto_spi_clock_div_num;//0~7
	unsigned int  cnt=strtoul((const char*)(argv[0]), (char **)NULL, 16);	//JSW check
	char* start = &_bootimg_start;
	char* end  = &_bootimg_end;	   
	unsigned int length = end - start;		
	printf("SPI Flash #%d will write 0x%X length of embedded boot code from 0x%X to 0x%X\n", cnt+1,length, start, end);
	printf("(Y)es, (N)o->");
	if (YesOrNo())
	{
		spi_pio_init();
		 #if defined(SUPPORT_SPI_MIO_8198_8196C)
			spi_flw_image_mio_8198(cnt, 0, (unsigned char*)start , length);	
	  	#else			
			spi_flw_image(cnt, 0, (unsigned char*)start , length);
		#endif
		printf("SPI Flash Burn OK!\n");
	}	
	else 
	{
        	printf("Abort!\n");	
	}	
  }





#endif
#endif
/*/
---------------------------------------------------------------------------
; Ethernet Download
---------------------------------------------------------------------------
*/




extern unsigned long ETH0_ADD;
int CmdCfn(int argc, char* argv[])
{
	unsigned long		Address;
	void	(*jump)(void);
	if( argc > 0 )
	{
		if(!Hex2Val( argv[0], &Address ))
		{
			printf(" Invalid Address(HEX) value.\n");
			return FALSE ;
		}
	}

	dprintf("---Jump to address=%X\n",Address);
	jump = (void *)(Address);
	outl(0,GIMR0); // mask all interrupt
	cli(); 
#if defined(RTL8196B) || defined(RTL8198)
#ifndef CONFIG_FPGA_PLATFORM
      /* if the jump-Address is BFC00000, then do watchdog reset */
      if(Address==0xBFC00000)
      	{
      	   *(volatile unsigned long *)(0xB800311c)=0; /*this is to enable 865xc watch dog reset*/
          for( ; ; );
      	}
     else /*else disable PHY to prevent from ethernet disturb Linux kernel booting */
     	{
           WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(~EnablePHYIf )) ); 
           WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)&(~EnablePHYIf )) ); 
           WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)&(~EnablePHYIf )) ); 
           WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)&(~EnablePHYIf )) ); 
           WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)&(~EnablePHYIf )) ); 
	flush_cache();
     	}
#endif
#endif
	jump();	
	
}

/*int CmdEDl( int argc, char* argv[] )
{

	int			address;
	unsigned char 		iChar[2];
	int 	bytecount=0;
	
//	address = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
//	prom_printf("Ethernet download %s to addr=%X\n? (Y/y/N/n)", argv[0], address);
	
		
//	GetLine( iChar, 2,1); 
//	if ((iChar[0] == 'Y') || (iChar[0] == 'y'))
//		printf("download! \n");
//	else
		printf("abort! \n");
	return TRUE;

}*/

/*int CmdEUl(int argc, char* argv[])
{

//	unsigned long		address;
//	unsigned long		bytecount;

//	unsigned char 		iChar[2];





//	address = strtoul((const char*)(argv[1]), (char **)NULL, 16);		
//	bytecount= strtoul((const char*)(argv[2]), (char **)NULL, 16);		

	
//	prom_printf("Ethernet upload %s from addr=%X with bytecount:%X\n? (Y/y/N/n)", 
//	argv[0], address,bytecount);	
	
//	GetLine( iChar, 2,1); 
//	if ((iChar[0] == 'Y') || (iChar[0] == 'y'))
//	        printf("\nUpload !\n");
//	else
	printf("abort! \n");
	
	
	return TRUE;
	
	
}*/

//---------------------------------------------------------------------------
#if defined(CONFIG_BOOT_DEBUG_ENABLE)	
//---------------------------------------------------------------------------
/* This command can be used to configure host ip and target ip	*/

extern char eth0_mac[6];
int CmdIp(int argc, char* argv[])
{
	unsigned char  *ptr;
	unsigned int i;
	int  ip[4];
	
	if (argc==0)
	{	
		printf(" Target Address=%d.%d.%d.%d\n",
		arptable_tftp[TFTP_SERVER].ipaddr.ip[0], arptable_tftp[TFTP_SERVER].ipaddr.ip[1], 
		arptable_tftp[TFTP_SERVER].ipaddr.ip[2], arptable_tftp[TFTP_SERVER].ipaddr.ip[3]);
#ifdef HTTP_SERVER
		printf("   Http Address=%d.%d.%d.%d\n",
		arptable_tftp[HTTPD_ARPENTRY].ipaddr.ip[0], arptable_tftp[HTTPD_ARPENTRY].ipaddr.ip[1], 
		arptable_tftp[HTTPD_ARPENTRY].ipaddr.ip[2], arptable_tftp[HTTPD_ARPENTRY].ipaddr.ip[3]);
#endif
		return;	 
	}			
	
	ptr = argv[0];

	for(i=0; i< 4; i++)
	{
		ip[i]=strtol((const char *)ptr,(char **)NULL, 10);		
		ptr = strchr(ptr, '.');
		ptr++;
	}
	arptable_tftp[TFTP_SERVER].ipaddr.ip[0]=ip[0];
	arptable_tftp[TFTP_SERVER].ipaddr.ip[1]=ip[1];
	arptable_tftp[TFTP_SERVER].ipaddr.ip[2]=ip[2];
	arptable_tftp[TFTP_SERVER].ipaddr.ip[3]=ip[3];
/*replace the MAC address middle 4 bytes.*/
	eth0_mac[1]=ip[0];
	eth0_mac[2]=ip[1];
	eth0_mac[3]=ip[2];
	eth0_mac[4]=ip[3];
	arptable_tftp[TFTP_SERVER].node[5]=eth0_mac[5];
	arptable_tftp[TFTP_SERVER].node[4]=eth0_mac[4];
	arptable_tftp[TFTP_SERVER].node[3]=eth0_mac[3];
	arptable_tftp[TFTP_SERVER].node[2]=eth0_mac[2];
	arptable_tftp[TFTP_SERVER].node[1]=eth0_mac[1];
	arptable_tftp[TFTP_SERVER].node[0]=eth0_mac[0];
	prom_printf("Now your Target IP is %d.%d.%d.%d\n", ip[0],ip[1],ip[2],ip[3]);
#if 0	
	ptr = argv[1];
	//prom_printf("You want to setup Host new ip as %s \n", ptr);	
	
	for(i=0; i< 4; i++)
	{
		ip[i]=strtol((const char *)ptr,(char **)NULL, 10);		
		ptr = strchr(ptr, '.');
		ptr++;
	}
	arptable[ARP_SERVER].ipaddr.ip[0]=ip[0];
	arptable[ARP_SERVER].ipaddr.ip[1]=ip[1];
	arptable[ARP_SERVER].ipaddr.ip[2]=ip[2];
	arptable[ARP_SERVER].ipaddr.ip[3]=ip[3];
	prom_printf("Now your Host IP is %d.%d.%d.%d\n", ip[0],ip[1],ip[2],ip[3]);
#endif	
		
}	
int CmdDumpWord( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned int len,i;

	if(argc<1)
	{	dprintf("Wrong argument number!\r\n");
		return;
	}
	
	if(argv[0])	
	{	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);
		if(src <0x80000000)
			src|=0x80000000;
	}
	else
	{	dprintf("Wrong argument number!\r\n");
		return;		
	}
				
	if(!argv[1])
		len = 1;
	else
	len= strtoul((const char*)(argv[1]), (char **)NULL, 10);			
	while ( (src) & 0x03)
		src++;

	for(i=0; i< len ; i+=4,src+=16)
	{	
		dprintf("%08X:	%08X	%08X	%08X	%08X\n",
		src, *(unsigned long *)(src), *(unsigned long *)(src+4), 
		*(unsigned long *)(src+8), *(unsigned long *)(src+12));
	}

}

//---------------------------------------------------------------------------
int CmdDumpByte( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned int len,i;

	if(argc<1)
	{	dprintf("Wrong argument number!\r\n");
		return;
	}
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	if(!argv[1])
		len = 16;
	else
	len= strtoul((const char*)(argv[1]), (char **)NULL, 10);			


	ddump((unsigned char *)src,len);
}

//---------------------------------------------------------------------------
int CmdWriteWord( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned int value,i;
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	while ( (src) & 0x03)
		src++;

	for(i=0;i<argc-1;i++,src+=4)
	{
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 16);	
		*(volatile unsigned int *)(src) = value;
	}
	
}
//---------------------------------------------------------------------------

#ifdef REMOVED_UNUSED
int CmdWriteAll( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned int value,i;
	unsigned int length;
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	while ( (src) & 0x03)
		src++;

	i = 0;
	value = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	length = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	printf("Write %x to %x for length %d\n",value,src,length);
	for(i=0;i<length;i+=4,src+=4)
	{
		*(volatile unsigned int *)(src) = value;
	}
	
}
//---------------------------------------------------------------------------

int CmdWriteHword( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned short value,i;
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	
	src &= 0xfffffffe;	

	for(i=0;i<argc-1;i++,src+=2)
	{
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 16);	
		*(volatile unsigned short *)(src) = value;
	}
	
}
#endif
//---------------------------------------------------------------------------
int CmdWriteByte( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned char value,i;
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		


	for(i=0;i<argc-1;i++,src++)
	{
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 16);	
		*(volatile unsigned char *)(src) = value;
	}
	
}
int CmdCmp(int argc, char* argv[])
{
	int i;
	unsigned long dst,src;
	unsigned long dst_value, src_value;
	unsigned int length;
	unsigned long error;

	if(argc < 3) {
		printf("Parameters not enough!\n");
		return 1;
	}
	dst = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	src = strtoul((const char*)(argv[1]), (char **)NULL, 16);
	length= strtoul((const char*)(argv[2]), (char **)NULL, 16);		
	error = 0;
	for(i=0;i<length;i+=4) {
		dst_value = *(volatile unsigned int *)(dst+i);
		src_value = *(volatile unsigned int *)(src+i);
		if(dst_value != src_value) {		
			printf("%dth data(%x %x) error\n",i, dst_value, src_value);
			error = 1;
		}
	}
	if(!error)
		printf("No error found\n");

}

//---------------------------------------------------------------------------
#ifndef RTL8197B
extern int autoBurn;
int CmdAuto(int argc, char* argv[])
{
	unsigned long addr;


	if(argv[0][0] == '0')
		autoBurn = 0 ;
	else
		autoBurn = 1 ;
	printf("AutoBurning=%d\n",autoBurn);
}
#endif


//---------------------------------------------------------------------------
int CmdLoad(int argc, char* argv[])
{
	unsigned long addr;


	image_address= strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	printf("Set TFTP Load Addr 0x%x\n",image_address);
}

#endif
/*
--------------------------------------------------------------------------
Flash Utility
--------------------------------------------------------------------------
*/
#ifndef CONFIG_SPI_FLASH
int CmdFlr(int argc, char* argv[])
{
	int i;
	unsigned long dst,src;
	unsigned int length;
	//unsigned char TARGET;
//#define  FLASH_READ_BYTE	4096

	dst = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	src = strtoul((const char*)(argv[1]), (char **)NULL, 16);
	length= strtoul((const char*)(argv[2]), (char **)NULL, 16);		
	//length= (length + (FLASH_READ_BYTE - 1)) & FLASH_READ_BYTE;

/*Cyrus Tsai*/
/*file_length_to_server;*/
//length=file_length_to_client;
//length=length & (~0xffff)+0x10000;
//dst=image_address;
file_length_to_client=length;
/*Cyrus Tsai*/

	printf("Flash read from %X to %X with %X bytes	?\n",src,dst,length);
	printf("(Y)es , (N)o ? --> ");

	if (YesOrNo())
	        //for(i=0;i<length;i++)
	        //   {
		//    if ( flashread(&TARGET, src+i,1) )
		//	printf("Flash Read Successed!, target %X\n",TARGET);
		//    else
		//	printf("Flash Read Failed!\n");
		//  }	
		    if (flashread(dst, src, length))
			printf("Flash Read Successed!\n");
		    else
			printf("Flash Read Failed!\n");
	else
		printf("Abort!\n");
//#undef	FLASH_READ_BYTE		4096

}
#endif


#ifndef RTL8197B
/* Setting image header */


#ifndef CONFIG_SPI_FLASH
int CmdFlw(int argc, char* argv[])
{
	unsigned long dst,src;
	unsigned long length;

#define FLASH_WRITE_BYTE 4096

	dst = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	src = strtoul((const char*)(argv[1]), (char **)NULL, 16);		
	length= strtoul((const char*)(argv[2]), (char **)NULL, 16);		
//	length= (length + (FLASH_WRITE_BYTE - 1)) & FLASH_WRITE_BYTE;

/*Cyrus Tsai*/
/*file_length_to_server;*/
//length=file_length_to_server;
//length=length & (~0xffff)+0x10000;
/*Cyrus Tsai*/

	
	printf("Flash Program from %X to %X with %X bytes	?\n",src,dst,length);
	printf("(Y)es, (N)o->");
	if (YesOrNo())
		if (flashwrite(dst, src, length))
			printf("Flash Write Successed!\n");
		else
			printf("Flash Write Failed!\n");
	else
		printf("Abort!\n");
#undef FLASH_WRITE_BYTE //4096

        //---------------------------------------------------------------------------

}




#endif
//---------------------------------------------------------------------------
#endif //RTL8197B

#if 0
//---------------------------------------------------------------------------
int CmdTimer(int argc, char* argv[])
{
//	extern void timer_task(void);
//	timer_task();	
#if 1 //wei edit
	int i=0;
	dprintf("Endless loop, every 1 sec show a message.\r\n");
	while(1)
	{
		i=jiffies;
		if(i%100 ==0 )		
			dprintf("1 sec.\r\n");

		while(i==jiffies) ;

	}
#endif
#if 0		//memory read and write test
	
	unsigned int val=0;
	volatile unsigned long *p;
	//for(p=0x81000000; p< 0x82000000; p+=4)	//32M16b test 16M
	for(p=0x80f00000; p< 0x81000000; p+=4)	//16M test 8M	
	{	*p=p;
		if( *p != p)
			dprintf("Error Addr=%x, Read=%x\r\n", p, *p);

	}
#endif
	return TRUE;
	
}
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if 0
int CmdTFTP(int argc, char* argv[])  //wei add
{
//	unsigned long addr;
	volatile unsigned int reg;	
//	addr = strtoul((const char*)(argv[1]), (char **)NULL, 10);
	
	eth_listening();	//do while(1) receive packet


}
#endif
#if !defined(CONFIG_BOOT_DEBUG_ENABLE)
extern char eth0_mac[6];
#endif
#if defined(CONFIG_BOOT_DEBUG_ENABLE)
#ifdef REMOVED_UNUSED
int CmdTFTP_TX(int argc, char* argv[])  //wei add
{
//	unsigned long addr;
	volatile unsigned int reg;	
//	addr = strtoul((const char*)(argv[1]), (char **)NULL, 10);
	int i;

//ARP protocol
	unsigned char tx_buffer[64]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //DMAC
							 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  //SMAC
							 0x08, 0x06, //ARP
							 0x00, 0x01, //ETH
							//----------
							 0x08, 0x00, //IP Protocol
							 0x06,0x04,
							 0x00,0x01, //OPcode request
							 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  //SMAC
							 0xc0, 0xa8,0x01,0x06,  //Sender IP:192.168.1.6
							 //-------------
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //Target MAC
							 0xc0, 0xa8, 0x01, 0x07,  //Target IP	:192.168.1.7	
							 //0x00,0x04,0x00,0x01,  //checksum
							 
							}; 
//UDP protocol
	unsigned char tx_buffer1[]={					 
	0x56,0xaa,0xa5,0x5a,0x7d,0xe8,
	0x00,0x1a,0x92,0x30,0xc1,0x49,
	0x08,0x00,
	0x45,0x00, 
	//----------          
	0x00,0x30,0x68,0x51,0x00,0x00,0x80,0x11,0x4f,0x0e,
	0xc0,0xa8,0x01,0x07,  //SIP
	0xc0,0xa8,0x01,0x06, //DIP
	0x13,0x3c,
	0x00,0x45,
	0x00,0x1c,0x8a,0x3a,0x00,0x02,0x70,0x64,0x66,0x69,
	//------------
	0x6e,0x66,0x6f,0x2e,0x69,0x6e,0x69,0x00,0x6f,0x63,0x74,0x65,0x74,0x00                                                                               
							 
							}; 


	
	dprintf("My MAC= %x : %x : %x : %x : %x : %x \n", eth0_mac[0]&0xff, eth0_mac[1]&0xff,eth0_mac[2]&0xff,
											eth0_mac[3]&0xff,eth0_mac[4]&0xff,eth0_mac[5]&0xff);

	if(argc < 1) 
	{	dprintf("Usage: TFTP pattern number \n");
		return 0;		
	}

	int patt=0;	
	patt = strtoul((const char*)(argv[0]), (char **)NULL, 16);

	



	if(patt==0)
	{

	for(i=0;i<6;i++)
		tx_buffer[6+i]=eth0_mac[i];
//	for(i=0;i<6;i++)
//		tx_buffer[22+i]=eth0_mac[i];
#if CONFIG_SW_DIFFIP
		tx_buffer[31]=5;
#endif
		unsigned int checksum=0; 
		for(i=0;i<60;i++)
			checksum+=tx_buffer[i];
		dprintf("checksum=%x \n", checksum);
		//tx_buffer[60]
	
		swNic_send(tx_buffer,60);//42+4);//strlen(tx_buffer));
	}
	else if(patt==1)
	{
		swNic_send(tx_buffer1,sizeof(tx_buffer1) );//strlen(tx_buffer));
	}
	else if(patt==8)
	{
		/*
		int *p=0x80300000;
		int len=file_length_to_server;
		dprintf("buff=%x, len=%d \n", p, len);
		swNic_send(p,len);//strlen(tx_buffer));
		*/

		//swNic_send(tx_buffer9,42);//strlen(tx_buffer));
	}
	else if(patt==9)
	{		

		//#define MSCR 0xbb804110
 		//REG32(MSCR) = 0;  // for NFBI all pkt coming
 		//REG32(0xbb804048)=1; //skip L2 checksum		
 
		//memset(tx_buffer,0,64);
		//for(i=0;i<60;i++)
		//	tx_buffer[i]=i;
		for(i=0;i<10;i++)
		{
			tx_buffer[16]=i;		
			swNic_send(tx_buffer,60);
		}
		
	}






}
#endif
#endif
/*
------------------------------------------  ---------------------------------
; Command Help
---------------------------------------------------------------------------
*/
  



int CmdHelp( int argc, char* argv[] )
{
	int	i, LineCount ;

    printf("----------------- COMMAND MODE HELP ------------------\n");
	for( i=0, LineCount = 0 ; i < (sizeof(MainCmdTable) / sizeof(COMMAND_TABLE)) ; i++ )
	{
		if( MainCmdTable[i].msg )
		{
			LineCount++ ;
			printf( "%s\n", MainCmdTable[i].msg );
			if( LineCount == PAGE_ECHO_HEIGHT )
			{
				printf("[Hit any key]\r");
				WaitKey();
				printf("	     \r");
				LineCount = 0 ;
			}
		}
	}
	/*Cyrus Tsai*/
#if 0
 	/*Cyrus Tsai*/
    flash_test();   
#endif    
    
	return TRUE ;
}



//---------------------------------------------------------------------------


int YesOrNo(void)
{
	unsigned char iChar[2];

	GetLine( iChar, 2,1);
	printf("\n");//vicadd
	if ((iChar[0] == 'Y') || (iChar[0] == 'y'))
		return 1;
	else
		return 0;
}
//---------------------------------------------------------------------------
#ifdef CONFIG_SPI_FLASH
int CmdSFlw(int argc, char* argv[])
{
	unsigned int  cnt2=0;//strtoul((const char*)(argv[3]), (char **)NULL, 16);	
	unsigned int  dst_flash_addr_offset=strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	unsigned int  src_RAM_addr=strtoul((const char*)(argv[1]), (char **)NULL, 16);
	unsigned int  length=strtoul((const char*)(argv[2]), (char **)NULL, 16);
	unsigned int  end_of_RAM_addr=src_RAM_addr+length;	
	printf("Write 0x%x Bytes to SPI flash#%d, offset 0x%x<0x%x>, from RAM 0x%x to 0x%x\n" ,length,cnt2+1,dst_flash_addr_offset,dst_flash_addr_offset+0xbd000000,src_RAM_addr,end_of_RAM_addr);
	printf("(Y)es, (N)o->");
	if (YesOrNo())
	{
		spi_pio_init();
		  #if defined(SUPPORT_SPI_MIO_8198_8196C) && defined(CONFIG_SPI_FLASH)
			spi_flw_image_mio_8198(cnt2, dst_flash_addr_offset, (unsigned char*)src_RAM_addr , length);	
		  #else			
			spi_flw_image(cnt2, dst_flash_addr_offset, (unsigned char*)src_RAM_addr , length);	
		 #endif
	}//end if YES
	else
		printf("Abort!\n");
}
#endif
//---------------------------------------------------------------------------
#ifdef SUPPORT_NAND 

int CmdNFlr(int argc, char* argv[])
{
	int i;
	unsigned long dst,src;
	unsigned int length;

	if(argc < 3) {
		printf("Parameters not enough!\n");
		return 1;
	}

	dst = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	src = strtoul((const char*)(argv[1]), (char **)NULL, 16);
	length= strtoul((const char*)(argv[2]), (char **)NULL, 16);		

	file_length_to_client=length;

	printf("Read NAND Flash from %X to %X with %X bytes	?\n",src,dst,length);
	printf("(Y)es , (N)o ? --> ");

	if (YesOrNo())
		    if (read_data(src,length,(unsigned char *)dst))
			printf("Read NAND Flash Successed!\n");
		    else
			printf("Read NAND Flash Failed!\n");
	else
		printf("Abort!\n");

}
//---------------------------------------------------------------------------
int CmdNFlw(int argc, char* argv[])
{
	unsigned long dst,src;
	unsigned long length;


	if(argc < 3) {
		printf("Parameters not enough!\n");
		return 1;
	}
	dst = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	src = strtoul((const char*)(argv[1]), (char **)NULL, 16);		
	length= strtoul((const char*)(argv[2]), (char **)NULL, 16);		

	printf("Program NAND flash from %X to %X with %X bytes	?\n",src,dst,length);
	printf("(Y)es, (N)o->");
	if (YesOrNo())
		if (write_data(dst, length, src))
			printf("Write Nand Flash Successed!\n");
		else
			printf("Write Nand Flash Failed!\n");
	else
		printf("Abort!\n");
				
}
#endif

//---------------------------------------------------------------------------

void RunMonitor(char *PROMOPT, COMMAND_TABLE *TestCmdTable, int len)
{
	char		buffer[ MAX_MONITOR_BUFFER +1 ];
	int		argc ;
	char**		argv ;
	int		i, retval ;
	
	
	while(1)
	{	
		//printf( "%s", TEST_PROMPT );
		dprintf( "%s", PROMOPT );
		memset( buffer, 0, MAX_MONITOR_BUFFER );
		GetLine( buffer, MAX_MONITOR_BUFFER,1);
		dprintf( "\n" );
		argc = GetArgc( (const char *)buffer );
		argv = GetArgv( (const char *)buffer );
		if( argc < 1 ) continue ;
		StrUpr( argv[0] );

		if(!strcmp( argv[0], "..") || !strcmp( argv[0], "Q") )
			return;
		
		//for( i=0 ; i < (sizeof(TestCmdTable) / sizeof(COMMAND_TABLE)) ; i++ )
		for( i=0 ; i < (len) ; i++ )
		{
			if( ! strcmp( argv[0], TestCmdTable[i].cmd ) )
			{
				if(TestCmdTable[i].func)
					retval = TestCmdTable[i].func( argc - 1 , argv+1 );
				//dprintf("End run code\n");
				memset(argv[0],0,sizeof(argv[0]));
				break;
			}
		}
		//if(i==sizeof(TestCmdTable) / sizeof(COMMAND_TABLE)) printf("Unknown command !\r\n");
		if(i==len) printf("Unknown command !\r\n");
	}
}

//---------------------------------------------------------------------------

#if defined(CONFIG_PCIE_MODULE) 
int CmdTestPCIE(int argc, char* argv[])
{
	extern void TestMonitorEntry();
	TestMonitorEntry();		
};
#endif
//---------------------------------------------------------------------------
#if defined(CONFIG_R8198EP_HOST) || defined(CONFIG_R8198EP_DEVICE)
int CmdTestSlavePCIE(int argc, char* argv[])
{
	extern void Test_Slv_PCIe_Entry();
	Test_Slv_PCIe_Entry();		
};
#endif
//---------------------------------------------------------------------------

#if 1//defined(CONFIG_R8198EP_HOST) || defined(CONFIG_R8198EP_DEVICE)


#ifdef  CONFIG_SPI_TEST
int CmdSTEST(int argc, char* argv[])
{
	
	unsigned short  test_cnt=strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned long DRAM_starting_addr=strtoul((const char*)(argv[1]), (char **)NULL, 16);
	unsigned int spi_clock_div_num=strtoul((const char*)(argv[2]), (char **)NULL, 16);

	printf("sizeof(unsigned short)=%d\n",sizeof(unsigned short));
	printf("sizeof(unsigned long)=%d\n",sizeof(unsigned long));
	printf("sizeof(unsigned int)=%d\n",sizeof(unsigned int));
	printf("sizeof(unsigned long long)=%d\n",sizeof(unsigned long long));
	//HEX_DRAM_test_starting_addr
	unsigned short i;
	for (i=1;i<=test_cnt;i++)
	{
		
		//auto_memtest(start,end);
		printf("\n================================\n");	
		printf("===  SPI/DRAM Test Count(%d)  ===\n",i);	
		
		printf("================================\n");	

		  #if defined(SUPPORT_SPI_MIO_8198_8196C) && defined(CONFIG_SPI_FLASH)
			auto_spi_memtest_8198(DRAM_starting_addr,spi_clock_div_num);
		  #else
			//auto_spi_memtest(DRAM_starting_addr,spi_clock_div_num); //for 8196 Auto test
		  #endif
	}
}

#endif //end if CONFIG_SPI_TEST


#ifdef CONFIG_NOR_TEST
  //JSW : For NOR/DRAM test  @2007/09/03
int CmdNTEST(int argc, char* argv[])
{
    unsigned short  test_cnt=strtoul((const char*)(argv[0]), (char **)NULL, 16);
    unsigned long DRAM_starting_addr=strtoul((const char*)(argv[1]), (char **)NULL, 16);
    unsigned short i;
    for (i=1;i<=test_cnt;i++)
    {
        //auto_memtest(start,end);
        printf("\n================================\n");
        printf("===  NOR/DRAM Test Count(%d)  ===\n",i);
        printf("================================\n");
        auto_nor_memtest(DRAM_starting_addr);
    }

}
#endif //end if CONFIG_NOR_TEST




        //in spi_flash.c
extern unsigned int rand2(void);
extern unsigned int get_ctimestamp( void );
//extern void srand2( unsigned int seed );
  //unsigned int dram_seed = 0xDeadC0de;


#define MAX_SAMPLE  0x8000
//#define START_ADDR  0x100000               //1MB
#define START_ADDR  0x800000              //8MB
//#define END_ADDR      0x800000		//8MB
#define END_ADDR      0x1000000         //16MB
//#define END_ADDR      0x2000000        //32MB
//#define END_ADDR      0x4000000       //64MB
//#define END_ADDR      0x8000000         //128MB      
#define BURST_COUNTS  256

#ifdef CONFIG_DRAM_TEST
void Dram_test(int argc, char* argv[])
{
    unsigned int i, j,k,k2=0;
    unsigned int cache_type=0;
    unsigned int access_type=0;
    unsigned int addr;
    unsigned int burst=0;
    unsigned long int wdata;
    unsigned int samples,test_range;

    unsigned int enable_delay,delay_time,PM_MODE;//JSW:For DRAM Power Management
       
    unsigned int wdata_array[BURST_COUNTS];         //JSW:It must equal to Burst size

    
   

    /*JSW: Auto set DRAM test range*/
    /*
    //unsigned int END_ADDR;
    if (REG32(MCR_REG)==0x52080000)//8MB
	END_ADDR=0x800000;
    else if ((REG32(MCR_REG)==0x5a080000) ||(REG32(MCR_REG)==0x52480000)) //16MB
       END_ADDR=0x1000000;
    else if ((REG32(MCR_REG)==0x5a480000) ||(REG32(MCR_REG)==0x54480000)) //32MB
       END_ADDR=0x2000000;
    else if ((REG32(MCR_REG)==0x5c480000) ||(REG32(MCR_REG)==0x54880000)) //64MB
       END_ADDR=0x4000000;
    else if(REG32(MCR_REG)==0x5c880000)  //128MB
       END_ADDR=0x4000000;
    */


    unsigned int keep_W_R_mode;
   
    //prom_printf("argc=%d\n",argc);

	
	if(argc<=1)
	{	 		
		prom_printf("ex:dramtest <R/W> <enable_random_delay> <PowerManagementMode>\r\n");
	       prom_printf("ex:<R/W>:<0>=R+W, <1>=R,<2>=W\r\n");
		prom_printf("ex:<enable_random_delay>: <0>=Disable, <1>=Enable\r\n");
		prom_printf("ex:<PowerManagementMode> : <0>=Normal, <1>=PowerDown, <2>=SeldRefresh\r\n");
		prom_printf("    <3>:Reserved,<4>:CPUSleep + Self Refresh in IMEM   \r\n"); 				
		return;	
	}

	 keep_W_R_mode= strtoul((const char*)(argv[0]), (char **)NULL, 16);
        enable_delay= strtoul((const char*)(argv[1]), (char **)NULL, 16);
	 PM_MODE= strtoul((const char*)(argv[2]), (char **)NULL, 16);

    while(1)
    {

      

#if 1                                       //RTL8196_208PIN_SUPPORT_DDR
        prom_printf("\n================================\n");
        k2++;
        prom_printf("\nBegin DRAM Test : %d\n",k2);
        printf("Dram Test parameter\n" );
        printf("1.DDCR(0xb8001050)=%x\n",READ_MEM32(DDCR_REG) );
        printf("2.DTR(0xb8001008)=%x\n",READ_MEM32(DTR_REG) );
        printf("3.DCR(0xb8001004)=%x\n",READ_MEM32(DCR_REG) );
        printf("4.DRAM Freq=%d MHZ\n",check_dram_freq_reg());
        printf("5.Burst Size=%d \n",burst);
        printf("6.cache_type(0:cache)(1:Un-cache)=%d \n",cache_type);
        printf("7.Access_type(0:8bit)(1:16bit)(2:32bit)=%d \n",access_type);
	 printf("8.Tested size=0x%x \n",END_ADDR);
        printf("9.Tested addr=%x \n",addr);

        prom_printf("\n===============================\n");
#endif

     

        for (samples = 0; samples < MAX_SAMPLE; samples++)
        {
            cache_type = rand2() % ((unsigned int) 2);

            access_type = rand2()  % ((unsigned int) 3);
            burst = rand2() % (unsigned int) BURST_COUNTS;

     

            addr = 0x80000000 + START_ADDR + (rand2() % (unsigned int) (END_ADDR - START_ADDR));

   
            addr = cache_type ? (addr | 0x20000000) : addr;
            wdata = rand2();

            if (access_type == 0)
            {
                wdata = wdata & 0xFF;
            }
            else if (access_type == 1)
            {
                addr = (addr) & 0xFFFFFFFE;
                wdata = wdata & 0xFFFF;
            }
            else
            {
                addr = (addr) & 0xFFFFFFFC;
            }

        /* Check if Exceed Limit */
            if ( ((addr + (burst << access_type)) & 0x1FFFFFFF) > END_ADDR)
            {
                burst = (END_ADDR - ((addr) & 0x1FFFFFFF)) >> access_type;
            }

#if 1
            if (samples % 100 == 0)
            {
                printf("\nSamples: %d", samples);
		  
		
		 
		   #if 1 //JSW @20091106 :For DRAM Test + Power Management 
		 	if(enable_delay)
		 	{
			     delay_time=rand2() % ((unsigned int) 1000*1000);
			     prom_printf("  delay_time=%d\n",delay_time);
			     for(k=0;k<=delay_time;k++); //delay_loop				     
		 	}

			
		 	if(PM_MODE)
			{
				
				  //set bit[31:30]=0 for default "Normal Mode"
			    REG32(MPMR_REG)= 0x3FFFFFFF ;

			    switch(PM_MODE)
			    {
			        case 0:
			            dprintf("\nDRAM : Normal mode\n");
				     //return 0;
			            break;

			        case 1:
			            dprintf("\nDRAM :Auto Power Down mode\n");
			            REG32(MPMR_REG)= READ_MEM32(MPMR_REG)|(0x1 <<30) ;
				     //return 0;
			            break;

			        case 2:
			            dprintf("\nDRAM : Self Refresh mode\n");
				     REG32(MPMR_REG)= 0x3FFFFFFF ;
				     delay_ms(100);
			            REG32(MPMR_REG)|= (0x2 <<30) ;
				     delay_ms(100);
			            REG32(MPMR_REG)|= (0x2 <<30) ;	
				     delay_ms(100);
				     //return 0;	
			            break;

			        case 3:
			            dprintf("\nReserved!\n");
			            //REG32(MPMR_REG)= READ_MEM32(MPMR_REG)|(0x3 <<30) ;
			            REG32(MPMR_REG)= 0x3FFFFFFF ;
				     //return 0;
			            break;
				#ifdef CONFIG_CPUsleep_PowerManagement_TEST
				case 4:
			            dprintf("\nCPUSleep + Self Refresh in IMEM!\n");
				     CmdCPUSleepIMEM();
			            //return 0;
			            break;
				#endif
			        default :
			            dprintf("\nError Input,should be 0~4\n");
			            break;
			     }   //end of switch(PM_MODE)
		 	}//end of if(PM_MODE)			
		 #endif
		 
            }//end of switch(PM_MODE)
#endif

     

        /* Prepare Write Data */
            for (i = 0; i < burst ; i++)
            {            
                    wdata = (unsigned int)(rand2());// ???

                if (access_type == 0)               //8 bit
                    wdata = wdata & 0xFF;
                else if (access_type == 1)          //16bit
                    wdata = wdata & 0xFFFF;

                wdata_array[i] = wdata;
     
            }

            keep_writing:
        /* Write */
            for (i = 0, j = addr; i < burst ; i++)
            {
                if (access_type == 0)
                    *(volatile unsigned char *) (j) = wdata_array[i];
                else if (access_type == 1)
                    *(volatile unsigned short *) (j) = wdata_array[i];
                else
                    *(volatile unsigned int *) (j) = wdata_array[i];

                j = j + (1 << access_type);
        //keep writing
                if (keep_W_R_mode==1)
                {
                    goto keep_writing;
                }
            }

            if (keep_W_R_mode==2)
            {
                keep_reading:
                WRITE_MEM32(0xa0800000,0xa5a55a5a);
       
                goto keep_reading;
            }

        /* Read Verify */
            for (i = 0, j = addr; i < burst ; i++)
            {
                unsigned rdata;

                if (access_type == 0)
                {
                    rdata = *(volatile unsigned char *) (j);
                }
                else if (access_type == 1)
                {
                    rdata = *(volatile unsigned short *) (j);
                }
                else
                {
                    rdata = *(volatile unsigned int *) (j);
                }
        //printf("\n==========In Read Verify========= \n");
        // printf("\nrdata: %d\n", rdata);
        //printf("\nwdata_array[i]: %d\n",wdata_array[i]);
        // printf("\n==========End Read Verify========= \n");

                if (rdata != wdata_array[i])
                {
                    printf("\nWrite Data Array: 0x%X", wdata_array[i]);

                    if (cache_type)
                        printf("\n==> Uncached Access Address: 0x%X, Type: %d bit, Burst: %d",
                            addr, (access_type == 0) ? 8 : (access_type == 1) ? 16 : 32, burst);
                    else
                        printf("\n==>   Cached Access Address: 0x%X, Type: %d bit, Burst: %d",
                            addr, (access_type == 0) ? 8 : (access_type == 1) ? 16 : 32, burst);

                    printf("\n====> Verify Error! Addr: 0x%X = 0x%X, expected to be 0x%X\n", j, rdata, wdata_array[i]);

        //HaltLoop:
        //goto HaltLoop;
                    return 0;

                }

                j = j + (1 << access_type);

            }                                       //end of reading

        }

    }                                               //end while(1)
}
#endif


#endif




   // 1 sec won't sleep,5 secs will sleep
#define MPMR_REG 0xB8001040
#ifdef CONFIG_CPUsleep_PowerManagement_TEST
int CmdCPUSleep(int argc, char* argv[])
{

     dprintf("\nCPU Sleep..\n");

   /*
            PM_MODE=0 , normal
            PM_MODE=1 , auto power down
            PM_MODE=2 , seld refresh
        */
    if( !argv[0])                                   //read
    {
        dprintf("Usage: sleep <0~2>  \r\n");
	 dprintf("sleep <0>:CPU sleep + DRAM Normal mode \r\n"); 
	 dprintf("sleep <1>:CPU sleep + DRAM Power down  \r\n"); 
	 dprintf("sleep <2>:CPU sleep + DRAM Self refresh  \r\n"); 
	 dprintf("sleep <3>:Reserved  \r\n"); 
	 dprintf("sleep <4>:CPUSleep + Self Refresh in IMEM  \r\n"); 
        return;
    }   
#if 0//def timer_test
    unsigned short sleep_time = strtoul((const char*)(argv[0]), (char **)NULL, 16);
    dprintf("About to sleep 0x%x secs\n",sleep_time );


    tc0_init(sleep_time);
        //tc1_init(sleep_time);
        //tc2_init(sleep_time);
        //tc3_init(sleep_time);


        // REG32(GIMR_REG) =  0x0;//Disable all interrupt

        //dprintf("GIMR_REG = 0x%x\n",*(volatile unsigned int *)GIMR_REG);
    dprintf("GIMR_REG = 0x%x\n",*(volatile unsigned int *)GIMR_REG);
    dprintf("GISR_REG = 0x%x\n",*(volatile unsigned int *)GISR_REG);
    dprintf("TCCNR_REG = 0x%x\n",*(volatile unsigned int *)TCCNR_REG);
    dprintf("TCIR_REG = 0x%x\n",*(volatile unsigned int *)TCIR_REG);

    unsigned short  i;
    for (i=0;i<=100;i++)
        dprintf("TC1CNT_REG = 0x%x\n",*(volatile unsigned int *)TC1CNT_REG);

    dprintf("GIMR_REG = 0x%x\n",*(volatile unsigned int *)GIMR_REG);
    dprintf("GISR_REG = 0x%x\n",*(volatile unsigned int *)GISR_REG);
    dprintf("TCCNR_REG = 0x%x\n",*(volatile unsigned int *)TCCNR_REG);
    dprintf("TCIR_REG = 0x%x\n",*(volatile unsigned int *)TCIR_REG);

        // Enter Sleep Mode
    dprintf("\nCPU Enter Sleep Mode ,it will wake up after %x secs.....\n",sleep_time);
#endif

 unsigned short PM_MODE = strtoul((const char*)(argv[0]), (char **)NULL, 16);


if(PM_MODE)
{
	
	  //set bit[31:30]=0 for default "Normal Mode"
    REG32(MPMR_REG)= 0x3FFFFFFF ;

    switch(PM_MODE)
    {
        case 0:
            dprintf("\nDRAM : Normal mode\n");
            break;

        case 1:
            dprintf("\nDRAM :Auto Power Down mode\n");
            REG32(MPMR_REG)= READ_MEM32(MPMR_REG)|(0x1 <<30) ;
            break;

        case 2:
            dprintf("\nDRAM : Self Refresh mode\n");
	     REG32(MPMR_REG)= 0x3FFFFFFF ;
	      delay_ms(1000);
            REG32(MPMR_REG)|= (0x2 <<30) ;
	    delay_ms(1000);
            REG32(MPMR_REG)|= (0x2 <<30) ;
			
            break;

        case 3:
            dprintf("\nReserved!\n");
            REG32(MPMR_REG)= READ_MEM32(MPMR_REG)|(0x3 <<30) ;
            break;

	case 4:
            dprintf("\nCPUSleep + Self Refresh in IMEM!\n");
	     CmdCPUSleepIMEM();
            
            break;

        default :
            dprintf("\nError Input,should be 0~3\n");
            break;
    }

    //dprintf("After setting, MPMR(0xB8001040)=%x\n",READ_MEM32(MPMR_REG) );
}                                                   //End of DRAMPM



   
     REG32(GIMR_REG)=0x0;
	//cli();    
   
        //JSW: SLEEP
  
    __asm__ __volatile__ (    "sleep\n\t"   );    

      //JSW: Make sure CPU do sleep and below won't be printed
     delay_ms(1000);                         //delay 1.25 sec in 40MHZ(current OSC), 25/40=1.25 sec
        // After Counter Trigger interrupt
      dprintf("Counter Trigger interrupt,CPI Leave Sleep...\n");

}



#define __IRAM_IN_865X      __attribute__ ((section(".iram-rtkwlan")))
#define __IRAM_FASTEXTDEV        __IRAM_IN_865X
__IRAM_FASTEXTDEV
void CmdCPUSleepIMEM()
{

	 
      //while( (*((volatile unsigned int *)(0xb8001050))& 0x40000000) != 0x40000000);

      //dprintf("\nDRAM : Self Refresh mode IMEM01\n");

	     REG32(MPMR_REG)= 0x3FFFFFFF ;
	     delay_ms(1000);
            REG32(MPMR_REG)|= (0x2 <<30) ;
	     delay_ms(1000);
            REG32(MPMR_REG)|= (0x2 <<30) ;


   
     REG32(GIMR_REG)=0x0;
	//cli();    
   
        //JSW: SLEEP  
    __asm__ __volatile__ (    "sleep\n\t"   );    

      //JSW: Just make sure CPU do sleep and below won't be printed
     delay_ms(1000);                        
     dprintf("Counter Trigger interrupt,CPI Leave Sleep...\n");

}



#endif  //end if CONFIG_CPUsleep_PowerManagement_TEST





//anson add
#ifdef CMD_PHY_RW
int CmdPHYregR(int argc, char* argv[])
{
    unsigned long phyid, regnum;
    unsigned int uid,tmp;

    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 16);
    regnum = strtoul((const char*)(argv[1]), (char **)NULL, 16);

    rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );
    uid=tmp;
    dprintf("PHYID=0x%x, regID=0x%x ,Find PHY Chip! UID=0x%x\r\n", phyid, regnum, uid);
}

int CmdPHYregW(int argc, char* argv[])
{
    unsigned long phyid, regnum;
    unsigned long data;
    unsigned int uid,tmp;

    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 16);
    regnum = strtoul((const char*)(argv[1]), (char **)NULL, 16);
    data= strtoul((const char*)(argv[2]), (char **)NULL, 16);

    rtl8651_setAsicEthernetPHYReg( phyid, regnum, data );
    rtl8651_getAsicEthernetPHYReg( phyid, regnum, &tmp );
    uid=tmp;
    dprintf("PHYID=0x%x ,regID=0x%x, Find PHY Chip! UID=0x%x\r\n", phyid, regnum, uid);
}
#endif

#if 0
int CmdGPHYW(int argc, char* argv[])
{

    unsigned long val, reg, page, phyid;
 	unsigned int wphyid=0;	//start
	unsigned int wphyid_end=1;   //end
    unsigned int uid,tmp;

    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 10);
    page = strtoul((const char*)(argv[1]), (char **)NULL, 10);
    reg = strtoul((const char*)(argv[2]), (char **)NULL, 10);
    val= strtoul((const char*)(argv[3]), (char **)NULL, 16);

	if(phyid==999)
	{	wphyid=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	wphyid=phyid;
		wphyid_end=phyid+1;
	}

	for(; wphyid<wphyid_end; wphyid++)
	{
		//change page 
		if(page>=40)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}

 		rtl8651_setAsicEthernetPHYReg( wphyid, reg, val  );
 		rtl8651_getAsicEthernetPHYReg( wphyid, reg, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,Page=%d ,regID=%d, Find PHY Chip! UID=0x%x\r\n", wphyid, page, reg, uid);

 		{	
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
 		}
 	}

}

int CmdGPHYR(int argc, char* argv[])
{

    unsigned long val, reg, page, phyid, length;
 	unsigned int wphyid=0;	//start
	unsigned int wphyid_end=1;   //end
    unsigned int uid,tmp,lngth,regtmp;

    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 10);
    page = strtoul((const char*)(argv[1]), (char **)NULL, 10);
    reg = strtoul((const char*)(argv[2]), (char **)NULL, 10);
    length = strtoul((const char*)(argv[3]), (char **)NULL, 10);

	
	if(phyid==999)
	{	wphyid=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	wphyid=phyid;
		wphyid_end=phyid+1;
	}

	for(; wphyid<wphyid_end; wphyid++)
	{
		//change page 
		if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}
		for(lngth=0; lngth<length ; lngth++)
		{
		regtmp = reg + lngth;
 		rtl8651_getAsicEthernetPHYReg( wphyid, regtmp, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,Page=%d ,regID=%d, Find PHY Chip! UID=0x%x\r\n", wphyid, page, regtmp, uid);
		}
 		{	
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
 		}
	   	dprintf("\n");

 	}

}
#endif

#ifdef CONFIG_RTL8198_TAROKO
int CmdIMEM98TEST(int argc, char* argv[])
{
	imem_test();
	return 0;
}

int CmdWBMG(int argc, char* argv[])
{
	write_buf_merge();
	return 0;
}

#endif


#if 0
int CmdRtlossG( int argc, char* argv[] ) //test GIGA return loss
{
//Stop microC
	Set_GPHYWB(999, 5, 5, 0, 0xFFF6);
	Set_GPHYWB(999, 5, 6, 0, 0);
//stop microC clock
	Set_GPHYWB(999, 5, 0, 0, 0xc6);
//Disable aldps
	Set_GPHYWB(999, 44, 21, 0, 0x350);
//Disable power saving
	Set_GPHYWB(999, 0, 21, 0, 0x6);
//Config Test mode
	Set_GPHYWB(999, 0, 9, 0, 0x8e00);
	
dprintf("\nstart GIGA return loss\n\n");

}	

int CmdRtloss( int argc, char* argv[] )	//test 10/100M return loss
{
//force link 100M
/*    REG32(0xbb804104) = 0x02EF003F;
    REG32(0xbb804108) = 0x06EF003F;
    REG32(0xbb80410c) = 0x0AEF003F;
    REG32(0xbb804110) = 0x0EEF003F;
    REG32(0xbb804114) = 0x12EF003F;
*/
	Set_GPHYWB(999, 0, 0, 0, 0x2100);
//Disable power saving
	Set_GPHYWB(999, 0, 21, 0, 6);
//force MDI
	Set_GPHYWB(999, 0, 16, 0, 0x5AE);
	Set_GPHYWB(999, 45, 24, 0, 0xF020);

	
dprintf("\nstart 100M return loss\n\n");

}	
#endif
#if gphy_rom_loop
int ReadROMcut( int argc, char* argv[] )
{

    unsigned long phyid;
 	unsigned int wphyid=0;	//start
	unsigned int wphyid_end=1;   //end
    unsigned int uid,tmp,rd;

    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 10);
/*

write rg31 0x0005
write rg05 0x8500
read rg06 -> pass count
write rg05 0x8502
read rg06 -> {fail count (h-byte), fail count (max_toggle) (h-byte)}
write rg05 0x8504
read rg06 -> fail count (l-byte), fail count (max_toggle) (l-byte)} 
write rg05 0x8506
read rg06 -> fail checksum
write rg05 0x8508
read rg06 -> fail checksum (max toggle)
 
*/
	if(phyid==999)
	{	wphyid=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	wphyid=phyid;
		wphyid_end=phyid+1;
	}

	for(; wphyid<wphyid_end; wphyid++)
	{
		//change page 
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 5);
		for(rd=1;rd<=2;rd++)
		{
		rtl8651_setAsicEthernetPHYReg( wphyid, 5, 0x8500);
 		rtl8651_getAsicEthernetPHYReg( wphyid, 6, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,PASS count=%d\r\n", wphyid, uid);
		rtl8651_setAsicEthernetPHYReg( wphyid, 5, 0x8502);
 		rtl8651_getAsicEthernetPHYReg( wphyid, 6, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,fail count (max_toggle) (h-byte)=%d\r\n", wphyid, uid);
		rtl8651_setAsicEthernetPHYReg( wphyid, 5, 0x8504);
 		rtl8651_getAsicEthernetPHYReg( wphyid, 6, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,fail count (max_toggle) (l-byte)=%d\r\n", wphyid, uid);
		rtl8651_setAsicEthernetPHYReg( wphyid, 5, 0x8506);
 		rtl8651_getAsicEthernetPHYReg( wphyid, 6, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,fail checksum====================%d\r\n", wphyid, uid);
		rtl8651_setAsicEthernetPHYReg( wphyid, 5, 0x8508);
 		rtl8651_getAsicEthernetPHYReg( wphyid, 6, &tmp  );
	    uid=tmp;
    	dprintf("PHYID=0x%x ,fail checksum (max toggle)=======%d\r\n", wphyid, uid);
		}
		rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
 		
	   	dprintf("\n");

 	}


}	
#endif

#if test_mdcmdio
int Testmdcmdio( int argc, char* argv[] )
{

    unsigned long number,phyid,reg,val;
    unsigned long val0,val1,val2,val3,val4,val5,val6,val7,val8,val9;
    unsigned int i,uid,tmp,rd,check;

    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 10);
    reg = strtoul((const char*)(argv[1]), (char **)NULL, 10);
    val = strtoul((const char*)(argv[2]), (char **)NULL, 16);
    number = strtoul((const char*)(argv[3]), (char **)NULL, 10);
    check = strtoul((const char*)(argv[4]), (char **)NULL, 10);
    val0 = strtoul((const char*)(argv[5]), (char **)NULL, 16);
	val1 = strtoul((const char*)(argv[6]), (char **)NULL, 16);
	val2 = strtoul((const char*)(argv[7]), (char **)NULL, 16);
	val3 = strtoul((const char*)(argv[8]), (char **)NULL, 16);
	val4 = strtoul((const char*)(argv[9]), (char **)NULL, 16);
	val5 = strtoul((const char*)(argv[10]), (char **)NULL, 16);
	val6 = strtoul((const char*)(argv[11]), (char **)NULL, 16);
	val7 = strtoul((const char*)(argv[12]), (char **)NULL, 16);
	val8 = strtoul((const char*)(argv[13]), (char **)NULL, 16);
	val9= strtoul((const char*)(argv[14]), (char **)NULL, 16);

		for(rd=1;rd<=number;rd++)
		{
		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0);
		delay_ms(10);
		if(check==1)
		{
 		rtl8651_getAsicEthernetPHYReg( phyid, reg, &tmp  );
	    uid=tmp;
		if(uid==val)
	    	dprintf("port%d Read phyReg%d = 0x%x PASS\r\n",phyid,reg,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}
		else
		{
		if((rd%10)==0)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 0, &tmp );
	    uid=tmp;
		if(uid==val0)
	    	dprintf("port%d Read phyReg0 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}
//############################################################################################################################
		else if((rd%10)==1)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 1, &tmp );
	    uid=tmp;
		if(uid==val1)
	    	dprintf("port%d Read phyReg1 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else if((rd%10)==2)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 2, &tmp );
	    uid=tmp;
		if(uid==val2)
	    	dprintf("port%d Read phyReg2 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else if((rd%10)==3)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 3, &tmp );
	    uid=tmp;
		if(uid==val3)
	    	dprintf("port%d Read phyReg3 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else if((rd%10)==4)
		{
		if((rd%4)==0)
		{
		rtl8651_setAsicEthernetPHYReg( phyid, 4, 0xd41 );
	    rtl8651_getAsicEthernetPHYReg( phyid, 4, &tmp );
	    uid=tmp;
		if(uid==0xd41)
	    	dprintf("port%d Read phyReg4 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


		if((rd%4)==1)
		{
		rtl8651_setAsicEthernetPHYReg( phyid, 4, 0xc01 );
	    rtl8651_getAsicEthernetPHYReg( phyid, 4, &tmp );
	    uid=tmp;
		if(uid==0xc01)
	    	dprintf("port%d Read phyReg4 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


		if((rd%4)==2)
		{
		rtl8651_setAsicEthernetPHYReg( phyid, 4, 0xca1 );
	    rtl8651_getAsicEthernetPHYReg( phyid, 4, &tmp );
	    uid=tmp;
		if(uid==0xca1)
	    	dprintf("port%d Read phyReg4 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


		if((rd%4)==3)
		{
		rtl8651_setAsicEthernetPHYReg( phyid, 4, 0x1e1 );
	    rtl8651_getAsicEthernetPHYReg( phyid, 4, &tmp );
	    uid=tmp;
		if(uid==0x1e1)
	    	dprintf("port%d Read phyReg4 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}
		}


//############################################################################################################################
		else if((rd%10)==5)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 5, &tmp );
	    uid=tmp;
		if(uid==val5)
	    	dprintf("port%d Read phyReg5 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else if((rd%10)==6)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 6, &tmp );
	    uid=tmp;
		if(uid==val6)
	    	dprintf("port%d Read phyReg6 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else if((rd%10)==7)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 7, &tmp );
	    uid=tmp;
		if(uid==val7)
	    	dprintf("port%d Read phyReg7 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else if((rd%10)==8)
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 8, &tmp );
	    uid=tmp;
		if(uid==val8)
	    	dprintf("port%d Read phyReg8 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}


//############################################################################################################################
		else
		{
		rtl8651_getAsicEthernetPHYReg( phyid, 9, &tmp );
	    uid=tmp;
		if(uid==val9)
	    	dprintf("port%d Read phyReg9 = 0x%x   PASS\r\n",phyid,uid);
		else
			{
			dprintf("port%d Read phyReg%d = 0x%x FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL FAIL \r\n",phyid,reg,uid);
			dprintf("port%d %d's Fail  \r\n",phyid,rd-1);
		return 0;
			}
		}
		}
		}
	dprintf("port%d PASS %d'S \r\n",phyid,rd-1);
}	
#endif

#if test_compatibility
int compatibility( int argc, char* argv[] )
{

    unsigned long number,phyid,val,LINK,sum=0;
    unsigned int i,j,uid,tmp,rd;
    unsigned int wphyid,wphyid_end;
    unsigned int speed1,speed2;
    unsigned int pass10,pass100,passgiga;
    unsigned int fail,snr;
//    int snr;

//    phyid = strtoul((const char*)(argv[0]), (char **)NULL, 10);
    wphyid = strtoul((const char*)(argv[0]), (char **)NULL, 10);
    wphyid_end = strtoul((const char*)(argv[1]), (char **)NULL, 10);
    number = strtoul((const char*)(argv[2]), (char **)NULL, 10);
    speed1 = strtoul((const char*)(argv[3]), (char **)NULL, 10);
    speed2 = strtoul((const char*)(argv[4]), (char **)NULL, 10);

//dprintf("phyid = %d \r\n",phyid);
//dprintf("number = %d \r\n",number);

//	if(phyid==999)
//	{	wphyid=0;
//		wphyid_end=4;    //total phyid=0~4
//	}
//	else
//	{	wphyid=phyid;
//		wphyid_end=phyid;
//	}
///////////////////////////////////////////////////////
//dprintf("wphyid = %d \r\n",wphyid);
//dprintf("wphyid_end = %d \r\n",wphyid_end);


for(i=wphyid; i<=wphyid_end; i++)
{
	pass10=0;
	pass100=0;
	passgiga=0;
	fail=0;
	
//##########  enable 10M EEE  ###########//
REG32(PCRP0+i*4) |= (EnForceMode);
Set_GPHYWB(i, 32, 21, 0, 0x1200);
REG32(PCRP0+i*4) &= ~(EnForceMode);

	
	if(i==0)
		LINK=PSRP0;
	else if(i==1)
		LINK=PSRP1;
	else if(i==2)
		LINK=PSRP2;
	else if(i==3)
		LINK=PSRP3;
	else if(i==4)
		LINK=PSRP4;
	else
		return 0;
	
for(j=speed2;j>=speed1;j--)
{
	if(j==1)//test 10M compatibility
	{
		rtl8651_getAsicEthernetPHYReg( i, 9, &tmp);
//dprintf("100M tmp = 0x%x \r\n",tmp);
		uid=tmp & ~(1<<9);
//dprintf("100M uid = 0x%x \r\n",uid);
		rtl8651_setAsicEthernetPHYReg( i, 9, uid);//disable giga ability
		
		rtl8651_getAsicEthernetPHYReg( i, 4, &tmp);
//dprintf("10M tmp = 0x%x \r\n",tmp);
		uid=tmp & ~(3<<7);
//dprintf("10M uid = 0x%x \r\n",uid);
		rtl8651_setAsicEthernetPHYReg( i, 4, uid);//disable 100M ability

	}
	else if(j==2)//test 100M compatibility
	{
		rtl8651_getAsicEthernetPHYReg( i, 9, &tmp);
//dprintf("100M tmp = 0x%x \r\n",tmp);
		uid=tmp & ~(1<<9);
//dprintf("100M uid = 0x%x \r\n",uid);
		rtl8651_setAsicEthernetPHYReg( i, 9, uid);//disable giga ability

		rtl8651_getAsicEthernetPHYReg( i, 0, &tmp);
//dprintf("100M tmp = 0x%x \r\n",tmp);
		uid=tmp | 1<<9;
//dprintf("100M uid = 0x%x \r\n",uid);
		rtl8651_setAsicEthernetPHYReg( i, 0, uid);//restart nway
	/*##########################         SNR          ################################
					rtl8651_getAsicEthernetPHYReg( i, 0, &tmp);//set signle	[mdcmdio_cmd r $phyID 29 -v]
					uid=tmp
					sum=sum+uid;//set sum [expr $sum + $signle]

					if(uid==0)
						dprintf("No SNR data\n");
					else
					{	
						snr=-10*log10(sum*2/262144);//set SNR_dB [expr -10*log10(double($SNR_Value)/262144.0)]
						dprintf("port%d SNR = %d\n",i,snr);//set per_msg [format "#Loop= %1s  PHY= %s  SNR = %f\n" $i $phyID $SNR_dB]
					}			
						}
					

/*##########################         SNR          ################################*/


	}
	else if(j==3)
	{
	}
	else
		return 0;

delay_ms(5000);

		for(rd=1;rd<=number;rd++)
		{
//dprintf("rd = %d \r\n",rd);
//		REG32(PCRP0+i*4) |= (EnForceMode);//port force mode
		rtl8651_getAsicEthernetPHYReg( i, 0, &tmp);
		uid=tmp | 1<<11;
		rtl8651_setAsicEthernetPHYReg( i, 0, uid);//port power down
		
//		rtl8651_getAsicEthernetPHYReg( i, 0, &tmp);
//		uid=tmp & ~(1<<11);
//		rtl8651_setAsicEthernetPHYReg( i, 0, uid);//port power on
//		REG32(PCRP0+i*4) &= ~(EnForceMode);

		delay_ms(5000);
		tmp=REG32(LINK);
		uid= tmp & 0x13;
		
//dprintf("tmp = 0x%x \r\n",tmp);
//dprintf("LINK = 0x%x \r\n",LINK);
//dprintf("uid = 0x%x \r\n",uid);

		if(uid==0x10)
			{
			if(rd==1)
	    	dprintf("port%d link up 10M PASS %d'S\r\n",i,rd);
			else
			dprintf(".");
			pass10=pass10+1;
			}
		else if(uid==0x11)
	    	{
			if(rd==1)
	    	dprintf("port%d link up 100M PASS %d'S\r\n",i,rd);
			else
			dprintf(".");
			pass100=pass100+1;
			}
		else if(uid==0x12)
	    	{
			if(rd==1)
	    	dprintf("port%d link up GIGA PASS %d'S\r\n",i,rd);
			else
			dprintf(".");
			passgiga=passgiga+1;
			}
		else
			{
			dprintf("port%d link up FAIL FAIL FAIL FAIL FAIL %d's \r\n",i,rd);
			fail=fail+1;
			}
		}
}
		dprintf("\n");
		dprintf("@@@@@  port%d link up 10M PASS %d'S\r\n",i,pass10);
		dprintf("@@@@@  port%d link up 100M PASS %d'S\r\n",i,pass100);
		dprintf("@@@@@  port%d link up GIGA PASS %d'S\r\n",i,passgiga);
		dprintf("@@@@@  port%d link up 10M FAIL FAIL FAIL FAIL %d'S\r\n\n",i,fail);

rtl8651_setAsicEthernetPHYReg( i, 9, 0xe00);
rtl8651_setAsicEthernetPHYReg( i, 4, 0xde1);
rtl8651_setAsicEthernetPHYReg( i, 0, 0x1300);//reset power on default

}	
}	
#endif

#if spd
int cmdspd( int argc, char* argv[] )
{

    unsigned long spd1,tmp,uid;
    int i;

    spd1 = strtoul((const char*)(argv[0]), (char **)NULL, 10);

for(i=0; i<=5; i++)
{
//	REG32(PCRP0+i*4) |= (EnForceMode);

	if(spd1==10)//test 10M compatibility
	{
		rtl8651_getAsicEthernetPHYReg( i, 9, &tmp);
		uid=tmp & ~(1<<9);
		rtl8651_setAsicEthernetPHYReg( i, 9, uid);//disable giga ability
		
		rtl8651_getAsicEthernetPHYReg( i, 4, &tmp);
		uid=tmp & ~(3<<7);
		rtl8651_setAsicEthernetPHYReg( i, 4, uid);//disable 100M ability

	}
	else if(spd1==100)//test 100M compatibility
	{
		rtl8651_getAsicEthernetPHYReg( i, 9, &tmp);
		uid=tmp & ~(1<<9);
		rtl8651_setAsicEthernetPHYReg( i, 9, uid);//disable giga ability

		rtl8651_getAsicEthernetPHYReg( i, 0, &tmp);
		uid=tmp | 1<<9;
		rtl8651_setAsicEthernetPHYReg( i, 0, uid);//restart nway

	}
	else if(spd1==1000)
	{
		rtl8651_setAsicEthernetPHYReg( i, 9, 0xe00);
		rtl8651_setAsicEthernetPHYReg( i, 4, 0xde1);
		rtl8651_setAsicEthernetPHYReg( i, 0, 0x1300);//reset power on default
	}
	else
		return 0;

//for(i=0; i<=5; i++)
//	REG32(PCRP0+i*4) &= ~(EnForceMode);
}

}
#endif




#if add_green
int greenC( int argc, char* argv[] )
{

    unsigned long lenght;
    int i;

    lenght = strtoul((const char*)(argv[0]), (char **)NULL, 10);

for(i=0; i<=5; i++)
	REG32(PCRP0+i*4) |= (EnForceMode);

		if(lenght==0x3)
			{
			Set_GPHYWB(999, 72, 21, 0, 0x7012);
			Set_GPHYWB(999, 72, 22, 0, 0x7012);
			Set_GPHYWB(999, 72, 23, 0, 0x7012);
			Set_GPHYWB(999, 72, 24, 0, 0x7012);
			Set_GPHYWB(999, 72, 25, 0, 0x7012);
			Set_GPHYWB(999, 72, 26, 0, 0x7012);

			Set_GPHYWB(999, 2, 7 , 0, 0x3620);
			Set_GPHYWB(999, 1, 27, 0, 0xc414);
			Set_GPHYWB(999, 1, 12, 0, 0xdbf0);

	    	dprintf("Ethernet length > 110M \r\n");
			}
		else if(lenght==0x2)
	    	{
			Set_GPHYWB(999, 72, 21, 0, 0x6412);
			Set_GPHYWB(999, 72, 22, 0, 0x6412);
			Set_GPHYWB(999, 72, 23, 0, 0x6412);
			Set_GPHYWB(999, 72, 24, 0, 0x6412);
			Set_GPHYWB(999, 72, 25, 0, 0x6412);
			Set_GPHYWB(999, 72, 26, 0, 0x6412);

			Set_GPHYWB(999, 2, 7 , 0, 0x3620);
			Set_GPHYWB(999, 1, 27, 0, 0xb414);
			Set_GPHYWB(999, 1, 12, 0, 0xdbf0);

	    	dprintf("Ethernet 20M < length < 110M \r\n");
			}
		else if(lenght==0x1)
	    	{
			Set_GPHYWB(999, 72, 21, 0, 0x6002);
			Set_GPHYWB(999, 72, 22, 0, 0x6002);
			Set_GPHYWB(999, 72, 23, 0, 0x6002);
			Set_GPHYWB(999, 72, 24, 0, 0x6002);
			Set_GPHYWB(999, 72, 25, 0, 0x6002);
			Set_GPHYWB(999, 72, 26, 0, 0x6002);

			Set_GPHYWB(999, 2, 7 , 0, 0x3610);
			Set_GPHYWB(999, 1, 27, 0, 0x8414);
			Set_GPHYWB(999, 1, 12, 0, 0x1bf0);

	    	dprintf("Ethernet length < 20M \r\n");
			}
		else
			{
			dprintf("Enter length error. \r\n");
			return 0;
			}
		
for(i=0; i<=5; i++)
	REG32(PCRP0+i*4) &= ~(EnForceMode);


}
#endif

//----------------------------------------------------------------------------------------------
#ifdef RTL8198
int CmdEEEPatch(int argc, char* argv[])
{

unsigned int p[]={
 0x1f, 0x0000,
 0x17, 0x2179,
 0x1f, 0x0007,
 0x1e, 0x0040,
 0x18, 0x0004,
 0x18, 0x0004,
 0x19, 0x4000,
 0x18, 0x0014,
 0x19, 0x7f00,
 0x18, 0x0024,
 0x19, 0x0000,
 0x18, 0x0034,
 0x19, 0x0100,
 0x18, 0x0044,
 0x19, 0xe000,
 0x18, 0x0054,
 0x19, 0x0000,
 0x18, 0x0064,
 0x19, 0x0000,
 0x18, 0x0074,
 0x19, 0x0000,
 0x18, 0x0084,
 0x19, 0x0400,
 0x18, 0x0094,
 0x19, 0x8000,
 0x18, 0x00a4,
 0x19, 0x7f00,
 0x18, 0x00b4,
 0x19, 0x4000,
 0x18, 0x00c4,
 0x19, 0x2000,
 0x18, 0x00d4,
 0x19, 0x0100,
 0x18, 0x00e4,
 0x19, 0x8400,
 0x18, 0x00f4,
 0x19, 0x7a00,
 0x18, 0x0104,
 0x19, 0x4000,
 0x18, 0x0114,
 0x19, 0x3f00,
 0x18, 0x0124,
 0x19, 0x0100,
 0x18, 0x0134,
 0x19, 0x7800,
 0x18, 0x0144,
 0x19, 0x0000,
 0x18, 0x0154,
 0x19, 0x0000,
 0x18, 0x0164,
 0x19, 0x0000,
 0x18, 0x0174,
 0x19, 0x0400,
 0x18, 0x0184,
 0x19, 0x8000,
 0x18, 0x0194,
 0x19, 0x7f00,
 0x18, 0x01a4,
 0x19, 0x8300,
 0x18, 0x01b4,
 0x19, 0x8300,
 0x18, 0x01c4,
 0x19, 0xe100,
 0x18, 0x01d4,
 0x19, 0x9c00,
 0x18, 0x01e4,
 0x19, 0x8800,
 0x18, 0x01f4,
 0x19, 0x0300,
 0x18, 0x0204,
 0x19, 0xe100,
 0x18, 0x0214,
 0x19, 0x0800,
 0x18, 0x0224,
 0x19, 0x4000,
 0x18, 0x0234,
 0x19, 0x7f00,
 0x18, 0x0244,
 0x19, 0x0400,
 0x18, 0x0254,
 0x19, 0x0100,
 0x18, 0x0264,
 0x19, 0x4000,
 0x18, 0x0274,
 0x19, 0x3e00,
 0x18, 0x0284,
 0x19, 0x0000,
 0x18, 0x0294,
 0x19, 0xe000,
 0x18, 0x02a4,
 0x19, 0x1200,
 0x18, 0x02b4,
 0x19, 0x8000,
 0x18, 0x02c4,
 0x19, 0x7f00,
 0x18, 0x02d4,
 0x19, 0x8900,
 0x18, 0x02e4,
 0x19, 0x8300,
 0x18, 0x02f4,
 0x19, 0xe000,
 0x18, 0x0304,
 0x19, 0x0000,
 0x18, 0x0314,
 0x19, 0x4000,
 0x18, 0x0324,
 0x19, 0x7f00,
 0x18, 0x0334,
 0x19, 0x0000,
 0x18, 0x0344,
 0x19, 0x2000,
 0x18, 0x0354,
 0x19, 0x4000,
 0x18, 0x0364,
 0x19, 0x3e00,
 0x18, 0x0374,
 0x19, 0xfd00,
 0x18, 0x0384,
 0x19, 0x0000,
 0x18, 0x0394,
 0x19, 0x1200,
 0x18, 0x03a4,
 0x19, 0xab00,
 0x18, 0x03b4,
 0x19, 0x0c00,
 0x18, 0x03c4,
 0x19, 0x0600,
 0x18, 0x03d4,
 0x19, 0xa000,
 0x18, 0x03e4,
 0x19, 0x3d00,
 0x18, 0x03f4,
 0x19, 0xfb00,
 0x18, 0x0404,
 0x19, 0xe000,
 0x18, 0x0414,
 0x19, 0x0000,
 0x18, 0x0424,
 0x19, 0x4000,
 0x18, 0x0434,
 0x19, 0x7f00,
 0x18, 0x0444,
 0x19, 0x0000,
 0x18, 0x0454,
 0x19, 0x0100,
 0x18, 0x0464,
 0x19, 0x4000,
 0x18, 0x0474,
 0x19, 0xc600,
 0x18, 0x0484,
 0x19, 0xff00,
 0x18, 0x0494,
 0x19, 0x0000,
 0x18, 0x04a4,
 0x19, 0x1000,
 0x18, 0x04b4,
 0x19, 0x0200,
 0x18, 0x04c4,
 0x19, 0x7f00,
 0x18, 0x04d4,
 0x19, 0x4000,
 0x18, 0x04e4,
 0x19, 0x7f00,
 0x18, 0x04f4,
 0x19, 0x0200,
 0x18, 0x0504,
 0x19, 0x0200,
 0x18, 0x0514,
 0x19, 0x5200,
 0x18, 0x0524,
 0x19, 0xc400,
 0x18, 0x0534,
 0x19, 0x7400,
 0x18, 0x0544,
 0x19, 0x0000,
 0x18, 0x0554,
 0x19, 0x1000,
 0x18, 0x0564,
 0x19, 0xbc00,
 0x18, 0x0574,
 0x19, 0x0600,
 0x18, 0x0584,
 0x19, 0xfe00,
 0x18, 0x0594,
 0x19, 0x4000,
 0x18, 0x05a4,
 0x19, 0x7f00,
 0x18, 0x05b4,
 0x19, 0x0000,
 0x18, 0x05c4,
 0x19, 0x0a00,
 0x18, 0x05d4,
 0x19, 0x5200,
 0x18, 0x05e4,
 0x19, 0xe400,
 0x18, 0x05f4,
 0x19, 0x3c00,
 0x18, 0x0604,
 0x19, 0x0000,
 0x18, 0x0614,
 0x19, 0x1000,
 0x18, 0x0624,
 0x19, 0x8a00,
 0x18, 0x0634,
 0x19, 0x7f00,
 0x18, 0x0644,
 0x19, 0x4000,
 0x18, 0x0654,
 0x19, 0x7f00,
 0x18, 0x0664,
 0x19, 0x0100,
 0x18, 0x0674,
 0x19, 0x2000,
 0x18, 0x0684,
 0x19, 0x0000,
 0x18, 0x0694,
 0x19, 0xe600,
 0x18, 0x06a4,
 0x19, 0xfe00,
 0x18, 0x06b4,
 0x19, 0x0000,
 0x18, 0x06c4,
 0x19, 0x5000,
 0x18, 0x06d4,
 0x19, 0x9d00,
 0x18, 0x06e4,
 0x19, 0xff00,
 0x18, 0x06f4,
 0x19, 0x8500,
 0x18, 0x0704,
 0x19, 0x7f00,
 0x18, 0x0714,
 0x19, 0xac00,
 0x18, 0x0724,
 0x19, 0x0800,
 0x18, 0x0734,
 0x19, 0xfc00,
 0x18, 0x0744,
 0x19, 0x9500,
 0x18, 0x0754,
 0x19, 0x0400,
 0x18, 0x0764,
 0x19, 0x4000,
 0x18, 0x0774,
 0x19, 0x4000,
 0x18, 0x0784,
 0x19, 0x1000,
 0x18, 0x0794,
 0x19, 0x4000,
 0x18, 0x07a4,
 0x19, 0x3f00,
 0x18, 0x07b4,
 0x19, 0x0200,
 0x18, 0x07c4,
 0x19, 0x0000,
 0x18, 0x07d4,
 0x19, 0xff00,
 0x18, 0x07e4,
 0x19, 0x7f00,
 0x18, 0x07f4,
 0x19, 0x0000,
 0x18, 0x0804,
 0x19, 0x4200,
 0x18, 0x0814,
 0x19, 0x0500,
 0x18, 0x0824,
 0x19, 0x9000,
 0x18, 0x0834,
 0x19, 0x8000,
 0x18, 0x0844,
 0x19, 0x7d00,
 0x18, 0x0854,
 0x19, 0x8c00,
 0x18, 0x0864,
 0x19, 0x8300,
 0x18, 0x0874,
 0x19, 0xe000,
 0x18, 0x0884,
 0x19, 0x0000,
 0x18, 0x0894,
 0x19, 0x4000,
 0x18, 0x08a4,
 0x19, 0x0400,
 0x18, 0x08b4,
 0x19, 0xff00,
 0x18, 0x08c4,
 0x19, 0x0500,
 0x18, 0x08d4,
 0x19, 0x8500,
 0x18, 0x08e4,
 0x19, 0x8c00,
 0x18, 0x08f4,
 0x19, 0xfa00,
 0x18, 0x0904,
 0x19, 0xe000,
 0x18, 0x0914,
 0x19, 0x0000,
 0x18, 0x0924,
 0x19, 0x4000,
 0x18, 0x0934,
 0x19, 0x1f00,
 0x18, 0x0944,
 0x19, 0x0000,
 0x18, 0x0954,
 0x19, 0xfe00,
 0x18, 0x0964,
 0x19, 0x7300,
 0x18, 0x0974,
 0x19, 0x0d00,
 0x18, 0x0984,
 0x19, 0x0300,
 0x18, 0x0994,
 0x19, 0x4000,
 0x18, 0x09a4,
 0x19, 0x2000,
 0x18, 0x09b4,
 0x19, 0x0000,
 0x18, 0x09c4,
 0x19, 0x0900,
 0x18, 0x09d4,
 0x19, 0x9d00,
 0x18, 0x09e4,
 0x19, 0x0800,
 0x18, 0x09f4,
 0x19, 0x9000,
 0x18, 0x0a04,
 0x19, 0x0700,
 0x18, 0x0a14,
 0x19, 0x7b00,
 0x18, 0x0a24,
 0x19, 0x4000,
 0x18, 0x0a34,
 0x19, 0x7f00,
 0x18, 0x0a44,
 0x19, 0x1000,
 0x18, 0x0a54,
 0x19, 0x0000,
 0x18, 0x0a64,
 0x19, 0x0000,
 0x18, 0x0a74,
 0x19, 0x0400,
 0x18, 0x0a84,
 0x19, 0x7300,
 0x18, 0x0a94,
 0x19, 0x0d00,
 0x18, 0x0aa4,
 0x19, 0x0100,
 0x18, 0x0ab4,
 0x19, 0x0900,
 0x18, 0x0ac4,
 0x19, 0x8e00,
 0x18, 0x0ad4,
 0x19, 0x0800,
 0x18, 0x0ae4,
 0x19, 0x7d00,
 0x18, 0x0af4,
 0x19, 0x4000,
 0x18, 0x0b04,
 0x19, 0x7f00,
 0x18, 0x0b14,
 0x19, 0x1000,
 0x18, 0x0b24,
 0x19, 0x0000,
 0x18, 0x0b34,
 0x19, 0x0200,
 0x18, 0x0b44,
 0x19, 0x0000,
 0x18, 0x0b54,
 0x19, 0x7000,
 0x18, 0x0b64,
 0x19, 0x0c00,
 0x18, 0x0b74,
 0x19, 0x0100,
 0x18, 0x0b84,
 0x19, 0x0900,
 0x18, 0x0b94,
 0x19, 0x7f00,
 0x18, 0x0ba4,
 0x19, 0x4000,
 0x18, 0x0bb4,
 0x19, 0x7f00,
 0x18, 0x0bc4,
 0x19, 0x3000,
 0x18, 0x0bd4,
 0x19, 0x8300,
 0x18, 0x0be4,
 0x19, 0x0200,
 0x18, 0x0bf4,
 0x19, 0x0000,
 0x18, 0x0c04,
 0x19, 0x7000,
 0x18, 0x0c14,
 0x19, 0x0d00,
 0x18, 0x0c24,
 0x19, 0x0100,
 0x18, 0x0c34,
 0x19, 0x9a00,
 0x18, 0x0c44,
 0x19, 0xff00,
 0x18, 0x0c54,
 0x19, 0x0a00,
 0x18, 0x0c64,
 0x19, 0x7d00,
 0x18, 0x0c74,
 0x19, 0x4000,
 0x18, 0x0c84,
 0x19, 0x7f00,
 0x18, 0x0c94,
 0x19, 0x1000,
 0x18, 0x0ca4,
 0x19, 0x0000,
 0x18, 0x0cb4,
 0x19, 0x8200,
 0x18, 0x0cc4,
 0x19, 0x0000,
 0x18, 0x0cd4,
 0x19, 0x7000,
 0x18, 0x0ce4,
 0x19, 0x0d00,
 0x18, 0x0cf4,
 0x19, 0x0100,
 0x18, 0x0d04,
 0x19, 0x8400,
 0x18, 0x0d14,
 0x19, 0x7f00,
 0x18, 0x0d24,
 0x19, 0x4000,
 0x18, 0x0d34,
 0x19, 0x7f00,
 0x18, 0x0d44,
 0x19, 0x1000,
 0x18, 0x0d54,
 0x19, 0x0000,
 0x18, 0x0d64,
 0x19, 0x0200,
 0x18, 0x0d74,
 0x19, 0x0000,
 0x18, 0x0d84,
 0x19, 0x7000,
 0x18, 0x0d94,
 0x19, 0x0f00,
 0x18, 0x0da4,
 0x19, 0x0100,
 0x18, 0x0db4,
 0x19, 0x9b00,
 0x18, 0x0dc4,
 0x19, 0x7f00,
 0x18, 0x0dd4,
 0x19, 0x4000,
 0x18, 0x0de4,
 0x19, 0x7f00,
 0x18, 0x0df4,
 0x19, 0x1000,
 0x18, 0x0e04,
 0x19, 0x9000,
 0x18, 0x0e14,
 0x19, 0x0200,
 0x18, 0x0e24,
 0x19, 0x0400,
 0x18, 0x0e34,
 0x19, 0x7300,
 0x18, 0x0e44,
 0x19, 0x1d00,
 0x18, 0x0e54,
 0x19, 0x0100,
 0x18, 0x0e64,
 0x19, 0x0b00,
 0x18, 0x0e74,
 0x19, 0x9000,
 0x18, 0x0e84,
 0x19, 0x8000,
 0x18, 0x0e94,
 0x19, 0x7d00,
 0x18, 0x0ea4,
 0x19, 0x4000,
 0x18, 0x0eb4,
 0x19, 0x7f00,
 0x18, 0x0ec4,
 0x19, 0x5000,
 0x18, 0x0ed4,
 0x19, 0x0000,
 0x18, 0x0ee4,
 0x19, 0x0200,
 0x18, 0x0ef4,
 0x19, 0x0000,
 0x18, 0x0f04,
 0x19, 0x7000,
 0x18, 0x0f14,
 0x19, 0x0f00,
 0x18, 0x0f24,
 0x19, 0x0100,
 0x18, 0x0f34,
 0x19, 0x9b00,
 0x18, 0x0f44,
 0x19, 0x7f00,
 0x18, 0x0f54,
 0x19, 0xe000,
 0x18, 0x0f64,
 0x19, 0xdd00,
 0x18, 0x0f74,
 0x19, 0x4000,
 0x18, 0x0f84,
 0x19, 0x7f00,
 0x18, 0x0f94,
 0x19, 0x1000,
 0x18, 0x0fa4,
 0x19, 0x0000,
 0x18, 0x0fb4,
 0x19, 0x0000,
 0x18, 0x0fc4,
 0x19, 0x0400,
 0x18, 0x0fd4,
 0x19, 0x7300,
 0x18, 0x0fe4,
 0x19, 0x0d00,
 0x18, 0x0ff4,
 0x19, 0x0100,
 0x18, 0x1004,
 0x19, 0x0400,
 0x18, 0x1014,
 0x19, 0x0300,
 0x18, 0x1024,
 0x19, 0xe000,
 0x18, 0x1034,
 0x19, 0x7400,
 0x18, 0x1044,
 0x19, 0x0500,
 0x18, 0x1054,
 0x19, 0x7b00,
 0x18, 0x1064,
 0x19, 0xe000,
 0x18, 0x1074,
 0x19, 0x9200,
 0x18, 0x1084,
 0x19, 0x4000,
 0x18, 0x1094,
 0x19, 0x7f00,
 0x18, 0x10a4,
 0x19, 0x0400,
 0x18, 0x10b4,
 0x19, 0x0100,
 0x18, 0x10c4,
 0x19, 0x4400,
 0x18, 0x10d4,
 0x19, 0x0000,
 0x18, 0x10e4,
 0x19, 0x0000,
 0x18, 0x10f4,
 0x19, 0x0000,
 0x18, 0x1104,
 0x19, 0x4000,
 0x18, 0x1114,
 0x19, 0x8000,
 0x18, 0x1124,
 0x19, 0x7f00,
 0x18, 0x1134,
 0x19, 0x4000,
 0x18, 0x1144,
 0x19, 0x3f00,
 0x18, 0x1154,
 0x19, 0x0400,
 0x18, 0x1164,
 0x19, 0x5000,
 0x18, 0x1174,
 0x19, 0xf800,
 0x18, 0x1184,
 0x19, 0x0000,
 0x18, 0x1194,
 0x19, 0xe000,
 0x18, 0x11a4,
 0x19, 0x4000,
 0x18, 0x11b4,
 0x19, 0x8000,
 0x18, 0x11c4,
 0x19, 0x7f00,
 0x18, 0x11d4,
 0x19, 0x8900,
 0x18, 0x11e4,
 0x19, 0x8300,
 0x18, 0x11f4,
 0x19, 0xe000,
 0x18, 0x1204,
 0x19, 0x0000,
 0x18, 0x1214,
 0x19, 0x4000,
 0x18, 0x1224,
 0x19, 0x7f00,
 0x18, 0x1234,
 0x19, 0x0200,
 0x18, 0x1244,
 0x19, 0x1000,
 0x18, 0x1254,
 0x19, 0x0000,
 0x18, 0x1264,
 0x19, 0xfc00,
 0x18, 0x1274,
 0x19, 0xff00,
 0x18, 0x1284,
 0x19, 0x0000,
 0x18, 0x1294,
 0x19, 0x4000,
 0x18, 0x12a4,
 0x19, 0xbc00,
 0x18, 0x12b4,
 0x19, 0x0e00,
 0x18, 0x12c4,
 0x19, 0xfe00,
 0x18, 0x12d4,
 0x19, 0x8a00,
 0x18, 0x12e4,
 0x19, 0x8300,
 0x18, 0x12f4,
 0x19, 0xe000,
 0x18, 0x1304,
 0x19, 0x0000,
 0x18, 0x1314,
 0x19, 0x4000,
 0x18, 0x1324,
 0x19, 0x7f00,
 0x18, 0x1334,
 0x19, 0x0100,
 0x18, 0x1344,
 0x19, 0xff00,
 0x18, 0x1354,
 0x19, 0x0000,
 0x18, 0x1364,
 0x19, 0xfc00,
 0x18, 0x1374,
 0x19, 0xff00,
 0x18, 0x1384,
 0x19, 0x0000,
 0x18, 0x1394,
 0x19, 0x4000,
 0x18, 0x13a4,
 0x19, 0x9d00,
 0x18, 0x13b4,
 0x19, 0xff00,
 0x18, 0x13c4,
 0x19, 0x8900,
 0x18, 0x13d4,
 0x19, 0x8300,
 0x18, 0x13e4,
 0x19, 0xe000,
 0x18, 0x13f4,
 0x19, 0x0000,
 0x18, 0x1404,
 0x19, 0xac00,
 0x18, 0x1414,
 0x19, 0x0800,
 0x18, 0x1424,
 0x19, 0xfa00,
 0x18, 0x1434,
 0x19, 0x4000,
 0x18, 0x1444,
 0x19, 0x3f00,
 0x18, 0x1454,
 0x19, 0x0200,
 0x18, 0x1464,
 0x19, 0x0000,
 0x18, 0x1474,
 0x19, 0xfd00,
 0x18, 0x1484,
 0x19, 0x7f00,
 0x18, 0x1494,
 0x19, 0x0000,
 0x18, 0x14a4,
 0x19, 0x4000,
 0x18, 0x14b4,
 0x19, 0x0500,
 0x18, 0x14c4,
 0x19, 0x9000,
 0x18, 0x14d4,
 0x19, 0x8000,
 0x18, 0x14e4,
 0x19, 0x7d00,
 0x18, 0x14f4,
 0x19, 0x8c00,
 0x18, 0x1504,
 0x19, 0x8300,
 0x18, 0x1514,
 0x19, 0xe000,
 0x18, 0x1524,
 0x19, 0x0000,
 0x18, 0x1534,
 0x19, 0x4000,
 0x18, 0x1544,
 0x19, 0x0400,
 0x18, 0x1554,
 0x19, 0xff00,
 0x18, 0x1564,
 0x19, 0x0500,
 0x18, 0x1574,
 0x19, 0x8500,
 0x18, 0x1584,
 0x19, 0x8c00,
 0x18, 0x1594,
 0x19, 0xfa00,
 0x18, 0x15a4,
 0x19, 0xe000,
 0x18, 0x15b4,
 0x19, 0x0000,
 0x18, 0x15c4,
 0x19, 0x4000,
 0x18, 0x15d4,
 0x19, 0x1f00,
 0x18, 0x15e4,
 0x19, 0x0000,
 0x18, 0x15f4,
 0x19, 0xfc00,
 0x18, 0x1604,
 0x19, 0x7300,
 0x18, 0x1614,
 0x19, 0x0d00,
 0x18, 0x1624,
 0x19, 0x0100,
 0x18, 0x1634,
 0x19, 0x4000,
 0x18, 0x1644,
 0x19, 0x2000,
 0x18, 0x1654,
 0x19, 0x0000,
 0x18, 0x1664,
 0x19, 0x0400,
 0x18, 0x1674,
 0x19, 0xdc00,
 0x18, 0x1684,
 0x19, 0x0800,
 0x18, 0x1694,
 0x19, 0x9100,
 0x18, 0x16a4,
 0x19, 0x0900,
 0x18, 0x16b4,
 0x19, 0x9900,
 0x18, 0x16c4,
 0x19, 0x0700,
 0x18, 0x16d4,
 0x19, 0x7900,
 0x18, 0x16e4,
 0x19, 0x4000,
 0x18, 0x16f4,
 0x19, 0x3f00,
 0x18, 0x1704,
 0x19, 0x0000,
 0x18, 0x1714,
 0x19, 0x0000,
 0x18, 0x1724,
 0x19, 0x0400,
 0x18, 0x1734,
 0x19, 0x7300,
 0x18, 0x1744,
 0x19, 0x0d00,
 0x18, 0x1754,
 0x19, 0x0100,
 0x18, 0x1764,
 0x19, 0x0900,
 0x18, 0x1774,
 0x19, 0x8d00,
 0x18, 0x1784,
 0x19, 0x0800,
 0x18, 0x1794,
 0x19, 0x7d00,
 0x18, 0x17a4,
 0x19, 0x4000,
 0x18, 0x17b4,
 0x19, 0x3f00,
 0x18, 0x17c4,
 0x19, 0x0000,
 0x18, 0x17d4,
 0x19, 0x0000,
 0x18, 0x17e4,
 0x19, 0x0000,
 0x18, 0x17f4,
 0x19, 0x7000,
 0x18, 0x1804,
 0x19, 0x0c00,
 0x18, 0x1814,
 0x19, 0x0100,
 0x18, 0x1824,
 0x19, 0x0900,
 0x18, 0x1834,
 0x19, 0x7f00,
 0x18, 0x1844,
 0x19, 0x4000,
 0x18, 0x1854,
 0x19, 0x3f00,
 0x18, 0x1864,
 0x19, 0x0000,
 0x18, 0x1874,
 0x19, 0x0000,
 0x18, 0x1884,
 0x19, 0x0000,
 0x18, 0x1894,
 0x19, 0x7000,
 0x18, 0x18a4,
 0x19, 0x0d00,
 0x18, 0x18b4,
 0x19, 0x0100,
 0x18, 0x18c4,
 0x19, 0x0b00,
 0x18, 0x18d4,
 0x19, 0x7f00,
 0x18, 0x18e4,
 0x19, 0x4000,
 0x18, 0x18f4,
 0x19, 0x3f00,
 0x18, 0x1904,
 0x19, 0x0000,
 0x18, 0x1914,
 0x19, 0x0000,
 0x18, 0x1924,
 0x19, 0x0000,
 0x18, 0x1934,
 0x19, 0x7200,
 0x18, 0x1944,
 0x19, 0x0d00,
 0x18, 0x1954,
 0x19, 0x0100,
 0x18, 0x1964,
 0x19, 0x0500,
 0x18, 0x1974,
 0x19, 0xc500,
 0x18, 0x1984,
 0x19, 0x0400,
 0x18, 0x1994,
 0x19, 0x7d00,
 0x18, 0x19a4,
 0x19, 0xe100,
 0x18, 0x19b4,
 0x19, 0x4300,
 0x18, 0x19c4,
 0x19, 0x4000,
 0x18, 0x19d4,
 0x19, 0x7f00,
 0x18, 0x19e4,
 0x19, 0x0400,
 0x18, 0x19f4,
 0x19, 0x0100,
 0x18, 0x1a04,
 0x19, 0x4000,
 0x18, 0x1a14,
 0x19, 0x3e00,
 0x18, 0x1a24,
 0x19, 0x0000,
 0x18, 0x1a34,
 0x19, 0xe000,
 0x18, 0x1a44,
 0x19, 0x1200,
 0x18, 0x1a54,
 0x19, 0x8000,
 0x18, 0x1a64,
 0x19, 0x7f00,
 0x18, 0x1a74,
 0x19, 0x8900,
 0x18, 0x1a84,
 0x19, 0x8300,
 0x18, 0x1a94,
 0x19, 0xe000,
 0x18, 0x1aa4,
 0x19, 0x0000,
 0x18, 0x1ab4,
 0x19, 0x4000,
 0x18, 0x1ac4,
 0x19, 0x7f00,
 0x18, 0x1ad4,
 0x19, 0x0000,
 0x18, 0x1ae4,
 0x19, 0x2000,
 0x18, 0x1af4,
 0x19, 0x4000,
 0x18, 0x1b04,
 0x19, 0x3e00,
 0x18, 0x1b14,
 0x19, 0xff00,
 0x18, 0x1b24,
 0x19, 0x0000,
 0x18, 0x1b34,
 0x19, 0x1200,
 0x18, 0x1b44,
 0x19, 0x8000,
 0x18, 0x1b54,
 0x19, 0x7f00,
 0x18, 0x1b64,
 0x19, 0x8600,
 0x18, 0x1b74,
 0x19, 0x8500,
 0x18, 0x1b84,
 0x19, 0x8900,
 0x18, 0x1b94,
 0x19, 0xfd00,
 0x18, 0x1ba4,
 0x19, 0xe000,
 0x18, 0x1bb4,
 0x19, 0x0000,
 0x18, 0x1bc4,
 0x19, 0x9500,
 0x18, 0x1bd4,
 0x19, 0x0400,
 0x18, 0x1be4,
 0x19, 0x4000,
 0x18, 0x1bf4,
 0x19, 0x4000,
 0x18, 0x1c04,
 0x19, 0x1000,
 0x18, 0x1c14,
 0x19, 0x4000,
 0x18, 0x1c24,
 0x19, 0x3f00,
 0x18, 0x1c34,
 0x19, 0x0200,
 0x18, 0x1c44,
 0x19, 0x4000,
 0x18, 0x1c54,
 0x19, 0x3700,
 0x18, 0x1c64,
 0x19, 0x7f00,
 0x18, 0x1c74,
 0x19, 0x0000,
 0x18, 0x1c84,
 0x19, 0x0200,
 0x18, 0x1c94,
 0x19, 0x0200,
 0x18, 0x1ca4,
 0x19, 0x9000,
 0x18, 0x1cb4,
 0x19, 0x8000,
 0x18, 0x1cc4,
 0x19, 0x7d00,
 0x18, 0x1cd4,
 0x19, 0x8900,
 0x18, 0x1ce4,
 0x19, 0x8300,
 0x18, 0x1cf4,
 0x19, 0xe000,
 0x18, 0x1d04,
 0x19, 0x0000,
 0x18, 0x1d14,
 0x19, 0x4000,
 0x18, 0x1d24,
 0x19, 0x0400,
 0x18, 0x1d34,
 0x19, 0xff00,
 0x18, 0x1d44,
 0x19, 0x0200,
 0x18, 0x1d54,
 0x19, 0x8500,
 0x18, 0x1d64,
 0x19, 0x8900,
 0x18, 0x1d74,
 0x19, 0xfa00,
 0x18, 0x1d84,
 0x19, 0xe000,
 0x18, 0x1d94,
 0x19, 0x0000,
 0x18, 0x1da4,
 0x19, 0x4000,
 0x18, 0x1db4,
 0x19, 0x7f00,
 0x18, 0x1dc4,
 0x19, 0x1000,
 0x18, 0x1dd4,
 0x19, 0x0000,
 0x18, 0x1de4,
 0x19, 0x4000,
 0x18, 0x1df4,
 0x19, 0x3700,
 0x18, 0x1e04,
 0x19, 0x7300,
 0x18, 0x1e14,
 0x19, 0x0500,
 0x18, 0x1e24,
 0x19, 0x0200,
 0x18, 0x1e34,
 0x19, 0x0100,
 0x18, 0x1e44,
 0x19, 0xd800,
 0x18, 0x1e54,
 0x19, 0x0400,
 0x18, 0x1e64,
 0x19, 0x7d00,
 0x18, 0x1e74,
 0x19, 0x4000,
 0x18, 0x1e84,
 0x19, 0x7f00,
 0x18, 0x1e94,
 0x19, 0x1000,
 0x18, 0x1ea4,
 0x19, 0x0000,
 0x18, 0x1eb4,
 0x19, 0x4000,
 0x18, 0x1ec4,
 0x19, 0x0000,
 0x18, 0x1ed4,
 0x19, 0x7200,
 0x18, 0x1ee4,
 0x19, 0x0400,
 0x18, 0x1ef4,
 0x19, 0x0000,
 0x18, 0x1f04,
 0x19, 0x0800,
 0x18, 0x1f14,
 0x19, 0x7f00,
 0x18, 0x1f24,
 0x19, 0x4000,
 0x18, 0x1f34,
 0x19, 0x7f00,
 0x18, 0x1f44,
 0x19, 0x3000,
 0x18, 0x1f54,
 0x19, 0x0000,
 0x18, 0x1f64,
 0x19, 0xc000,
 0x18, 0x1f74,
 0x19, 0x0000,
 0x18, 0x1f84,
 0x19, 0x7200,
 0x18, 0x1f94,
 0x19, 0x0500,
 0x18, 0x1fa4,
 0x19, 0x0000,
 0x18, 0x1fb4,
 0x19, 0x0400,
 0x18, 0x1fc4,
 0x19, 0xeb00,
 0x18, 0x1fd4,
 0x19, 0x8400,
 0x18, 0x1fe4,
 0x19, 0x7d00,
 0x18, 0x1ff4,
 0x19, 0x4000,
 0x18, 0x2004,
 0x19, 0x7f00,
 0x18, 0x2014,
 0x19, 0x1000,
 0x18, 0x2024,
 0x19, 0x0000,
 0x18, 0x2034,
 0x19, 0x4000,
 0x18, 0x2044,
 0x19, 0x0000,
 0x18, 0x2054,
 0x19, 0x7200,
 0x18, 0x2064,
 0x19, 0x0700,
 0x18, 0x2074,
 0x19, 0x0000,
 0x18, 0x2084,
 0x19, 0x0400,
 0x18, 0x2094,
 0x19, 0xde00,
 0x18, 0x20a4,
 0x19, 0x9b00,
 0x18, 0x20b4,
 0x19, 0x7d00,
 0x18, 0x20c4,
 0x19, 0x4000,
 0x18, 0x20d4,
 0x19, 0x7f00,
 0x18, 0x20e4,
 0x19, 0x1000,
 0x18, 0x20f4,
 0x19, 0x9000,
 0x18, 0x2104,
 0x19, 0x4000,
 0x18, 0x2114,
 0x19, 0x0400,
 0x18, 0x2124,
 0x19, 0x7300,
 0x18, 0x2134,
 0x19, 0x1500,
 0x18, 0x2144,
 0x19, 0x0000,
 0x18, 0x2154,
 0x19, 0x0400,
 0x18, 0x2164,
 0x19, 0xd100,
 0x18, 0x2174,
 0x19, 0x9400,
 0x18, 0x2184,
 0x19, 0x9200,
 0x18, 0x2194,
 0x19, 0x8000,
 0x18, 0x21a4,
 0x19, 0x7b00,
 0x18, 0x21b4,
 0x19, 0x4000,
 0x18, 0x21c4,
 0x19, 0x7f00,
 0x18, 0x21d4,
 0x19, 0x5000,
 0x18, 0x21e4,
 0x19, 0x0000,
 0x18, 0x21f4,
 0x19, 0x4000,
 0x18, 0x2204,
 0x19, 0x0000,
 0x18, 0x2214,
 0x19, 0x7200,
 0x18, 0x2224,
 0x19, 0x0700,
 0x18, 0x2234,
 0x19, 0x0000,
 0x18, 0x2244,
 0x19, 0x0400,
 0x18, 0x2254,
 0x19, 0xc200,
 0x18, 0x2264,
 0x19, 0x9b00,
 0x18, 0x2274,
 0x19, 0x7d00,
 0x18, 0x2284,
 0x19, 0xe200,
 0x18, 0x2294,
 0x19, 0x0c00,
 0x18, 0x22a4,
 0x19, 0x4000,
 0x18, 0x22b4,
 0x19, 0x7f00,
 0x18, 0x22c4,
 0x19, 0x1000,
 0x18, 0x22d4,
 0x19, 0x0000,
 0x18, 0x22e4,
 0x19, 0x4000,
 0x18, 0x22f4,
 0x19, 0x3700,
 0x18, 0x2304,
 0x19, 0x7300,
 0x18, 0x2314,
 0x19, 0x0500,
 0x18, 0x2324,
 0x19, 0x0000,
 0x18, 0x2334,
 0x19, 0x0100,
 0x18, 0x2344,
 0x19, 0x0300,
 0x18, 0x2354,
 0x19, 0xe100,
 0x18, 0x2364,
 0x19, 0xbc00,
 0x18, 0x2374,
 0x19, 0x0200,
 0x18, 0x2384,
 0x19, 0x7b00,
 0x18, 0x2394,
 0x19, 0xe100,
 0x18, 0x23a4,
 0x19, 0xda00,
 0x18, 0x0000,
 0x1f, 0x0000,
 0x17, 0x2100,

    
    //##### patch INRX sram code #####
    0x1f,0x0007,
    0x1e, 0x0020,   
    0x15, 0x0100,   
    0x17, 0x000A,   
    0x18, 0x0003,   
    0x1b, 0x2F3C,                                  
    //#INRX PRG FSM Enable       
    0x1e, 0x0040,   
    0x18, 0x0001,   
    //#PCS Dedicate FSM Enable   
    0x1e, 0x0023,   
    0x16, 0x000A,   
    //##EEE buffer setting       
    //#mdcmdio_cmd w $port 0x1e 0x0028;   
    //#mdcmdio_cmd w $port 0x15 0x0010;                                 
    //#change EEE giga circuit   
    0x1e, 0x0041, 
    0x1c, 0x0008,   
    
    
    //####### phy patch###############
    0x1e, 0x0021,
    0x19, 0x2828,
    0x1e, 0x0020,
    0x1b, 0x2f38,
    
     
                               
    //######## Restart nway ##########
    0x1f, 0x0000,   
    0x00, 0x9140, //#reset PHY
    0x09, 0x0000,    
    0x00, 0x1340,    

};

	int len=sizeof(p)/sizeof(unsigned int);
	int port;
	int i;

	dprintf("Writing EEE pattern %08d records \n", len/2);
	for(port=0; port<5; port++)
	{
		for(i=0;i<len/2;i++)
		{	rtl8651_setAsicEthernetPHYReg(port, p[i], p[i+1]);
		}
	}

};
#endif
//-----------------------------------------------------------------------------------------------


