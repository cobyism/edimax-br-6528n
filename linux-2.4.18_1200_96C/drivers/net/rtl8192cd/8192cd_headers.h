/*
 *  Headler file defines global functions PROTO types
 *
 *  $Id: 8192cd_headers.h,v 1.32.2.26 2011/04/29 09:01:39 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*-----------------------------------------------------------------------------
				This file define the header files
------------------------------------------------------------------------------*/

#ifndef _8192CD_HEADERS_H_
#define _8192CD_HEADERS_H_

#ifdef __KERNEL__
#include <linux/wireless.h>
#endif

#include "./8192cd_tx.h"


/*-----------------------------------------------------------------------------
								8192cd_util.c
------------------------------------------------------------------------------*/
#ifndef	_8192CD_UTILS_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN int enque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned int ffptr, int ffsize, void *elm);
EXTERN unsigned int *deque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned int ffptr, int ffsize);
EXTERN void initque(struct rtl8192cd_priv *priv, int *head, int *tail);
EXTERN int isFFempty(int head, int tail);
EXTERN unsigned int find_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, int mode, int isBasicRate);
EXTERN unsigned int find_rate_MP(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct ht_cap_elmt * peer_ht_cap, int peer_ht_cap_len, char * peer_rate, int peer_rate_len, int mode, int isBasicRate);
EXTERN void init_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void release_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN struct stat_info *alloc_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr, int id);
EXTERN void free_sta_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN int free_stainfo(struct rtl8192cd_priv *priv, struct	stat_info *pstat);
EXTERN struct stat_info *get_stainfo (struct rtl8192cd_priv *priv, unsigned char *hwaddr);
EXTERN struct stat_info *get_aidinfo (struct rtl8192cd_priv *priv, unsigned int aid);
EXTERN int IS_BSSID(struct rtl8192cd_priv *priv, unsigned char *da);
EXTERN int IS_MCAST(unsigned char *da);
EXTERN int p80211_stt_findproto(UINT16 proto);
EXTERN int skb_ether_to_p80211(struct sk_buff *skb, UINT32 ethconv);
EXTERN int skb_p80211_to_ether(struct net_device *dev, int wep_mode, struct rx_frinfo *pfrinfo);
EXTERN int strip_amsdu_llc(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat);
EXTERN unsigned char *get_da(unsigned char *pframe);
EXTERN unsigned char *get_bssid_mp(unsigned char *pframe);
EXTERN unsigned char get_hdrlen(struct rtl8192cd_priv *priv, UINT8 *pframe);
EXTERN unsigned int get_mcast_privacy(struct rtl8192cd_priv *priv, unsigned int *iv, unsigned int *icv,
				unsigned int *mic);
