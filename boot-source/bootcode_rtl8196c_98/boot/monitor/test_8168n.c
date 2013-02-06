#include "test_8168n.h"
#include <asm/rtl8196.h>

/*
 * Register Definitions
 */

/* GPIO */
#define GPIO_BASE       0xB8003500
#define PABCD_CNR       (GPIO_BASE + 0x00)
#define PABCD_DIR       (GPIO_BASE + 0x08)
#define PABCD_DAT       (GPIO_BASE + 0x0C)
#define PABCD_ISR       (GPIO_BASE + 0x10)
#define PAB_IMR         (GPIO_BASE + 0x14)
#define PCD_IMR         (GPIO_BASE + 0x18)


/*
 * Utility Macros
 */
#define htonl(A)                 ((((unsigned int)(A) & 0xff000000) >> 24) | \
                                  (((unsigned int)(A) & 0x00ff0000) >> 8)  | \
                                  (((unsigned int)(A) & 0x0000ff00) << 8)  | \
                                  (((unsigned int)(A) & 0x000000ff) << 24) )

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *) (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *) (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *) (addr))


/*
 * Tunable Parameters
 */

#define DEBUG_PRINT   1

//===============================================================================
static __inline void *memset(void *s, int c, int size)
{
   unsigned char *__s = (unsigned char *) s;
   unsigned char __c = (unsigned char) c;

   while (size--)
      *__s++ = __c;

   return s;
}
//===============================================================================
static __inline void *memcpy(void *dest, void *src, int size)
{
   unsigned char *__d = (unsigned char *) dest;
   unsigned char *__s = (unsigned char *) src;

   while (size--)
      *__d++ = *__s++;

   return dest;
}
//===============================================================================
unsigned char *tx1_desc_addr, *rx1_desc_addr;
unsigned char *tx1_buff_addr, *rx1_buff_addr;

unsigned char *tx2_desc_addr, *rx2_desc_addr;
unsigned char *tx2_buff_addr, *rx2_buff_addr;



