
/*==========================================

These code  is just for test PCI-e IP.
Written by George.
===========================================*/



#include <linux/interrupt.h>
#include <asm/system.h>
#include "monitor.h"


#include <asm/mipsregs.h>	//wei add
#include <asm/rtl8196.h>
//#include <rtl8650/asicregs.h>
#include "test_hw_96c.h"


//-----------------------------------------------------------
#ifdef DBG_PRINT
#undef DBG_PRINT
#endif

#define DBG_PRINT dprintf
//#define DBG_PRINT(...) 
//-----------------------------------------------------------

extern unsigned short example(int portnum, int quietmode);
void RunMonitor(char *PROMOPT, COMMAND_TABLE *TestCmdTable, int len);
unsigned int HostPCIe_SetPhyMdioRead(unsigned int regaddr);
void HostPCIe_SetPhyMdioWrite(unsigned int regaddr, unsigned short val);
unsigned int ExtractACmdLine2(const char *pattern,  char *cmdstring, int first);
int RunAutoTestCmd2(const char *cmdstr, COMMAND_TABLE *pTestCmdTable, int len );


#if CONFIG_TEST_HW
//extern public func
extern int CmdDumpWord( int argc, char* argv[] );
extern int CmdDumpByte( int argc, char* argv[] ); //wei add
extern int CmdWriteWord( int argc, char* argv[] );


int TestCmdHelp( int argc, char* argv[] );
int TestCryptoRst( int argc, char* argv[] );
int CmdCP0SR(int argc, char* argv[]);  //wei add
int CmdTestGDMA(int argc, char* argv[]);  //wei add
int CmdTestGDMA_PATT(int argc, char* argv[]);  //wei add
int CmdTestGDMA_MSET(int argc, char* argv[]);  
int CmdTestCRYPTO(int argc, char* argv[]);  //wei add
int TestCryptoRst( int argc, char* argv[] );
int CmdTestSRAMOCP(int argc, char* argv[]);  //wei add
int CmdTestSRAMLX1(int argc, char* argv[]);  //wei add
int CmdTestNFBISRAM(int argc, char* argv[]);  //wei add

int TestCmd_Sata( int argc, char* argv[] );


int TestCmd_TS( int argc, char* argv[] );

int CmdCPUCLK(int argc, char* argv[]);  //wei add
int CmdMEMCLK(int argc, char* argv[]);  //wei add
int CmdLXCLK(int argc, char* argv[]);  //wei add

int CmdEnWDog(int argc, char* argv[]);  //wei add
int CmdSLEEP(int argc, char* argv[]);  //wei add
int CmdFILLCMP(int argc, char* argv[]);  //wei add

int CmdTestIDMEM(int argc, char* argv[]);  //wei add

int TestCmd_NFBIINT( int argc, char* argv[] );
int TestCmd_SETRSTCODE( int argc, char* argv[] );
int TestGPIO(int argc, char* argv[]); 
int TestMIBVIEW(int argc, char* argv[]); 
int TestPHYSpeed(int argc, char* argv[]); 
int TestDumpSw(int argc, char* argv[]); 

int TestSwitchMsg(int argc, char* argv[]); 
int TestStrapPin(int argc, char* argv[]); 
int Test_HostPCIE_DataLoopback(int argc, char* argv[]); 
int TestPCIe(int argc, char* argv[]); 

int PCIE_Host_RESET(int argc, char* argv[]); 
int PCIE_8196BU_Pattern(int argc, char* argv[]); 

int HostPCIe_MDIORead(int argc, char* argv[]); 
int HostPCIe_MDIOWrite(int argc, char* argv[]); 

int  PCIE_PowerDown(int argc, char* argv[]); 

//auti test use
#define SUPPORT_AUTOTEST 1
#ifdef SUPPORT_AUTOTEST
int RegRead(int argc, char* argv[]); 
int RegWrite(int argc, char* argv[]); 
int CMD_AutoTest_2(int argc, char* argv[]); 
int CMD_Wait2(int argc, char* argv[]); 
int at2_errcnt=0;
int at2_mode=0;
const char *test_pattern_ptr2;    //point to test script buffer


const char PCIE_PATT0[]={"hrst 1\n"\
						"hloop 10\n"\
						"~" 			};


#endif

