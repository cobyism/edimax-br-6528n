/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Implementation of Glue Functions of Model Code
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: rtl_glue.c,v 1.6 2008/11/18 06:57:50 joeylin Exp $
*/

#include "rtl_types.h"
#if defined(CONFIG_RTL865X) || defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) || defined(CONFIG_RTL8196B)
#include "asicregs.h"
#include "mbuf.h"
#endif

#if !defined(RTL865X_TEST) && !defined(RTL865X_MODEL_USER)
#ifdef __linux__
#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#endif


#include "swNic2.h"
#include "rtl_glue.h"
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include "rtl_utils.h"
#include "assert.h"
#include "rtl8651_tblDrv.h"
#include "rtl8651_tblDrvFwd.h"
#include "rtl8651_layer2fwd.h"
#endif

#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#if (defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)) && !defined(CONFIG_RTL865X_LIGHT_ROMEDRV)
#include "extPortModule.h"
#endif
#if (defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)) && !defined(CONFIG_RTL865X_LIGHT_ROMEDRV)
#include <mbufGen.h>
#endif
#include <time.h>
#include <sys/time.h>
#endif


#ifdef __linux__
#include <asm/unistd.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif


/* =============================	*/
/*	These header files should be removed.		*/
/*	Only for model compilation temporarily.	*/
/* =============================	*/
#ifdef RTL865X_MODEL_USER
#include "rtl865x/rtl8651_tblDrvLocal.h"
#include "rtl865x/rtl8651_layer2local.h"
#include "rtl865x/rtl8651_alg_qos.h"
#endif
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#include "rtl8651_layer2fwdLocal.h"
#include "rtl8651_layer2fwd.h"
#endif


#ifdef AIRGO_FAST_PATH
typedef unsigned int (*fast_free_fn)(void * pkt);
extern fast_free_fn rtl865x_freeMbuf_callBack_f;
#ifdef CONFIG_RTL8190_FASTPATH
#define RTL865X_EXTDEV_SKBCB_PKTHDR_OFFSET			12			/* Record the offset of Packet header in CB field in Socket buffer */
#else
#define RTL865X_EXTDEV_SKBCB_PKTHDR_OFFSET			4			/* Record the offset of Packet header in CB field in Socket buffer */
#endif
#endif

#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
static int32 _perpciStart = FALSE;
static int32 _perpciInst = TRUE;
static uint32 _perpciPktLimit = 10000;
static uint32 _perpciPktCount = 0;
static uint32 _perpciPktDmzCount = 0;
static uint32 _perpciByteCount = 0;
static struct sk_buff *_perpciSkb; 
#endif

/* ========================================
		For TEST / user space model:
			No any symbol need to export
    ======================================== */
#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)
#define EXPORT_SYMBOL(func)
#endif

#ifdef CONFIG_RTL865X_MODULE_ROMEDRV
#ifdef  SWNIC_DEBUG
extern int32 *module_external_data_nicDbgMesg;
#define	nicDbgMesg		(*module_external_data_nicDbgMesg)
#endif

extern int32 *module_external_data_rtl8651_totalExtPortNum;
#define	rtl8651_totalExtPortNum	(*module_external_data_rtl8651_totalExtPortNum)

#ifdef CONFIG_RTL865XB
extern uint32 *module_external_data_totalRxPkthdr;
extern uint32 *module_external_data_totalTxPkthdr;

#define totalRxPkthdr	(*module_external_data_totalRxPkthdr)
#define totalTxPkthdr	(*module_external_data_totalTxPkthdr)
#endif

#endif

/*put the definition ahead to avoid compile warning(error)*/
#ifdef CONFIG_RTL865XB
extern void lx4180_writeCacheCtrl(int32  value);
#endif

/* seed for OS independent random function. */
static uint32 rtl_seed = 0xDeadC0de;


/*
 * Since tblDrv cannot know how freq tick is,
 *   this function returns system tick freq.
 * In linux, a typical case of system tick is 100HZ.
 */
uint32 rtl865x_getHZ(void)
{
#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)
	return 100;
#else/*RTL865X_TEST*/
	return HZ;
#endif/*RTL865X_TEST*/
}

// for code easy-reading with SourceInsight
#if  0 //defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)
//------------------------  MODULE TEST CODE GLUE STARTS HERE---------------//
int test_drvMutex = 0;
int test_mbufMutex = 0;
//This is not real Linux sk_buff. Simplified for module test.
struct sk_buff {
        /* These two members must be first. */
        struct sk_buff  * next;                 /* Next buffer in list                          */
        struct sk_buff  * prev;                 /* Previous buffer in list                      */
        struct sk_buff_head * list;             /* List we are on                               */
        struct sock     *sk;                    /* Socket we are owned by                       */
        unsigned int    len;                    /* Length of actual data                        */
        unsigned int    data_len;
        unsigned int    csum;                   /* Checksum                                     */
        unsigned char   cloned;                 /* head may be cloned (check refcnt to be sure). */
        unsigned int    truesize;               /* Buffer size                                  */

        unsigned char   *head;                  /* Head of buffer                               */
        unsigned char   *data;                  /* Data head pointer                            */
        unsigned char   *tail;                  /* Tail pointer                                 */
        unsigned char   *end;                   /* End pointer                                  */
};

struct sk_buff_head {
        struct sk_buff  * next;
        struct sk_buff  * prev;

        uint32          qlen;
};

struct skb_shared_info {
        unsigned int    nr_frags;
        struct sk_buff  *frag_list;
};

#ifdef RTL865X_TEST
/* Only test code needs this function, model code will defined in rtl_glue.h. */
int32 spin_lock_irqsave(spinlock_t *spinlock, int32 s){return 0;}
int32 spin_unlock_irqrestore(spinlock_t *spinlock, int32 s){return 0;}

/* Only test code needs this function, model code will include swNic2.c. */
#endif
/*
int32 rtlglue_drvMutexLock(void){test_drvMutex ++; return 0;}
int32 rtlglue_drvMutexUnlock(void){test_drvMutex --; return 0;}
*/
int32 rtlglue_getDrvMutex(void){return test_drvMutex;}

int32 rtlglue_reinitDrvMutex(void){test_drvMutex=0; return 0;}
/*
int32 rtlglue_mbufMutexLock(void){test_mbufMutex ++; return 0;}
int32 rtlglue_mbufMutexUnlock(void){test_mbufMutex --;  return 0;}
*/
int32 rtlglue_getMbufMutex(void){return test_mbufMutex;}
int32 rtlglue_reinitMbufMutex(void){test_mbufMutex=0; return 0;}
void local_irq_restore(void){}
void local_irq_save(void){}

void rtlglue_getMacAddress(ether_addr_t * macAddress, uint32 * number){
	uint8 mac[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x08 };
        memcpy((int8*)macAddress, (int8 *)mac, 6);
        *number = 8;
}

