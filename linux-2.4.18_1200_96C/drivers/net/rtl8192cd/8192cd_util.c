/*
 *  Utility routines
 *
 *  $Id: 8192cd_util.c,v 1.52.2.32 2011/05/12 09:46:14 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_UTILS_C_

#ifdef __KERNEL__
#include <linux/circ_buf.h>
#include <linux/sched.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif


UINT8 Realtek_OUI[]={0x00, 0xe0, 0x4c};
UINT8 dot11_rate_table[]={2,4,11,22,12,18,24,36,48,72,96,108,0}; // last element must be zero!!

#define WLAN_PKT_FORMAT_ENCAPSULATED	0x01
#define WLAN_PKT_FORMAT_CDP				0x06
#ifndef CONFIG_RTK_MESH  // mesh project moves these define to 8190n_headers.h
#define WLAN_PKT_FORMAT_SNAP_RFC1042	0x02
#define WLAN_PKT_FORMAT_SNAP_TUNNEL		0x03
#define WLAN_PKT_FORMAT_IPX_TYPE4		0x04
#define WLAN_PKT_FORMAT_APPLETALK		0x05
#define WLAN_PKT_FORMAT_OTHERS			0x07
#endif

unsigned char SNAP_ETH_TYPE_IPX[2] = {0x81, 0x37};
unsigned char SNAP_ETH_TYPE_APPLETALK_AARP[2] = {0x80, 0xf3};
unsigned char SNAP_ETH_TYPE_APPLETALK_DDP[2] = {0x80, 0x9B};
unsigned char SNAP_HDR_APPLETALK_DDP[3] = {0x08, 0x00, 0x07}; // Datagram Delivery Protocol

#ifndef _11s_TEST_MODE_
 static
#endif
__inline__ void release_buf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf, struct list_head	*phead, unsigned int *count)
{
	struct list_head *plist;
#if 0
	if (*count >= PRE_ALLOCATED_HDR) {
		_DEBUG_ERR("over size free buf phead=%X, *count=%d\n", (unsigned int)phead, *count);
		return;
	}
#endif

	*count = *count + 1;
	plist = (struct list_head *)((unsigned int)pbuf - sizeof(struct list_head));
	list_add_tail(plist, phead);
}

#ifndef _11s_TEST_MODE_
 static
#endif
__inline__ unsigned char *get_buf_from_poll(struct rtl8192cd_priv *priv, struct list_head *phead, unsigned int *count)
{
	unsigned char *buf;
	struct list_head *plist;

	if (list_empty(phead)) {
//		_DEBUG_ERR("phead=%X buf is empty now!\n", (unsigned int)phead);
		return NULL;
	}

	if (*count == 0) {
		_DEBUG_ERR("phead=%X under-run!\n", (unsigned int)phead);
		return NULL;
	}

	*count = *count - 1;
	plist = phead->next;
	list_del_init(plist);
	buf = (UINT8 *)((unsigned int)plist + sizeof (struct list_head));
	return buf;
}

#ifdef PRIV_STA_BUF
struct priv_obj_buf {
	unsigned char magic[8];
	struct list_head	list;
	struct aid_obj obj;
};

#if defined(WIFI_WMM) && defined(WMM_APSD)
struct priv_apsd_que {
	unsigned char magic[8];
	struct list_head	list;
	struct apsd_pkt_queue que;
};
#endif

#if defined(WIFI_WMM)
struct priv_dz_mgt_que {
	unsigned char magic[8];
	struct list_head	list;
	struct dz_mgmt_queue que;
};
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
struct priv_wpa_buf {
	unsigned char magic[8];
	struct list_head	list;
	WPA_STA_INFO wpa;
};
#endif

#define MAGIC_CODE_BUF			"8192"

#define MAX_PRIV_OBJ_NUM		NUM_STAT
static struct priv_obj_buf obj_buf[MAX_PRIV_OBJ_NUM];
static struct list_head objbuf_list;
static int free_obj_buf_num;


#if defined(WIFI_WMM) && defined(WMM_APSD)
	#define MAX_PRIV_QUE_NUM	(MAX_PRIV_OBJ_NUM*4)
	static struct priv_apsd_que que_buf[MAX_PRIV_QUE_NUM];
	static struct list_head quebuf_list;
	static int free_que_buf_num;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	#define MAX_PRIV_WPA_NUM	MAX_PRIV_OBJ_NUM
	static struct priv_wpa_buf wpa_buf[MAX_PRIV_WPA_NUM];
	static struct list_head wpabuf_list;
	static int free_wpa_buf_num;
#endif

void init_priv_sta_buf(struct rtl8192cd_priv *priv)
{
	int i;
	memset(obj_buf, '\0', sizeof(struct priv_obj_buf)*MAX_PRIV_OBJ_NUM);
	INIT_LIST_HEAD(&objbuf_list);
	for (i=0; i<MAX_PRIV_OBJ_NUM; i++)  {
		memcpy(obj_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&obj_buf[i].list);
		list_add_tail(&obj_buf[i].list, &objbuf_list);
	}
	free_obj_buf_num = i;

#if defined(WIFI_WMM) && defined(WMM_APSD)
	memset(que_buf, '\0', sizeof(struct priv_apsd_que)*MAX_PRIV_QUE_NUM);
	INIT_LIST_HEAD(&quebuf_list);
	for (i=0; i<MAX_PRIV_QUE_NUM; i++)  {
		memcpy(que_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&que_buf[i].list);
		list_add_tail(&que_buf[i].list, &quebuf_list);
	}
	free_que_buf_num = i;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	memset(wpa_buf, '\0', sizeof(struct priv_wpa_buf)*MAX_PRIV_WPA_NUM);
	INIT_LIST_HEAD(&wpabuf_list);
	for (i=0; i<MAX_PRIV_WPA_NUM; i++)  {
		memcpy(wpa_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&wpa_buf[i].list);
		list_add_tail(&wpa_buf[i].list, &wpabuf_list);
	}
	free_wpa_buf_num = i;
#endif
}

struct aid_obj *alloc_sta_obj(struct rtl8192cd_priv *priv)
{
	unsigned long flags;
	struct aid_obj *priv_obj;

	SAVE_INT_AND_CLI(flags);
	priv_obj = (struct aid_obj  *)get_buf_from_poll(NULL, &objbuf_list, (unsigned int *)&free_obj_buf_num);
	RESTORE_INT(flags);

#ifdef __KERNEL__
	if (priv_obj == NULL)
		return ((struct aid_obj *)kmalloc(sizeof(struct aid_obj), GFP_ATOMIC));
	else
#endif		
		return priv_obj;
}

void free_sta_obj(struct rtl8192cd_priv *priv, struct aid_obj *obj)
{
	unsigned long offset = (unsigned long)(&((struct priv_obj_buf *)0)->obj);
	struct priv_obj_buf *priv_obj = (struct priv_obj_buf *)(((unsigned long)obj) - offset);
	unsigned long flags;

	if (!memcmp(priv_obj->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_obj->obj) ==  ((unsigned long)obj)) {
		SAVE_INT_AND_CLI(flags);

		release_buf_to_poll(priv, (unsigned char *)obj, &objbuf_list, (unsigned int *)&free_obj_buf_num);

		RESTORE_INT(flags);
	}
	else 
#ifdef __ECOS
		ASSERT(0);
#else
		kfree(obj);
#endif
}

#if defined(WIFI_WMM) && defined(WMM_APSD)
static struct apsd_pkt_queue *alloc_sta_que(struct rtl8192cd_priv *priv)
{
	unsigned long flags;
	struct apsd_pkt_queue *priv_que;
	SAVE_INT_AND_CLI(flags);
	priv_que = (struct apsd_pkt_queue*)get_buf_from_poll(priv, &quebuf_list, (unsigned int *)&free_que_buf_num);
	RESTORE_INT(flags);

#ifdef __KERNEL__
	if (priv_que == NULL)
		return ((struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC));
	else
#endif		
		return priv_que;
}

void free_sta_que(struct rtl8192cd_priv *priv, struct apsd_pkt_queue *que)
{
	unsigned long offset = (unsigned long)(&((struct priv_apsd_que *)0)->que);
	struct priv_apsd_que *priv_que = (struct priv_apsd_que *)(((unsigned long)que) - offset);
	unsigned long flags;

	if (!memcmp(priv_que->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_que->que) ==  ((unsigned long)que)) {
		SAVE_INT_AND_CLI(flags);
			
		release_buf_to_poll(priv, (unsigned char *)que, &quebuf_list, (unsigned int *)&free_que_buf_num);

		RESTORE_INT(flags);		
	}
	else
#ifdef __ECOS
		ASSERT(0);
#else
		kfree(que);
#endif
}
#endif // defined(WIFI_WMM) && defined(WMM_APSD)

#if defined(WIFI_WMM)
static struct dz_mgmt_queue *alloc_sta_mgt_que(struct rtl8192cd_priv *priv)
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);

	struct dz_mgmt_queue *priv_que = (struct dz_mgmt_queue*)get_buf_from_poll(priv, &quebuf_list, (unsigned int *)&free_que_buf_num);

	RESTORE_INT(flags);

#ifdef __KERNEL__
	if (priv_que == NULL)
		return ((struct dz_mgmt_queue *)kmalloc(sizeof(struct dz_mgmt_queue), GFP_ATOMIC));
	else
#endif		
		return priv_que;
}

void free_sta_mgt_que(struct rtl8192cd_priv *priv, struct dz_mgmt_queue *que)
{
	unsigned long offset = (unsigned long)(&((struct priv_dz_mgt_que *)0)->que);
	struct priv_dz_mgt_que *priv_que = (struct priv_dz_mgt_que *)(((unsigned long)que) - offset);
	unsigned long flags;

	if (!memcmp(priv_que->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_que->que) ==  ((unsigned long)que)) {
		SAVE_INT_AND_CLI(flags);
			
		release_buf_to_poll(priv, (unsigned char *)que, &quebuf_list, (unsigned int *)&free_que_buf_num);

		RESTORE_INT(flags);		
	}
	else
		kfree(que);
}
#endif // defined(WIFI_WMM) 


#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
static WPA_STA_INFO *alloc_wpa_buf(struct rtl8192cd_priv *priv)
{
	unsigned long flags;
	WPA_STA_INFO *priv_buf;
	SAVE_INT_AND_CLI(flags);
	priv_buf = (WPA_STA_INFO *)get_buf_from_poll(priv, &wpabuf_list, (unsigned int *)&free_wpa_buf_num);
	RESTORE_INT(flags);

#ifdef __ECOS
	ASSERT(priv_buf != NULL);
	return priv_buf;

#else	
	if (priv_buf == NULL)
		return ((WPA_STA_INFO *)kmalloc(sizeof(WPA_STA_INFO), GFP_ATOMIC));
	else
		return priv_buf;
#endif		
}

void free_wpa_buf(struct rtl8192cd_priv *priv, WPA_STA_INFO *buf)
{
	unsigned long offset = (unsigned long)(&((struct priv_wpa_buf *)0)->wpa);
	struct priv_wpa_buf *priv_buf = (struct priv_wpa_buf *)(((unsigned long)buf) - offset);
	unsigned long flags;

	if (!memcmp(priv_buf->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_buf->wpa) ==  ((unsigned long)buf)) {
		SAVE_INT_AND_CLI(flags);
			
		release_buf_to_poll(priv, (unsigned char *)buf, &wpabuf_list, (unsigned int *)&free_wpa_buf_num);

		RESTORE_INT(flags);
	}
	else
#ifdef __ECOS
		ASSERT(0);
#else	
		kfree(buf);
#endif
}
#endif // INCLUDE_WPA_PSK
#endif // PRIV_STA_BUF


#ifdef CONFIG_RTL8190_PRIV_SKB
static struct sk_buff *dev_alloc_skb_priv(struct rtl8192cd_priv *priv, unsigned int size);
#endif

int	enque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned int ffptr, int ffsize, void *elm)
{
	// critical section!
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);
	if (CIRC_SPACE(*head, *tail, ffsize) == 0) {
		RESTORE_INT(flags);
		return FALSE;
	}

	*(unsigned int *)(ffptr + (*head)*(sizeof(void *))) = (unsigned int)elm;
	*head = (*head + 1) & (ffsize - 1);
	RESTORE_INT(flags);
	return TRUE;
}


unsigned int *deque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned int ffptr, int ffsize)
{
	// critical section!
	unsigned int  i;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);
	if (CIRC_CNT(*head, *tail, ffsize) == 0) {
		RESTORE_INT(flags);
		return NULL;
	}

	i = *tail;
	*tail = (*tail + 1) & (ffsize - 1);
	RESTORE_INT(flags);
	return (unsigned int *)(*(unsigned int *)(ffptr + i*(sizeof(void *))));
}


void initque(struct rtl8192cd_priv *priv, int *head, int *tail)
{
	// critical section!
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);
	*head = *tail = 0;
	RESTORE_INT(flags);
}


int	isFFempty(int head, int tail)
{
	return (head == tail);
}


#ifdef CONFIG_RTK_MESH
unsigned int find_rate_MP(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct ht_cap_elmt * peer_ht_cap,  int peer_ht_cap_len, char *peer_rate,int peer_rate_len,int mode, int isBasicRate)
{
	unsigned int len, i, hirate, lowrate, rate_limit, OFDM_only=0;
	unsigned char *rateset, *p;

	if ((get_rf_mimo_mode(priv)== MIMO_1T2R) || (get_rf_mimo_mode(priv)== MIMO_1T1R))
		rate_limit = 8;
	else
		rate_limit = 16;

	if (pstat) {
		rateset = pstat->bssrateset;
		len = pstat->bssratelen;
	}
	else {
		rateset = peer_rate;
		len = peer_rate_len;
	}

	hirate = _1M_RATE_;
	lowrate = _54M_RATE_;
	if (priv->pshare->curr_band == BAND_5G)
		OFDM_only = 1;

	for(i=0,p=rateset; i<len; i++,p++)
	{
		if (*p == 0x00)
			break;

		if ((isBasicRate & 1) && !(*p & 0x80))
			continue;

		if ((isBasicRate & 2) && !is_CCK_rate(*p & 0x7f))
			continue;

		if ((*p & 0x7f) > hirate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				hirate = (*p & 0x7f);

		if ((*p & 0x7f) < lowrate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				lowrate = (*p & 0x7f);
	}

	if (pstat) {
		if ((mode == 1) && (isBasicRate == 0) && pstat->ht_cap_len) {
			for (i=0; i<rate_limit; i++)
			{
				if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8)) {
					hirate = i;
					hirate |= 0x80;
				}
			}
		}
	}
	else {
		if ((mode == 1) && (isBasicRate == 0) && priv->ht_cap_len && peer_ht_cap_len) {
			for (i=0; i<rate_limit; i++)
			{
				if (peer_ht_cap->support_mcs[i/8] & BIT(i%8)) {
					hirate = i;
					hirate |= 0x80;
				}
			}
		}
	}

	if (mode == 0)
		return lowrate;
	else
		return hirate;
}

#endif


// rateset: is the rateset for searching
// mode: 0: find the lowest rate, 1: find the highest rate
// isBasicRate: bit0-1: find from basic rate set, bit0-0: find from supported rate set. bit1-1: find CCK only
unsigned int find_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, int mode, int isBasicRate)
{
	unsigned int len, i, hirate, lowrate, rate_limit, OFDM_only=0;
	unsigned char *rateset, *p;

	if ((get_rf_mimo_mode(priv)== MIMO_1T2R) || (get_rf_mimo_mode(priv)== MIMO_1T1R))
		rate_limit = 8;
	else
		rate_limit = 16;

	if (pstat) {
		rateset = pstat->bssrateset;
		len = pstat->bssratelen;
	}
	else {
		rateset = AP_BSSRATE;
		len = AP_BSSRATE_LEN;
	}

	hirate = _1M_RATE_;
	lowrate = _54M_RATE_;
	if (priv->pshare->curr_band == BAND_5G)
		OFDM_only = 1;

	for(i=0,p=rateset; i<len; i++,p++)
	{
		if (*p == 0x00)
			break;

		if ((isBasicRate & 1) && !(*p & 0x80))
			continue;

		if ((isBasicRate & 2) && !is_CCK_rate(*p & 0x7f))
			continue;

		if ((*p & 0x7f) > hirate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				hirate = (*p & 0x7f);

		if ((*p & 0x7f) < lowrate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				lowrate = (*p & 0x7f);
	}

	if (pstat) {
		if ((mode == 1) && (isBasicRate == 0) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
			for (i=0; i<rate_limit; i++)
			{
				if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i&0x7)) {
					hirate = i;
					hirate |= 0x80;
				}
			}
		}
	}
	else {
		if ((mode == 1) && (isBasicRate == 0) && priv->ht_cap_len) {
			for (i=0; i<rate_limit; i++)
			{
				if (priv->ht_cap_buf.support_mcs[i/8] & BIT(i%8)) {
					hirate = i;
					hirate |= 0x80;
				}
			}
		}
	}

	if (mode == 0)
		return lowrate;
	else
		return hirate;
}


UINT8 get_rate_from_bit_value(int bit_val)
{
	int i;

	if (bit_val == 0)
		return 0;

	i = 0;
	while ((bit_val & BIT(i)) == 0)
		i++;

	if (i < 12)
		return dot11_rate_table[i];
	else if (i < 28)
		return ((i - 12) | 0x80);
	else
		return 0;
}


int get_rate_index_from_ieee_value(UINT8 val)
{
	int i;
	for (i=0; dot11_rate_table[i]; i++) {
		if (val == dot11_rate_table[i]) {
			return i;
		}
	}
	_DEBUG_ERR("Local error, invalid input rate for get_rate_index_from_ieee_value() [%d]!!\n", val);
	return 0;
}


int get_bit_value_from_ieee_value(UINT8 val)
{
	int i=0;
	while(dot11_rate_table[i] != 0) {
		if (dot11_rate_table[i] == val)
			return BIT(i);
		i++;
	}
	return 0;
}


void init_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct wifi_mib	*pmib = priv->pmib;
	unsigned long	offset;
	int i, j;

#ifdef WDS
	static unsigned char bssrateset[32];
	unsigned int	bssratelen=0;
	unsigned int	current_tx_rate=0;
#endif
	unsigned short	bk_aid;
	unsigned char		bk_hwaddr[MACADDRLEN];

	// init linked list header
	// BUT do NOT init hash_list
	INIT_LIST_HEAD(&pstat->asoc_list);
	INIT_LIST_HEAD(&pstat->auth_list);
	INIT_LIST_HEAD(&pstat->sleep_list);
	INIT_LIST_HEAD(&pstat->defrag_list);
	INIT_LIST_HEAD(&pstat->wakeup_list);
	INIT_LIST_HEAD(&pstat->frag_list);

	// to avoid add RAtid fail
	INIT_LIST_HEAD(&pstat->addRAtid_list);
	INIT_LIST_HEAD(&pstat->addrssi_list);

#ifdef CONFIG_RTK_MESH
	INIT_LIST_HEAD(&pstat->mesh_mp_ptr);
#endif	// CONFIG_RTK_MESH

#ifdef A4_STA
	INIT_LIST_HEAD(&pstat->a4_sta_list);
#endif

	skb_queue_head_init(&pstat->dz_queue);

	// we do NOT reset MAC here

#ifdef WDS
	if (pstat->state & WIFI_WDS) {
		bssratelen = pstat->bssratelen;
		memcpy(bssrateset, pstat->bssrateset, bssratelen);
		current_tx_rate = pstat->current_tx_rate;
	}
#endif

	// zero out all the rest
	bk_aid = pstat->aid;
	memcpy(bk_hwaddr, pstat->hwaddr, MACADDRLEN);

	offset = (unsigned long)(&((struct stat_info *)0)->auth_seq);
	memset((void *)((unsigned long)pstat + offset), 0, sizeof(struct stat_info)-offset);

	pstat->aid = bk_aid;
	memcpy(pstat->hwaddr, bk_hwaddr, MACADDRLEN);

#ifdef WDS
	if (bssratelen) {
		pstat->bssratelen = bssratelen;
		memcpy(pstat->bssrateset, bssrateset, bssratelen);
		pstat->current_tx_rate = current_tx_rate;
		pstat->state |= WIFI_WDS;
	}
#endif

	// some variables need initial value
	pstat->ieee8021x_ctrlport = pmib->dot118021xAuthEntry.dot118021xDefaultPort;
	pstat->expire_to = priv->expire_to;
	for (i=0; i<8; i++)
		for (j=0; j<TUPLE_WINDOW; j++)
			pstat->tpcache[i][j] = 0xffff;
			// Stanldy mesh: pstat->tpcache[i][j] = j+1 is best solution, because its a hash table, fill slot[i] with i+1 can prevent collision,fix the packet loss of first unicast
	pstat->tpcache_mgt = 0xffff;

#ifdef GBWC
	for (i=0; i<priv->pmib->gbwcEntry.GBWCNum; i++) {
		if (!memcmp(pstat->hwaddr, priv->pmib->gbwcEntry.GBWCAddr[i], MACADDRLEN)) {
			pstat->GBWC_in_group = TRUE;
			break;
		}
	}
#endif

// button 2009.05.21
#ifdef INCLUDE_WPA_PSK
	pstat->wpa_sta_info->clientHndshkProcessing = pstat->wpa_sta_info->clientHndshkDone = FALSE;
#endif

#ifdef CONFIG_RTK_MESH
	pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_EXPIRE_TO;
#endif	//CONFIG_RTK_MESH

	for (i=0; i<8; i++)
		memset(&pstat->rc_entry[i], 0, sizeof(struct reorder_ctrl_entry));

#ifdef SUPPORT_TX_AMSDU
	for (i=0; i<8; i++)
		skb_queue_head_init(&pstat->amsdu_tx_que[i]);
#endif

#ifdef HW_ANT_SWITCH
	pstat->CurAntenna = priv->pshare->rf_ft_var.CurAntenna;
#endif

}


void free_sta_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#if defined(WIFI_WMM) && defined(WMM_APSD)
	int				hd, tl;
#endif
	int				i, j;
	struct sk_buff	*pskb;

	// free all skb in dz_queue
	while (skb_queue_len(&pstat->dz_queue)) {
		pskb = skb_dequeue(&pstat->dz_queue);
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
	}

#if defined(WIFI_WMM) && defined(WMM_APSD)
	hd = pstat->VO_dz_queue->head;
	tl = pstat->VO_dz_queue->tail;
	while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
		pskb = pstat->VO_dz_queue->pSkb[tl];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		tl++;
		tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
	}
	pstat->VO_dz_queue->head = 0;
	pstat->VO_dz_queue->tail = 0;

	hd = pstat->VI_dz_queue->head;
	tl = pstat->VI_dz_queue->tail;
	while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
		pskb = pstat->VI_dz_queue->pSkb[tl];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		tl++;
		tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
	}
	pstat->VI_dz_queue->head = 0;
	pstat->VI_dz_queue->tail = 0;

	hd = pstat->BE_dz_queue->head;
	tl = pstat->BE_dz_queue->tail;
	while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
		pskb = pstat->BE_dz_queue->pSkb[tl];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		tl++;
		tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
	}
	pstat->BE_dz_queue->head = 0;
	pstat->BE_dz_queue->tail = 0;

	hd = pstat->BK_dz_queue->head;
	tl = pstat->BK_dz_queue->tail;
	while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
		pskb = pstat->BK_dz_queue->pSkb[tl];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		tl++;
		tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
	}
	pstat->BK_dz_queue->head = 0;
	pstat->BK_dz_queue->tail = 0;
#endif
#if defined(WIFI_WMM)
	hd = pstat->MGT_dz_queue->head;
	tl = pstat->MGT_dz_queue->tail;
	while (CIRC_CNT(hd, tl, NUM_DZ_MGT_QUEUE)) {
		struct tx_insn *ptx_insn = pstat->MGT_dz_queue->ptx_insn[tl];
		release_mgtbuf_to_poll(priv, ptx_insn->pframe);
		release_wlanhdr_to_poll(priv, ptx_insn->phdr);
		kfree(ptx_insn);
		tl++;
		tl = tl & (NUM_DZ_MGT_QUEUE - 1);
	}
	pstat->MGT_dz_queue->head = 0;
	pstat->MGT_dz_queue->tail = 0;

#endif

	// free all skb in frag_list
	while (!list_empty(&(pstat->frag_list)))
	{
		pskb = get_skb_frlist((pstat->frag_list.next),
			(unsigned long)(&((struct rx_frinfo *)0)->mpdu_list));
		list_del((pstat->frag_list.next));
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
	}

	// free all skb in rc queue
	for (i=0; i<8; i++) {
		pstat->rc_entry[i].start_rcv = FALSE;
		for (j=0; j<128; j++) {
			if (pstat->rc_entry[i].packet_q[j]) {
				pskb = pstat->rc_entry[i].packet_q[j];
				rtl_kfree_skb(priv, pskb, _SKB_RX_);
				pstat->rc_entry[i].packet_q[j] = NULL;
			}
		}
	}
}


void release_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int				i;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	unsigned long		flags;
#endif
	if (priv->pshare->is_40m_bw && pstat->is_marvell_sta == 1){
#ifdef STA_EXT
		if (pstat->aid > FW_NUM_STAT)
			priv->pshare->marvellMapBitExt &= ~BIT(pstat->aid - FW_NUM_STAT - 1);
		else
#endif
			priv->pshare->marvellMapBit &= ~BIT(pstat->aid - 1);

#ifdef STA_EXT
		if ( (priv->pshare->marvellMapBit == 0 && priv->pshare->marvellMapBitExt == 0) && (priv->pshare->Reg_RRSR_2 != 0) && (priv->pshare->Reg_81b != 0))
#else
			if ( (priv->pshare->marvellMapBit == 0) && (priv->pshare->Reg_RRSR_2 != 0) && (priv->pshare->Reg_81b != 0))
#endif
			{
				RTL_W8(RRSR+2, priv->pshare->Reg_RRSR_2);
				RTL_W8(0x81b, priv->pshare->Reg_81b);
				priv->pshare->Reg_RRSR_2 = 0;
				priv->pshare->Reg_81b = 0;
			}
	}

	// flush the stainfo cache
	//if (!memcmp(pstat->hwaddr, priv->stainfo_cache.hwaddr, MACADDRLEN))
	//	memset(&(priv->stainfo_cache), 0, sizeof(priv->stainfo_cache));
	if (pstat == priv->pstat_cache)
		priv->pstat_cache = NULL;

	// free all queued skb
	free_sta_skb(priv, pstat);

	// delete all list
	// BUT do NOT delete hash list
	if (!list_empty(&(pstat->asoc_list)))
		list_del_init(&(pstat->asoc_list));

	if (!list_empty(&(pstat->auth_list)))
		list_del_init(&(pstat->auth_list));

	if (!list_empty(&(pstat->sleep_list)))
		list_del_init(&(pstat->sleep_list));

	if (!list_empty(&(pstat->defrag_list)))
		list_del_init(&(pstat->defrag_list));

	if (!list_empty(&(pstat->wakeup_list)))
		list_del_init(&(pstat->wakeup_list));

	// to avoid add RAtid fail
	if (!list_empty(&(pstat->addRAtid_list)))
		list_del_init(&(pstat->addRAtid_list));

	if (!list_empty(&(pstat->addrssi_list)))
		list_del_init(&(pstat->addrssi_list));

#ifdef CONFIG_RTK_MESH
	if (!list_empty(&(pstat->mesh_mp_ptr)))
		list_del_init(&(pstat->mesh_mp_ptr));

	pstat->mesh_neighbor_TBL.State = MP_UNUSED;	// reset state (clean is high priority)

	PathSelection_del_tbl_entry(priv, pstat->hwaddr);	// add by Galileo
//yschen 2009-03-04
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID) // 1.Proxy_table in root interface NOW!! 2.Spare for Mesh work with Multiple AP (Please see Mantis 0000107 for detail)
    if(IS_ROOT_INTERFACE(priv))
#endif
	{
		HASH_DELETE(priv->proxy_table, pstat->hwaddr);
	}
#endif

#ifdef A4_STA
	if (!list_empty(&pstat->a4_sta_list))
		list_del_init(&pstat->a4_sta_list);	
#endif

	// remove key in CAM
	if (pstat->dot11KeyMapping.keyInCam == TRUE) {
		if (priv->drv_state & DRV_STATE_OPEN) {
			if (CamDeleteOneEntry(priv, pstat->hwaddr, 0, 0)) {
				pstat->dot11KeyMapping.keyInCam = FALSE;
				priv->pshare->CamEntryOccupied--;
			}
		}
	}

	for (i=0; i<RC_TIMER_NUM; i++)
		if (priv->pshare->rc_timer[i].pstat == pstat)
			priv->pshare->rc_timer[i].pstat = NULL;

#ifdef WDS
	pstat->state &= WIFI_WDS;
#else
	pstat->state = 0;
#endif

#ifdef TX_SHORTCUT
	for (i=0; i<TX_SC_ENTRY_NUM; i++)
		memset(&pstat->tx_sc_ent[i], 0, sizeof(struct tx_sc_entry));
#endif

#ifdef RX_SHORTCUT
	pstat->rx_payload_offset = 0;
#endif

#ifdef INCLUDE_WPA_PSK
	if (timer_pending(&pstat->wpa_sta_info->resendTimer))
		del_timer_sync(&pstat->wpa_sta_info->resendTimer);
#endif

#ifdef STA_EXT
	release_remapAid(priv, pstat);
#endif

#ifdef SUPPORT_TX_AMSDU
	for (i=0; i<8; i++)
		free_skb_queue(priv, &pstat->amsdu_tx_que[i]);
#endif

#if 1
#if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
	{
		extern unsigned char cached_br_sta_mac[MACADDRLEN];
		extern struct net_device *cached_br_sta_dev;
		memset(cached_br_sta_mac, 0, MACADDRLEN);
		cached_br_sta_dev = NULL;
	}
#endif
#else
#ifdef BR_SHORTCUT
	clear_shortcut_cache();
#endif
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	SAVE_INT_AND_CLI(flags);
	if (pstat->wapiInfo)
	{
		del_timer(&pstat->wapiInfo->waiResendTimer);
		del_timer(&pstat->wapiInfo->waiUCastKeyUpdateTimer);
		wapiReleaseFragementQueue(pstat->wapiInfo);
		if (pstat->wapiInfo->waiCertCachedData!=NULL)
		{
			kfree(pstat->wapiInfo->waiCertCachedData);
			pstat->wapiInfo->waiCertCachedData = NULL;
		}
		kfree(pstat->wapiInfo);
		pstat->wapiInfo = NULL;
	}
	RESTORE_INT(flags);
#endif
}


struct	stat_info *alloc_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr, int id)
{
	unsigned long	flags;
	unsigned int	i,index;
	struct list_head	*phead, *plist;
	struct stat_info	*pstat;

	SAVE_INT_AND_CLI(flags);

	if (id < 0) { // not from FAST_RECOVERY
	// any free sta info?
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == FALSE))
			{
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
				priv->pshare->aidarray[i]->priv = priv;
#endif
				priv->pshare->aidarray[i]->used = TRUE;
				pstat = &(priv->pshare->aidarray[i]->station);
				memcpy(pstat->hwaddr, hwaddr, MACADDRLEN);
				init_stainfo(priv, pstat);

				// insert to hash list
				index = wifi_mac_hash(hwaddr);
				plist = priv->stat_hash;
				plist += index;
				list_add_tail(&(pstat->hash_list), plist);

				RESTORE_INT(flags);
				return pstat;
			}
		}

		// allocate new sta info
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] == NULL)
				break;
		}
	}
	else
		i = id;

	if (i < NUM_STAT) {
#ifdef RTL8192CD_VARIABLE_USED_DMEM
			priv->pshare->aidarray[i] = (struct aid_obj *)rtl8192cd_dmem_alloc(AID_OBJ, &i);
#else
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i] = alloc_sta_obj(priv);
#else
			priv->pshare->aidarray[i] = (struct aid_obj *)kmalloc(sizeof(struct aid_obj), GFP_ATOMIC);
#endif
#endif
			if (priv->pshare->aidarray[i] == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i], 0, sizeof(struct aid_obj));

#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.VO_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.VO_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.VO_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.VO_dz_queue, 0, sizeof(struct apsd_pkt_queue));

#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.VI_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.VI_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.VI_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.VI_dz_queue, 0, sizeof(struct apsd_pkt_queue));

#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.BE_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.BE_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.BE_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.BE_dz_queue, 0, sizeof(struct apsd_pkt_queue));

#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.BK_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.BK_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.BK_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.BK_dz_queue, 0, sizeof(struct apsd_pkt_queue));
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.MGT_dz_queue = alloc_sta_mgt_que(priv);
#else
			priv->pshare->aidarray[i]->station.MGT_dz_queue = (struct dz_mgmt_queue *)kmalloc(sizeof(struct dz_mgmt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.MGT_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.MGT_dz_queue, 0, sizeof(struct dz_mgmt_queue));
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.wpa_sta_info = alloc_wpa_buf(priv);
#else
			priv->pshare->aidarray[i]->station.wpa_sta_info = (WPA_STA_INFO *)kmalloc(sizeof(WPA_STA_INFO), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.wpa_sta_info == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.wpa_sta_info, 0, sizeof(WPA_STA_INFO));
#endif

#ifdef WIFI_HAPD
			memset(priv->pshare->aidarray[i]->station.wpa_ie, 0, 256);
#ifndef HAPD_DRV_PSK_WPS
			memset(priv->pshare->aidarray[i]->station.wps_ie, 0, 256);
#endif
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			priv->pshare->aidarray[i]->priv = priv;
#endif
			INIT_LIST_HEAD(&(priv->pshare->aidarray[i]->station.hash_list));
			priv->pshare->aidarray[i]->station.aid = i + 1; //aid 0 is reserved for AP
			priv->pshare->aidarray[i]->used = TRUE;
			pstat = &(priv->pshare->aidarray[i]->station);
			memcpy(pstat->hwaddr, hwaddr, MACADDRLEN);
			init_stainfo(priv, pstat);

			// insert to hash list
			index = wifi_mac_hash(hwaddr);
			plist = priv->stat_hash;
			plist += index;
			list_add_tail(&(pstat->hash_list), plist);

			RESTORE_INT(flags);
			return pstat;
	}

	// no more free sta info, check idle sta
	phead = &priv->asoc_list;
	plist = phead->next;

	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if ((pstat->expire_to == 0)
#ifdef WDS
#ifdef LAZY_WDS
			&& ((pstat->state & WIFI_WDS_LAZY) ||
				(!(pstat->state & WIFI_WDS_LAZY) && !(pstat->state & WIFI_WDS)))
#else
			&& !(pstat->state & WIFI_WDS)
#endif
#endif
		)
		{
			i = pstat->aid - 1;
			release_stainfo(priv, pstat);
			list_del_init(&(pstat->hash_list));

			priv->pshare->aidarray[i]->used = TRUE;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			priv->pshare->aidarray[i]->priv = priv;
#endif
			memcpy(pstat->hwaddr, hwaddr, MACADDRLEN);
			init_stainfo(priv, pstat);

			// insert to hash list
			index = wifi_mac_hash(hwaddr);
			plist = priv->stat_hash;
			plist += index;
			list_add_tail(&(pstat->hash_list), plist);

			RESTORE_INT(flags);
			return pstat;
		}
		else
			plist = plist->next;
	}

	RESTORE_INT(flags);
	DEBUG_ERR("AID buf is not enough\n");
	return	(struct stat_info *)NULL;

no_free_memory:

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	if (priv->pshare->aidarray[i]->station.wpa_sta_info)
#ifdef PRIV_STA_BUF
		free_wpa_buf(priv, priv->pshare->aidarray[i]->station.wpa_sta_info);
#else
		kfree(priv->pshare->aidarray[i]->station.wpa_sta_info);
#endif
#endif
#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
	if (priv->pshare->aidarray[i]->station.VO_dz_queue)
		free_sta_que(priv, priv->pshare->aidarray[i]->station.VO_dz_queue);
	if (priv->pshare->aidarray[i]->station.VI_dz_queue)
		free_sta_que(priv, priv->pshare->aidarray[i]->station.VI_dz_queue);
	if (priv->pshare->aidarray[i]->station.BE_dz_queue)
		free_sta_que(priv, priv->pshare->aidarray[i]->station.BE_dz_queue);
	if (priv->pshare->aidarray[i]->station.BK_dz_queue)
		free_sta_que(priv, priv->pshare->aidarray[i]->station.BK_dz_queue);
#else
	if (priv->pshare->aidarray[i]->station.VO_dz_queue)
		kfree(priv->pshare->aidarray[i]->station.VO_dz_queue);
	if (priv->pshare->aidarray[i]->station.VI_dz_queue)
		kfree(priv->pshare->aidarray[i]->station.VI_dz_queue);
	if (priv->pshare->aidarray[i]->station.BE_dz_queue)
		kfree(priv->pshare->aidarray[i]->station.BE_dz_queue);
	if (priv->pshare->aidarray[i]->station.BK_dz_queue)
		kfree(priv->pshare->aidarray[i]->station.BK_dz_queue);
#endif
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
	if (priv->pshare->aidarray[i]->station.MGT_dz_queue)
		free_sta_mgt_que(priv, priv->pshare->aidarray[i]->station.MGT_dz_queue);
#else
	if (priv->pshare->aidarray[i]->station.MGT_dz_queue)
		kfree(priv->pshare->aidarray[i]->station.MGT_dz_queue);

#endif
#endif


	if (priv->pshare->aidarray[i]) {
#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(AID_OBJ, &i);
#else
#ifdef PRIV_STA_BUF
		free_sta_obj(priv, priv->pshare->aidarray[i]);
#else
		kfree(priv->pshare->aidarray[i]);
#endif
#endif
	}
	priv->pshare->aidarray[i] = NULL;
	RESTORE_INT(flags);
	DEBUG_ERR("No free memory to allocate station info\n");
	return NULL;
}


int	free_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long	flags;
	unsigned int	i;

	if (pstat == (struct stat_info *)NULL)
	{
		DEBUG_ERR("illegal free an NULL stat obj\n");
		return FAIL;
	}

	for(i=0; i<NUM_STAT; i++)
	{
		if (priv->pshare->aidarray[i] &&
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			(priv->pshare->aidarray[i]->priv == priv) &&
#endif
			(priv->pshare->aidarray[i]->used == TRUE) &&
			(&(priv->pshare->aidarray[i]->station) == pstat))
		{
			DEBUG_INFO("free station info of %02X%02X%02X%02X%02X%02X\n",
				pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
				pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);

			SAVE_INT_AND_CLI(flags);
#ifdef WDS
#ifdef LAZY_WDS
			if (!(pstat->state & WIFI_WDS) || (pstat->state & WIFI_WDS_LAZY))
#else
			if (!(pstat->state & WIFI_WDS))
#endif
#endif
			{
				priv->pshare->aidarray[i]->used = FALSE;
				// remove from hash_list
				if (!list_empty(&(pstat->hash_list)))
					list_del_init(&(pstat->hash_list));
			}

			release_stainfo(priv, pstat);
			RESTORE_INT(flags);
			return SUCCESS;
		}
	}

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	wapiAssert(pstat->wapiInfo==NULL);
#endif
	DEBUG_ERR("pstat can not be freed \n");
	return	FAIL;
}


/* any station allocated can be searched by hash list */
__MIPS16
__IRAM_IN_865X
struct stat_info *get_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct list_head	*plist;
	struct stat_info	*pstat;
	unsigned int	index;

	//if (!memcmp(hwaddr, priv->stainfo_cache.hwaddr, MACADDRLEN) &&  priv->stainfo_cache.pstat)
	pstat = priv->pstat_cache;
	if (pstat && !memcmp(hwaddr, pstat->hwaddr, MACADDRLEN))
		return pstat;

	index = wifi_mac_hash(hwaddr);
	plist = &priv->stat_hash[index];

	while (plist->next != &(priv->stat_hash[index]))
	{
		plist = plist->next;
		pstat = list_entry(plist, struct stat_info ,hash_list);

		if (pstat == NULL) {
			printk("%s: pstat=NULL!\n", __FUNCTION__);
			break;
		}

		if (!(memcmp((void *)pstat->hwaddr, (void *)hwaddr, MACADDRLEN))) { // if found the matched address
			//memcpy(priv->stainfo_cache.hwaddr, hwaddr, MACADDRLEN);
			priv->pstat_cache = pstat;
			return pstat;
		}
		if (plist == plist->next)
			break;
	}
	return (struct stat_info *)NULL;
}


