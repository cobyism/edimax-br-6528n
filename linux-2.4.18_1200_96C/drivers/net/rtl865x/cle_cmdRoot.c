
#ifdef __linux__
#include "version.h"
#include <linux/autoconf.h>
#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#endif

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include "rtl_types.h"
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include "asicregs.h"
#endif

#include "rtl8651_tblDrv.h"

/*command line engine supported*/
#include "cle/rtl_cle.h"
#include "swNic2.h"

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include "rtl_types.h"
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include "asicregs.h"
#endif

#ifdef CONFIG_RTL865X_MULTILAYER_BSP
//CLE commands for Multilayer BSP
#include "tblDrv/rtl8651_tblDrv_cmd.h"
#include "tblDrv/rtl8651_tblDrvFwd_cmd.h"
#endif
#include "rtl8651_tblAsicDrv_cmd.h"

#if defined(CONFIG_SMI)
#include "rtl8366/smi_cmd.h"
#endif

#if defined(CONFIG_RTL8366) 
#include "rtl8366/rtl8366_cmd.h"
#endif 

#if defined(CONFIG_RTL8310P)
#include "rtl8310p/rtl8310p_cmd.h"
#endif

#if  defined(CONFIG_RTL8316S) & !defined(RTL8316S_MODEL_USER)	
#include "rtl8316s/rtl8316s_cmd.h"
#endif

//for RTL8366S design
#if defined(CONFIG_RTL8366S) 
#include "rtl8366s/rtl8366s_cmd.h"
#endif

#ifdef CONFIG_MDCMDIO
#include "mdcmdio_cmd.h"
#endif

#ifdef CONFIG_RTL8305S
#include "rtl865x/rtl8305s_cmd.h"
#endif


#ifdef CONFIG_RTL8306SDM
#include "../switch/rtl8306/mdcmdio_cmd.h"
#include "../switch/rtl8306/Rtl8306_AsicDrv_cmd.h"
#endif


#ifdef CONFIG_RTL8363
#include "rtl8363/testcode/Rtl8363_Asic_cmd.h"
#include "rtl8363/oam/rtl_1ah_OAM_cmd.h"
#include "rtl8363/rtl8363_asicdrv_cmd.h"
#endif

#ifdef CONFIG_RTL8309SB
#include "./rtl8309sb/rtl8309sb_cmd.h"
#endif

#ifdef CONFIG_IGMP_SNOOPING
#include "../switch/multicast/rtl_igmp_cmd.h"
#endif

#ifdef CONFIG_MULTICAST_SNOOPING
#include "../switch/multicastV2/rtl_multicast_cmd.h"
#endif


#if defined (CONFIG_RTL8185) || defined (CONFIG_RTL8185B)
#include "flashdrv.h"
#endif

#ifdef CONFIG_RTL865x_SWPUBKEY
#include "crypto/rtl_crypt_cmd.h"
#endif

#ifdef CONFIG_RTL865XB_EXP_CRYPTOENGINE
//CLE commands for Crypto engine
	#ifdef CONFIG_RTL865XB
		#include "crypto/865xb/cryptoCmd.h"
	#elif defined (CONFIG_RTL865XC)	
		#include "crypto/865xc/cryptoCmd.h"
	#else
		#error Neither CONFIG_RTL865XB nor CONFIG_RTL865XC is defined.
	#endif
#endif

#ifdef CONFIG_RTL865X_BENCHMARK
#include "benchmark/benchmark.h"
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
#include "romeperf.h"
#endif

#ifdef CONFIG_RTL865X_PCM
#include "voip/voip_cmd.h"
#endif

#if defined(RTL865X_MODEL_USER)||defined(RTL865X_MODEL_KERNEL)
#include "model/model_cmd.h"
#include "cle/cle_userglue.h"
#endif

#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
#include "cle_cmdRoot.h"
#endif


#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
#include "tblDrv/light_cmd.h"
#endif

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV

#ifdef CONFIG_RTL865XB_EXP_CRYPTOENGINE
extern unsigned long volatile jiffies;
static int8 *cryptKey, *cryptIv;
int8 destBuffer[8192];
static int8 srcBuffer[8192];
int32 maxPoll=0;
int32 cryptoMemcpyInit=0;

static int32 cmdCrymemcpy(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	uint32 start, length,cryptCopyBytes,iteration;
	int32 emulate=0, mode=7, pollFreq=0;
	int32 i;
	#define UNCACHED_MALLOC(x)  (void *) (0xa0000000 | (uint32) rtlglue_malloc(x))
	if(cryptoMemcpyInit==0){
		rtl8651b_cryptoEngine_init(4,FALSE );
		cryptKey = (int8 *) UNCACHED_MALLOC(24);
		cryptIv = (int8 *) UNCACHED_MALLOC(8);
		for(i=0; i<24; i++)
			cryptKey[i] = 0x01;
		for(i=0; i<8; i++)
			cryptIv[i] = 0x01;
		cryptoMemcpyInit=1;
		memset(destBuffer, 1, sizeof(destBuffer));
		memset(srcBuffer, 1, sizeof(srcBuffer));		
	}
	rtlglue_printf("(Each 8K)Src: %08x, Dst: %08x\n", (uint32)srcBuffer, (uint32)destBuffer);
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(!size){
		if(strcmp("emulate", nextToken)!=0)
			return SUCCESS;
		emulate=1;
		cle_getNextCmdToken(&nextToken,&size,saved); 
	}
	length = U32_value(nextToken);
	if(length>sizeof(destBuffer))
		length=sizeof(destBuffer);
	cle_getNextCmdToken(&nextToken,&size,saved); 
	cryptCopyBytes = U32_value(nextToken);
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(!size){
		mode |= 0x8; //polling		
		cle_getNextCmdToken(&nextToken,&size,saved); 
		pollFreq = U32_value(nextToken); //polling frequency
		cle_getNextCmdToken(&nextToken,&size,saved); 
	}
	iteration = U32_value(nextToken);
	maxPoll=0;
	start = jiffies;
	for(i=0;i<iteration;i++){
		int32 poll;		
		if(cryptCopyBytes&7){
			cryptCopyBytes&=~7;
			cryptCopyBytes+=8;
		}
		if(cryptCopyBytes>0){
			rtl8651b_cryptoMemcpy(destBuffer); //specify destination address
		//rtl8651b_cryptoMemcpy(srcBuffer); //specify destination address
		if(emulate==1)
			mode&=~0x10;
		else
			mode |= 0x10;			
		if(FAILED==rtl8651b_cryptoEngine_des(mode, (void *)(0x20000000|((uint32)&srcBuffer[0])),cryptCopyBytes, cryptKey, cryptIv))
			rtlglue_printf("%s:cryptCopyBytes:%uB  crypto:%u, memcpy:%u FAILED\n", 
				(mode&8)?"Nonblock":"Blocking",(uint32)length, (uint32)cryptCopyBytes,(uint32) length- cryptCopyBytes);
		}
	
		if(length-cryptCopyBytes>0){
			//always copy to uncached.
			memcpy((void *)(0x20000000|((uint32)&destBuffer[cryptCopyBytes])), (void *)((0x20000000|(uint32)&srcBuffer[cryptCopyBytes])),length-cryptCopyBytes);
		}

		if(cryptCopyBytes>0 && (mode&8)){
			poll=rtl8651b_cryptoEngine_des_poll(pollFreq);
			if(poll<0){
				if(-poll>maxPoll){
					rtlglue_printf("Poll: %d, max:%d\n", -poll, maxPoll);					
					maxPoll=-poll;
				}
			}
		}
	}
	rtlglue_printf("Mode %x: Copy %d * %d times. Total %d bytes, %d ms.  %u KBps\n",mode, length, iteration, (uint32)(length*iteration),(uint32)((jiffies-start)*10), (uint32)((length*iteration))/(10*((uint32)(jiffies-start))));
	return SUCCESS;
}
#endif/*#ifdef CONFIG_RTL865XB_EXP_CRYPTOENGINE	*/
#endif /*#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV*/

#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION

void startCOP3Counters(int32 countInst){
	/* counter control modes:
		0x10	cycles
		0x11	new inst fetches
		0x12	inst fetch cache misses
		0x13	inst fetch miss busy cycles
		0x14	data store inst
		0x15	data load inst
		0x16	load or store inst
		0x1a	load or store cache misses
		0x1b	load or store miss busy cycles
		*/
	uint32 cntmode;

	if(countInst == TRUE)
		cntmode = 0x13121110;
	else
		cntmode = 0x1b1a1610;
	
	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9,$12\n\t"
		"nop\n\t"
		"la $10,0x80000000\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		/* stop counters */
		"ctc3 $0,$0\n\t"
		/* clear counters */
		"mtc3 $0,$8\n\t"
		"mtc3 $0,$9\n\t"
		"mtc3 $0,$10\n\t"
		"mtc3 $0,$11\n\t"
		"mtc3 $0,$12\n\t"
		"mtc3 $0,$13\n\t"
		"mtc3 $0,$14\n\t"
		"mtc3 $0,$15\n\t"
		/* set counter controls */
		"ctc3 %0,$0"
		: /* no output */
		: "r" (cntmode)
		);
}


uint32 stopCOP3Counters(void){
	uint32 cntmode;
	uint32 cnt0h, cnt0l, cnt1h, cnt1l, cnt2h, cnt2l, cnt3h, cnt3l;
	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9,$12\n\t"
		"nop\n\t"
		"la $10,0x80000000\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		/* get counter controls */
		"cfc3 %0,$0\n\t"
		/* stop counters */
		"ctc3 $0,$0\n\t"
		/* save counter contents */
		"mfc3 %1,$9\n\t"
		"mfc3 %2,$8\n\t"
		"mfc3 %3,$11\n\t"
		"mfc3 %4,$10\n\t"
		"mfc3 %5,$13\n\t"
		"mfc3 %6,$12\n\t"
		"mfc3 %7,$15\n\t"
		"mfc3 %8,$14\n\t"
		"nop\n\t"
		"nop\n\t"
		: "=r" (cntmode), "=r" (cnt0h), "=r" (cnt0l), "=r" (cnt1h), "=r" (cnt1l), "=r" (cnt2h), "=r" (cnt2l), "=r" (cnt3h), "=r" (cnt3l)
		: /* no input */
		);
	if(cntmode == 0x13121110){
		rtlglue_printf("COP3 counter for instruction access\n");
		rtlglue_printf("%10d cycles\n", cnt0l);
		rtlglue_printf("%10d new inst fetches\n", cnt1l);
		rtlglue_printf("%10d inst fetch cache misses\n", cnt2l);
		rtlglue_printf("%10d inst fetch miss busy cycles\n", cnt3l);
		}
	else{
		rtlglue_printf("COP3 counter for data access\n");
		rtlglue_printf("%10d cycles\n", cnt0l);
		rtlglue_printf("%10d load or store inst\n", cnt1l);
		rtlglue_printf("%10d load or store cache misses\n", cnt2l);
		rtlglue_printf("%10d load or store miss busy cycles\n", cnt3l);
		}
	return cnt0l;
}

