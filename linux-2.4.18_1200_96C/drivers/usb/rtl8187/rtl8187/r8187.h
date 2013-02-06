/* 
   This is part of rtl8187 OpenSource driver.
   Copyright (C) Andrea Merello 2004-2005  <andreamrl@tiscali.it> 
   Released under the terms of GPL (General Public Licence)
   
   Parts of this driver are based on the GPL part of the 
   official realtek driver
   
   Parts of this driver are based on the rtl8180 driver skeleton 
   from Patric Schenke & Andres Salomon
   
   Parts of this driver are based on the Intel Pro Wireless 2100 GPL driver
   
   We want to tanks the Authors of those projects and the Ndiswrapper 
   project Authors.
*/

#ifndef R8180H
#define R8180H


#define RTL8187_MODULE_NAME "rtl8187"
#define DMESG(x,a...) printk(KERN_INFO RTL8187_MODULE_NAME ": " x "\n", ## a)
#define DMESGW(x,a...) printk(KERN_WARNING RTL8187_MODULE_NAME ": WW:" x "\n", ## a)
#define DMESGE(x,a...) printk(KERN_WARNING RTL8187_MODULE_NAME ": EE:" x "\n", ## a)

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
//#include <linux/pci.h>
#include <linux/usb.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/rtnetlink.h>	//for rtnl_lock()
#include <linux/wireless.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>	// Necessary because we use the proc fs
#include <linux/if_arp.h>
#include <linux/random.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/semaphore.h>

#include "ieee80211.h"

#define EPROM_93c46 0
#define EPROM_93c56 1

#define DEFAULT_FRAG_THRESHOLD 2342U
#define MIN_FRAG_THRESHOLD     256U
#define DEFAULT_BEACONINTERVAL 0x64U
#define DEFAULT_BEACON_ESSID "Rtl8187"

#define DEFAULT_SSID ""
#define DEFAULT_RETRY_RTS 7
#define DEFAULT_RETRY_DATA 7
#define PRISM_HDR_SIZE 64

#define RTL_IOCTL_WPA_SUPPLICANT		SIOCIWFIRSTPRIV+30
//#define DEBUG_RX

typedef struct buffer
{
	struct buffer *next;
	u32 *buf;
	
} buffer;

#if 0

typedef struct tx_pendingbuf
{
	struct ieee80211_txb *txb;
	short ispending;
	short descfrag;
} tx_pendigbuf;

#endif

typedef struct Stats
{
	unsigned long txrdu;
//	unsigned long rxrdu;
	//unsigned long rxnolast;
	//unsigned long rxnodata;
//	unsigned long rxreset;
//	unsigned long rxwrkaround;
//	unsigned long rxnopointer;
	unsigned long rxok;
	unsigned long rxurberr;
	unsigned long rxstaterr;
	unsigned long txnperr;
	unsigned long txnpdrop;
	unsigned long txresumed;
//	unsigned long rxerr;
//	unsigned long rxoverflow;
//	unsigned long rxint;
	unsigned long txnpokint;
//	unsigned long txhpokint;
//	unsigned long txhperr;
//	unsigned long ints;
//	unsigned long shints;
	unsigned long txoverflow;
//	unsigned long rxdmafail;
//	unsigned long txbeacon;
//	unsigned long txbeaconerr;
	unsigned long txlpokint;
	unsigned long txlpdrop;
	unsigned long txlperr;
	unsigned long txdatapkt;
} Stats;



