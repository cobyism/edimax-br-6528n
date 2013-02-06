

#include <linux/interrupt.h>
#include <asm/system.h>
#include "monitor.h"


#include <asm/mipsregs.h>	//wei add
#include <asm/rtl8196.h>


#include "Test_SlvPcie.h"

#ifdef CONFIG_R8198EP_HOST
#include "pcie/boot98.h"
#endif

#ifdef CONFIG_R8198EP_HOST
const char SLV_PATT0[]={ "drst 2\n"\
"wait t 500\n"\
"init 1\n"\
"wait t 1000\n"\
"sendf 1 1\n"\
"~"};


const char SLV_PATT1[]={"init 9"\
"regw iocmd 4"\
"enirq 1"\
"~"};


const char SLV_PATT2[]={"init 9"\
"regw iocmd 4"\
"enirq 1"\
"~"};

#endif
#define DBG_PRINT dprintf
//#define DBG_PRINT(...) 
//-----------------------------------------------------------
//extern public func
extern int CmdDumpWord( int argc, char* argv[] );
extern int CmdDumpByte( int argc, char* argv[] ); //wei add
extern int CmdWriteWord( int argc, char* argv[] );

//pcie func
int SlvPCIe_CmdHelp( int argc, char* argv[] );
int SlvPCIe_RegRead(int argc, char* argv[]); 
int SlvPCIe_RegWrite(int argc, char* argv[]); 
int SlvPCIe_Rst(int argc, char* argv[]); 

int SlvPCIe_MDIORead(int argc, char* argv[]); 
int SlvPCIe_MDIOWrite(int argc, char* argv[]); 

int CMD_Dump(int argc, char* argv[]); 
int CMD_HostInit(int argc, char* argv[]); 
int CMD_DMATx(int argc, char* argv[]); 
int CMD_DMARx(int argc, char* argv[]); 

int CMD_AutoTest(int argc, char* argv[]); 
int CMD_PktGen(int argc, char* argv[]); 

int CMD_DMALoop(int argc, char* argv[]); 
int CMD_ENIRQ(int argc, char* argv[]); 
int CMD_DbgMsg(int argc, char* argv[]); 

//----------------------------------------------
#ifdef CONFIG_R8198EP_HOST
int CMD_HostRST(int argc, char* argv[]); 
//NFBI
int TestCmd_SetJumpCode(int argc, char* argv[]); 
int TestCmd_TESTRAM(int argc, char* argv[]); 
int CMD_LoadF(int argc, char* argv[]); 
int CMD_SendF(int argc, char* argv[]); 
//auti test use
int CMD_Wait(int argc, char* argv[]); 
int SlvPCIe_VarRead(int argc, char* argv[]); 
int SlvPCIe_VarWrite(int argc, char* argv[]); 
int SlvPCIe_IsmRead(int argc, char* argv[]); 
int SlvPCIe_IsmWrite(int argc, char* argv[]); 
#endif

void DMA_RxPoll();
void DMA_Stop(void);
void DMA_Init();
void EnIRQ(int enableirq);
void DDR_calibration();
unsigned int pktgen(unsigned int mode, unsigned char *pbuff, unsigned int pklen ,unsigned char initval);
//=================================================================

COMMAND_TABLE	SlvPCIe_CmdTable[] =
{
	{ "?"	  ,0, SlvPCIe_CmdHelp			, "HELP (?)				    : Print this help message"					},
	{ "Q"   ,0, NULL			, "Q: return to previous menu"					}, 	//wei add		
	{ "HELP"  ,0, SlvPCIe_CmdHelp			, NULL					},

	//extern func
	{ "DW"	  ,2, CmdDumpWord		, "DW <Address> <Len>"},
	{ "EW",2, CmdWriteWord, "EW <Address> <Value1> <Value2>..."},

	{ "REGR"   ,1, SlvPCIe_RegRead			, "REGR: Slave PCIe Reg Read"},	
	{ "REGW"   ,1, SlvPCIe_RegWrite			, "REGW <offset> <val>: Slave PCIe Reg Write "},	

	{ "MDIOR"   ,1, SlvPCIe_MDIORead			, "MDIOR: Reg Read"},	
	{ "MDIOW"   ,1, SlvPCIe_MDIOWrite			, "MDIOW <reg> <val>:  "},	

	{ "INIT"   ,1, CMD_HostInit			, "INIT:  "},		
	{ "DUMP"   ,1, CMD_Dump			, "DUMP <rc|ep>:  "},		
	{ "TX"   ,1, CMD_DMATx			, "TX:  "},	
	{ "RX"   ,1, CMD_DMARx			, "RX:  "},		

	{ "PKGEN"   ,1, CMD_PktGen			, "PKTGEN: mode "},	
	{ "DMALOOP"   ,1, CMD_DMALoop			, "DMALOOP: mode "},	

	{ "ENIRQ"   ,1, CMD_ENIRQ			, "ENIRQ: mode "},		
	{ "DMSG"   ,1, CMD_DbgMsg			, "DMSG: mode "},		
	
#ifdef CONFIG_R8198EP_HOST
	{ "HRST"   ,1, CMD_HostRST			, "HRST: Host Reset device"},
	{ "DRST"   ,1, SlvPCIe_Rst			, "DRST :Device Reset "},		
	
	{ "TRAM"   ,1, TestCmd_TESTRAM			, "TRAM: test ram "},		
	{ "SENDJ"   ,1, TestCmd_SetJumpCode			, "SendJ:  mode Addr Len chk "},			
//	{ "LOADF"   ,1, CMD_LoadF			, "LoadF:  mode  "},	
	{ "SENDF"   ,1, CMD_SendF			, "SendF:  mode Addr Len chk "},		

	{ "AT"   ,1, CMD_AutoTest			, "AUTO: auto test "},	
	{ "WAIT"   ,1, CMD_Wait			, "Wait:  unit : sec "},		
	
	{ "VARR"   ,1, SlvPCIe_VarRead			, "VARR: Slave PCIe Reg Read"},	
	{ "VARW"   ,1, SlvPCIe_VarWrite			, "VARW <offset> <val>: Slave PCIe Reg Write "},			
	{ "ISMR"   ,1, SlvPCIe_IsmRead			, "ISMR: <reg> <mask> : <expval>"},	
	{ "ISMW"   ,1, SlvPCIe_IsmWrite			, "ISMW <offset> <val>  "},
	

#endif	
#ifdef CONFIG_R8198EP_DEVICE
	{ "RST"   ,1, SlvPCIe_Rst			, "RST :DMA Reset "},		
#endif
	



};

int dbg_msg=0;

int at_errcnt=0;

int DMA_mode=0;
#define DMAMODE_DEFAULT 0
#define DMAMODE_HOSTTX_LOOP 1
#define DMAMODE_DEVICETX_LOOP 2
#define DMAMODE_TXSEQ 4
#define DMAMODE_ISM 8
/*
DMAMODE_TXSEQ mode
= v, v+1, v+2, v+3, v+4 ......len is random.
*/

#define PKTGEN_ValInc (1<<0)
#define PKTGEN_ValFix (1<<1)
#define PKTGEN_ValRand  (1<<2)

#define PKTGEN_LenInc  (1<<3)
#define PKTGEN_LenFix  (1<<4)
#define PKTGEN_LenRand  (1<<5)
#define PKTGEN_LenRand4B  (1<<6)

#define PLTGEN_AllRand (PKTGEN_LenRand|PKTGEN_ValRand)


//DMA test use
unsigned int DMATEST_TxPktNum=0;
unsigned int DMATEST_TxPktLen=0;
unsigned int DMATEST_TxPktMaxNum=0;


//DMA Tx seq use
int DMATEST_TXFB=0;  //last packet, first byte content
int DMATEST_TXLEN=0;
int DMATEST_RXFB=0;
int DMATEST_RXLEN=0;


//DMA frag use
int DMATEST_FRAG_EN=0;
int DMATEST_FRAG_SUMLEN=0;

//counter
int TxPktOk=0;
int TxPktErr=0;
int TxPktLen=0;
int RxPktOk=0;
int RxPktErr=0;
int RxPktLen=0;
//----------------------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_HOST
//CNT  set 0: is GPIO   
//DIR set 0: is input, 1 is output
#define Set_NFBI_RESET_INIT()  {  REG32(PEFGHCNR_REG) &=  ~(0x1<<8) ;	\	
	      						 REG32(PEFGHDIR_REG) |=  (0x1<<8);  	}        
						   
#define Set_NFBI_RESET_L()	(REG32(PEFGHDAT_REG) &= (~(1<<8)) )  
#define Set_NFBI_RESET_H()	(REG32(PEFGHDAT_REG) |=  (1<<8))
#endif
//---------------------------------------------------------------------------------------
void Test_Slv_PCIe_Entry(void)
{
	int i;
#ifdef CONFIG_R8198EP_HOST
	Set_NFBI_RESET_INIT();		
	CMD_HostRST(0,"");
#else
//	GPIO_INIT(0);
//	GPIO_INIT(1);
	DBG_PRINT("switch to slave-pcie device \n");
	REG32(0xb8000048)&= ~(1<<27);  //set 0 is slave PCIe, set 1 is RC.
#endif
	RunMonitor(TEST_SlvPCIe_PROMPT, SlvPCIe_CmdTable, sizeof(SlvPCIe_CmdTable) / sizeof(COMMAND_TABLE) );
	EnIRQ(0);
}

//----------------------------------------------------------------------------
int SlvPCIe_CmdHelp( int argc, char* argv[] )
{
	int	i, LineCount ;

    printf("----------------- COMMAND MODE HELP ------------------\n");
	for( i=0, LineCount = 0 ; i < (sizeof(SlvPCIe_CmdTable) / sizeof(COMMAND_TABLE)) ; i++ )
	{
		if( SlvPCIe_CmdTable[i].msg )
		{
			LineCount++ ;
			printf( "%s\n", SlvPCIe_CmdTable[i].msg );
			
		}
	}
    
	return TRUE ;
}

//----------------------------------------------------------------------------
int SlvPCIe_RegRead(int argc, char* argv[])
{

	if(argc<1)
	{//dump all	
	       DBG_PRINT("\n"); 
		   
		DBG_PRINT("DMA_TXFDP=%08x\n", REG32(SPE_DMA_TXFDP));
	 	DBG_PRINT("DMA_TXCDO=%x\n", REG32(SPE_DMA_TXCDO));  		  
	 	DBG_PRINT("DMA_RXFDP=%08x\n", REG32(SPE_DMA_RXFDP));                  
	 	DBG_PRINT("DMA_RXCDO=%x\n", REG32(SPE_DMA_RXCDO));                  
 	DBG_PRINT("DMA_TXOKCNT=%08x\n", REG32(SPE_DMA_TXOKCNT));                  
 	DBG_PRINT("DMA_RXOKCNT=%08x\n", REG32(SPE_DMA_RXOKCNT));                  

	 	DBG_PRINT("DMA_IOCMD=%x\n", REG32(SPE_DMA_IOCMD));                  
	 	DBG_PRINT("DMA_IM=%x\n", REG32(SPE_DMA_IM ));  	          
	 	DBG_PRINT("DMA_IMR=%x\n", REG32(SPE_DMA_IMR ));  	          
	 	DBG_PRINT("DMA_ISR=%x\n", REG32(SPE_DMA_ISR ));                    
#ifdef CONFIG_R8198EP_HOST
	 	DBG_PRINT("DMA_SIZE=%x\n", REG32(SPE_DMA_SIZE ));   		
#endif                                                        
	 	DBG_PRINT("ISM_LR=%x\n", REG32(SPE_ISM_LR));  
 #ifdef CONFIG_R8198EP_HOST	          
	 	DBG_PRINT("ISM_OR=%x\n", REG32(SPE_ISM_OR));   
	 	DBG_PRINT("ISM_DR=%x\n", REG32(SPE_ISM_DR));   
#else 	          
	 	DBG_PRINT("ISM_BAR=%08x\n", REG32(SPE_ISM_BAR));  
#endif		
	                                                        
#ifdef CONFIG_R8198EP_HOST         
	 	DBG_PRINT("NFBI_CMD=%x\n", REG32(SPE_NFBI_CMD)); 
	 	DBG_PRINT("NFBI_ADDR=%x\n", REG32(SPE_NFBI_ADDR)); 
	 	DBG_PRINT("NFBI_DR=%x\n", REG32(SPE_NFBI_DR)); 
#endif                                               
	 	DBG_PRINT("NFBI_SYSSR=%x\n", REG32(SPE_NFBI_SYSSR));               
	 	DBG_PRINT("NFBI_SYSCR=%x\n", REG32(SPE_NFBI_SYSCR));      
		
#ifdef CONFIG_R8198EP_HOST
	 	DBG_PRINT("NFBI_DCR=%x\n", REG32(SPE_NFBI_DCR));               
	 	DBG_PRINT("NFBI_DTR=%x\n", REG32(SPE_NFBI_DTR));     
	 	DBG_PRINT("NFBI_DDCR=%x\n", REG32(SPE_NFBI_DDCR));               
	 	DBG_PRINT("NFBI_TRXDLY=%x\n", REG32(SPE_NFBI_TRXDLY));     	
#endif
#ifdef CONFIG_R8198EP_DEVICE
	 	DBG_PRINT("MDIO=%x\n", REG32(SPE_MDIO));               
	 	DBG_PRINT("PWRCR=%x\n", REG32(SPE_PWRCR));     
	 	DBG_PRINT("BIST1=%x\n", REG32(SPE_BIST_FAILCHK1));               
	 	DBG_PRINT("BIST2=%x\n", REG32(SPE_BIST_FAILCHK1));     
	 	DBG_PRINT("SRAMEMA=%x\n", REG32(SPE_SRAMEMA));   
#endif

		return;
	}

	int addr,val;
	unsigned int check=0,mask=0xffffffff,expval=0;
	
	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "TXFDP" ) )		addr=SPE_DMA_TXFDP;
	else if( ! strcmp( argv[0], "TXCDO" ) )		addr=SPE_DMA_TXCDO;	
	else if( ! strcmp( argv[0], "RXFDP" ) )		addr=SPE_DMA_RXFDP;
	else if( ! strcmp( argv[0], "RXCDO" ) )		addr=SPE_DMA_RXCDO;

	else if( ! strcmp( argv[0], "TXOKCNT" ) )		addr=SPE_DMA_TXOKCNT;
	else if( ! strcmp( argv[0], "RXOKCNT" ) )		addr=SPE_DMA_RXOKCNT;
	
	else if( ! strcmp( argv[0], "IOCMD" ) )		addr=SPE_DMA_IOCMD;
	else if( ! strcmp( argv[0], "IM" ) )		addr=SPE_DMA_IM;
	else if( ! strcmp( argv[0], "IMR" ) )		addr=SPE_DMA_IMR;
	else if( ! strcmp( argv[0], "ISR" ) )		addr=SPE_DMA_ISR;	
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "SIZE" ) )		addr=SPE_DMA_SIZE;	
#endif	
	else if( ! strcmp( argv[0], "ILR" ) )		addr=SPE_ISM_LR;	
#ifdef CONFIG_R8198EP_HOST
	else if( ! strcmp( argv[0], "IOR" ) )		addr=SPE_ISM_OR;	
	else if( ! strcmp( argv[0], "IDR" ) )		addr=SPE_ISM_DR;		
#else
	else if( ! strcmp( argv[0], "IBAR" ) )		addr=SPE_ISM_BAR;		
#endif	
#ifdef CONFIG_R8198EP_HOST
	else if( ! strcmp( argv[0], "CMD" ) )		addr=SPE_NFBI_CMD;		
	else if( ! strcmp( argv[0], "ADDR" ) )		addr=SPE_NFBI_ADDR;	
	else if( ! strcmp( argv[0], "DR" ) )		addr=SPE_NFBI_DR;		
