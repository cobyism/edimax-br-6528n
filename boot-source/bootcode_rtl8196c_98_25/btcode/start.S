#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include "start.h"
#include "../autoconf.h"

		.text
		.set noreorder
		.globl __start
__start:
		j 	load_boot
		nop              
//--------------------------------------------------------------------------------              
 load_boot:   
 		move 	t0, zero
		mtc0 	t0, $12

		nop
#if defined (CONFIG_RTL8196C_REVISION_B)
         #define CLK_MANAGE     0xb8000010
       	REG32_ANDOR(CLK_MANAGE, 0xffffffff,   (1<<11) );        //enable active_pcie0
        REG32_W(0xb8000050, 0xb);  //port 0 mdio reset
        REG32_W(0xb8b01000, (0x0a00<<16) | (0x0f<<8) | (1) );  //enable REV_B setting feature
       	REG32_ANDOR(CLK_MANAGE, 0xffffffff- (1<<11), (0<<11));        //disable active_pcie0
#endif

#ifdef CONFIG_BOOT_TIME_MEASURE

#define CP3_MONCNT0LO $8
#define CP3_MONCNT0HI $9
#define CP3_CONTROL0 $0

		la	t1, 0x80000000
		mtc0	t1, CP0_STATUS
		nop
		nop
		ctc3	zero, CP3_CONTROL0
		mtc3	zero, CP3_MONCNT0LO
		mtc3	zero, CP3_MONCNT0HI
  #if defined(__m5280) || defined(__m5181) || defined(__m4181)
		la	t1, 0x10
  #else
		la	t1, 0x8
  #endif
		ctc3	t1, CP3_CONTROL0
#endif


//-------------------------------------------------------------------------------- 
//============================================================================
		jal uart_show    //show boot msg
		nop
//============================================================================

//------------------------
		jal Setting_Mem_Controller
		nop
		//----------------------------TRX DELAY
#if 1	
#ifdef CONFIG_RTL8198
			
	#if defined(CONFIG_DDR1_SDRAM)
		li      t6,0xb8000000
        	lw      t7,0(t6)
	        nop
        	nop
	        and     t7,t7,0x01
	        bnez    t7,rev_b

	#ifdef CONFIG_RTL89xxC
		REG32_W(0xb8000010, 0x01FFFCEF);  //98 DDR
	#else
		#ifndef CONFIG_A4S12D40ETPG5
		REG32_W(0xb8000010, 0x01FFFF4F);   //98,trxdly value[3,16] DDR  200MHZ ,old 8198	
		#endif
	#endif
		nop
		nop
rev_b:
		//REG32_ANDOR(0xb8000048, 0xffffffff,   (3<<22) );        //Set pad_control [23:22]=3,Memory clock driving to maximun
		REG32_W(0xb8000048, 0x0ECADFFF);   //anson final

	#ifdef CONFIG_RTL89xxC
		REG32_W(0xb8000010, 0x01FFFD2E);   //89xxC
	#else
		#ifdef CONFIG_A4S12D40ETPG5
		REG32_W(0xb8000010, 0x017FFDAA);   //Edimax A4S12D40ETP-G5
		#else
		REG32_W(0xb8000010, 0x01FFFD2E);   //98,trxdly value[9,14] DDR 125/168/200 MHZ OK ,new 8198 ,demo Board OK
		#endif
	#endif
					//REG32_ANDOR(0xb8000048, 0xffffffff, (1<<24)|(1<<25)|(1<<26));//set bigger current for SDRAM
	#elif defined(CONFIG_DDR2_SDRAM)
		REG32_ANDOR(0xb8000048, 0xffffffff,   (3<<22) );        //Set pad_control [23:22]=3,Memory clock driving to maximun
		REG32_W(0xb8000010, 0x01FFFD2E);   //98,trxdly value[9,14] DDR2 125/168/200 MHZ OK ,new 8198
		REG32_W(0xb8000048, 0x004ADFBF);   //anson DRAM=168MHZ final		
		REG32_W(0xb8000038, 0x1F94A104);   //anson 1.8v=>1.816V(3'd3),DRAM=168MHZ final
		#if defined(CONFIG_D128_16)
		//REG32_W(0xb800100c, 0x78004002); 
		REG32_W(0xb800100c, 0x78004402);   // disable #DQS in DDR2 EMRS(1) mode
		#else
		//REG32_W(0xb800100c, 0x38004002);   //anson DDR to 98 driving down,DRAM=168MHZ final
		REG32_W(0xb800100c, 0x38004402);   // disable #DQS in DDR2 EMRS(1) mode
		#endif
	#else
		li      t6,0xb8000000
                lw      t7,0(t6)
                nop
                nop
                and     t7,t7,0x01
                bnez    t7,rev_b_1
		nop
		nop
		REG32_W(0xb8000010, 0x01FFFF8A);   //98,trxdly value[28,10] SDR 166MHZ