/* aid is only meaningful for assocated stations... */
struct stat_info *get_aidinfo(struct rtl8192cd_priv *priv, unsigned int aid)
{
	struct list_head	*plist, *phead;
	struct stat_info	*pstat;

	if (aid == 0)
		return (struct stat_info *)NULL;

	phead = &priv->asoc_list;
	plist = phead->next;

	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;
		if (pstat->aid == aid)
			return pstat;
	}
	return (struct stat_info *)NULL;
}

#ifdef TXREPORT
struct stat_info *get_macidinfo(struct rtl8192cd_priv *priv, unsigned int aid)
{
	struct list_head	*plist, *phead;
	struct stat_info	*pstat;

	if (aid == 0)
		return (struct stat_info *)NULL;

	phead = &priv->asoc_list;
	plist = phead->next;

	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;
		if (REMAP_AID(pstat) == aid)
			return pstat;
	}
	return (struct stat_info *)NULL;
}
#endif

int IS_BSSID(struct rtl8192cd_priv *priv, unsigned char *da)
{
	unsigned char *bssid;
	bssid = priv->pmib->dot11StationConfigEntry.dot11Bssid;

	if (!memcmp(da, bssid, 6))
		return TRUE;
	else
		return FALSE;
}


int IS_MCAST(unsigned char *da)
{
	if ((*da) & 0x01)
		return TRUE;
	else
		return FALSE;
}


 UINT8 oui_rfc1042[] = {0x00, 0x00, 0x00};
 UINT8 oui_8021h[] = {0x00, 0x00, 0xf8};
 UINT8 oui_cisco[] = {0x00, 0x00, 0x0c};