#endif	
	else if( ! strcmp( argv[0], "SYSCR" ) )		addr=SPE_NFBI_SYSCR;	
	else if( ! strcmp( argv[0], "SYSSR" ) )		addr=SPE_NFBI_SYSSR;	
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "DCR" ) )		addr=SPE_NFBI_DCR;	
	else if( ! strcmp( argv[0], "DTR" ) )		addr=SPE_NFBI_DTR;		
	else if( ! strcmp( argv[0], "DDCR" ) )		addr=SPE_NFBI_DDCR;	
	else if( ! strcmp( argv[0], "TRXDLY" ) )		addr=SPE_NFBI_TRXDLY;		
#endif
	//else {	DBG_PRINT("Wrong Reg Name \n");	return; }
	else
	{
		int offset= strtoul((const char*)(argv[0]), (char **)NULL, 16);
		if(offset<0x80000000)		
#ifdef CONFIG_R8198EP_HOST
		{	addr=PCIE0_RC_CFG_BASE+offset;  	}  //base=PCIE0_RC_CFG_BASE		
#else
		{	addr=SPE_EP_CFG_BASE+offset;		}
#endif
		else
			addr=offset;						//base=offset, absoluate memory address.
	}

	val=PCIE_MEM32_READ(addr);

	//regr iocmd
	if(argc==1)
	{	//DBG_PRINT("Addr %s=%x \n", argv[0], val );
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
		DBG_PRINT("Addr %08x, %s=%08x \n", addr, argv[0],val&mask );			
	}
	else
	{
		if( (val&mask) !=expval)
		{	DBG_PRINT("Fail, addr=%08x val=%x, expval=%x \n", addr, val, expval);
			at_errcnt++;
		}
		else
			DBG_PRINT("Pass \n");

	}		

}
//----------------------------------------------------------------------------
int SlvPCIe_RegWrite(int argc, char* argv[])
{
	if(argc<2)
	{	 
		DBG_PRINT("regw <reg_name> <val> \n");		
		DBG_PRINT("regw <reg_name> <mask> <value>\n");			
		DBG_PRINT("ex: regw IMR  ffffffff \n");			
		return;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned int addr,mask=0,val;

	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "TXFDP" ) )		addr=SPE_DMA_TXFDP;
	else if( ! strcmp( argv[0], "TXCDO" ) )		addr=SPE_DMA_TXCDO;	
	else if( ! strcmp( argv[0], "RXFDP" ) )		addr=SPE_DMA_RXFDP;
	else if( ! strcmp( argv[0], "RXCDO" ) )		addr=SPE_DMA_RXCDO;
	
	else if( ! strcmp( argv[0], "TXOKCNT" ) )	addr=SPE_DMA_TXOKCNT;
	else if( ! strcmp( argv[0], "RXOKCNT" ) )	addr=SPE_DMA_RXOKCNT;
	
	else if( ! strcmp( argv[0], "IOCMD" ) )		addr=SPE_DMA_IOCMD;
	else if( ! strcmp( argv[0], "IM" ) )		addr=SPE_DMA_IM;
	else if( ! strcmp( argv[0], "IMR" ) )		addr=SPE_DMA_IMR;
	else if( ! strcmp( argv[0], "ISR" ) )		addr=SPE_DMA_ISR;	
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "SIZE" ) )		addr=SPE_DMA_SIZE;	
#endif	
        //ISM
	else if( ! strcmp( argv[0], "ILR" ) )		addr=SPE_ISM_LR;	
#ifdef CONFIG_R8198EP_HOST
	else if( ! strcmp( argv[0], "IOR" ) )		addr=SPE_ISM_OR;	
	else if( ! strcmp( argv[0], "IDR" ) )		addr=SPE_ISM_DR;		
#else
	else if( ! strcmp( argv[0], "IBAR" ) )		addr=SPE_ISM_BAR;	
#endif
#ifdef CONFIG_R8198EP_HOST
	//NFBI
	else if( ! strcmp( argv[0], "CMD" ) )		addr=SPE_NFBI_CMD;		
	else if( ! strcmp( argv[0], "ADDR" ) )		addr=SPE_NFBI_ADDR;	
	else if( ! strcmp( argv[0], "DR" ) )		addr=SPE_NFBI_DR;		
#endif	
	else if( ! strcmp( argv[0], "SYSCR" ) )		addr=SPE_NFBI_SYSCR;	
	else if( ! strcmp( argv[0], "SYSSR" ) )		addr=SPE_NFBI_SYSSR;	
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "DCR" ) )		addr=SPE_NFBI_DCR;	
	else if( ! strcmp( argv[0], "DTR" ) )		addr=SPE_NFBI_DTR;		
	else if( ! strcmp( argv[0], "DDCR" ) )		addr=SPE_NFBI_DDCR;	
	else if( ! strcmp( argv[0], "TRXDLY" ) )		addr=SPE_NFBI_TRXDLY;		
#endif	
	//else DBG_PRINT("Wrong Reg Name \n");	
	else
	{
		int offset= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		if(offset<0x80000000)			
#ifdef CONFIG_R8198EP_HOST
		{	addr=PCIE0_RC_CFG_BASE+offset;		}
#else
		{	addr=SPE_EP_CFG_BASE+offset;		}
#endif
		else
			addr=offset;
	}	

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
		PCIE_MEM32_WRITE(addr,  val );		//avoid DR issue.
	else		
		PCIE_MEM32_WRITE(addr, (PCIE_MEM32_READ(addr) & mask) | val );	
	
}
//----------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_HOST
int SlvPCIe_VarRead(int argc, char* argv[])
{
	unsigned int val;
	unsigned int check=0,mask=0xffffffff,expval=0;

	if(argc<1)
	{//dump all	
	       DBG_PRINT("\n"); 		   
		DBG_PRINT("INT=%08x\n", 	SPE_IntFlag);
		return;
	}


	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "INT" ) )		val=SPE_IntFlag;	
	else {	DBG_PRINT("Wrong Reg Name \n");	return; }
/*	
	else
	{
		return;
	}
*/
	//regr iocmd
	if(argc==1)
	{	
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
		DBG_PRINT(" %s=%08x \n",  argv[0],val&mask );			
	}
	else
	{
		if( (val&mask) !=expval)
		{	DBG_PRINT("Fail, %s val=%x, expval=%x \n", argv[0], val, expval);
			at_errcnt++;
		}
		else
			DBG_PRINT("Pass \n");
	}	


}
//----------------------------------------------------------------------------
int SlvPCIe_VarWrite(int argc, char* argv[])
{
	if(argc<2)
	{	 
		DBG_PRINT("varw <name> <val> \n");		
		DBG_PRINT("varw <name> <mask> <value>\n");			
		DBG_PRINT("ex: varw IMR  ffffffff \n");			
		return;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned int mask=0,val;
	unsigned int *addr;

	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "INT" ) )		addr=&SPE_IntFlag;
	//else if( ! strcmp( argv[0], "TXCDO" ) )		addr=SPE_DMA_TXCDO;	
	//else if( ! strcmp( argv[0], "RXFDP" ) )		addr=SPE_DMA_RXFDP;
	
	//else DBG_PRINT("Wrong Reg Name \n");	
	else
	{
	
	}	

	//regw iocmd 0x0001
	if(argc==2)
	{	mask=0;
		val = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	}

	//regw iocmd 0x0001 0x0001
	else if(argc>=3)
	{	mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		val = strtoul((const char*)(argv[2]), (char **)NULL, 16);
	}
		
	*(addr)=(*(addr) & mask) | val;
	
}
#endif
//----------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_HOST