rev_b_1:
		REG32_W(0xb8000010, 0x01FFFD2A);    //(tx,rx)=(9,10)
		REG32_W(0xb8000008, 0xFF00D075);   //anson sdram clock = 156.25MHz
	#endif
//EDX BSP change LED scan mode
	#ifdef CONFIG_EDX_LED_MOD3
		REG32_W(0xbb804300, 0x000fff00);
	#else
		REG32_W(0xbb804300, 0x00055500);
	#endif
//EDX BSP end
#else
	#ifdef CONFIG_RTL8196C_REVISION_B
		REG32_W(0xb8000010, 0x00245);  //96C for (TxDly,RxDly):(4,5)
 
                #define PAD_CONTROL 0xb8000048
                li      t6,0xb8000008
                li      t1,0x00100000
                lw      t7,0(t6)
                nop
                nop
                and     t7,t7,0x00100000
                bnez    t7,router_set
        ap_set:
                REG32_ANDOR(PAD_CONTROL, 0xffffffff-(1<<6|1<<19), 1<<7);   //set dis_ssc_norm=0, open ssc spreding
                j       rf_test_end
        router_set:
                REG32_ANDOR(PAD_CONTROL, 0xffffffff-(1<<6), 1<<7);   //set dis_ssc_norm=0, open ssc spreding
        rf_test_end:

	#else
		REG32_W(0xb8000010, 0x00279);  //96C for (TxDly,RxDly):(7,9)
	#endif
		
		REG32_ANDOR(0xb8000040,0xffffffff-(3<<18), (1<<18) );  //turn on cke
		
	#ifndef CONFIG_USING_JTAG		
		REG32_ANDOR(0xb8000040,0xffffffff-(3<<20), (3<<20) );  //switch jtag to gpio
	#endif	
	
#endif
#else
		jal Auto_adjust_trxdly
		nop
#endif		

 /*JSW: 8198 DDR2 CKE must do this WatchDog reset procedure*/
//--------------------------------------------------------------------------------
#if defined(CONFIG_DDR2_SDRAM)
//20101231,set MPMR = DRAM power down mode		
		//UART_PRINT_DELAY(MPMR_msg);   //Show watchdog_msg test message for CKE function
		//REG32_W(0xb8001040, 0x7FFFFF80);     //3.1 MPMR,set TRSD[29:20]=3  ,Fail
	     
		REG32_W(0xb8001040, 0x003FFF80);     //3.1 MPMR,set TRSD[29:20]=3  ,OK
		REG32_W(0xb8001040, 0x403FFF80);     //3.2 MPMR,Enable CKE Power down mode  ,OK
//		REG32_ANDOR(0xb8001040,0xffffffff-(3<<30), (1<<30) );
		nop 		  
		
		REG32_W(0xb8003118, 0xe0000);     //Set CDBR (About 0.2 secs) , OK