void *rtlglue_malloc(uint32 NBYTES){return malloc(NBYTES);}
void rtlglue_free(void *APTR){free(APTR);}

int32 rtlglue_mbufClusterRefcnt(void *buffer, uint32 id, uint32 *count, int8 operation){
        if (operation==0){
                if (count)
                        *count=1;
        }
        return SUCCESS;
}
int32 rtlglue_mbufAllocCluster(void **buffer, uint32 size, uint32 *id){
        struct sk_buff *skb;
	void* data;
	/*
	skb = (struct sk_buff *)malloc((sizeof(struct sk_buff_head) + size + sizeof(struct skb_shared_info)));
	data = skb + sizeof(struct sk_buff_head);
	*/
	skb = (struct sk_buff *)malloc(sizeof(struct sk_buff) + size );/*by qjj*/
	data =(uint8 *)skb + sizeof(struct sk_buff);
	
	skb->truesize = size + sizeof(struct sk_buff);
	skb->head = data;
	skb->data = data;
	skb->tail = data;
	skb->end = data+size;
	skb->len = 0;
        skb->cloned = 0;
        skb->data_len = 0;

	*id = (int)skb;
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
	*buffer = (void*)((uint32)skb->data);
#else
	*buffer = (void*)((uint32)skb->data|0xa0000000);
#endif
	return SUCCESS;
}
int32 rtlglue_mbufFreeCluster(void *buffer, uint32 size, uint32 id){
	free((void *)id);
        return 0;
}

#if defined(CONFIG_RTL865X) || defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) || defined(CONFIG_RTL8196B)
void rtlglue_extDeviceSend(struct rtl_pktHdr *pktHdr, void *txExtDev)
{
	pktHdr->ph_portlist=1<<(((struct extPortDevice *)txExtDev)->extPortNum);
	#ifdef EXT_DEVICE_MODULE
	#if defined(RTL865X_MODEL_USER)
		 /* Since user model code does not support EXT_DEVICE_MODULE, we do not call _rtlglue_sendCallback(). */
		/*but some 865xb/865xc test code use this function, so i hope the macro
		RTL865X_TEST can solve this problem*/
		#ifdef RTL865X_TEST
			_rtlglue_sendCallback(pktHdr, txExtDev);
		#endif
	#else
	_rtlglue_sendCallback(pktHdr, txExtDev);
	#endif
	#else
	/* We don't need to implement this function in user model code mode, just free it. */
	freeMbuf(pktHdr->ph_mbuf);
	#endif
}

/*
	Flush D-Cache in processor ( with Write-Back )
*/
inline int32 rtlglue_flushDCache(uint32 start, uint32 size)
{
	return SUCCESS;
}

/*
	Flush D-Cache in processor ( without Write-Back )
*/
inline int32 rtlglue_clearDCache(uint32 start, uint32 size)
{
	return SUCCESS;
}

void rtlglue_drvSend(void * pkthdr)
{
	struct rtl_mBuf* new_m;
	// duplicate packet

	new_m = mBuf_dupPacket(((struct rtl_pktHdr*)pkthdr)->ph_mbuf, MBUF_DONTWAIT);	
	if (!new_m){
		
		printf("@mbuf duplicate error!\n");
		return;
	}

	// free original packet	
	freeMbuf(((struct rtl_pktHdr*)pkthdr)->ph_mbuf);

	// enqueue new packet
	if (mbufList_enqueue(new_m)!=0){

		printf("@mbuf enqueue error!\n");
	}
	return;
}
void * rtlglue_mbufClusterToData(void *buffer){
	return buffer;
}
#endif	/* defined(CONFIG_RTL865X) || defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) */

#ifdef CONFIG_RTL865XC 
void rtlglue_getRingSize(uint32 *rx, uint32 *tx,int whichDesc)
#else
void rtlglue_getRingSize(uint32 *rx, uint32 *tx)
#endif
{
	if (rx)
		*rx = 8;
	if (tx)
		*tx = 8;
}

#if defined(CONFIG_RTL865X) || defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) || defined(CONFIG_RTL8196B)
int32 rtlglue_reclaimRxBD(uint32 rxDescIdx,struct rtl_pktHdr *pThisPkthdr, struct rtl_mBuf *pThisMbuf){pThisPkthdr->ph_rxPkthdrDescIdx = -1; mBuf_driverFreeMbufChain(pThisMbuf);  return 0;};
#endif	/* defined(CONFIG_RTL865X) || defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) */
int32 rtlglue_mbufTsleep(uint32 wait_channel){return 0;}
int32 rtlglue_mbufTwakeup(uint32 wait_channel){return 0;}

/* =========================================================	*/
/*	The following are functions that is used in table driver but defined 					*/
/*	in other modules that has not yet been ported to RTL865xC driver.					*/
/*	Therefore, the following functions are fake temporarily.							*/
/* =========================================================	*/
#ifdef RTL865X_MODEL_USER

/* Empty now */

#endif


#if 1 /* 1: Simulate psuedo random data of kernel mode.
       * 0: use libc random for user-space program. */
/*
@func void	| rtlglue_srandom	| The OS independent seed function for random.
@parm uint32 | seed			| seed
@comm
 */
void rtlglue_srandom( uint32 seed )
{
	rtl_seed = seed;
}

/*
@func uint32	| rtlglue_random	| The OS independent random function.
@parm void | 			| 
@comm
 */
uint32 rtlglue_random( void )
{
	uint32 hi32, lo32;

	hi32 = (rtl_seed>>16)*19;
	lo32 = (rtl_seed&0xffff)*19+37;
	hi32 = hi32^(hi32<<16);
	return ( rtl_seed = (hi32^lo32) );
}
#else
/*
@func void	| rtlglue_srandom	| The OS independent seed function for random.
@parm uint32 | seed			| seed
@comm
 */
void rtlglue_srandom( uint32 seed )
{
	srandom( seed );
}

/*
@func uint32	| rtlglue_random	| The OS independent random function.
@parm void | 			| 
@comm
 */
uint32 rtlglue_random( void )
{
	return random();
}
#endif


uint32 rtlglue_getmstime( uint32* pTime )
{
	struct timeval tm;
	gettimeofday( &tm, NULL );
	tm.tv_sec = tm.tv_sec % 86400; /* to avoid overflow for 1000times, we wrap to the seconds in a day. */
	/* rtlglue_printf( "%u.%u\n", (uint32)tm.tv_sec, (uint32)tm.tv_usec ); */
	/* rtlglue_printf( "%u\n", (uint32)(tm.tv_sec*1000 + tm.tv_usec/1000) ); */
	return *pTime = ( tm.tv_sec*1000 + tm.tv_usec/1000 );
}


/*
@func uint32 | rtlglue_time	| The OS dependent time function.
@parm uint32* | t			| address to store time
@comm
 */