int p80211_stt_findproto(UINT16 proto)
{
	/* Always return found for now.	This is the behavior used by the */
	/*  Zoom Win95 driver when 802.1h mode is selected */
	/* TODO: If necessary, add an actual search we'll probably
		 need this to match the CMAC's way of doing things.
		 Need to do some testing to confirm.
	*/

	if (proto == 0x80f3 ||   /* APPLETALK */
		proto == 0x8137 ) /* DIX II IPX */
		return 1;

	return 0;
}


void eth_2_llc(struct wlan_ethhdr_t *pethhdr, struct llc_snap *pllc_snap)
{
	pllc_snap->llc_hdr.dsap=pllc_snap->llc_hdr.ssap=0xAA;
	pllc_snap->llc_hdr.ctl=0x03;

	if (p80211_stt_findproto(ntohs(pethhdr->type))) {
		memcpy((void *)pllc_snap->snap_hdr.oui, oui_8021h, WLAN_IEEE_OUI_LEN);
	}
	else {
		memcpy((void *)pllc_snap->snap_hdr.oui, oui_rfc1042, WLAN_IEEE_OUI_LEN);
	}
	pllc_snap->snap_hdr.type = pethhdr->type;
}


void eth2_2_wlanhdr(struct rtl8192cd_priv *priv, struct wlan_ethhdr_t *pethhdr, struct tx_insn *txcfg)
{
	unsigned char *pframe = txcfg->phdr;
	unsigned int to_fr_ds = get_tofr_ds(pframe);

	switch (to_fr_ds)
	{
		case 0x00:
			memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			memcpy(GetAddr2Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
			memcpy(GetAddr3Ptr(pframe), BSSID, WLAN_ADDR_LEN);
			break;
		case 0x01:
			{
				memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
				memcpy(GetAddr2Ptr(pframe), BSSID, WLAN_ADDR_LEN);
				memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
			}
			break;
		case 0x02:
			{
				memcpy(GetAddr1Ptr(pframe), BSSID, WLAN_ADDR_LEN);
				memcpy(GetAddr2Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
				memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			}
			break;
		case 0x03:
#ifdef WDS
#ifdef MP_TEST
			if (OPMODE & WIFI_MP_STATE)
				memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			else
#endif
#ifdef CONFIG_RTK_MESH
			if(txcfg->is_11s)
				memcpy(GetAddr1Ptr(pframe), txcfg->nhop_11s, WLAN_ADDR_LEN);
			else
#endif
				memcpy(GetAddr1Ptr(pframe), priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr, WLAN_ADDR_LEN);

#ifdef MP_TEST
			if (OPMODE & WIFI_MP_STATE)
				memcpy(GetAddr2Ptr(pframe), priv->dev->dev_addr, WLAN_ADDR_LEN);
			else
#endif

#ifdef __DRAYTEK_OS__
				memcpy(GetAddr2Ptr(pframe), priv->dev->dev_addr, WLAN_ADDR_LEN);
#else
#ifdef CONFIG_RTK_MESH
			if(txcfg->is_11s)
				memcpy(GetAddr2Ptr(pframe), GET_MY_HWADDR, WLAN_ADDR_LEN);
			else
#endif // CONFIG_RTK_MESH
				memcpy(GetAddr2Ptr(pframe), priv->wds_dev[txcfg->wdsIdx]->dev_addr , WLAN_ADDR_LEN);
#endif
			memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			memcpy(GetAddr4Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);

#ifdef CONFIG_RTK_MESH
			if(txcfg->is_11s && is_qos_data(pframe))
				memset( pframe+WLAN_HDR_A4_LEN, 0, 2); // qos
#endif	// CONFIG_RTK_MESH

#else // not WDS
#ifdef CONFIG_RTK_MESH
			if(txcfg->is_11s) {
				memcpy(GetAddr1Ptr(pframe), txcfg->nhop_11s, WLAN_ADDR_LEN);
				memcpy(GetAddr2Ptr(pframe), GET_MY_HWADDR, WLAN_ADDR_LEN);
				memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
				memcpy(GetAddr4Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);

				//if((*pframe) & 0x80) //if qos is enable, the bit 7 of frame control will set to 1
				if(is_qos_data(pframe))
					memset(pframe+WLAN_HDR_A4_LEN, 0, 2); // qos
			} else
#endif // CONFIG_RTK_MESH

#ifdef A4_STA
			if (priv->pshare->rf_ft_var.a4_enable && txcfg->pstat && 
											(txcfg->pstat->state & WIFI_A4_STA)) {
				memcpy(GetAddr1Ptr(pframe), txcfg->pstat->hwaddr, WLAN_ADDR_LEN);
				memcpy(GetAddr2Ptr(pframe), GET_MY_HWADDR, WLAN_ADDR_LEN);
				memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
				memcpy(GetAddr4Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);				
			}
			else
#endif
			{
			DEBUG_ERR("no support for WDS!\n");
			memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			memcpy(GetAddr2Ptr(pframe), (const void *)BSSID, WLAN_ADDR_LEN);
			memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
			} // else of if(txcfg->is_11s)
#endif	// WDS
			break;
	}
}


int skb_p80211_to_ether(struct net_device *dev, int wep_mode, struct rx_frinfo *pfrinfo)
{
	UINT	to_fr_ds;
	INT		payload_length;
	INT		payload_offset, trim_pad;
	UINT8	daddr[WLAN_ETHADDR_LEN];
	UINT8	saddr[WLAN_ETHADDR_LEN];
	UINT8	*pframe;
#ifdef CONFIG_RTK_MESH
	INT 	mesh_header_len=0;
#endif
	struct wlan_hdr *w_hdr;
	struct wlan_ethhdr_t   *e_hdr;
	struct wlan_llc_t      *e_llc;
	struct wlan_snap_t     *e_snap;
	struct rtl8192cd_priv	   *priv = (struct rtl8192cd_priv *)dev->priv;
	int wlan_pkt_format;
	struct sk_buff *skb = get_pskb(pfrinfo);

#if defined(RX_SHORTCUT)
#ifndef MESH_AMSDU
	struct stat_info 	*pstat = get_stainfo(priv, GetAddr2Ptr(skb->data));
#else
	struct stat_info 	*pstat;
	if( pfrinfo->is_11s &8 )
		pstat = NULL;
	else
		pstat = get_stainfo(priv, GetAddr2Ptr(skb->data));
#endif // MESH_AMSDU
#endif

#ifdef RX_SHORTCUT
	int 				privacy;

	if (pstat)
		pstat->rx_payload_offset = 0;
#endif // RX_SHORTCUT

	pframe = get_pframe(pfrinfo);
	to_fr_ds = get_tofr_ds(pframe);
#ifndef MESH_AMSDU
	payload_offset = get_hdrlen(priv, pframe);
#else
	// Get_hdrlen needs to access pframe+32
	// When is_11s=8, the length of a frame might be less than 32 bytes, so we need to protect it
	if( pfrinfo->is_11s &8 )
		payload_offset = 0;
	else
		payload_offset = get_hdrlen(priv, pframe);
#endif // MESH_AMSDU
	trim_pad = 0; // _CRCLNG_ has beed subtracted in isr
	w_hdr = (struct wlan_hdr *)pframe;

#ifdef MESH_AMSDU
	//nctu  Gakki
	if( pfrinfo->is_11s &8 )
	{
		struct wlan_ethhdr_t eth;
		struct  MESH_HDR* mhdr = (struct MESH_HDR*) (pframe+sizeof(struct wlan_ethhdr_t));
		const short mlen =  (mhdr->mesh_flag &1) ? 16 : 4;
		memcpy( &eth, pframe, sizeof(struct wlan_ethhdr_t));
		if( mlen &16 )
			memcpy(&eth, mhdr->DestMACAddr, WLAN_ETHADDR_LEN<<1 );
		memcpy(skb_pull(skb, mlen), &eth, sizeof(struct wlan_ethhdr_t));
		return SUCCESS;
	}
#endif

	if ( to_fr_ds == 0x00) {
		memcpy(daddr, (const void *)w_hdr->addr1, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr2, WLAN_ETHADDR_LEN);
	}
	else if( to_fr_ds == 0x01) {
		{
			memcpy(daddr, (const void *)w_hdr->addr1, WLAN_ETHADDR_LEN);
			memcpy(saddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
		}
	}
	else if( to_fr_ds == 0x02) {
		{
			memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
			memcpy(saddr, (const void *)w_hdr->addr2, WLAN_ETHADDR_LEN);
		}
	}
	else {
#ifdef CONFIG_RTK_MESH
		// Gallardo 2008.10.17
		// mantis bug 0000109
		// WIFI_11S_MESH = WIFI_QOS_DATA
		if(pfrinfo->is_11s &1)
		{

	 		if( GetFrameSubType(pframe) == WIFI_11S_MESH)
			{
				if ( pfrinfo->mesh_header.mesh_flag &1)
				{
					memcpy(daddr, (const void *)pfrinfo->mesh_header.DestMACAddr, WLAN_ETHADDR_LEN);
					memcpy(saddr, (const void *)pfrinfo->mesh_header.SrcMACAddr, WLAN_ETHADDR_LEN);
					mesh_header_len = 16;
				}
				else
				{
					memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
					memcpy(saddr, (const void *)w_hdr->addr4, WLAN_ETHADDR_LEN);
					mesh_header_len = 4;
				}
			}
		}
		else
#endif // CONFIG_RTK_MESH
		{
			memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
			memcpy(saddr, (const void *)w_hdr->addr4, WLAN_ETHADDR_LEN);
		}
	}

	if (GetPrivacy(pframe)) {
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if ((wep_mode == _WAPI_SMS4_)) {
			payload_offset += (WAPI_EXT_LEN-WAPI_ALIGNMENT_OFFSET);
			trim_pad += SMS4_MIC_LEN;
		} else
#endif
		if (((wep_mode == _WEP_40_PRIVACY_) || (wep_mode == _WEP_104_PRIVACY_))) {
			payload_offset += 4;
			trim_pad += 4;
		}
		else if ((wep_mode == _TKIP_PRIVACY_)) {
			payload_offset += 8;
			trim_pad += (8 + 4);
		}
		else if ((wep_mode == _CCMP_PRIVACY_)) {
			payload_offset += 8;
			trim_pad += 8;
		}
		else {
			DEBUG_ERR("drop pkt due to unallowed wep_mode privacy=%d\n", wep_mode);
			return FAIL;
		}
	}

	payload_length = skb->len - payload_offset - trim_pad;

#ifdef CONFIG_RTK_MESH
	  payload_length -=	mesh_header_len;
#endif

	if (payload_length <= 0) {
		DEBUG_ERR("drop pkt due to payload_length<=0\n");
		return FAIL;
	}

	e_hdr = (struct wlan_ethhdr_t *) (pframe + payload_offset);
	e_llc = (struct wlan_llc_t *) (pframe + payload_offset);
	e_snap = (struct wlan_snap_t *) (pframe + payload_offset + sizeof(struct wlan_llc_t));

	if ((e_llc->dsap==0xaa) && (e_llc->ssap==0xaa) && (e_llc->ctl==0x03))
	{
		if (!memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if (!memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
				 !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2))
			wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if (!memcmp(e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
		else if (!memcmp(e_snap->oui, oui_cisco, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_CDP;
		else {
			DEBUG_ERR("drop pkt due to invalid frame format!\n");
			return FAIL;
		}
	}
	else if ((memcmp(daddr, e_hdr->daddr, WLAN_ETHADDR_LEN) == 0) &&
			 (memcmp(saddr, e_hdr->saddr, WLAN_ETHADDR_LEN) == 0))
		wlan_pkt_format = WLAN_PKT_FORMAT_ENCAPSULATED;
	else
		wlan_pkt_format = WLAN_PKT_FORMAT_OTHERS;

	DEBUG_INFO("Convert 802.11 to 802.3 in format %d\n", wlan_pkt_format);

	if ((wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
		(wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) ||
		(wlan_pkt_format == WLAN_PKT_FORMAT_CDP)) {
		/* Test for an overlength frame */
		payload_length = payload_length - sizeof(struct wlan_llc_t) - sizeof(struct wlan_snap_t);

		if ((payload_length+WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("SNAP frame too large (%d>%d)\n",
				(payload_length+WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

#ifdef RX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_rxsc && pstat) {
#ifdef CONFIG_RTK_MESH
			if (pfrinfo->is_11s)
				privacy = get_sta_encrypt_algthm(priv, pstat);
			else
#endif // CONFIG_RTK_MESH
#ifdef WDS
			if (pfrinfo->to_fr_ds == 3)
				privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
			else
#endif
				privacy = get_sta_encrypt_algthm(priv, pstat);
			if ((GetFragNum(pframe)==0) &&
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
				   (!pfrinfo->is_11s || !IS_MCAST(GetAddr1Ptr(pframe)))&&
#endif
				((privacy == 0) ||
#ifdef CONFIG_RTL_WAPI_SUPPORT
				(privacy==_WAPI_SMS4_) ||
#endif
				(privacy && !UseSwCrypto(priv, pstat, IS_MCAST(GetAddr1Ptr(pframe))))))
			{
				memcpy((void *)&pstat->rx_wlanhdr, pframe, pfrinfo->hdr_len);
				pstat->rx_payload_offset = payload_offset;
				pstat->rx_trim_pad = trim_pad;
				pstat->rx_privacy = GetPrivacy(pframe);

#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
				if ( (pfrinfo->is_11s &3) && (pfrinfo->mesh_header.mesh_flag &1))
				{
					memcpy((void *)&pstat->rx_wlanhdr.wlanhdr.meshhdr.DestMACAddr, (const void *)pfrinfo->mesh_header.DestMACAddr, WLAN_ETHADDR_LEN);
					memcpy((void *)&pstat->rx_wlanhdr.wlanhdr.meshhdr.SrcMACAddr, (const void *)pfrinfo->mesh_header.SrcMACAddr, WLAN_ETHADDR_LEN);
				}
#endif
			}
		}
#endif // RX_SHORTCUT


		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		if ((wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
			(wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL))
		{
			/* chop llc header from skb. */
			skb_pull(skb, sizeof(struct wlan_llc_t));

			/* chop snap header from skb. */
			skb_pull(skb, sizeof(struct wlan_snap_t));
		}

#ifdef CONFIG_RTK_MESH
		/* chop mesh header from skb. */
		skb_pull(skb, mesh_header_len);
#endif

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		if (wlan_pkt_format == WLAN_PKT_FORMAT_CDP)
			e_hdr->type = payload_length;
		else
			e_hdr->type = e_snap->type;
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length + WLAN_ETHHDR_LEN);

#ifdef RX_SHORTCUT
		if (pstat && pstat->rx_payload_offset > 0) {
			if ((cpu_to_be16(e_hdr->type) != 0x888e) && // for WIFI_SIMPLE_CONFIG
#ifdef CONFIG_RTL_WAPI_SUPPORT
				 (cpu_to_be16(e_hdr->type)  != ETH_P_WAPI)&&
#endif
				(cpu_to_be16(e_hdr->type) != ETH_P_ARP)&&
				(wlan_pkt_format != WLAN_PKT_FORMAT_CDP))
				memcpy((void *)&pstat->rx_ethhdr, (const void *)e_hdr, sizeof(struct wlan_ethhdr_t));
			else {
				//printk("reset rx_payload_offset e_hdr->type %04x\n", cpu_to_be16(e_hdr->type));
				pstat->rx_payload_offset = 0;
			}
		}
#endif
	}
	else if ((wlan_pkt_format == WLAN_PKT_FORMAT_OTHERS) ||
			 (wlan_pkt_format == WLAN_PKT_FORMAT_APPLETALK) ||
			 (wlan_pkt_format == WLAN_PKT_FORMAT_IPX_TYPE4)) {

		/* Test for an overlength frame */
		if ( (payload_length + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("IPX/AppleTalk frame too large (%d>%d)\n",
				(payload_length + WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

#ifdef CONFIG_RTK_MESH
		/* chop mesh header from skb. */
		skb_pull(skb, mesh_header_len);
#endif

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);
		e_hdr->type = htons(payload_length);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length+WLAN_ETHHDR_LEN);
	}
	else if (wlan_pkt_format == WLAN_PKT_FORMAT_ENCAPSULATED) {

		if ( payload_length > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("Encapsulated frame too large (%d>%d)\n",
				payload_length, WLAN_MAX_ETHFRM_LEN);
		}

		/* Chop off the 802.11 header. */
		skb_pull(skb, payload_offset);

#ifdef CONFIG_RTK_MESH
		/* chop mesh header from skb. */
		skb_pull(skb, mesh_header_len);
#endif

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length);
	}

#ifdef __KERNEL__
#ifdef LINUX_2_6_22_
	skb->mac_header = (unsigned char *) skb->data; /* new MAC header */
#else
	skb->mac.raw = (unsigned char *) skb->data; /* new MAC header */
#endif
#endif

	return SUCCESS;
}


int strip_amsdu_llc(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
{
	INT		payload_length;
	INT		payload_offset;
	UINT8	daddr[WLAN_ETHADDR_LEN];
	UINT8	saddr[WLAN_ETHADDR_LEN];
	struct wlan_ethhdr_t	*e_hdr;
	struct wlan_llc_t		*e_llc;
	struct wlan_snap_t		*e_snap;
	int		pkt_format;

	memcpy(daddr, skb->data, MACADDRLEN);
	memcpy(saddr, skb->data+MACADDRLEN, MACADDRLEN);
	payload_length = skb->len - WLAN_ETHHDR_LEN;
	payload_offset = WLAN_ETHHDR_LEN;

	e_hdr = (struct wlan_ethhdr_t *) (skb->data + payload_offset);
	e_llc = (struct wlan_llc_t *) (skb->data + payload_offset);
	e_snap = (struct wlan_snap_t *) (skb->data + payload_offset + sizeof(struct wlan_llc_t));

	if ((e_llc->dsap==0xaa) && (e_llc->ssap==0xaa) && (e_llc->ctl==0x03))
	{
		if (!memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2))
				pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if (!memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
				 !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2))
			pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if (!memcmp(e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
		else if (!memcmp(e_snap->oui, oui_cisco, WLAN_IEEE_OUI_LEN))
			pkt_format = WLAN_PKT_FORMAT_CDP;
		else {
			DEBUG_ERR("drop pkt due to invalid frame format!\n");
			return FAIL;
		}
	}
	else if ((memcmp(daddr, e_hdr->daddr, WLAN_ETHADDR_LEN) == 0) &&
			 (memcmp(saddr, e_hdr->saddr, WLAN_ETHADDR_LEN) == 0))
		pkt_format = WLAN_PKT_FORMAT_ENCAPSULATED;
	else
		pkt_format = WLAN_PKT_FORMAT_OTHERS;

	DEBUG_INFO("Convert 802.11 to 802.3 in format %d\n", pkt_format);

	if ((pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
		(pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) ||
		(pkt_format == WLAN_PKT_FORMAT_CDP)) {
		/* Test for an overlength frame */
		payload_length = payload_length - sizeof(struct wlan_llc_t) - sizeof(struct wlan_snap_t);

		if ((payload_length+WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("SNAP frame too large (%d>%d)\n",
				(payload_length+WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		if ((pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
			(pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL))
		{
			/* chop llc header from skb. */
			skb_pull(skb, sizeof(struct wlan_llc_t));

			/* chop snap header from skb. */
			skb_pull(skb, sizeof(struct wlan_snap_t));
		}

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		if (pkt_format == WLAN_PKT_FORMAT_CDP)
			e_hdr->type = payload_length;
		else
			e_hdr->type = e_snap->type;
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length + WLAN_ETHHDR_LEN);
	}
	else if ((pkt_format == WLAN_PKT_FORMAT_OTHERS) ||
			 (pkt_format == WLAN_PKT_FORMAT_APPLETALK) ||
			 (pkt_format == WLAN_PKT_FORMAT_IPX_TYPE4)) {

		/* Test for an overlength frame */
		if ( (payload_length + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("IPX/AppleTalk frame too large (%d>%d)\n",
				(payload_length + WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);
		e_hdr->type = htons(payload_length);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length+WLAN_ETHHDR_LEN);
	}
	else if (pkt_format == WLAN_PKT_FORMAT_ENCAPSULATED) {

		if ( payload_length > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("Encapsulated frame too large (%d>%d)\n",
				payload_length, WLAN_MAX_ETHFRM_LEN);
		}

		/* Chop off the 802.11 header. */
		skb_pull(skb, payload_offset);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length);
	}

#ifdef __KERNEL__
#ifdef LINUX_2_6_22_
	skb->mac_header = (unsigned char *) skb->data; /* new MAC header */
#else
	skb->mac.raw = (unsigned char *) skb->data; /* new MAC header */
#endif
#endif

	return SUCCESS;
}


unsigned int get_sta_encrypt_algthm(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned int privacy = 0;

#ifdef CONFIG_RTK_MESH
	if( isMeshPoint(pstat) )
		return priv->pmib->dot11sKeysTable.dot11Privacy ;
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (pstat&&pstat->wapiInfo&&pstat->wapiInfo->wapiType!=wapiDisable)
	{
		return _WAPI_SMS4_;
	}
	else
#endif
	{
	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm) {
		if (pstat)
			privacy = pstat->dot11KeyMapping.dot11Privacy;
		else
			DEBUG_ERR("pstat == NULL\n");
	}
		else
		{
		// legacy system
		privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm; //could be wep40 or wep104
	}
	}

	return privacy;
}


unsigned int get_mcast_encrypt_algthm(struct rtl8192cd_priv *priv)
{
	unsigned int privacy;

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		return _WAPI_SMS4_;
	} else
#endif
	{
	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm) {
		// check station info
		privacy = priv->pmib->dot11GroupKeysTable.dot11Privacy;
	}
	else {	// legacy system
		privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;//must be wep40 or wep104
	}
	}
	return privacy;
}


unsigned int get_privacy(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				unsigned int *iv, unsigned int *icv, unsigned int *mic)
{
	unsigned int privacy;
	*iv = 0;
	*icv = 0;
	*mic = 0;

	privacy = get_sta_encrypt_algthm(priv, pstat);

	switch (privacy)
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
	case _WAPI_SMS4_:
		*iv = WAPI_PN_LEN+2;
		*icv = 0;
		*mic = SMS4_MIC_LEN;
		break;
#endif
	case _NO_PRIVACY_:
		*iv  = 0;
		*icv = 0;
		*mic = 0;
		break;
	case _WEP_40_PRIVACY_:
	case _WEP_104_PRIVACY_:
		*iv = 4;
		*icv = 4;
		*mic = 0;
		break;
	case _TKIP_PRIVACY_:
		*iv = 8;
		*icv = 4;
		*mic = 0;	// mic of TKIP is msdu based
		break;
	case _CCMP_PRIVACY_:
		*iv = 8;
		*icv = 0;
		*mic = 8;
		break;
	default:
		DEBUG_WARN("un-awared encrypted type %d\n", privacy);
		*iv = *icv = *mic = 0;
		break;
	}

	return privacy;
}


unsigned int get_mcast_privacy(struct rtl8192cd_priv *priv, unsigned int *iv, unsigned int *icv,
				unsigned int *mic)
{
	unsigned int privacy;
	*iv  = 0;
	*icv = 0;
	*mic = 0;

	privacy = get_mcast_encrypt_algthm(priv);

	switch (privacy)
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
	case _WAPI_SMS4_:
		*iv = WAPI_PN_LEN+2;
		*icv = 0;
		*mic = SMS4_MIC_LEN;
		break;
#endif
	case _NO_PRIVACY_:
		*iv = 0;
		*icv = 0;
		*mic = 0;
		break;
	case _WEP_40_PRIVACY_:
	case _WEP_104_PRIVACY_:
		*iv = 4;
		*icv = 4;
		*mic = 0;
		break;
	case _TKIP_PRIVACY_:
		*iv = 8;
		*icv = 4;
		*mic = 0; // mic of TKIP is msdu based
		break;
	case _CCMP_PRIVACY_:
		*iv = 8;
		*icv = 0;
		*mic = 8;
		break;
	default:
		DEBUG_WARN("un-awared encrypted type %d\n", privacy);
		*iv = 0;
		*icv = 0;
		*mic = 0;
		break;
	}

	return privacy;
}

unsigned char *get_da(unsigned char *pframe)
{
	unsigned char 	*da;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			da = GetAddr1Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			da = GetAddr1Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			da = GetAddr3Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			da = GetAddr3Ptr(pframe);
			break;
	}

	return da;
}


unsigned char *get_sa(unsigned char *pframe)
{
	unsigned char 	*sa;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			sa = GetAddr2Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			sa = GetAddr3Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			sa = GetAddr2Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			sa = GetAddr4Ptr(pframe);
			break;
	}

	return sa;
}


__MIPS16
__IRAM_IN_865X
unsigned char get_hdrlen(struct rtl8192cd_priv *priv, UINT8 *pframe)
{
	if (GetFrameType(pframe) == WIFI_DATA_TYPE)
	{
#ifdef CONFIG_RTK_MESH
		if ((get_tofr_ds(pframe) == 0x03) && ( (GetFrameSubType(pframe) == WIFI_11S_MESH) || (GetFrameSubType(pframe) == WIFI_11S_MESH_ACTION)))
		{
			if(GetFrameSubType(pframe) == WIFI_11S_MESH)  // DATA frame, qos might be on (TRUE on 8186)
			{
					return WLAN_HDR_A4_QOS_LEN;
			} // WIFI_11S_MESH
			else // WIFI_11S_MESH_ACTION frame, although qos flag is on, the qos field(2bytes) is not used for 8186
			{
				if(is_mesh_6addr_format_without_qos(pframe)) {
					return WLAN_HDR_A6_MESH_DATA_LEN;
				} else {
					return WLAN_HDR_A4_MESH_DATA_LEN;
				}
			}
		} // end of get_tofr_ds == 0x03 & (MESH DATA or MESH ACTION)
		else
#endif // CONFIG_RTK_MESH
		if (is_qos_data(pframe)) {
			if (get_tofr_ds(pframe) == 0x03)
				return WLAN_HDR_A4_QOS_LEN;
			else
				return WLAN_HDR_A3_QOS_LEN;
		}
		else {
			if (get_tofr_ds(pframe) == 0x03)
				return WLAN_HDR_A4_LEN;
			else
				return WLAN_HDR_A3_LEN;
		}
	}
	else if (GetFrameType(pframe) == WIFI_MGT_TYPE)
		return 	WLAN_HDR_A3_LEN;
	else if (GetFrameType(pframe) == WIFI_CTRL_TYPE)
	{
		if (GetFrameSubType(pframe) == WIFI_PSPOLL)
			return 16;
		else if (GetFrameSubType(pframe) == WIFI_BLOCKACK_REQ)
			return 16;
		else if (GetFrameSubType(pframe) == WIFI_BLOCKACK)
			return 16;
		else
		{
#ifdef _DEBUG_RTL8192CD_
			printk("unallowed control pkt type! 0x%04X\n", GetFrameSubType(pframe));
#endif
			return 0;
		}
	}
	else
	{
#ifdef _DEBUG_RTL8192CD_
		printk("unallowed pkt type! 0x%04X\n", GetFrameType(pframe));
#endif
		return 0;
	}
}


unsigned char *get_mgtbuf_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);
	
	ret = get_buf_from_poll(priv, &priv->pshare->wlanbuf_list, (unsigned int *)&priv->pshare->pwlanbuf_poll->count);

	RESTORE_INT(flags);
	return ret;
}


void release_mgtbuf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanbuf_list, (unsigned int *)&priv->pshare->pwlanbuf_poll->count);

	RESTORE_INT(flags);
}


unsigned char *get_wlanhdr_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *pbuf;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);
	pbuf = get_buf_from_poll(priv, &priv->pshare->wlan_hdrlist, (unsigned int *)&priv->pshare->pwlan_hdr_poll->count);

	RESTORE_INT(flags);
	return pbuf;
}


void release_wlanhdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlan_hdrlist, (unsigned int *)&priv->pshare->pwlan_hdr_poll->count);

	RESTORE_INT(flags);
}


//__MIPS16
__IRAM_IN_865X
unsigned char *get_wlanllchdr_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *pbuf;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	pbuf = get_buf_from_poll(priv, &priv->pshare->wlanllc_hdrlist, (unsigned int *)&priv->pshare->pwlanllc_hdr_poll->count);

	RESTORE_INT(flags);
	return pbuf;
}


void release_wlanllchdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanllc_hdrlist, (unsigned int *)&priv->pshare->pwlanllc_hdr_poll->count);

	RESTORE_INT(flags);
}


unsigned char *get_icv_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	ret = get_buf_from_poll(priv, &priv->pshare->wlanicv_list, (unsigned int *)&priv->pshare->pwlanicv_poll->count);

	RESTORE_INT(flags);
	return ret;
}