//		REG32_W(0xb8003118, 0x20000000);     //Set CDBR (About 2 secs) , fail	    
		li      t6,0xb8000008
		lw      t7,0(t6)
		nop
		nop
		and     t7,t7,0x00800000   //bit[23]== 1 if watchdog ever reboot
         	
		bnez     t7,SKIP_CKE_WatchDog //t7==1 ,WD happened(skip WD) ; t7==0 ,WD never happened (do WD)
		nop			

		UART_PRINT(UART_THR, boot_msg);	
		nop
//		UART_PRINT_DELAY(watchdog_msg);   //Show watchdog_msg test message for CKE function
		REG32_ANDOR(0xb800311c, 0x00ffffff, 0);      //WD start,set [31:24] to not "A5"
			                 
SKIP_CKE_WatchDog:	 
//		REG32_W(0xb8001040, 0x7FFFFF80);     //Timing default value
//		REG32_ANDOR(0xb800311c, 0x00ffffff, 0xa5000000);      //WD stop,set [31:24] to  "A5"
#endif

//--------------------------------------------------------------------------------
#if defined(CONFIG_DDR_SDRAM) && !defined(CONFIG_NFBI) 
		jal DDR_Auto_Calibration
		nop
		nop
#endif			
//--------------------------------------------------------------------------------
//for NFBI test
         	//li     t0, 0x55aaFFFF 
 		//li     t1, 0xb8003104
 	      	//sw     t0, 0(t1)

#if 0  //RESET test 	      	
		li     t0, 0x042AAA80  //set PCM
 		li     t1, 0xb8000030
 	      	sw     t0, 0(t1)

		li     t0, 0x002AAA80  //set RESETn
 		li     t1, 0xb8000030
 	      	sw     t0, 0(t1) 
#endif 
//--------------------------------------------------------------------------------
#ifdef CONFIG_RTL8196C
// ################  JSW@20091013 :For 8196C FCS1=CKE  ######################

	li     t0, 0x00040000    // For 8196C
	
	li     t1, 0xb8000040
        sw     t0, 0(t1)
	nop
	nop 
 	REG32_W(0xb8003500, 0xFFFF7F86);  //96C
	nop
	nop
	REG32_W(0xb8003508, 0x00000050);  //96C
        nop
        nop
	REG32_W(0xb800350C, 0x00000040);  //96C
        nop
        nop    
// ################  For 8196C FCS1=CKE end ######################
#endif

//timlee
#ifdef CONFIG_R89xxC_IOH
	REG32_W(0xb8000054, 0xb);  //port 1 phy reset

	//REG32_W(0xb8b21000, (0xd087<<16) | (0x00<<8) | (1) );  //old 1
	REG32_W(0xb8b21000, (0x1087<<16) | (0x00<<8) | (1) );  //bokai 1
	
	//REG32_W(0xb8b21000, (0x4d18<<16) | (0x02<<8) | (1) );  //old 2	
	REG32_W(0xb8b21000, (0x2d16<<16) | (0x02<<8) | (1) ); //bokai 2

//	REG32_W(0xb8b21000, (0xf448<<16) | (6<<8) | (1) );  //eat 25MHz	
	REG32_W(0xb8b21000, (0xf048<<16) | (0x06<<8) | (1) );    //eat 100MHz
	REG32_W(0xb8b21000, (0x18d5<<16) | (0x08<<8) | (1) );  	
	REG32_W(0xb8b21000, (0x531c<<16) | (0x09<<8) | (1) );  
	//REG32_W(0xb8b21000, (0x00f1<<16) | (0x0a<<8) | (1) );    //try	
	REG32_W(0xb8b21000, (0x1766<<16) | (0x0d<<8) | (1) );  	
	REG32_W(0xb8b21000, (0x0010<<16) | (0x0f<<8) | (1) );  
	
	//REG32_W(0xb8b21000, (0xFD20<<16) | (0x19<<8) | (1) );  		//old 3
	REG32_W(0xb8b21000, (0xFC60<<16) | (0x19<<8) | (1) );    //bokai 3 
	
	REG32_W(0xb8b21000, (0xc280<<16) | (0x1e<<8) | (1) );  

	REG32_ANDOR(0xb8000048,0xffffffff-(1<<27), 0);   //switch to pcie port 1 to end point