uint32 rtlglue_time( uint32* t )
{
	return time( (time_t*)t );
}

//------------------------MODULE  TEST CODE GLUE ENDS HERE---------------//

#else /*RTL865X_TEST*/

//------------------------  LINUX DEPEND GLUE STARTS HERE-------------//

/*  ==============================================
	RTL_Glue interface initialization
     ============================================== */
int32 rtlglue_init(void)
{
	return 0;
}

/*  ==============================================
	Processor related processing
     ============================================== */
#define RTLGLUE_PKT_PROCESS
#if 0
/*
	Flush D-Cache in processor (with Write-Back)
*/
inline int32 rtlglue_flushDCache(uint32 start, uint32 size)
{
#ifdef CONFIG_RTL865X_ROMEPERF
	rtl8651_romeperfEnterPoint(ROMEPERF_INDEX_FLUSHDCACHE);
#endif

#ifdef CONFIG_RTL865XC
	/*
		Processor : RLX5181
	*/
	rtlglue_drvMutexLock();
	dma_cache_wback_inv(	start?CACHED(start):0,
						(size == 0)?(r3k_cache_size(ST0_ISC)):size);
	rtlglue_drvMutexUnlock();

#elif defined (CONFIG_RTL865XB)
	/*
		Processor : RLX5280
	*/
	lx4180_writeCacheCtrl(0);
	lx4180_writeCacheCtrl(1);
	lx4180_writeCacheCtrl(0);
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
	rtl8651_romeperfExitPoint(ROMEPERF_INDEX_FLUSHDCACHE);
#endif
	return SUCCESS;
}

/*
	Flush D-Cache in processor (without Write-Back)
*/
inline int32 rtlglue_clearDCache(uint32 start, uint32 size)
{
#ifdef CONFIG_RTL865XC
	/*
		Processor : RLX5181
	*/
	rtlglue_drvMutexLock();

	{
		uint32 actualStart;
		uint32 actualSize;

		actualStart = start?CACHED(start):0;
		actualSize = size;

		if (	( actualSize == 0 )	||
			( actualSize >= r3k_cache_size(ST0_ISC) ) ||
			( KSEGX(actualStart) != KSEG0  )	)
		{
			__asm__ volatile(
				"mtc0 $0,$20\n\t"
				"nop\n\t"
				"li $8,1\n\t"
				"mtc0 $8,$20\n\t"
				"nop\n\t"
				"nop\n\t"
				"mtc0 $0,$20\n\t"
				"nop"
				: /* no output */
				: /* no input */
			        );
		} else
		{
			unsigned char *p;
			unsigned int flags;
			unsigned int i;
			
			/* Start to isolate cache space */
			p = (char *)start;

			flags = read_c0_status();

			/* isolate cache space */
			write_c0_status( (ST0_ISC | flags) &~ ST0_IEC );

			for (i = 0; i < actualSize; i += 0x040)
			{
				asm (
					"cache 0x11, 0x000(%0)\n\t"
					"cache 0x11, 0x010(%0)\n\t"
					"cache 0x11, 0x020(%0)\n\t"
					"cache 0x11, 0x030(%0)\n\t"
					:               /* No output registers */
					:"r"(p)         /* input : 'p' as %0 */
				);
				p += 0x040;
			}

			write_c0_status(flags);
		}

	}
	rtlglue_drvMutexUnlock();

#elif defined (CONFIG_RTL865XB)
	/*
		Processor : RLX5280
	*/
	lx4180_writeCacheCtrl(0);
	lx4180_writeCacheCtrl(1);
	lx4180_writeCacheCtrl(0);
#else
#error "Unknown CHIP Version"
#endif
	return SUCCESS;
}

inline int32 rtlglue_pktToProtocolStackPreprocess(void)
{
	rtlglue_flushDCache(0, 0);
	return SUCCESS;
}
#endif

/*  ==============================================
	Critical section protection : MUTEX
     ============================================== */
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
static spinlock_t mbuf_lock = SPIN_LOCK_UNLOCKED;
static int __mbuf_flags;
int dd_cc;

int32 rtlglue_mbufMutexLock(void){
	if (dd_cc>=1){
		dd_cc++;
		return 0;
	}	
	spin_lock_irqsave(&mbuf_lock,__mbuf_flags);
	//enableLbcTimeout();
	dd_cc++;
        return 0;
}

int32 rtlglue_mbufMutexUnlock(void){
	dd_cc--;
	if (!dd_cc)
		spin_unlock_irqrestore(&mbuf_lock,__mbuf_flags);
        return 0;
}

int32 rtlglue_mbufTsleep(uint32 wait_channel){
        return 0;
}

int32 rtlglue_mbufTwakeup(uint32 wait_channel){
        return 0;
}

int32 rtlglue_mbufClusterRefcnt(void *buffer, uint32 id, uint32 *count, int8 operation){
	if (operation==0)
	{
		if (count)
			*count=1;

	}
        return SUCCESS;
}

void * rtlglue_mbufClusterToData(void *buffer){
	
	return (buffer);
}

int32 rtlglue_mbufAllocCluster(void **buffer, uint32 size, uint32 *id){
	struct sk_buff *skb;
	skb = dev_alloc_skb (size);
	if (skb)
	{
		*id = (int)skb;
		//skb->data was allocated as cached but we make it uncached to mbuf chain
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
		*buffer = (void*)((u32)skb->data);
#else
		*buffer = (void*)UNCACHE((u32)skb->data);
#endif
      		 return SUCCESS;
	}
	else 
	{
		rtlglue_printf("skb exhausted. alloc failed\n");
		return FAILED;
	}
}

int32 rtlglue_mbufFreeCluster(void *buffer, uint32 size, uint32 id)
{
	struct sk_buff *skb = (struct sk_buff*)id;

	if (skb == NULL)
	{
		return 0;
	}

#ifdef AIRGO_FAST_PATH
	/*
		This skbuff is allocated by AIRGO's driver,
		so we would call its callback function to free this packet.
	*/

	if (skb->pkt_type == PACKET_AIRGO)
	{
		if (rtl865x_freeMbuf_callBack_f)
		{
			unsigned int *p = (unsigned int*)skb->cb;
			(*rtl865x_freeMbuf_callBack_f)(*(struct rtl_pktHdr **)((uint32)p + RTL865X_EXTDEV_SKBCB_PKTHDR_OFFSET));
			return 0;
		}
	}
#endif
	dev_kfree_skb_any(skb);
        return 0;
}


int int_cc=0;
uint32 saveGimr = 0;