COMMAND_TABLE	TestCmdTable[] =
{
	{ "?"	  ,0, TestCmdHelp			, "HELP (?)				    : Print this help message"					},
	{ "Q"   ,0, NULL			, "Q: return to previous menu"					}, 	//wei add			
	{ "HELP"  ,0, TestCmdHelp			, NULL																	},

	//extern func
	{ "DW"	  ,2, CmdDumpWord		, "DW <Address> <Len>"},
	{ "EW",2, CmdWriteWord, "EW <Address> <Value1> <Value2>..."},

	



	{ "NULL"   ,0, NULL			, "-------Host PCI-E test -----------"},	
	{ "HRST"   ,1, PCIE_Host_RESET			, "HRST: Test Host Pci-E <enable mdio reset>: "},
	{ "HLOOP"   ,1, Test_HostPCIE_DataLoopback			, "HLOOP: Test Pci-E data loopback <cnt> "},	
	{ "EPDN"   ,1, PCIE_PowerDown			, "EPDN: PCIE Power Down test <mode> "},		
	{ "EMDIOR"   ,1, HostPCIe_MDIORead			, "EMDIOR: Reg Read"},	
	{ "EMDIOW"   ,1, HostPCIe_MDIOWrite			, "EMDIOW <reg> <val>:  "},	


#ifdef SUPPORT_AUTOTEST	
	{ "NULL"   ,0, NULL			, "-------Auto test -----------"},	
	{ "REGR"   ,1,  RegRead			, "REGR: Slave PCIe Reg Read"},	
	{ "REGW"   ,1, RegWrite			, "REGW <offset> <val>: Slave PCIe Reg Write "},	
	{ "AT"   ,1, CMD_AutoTest_2			, "at: auto test "},	
	{ "WAIT"   ,1, CMD_Wait2			, "Wait:  unit : sec "},		
#endif
};

//---------------------------------------------------------------------------------------
void TestMonitorEntry(void)
{
	int i;
	//REG32(CLKMANAGE)=ACTIVE_DEFAULT; //enable all interface
	//dprintf("Enable all interface\r\n");
	#define TEST_PROMPT		"<RealTek/Test96C>"
	RunMonitor(TEST_PROMPT, TestCmdTable, sizeof(TestCmdTable) / sizeof(COMMAND_TABLE) );
}
#if 0
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
#endif

//----------------------------------------------------------------------------
int TestCmdHelp( int argc, char* argv[] )
{
	int	i, LineCount ;

    printf("----------------- COMMAND MODE HELP ------------------\n");
	for( i=0, LineCount = 0 ; i < (sizeof(TestCmdTable) / sizeof(COMMAND_TABLE)) ; i++ )
	{
		if( TestCmdTable[i].msg )
		{
			LineCount++ ;
			printf( "%s\n", TestCmdTable[i].msg );
			
		}
	}
    
	return TRUE ;
}

//----------------------------------------------------------------------------