//---------------------------------------------------------------------------
// return x;   0: mean communication fail, 1: communication scuess 
// retval:     command return val;
//----------------------------------------------------------------------------
unsigned int Fill_ISM_Mag(unsigned int mode, unsigned int argv1, unsigned int argv2, unsigned int argv3, unsigned int *retval)
{
	int rc=0;
	if(REG32(SPE_ISM_LR)<0x10)
	{
		DBG_PRINT("Error ISM Length is 0 \n");
		return;
	}
	REG32(SPE_ISM_OR)=0;
	
	REG32(SPE_ISM_DR)=('w'<<24)|('e' <<16) | ('i'<<8) | (mode<<0) ;	//fill magic word

	REG32(SPE_ISM_DR)=argv1;
	REG32(SPE_ISM_DR)=argv2;
	REG32(SPE_ISM_DR)=argv3;

	SPE_IntFlag=0;
	REG32(SPE_DMA_IOCMD)|=SPE_DMA_IOCMD_SWINT;  //trigger

	int st,et,wait=3;
	st=get_timer_jiffies();
				
	while(1)
	{

		if(SPE_IntFlag& SPE_DMA_ISR_SWINT)   //interrupt
			break;
		if(REG32(SPE_DMA_ISR)&SPE_DMA_ISR_SWINT)   //polling
			break;

		et=get_timer_jiffies();
		
			if((et-st)  > wait*100)   //wait 3 sec
			{	
				DBG_PRINT("ISM timeout!\n");	
				return rc;
			}

	}
	
	//read back
	delay_ms(100);
	REG32(SPE_ISM_OR)=0;
	int mag=REG32(SPE_ISM_DR);
	if(mag== ( ('w'<<24)|('e' <<16) | ('i'<<8) | (mode<<0) ))    //still host write, not response.
	{	DBG_PRINT("Fail! Device not response. ISM MAG=%x \n",mag);
		return rc;
	};	
	
	if(mag!= (('w'<<24)|('e' <<16) | ('o'<<8) | (mode<<0) ))     //device response, but not expect.
	{	DBG_PRINT("Fail! Device response, but not expect. ISM MAG=%x \n",mag);
		return rc;
	};	
	
		
	rc=1;
	unsigned int ra1=0,ra2=0,ra3=0;
	ra1=REG32(SPE_ISM_DR);
	ra2=REG32(SPE_ISM_DR);
	ra3=REG32(SPE_ISM_DR);
	

	switch(mode)
	{
		case SPE_ISM_MAG_REGR:		*retval=ra3; break;
		case SPE_ISM_MAG_REGW:     *retval=1; break;
		

	}
	return rc;


}
//----------------------------------------------------------------------------
SlvPCIe_IsmRead(int argc, char* argv[])
{

	unsigned int rc,val,retval;	
	unsigned int check=0,mask=0xffffffff,expval=0;

	if(argc<1)
	{//dump all	
	       DBG_PRINT("\n"); 		   
		DBG_PRINT("ismr reg mask : expval \n" );
		return;
	}

	unsigned int reg= strtoul((const char*)(argv[0]), (char **)NULL, 16);


	rc=Fill_ISM_Mag(SPE_ISM_MAG_REGR,reg,0,0, &retval);
	if(rc==0)
	{
		DBG_PRINT("Fail ISM commnuication !\n");
		return 0;
	}
	val=retval;
		
	//regr iocmd
	if(argc==1)
	{	
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
		DBG_PRINT(" %s=%08x \n",  argv[0],val&mask );			
	}
	else
	{
		if( (val&mask) !=expval)
		{	DBG_PRINT("Fail, %s val=%x, expval=%x \n", argv[0], val, expval);
			at_errcnt++;
		}
		else
			DBG_PRINT("Pass \n");
	}	


}
//----------------------------------------------------------------------------
int SlvPCIe_IsmWrite(int argc, char* argv[])
{
	if(argc<2)
	{	 
		DBG_PRINT("ismw <name> <val> \n");		
		DBG_PRINT("ismw <name> <mask> <value>\n");			
		DBG_PRINT("ex: ismw IMR  ffffffff \n");			
		return;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned int mask=0,val,retval;
	unsigned int *addr;

	unsigned int reg= strtoul((const char*)(argv[0]), (char **)NULL, 16);
	
	//regw iocmd 0x0001
	if(argc==2)
	{	mask=0;
		val = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	}

	//regw iocmd 0x0001 0x0001
	else if(argc>=3)
	{	mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		val = strtoul((const char*)(argv[2]), (char **)NULL, 16);
	}
		
	if(Fill_ISM_Mag(SPE_ISM_MAG_REGW,reg,mask,val,&retval)==0)
		DBG_PRINT("Write Fail, maybe ISM have error! \n");
	
}
#endif
//----------------------------------------------------------------------------

int SlvPCIe_Rst( int argc, char* argv[] )
{
#ifdef CONFIG_R8198EP_HOST
	 #define CLK_MANAGE 	0xb8000010

	if(argc<1)
	{	 
		DBG_PRINT("Usage: drst 9 : DMA reset \n");		
		DBG_PRINT("	drst 0:  chip_reboot_n reset Low\n");	
		DBG_PRINT("	drst 1:  chip_reboot_n reset High \n");
		DBG_PRINT("	drst 2:  chip_reboot_n reset 0->1 \n");		
		return;	
	}

	int reset_no = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	if(reset_no==9)
	{
		 REG32(SPE_DMA_IOCMD)=1;
		 delay(1000);
		 
		 EnIRQ(0);	 
		 REG32(CLK_MANAGE) &= ~(1<<24);  
		 delay(1000);	
		 REG32(CLK_MANAGE) |= (1<<24);   	 //do  PCIE Device Reset
		 delay(1000);	 
		 return 0;

	}

	if(reset_no==0)
	{
		Set_NFBI_RESET_L();		
	}
	if(reset_no==1)
	{
		Set_NFBI_RESET_H();
	}	
	if(reset_no==2)
	{
		 EnIRQ(0);	
		delay_ms(500);		 
		//Set_NFBI_RESET_H();
		Set_NFBI_RESET_L();	
		delay_ms(500);		
		Set_NFBI_RESET_H();
		delay_ms(500);		
	}
#endif


#ifdef CONFIG_R8198EP_DEVICE

	volatile int i;


	//fix phy parameter.
	DBG_PRINT("do PHY MDIO write\n");	
	SetPhyMdioWrite( 0x00, 0x1065);
	SetPhyMdioWrite( 0x01, 0x0003);
	SetPhyMdioWrite( 0x03, 0x6D49);
	SetPhyMdioWrite( 0x06, 0xF008);
	SetPhyMdioWrite( 0x07, 0x31FF);
	SetPhyMdioWrite( 0x08, 0x18D7);
	SetPhyMdioWrite( 0x09, 0x930C);
	SetPhyMdioWrite( 0x0A, 0x03C9);
	SetPhyMdioWrite( 0x0B, 0x0111);
	SetPhyMdioWrite( 0x0D, 0x1466);


	//Power Control register
	//REG32(SPE_PWRCR) |=(SPE_PWRCR_ENTERL1 | SPE_PWRCR_AUXPWRDET | SPE_PWRCR_LOWPWREN | SPE_PWRCR_CLKREQ );
	//DBG_PRINT("do PWRCR regsiter\n");	
	
	//DMA RST
	REG32(SPE_DMA_IOCMD) |= SPE_DMA_IOCMD_RST;    
	for(i=0;i<1000000;i++)		;
	DBG_PRINT("do DMA reset 0\n");		

	//phy reset
	REG32(SPE_PWRCR)=REG32(SPE_PWRCR) &  (~(1<<7));   //write 0
	for(i=0;i<1000000;i++)		;	
	DBG_PRINT("do phy reset 0\n");	
	
	REG32(SPE_PWRCR)=REG32(SPE_PWRCR) |  (1<<7);   //write 1
	DBG_PRINT("do phy reset 1\n");	

#endif
}

//----------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_HOST
//This code come from frank, JSW:PCIE reset procedure
int CMD_HostRST(int argc, char* argv[])
{

	EnIRQ(0); //avoid NeedBootCode interrupt happen!
	//---------------------------------------
	#define Use_External_PCIE_CLK 0
	int  PCIE_Port0and1_8196B_208pin= 0;  //0: one port, 1: two port
	 
	//PCIE Register
	 #define CLK_MANAGE 	0xb8000010

	 #define PCIE_MDIO_P0	0xb800003C
	 #define PCIE_MDIO_P1	0xb8000040
	 
	 #define PCIE_PLL 	0xb8000044

	 #define PCIE_PHY0_REG 	0xb8b01000
	 #define PCIE_PHY1_REG 	0xb8b21000
	 #define PCIE_PHY0 	0xb8b01008
	 #define PCIE_PHY1 	0xb8b21008
		

	 //1. "Enable internal PCIE PLL 	 
	 #if Use_External_PCIE_CLK  
	  REG32(PCIE_PLL) = 0x358; //Use External PCIE CLK (clock chip)
	 #else
	  //REG32(PCIE_PLL) = 0x359; //Use Internal PCIE CLK
	  REG32(PCIE_PLL) = 0x09; //Use Internal PCIE CLK and PCIE fine-tune
	 #endif
	 
	 //2.Active LX & PCIE Clock
	 REG32(CLK_MANAGE) |= 0x00FFFFC0; 
	 
	 // 3.MDIO Reset
	 REG32(PCIE_MDIO_P0) = 0x01;
	 REG32(PCIE_MDIO_P0) = 0x03;
	 
	 if( PCIE_Port0and1_8196B_208pin==1)
	 {
		  REG32(PCIE_MDIO_P1) = 0x01;
		  REG32(PCIE_MDIO_P1) = 0x03;
	 }
	 
	 //4. PCIE PHY Reset
	 REG32(PCIE_PHY0) = 0x01;
	 REG32(PCIE_PHY0) = 0x81;
	 
	 if( PCIE_Port0and1_8196B_208pin==1)
	 {
		  REG32(PCIE_PHY1) = 0x01;
		  REG32(PCIE_PHY1) = 0x81;
	 }
	 

	 //5. PCIE PHY Analog Fine-tune
	 //PORT0
	 REG32(PCIE_PHY0_REG) = 0xC39A0301; //for port 0 fine-tune  
	 
	 //REG32(PCIE_PHY0_REG) = 0xC3930301; //for port 0 fine-tune    //96BU	 
	 
	 if( PCIE_Port0and1_8196B_208pin==1)	  
	{	  REG32(PCIE_PHY1_REG) = 0xC39A0301; //for port 1 fine-tune
	 }  
	
	 
	 // 6. PCIE Device Reset
	 REG32(CLK_MANAGE) |=  (1<<24);

	 DBG_PRINT("PCIE_Reset finish \n");
}; 
#endif
//----------------------------------------------------------------------------

int SetPhyMdioWrite(unsigned int regaddr, unsigned short val)
{
	REG32(SPE_MDIO)= ( (regaddr&0x1f)<<SLVPE_MDIO_REG_OFFSET) | ((val&0xffff)<<SLVPE_MDIO_DATA_OFFSET)  | (1<<SLVPE_MDIO_RDWR_OFFSET) ; 
	//delay 
	volatile int i;
	for(i=0;i<5555;i++)  ;
}


//----------------------------------------------------------------------------
int SlvPCIe_MDIORead(int argc, char* argv[])
{

	unsigned int i,val,j;
	for(i=0; i<=0x1f; i++)
	{
		REG32(SPE_MDIO)= (i<<SLVPE_MDIO_REG_OFFSET) | (0<<SLVPE_MDIO_RDWR_OFFSET) ;   
		for(j=0;j<0x5555;j++) ;
		
		val=REG32(SPE_MDIO);
		val=( val& SLVPE_MDIO_DATA_MASK ) >> SLVPE_MDIO_DATA_OFFSET;
		DBG_PRINT("MDIO Reg %x=%x \n", i,val);

	}



}; 
//----------------------------------------------------------------------------
int SlvPCIe_MDIOWrite(int argc, char* argv[])
{

	if(argc<2)
	{	 
		DBG_PRINT("mdiow <addr> <val> \n");		
		DBG_PRINT("ex: mdiow 00  ffff \n");			
		return;	
	}


	unsigned int addr = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	unsigned int val = strtoul((const char*)(argv[1]), (char **)NULL, 16);	

	REG32(SPE_MDIO)= (addr<<SLVPE_MDIO_REG_OFFSET) | (val<<SLVPE_MDIO_DATA_OFFSET)  | (1<<SLVPE_MDIO_RDWR_OFFSET) ;   ;   

}; 
//----------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_HOST
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

void PCIE_IOMEM_ConfigInit()
{
      PCIE_MEM32_WRITE(PCIE0_RC_CFG_BASE + 0x04, 0x00100007);
      PCIE_MEM8_WRITE(PCIE0_RC_CFG_BASE + 0x78, (PCIE_MEM8_READ(PCIE0_EP_CFG_BASE + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B

      // 1. Set 8111C EP
      //PCIE_MEM32_WRITE(PCIE0_EP_CFG_BASE + 0x04, 0x00180007);  // Mem, IO Enable
      PCIE_MEM32_WRITE(PCIE0_EP_CFG_BASE + 0x04, 0x00000007);  // Mem, IO Enable
      
      PCIE_MEM32_WRITE(PCIE0_EP_CFG_BASE + 0x10, (PCIE0_EP_IO_BASE | 0x00000001) & 0x1FFFFFFF);  // Set BAR0
      if(PCIE_MEM32_READ(PCIE0_EP_CFG_BASE + 0x10) != ((PCIE0_EP_IO_BASE | 0x00000001) & 0x1FFFFFFF))
      {	at_errcnt++;
      		DBG_PRINT("Fail, Read Bar0=%x \n", PCIE_MEM32_READ(PCIE0_EP_CFG_BASE + 0x10)); //for test
      	}
	  
      PCIE_MEM32_WRITE(PCIE0_EP_CFG_BASE + 0x14, (PCIE0_EP_MEM_BASE | 0x00000004) & 0x1FFFFFFF);  // Set BAR1
	if(PCIE_MEM32_READ(PCIE0_EP_CFG_BASE + 0x14)!=(PCIE0_EP_MEM_BASE & 0x1FFFFFFF))
	{	at_errcnt++;
      		DBG_PRINT("Fail, Read Bar1=%x \n", PCIE_MEM32_READ(PCIE0_EP_CFG_BASE + 0x14));      //for test
	}
	DBG_PRINT("Set BAR finish \n");
	
      PCIE_MEM8_WRITE(PCIE0_EP_CFG_BASE + 0x78, (PCIE_MEM8_READ(PCIE0_EP_CFG_BASE + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
      PCIE_MEM8_WRITE(PCIE0_EP_CFG_BASE + 0x79, (PCIE_MEM8_READ(PCIE0_EP_CFG_BASE + 0x79) & (~0x70)) | MAX_READ_REQSIZE_128B);  // Set MAX_REQ_SIZE to 128B


	volatile int i=5555;
	while(i--) ;

	int vid=PCIE_MEM32_READ(PCIE0_EP_CFG_BASE) &0x0000ffff;   //0x819810EC
	int pid=PCIE_MEM32_READ(PCIE0_EP_CFG_BASE) >>16;
	DBG_PRINT("VID=%x, PID=%x \n", vid, pid );
	if( (vid!= 0x10ec) || (pid!=0x8198))
	{	DBG_PRINT(" !!!  Read VID PID fail !!! \n");
		at_errcnt++;
	}
}
#endif
//-------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_HOST
void DumpConfigSpace(unsigned int addr)
{

	dwdump(addr, 16);
	
	DBG_PRINT(" 00:Device:Vender=%08x \n", PCIE_MEM32_READ(addr) );
	DBG_PRINT(" 04:Status : Cmd=%08x \n", PCIE_MEM32_READ(addr+0x04)  );
	DBG_PRINT(" 08:Class=%08x \n", PCIE_MEM32_READ(addr+0x08 ));
	DBG_PRINT(" 0C:BIST=%08x \n", PCIE_MEM32_READ(addr+0x0c ));
	DBG_PRINT(" 10:BAR 0=%08x \n", PCIE_MEM32_READ(addr+0x10 ));
	DBG_PRINT(" 14:BAR 1=%08x \n", PCIE_MEM32_READ(addr+0x14 ));
	DBG_PRINT(" 18:BAR 2=%08x \n", PCIE_MEM32_READ(addr+0x18 ));
	DBG_PRINT(" 1C:BAR 3=%08x \n", PCIE_MEM32_READ(addr+0x1c ));
	DBG_PRINT(" 20:BAR 4=%08x \n", PCIE_MEM32_READ(addr+0x20 ));
	DBG_PRINT(" 24:BAR 5=%08x \n", PCIE_MEM32_READ(addr+0x24 ));
	DBG_PRINT(" 28:CIS=%08x \n", PCIE_MEM32_READ(addr+0x28 ));	
	DBG_PRINT(" 2C:sub-class=%08x \n", PCIE_MEM32_READ(addr+0x2c ));	
	DBG_PRINT(" 30:rom=%08x \n", PCIE_MEM32_READ(addr+0x30 ));	
	DBG_PRINT(" 34:reserved=%08x \n", PCIE_MEM32_READ(addr+0x34 ));	
	DBG_PRINT(" 38:reserved=%08x \n", PCIE_MEM32_READ(addr+0x38 ));		
	DBG_PRINT(" 3C:Max=%08x \n", PCIE_MEM32_READ(addr+0x3c ));		
	DBG_PRINT("-------------------------------------------\n");
}
//-------------------------------------------------------------------------
void DumpCapability(unsigned int addr)
{

	DBG_PRINT(" 40:PM=%08x \n", PCIE_MEM32_READ(addr) );   
	DBG_PRINT(" 44:PMCSR : Cmd=%08x \n", PCIE_MEM32_READ(addr+0x04)  );
	DBG_PRINT(" 48:reserved=%08x \n", PCIE_MEM32_READ(addr+0x08 ));
	DBG_PRINT(" 4C:reserved=%08x \n", PCIE_MEM32_READ(addr+0x0c ));
	
	DBG_PRINT(" 50:MSI=%08x \n", PCIE_MEM32_READ(addr+0x10 ));	
	DBG_PRINT(" 54:MSI_Low=%08x \n", PCIE_MEM32_READ(addr+0x14 ));
	DBG_PRINT(" 58:MSI High=%08x \n", PCIE_MEM32_READ(addr+0x18 ));
	DBG_PRINT(" 5C:MSI Data=%08x \n", PCIE_MEM32_READ(addr+0x1c ));

	
	DBG_PRINT(" 70:PCIE=%08x \n", PCIE_MEM32_READ(addr+0x30 ));	

	DBG_PRINT(" 74:Device cap=%08x \n", PCIE_MEM32_READ(addr+0x34 ));	
	DBG_PRINT(" 78:Device status ctrl=%08x \n", PCIE_MEM32_READ(addr+0x38 ));		
	
	DBG_PRINT(" 7C:Link cap=%08x \n", PCIE_MEM32_READ(addr+0x3c ));	
	DBG_PRINT(" 80:Link status: link ctrl=%08x \n", PCIE_MEM32_READ(addr+0x40 ));		
	
}
#endif
//-------------------------------------------------------------------------

int CMD_HostInit(int argc, char* argv[])
{
#ifdef CONFIG_R8198EP_HOST
	volatile int i;
	unsigned int initmode=9;
	if(argc<1)
	{

		DBG_PRINT("init 1: set bar \n");
		DBG_PRINT("init 2: init dma without irq \n");		
		DBG_PRINT("init 3: init dma with irq \n");	
		DBG_PRINT("init 9: init all \n");		
		return;
	}
	if(argc>=1)	
		initmode = strtoul((const char*)(argv[0]), (char **)NULL, 16);	

	if( initmode>=1 )
		PCIE_IOMEM_ConfigInit();


	if( initmode>=2 )
	{
		EnIRQ(0);
		DMA_Stop();
		DMA_Init();	
	}
	
	if(initmode>=3)
	{	EnIRQ(1);
		DMA_RxPoll();
		REG32(SPE_NFBI_CMD)=NFBI_CMD_SWAP;
	}
#else
	unsigned int val=0; 

	if(argc>=1)		
	val = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	
	EnIRQ(0);
	DMA_Stop();
	DMA_Init();
	if(val==1)
	{	
		EnIRQ(1);
}
	DMA_RxPoll();
#endif
}



//================================================================

int CMD_Dump(int argc, char* argv[])
{
	int i,len=0x80;
	if(argc<2)
	{
#ifdef CONFIG_R8198EP_HOST
		DBG_PRINT("PowerStat=%x (0x11:L0, 0x12:L0s, 0x14:L1 idle, 0x15: L2 idle)\n", REG32(PCIE0_RC_CFG_BASE+0x728)&0x001f );  //0x7028 bit [4:0] 
		DBG_PRINT("LinkUp=%x (0x01:link up)\n\n", (REG32(PCIE0_RC_CFG_BASE+0x72c)&0x0010) >>4);  //0x702c bit 4
	
		DBG_PRINT("dump <rc|ep> <offset>  \n");	
		DBG_PRINT("dump <ISM> <Len> \n");	
#endif
		DBG_PRINT("dump <txd/rxd> \n");		
		DBG_PRINT("dump <txb/rxb> <number> <Len> \n");	
		return;
	}
	StrUpr( argv[0] );

	//----------------------------------------
	if( ! strcmp( argv[0], "TXD" ) )	
	{

		//unsigned int num = strtoul((const char*)(argv[1]), (char **)NULL, 16);			
		dwdump( &pTxDescPtr[0], TX_DESC_NUM*4);
	}
	else if( ! strcmp( argv[0], "RXD" ) )	
	{
		//unsigned int num = strtoul((const char*)(argv[1]), (char **)NULL, 16);			
		dwdump(&pRxDescPtr[0], RX_DESC_NUM*4);		

	}
	//-------------------------------------------
	else if( ! strcmp( argv[0], "TXB" ) )	
	{
		unsigned int num=0;	
		if(argc>= 2)		 num = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		if(argc>=3)	 len = strtoul((const char*)(argv[2]), (char **)NULL, 16);		
		//ddump( (int)pTxBuffPtr[num] | UNCACHE_MASK, BUF_SIZE);
		ddump( (int)pTxBuffPtr[num] | UNCACHE_MASK, len);		
	}
	else if( ! strcmp( argv[0], "RXB" ) )	
	{
		unsigned int num=0;	
		if(argc>= 2)		 num = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
		if(argc>=3)	 len = strtoul((const char*)(argv[2]), (char **)NULL, 16);		
		//ddump( (int)pRxBuffPtr[num] | UNCACHE_MASK, BUF_SIZE);
		ddump( (int)pRxBuffPtr[num] | UNCACHE_MASK, len);		

	}	



#ifdef CONFIG_R8198EP_HOST	
	if( ! strcmp( argv[0], "RC" ) )
	{
		DBG_PRINT("RC Cfg \n");
		DumpConfigSpace(PCIE0_RC_CFG_BASE);
		DumpCapability(PCIE0_RC_CFG_BASE+0x40);
	}
	else if( ! strcmp( argv[0], "EP" ) )	
	{
		DBG_PRINT("EP Cfg \n");
		DumpConfigSpace(PCIE0_EP_CFG_BASE);
		DumpCapability(PCIE0_EP_CFG_BASE+0x40);		

	}
	//---------------------------------------------
	else if( ! strcmp( argv[0], "ISM" ) )	
	{

		if(argc>= 2)		 len = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		else				len=REG32(SPE_ISM_LR);
		
		REG32(SPE_ISM_OR)=0;

		
		for(i=0; i<len; i=i+4)
		{
			if(i%16==0)
			{
				DBG_PRINT("\nOff:[%08x]    ", i);
			}			
			DBG_PRINT("%08x    ",REG32(SPE_ISM_DR));			
		}		
		DBG_PRINT("\n");

	}	

	//---------------------------------------------
	else if( ! strcmp( argv[0], "NFBI" ) )	
	{	unsigned int offset = strtoul((const char*)(argv[1]), (char **)NULL, 16);	

		REG32(SPE_NFBI_ADDR)=offset;
		if(argc>= 2)		 len = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		else				len=0x80;

		
		for(i=0; i<len; i=i+4)
		{
			if(i%16==0)
			{
				DBG_PRINT("\nAddr:[%08x]    ", offset+i);
			}			
			DBG_PRINT("%08x    ",REG32(SPE_NFBI_DR));			
		}		
		DBG_PRINT("\n");

	}
#endif
	//---------------------------------------------
	else if( ! strcmp( argv[0], "CNT" ) )	
	{
		DBG_PRINT("TxPktOk=%x \n", TxPktOk);
		DBG_PRINT("TxPktErr=%x \n", TxPktErr);
		DBG_PRINT("TxPktLen=%x \n", TxPktLen);
		DBG_PRINT("RxPktOk=%x \n", RxPktOk);
		DBG_PRINT("RxPktErr=%x \n", RxPktErr);
		DBG_PRINT("RxPktLen=%x \n", RxPktLen);		
	}	
	//---------------------------------------------
	else if( ! strcmp( argv[0], "PKGEN" ) )	
	{	
		DBG_PRINT("Pklen=%x bytes\n", DMA_len);
		ddump(DMA_buff,DMA_len);
	}
}
//-------------------------------------------------------------------------------
void ClearAllPktCount(void)
{
			//txseq mode	 		
			DMATEST_RXFB=0;	DMATEST_RXLEN=0;	//clear. Because compare will use last packet.
			DMATEST_TXFB=0;  DMATEST_TXLEN=0;			

			 TxPktOk=0;
			 TxPktErr=0;
			 TxPktLen=0;
			 RxPktOk=0;
			 RxPktErr=0;
			 RxPktLen=0;
}

//================================================================
#ifdef CONFIG_R8198EP_HOST
const char *test_pattern_ptr;    //point to test script buffer

//return position 
unsigned int ExtractACmdLine(const char *pattern,  char *cmdstring, int first)
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

	//DBG_PRINT("test=> %s \r\n", cmdstring);
	return idx;	

}
//================================================================
int CMD_AutoTest(int argc, char* argv[])
{

	if(argc<1)
	{
		DBG_PRINT("at <pattern no>  <loop times> <1: fail stop>\n");	
		DBG_PRINT("patt=0: download fw\n");			
		DBG_PRINT("patt=1: init dma\n");		
		DBG_PRINT("patt=9: load from memory 0x80300000\n");			
		return;
	}

	char		AutoTestCmdLine[ MAX_MONITOR_BUFFER +1 ];

	
	unsigned int patno=1, loopno=1, failstop=0; 
	if(argc>=1)		patno = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	if(argc>=2)		loopno = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	if(argc>=3)		failstop = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	
	switch(patno)
	{
		case 0:	test_pattern_ptr=SLV_PATT0; 			break;
		case 1:	test_pattern_ptr=SLV_PATT1; 			break;
		case 9:	test_pattern_ptr=0x80300000; 			break;		
		default: test_pattern_ptr=NULL; 					break;
	}

	if(test_pattern_ptr==NULL)		
		return;

	int i;
	at_errcnt=0;	
	for(i=0;i<loopno;i++)
	{
	int rc=0;
	ExtractACmdLine(test_pattern_ptr,  AutoTestCmdLine, 1);  //init

	while(1)
	{
		memset(AutoTestCmdLine,0,MAX_MONITOR_BUFFER+1);
		rc=ExtractACmdLine(test_pattern_ptr,  AutoTestCmdLine, 0);
		if(rc==0)
			break;
		
		DBG_PRINT("\n=> %s  ....",AutoTestCmdLine);
		
		rc=RunAutoTestCmd(AutoTestCmdLine);
		if(rc==1)
			break;

		if((failstop==1) && (at_errcnt!=0))
			break;

		
	}

		
	DBG_PRINT("\n\n ****** Test %d times, result: Error count=%d \n\n", i,  at_errcnt);
	if((failstop==1) && (at_errcnt!=0))
			break;
	
	}

	
	DBG_PRINT(" ****** Total test %d times, result: Error count=%d \n", i,  at_errcnt);
}


//---------------------------------------------------------------------------
int RunAutoTestCmd(const char *cmdstr)
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
		int len=sizeof(SlvPCIe_CmdTable) / sizeof(COMMAND_TABLE);
		for( i=0 ; i < len ; i++ )
		{
			if( ! strcmp( argv[0], SlvPCIe_CmdTable[i].cmd ) )
			{				
				if(SlvPCIe_CmdTable[i].func)
					retval = SlvPCIe_CmdTable[i].func( argc - 1 , argv+1 );
				memset(argv[0],0,sizeof(argv[0]));
				break;
			}
		}
		if(i==len) DBG_PRINT("Unknown command !\r\n");

		return 0;
}

//---------------------------------------------------------------------------

#endif

//===================================================================

//-----------------------------------------------------------------------------------
int Lan_Initialed = 0;
//-----------------------------------------------------------------------------

inline void DMA_TxPoll()
{
	EP_REG32_WRITE(SPE_DMA_IOCMD, EP_REG32_READ(SPE_DMA_IOCMD)|SPE_DMA_IOCMD_TXPOLL); 
}

inline void DMA_RxPoll()
{
	EP_REG32_WRITE(SPE_DMA_IOCMD, EP_REG32_READ(SPE_DMA_IOCMD) |SPE_DMA_IOCMD_RXEN); 
}

void DMA_Stop(void)
{	
    Lan_Initialed = 0;
	EP_REG32_WRITE(SPE_DMA_IOCMD, EP_REG32_READ(SPE_DMA_IOCMD) & ~(SPE_DMA_IOCMD_RXEN) );  	
	EP_REG32_WRITE(SPE_DMA_IMR,0x00000000);
	//EnIRQ(0);
}	  

//-----------------------------------------------------------------------------
static int firstTime=1;
int DMA_Receive(void** input, unsigned int* pLen,  
			unsigned int *prxinfo1, unsigned int *prxinfo2, unsigned int *prxinfo3, 
			unsigned int *drop)
{
	struct slvpcie_desc_t *pDesc; 
	unsigned char	*pData;
	unsigned int len;

	if(pRxDescPtr==NULL)
	{	DBG_PRINT("pRxDescPtr =NULL!\n");
		return -1;
	}
	
	if(pRxDescPtr[SPE_rxtail].flag & SPE_DESC_OWN_NIC)
	{	DMA_RxPoll();	
		if(dbg_msg)
			DBG_PRINT("Rx empty!\n");
		*input=NULL;
		return -1;
	}
	//REG32(DMA_ISR)=0xffffffff;



	pDesc = (unsigned char *)&pRxDescPtr[SPE_rxtail];
	if(pDesc==NULL)
	{
			DBG_PRINT("Rx desc zero!\r\n");
			return -1;
	}
	pData = (unsigned char*)Physical2NonCache(pRxDescPtr[SPE_rxtail].DataPtr);
	if(pData==NULL)
	{
			DBG_PRINT("Rx buffer pointer is  zero!\r\n");
			return -1;
	}
	
	len= pRxDescPtr[SPE_rxtail].flag & SPE_DESC_BUFFLEN_MASK;
	
	if(dbg_msg)
	{
		DBG_PRINT("--rx ------------------------------curr offset=%d\r\n", SPE_rxtail);
		dwdump(pDesc,4);
		ddump(pData,len);	
	}
		

		
		int idx;
	if(firstTime==1)
	{	//first time, do'nt prepare last own bit
		firstTime=0;	
	}
	else
	{

		if(SPE_rxtail !=0)
			idx=SPE_rxtail-1;	
		else			
			idx=RX_DESC_NUM-1;

		//reset previous desc	
		pRxDescPtr[idx].flag= BUF_SIZE;	
		pRxDescPtr[idx].DataPtr = Virtual2Physical((unsigned int)pRxBuffPtr[idx] ); 	
		//pRxDescPtr[idx].info2= 0;	
		//pRxDescPtr[idx].info3= 0;				

		if(idx==RX_DESC_NUM-1)
		{	pRxDescPtr[idx].flag |= SPE_DESC_EOR;	
		}
		pRxDescPtr[idx].flag |= SPE_DESC_OWN_NIC;		
	}

/*
	DBG_PRINT("S=%x H=%x\n",SPE_rxtail, REG32(SPE_DMA_RXCDO));
	dwdump(&pRxDescPtr[0], RX_DESC_NUM*4);		
*/	
	SPE_rxtail++;
	SPE_rxtail %= RX_DESC_NUM;

	
	DMA_RxPoll();	
	if(drop)	*drop=((pDesc->flag) & SPE_DESC_DROP_MASK)>>SPE_DESC_DROP_OFFSET;
	if(prxinfo1) *prxinfo1=((pDesc->flag) & SPE_DESC_INFO1_MASK)>>SPE_DESC_INFO1_OFFSET;
	if(prxinfo2) *prxinfo2=pDesc->info2;
	if(prxinfo3) *prxinfo3=pDesc->info3;	
	*input=pData;
	*pLen=len;
	

	return (*pLen);
}
//-----------------------------------------------------------------------------
inline volatile unsigned int DESC_SWAP32(unsigned int data)   //wei add, for sata endian swap
{
	unsigned int cmd=data;
	unsigned char *p=&cmd;	
	return ( (p[3]<<24) |  (p[2]<<16) | (p[1]<<8)  | p[0] );	
}

//-----------------------------------------------------------------------------
inline int Check_DMATxFull()
{
	if(pTxDescPtr==NULL)
	{	DBG_PRINT("pTxDescPtr=NULL \n");
			return -1;
	}
	if(pTxDescPtr[SPE_txtail].flag & SPE_DESC_OWN_NIC)			
		return 1;	
	return 0;
}
//-----------------------------------------------------------------------------
int DMA_Transmit(void * buff, unsigned int length, 
			unsigned int fs, unsigned int ls, 
			unsigned int txinfo1, unsigned int txinfo2, unsigned int txinfo3,
			unsigned int offset)
{
	int i;


	if(pTxDescPtr==NULL)
	{	DBG_PRINT("pTxDescPtr=NULL \n");
			return -1;
	}

#if 1	
	if(pTxDescPtr[SPE_txtail].flag & SPE_DESC_OWN_NIC)
	{	if(dbg_msg)
			DBG_PRINT("Tx full\r\n");		
		return -1;
	}

#endif
#if 1 	//use internal Buffer
	pTxDescPtr[SPE_txtail].flag=length;
	/* Copy Packet Content */	
	unsigned char *pData=(unsigned int)pTxBuffPtr[SPE_txtail] | UNCACHE_MASK;
	pData+=(offset&0x3);
	//if(buff!=pData)
	memcpy(pData, buff, length);	
	pTxDescPtr[SPE_txtail].DataPtr = Virtual2Physical((unsigned int)pData);

#else  //use external buffer
	unsigned char *pData=(unsigned int)buff| UNCACHE_MASK;
	pTxDescPtr[SPE_txtail].DataPtr = Virtual2Physical(buff);
#endif

	//fill tx info
	pTxDescPtr[SPE_txtail].flag |=(txinfo1 & 0xfff)<<SPE_DESC_INFO1_OFFSET;
	pTxDescPtr[SPE_txtail].info2=txinfo2;
	pTxDescPtr[SPE_txtail].info3=txinfo3;

	//pTxDescPtr[SPE_txtail].flag &= ~(SPE_DESC_FS | SPE_DESC_LS);	
	//pTxDescPtr[SPE_txtail].flag |= SPE_DESC_FS | SPE_DESC_LS;	
	if(fs==1)	    pTxDescPtr[SPE_txtail].flag |= SPE_DESC_FS;
	if(ls==1)    pTxDescPtr[SPE_txtail].flag |= SPE_DESC_LS;
	


	if(SPE_txtail==TX_DESC_NUM-1)
		pTxDescPtr[SPE_txtail].flag |=SPE_DESC_EOR;

	// trigger to send		
	pTxDescPtr[SPE_txtail].flag |= SPE_DESC_OWN_NIC;

	if(dbg_msg)
	{

		DBG_PRINT("--tx ------------------------curr offset=%d\r\n", SPE_txtail);
		dwdump(&pTxDescPtr[SPE_txtail],4);
		ddump(pData,length);	
	}



	DMA_TxPoll();

#if 0  //wait tx finish	
	while( pTxBDPtr[txBDtail].flag & OWN_BIT)
		{
			//delay_msec(1);		
			//DBG_PRINT("OWN=%x\r\n",(volatile unsigned int)pTxBDPtr[txBDtail].flag & OWN_BIT);
		}
//DBG_PRINT("OWN_2=%x\r\n",(volatile unsigned int)pTxBDPtr[txBDtail].flag & OWN_BIT);	
#endif

	/* advance one */
	SPE_txtail++;
	SPE_txtail %= TX_DESC_NUM;
	TxPktLen+=length;
	
	return length;
}



//----------------------------------------------------------------------------
#ifdef CONFIG_R8198EP_DEVICE
Analyze_TCPIP_Pkt(unsigned char *buf, unsigned int len)
{
	int i;
	
	typedef union {
		unsigned long	s_addr;
		unsigned char 	ip[4];
	} in_addr;
	#define ETH_IPADDRLEN		4	/* Size of ip address */
	#define ETH_MACLEN		6	/* Size of Ethernet address */
	

	#define ETH_ZLEN		60	/* Minimum packet */
	#define FRAME_IP		0x0800
	#define FRAME_ARP		0x0806	
	


	//------------------------------------------

	struct ethIIhdr{	
				unsigned char dhwaddr[6];   //6 bytes
				unsigned char shwaddr[6];	//6 bytes	
				unsigned short type;          // 2 bytes
			};  //14 bytes
			
			

	#define ETH_HLEN		14	/* Size of ethernet header */
	//---------------------------------------
	struct arphdr {
				unsigned short hwtype;
				unsigned short protocol;
				char hwlen;
				char protolen;
				unsigned short opcode;
				char shwaddr[6];
				char sipaddr[4];
				char thwaddr[6];
				char tipaddr[4];
		};

	#define ETH_ARPLEN 28

	#define ARP_REQUEST	1
	#define ARP_REPLY	2
	#define RARP_REQUEST	3
	#define RARP_REPLY	4	
	//----------------------------------------
	struct iphdr {
				char verhdrlen;
				char service;
				unsigned short len;
				unsigned short ident;
				unsigned short frags;
				char ttl;
				char protocol;
				unsigned short chksum;				
				//in_addr src;
				//in_addr dest;
				char sipaddr[4];
				char dipaddr[4];
		};
	#define ETH_IPLEN 20	
	#define IP_ICMP 0x01
	//----------------------------------------
	struct icmphdr {
				u8 type;
				u8 code;
				u16 checksum;
				u16 id;
				u16 seq;	
				u8 data[56];
		};
	#define ICMP_ECHO_REQUEST 0x08
	#define ICMP_ECHO_REPLY 0x00

	#define ETH_ICMPLEN 64
	//--------------------------------------	
		const unsigned char mymac[6]={ 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c };
		

		//ARP protocol
		unsigned char arp_req[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		
 
		//ARP protocol
		unsigned char txbuf[256]={0};



		struct ethIIhdr *rx_ethiipkt=(struct ethIIhdr *) &(buf[0]);	
		
		if( (memcmp(rx_ethiipkt->dhwaddr, arp_req, 6)==0 ) && (rx_ethiipkt->type==FRAME_ARP) )
		{
			DBG_PRINT("arp pkt coming, len=%d \n",len);

			//struct ethII *rx_ethiipkt=(struct ethiipacket *) &(buf[0]);	
 			struct arphdr *rx_arppkt =(struct arphdr *) &(buf[ETH_HLEN]);

			struct ethIIhdr *ethiireply=(struct ethIIhdr *) &(txbuf[0]);	
 			struct arphdr *arpreply=(struct arphdr *) &(txbuf[ETH_HLEN]);	


			//Fill EthII hdr
			memcpy(ethiireply->dhwaddr, rx_ethiipkt->shwaddr, ETH_MACLEN);
			memcpy(ethiireply->shwaddr, mymac, ETH_MACLEN);
			ethiireply->type=FRAME_ARP;
 	
			
		    // Fill i arp hdr
		    arpreply->hwtype = htons(1);
		    arpreply->protocol = htons(FRAME_IP);  /*that is 0x0800*/
		    arpreply->hwlen = ETH_MACLEN;
		    arpreply->protolen = 4;
		    arpreply->opcode = htons(ARP_REPLY);
			
		    memcpy(&(arpreply->shwaddr), &(mymac), ETH_MACLEN);
		    memcpy(&(arpreply->sipaddr), &(rx_arppkt->tipaddr), ETH_IPADDRLEN);
		    memcpy(&(arpreply->thwaddr), &(rx_arppkt->shwaddr), ETH_MACLEN);
		    memcpy(&(arpreply->tipaddr), &(rx_arppkt->sipaddr), ETH_IPADDRLEN);



			DBG_PRINT("tx arp pkt \n");
			while(DMA_Transmit(txbuf, ETH_HLEN+ETH_ARPLEN, 1, 1, 0, 0, 0, 0)==-1)
				;
			return;
		}	
		if( (memcmp(rx_ethiipkt->dhwaddr, mymac, 6)==0 ) && (rx_ethiipkt->type==FRAME_IP) )
		{
			DBG_PRINT("ip pkt coming, len=%d \n",len);
			
 			struct iphdr *rx_iphdr =(struct iphdr *) &(buf[ETH_HLEN]);	
 			struct icmphdr *rx_icmphdr =(struct icmphdr *) &(buf[ETH_HLEN+ETH_IPLEN]);				

			struct ethIIhdr *tx_ethiihdr=(struct ethIIhdr *) &(txbuf[0]);	
			struct iphdr *tx_iphdr=(struct iphdr *) &(txbuf[ETH_HLEN]);	
			struct icmphdr *tx_icmphdr=(struct icmphdr *) &(txbuf[ETH_HLEN+ETH_IPLEN]);				

	
			//Fill EthII hdr
			memcpy(tx_ethiihdr->dhwaddr, rx_ethiipkt->shwaddr, ETH_MACLEN);
			memcpy(tx_ethiihdr->shwaddr, mymac, ETH_MACLEN);
			tx_ethiihdr->type=FRAME_IP;	
			 
			 //Fill IP header			 
			 tx_iphdr->verhdrlen = 0x45;
			 tx_iphdr->service = 0;
			 tx_iphdr->len = (32);
			 tx_iphdr->ident = 0;
			 tx_iphdr->frags = 0;
			 tx_iphdr->ttl = 80;
			 tx_iphdr->protocol = IP_ICMP;
			 tx_iphdr->chksum = 0;
			memcpy(tx_iphdr->sipaddr, rx_iphdr->dipaddr, ETH_IPADDRLEN);
			memcpy(tx_iphdr->dipaddr, rx_iphdr->sipaddr, ETH_IPADDRLEN); 

		 
			 tx_iphdr->chksum = ipheader_chksum((Int16 *)tx_iphdr, sizeof(struct iphdr));

			//fill icmp header
			tx_icmphdr->type=ICMP_ECHO_REPLY;
			tx_icmphdr->code=0;
			//do checksum last.
			tx_icmphdr->id=rx_icmphdr->id;
			tx_icmphdr->seq=rx_icmphdr->seq;	
			memcpy(tx_icmphdr->data, rx_icmphdr->data, 56);		
			
			tx_icmphdr->checksum= ipheader_chksum((Int16 *)tx_icmphdr, sizeof(struct icmphdr));;

			
			DBG_PRINT("tx icmp pkt \n");			
			while(DMA_Transmit(txbuf, ETH_HLEN+ETH_IPLEN+ETH_ICMPLEN , 1, 1, 0, 0, 0, 0)==-1)
				;
			return;
		}		


};
#endif

//-----------------------------------------------------------------------------
void processRx()
{
		int i;
		unsigned char *pbuff;
		int Len;
		unsigned int txinfo1, txinfo2,txinfo3,drop;

		unsigned int rxhookcnt=0;
		while(DMA_Receive(&pbuff, &Len,  &txinfo1, &txinfo2, &txinfo3, &drop)!=-1)
		{

			if(drop==1)
			{	RxPktErr++;
				DBG_PRINT("Drop pkt !\n");
			}
#ifdef CONFIG_R8198EP_HOST
			if(txinfo1&DMAMODE_HOSTTX_LOOP)  //  host -> device -> host
#else
			if(txinfo1&DMAMODE_DEVICETX_LOOP)  //   device -> host
#endif
			{	//compare
				int cnt=pbuff[0];	
				if(cnt<DMATEST_RXFB)
				{
					DBG_PRINT("fail, re-send pkt coming, cnt=%x expcnt=%x \n", cnt, DMATEST_RXFB);
					at_errcnt++;
				}
				else if(cnt>DMATEST_RXFB )
				{
					DBG_PRINT("fail, lost pkt, cnt=%x expcnt=%x \n", cnt, DMATEST_RXFB);
					at_errcnt++;
				}
				DMATEST_RXFB=cnt;
			}
#ifdef CONFIG_R8198EP_HOST		
			else if(txinfo1&DMAMODE_DEVICETX_LOOP)  // device -> host
#else
			else if(txinfo1&DMAMODE_HOSTTX_LOOP) //  host -> device -> host
#endif
			{	//host only re-send pkt back.
				int offset=((int)pbuff)&0x03;
				while(DMA_Transmit(pbuff, Len,1,1, txinfo1, txinfo2, txinfo3, offset)==-1)
				{
				}					
			}
				
			else if(txinfo1&DMAMODE_TXSEQ)  //  tx sequence
			{	
				unsigned char currcnt=pbuff[0];	
				unsigned char exp=(DMATEST_RXFB+DMATEST_RXLEN);
				if(currcnt != exp)
				{	DBG_PRINT("fail, curr rx=%x exp=%x\n", currcnt, exp);
					at_errcnt++;
				}

				//check content
				for(i=1; i<Len; i++ )
				{
					if( ((pbuff[i-1]+1)&0xff)  != (pbuff[i]&0xff) )
					{	DBG_PRINT("fail, Addr=%08x, buff[%x]=%x, buff[%x]=%x \n", &pbuff[i-1], i-1,  pbuff[i-1], i, pbuff[i]);
						at_errcnt++;
						break;
					}
				}

				DMATEST_RXFB=currcnt;
				DMATEST_RXLEN=Len;
			}
			//-----------------------------------------------
			else if(txinfo1==DMAMODE_ISM) 
			{       
				int ism_len=*((unsigned int *)pbuff);
				if(Len!=ism_len)
				{	DBG_PRINT("Fail, ism_len=%x, but dma len=%x, not the same \n", ism_len, Len );
					return ;
				}
#ifdef CONFIG_R8198EP_HOST
				//Get pkt, and use ISM for Device read and compare
				REG32(SPE_ISM_OR)=0;				
				for(i=0;i<ism_len; i=i+4)
					REG32(SPE_ISM_DR)=((int)pbuff[i+0]<<24) |((int)pbuff[i+1]<<16) | ((int)pbuff[i+2]<<8)| ((int)pbuff[i+3]<<0)   ;
#else
				//Get pkt, and use ISM for Host read and compare
				memcpy(DMA_buff, pbuff, Len);
				REG32(SPE_ISM_BAR)=DMA_buff;
				REG32(SPE_ISM_LR)=Len;
				DBG_PRINT("Issue SWINT out \n");
#endif
				REG32(SPE_DMA_IOCMD)=REG32(SPE_DMA_IOCMD) | SPE_DMA_IOCMD_SWINT;
			}
			//-----------------------------------------------
#ifdef CONFIG_R8198EP_DEVICE
			else if(txinfo1==DMAMODE_DEFAULT) 
			{	Analyze_TCPIP_Pkt(pbuff, Len);
			}
#endif			
#if 0	//open this, will lost some interrupt
			rxhookcnt++;  //because FPGA is slowly, so will in-loop here
			if(rxhookcnt>=RX_DESC_NUM/3)
				break;			
#endif		
		};	
		
}
//-----------------------------------------------------------------------------
inline int CheckDMATestTxFinish(unsigned int currpktnum, unsigned int maxpktnum)
{
#if 1	//show message		
			if((currpktnum&0xff)==0)
				DBG_PRINT("=> TxPktNum=%x \n", currpktnum);
#endif	

			if(maxpktnum!=0) 
			{
				if(currpktnum >= maxpktnum)
					return 1;
			}
			
		return 0;
}
//===================================================================
void processTx()
{
		int txhookcnt=0;
		int offset=0;
		unsigned int currcnt;	
		unsigned int currlen;
			
		if(DMA_mode==DMAMODE_TXSEQ)
		{

			if(Check_DMATxFull()==1)
				return;			
			if(CheckDMATestTxFinish(DMATEST_TxPktNum , DMATEST_TxPktMaxNum)==1)
				return;

			int txinfo1=DMAMODE_TXSEQ, txinfo2=0, txinfo3=0;
			int fs=1,ls=1;
			
			if(DMATEST_FRAG_EN==1)
			{
				if(DMATEST_TxPktLen <= DMATEST_FRAG_SUMLEN)
					DMATEST_FRAG_SUMLEN=0;	
				
	
				currcnt=DMATEST_TXFB+DMATEST_TXLEN;
				currlen=pktgen(PKTGEN_LenRand4B|PKTGEN_ValInc, &DMA_buff, 
								(DMATEST_TxPktLen - DMATEST_FRAG_SUMLEN), 
								currcnt);
				
				if(DMATEST_FRAG_SUMLEN==0)
				{	 fs=1; ls=0; 
				}
				else if( (DMATEST_FRAG_SUMLEN+currlen) <DMATEST_TxPktLen)
				{	 fs=0; ls=0; 			 
				}
				else if( (DMATEST_FRAG_SUMLEN+currlen) >=DMATEST_TxPktLen)				
				{	 fs=0; ls=1; 				 
				}
				if(DMATEST_TxPktNum == DMATEST_TxPktMaxNum-1)
					ls=1;
				// DBG_PRINT("1 DMATEST_TxPktLen=%x, DMATEST_FRAG_SUMLEN=%x , currlen=%x fs=%d, ls=%d \n",
				// 	DMATEST_TxPktLen, DMATEST_FRAG_SUMLEN, currlen, fs,ls);		
				
				
			}
			else  //DMATEST_FRAG_EN=0
			{		
				//continue tx
				currcnt=DMATEST_TXFB+DMATEST_TXLEN;			
				currlen=pktgen(PKTGEN_LenFix|PKTGEN_ValInc, &DMA_buff,DMATEST_TxPktLen, currcnt);

			}

			
			while(DMA_Transmit(DMA_buff, DMA_len,fs,ls, txinfo1, txinfo2, txinfo3, offset)!=-1)			
			{
				//tx scuess, 
				//DBG_PRINT("(%d,%d) L=%x \n", fs,ls,DMA_len);

				DMATEST_TXFB=currcnt;
				DMATEST_TXLEN=currlen;
				
				if(DMATEST_FRAG_EN==1)
				{
					DMATEST_FRAG_SUMLEN+=currlen;  //frag			
					if(DMATEST_TxPktLen <= DMATEST_FRAG_SUMLEN)
						DMATEST_FRAG_SUMLEN=0;	

				}
				else
				{
				}
				DMATEST_TxPktNum++;
				//check if stop tx
				if(Check_DMATxFull()==1)
					return;				
				if(CheckDMATestTxFinish(DMATEST_TxPktNum,DMATEST_TxPktMaxNum)==1)
					return;
				
#if 0	//open this, will lost some interrupt				
				txhookcnt++;  //because FPGA is slowly, so will in-loop here
				if(txhookcnt>=TX_DESC_NUM/3)
					break;
#endif	

				//prepare next pkt				
				if(DMATEST_FRAG_EN==1)
				{					
					currcnt=DMATEST_TXFB+DMATEST_TXLEN;	
					currlen=pktgen(PKTGEN_LenRand4B|PKTGEN_ValInc, &DMA_buff, 
								 (DMATEST_TxPktLen - DMATEST_FRAG_SUMLEN), 
								 currcnt);
					
					if(DMATEST_FRAG_SUMLEN==0)
					{	 fs=1; ls=0; 			
					}
					else if( (DMATEST_FRAG_SUMLEN+currlen) <DMATEST_TxPktLen)
					{	 fs=0; ls=0; 					
					}
					else if( (DMATEST_FRAG_SUMLEN+currlen) >=DMATEST_TxPktLen)				
					{	 fs=0; ls=1; 								
					}
					if(DMATEST_TxPktNum == DMATEST_TxPktMaxNum-1)
						ls=1;					
				 //DBG_PRINT("2 DMATEST_TxPktLen=%x, DMATEST_FRAG_SUMLEN=%x , currlen=%x fs=%d, ls=%d \n",
				 //	DMATEST_TxPktLen, DMATEST_FRAG_SUMLEN, currlen, fs,ls);	
				 
				}
				else
				{
					currcnt=DMATEST_TXFB+DMATEST_TXLEN;			
					currlen=pktgen(PKTGEN_LenFix|PKTGEN_ValInc,&DMA_buff,DMATEST_TxPktLen, currcnt);
				}			
			
		}
	}		
	//--------------------------------------------------------------------------------------------
	else if(DMA_mode==DMAMODE_ISM)
	{
			if(CheckDMATestTxFinish(DMATEST_TxPktNum,DMATEST_TxPktMaxNum)==1)
				return;
			
			int txinfo1=DMAMODE_ISM, txinfo2=0, txinfo3=0;
			int fs=1,ls=1,offset=0;

			pktgen(PKTGEN_LenFix|PKTGEN_ValRand,&DMA_buff,DMATEST_TxPktLen, 0);
			DMA_len=(DMA_len+4)&(~3);  //only support 4 bytes alignment
			*(unsigned int *)(DMA_buff)=DMA_len;
			while(DMA_Transmit(DMA_buff, DMA_len,fs,ls, txinfo1, txinfo2, txinfo3, offset)!=-1)
			{

				//check if stop tx
				DMATEST_TxPktNum++;
				if(CheckDMATestTxFinish(DMATEST_TxPktNum,DMATEST_TxPktMaxNum)==1)
					return;
				
			}
	}

	
}
//-----------------------------------------------------------------------------
void processError(int mode)
{
	if(mode==SPE_DMA_ISR_RXERR)  //rx
	{
		dwdump(pRxDescPtr,RX_DESC_NUM*4);
		//int i=REG32(SPE_ISM_LR);  //triger analyzer
		
	}
	else if(mode==SPE_DMA_ISR_TXERR)  //tx
	{
		dwdump(pTxDescPtr,TX_DESC_NUM*4);

	}



}
//-----------------------------------------------------------------------------
void processISM()
{
#ifdef CONFIG_R8198EP_HOST  //select ism
		unsigned int i,t;
		unsigned char t1,t2,t3,t4;
		unsigned int error=0;

		int maxlen=REG32(SPE_ISM_LR);
#ifdef CONFIG_R8198EP_HOST
		REG32(SPE_ISM_OR)=0;
		int len=REG32(SPE_ISM_DR); //*(unsigned int *)(DMA_buff)=DMA_len;
#else
		unsigned int *pbar=REG32(SPE_ISM_BAR);
		int len=pbar[0];
#endif
		if(len> maxlen)
		{	DBG_PRINT("Fail, rxlen=%x is big than max len=%x \n",len,maxlen);
			at_errcnt++;
			return;
		}

		unsigned char *p=DMA_buff;
		for(i=4; i<len; i=i+4)
		{
#ifdef CONFIG_R8198EP_HOST		
			t=REG32(SPE_ISM_DR);
#else
			t=pbar[i];
#endif
			
			//if(t != *(unsigned int *)(p+i) )
			//	DBG_PRINT("Fail, Rx offset:[%x]=%x, but exp is %x \n", i, t, *(unsigned int *)(p+i) );			

			t1=t>>24;
			t2=(t&0x00ff0000) >>16;
			t3=(t&0x0000ff00) >>8;
			t4=t&0x000000ff;
			
			if(t1  != *(p+i+0) )
			{	DBG_PRINT("Fail, Rx offset:[%x]=%x, but exp is %x \n", i+0, t1, *(p+i+0) );				
				error++;  
				break;
			}
			if(t2  != *(p+i+1) )
			{	DBG_PRINT("Fail, Rx offset:[%x]=%x, but exp is %x \n", i+1, t2, *(p+i+1) );
				error++;
				break;
			}
			if(t3  != *(p+i+2) )
			{	DBG_PRINT("Fail, Rx offset:[%x]=%x, but exp is %x \n", i+2, t3, *(p+i+2) );
				error++;
				break;
			}
			if(t4  != *(p+i+3) )
			{	DBG_PRINT("Fail, Rx offset:[%x]=%x, but exp is %x \n", i+3, t4, *(p+i+3) );	
				error++;
				break;
			}
						
		}
		if(error==0)
			DBG_PRINT("Got ISM, Data is PASS \n");
		else
			at_errcnt++;
#endif
#ifdef CONFIG_R8198EP_DEVICE  //select ism
	unsigned int *p=Virtual2NonCache(ISM_buff);

	if( (p[0]&0xffffff00) !=SPE_ISM_MAGNUMI )
	{	DBG_PRINT("Fail ISM MAG=%x \n", p[0]);
		return 0;
	};	

	unsigned char mode=p[0]&0xff;
	
	
	unsigned int a1=p[1];
	unsigned int a2=p[2];
	unsigned int a3=p[3];	
	
	
	switch(mode)
	{
		case SPE_ISM_MAG_REGR:		p[3]=REG32(a1); break;
		case SPE_ISM_MAG_REGW:     REG32(a1)=(REG32(a1)&a2)|a3;  break;		
		default: return ;
	}

	p[0]=('w'<<24) | ('e'<<16) | ('o'<<8) | (mode<<0);
	REG32(SPE_DMA_IOCMD)|=SPE_DMA_IOCMD_SWINT;  //trigger

#endif
}
//=======================================================================
void SPE_isr(void)  
{
	//DBG_PRINT("IRQ IN\n");	
#ifdef CONFIG_R8198EP_HOST
	//-------------------------check BAR1
	if(REG32(PCIE0_EP_CFG_BASE+0x14) != 0x19000000)
	{
		DBG_PRINT("BAR1 Miss ! \n");
		return ;
	}
#endif
	
	SPE_IntNum++;		
	unsigned int status=REG32(SPE_DMA_ISR)&REG32(SPE_DMA_IMR);
	REG32(SPE_DMA_ISR)=status;
	SPE_IntFlag|=status;		
	//DBG_PRINT("=> ISR=%x \n", status);

/*  //test irq problem
	DBG_PRINT("In ");
	char c;
	while(1)
	{	
		c = 	serial_inc();
		if(c==-1)
			continue;
		if(c=='y')
		{
			DBG_PRINT("Out \n");
			break;
		}
	}
	return;
*/	
	//---------------------------check rx	
	if(status &SPE_DMA_ISR_RXOK)
	{	//DBG_PRINT("RXOK \n");
		processRx();	
	}	

	if(status &SPE_DMA_ISR_RXERR)
	{	DBG_PRINT("=> RXERR \n");
		//RxPktErr++;
		//processError(SPE_DMA_ISR_RXERR);
	}
	
	if(status &SPE_DMA_ISR_RXDU)  //rx full
	{	//DBG_PRINT("=> RXDU \n");
		processRx();	
	}
	
	//---------------------------check tx
	if(status &SPE_DMA_ISR_TXERR)
	{	DBG_PRINT("=> TXERR \n");
		TxPktErr++;
		processError(SPE_DMA_ISR_TXERR);
	}
	if(status &SPE_DMA_ISR_TXOK)
	{	//DBG_PRINT("=> TXOK \n");
		processTx();
	
	}	
	if(status &SPE_DMA_ISR_TXTMR)
	{	DBG_PRINT("=> TXTMR \n");
	}
	if(status &SPE_DMA_ISR_TXDU)  //tx empty
	{	//DBG_PRINT("=> TXDU \n");
		processTx();

	}
	if(status &SPE_DMA_ISR_TXLEN0)
	{	DBG_PRINT("=> TXLEN0 \n");
		TxPktErr++;
		processError(SPE_DMA_ISR_TXERR);
	}
#ifdef CONFIG_R8198EP_HOST	
	//---------------------------check nfbi	
	if(status & SPE_DMA_ISR_NEEDBTCODE)
	{

		DBG_PRINT("=> NeedBtcode \n");
	}
#endif

	if(status & SPE_DMA_ISR_SWINT)
	{		
		DBG_PRINT("=> SWINT \n");
#ifdef CONFIG_R8198EP_HOST
		REG32(SPE_ISM_OR)=0;
		if((REG32(SPE_ISM_DR)&0xffffff00) !=SPE_ISM_MAGNUMO)		
		processISM();
#else
		processISM();

#endif
	}


#ifdef CONFIG_R8198EP_DEVICE
	if(status & SPE_DMA_ISR_CUSTUSEALL)
	{
		int syscr=status & SPE_DMA_ISR_CUSTUSEALL;
		int i;
		for(i=0 ; i<16; i++)
		{
			if(syscr &  (1<<i) )
			{	DBG_PRINT("=> Got IRQ CUSTUSR#%02d \n", i);
				//REG32(SPE_NFBI_SYSSR)^= (1<<i);
			}
		}		
		REG32(SPE_NFBI_SYSSR)= REG32(SPE_NFBI_SYSCR);	
	}		
#endif
		
}


struct irqaction irq_SPE = {SPE_isr, (unsigned long)NULL, (unsigned long)SPE_IRQ_NO,"SlvPCIe", (void *)NULL, (struct irqaction *)NULL};   

//-----------------------------------------------------------------------------
//=======================================================================
void DMA_Init()
{
	int i;
	//---------------------------------	
#ifdef CONFIG_R8198EP_HOST
	pTxDescPtr=(struct slvpcie_desc_t *)0xa0500000;	
	pRxDescPtr=(struct slvpcie_desc_t *)0xa0580000;	
#else
	pTxDescPtr = (struct slvpcie_desc_t *)((((unsigned int)(SlvPE_tx_desc+0x04))& 0xfffffffc)|UNCACHE_MASK);	
	pRxDescPtr = (struct slvpcie_desc_t *)((((unsigned int)(SlvPE_rx_desc+0x04))& 0xfffffffc)|UNCACHE_MASK);	
#endif

	for(i=0;i<RX_DESC_NUM;i++)
	{	
#ifdef CONFIG_R8198EP_HOST
		pRxBuffPtr[i]=0xa0680ff8+i*BUF_SIZE;
#else
		pRxBuffPtr[i]=Virtual2NonCache(&SlvPE_rx_buf[i][0]);
#endif
	}

	for(i=0;i<TX_DESC_NUM;i++)
	{
#ifdef CONFIG_R8198EP_HOST
		pTxBuffPtr[i]=0xa0600ff8+i*BUF_SIZE;
#else
		pTxBuffPtr[i]=Virtual2NonCache(&SlvPE_tx_buf[i][0]);
#endif
	}	
	//-----------------------------------------------------------
	//init Rx desc		
	for(i=0;i<RX_DESC_NUM;i++)
	{	
		pRxDescPtr[i].flag=SPE_DESC_OWN_NIC | BUF_SIZE;		
		pRxDescPtr[i].DataPtr = Virtual2Physical((unsigned int)pRxBuffPtr[i]);  	
		pRxDescPtr[i].info2= 0;	
		pRxDescPtr[i].info3= 0;		
		
	}
	pRxDescPtr[RX_DESC_NUM-1].flag |= SPE_DESC_EOR;

	//init Tx desc
	for(i=0;i<TX_DESC_NUM;i++)
	{
		pTxDescPtr[i].flag= SPE_DESC_OWN_CPU | 0;		
		pTxDescPtr[i].DataPtr= 0;	
		pTxDescPtr[i].info2= 0;	
		pTxDescPtr[i].info3= 0;			
		
	}
	pTxDescPtr[TX_DESC_NUM-1].flag |= SPE_DESC_EOR;

	SPE_txtail = 0;	// index for system to set buf to BD
	SPE_rxtail = 0;	// index for system to set buf to BD
	SPE_IntNum=0;
	ClearAllPktCount	();
	//-----------------------------------------------------------
	DBG_PRINT("Rx\n");
	dwdump(pRxDescPtr, RX_DESC_NUM*4);	
	DBG_PRINT("Tx\n");	
	dwdump(pTxDescPtr, TX_DESC_NUM*4);

	//EP_REG32_WRITE(SPE_DMA_IOCMD, EP_REG32_READ(SPE_DMA_IOCMD) |SPE_DMA_IOCMD_RST);  //rst
	//rx
	EP_REG32_WRITE(SPE_DMA_RXFDP,  Virtual2Physical(pRxDescPtr) );
	EP_REG32_WRITE(SPE_DMA_RXCDO, 0);
	
	//tx
	EP_REG32_WRITE(SPE_DMA_TXFDP, Virtual2Physical(pTxDescPtr) );
	//EP_REG32_WRITE(SPE_DMA_TXFDP, 0x3c500 );
	EP_REG32_WRITE(SPE_DMA_TXCDO,  0);		


	EP_REG32_WRITE(SPE_DMA_ISR, SPE_DMA_ISR_DMAALL);  //init rx packet			
	//EP_REG32_WRITE(SPE_DMA_IMR, SPE_DMA_IMR_DMAALL);

	
	//IOCMD
	//DMA_RxPoll();
	//DMA_TxPoll();
	

	//interrupt
	//EnIRQ(1);

	DBG_PRINT("DMA init finish \n");

}
//----------------------------------------------------------------
int CMD_ENIRQ(int argc, char* argv[])
{
	if( argc < 1 ) 
	{
		DBG_PRINT("enirq <0: disable, 1:enable> \n");
		return;
	}
	unsigned int en = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	EnIRQ(en);
	delay_ms(500);	//PCIE TLP need some time to finish.

}

//----------------------------------------------------------------
void EnIRQ(int enableirq)
{

	if(enableirq)
	{
		REG32(SPE_DMA_IMR)=0xffffffff;   //accept all interrupt
	
		int irraddr=IRR_REG+SPE_IRR_NO*4;		
	  	REG32(irraddr) = (REG32(irraddr) &~(0x0f<<SPE_IRR_OFFSET)) | (4<<SPE_IRR_OFFSET);	
		SPE_IntFlag=0;
		request_IRQ(SPE_IRQ_NO, &irq_SPE, NULL); 
		DBG_PRINT("Enable PCIE ISR\r\n");
	}
	else
	{
		REG32(SPE_DMA_IMR)=0;   //disable all interrupt
		free_IRQ(SPE_IRQ_NO);
		DBG_PRINT("Disable PCIE ISR\r\n");
		SPE_IntFlag=0;
	}
}

//----------------------------------------------------------------

int CMD_DMATx(int argc, char* argv[])
{

	int Len=BUF_SIZE;
	
	unsigned int i;	
	unsigned int len=8; 
	unsigned int txinfo1=0, txinfo2=0, txinfo3=0;	
	int offset=0;
	int fs,ls;

	if( argc < 1 ) 
	{
		DBG_PRINT("tx <mode>  <len> <info1>	<info2>	<info3>	<offset> \n");
		DBG_PRINT("tx 1t 128\n");			
		DBG_PRINT("tx <RP/UDP> \n");		
		return 0;
	}


	StrUpr( argv[0] );	
	 if( ! strcmp( argv[0], "1T" ) )
	 {
		fs=1; ls=1;
	 }
	 else  if( ! strcmp( argv[0], "MTF" ) )
	 {
		fs=1; ls=0;
	 }
	 else  if( ! strcmp( argv[0], "MTM" ) )
	 {
		fs=0; ls=0;
	 }
	 else  if( ! strcmp( argv[0], "MTL" ) )
	 {
		fs=0; ls=1;
	 }
	 else  if( ! strcmp( argv[0], "ARP" ) )
	 {
		fs=1; ls=1;
		//ARP protocol
		unsigned char tx_buffer[64]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //0-5 DMAC
							 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  //6-11 SMAC
							 0x08, 0x06, //12-13 ARP
							 0x00, 0x01, //14-15 ETH
							//----------
							 0x08, 0x00, //16-17 IP Protocol
							 0x06,0x04,  //18-19
							 0x00,0x01, //20-21 OPcode request
							 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  //22-27 SMAC
							 0xc0, 0xa8,0x01,0x06,  //28-31  Sender IP:192.168.1.6
							 //-------------
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //32-37 Target MAC
							 0xc0, 0xa8, 0x01, 0x07,  //38-41  Target IP	:192.168.1.7	
							 //0x00,0x04,0x00,0x01,  //checksum
							 
							}; 

		memcpy(DMA_buff ,tx_buffer, 64);

	 }	 


	 else  if( ! strcmp( argv[0], "UDP" ) )
	 {
		fs=1; ls=1;
		
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
			
		memcpy(DMA_buff ,tx_buffer1, 64);
	 }
 	else
 	{
 		fs=1; ls=1;	
 	}

	if(argc>=2)
	len= strtoul((const char*)(argv[1]), (char **)NULL, 16);

	if(argc>=3)
	txinfo1= strtoul((const char*)(argv[2]), (char **)NULL, 16);

	if(argc>=4)
	txinfo2= strtoul((const char*)(argv[3]), (char **)NULL, 16);

	if(argc>=5)
	txinfo3= strtoul((const char*)(argv[4]), (char **)NULL, 16);	

	if(argc>=6)
	offset= strtoul((const char*)(argv[5]), (char **)NULL, 16);	

	DMA_Transmit(DMA_buff, len,fs,ls, txinfo1, txinfo2, txinfo3, offset);


}; 
//----------------------------------------------------------------

int CMD_DMARx(int argc, char* argv[])
{
	unsigned char *pbuff;
	int rxlen;
	int rxoffset=0;
	unsigned int rxinfo1, rxinfo2,rxinfo3,drop;



	if(argc<=0)
	{
		if(DMA_Receive(&pbuff, &rxlen,&rxinfo1, &rxinfo2, &rxinfo3, &drop)==-1)
		{
			DBG_PRINT("no rx pkt, at index=%x\n", SPE_rxtail);
			return -1;
		};	
	
		return 0;
	}


	if(DMA_Receive(&pbuff, &rxlen,&rxinfo1, &rxinfo2, &rxinfo3, &drop)==-1)
	{
		DBG_PRINT("no rx pkt, at index=%x\n", SPE_rxtail);
		return -1;
	};	

	unsigned int txlen=0; 
	unsigned int txinfo1=0, txinfo2=0, txinfo3=0;	
	int txoffset=0;

	rxoffset=(int)pbuff&0x03;

	if(argc>=1)
		StrUpr( argv[0] );	


	if(argc>=2)
	txlen= strtoul((const char*)(argv[1]), (char **)NULL, 16);

	if(argc>=3)
	txinfo1= strtoul((const char*)(argv[2]), (char **)NULL, 16);

	if(argc>=4)
	txinfo2= strtoul((const char*)(argv[3]), (char **)NULL, 16);

	if(argc>=5)
	txinfo3= strtoul((const char*)(argv[4]), (char **)NULL, 16);	

	if(argc>=6)
	txoffset= strtoul((const char*)(argv[5]), (char **)NULL, 16);	
	
	 if( ! strcmp( argv[0], "CHK" ) )
	 {

	 	if(txlen!=rxlen)		{	DBG_PRINT("fail, txlen=%x rxlen=%x \n", txlen, rxlen);at_errcnt++; return -1; }
		if(txoffset!=rxoffset)	{	DBG_PRINT("fail, txoffset=%x rxoffset=%x \n", txoffset, rxoffset);at_errcnt++; return -1; }

		if(txinfo1!=rxinfo1)	{	DBG_PRINT("fail, txinfo1=%x rxinfo1=%x \n", txinfo1, rxinfo1); at_errcnt++;return -1; }
		if(txinfo2!=rxinfo2)	{	DBG_PRINT("fail, txinfo2=%x rxinfo2=%x \n", txinfo1, rxinfo1); at_errcnt++;return -1; }
		if(txinfo3!=rxinfo3)	{	DBG_PRINT("fail, txinfo3=%x rxinfo3=%x \n", txinfo1, rxinfo1); at_errcnt++;return -1; }

#if 0
		int ret=memcmp(DMA_buff,pbuff, txlen) ;
		if(ret !=0  )	
			{	DBG_PRINT("fail, addr=%x txdata=%x rxdata=%x \n", ret, DMA_buff[ret], pbuff[ret]); return -1; }
#endif
		int i;
		for(i=0; i<txlen; i++)
		{
			if(DMA_buff[i]!=pbuff[i]) 		
			{	DBG_PRINT("fail,  txdata[%x]=%x rxdata[%x]=%x \n", i, DMA_buff[i],  i,pbuff[i]); at_errcnt++;return -1; 
			}
		
		}

		DBG_PRINT("Pass\n");
		
	 }
	
	
}; 


//----------------------------------------------------------------
int CMD_PktGen(int argc, char* argv[])
{
	int i;
	if( argc < 4 ) 
	{
		DBG_PRINT("pktgen <mode> <len> <mode> <initval>\n");
		DBG_PRINT("pktgen fix 128 fix 55\n");	
		DBG_PRINT("pktgen fix 128 inc 55\n");	
		DBG_PRINT("pktgen rand[4B] 0 rand 0 \n");			
		return 0;
	}

	unsigned int len=4,val=0;	
	unsigned int mode=0;

	
	StrUpr( argv[0] );		
	 if( ! strcmp( argv[0], "FIX" ) )
	 {	mode=PKTGEN_LenFix;
	 }	 
	 else if( ! strcmp( argv[0], "RAND" ) )
	 {	mode=PKTGEN_LenRand;	
	 }	 
	 else if( ! strcmp( argv[0], "RAND4B" ) )
	 {	mode=PKTGEN_LenRand4B;	
	 }	
	 len = strtoul((const char*)(argv[1]), (char **)NULL, 16);			 
	//
	StrUpr( argv[2] );		
	 if( ! strcmp( argv[2], "FIX" ) )
	 {	mode|=PKTGEN_ValFix;
	 }	 
	 else if( ! strcmp( argv[2], "INC" ) )
	 {	mode|=PKTGEN_ValInc;	
	 }	 
	 else if( ! strcmp( argv[2], "RAND" ) )
	 {	mode|=PKTGEN_ValRand;	
	 }		 
	val = strtoul((const char*)(argv[3]), (char **)NULL, 16);	
	
	len=pktgen(mode, &DMA_buff,len, val);
	DBG_PRINT("Gen len=0x%x bytes \n", len);		


		
}; 

unsigned int pktgen(unsigned int mode, unsigned char *pbuff, unsigned int pklen ,unsigned char initval)
{
	unsigned int len;
	unsigned int val;
		int i;	

	//cal len
	if(mode&PKTGEN_LenFix)
	{	
		len=pklen;
		if(pklen==0)			
			len=(int)random(BUF_SIZE+1);
		
	}
	else if(mode&PKTGEN_LenRand)
	{	len=(int)random(BUF_SIZE+1);
	}
	else if(mode&PKTGEN_LenRand4B)
	{	len=(int)random(pklen+1)&~(3);
	}


	//protect
	if(len==0) 
	{	len++;
		if(mode&PKTGEN_LenRand4B)
			len=4;
	}
	if(len>BUF_SIZE) len=BUF_SIZE;
	DMA_len=len;

	//fill content
	if(mode&PKTGEN_ValInc)
	{	for(i=0;i<len;i++)
		{	pbuff[i]=(unsigned char)(initval+i)&0xff;
		}
	}
	else if(mode&PKTGEN_ValRand)
	{	for(i=0;i<len;i++)
		{	pbuff[i]=(unsigned char)random(256)&0xff;
		}
	}
	else if(mode&PKTGEN_ValFix)
	{	for(i=0;i<len;i++)
		{	pbuff[i]=(unsigned char)(initval)&0xff;
		}
	}	
	return len;
}
//----------------------------------------------------------------
int CMD_DMALoop(int argc, char* argv[])
{

	int i;
	unsigned int   st=0,et=0,timeout=0;
	if( argc < 1 ) 
	{
		DBG_PRINT("dmaloop <mode> <pknum> <pklen> <timeout> <para1>\n");
		DBG_PRINT("dmaloop txseq 8 0\n");		
		DBG_PRINT("dmaloop loop 128 128 10\n");
		DBG_PRINT("dmaloop ism 1 128 10\n");		
		return 0;
	}

	StrUpr( argv[0] );	
	unsigned int pknum,txpklen,wait=3;
	unsigned int para1=0;
	
	if(argc>=2)
		pknum = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	if(argc>=3)	
		txpklen = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	if(argc>=4)	
		wait = strtoul((const char*)(argv[3]), (char **)NULL, 16);		
	if(argc>=5)	
		para1 = strtoul((const char*)(argv[4]), (char **)NULL, 16);		
	
	unsigned int rxpklen=0;

	//------------------------------------------
	DMATEST_TxPktMaxNum=pknum;	
	DMATEST_TxPktLen=txpklen;		

	DMATEST_TxPktNum=0;	
	//-------------------------------------------	
	 if( ! strcmp( argv[0], "INIT" ) )
	 {		
			ClearAllPktCount();			
			return;

	 }
	//-------------------------------------------
	else if( ! strcmp( argv[0], "TXSEQ" ) )
	 {	
	
		DMATEST_FRAG_EN=para1;
		
		DMA_mode=DMAMODE_TXSEQ;
		
		st=get_timer_jiffies();   //ubit 10 msec
		volatile int end;


		DMA_TxPoll();  //using TXDU interrupt to generate tx packets

		while(1)
		{
			if(DMATEST_TxPktMaxNum!=0)
			{
				if(DMATEST_TxPktNum >= DMATEST_TxPktMaxNum)
					break;
			}
			
			
			et=get_timer_jiffies();
			if((et-st)  > wait*100)
			{	timeout=1;
				DBG_PRINT("timeout!, stop at  pktnum=%d\n", DMATEST_TxPktNum);	
				break;
			}
			
		}
		DMA_mode=DMAMODE_DEFAULT;

		if((DMATEST_TxPktNum==0) && (timeout==1))
		{	DBG_PRINT("fail, cannot tx packet out \n");
			at_errcnt++;

		}
		
		DBG_PRINT("End TxPktNum=%x \n", DMATEST_TxPktNum );
		DBG_PRINT("End TxFstByte=%x \n", DMATEST_TXFB&0xff );
		DBG_PRINT("End TxLEN=%x \n", DMATEST_TXLEN );	
		if(DMATEST_FRAG_EN==1)
			DBG_PRINT("End FRAG_SUMLEN=%x \n", DMATEST_FRAG_SUMLEN );	
		
		if(((et-st)/100)!=0)
		DBG_PRINT("throughtput=%d bytes/sec \n", (DMATEST_TxPktNum*DMATEST_TxPktLen)/((et-st)/100) );		
		DMA_mode=DMAMODE_DEFAULT;
		return 0;
	 }

	//-------------------------------------------
	else if( ! strcmp( argv[0], "LOOP" ) )
	{
		DMA_mode=DMAMODE_HOSTTX_LOOP;
		unsigned char *pbuff;
		int failcnt=0;
		st=get_timer_jiffies();

		int end=0;
		int timeout=0;
		int txpkcnt=0;
		while(!end)
		{	

			if(txpklen==0)
				txpklen=pktgen(PLTGEN_AllRand, DMA_buff,0,0);
			else
				//pktgen(PKTGEN_LenFix|PKTGEN_ValRand, DMA_buff,txpklen,0);
				pktgen(PKTGEN_LenFix|PKTGEN_ValFix, DMA_buff,txpklen,txpkcnt+1);		

#ifdef CONFIG_R8198EP_HOST
			while(DMA_Transmit(DMA_buff, txpklen,1,1,    DMAMODE_HOSTTX_LOOP,0,0   ,0)==-1)
#else
			while(DMA_Transmit(DMA_buff, txpklen,1,1,    DMAMODE_DEVICETX_LOOP,0,0   ,0)==-1)
#endif
			{	
				et=get_timer_jiffies();
				if((et-st)>wait*100)
				{	timeout=1;
					DBG_PRINT("Wait tx timeout!, pktnum=%d\n", txpkcnt);
					if(txpkcnt==0)
						at_errcnt++;					
					break;
				}
			}	

			
			while(DMA_Receive(&pbuff, &rxpklen, 0,0,0, 0)==-1)
			{
				et=get_timer_jiffies();
				if((et-st)> wait*100)
				{	timeout=1;
					DBG_PRINT("Wait rx timeout!, pktnum=%d\n", txpkcnt);
					if(txpkcnt==0)
						at_errcnt++;
					break;
				}					
			}	


			et=get_timer_jiffies();
			if((et-st)>wait*100)
			{	timeout=1;
				DBG_PRINT("Test time finish! \n");
				break;
			}

				
			if(timeout==1)			
				break;
			
			//compare
			if(txpklen == rxpklen)
			{				
				//if(memcmp(DMA_buff, pbuff, txpklen)!=0)
				for(i=0; i<txpklen; i++)
					if(DMA_buff[i]!=pbuff[i])				
					{	DBG_PRINT("Fail at txpkcnt=%x, txbuf[%x]=%x, rxbuf[%x]=%x \n",txpkcnt, i,DMA_buff[i], i,pbuff[i]  );
						at_errcnt++;
						failcnt++;
					}
			}
			else
			{
				DBG_PRINT("Fail at txpkcnt=%x, txpklen=%x, rxpklen=%x \n",txpkcnt,txpklen,rxpklen  );
				at_errcnt++;
				failcnt++;
			}


			//check if stop dma test
			txpkcnt++;
			if(pknum!=0)
			{	
				if(txpkcnt >= pknum)
				{	end=1;					
				}
			}
			
				
		}

		DMA_mode=DMAMODE_DEFAULT;	
		DBG_PRINT("Tx scuess PktCnt=%x \n", txpkcnt);	
		DBG_PRINT("Tx spend time=%d sec \n", (et-st)/100);			
		DBG_PRINT("Loop FailCnt=%d \n", failcnt);
		

	} 
	//-------------------------------------------
	else if( ! strcmp( argv[0], "ISM" ) )
	{
		st=get_timer_jiffies();	
		DMA_mode=DMAMODE_ISM;
		DMA_TxPoll();  //using TXDU interrupt to generate tx packets

		delay_sec(wait);
		
		DMA_mode=DMAMODE_DEFAULT;

		
	}
	
}



//----------------------------------------------------------------
int CMD_DbgMsg(int argc, char* argv[]) 
{
	int en=0;
	if( argc < 1 ) 
	{
		DBG_PRINT("dbg <mode>  \n");
		DBG_PRINT("dbg 1: show msg\n");				
		return 0;
	}
	
	if(argc>=1)
		en = strtoul((const char*)(argv[0]), (char **)NULL, 16);		

	if(en==1)
		DBG_PRINT("open dbg msg\n");
	else 
	{	DBG_PRINT("close dbf msg\n");
		en=0;
	}
	
	dbg_msg=en;

}

//----------------------------------------------------------------
int CMD_Wait(int argc, char* argv[])
{
	if( argc < 1 ) 
	{
		DBG_PRINT("wait  t msec.\n");		
		DBG_PRINT("wait  r reg mask expval timeout \n");	
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
//===================================================
#ifdef CONFIG_R8198EP_HOST
//----------------------------------------------------------------
inline void NFBI_WRITE_MEM32(unsigned int addr,unsigned int data)
{
	REG32(SPE_NFBI_ADDR)=addr;
	REG32(SPE_NFBI_DR)=data;
}


inline unsigned int NFBI_READ_MEM32(unsigned int addr)
{	
	REG32(SPE_NFBI_ADDR)=addr;
	unsigned int val=	REG32(SPE_NFBI_DR);
	return val;
}

//--------------------------------------------------------------
void SettingDRAMConfig()
{
	REG32(SPE_NFBI_CMD)=NFBI_CMD_SWAP;	

	DBG_PRINT("Set T/RX dly \n");
	REG32(SPE_NFBI_TRXDLY)=DRAM_TRXDLY_VAL;
	if( REG32(SPE_NFBI_TRXDLY) != DRAM_TRXDLY_VAL)
	{
		at_errcnt++;
		DBG_PRINT("write fail! NFBI_TRXDLY=%x \n", REG32(SPE_NFBI_TRXDLY));
	}


	DBG_PRINT("Set DRAM cfg\r\n");
	REG32(SPE_NFBI_DTR)=DRAM_TIMING_VAL;
	if(REG32(SPE_NFBI_DTR)!= DRAM_TIMING_VAL) 
	{	at_errcnt++;
		DBG_PRINT("write fail! NFBI_DTR=%x \n", REG32(SPE_NFBI_DTR));	
	}
	
	REG32(SPE_NFBI_DCR)= DRAM_CONFIG_VAL;
	if(REG32(SPE_NFBI_DCR)!= DRAM_CONFIG_VAL) 
	{	at_errcnt++;
		DBG_PRINT("writing fail! NFBI_DCR=%x  \n", REG32(SPE_NFBI_DCR));
	}


	if( (REG32(SPE_NFBI_CMD)&NFBI_CMD_DRAMTYPE_DDR) == NFBI_CMD_DRAMTYPE_DDR)
	{
		DBG_PRINT("Do DDR calibration \n");
		DDR_calibration();  //just for DDR
	}

}
//------------------------------------------------------------
void DDR_calibration()
{ 
	#define DDR_DBG 0
	#define IMEM_DDR_CALI_LIMITS 1


	int i,j,k;
   
        int L0 = 0, R0 = 33, L1 = 0, R1 = 33;
 
	// int  DRAM_ADR = 0xa0400000;//0xa0400000 is virtual
        int  DRAM_ADR = 0x400000;//Physical address 
	int  DRAM_VAL = 0x5A5AA5A5;
 
       // int  DDCR_ADR = 0xB8001050;//Virtual
        int  DDCR_ADR = SPE_NFBI_DDCR;//Physical    
        int  DDCR_VAL = 0x80000000; //Digital
        //int  DDCR_VAL = 0x0; //Analog (JSW: N/A for 8198 FPGA)   
  
 
        NFBI_WRITE_MEM32(DRAM_ADR, DRAM_VAL);   
        //while( (NFBI_READ_MEM32(DDCR_ADR)& 0x40000000) != 0x40000000);
       
 
  for(k=1;k<=IMEM_DDR_CALI_LIMITS;k++)  //Calibration times
    //while(1)
    {
  
         // Calibrate for DQS0
         for (i = 1; i <= 31; i++)
         {           
       	EP_REG32_WRITE(DDCR_ADR,  (DDCR_VAL & 0x80000000) | ((i-1) << 25));
		
 		#if DDR_DBG
              __delay(10000);
             //DBG_PRINT("DQS0(i=%d),(DDCR=0x%x) \n", i,EP_REG32_READ(DDCR_ADR));
	 	#endif
		
            if (L0 == 0)
            {              
          		if ((NFBI_READ_MEM32(DRAM_ADR) & 0x00FF00FF) == 0x005A00A5)
			{
				L0 = i;
			}
            }
            else
            {       
		#if DDR_DBG
         	DBG_PRINT("  DRAM(0x%x)=%x\n", DRAM_ADR,NFBI_READ_MEM32(DRAM_ADR));
      		#endif
			
               if ((NFBI_READ_MEM32(DRAM_ADR) & 0x00FF00FF) != 0x005A00A5)
               {
                  //DBG_PRINT("\n\n\nError!DQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_ADR));
			#if DDR_DBG
			DBG_PRINT("  DRAM(0x%x)=%x\n\n\n", DRAM_ADR,NFBI_READ_MEM32(DRAM_ADR));
			#endif
                  R0 = i - 1;
                  //R0 = i - 3;  //JSW
                  break;
               }
            } //end else

		//DBG_PRINT("i=%d L0=%d R0=%d \n",i,L0,R0);
         }
        
	DDCR_VAL = (DDCR_VAL & 0xC0000000) | (((L0 + R0) >> 1) << 25); // ASIC
	DBG_PRINT("DDCR_VAL =%x \n", DDCR_VAL );
	EP_REG32_WRITE(DDCR_ADR,  DDCR_VAL);
 
 	//----------------------------------------------------------------
         // Calibrate for DQS1
         for (i = 1; i <= 31; i++)
         {
        #if DDR_DBG
               __delay(10000);
              DBG_PRINT("DQS1(i=%d),(DDCR=0x%x) \n", i,EP_REG32_READ(DDCR_ADR));
 #endif
        
             
   EP_REG32_WRITE(DDCR_ADR,  (DDCR_VAL & 0xFE000000) | ((i-1) << 20));
 
            if (L1 == 0)
            {
               if ((NFBI_READ_MEM32(DRAM_ADR) & 0xFF00FF00) == 0x5A00A500)
               {
                  L1 = i;
               }
            }
            else
            {
			#if DDR_DBG
			DBG_PRINT("  DRAM(0x%x)=%x \n", DRAM_ADR,NFBI_READ_MEM32(DRAM_ADR));
			#endif
               if ((NFBI_READ_MEM32(DRAM_ADR) & 0xFF00FF00) != 0x5A00A500)
               {
                 //DBG_PRINT("\n\n\nError!DQS1(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
           // DBG_PRINT("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
                  R1 = i - 1;
      //R1 = i - 3;
                  break;
               }
            }
         }
 
         DDCR_VAL = (DDCR_VAL & 0xFE000000) | (((L1 + R1) >> 1) << 20); // ASIC
         DBG_PRINT("\nDDCR_VAL =%x\n", DDCR_VAL );
          EP_REG32_WRITE(DDCR_ADR,  DDCR_VAL);
 
        /* wait a little bit time, necessary */       
       __delay(100);
  #if 1  
		DBG_PRINT("R0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
		DBG_PRINT("R1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
                 
              DBG_PRINT("=>After IMEM Cali,DDCR(%d)=0x%x \n",k ,EP_REG32_READ(DDCR_ADR));
		DBG_PRINT("=================================\n");
 #endif
  
 
     }//end of while(1) 
 
 
 
}






//----------------------------------------------------------------------------------------
void SettingJumpCode(unsigned int saddr, unsigned int daddr)
{

//	int saddr=0x80700000;
//	int daddr=0x80700000;
	int i,off=0;
	unsigned int dramcode[]={
							0x3c085448, 	// lui	t0,0x5448			//0
							0x35080000, 	// ori	t0,t0,0x0000		//1				
							0x3c09b800, 	// lui	t1,0xb800
							0x35291004, 	// ori	t1,t1,0x1004
							0xad280000, 	// sw	t0,0(t1)
							
							0x3c08ffff, 	// lui	t0,0xffff			//5
							0x350805c0, 	// ori	t0,t0,0x5c0		//6
							0x3c09b800, 	// lui	t1,0xb800
							0x35291008, 	// ori	t1,t1,0x1008
							0xad280000, 	// sw	t0,0(t1)


							//	
							0x3c0800ff, 	// lui	t0,0x00ff			//10
							0x3508ffd6, 	// ori	t0,t0,0xffd6		//11
							0x3c09b800, 	// lui	t1,0xb800
							0x35290010, 	// ori	t1,t1,0x0010
							0xad280000, 	// sw	t0,0(t1)
							};

	
	unsigned int jmpcode[]={
							0x3c1aa070, 	//		lui	k0,0x8123 
							0x375a0000, 	// 		ori	k0,k0,0x4567  
							0x03400008, 	//		jr	k0 
							0x0	,		// 		nop
							0x5555aaaa,	// 		magic numbeer
							};

	unsigned int dbgcode[]={
							0x3c080000, 	// lui	t0,0x0000			//0
							0x35080001, 	// ori	t0,t0,0x0001		//1				
							0x3c09b8b4, 	// lui	t1,0xb8b4
							0x3529108C, 	// ori	t1,t1,0x108C
							0xad280000, 	// sw	t0,0(t1)
							0x0			// 		nop
							};	

		//setting remote DCR and DTR register
		dramcode[0]=(dramcode[0] &0xffff0000) | DRAM_CONFIG_VALH;
		dramcode[1]=(dramcode[1] &0xffff0000) | DRAM_CONFIG_VALL;
		dramcode[5]=(dramcode[5] &0xffff0000) | DRAM_TIMING_VALH;
		dramcode[6]=(dramcode[6] &0xffff0000) | DRAM_TIMING_VALL;

		jmpcode[0]=(jmpcode[0] &0xffff0000) | (( (daddr|0xa0000000) & 0xffff0000)>>16);
		jmpcode[1]=(jmpcode[1] &0xffff0000) | ( (daddr|0xa0000000) & 0x0000ffff);			



	//--------------------------------------------	

		DBG_PRINT("Set Reset Jump Code,  from %08x jump to %08x\n", saddr, daddr);
		saddr=saddr&0x0fffffff;

		for(i=0;i<6;i++)
		NFBI_WRITE_MEM32(saddr+(off++)*4, dbgcode[i] );			
		
		for(i=0;i<15;i++)
		NFBI_WRITE_MEM32(saddr+(off++)*4, dramcode[i] );			
		for(i=0;i<5;i++)
		NFBI_WRITE_MEM32(saddr+(off++)*4, jmpcode[i] );	
		

}

//-------------------------------------------------------------------------
int TestCmd_SetJumpCode(int argc, char* argv[])
{
	int saddr=0x80700000;
	int daddr=0x80700000;

	if(argc<1)
	{	DBG_PRINT("sendj 0  , set configuration\n");
		DBG_PRINT("Usage: sendj from_addr to_addr\n");	
		DBG_PRINT("Usage: sendj 00008000 80700000\n");				
		return;
	}
	if(argc ==1 )
	{
		SettingDRAMConfig();	
		return;

	}
	if(argc >=2 )
	{
		saddr= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		daddr= strtoul((const char*)(argv[1]), (char **)NULL, 16);
		SettingDRAMConfig();		
		SettingJumpCode(saddr, daddr);		
	}
	
}
//-----------------------------------------------------------------

int TestCmd_TESTRAM(int argc, char* argv[])
{

	unsigned int daddr,dlen;
	unsigned int i,sidx;
	unsigned int t,v,exp;

	if(argc<1)
	{	
		DBG_PRINT("Usage: tram <remote_addr> <remote_len>, ps: using pkgen first. \n");			
		return;
	}


	if(argc >=2 )
	{
		daddr= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		dlen= strtoul((const char*)(argv[1]), (char **)NULL, 16);
	}

	SettingDRAMConfig();
	DBG_PRINT("\nWriting ram addr=0x%08x, len=0x%08x \n",daddr, dlen);

	REG32(SPE_NFBI_ADDR)=daddr ;
	sidx=0;
	for(i=0;i<dlen;i+=4)
	{	
		sidx=i%DMA_len;
		REG32(SPE_NFBI_DR)=*(unsigned int *)(DMA_buff+sidx);
		
		if( !(i&0xffff))	DBG_PRINT(".");		
	}

	//------------------------------read bootcode back
	if(1)
	{
		DBG_PRINT("\ncode Reading Back\n");
		REG32(SPE_NFBI_ADDR)=daddr;
	
		for(i=0;i<dlen;i+=4)
		{	
			v=REG32(SPE_NFBI_DR);	
			sidx=i%DMA_len;	
			t=*(unsigned int *)(DMA_buff+sidx);
			if(v!=  t)
				DBG_PRINT("Error! at addr=%x value=%x Expect val=%x\n", daddr+i, v, t);		
			if( !(i&0xffff))	DBG_PRINT(".");	
		
		}	
	}	
	//--------------------------------------

	
	return 1;

}
//-----------------------------------------------------------------------------------------


int NFBI_SendCode(unsigned int addr, unsigned char *buff, unsigned int len, unsigned int send,unsigned int verify )
{


	unsigned int i;
	unsigned int t;
	unsigned char t0,t1,t2,t3;
	int remain;
	unsigned int verify_errcnt=0;

	if(send)
	{
	DBG_PRINT("Writing local buff addr=%08x len=%x to remote addr=%08x ...\n",buff,len,addr);
	REG32(SPE_NFBI_ADDR)=addr; 

	for(i=0;i<len;i+=4)
	{		
		//if( !(i&0xfff))	DBG_PRINT(".");	
		REG32(SPE_NFBI_DR)=*(unsigned int *)(buff+i) ; 
		//DBG_PRINT("Write data=%x \n", *(unsigned int *)(buff+i));
	}
	if( (len%4) )
	{
		remain=(len%4);
		DBG_PRINT("bootcode size is not 4 factor \n");
#if 0
		switch(remain)
		{
			case 1:	REG32(0x13,(bootstart[i] <<8)| 0 ); //data 
					REG32(0x14, 0 | 0 );
					break;
			case 2:	REG32(0x13,(bootstart[i] <<8)| bootstart[i+1] ); //data
					REG32(0x14, 0 | 0 );
					break;
				
			case 3:	REG32(0x13,(bootstart[i] <<8)| bootstart[i+1] ); //data
					REG32(0x14, bootstart[i+2]<<8 | 0 );
					break;
		}
#endif
	}
	}
	//------------------------------read bootcode back
	if(verify)
	{
		DBG_PRINT("code Reading Back...\r\n");
		REG32(SPE_NFBI_ADDR)=addr; //address H
		
		for(i=0;i<len;i+=4)
		{	
			t=REG32(SPE_NFBI_DR);
			t0=(unsigned char)(t>>24) ;
			t1=(unsigned char)((t&0x00ff0000)>>16) ;
			t2=(unsigned char)((t&0x0000ff00)>>8);
			t3=(unsigned char)(t&0x0000ff) ;
			
			if( t0 != buff[i+0] )		 
			{	DBG_PRINT("Error! at offset=%x value=%02x Expect val=%02x\r\n", i,t0, buff[i] );
				verify_errcnt++;				
			}
			if( t1  != buff[i+1]  )
			{	DBG_PRINT("Error! at offset=%x value=%02x Expect val=%02x\r\n", i+1, t1, buff[i+1] );				
				verify_errcnt++;
			}
			if(t2 != buff[i+2] )		 
			{	DBG_PRINT("Error! at offset=%x value=%02x Expect val=%02x\r\n", i+2, t2, buff[i+2] );
				verify_errcnt++;
			}
			if(t3 != buff[i+3]  )
			{	DBG_PRINT("Error! at offset=%x value=%02x Expect val=%02x\r\n", i+3,t3, buff[i+3] );		
				verify_errcnt++;
			}
			if(verify_errcnt>32)
				break;
		}	
	}

	if(verify_errcnt!=0)
		at_errcnt++;
	return 1;
}

//----------------------------------------------------------------------------


	


//----------------------------------------------------------------------------
//extern unsigned int file_length_to_server;

int CMD_SendF( int argc, char* argv[] )
{

	unsigned char* bootstart=0x80300000; 
	//unsigned char* bootend=0x80300100;
	unsigned char *kernelstart=0x80400000;
	//unsigned char *kernelend=0x80400100;

	unsigned int kernellen,bootlen;

	unsigned int i;
	unsigned short t;

	//unsigned int bootlen=bootend-bootstart;
	//unsigned int kernellen=kernelend-kernelstart;
	//unsigned int kernellen=file_length_to_server;

	bootstart=boot98;
	bootlen=sizeof(boot98)/sizeof(char);	
	kernelstart=boot98;
	kernellen=sizeof(boot98)/sizeof(char);	

	
	int remain;
	unsigned int retry;
	unsigned int time_start,time_end;	


	
	//s0	
	//DBG_PRINT("1'st file start=0x%x, size=%d \n",bootstart, bootlen);
	DBG_PRINT("2'nd file start=0x%x, size=%d bytes\n",kernelstart,kernellen);



	int verify=0,send=1;
	//DBG_PRINT("argc=%d argv[0]=%x\r\n",argc,*(char *)argv[0]);
	if(argc >=2 )
	{
		send= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		verify= strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	}
	

	//--------------------------------------s2 read NeedBootCode
#if 0	
	DBG_PRINT("Waiting NeedBootCodeBit=1...");
	retry=NFBI_RETRY;
	while(1)
	{
		if(NFBI_MIIRead(NFBI_ISR) & NFBI_INT_NeedBootCode )
		{
			DBG_PRINT("PASS! NeedBootCode=1\r\n");
			break;
		}
		if(--retry ==0)
		{	DBG_PRINT("timeout! Read NeedBootCode bit Error!\r\n");	break;	}	
	}

#else
	DBG_PRINT("NeedBootCodeBit=%x\n", REG32(SPE_DMA_ISR) & SPE_DMA_ISR_NEEDBTCODE);
#endif
	//------------------------------------------s3
	SettingDRAMConfig();

	//--------------------------------------s4 write bootcode
#if 0	
	time_start=get_timer_jiffies();		
	NFBI_SendCode(NFBI_BOOTADDR, bootstart, bootlen, send, verify);
	time_end=get_timer_jiffies();	
	DBG_PRINT("send 1'st file, spend time=%d sec \n", (time_end-time_start)*10/1000); 	
#else
	SettingJumpCode(NFBI_BOOTADDR,  NFBI_KERNADDR);
#endif
	//---------------------------------s5 write kernel	
	
	time_start=get_timer_jiffies();		
	NFBI_SendCode(NFBI_KERNADDR, kernelstart, kernellen, send, verify);
	time_end=get_timer_jiffies();	
	DBG_PRINT("send 2'nd file, spend time=%d sec\n", (time_end-time_start)*10/1000); 	
	
	//----------------------------------
	REG32(SPE_DMA_ISR)|=SPE_DMA_ISR_NEEDBTCODE; //clear needbootcode
	if( (REG32(SPE_DMA_ISR) & SPE_DMA_ISR_NEEDBTCODE)  == SPE_DMA_ISR_NEEDBTCODE)
		DBG_PRINT("Cannot Clear needbootcode bit \n");

	DBG_PRINT("Start to booting \n");	
	REG32(SPE_NFBI_CMD)|=1; //start run boot code
	if( (REG32(SPE_NFBI_CMD) &1) != 1)
		DBG_PRINT("Cannot pull high StartRunBootCode bit \n");



	
}
//---------------------------------------------------------------------------
#endif

#ifdef CONFIG_R8198EP_DEVICE
//----------------------------------------------------------------
void GPIO_isr(void) //PERST
{
		//if(REG32(PABCDDAT& ~(1<<0))
		{
			DBG_PRINT("Got GPIO INT\n");			
			SlvPCIe_Rst(0,"");
			DBG_PRINT("do phy reset finsih\n");
			char *argv[]={"1","2"};
			CMD_HostInit(1,argv);			
		}
		REG32(PABCDISR_REG)=	REG32(PABCDISR_REG); //clear A0

		//init ISM
		REG32(SPE_ISM_BAR)=ISM_buff;
		REG32(SPE_ISM_LR)=ISM_len;			

}
struct irqaction irq_GPIO = {GPIO_isr, (unsigned long)NULL, (unsigned long)GPIO_IRQ_NO,"GPIO", (void *)NULL, (struct irqaction *)NULL};   

void GPIO_INIT(int en)
{

	if(en==1 )
	{
		
		REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG)& (~(0x01<<0) ); //0 is set gpio, bit 1 set gpio
	       REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & ~(0x01<<0);  //0 input, 1 output, set  bit 0 input
		
 		REG32(PABIMR_REG)|=(0x02 <<0); //set A0 rasing interrupt, 0x00 diable, 0x01 falling, 0x02 rasing, 0x03 both
		//REG32(PABCDISR_REG)=	REG32(PABCDISR_REG) | (0x01 <<0x00); //clear A0

		int irraddr=IRR_REG+GPIO_IRR_NO*4;		
	  	REG32(irraddr) = (REG32(irraddr) &~(0x0f<<GPIO_IRR_OFFSET)) | (5<<GPIO_IRR_OFFSET);	
		request_IRQ(GPIO_IRQ_NO, &irq_GPIO, NULL); 
		
		DBG_PRINT("Enable Slave PCIE GPIO ISR\r\n");		
	
	}
	else if(en==0)
		free_IRQ(GPIO_IRQ_NO);	






}
#endif
//----------------------------------------------------------------


