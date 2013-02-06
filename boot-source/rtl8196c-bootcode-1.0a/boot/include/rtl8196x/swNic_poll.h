/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
*
* Abstract: Switch core polling mode NIC header file.
*
* ---------------------------------------------------------------
*/


#ifndef _SWNIC_H
#define _SWNIC_H
#define CONFIG_RTL865XC 1
#ifdef CONFIG_RTL865XC
/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch NIC.
 * INPUT   : 
        userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC]: Number of Rx pkthdr descriptors. of each ring
        userNeedRxMbufRingCnt: Number of Rx mbuf descriptors.
        userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC]: Number of Tx pkthdr descriptors. of each ring
        clusterSize: Size of a mbuf cluster.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns ENOERR. 
        Otherwise, 
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[],
                 uint32 clusterSize);
#endif /* CONFIG_RTL865XC */



/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_intHandler
 * --------------------------------------------------------------------
 * FUNCTION: This function is the NIC interrupt handler.
 * INPUT   :
		intPending: Pending interrupts.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
void swNic_intHandler(uint32 intPending);

int32 swNic_receive(void** input, uint32* pLen);
int32 swNic_send(void * output, uint32 len);
void swNic_txDone(void); 

#endif /* _SWNIC_H */