void release_icv_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanicv_list, (unsigned int *)&priv->pshare->pwlanicv_poll->count);

	RESTORE_INT(flags);
}


unsigned char *get_mic_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	ret = get_buf_from_poll(priv, &priv->pshare->wlanmic_list, (unsigned int *)&priv->pshare->pwlanmic_poll->count);

	RESTORE_INT(flags);
	return ret;
}


void release_mic_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanmic_list, (unsigned int *)&priv->pshare->pwlanmic_poll->count);

	RESTORE_INT(flags);
}


unsigned short get_pnl(union PN48 *ptsc)
{
	return (((ptsc->_byte_.TSC1) << 8) | (ptsc->_byte_.TSC0));
}


unsigned int get_pnh(union PN48 *ptsc)
{
	return 	(((ptsc->_byte_.TSC5) << 24) |
			 ((ptsc->_byte_.TSC4) << 16) |
			 ((ptsc->_byte_.TSC3) << 8) |
			  (ptsc->_byte_.TSC2));
}


int UseSwCrypto(struct rtl8192cd_priv *priv, struct stat_info *pstat, int isMulticast)
{
	if (SWCRYPTO)
		return 1;
	else // hw crypto
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (pstat && pstat->wapiInfo && pstat->wapiInfo->wapiType!=wapiDisable)
		{
			return 1;
		}
#endif

#ifdef CONFIG_RTK_MESH
	if( isMeshPoint(pstat) )
//		return 0;
		return	(pstat->dot11KeyMapping.keyInCam || isMulticast) ? 0 : 1;

#endif

#ifdef WDS
		if (pstat && (pstat->state & WIFI_WDS) && !(pstat->state & WIFI_ASOC_STATE)) {
#ifndef CONFIG_RTL8186_KB
			if (!pstat->dot11KeyMapping.keyInCam)
				return 1;
			else
#endif
				return 0;
		}
#endif

		if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm) {
			if (isMulticast) { // multicast
				if (!priv->pmib->dot11GroupKeysTable.keyInCam)
					return 1;
				else
					return 0;
			}
			else {
				if (!pstat->dot11KeyMapping.keyInCam)
					return 1;
				else // key is in CAM
					return 0;
			}
		}
		else { // legacy 802.11 auth (wep40 || wep104)
#ifdef MBSSID
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			{
				if (GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
					if (isMulticast)
						return 1;
					else {
						if (!pstat->dot11KeyMapping.keyInCam)
							return 1;
						else // key is in CAM
							return 0;
					}
				}
			}
#endif

#ifdef USE_WEP_DEFAULT_KEY
			if (isMulticast && 	!priv->pmib->dot11GroupKeysTable.keyInCam)
				return 1;
#else			
			if (isMulticast) {
				if (!priv->pmib->dot11GroupKeysTable.keyInCam)
					return 1;
			}
			else {
				if (!pstat->dot11KeyMapping.keyInCam)
					return 1;				
			}			
#endif			

			return 0;
		}
	}
}