//---------
//#if 0  //intel board cannot get INTA interrupt when booting
#if 1
	REG32_W(UART_THR, 'T'<<24);	
getlink:
	REG32_R(0xb8b40728, t1);
	and t0,t1,0x1f
	bne t0,0x11,getlink
	nop
	REG32_ANDOR(0xb8b40004,0xffffffff, (1<<10));  //disable IRQ
#endif	
//-----------	

	REG32_W(UART_THR, 'W'<<24);	
1:

	REG32_R(0xb8b41090, t0);     //wait host write trxdly
	beq t0,0, 1b	
	nop

	REG32_W(UART_THR, 'O'<<24);	
	REG32_W(UART_THR, 'K'<<24);	

	REG32_ANDOR(0xb8b40004, 0xffffffff-(1<<10), 0x0);  //enable IRQ
#endif
//--------------------------------------------------------------------------------        

// flush all cache
		mtc0	zero, $20	//cache control register:  *(20)=0x00
		nop
		nop
		li		t0, 0x3
		mtc0	t0, $20		//cache control register:  *(20)=0x03
		nop
		nop
		mtc0	zero, $20	//cache control register:  *(20)=0x00

//--------------------------------------------------------------------------------		
// Load to 80100000 for compress
// Load to 80000000 for non-compress

//#define __boot_start 0xbfc00000
//#define __boot_end   0xbfcd0000 
//#define BOOT_ADDR    0x80100000

		la		k0, __boot_start
		la		k1, (__boot_end + 4)
		la		t1, BOOT_ADDR
1:
		lw		t0, 0(k0)
		nop

		sw		t0, 0(t1)	//*(t1)=*(t0)   t1+=4;  t0+=4;   t1=BOOT_ADDR, t0=__boot_start, run loop
		nop

		addu	t1, 4
		
		addu	k0, 4
		
		bne		k1, k0, 1b
		nop
		nop
#if defined(CONFIG_DDR2_SDRAM)		
		REG32_W(0xb8001040, 0x7FFFFF80);     //Timing default value JSW
#endif

//--------------------------------------------------------------------------------				
//		Jump to booting	
		li		k0, BOOT_ADDR
		jr		k0
		nop 
              //all END              
//--------------------------------------------------------------------------------
//======================================================================

	EXPORT(Setting_Mem_Controller)
	
                //-------------------------DTR parameter setting
#if defined(CONFIG_DDR1_SDRAM)
//		REG32_W(0xb8001008, 0x6CED8480);  //For DDR1 V58C2256164S"H"I5
//		REG32_W(0xb8001008, 0x6CED9480);  //DDR USE close timming spec.
		#ifdef CONFIG_A4S12D40ETPG5
		REG32_W(0xb8001008, 0x48EDA480);  //DDR USE close timming spec.Edimax A4S12D340ETP-G5
		#else
		REG32_W(0xb8001008, 0x6CED7480);  //EDX DDR USE close timming spec.for Zentel A3S56D40FTP-G5
		#endif
		//REG32_W(0xb8001008, 0xFFFF05C0);  //safe parameter
#elif defined(CONFIG_DDR2_SDRAM)
		//REG32_W(0xb8001008, 0xFFFF05C0);  //FPGA safe parameter 
		nop
		nop
		li	t6,0xb8000008
		lw	t7,0(t6)
		nop
		nop
		and	t7,t7,0x1000
		bnez     t7,clk_162
		nop
		nop