//--------------------------------------------------------------------------------
int CmdTestIDMEM(int argc, char* argv[])
{
#if 0
	int mode;
	int i;
	if( !argv[0])	//read
	{
		
		dprintf("IDMEM mode\n");	
		dprintf("Usage: idmem 1\n");		
		return;	
	}
	
	if(argv[0])	mode = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	
	


	switch(mode) 
	{	
		case 0:
				dprintf("\ntest #1 sdram_to_imem \n");
				sdram_to_imem();
				dprintf("\ntest #2 imem_to_sdram \n");
				imem_to_sdram();
				dprintf("\ntest #3 sdram_to_dmem \n");
				sdram_to_dmem();
				dprintf("\ntest #4 dmem_to_sdram \n");
				dmem_to_sdram();
				break;
				
		case 1:	dprintf("\ntest sdram_to_imem \n");
				sdram_to_imem();
				break;
		case 2:	dprintf("\ntest imem_to_sdram \n");
				imem_to_sdram();
				break;
		case 3:	dprintf("\ntest sdram_to_dmem \n");
				sdram_to_dmem();
				break;
		case 4:	dprintf("\ntest dmem_to_sdram \n");
				dmem_to_sdram();
				break;	

				
	}
#endif

}
//-----------------------------------------------------------------
void ShowStrapMsg()
{
//	const char *boot_type_tab[]={ {"NFBI Mode"} , {"FLASH Mode"} };
//	const char *dram_type_tab[]={ {"SDR Ram"} , {"DDR Ram"} };
	const char *flash_type_tab[]={ {"NOR Flash"} , {"SPI Flash"} };
	
	unsigned int ck_cpu_freq_sel_tab[16]={ 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400	};
	unsigned int ck_cpu_div_sel_tab[]={1, 1, 2};  	
	unsigned int ck_m2x_freq_sel_tab[8]={ 131, 156, 250, 300, 312, 337, 387 	};

	
	unsigned int val;
	val=REG32(SYS_HW_STRAP);

//	unsigned int bootpinsel=(val& ST_BOOTPINSEL) >>0 ;
//	unsigned int dram_type=(val& ST_DRAMTYPE) >>1;
	unsigned int bootsel=(val& ST_BOOTSEL) >>2;
//	unsigned int phyidsel=(val& ST_PHYID) >>3;
	unsigned int clklx_from_clkm=(val& ST_CLKLX_FROM_CLKM) >>7;
	unsigned int en_ext_rstn=(val& ST_EN_EXT_RST) >>8;
	unsigned int sync_oc=(val& ST_SYNC_OCP) >>9;	
	unsigned int ck_m2x_freq_sel=(val& CK_M2X_FREQ_SEL) >>10;
	unsigned int ck_cpu_freq_sel=(val& ST_CPU_FREQ_SEL) >>13;
	unsigned int nrfrst_type=(val& ST_NRFRST_TYPE) >>17;
//	unsigned int sync_lx=(val& ST_SYNC_LX) >>18;
	unsigned int ck_cpu_div_sel=(val& ST_CPU_FREQDIV_SEL) >>19;
	unsigned int swap_halfword=(val& ST_SWAP_DBG_HALFWORD) >>22;
	unsigned int ever_reboot_once=(val& ST_EVER_REBOOT_ONCE) >>23;
	unsigned int sys_debug_sel=(val& ST_SYS_DBG_SEL) >>24;
	unsigned int bus_dbgsel=(val& ST_PINBUS_DBG_SEL) >>30;
	
	
	
	printf("---------------------\n");
	printf("HW_STRAP_VAL= 0x%08x \n", val);
//	printf("[00:00] ST_BOOTPINSEL= 0x%x  \n", bootpinsel);		
//	printf("[01:01] ST_DRAMTYPE= 0x%x      	\n", 	dram_type);	
	printf("[02:02] ST_BOOTSEL= 0x%x 	\n", 	bootsel);			
//	printf("[04:03] ST_PHYID= 0x%x \n", 		phyidsel);		
	printf("[07:07] ST_CLKLX_FROM_CLKM= 0x%x \n", clklx_from_clkm);
	printf("[08:08] ST_EN_EXT_RST= 0x%x \n", en_ext_rstn);	
	printf("[09:09] ST_SYNC_OCP= 0x%x \n", sync_oc );	
	printf("[12:10] CK_M2X_FREQ_SEL= 0x%x \n", ck_m2x_freq_sel);	
	printf("[16:13] ST_CPU_FREQ_SEL= 0x%x \n", ck_cpu_freq_sel);
	printf("[17:17] ST_NRFRST_TYPE= 0x%x \n", nrfrst_type);	
//	printf("[18:18] ST_SYNC_LX= 0x%x \n", sync_lx);	
	printf("[21:19] ST_CPU_FREQDIV_SEL= 0x%x \n", ck_cpu_div_sel);
	printf("[22:22] ST_DBG_halfword= 0x%x \n", swap_halfword);
	printf("[23:23] ST_EVER_REBOOT_ONCE= 0x%x \n", ever_reboot_once);	
	printf("[29:24] ST_SYS_DBG_SEL= 0x%x \n", sys_debug_sel);	
	printf("[31:30] ST_PINBUS_DBG_SEL= 0x%x \n", bus_dbgsel);	
	
	printf("\n");
	printf("%s,  CPU=%d MHz, Mem2x=%d MHz, \n", 
		//				boot_type_tab[bootpinsel],	
		//				dram_type_tab[dram_type],
						flash_type_tab[bootsel],	
						ck_cpu_freq_sel_tab[ck_cpu_freq_sel]/ck_cpu_div_sel_tab[ck_cpu_div_sel] ,
						ck_m2x_freq_sel_tab[ck_m2x_freq_sel]
						);
						
}
//------------------------------------------------------------------------

//------------------------------------------------------------------------


//------------------------------------------------------------------------

void PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset)
{
        #define SYS_PCIE_PHY0   (0xb8000000 +0x50)
	//PCIE Register
	 #define CLK_MANAGE 	0xb8000010

	 //#define PCIE_PHY0_REG 	0xb8b01000
	 //#define PCIE_PHY1_REG 	0xb8b21000
	 #define PCIE_PHY0 	0xb8b01008
	// #define PCIE_PHY1 	0xb8b21008


	 
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
    __delay(1000);
#endif

        //2.Active LX & PCIE Clock
    REG32(CLK_MANAGE) |=  (1<<11);        //enable active_pcie0
    __delay(1000);
 
#if 1
	if(mdio_reset)
	{
		dprintf("Do MDIO_RESET\n");
   	    // 3.MDIO Reset  
 	   REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,     	    
 	   REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1,   
 	   REG32(SYS_PCIE_PHY0) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
	} 
        //4. PCIE PHY Reset       
    REG32(PCIE_PHY0) = 0x01;	//bit7 PHY reset=0   bit0 Enable LTSSM=1
    REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
      __delay(100000);
   
#endif

        __delay(100000);    
 
#if 0
	//#if Use_External_PCIE_CLK
	if(Use_External_PCIE_CLK)
	{
		//5. PCIE P0 PHY external clock
		 __delay(1000); 
		 REG32(PCIE_PHY0_REG) = 0xC3930301; //bit[3] must be "0" for external clock
		 //REG32(PCIE_PHY0_REG) = 0xC39B0301; //20090304:RDC for for PCIE port 0 Refine-tune
		  __delay(1000);       
		 
		     
		        REG32(PCIE_PHY0_REG) = 0x3c011901; //close PHY 0 ,"0"=R,"1"=W
		        __delay(1000);
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

#define PHY_USE_TEST_CHIP 1   // 1: test chip, 0: fib chip
#define PHY_EAT_40MHZ 1   // 0: 25MHz, 1: 40MHz

		HostPCIe_SetPhyMdioWrite(0, 0x5027);
		HostPCIe_SetPhyMdioWrite(2, 0x6d18);
		HostPCIe_SetPhyMdioWrite(6, 0x8828);
		HostPCIe_SetPhyMdioWrite(7, 0x30ff);
#if 	PHY_USE_TEST_CHIP	
		HostPCIe_SetPhyMdioWrite(8, 0x18dd);     	        //FIB dont use
#endif
		HostPCIe_SetPhyMdioWrite(0xa, 0xe9);
		HostPCIe_SetPhyMdioWrite(0xb, 0x0511);
#if 	PHY_USE_TEST_CHIP			
		HostPCIe_SetPhyMdioWrite(0xd, 0x15b6);		//FIB dont use
#endif
		HostPCIe_SetPhyMdioWrite(0xf, 0x0f0f);				
		
#if 0 //saving more power
		HostPCIe_SetPhyMdioWrite(0xa, 0xeb);
		HostPCIe_SetPhyMdioWrite(0x9, 0x538c);
		
//		HostPCIe_SetPhyMdioWrite(0xc, 0xC828);  //original 
//		HostPCIe_SetPhyMdioWrite(0x0, 0x502F);  //fix
		
		HostPCIe_SetPhyMdioWrite(0xc, 0x8828);  //new
		HostPCIe_SetPhyMdioWrite(0x0, 0x502F);  //fix		
#endif
#if  PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(5, 0xbcb);    //[9:3]=1111001 (binary)   121 (10)
		HostPCIe_SetPhyMdioWrite(6, 0x8128);  //[11]=0   [9:8]=01
#endif

	  	}

	//---------------------------------------
         // 6. PCIE Device Reset
     REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.
        __delay(100000);   
        __delay(100000);   
        __delay(100000);   
		
    REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
    //prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

	
        //4. PCIE PHY Reset       
    REG32(PCIE_PHY0) = 0x01;	//bit7 PHY reset=0   bit0 Enable LTSSM=1
    REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1

#if 1  //wait for LinkUP
	int i=1000;
	while(--i)
	{
	      if( (REG32(0xb8b00728)&0x1f)==0x11)
		  	break;
      		__delay(100000);		  

	}
	if(i==0)
	{	if(at2_mode==0)  //not auto test, show message
		dprintf("i=%x Cannot LinkUP \n",i);
	}
#endif

	  
	
}