static int8 cop3InstMode;
static uint64 cop3Cycles, cop3InstFetches, cop3InstCacheMisses, cop3InstcacheMissBusyCycles;
static uint64 cop3DataInst, cop3DataCacheMisses, cop3DataCacheMissCycles;

void clearCOP3Counters(void){
	cop3InstMode = 0;
	cop3Cycles = 0;
	cop3InstFetches = 0;
	cop3InstCacheMisses = 0;
	cop3InstcacheMissBusyCycles = 0;
	cop3DataInst = 0;
	cop3DataCacheMisses = 0;
	cop3DataCacheMissCycles = 0;
}

void pauseCOP3Counters(void){
	uint32 cntmode;
	uint32 cnt0h, cnt0l, cnt1h, cnt1l, cnt2h, cnt2l, cnt3h, cnt3l;
	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9,$12\n\t"
		"nop\n\t"
		"la $10,0x80000000\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		/* get counter controls */
		"cfc3 %0,$0\n\t"
		/* stop counters */
		"ctc3 $0,$0\n\t"
		/* save counter contents */
		"mfc3 %1,$9\n\t"
		"mfc3 %2,$8\n\t"
		"mfc3 %3,$11\n\t"
		"mfc3 %4,$10\n\t"
		"mfc3 %5,$13\n\t"
		"mfc3 %6,$12\n\t"
		"mfc3 %7,$15\n\t"
		"mfc3 %8,$14\n\t"
		"nop\n\t"
		"nop\n\t"
		: "=r" (cntmode), "=r" (cnt0h), "=r" (cnt0l), "=r" (cnt1h), "=r" (cnt1l), "=r" (cnt2h), "=r" (cnt2l), "=r" (cnt3h), "=r" (cnt3l)
		: /* no input */
		);
	if(cntmode == 0x13121110){
		cop3InstMode = 1;
		cop3Cycles += cnt0l;
		cop3InstFetches += cnt1l;
		cop3InstCacheMisses = cnt2l;
		cop3InstcacheMissBusyCycles = cnt3l;
		}
	else{
		cop3InstMode = 0;
		cop3Cycles += cnt0l;
		cop3DataInst += cnt1l;
		cop3DataCacheMisses += cnt2l;
		cop3DataCacheMissCycles += cnt3l;
		}
}

void DisplayCOP3Counters(void){
#define _displayy(val, str) if(val > 0xffffffffUL)\
	rtlglue_printf("0x%x%08x %s\n", (uint32) (val >> 32), (uint32) (val & 0xffffffffUL), str);\
else\
	rtlglue_printf("%10d %s\n", (uint32) val, str);

	if(cop3InstMode == 1){
		rtlglue_printf("COP3 counter for instruction access\n");
		_displayy(cop3Cycles, "cycles");
		_displayy(cop3InstFetches, "new instruction fetches");
		_displayy(cop3InstCacheMisses,"instruction fetch cache misses");
		_displayy(cop3InstcacheMissBusyCycles, "instruction fetch miss busy cycles");
		}
	else{
		rtlglue_printf("COP3 counter for data access\n");
		_displayy(cop3Cycles, "cycles");
		_displayy(cop3DataInst, "load or store instruction");
		_displayy(cop3DataCacheMisses,"load or store cache misses");
		_displayy(cop3DataCacheMissCycles, "load or store miss busy cycles");
		}
}
#endif

#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
static int32 _rtl8651_startCOP3Counters(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(strcmp("inst", nextToken)==0){
		startCOP3Counters(TRUE);
		}
	else if(strcmp("data", nextToken)==0){
		startCOP3Counters(FALSE);
		}
	else if(strcmp("end", nextToken)==0){
		stopCOP3Counters();
		}
	else
		return FAILED;

	return SUCCESS;
}


 static int32 _rtl8651_permemcpyCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	int8 *pSrcBuffer;
	int8 *pDestBuffer;
	uint32 start, length, iteration;
	int32 i;
	int32 cop3i, cop3d;

	cop3i = cop3d = FALSE;
	pSrcBuffer = (int8*)((uint32)srcBuffer | 0x20000000UL);
	pDestBuffer = (int8*)((uint32)destBuffer | 0x20000000UL);
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(!size){
		if(strcmp("cached", nextToken)==0){
			pSrcBuffer = srcBuffer;
			pDestBuffer = destBuffer;
			cle_getNextCmdToken(&nextToken,&size,saved); 
			}
		if(strcmp("COP3I", nextToken)==0){
			cop3i = TRUE;
			cle_getNextCmdToken(&nextToken,&size,saved); 
			}
		else if(strcmp("COP3D", nextToken)==0){
			cop3d = TRUE;
			cle_getNextCmdToken(&nextToken,&size,saved); 
			}
	}
	length = U32_value(nextToken);
	if(length>sizeof(destBuffer))
		length=sizeof(destBuffer);
	cle_getNextCmdToken(&nextToken,&size,saved); 
	iteration = U32_value(nextToken);
	if(cop3i || cop3d)
		startCOP3Counters(cop3i);
	start = jiffies;
	for(i=0;i<iteration;i++)
		memcpy(pDestBuffer, pSrcBuffer, length);
	rtlglue_printf("(each 8K)Src: %08x, Dst: %08x\n", (uint32)pSrcBuffer, (uint32)pDestBuffer);
	rtlglue_printf("Copy %d * %d times. Total %d bytes, %d ms.  %u KBps\n", length, iteration, (uint32)(length*iteration),(uint32)((jiffies-start)*10), (uint32)((length*iteration))/(10*((uint32)(jiffies-start))));
	if(cop3i || cop3d)
		stopCOP3Counters();

	return SUCCESS;
}


 static int32 _rtl8651_pernicrxCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	int32 instMode;

	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(strcmp("inst", nextToken)==0)
		instMode = TRUE;
	else if(strcmp("data", nextToken)==0)
		instMode = FALSE;
	else
		return FAILED;

	cle_getNextCmdToken(&nextToken,&size,saved);
	swNic_pernicrxStart(instMode, U32_value(nextToken));
	return SUCCESS;
}


 static int32 _rtl8651_perpcitxCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	int32 instMode;

	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(strcmp("inst", nextToken)==0)
		instMode = TRUE;
	else if(strcmp("data", nextToken)==0)
		instMode = FALSE;
	else if(strcmp("start", nextToken)==0){
		rtlglue_perpcitxGo();
		return SUCCESS;
		}
	else
		return FAILED;

	cle_getNextCmdToken(&nextToken,&size,saved);
	rtlglue_perpcitxStart(instMode, U32_value(nextToken));
	return SUCCESS;
}
#endif


#if defined(CONFIG_RTL865X_MULTILAYER_BSP) && defined(CONFIG_RTL865X_ROMEDRV)
static int32 _rtl8651_debugCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size, level;
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(size){
		level = U32_value(nextToken);
	}else if(strcmp(nextToken, "on") == 0) 
		level=1;
	else
		level=0;
	wlan_acc_debug=level;
	return SUCCESS;
}

static int32	_rtl8651_resetCmd(uint32 userId,  int32 argc,int8 **saved){
	return rtl8651_tblDrvReset();
}
#endif /*CONFIG_RTL865X_MULTILAYER_BSP*/

#ifdef RTL865X_MODEL_USER
static int32	cmdPci(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	uint32 *startaddr;
	uint32 value,value2;
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(strcmp(nextToken, "read") == 0) {
		cle_getNextCmdToken(&nextToken,&size,saved); //base address
		startaddr = (uint32*)U32_value(nextToken);
		cle_getNextCmdToken(&nextToken,&size,saved); //length

		value = (uint32)U32_value(nextToken);
		
		if(value==1)
		{
			u8 val;
			val=rtl865x_pci_ioread8((uint32)startaddr);
			rtlglue_printf("REG8(%x)=%02x\n",(uint32)startaddr,val);			
		}
		else if(value==2)
		{
			u16 val;
			val=rtl865x_pci_ioread16((uint32)startaddr);
			rtlglue_printf("REG16(%x)=%04x\n",(uint32)startaddr,val);			
		}
		else if(value==4)
		{
			u32 val;
			val=rtl865x_pci_ioread32((uint32)startaddr);
			rtlglue_printf("REG32(%x)=%08x\n",(uint32)startaddr,val);			
		}
		
		


	}else {

		cle_getNextCmdToken(&nextToken,&size,saved); //base address
		startaddr = (uint32 *)U32_value(nextToken);
		
		cle_getNextCmdToken(&nextToken,&size,saved);
		value = (uint32)U32_value(nextToken);

		cle_getNextCmdToken(&nextToken,&size,saved);
		value2 = (uint32)U32_value(nextToken);
		

		
		if(value2==1)
		{
			u8 val;
			rtl865x_pci_iowrite8((uint32)startaddr,value);
			val=rtl865x_pci_ioread8((uint32)startaddr);
			rtlglue_printf("REG8(%x)=%02x\n",(uint32)startaddr,val);			
		}
		else if(value2==2)
		{
			u16 val;
			rtl865x_pci_iowrite16((uint32)startaddr,value);
			val=rtl865x_pci_ioread16((uint32)startaddr);
			rtlglue_printf("REG16(%x)=%04x\n",(uint32)startaddr,val);			
		}
		else if(value2==4)
		{
			u32 val;
			rtl865x_pci_iowrite32((uint32)startaddr,value);
			val=rtl865x_pci_ioread32((uint32)startaddr);
			rtlglue_printf("REG32(%x)=%08x\n",(uint32)startaddr,val);			
		}		
		
	}

	return SUCCESS;
}
#endif