//		REG32_W(0xb8001008,0x490DA4C0);  //32MB DDR2-->181Mhz
//		REG32_W(0xb8001008,0x6D1594C0);  //64MB DDR2-->168Mhz anson
		REG32_W(0xb8001008,0x4913A4C0);  //64MB DDR2-->181Mhz anson 
//		REG32_W(0xb8001008,0xFFFF05C0);  //64MB DDR2-->Save 
		
		j clk_end
		nop
		nop
		
clk_162:
                REG32_W(0xb8001008,0x6D1594C0);  //64MB DDR2
		
		//REG32_W(0xb8001008,0x490EA4C0);  //64MB -->187Mhz			
clk_end:
		nop
                nop
#else  // CONFIG_SDRAM
		REG32_W(0xb8001008, 0x6cca0c80); //JSW@20091106:For RTL8196C SDR 156MHZ
		//REG32_W(0xb8001008, 0x48EA0C80);  //new tune for jason RAM=148MHz
		//REG32_W(0xb8001008, 0xFFFF05C0);  //safe parameter
#endif

#if defined(CONFIG_DDR_SDRAM)
                li      t6,0xb8000000
                lw      t7,0(t6)
                nop
                nop
                and     t7,t7,0x01
                bnez    t7,rev_b_2
                nop
                nop

		REG32_W(0xb8000048, 0x0ECADFBF);//set driving for DDR
		REG32_W(0xb8000010, 0x800);//set to default
rev_b_2:
		nop
		nop
#endif

              //-------------------------DCR parameter settiing
#if defined(CONFIG_D8_16)
		REG32_W(0xb8001004, 0x52080000);          	
#elif defined(CONFIG_D16_16)
 		REG32_W(0xb8001004, 0x52480000); 
#elif defined(CONFIG_D32_16)
 		REG32_W(0xb8001004, 0x54480000); 
#elif defined(CONFIG_D64_16)
		REG32_W(0xb8001004, 0x54880000); 
#elif defined(CONFIG_D128_16)
		REG32_W(0xb8001004, 0x54880000);
		REG32_ANDOR(0xb800100C, 0xffffffff, (1<<30)); //EDTCR(0xB800100c) for DDR2 BA0~BA2( 128MBx1) bit[30]=1 
#elif defined(CONFIG_D8_16x2)
 		REG32_W(0xb8001004, 0x5a080000);  //8MBx2
#elif defined(CONFIG_D16_16x2)
 		REG32_W(0xb8001004, 0x5a480000);  //16MBx2
#elif defined(CONFIG_D32_16x2)
 		REG32_W(0xb8001004, 0x5c480000);  //32MBx2
#elif defined(CONFIG_D64_16x2)
		REG32_W(0xb8001004, 0x5c880000); //64MBx2
#endif
   

		jr ra
		nop
	    

//======================================================================
#if defined (CONFIG_DDR_SDRAM)
	EXPORT(DDR_Auto_Calibration)	
DDR_Auto_Calibration:
################DDR DDCR calibration######################

		  //Pre-setting if you know target DRAM chip's DDCR
	         // li     t0,  0x48400000  # mem timing configuration Reg1 init
 		  // li     t1, 0xb8001050
 	         // sw     t0, 0(t1)  
			
			//lock bus 
#ifdef CONFIG_RTL8198
                li      t6,0xb8000000
                lw      t7,0(t6)
                nop
                nop
                and     t7,t7,0x01
                bnez    t7,rev_b_3
                nop
                nop

		REG32_W(0xb8000014, 0x21c);  //bist controller   // hold LX0/1/2 Bus    //bit [4:2]=1 , [bit9]bist_retn=1
rev_b_3:
		nop
		nop
#else
			REG32_W(0xb8000014, 0x1c);  //bist controller   // hold LX0/1/2 Bus    //bit [4:2]=1
