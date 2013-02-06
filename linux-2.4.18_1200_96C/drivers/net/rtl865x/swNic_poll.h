/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/swNic_poll.h,v 1.11.4.3 2011/04/29 02:40:03 bruce Exp $
*
* Abstract: Switch core polling mode NIC header file.
*
* $Author: bruce $
*
* $Log: swNic_poll.h,v $
* Revision 1.11.4.3  2011/04/29 02:40:03  bruce
* merge vlan modification for zion into branch
*
* Revision 1.11.4.2  2010/12/16 05:32:20  bruce
* merge vlan bridge/route selection
*
* Revision 1.11.4.1  2010/11/29 03:27:10  keith_huang
* Add ipv6 pass throuth supported
*
* Revision 1.11  2010/01/18 07:04:11  joeylin
* use 1 rx/tx pkthdr descriptor ring for 8196c
*
* Revision 1.10  2009/03/23 11:51:08  jasonwang
* Added rtl8366 giga demo board support.
*
* Revision 1.9  2009/02/15 13:56:25  joeylin
* support Guest Zone feature for D-Link 605 project
*
* Revision 1.8  2008/12/29 01:52:34  joeylin
* remove CONFIG_RTL8196B_PATCH1 flag
*
* Revision 1.7  2008/11/13 08:58:58  jasonwang
* Modified for version B to disbale switch PATCH.
*
* Revision 1.6  2008/10/24 11:09:06  davidhsu
* Update for pana
*
* Revision 1.5  2008/10/23 12:49:23  jasonwang
* Modified definition for RTL8196B.
*
* Revision 1.4  2008/10/06 14:28:48  jasonwang
* Added software support for rtl8196b.
*
* Revision 1.3  2008/09/18 08:16:43  joeylin
* fix LAN broadcasting issue
*
* Revision 1.2  2007/11/11 02:51:27  davidhsu
* Fix the bug that do not fre rx skb in rx descriptor when driver is shutdown
*
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.4  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.3  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/


#ifndef _SWNIC_H
#define _SWNIC_H
#include <linux/config.h>
/* refer to rtl865xc_swNic.c & rtl865xc_swNic.h
 */
#ifdef CONFIG_RTL8196C
#define RTL865X_SWNIC_RXRING_MAX_PKTDESC    1
#define RTL865X_SWNIC_TXRING_MAX_PKTDESC    1
#else 
#define RTL865X_SWNIC_RXRING_MAX_PKTDESC    6
#define RTL865X_SWNIC_TXRING_MAX_PKTDESC    2
#endif
#define UNCACHE_MASK   0x20000000

#define WAN_VID 	0x8
#define LAN_VID 		0x9
#define LAN2_VID 	0xa
#define LAN3_VID 	0xb
#define LAN4_VID 	0xc
#define LAN5_VID 	0xd
#define LAN6_VID 	0xe

#ifdef CONFIG_RTL8196_RTL8366
#define	RTL_WANPORT_MASK		0x1C1
#define	RTL_LANPORT_MASK		0x1C1
#define RTL8366RB_GMIIPORT		0x20
#define RTL8366RB_LANPORT		0xEF
#define RTL8366RB_WANPORT		0x10
#define	RTL_WANVLANID			WAN_VID
#define	RTL_LANVLANID			LAN_VID
#define	RTL_PPTPL2TP_VLANID	999

#endif

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

//int32 swNic_receive(void** input, uint32* pLen);
int32 swNic_receive(void** input, uint32* pLen, unsigned int *vid, unsigned int *pid);
//int32 swNic_send(void * output, uint32 len);
int32 swNic_send(void *skb, void * output, uint32 len, unsigned int vid, unsigned int pid);
int32 swNic_txDone(void);
void swNic_freeRxBuf(void);


//exported function by eth8186.c
extern void *UNCACHED_MALLOC(int size);
extern unsigned char *alloc_rx_buf(void **skb, int buflen);
extern void free_rx_buf(void *skb);
extern void tx_done_callback(void *skb);
extern void eth_save_and_cli(unsigned long *flags);
extern void eth_restore_flags(unsigned long flags);

#endif /* _SWNIC_H */
