/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/rtl865xc_swNic.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
*
* Abstract: Switch core NIC header file for RTL865XC.
*
* $Author: joeylin $
*
* ---------------------------------------------------------------
*/
#ifndef RTL865XC_SWNIC_H
#define RTL865XC_SWNIC_H

/* =======================================================================
		Internal control compilation-flags
    ======================================================================= */

#include "mbuf.h"

/* =======================================================================
		Internal control compilation-flags
    ======================================================================= */

/* <-------------------------- RX Align mode --------------------------> */
#define SWNIC_RX_ALIGNED_IPHDR

/* <-------------------------- Statistics mode --------------------------> */
#define SWNIC_STATISTICS

/* <-------------------------- Enable TX CallBack function call --------------------------> */
#define SWNIC_TX_CALLBACK

/* <-------------------------- Accurate statistics mode--------------------------> */
/*
	If this mode is turned ON. SwNIC statistics would be more accurate, but performance might be hurt.
*/
#undef SWNIC_ACCURATE_STATISTICS

/* <-------------------------- SwNIC Rx/Tx ring count  --------------------------> */
#ifdef CONFIG_RTL865XB
#define RTL865X_SWNIC_RXRING_MAX_PKTDESC		1
#define RTL865X_SWNIC_TXRING_MAX_PKTDESC		1
#else
#define RTL865X_SWNIC_RXRING_MAX_PKTDESC		6
#define RTL865X_SWNIC_TXRING_MAX_PKTDESC		2
#endif
 
/* <-------------------------- Early STOP mechanism in swNIC --------------------------> */
#undef SWNIC_EARLYSTOP 

/* <-------------------------- Tx-Align mechanism in swNIC --------------------------> */
#undef SWNIC_TXALIGN
#ifdef CONFIG_RTL865XB
/* Only RTL865xB Chip support Tx Alignment for internal packet length alignment. */
#define SWNIC_TXALIGN
#endif

/* <-------------------------- Fast external device support--------------------------> */
#ifdef AIRGO_FAST_PATH
#define RTL865X_SWNIC_FAST_EXTDEV_SUPPORT
#else
#undef  RTL865X_SWNIC_FAST_EXTDEV_SUPPORT
#endif

/* =======================================================================
		External Structure / Variables
    ======================================================================= */
extern int8 swNic_Id[];
extern spinlock_t *rtl865xSpinlock;

typedef int32 (*proc_input_pkt_funcptr_t) (struct rtl_pktHdr*);		/* Hooking point of packet processing for bottom-half */

#ifdef SWNIC_TX_CALLBACK
typedef void (*swNic_txCallBack_funcptr_t)(unsigned long);		/* Tx CallBack function for SWNIC */
#endif
/* =======================================================================
		External Functions / hooking points
    ======================================================================= */
__IRAM_FWD void swNic_txRecycle(uint32 dummy);
__IRAM_FWD int32 swNic_isrReclaim(	uint32 rxPkthdrRingProcIndex,
										uint32 rxPkthdrDescIndex,
										struct rtl_pktHdr *pkt_p,
										struct rtl_mBuf *firstMbuf_p);
int32 swNic_isrTxRecycle(int32 dummy, int32 txPkthdrRingIndex);
int32 swNic_txRxSwitch(int32 tx, int32 rx);
int32 swNic_installRxCallBackFunc(	proc_input_pkt_funcptr_t rxFastProcFunc,
										proc_input_pkt_funcptr_t rxPreProcFunc,
										proc_input_pkt_funcptr_t rxProcFunc,
										proc_input_pkt_funcptr_t* orgRxFastProcFunc_p,
										proc_input_pkt_funcptr_t* orgRxPreProcFunc_p,
										proc_input_pkt_funcptr_t* orgRxProcFunc_p);


/* =================================================
		SwNIC Bottom-HALF
    ================================================= */
int32 swNic_intHandler(int32 *param);
void swNic_rxThread(unsigned long param);

/* =================================================
		SwNIC Top-HALF
    ================================================= */
#define RTL865X_SWNIC_ETHERNET_MIN_TRASFER_UNIT	60

int32 swNic_init(	uint32 userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC],
					uint32 userNeedRxMbufRingCnt,
					uint32 userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC],
					uint32 clusterSize,
					proc_input_pkt_funcptr_t rxProcFunc, 
					proc_input_pkt_funcptr_t rxPreProcFunc,
					proc_input_pkt_funcptr_t rxFastProcFunc);
void swNic_reset(void);
int32 swNic_hwSetup(void);
int32 swNic_write(void *outPkt, int32 txPkthdrRingIndex);
int32 swNic_descRingCleanup(void);