__IRAM_GEN int32 rtlglue_drvMutexLock(void)
{	
	spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
	int flagsForCC;
	int needLock;

	spin_lock_irqsave(&lockForCC,flagsForCC);

	needLock = (int_cc >= 1)?FALSE:TRUE;
	int_cc ++;

	if ( needLock == TRUE )
	{
		uint32 dummy;
 		saveGimr = READ_MEM32( GIMR );
 		WRITE_MEM32( GIMR, 0 );
		/*
			For RLX5181, LW/SW for external IPs (ex. GIMR) would be slow because one
			CMD queue is implement.

			So, we would always read GIMR back before calling <spin_unlock_irqrestore()> to
			make sure GIMR has be set before spin-unlock.
		*/
		dummy = READ_MEM32( GIMR );

 	}
 	
	spin_unlock_irqrestore(&lockForCC, flagsForCC);

	return 0;
}

__IRAM_GEN int32 rtlglue_drvMutexUnlock(void)
{
	spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
	int flagsForCC;
	int needUnlock;
	
	spin_lock_irqsave(&lockForCC,flagsForCC);
	
	int_cc--;
	needUnlock = ( int_cc==0 )?TRUE:FALSE;

	if ( needUnlock == TRUE )
	{
 		WRITE_MEM32( GIMR, saveGimr );
		/*
			For RLX5181, LW/SW for external IPs (ex. GIMR) would be slow because one
			CMD queue is implement.

			So, we would always read GIMR back before calling <spin_unlock_irqrestore()> to
			make sure GIMR has be set before spin-unlock.
		*/
		saveGimr = READ_MEM32( GIMR );
	}

	spin_unlock_irqrestore(&lockForCC, flagsForCC);

	return 0;
}

int chkValidAddr(uint32 addr)
{
	if(addr&0x80000000){
		if(addr>>28==0x8 || addr>>28==0xa||addr>>28==0xb)
			return 0;
	}
	rtlglue_printf("%08x invalid!!\n", addr);
	return 1;
}
int checkSkbAddr(struct sk_buff *skb){
	if(chkValidAddr((uint32)skb->head))
		return 1;
	if(chkValidAddr((uint32)skb->data))
		return 2;
	if(chkValidAddr((uint32)skb->tail))
		return 3;
	if(chkValidAddr((uint32)skb->end))
		return 4;
	return 0;
}
#endif	/* CONFIG_RTL865X */

// CONFIG_RE865X is defined in 865x SDK
#define CONFIG_RE865X
#undef NIC_DEBUG_PKTDUMP

#ifdef CONFIG_RE865X  //switch core and NIC is enabled
#ifdef AIRGO_FAST_PATH
extern void rtlairgo_fast_tx_register(void * fn);
#endif

#if 0
void rtlglue_drvSend(void *data)
{
struct rtl_pktHdr *pktHdr = (struct rtl_pktHdr *)data;

#if defined(CONFIG_RE865X) && defined(CONFIG_RTL865XC)
	/* =============================================

		RTL865xC
	
	     ============================================= */
	if (FAILED == swNic_write(	data,
								0))
#else
	/* =============================================

		RTL865xB
	
	     ============================================= */
	if (FAILED == swNic_write(data))
#endif

	{
		mBuf_freeMbufChain(pktHdr->ph_mbuf);
	}
}
#endif


int32 rtlglue_reclaimRxBD(uint32 rxDescIdx, struct rtl_pktHdr *pThisPkthdr, struct rtl_mBuf *pThisMbuf)
{
#if 1
    return SUCCESS;
#else
	int32 s,retval;

	spin_lock_irqsave(rtl865xSpinlock,s);
	assert(!pThisMbuf->m_next);

#ifdef CONFIG_RTL865XC

#ifdef AIRGO_FAST_PATH
#if CONFIG_RTL865X_MODULE_ROMEDRV
	if (__module_romeDrv_function_swNic_fastExtDevFreePkt(pThisPkthdr,  pThisMbuf) == SUCCESS)
#else
	if (swNic_fastExtDevFreePkt(pThisPkthdr,  pThisMbuf) == SUCCESS)
#endif
	{
		retval = SUCCESS;
	} else
#endif

#else

#ifdef AIRGO_FAST_PATH
	/*
		This PKTHDR is allocated in AIRGO driver.
		We free it using AIRGO's callback function here.
	*/

	if (	(pThisMbuf->m_unused1==0x66) &&
		(rtl865x_freeMbuf_callBack_f))
	{
		(*rtl865x_freeMbuf_callBack_f)(pThisPkthdr);
		retval = SUCCESS;
	} else

#endif

#endif

	{

#if defined(CONFIG_RE865X) && defined(CONFIG_RTL865XC)

  		retval =
				swNic_isrReclaim(pThisPkthdr->ph_rxPkthdrDescIdx,
  														rxDescIdx,
  														pThisPkthdr,
  														pThisMbuf);

#else

		retval=swNic_isrReclaim(rxDescIdx, pThisPkthdr,  pThisMbuf);
#endif
	}

	spin_unlock_irqrestore(rtl865xSpinlock,s);
	return retval;
#endif    
}


#if defined (CONFIG_RTL865X_MULTILAYER_BSP) || defined(CONFIG_RTL865X_LIGHT_ROMEDRV)
#if !defined(CONFIG_RTL865X_MODULE_ROMEDRV) && !defined (CONFIG_RTL865X_LIGHT_ROMEDRV)
EXPORT_SYMBOL(rtl8651_fwdEngineExtPortRecv);
EXPORT_SYMBOL(rtl8651_fwdEngineRemoveHostsOnExtLinkID);
EXPORT_SYMBOL(rtl8651_fwdEngineDelWlanSTA);
EXPORT_SYMBOL(rtl8651_fwdEngineAddWlanSTA);
EXPORT_SYMBOL(rtl8651_fwdEngineExtPortUcastFastRecv);
#endif	/* CONFIG_RTL865X_MODULE_ROMEDRV */


int32 _devglue_regExtDevice(int8 *devName, uint16 vid, uint8 extPortNum, uint32 *linkId);
int32 _devglue_unregExtDevice(uint32 linkId);
int32 _devglue_getLinkIDByName(int8 *devName);
int32 _devglue_setExtDevVlanProperty(int8 * devName, uint16 vid, uint8 isMbr, uint8 isTag);

//This function register a WLAN driver instance or a WDS link to a locally maintained database. 
//This function is not pat of 8651 driver glue interface but is implemented here to demonstrate how 
//to create linkId and bind it with OS-dependent driver interface.
//Valid linkId number can be any value except 0. linkId=0 would be used by 8651 driver when 
//rtlglue_extDeviceSend() is called to tell WLAN driver that a broadcast must be sent to all WLAN 
//cards or all WDS links on the designated vlan..
// ***This function is not called/used by 8651 driver 
int32 devglue_regExtDevice(int8 *devName, uint16 vid, uint8 extPortNum, uint32 *linkId)
{
	int32 retval;

	retval = _devglue_regExtDevice(devName, vid, extPortNum, linkId);

	return retval;
}