#ifndef RTK_X86_CLE//RTK-CNSD2-NickWu-20061222: for x86 compile
static int32	_rtl8651_memdump(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	uint32 *startaddr, *addr;
	uint32 len, value;

	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(strcmp(nextToken, "read") == 0) {
		cle_getNextCmdToken(&nextToken,&size,saved); //base address
		startaddr = (uint32*)U32_value(nextToken);
		cle_getNextCmdToken(&nextToken,&size,saved); //length
		len=U32_value(nextToken);

	}else {
		len=0;
		cle_getNextCmdToken(&nextToken,&size,saved); //base address
		addr=startaddr = (uint32 *)U32_value(nextToken);
		//while(
		cle_getNextCmdToken(&nextToken,&size,saved);// !=FAILED){
			value = (uint32)U32_value(nextToken);
			WRITE_MEM32( (uint32)addr, value );
			len+=4;
			addr++;
		//}
	}

#if defined(RTL865X_MODEL_USER) || defined(RTL865X_MODEL_KERNEL)
	/* According to asicRegs.h's pVirtualSWReg allocation, re-map the following area. */
	if ( ( (uint32)startaddr >= REAL_SWCORE_BASE ) && ( (uint32)startaddr < REAL_SWCORE_BASE+SWCORE_SIZE ) )
	{
		startaddr = (uint32*)(((uint32)startaddr)-REAL_SWCORE_BASE+SWCORE_BASE);
	}
#ifdef CONFIG_RTL865XB
	else if ( ( (uint32)startaddr >= REAL_BD01_BASE ) && ( (uint32)startaddr <= REAL_BD01_BASE+BD01_SIZE ) )
	{
		startaddr = (uint32*)(((uint32)startaddr)-REAL_BD01_BASE+BD01_BASE);
	}
#else /*CONFIG_RTL865XC*/
	else if ( ( (uint32)startaddr >= REAL_SYSTEM_BASE ) && ( (uint32)startaddr <= REAL_SYSTEM_BASE+SYSTEM_SIZE ) )
	{
		startaddr = (uint32*)(((uint32)startaddr)-REAL_SYSTEM_BASE+SYSTEM_BASE);
	}
#endif
	else if ( ( (uint32)startaddr >= REAL_HSB_BASE ) && ( (uint32)startaddr <= REAL_HSB_BASE+HSB_SIZE ) )
	{
		startaddr = (uint32*)(((uint32)startaddr)-REAL_HSB_BASE+HSB_BASE);
	}
	else if ( ( (uint32)startaddr >= REAL_HSA_BASE ) && ( (uint32)startaddr <= REAL_HSA_BASE+HSA_SIZE ) )
	{
		startaddr = (uint32*)(((uint32)startaddr)-REAL_HSA_BASE+HSA_BASE);
	}
#endif

	memDump(startaddr, len, "Result");
	return SUCCESS;
}
#endif


#if defined (CONFIG_RTL8185) || defined (CONFIG_RTL8185B)
/* ====================================================================

		RTK 8185 setting


    ==================================================================== */
/* Board Information */
bdinfo_t currentBdInfo;

/* EEPROM Access */
typedef struct _rtl8651_8185eeprom_info_s {
	char* itemName;
	char *itemData;
	int32 itemDataSize;
	int32 itemDataTotalSize;
} _rtl8651_8185eeprom_info_t;

char _rtl8651_8185eepromData_devName[20];	/* string to store Device Name in EEPROM data cache */
#define _RTL8651_8185EEPROM_SETDEVNAME(name)				\
do {																\
	memset(	_rtl8651_8185eepromData_devName,				\
				0,												\
				sizeof(_rtl8651_8185eepromData_devName));		\
	if (strlen(name) < sizeof(_rtl8651_8185eepromData_devName))	\
	{															\
		strncpy(	_rtl8651_8185eepromData_devName,				\
				name,											\
				sizeof(_rtl8651_8185eepromData_devName));		\
	}															\
} while (0)
#define _RTL8651_8185EEPROM_CHECKDEVNAME(name)						\
	(	(name) &&														\
		(strlen(name) == strlen(_rtl8651_8185eepromData_devName)) &&	\
		(strcmp(name, _rtl8651_8185eepromData_devName) == 0)			)

char _rtl8651_8185eepromData_RFChipID[10];
char _rtl8651_8185eepromData_Mac[10];
char _rtl8651_8185eepromData_TxPowerCCK[32];
char _rtl8651_8185eepromData_TxPowerOFDM[512];

_rtl8651_8185eeprom_info_t _rtl8651_8185eeprom[] =
{
	{"RFChipID",		_rtl8651_8185eepromData_RFChipID,			0,	sizeof(_rtl8651_8185eepromData_RFChipID)},
	{"Mac",			_rtl8651_8185eepromData_Mac,				0,	sizeof(_rtl8651_8185eepromData_Mac)},
	{"TxPowerCCK",	_rtl8651_8185eepromData_TxPowerCCK,		0,	sizeof(_rtl8651_8185eepromData_TxPowerCCK)},
	{"TxPowerOFDM",	_rtl8651_8185eepromData_TxPowerOFDM,		0,	sizeof(_rtl8651_8185eepromData_TxPowerOFDM)},
	{"\0",			NULL}	/* Last item */
};


#undef RTL8651_8185_DUMP_EEPROM

static int32 _rtl8651_8185_fetch_EEPROM_info(struct net_device *dev);
static _rtl8651_8185eeprom_info_t *_rtl8651_8185_get_EEPROM_info(struct net_device *dev, char *itemName);
#ifdef RTL8651_8185_DUMP_EEPROM
static int32 _rtl8651_8185_dump_EEPROM_info(struct net_device *dev);
#endif

static void _rtl8651_8185_atof(char *string, int32 *integerPart_p, int32 *decimalPart_p)
{
	int isIntegerPart = 1; /* isIntegerPart == 1: Integer Part , isIntegerPart == 0: decimal Part */
	int32 integerPart = 0;
	int32 decimalPart = 0;
	char *str_p;

	str_p = string;

	while (	isdigit(*str_p) ||
			(*str_p == '.'))
	{
		/*
			Process '.'
		*/
		if (*str_p == '.')
		{
			if (isIntegerPart == 1)
			{
				isIntegerPart = 0;
			} else
			{
				goto errout;
			}
		} else
		{
			int32 digit;
			/*
				Process digit
			*/
			digit = (int32)((unsigned char)(*str_p) - (unsigned char)('0'));

			if (isIntegerPart == 1)
			{
				integerPart = (integerPart * 10) + digit;
			} else
			{
				decimalPart = (decimalPart * 10) + digit;
			}
		}
		/* Next */
		str_p ++;
	}

	goto out;

errout:
	integerPart = 0;
	decimalPart = 0;

out:

	if (integerPart_p)
	{
		*integerPart_p = integerPart;
	}

	if (decimalPart_p)
	{
		*decimalPart_p = decimalPart;
	}

	return;
}