#endif
		################ DDR DDCR calibration assembly code start ######################
        	   			

			li	t3,0xa0000000    	//(t3)=dram test address		                      
			li	v0,0x5a5aa5a5      	//(v0)=dram test pattern
			li	t2,0xb8001050        //(t2)=DDR calibration register                       

			li	t1,0x80000000 //digital delay line(SW)     //(t1)=DDCR_Pattern
			  // lui		t1,0x0     //analog delay line(HW)    

			li	a2,0             //L0
			li	t6,33        	//R0
			
			li	t5,0                //L1
			li	t7,33              //R1
			
			sw	v0,0(t3)        // DRAM_ADDR=0x5a5aa5a5                             

			li	v1,1           	        //v1=1,Calibration for DQS0 ,v1 will run 1~32 times   //(v1)=DQS0
			move	a0,t1              //(a0)=DDCR_Pattern        
			
			li	t4,0x00ff00ff          // (t4)=mask
		   		                
			li	t0,0x005a00a5            //(t0)=expect valeue		              
		       	             
			move	a3,t4                         
			move	a1,t0                             
			addiu	v0,v1,-1                    

	lab_loop_DQS0:
			sll	v0,v1,25     // tmp=(DQS0<< DQS0_offset)                   
			or	v0,a0,v0                 //tmp=DDCR_Pattern | tmp
			
			bnez	a2,lab_notzero      //if(L0==0), continue.
			sw	v0,0(t2)               //DDCR=tmp     

	lab_zero: //wei add, just for easy read		
			lw	v0,0(t3)         //check read, (v0)= val                      
			nop                                        
			and	v0,v0,t4                       // val= val &mask        
			bne	v0,t0,lab_next_DQS0    // compare val and exp_value
			nop           
			
			j lab_next_DQS0             
			move	a2,v1    //save to L0

	lab_notzero:                           
			lw	v0,0(t3)                                
			nop                                        
			and	v0,v0,a3               // val= val &mask         
			bne	v0,a1,lab_end_DQS0     
			nop     

	lab_next_DQS0:                                   
			addiu	v1,v1,1          //V1++  , DQS0++                  
			sltiu	v0,v1,33                    
			bnez	 v0,lab_loop_DQS0      

       lab_end_DQS0:   
			  
			addiu	t8,v1,-1 	//saved R0's value to t8
#ifdef CONFIG_RTL8198
			lui	v0,0x4000 //analog (HW)   
#else
			li	v0,0xc0000000 //digital (SW)    
#endif
			and	t1,t1,v0                               
			//addu	v0,a2,t6   
			
			add	v0,t8,a2   	//(L0+R0)			
			srl	v0,v0,0x1      //c0=(L0+R0)/2
	                li      t6,0xb8000000
	                lw      t7,0(t6)
        	        nop
                	nop
	                and     t7,t7,0x01
        	        bnez    t7,DDCR_SHIFT_EXIT
                	nop
                	nop

//if a2(L0) not equal to 1,then go to DDCR_SHIFT_EXIT without shift
			bne  a2,1,DDCR_SHIFT_EXIT
			nop
			nop
#if 1//def CONFIG_RTL8198 //shift c0 anson@SD8 advise
			li	s1, -3
			li	s2, -2
			li	s3, -1
			sltiu	s0, v0, 9
			movn	s1, s2, s0
			sltiu	s0, v0, 6
			movn	s1, s3, s0
			sltiu	s0, v0, 5
			movn	s1, zero, s0
			addu	v0, v0, s1