/* internal API called by devglue_regExtDevice() */
int32 _devglue_regExtDevice(int8 *devName, uint16 vid, uint8 extPortNum, uint32 *linkId)
{
	int lenDevice;
	struct net_device *netDev = NULL;
	
	if (!devName)
	{
		rtlglue_printf("%s:No device name given\n", __FUNCTION__);
		return FAILED;
	}

	lenDevice = strlen(devName);
	if (lenDevice==0 || lenDevice>15)
	{
		rtlglue_printf("%s:Device Name length is illegal: %d\n", __FUNCTION__, lenDevice);
		return FAILED;
	}

	if (!extPortNum || extPortNum>RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
	{
		rtlglue_printf("%s:Illegal 865XB ext port 0x%x\n", __FUNCTION__, extPortNum);
		return FAILED;
	}

	if (!linkId)
	{
		rtlglue_printf("%s:Pointer of LinkID can't be NULL\n", __FUNCTION__);
		return FAILED;
	}

	netDev = __dev_get_by_name(devName);

	if (netDev == NULL)
	{
		rtlglue_printf("No such device : %s\n", devName);
		return FAILED;
	}

	rtlglue_printf("=> Register External Device (%s) vid (%d) extPortNum (%d) -- Dev (0x%p)\n", devName, vid, extPortNum, netDev);

	/* Register this extension device into Rome Driver */
	if (rtl8651_fwdEngineRegExtDevice(	extPortNum,
										vid,
										linkId,
										(void*)netDev) != SUCCESS)										
	{
		rtlglue_printf("Fail When Register this extension device.\n");
	}

	return SUCCESS; 
}

/*
	Un-registration API
*/
int32 devglue_unregExtDevice(uint32 linkId)
{
	int32 retval;

	retval = _devglue_unregExtDevice(linkId);

	return retval;
}

/* internal API called by devglue_unregExtDevice() */
int32 _devglue_unregExtDevice(uint32 linkId)
{
	int32 retval = SUCCESS;

	if ((retval = rtl8651_fwdEngineUnregExtDevice(linkId)) != SUCCESS)
	{
		rtlglue_printf("UnRegister Extension Device not success: %d\n", retval);
	}

	return retval;
}

//This function is for OS-dependent part of WLAN driver to get 'devName's liniId number 
//before sending the pkt to 8651 river. Each WLAN pkt to be fast forwarded by 8651 ASIC 
//must be passed with its belonging linkId number to 8651 driver when 
//rtl8651_fwdEngineExtPortRecv() is called.
// ***This function is not called/used by 8651 driver
int32 devglue_getLinkIDByName(int8 *devName)
{
	int32 retval;

	retval = _devglue_getLinkIDByName(devName);

	return retval;
}

/* internal API called by devglue_getLinkIDByName() */
int32 _devglue_getLinkIDByName(int8 *devName)
{
	struct net_device *netDev = NULL;
	int lenDevice;
	int32 linkId = 0;

	if (!devName)
	{
		rtlglue_printf("%s:No device name given\n", __FUNCTION__);
		goto out;
	}

	lenDevice = strlen(devName);
	if (	(lenDevice == 0) ||
		(lenDevice > 15))
	{
		rtlglue_printf("%s:Device Name length is illegal: %d\n", __FUNCTION__, lenDevice);
		goto out;
	}

	netDev = __dev_get_by_name(devName);

	linkId = rtl8651_fwdEngineGetLinkIDByExtDevice((void*)netDev);
out:
	return linkId;
}

/*
	Set extension device's VLAN property
		- Add the device being the member of a VLAN
		- Remove the device from member set of a VLAN

	- NOTE: the parameter "isTag" might be used in the future and isn't used now.

*/
int32 devglue_setExtDevVlanProperty(int8 *devName, uint16 vid, uint8 isMbr, uint8 isTag)
{
	int32 retval = FAILED;

	retval = _devglue_setExtDevVlanProperty(devName, vid, isMbr, isTag);

	return retval;
}

/* internal API called by devglue_setExtDevVlanProperty() */
int32 _devglue_setExtDevVlanProperty(int8 *devName, uint16 vid, uint8 isMbr, uint8 isTag)
{
	int32 linkId;

	linkId = _devglue_getLinkIDByName(devName);

	if (linkId == 0)
	{
		rtlglue_printf("LinkID for device [%s] get failed.\n", devName);
		return FAILED;
	}

	return rtl8651_fwdEngineSetExtDeviceVlanProperty(	linkId,
														vid,
														((isMbr == TRUE)?
															RTL8651_EXTDEV_VLANPROPER_ISMBR:
															RTL8651_EXTDEV_VLANPROPER_NOTMBR));
}

void _mbuf2skbDebug(struct rtl_pktHdr* pPkt, struct rtl_mBuf *pkt_mbuf){
		uint8 *data= pkt_mbuf->m_data;
		uint8 macStr[sizeof"ff:ff:ff:ff:ff:ff"];			
		ether_ntoa_r((ether_addr_t *)&data[6], macStr);
		rtlglue_printf("\nPCI Tx(%d)%s->",pPkt->ph_rxdesc,macStr);
		ether_ntoa_r((ether_addr_t *)&data[0], macStr);
		rtlglue_printf("%s  0x%x  Len:%d...\n",macStr,*((uint16 *)(data+12)), pPkt->ph_len);
		memDump(data+14,28,"PCI Tx");
}

#define PKT_BUF_SZ 2048
struct sk_buff *re865x_mbuf2skb(struct rtl_pktHdr* pPkt)
{
	struct sk_buff *new_skb, *pkt_skb;
	struct rtl_mBuf *pkt_mbuf = pPkt->ph_mbuf;
	struct sk_buff *retval=NULL;

	assert(pPkt->ph_mbuf);	
	
	#ifdef RTL865X_DEBUG
	if (wlan_acc_debug>1)
	{
		_mbuf2skbDebug(pPkt, pkt_mbuf);
	}
	#endif

	if (pPkt->ph_rxPkthdrDescIdx < PH_RXPKTHDRDESC_MINIDX)
	{
		struct rtl_mBuf *m = pkt_mbuf;
		assert(m);

		pkt_skb = (struct sk_buff *)pkt_mbuf->m_extClusterId;



#ifdef CONFIG_RTL865XB_SW_LSO
		if(pkt_skb==NULL)
		{			
			pkt_skb = dev_alloc_skb (PKT_MBUF_CLUSTER_LEN);
			skb_put(pkt_skb,pkt_mbuf->m_len);
			
		}
		else
		{
#endif		
			if(pkt_skb->len==0){
				int32 lead= UNCACHE(pkt_mbuf->m_data)-UNCACHE(pkt_mbuf->m_extbuf);
				pkt_skb->data+= lead;
				skb_put(pkt_skb, pPkt->ph_len);
			}			
#ifdef CONFIG_RTL865XB_SW_LSO			
			pkt_skb->len=pkt_mbuf->m_len;
		}
		memcpy(pkt_skb->data,(uint8*)UNCACHE(pkt_mbuf->m_data),pkt_mbuf->m_len);
#endif		

#ifdef CONFIG_RTL865XB_SW_LSO			
		{
			struct ip *iphdr;
			int32 iphdrOffset=-1;	
			uint16 EtherType;
			EtherType = *((unsigned short*)(pkt_skb->data + 12));
			if(EtherType==0x0800)  /* etherhdr */
				iphdrOffset=14;
			else if(EtherType==0x8864){ /* etherhdr/pppoehdr/ppphdr/ip */
				if (*((unsigned short*)(pkt_skb->data + 20))==0x0021)
					iphdrOffset=22;
			}

			if(iphdrOffset>=0)
			{
				iphdr = (struct ip *)&(pkt_skb->data[iphdrOffset]);	
				
				if(iphdr->ip_p==IPPROTO_TCP)
				{
					struct tcphdr *tc;
					iphdr->ip_sum=0;
					iphdr->ip_sum=_rtl8651_ipChecksum(iphdr);
					tc = (struct tcphdr *)((int8 *)iphdr + (((*(uint8*)iphdr)&0xf) << 2));
					tc->th_sum = 0;
					tc->th_sum = _rtl8651_tcpChecksum(iphdr);	
				}
				pkt_skb->ip_summed=CHECKSUM_NONE;
			}
		}
#endif

		//free mbuf/pkthdr
		mBuf_freeOneMbufPkthdr(m, (void * *) &m->m_extbuf, &m->m_extClusterId,&m->m_extsize);

//#ifdef CONFIG_RTL865XB_SW_LSO
#if 1
		pkt_skb->data_len=0;		
#endif
		
		return pkt_skb;
	}
	//not from upper layer stack, allocate a new pkt buffer and skb header.
	//new pkt buffer would be linked with pPkt so as to refill swnic's rx ring
	//new skb would have its skb->data points to pPkt->ph_mbuf->m_data.
	new_skb = dev_alloc_skb (PKT_MBUF_CLUSTER_LEN);
	if (new_skb) 
	{
		int32 align=0;
		pkt_skb = (struct sk_buff *)pkt_mbuf->m_extClusterId;
		pkt_skb->ip_summed = CHECKSUM_NONE;
		//force set length to 0
		pkt_skb->data = pkt_skb->tail = pkt_skb->head ;
		pkt_skb->len=0;
		//adjust data pointer by mbuf
		skb_reserve(pkt_skb, 16+mBuf_leadingSpace(pkt_mbuf));
		//set real pkt length
		skb_put(pkt_skb, pPkt->ph_len);
#ifdef CONFIG_RTL865X_MBUF_HEADROOM
		align=CONFIG_RTL865X_MBUF_HEADROOM;
#endif
		//refill Rx ring, use new buffer
		mBuf_attachCluster(pkt_mbuf, (void *)UNCACHE(new_skb->data), (uint32)new_skb, PKT_MBUF_CLUSTER_LEN, 0, align);

		if (rtlglue_reclaimRxBD(pPkt->ph_rxdesc, pPkt, pkt_mbuf))
			mBuf_freeMbufChain(pkt_mbuf);
		retval= pkt_skb; 
	}else{
		mBuf_freeMbufChain(pkt_mbuf);
		retval= NULL;
	}
	return retval;
}


#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
void rtlglue_perpcitxStart(int32 instMode, uint32 totalPkt){
	_perpciPktCount = _perpciByteCount = _perpciPktDmzCount = 0;
	_perpciInst = instMode;
	_perpciPktLimit = totalPkt;
	_perpciStart = TRUE;
}

void rtlglue_perpcitxGo(void){
	if(_perpciStart == TRUE){
		struct sk_buff *tmp_skb;
		
		startCOP3Counters(_perpciInst);
		
		while(_perpciPktCount < _perpciPktLimit){
			tmp_skb = skb_clone(_perpciSkb, GFP_ATOMIC);
			dev_queue_xmit(tmp_skb);
			_perpciPktCount++;
			_perpciByteCount += tmp_skb->len + 4;
			}
		}
}

void rtlglue_perpciCount(void){
	if(_perpciStart == FALSE)
		return;
	
	if(++_perpciPktDmzCount == _perpciPktLimit){
		uint32 cycles;
		cycles = stopCOP3Counters();
		rtlglue_printf("%d pkts. Total %d bytes, %d ms.  %u Kbps\n", _perpciPktCount, _perpciByteCount, (uint32)(cycles/200000), (uint32)(_perpciByteCount*200000/cycles));
		_perpciStart = FALSE;
		}
}
#endif

#if 0
__IRAM_EXTDEV void rtlglue_extDeviceSend(struct rtl_pktHdr *pktHdr, void *txExtDev)
{
	struct sk_buff *skb;
	int32 ret;

	/* ======================================================
							Error check
	    ====================================================== */

	if (	(txExtDev == NULL) ||
		(!netif_running((struct net_device *)txExtDev)))
	{
		goto free_out;
	}

	/* ======================================================
							Dump packet content
	    ====================================================== */
#ifdef CONFIG_RTL865XC
#ifdef NIC_DEBUG_PKTDUMP
	swNic_pktdump(	NIC_EXT_TX_PKTDUMP,
					pktHdr,
					pktHdr->ph_mbuf->m_data,
					pktHdr->ph_len,
					(uint32)(((struct net_device*)txExtDev)->name));
#endif

#else

#ifdef SWNIC_DEBUG
	if (nicDbgMesg)
	{
		struct rtl_mBuf *m = pktHdr->ph_mbuf;
		if (nicDbgMesg & NIC_EXT_TX_PKTDUMP)
		{
			rtlglue_printf("865x->WLAN  L:%d\n", pktHdr->ph_len);
			memDump(m->m_data,m->m_len>64?64:m->m_len,"");
		}
	}
#endif

#endif

	/* ======================================================
							SKB process
	    ====================================================== */

	skb = (struct sk_buff *)re865x_mbuf2skb(pktHdr); /* mbuf/pkthdr is already freed after re865x_mbuf2skb() */

	//RTL865X_SET_PKTHDR(skb, pktHdr);
		
	if (skb == NULL)
	{
		goto out;
	}

	skb->dev = txExtDev;

	#ifdef RTL865X_DEBUG
	if (wlan_acc_debug>1)
	{
		rtlglue_printf("to pci (linkId %d) %s...\n", thisLink, skb->dev->name);
	}
	#endif

#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
	if ((_perpciStart == TRUE) && (_perpciPktCount == 0))
			_perpciSkb = skb_clone(skb, GFP_ATOMIC);
#endif

#if defined(CONFIG_RTL865XB_3G) ||defined(CONFIG_RTL865XB_GLUEDEV)
	/* for Globalsun 3G project, we set CB[] to indicate this packet is from ROMEDRV */
	strncpy(skb->cb, "FROM_ROMEDRV", 12);
#endif

/*for usrTunnel*/
#ifdef CONFIG_RTL865X_USR_DEFINE_TUNNEL
	strncpy(skb->cb,"FROM_ROMEDRV",12);
#endif

	/* ======================================================
								TX process
	    ====================================================== */

	ret = dev_queue_xmit(skb);

out:
	return;

free_out:
	mBuf_freeMbufChain(pktHdr->ph_mbuf);
}
#endif    

#else
/* empty function for non-multiple BSP */
int32 devglue_regExtDevice(int8 *devName, uint16 vid, uint8 extPortNum, uint32 *linkId){ return FAILED; }
int32 devglue_unregExtDevice(uint32 linkId){ return FAILED; }
int32 devglue_getLinkIDByName(int8 *devName){ return FAILED; }
int32 devglue_setExtDevVlanProperty(int8 *devName, uint16 vid, uint8 isMbr, uint8 isTag){ return FAILED; };
#endif	/* CONFIG_RTL865X_MULTILAYER_BSP */

#if 0
int32 rtlglue_extPortRecv(void *id, uint8 *data,  uint32 len, uint16 myvid, uint32 myportmask, uint32 linkID)
{
/* =========================================
	Dump Packet's information which is from extension devices
     ========================================= */

#ifdef CONFIG_RTL865XC

#ifdef NIC_DEBUG_PKTDUMP
	swNic_pktdump(	NIC_EXT_RX_PKTDUMP,
					NULL,	/* No Packet header */
					data,
					len,
					linkID);
#endif

#else/* 865xB */

#ifdef SWNIC_DEBUG
	if (nicDbgMesg)
	{
		if (nicDbgMesg&NIC_EXT_RX_PKTDUMP)
		{
			rtlglue_printf("WLAN->865x Len%d\n",  len);
			memDump(data,len>64?64:len,"");
		}
	}
#endif

#endif

/* =========================================
	Forward packet to driver
     ========================================= */

#if defined(CONFIG_RTL865X_MULTILAYER_BSP) || defined(CONFIG_RTL865X_LIGHT_ROMEDRV)
	return rtl8651_fwdEngineExtPortRecv(	id,
										data,
										len,
										myvid,
										myportmask,
										linkID);
#else
	return FAILED;
#endif
}

 
#define BDINFO_ADDR 0xbfc04000
void rtlglue_getMacAddress(ether_addr_t * macAddress, uint32 * number) {
#if CONFIG_RTL865X_MODULE_ROMEDRV
  	memcpy((char*)macAddress,(char*)BDINFO_ADDR,6);
#else  
	rtl8651_memcpy((char*)macAddress,(char*)BDINFO_ADDR,6);
#endif	
}

#ifdef CONFIG_RTL865XC 
void rtlglue_getRingSize(uint32 *rx, uint32 *tx, int whichDesc)
#else
void rtlglue_getRingSize(uint32 *rx, uint32 *tx)
#endif
{
#ifdef CONFIG_RTL865XC 

	if (rx)
	{
		swNic_getRingSize
							(
							SWNIC_GETRINGSIZE_RXRING,
							rx,
							whichDesc);
	}
	if (tx)
	{
		swNic_getRingSize
							(
							SWNIC_GETRINGSIZE_TXRING,
							tx,
							whichDesc);

	}

#else
	if(rx)
	{
		*rx = totalRxPkthdr;
	}
	if(tx)
	{
		*tx = totalTxPkthdr;
	}

#endif	
}
#endif

#endif /*CONFIG_RE865X*/

//extern int jiffies;
/*
 * Note: the unit of jiffies is 10ms.
 */

/*
 * getsectime() returns seconds.
 */
uint32 getsectime(void) {
	return jiffies/100;
		
}

/*
 * getmstime() returns milli-second (1/1000 sec).
 */
uint32 getmstime(void) {
        return jiffies*10;
}

uint32 rtlglue_getmstime( uint32* pTime )
{
	return *pTime = getmstime();
}


static int totalmem;
void *rtlglue_malloc(size_t NBYTES) {
	totalmem = totalmem+NBYTES;
	if(NBYTES==0) return NULL;
	return (void *)kmalloc(NBYTES,GFP_ATOMIC);
}

void rtlglue_free(void *APTR) {
	kfree(APTR);
}




/*
@func void	| rtlglue_srandom	| The OS independent seed function for random.
@parm uint32 | seed			| seed
@comm
 */
void rtlglue_srandom( uint32 seed )
{
	rtl_seed = seed;
}

/*
@func uint32	| rtlglue_random	| The OS independent random function.
@parm void | 			| 
@comm
 */
uint32 rtlglue_random( void )
{
	uint32 hi32, lo32;

	hi32 = (rtl_seed>>16)*19;
	lo32 = (rtl_seed&0xffff)*19+37;
	hi32 = hi32^(hi32<<16);
	return ( rtl_seed = (hi32^lo32) );
}
/*
@func uint32 | rtlglue_time	| The OS dependent time function.
@parm uint32* | t			| address to store time
@comm
 */
uint32 rtlglue_time( uint32* t )
{
	uint32 tt;

	rtlglue_getmstime( &tt );

	return (*t=tt/1000);
}

//------------------------  LINUX DEPEND GLUE STARTS HERE-------------//
#endif /*RTL865X_TEST*/

#if 0
#ifdef __linux__
/*
@func int | rtlglue_open	| The OS dependent open function.
@parm const char* | path			| filename or path
@parm int | flags			| file attribute
@parm int | mode			| file mode
@comm
 */
int rtlglue_open(const char *path, int flags, int mode )
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_open( path, flags, mode );
	set_fs(fs);        /* recover value*/
	return ret;
}