/*
	Fetch 8185 Data from EEPROM

	Return value:
		0	:	Access SUCCESS
		-1	:	Access FAILED
*/
static int32 _rtl8651_8185_fetch_EEPROM_info(struct net_device *dev)
{
	_rtl8651_8185eeprom_info_t *eepromItem_p = _rtl8651_8185eeprom;
	int32 read_len;
	int32 rtl8185_wlanDrvVersion_integerPart;
	int32 rtl8185_wlanDrvVersion_decimalPart;

	_RTL8651_8185EEPROM_SETDEVNAME("\0");

	if ((dev == NULL) || (dev->priv == NULL))
	{
		goto errout;
	}

	_rtl8651_8185_atof(	WLANDRVPKG_VERSION,
						&rtl8185_wlanDrvVersion_integerPart,
						&rtl8185_wlanDrvVersion_decimalPart);

	if (	(rtl8185_wlanDrvVersion_integerPart > 1) ||
		(rtl8185_wlanDrvVersion_decimalPart >= 12)	)
	{
		/*
			New 8185 driver : we use new method to get information from Driver.
		*/

		eepromItem_p = _rtl8651_8185eeprom;

		while( eepromItem_p->itemData != NULL )	/* last item */
		{
			memset(	eepromItem_p->itemData,
						0,
						eepromItem_p->itemDataTotalSize);

			eepromItem_p->itemDataSize = 0;

			/* Follow RTL8185 driver's convention to get EEPROM data: Copy ItemName into ptr to get data */
			if (strlen(eepromItem_p->itemName) >= eepromItem_p->itemDataTotalSize)
			{
				rtlglue_printf("[%s:%d] ERROR: item [%s] process failed.\n", __FUNCTION__, __LINE__, eepromItem_p->itemName);
				goto errout;
			}

			strncpy(	eepromItem_p->itemData,
					eepromItem_p->itemName,
					strlen(eepromItem_p->itemName));

			if((read_len = get_eeprom_info(dev->priv, eepromItem_p->itemData)) <= 0)
			{
				goto errout;
			} else if (read_len > eepromItem_p->itemDataTotalSize)
			{
				rtlglue_printf("[%s:%d] FATAL:\n\tEEPROM READ of [%s] FAILED: (Read Len: %d, Buff Size: %d).\n",
							__FUNCTION__,
							__LINE__,
							eepromItem_p->itemName,
							read_len,
							eepromItem_p->itemDataTotalSize);
				goto errout;
			}

			eepromItem_p->itemDataSize = read_len;

			/* Next */
			eepromItem_p ++;
		}

	}else
	{
		/*
			Old 8185 driver : we use old method to get information from Driver.
		*/

		char _eepromData[300];
		#define RTL8185_EEPROM_CHIPID_OFFSET			0
		#define RTL8185_EEPROM_CHIPID_LEN				1
		#define RTL8185_EEPROM_MAC_OFFSET			1
		#define RTL8185_EEPROM_MAC_LEN				6
		#define RTL8185_EEPROM_CCKPOWER_OFFSET		7
		#define RTL8185_EEPROM_CCKPOWER_LEN			14
		#define RTL8185_EEPROM_OFDMPOWER_OFFSET	21
		#define RTL8185_EEPROM_OFDMPOWER_LEN		162

		memset(_eepromData, 0, sizeof(_eepromData));
		read_len = get_eeprom_info(dev->priv, _eepromData);
		if (read_len == 0)
		{
			goto errout;
		} else if (read_len > sizeof(_eepromData))
		{
			rtlglue_printf("[%s:%d] FATAL:\n\tEEPROM READ of [%s] FAILED: (Read Len: %d, Buff Size: %d).\n",
						__FUNCTION__,
						__LINE__,
						eepromItem_p->itemName,
						read_len,
						eepromItem_p->itemDataTotalSize);
			goto errout;
		}

		eepromItem_p = _rtl8651_8185eeprom;

		while( eepromItem_p->itemData != NULL )	/* last item */
		{
			memset(	eepromItem_p->itemData,
						0,
						eepromItem_p->itemDataTotalSize);

			eepromItem_p->itemDataSize = 0;

			/* RFChipID */
			if (	(strlen("RFChipID") == strlen(eepromItem_p->itemName)) &&
				(strcmp("RFChipID", eepromItem_p->itemName) == 0)	)
			{
				memcpy(	eepromItem_p->itemData,
							&(_eepromData[RTL8185_EEPROM_CHIPID_OFFSET]),
							RTL8185_EEPROM_CHIPID_LEN);
				eepromItem_p->itemDataSize = RTL8185_EEPROM_CHIPID_LEN;
			}
			/* MAC */
			if (	(strlen("Mac") == strlen(eepromItem_p->itemName)) &&
				(strcmp("Mac", eepromItem_p->itemName) == 0)	)
			{
				memcpy(	eepromItem_p->itemData,
							&(_eepromData[RTL8185_EEPROM_MAC_OFFSET]),
							RTL8185_EEPROM_MAC_LEN);
				eepromItem_p->itemDataSize = RTL8185_EEPROM_MAC_LEN;
			}
			/* CCK */
			if (	(strlen("TxPowerCCK") == strlen(eepromItem_p->itemName)) &&
				(strcmp("TxPowerCCK", eepromItem_p->itemName) == 0)	)
			{
				memcpy(	eepromItem_p->itemData,
							&(_eepromData[RTL8185_EEPROM_CCKPOWER_OFFSET]),
							RTL8185_EEPROM_CCKPOWER_LEN);
				eepromItem_p->itemDataSize = RTL8185_EEPROM_CCKPOWER_LEN;
			}
			/* OFDM */
			if (	(strlen("TxPowerOFDM") == strlen(eepromItem_p->itemName)) &&
				(strcmp("TxPowerOFDM", eepromItem_p->itemName) == 0)	)
			{
				memcpy(	eepromItem_p->itemData,
							&(_eepromData[RTL8185_EEPROM_OFDMPOWER_OFFSET]),
							RTL8185_EEPROM_OFDMPOWER_LEN);
				eepromItem_p->itemDataSize = RTL8185_EEPROM_OFDMPOWER_LEN;
			}

			/* Next */
			eepromItem_p ++;
		}
	}

	_RTL8651_8185EEPROM_SETDEVNAME(dev->name);

	return 0;

errout:

	return -1;
}

/*
	Find 8185 EEPROM data cache
*/
static _rtl8651_8185eeprom_info_t *_rtl8651_8185_get_EEPROM_info(struct net_device *dev, char *itemName)
{
	_rtl8651_8185eeprom_info_t *eepromItem_p = NULL;

	if (	(dev == NULL) ||
		(dev->priv == NULL) ||
		(itemName == NULL))
	{
		goto out;
	}

	if (	(! _RTL8651_8185EEPROM_CHECKDEVNAME(dev->name) ) &&
		(_rtl8651_8185_fetch_EEPROM_info(dev) != 0)	)
	{
		goto out;
	}

	eepromItem_p = _rtl8651_8185eeprom;

	while( eepromItem_p->itemData != NULL )	/* last item */
	{
		if (	(strlen(itemName) == strlen(eepromItem_p->itemName)) &&
			(strcmp(itemName, eepromItem_p->itemName) == 0)	)
		{
			/* match ! */
			goto out;
		}

		/* Next */
		eepromItem_p ++;
	}

	/* Not found */
	eepromItem_p = NULL;

out:
	return eepromItem_p;
}

/*
	Dump 8185 EEPROM data in cache
*/
#ifdef RTL8651_8185_DUMP_EEPROM
static int32 _rtl8651_8185_dump_EEPROM_info(struct net_device *dev)
{
	_rtl8651_8185eeprom_info_t *eepromItem_p;

	if ((dev == NULL) || (dev->priv == NULL))
	{
		goto errout;
	}

	if (	(! _RTL8651_8185EEPROM_CHECKDEVNAME(dev->name) ) &&
		(_rtl8651_8185_fetch_EEPROM_info(dev) != 0)	)
	{
		goto errout;
	}

	eepromItem_p = _rtl8651_8185eeprom;

	rtlglue_printf("[ 8185 EEPROM Info for <%s>]\n", dev->name);
	rtlglue_printf("=====================================================\n");
	while( eepromItem_p->itemData != NULL )	/* last item */
	{
		rtlglue_printf("[ %s ] : Size %d\n", eepromItem_p->itemName, eepromItem_p->itemDataSize);

		if (eepromItem_p->itemDataSize > 0)
		{
			int32 idx;
			int32 oneRowCnt;

			rtlglue_printf("* ------------------------------------------------------- *\n");

			for (idx = 0, oneRowCnt = 0 ; idx < eepromItem_p->itemDataSize ; idx ++)
			{
				rtlglue_printf("0x%02x ", eepromItem_p->itemData[idx]);

				/* change ROW */
				oneRowCnt ++;
				if (oneRowCnt >= 10)
				{
					oneRowCnt = 0;
					rtlglue_printf("\n");
				}
			}
			/* Change line after all data are printed : check "oneRowCnt" to prevent from duplicate "new-line" */
			if (oneRowCnt != 0)
			{
				rtlglue_printf("\n");
			}
		}

		/* Next */
		eepromItem_p ++;
	}
	rtlglue_printf("=====================================================\n");

	return 0;

errout:
	return -1;
}
#endif

/* RF Chip */
#define RTL8185_RFCHIP_OMC8255		0x0a
#define RTL8185_RFCHIP_ZEBRA8225		0x09

static int32 _rtl8651_8185flashCfgCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size, i;
	uint32 value;
	uint32 cardIndex;
	int chIndex;
	uint16 RFChipID = 0;
	struct net_device *dev;
	uint8 eepromExist;
	char tmpbuf[32];

	cle_getNextCmdToken(&nextToken,&size,saved); 
	cardIndex = U32_value(nextToken);
	cle_getNextCmdToken(&nextToken,&size,saved);

	sprintf(tmpbuf, "wlan%d", cardIndex);
	dev=__dev_get_by_name(tmpbuf);

	if (dev == NULL)
	{
		rtlglue_printf("@ Unknown interface: wlan%d\n", cardIndex);
		return FAILED;
	}

	if (cardIndex > 2)
	{
		rtlglue_printf("@ Unknown wlan Index : %d\n", cardIndex);
		return FAILED;
	}

	/* ========================================================================
			Get EEPROM / BDInfo
	    ======================================================================== */

	/* get information from EEPROM */
	eepromExist = TRUE;

	if (_rtl8651_8185_fetch_EEPROM_info(dev) != 0)
	{	/* whole eepromData are the initial value --> EEPROM access fail ! */
		rtlglue_printf("@ EEPROM access failed!\n");
		eepromExist = FALSE;
	}

	/* dump EEPROM infomation */
#ifdef RTL8651_8185_DUMP_EEPROM
	_rtl8651_8185_dump_EEPROM_info(dev);