//------------------------------------------------------------------------
int Test_HostPCIE_DataLoopback(int argc, char* argv[])
{
//	extern void example();

//    PCIE_reset_procedure(0,0,1);

    unsigned int test_packet_num=1;

	if(argc >= 1) 
	{	test_packet_num= strtoul((const char*)(argv[0]), (char **)NULL, 16);
	}

        //GPIO setting
    //REG32(PIN_MUX_SEL) = 0x0BFAA800;                //set LEDP1/P2/P3 as GPIOC1/C2/C3
        //dprintf("Re-Check PIN_MUX_SEL(0xb8000030)=%x\n",*(volatile unsigned int*)PIN_MUX_SEL);
    unsigned int cnt;

    unsigned int PCIE_Test_cnt;
    for (PCIE_Test_cnt=1; PCIE_Test_cnt<=test_packet_num; PCIE_Test_cnt++)
    {
#if DBG
        prom_printf("\n==================(Start)======================\n");
        prom_printf("\nPCIE_Test_cnt:%d\n",PCIE_Test_cnt);
#endif
        //JSW 20090214: For tx/rx delay
        //__delay(1000*1000*10);
        if(example(0, at2_mode)==0)   //0: mean fail
			at2_errcnt++;
    }

	
}; 
//------------------------------------------------------------------------
int PCIE_Host_RESET(int argc, char* argv[])
{
	int Use_External_PCIE_CLK=0;
	int mdio_reset=0;
	if(argc >= 1) 
	{	//Use_External_PCIE_CLK= strtoul((const char*)(argv[0]), (char **)NULL, 10);	
		mdio_reset= strtoul((const char*)(argv[0]), (char **)NULL, 10);	
	}
	dprintf("ext clk=%x \n", Use_External_PCIE_CLK);
		 
	
	//#define Use_External_PCIE_CLK 1
	int  PCIE_Port0and1_8196B_208pin= 0;  //0: one port, 1: two port
	 
	PCIE_reset_procedure(PCIE_Port0and1_8196B_208pin, Use_External_PCIE_CLK, mdio_reset);


}; 


//------------------------------------------------------------------------