/*
@func int | rtlglue_read	| The OS dependent read function.
@parm int | fd			| file descriptor
@parm void* | buf			| read buffer
@parm int | nbytes			| number of bytes to read in buffer
@comm
 */
int rtlglue_read(int fd, void* buf, int nbytes)
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_read( fd, buf, nbytes );
	set_fs(fs);        /* recover value*/
	return ret;
}

/*
@func int | rtlglue_write	| The OS dependent write function.
@parm int | fd			| file descriptor
@parm void* | buf			| write buffer
@parm int | nbytes			| number of bytes to write in buffer
@comm
 */
int rtlglue_write(int fd, void* buf, int nbytes)
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_write( fd, buf, nbytes );
	set_fs(fs);        /* recover value*/
	return ret;
}

/*
@func int | rtlglue_close	| The OS dependent close function.
@parm int | fd			| file descriptor
@comm
 */
int rtlglue_close(int fd)
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_close( fd );
	set_fs(fs);        /* recover value*/
	return ret;
}
#else
/*
@ open in user space
 */
int rtlglue_open(const char *path, int flags, int mode )
{
	int ret;
	ret = open( path, flags, mode );
	return ret;
}

/*
@ write in user space
*/
int rtlglue_write(int fd, void* buf, int nbytes)
{
	int ret;
	ret = write( fd, buf, nbytes );
	return ret;
}