EXTERN unsigned int	get_privacy(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				unsigned int *iv, unsigned int *icv, unsigned int *mic);
EXTERN unsigned char *get_mgtbuf_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_mgtbuf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_wlanhdr_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_wlanhdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_wlanllchdr_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_wlanllchdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_icv_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_icv_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned char *get_mic_from_poll(struct rtl8192cd_priv *priv);
EXTERN void release_mic_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf);
EXTERN unsigned short get_pnl(union PN48 *ptsc);
EXTERN unsigned int get_pnh(union PN48 *ptsc);
EXTERN void eth_2_llc(struct wlan_ethhdr_t *pethhdr, struct llc_snap *pllc_snap);
EXTERN void eth2_2_wlanhdr(struct rtl8192cd_priv *priv, struct wlan_ethhdr_t *pethhdr, struct tx_insn *txcfg);
EXTERN unsigned char *get_sa(unsigned char *pframe);
EXTERN unsigned int get_mcast_encrypt_algthm(struct rtl8192cd_priv *priv);
EXTERN unsigned int get_sta_encrypt_algthm(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN UINT8 get_rate_from_bit_value(int bit_val);
EXTERN int get_rate_index_from_ieee_value(UINT8 val);
EXTERN int get_bit_value_from_ieee_value(UINT8 val);
EXTERN int UseSwCrypto(struct rtl8192cd_priv *priv, struct stat_info *pstat, int isMulticast);

#ifdef WDS
EXTERN int getWdsIdxByDev(struct rtl8192cd_priv *priv, struct net_device *dev);
EXTERN struct net_device *getWdsDevByAddr(struct rtl8192cd_priv *priv, unsigned char *addr);
#endif

EXTERN void check_protection_shortslot(struct rtl8192cd_priv *priv);
EXTERN void check_sta_characteristic(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act);
EXTERN int should_forbid_Nmode(struct rtl8192cd_priv *priv);
EXTERN int should_restrict_Nrate(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void validate_oper_rate(struct rtl8192cd_priv *priv);
EXTERN void get_oper_rate(struct rtl8192cd_priv *priv);
EXTERN int get_bssrate_set(struct rtl8192cd_priv *priv, int bssrate_ie, unsigned char **pbssrate, int *bssrate_len);
EXTERN int get_available_channel(struct rtl8192cd_priv *priv);
EXTERN void cnt_assoc_num(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act, char *func);
EXTERN int get_assoc_sta_num(struct rtl8192cd_priv *priv);
EXTERN void event_indicate(struct rtl8192cd_priv *priv, unsigned char *mac, int event);
#ifdef WIFI_HAPD
EXTERN int event_indicate_hapd(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra);
#endif
#ifdef USE_WEP_DEFAULT_KEY
EXTERN void init_DefaultKey_Enc(struct rtl8192cd_priv *priv, unsigned char *key, int algorithm);
#endif

#ifdef UNIVERSAL_REPEATER
EXTERN void disable_vxd_ap(struct rtl8192cd_priv *priv);
EXTERN void enable_vxd_ap(struct rtl8192cd_priv *priv);
#endif

#ifdef GBWC
EXTERN void rtl8192cd_GBWC_timer(unsigned long task_priv);
#endif
EXTERN void add_update_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat);

#ifdef STA_EXT
EXTERN void release_remapAid(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif

EXTERN void add_update_rssi(struct rtl8192cd_priv * priv, struct stat_info * pstat);

// to avoid add RAtid fail
EXTERN void add_RATid_timer(unsigned long task_priv);
EXTERN void add_rssi_timer(unsigned long task_priv);
EXTERN unsigned int is_h2c_buf_occupy(struct rtl8192cd_priv *priv);
EXTERN short signin_h2c_cmd(struct rtl8192cd_priv *priv, unsigned int content, unsigned short ext_content);
EXTERN void add_ps_timer(unsigned long task_priv);
EXTERN void add_update_ps(struct rtl8192cd_priv *priv, struct stat_info *pstat);

EXTERN void refill_skb_queue(struct rtl8192cd_priv *priv);
EXTERN void free_skb_queue(struct rtl8192cd_priv *priv, struct sk_buff_head *skb_que);

#ifdef FAST_RECOVERY
EXTERN void *backup_sta(struct rtl8192cd_priv *priv);
EXTERN void restore_backup_sta(struct rtl8192cd_priv *priv, void *pInfo);
#endif

#ifdef RTK_QUE
EXTERN void rtk_queue_init(struct ring_que *que);
EXTERN void free_rtk_queue(struct rtl8192cd_priv *priv, struct ring_que *skb_que);
#endif


#ifdef CONFIG_RTL8190_PRIV_SKB
EXTERN void init_priv_skb_buf(struct rtl8192cd_priv *priv);
EXTERN int is_rtl8190_priv_buf(unsigned char *head);
EXTERN void free_rtl8190_priv_buf(unsigned char *head);
#endif

EXTERN void choose_IOT_main_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void rxBB_dm(struct rtl8192cd_priv *priv);
EXTERN void IOT_engine(struct rtl8192cd_priv *priv);

#ifdef STA_EXT
EXTERN unsigned char fw_was_full(struct rtl8192cd_priv *priv);
EXTERN int realloc_RATid(struct rtl8192cd_priv * priv);
#endif

// global variables
EXTERN UINT8 Realtek_OUI[];
EXTERN UINT8 dot11_rate_table[];

/*
EXTERN unsigned int set_fw_reg(struct rtl8192cd_priv *priv, unsigned int cmd, unsigned int val, unsigned int with_val);
EXTERN void set_fw_A2_entry(struct rtl8192cd_priv * priv, unsigned int cmd, unsigned char * addr);
*/
EXTERN void set_RATid_cmd(struct rtl8192cd_priv * priv, unsigned int macid, unsigned int rateid, unsigned int ratemask);

#ifdef CONFIG_RTK_MESH
#ifdef _11s_TEST_MODE_
EXTERN __inline__ unsigned char *get_buf_from_poll(struct rtl8192cd_priv *priv, struct list_head *phead, unsigned int *count);
EXTERN __inline__ void release_buf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf, struct list_head	*phead, unsigned int *count);
#endif

EXTERN UINT8 oui_rfc1042[];
EXTERN UINT8 oui_8021h[];
EXTERN unsigned char SNAP_ETH_TYPE_IPX[];
EXTERN unsigned char SNAP_ETH_TYPE_APPLETALK_AARP[];
EXTERN unsigned char SNAP_ETH_TYPE_APPLETALK_DDP[];
EXTERN unsigned char SNAP_HDR_APPLETALK_DDP[];

#define WLAN_PKT_FORMAT_SNAP_RFC1042    0x02
#define WLAN_PKT_FORMAT_SNAP_TUNNEL             0x03
#define WLAN_PKT_FORMAT_IPX_TYPE4               0x04
#define WLAN_PKT_FORMAT_APPLETALK               0x05
#define WLAN_PKT_FORMAT_OTHERS                  0x07
#endif

#ifdef PRIV_STA_BUF
EXTERN void init_priv_sta_buf(struct rtl8192cd_priv *priv);
EXTERN struct aid_obj *alloc_sta_obj(struct rtl8192cd_priv*);
EXTERN void free_sta_obj(struct rtl8192cd_priv *priv, struct aid_obj *obj);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
EXTERN void free_wpa_buf(struct rtl8192cd_priv *priv, WPA_STA_INFO *buf);
#endif
#if defined(WIFI_WMM) && defined(WMM_APSD)
EXTERN void free_sta_que(struct rtl8192cd_priv *priv, struct apsd_pkt_queue *que);
#endif
#if defined(WIFI_WMM)
EXTERN void free_sta_mgt_que(struct rtl8192cd_priv *priv, struct dz_mgmt_queue *que);
#endif
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
EXTERN int string_to_hex(char *string, unsigned char *key, int len);
#endif

#ifdef TXREPORT
EXTERN void requestTxReport(struct rtl8192cd_priv *priv);
EXTERN void C2H_isr(struct rtl8192cd_priv *priv);
EXTERN void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat );
EXTERN void RetryLimitRecovery(unsigned long task_priv);
EXTERN void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv);
#endif

#ifdef SW_ANT_SWITCH
EXTERN struct stat_info* findNextSTA(struct rtl8192cd_priv *priv, int *idx);
#endif


#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_tx.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_TX_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN void assign_wlanseq(struct rtl8192cd_hw *phw, unsigned char *pframe, struct stat_info *pstat, struct wifi_mib *pmib
#ifdef CONFIG_RTK_MESH	// For message if broadcast data frame via mesh (ex:ARP requst)
	, unsigned char is_11s
#endif
	);
#if 0
EXTERN void signin_txdesc(struct rtl8190_priv *priv, struct tx_insn* txcfg);
#endif
EXTERN int rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev);
EXTERN int rtl8192cd_wlantx(struct rtl8192cd_priv *priv, struct tx_insn *txcfg);
EXTERN void rtl8192cd_tx_dsr(unsigned long task_priv);
EXTERN int rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn *txcfg);
EXTERN int rtl8192cd_signin_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);


#ifdef TX_SHORTCUT
//EXTERN void signin_txdesc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg);
EXTERN int rtl8192cd_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx);
#endif
EXTERN int SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content,
				unsigned short txLength, unsigned char LastPkt);
EXTERN void amsdu_timeout(struct rtl8192cd_priv *priv, unsigned int current_time);

#ifdef FW_SW_BEACON
EXTERN int rtl8192cd_SendBeaconByCmdQ(struct rtl8192cd_priv *priv, unsigned char *dat_content, unsigned short txLength);
#endif

EXTERN int rtl8192cd_SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content, unsigned short txLength, unsigned char LastPkt);