#endif
DDCR_SHIFT_EXIT:
			move t9,v0	  //command t9 if no set DQS1	
			                              
			sll	v0,v0,25       //DQS0 Offset
			sll 	t9,t9,20  //command t9 if no set DQS1	//DQS1 Offset
			or	t1,t1,v0
			or    t1,t1,t9		  //command t9(and DDR_Calibration_end) if no set DQS1	
			
			sw	t1,0(t2)      //set DDCR DQS0/DQS1   //DDCR=(0xc0000000) | (C0<<25) | (C0<<20)
			nop
			nop

			 j DDR_Calibration_end //end of set DDCR if don't try DQS1 
			nop


			//Start of DQS1, Not need to try if DQ8~15's length is equal to DQ0~7 )		
			
			//move t6,v0    			
			//lui	t4,0xff00              
			//ori	t4,t4,0xff00      	

			//li	v1,1                
			
			//	lui	 v0,0x0000   //analog delay line(HW)    
			//and	a0,t1,v0                               
			//lui	t0,0xff00                              
			//ori	t0,t0,0xff00     
			
			//lui	a3,0x5a00                              
			//ori	a3,a3,0xa500      
			
			//move	a2,t0                                
			//move	a1,a3     

	//func_start_off_0xf8:   
			//addiu	v0,v1,-1
			//sll	v0,v0,0x14       
			
			//or	v0,t6,v0           //t6 keep DQS0's tap                    
			//bnez 	t5,func_start_off_0x124     
			//sw	v0,0(t2)    
			//nop                                        
			        
			//move	v0,a1   
			//bne	v0,a3,func_start_off_0x1f9c    
			//nop                                        
			
			//move	t5,v1  

	//func_start_off_0x124:                               
			//lw	v0,0(t3)                               
			//nop                                        
			//and	v0,v0,a2                               
			//bne	v0,a1,func_start_off_0x1f9c  
			//nop
			//nop
			//beq	v0,a1,func_start_off_0xf8 
			//addiu	v1,v1,1     
			//nop
			

	//Set DQS1 
	//func_start_off_0x1f9c:   		
			   
			//addiu	t7,v1,-1    
			//addu	v0,t5,t7                             
			//srl	v0,v0,0x1   
			
			//sll	 v0,v0,0x14     
			//move t9,zero
			//or	 t9,t6,v0  			
			
			//sw	t9,0(t2)      //set DDCR DQS0/DQS1   
			//nop
	############### DDR DDCR calibration assembly code end  #####################	
			
	
	DDR_Calibration_end:  	

		//un-lock bus
#ifdef CONFIG_RTL8198
		REG32_W(0xb8000014, 0x200);   //bist controller register   //enable LX0/1/2 Bus, [bit9]bist_retn=1
#else
		REG32_W(0xb8000014, 0);   //bist controller register   //enable LX0/1/2 Bus
#endif
		jr ra
		nop

#endif
//======================================================================

	LEAF(Auto_adjust_trxdly)
Auto_adjust_trxdly:

#define TXDLY_OFFSET 3
#define RXDLY_OFFSET 0
#define TXDLY_MAX 7
#define RXDLY_MAX 7

#define CLKMGR_ADR 0xb8000010
#define SDRAM_ADR 0xa0301234


	//register.
	//clk_mgr [5:3]=txdly   [2:0]=rxdly
	
	//operation register
	// (k1,k0)= (txldy, rxdly),  (7-0,7-0)
	
	//save boundary region
	//(a1,a0)=Rxdly OK(max,min)
	//(a3,a2)=Txdly OK(max.min)
	li  k0,0     
	li  k1,0     

	//assume all not good.
	li a0,RXDLY_MAX
	li a1,0
	li a2,TXDLY_MAX
	li a3,0