//------------------------------------------------------------------------
unsigned int HostPCIe_SetPhyMdioRead(unsigned int regaddr)
{
	REG32(PCIE0_MDIO)= ((regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET)  | (0<<PCIE_MDIO_RDWR_OFFSET); 
	//delay 
	volatile int i;
	for(i=0;i<5555;i++)  ;

	int val;
	val=REG32(PCIE0_MDIO)&  (0xffff <<PCIE_MDIO_DATA_OFFSET) ;
	return ((val>>PCIE_MDIO_DATA_OFFSET)&0xffff);
}


void HostPCIe_SetPhyMdioWrite(unsigned int regaddr, unsigned short val)
{
	REG32(PCIE0_MDIO)= ( (regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET) | ((val&0xffff)<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ; 
	//delay 
	volatile int i;
	for(i=0;i<5555;i++)  ;
}


//----------------------------------------------------------------------------
int HostPCIe_MDIORead(int argc, char* argv[])
{

	unsigned int i,val,j;
	for(i=0; i<=0x1f; i++)
	{/*
		REG32(PCIE0_MDIO)= (i<<PCIE_MDIO_REG_OFFSET) | (0<<PCIE_MDIO_RDWR_OFFSET) ;   
		for(j=0;j<0x5555;j++) ;
		val=REG32(PCIE0_MDIO);
		val=( val& PCIE_MDIO_DATA_MASK ) >> PCIE_MDIO_DATA_OFFSET;		
	*/
		val=HostPCIe_SetPhyMdioRead(i);

		dprintf("MDIO Reg %x=%x \n", i,val);

	}



}; 
//----------------------------------------------------------------------------
int HostPCIe_MDIOWrite(int argc, char* argv[])
{

	if(argc<2)
	{	 
		dprintf("mdiow <addr> <val> \n");		
		dprintf("ex: mdiow 00  ffff \n");			
		return;	
	}


	unsigned int addr = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	unsigned int val = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
/*
	REG32(PCIE0_MDIO)= (addr<<PCIE_MDIO_REG_OFFSET) | (val<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ;   ;   
*/
	HostPCIe_SetPhyMdioWrite(addr, val);

}; 


//---------------------------------------------------------------------------
//----------------------------------------------------------------------------
int RegRead(int argc, char* argv[])
{

	if(argc<1)
	{//dump all	
	       dprintf("\n"); 		   
		dprintf("regr addr \n");
		return;
	}

	int addr,val;
	unsigned int check=0,mask=0xffffffff,expval=0;
	
	StrUpr( argv[0] );
	
	addr= strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	val=REG32(addr);

	//regr iocmd
	if(argc==1)
	{	//dprintf("%x\n", val );
	}

	//regr iocmd 0001
	else if(argc==2)
	{
		mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);		
	}
	//regr iocmd : 0x0001
	else if(   argc>=3  &&  *(argv[1])==':' )
	{	check=1;
		expval = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	}
	//regr iocmd 0x0001 : 0x0001
	else if(argc>=3  && *(argv[1]) != '\0')
	{	
		mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);		
		if(argc>=3 && *(argv[2]) == ':')
		{	check=1;
			expval = strtoul((const char*)(argv[3]), (char **)NULL, 16);	
		}
	}


	//verify
	if(!check)
	{
		dprintf("Addr %08x, %s=%08x \n", addr, argv[0],val&mask );			
	}
	else
	{
		if( (val&mask) !=expval)
		{	dprintf("Fail, addr=%08x val=%x, expval=%x \n", addr, val, expval);
			at2_errcnt++;
		}
		else
			dprintf("Pass \n");

	}		

}
//----------------------------------------------------------------------------
int RegWrite(int argc, char* argv[])
{
	if(argc<2)
	{	 
		dprintf("regw <addr> <val> \n");		
		dprintf("regw <addr> <mask> <value>\n");			
		dprintf("ex: regw b8001000  ffffffff \n");			
		return;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned int addr,mask=0,val;

	StrUpr( argv[0] );
	addr= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		
	//regw iocmd 0x0001
	if(argc==2)
	{	val = strtoul((const char*)(argv[1]), (char **)NULL, 16);		
	}
	//regw iocmd 0x0001 0x0001
	else if(argc>=3)
	{
		mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		val = strtoul((const char*)(argv[2]), (char **)NULL, 16);
	}

	if(mask==0)
		REG32(addr)=val ;		//avoid DR issue.
	else		
		REG32(addr)= (REG32(addr) & mask) | val ;	
	
}
//----------------------------------------------------------------------------



//return position 
unsigned int ExtractACmdLine2(const char *pattern,  char *cmdstring, int first)
{
	//first=1 reset index, to buffer head
	//first=0 continue read a line

	static unsigned int idx=0;
	unsigned char *p=pattern+idx;
	int push=0;
	
	if(first==1)
	{	idx=0;
		return 0;
	}
	
	memset( cmdstring, 0, MAX_MONITOR_BUFFER );


	int n=0;
	while( *p )
	{
		if(n==0)
		{
			//skip first return-line
			while( *p && ((*p == 0x0d) ||(*p==0x0a) ||(*p=='\t') ||(*p==' ') ) )
				p++;
		}
#if 0
		//mark
		if( (*p == '/')  || (*(p+1) == '/' ))
		{	cmdstring[n] = 0 ;
			//search until reurn-line
			while( *p && (*p != 0x0d) && (*p!=0x0a) )
				p++;
			break;
		}		
#endif
		if(*p =='~')
			return 0;
		else if(*p =='#')
		{	//skip word until to newline
			while( *p && (*p != 0x0d) && (*p!=0x0a) )
				p++;
			continue;			
		}
		
		else if( (*p == '/')  && (*(p+1) == '/' ))	//search mark
		{
			p+=2;
			while( *p && (*p != 0x0d) && (*p!=0x0a) )
				p++;	
			continue;
		}
		else if( (*p == '/')  && (*(p+1) == '*' ))		//search mark
		{
			p+=2;
			while( *p && ((*p != '*') || (*(p+1)!='/')) )
				p++;			
			p+=2;
			continue;

		}	
		//end
		if(n!=0)
		{
			if( (*p == 0x0d)  || (*p == 0x0a)  || (*p == '#'))
			{	cmdstring[n] = 0 ;
				break;
			}
		}



	
		cmdstring[n] = *p ;
		n++;		
		p++;	
		if (n == 80) break;
	}
	idx= (int)p-(int)pattern+1;

	//thrim last space
	for(;n>1;n--)
		if( (cmdstring[n-1]!=' ')  &&  (cmdstring[n-1]!='\t') )
		{	cmdstring[n]=0;
			break;
		}

	//dprintf("test=> %s \r\n", cmdstring);
	return idx;	

}
//================================================================
int CMD_AutoTest_2(int argc, char* argv[])
{

	if(argc<1)
	{
		dprintf("at <pattern no>  <loop times> <1: fail stop> <1: quiet mode>\n");	
		dprintf("patt=0: download fw\n");			
		dprintf("patt=1: init dma\n");		
		dprintf("patt=9: load from memory 0x80300000\n");			
		return;
	}

	char		AutoTestCmdLine[ MAX_MONITOR_BUFFER +1 ];

	
	unsigned int patno=1, loopno=1, failstop=0, quiet=0; 
	if(argc>=1)		patno = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	if(argc>=2)		loopno = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	if(argc>=3)		failstop = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	if(argc>=4)		quiet = strtoul((const char*)(argv[3]), (char **)NULL, 16);	
	
	switch(patno)
	{
		case 0:	test_pattern_ptr2=PCIE_PATT0; 			break;
		//case 1:	test_pattern_ptr2=SLV_PATT1; 			break;
		case 9:	test_pattern_ptr2=0x80300000; 			break;		
		default: test_pattern_ptr2=NULL; 					break;
	}

	if(test_pattern_ptr2==NULL)		
		return;

	int i;
	at2_errcnt=0;	
	at2_mode=1;
	for(i=0;i<loopno;i++)
	{
	int rc=0;
	ExtractACmdLine2(test_pattern_ptr2,  AutoTestCmdLine, 1);  //init

	while(1)
	{
		memset(AutoTestCmdLine,0,MAX_MONITOR_BUFFER+1);
		rc=ExtractACmdLine2(test_pattern_ptr2,  AutoTestCmdLine, 0);
		if(rc==0)
			break;
		
		if(quiet==0)	
		dprintf("\n=> %s  ....",AutoTestCmdLine);
		
		rc=RunAutoTestCmd2(AutoTestCmdLine, TestCmdTable, sizeof(TestCmdTable) / sizeof(COMMAND_TABLE) );
		if(rc==1)
			break;

		if((failstop==1) && (at2_errcnt!=0))
			break;

		
	}

	if(quiet==0)	
	dprintf("\n\n ****** Test %d times, result: Error count=%d \n\n", i,  at2_errcnt);
	if((failstop==1) && (at2_errcnt!=0))
			break;
	
	}

	if(quiet==0)	
		dprintf(" ****** Total test %d times, result: Error count=%d \n", i,  at2_errcnt);
	
	if(quiet==1)
	{
		if(at2_errcnt==0)	dprintf("PASS\n");
		else					dprintf("FAIL\n");			
	}

	at2_mode=0;
}


//---------------------------------------------------------------------------
int RunAutoTestCmd2(const char *cmdstr, COMMAND_TABLE *pTestCmdTable, int len )
{
	int		argc ;
	char**		argv ;
	int		i, retval ;
		
		argc = GetArgc( (const char *)cmdstr );
		argv = GetArgv( (const char *)cmdstr );
		if( argc < 1 ) return 0;
		StrUpr( argv[0] );



		//----		
		if(!strcmp( argv[0], "~") || !strcmp( argv[0], "Q") )	//return 1 to go back up directory
			return 1;
		if( *(argv[0])=='#' )	
			return 0;		
		if( *(argv[0])=='/' && *(argv[0]+1)=='/' )		// meet "//" to do nothing
			return 0;
/*
		if(argv[1])	cmd_line.fr= strtoul((const char*)(argv[1]), (char **)NULL, 16);	
		else			cmd_line.fr=0;
		if(argv[2])	cmd_line.sc= strtoul((const char*)(argv[2]), (char **)NULL, 16);	
*/
		//execute function
		//int len=sizeof(pTestCmdTable) / sizeof(COMMAND_TABLE);

		for( i=0 ; i < len ; i++ )
		{
			if( ! strcmp( argv[0], pTestCmdTable[i].cmd ) )
			{				
				if(pTestCmdTable[i].func)
					retval = pTestCmdTable[i].func( argc - 1 , argv+1 );
				memset(argv[0],0,sizeof(argv[0]));
				break;
			}
		}
		if(i==len) dprintf("Unknown command !\r\n");

		return 0;
}

//---------------------------------------------------------------------------
int CMD_Wait2(int argc, char* argv[])
{
	if( argc < 1 ) 
	{
		dprintf("wait  t msec.\n");		
		dprintf("wait  r reg mask expval timeout \n");	
		return 0;
	}

	StrUpr( argv[0] );

	if( ! strcmp( argv[0], "T" ) )
	{
		if(argc>=2)
		{	
			int wait = strtoul((const char*)(argv[1]), (char **)NULL, 10);		
			delay_ms(wait);
		}
	}
	else if( ! strcmp( argv[0], "R" ) )	
	{

		if(argc>=4)
		{
			unsigned int regaddr = strtoul((const char*)(argv[1]), (char **)NULL, 10);
			unsigned int mask = strtoul((const char*)(argv[2]), (char **)NULL, 10);
			unsigned int expval = strtoul((const char*)(argv[3]), (char **)NULL, 10);

			while(1)
			{
				if( (REG32(regaddr) & mask ) ==  expval)
					break;
			}
		}

	}
		
}; 
//---------------------------------------------------------------------------

int  PCIE_PowerDown(int argc, char* argv[])
{
	 #define PCIE_PHY0 	0xb8b01008
	 
	if( argc < 1 ) 
	{
		dprintf("epdn mode.\n");	
		dprintf("epdn 0: D0 ->L0 \n");			
		dprintf("epdn 3: D3hot ->L1 \n");
		dprintf("epdn 4: board cast PME_TurnOff \n");	
		
		dprintf("epdn 7: enable aspm and L0 entry \n");	
		dprintf("epdn 8: enable aspm and L1 entry \n");	
		dprintf("epdn 9: diable  aspm \n");	
			
		dprintf("Link status=%x \n", REG32(0xb8b00728)&0x1f );		
		return 0;
	}


	int mode = strtoul((const char*)(argv[0]), (char **)NULL, 10);	
	int tmp;



	if(mode==0)
	{

		#if 1 //saving more power, leave L1 write
		HostPCIe_SetPhyMdioWrite(0xf, 0x0f0f);
		#endif	
		
		tmp = REG32(0xb8b10044) &( ~(3));  //D0
		REG32(0xb8b10044) = tmp|  (0);  //D0	
		dprintf("D0 \n");
	
	}

	
	if(mode==3)
	{

		#if 1 //saving more power
		REG32(0xb8b10080)|= (0x100);  //enable clock PM
		#endif
		
		tmp = REG32(0xb8b10044) &( ~(3));  //D0
		REG32(0xb8b10044) = tmp|  (3);  //D3	
		//HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
		dprintf("D3 hot \n");		

		#if 1 //saving more power		
		HostPCIe_SetPhyMdioWrite(0xf, 0x0708);
		#endif
	}

	if(mode==4)
	{	
		#if 1 //saving more power   leave L1 write
		HostPCIe_SetPhyMdioWrite(0xf, 0x0f0f);
		#endif	
	
		REG32(0xb8b01008) |= (0x200);  		
		dprintf("Host boardcase PME_TurnOff \n");		
	}
	if(mode==7)
	{
		REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
		REG32(0xb8b1070c) |=  ((3)<<27) | ((1)<<24);   //L1=3us, L0s=1us

		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);		

		REG32(0xb8b00080) |= 1;   //L0s
		REG32(0xb8b10080) |= 1;				

	}



	if(mode==8)
	{
		REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
		REG32(0xb8b1070c) |=  ((1)<<27) | ((3)<<24);   //L1=1us, L0s=3us

		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);		

		REG32(0xb8b00080) |= 3;   //L1
		REG32(0xb8b10080) |= 3;	//L1			

	}

	if(mode==9)
	{
		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);
	}
		
	
#if 0
	if(powerdown==1)
	{
//		REG32(0xb8b00044) &= ~(3);	//D0	
		HostPCIe_SetPhyMdioWrite(0xd, 0x15b6);

    REG32(PCIE_PHY0) = 0x1;	//bit7 PHY reset=0   bit0 Enable LTSSM=1
    REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1


//	__delay(9000000);  //OK
	__delay(9000000);	
//	HostPCIe_SetPhyMdioWrite(0xd, 0x15b6);

	REG32(PCIE_PHY0) = 0x1;	//bit7 PHY reset=0   bit0 Enable LTSSM=1
        REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1

//    REG32(PCIE_PHY0) = 0x1;	//bit7 PHY reset=0   bit0 Enable LTSSM=1
//    REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	
	
	//	REG32(0xb8b10044) &= ~(3);	//D0		
		dprintf("333 \n");	
		dprintf("status=%x \n", REG32(0xb8b00728) );		
	}
#endif
		dprintf("Link status=%x \n", REG32(0xb8b00728)&0x1f );			
		
};
#endif  //end CONFIG_TEST_HW