typedef struct r8180_priv
{
	struct usb_device *udev;
	short epromtype;
	int irq;
	struct ieee80211_device *ieee80211;
	
	short card_8185; /* O: rtl8180, 1:rtl8185 V B/C, 2:rtl8185 V D */
	short card_8185_Bversion; /* if TCR reports card V B/C this discriminates */
	short phy_ver; /* meaningful for rtl8225 1:A 2:B 3:C */
	short enable_gpio0;
	enum card_type {PCI,MINIPCI,CARDBUS,USB/*rtl8187*/}card_type;
	short hw_plcp_len;
	short plcp_preamble_mode;
		
//	spinlock_t irq_lock;
//	spinlock_t irq_th_lock;
	spinlock_t tx_lock;
	
	u16 irq_mask;
//	short irq_enabled;
	struct net_device *dev;
	short chan;
	short sens;
	short max_sens;
	u8 chtxpwr[15]; //channels from 1 to 14, 0 not used
	u8 chtxpwr_ofdm[15]; //channels from 1 to 14, 0 not used
	u8 cck_txpwr_base;
	u8 ofdm_txpwr_base;
	u8 challow[15]; //channels from 1 to 14, 0 not used
	short up;
	short crcmon; //if 1 allow bad crc frame reception in monitor mode
//	short prism_hdr;
	
//	struct timer_list scan_timer;
	/*short scanpending;
	short stopscan;*/
//	spinlock_t scan_lock;
//	u8 active_probe;
	//u8 active_scan_num;
	struct semaphore wx_sem;
//	short hw_wep;
		
//	short digphy;
//	short antb;
//	short diversity;
//	u8 cs_treshold;
//	short rcr_csense;
	short rf_chip;
//	u32 key0[4];
	short (*rf_set_sens)(struct net_device *dev,short sens);
	void (*rf_set_chan)(struct net_device *dev,short ch);
	void (*rf_close)(struct net_device *dev);
	void (*rf_init)(struct net_device *dev);
	//short rate;
	short promisc;	
	/*stats*/
	struct Stats stats;
	struct iw_statistics wstats;
	struct proc_dir_entry *dir_dev;
	
	/*RX stuff*/
//	u32 *rxring;
//	u32 *rxringtail;
//	dma_addr_t rxringdma;
	struct urb **rx_urb;
	
	//struct buffer *rxbuffer;
	//struct buffer *rxbufferhead;
	//int rxringcount;
	//u16 rxbuffersize;
	
	//struct sk_buff *rx_skb; 

	//short rx_skb_complete;

	//u32 rx_prevlen;
	atomic_t tx_lp_pending;
	atomic_t tx_np_pending;
#ifdef CONFIG_RTL865X
	u8 rtl8187extPortNum;		/*Ext Port Number*/
	u32 rtl8187linkNum[16];
#endif
#if 0	
	/*TX stuff*/
	u32 *txlpring;
	u32 *txhpring;
	u32 *txnpring;
	dma_addr_t txlpringdma;
	dma_addr_t txhpringdma;
	dma_addr_t txnpringdma;
	u32 *txlpringtail;
	u32 *txhpringtail;
	u32 *txnpringtail;
	u32 *txlpringhead;
	u32 *txhpringhead;
	u32 *txnpringhead;
	struct buffer *txlpbufs;
	struct buffer *txhpbufs;
	struct buffer *txnpbufs;
	struct buffer *txlpbufstail;
	struct buffer *txhpbufstail;
	struct buffer *txnpbufstail;
	int txringcount;
	int txbuffsize;

	//struct tx_pendingbuf txnp_pending;
	struct tasklet_struct irq_tx_tasklet;
#endif
	struct tasklet_struct irq_rx_tasklet;
	struct urb *rxurb_task;
//	u8 dma_poll_mask;
	//short tx_suspend;
	
	/* adhoc/master mode stuff */
#if 0
	u32 *txbeacontail;
	dma_addr_t txbeaconringdma;
	u32 *txbeaconring;
	int txbeaconcount;
#endif
//	struct ieee_tx_beacon *beacon_buf;
	//char *master_essid;
//	dma_addr_t beacondmabuf;
	//u16 master_beaconinterval;
//	u32 master_beaconsize;
	//u16 beacon_interval;
	
	u8 retry_data;
	u8 retry_rts;
	u16 rts;
	
	//struct work_struct reset_wq;
	struct tq_struct reset_wq;
	
}r8180_priv;


typedef enum{ 
	LOW_PRIORITY ,
	NORM_PRIORITY 
	} priority_t;


short rtl8180_tx(struct net_device *dev,u32* skbuf, int len,priority_t priority,short morefrag,short rate);

u8 read_nic_byte(struct net_device *dev, int x);
u8 read_nic_byte_E(struct net_device *dev, int x);
u32 read_nic_dword(struct net_device *dev, int x);
u16 read_nic_word(struct net_device *dev, int x) ;
void write_nic_byte(struct net_device *dev, int x,u8 y);
void write_nic_byte_E(struct net_device *dev, int x,u8 y);
void write_nic_word(struct net_device *dev, int x,u16 y);
void write_nic_dword(struct net_device *dev, int x,u32 y);
void force_pci_posting(struct net_device *dev);

void rtl8180_rtx_disable(struct net_device *);
void rtl8180_rx_enable(struct net_device *);
void rtl8180_tx_enable(struct net_device *);

void rtl8180_disassociate(struct net_device *dev);
//void fix_rx_fifo(struct net_device *dev);
void rtl8185_set_rf_pins_enable(struct net_device *dev,u32 a);

void rtl8180_set_anaparam(struct net_device *dev,u32 a);
void rtl8185_set_anaparam2(struct net_device *dev,u32 a);
void rtl8180_update_msr(struct net_device *dev);
int rtl8180_down(struct net_device *dev);
int rtl8180_up(struct net_device *dev);
void rtl8180_commit(struct net_device *dev);
void rtl8180_set_chan(struct net_device *dev,short ch);
void write_phy(struct net_device *dev, u8 adr, u8 data);
void write_phy_cck(struct net_device *dev, u8 adr, u32 data);
void write_phy_ofdm(struct net_device *dev, u8 adr, u32 data);
void rtl8185_tx_antenna(struct net_device *dev, u8 ant);
void rtl8187_set_rxconf(struct net_device *dev);
#endif