#endif

	/* get information from BDINFO */
	memset(&currentBdInfo, 0xff, sizeof(bdinfo_t));
	flashdrv_read((void *)&currentBdInfo, (void *)flashdrv_getBoardInfoAddr(), sizeof(bdinfo_t));

	/* check result */
	{
		bdinfo_t invalid;
		memset(&invalid, 0xff, sizeof(bdinfo_t));
		if (memcmp(&invalid, &currentBdInfo, sizeof(bdinfo_t)) == 0)
		{
			rtlglue_printf("@ FATAL: BoradInfo access failed!\n");
			return FAILED;
		}
	}

	/* ========================================================================
			Get Chip Version and ID
				1. From WLAN's EEPROM
				2. If (1) is failed, from bdInfo
	    ======================================================================== */

	if (eepromExist == TRUE)
	{
		_rtl8651_8185eeprom_info_t *_rtl8185_eepromRFChipIDData_p;
		if (	(_rtl8185_eepromRFChipIDData_p =
				_rtl8651_8185_get_EEPROM_info(	dev,
												"RFChipID"))
			!= NULL	)
		{
			RFChipID = _rtl8185_eepromRFChipIDData_p->itemData[0] & 0x0f;
		} else
		{
			rtlglue_printf("[FATAL] (%s:%d) RF Chip ID Load FAILED.\n", __FUNCTION__, __LINE__);
		}
	} else
	{
		RFChipID = RTL8185_RFCHIP_ZEBRA8225;
	}

	rtlglue_printf("@ Interface: wlan%d\n", cardIndex);

	rtlglue_printf("@ RF ChipID: %d ", RFChipID);

	switch (RFChipID)
	{
		case RTL8185_RFCHIP_ZEBRA8225:
			rtlglue_printf("[Zebra - RTL8225]\n");
			break;
		case RTL8185_RFCHIP_OMC8255:
			rtlglue_printf("[OMC - RTL8255]\n");
			break;
		default:
			rtlglue_printf("[Unknown RF chip]\n");
	}

	rtlglue_printf("@ -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n");

	/* ========================================================================
	     ********************************************************************************************

		Set factory default:

			Read configuration from EEPROM/code and store it to flash.

	    *********************************************************************************************
	    ======================================================================== */
	if(!strcmp(nextToken, "default"))
	{
		#define RTL8185_DEFAULTFROMUSER_ENABLE	/* default value use per-defined setting instead of information from EEPROM */

		#define RTL8185_DEFAULT_FROM_EEPROM		0x01
		#define RTL8185_DEFAULT_FROM_USER		0x02

		uint8 rtl8185_defaultValueSource = RTL8185_DEFAULT_FROM_EEPROM;
		uint8 updateBDInfo = FALSE;
		bdinfo_wlan_t def;

		_rtl8651_8185eeprom_info_t *_rtl8185_eepromMacData_p = NULL;
		_rtl8651_8185eeprom_info_t *_rtl8185_eepromOFDMData_p = NULL;
		_rtl8651_8185eeprom_info_t *_rtl8185_eepromCCKData_p = NULL;

		/*=====================================================================
				TxPower Management
		   =====================================================================*/
		typedef enum{
			minTxPower = 0,
			maxTxPower,
			TxPowerRange,
		} _minMax;

		typedef enum{
			_24Gcck = 0,
			_24Gofdm,
			_5Gofdm,
			AllSpectrumNum,
		} _Spectrum;

		uint8 updateTxPower[AllSpectrumNum];	/* for TxPower update check */
		uint8 txPower[AllSpectrumNum];
		uint8 oldTxPower[AllSpectrumNum];

		#define INIT_TXPOWER(RF_ID)\
			do { \
				memset(updateTxPower, 0, sizeof(updateTxPower));\
				memset(oldTxPower, 0xff, sizeof(oldTxPower));\
				switch (RF_ID)\
				{\
					case RTL8185_RFCHIP_OMC8255:\
						memcpy(txPower, omc8255TxPowerDefault, sizeof(txPower));\
						break;\
					case RTL8185_RFCHIP_ZEBRA8225:\
						memcpy(txPower, zebra8225TxPowerDefault, sizeof(txPower));\
						break;\
				}\
			} while (0);

		#define UPDATE_TXPOWER(newTxPower, Spectrum) \
			do {	\
				if (oldTxPower[Spectrum] != 0xff)\
				{\
					updateTxPower[Spectrum] = TRUE; \
				}\
				oldTxPower[Spectrum] = txPower[Spectrum]; \
				txPower[Spectrum] = (newTxPower); \
			} while (0);

		#define TXPOWER(Spectrum) (txPower[(Spectrum)])
		#define OLD_TXPOWER(Spectrum) (oldTxPower[(Spectrum)])
		#define TXPOWER_IS_UPDATED(Spectrum) (updateTxPower[Spectrum]==TRUE)

		/* default TxPower */
		/*==================================================================*/
		/* 8225 */
#ifdef CONFIG_RTL8185B
		const uint8 zebra8225TxPowerDefault[AllSpectrumNum] =
		{
			0x0f,			/* 2.4G CCK		*/
			0x1a,			/* 2.4G OFDM		*/
		};
#else
		const uint8 zebra8225TxPowerDefault[AllSpectrumNum] =
		{
			0x0c,			/* 2.4G CCK		*/
			0x12,			/* 2.4G OFDM		*/
		};
#endif
		/* 8255 */
		const uint8 omc8255TxPowerDefault[AllSpectrumNum] =
		{
			0x0f,			/* 2.4G CCK		*/
			0x12,			/* 2.4G OFDM		*/
			0x12,			/* 5G OFDM		*/
		};
		/*==================================================================*/


		/* Valid TxPower Range */
		/*==================================================================*/
		/* 8225 */
#ifdef CONFIG_RTL8185B
		const uint8 zebra8225TxPowerRange[AllSpectrumNum][TxPowerRange] =
		{/*	MIN		MAX		*/
			{0x00,	0x14},	/* 2.4G CCK		*/
			{0x00,	0x1d},	/* 2.4G OFDM		*/
			{0,		0}		/* 5G OFDM		*/			
		};
#else
		const uint8 zebra8225TxPowerRange[AllSpectrumNum][TxPowerRange] =
		{/*	MIN		MAX		*/
			{0x00,	0x2f},	/* 2.4G CCK		*/
			{0x00,	0x2f},	/* 2.4G OFDM		*/
			{0,		0}		/* 5G OFDM		*/			
		};
#endif
		/* 8255 */
		const uint8 omc8255TxPowerRange[AllSpectrumNum][TxPowerRange] =
		{/*	MIN		MAX		*/
			{0x03,	0x2f},	/* 2.4G CCK		*/
			{0x03,	0x2f},	/* 2.4G OFDM		*/
			{0x03,	0x2f}	/* 5G OFDM		*/
		};

		#define UPDATE_INVALID_TXPOWER(RF_ID, Spectrum) \
			do {\
				uint8 __minTxPower = 0;\
				uint8 __maxTxPower = 0xff;\
				uint8 __defaultTxPower = 0;\
				switch(RF_ID)\
				{\
					case RTL8185_RFCHIP_OMC8255:\
						__minTxPower = omc8255TxPowerRange[Spectrum][minTxPower];\
						__maxTxPower = omc8255TxPowerRange[Spectrum][maxTxPower];\
						__defaultTxPower = omc8255TxPowerDefault[Spectrum];\
						break;\
					case RTL8185_RFCHIP_ZEBRA8225:\
						__minTxPower = zebra8225TxPowerRange[Spectrum][minTxPower];\
						__maxTxPower = zebra8225TxPowerRange[Spectrum][maxTxPower];\
						__defaultTxPower = zebra8225TxPowerDefault[Spectrum];\
						break;\
				}\
				if (txPower[Spectrum]>__maxTxPower || txPower[Spectrum]<__minTxPower)\
				{\
					if (oldTxPower[Spectrum] != 0xff)\
					{\
						updateTxPower[Spectrum] = TRUE; \
					}\
					oldTxPower[Spectrum] = txPower[Spectrum]; \
					txPower[Spectrum] = __defaultTxPower; \
				}\
			}while(0);

		/*==================================================================*/

		memset(&def, 0, sizeof(bdinfo_wlan_t));

		/*
			check default source
		*/
		#ifdef RTL8185_DEFAULTFROMUSER_ENABLE
		nextToken = NULL;
		cle_getNextCmdToken(&nextToken, &size, saved);
		if ((nextToken == NULL) || (!strcmp(nextToken, "eeprom")))
		#endif	/* RTL8185_DEFAULTFROMUSER_ENABLE */
		{
			if (eepromExist==TRUE)
			{
				rtl8185_defaultValueSource = RTL8185_DEFAULT_FROM_EEPROM;
			} else
			{
				rtlglue_printf("@ EEPROM load fail, get default value from per-defined hard code\n");
				rtl8185_defaultValueSource = RTL8185_DEFAULT_FROM_USER;
			}
		}
		#ifdef RTL8185_DEFAULTFROMUSER_ENABLE
		else if (!strcmp(nextToken, "user"))
		{
			rtl8185_defaultValueSource = RTL8185_DEFAULT_FROM_USER;
		}
		#endif	/* RTL8185_DEFAULTFROMUSER_ENABLE */

		/*
			verify default source
		*/
		switch (rtl8185_defaultValueSource)
		{
			case RTL8185_DEFAULT_FROM_EEPROM:
			{
				rtlglue_printf("@ Load default cfg from EEPROM ... ");

				if (	((	_rtl8185_eepromMacData_p =
							_rtl8651_8185_get_EEPROM_info(	dev,
															"Mac")	)
						!= NULL) &&
					((	_rtl8185_eepromCCKData_p =
							_rtl8651_8185_get_EEPROM_info(	dev,
															"TxPowerCCK")	)
						!= NULL) &&
					((	_rtl8185_eepromOFDMData_p =
							_rtl8651_8185_get_EEPROM_info(	dev,
															"TxPowerOFDM")	)
						!= NULL)	)
				{
					rtlglue_printf("SUCCESS !\n");
				} else
				{
					rtlglue_printf("[FATAL] ERROR!\n");
				}
				break;
			}
			case RTL8185_DEFAULT_FROM_USER:
			{
				rtlglue_printf("@ Load default cfg from code default\n");
				break;
			}
			default:
				rtlglue_printf("[FATAL] (%s:%d) Error default value source.\n", __FUNCTION__, __LINE__);
				return 0;
		}

		/* ==================================================================
			Update Information
		    ==================================================================*/

		switch (RFChipID)
		{
			/*
				Zebra - b/g
			*/
			case RTL8185_RFCHIP_ZEBRA8225:
			{
				int32 i;
				uint8 defMac[6]={0,0xe0,0x4c,0,0x81,0x85};
				bdinfo_wlan_t def_8225 = {
					type: 0x81,
					version: 0x85,
					RFChipID: RTL8185_RFCHIP_ZEBRA8225,
					len: sizeof(bdinfo_wlan_t),
					rxChargePump:3,
					txChargePump:6,
				};

				memcpy(&def, &def_8225, sizeof(bdinfo_wlan_t));

				/*==================================
						Set RF Chip ID
				   ==================================*/
				if (RFChipID != def_8225.RFChipID)
				{
					updateBDInfo = TRUE;
				}

				/*==================================
						Set MAC Addr
				   ==================================*/
				switch (rtl8185_defaultValueSource)
				{
					case RTL8185_DEFAULT_FROM_EEPROM:
					{
						memcpy(	def.mac,
									_rtl8185_eepromMacData_p->itemData,
									sizeof(ether_addr_t));
						break;
					}
					case RTL8185_DEFAULT_FROM_USER:
					{
						memcpy(def.mac, defMac, sizeof(ether_addr_t));
						def.mac[3]=cardIndex;
						break;
					}
				}

				rtlglue_printf("@ Set %s default MAC as ", tmpbuf);

				for(i=0;i<6;i++)
				{
					rtlglue_printf("%02x",(unsigned char)def.mac[i]);
					if (i<5)
					{
						rtlglue_printf(":");
					}
				}
				rtlglue_printf("\n");

				/*==================================
						Set 2.4G TxPower

						- OFDM
						- CCK
				   ==================================*/

				rtlglue_printf("====================== 2.4G ======================\n");
				for(i=0;i< BDINFO_24GRF_CHANNELS;i++)
				{
					INIT_TXPOWER(RFChipID);	/* we MUST re-init database before processing each entry */
					/* 802.11g */
					switch (rtl8185_defaultValueSource)
					{
						case RTL8185_DEFAULT_FROM_EEPROM:
						{
							/* CCK */
							UPDATE_TXPOWER(
								_rtl8185_eepromCCKData_p->itemData[i],
								_24Gcck
							);
							/*
								check the correctness of the information gotten from EEPROM,
								if it is invalid, we set the from user.
							*/
							UPDATE_INVALID_TXPOWER(RFChipID, _24Gcck);

							/* OFDM */
							UPDATE_TXPOWER(
								_rtl8185_eepromOFDMData_p->itemData[i],
								_24Gofdm
							);
							/*
								check the correctness of the information gotten from EEPROM,
								if it is invalid, we set the from user.
							*/
							UPDATE_INVALID_TXPOWER(RFChipID, _24Gofdm);
							break;
						}
						case RTL8185_DEFAULT_FROM_USER:
						{
							INIT_TXPOWER(RFChipID);		/* this macro would init TxPower according to RFChipID */
							break;
						}
					}

					{
						int cntloop, pwr;
						for (cntloop=0;cntloop<4;cntloop++)
						{
							pwr=TXPOWER(_24Gofdm)-(3*cntloop);
							def.txPower[i][cntloop]=pwr>0?pwr:0; /* strongest */
							pwr=TXPOWER(_24Gcck)-(3*cntloop);
							def.cckTxPower[i][cntloop]=pwr>0?pwr:0; /* strongest */
							
						}
					}
#if 0
					def.txPower[i][0]=TXPOWER(_24Gofdm); /* strongest */
					def.txPower[i][1]=TXPOWER(_24Gofdm)-1;
					def.txPower[i][2]=TXPOWER(_24Gofdm)-2;
					def.txPower[i][3]=TXPOWER(_24Gofdm)-3;
					def.cckTxPower[i][0]=TXPOWER(_24Gcck); /* strongest */
					def.cckTxPower[i][1]=TXPOWER(_24Gcck);
					def.cckTxPower[i][2]=TXPOWER(_24Gcck)-1;
					def.cckTxPower[i][3]=TXPOWER(_24Gcck)-2;
#endif
					rtlglue_printf("[%d] 2.4G Ch %d: OFDM %d,%d,%d,%d%s  CCK %d,%d,%d,%d%s\n",
						i,	/* index */
						i+1,
						def.txPower[i][0],def.txPower[i][1],def.txPower[i][2],def.txPower[i][3],
						(TXPOWER_IS_UPDATED(_24Gofdm)?"*":""),
						def.cckTxPower[i][0],def.cckTxPower[i][1],def.cckTxPower[i][2],def.cckTxPower[i][3],
						(TXPOWER_IS_UPDATED(_24Gcck)?"*":"")
					);
				}
				rtlglue_printf("\n");

				updateBDInfo = TRUE;
				break;
			}

			/*
				OMC, have a or b/g
			*/
			case RTL8185_RFCHIP_OMC8255:
			{
				int32 i,j;
				uint8 defMac[6]={0,0xe0,0x4c,0,0x81,0x85};
				
				bdinfo_wlan_t def_8255 = {
					type: 0x81,
					version: 0x85,
					RFChipID: RTL8185_RFCHIP_OMC8255,
					len: sizeof(bdinfo_wlan_t),
				};

				memcpy(&def, &def_8255, sizeof(bdinfo_wlan_t));

				/*==================================
						Set RF Chip ID
				   ==================================*/
				if (RFChipID != def_8255.RFChipID)
				{
					updateBDInfo = TRUE;
				}

				/*==================================
						Set MAC Addr
				   ==================================*/
				switch (rtl8185_defaultValueSource)
				{
					case RTL8185_DEFAULT_FROM_EEPROM:
					{
						memcpy(	def.mac,
									_rtl8185_eepromMacData_p->itemData,
									sizeof(ether_addr_t));
						break;
					}
					case RTL8185_DEFAULT_FROM_USER:
					{
						memcpy(def.mac, defMac, sizeof(ether_addr_t));
						def.mac[3]=cardIndex;
						break;
					}
				}

				rtlglue_printf("@ Set %s default MAC as ", tmpbuf);

				for(i=0;i<6;i++)
				{
					rtlglue_printf("%02x",(unsigned char)def.mac[i]);
					if (i<5)
					{
						rtlglue_printf(":");
					}
				}
				rtlglue_printf("\n");

				/*==================================
						Set 2.4G TxPower

						- OFDM
						- CCK
				   ==================================*/
				/*
					For 802.11 b/g 2.4G spectrum
				*/
				rtlglue_printf("====================== 2.4G ======================\n");
				for(i=0;i< BDINFO_24GRF_CHANNELS;i++)
				{
					INIT_TXPOWER(RFChipID);	/* we MUST re-init database before processing each entry */
					/* 802.11g */
					switch (rtl8185_defaultValueSource)
					{
						case RTL8185_DEFAULT_FROM_EEPROM:
						{
							/* CCK */
							UPDATE_TXPOWER(
								_rtl8185_eepromCCKData_p->itemData[i],
								_24Gcck
							);
							/*
								check the correctness of the information gotten from EEPROM,
								if it is invalid, we set the from user.
							*/
							UPDATE_INVALID_TXPOWER(RFChipID, _24Gcck);

							/* OFDM */
							UPDATE_TXPOWER(
								_rtl8185_eepromOFDMData_p->itemData[i],
								_24Gofdm
							);
							/*
								check the correctness of the information gotten from EEPROM,
								if it is invalid, we set the from user.
							*/
							UPDATE_INVALID_TXPOWER(RFChipID, _24Gofdm);
							break;
						}
						case RTL8185_DEFAULT_FROM_USER:
						{
							INIT_TXPOWER(RFChipID);		/* this macro would init TxPower according to RFChipID */
							break;
						}
					}

					def.txPower[i][0]=TXPOWER(_24Gofdm); /* strongest */
					def.txPower[i][1]=TXPOWER(_24Gofdm)+1;
					def.txPower[i][2]=TXPOWER(_24Gofdm)+2;
					def.txPower[i][3]=TXPOWER(_24Gofdm)+3;
					def.cckTxPower[i][0]=TXPOWER(_24Gcck); /* strongest */
					def.cckTxPower[i][1]=TXPOWER(_24Gcck);
					def.cckTxPower[i][2]=TXPOWER(_24Gcck)-1;
					def.cckTxPower[i][3]=TXPOWER(_24Gcck)-2;
					rtlglue_printf("[%d] 2.4G Ch %d: OFDM %d,%d,%d,%d%s  CCK %d,%d,%d,%d%s\n",
						i,	/* index */
						i+1,
						def.txPower[i][0],def.txPower[i][1],def.txPower[i][2],def.txPower[i][3],
						(TXPOWER_IS_UPDATED(_24Gofdm)?"*":""),
						def.cckTxPower[i][0],def.cckTxPower[i][1],def.cckTxPower[i][2],def.cckTxPower[i][3],
						(TXPOWER_IS_UPDATED(_24Gcck)?"*":"")
					);
				}
				rtlglue_printf("\n");

				/*==================================
						Set 5G TxPower

						- OFDM
				   ==================================*/
				/*
					For 802.11a 5G spectrum
				*/
				rtlglue_printf("======================  5G  ======================\n");

				for(i=BDINFO_24GRF_CHANNELS, j=0;i< BDINFO_24GRF_CHANNELS+BDINFO_5GRF_CHANNELS;i++,j++)
				{
					int ch = 36;

					INIT_TXPOWER(RFChipID);	/* we MUST re-init database before processing each entry */

					ch = 36 + (j * 4);
					switch (rtl8185_defaultValueSource)
					{
						case RTL8185_DEFAULT_FROM_EEPROM:
						{
							/* OFDM */
							UPDATE_TXPOWER(
								_rtl8185_eepromOFDMData_p->itemData[ch - 1],
								_5Gofdm
							);
							/*
								check the correctness of the information gotten from EEPROM,
								if it is invalid, we set the from user.
							*/
							UPDATE_INVALID_TXPOWER(RFChipID, _5Gofdm);
							break;
						}
						case RTL8185_DEFAULT_FROM_USER:
						{
							/*
								chenyl (2005/05/14):
									Jimmy said that when TxPower of 11a OFDM is 0x12,
									the TX performance will be stable and good.
									So I would apply this value in our platform's minimum range's setting (It's the default).
							*/
							INIT_TXPOWER(RFChipID);		/* this macro would init TxPower according to RFChipID */
							break;
						}
					}

					/* set TxPower level */
					def.txPower[i][0]=TXPOWER(_5Gofdm);		/* strongest */
					def.txPower[i][1]=TXPOWER(_5Gofdm)+1;
					def.txPower[i][2]=TXPOWER(_5Gofdm)+2;
					def.txPower[i][3]=TXPOWER(_5Gofdm)+3;

					/* 5G spectrum has no CCK */

					rtlglue_printf("[%d] 5G Ch %d: OFDM %d,%d,%d,%d%s\n",
						i, /* index */
						ch,
						def.txPower[i][0],def.txPower[i][1],def.txPower[i][2],def.txPower[i][3],
						(TXPOWER_IS_UPDATED(_5Gofdm)?"*":"")
					);
				}
				rtlglue_printf("\n");
				updateBDInfo = TRUE;
				break;
			}

			default:
				rtlglue_printf("@ ==> Unknown chip ID, no any default value found.\n");
				updateBDInfo = FALSE;
				break;
		}

		/*
			Update BDInfo if necessary.
		*/
		if (updateBDInfo == TRUE)
		{
			rtlglue_printf("@ Update board info ... ");
			memcpy(&currentBdInfo.wlan[cardIndex], &def, sizeof(bdinfo_wlan_t));
			/* memcpy(&currentBdInfo.wlan[cardIndex].mac[0], &currentBdInfo.mac[0], 6); */
			flashdrv_updateBdInfo(&currentBdInfo);
			rtlglue_printf("DONE!\n\n");
		}
		
	}else if(!strcmp(nextToken, "get")) {
	/* ========================================================================
	     ********************************************************************************************

		Get Flash info:

			Dump flash information from BDINFO.

	    *********************************************************************************************
	    ======================================================================== */
		rtlglue_printf("MAC: %x:%x:%x:%x:%x:%x, ",currentBdInfo.mac[0],currentBdInfo.mac[1],currentBdInfo.mac[2],currentBdInfo.mac[3],currentBdInfo.mac[4],currentBdInfo.mac[5]);
		rtlglue_printf("RAM:%08x, ",  currentBdInfo.ramStartAddress);
		rtlglue_printf("Root:%08x\n",  currentBdInfo.rootStartOffset);
		memDump(&currentBdInfo, sizeof(bdinfo_t),"Raw data");
	} else {
	/* ========================================================================
	     ********************************************************************************************

		Write Configuration to BDINFO

	    *********************************************************************************************
	    ======================================================================== */
		do {
			if (strcmp(nextToken, "rfchip") == 0) {
				cle_getNextCmdToken(&nextToken, &size, saved);
				value = U8_value(nextToken);
				currentBdInfo.wlan[cardIndex].RFChipID = value;
			} else if (strcmp(nextToken, "mac") ==0 ) {
				cle_getNextCmdToken(&nextToken,&size,saved); 
				  *((ether_addr_t *)currentBdInfo.wlan[cardIndex].mac) = *((ether_addr_t *)nextToken);
			}else if (strcmp(nextToken, "rxcp")==0) {
				cle_getNextCmdToken(&nextToken,&size,saved); 
				value = U32_value(nextToken);
				currentBdInfo.wlan[cardIndex].rxChargePump = value;
			}else if (strcmp(nextToken, "txcp") ==0 ) {
				cle_getNextCmdToken(&nextToken,&size,saved); 
				value = U32_value(nextToken);
				currentBdInfo.wlan[cardIndex].rxChargePump = value;
			}else{
				int32 isOfdm=1;
				if (strcmp(nextToken, "cck") ==0 ||strcmp(nextToken, "B") ==0){
					isOfdm=0;
					chIndex=0;
				}else if (strcmp(nextToken, "A") ==0) {
					chIndex=14;
				}else if(strcmp(nextToken, "G") ==0){
					chIndex=0;
				}else{ 
					chIndex=0; //default is 11b/g
					isOfdm=0;
				}
				cle_getNextCmdToken(&nextToken,&size,saved); 
				value = U32_value(nextToken);
				if(isOfdm && value>=36)
					chIndex+=(value-36)/4;
				else
					chIndex=value-1;
				for(i=0;i<4;i++){
					cle_getNextCmdToken(&nextToken,&size,saved); 
					if(isOfdm==0)
						currentBdInfo.wlan[cardIndex].cckTxPower[chIndex][i] = U32_value(nextToken);
					else
					currentBdInfo.wlan[cardIndex].txPower[chIndex][i] = U32_value(nextToken);
				}
			}
		}while(cle_getNextCmdToken(&nextToken,&size,saved) !=FAILED);
		flashdrv_updateBdInfo(&currentBdInfo);
	}
	rtlglue_printf("@ -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n");
	return SUCCESS;
}
#endif