//===============================================================================
//unsigned short example();
unsigned short example(int portnum, int quietmode)     // 0:port 0, 1:port 1,  2:port 0 and port 1, rc=0, fail, rc=1 pass
{

	int PCIE_Port0and1_8196B_208pin=portnum;     /*"2"=test P1 and P0  , "1"=test P1 , "0"= test P0  */

   /*
    * It seems for 8111C, first byte shall not be multicast.
    * Set 0x00 for safety
    */
   	
   static unsigned char *rx1_str = (unsigned char *) TX1_BUFF_ADDR;  
   static unsigned char *rx2_str = (unsigned char *) TX2_BUFF_ADDR;   


   int length1;
   int i,test_cnt;

#if Test_with_GPIO_C1_trigger
   /* For 8196B, Enable GPIO LED-PHASE[1-3] */
  #ifndef CONFIG_RTL8198
   WRITE_MEM32(0xB8000030, READ_MEM32(0xB8000030) | 0x03F00000);
  #endif
/*C1=input trigger pin from TESTER.
*  C2=Done pin,"1"=Done , "0"=Not done
*  C3=Result ,"1"=Pass , "0"=Fail
*/
 
   WRITE_MEM32(PABCD_CNR, 0xFFF1FFFF); // Enable
   WRITE_MEM32(PABCD_DIR, 0x000C0000); // C2, C3 as ouput; C1 as input
   WRITE_MEM32(PABCD_DAT, 0x00000000); // Output 0
#endif

   /* Get Random Data Length */
   //length1 = rand() & 0xFFF; // limit to 4095
   #if PCIE_Test_With_8102E
        // length1 = rand2() & 0xFFF; // limit to 4095
  	 // length1 = rand2() & 0x700; // limit to 1972 ,OK with 8102E
  	//     length1 = rand2() & 0x800; // limit to2048 ,fail
  	 //     length1 = rand2() & 0x7ff; // limit to2048 ,fail
  	 //     length1 = rand2() & 0x7cf; // limit to 1999 ,fail
       length1 = rand2() & 0x5ff; // limit to1535 ,OK for 8102E FT2

   	//length1 = rand2() & 0x600; // limit to1536 ,OK for 8102E FT2
  	
  	 //length1 = rand2() & 0xa00; // limit to 2560  ,fail with 8102E
  	 // length1 = 64; // limit to 64  
   #else
        length1 = rand2() & 0xfff; // limit to 64~4095
        //   length1 = 4092 ; // fixed to 4092 ,error
        //length1 = 4090 ; // fixed to 4090 ,error
        //length1 = 3834 ; // fixed to 4092 ,error

   	//    For SW test
       //	  length1 =  0x5ee; // limit to1518 ,OK
       //   	  length1 = rand2() & 0x5ee; // limit to1518 ,OK
       //  	    length1 = rand2() & 0x5f0; // limit to1520 ,OK
       //  	    length1 = rand2() & 0x5f5; // limit to1525 , OK
        // 	    length1 = rand2() & 0x5f7; // limit to1527 , OK
        //  	    length1 = rand2() & 0x5f8; // limit to1528 , OK
       // length1 = rand2() & 0x5f9; // limit to1529 , fail
       //length1 = rand2() & 0x5fA; // limit to1530 , fail
      
       //  length1 = rand2() & 0x5ff; // limit to1535 ,fail
       
        //length1 = 121 ; // fixed to 4092 ,error
   #endif
   
   length1 = (length1 < 64) ? 64 : length1; 

   /* Set MAC */
   rx1_str[0] = 0x00;
   rx1_str[1] = 0x11;
   rx1_str[2] = 0x22;
   rx1_str[3] = 0x33;
   rx1_str[4] = 0x44;
   rx1_str[5] = 0x55;

	
    #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
	   if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))//PORT1
	   {
	   	   //prom_printf("\nTest PCIE P1 \n");
		   rx2_str[0] = 0x00;
		   rx2_str[1] = 0x11;
		   rx2_str[2] = 0x22;
		   rx2_str[3] = 0x33;
		   rx2_str[4] = 0x44;
		   rx2_str[5] = 0x55;
	   }
   #endif

   /* Set Sequential Data */
   for (i = 6; i < length1; i++)
   {
            rx1_str[i] = (i - 6) & 0xFF; //default
        //   rx1_str[i] = ((i - 6) & 0xFF)|0x10;//JSW 20090214: test for PCIE_MacLoopBack ,bit4 always 1
        //   rx1_str[i] = ((i - 6) & 0xFF)|0x4;//JSW 20090214: test for PCIE_MacLoopBack ,bit2 always 1
        //       rx1_str[i] = ((i - 6) & 0xFF)|0x1C;//JSW 20090214: test for PCIE_MacLoopBack ,bit2/3/4 always 1
        //       rx1_str[i] = ((i - 6) & 0xFF)|0x8;//JSW 20090214: test for PCIE_MacLoopBack ,bit3 always 1
         //    rx1_str[i] = (i - 6) & 0xe3; //
          //     rx1_str[i] = 0xa5; //10100101
		 
	 #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
	   if(READ_MEM32(Bond_Option_REG)&0x00000040)
	   {
     		 rx2_str[i] = (i - 6) & 0xFF;	
	   }
	#endif
   }

 #if Test_with_GPIO_C1_trigger
	  dprintf("\n=========================\n");
	  dprintf("\nTest1:READ PCIE ID Device Test \n\n");
	  dprintf("\nWait for C1 Trigger... \n\n");
 #endif 
   
   /* Wait for C1 Trigger */
   #if Test_with_GPIO_C1_trigger
	   while (1)
	   {	
	   	if (	READ_MEM32(PABCD_DAT) & 0x00020000 )
			break;
	   }
   #endif
   
   /*
    * READ ID Test:
    * Read 8111C Vendor/Device ID
    */
 
   if ((PCIE_Port0and1_8196B_208pin==0) |(PCIE_Port0and1_8196B_208pin==2))
   {
	  if ((READ_MEM32(PCIE1_EP_CFG) == 0x816810EC) |(READ_MEM32(PCIE1_EP_CFG) == 0x819210EC)|\
		(READ_MEM32(PCIE1_EP_CFG) == 0x813610EC) )//  //
	   {
	      // Successful (C2 = 1, C3 = 1)
	      WRITE_MEM32(PABCD_DAT, 0x000C0000);
	       #if Test_with_GPIO_C1_trigger
		      dprintf("\n======================================\n");
		      dprintf("\nRead 8111/8192/8102 ID PASS !(Set and check C2 = 1, C3 = 1)");
		      dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT); 		
		      dprintf("\n=>PASS,PCIE P0's ID (0xb8b10000)=%x\n",READ_MEM32(PCIE1_EP_CFG)); 
		      dprintf("\n======================================\n");
		#endif
	   }
	   else
	   {
	      // Failed (C2 = 1, C3 = 0)
	      WRITE_MEM32(PABCD_DAT, 0x00040000);
	       #if DBG
		      dprintf("\nRead 8111/8192/8102 ID Fail !(Set and check C2 = 1, C3 = 0) \n");
		      dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT);  
			
	      #endif
		  if(quietmode==0)
		 dprintf("\n=>Fail,PCIE P0's ID (0xb8b10000)=%x\n",READ_MEM32(PCIE1_EP_CFG)); 	
		return 0;
	   }
   }