void check_protection_shortslot(struct rtl8192cd_priv *priv)
{
	if (priv->pmib->dot11ErpInfo.nonErpStaNum == 0 &&
			priv->pmib->dot11ErpInfo.olbcDetected == 0)
	{
		if (priv->pmib->dot11ErpInfo.protection)
			priv->pmib->dot11ErpInfo.protection = 0;
	}
	else
	{
		if (!priv->pmib->dot11StationConfigEntry.protectionDisabled &&
					priv->pmib->dot11ErpInfo.protection == 0)
			priv->pmib->dot11ErpInfo.protection = 1;
	}

	if (priv->pmib->dot11ErpInfo.nonErpStaNum == 0)
	{
		if (priv->pmib->dot11ErpInfo.shortSlot == 0)
		{
			priv->pmib->dot11ErpInfo.shortSlot = 1;
#ifdef MBSSID
			if ((IS_ROOT_INTERFACE(priv))
#ifdef UNIVERSAL_REPEATER
				|| (IS_VXD_INTERFACE(priv))
#endif
				)
#endif
			set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
			SET_SHORTSLOT_IN_BEACON_CAP;
			DEBUG_INFO("set short slot time\n");
		}
	}
	else
	{
		if (priv->pmib->dot11ErpInfo.shortSlot)
		{
			priv->pmib->dot11ErpInfo.shortSlot = 0;
#ifdef MBSSID
			if ((IS_ROOT_INTERFACE(priv))
#ifdef UNIVERSAL_REPEATER
				|| (IS_VXD_INTERFACE(priv))
#endif
				)
#endif
			set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
			RESET_SHORTSLOT_IN_BEACON_CAP;
			DEBUG_INFO("reset short slot time\n");
		}
	}
}


void check_sta_characteristic(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act)
{
	if (act == INCREASE) {
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && !isErpSta(pstat)) {
			priv->pmib->dot11ErpInfo.nonErpStaNum++;
			check_protection_shortslot(priv);

			if (!pstat->useShortPreamble)
				priv->pmib->dot11ErpInfo.longPreambleStaNum++;
		}

		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (pstat->ht_cap_len == 0))
			priv->ht_legacy_sta_num++;
	}
	else {
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && !isErpSta(pstat)) {
			priv->pmib->dot11ErpInfo.nonErpStaNum--;
			check_protection_shortslot(priv);

			if (!pstat->useShortPreamble && priv->pmib->dot11ErpInfo.longPreambleStaNum > 0)
				priv->pmib->dot11ErpInfo.longPreambleStaNum--;
		}

		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (pstat->ht_cap_len == 0))
			priv->ht_legacy_sta_num--;
	}
}

int should_forbid_Nmode(struct rtl8192cd_priv *priv)
{
	if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))
		return 0;

	if (!(priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(3)))
		return 0;

	// if pure TKIP, change N mode to G mode
	if (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(1))	{
		if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 0 
			&& priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm == 1) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPACipher == 2
				|| priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 2)
				return 1;
		}
		else if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 1) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPACipher == 2)
				return 1;
		}
		else if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 2) {
			if (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 2)
				return 1;
		}
		else if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 3) {
			if ((priv->pmib->dot1180211AuthEntry.dot11WPACipher == 2) &&
				(priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 2))
				return 1;
		}
	}

	// if WEP, forbid  N mode
	if ((priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(0)) &&
		 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_40_PRIVACY_ ||
		  priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_104_PRIVACY_) )
		return 1;

	return 0;
}


int should_restrict_Nrate(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if (OPMODE & WIFI_AP_STATE)
	{
		if (pstat->is_legacy_encrpt == 1) {
			if (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(1)) {
				if (!pstat->is_realtek_sta || (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(2)))
					return 1;
			}
		}
		else if (pstat->is_legacy_encrpt == 2) {
			if (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(0)) {
				if (!pstat->is_realtek_sta || (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(2)))
					return 1;
			}
		}
	}
// Client mode IOT issue, Button 2009.07.17
#ifdef CLIENT_MODE
	else if(OPMODE & WIFI_STATION_STATE)
	{
		if (!pstat->is_realtek_sta && !pstat->is_marvell_sta && pstat->is_legacy_encrpt)
			return 1;
	}
#endif

	return 0;
}


#ifdef WDS
int getWdsIdxByDev(struct rtl8192cd_priv *priv, struct net_device *dev)
{
	int i;

#ifdef LAZY_WDS
	int max_num;
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE)
		max_num = NUM_WDS;
	else
		max_num = priv->pmib->dot11WdsInfo.wdsNum;

	for (i=0; i<max_num; i++) {
#else

	for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
#endif

		if (dev == priv->wds_dev[i])
			return i;
	}
	return -1;
}


struct net_device *getWdsDevByAddr(struct rtl8192cd_priv *priv, unsigned char *addr)
{
	int i;

#ifdef LAZY_WDS
	int max_num;
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE)
		max_num = NUM_WDS;
	else
		max_num = priv->pmib->dot11WdsInfo.wdsNum;

	for (i=0; i<max_num; i++) {
#else

	for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
#endif
			if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, addr, 6))
				return priv->wds_dev[i];
	}
	return NULL;
}
#endif // WDS


void validate_oper_rate(struct rtl8192cd_priv *priv)
{
	unsigned int supportedRates;
	unsigned int basicRates;

	if (OPMODE & WIFI_AP_STATE)
	{
		supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
		basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

#ifndef CONFIG_RTL8186_KB
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
				// if use B only, mask G high rate
				supportedRates &= 0xf;
				basicRates &= 0xf;
			}
		}
		else {
			// if use A or G mode, mask B low rate
			supportedRates &= 0xff0;
			basicRates &= 0xff0;
		}

		if (supportedRates == 0) {
			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G | WIRELESS_11A))
				supportedRates = 0xff0;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)
				supportedRates |= 0xf;

			PRINT_INFO("invalid supproted rate, use default value [%x]!\n", supportedRates);
		}

		if (basicRates == 0) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
				basicRates = 0x1f0;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)
				basicRates = 0xf;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))
					basicRates = 0x1f0;
			}

			PRINT_INFO("invalid basic rate, use default value [%x]!\n", basicRates);
		}

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
				if ((basicRates & 0xf) == 0)		// if no CCK rates. jimmylin 2004/12/02
					basicRates |= 0xf;
				if ((supportedRates & 0xf) == 0)	// if no CCK rates. jimmylin 2004/12/02
					supportedRates |= 0xf;
			}
			if ((supportedRates & 0xff0) == 0) {	// no ERP rate existed
				supportedRates |= 0xff0;

				PRINT_INFO("invalid supported rate for 11G, use default value [%x]!\n",
																	supportedRates);
			}
		}
#endif // !CONFIG_RTL8186_KB

		priv->supported_rates = supportedRates;
		priv->basic_rates = basicRates;

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			if (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS == 0)
				priv->pmib->dot11nConfigEntry.dot11nSupportedMCS = 0xffff;
		}
	}
#ifdef CLIENT_MODE
	else
	{
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G))
				priv->dual_band = 1;
			else
				priv->dual_band = 0;
		}
		else
			priv->dual_band = 0;

		if (priv->dual_band) {
			// for 2.4G band
			supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
			basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
					supportedRates &= 0xf;
					basicRates &= 0xf;
				}
				if ((supportedRates & 0xf) == 0)
					supportedRates |= 0xf;
				if ((basicRates & 0xf) == 0)
					basicRates |= 0xf;
			}
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)) {
					supportedRates &= 0xff0;
					basicRates &= 0xff0;
				}
				if ((supportedRates & 0xff0) == 0)
					supportedRates |= 0xff0;
				if ((basicRates & 0xff0) == 0)
					basicRates |= 0x1f0;
			}

			priv->supported_rates = supportedRates;
			priv->basic_rates = basicRates;

			// for 5G band
			supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
			basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

			supportedRates &= 0xff0;
			basicRates &= 0xff0;
			if (supportedRates == 0)
				supportedRates |= 0xff0;
			if (basicRates == 0)
				basicRates |= 0x1f0;

			priv->supported_rates_alt = supportedRates;
			priv->basic_rates_alt = basicRates;
		}
		else {
			supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
			basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
					supportedRates &= 0xf;
					basicRates &= 0xf;
				}
				if ((supportedRates & 0xf) == 0)
					supportedRates |= 0xf;
				if ((basicRates & 0xf) == 0)
					basicRates |= 0xf;
			}
			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G | WIRELESS_11A)) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)) {
					supportedRates &= 0xff0;
					basicRates &= 0xff0;
				}
				if ((supportedRates & 0xff0) == 0)
					supportedRates |= 0xff0;
				if ((basicRates & 0xff0) == 0)
					basicRates |= 0x1f0;
			}

			priv->supported_rates = supportedRates;
			priv->basic_rates = basicRates;
		}

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			if (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS == 0)
				priv->pmib->dot11nConfigEntry.dot11nSupportedMCS = 0xffff;
		}
	}
#endif
}


void get_oper_rate(struct rtl8192cd_priv *priv)
{
	unsigned int supportedRates=0;
	unsigned int basicRates=0;
	unsigned char val;
	int i, idx=0;

	memset(AP_BSSRATE, 0, sizeof(AP_BSSRATE));
	AP_BSSRATE_LEN = 0;

	if (OPMODE & WIFI_AP_STATE) {
		supportedRates = priv->supported_rates;
		basicRates = priv->basic_rates;
	}
#ifdef CLIENT_MODE
	else {
		if (priv->dual_band && (priv->pshare->curr_band == BAND_5G)) {
			supportedRates = priv->supported_rates_alt;
			basicRates = priv->basic_rates_alt;
		}
		else {
			supportedRates = priv->supported_rates;
			basicRates = priv->basic_rates;
		}
	}
#endif

	for (i=0; dot11_rate_table[i]; i++) {
		int bit_mask = 1 << i;
		if (supportedRates & bit_mask) {
			val = dot11_rate_table[i];

#ifdef SUPPORT_SNMP_MIB
			SNMP_MIB_ASSIGN(dot11SupportedDataRatesSet[i], ((unsigned int)val));
			SNMP_MIB_ASSIGN(dot11OperationalRateSet[i], ((unsigned char)val));
#endif

			if (basicRates & bit_mask)
				val |= 0x80;

			AP_BSSRATE[idx] = val;
			AP_BSSRATE_LEN++;
			idx++;
		}
	}

#ifdef SUPPORT_SNMP_MIB
	SNMP_MIB_ASSIGN(dot11SupportedDataRatesNum, ((unsigned char)AP_BSSRATE_LEN));
#endif

}


// bssrate_ie: _SUPPORTEDRATES_IE_ get supported rate set
// bssrate_ie: _EXT_SUPPORTEDRATES_IE_ get extended supported rate set
int get_bssrate_set(struct rtl8192cd_priv *priv, int bssrate_ie, unsigned char **pbssrate, int *bssrate_len)
{
	int i;

	if (priv->pshare->curr_band == BAND_5G)
	{
		if (bssrate_ie == _SUPPORTEDRATES_IE_)
		{
			for(i=0; i<AP_BSSRATE_LEN; i++)
				if (!is_CCK_rate(AP_BSSRATE[i] & 0x7f))
					break;

			if (i == AP_BSSRATE_LEN)
				return FALSE;
			else {
				*pbssrate = &AP_BSSRATE[i];
				*bssrate_len = AP_BSSRATE_LEN - i;
				return TRUE;
			}
		}
		else
			return FALSE;
	}
	else
	{
		if (bssrate_ie == _SUPPORTEDRATES_IE_)
		{
			*pbssrate = AP_BSSRATE;
			if (AP_BSSRATE_LEN > 8)
				*bssrate_len = 8;
			else
				*bssrate_len = AP_BSSRATE_LEN;
			return TRUE;
		}
		else
		{
			if (AP_BSSRATE_LEN > 8) {
				*pbssrate = &AP_BSSRATE[8];
				*bssrate_len = AP_BSSRATE_LEN - 8;
				return TRUE;
			}
			else
				return FALSE;
		}
	}
}


struct channel_list{
	unsigned char	channel[31];
	unsigned char	len;
};
static struct channel_list reg_channel_2_4g[] = {
	/* FCC */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* IC */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* ETSI */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* SPAIN */		{{10,11},2},
	/* FRANCE */	{{10,11,12,13},4},
	/* MKK */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* ISRAEL */	{{3,4,5,6,7,8,9,10,11,12,13},11},
	/* MKK1 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* MKK2 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* MKK3 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
};
#if 0
static struct channel_list reg_channel_5g_low_band[] = {
	/* FCC */		{{36,40,44,48,52,56,60,64},8},
	/* IC */		{{36,40,44,48,52,56,60,64},8},
	/* ETSI */		{{36,40,44,48,52,56,60,64},8},
	/* SPAIN */		{{36,40,44,48,52,56,60,64},8},
	/* FRANCE */	{{36,40,44,48,52,56,60,64},8},
	/* MKK */		{{34,36,38,40,42,44,46,48,52,56,60,64},12},
	/* ISRAEL */	{{36,40,44,48,52,56,60,64},8},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48,52,56,60,64},8},
};
#endif
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,52,56,60,64,149,153,157,161},12},
	/* IC */		{{36,40,44,48,52,56,60,64,149,153,157,161},12},
	/* ETSI */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* SPAIN */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* FRANCE */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* MKK */		{{34,36,38,40,42,44,46,48,52,56,60,64},12},
	/* ISRAEL */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48,52,56,60,64,
					  183,184,185,186,187,188,189,190,192,194,196,207,208,209,210,211,212,214,216},27},
};

int get_available_channel(struct rtl8192cd_priv *priv)
{
	int i, reg;
	struct channel_list *ch_5g_lst=NULL;

	priv->available_chnl_num = 0;
	reg = priv->pmib->dot11StationConfigEntry.dot11RegDomain;

	if ((reg < DOMAIN_FCC) || (reg >= DOMAIN_MAX))
		return FAIL;

	if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G)) {
		for (i=0; i<reg_channel_2_4g[reg-1].len; i++)
			priv->available_chnl[i] = reg_channel_2_4g[reg-1].channel[i];
		priv->available_chnl_num += reg_channel_2_4g[reg-1].len;
	}

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
		ch_5g_lst = reg_channel_5g_full_band;

		for (i=0; i<ch_5g_lst[reg-1].len; i++)
			priv->available_chnl[priv->available_chnl_num+i] = ch_5g_lst[reg-1].channel[i];
		priv->available_chnl_num += ch_5g_lst[reg-1].len;

#ifdef DFS
		// remove the blocked channels from available_chnl[32]
		if (priv->NOP_chnl_num)
			for (i=0; i<priv->NOP_chnl_num; i++)
				RemoveChannel(priv->available_chnl, &priv->available_chnl_num, priv->NOP_chnl[i]);
#endif
	}

// add by david ---------------------------------------------------
	if (priv->pmib->dot11RFEntry.dot11ch_low ||  priv->pmib->dot11RFEntry.dot11ch_hi) {
		unsigned int tmpbuf[100];
		int num=0;
		for (i=0; i<priv->available_chnl_num; i++) {
			if ( (priv->pmib->dot11RFEntry.dot11ch_low &&
					priv->available_chnl[i] < priv->pmib->dot11RFEntry.dot11ch_low) ||
				(priv->pmib->dot11RFEntry.dot11ch_hi &&
					priv->available_chnl[i] > priv->pmib->dot11RFEntry.dot11ch_hi))
				continue;
			else
				tmpbuf[num++] = priv->available_chnl[i];
		}
		if (num) {
			memcpy(priv->available_chnl, tmpbuf, num*4);
			priv->available_chnl_num = num;
		}
	}
//------------------------------------------------------ 2007-04-14

	return SUCCESS;
}


void cnt_assoc_num(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act, char *func)
{
	if (act == INCREASE) {
		if (priv->assoc_num <= NUM_STAT) {
			priv->assoc_num++;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
				if (priv->assoc_num > 1)
					check_DIG_by_rssi(priv, 0);	// force DIG temporary off for association after the fist one

			if (pstat->ht_cap_len) {
				priv->pshare->ht_sta_num++;
				if (priv->pshare->iot_mode_enable && (priv->pshare->ht_sta_num == 1)
#ifdef RTL_MANUAL_EDCA
						&& (priv->pmib->dot11QosEntry.ManualEDCA == 0)
#endif
					)
					IOT_EDCA_switch(priv, priv->pmib->dot11BssType.net_work_type, priv->pshare->iot_mode_enable);

#ifdef WIFI_11N_2040_COEXIST
				if (priv->pmib->dot11nConfigEntry.dot11nCoexist && (OPMODE & WIFI_AP_STATE)
					&& priv->pshare->is_40m_bw) {
					if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_40M_INTOLERANT_)) {
#ifdef STA_EXT
						if (pstat->aid <= FW_NUM_STAT)
							priv->force_20_sta |= BIT(pstat->aid - 1);
						else
							priv->force_20_sta_ext |= BIT(pstat->aid - 1 - FW_NUM_STAT);
#else
						priv->force_20_sta |= BIT(pstat->aid -1);
#endif
					}
				}
#endif

				check_NAV_prot_len(priv, pstat, 0);
			}
		}
		else
			DEBUG_ERR("Association Number Error (%d)!\n", NUM_STAT);
	}
	else {
		if (priv->assoc_num > 0) {
			priv->assoc_num--;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
				if (!priv->assoc_num)
					check_DIG_by_rssi(priv, 0);	// force DIG off while no station associated

			if (pstat->ht_cap_len) {
				if (--priv->pshare->ht_sta_num < 0)
					printk("ht_sta_num error\n");  // this should not happen
				else{
					if (priv->pshare->iot_mode_enable && !priv->pshare->ht_sta_num
#ifdef RTL_MANUAL_EDCA
							&& (priv->pmib->dot11QosEntry.ManualEDCA == 0)
#endif
						)
						IOT_EDCA_switch(priv, priv->pmib->dot11BssType.net_work_type, priv->pshare->iot_mode_enable);

#ifdef WIFI_11N_2040_COEXIST
					if (priv->pmib->dot11nConfigEntry.dot11nCoexist && (OPMODE & WIFI_AP_STATE)
						&& priv->pshare->is_40m_bw) {
						if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_40M_INTOLERANT_)) {
#ifdef STA_EXT
							if (pstat->aid <= FW_NUM_STAT)
								priv->force_20_sta &= ~BIT(pstat->aid -1);
							else
								priv->force_20_sta_ext &= ~BIT(pstat->aid - 1 - FW_NUM_STAT);
#else
							priv->force_20_sta &= ~BIT(pstat->aid -1);
#endif
						}
					}
#endif

					check_NAV_prot_len(priv, pstat, 1);
				}
			}
		}
		else
			DEBUG_ERR("Association Number Error (0)!\n");
	}

	DEBUG_INFO("assoc_num%s(%d) in %s %02X%02X%02X%02X%02X%02X\n",
		act?"++":"--",
		priv->assoc_num,
		func,
		pstat->hwaddr[0],
		pstat->hwaddr[1],
		pstat->hwaddr[2],
		pstat->hwaddr[3],
		pstat->hwaddr[4],
		pstat->hwaddr[5]);
}