looptx:

	bgt k1,TXDLY_MAX,end_cali     //if (rxdly != 0) goto decrx
	nop
	nop

		li k0,0  //re-scan rxdly
	looprx:
		bgt k0,RXDLY_MAX,next_tx     //if (rxdly != 0) goto decrx
		nop
		nop
	
	settrx:
		sll t1,k1,TXDLY_OFFSET   
		sll t0,k0,RXDLY_OFFSET	
		or t1,t1,t0    //  t1= (K1<<3) | (k0<<0)
			
		li t0,CLKMGR_ADR     //clk_mgr
		lw t7,0(t0)
		nop
		sw t1,0(t0)
		nop


		//---------------mem access
		li t0,SDRAM_ADR    //memr
		li t1,0x00000000
		sw t1,0(t0)
		nop
		nop

		lw t2,0(t0)
		nop
		nop

		bne t2,t1,memfail
		nop
		nop

		//---------------
		li t1,0xffffffff
		sw t1,0(t0)
		nop
		nop

		lw t2,0(t0)
		nop
		nop

		bne t2,t1,memfail
		nop
		nop
		//---------------
		li t1,0x55555555
		sw t1,0(t0)
		nop
		nop

		lw t2,0(t0)
		nop
		nop

		bne t2,t1,memfail
		nop
		nop
		//---------------
		li t1,0xaaaaaaaa
		sw t1,0(t0)
		nop
		nop

		lw t2,0(t0)
		nop
		nop

		bne t2,t1,memfail
		nop
		nop
		//---------------
		li t1,0x11223344
		sw t1,0(t0)
		nop
		nop

		lw t2,0(t0)
		nop
		nop

		bne t2,t1,memfail
		nop
		nop
		//---------------
		li t1,0x55667788
		sw t1,0(t0)
		nop
		nop

		lw t2,0(t0)
		nop
		nop

		bne t2,t1,memfail
		nop
		nop
		//---------------memory test ok, then save boundary

		blt k0,a0,save_rxdly_left
		nop
		nop
		b 1f
		nop
		nop
		
	save_rxdly_left:
		move a0,k0

	1:
		
		bgt k0,a1,save_rxdly_right
		nop
		nop
		b 1f
		nop
		nop
	save_rxdly_right:
		move a1,k0

	1:
		
		blt k1,a2,save_txdly_left
		nop
		nop	
		b 1f
		nop
		nop		
	save_txdly_left:
		move a2,k1
	1:
		
		bgt k1,a3,save_txdly_right
		nop
		nop
		b 1f
		nop
		nop
	save_txdly_right:
		move a3,k1

	1:	
		b next_rx
		nop
		nop
	//--------------------




	//-------------------
	memfail:
	next_rx:
		add k0,k0,1	//txdly++
		b looprx
		nop
		nop
	
next_tx:
	add k1,k1,1	//rxdly++
	b looptx
	nop
	nop
	


end_cali:
	add k0,a0,a1    //k0=(a0+a1)/2
	srl k0,k0,1
	add k0,k0,1  //k0++
	add k1,a2,a3  //k1=(a2+a3)/2
	srl k1,k1,1

	sll k0,k0,RXDLY_OFFSET
	sll k1,k1,TXDLY_OFFSET
	or t1,k1,k0
	and t7,t7,0xffffffc0   //mask clk mgr register
	or t1,t1,t7


	li t0,CLKMGR_ADR     //clk_mgr
	sw t1,0(t0)
	nop

	jr ra
	nop
	END(Auto_adjust_trxdly)
//==========================================================================	


	EXPORT(uart_show)
uart_show:	

	//uart_init:		
  	REG32_W( UART_LCR,0x03000000);
  		
  	REG32_W( UART_FCR,0xc7000000);
  	REG32_W( UART_IER,0x00000000);
  	dl = (SYS_CLK_RATE /16)/BAUD_RATE-1;

  	dll = dl & 0xff;
  	dlm = dl / 0x100;
  	REG32_W( UART_LCR,0x83000000);
  	REG32_W( UART_DLL,dll*0x1000000);
   	REG32_W( UART_DLM,dlm*0x1000000); 
	REG32_W( UART_LCR,0x83000000& 0x7fffffff);

/*   
	//uart_print: 
	REG32_W(UART_THR, 'B'<<24);
	REG32_W(UART_THR, 'o'<<24);
	REG32_W(UART_THR, 'o'<<24);
	REG32_W(UART_THR, 't'<<24);	
*/
//	UART_PRINT(UART_THR, boot_msg);
	j ra 
 	nop

boot_msg:	.asciiz "\r\nBooting...\r\n"	 