static uint32 irq_saved;
static uint32 flags;

inline void IRQ_DISABLE(void)
{
    irq_saved = (*((volatile uint32 *)(0xBD012000)));
    rtlglue_printf("IRQ saved: %u\n", irq_saved);

    (*((volatile uint32 *)(0xBD012000))) = 0;

    flags = read_c0_status();
    rtlglue_printf("Status saved: %u\n", flags);

    write_c0_status(flags&(~0xFF01));
}


inline void IRQ_ENABLE(void)
{
    (*((volatile uint32 *)(0xBD012000))) = irq_saved;

    write_c0_status(flags);
}


#ifdef CONFIG_RTL865X_BENCHMARK

static int32 _benchmarking(uint32 userId,  int32 argc,int8 **saved)
{
	int8 *nextToken;
	int32 size, round;
	
	cle_getNextCmdToken(&nextToken,&size,saved);
	if(strcmp(nextToken, "dhry21") == 0) {
		cle_getNextCmdToken(&nextToken,&size,saved);
		round = U32_value(nextToken);
		dhrystone21(round);
	}else if(strcmp(nextToken, "dhry11") == 0) {
		cle_getNextCmdToken(&nextToken,&size,saved);
		round = U32_value(nextToken); 
		dhrystone11(round);
	}else if(strcmp(nextToken, "fib") == 0) {
		fibBenchmark();
	}else if(strcmp(nextToken, "hanoi") == 0) {
		hanoiBenchmark();
	}else if(strcmp(nextToken, "heapsort") == 0) {
		heapsortBenchmark();
	}else if(strcmp(nextToken, "bogomips") == 0) {
		bogoMips();
	}
	
	return SUCCESS;
}