/*
 * Use this function to get the number of associated station, no matter
 * it is expired or not. And don't count WDS peers in.
 */
int get_assoc_sta_num(struct rtl8192cd_priv *priv)
{
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	int sta_num;

	sta_num = 0;
	phead = &priv->asoc_list;
	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat->state & WIFI_ASOC_STATE)
			sta_num++;
		plist = plist->next;
	}
	return sta_num;
}


void event_indicate(struct rtl8192cd_priv *priv, unsigned char *mac, int event)
{
#ifdef LINUX_2_6_27_
	struct pid *pid;
#endif
#ifdef __KERNEL__
#ifdef USE_CHAR_DEV
	if (priv->pshare->chr_priv && priv->pshare->chr_priv->asoc_fasync)
		kill_fasync(&priv->pshare->chr_priv->asoc_fasync, SIGIO, POLL_IN);
#endif
#ifdef USE_PID_NOTIFY
	if (priv->pshare->wlanapp_pid > 0)
#ifdef LINUX_2_6_27_
		rcu_read_lock();
		pid = get_pid(find_vpid(priv->pshare->wlanapp_pid));
		rcu_read_unlock();
		kill_pid(pid, SIGIO, 1);
#else
		kill_proc(priv->pshare->wlanapp_pid, SIGIO, 1);
#endif
#endif
#endif

#ifdef __DRAYTEK_OS__
	if (event == 2)
		cb_disassoc_indicate(priv->dev, mac);
#endif

#ifdef GREEN_HILL
	extern void indicate_to_upper(int reason, unsigned char *addr);
	if (event > 0)
		indicate_to_upper(event, mac);
#endif
}

#ifdef WIFI_HAPD
int event_indicate_hapd(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra)
{
	struct net_device	*dev = (struct net_device *)priv->dev;
	union iwreq_data wreq;

	//printk("event_indicate_hapd +++, event =0x%x\n", event);

	memset(&wreq, 0, sizeof(wreq));

	if(event == HAPD_EXIRED)
		{
			memcpy(wreq.addr.sa_data, mac, 6);
			wireless_send_event(dev, IWEVEXPIRED, &wreq, NULL);
			return 0;
		}
	else if(event == HAPD_REGISTERED)
		{
			memcpy(wreq.addr.sa_data, mac, 6);
			wireless_send_event(dev, IWEVREGISTERED, &wreq, NULL);
			return 0;
		}
	else if(event == HAPD_MIC_FAILURE)
		{
			char buf[6];
			memcpy(buf, mac, 6);
			wreq.data.flags = event;
			wreq.data.length = 6;
			wireless_send_event(dev, IWEVCUSTOM, &wreq, buf);
			return 0;
		}
	else if(event == HAPD_WPS_PROBEREQ)
		{
			wreq.data.flags = event;
			wreq.data.length = sizeof(struct _DOT11_PROBE_REQUEST_IND);
			wireless_send_event(dev, IWEVGENIE, &wreq, extra); //IW_CUSTOM_MAX is 256, can NOT afford  _DOT11_PROBE_REQUEST_IND
		}		
	else{
			//Not used yet
			wreq.data.flags = event;
			wireless_send_event(dev, IWEVCUSTOM, &wreq, extra);
			return 0;
		}

	return -1;
}
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
void wapi_event_indicate(struct rtl8192cd_priv *priv)
{
	if (priv->pshare->wlanwapi_pid > 0)
#ifdef LINUX_2_6_27_
		kill_pid(priv->pshare->wlanwapi_pid, SIGIO, 1);
#else
		kill_proc(priv->pshare->wlanwapi_pid, SIGIO, 1);
#endif
}
#endif

#ifdef USE_WEP_DEFAULT_KEY
void init_DefaultKey_Enc(struct rtl8192cd_priv *priv, unsigned char *key, int algorithm)
{
	unsigned char defaultmac[4][6];
	int i;

	memset(defaultmac, 0, sizeof(defaultmac));
	for(i=0; i<4; i++)
		defaultmac[i][5] = i;

	for(i=0; i<4; i++)
	{
		CamDeleteOneEntry(priv, defaultmac[i], i, 1);
		if (key == NULL)
			CamAddOneEntry(priv, defaultmac[i], i,
					(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)<<2,
					1, priv->pmib->dot11DefaultKeysTable.keytype[i].skey);
		else
			CamAddOneEntry(priv, defaultmac[i], i,
					algorithm<<2,
					1, key);
	}
	priv->pshare->CamEntryOccupied += 4;
}
#endif


#ifdef UNIVERSAL_REPEATER
//
// Disable AP function in virtual interface
//
void disable_vxd_ap(struct rtl8192cd_priv *priv)
{
	unsigned long flags;

	if ((priv==NULL) || !(priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE))
		return;

	if (!(priv->drv_state & DRV_STATE_VXD_AP_STARTED))
		return;
	else
		priv->drv_state &= ~DRV_STATE_VXD_AP_STARTED;

	DEBUG_INFO("Disable vxd AP\n");

	if (IS_DRV_OPEN(priv))
		rtl8192cd_close(priv->dev);

	SAVE_INT_AND_CLI(flags);

	RTL_W32(HIMR, RTL_R32(HIMR) & ~(HIMR_BCNDOK0));
	//RTL_W16(ATIMWND, 2);
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));

	RESTORE_INT(flags);
}


//
// Enable AP function in virtual interface
//
void enable_vxd_ap(struct rtl8192cd_priv *priv)
{
	unsigned long flags;

	if ((priv==NULL) || !(priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) ||
		!(priv->drv_state & DRV_STATE_VXD_INIT))
		return;

	if (priv->drv_state & DRV_STATE_VXD_AP_STARTED)
		return;
	else
		priv->drv_state |= DRV_STATE_VXD_AP_STARTED;

	DEBUG_INFO("Enable vxd AP\n");

	priv->pmib->dot11RFEntry.dot11channel = GET_ROOT_PRIV(priv)->pmib->dot11Bss.channel;
	//priv->pmib->dot11BssType.net_work_type = GET_ROOT_PRIV(priv)->oper_band;
	priv->pmib->dot11BssType.net_work_type = GET_ROOT_PRIV(priv)->pmib->dot11BssType.net_work_type &
		GET_ROOT_PRIV(priv)->pmib->dot11Bss.network;

	if (!IS_DRV_OPEN(priv))
		rtl8192cd_open(priv->dev);
	else {
		//priv->oper_band = priv->pmib->dot11BssType.net_work_type;
		validate_oper_rate(priv);
		get_oper_rate(priv);
	}

	memcpy(priv->pmib->dot11StationConfigEntry.dot11Bssid, GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
	memcpy(GET_MY_HWADDR, priv->pmib->dot11StationConfigEntry.dot11Bssid, MACADDRLEN);
	memcpy(priv->pmib->dot11Bss.bssid, priv->pmib->dot11StationConfigEntry.dot11Bssid, MACADDRLEN);

	SAVE_INT_AND_CLI(flags);
	priv->ht_cap_len = 0;
	init_beacon(priv);

	RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_BCNDOK0);
	//RTL_W16(ATIMWND, 0x0030);
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
	if (!IS_TEST_CHIP(priv)) {
		RTL_W8(0x422, RTL_R8(0x422) | BIT(6));
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
		RTL_W8(BCN_CTRL, DIS_TSF_UPDATE_N| EN_BCN_FUNCTION | DIS_SUB_STATE_N | EN_TXBCN_RPT);
	}

	RESTORE_INT(flags);
}
#endif // UNIVERSAL_REPEATER


#ifdef GBWC
void rtl8192cd_GBWC_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct sk_buff *pskb;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (priv->pmib->gbwcEntry.GBWCMode == 0) {
		mod_timer(&priv->GBWC_timer, jiffies + GBWC_TO);
		return;
	}

	priv->GBWC_consuming_Q = 1;

	// clear bandwidth control counter
	priv->GBWC_tx_count = 0;
	priv->GBWC_rx_count = 0;

	// consume Tx queue
	while(1)
	{
		pskb = (struct sk_buff *)deque(priv, &(priv->GBWC_tx_queue.head), &(priv->GBWC_tx_queue.tail),
			(unsigned int)(priv->GBWC_tx_queue.pSkb), NUM_TXPKT_QUEUE);

		if (pskb == NULL)
			break;

#ifdef ENABLE_RTL_SKB_STATS
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif

		if (rtl8192cd_start_xmit(pskb, priv->dev))
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
	}

	// consume Rx queue
	while(1)
	{
		pskb = (struct sk_buff *)deque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
			(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE);

		if (pskb == NULL)
			break;

		rtl_netif_rx(priv, pskb, (struct stat_info *)*(unsigned int *)&(pskb->cb[4]));
	}

	priv->GBWC_consuming_Q = 0;

	mod_timer(&priv->GBWC_timer, jiffies + GBWC_TO);
}
#endif


#ifdef STA_EXT
unsigned char fw_was_full(struct rtl8192cd_priv *priv)
{
	struct list_head *phead;
	struct list_head *plist;
	struct stat_info *pstat;

	if(list_empty(&priv->asoc_list))
		return 0;

        phead = &priv->asoc_list;
        plist = phead->next;

        while (plist != phead) {
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->next;
			if(pstat->remapped_aid == FW_NUM_STAT-1)
				return 1;
	}
	return 0;
}


// Support more STAs than fw could support
// This function returns one free AID number for newly added rateid
unsigned int find_reampped_aid(struct rtl8192cd_priv *priv, unsigned int rateid)
{
	int i, j;

	for (i = 1; i < NUM_STAT; i++) {
		if (priv->pshare->remapped_aidarray[i] == rateid)
			break;
	}

	for (j = 1; j < NUM_STAT; j++) {
		if (priv->pshare->remapped_aidarray[j] == 0)
			break;
	}

	if ((i == NUM_STAT) && (j == NUM_STAT))
		return 0; //ERROR! this should not happen

	if ((i < (FW_NUM_STAT-1)) || (j == NUM_STAT) || (i <= j))
		return i;
	else
		return j;
}


int realloc_RATid(struct rtl8192cd_priv *priv)
{
	struct list_head *phead;
	struct list_head *plist;
	struct stat_info *pstat =NULL, *pstat_chosen = NULL;
	unsigned int max_through_put = 0;
	unsigned int have_chosen = 0;

	if(list_empty(&priv->asoc_list))
		return 0;

	phead = &priv->asoc_list;
	plist = phead->next;

	while (plist != phead) {
		int temp_through_put ;
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if (pstat->remapped_aid < FW_NUM_STAT-1)// STA has rate adaptive
			continue;

		temp_through_put =  pstat->tx_avarage + pstat->rx_avarage;

		if (temp_through_put >= max_through_put){
			pstat_chosen = pstat;
			max_through_put = temp_through_put;
			have_chosen = 1;
		}
	}

	if (have_chosen == 0)
		return 0;

	// for debug
	//	printk("realloc_RATid, chosen aid is %d, throughput is %d\n", pstat_chosen->aid, max_through_put);

#ifdef STA_EXT
	release_remapAid(priv,pstat_chosen);
#endif

	add_update_RATid(priv, pstat_chosen);

	return 1;
}
#endif


unsigned int is_h2c_buf_occupy(struct rtl8192cd_priv *priv)
{
	 unsigned int occupied = 0;

	if ((IS_TEST_CHIP(priv) && RTL_R8(0x1c0+priv->pshare->fw_q_fifo_count)) ||
		(RTL_R8(0x1cc) & BIT(priv->pshare->fw_q_fifo_count)))
		occupied++;

	return occupied;
}


short signin_h2c_cmd(struct rtl8192cd_priv *priv, unsigned int content, unsigned short ext_content)
{
	int c=0;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		goto SigninFAIL;
#endif

	/*
	 *	Check if h2c cmd signin buffer is occupied,
	 *	for Power Saving related functions only
	 */
	if ((content & 0x7f) < H2C_CMD_RSSI) {
		while (is_h2c_buf_occupy(priv)) {
			delay_us(10);
			if(++c ==30)
				goto SigninFAIL;
		}
	}

	/*
	 * signin reg in order to fit hw requirement
	 */
	if(content & BIT(7))
		RTL_W16(0x88+(priv->pshare->fw_q_fifo_count*2), ext_content);

	RTL_W32(HMEBOX_0+(priv->pshare->fw_q_fifo_count*4), content);

#ifdef TESTCHIP_SUPPORT
	/*
	 * set own bit
	 */
	if(IS_TEST_CHIP(priv))
		RTL_W8(0x1c0+priv->pshare->fw_q_fifo_count, 1);
#endif

	/*
	 * rollover ring buffer count
	 */
	if (++priv->pshare->fw_q_fifo_count > 3)
		priv->pshare->fw_q_fifo_count = 0;

	return 0;

SigninFAIL:
	return 1;
}


void set_ps_cmd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;
	unsigned int content = 0;

	SAVE_INT_AND_CLI(flags);

	/*
	 * set ps state
	 */
	 if (pstat->state & WIFI_SLEEP_STATE)
	 	content |= BIT(24);

	/*
	 * set macid
	 */
	 content |= pstat->aid << 8;

	/*
	 * set cmd id
	 */
	 content |= H2C_CMD_PS;

	signin_h2c_cmd(priv, content, 0);

	RESTORE_INT(flags);
}


void add_ps_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;
	unsigned long flags;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->add_ps_timer))
		del_timer_sync(&priv->add_ps_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
		return;
#endif

	if (!list_empty(&priv->addps_list)) {
		pstat = list_entry(priv->addps_list.next, struct stat_info, addps_list);
		if (!pstat)
			return;

		if (!is_h2c_buf_occupy(priv)) {
			set_ps_cmd(priv, pstat);
			if (!list_empty(&pstat->addps_list)) {
				SAVE_INT_AND_CLI(flags);
				list_del_init(&pstat->addps_list);
				RESTORE_INT(flags);
			}

			if (!list_empty(&priv->addps_list))
				set_timer++;
		} else {
			set_timer++;
		}
	}

	if (set_timer)
		mod_timer(&priv->add_ps_timer, jiffies + 5);	// 50 ms
}


void add_update_ps(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;

	if (is_h2c_buf_occupy(priv)) {
		if (list_empty(&pstat->addps_list)) {
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&(pstat->addps_list), &(priv->addps_list));
			RESTORE_INT(flags);

			if (!timer_pending(&priv->add_ps_timer))
				mod_timer(&priv->add_ps_timer, jiffies + 5);	// 50 ms
		}
	} else {
		set_ps_cmd(priv, pstat);
	}
}


void set_rssi_cmd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;
	unsigned int content = 0;

	int rssi = pstat->rssi;

#ifdef HIGH_POWER_EXT_PA
	if( priv->pshare->rf_ft_var.use_ext_pa ) 
		rssi += RSSI_DIFF_PA;
	if( rssi > 100)
		rssi = 100;
#endif

	
	SAVE_INT_AND_CLI(flags);

	/*
	 * set rssi
	 */
	 content = rssi<< 24;

	/*
	 * set macid
	 */
	 content |= pstat->aid << 8;

	/*
	 * set cmd id
	 */
	 content |= H2C_CMD_RSSI;

	signin_h2c_cmd(priv, content, 0);

	RESTORE_INT(flags);
}


void add_rssi_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;
	unsigned long flags;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->add_rssi_timer))
		del_timer_sync(&priv->add_rssi_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
		return;
#endif

	if (!list_empty(&priv->addrssi_list)) {
		pstat = list_entry(priv->addrssi_list.next, struct stat_info, addrssi_list);
		if (!pstat)
			return;

		if (!is_h2c_buf_occupy(priv)) {
			set_rssi_cmd(priv, pstat);
			if (!list_empty(&pstat->addrssi_list)) {
				SAVE_INT_AND_CLI(flags);
				list_del_init(&pstat->addrssi_list);
				RESTORE_INT(flags);
			}

			if (!list_empty(&priv->addrssi_list))
				set_timer++;
		} else {
			set_timer++;
		}
	}

	if (set_timer)
		mod_timer(&priv->add_rssi_timer, jiffies + 5);	// 50 ms
}


void add_update_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;

	if (is_h2c_buf_occupy(priv)) {
		if (list_empty(&pstat->addrssi_list)) {
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&(pstat->addrssi_list), &(priv->addrssi_list));
			RESTORE_INT(flags);

			if (!timer_pending(&priv->add_rssi_timer))
				mod_timer(&priv->add_rssi_timer, jiffies + 5);	// 50 ms
		}
	} else {
		set_rssi_cmd(priv, pstat);
	}
}


void set_RATid_cmd(struct rtl8192cd_priv *priv, unsigned int macid, unsigned int rateid, unsigned int ratemask)
{
	unsigned int content = 0;
	unsigned short ext_content = 0;

	/*
	 * set ratemask
	 */
	ext_content = ratemask & 0xffff;
	content = ((ratemask & 0xfff0000) >> 16) << 8;

	/*
	 * set short GI
	 */
	if (ratemask & BIT(28))
		content |= BIT(29);

	/*
	 * set macid (station aid)
	 */
	content |= (macid & 0x1f) << 24;

	/*
	 * set rateid (ARFR table)
	 */
	content |= (rateid & 0xf) << 20;

	/*
	 * set ext_content used
	 */
	content |= BIT(7);

	/*
	 * set cmd id
	 */
	content |= H2C_CMD_MACID;

	signin_h2c_cmd(priv, content, ext_content);
}