#define SWNIC_GETRINGSIZE_RXRING		1
#define SWNIC_GETRINGSIZE_TXRING		2
#define SWNIC_GETRINGSIZE_MBUFRING	3
int32 swNic_getRingSize(uint32 ringType, uint32 *ringSize, uint32 ringIdx);

int32 swNic_enableSwNicWriteTxFN(uint32 enable);

#ifdef SWNIC_TX_CALLBACK
void swNic_registerTxCallBackFunc(swNic_txCallBack_funcptr_t txCallBack_f);
#endif
/* =================================================
		SwNIC Statistics
    ================================================= */
typedef struct swNicCounter_s {
	uint32 rxIntNum, txIntNum, rxPkthdrRunoutNum, rxMbufRunoutNum, rxPkthdrRunoutSolved, rxPktErrorNum, txPktErrorNum, rxPktCounter, txPktCounter;
	uint32 linkChanged;
	uint32 maxCountinuousRxCount;
	uint32 fastExtDevRxCount;
	uint32 rxPreProcessFailCount;


	uint32 currRxPkthdrRingIndex[RTL865X_SWNIC_RXRING_MAX_PKTDESC];
	uint32 currMbufRingIndex;

	uint32 currTxPkthdrRingDoneIndex[RTL865X_SWNIC_TXRING_MAX_PKTDESC];
	uint32 currTxPkthdrRingFreeIndex[RTL865X_SWNIC_TXRING_MAX_PKTDESC];
} swNicCounter_t;

#ifdef SWNIC_STATISTICS

#define STATISTICS_INC(var, cnt)		do {		(var) += (cnt);		} while(0)
#define STATISTICS_RESET(var)		do {		(var) = 0;			} while(0)

#else

#define STATISTICS_INC(var, cnt)
#define STATISTICS_RESET(var)

#endif

/* =================================================
		SwNIC Debugging messages
    ================================================= */
/* ---------------------------------------------------
	Debugging meesage related masks
    --------------------------------------------------- */
#define NIC_DEBUG_PKTDUMP_MASK	(0x000000ff)
#define NIC_DEBUG_DBGMSG_MASK	(0x0000ff00)

/* <------------ Packet dump ------------> */
#define NIC_PHY_RX_PKTDUMP	(1 << 0)
#define NIC_PHY_TX_PKTDUMP	(1 << 1)
#define NIC_EXT_RX_PKTDUMP		(1 << 2)
#define NIC_EXT_TX_PKTDUMP		(1 << 3)
#define NIC_PS_RX_PKTDUMP		(1 << 4)
#define NIC_PS_TX_PKTDUMP		(1 << 5)

	/* Complex */
#define NIC_ALL_RX_PKTDUMP		(	NIC_PHY_RX_PKTDUMP |\
									NIC_EXT_RX_PKTDUMP |\
									NIC_PS_RX_PKTDUMP		)
#define NIC_ALL_TX_PKTDUMP		(	NIC_PHY_TX_PKTDUMP |\
									NIC_EXT_TX_PKTDUMP |\
									NIC_PS_TX_PKTDUMP		)

#define NIC_PHY_PKTDUMP		(	NIC_PHY_RX_PKTDUMP |\
									NIC_PHY_TX_PKTDUMP		)

#define NIC_EXT_PKTDUMP		(	NIC_EXT_RX_PKTDUMP |\
									NIC_EXT_TX_PKTDUMP		)

#define NIC_PS_PKTDUMP			(	NIC_PS_RX_PKTDUMP |\
									NIC_PS_TX_PKTDUMP		)

#define NIC_ALL_PKTDUMP		(	NIC_ALL_RX_PKTDUMP |\
									NIC_ALL_TX_PKTDUMP		)


/* <------------ Debugging message ------------> */
#define NIC_DBGMSG_TRACE_IN		(1 << 8)
#define NIC_DBGMSG_TRACE			(1 << 9)
#define NIC_DBGMSG_TRACE_OUT		(1 << 10)
#define NIC_DBGMSG_INFO			(1 << 11)
#define NIC_DBGMSG_WARN			(1 << 12)
#define NIC_DBGMSG_ERROR			(1 << 13)
#define NIC_DBGMSG_FATAL			(1 << 14)

/* ====================================
		Switch to turn ON/OFF debugging messages
    ==================================== */

#define SWNIC_DEBUG  
#ifdef SWNIC_DEBUG 

#ifdef CONFIG_RTL865X_CLE
#define NIC_DEBUG_PKTDUMP
#undef NIC_DEBUG_PKTGEN
#endif