/*
@ close for user space
 */
int rtlglue_close(int fd)
{
	int ret;
	ret = close( fd );
	return ret;
}
#endif
#endif

// CONFIG_BRIDGE is undefined in 865x SDK
#undef CONFIG_BRIDGE

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
int32 rtlglue_extDevMakeBlocking(void * extDev)
{	
    	struct net_device *dev ;
	dev = extDev;
	if (dev == NULL)
		return FAILED;
#ifdef __linux__
	if (dev->br_port == NULL)
		return FAILED;
#if defined(CONFIG_BRIDGE) ||defined(CONFIG_BRIDGE_MODULE)	
	rtlglue_drvMutexLock();
	br_stp_enable_port(dev->br_port);
	rtlglue_drvMutexUnlock();	
#endif
#endif /*__linux__*/
	return SUCCESS;
}


int32 rtlglue_getExtDeviceName(void * device, char* name)
{
	if (device == NULL)
	{
		return FAILED;
	}
	rtlglue_drvMutexLock();

#ifdef __linux__
	if (name)
	{
		memcpy(	name,
					((struct net_device* )device)->name,
					sizeof(((struct net_device* )device)->name));
	}
#endif
	rtlglue_drvMutexUnlock();
	return SUCCESS;
}
#endif	/* defined(CONFIG_RTL865X) */