void add_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char limit=16;
	int i;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	pstat->tx_ra_bitmap = 0;

	for (i=0; i<32; i++) {
		if (pstat->bssrateset[i])
			pstat->tx_ra_bitmap |= get_bit_value_from_ieee_value(pstat->bssrateset[i]&0x7f);
	}

	if (pstat->ht_cap_len) {
		if ((pstat->MIMO_ps & _HT_MIMO_PS_STATIC_) ||
			(get_rf_mimo_mode(priv)== MIMO_1T2R) ||
			(get_rf_mimo_mode(priv)== MIMO_1T1R))
			limit=8;

		for (i=0; i<limit; i++) {
			if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8))
				pstat->tx_ra_bitmap |= BIT(i+12);
		}
	}
	if (pstat->ht_cap_len &&
		((priv->pshare->is_40m_bw && pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_) &&
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)) ||
		(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_) &&
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M && (pstat->tx_bw == HT_CHANNEL_WIDTH_20))))
		pstat->tx_ra_bitmap |= BIT(28);

	if ((pstat->rssi_level < 1) || (pstat->rssi_level > 3)) {
		if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
			pstat->rssi_level = 1;
		else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
			((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
			(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
			(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
			pstat->rssi_level = 2;
		else
			pstat->rssi_level = 3;
	}

	// rate adaptive by rssi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
		if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)) {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x100f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x100ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x100ff005;
					else
						pstat->tx_ra_bitmap &= 0x100ff001;
					break;
			}
		} else {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x1f8f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x1f8ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x010ff005;
					else
						pstat->tx_ra_bitmap &= 0x010ff001;
					break;
			}

			// Don't need to mask high rates due to new rate adaptive parameters
			//if (pstat->is_broadcom_sta)		// use MCS12 as the highest rate vs. Broadcom sta
			//	pstat->tx_ra_bitmap &= 0x81ffffff;

			// NIC driver will report not supporting MCS15 and MCS14 in asoc req
			//if (pstat->is_rtl8190_sta && !pstat->is_2t_mimo_sta)
			//	pstat->tx_ra_bitmap &= 0x83ffffff;		// if Realtek 1x2 sta, don't use MCS15 and MCS14
		}
	} else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && isErpSta(pstat)) {
		switch (pstat->rssi_level) {
			case 1:
				pstat->tx_ra_bitmap &= 0x00000f00;
				break;
			case 2:
				pstat->tx_ra_bitmap &= 0x00000ff0;
				break;
			case 3:
				pstat->tx_ra_bitmap &= 0x00000ff5;
				break;
		}
	} else {
		pstat->tx_ra_bitmap &= 0x0000000d;
	}

// Client mode IOT issue, Button 2009.07.17
#ifdef CLIENT_MODE
	if(OPMODE & WIFI_STATION_STATE) {
		if(!pstat->is_rtl8192s_sta && pstat->is_realtek_sta && pstat->is_legacy_encrpt)
			pstat->tx_ra_bitmap &= 0x0001ffff;					// up to MCS4
	}
#endif

#ifdef STA_EXT
	//update STA_map
	{
		int remapped_aid = 0;
		remapped_aid = find_reampped_aid(priv, pstat->aid);
		if(remapped_aid == 0) {
			/*WARNING:  THIS SHOULD NOT HAPPEN*/
			printk("add AID fail!!\n");
			BUG();
		}
		if(remapped_aid >= (FW_NUM_STAT-1)){// no room for the STA
//			priv->STA_map |= (1<< pstat->aid) ;
			pstat->remapped_aid = FW_NUM_STAT-1;
			pstat->sta_in_firmware = 0; // this value will updated in expire_timer
		} else if(priv->pshare->remapped_aidarray[remapped_aid]  == 0) { // if not 0, it should have been added before
			//we got a room
			//clear STA_map
//			priv->STA_map &= ~(BIT(pstat->aid));
			pstat->remapped_aid = remapped_aid;
			priv->pshare->remapped_aidarray[remapped_aid] = pstat->aid;
			pstat->sta_in_firmware = 1; // this value will updated in expire_timer
			priv->pshare->fw_free_space --;
		} else {// added before
			pstat->sta_in_firmware = 1;
		}
	}
#endif// STA_EXT

#ifdef STA_EXT
	if (REMAP_AID(pstat) < FW_NUM_STAT-1)
#else
	if (REMAP_AID(pstat) < 32)
#endif
	{
		if (pstat->tx_ra_bitmap & 0xff00000) {
			if (priv->pshare->is_40m_bw)
				set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_40M, pstat->tx_ra_bitmap);
			else
				set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_2T_20M, pstat->tx_ra_bitmap);
		} else if (pstat->tx_ra_bitmap & 0xff000) {
			if (priv->pshare->is_40m_bw)
				set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_1T_40M, pstat->tx_ra_bitmap);
			else
				set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_1T_20M, pstat->tx_ra_bitmap);
		} else if (pstat->tx_ra_bitmap & 0xff0) {
			set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_BG_MIX, pstat->tx_ra_bitmap);
		} else {
			set_RATid_cmd(priv, REMAP_AID(pstat), ARFR_B_ONLY, pstat->tx_ra_bitmap);
		}

		DEBUG_INFO("Add id %d val %08x to ratr\n", pstat->aid, pstat->tx_ra_bitmap);
	} else {
#ifdef STA_EXT
		if (priv->pshare->is_40m_bw)
			set_RATid_cmd(priv, (FW_NUM_STAT-1), ARFR_2T_40M, 0x1fffffff);
		else
			set_RATid_cmd(priv, (FW_NUM_STAT-1), ARFR_2T_20M, 0x1fffffff);
#else
		DEBUG_ERR("station aid %d exceed the max number\n", pstat->aid);
#endif
	}

	RESTORE_INT(flags);
}


#ifdef STA_EXT
void release_remapAid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if(pstat->remapped_aid != FW_NUM_STAT-1){
		int i;
		for(i = 1; i < NUM_STAT; i++)
			if(priv->pshare->remapped_aidarray[i] == pstat->aid){
				priv->pshare->remapped_aidarray[i] = 0;
				priv->pshare->fw_free_space ++;
				break;
			}
	}
	pstat->sta_in_firmware = -1;

	DEBUG_INFO("Remove id %d from ratr\n", pstat->aid);
}
#endif


void add_RATid_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;
	unsigned long flags;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->add_RATid_timer))
		del_timer_sync(&priv->add_RATid_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
		return;
#endif

	if (!list_empty(&priv->addRAtid_list)) {
		pstat = list_entry(priv->addRAtid_list.next, struct stat_info, addRAtid_list);
		if (!pstat)
			return;

		if (!is_h2c_buf_occupy(priv)) {
			add_RATid(priv, pstat);
			if (!list_empty(&pstat->addRAtid_list)) {
				SAVE_INT_AND_CLI(flags);
				list_del_init(&pstat->addRAtid_list);
				RESTORE_INT(flags);
			}

			if (!list_empty(&priv->addRAtid_list))
				set_timer++;
		} else {
			set_timer++;
		}
	}

	if (set_timer)
		mod_timer(&priv->add_RATid_timer, jiffies + 5);	// 50 ms
}


void add_update_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;

	if (is_h2c_buf_occupy(priv)) {
		if (list_empty(&pstat->addRAtid_list)) {
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&(pstat->addRAtid_list), &(priv->addRAtid_list));
			RESTORE_INT(flags);

			if (!timer_pending(&priv->add_RATid_timer))
				mod_timer(&priv->add_RATid_timer, jiffies + 5);	// 50 ms
		}
	} else {
		add_RATid(priv, pstat);
	}
}


#ifdef RTK_QUE
void rtk_queue_init(struct ring_que *que)
{
	memset(que, '\0', sizeof(struct ring_que));
	que->qmax = MAX_PRE_ALLOC_SKB_NUM;
}

static int rtk_queue_tail(struct rtl8192cd_priv *priv, struct ring_que *que, struct sk_buff *skb)
{
	int next;
	unsigned long x;

	SAVE_INT_AND_CLI(x);

	if (que->head == que->qmax)
		next = 0;
	else
		next = que->head + 1;

	if (que->qlen >= que->qmax || next == que->tail) {
		printk("%s: ring-queue full!\n", __FUNCTION__);
		RESTORE_INT(x);
		return 0;
	}

	que->ring[que->head] = skb;
	que->head = next;
	que->qlen++;

	RESTORE_INT(x);
	return 1;
}


#ifndef __LINUX_2_6__
__IRAM_IN_865X
#endif
static struct sk_buff *rtk_dequeue(struct rtl8192cd_priv *priv, struct ring_que *que)
{
	struct sk_buff *skb;
	unsigned long x;

	SAVE_INT_AND_CLI(x);

	if (que->qlen <= 0 || que->tail == que->head) {
		RESTORE_INT(x);
		return NULL;
	}

	skb = que->ring[que->tail];

	if (que->tail == que->qmax)
		que->tail  = 0;
	else
		que->tail++;

	que->qlen--;

	RESTORE_INT(x);
	return (struct sk_buff *)skb;
}


void free_rtk_queue(struct rtl8192cd_priv *priv, struct ring_que *skb_que)
{
	struct sk_buff *skb;

	while (skb_que->qlen > 0) {
		skb = rtk_dequeue(priv, skb_que);
		if (skb == NULL)
			break;
		dev_kfree_skb_any(skb);
	}
}
#endif // RTK_QUE


void refill_skb_queue(struct rtl8192cd_priv *priv)
{
	struct sk_buff *skb;
#ifdef DELAY_REFILL_RX_BUF
	extern int refill_rx_ring(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned char *data);
	struct rtl8192cd_hw *phw=GET_HW(priv);
#endif

	while (priv->pshare->skb_queue.qlen < MAX_PRE_ALLOC_SKB_NUM) {

#ifdef CONFIG_RTL8190_PRIV_SKB
		skb = dev_alloc_skb_priv(priv, RX_BUF_LEN);
#else
		skb = dev_alloc_skb(RX_BUF_LEN);
#endif

		if (skb == NULL) {
//			DEBUG_ERR("dev_alloc_skb() failed!\n");
			return;
		}

#ifdef DELAY_REFILL_RX_BUF
		if (phw->cur_rx_refill != phw->cur_rx) {
			refill_rx_ring(priv, skb, NULL);
			continue;
		}
#endif

#ifdef RTK_QUE
		rtk_queue_tail(priv, &priv->pshare->skb_queue, skb);
#else
		__skb_queue_tail(&priv->pshare->skb_queue, skb);
#endif
	}
}


#ifndef __LINUX_2_6__
__MIPS16
__IRAM_IN_865X
#endif
struct sk_buff *alloc_skb_from_queue(struct rtl8192cd_priv *priv)
{
	struct sk_buff *skb;

	if (priv->pshare->skb_queue.qlen == 0) {
		struct sk_buff *skb;
#ifdef CONFIG_RTL8190_PRIV_SKB
		skb = dev_alloc_skb_priv(priv, RX_BUF_LEN);
#else
		skb = dev_alloc_skb(RX_BUF_LEN);
#endif
		if (skb == NULL) {
			DEBUG_ERR("dev_alloc_skb() failed!\n");
		}

		return skb;
	}
#ifdef RTK_QUE
	skb = rtk_dequeue(priv, &priv->pshare->skb_queue);
#else
	skb = __skb_dequeue(&priv->pshare->skb_queue);
#endif

	return skb;
}


void free_skb_queue(struct rtl8192cd_priv *priv, struct sk_buff_head	*skb_que)
{
	struct sk_buff *skb;
	unsigned long flags;

	while (skb_que->qlen > 0) {
// 2009.09.08
		SAVE_INT_AND_CLI(flags);
		skb = __skb_dequeue(skb_que);
		RESTORE_INT(flags);
		if (skb == NULL)
			break;
		dev_kfree_skb_any(skb);
	}
}

#ifdef FAST_RECOVERY
struct backup_info {
	struct aid_obj *sta[NUM_STAT];
	struct Dot11KeyMappingsEntry gkey;
#ifdef WDS
	struct wds_info	wds;
#endif
};

void *backup_sta(struct rtl8192cd_priv *priv)
{
	int i;
	struct backup_info *pBackup;

	pBackup = (struct backup_info *)kmalloc((sizeof(struct backup_info)), GFP_ATOMIC);
	if (pBackup == NULL) {
		printk("%s: kmalloc() failed!\n", __FUNCTION__);
		return NULL;
	}
	memset(pBackup, '\0', sizeof(struct backup_info));
	for (i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv !=  priv->pshare->aidarray[i]->priv)
				continue;
#endif
			pBackup->sta[i] = (struct aid_obj *)kmalloc((sizeof(struct aid_obj)), GFP_ATOMIC);
			if (pBackup->sta[i] == NULL) {
				printk("%s: kmalloc(sta) failed!\n", __FUNCTION__);
				kfree(pBackup);
				return NULL;
			}
			memcpy(pBackup->sta[i], priv->pshare->aidarray[i], sizeof(struct aid_obj));
		}
	}

#ifdef WDS
	memcpy(&pBackup->wds, &priv->pmib->dot11WdsInfo, sizeof(struct wds_info));
#endif
	memcpy(&pBackup->gkey, &priv->pmib->dot11GroupKeysTable, sizeof(struct Dot11KeyMappingsEntry));

	return (void *)pBackup;
}


void restore_backup_sta(struct rtl8192cd_priv *priv, void *pInfo)
{
	unsigned int i, offset;
	struct stat_info *pstat;
	unsigned char	key_combo[32];
	struct backup_info *pBackup=(struct backup_info *)pInfo;
#ifdef CONFIG_RTK_MESH
	unsigned char	is_11s_MP = FALSE;
#endif

	for (i=0; i<NUM_STAT; i++) {
		if (pBackup->sta[i]) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv !=  pBackup->sta[i]->priv)
				continue;
#endif

#ifdef CONFIG_RTK_MESH	// Restore Establish MP ONLY
			if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && !isSTA2(pBackup->sta[i]->station)) {
				UINT8 State = pBackup->sta[i]->station.mesh_neighbor_TBL.State;

				if ((State == MP_SUPERORDINATE_LINK_UP) || (State == MP_SUBORDINATE_LINK_UP)
						|| (State == MP_SUPERORDINATE_LINK_DOWN) || (State == MP_SUBORDINATE_LINK_DOWN_E))
					is_11s_MP = TRUE;
				else	// is MP, but not establish, Give up.
				{
					kfree(pBackup->sta[i]);
					continue;
				}
			}
#endif

			pstat = alloc_stainfo(priv, pBackup->sta[i]->station.hwaddr, i);
			if (!pstat) {
				printk("%s: alloc_stainfo() failed!\n", __FUNCTION__);
				return;
			}

			offset = (unsigned long)(&((struct stat_info *)0)->aid);
			memcpy(((unsigned char *)pstat)+offset,
				((unsigned char *)&pBackup->sta[i]->station)+offset, sizeof(struct stat_info)-offset);
			list_add_tail(&pstat->asoc_list, &priv->asoc_list);

#ifdef CONFIG_RTK_MESH
			if (TRUE == is_11s_MP) {
				is_11s_MP = FALSE;
				list_add_tail(&pstat->mesh_mp_ptr, &priv->mesh_mp_hdr);
				mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, INCREASE);
			}
#endif

#ifdef WDS
			if (!(pstat->state & WIFI_WDS))
#endif
			if (pstat->expire_to > 0) 
				cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);

			if ((pstat->expire_to > 0) 
#ifdef WDS
				|| (pstat->state & WIFI_WDS)
#endif
			) {
				check_sta_characteristic(priv, pstat, INCREASE);
				if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
					construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);

#ifndef USE_WEP_DEFAULT_KEY
				set_keymapping_wep(priv, pstat);
#endif
				if (!SWCRYPTO && pstat->dot11KeyMapping.dot11Privacy && pstat->dot11KeyMapping.keyInCam == TRUE) {
					memcpy(key_combo,
						pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey,
						pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen);
					memcpy(&key_combo[pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen],
						pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey1.skey,
						pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen);

					CamAddOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid,
						pstat->dot11KeyMapping.dot11Privacy<<2, 0, key_combo);
					priv->pshare->CamEntryOccupied++;
				}
			}
			// to avoid add RAtid fail
			add_update_RATid(priv, pstat);
			kfree(pBackup->sta[i]);

			if ( priv->pshare->is_40m_bw && pstat->is_marvell_sta){
#ifdef STA_EXT
				if (pstat->aid > FW_NUM_STAT)
					priv->pshare->marvellMapBitExt |= BIT(pstat->aid - FW_NUM_STAT - 1);
				else
#endif
					priv->pshare->marvellMapBit |= BIT(pstat->aid - 1);

				if (priv->pshare->Reg_RRSR_2 == 0 && priv->pshare->Reg_81b == 0){
					priv->pshare->Reg_RRSR_2 = RTL_R8(RRSR+2);
					priv->pshare->Reg_81b = RTL_R8(0x81b);
					RTL_W8(RRSR+2, priv->pshare->Reg_RRSR_2 | 0x60);
					RTL_W8(0x81b, priv->pshare->Reg_81b | 0x0E); 
				} 
			}
		}
	}

	memcpy(&priv->pmib->dot11GroupKeysTable, &pBackup->gkey, sizeof(struct Dot11KeyMappingsEntry));
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		if (!SWCRYPTO && priv->pmib->dot11GroupKeysTable.keyInCam) {
			memcpy(key_combo,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen);

			memcpy(&key_combo[priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen],
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey1.skey,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen);

			CamAddOneEntry(priv, "\xff\xff\xff\xff\xff\xff", priv->pmib->dot11GroupKeysTable.keyid,
				priv->pmib->dot11GroupKeysTable.dot11Privacy<<2, 0, key_combo);
			priv->pshare->CamEntryOccupied++;
		}
	}

#ifdef WDS
	memcpy(&priv->pmib->dot11WdsInfo, &pBackup->wds, sizeof(struct wds_info));
#endif
	kfree(pInfo);
}
#endif // FAST_RECOVERY

#ifdef CONFIG_RTL8190_PRIV_SKB
		#ifdef DELAY_REFILL_RX_BUF
			#if defined(CONFIG_RTL8196B_GW_8M)
//#ifdef CONFIG_MSC
				#define MAX_SKB_NUM		100
//#else
//				#define MAX_SKB_NUM		56
//#endif				
			#elif defined(CONFIG_RTL8672)
				#define MAX_SKB_NUM		256
			#else
				#define MAX_SKB_NUM		160
			#endif
		#else
			#define MAX_SKB_NUM		580
		#endif

#if defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
#ifdef __LINUX_2_6__
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128+128)
#else
#if defined(CONFIG_RTL8198_AP_ROOT)
		#define SKB_BUF_SIZE	(MAX_RX_BUF_LEN+sizeof(struct skb_shared_info)+128)
#else
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128)
#endif
#endif
#else
#ifdef __LINUX_2_6__
		#define SKB_BUF_SIZE	(MAX_RX_BUF_LEN+sizeof(struct skb_shared_info)+128+128)
#else
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128)
#endif
#endif

#define MAGIC_CODE		"8190"

struct priv_skb_buf {
	unsigned char magic[4];
	unsigned int buf_pointer;
	struct list_head	list;
	unsigned char buf[SKB_BUF_SIZE];
};