#endif


static cle_exec_t _initCmdList[] = {
#if defined (CONFIG_RTL8185) || defined (CONFIG_RTL8185B)
	{	"8185",
		"rtl8185 MP flash read/write command",
		" %d'WLAN card index' "
		"{ default'Set default RF cfg to flash' { [ user'default value from pre-defined setting'  ] [ eeprom'default value from eeprom' ] } | "
		"get'Get card config from flash' | "
		"{ [ mac'Write MAC to flash' %mac'MAC address in XX:XX:XX:XX:XX:XX format' ] "
		"[ rfchip'Write RF Chip ID to flash' %d'9 for Zebra8225, 10 for OMC8255' ] "
		"[ rxcp'Write Rx charge pump cfg to flash' %d'0~7 for 200mA~550mA' ] [ txcp'Write Tx charge pump cfg to flash' %d'0~7 for 200~550mA' ] "
		"[ { A'Select 802.11a band...' %d'select channel (36~64)...' | B'Select 802.11b band...' %d'select channel (1~14)' | G'Select 802.11g band...' %d'select channel (1~14)' | cck %d'select channel (1~14)'  } %d'tx pwr(Max)' %d'tx pwr(2)' %d'tx pwr(3)' %d'tx pwr(min)' ] } }",
		_rtl8651_8185flashCfgCmd,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},
#endif
	
#if defined(CONFIG_RTL865X_MULTILAYER_BSP) && defined(CONFIG_RTL865X_ROMEDRV)
	{	"debug",
		"Display debug info",
		"{ on | off  | %d'Debug level' }",
		_rtl8651_debugCmd,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},
	{	"reset",
		"Reset to code default. Flush all tables & settings.",
		"",
		_rtl8651_resetCmd,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},	
#endif

#ifndef RTK_X86_CLE//RTK-CNSD2-NickWu-20061222: for x86 compile
	{	"memory",
		"read/write memory",
		"{ read'read memory from specified base address' %d'base address' %d'Length' | write'write memory from specified base address' %d'base address' %d'4 byte value' } ",
		_rtl8651_memdump,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},	
#endif

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV	
#ifdef CONFIG_RTL865XB_EXP_CRYPTOENGINE	
	{	"dmacpy",
		"Memcpy test",
		" { init | { [ emulate ] %d'Pktlen' %d'# of bytes copy by crypto engine' [ poll %d ] %d'iteration' } }",
		cmdCrymemcpy,
		CLE_USECISCOCMDPARSER,
		0,
		NULL
	},	
#endif
#endif
#ifdef RTL865X_MODEL_USER
	{	"pci",
		"read/write io address",
		"{ read'read memory from specified base address' %d'base address' %d '1:byte,2:word,4:dword' | write'write memory from specified base address' %d'base address' %d'value' %d' 1:byte,2:word,4:dword' } ",
		cmdPci,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},		
#endif

#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
	{	"permemcpy",
		"Memcpy performance test",
		"[ cached ] { COP3I | COP3D } %d'Pktlen' %d'iteration'",
		_rtl8651_permemcpyCmd,
		CLE_USECISCOCMDPARSER,
		0,
		NULL
	},	
	{	"pernicrx",
		"NIC Rx performance test (drop and count)",
		"{ inst | data } %d'Pkt number to test'",
		_rtl8651_pernicrxCmd,
		CLE_USECISCOCMDPARSER,
		0,
		NULL
	},	
	{	"perpcitx",
		"PCI Tx performance test (capture one packet to PCI and send it out repeatedly)",
		"{ { inst | data } %d'Pkt number to test' | start }",
		_rtl8651_perpcitxCmd,
		CLE_USECISCOCMDPARSER,
		0,
		NULL
	},	
	{	"cop3",
		"Coprocessor 3 counters",
		"{ inst | data | end }",
		_rtl8651_startCOP3Counters,
		CLE_USECISCOCMDPARSER,
		0,
		NULL
	},	
#endif

#ifdef CONFIG_RTL865X_BENCHMARK
	{	"benchmark",
		"Running benchmarking tools",
		" { dhry21 %d'Dhrystone 2.1 repeat rounds' | dhry11 %d'Dhrystone 1.1 repeat rounds' | fib | hanoi | heapsort | bogomips } ",
		_benchmarking,
		CLE_USECISCOCMDPARSER,
		0,
		NULL
	},	
#endif

};


