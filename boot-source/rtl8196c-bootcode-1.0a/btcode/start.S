#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include "start.h"
#include "../autoconf.h"



#define USING_RTL8196B_MEMCTRL 1
#define REG32_ANDOR(addr,andV,orV)       li t7,addr;\
                                         lw t6, 0(t7);\
                                         and t6,t6,andV;\
                                         or t6,t6,orV;\
                                         sw t6, 0(t7);\
                                         nop;\
					 nop;


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
		//jal uart_show    //show boot msg
		//nop
//============================================================================
#if USING_RTL8196B_MEMCTRL
		jal Setting_Mem_Controller
		nop
		//----------------------------TRX DELAY
#if 1	
#ifndef CONFIG_RTL8196C
  #ifndef CONFIG_RTL8198
		REG32_W(0xb8000010, 0x00FFFFD6);  //96B
		//REG32_W(0xb8000010, 0x004FE7CB);    //cvs code
  #else
		//REG32_W(0xb8000010, 0x5f);  //98
		REG32_W(0xb8000010, 0x01FFFCEF);  //98
  #endif
#else
		#ifdef CONFIG_RTL8196C_REVISION_B
			#ifdef CONFIG_EDX_TRX_DELAY_45
				REG32_W(0xb8000010, 0x00245);  //96C for (TxDly,RxDly):(4,5)
			#elif CONFIG_EDX_TRX_DELAY_46
				REG32_W(0xb8000010, 0x00246);  //96C for (TxDly,RxDly):(4,6)
			#elif CONFIG_EDX_TRX_DELAY_47
				REG32_W(0xb8000010, 0x00247);  //96C for (TxDly,RxDly):(4,7)
			#elif CONFIG_EDX_TRX_DELAY_38
				REG32_W(0xb8000010, 0x00238);  //96C for (TxDly,RxDly):(4,7)
			#else
				REG32_W(0xb8000010, 0x00248);  //96C for (TxDly,RxDly):(4,8) EDX Jeff for Nanya SDRAM
			#endif
		#define PAD_CONTROL 0xb8000048
		li	t6,0xb8000008
		li	t1,0x00100000
		lw	t7,0(t6)
		nop
		nop
		and	t7,t7,0x00100000
		bnez	t7,router_set
	ap_set:
		REG32_ANDOR(PAD_CONTROL, 0xffffffff-(1<<6|1<<19), 1<<7);   //set dis_ssc_norm=0, open ssc spreding
		j	rf_test_end
	router_set:
		#ifdef CONFIG_EDX_DRAM_SPREADING_OFF
			REG32_ANDOR(PAD_CONTROL, 0xffffffff-(1<<6), 1<<7);   //set dis_ssc_norm=0, open ssc spreding
		#else
			REG32_ANDOR(PAD_CONTROL, 0xffffffff-(1<<6|1<<19), 1<<7);   //set dis_ssc_norm=0, open ssc spreding
		#endif
	rf_test_end:




		

		#enidif
		#else
		REG32_W(0xb8000010, 0x00279);  //96C for (TxDly,RxDly):(7,9)
		#endif
		REG32_W(0xb8000040, 0x00340000);  //96C
	
#endif
#else
		jal Auto_adjust_trxdly
		nop
#endif		
#endif	               
//--------------------------------------------------------------------------------
#ifdef CONFIG_DDR_SDRAM
		jal DDR_Auto_Calibration
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
#ifndef CONFIG_RTL8198
#if 1
 ################  JSW@20091013 :For 8196C FCS1=CKE  ######################

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
 ################  For 8196C FCS1=CKE end ######################
#endif
#endif


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

//--------------------------------------------------------------------------------				
//		Jump to booting	
		li		k0, BOOT_ADDR
		jr		k0
		nop 
              //all END              
//--------------------------------------------------------------------------------
//======================================================================
#if USING_RTL8196B_MEMCTRL
	EXPORT(Setting_Mem_Controller)
	
		//li     t0, 0x6ab20000
		//li     t1, 0xb8001000
		//sw     t0, 0(t1)       	

                //-------------------------DTR parameter setting
		//REG32_W(0xb8001008, 0xFFFF05C0);  //NFBI use
		REG32_W(0xb8001008, 0x6cca0c80); //JSW@20091106:For RTL8196C SDR 156MHZ
		//REG32_W(0xb8001008, 0x48EA0C80);  //new tune for jason RAM=148MHz
#ifdef CONFIG_RTL8198
                //----------------------98 memory parameter setting
		REG32_W(0xb8000048, 0x0f0adfff);//set bigger current for SDRAM
		REG32_W(0xb8000010, 0x800);//set to default
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
#endif
   

		jr ra
		nop
#endif	    

//======================================================================
#ifdef CONFIG_DDR_SDRAM
	EXPORT(DDR_Auto_Calibration)	
DDR_Auto_Calibration:
################DDR DDCR calibration######################

		  //Pre-setting if you know target DRAM chip's DDCR
	         // li     t0,  0x48400000  # mem timing configuration Reg1 init
 		  // li     t1, 0xb8001050
 	         // sw     t0, 0(t1)  
			
			//lock bus 
#ifdef CONFIG_RTL8198
			REG32_W(0xb8000014, 0x21c);  //bist controller   // hold LX0/1/2 Bus    //bit [4:2]=1 , [bit9]bist_retn=1
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

#ifdef CONFIG_RTL8198 //shift c0 anson@SD8 advise
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
#if 0
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
	UART_PRINT(UART_THR, boot_msg);
	j ra 
 	nop

boot_msg:	.asciiz "\r\nBooting..."
#endif	 
