/*
 *  Header files defines some RX inline routines
 *
 *  $Id: 8192cd_rx.h,v 1.4.4.6 2011/03/01 13:36:59 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_RX_H_
#define _8192CD_RX_H_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"

#define SN_NEXT(n)		((n + 1) & 0xfff)
#define SN_LESS(a, b)	(((a - b) & 0x800) != 0)
#define SN_DIFF(a, b)	((a >= b)? (a - b):(0xfff - b + a + 1))


#define init_frinfo(pinfo) \
	do	{	\
			pinfo->pskb = pskb;		\
			pinfo->rssi = 0;		\
			INIT_LIST_HEAD(&(pinfo->mpdu_list)); \
			INIT_LIST_HEAD(&(pinfo->rx_list)); \
	} while(0)

static __inline__ void init_rxdesc(struct sk_buff *pskb, int i, struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw	*phw;
	struct rx_frinfo	*pfrinfo;
	int offset;

	phw = GET_HW(priv);

	offset = 0x20 - ((((unsigned long)pskb->data) + sizeof(struct rx_frinfo)) & 0x1f);	// need 32 byte aligned
	skb_reserve(pskb, sizeof(struct rx_frinfo) + offset);
	pfrinfo = get_pfrinfo(pskb);

	init_frinfo(pfrinfo);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	pfrinfo->is_br_mgnt = 0;
#endif
#ifdef RX_BUFFER_GATHER
	pfrinfo->gather_flag = 0;
#endif
	phw->rx_infoL[i].pbuf  = (void *)pskb;
	phw->rx_infoL[i].paddr = get_physical_addr(priv, pskb->data, (RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
	phw->rx_descL[i].Dword6 = set_desc(phw->rx_infoL[i].paddr);
	rtl_cache_sync_wback(priv, phw->rx_infoL[i].paddr, RX_BUF_LEN - sizeof(struct rx_frinfo)-64, PCI_DMA_FROMDEVICE);
	phw->rx_descL[i].Dword0 = set_desc((i == (NUM_RX_DESC - 1)? RX_EOR : 0) | RX_OWN |((RX_BUF_LEN - sizeof(struct rx_frinfo)-64) & RX_PktLenMask));	//32 for alignment, 32 for TKIP MIC

}

static __inline__ unsigned char cal_rssi_avg(unsigned int agv, unsigned int pkt_rssi)
{
	unsigned int rssi;

	rssi = ((agv * 19) + pkt_rssi) / 20;
	if (pkt_rssi > agv)
		rssi++;

	return (unsigned char)rssi;
}


static __inline__ void update_sta_rssi(struct rtl8192cd_priv *priv,
				struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	int i;

#ifdef SW_ANT_SWITCH
	if(priv->pshare->RSSI_test == TRUE)
		return;
#endif

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		if (priv->pshare->rf_ft_var.rssi_dump) {
			priv->pshare->mp_rssi = cal_rssi_avg(priv->pshare->mp_rssi, pfrinfo->rssi);
			priv->pshare->mp_sq   = pfrinfo->sq;
			for (i=0; i<2; i++)
				priv->pshare->mp_rf_info.mimorssi[i] = cal_rssi_avg(priv->pshare->mp_rf_info.mimorssi[i], pfrinfo->rf_info.mimorssi[i]);
			memcpy(&priv->pshare->mp_rf_info.mimosq[0], &pfrinfo->rf_info.mimosq[0], sizeof(struct rf_misc_info) - 2);
		}
		return;
	}
#endif

	if (pfrinfo->physt) {
			if (pfrinfo->rssi) {

#if defined(HW_ANT_SWITCH)
			if (pfrinfo->driver_info_size > 0) {
				unsigned char *phystatus =	(unsigned char*)pfrinfo->driver_info;			 
				int i = 1&(phystatus[27]>>7);
				if (is_CCK_rate(pfrinfo->rx_rate)) 
					++(pstat->cckPktCount[i]);				
				else 
					++(pstat->hwRxAntSel[i]);			
				if(!pstat->AntRSSI[i])
					pstat->AntRSSI[i] = pfrinfo->rssi;
				else
					pstat->AntRSSI[i] = cal_rssi_avg(pstat->AntRSSI[i], pfrinfo->rssi);
	
				if(priv->pshare->rf_ft_var.ant_dump&1) 
					panic_printk("pkt--> cck:%d, a:%d, R:(%d) L:%d\n", is_CCK_rate(pfrinfo->rx_rate), i, pfrinfo->rssi, pfrinfo->pktlen);	
	
			}	
#endif	
			
			pstat->rssi             = cal_rssi_avg(pstat->rssi, pfrinfo->rssi);
			pstat->sq               = pfrinfo->sq;
			pstat->rx_rate          = pfrinfo->rx_rate;
			pstat->rx_bw            = pfrinfo->rx_bw;
			pstat->rx_splcp         = pfrinfo->rx_splcp;

			if (pfrinfo->rf_info.mimorssi[0] != 0)
				for (i=0; i<2; i++)
					pstat->rf_info.mimorssi[i] = cal_rssi_avg(pstat->rf_info.mimorssi[i], pfrinfo->rf_info.mimorssi[i]);

			if (priv->pshare->rf_ft_var.rssi_dump)
				memcpy(&pstat->rf_info.mimosq[0], &pfrinfo->rf_info.mimosq[0], sizeof(struct rf_misc_info) - 2);

			if (pstat->highest_rx_rate < pstat->rx_rate)
				pstat->highest_rx_rate = pstat->rx_rate;
		}
	}
}
#endif // _8192CD_RX_H_