//=====================================================

 //Auto-test PCIE Port1 by recognize Bond_Option	
 #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)	
	 if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))//PORT1
	 {
	   if ((READ_MEM32(PCIE2_EP_CFG) == 0x816810EC) |(READ_MEM32(PCIE2_EP_CFG) == 0x819210EC)|\
			(READ_MEM32(PCIE2_EP_CFG) == 0x813610EC) )//  //
	   {
	      // Successful (C2 = 1, C3 = 1)
	      WRITE_MEM32(PABCD_DAT, 0x000C0000);
	       #if Test_with_GPIO_C1_trigger
		      dprintf("\n======================================\n");
		      dprintf("\nRead 8111/8192/8102 ID PASS !(Set and check C2 = 1, C3 = 1)");
		      dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT); 	
		      dprintf("\n=>PASS,PCIE P1's ID (0xb8b30000)=%x\n",READ_MEM32(PCIE2_EP_CFG)); 	
		      dprintf("\n======================================\n");
		#endif	
	   }
	   else
	   {
	      // Failed (C2 = 1, C3 = 0)
	      WRITE_MEM32(PABCD_DAT, 0x00040000);
	       #if DBG
		      dprintf("\nRead 8111/8192/8102 ID Fail !(Set and check C2 = 1, C3 = 0) \n");
		      dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT); 
		      dprintf("\n=>Fail,PCIE P1's ID (0xb8b30000)=%x\n",READ_MEM32(PCIE2_EP_CFG)); 	
			
	      #endif
		return 0;
	   }
	 }