static cle_grp_t _initCmdGrpList[] = {
#if defined(CONFIG_RTL865XB)||defined(CONFIG_RTL865XC)
#if defined(CONFIG_RTL865X_MULTILAYER_BSP) && defined(CONFIG_RTL865X_ROMEDRV)
	{
		"drv",							//cmdStr
		"Config RTL8651 high level driver",			//cmdDesc
		NULL,							//cmdPrompt
		rtl8651_tblDrv_cmds,	    			//exec array
		NULL,		        				//group array
		CMD_RTL8651_TBLDRV_CMD_NUM,		//exec number
		0,		            					//group number
		0,		            					//access level
	},
	{
		"fwd",			    			//cmdStr
		"Config RTL8651 forwarding engine",	//cmdDesc
		NULL,							//cmdPrompt
		rtl8651_tblDrvFwd_cmds,		    	//exec array
		NULL,		        				//group array
		CMD_RTL8651_TBLDRVFWD_CMD_NUM,		//exec number
		0,		            					//group number
		0,		            					//access level
	},
	{
		"nic",			    				//cmdStr
		"RTL8651 NIC config & status",			//cmdDesc
		NULL,							//cmdPrompt
		swNic_cmds,	   			 //exec array
		NULL,		        				//group array
		CMD_RTL8651_SWNIC_CMD_NUM,		//exec number
		0,		            					//group number
		0,		            					//access level
	},	
#endif /*CONFIG_RTL865X_MULTILAYER_BSP*/

#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
	{
		"lightrome",
		"Configure Light-Rome Driver",
		NULL,
		rtl865x_lightRome_cmds,
		NULL,
		CMD_RTL865X_LIGHTROME_CMD_NUM,
		0,
		0,
	},
#endif

#ifdef RTL865X_MODEL_USER
	/* In Model User mode, we did not support ASIC driver yet. */
#else
	{
		"asic",			    				//cmdStr
		"Config RTL8651 ASIC driver",			//cmdDesc
		NULL,							//cmdPrompt
		rtl8651_tblAsicDrv_cmds,	   			//exec array
		NULL,
		CMD_RTL8651_TBLASICDRV_CMD_NUM,			//exec number
		0,
		0,		            					//access level
	},
#endif

#ifdef CONFIG_RTL865x_SWPUBKEY
	{
		"pubkey",			    				//cmdStr
		"Using rtl865x software public key driver",			//cmdDesc
		NULL,							//cmdPrompt
		rtl865x_pubkey_cmds,	   			 //exec array
		NULL,		        				//group array
		CMD_RTL865x_PUBKEY_CMD_NUM,		//exec number
		0,		            					//group number
		0,		            					//access level
	},
#endif

#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
 #ifdef CONFIG_RTL865XB_EXP_CRYPTOENGINE
	{
		"crypto",			    				//cmdStr
		"Using rtl865xB hardware cryptography engine",			//cmdDesc
		NULL,							//cmdPrompt
		rtl865x_crypt_cmds,	   			 //exec array
		NULL,		        				//group array
		CMD_RTL865XB_CRYPT_CMD_NUM,		//exec number
		0,		            					//group number
		0,		            					//access level
	},
#endif
#endif
#endif	/* defined(CONFIG_RTL865XB)||defined(CONFIG_RTL865XC) */


#ifdef CONFIG_RTL8305S
	{
		"8305s",			    				//cmdStr
		"Control the MII/GPIO controlled 8305SB/8305SB",			//cmdDesc
		NULL,							//cmdPrompt
		rtl8305s_cmds,	   			 //exec array
		NULL,		        				//group array
		CMD_RTL8305S_CMD_NUM,		//exec number
		0,		            					//group number
		0,		            					//access level
	},
#endif
#ifdef CONFIG_RTL865X_PCM
	{
		"voip",			    				//cmdStr
		"VoIP functions",					//cmdDesc
		NULL,								//cmdPrompt
		voip_cmds,	  			 			//exec array
		_voipCmdGrpList,       				//group array
		CMD_VOIP_CMD_NUM,					//exec number
		CMD_VOIP_GRP_NUM,					//group number
		0,		            				//access level
	},	
#endif

#ifndef RTK_X86_CLE//RTK-CNSD2-NickWu-20061222: for x86 compile
#if defined(RTL865X_MODEL_USER) || defined(RTL865X_MODEL_KERNEL)
	{
		"model",			    			//cmdStr
		"Run model test code",				//cmdDesc
		NULL,								//cmdPrompt
		model_cmds,	  			 			//exec array
		model_cmdGrpList,      				//group array
		CMD_MODEL_CMD_NUM,					//exec number
		CMD_MODEL_GRP_NUM,					//group number
		0,		            				//access level
	},	
#endif
#endif

#if defined(CONFIG_SMI)
/* SMI Comand */
	{
		"smi",				    			//cmdStr
		"Command through SMI Interface",	//cmdDesc
		NULL,								//cmdPrompt
		smi_cmds,							//exec array
		NULL,								//group	array
		CMD_SMI_CMD_NUM,					//exec number
		0,									//group number
		0,		            				//access level
	},
#endif

#ifdef CONFIG_RTL8366	
/* Rtl8366 Comand */
	{
		"rtl8366",								//cmdStr
		"Command of RTL8366 ASIC driver",		//cmdDesc
		NULL,									//cmdPrompt
		rtl8366_cmds,							//exec array
		NULL,									//group array
		CMD_RTL8366_CMD_NUM,					//exec number	
		0,										//group number		
		0,		            					//access level
	},
#endif 

#ifdef CONFIG_RTL8310P
/* Rtl8310P Comand */
	{
		"rtl8310p",								//cmdStr
		"Command of RTL8310P ASIC driver",		//cmdDesc
		NULL,									//cmdPrompt
		rtl8310p_cmds,							//exec array
		NULL,									//group array
		CMD_RTL8310P_CMD_NUM,					//exec number	
		0,										//group number		
		0,		            					//access level
	},
#endif


#ifdef CONFIG_RTL8366S	
/* Rtl8366S Comand */
	{
		"rtl8366s",								//cmdStr
		"Command of RTL8366S ASIC driver",		//cmdDesc
		NULL,									//cmdPrompt
		rtl8366s_cmds,							//exec array
		NULL,									//group array
		CMD_RTL8366S_CMD_NUM,					//exec number	
		0,										//group number		
		0,		            					//access level
	},
#endif
#if  defined(CONFIG_RTL8316S) & !defined(RTL8316S_MODEL_USER)	
/* Rtl8316S Comand */
	{
		"rtl8316s",								//cmdStr
		"Command of RTL8316S model core",		//cmdDesc
		NULL,									//cmdPrompt
		rtl8316s_cmds,							//exec array
		NULL,									//group array
		CMD_RTL8316S_CMD_NUM,					//exec number	
		0,										//group number		
		0,		            					//access level
	},
#endif

#ifdef CONFIG_MDCMDIO
/* MDC/MDIO Comand */
	{
		"mdcmdio",
		"Command through MDC/MDIO Interface",
		NULL,
		mdcmdio_cmds,
		NULL,
		CMD_MDCMDIO_CMD_NUM,				
		0,					
		0,		            
	},
#endif

#ifdef CONFIG_RTL8306SDM	
/* Rtl8306sdm_AiscDrv Comand */
	{
		"8306sdm",
		"Command of RTL8306SDM ASIC driver",
		NULL,
		rtl8306sdm_asicdrv_cmds,
		NULL,
		CMD_RTL8306SDM_ASICDRV_CMD_NUM,				
		0,					
		0,		            
	},	
#endif

#ifdef CONFIG_RTL8363	
/* Rtl8363_Aisc Comand */
 
       {
		"8363veri",
		"Command of RTL8363 ASIC Testing code",
		NULL,
		rtl8363_asic_cmds,
		NULL,
		CMD_RTL8363_ASIC_CMD_NUM,				
		0,					
		0,		            
	},

      {
            "oam",
            "Command of IEEE 802.3ah OAM",
            NULL,
            rtl_oam_cmds,
            NULL,
            CMD_RTL_OAM_CMD_NUM,
            0,
            0,            
      },     
      
      {
		"8363drv",
		"Command of RTL8363 ASIC ASIC Driver",
		NULL,
		rtl8363_asicdrv_cmds,
		NULL,
		CMD_RTL_ASICDRV_CMD_NUM,				
		0,					
		0,		            
	},

               

            
#endif

#ifdef CONFIG_IGMP_SNOOPING
/* rtl8306sdm igmp snooping Comand */
	{
		"igmp",
		"Command of igmp snooping",
		NULL,
		rtl_igmp_cmds,
		NULL,
		CMD_RTL_IGMP_CMD_NUM,				
		0,					
		0,		            
	},	
#endif

#ifdef CONFIG_MULTICAST_SNOOPING	
/* rtl8306sdm multicast snooping Comand */
	{
		"multicast",
		"Command of multicast snooping",
		NULL,
		rtl_multicast_cmds,
		NULL,
		CMD_RTL_MULTICAST_CMD_NUM,				
		0,					
		0,		            
	},	
#endif

#ifdef CONFIG_RTL8309SB	
/* Rtl8309sb  Comand */
	{
		"8309sb",
		"Command of RTL8309SB ASIC driver",
		NULL,
		rtl8309sb_cmds,
		NULL,
		CMD_RTL8309SB_CMD_NUM,				
		0,					
		0,		            
	},	
#endif

	/* more ...... */
};

cle_grp_t cle_newCmdRoot[] = {
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	{
		"rtl865x",								//cmdStr
		"RTL865x command line interface",	//cmdDesc
		"rtl865x",								//cmdPrompt
		_initCmdList,					//List of commands in this layer	/* this line used by re_core.c for modulization */
		_initCmdGrpList,						//List of child command groups
		sizeof(_initCmdList)/sizeof(cle_exec_t),		//Number of commands in this layer /* this line used by re_core.c for modulization */
		sizeof(_initCmdGrpList)/sizeof(cle_grp_t),	//Number of child command groups
		0,									//access level
	}
#endif
	/* more ...... */
};

#ifndef	CONFIG_RTL865X_MODULE_ROMEDRV
#if defined (CONFIG_RTL8185) || defined (CONFIG_RTL8185B) || defined (CONFIG_RTL8185_MODULE)
//glue function for 8185 iwpriv ioctl to call our own CLE engine.
void rtl8651_8185flashCfg(int8 *cmd, uint32 cmdLen){
	int i, ret,total=sizeof(_initCmdList)/sizeof(cle_exec_t);
	cle_exec_t *CmdFmt=NULL;
	for(i=0;i<total;i++){
		if(strcmp(_initCmdList[i].cmdStr, "8185")==0){
			CmdFmt=&_initCmdList[i];
			break;
		}
	}
	ret=cle_cmdParser(cmd, CmdFmt,"/,= ");
	if(ret<1)
		rtlglue_printf("Cmd failed, ret=%d\n", ret);
}
EXPORT_SYMBOL(rtl8651_8185flashCfg);
#endif
#endif