/*
	Default Debugging setup

		NIC_DEBUG_DEFAULT_VALUE		: Decided @ compile time
		NIC_DEBUGMSG_DEFAULT_VALUE	: Can be changed @ run time		( if one mask is disabled in compile time, it can not be enabled in run time )
*/
#define NIC_DEBUG_DEFAULT_VALUE		(NIC_DBGMSG_ERROR|NIC_DBGMSG_FATAL)
#define NIC_DEBUGMSG_DEFAULT_VALUE	(NIC_DBGMSG_ERROR|NIC_DBGMSG_FATAL)

#else
#define NIC_DEBUG_DEFAULT_VALUE		0
#define NIC_DEBUGMSG_DEFAULT_VALUE	0
#endif

/* ================================
		Debugging messages
    ================================ */

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_TRACE_IN)
#define NIC_TRACE_IN(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_TRACE_IN) \
	{ \
		rtlglue_printf("[%s-%d] -TRACE IN-: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_TRACE_IN(fmt, args...)
#endif

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_TRACE)
#define NIC_TRACE(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_TRACE) \
	{ \
		rtlglue_printf("[%s-%d] -TRACE-: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_TRACE(fmt, args...)
#endif

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_TRACE_OUT)
#define NIC_TRACE_OUT(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_TRACE_OUT) \
	{ \
		rtlglue_printf("[%s-%d] -TRACE OUT-: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_TRACE_OUT(fmt, args...)
#endif

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_INFO)
#define NIC_INFO(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_INFO) \
	{ \
		rtlglue_printf("[%s-%d] -INFO-: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_INFO(fmt, args...)
#endif

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_WARN)
#define NIC_WARN(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_WARN) \
	{ \
		rtlglue_printf("[%s-%d] -WARN-: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_WARN(fmt, args...)
#endif

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_ERROR)
#define NIC_ERR(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_ERROR) \
	{ \
		rtlglue_printf("\n\n\n[%s-%d] -!! ERROR !!-: " fmt "\n\n\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_ERR(fmt, args...)
#endif

#if (NIC_DEBUG_DEFAULT_VALUE & NIC_DBGMSG_FATAL)
#define NIC_FATAL(fmt, args...) \
	if ((nicDbgMesg) &  NIC_DBGMSG_FATAL) \
	{ \
		rtlglue_printf("\n\n\n[%s-%d] -!! FATAL !!-: " fmt "\n\n\n", __FUNCTION__, __LINE__, ## args); \
	}
#else
#define NIC_FATAL(fmt, args...)
#endif

/* ================================
	Part to co-operate with CLESHELL
    ================================ */
 
#ifdef NIC_DEBUG_PKTDUMP

#define NIC_PKTDUMP_MAXLEN	64

#define NIC_PRINT(fmt, args...) \
	{ \
		rtlglue_printf("[%s] -: " fmt "\n", __FUNCTION__, ## args); \
	}

void swNic_pktdump(	uint32 currentCase,
						struct rtl_pktHdr *pktHdr_p,
						char *pktContent_p,
						uint32 pktLen,
						uint32 additionalInfo);
#endif


/* =================================================
		CLE Shell support
    ================================================= */
#ifdef CONFIG_RTL865X_CLE
#include "cle/rtl_cle.h"
extern cle_exec_t swNic_cmds[];
#ifdef NIC_DEBUG_PKTDUMP
#define CMD_RTL8651_SWNIC_CMD_NUM		7
#else
#define CMD_RTL8651_SWNIC_CMD_NUM		6
#endif
#endif	/* CONFIG_RTL865X_CLE */

/* =================================================
		Ring infomation support
    ================================================= */
void swNic_dumpPkthdrDescRing(void);
void swNic_dumpTxDescRing(void);
void swNic_dumpMbufDescRing(void);

/* =================================================
		Fast extension device support
    ================================================= */
#ifdef RTL865X_SWNIC_FAST_EXTDEV_SUPPORT
typedef uint32 (*swNic_FastExtDevFreeCallBackFuncType_f)(void * pkt);
int32 swNic_registerFastExtDevFreeCallBackFunc(swNic_FastExtDevFreeCallBackFuncType_f callBackFunc);
int32 swNic_fastExtDevFreePkt(struct rtl_pktHdr *pktHdr_p, struct rtl_mBuf *mbuf_p);
#endif

/* =================================================
		Rome-perf support
    ================================================= */
#ifdef CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION
void swNic_pernicrxStart(int32 instMode, uint32 totalPkt);
void swNic_pernicrxEnd(uint32 *pktCount, uint32 *byteCount);
#endif	/* CONFIG_RTL865XB_EXP_PERFORMANCE_EVALUATION */

#endif