#endif
//=====================================================

     WRITE_MEM32(PABCD_DAT, 0x00000000); // Output 0

 
    #if Test_with_GPIO_C1_trigger	 
	   dprintf("\n=========================\n");
	   dprintf("\nclear c2,c3 as Output 0\n");
	   WRITE_MEM32(PABCD_DAT, 0x00000000); // clear c2,c3 as Output 0
    #endif
	


   
    #if Test_with_GPIO_C1_trigger
	   dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT);    
	   dprintf("\n=========================\n");
	   dprintf("\nTest2: Send Descriptor test \n\n");	
	   dprintf("(Y)es, (N)o->");
    #endif 
  	
	
	//if (YesOrNo())
	if (1)
	{
		#if Test_with_GPIO_C1_trigger
			dprintf("\nWait for C1 Trigger... \n\n");

		  	 while (1)
			  {
			   	if (READ_MEM32(PABCD_DAT) & 0x00020000 )
					break;
			  }
		#endif
		
	   /*	
	    * MAC Loopback Test:
	    * TX 1 Packet and then RX compare
	    */

	  /*"2"=test P1 and P0  , "1"=test P1 , "0"= test P0  */
	   rtl8168_init(PCIE_Port0and1_8196B_208pin ); 
    	  
	
	   rtl8168_tx(rx1_str, length1, PCIE_Port0and1_8196B_208pin);

	   //JSW 20090214: For tx/rx delay
	  // __delay(1000*100*10);
	    #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
	   	  if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))
	   	  {
	 	  	rtl8168_tx(rx2_str, length1, 2);
	   	  }
	   #endif

	   rtl8168_tx_trigger(PCIE_Port0and1_8196B_208pin);

	   if((PCIE_Port0and1_8196B_208pin==0)|(PCIE_Port0and1_8196B_208pin==2))
	   {
		   if (rtl8168_rx(rx1_str, length1, PCIE_Port0and1_8196B_208pin) == 0)   //"0"==compare OK,"-1"=fail
		   {
		   
		      // Successful (C2 = 1, C3 = 1)
		      WRITE_MEM32(PABCD_DAT, 0x000C0000);

			 // dprintf("PCIE_P0 => PASS !(Set and check C2 = 1, C3 = 1)\n");
			 if(quietmode==0)
			  dprintf("PCIE_P0 => PASS !\n");
		       #if Test_with_GPIO_C1_trigger
			      
			      //dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT);
			#endif
		   }
		   else
		   {
		      // Failed (C2 = 1, C3 = 0)
		      WRITE_MEM32(PABCD_DAT, 0x00040000);
		       #if DBG
			      //dprintf("PCIE_P0 => Fail !(Set and check C2 = 1, C3 = 0) \n");
			 if(quietmode==0)			      
    			      dprintf("PCIE_P0 => Fail ! \n");
			      //dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT); 
			#endif
			return 0;
		   }
	   }	
	   
	    #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)		   	  
	   	  if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))
	   	  {
			  if (rtl8168_rx(rx2_str, length1, 2) == 0)   //"0"==compare OK,"-1"=fail
			   {
			   
			      // Successful (C2 = 1, C3 = 1)
			      WRITE_MEM32(PABCD_DAT, 0x000C0000);
			       #if Test_with_GPIO_C1_trigger
				      dprintf("PCIE_P1  => PASS !(Set and check C2 = 1, C3 = 1)\n");
				      //dprintf("\nPABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT);
				#endif
			   }
			   else
			   {
			      // Failed (C2 = 1, C3 = 0)
			      WRITE_MEM32(PABCD_DAT, 0x00040000);
			       #if DBG
				      dprintf("PCIE_P1  => Fail !(Set and check C2 = 1, C3 = 0) \n");
				      dprintf("PABCD_DAT(0xb800350C)=%x\n",*(volatile unsigned int*)PABCD_DAT); 
				#endif
				return 0;
			   }	   
	   	   }
	   #endif  		
   			
	  	 
		  return 1;
	}//endif y or n
	else
	{
		 #if DBG
		      dprintf("\nExit PCIE Test !\n");
		     
		 #endif
		 return 0;		
	}
	
	
	
}
//===============================================================================
void rtl8168_init(int no)
{
   int i;

   if ((no == 0) |(no == 2))
   {
      // 0. Set PCIE RootComplex
      WRITE_MEM32(PCIE1_RC_CFG + 0x04, 0x00100007);
      WRITE_MEM8(PCIE1_RC_CFG + 0x78, (READ_MEM8(PCIE1_EP_CFG + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B,default
      //WRITE_MEM8(PCIE1_RC_CFG + 0x78, (READ_MEM8(PCIE1_EP_CFG + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_256B);  // Set MAX_PAYLOAD_SIZE to 256B

      // 1. Set 8111C EP
      WRITE_MEM32(PCIE1_EP_CFG + 0x04, 0x00180007);  // Mem, IO Enable
      WRITE_MEM32(PCIE1_EP_CFG + 0x10, (PCIE1_EP_IO | 0x00000001) & 0x1FFFFFFF);  // Set BAR
      WRITE_MEM32(PCIE1_EP_CFG + 0x18, (PCIE1_EP_MEM | 0x00000004) & 0x1FFFFFFF);  // Set BAR

      WRITE_MEM8(PCIE1_EP_CFG + 0x78, (READ_MEM8(PCIE1_EP_CFG + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
     // WRITE_MEM8(PCIE1_EP_CFG + 0x79, (READ_MEM8(PCIE1_EP_CFG + 0x79) & (~0x70)) | MAX_READ_REQSIZE_128B);  // Set MAX_REQ_SIZE to 128B 
      WRITE_MEM8(PCIE1_EP_CFG + 0x79, (READ_MEM8(PCIE1_EP_CFG + 0x79) & (~0x70)) | MAX_READ_REQSIZE_256B);  // Set MAX_REQ_SIZE to 256B,default

      // 2. Reset EP
      WRITE_MEM8(PCIE1_EP_MEM + ChipCmd, 0x10);

      // 3. Set MAC Loopback & Disable TX CRC & TxDMA Size
      WRITE_MEM32(PCIE1_EP_MEM + TxConfig, (READ_MEM32(PCIE1_EP_MEM + TxConfig) & (~0x700)) | TxDMA1KB | TxMACLoopBack | (1 << 16));
	
      // Enable Runt & Error Accept of RX Config
      WRITE_MEM32(PCIE1_EP_MEM + RxConfig, (READ_MEM32(PCIE1_EP_MEM + RxConfig) & (~0x700)) | RxDMA512B | AcceptErr | AcceptRunt | (1 << 7));

      // 4. Set TX/RX Desciptor Starting Address
      WRITE_MEM32(PCIE1_EP_MEM + TxDescStartAddrLow, PADDR(TX1_DESC_ADDR));
      WRITE_MEM32(PCIE1_EP_MEM + TxDescStartAddrHigh, 0);
      WRITE_MEM32(PCIE1_EP_MEM + RxDescAddrLow, PADDR(RX1_DESC_ADDR));
      WRITE_MEM32(PCIE1_EP_MEM + RxDescAddrHigh, 0);

      // 5. Set TX Ring - Descriptor Assigned to CPU
      memset((unsigned char *) TX1_DESC_ADDR, 0x0, NUM_TX_DESC * TX1_DESC_SIZE);

      for (i = 0; i < NUM_TX_DESC; i++)
      {
         if(i == (NUM_TX_DESC - 1))
            WRITE_MEM32(TX1_DESC_ADDR + TX1_DESC_SIZE * i, htonl(PADDR(RingEnd)));
      }

      // 6. Set RX Ring - Descriptor Assigned to NIC
      memset((unsigned char *) RX1_DESC_ADDR, 0x0, NUM_RX_DESC * RX1_DESC_SIZE);

      for (i = 0; i < NUM_RX_DESC; i++)
      {
         if(i == (NUM_RX_DESC - 1))
            WRITE_MEM32(RX1_DESC_ADDR + RX1_DESC_SIZE * i, htonl(DescOwn | RingEnd | RX1_BUFF_SIZE));
         else
            WRITE_MEM32(RX1_DESC_ADDR + RX1_DESC_SIZE * i, htonl(DescOwn | RX1_BUFF_SIZE));
      }

      tx1_desc_addr = (unsigned char *) TX1_DESC_ADDR;
      tx1_buff_addr = (unsigned char *) TX1_BUFF_ADDR;
      rx1_desc_addr = (unsigned char *) RX1_DESC_ADDR;
      rx1_buff_addr = (unsigned char *) RX1_BUFF_ADDR;
   }

   //else  
	
   
   #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
   if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))//PORT1
   {
      // 0. Set PCIE RootComplex
      WRITE_MEM32(PCIE2_RC_CFG + 0x04, 0x00100007);
      WRITE_MEM8(PCIE2_RC_CFG + 0x78, (READ_MEM8(PCIE2_EP_CFG + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B

      // 1. Set 8111C EP
      WRITE_MEM32(PCIE2_EP_CFG + 0x04, 0x00180007);  // Mem, IO Enable
      WRITE_MEM32(PCIE2_EP_CFG + 0x10, (PCIE2_EP_IO | 0x00000001) & 0x1FFFFFFF);  // Set BAR
      WRITE_MEM32(PCIE2_EP_CFG + 0x18, (PCIE2_EP_MEM | 0x00000004) & 0x1FFFFFFF);  // Set BAR

      WRITE_MEM8(PCIE2_EP_CFG + 0x78, (READ_MEM8(PCIE2_EP_CFG + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
      WRITE_MEM8(PCIE2_EP_CFG + 0x79, (READ_MEM8(PCIE2_EP_CFG + 0x79) & (~0x70)) | MAX_READ_REQSIZE_256B);  // Set MAX_REQ_SIZE to 128B

      // 2. Reset EP
      WRITE_MEM8(PCIE2_EP_MEM + ChipCmd, 0x10);

      // 3. Set MAC Loopback & Disable TX CRC & TxDMA Size
     
      	WRITE_MEM32(PCIE2_EP_MEM + TxConfig, (READ_MEM32(PCIE2_EP_MEM + TxConfig) & (~0x700)) | TxDMA1KB | TxMACLoopBack | (1 << 16));
	
      // Enable Runt & Error Accept of RX Config
      WRITE_MEM32(PCIE2_EP_MEM + RxConfig, (READ_MEM32(PCIE2_EP_MEM + RxConfig) & (~0x700)) | RxDMA512B | AcceptErr | AcceptRunt | (1 << 7));

      // 4. Set TX/RX Desciptor Starting Address
      WRITE_MEM32(PCIE2_EP_MEM + TxDescStartAddrLow, PADDR(TX2_DESC_ADDR));
      WRITE_MEM32(PCIE2_EP_MEM + TxDescStartAddrHigh, 0);
      WRITE_MEM32(PCIE2_EP_MEM + RxDescAddrLow, PADDR(RX2_DESC_ADDR));
      WRITE_MEM32(PCIE2_EP_MEM + RxDescAddrHigh, 0);

      // 5. Set TX Ring - Descriptor Assigned to CPU
      memset((unsigned char *) TX2_DESC_ADDR, 0x0, NUM_TX_DESC * TX2_DESC_SIZE);

      for (i = 0; i < NUM_TX_DESC; i++)
      {
         if(i == (NUM_TX_DESC - 1))
            WRITE_MEM32(TX2_DESC_ADDR + TX2_DESC_SIZE * i, htonl(PADDR(RingEnd)));
      }

      // 6. Set RX Ring - Descriptor Assigned to NIC
      memset((unsigned char *) RX2_DESC_ADDR, 0x0, NUM_RX_DESC * RX2_DESC_SIZE);

      for (i = 0; i < NUM_RX_DESC; i++)
      {
         if(i == (NUM_RX_DESC - 1))
            WRITE_MEM32(RX2_DESC_ADDR + RX2_DESC_SIZE * i, htonl(DescOwn | RingEnd | RX2_BUFF_SIZE));
         else
            WRITE_MEM32(RX2_DESC_ADDR + RX2_DESC_SIZE * i, htonl(DescOwn | RX2_BUFF_SIZE));
      }

      tx2_desc_addr = (unsigned char *) TX2_DESC_ADDR;
      tx2_buff_addr = (unsigned char *) TX2_BUFF_ADDR;
      rx2_desc_addr = (unsigned char *) RX2_DESC_ADDR;
      rx2_buff_addr = (unsigned char *) RX2_BUFF_ADDR;
   }
  #endif
   
}

//===============================================================================
void rtl8168_tx(unsigned char *content, unsigned int size, int no)
{
   unsigned int i;

   if ((no == 0) |(no == 2) )
   {
      // Fill RX Descriptor
      WRITE_MEM32(RX1_DESC_ADDR + 0x08, htonl(PADDR(RX1_BUFF_ADDR)));  // RX Buffer Address
      WRITE_MEM32(RX1_DESC_ADDR + 0x18, htonl(PADDR(RX1_BUFF_ADDR)));  // RX Buffer Address

      // Ensure Descriptor is updated
      READ_MEM32(RX1_DESC_ADDR + 0x8);

      // Enable TX/RX (This seems to trigger NIC prefetching RX descriptor)
      WRITE_MEM8(PCIE1_EP_MEM + ChipCmd, 0x0C);

      #if 1
      // Fill Descriptor
      WRITE_MEM32(TX1_DESC_ADDR + 0x0, htonl(DescOwn | FirstFrag | LastFrag | RingEnd | size));
      WRITE_MEM32(TX1_DESC_ADDR + 0x8, htonl(PADDR((unsigned int) TX1_BUFF_ADDR)));

      // Ensure Descriptor is updated
      READ_MEM32(TX1_DESC_ADDR + 0x8);
      #else
      // Fill TX Descriptor, Buffer
      while (size > 0)
      {
         unsigned int buf_size = MIN(size, TX1_BUFF_SIZE);

         // Fill Buffer
         memcpy(tx1_buff_addr, content, buf_size);
         content += buf_size;
         size -= buf_size;

         // Fill Descriptor
         WRITE_MEM32(TX1_DESC_ADDR + 0x0, htonl(DescOwn | FirstFrag | LastFrag | RingEnd | buf_size));
         WRITE_MEM32(TX1_DESC_ADDR + 0x8, htonl(PADDR((unsigned int) TX1_BUFF_ADDR)));

         // Ensure Descriptor is updated
         READ_MEM32(TX1_DESC_ADDR + 0x8);

         tx1_desc_addr += TX1_DESC_SIZE;
         tx1_buff_addr += TX1_BUFF_SIZE;
      }
      #endif
   }

  #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
   if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))//PORT1
   {
      // Fill RX Descriptor
      WRITE_MEM32(RX2_DESC_ADDR + 0x08, htonl(PADDR(RX2_BUFF_ADDR)));  // RX Buffer Address
      WRITE_MEM32(RX2_DESC_ADDR + 0x18, htonl(PADDR(RX2_BUFF_ADDR)));  // RX Buffer Address

      // Ensure Descriptor is updated
      READ_MEM32(RX2_DESC_ADDR + 0x8);

      // Enable TX/RX (This seems to trigger NIC prefetching RX descriptor)
      WRITE_MEM8(PCIE2_EP_MEM + ChipCmd, 0x0C);

      #if 1
      // Fill Descriptor
      WRITE_MEM32(TX2_DESC_ADDR + 0x0, htonl(DescOwn | FirstFrag | LastFrag | RingEnd | size));
      WRITE_MEM32(TX2_DESC_ADDR + 0x8, htonl(PADDR((unsigned int) TX2_BUFF_ADDR)));

      // Ensure Descriptor is updated
      READ_MEM32(TX2_DESC_ADDR + 0x8);
      #else
      // Fill TX Descriptor, Buffer
      while (size > 0)
      {
         unsigned int buf_size = MIN(size, TX2_BUFF_SIZE);

         // Fill Buffer
         memcpy(tx2_buff_addr, content, buf_size);
         content += buf_size;
         size -= buf_size;

         // Fill Descriptor
         WRITE_MEM32(TX2_DESC_ADDR + 0x0, htonl(DescOwn | FirstFrag | LastFrag | RingEnd | buf_size));
         WRITE_MEM32(TX2_DESC_ADDR + 0x8, htonl(PADDR((unsigned int) TX2_BUFF_ADDR)));

         // Ensure Descriptor is updated
         READ_MEM32(TX2_DESC_ADDR + 0x8);

         tx2_desc_addr += TX2_DESC_SIZE;
         tx2_buff_addr += TX2_BUFF_SIZE;
      }
      #endif
     }
 #endif
}
//===============================================================================
int rtl8168_rx(unsigned char *content, unsigned int size, int no)
{
   unsigned int i;
   static unsigned int P0_PCIE_error_count=0;
   static unsigned int P1_PCIE_error_count=0;

   
   
  #if DBG	  
     prom_printf("\nTest Rx Bytes=%d\n",size); //OK   
     prom_printf("P0 PCIE Error count: %d\n",P0_PCIE_error_count);
     prom_printf("P1 PCIE Error count: %d\n",P1_PCIE_error_count);
  #endif 
   
  
    if ((no == 0) |(no == 2) )
   {
      // Wait RX Packet   

	// #if PCIE_Test_With_8102E
	 #if 0
	      while ((READ_MEM32(RX1_DESC_ADDR) & htonl(DescOwn)) != 0)
	      {
		  	    //for 8102E
		  	    if (READ_MEM32(PCIE1_EP_CFG) == 0x813610EC) 
		  	   {
		  	   	   __delay(10);
				   break;
			    }
	      }		 	 
	    
	#else
		  //loop here when OWN bit=1 (means Memory is owned by PCIE IP)
		  while ((READ_MEM32(RX1_DESC_ADDR) & htonl(DescOwn)) != 0); //Memory own,"0":CPU,"1":PCIE IP
       #endif
	    
	

	
      // Check RX Packet Content
      //#if PCIE_Test_With_8102E
      int i2;
	   
      for (i = 0; i < size; i++)        
      {
      
         if (READ_MEM8(RX1_BUFF_ADDR + i) != content[i])
         {
           #if DBG//JSW:DEBUG_PRINT
		  P0_PCIE_error_count++;
		     		  
	         prom_printf("Compare Error, No: Port0, Size: %d, Offset: %d, Content: 0x%X, Expected: 0x%X\n",
	                     size, i, READ_MEM8(RX1_BUFF_ADDR + i), content[i]);
		   //prom_printf("PCIE Mac LoopBack Compare Error!!! \n");
		   for (i2=1;i2<=4;i2++)
		   {
		   	 prom_printf("\nOffset(%d),Content:%x,Expected:%x\n",(i+i2),READ_MEM8(RX1_BUFF_ADDR + i+i2),content[i+i2]);

		   }
		   
		  
		   //prom_printf("\ncontent=%x\n",&content);
		   prom_printf("\n==================================\n");
		   prom_printf("\nOK Tx(0x%x) content=%x\n",(0xa0420000+i),READ_MEM32(0xa0420000+i));
		   prom_printf("\nFail Rx(0x%x) content=%x\n",(0xa0630000+i),READ_MEM32(0xa0630000+i));
		   prom_printf("\n==================================\n");

		
		   prom_printf("\nCheck last 4 Bytes,Tx(0x%x) content=%x\n",(0xa0420000+i-4),READ_MEM32(0xa0420000+i-4));
		   prom_printf("\nCheck last 4 Bytes,Rx(0x%x) content=%x\n",(0xa0630000+i-4),READ_MEM32(0xa0630000+i-4));
		   
		   
		 //#if 1 //for 2 port
		 		 	
		 	return -1; // RX Error
		 
		  
	    #else
 		   dprintf("Compare Error, No: Port0, Size: %d, Offset: %d, Content: 0x%X, Expected: 0x%X\n",
	                     size, i, READ_MEM8(RX1_BUFF_ADDR + i), content[i]);
			return -1;
	   #endif
	
            
//   halt_rx1:
//            goto halt_rx1;
         }
      }
   }

  #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
   if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))//PORT1
   {
      // Wait RX Packet

       	// #if PCIE_Test_With_8102E
	 #if 0
	      while ((READ_MEM32(RX1_DESC_ADDR) & htonl(DescOwn)) != 0)
	      {
		  	    //for 8102E
		  	    if (READ_MEM32(PCIE1_EP_CFG) == 0x813610EC) 
		  	   {
		  	   	   __delay(10);
				   break;
			    }
	      }		 	 
	    
	#else
		  while ((READ_MEM32(RX2_DESC_ADDR) & htonl(DescOwn)) != 0);
       #endif

       int i3;
      // Check RX Packet Content
        for (i = 0; i < size; i++)        
      {
      
         if (READ_MEM8(RX2_BUFF_ADDR + i) != content[i])
         {
           #if DBG//JSW:DEBUG_PRINT
		   P1_PCIE_error_count++;	
		  
	         prom_printf("Compare Error, No: Port1, Size: %d, Offset: %d, Content: 0x%X, Expected: 0x%X\n",
	                     size, i, READ_MEM8(RX2_BUFF_ADDR + i), content[i]);
		   //prom_printf("PCIE Mac LoopBack Compare Error!!! \n");
		   for (i3=1;i3<=4;i3++)
		   {
		   	 prom_printf("\nOffset(%d),Content:%x,Expected:%x\n",(i+i3),READ_MEM8(RX2_BUFF_ADDR + i+i3),content[i+i3]);

		   }
		   		   
			  
  		   
		   //prom_printf("\ncontent=%x\n",&content);
		   prom_printf("\n================================================\n");
		   prom_printf("\nOK Tx(0x%x) content=%x\n",(0xa0820000+i),READ_MEM32(0xa0820000+i));
		   prom_printf("\nFail Rx(0x%x) content=%x\n",(0xa0a30000+i),READ_MEM32(0xa0a30000+i));
		   prom_printf("\n================================================\n");

		   prom_printf("\nCheck last 4 Bytes,Tx(0x%x) content=%x\n",(0xa0820000+i-4),READ_MEM32(0xa0820000+i-4));
		   prom_printf("\nCheck last 4 Bytes,Rx(0x%x) content=%x\n",(0xa0a30000+i-4),READ_MEM32(0xa0a30000+i-4));
		   prom_printf("\n================================================\n");
		   return -1;
	    #else
 		   dprintf("Compare Error, No: Port1, Size: %d, Offset: %d, Content: 0x%X, Expected: 0x%X\n",
	                     size, i, READ_MEM8(RX2_BUFF_ADDR + i), content[i]);
		   return -1;
	   #endif
	
            
//   halt_rx1:
//            goto halt_rx1;
         }
      }
   }
 #endif

   return 0;
}
//===============================================================================
void rtl8168_tx_trigger(int no)
{
    if ((no == 0) |(no == 2) )
   {
      // Indicate TX Packet
      WRITE_MEM8(PCIE1_EP_MEM + TxPoll, 0x40);
   }

    #if (RTL8196B_ASIC_FT2) ||(TEST_8196_PCIE_P1)
	  if((READ_MEM32(Bond_Option_REG)&0x00000040)||(PCIE_Port0and1_8196B_208pin==2))//PORT1
	   {
	      // Indicate TX Packet
	      WRITE_MEM8(PCIE2_EP_MEM + TxPoll, 0x40);
	   }
   #endif
}

//===============================================================================