#ifdef CONFIG_RTK_MESH
//#ifdef MESH_AMSDU	//plus mark it for build error
EXTERN int amsdu_check(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat, struct tx_insn *txcfg);
//#endif
EXTERN unsigned int get_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN unsigned int get_lowest_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat,	unsigned int tx_rate);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_sme.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_SME_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN void	rtl8192cd_frag_timer(unsigned long task_priv);
EXTERN void rtl8192cd_expire_timer(unsigned long task_priv);
EXTERN void rtl8192cd_1sec_timer(unsigned long task_priv);
EXTERN void mgt_handler(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void pwr_state(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void update_beacon(struct rtl8192cd_priv *priv);
EXTERN void init_beacon(struct rtl8192cd_priv *priv);
EXTERN void issue_deauth(struct rtl8192cd_priv *priv,	unsigned char *da, int reason);
EXTERN void start_clnt_ss(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_ss_timer(unsigned long task_priv);
EXTERN void process_dzqueue(struct rtl8192cd_priv *priv);
EXTERN void issue_asocrsp(struct rtl8192cd_priv *priv,	unsigned short status, struct stat_info *pstat, int pkt_type);
EXTERN void issue_disassoc(struct rtl8192cd_priv *priv, unsigned char *da, int reason);
EXTERN void assign_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);
EXTERN void assign_aggre_mthod(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void assign_aggre_size(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN int isErpSta(struct stat_info *pstat);
EXTERN void enable_sw_LED(struct rtl8192cd_priv *priv, int init);
EXTERN void disable_sw_LED(struct rtl8192cd_priv *priv);
EXTERN void calculate_sw_LED_interval(struct rtl8192cd_priv *priv);
EXTERN int chklink_wkstaQ(struct rtl8192cd_priv *priv);

#ifdef CLIENT_MODE
EXTERN void start_clnt_join(struct rtl8192cd_priv *priv);
EXTERN void start_clnt_lookup(struct rtl8192cd_priv *priv, int rescan);
EXTERN void rtl8192cd_reauth_timer(unsigned long task_priv);
EXTERN void rtl8192cd_reassoc_timer(unsigned long task_priv);
EXTERN void rtl8192cd_idle_timer(unsigned long task_priv);
#endif

#ifdef DFS
EXTERN void rtl8192cd_DFS_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch_avail_chk_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch52_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch56_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch60_timer(unsigned long task_priv);
EXTERN void rtl8192cd_ch64_timer(unsigned long task_priv);
EXTERN unsigned int DFS_SelectChannel(void);
EXTERN int RemoveChannel(unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel);
EXTERN int InsertChannel(unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel);
EXTERN void DetermineDFSDisable(struct rtl8192cd_priv *priv);
#endif

#ifdef WIFI_WMM
EXTERN void init_WMM_Para_Element(struct rtl8192cd_priv *priv, unsigned char *temp);
EXTERN void issue_ADDBAreq(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char TID);
#endif

EXTERN void process_mcast_dzqueue(struct rtl8192cd_priv *priv);
EXTERN void construct_ht_ie(struct rtl8192cd_priv *priv, int use_40m, int offset);

#ifdef CHECK_HANGUP
EXTERN int check_hangup(struct rtl8192cd_priv *priv);
#endif

#ifndef USE_WEP_DEFAULT_KEY
EXTERN void set_keymapping_wep(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
EXTERN void register_usb_pkt_cnt_f(void *usbPktFunc);
EXTERN void usbPkt_timer_handler(struct rtl8192cd_priv *priv);
#endif

#ifdef RTK_WOW
EXTERN void issue_rtk_wow(struct rtl8192cd_priv * priv, unsigned char * da);
#endif

EXTERN void control_wireless_led(struct rtl8192cd_priv *priv, int enable);

#ifdef CONFIG_RTK_MESH
EXTERN unsigned char WMM_IE[];
EXTERN unsigned char WMM_PARA_IE[];
EXTERN int check_basic_rate(struct rtl8192cd_priv *priv, unsigned char *pRate, int pLen);
EXTERN int collect_bss_info(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned char *construct_ht_ie_old_form(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen);
EXTERN void get_matched_rate(struct rtl8192cd_priv *priv, unsigned char *pRate, int *pLen, int which);
EXTERN unsigned char *get_ie(unsigned char *pbuf, int index, int *len, int limit);
EXTERN void issue_auth(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned short status);
EXTERN void issue_deauth_MP(struct rtl8192cd_priv *priv,	unsigned char *da, int reason, UINT8 is_11s);
EXTERN void issue_probersp_MP(struct rtl8192cd_priv *priv, unsigned char *da, UINT8 *ssid, int ssid_len, int set_privacy, UINT8 is_11s);
EXTERN unsigned char *set_fixed_ie(unsigned char *pbuf, unsigned int len, unsigned char *source, unsigned int *frlen);
EXTERN unsigned char *set_ie(unsigned char *pbuf, int index, unsigned int len, unsigned char *source, unsigned int *frlen);
EXTERN void update_support_rate(struct	stat_info *pstat, unsigned char* buf, int len);
EXTERN void proxy_table_chkcln(struct rtl8192cd_priv* priv, struct sk_buff *pskb);
EXTERN int fire_data_frame(struct sk_buff *skb, struct net_device *dev, struct tx_insn* txinsn);

#endif
EXTERN void reset_1r_sta_RA(struct rtl8192cd_priv *priv, unsigned int sg_rate);

EXTERN void default_WMM_para(struct rtl8192cd_priv *priv);

#undef EXTERN



/*-----------------------------------------------------------------------------
								8192cd_rx.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_RX_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

#ifdef RX_TASKLET
EXTERN void rtl8192cd_rx_tkl_isr(unsigned long task_priv);
#endif
EXTERN void rtl8192cd_rx_isr(struct rtl8192cd_priv *priv);
EXTERN void rtl8192cd_rx_dsr(unsigned long task_priv);
EXTERN void rtl_netif_rx(struct rtl8192cd_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);
//EXTERN void rtl8190_rxcmd_isr(struct rtl8190_priv *priv);
EXTERN void reorder_ctrl_timeout(unsigned long task_priv);

#ifdef CONFIG_RTK_MESH
EXTERN void rtl8192cd_rx_mgntframe(struct rtl8192cd_priv*, struct list_head *,struct rx_frinfo*);
EXTERN void rtl8192cd_rx_dataframe(struct rtl8192cd_priv*, struct list_head *,struct rx_frinfo*);
#endif

#ifdef PREVENT_BROADCAST_STORM
EXTERN unsigned int get_free_memory(void);
#endif

#ifdef RX_BUFFER_GATHER
EXTERN void flush_rx_list(struct rtl8192cd_priv *priv);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_hw.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_HW_C_
#define EXTERN	extern
#else
#define EXTERN
#endif

EXTERN void check_chipID_MIMO(struct rtl8192cd_priv *priv);
#ifdef EN_EFUSE
EXTERN int efuse_get(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int efuse_set(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int efuse_sync(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void ReadTxPowerInfoFromHWPG(struct rtl8192cd_priv *priv);
EXTERN void ReadThermalMeterFromEfuse(struct rtl8192cd_priv *priv);
EXTERN int ReadAdapterInfo8192CE(struct rtl8192cd_priv *priv);
#endif

#ifdef PCIE_POWER_SAVING
EXTERN void PCIE_reset_procedure3(void);
EXTERN void PCIe_power_save_timer(unsigned long task_priv);
EXTERN void PCIe_power_save_tasklet(unsigned long task_priv);
EXTERN void init_pcie_power_saving(struct rtl8192cd_priv *priv);
EXTERN void switch_to_1x1(struct rtl8192cd_priv *priv, int mode) ;
EXTERN void PCIeWakeUp(struct rtl8192cd_priv *priv, unsigned int expTime);
EXTERN void radio_off(struct rtl8192cd_priv *priv);
#ifdef GPIO_WAKEPIN
EXTERN int request_irq_for_wakeup_pin(struct net_device *dev);
#endif
#endif

EXTERN void set_slot_time(struct rtl8192cd_priv *priv, int use_short);
EXTERN void SetTxPowerLevel(struct rtl8192cd_priv *priv);
EXTERN void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset);
EXTERN void enable_hw_LED(struct rtl8192cd_priv *priv, unsigned int led_type);
EXTERN void init_EDCA_para(struct rtl8192cd_priv *priv, int mode);
EXTERN unsigned int PHY_QueryRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid);
EXTERN unsigned int PHY_QueryBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask);
EXTERN void PHY_SetBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data);
EXTERN void PHY_SetRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data);
EXTERN int phy_RF8256_Config_ParaFile(struct rtl8192cd_priv *priv);
//EXTERN int PHY_ConfigMACWithParaFile(struct rtl8192cd_priv *priv);
EXTERN void SwBWMode(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset);
EXTERN void check_EDCCA(struct rtl8192cd_priv * priv, short rssi);
EXTERN void setup_timer1(struct rtl8192cd_priv *priv, int timeout);
EXTERN void cancel_timer1(struct rtl8192cd_priv *priv);
EXTERN void setup_timer2(struct rtl8192cd_priv *priv, unsigned int timeout);
EXTERN void cancel_timer2(struct rtl8192cd_priv *priv);
EXTERN void set_DIG_state(struct rtl8192cd_priv *priv, int state);
EXTERN void DIG_process(struct rtl8192cd_priv *priv);
EXTERN void check_DIG_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength);
EXTERN void DIG_for_site_survey(struct rtl8192cd_priv *priv, int do_ss);
EXTERN void CCK_txpower_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength);
EXTERN int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv);
EXTERN int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv);

#ifdef WIFI_WMM
EXTERN void IOT_EDCA_switch(struct rtl8192cd_priv *priv, int mode, char enable);
#endif
//EXTERN void tx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char enable);
//EXTERN void rx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable);
EXTERN void rx_path_by_rssi_cck_v2(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#ifdef HIGH_POWER_EXT_PA
EXTERN void tx_power_control(struct rtl8192cd_priv *priv);
#endif
EXTERN void tx_power_tracking(struct rtl8192cd_priv *priv);
//EXTERN void rtl8192cd_tpt_timer(unsigned long task_priv);

#ifdef SW_ANT_SWITCH
EXTERN void dm_SW_AntennaSwitchCallback(unsigned long task_priv) ;
EXTERN void dm_SWAW_RSSI_Check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void dm_SW_AntennaSwitchInit(struct rtl8192cd_priv *priv);
EXTERN void dm_SW_AntennaSwitch(struct rtl8192cd_priv *priv, char Step);
#endif

#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)
EXTERN int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data);
#endif
#if defined(HW_ANT_SWITCH)
EXTERN void dm_HW_AntennaSwitchInit(struct rtl8192cd_priv *priv);
EXTERN void setRxIdleAnt(struct rtl8192cd_priv *priv, char nextAnt) ;
EXTERN void dm_STA_Ant_Select(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void dm_HW_IdleAntennaSelect(struct rtl8192cd_priv *priv);

#endif

//EXTERN void SwitchExtAnt(struct rtl8192cd_priv *priv, unsigned char EXT_ANT_PATH);
//EXTERN void rtl8192cd_ePhyInit(struct rtl8192cd_priv * priv);
//EXTERN void Switch_1SS_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath );
//EXTERN void Switch_OFDM_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath );
EXTERN void FA_statistic(struct rtl8192cd_priv * priv);
#ifdef CONFIG_RTL_NEW_AUTOCH
EXTERN void _FA_statistic(struct rtl8192cd_priv* priv);
EXTERN void reset_FA_reg(struct rtl8192cd_priv* priv);
EXTERN void hold_CCA_FA_counter(struct rtl8192cd_priv* priv);
EXTERN void release_CCA_FA_counter(struct rtl8192cd_priv* priv);
#endif

EXTERN void CCK_CCA_dynamic_enhance(struct rtl8192cd_priv * priv, unsigned char rssi_strength);
EXTERN void check_NAV_prot_len(struct rtl8192cd_priv * priv, struct stat_info * pstat, unsigned int disassoc);

// CAM related functions
EXTERN int CamAddOneEntry(struct rtl8192cd_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned long ulEncAlg, unsigned long ulUseDK,
				unsigned char *pucKey);
EXTERN int CamDeleteOneEntry(struct rtl8192cd_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned int useDK);
EXTERN void CamResetAllEntry(struct rtl8192cd_priv *priv);
EXTERN void CamDumpAll(struct rtl8192cd_priv *priv);
EXTERN void CAM_read_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char *macad,
				unsigned char *key128, unsigned short *config);
EXTERN unsigned char *get_line(unsigned char **line);

#ifdef MERGE_FW

#ifdef TESTCHIP_SUPPORT
EXTERN unsigned char *data_AGC_TAB_start, *data_AGC_TAB_end;
EXTERN unsigned char *data_PHY_REG_2T_start, *data_PHY_REG_2T_end;
EXTERN unsigned char *data_PHY_REG_1T_start, *data_PHY_REG_1T_end;
EXTERN unsigned char *data_radio_a_1T_start, *data_radio_a_1T_end;
EXTERN unsigned char *data_radio_a_2T_start, *data_radio_a_2T_end;
EXTERN unsigned char *data_radio_b_2T_start, *data_radio_b_2T_end;
EXTERN unsigned char *data_rtl8192cfw_start, *data_rtl8192cfw_end;
#endif

EXTERN unsigned char *data_AGC_TAB_n_start, *data_AGC_TAB_n_end;
EXTERN unsigned char *data_PHY_REG_2T_n_start, *data_PHY_REG_2T_n_end;
EXTERN unsigned char *data_PHY_REG_1T_n_start, *data_PHY_REG_1T_n_end;
EXTERN unsigned char *data_radio_a_2T_n_start, *data_radio_a_2T_n_end;
EXTERN unsigned char *data_radio_b_2T_n_start, *data_radio_b_2T_n_end;
EXTERN unsigned char *data_radio_a_1T_n_start, *data_radio_a_1T_n_end;
EXTERN unsigned char *data_rtl8192cfw_n_start, *data_rtl8192cfw_n_end;
EXTERN unsigned char *data_rtl8192cfw_ua_start, *data_rtl8192cfw_ua_end;

EXTERN unsigned char *data_MACPHY_REG_start, *data_MACPHY_REG_end;
EXTERN unsigned char *data_PHY_REG_PG_start, *data_PHY_REG_PG_end;
EXTERN unsigned char *data_PHY_REG_MP_n_start, *data_PHY_REG_MP_n_end;

EXTERN unsigned char *data_AGC_TAB_n_hp_start, *data_AGC_TAB_n_hp_end;
EXTERN unsigned char *data_PHY_REG_2T_n_hp_start, *data_PHY_REG_2T_n_hp_end;
EXTERN unsigned char *data_PHY_REG_1T_n_hp_start, *data_PHY_REG_1T_n_hp_end;
EXTERN unsigned char *data_radio_a_2T_n_lna_start, *data_radio_a_2T_n_lna_end;
EXTERN unsigned char *data_radio_b_2T_n_lna_start, *data_radio_b_2T_n_lna_end;

#ifdef HIGH_POWER_EXT_PA
EXTERN unsigned char *data_radio_a_2T_n_hp_start, *data_radio_a_2T_n_hp_end;
EXTERN unsigned char *data_radio_b_2T_n_hp_start, *data_radio_b_2T_n_hp_end;
EXTERN unsigned char *data_PHY_REG_PG_hp_start, *data_PHY_REG_PG_hp_end;
#endif

#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_ioctl.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_IOCTL_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_RTL_COMAPI_WLTOOLS

EXTERN const struct iw_handler_def rtl8192cd_iw_handler_def;

#endif

EXTERN int _atoi(char *s, int base);
EXTERN void set_mib_default_tbl(struct rtl8192cd_priv *priv);
EXTERN int get_array_val (unsigned char *dst, char *src, int len);
EXTERN int del_sta(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void drvmac_loopback(struct rtl8192cd_priv *priv);
EXTERN int rtl8192cd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

#ifdef INCLUDE_WPS
EXTERN	int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN 	int get_mib(struct rtl8192cd_priv *priv, unsigned char *data);
#endif

#ifdef CONFIG_RTL8186_KB
EXTERN int set_guestmacinvalid(struct rtl8192cd_priv *priv, char *buf);
#endif

#ifdef CONFIG_RTL8672
// MBSSID Port Mapping
struct port_map {
	struct net_device *dev_pointer;
	int dev_ifgrp_member;
};
#endif

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
EXTERN int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int rtl8192cd_ss_req(struct rtl8192cd_priv *priv, unsigned char *data, int len);
#endif
EXTERN void delay_us(unsigned int t);
EXTERN void delay_ms(unsigned int t);

#undef EXTERN




#if 0	//move to hw.c
/*-----------------------------------------------------------------------------
								8190n_cam.c
------------------------------------------------------------------------------*/
#ifndef _8190N_CAM_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int CamAddOneEntry(struct rtl8190_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned long ulEncAlg, unsigned long ulUseDK,
				unsigned char *pucKey);
EXTERN int CamDeleteOneEntry(struct rtl8190_priv *priv, unsigned char *pucMacAddr,
				unsigned long ulKeyId, unsigned int useDK);
EXTERN void CamResetAllEntry(struct rtl8190_priv *priv);
EXTERN void CamDumpAll(struct rtl8190_priv *priv);
EXTERN void CAM_read_entry(struct rtl8190_priv *priv, unsigned char index, unsigned char *macad,
				unsigned char *key128, unsigned short *config);

#undef EXTERN
#endif




/*-----------------------------------------------------------------------------
								8192cd_security.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_SECURITY_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int DOT11_Indicate_MIC_Failure(struct net_device *dev, struct stat_info *pstat);
EXTERN void DOT11_Indicate_MIC_Failure_Clnt(struct rtl8192cd_priv *priv, unsigned char *sa);
EXTERN void DOT11_Process_MIC_Timerup(unsigned long data);
EXTERN void DOT11_Process_Reject_Assoc_Timerup(unsigned long data);
EXTERN int rtl8192cd_ioctl_priv_daemonreq(struct net_device *dev, struct iw_point *data);
EXTERN int DOT11_Process_Set_Key(struct net_device *dev, struct iw_point *data,
				DOT11_SET_KEY *pSetKey, unsigned char *pKey);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_tkip.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_TKIP_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void tkip_icv(unsigned char *picv, unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2,unsigned int frag2_len);
EXTERN void tkip_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned int hdrlen, unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2, unsigned int frag2_len, unsigned char *frag3,
				unsigned int frag3_len);
EXTERN void michael(struct rtl8192cd_priv *priv, unsigned char *key, unsigned char *hdr,
				unsigned char *llc, unsigned char *message, int message_length, unsigned char *mic, int tx);
EXTERN unsigned int tkip_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo,
				unsigned int fr_len);
EXTERN int tkip_rx_mic(struct rtl8192cd_priv *priv, unsigned char *pframe, unsigned char *da,
				unsigned char *sa, unsigned char priority, unsigned char *pbuf, unsigned int len,
				unsigned char *tkipmic, int no_wait);
EXTERN void debug_out(unsigned char *label, unsigned char *data, int data_length);
EXTERN unsigned int wep_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo,
				unsigned int fr_len, int type, int keymap);
EXTERN void wep_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr, unsigned int hdrlen,
				unsigned char *frag1, unsigned int frag1_len,
				unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3, unsigned int frag3_len,
				int type);
EXTERN void init_crc32_table(void);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_aes.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_AES_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void aesccmp_encrypt(struct rtl8192cd_priv *priv, unsigned char *pwlhdr,
				unsigned char *frag1,unsigned char *frag2, unsigned int frag2_len,
				unsigned char *frag3);
EXTERN unsigned int aesccmp_decrypt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_proc.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_PROC_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef _INCLUDE_PROC_FS_
EXTERN void rtl8192cd_proc_init (struct net_device *dev);
EXTERN void rtl8192cd_proc_remove (struct net_device *dev);
#endif

#ifdef CONFIG_RTK_MESH
//EXTERN const unsigned short MCS_DATA_RATE[2][2][16];
EXTERN const unsigned char* MCS_DATA_RATEStr[2][2][16];
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_br_ext.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_BR_EXT_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef RTK_BR_EXT
EXTERN void nat25_db_cleanup(struct rtl8192cd_priv *priv);
EXTERN void nat25_db_expire(struct rtl8192cd_priv *priv);
EXTERN int nat25_db_handle(struct rtl8192cd_priv *priv,	struct sk_buff *skb, int method);
EXTERN int nat25_handle_frame(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN int mac_clone_handle_frame(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN void dhcp_flag_bcast(struct rtl8192cd_priv *priv, struct sk_buff *skb);
EXTERN void *scdb_findEntry(struct rtl8192cd_priv *priv, unsigned char *macAddr, unsigned char *ipAddr);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_eeprom.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_EEPROM_C_
#define EXTERN  extern
#else
#define EXTERN
#endif
/*
EXTERN int ReadAdapterInfo(struct rtl8192cd_priv *priv, int entry_id, void *data);
EXTERN int WriteAdapterInfo(struct rtl8192cd_priv *priv, int entry_id, void *data);
*/
#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_osdep.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_OSDEP_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int  rtl8192cd_set_hwaddr(struct net_device *dev, void *addr);
EXTERN int rtl8192cd_open (struct net_device *dev);
EXTERN int rtl8192cd_close(struct net_device *dev);
EXTERN void update_fwtbl_asoclst(struct rtl8192cd_priv *priv, struct stat_info *pstat);
EXTERN void clear_shortcut_cache(void);

#ifdef WDS
#ifdef LAZY_WDS
EXTERN void delete_wds_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif
EXTERN struct stat_info *add_wds_entry(struct rtl8192cd_priv *priv, int idx, unsigned char *mac);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_psk.c
------------------------------------------------------------------------------*/
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
#ifndef _8192CD_PSK_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void derivePSK(struct rtl8192cd_priv *priv);
EXTERN void psk_init(struct rtl8192cd_priv *priv);
EXTERN int psk_indicate_evt(struct rtl8192cd_priv *priv, int id,
				unsigned char *mac, unsigned char *msg, int len);
EXTERN void ToDrv_SetGTK(struct rtl8192cd_priv *priv);
#ifdef WDS
EXTERN void wds_psk_init(struct rtl8192cd_priv *priv);
EXTERN void wds_psk_set(struct rtl8192cd_priv *priv, int idx, unsigned char *key);
#ifdef WIFI_HAPD
EXTERN void hapd_set_wdskey(struct net_device *dev, char *wdsPskPassPhrase, char *ssid, int wds_num);
#endif
#endif
#endif // INCLUDE_WPA_PSK

#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_mp.c
------------------------------------------------------------------------------*/
#ifndef _8192CD_MP_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void mp_start_test(struct rtl8192cd_priv *priv);
EXTERN void mp_stop_test(struct rtl8192cd_priv *priv);
EXTERN void mp_set_datarate(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_channel(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_bandwidth(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_tx_power(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_ctx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_stats(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_txpower_tracking(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_tssi(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_query_ther(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_tx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_validate_rx_packet(struct rtl8192cd_priv *priv, unsigned char *data, int len);
EXTERN int mp_brx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN int mp_arx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_bssid(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_ant_tx(struct rtl8192cd_priv *priv, unsigned char *data);
EXTERN void mp_set_ant_rx(struct rtl8192cd_priv *priv, unsigned char *data);
//EXTERN void mp_set_phypara(struct rtl8192cd_priv *priv, unsigned char *data);


#undef EXTERN




/*-----------------------------------------------------------------------------
								8192cd_mib.c
------------------------------------------------------------------------------*/
#ifdef SUPPORT_SNMP_MIB

#ifndef _8192CD_MIB_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int mib_get(struct rtl8192cd_priv *priv, char *oid, unsigned char *data, int *pLen);
EXTERN void mib_init(struct rtl8192cd_priv *priv);

#endif // SUPPORT_SNMP_MIB

#undef EXTERN


/*-----------------------------------------------------------------------------
								8192cd_comapi.c
------------------------------------------------------------------------------*/

#ifndef _8192CD_COMAPI_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
#include <net/iw_handler.h>
/*
EXTERN struct iw_statistics *rtl8192cd_get_wireless_stats(struct net_device *net_dev);
*/
EXTERN int rtl_siwfreq(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwfreq(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwmode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwmode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwrange(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwap(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwap(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_iwaplist(struct net_device *dev, struct iw_request_info *info, struct iw_point *data, char *extra);
EXTERN int rtl_siwessid(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *essid);
EXTERN int rtl_giwessid(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *essid);
EXTERN int rtl_siwrate(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwrate(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwrts(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwrts(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwfrag(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwfrag(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwretry(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwretry(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwencode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *keybuf);
EXTERN int rtl_giwencode(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *key);
EXTERN int rtl_giwpower(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_siwscan(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_giwscan(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif


#ifdef CONFIG_RTL_COMAPI_CFGFILE

EXTERN int CfgFileProc(struct net_device *dev);
EXTERN int CfgFileRead(struct net_device *dev, char *buf);

#endif // CONFIG_RTL_COMAPI_CFGFILE

#undef EXTERN


#ifdef CONFIG_RTL865X

#define EXTERN extern

/* 865x platform utilities */
EXTERN int GetChipVersion(char *name, unsigned int size, int *rev);
EXTERN long rtlglue_extPortRecv(void *id, unsigned char *data,  unsigned long len, unsigned short myvid, unsigned long myportmask, unsigned long linkID);
EXTERN long devglue_regExtDevice(unsigned char *devName, unsigned short vid, unsigned char extPortNum, unsigned long *linkId);
EXTERN long devglue_unregExtDevice(unsigned long linkId);

#undef EXTERN

#endif


/*-----------------------------------------------------------------------------
								8192cd_net80211.c
------------------------------------------------------------------------------*/

#ifdef WIFI_HAPD

#define EXTERN extern
EXTERN int rtl_net80211_setparam(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_setappiebuf(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_setmlme(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_setkey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_delkey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_wdsaddmac(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_wdsdelmac(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_net80211_getwpaie(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN int rtl_hapd_config(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
EXTERN void rsn_init(struct rtl8192cd_priv *priv);
#undef EXTERN

#endif


#ifdef CONFIG_RTK_MESH
/*-----------------------------------------------------------------------------
								mesh_ext/mesh_proc.c
------------------------------------------------------------------------------*/
#ifndef _MESH_PROC_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int dump_mesh_one_mpflow_sta(int num, struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtk8190_proc_flow_stats(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtk8190_proc_flow_stats_write(struct file *file, const char *buffer, unsigned long count, void *data);
EXTERN int rtl8190_mesh_assoc_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data);

#ifdef MESH_BOOTSEQ_AUTH
EXTERN int rtl8190_mesh_auth_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif

EXTERN int rtl8190_mesh_unEstablish_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtl8190_pathsel_routetable_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtl8190_portal_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtl8190_proc_mesh_stats(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtl8190_proxy_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int rtl8190_root_info(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#ifdef MESH_USE_METRICOP
EXTERN int rtl8190_mesh_metric_w (struct file *file, const char *buffer, unsigned long count, void *data);
EXTERN int rtl8190_mesh_metric_r(char *buf, char **start, off_t offset, int length, int *eof, void *data);
#endif
#if DBG_NCTU_MESH
EXTERN int rtl8190_setMeshDebugLevel (struct file *file, const char *buffer, unsigned long count, void *data);
EXTERN int hasMeshDebugLevel (int lv);
EXTERN int clearMeshDebugLevel (int lv);
EXTERN int showAllSkbs(char *buf, char **start, off_t offset, int length, int *eof, void *data);
EXTERN int decMySkb(struct sk_buff *);
EXTERN int tagMySkb(struct sk_buff *, UINT8);
EXTERN int isMySkb(struct sk_buff *);
EXTERN int showSpecificSkbs(UINT8 type);
#endif


#undef EXTERN



/*-----------------------------------------------------------------------------
								mesh_ext/mesh_route.c
------------------------------------------------------------------------------*/
#ifndef _MESH_ROUTE_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN	void aodv_expire(struct rtl8192cd_priv *priv);
EXTERN unsigned short chkMeshSeq(struct rtl8192cd_priv *priv, unsigned char *srcMac, unsigned short seq);
EXTERN void GEN_PREQ_PACKET(char *targetMac, struct rtl8192cd_priv *priv, char insert);
EXTERN unsigned short getMeshSeq(struct rtl8192cd_priv *priv);
EXTERN void init_mpp_pool(struct mpp_tb* pTB);
EXTERN void notifyPathSelection(void);
EXTERN struct path_sel_entry *pathsel_query_table(struct rtl8192cd_priv* ,unsigned char destaddr[MACADDRLEN] );
EXTERN int pathsel_modify_table_entry(struct rtl8192cd_priv *priv, struct path_sel_entry *pEntry);
EXTERN int pathsel_table_entry_insert_tail(struct rtl8192cd_priv *priv, struct path_sel_entry *pEntry);
EXTERN int remove_path_entry(struct rtl8192cd_priv *priv,unsigned char invalid_addr[MACADDRLEN]);
EXTERN void route_maintenance(struct rtl8192cd_priv *priv);

#ifdef PU_STANDARD
EXTERN UINT8 getPUSeq(struct rtl8192cd_priv *priv);
#endif

EXTERN unsigned char* getMeshHeader(struct rtl8192cd_priv *priv, int wep_mode, unsigned char* pframe);

#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_rx.c
------------------------------------------------------------------------------*/
#ifndef _MESH_RX_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int process_11s_datafrme(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat);
EXTERN int rtl8190_rx_dispatch_mesh(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_tx.c
------------------------------------------------------------------------------*/
#ifndef _MESH_TX_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void do_aodv_routing(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned char *Mesh_dest);
#if defined(MESH_TX_SHORTCUT)
//EXTERN int mesh_txsc_decision(struct stat_info * pstat, struct tx_insn *txcfg);
EXTERN int mesh_txsc_decision(struct tx_insn* cfgNew, struct tx_insn* cfgOld);
#endif
EXTERN int dot11s_datapath_decision(struct sk_buff *skb, /*struct net_device *dev,*/ struct tx_insn* ptxinsn, int isUpdateProxyTable);
EXTERN int issue_11s_mesh_action(struct sk_buff *skb, struct net_device *dev);
EXTERN int notify_path_found(unsigned char *destaddr, struct rtl8192cd_priv *priv);
EXTERN int rtl8190_start_relay_11s_dataframe(struct sk_buff *skb, /*struct net_device *dev,*/ int privacy, struct rx_frinfo *pfrinfo);
EXTERN void toAllPortal(struct sk_buff *pskb,struct rtl8192cd_priv *priv);

#ifdef	_11s_TEST_MODE_
EXTERN void galileo_timer(unsigned long task_priv);
EXTERN void issue_test_traffic(struct sk_buff *skb);
EXTERN void signin_txdesc_galileo(struct rtl8192cd_priv *priv, struct tx_insn* txcfg);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_sme.c
------------------------------------------------------------------------------*/
#ifndef _MESH_SME_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN unsigned int acl_query(struct rtl8192cd_priv *priv, unsigned char *sa);
EXTERN int close_MeshPeerLink(struct rtl8192cd_priv *priv, UINT8 *da);
EXTERN int is_11s_mgt_frame(int num, struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int issue_assocreq_MP(struct rtl8192cd_priv *priv,  struct stat_info *pstat);
EXTERN void issue_assocrsp_MP(struct rtl8192cd_priv *priv, unsigned short status, struct stat_info *pstat, int pkt_type);
EXTERN void issue_disassoc_MP(struct rtl8192cd_priv * priv, struct stat_info * pstat, int reason, UINT8 peerLinkReason);
EXTERN void issue_probereq_MP(struct rtl8192cd_priv *priv, unsigned char *ssid, int ssid_len, unsigned char *da, int is_11s);

#ifdef MESH_BOOTSEQ_AUTH
EXTERN void mesh_auth_timer(unsigned long pVal);
#endif

EXTERN void mesh_expire(struct rtl8192cd_priv* priv);
EXTERN unsigned int mesh_ie_MeshID(struct rtl8192cd_priv *priv, UINT8 meshiearray[], UINT8 isWildcard);
EXTERN unsigned int mesh_ie_WLANMeshCAP(struct rtl8192cd_priv *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_MKDDIE(struct rtl8192cd_priv *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_DTIM(struct rtl8192cd_priv *priv, UINT8 meshiearray[]);
EXTERN unsigned int mesh_ie_EMSAIE(struct rtl8192cd_priv *priv, struct stat_info *pstat, UINT8 meshiearray[]);
EXTERN void mesh_peer_link_timer(unsigned long pVal);
EXTERN unsigned int OnAssocReq_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnAssocRsp_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnBeacon_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnDisassoc_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnProbeReq_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnProbeRsp_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN unsigned int OnPathSelectionManagFrame(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, int Is_6Addr);
EXTERN void OnLocalLinkStateANNOU_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN int start_MeshPeerLink(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat, UINT16 cap, UINT8 OFDMparam);

#ifdef PU_STANDARD
EXTERN void issue_proxyupdate_MP(struct rtl8192cd_priv *priv, struct proxyupdate_table_entry *);
EXTERN void OnProxyUpdate_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
EXTERN void OnProxyUpdateConfirm_MP(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo);
#endif
//#ifdef	_11s_TEST_MODE_
EXTERN void mac12_to_6(unsigned char *in, unsigned char *out);
//#endif
#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_util.c
------------------------------------------------------------------------------*/
#ifndef _MESH_UTIL_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void mesh_cnt_ASSOC_PeerLink_CAP(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act);
EXTERN void mesh_set_PeerLink_CAP(struct  rtl8192cd_priv *priv, UINT16 meshCapSetValue);

#ifdef	_11s_TEST_MODE_
EXTERN void mac12_to_6(unsigned char*, unsigned char*);
#endif

#undef EXTERN




/*-----------------------------------------------------------------------------
								mesh_ext/mesh_security.c
------------------------------------------------------------------------------*/
#ifndef _MESH_SECURITY_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int DOT11_EnQueue2(unsigned long task_priv, DOT11_QUEUE2 *q, unsigned char *item, int itemsize);

EXTERN int DOT11_DeQueue2(unsigned long task_priv, DOT11_QUEUE2 *q, unsigned char *item, int *itemsize);
EXTERN void DOT11_InitQueue2(DOT11_QUEUE2 * q, int szMaxItem, int szMaxData);

#undef EXTERN

#endif // CONFIG_RTK_MESH


/*-----------------------------------------------------------------------------
								8192cd_a4_sta.c
------------------------------------------------------------------------------*/
#ifdef A4_STA
#ifndef _8192CD_A4_STA_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN void a4_sta_cleanup(struct rtl8192cd_priv *priv);
EXTERN void a4_sta_expire(struct rtl8192cd_priv *priv);
EXTERN void a4_sta_add(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *mac);
EXTERN struct stat_info *a4_sta_lookup(struct rtl8192cd_priv *priv, unsigned char *mac);

#undef EXTERN
#endif
/*-----------------------------------------------------------------------------
								8192cd_wscd.c
------------------------------------------------------------------------------*/
#ifdef WIFI_SIMPLE_CONFIG
#ifdef INCLUDE_WPS

#ifndef _8192CD_WSCD_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

EXTERN int wps_get_deviceinfo(struct soap_t *);
EXTERN int wps_put_wlanrsp(struct soap_t *);
EXTERN int wps_set_registrar(struct soap_t *);
EXTERN int wps_put_message(struct soap_t *);

#ifdef CONFIG_MSC

EXTERN int wps_start(WSC_CONF * config_set);
EXTERN void wps_stop(void);
EXTERN int wps_report_current_state(void);

EXTERN int wps_pbc(void);
EXTERN int wps_pin(char *);
// 2010-0403 add
EXTERN int wps_get_wlan_macaddr(char* ifname , char *macaddr);
EXTERN int wps_get_uuid(char* ifname , char *uuid_Ptr);
EXTERN void wsc_debug_out(unsigned char *label, unsigned char *data, int data_length);
EXTERN void wps_subscribe_notify(int add_del);
#else
EXTERN int receive_config_setting(struct rtl8192cd_priv* priv , WSC_CONF *config_set );
#endif
EXTERN int PWSCUpnpCallbackEventHandler(struct WSC_packet *packet);
EXTERN int wps_init(struct rtl8192cd_priv *priv);
// for notify have event to wps
EXTERN void wps_indicate_evt(struct rtl8192cd_priv *priv);
EXTERN	void wps_1sec_routine(struct rtl8192cd_priv* priv );
EXTERN void wps_NonQueue_indicate_evt(struct rtl8192cd_priv *priv , char *data , int len);

#undef EXTERN
#endif
#endif
/*-----------------------------------------------------------------------------
								sercomm_intf.c
------------------------------------------------------------------------------*/
#ifndef _SERCOMM_INTF_C_
#define EXTERN  extern
#else
#define EXTERN
#endif

#ifdef CONFIG_MSC

EXTERN int  sys_reg_upnp_dev(struct wps_profile_t *profilePtr);
EXTERN int  sys_led_control(int status);
EXTERN int  sys_send_event(char *packet);
EXTERN void sys_exit_wsc(void);
EXTERN int  sys_wps_terminate(int status);
EXTERN int  sys_write_flash(FLASH_CMD *fcmd);
EXTERN void sys_reinit(void);
EXTERN void sys_txUPNPEvent(unsigned char *ip, char *outMsg, int outMsgLen);
EXTERN int sys_getTotalSubScribe(void);
#endif

#endif // _8192CD_HEADERS_H_