#if 0//ndef CONFIG_WIRELESS_LAN_MODULE
#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
EXPORT_SYMBOL(devglue_regExtDevice);
EXPORT_SYMBOL(devglue_getLinkIDByName);
EXPORT_SYMBOL(devglue_unregExtDevice);
EXPORT_SYMBOL(rtlglue_extPortRecv);
EXPORT_SYMBOL(devglue_setExtDevVlanProperty);
EXPORT_SYMBOL(rtlglue_reclaimRxBD);
EXPORT_SYMBOL(rtlglue_free);
EXPORT_SYMBOL(rtlglue_mbufTsleep);
EXPORT_SYMBOL(rtlglue_mbufMutexLock);
EXPORT_SYMBOL(rtlglue_mbufFreeCluster);
EXPORT_SYMBOL(rtlglue_drvMutexUnlock);
EXPORT_SYMBOL(rtl865x_getHZ);
EXPORT_SYMBOL(rtlglue_mbufMutexUnlock);
EXPORT_SYMBOL(rtlglue_mbufTwakeup);
EXPORT_SYMBOL(rtlglue_getmstime);
EXPORT_SYMBOL(rtlglue_init);
EXPORT_SYMBOL(rtlglue_drvMutexLock);
EXPORT_SYMBOL(rtlglue_getMacAddress);
EXPORT_SYMBOL(rtlglue_drvSend);
EXPORT_SYMBOL(rtlglue_mbufAllocCluster);
#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
EXPORT_SYMBOL(rtlglue_extDeviceSend);
#endif
EXPORT_SYMBOL(rtlglue_getRingSize);
EXPORT_SYMBOL(rtlglue_malloc);
EXPORT_SYMBOL(rtlglue_mbufClusterToData);
EXPORT_SYMBOL(rtlglue_mbufClusterRefcnt);
#endif	/* defined(CONFIG_RTL865X) */

#ifdef CONFIG_RTL865XB
EXPORT_SYMBOL(lx4180_writeCacheCtrl);
#endif

#ifdef	CONFIG_RTL865XB_EXP_CRYPTOENGINE
	#ifdef	CONFIG_RTL865XC
EXPORT_SYMBOL(rtlglue_srandom);
EXPORT_SYMBOL(rtlglue_random);

	#endif
#endif
#endif

#ifdef	CONFIG_RTL865XC
/*define for version control --Mark*/
#define RLRevID_OFFSET  12
#define RLRevID_MASK    0x0f
#define A_DIFF_B_ADDR   (PCI_CTRL_BASE+0x08) /*B800-3408*/


extern unsigned int chip_id, chip_revision_id;

/* get CHIP version */
int32 rtl8651_getChipVersion(int8 *name,uint32 size, int32 *rev)
{
	int32 revID;
	uint32 val;
	
	revID = ((READ_MEM32(CRMR)) >> RLRevID_OFFSET) & RLRevID_MASK ;

	if (chip_id == 0x8196) 
		strncpy(name,"8196B",size);
	else
		strncpy(name,"865xC",size);

	if(rev == NULL)
		return SUCCESS;

	if (chip_id == 0x8196) {
		*rev = chip_revision_id;
		return SUCCESS;
	}
		
	/*modified by Mark*/
	/*if RLRevID >= 1 V.B  *rev = RLRevID*/
	/*RLRevID == 0 ,then need to check [B800-3408]*/	

	if(revID >= RTL865X_CHIP_REV_B )
	   *rev = revID ;
	else /*A-CUT or B-CUT*/
	{
		val = READ_MEM32(A_DIFF_B_ADDR);
		if(val == 0)			
			*rev = RTL865X_CHIP_REV_A; /* RTL865X_CHIP_REV_A*/
		else
			*rev = RTL865X_CHIP_REV_B; /* RTL865X_CHIP_REV_B*/
	}	
	return SUCCESS;
}
#else
int32 rtl8651_getChipVersion(int8 *name,uint32 size, int32 *rev)
{

	uint32 id = READ_MEM32(CRMR);
	if(!id){
		id = READ_MEM32(CHIPID);
		if((uint16)(id>>16)==0x8650)
			strncpy(name,"8650",size);
		else
			strncpy(name,"8651",size);
		if(rev)
			*rev=0;
	}else if((uint16)id==0x5788){
		int revId;
		revId = (id>>16)&0xf;
#if 1
		/* chenyl: in 865xB rev.C, we can't and don't distinguish the difference between 8650B and 8651B */
		if (revId >= 2 /* rev.C */)
		{
			strncpy(name, "865xB", size);
		}else
#endif
		/* in 865xB rev.A and rev.B, we can distinguish the difference between 8650B and 8651B */
		{
			if(id&0x02000000)
				strncpy(name,"8651B",size);
			else
				strncpy(name,"8650B",size);
		}

		if(rev)
			*rev=revId;
	}else
		snprintf(name, size, "%08x", id);
	//rtlglue_printf("Read CRMR=%08x, CHIP=%08x\n", READ_MEM32(CRMR),READ_MEM32(CHIPID));
	return SUCCESS;
}

#endif	/* CONFIG_RTL865XC */
#if 0//ndef CONFIG_WIRELESS_LAN_MODULE
#ifdef	CONFIG_RTL865XC
EXPORT_SYMBOL(rtl8651_getChipVersion);
#endif
#endif