static struct priv_skb_buf skb_buf[MAX_SKB_NUM];
static struct list_head skbbuf_list;
static struct rtl8192cd_priv *root_priv;

#ifdef CONFIG_WIRELESS_LAN_MODULE
static int skb_free_num = MAX_SKB_NUM;
#else
int skb_free_num = MAX_SKB_NUM;
#endif

extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);


void init_priv_skb_buf(struct rtl8192cd_priv *priv)
{
	int i;

	memset(skb_buf, '\0', sizeof(struct priv_skb_buf)*MAX_SKB_NUM);

	INIT_LIST_HEAD(&skbbuf_list);

	for (i=0; i<MAX_SKB_NUM; i++)  {
		memcpy(skb_buf[i].magic, MAGIC_CODE, 4);
		skb_buf[i].buf_pointer = (unsigned int)&skb_buf[i];				
		INIT_LIST_HEAD(&skb_buf[i].list);
		list_add_tail(&skb_buf[i].list, &skbbuf_list);
	}
	root_priv = priv;
}


static __inline__ unsigned char *get_priv_skb_buf(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	ret = get_buf_from_poll(root_priv, &skbbuf_list, (unsigned int *)&skb_free_num);

	RESTORE_INT(flags);	
	return ret;
}


#ifndef __LINUX_2_6__
__MIPS16
__IRAM_IN_865X
#endif
static struct sk_buff *dev_alloc_skb_priv(struct rtl8192cd_priv *priv, unsigned int size)
{
	struct sk_buff *skb;

	unsigned char *data = get_priv_skb_buf(priv);
	if (data == NULL) {
//		_DEBUG_ERR("wlan: priv skb buffer empty!\n");
		return NULL;
	}

	skb = dev_alloc_8190_skb(data, size);
	if (skb == NULL) {
		free_rtl8190_priv_buf(data);
		return NULL;
	}
	return skb;
}


int is_rtl8190_priv_buf(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf = (struct priv_skb_buf *)(((unsigned long)head) - offset);

	if (!memcmp(priv_buf->magic, MAGIC_CODE, 4) &&
			(priv_buf->buf_pointer == (unsigned int)priv_buf))
		return 1;
	else
		return 0;
}


void free_rtl8190_priv_buf(unsigned char *head)
{

	struct rtl8192cd_priv *priv = root_priv;
	unsigned long x;

#if 0
	if (!is_rtl8190_priv_buf(head)) {
		printk("wlan: free invalid priv skb buf!\n");
		return;
	}
#endif

#ifdef DELAY_REFILL_RX_BUF
	extern int refill_rx_ring(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned char *data);
	int ret;

	SAVE_INT_AND_CLI(x);
	ret = refill_rx_ring(priv, NULL, head);
	if (ret) {
		RESTORE_INT(x);
		return;
	}
	else
		release_buf_to_poll(root_priv, head, &skbbuf_list, (unsigned int *)&skb_free_num);
	RESTORE_INT(x);
#else

	SAVE_INT_AND_CLI(x);
	release_buf_to_poll(root_priv, head, &skbbuf_list, (unsigned int *)&skb_free_num);
	RESTORE_INT(x);	
#endif
}
#endif //CONFIG_RTL8190_PRIV_SKB


void choose_IOT_main_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if ((GET_ROOT(priv)->up_time % 2) == 0) {
		unsigned int tx_2s_avg = 0;
		unsigned int rx_2s_avg = 0;
		unsigned long total_sum = (priv->pshare->current_tx_bytes+priv->pshare->current_rx_bytes);

		pstat->current_tx_bytes += pstat->tx_byte_cnt;
		pstat->current_rx_bytes += pstat->rx_byte_cnt;

		if (total_sum != 0) {
			if (total_sum <= 100) {
				tx_2s_avg = (unsigned int)((pstat->current_tx_bytes*100) / total_sum);
				rx_2s_avg = (unsigned int)((pstat->current_rx_bytes*100) / total_sum);
			} else {
				tx_2s_avg = (unsigned int)(pstat->current_tx_bytes / (total_sum / 100));
				rx_2s_avg = (unsigned int)(pstat->current_rx_bytes / (total_sum / 100));
			}
		}

		if (pstat->ht_cap_len) {
			if ((tx_2s_avg + rx_2s_avg) >= 50) {
				priv->pshare->highTP_found_pstat = pstat;
			}
		}
	}
	else {
		pstat->current_tx_bytes = pstat->tx_byte_cnt;
		pstat->current_rx_bytes = pstat->rx_byte_cnt;
	}
}


void rxBB_dm(struct rtl8192cd_priv *priv)
{
	if ((priv->up_time % 3) == 1) {
		if (priv->pshare->rssi_min != 0xff) {
			if (priv->pshare->rf_ft_var.dig_enable) {
				// for DIG checking
				check_DIG_by_rssi(priv, priv->pshare->rssi_min);
			}
		}

		check_EDCCA(priv, priv->pshare->rssi_min);

#ifdef MP_TEST
		if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
		{
			if (!priv->pshare->rf_ft_var.use_ext_lna)
				CCK_CCA_dynamic_enhance(priv, priv->pshare->rssi_min);
		}
	}
}

/*
 * IOT related functions
 */
void IOT_engine(struct rtl8192cd_priv *priv)
{
#ifdef WIFI_WMM
	unsigned int switch_turbo = 0;
#endif
	struct stat_info *pstat = priv->pshare->highTP_found_pstat;

#if defined(RTL_MANUAL_EDCA) && defined(WIFI_WMM)
	if(priv->pmib->dot11QosEntry.ManualEDCA)
		return ;
#endif



#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if (!priv->pmib->dot11OperationEntry.wifi_specific || (priv->pmib->dot11OperationEntry.wifi_specific == 2)
			) {
			if (priv->pshare->iot_mode_enable &&
				((priv->pshare->phw->VO_pkt_count > 50) ||
				 (priv->pshare->phw->VI_pkt_count > 50) ||
				 (priv->pshare->phw->BK_pkt_count > 50))) {
				priv->pshare->iot_mode_enable = 0;
				switch_turbo++;
			} else if ((!priv->pshare->iot_mode_enable) &&
				((priv->pshare->phw->VO_pkt_count < 50) &&
				 (priv->pshare->phw->VI_pkt_count < 50) &&
				 (priv->pshare->phw->BK_pkt_count < 50))) {
				priv->pshare->iot_mode_enable++;
				switch_turbo++;
			}
		}

		if (priv->pmib->dot11OperationEntry.wifi_specific) {
			if (!priv->pshare->iot_mode_VO_exist && (priv->pshare->phw->VO_pkt_count > 50)) {
				priv->pshare->iot_mode_VO_exist++;
				switch_turbo++;
			} else if (priv->pshare->iot_mode_VO_exist && (priv->pshare->phw->VO_pkt_count < 50)) {
				priv->pshare->iot_mode_VO_exist = 0;
				switch_turbo++;
			}
		}

		priv->pshare->phw->VO_pkt_count = 0;
		priv->pshare->phw->VI_pkt_count = 0;
		priv->pshare->phw->BK_pkt_count = 0;
	}
#endif

	if ((priv->up_time % 2) == 0) {
		/*
		 * decide EDCA content for different chip vendor
		 */
#ifdef WIFI_WMM
		if ((!priv->pmib->dot11OperationEntry.wifi_specific || (priv->pmib->dot11OperationEntry.wifi_specific == 2))
			&& QOS_ENABLE) {
			if (pstat && pstat->rssi >= priv->pshare->rf_ft_var.txop_enlarge_upper) {
				if (priv->pshare->txop_enlarge != 2) {
					if (pstat->is_intel_sta)
						priv->pshare->txop_enlarge = 0xe;
					else
						priv->pshare->txop_enlarge = 2;

					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
			} else if (!pstat || pstat->rssi < priv->pshare->rf_ft_var.txop_enlarge_lower) {
				if (priv->pshare->txop_enlarge) {
					priv->pshare->txop_enlarge = 0;
					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
			}
		}
#endif

		priv->pshare->current_tx_bytes = 0;
		priv->pshare->current_rx_bytes = 0;
	}

#ifdef WIFI_WMM
	if (switch_turbo)
		IOT_EDCA_switch(priv, priv->pmib->dot11BssType.net_work_type, priv->pshare->iot_mode_enable);
#endif
}



/*
unsigned int set_fw_reg(struct rtl8192cd_priv *priv, unsigned int cmd, unsigned int val, unsigned int with_val)
{
	static unsigned int delay_count;

	delay_count = 10;

	do {
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);
	delay_count = 10;

	if (with_val == 1)
		RTL_W32(0x2c4, val);

	RTL_W32(0x2c0, cmd);

	do {
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);

	return 0;
}


void set_fw_A2_entry(struct rtl8192cd_priv *priv, unsigned int cmd, unsigned char *addr)
{
	unsigned int delay_count = 10;

	do{
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);
	delay_count = 10;

	RTL_W32(0x2c4, addr[3]<<24 | addr[2]<<16 | addr[1]<<8 | addr[0]);
	RTL_W32(0x2c8, addr[5]<<8 | addr[4]);
	RTL_W32(0x2c0, cmd);

	do{
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);
}
*/

#if defined(TXREPORT) || defined(SW_ANT_SWITCH)

struct stat_info* findNextSTA(struct rtl8192cd_priv *priv, int *idx)
{
	int i;
	for(i= *idx; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used == TRUE) {
			*idx = (i+1);
#ifdef STA_EXT
			if(priv->pshare->aidarray[i]->station.remapped_aid == FW_NUM_STAT-1)
#else
			if(priv->pshare->aidarray[i]->station.aid == FW_NUM_STAT)
#endif
				continue;

			return &(priv->pshare->aidarray[i]->station);
		}
	}
	return NULL;
}
#endif
#ifdef TXREPORT
void requestTxReport(struct rtl8192cd_priv *priv)
{
	int h2ccmd, counter=20;
	struct stat_info *sta;

	if( priv->pshare->sta_query_idx == -1)
		return;
	
	while(is_h2c_buf_occupy(priv)) {
		delay_ms(2);
		if(--counter==0)
			break;
	}
	if(!counter) 
		return;

	h2ccmd= AP_REQ_RPT;
	
	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if(sta)
		h2ccmd |= (REMAP_AID(sta)<<24);
	else {
		priv->pshare->sta_query_idx = -1;
		return;		
	}
	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if(sta)	{	
		h2ccmd |= (REMAP_AID(sta)<<16);
	} else {
		priv->pshare->sta_query_idx = -1;	
	}

	signin_h2c_cmd(priv, h2ccmd , 0);
	DEBUG_INFO("signin h2c:%x\n", h2ccmd);

}


void txrpt_handler(struct rtl8192cd_priv *priv, struct tx_rpt *report)
{
	struct stat_info	*pstat;	
#ifdef MBSSID
	int i;
#endif
	pstat = get_macidinfo(priv, report->macid);
	if(pstat) {
		priv->net_stats.tx_errors += report->txfail;
		pstat->tx_fail += report->txfail;
		pstat->tx_pkts += report->txok+report->txfail;

		DEBUG_INFO("debug[%02X%02X%02X%02X%02X%02X]:id=%d,ok=%d,fail=%d\n", 
		pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],
		report->macid, report->txok, report->txfail);
#ifdef DETECT_STA_EXISTANCE
		// Check for STA existance; added by Annie, 2010-08-10.
		DetectSTAExistance(priv, report, pstat);
#endif		
	}

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))	{
		pstat = get_macidinfo(GET_VXD_PRIV(priv), report->macid);
		if(pstat) {
			GET_VXD_PRIV(priv)->net_stats.tx_errors += report->txfail;
			pstat->tx_fail += report->txfail;
			pstat->tx_pkts += report->txok+ report->txfail;						
#ifdef DETECT_STA_EXISTANCE
			// Check for STA existance; added by Annie, 2010-08-10.
			DetectSTAExistance(GET_VXD_PRIV(priv), report, pstat);
#endif			
		}					
	}
#endif
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) 		{
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i])) {
				pstat = get_macidinfo(priv->pvap_priv[i], report->macid);
				if(pstat) {
					priv->pvap_priv[i]->net_stats.tx_errors += report->txfail;
					pstat->tx_fail += report->txfail;
					pstat->tx_pkts += report->txok + report->txfail;
#ifdef DETECT_STA_EXISTANCE
					// Check for STA existance; added by Annie, 2010-08-10.
					DetectSTAExistance(priv->pvap_priv[i], report, pstat);
#endif
				}
			}
		}
	}
#endif

}

void C2H_isr(struct rtl8192cd_priv *priv)
{
	struct tx_rpt rpt1;
	int j, tmp32, idx=0x1a2;
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);
	tmp32 = RTL_R16(0x1a0);
	if( (tmp32&0xff)==0xc2 ) {
		for(j=0; j<2; j++) {
			rpt1.macid= (0x1f) & RTL_R8(idx+4);
			if(rpt1.macid) {
				rpt1.txok = le16_to_cpu(RTL_R16(idx+2));
				rpt1.txfail = le16_to_cpu(RTL_R16(idx));
				txrpt_handler(priv, &rpt1);
			}			
			idx+=6;		
		}
	}
	RTL_W8( 0x1af, 0);
	requestTxReport(priv);
	RESTORE_INT(flags);
}

#ifdef DETECT_STA_EXISTANCE
// Check for STA existance. If STA disappears, disconnect it. Added by Annie, 2010-08-10.
void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];

	// Parameters
	const unsigned int		maxTxFailCnt = 300;		// MAX Tx fail packet count
	const unsigned int		minTxFailCnt = 30;		// MIN Tx fail packet count; this value should be less than maxTxFailCnt.
	const unsigned int		txFailSecThr= 3;			// threshold of Tx Fail Time (in second)

	// Temporarily change Retry Limit when TxFail. (tfrl: TxFailRetryLimit)
	const unsigned char	TFRL = 7;				// New Retry Limit value
	const unsigned char	TFRL_FailCnt = 2;		// Tx Fail Count threshold to set Retry Limit
	const unsigned char	TFRL_SetTime = 2;		// Time to set Retry Limit (in second)
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)

	if( report->txok != 0 )
	{ // Reset Counter
		pstat->tx_conti_fail_cnt = 0;
		pstat->tx_last_good_time = priv->up_time;
		pstat->leave = 0;
	}
	else if( report->txfail != 0 )
	{
		pstat->tx_conti_fail_cnt += report->txfail;
		DEBUG_WARN( "detect: txfail=%d, tx_conti_fail_cnt=%d\n", report->txfail, pstat->tx_conti_fail_cnt );

		if(	priv->up_time >= (pstat->tx_last_good_time+TFRL_SetTime) &&
			pstat->tx_conti_fail_cnt >= TFRL_FailCnt && 
			//!pstat->ht_cap_len && // legacy rate only
			!priv->pshare->bRLShortened )
		{ // Shorten retry limit, because AP spending too much time to send out g mode STA pending packets in HW queue.
			RTL_W16(RL, (TFRL&SRL_Mask)<<SRL_SHIFT|(TFRL&LRL_Mask)<<LRL_SHIFT);
			priv->pshare->bRLShortened = TRUE;
			DEBUG_WARN( "== Shorten RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
		}

		if( 	(pstat->tx_conti_fail_cnt >= maxTxFailCnt) ||
			(pstat->tx_conti_fail_cnt >= minTxFailCnt && priv->up_time >= (pstat->tx_last_good_time+txFailSecThr) )
			)
		{ // This STA is considered as disappeared, so delete it.
			DEBUG_WARN( "** tx_conti_fail_cnt=%d (min=%d,max=%d)\n", pstat->tx_conti_fail_cnt, minTxFailCnt, maxTxFailCnt);
			DEBUG_WARN( "** tx_last_good_time=%d, up_time=%d (Thr:%d)\n", (int)pstat->tx_last_good_time, (int)priv->up_time, txFailSecThr );
			DEBUG_WARN( "AP is going to del_sta %02X:%02X:%02X:%02X:%02X:%02X\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5] );

			sprintf(tmpbuf, "%02x%02x%02x%02x%02x%02x", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		
//			del_sta(priv, tmpbuf);
			++(pstat->leave);

			if (timer_pending(&priv->pshare->rl_recover_timer))
				del_timer_sync (&priv->pshare->rl_recover_timer);
			mod_timer(&priv->pshare->rl_recover_timer, jiffies + EXPIRE_TO*TFRL_RcvTime);

			// Reset Counter
			pstat->tx_conti_fail_cnt = 0;
			pstat->tx_last_good_time = priv->up_time;
		}
	}
}

// Timer callback function to recover hardware retry limit register. Added by Annie, 2010-08-10.
void RetryLimitRecovery(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	if( priv->pshare->bRLShortened )
	{
		RTL_W16(RL, (priv->pshare->RLShort&SRL_Mask)<<SRL_SHIFT|(priv->pshare->RLLong&LRL_Mask)<<LRL_SHIFT);
		priv->pshare->bRLShortened = FALSE;
		DEBUG_WARN( "== Recover RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
	}
}

// Chack STA leaving status; per interface. Added by Annie, 2010-08-10.
unsigned char NoLeavingSTA(struct rtl8192cd_priv *priv)
{
	unsigned char bStaAllOK = TRUE;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	phead = &priv->asoc_list;
	if (!netif_running(priv->dev) || list_empty(phead))
		return bStaAllOK;

	plist = phead->next;
	while (plist != phead)  {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if( pstat->tx_conti_fail_cnt != 0 ) {
			bStaAllOK = FALSE;
			break;
		}
		plist = plist->next;
	}

	return bStaAllOK;
}

// Chack STA leaving status for all active interface and recover retry limit register value. Added by Annie, 2010-08-10.
void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv)
{
	unsigned char bIfAllOK = TRUE;
	static int AllOKTimes = 0;
	int i;

	// Parameter
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)

	if( !NoLeavingSTA(priv) )
		bIfAllOK = FALSE;

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) && GET_VXD_PRIV(priv) ) {
		if( !NoLeavingSTA(GET_VXD_PRIV(priv)) )
			bIfAllOK = FALSE;
	}
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i])) {
					if( !NoLeavingSTA(priv->pvap_priv[i]) )
						bIfAllOK = FALSE;
				}
			}
		}
	}
#endif

	if( bIfAllOK ) {
		AllOKTimes ++;

		if( AllOKTimes >= TFRL_RcvTime )
			RetryLimitRecovery((unsigned long)priv);
	}
	else {
		AllOKTimes = 0;
	}
}
#endif
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}


int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) _atoi(tmpBuf,16);
	}
	return 1;
}
#endif
