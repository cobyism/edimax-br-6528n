/*
   This is part of rtl8187 OpenSource driver - v 0.1
   Copyright (C) Andrea Merello 2005  <andreamrl@tiscali.it> 
   Released under the terms of GPL (General Public License)
   
   
   Parts of this driver are based on the rtl8180 driver skeleton 
   from Patric Schenke & Andres Salomon.

   Parts of this driver are based on the Intel Pro Wireless 2*00 GPL drivers.
   
   some ideas might be derived from David Young rtl8180 netbsd driver.
   
   Parts of the usb code are from the r8150.c driver in linux kernel
   
   Some ideas borrowed from the 8139too.c driver included in linux kernel.
   
   We (I?) want to thanks the Authors of those projecs and also the 
   Ndiswrapper's project Authors.
   
   A special big thanks goes also to Realtek corp. for their help in my 
   attempt to add RTL8187 and RTL8225 support, and to David Young also. 

	- Please note that this file is a modified version from rtl8180-sa2400 
	drv. So some other people have contributed to this project, and they are
	thanked in the rtl8180-sa2400 CHANGELOG.
*/

#ifndef CONFIG_FORCE_HARD_FLOAT
double __floatsidf (int i) { return i; }
unsigned int __fixunsdfsi (double d) { return d; }
double __adddf3(double a, double b) { return a+b; }
double __addsf3(float a, float b) { return a+b; }
double __subdf3(double a, double b) { return a-b; }
double __extendsfdf2(float a) {return a;}
#endif

#undef LOOP_TEST
#undef DUMP_RX
#undef DUMP_TX
#undef DEBUG_TX_DESC2
#undef RX_DONT_PASS_UL
#undef DEBUG_EPROM
#undef DEBUG_RX_VERBOSE
#undef DUMMY_RX
#undef DEBUG_ZERO_RX
#undef DEBUG_RX_SKB
#undef DEBUG_TX_FRAG
#undef DEBUG_RX_FRAG
#undef DEBUG_TX_FILLDESC
#undef DEBUG_TX
#undef DEBUG_IRQ
#undef DEBUG_RX
#undef DEBUG_RXALLOC
#undef DEBUG_REGISTERS
#undef DEBUG_RING
#undef DEBUG_IRQ_TASKLET
#undef DEBUG_TX_ALLOC
#undef DEBUG_TX_DESC

//#define CONFIG_RTL8180_IO_MAP

#include "r8180_hw.h"
#include "r8187.h"
#include "r8180_rtl8225.h" /* RTL8225 Radio frontend */
#include "r8180_93cx6.h"   /* Card EEPROM */
#include "r8180_wx.h"

//#define DEBUG_RX

// FIXME: check if 2.6.7 is ok
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7))
#define usb_kill_urb usb_unlink_urb
#endif

#ifdef CONFIG_RTL8180_PM
#include "r8180_pm.h"
#endif

EXPORT_NO_SYMBOLS;

#ifndef USB_VENDOR_ID_REALTEK
#define USB_VENDOR_ID_REALTEK		0x0bda
#endif
#ifndef USB_VENDOR_ID_NETGEAR
#define USB_VENDOR_ID_NETGEAR		0x0846
#endif

static struct usb_device_id rtl8187_usb_id_tbl[] = {
	{USB_DEVICE(USB_VENDOR_ID_REALTEK, 0x8187)},
	{USB_DEVICE(USB_VENDOR_ID_NETGEAR, 0x6100)},
	{USB_DEVICE(USB_VENDOR_ID_NETGEAR, 0x6a00)},

	{}
};

static char* ifname = "wlan%d";
#if 0
static int hwseqnum = 0;
static int hwwep = 0;
#endif
static int channels = 0x3fff;

MODULE_LICENSE("GPL");
//MODULE_VERSION("V 1.1");
MODULE_DEVICE_TABLE(usb, rtl8187_usb_id_tbl);
MODULE_AUTHOR("Andrea Merello <andreamrl@tiscali.it>");
MODULE_DESCRIPTION("Linux driver for Realtek RTL8187 WiFi cards");

#if 0
MODULE_PARM(ifname,"s");
MODULE_PARM_DESC(devname," Net interface name, wlan%d=default");

MODULE_PARM(hwseqnum,"i");
MODULE_PARM_DESC(hwseqnum," Try to use hardware 802.11 header sequence numbers. Zero=default");

MODULE_PARM(hwwep,"i");
MODULE_PARM_DESC(hwwep," Try to use hardware WEP support. Still broken and not available on all cards");

MODULE_PARM(channels,"i");
MODULE_PARM_DESC(channels," Channel bitmask for specific locales. NYI");
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)
module_param(ifname, charp, S_IRUGO|S_IWUSR );
//module_param(hwseqnum,int, S_IRUGO|S_IWUSR);
//module_param(hwwep,int, S_IRUGO|S_IWUSR);
module_param(channels,int, S_IRUGO|S_IWUSR);
#else
MODULE_PARM(ifname, "s");
//MODULE_PARM(hwseqnum,"i");
//MODULE_PARM(hwwep,"i");
MODULE_PARM(channels,"i");
#endif

MODULE_PARM_DESC(devname," Net interface name, wlan%d=default");
//MODULE_PARM_DESC(hwseqnum," Try to use hardware 802.11 header sequence numbers. Zero=default");
//MODULE_PARM_DESC(hwwep," Try to use hardware WEP support. Still broken and not available on all cards");
MODULE_PARM_DESC(channels," Channel bitmask for specific locales. NYI");

//static int __devinit rtl8187_usb_probe(struct usb_interface *intf,
//			 const struct usb_device_id *id);
#if 0
static void * __devinit rtl8187_usb_probe(struct usb_device *dev,
	    		unsigned intf,			
	    		const struct usb_device_id *id);
#endif
static void * rtl8187_usb_probe(struct usb_device *dev,
	    		unsigned int intf,			
	    		const struct usb_device_id *id);
			 
//static void __devexit rtl8187_usb_disconnect(struct usb_interface *intf);
#if 0
static void __devexit rtl8187_usb_disconnect(struct usb_device *dev, void *ptr);
#endif
static void rtl8187_usb_disconnect(struct usb_device *dev, void *ptr);
static struct usb_driver rtl8187_usb_driver = {

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 16) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 20)
	.owner		= THIS_MODULE,
#endif
	.name		= RTL8187_MODULE_NAME,	          /* Driver name   */
	.id_table	= rtl8187_usb_id_tbl,	          /* PCI_ID table  */
	.probe		= rtl8187_usb_probe,	          /* probe fn      */
	.disconnect	= rtl8187_usb_disconnect,	  /* remove fn     */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 0)
#ifdef CONFIG_RTL8180_PM
	.suspend	= rtl8180_suspend,	          /* PM suspend fn */
	.resume		= rtl8180_resume,                 /* PM resume fn  */
#else
	.suspend	= NULL,			          /* PM suspend fn */
	.resume      	= NULL,			          /* PM resume fn  */
#endif
#endif
};


void write_nic_byte_E(struct net_device *dev, int indx, u8 data)
{
	
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	
	usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx|0xfe00, 0, &data, 1, HZ / 2);
}


void write_nic_byte(struct net_device *dev, int indx, u8 data)
{
	
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	
	usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx|0xff00, 0, &data, 1, HZ / 2);
}


void write_nic_word(struct net_device *dev, int indx, u16 data)
{
	
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	data=cpu_to_le16(data);
	usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx|0xff00, 0, &data, 2, HZ / 2);
}


void write_nic_dword(struct net_device *dev, int indx, u32 data)
{
	
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	data=cpu_to_le32(data);
	usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx|0xff00, 0, &data, 4, HZ / 2);
}
 
 
 
u8 read_nic_byte(struct net_device *dev, int indx)
{
	u8 data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	
	usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx|0xff00, 0, &data, 1, HZ / 2);
	return data;
}

u8 read_nic_byte_E(struct net_device *dev, int indx)
{
	u8 data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	
	usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx|0xfe00, 0, &data, 1, HZ / 2);
	return data;
}

 
u16 read_nic_word(struct net_device *dev, int indx)
{
	u16 data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	
	usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx|0xff00, 0, &data, 2, HZ / 2);
	data=le16_to_cpu(data);
	return data;
}


u32 read_nic_dword(struct net_device *dev, int indx)
{
	u32 data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;
	
	usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx|0xff00, 0, &data, 4, HZ / 2);
	data=le32_to_cpu(data);
	return data;
}

/* this might still called in what was the PHY rtl8185/rtl8187 common code 
 * plans are to possibilty turn it again in one common code...
 */
inline void force_pci_posting(struct net_device *dev)
{
}


//irqreturn_t rtl8180_interrupt(int irq, void *netdev, struct pt_regs *regs);
//void set_nic_rxring(struct net_device *dev);
//void set_nic_txring(struct net_device *dev);
static struct net_device_stats *rtl8180_stats(struct net_device *dev);
void rtl8180_commit(struct net_device *dev);
void rtl8180_restart(struct net_device *dev);

/****************************************************************************
   -----------------------------PROCFS STUFF-------------------------
*****************************************************************************/

static struct proc_dir_entry *rtl8180_proc = NULL;

static int proc_get_registers(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
//	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	int len = 0;
	int i,n;
			
	int max=0xff;
	
	/* This dump the current register page */
	for(n=0;n<=max;)
	{
		//printk( "\nD: %2x> ", n);
		len += snprintf(page + len, count - len,
			"\nD:  %2x > ",n);

		for(i=0;i<16 && n<=max;i++,n++)
		len += snprintf(page + len, count - len,
			"%2x ",read_nic_byte(dev,n));

		//	printk("%2x ",read_nic_byte(dev,n));
	}
	len += snprintf(page + len, count - len,"\n");


		
	*eof = 1;
	return len;

}

#if 0
static int proc_get_stats_hw(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	int len = 0;
	
	len += snprintf(page + len, count - len,
		"NIC int: %lu\n"
		"Total int: %lu\n",
		priv->stats.ints,
		priv->stats.shints);
			
	*eof = 1;
	return len;
}
#endif

static int proc_get_stats_tx(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	int len = 0;
	
	len += snprintf(page + len, count - len,
		"TX normal priority ok int: %lu\n"
		"TX normal priority error int: %lu\n"
//		"TX high priority ok int: %lu\n"
//		"TX high priority failed error int: %lu\n"
		"TX low priority ok int: %lu\n"
		"TX low priority failed error int: %lu\n"
		"TX queue resume: %lu\n"
		"TX queue stopped?: %d\n"
		"TX fifo overflow: %lu\n"
//		"TX beacon: %lu\n"
		"TX lp queue: %d\n"
		"TX np queue: %d\n"
		"TX HW queue: %d\n"
		"RX HW queue: %d\n"
		"TX lp dropped: %lu\n"
		"TX np dropped: %lu\n"
		"TX total data packets %lu\n",		
//		"TX beacon aborted: %lu\n",
		priv->stats.txnpokint,
		priv->stats.txnperr,
//		priv->stats.txhpokint,
//		priv->stats.txhperr,
		priv->stats.txlpokint,
		priv->stats.txlperr,
		priv->stats.txresumed,
		netif_queue_stopped(dev),
		priv->stats.txoverflow,
//		priv->stats.txbeacon,
		atomic_read(&(priv->tx_lp_pending)),
		atomic_read(&(priv->tx_np_pending)),
		read_nic_byte(dev, TXFIFOCOUNT),
		read_nic_byte(dev, RXFIFOCOUNT),
		priv->stats.txlpdrop,
		priv->stats.txnpdrop,
		priv->stats.txdatapkt
//		priv->stats.txbeaconerr
		);
			
	*eof = 1;
	return len;
}		



static int proc_get_stats_rx(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	int len = 0;
	
	len += snprintf(page + len, count - len,
		"RX packets: %lu\n"
		"RX urb status error: %lu\n"
		"RX invalid urb error: %lu\n",
		priv->stats.rxok,
		priv->stats.rxstaterr,
		priv->stats.rxurberr);
			
	*eof = 1;
	return len;
}		


static struct iw_statistics *r8180_get_wireless_stats(struct net_device *dev)
{
       struct r8180_priv *priv = ieee80211_priv(dev);

       return &priv->wstats;
}

void rtl8180_proc_module_init(void)
{	
	DMESG("Initializing proc filesystem");
	rtl8180_proc=create_proc_entry(RTL8187_MODULE_NAME, S_IFDIR, proc_net);
}


void rtl8180_proc_module_remove(void)
{
	remove_proc_entry(RTL8187_MODULE_NAME, proc_net);
}


void rtl8180_proc_remove_one(struct net_device *dev)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	if (priv->dir_dev) {
	//	remove_proc_entry("stats-hw", priv->dir_dev);
		remove_proc_entry("stats-tx", priv->dir_dev);
		remove_proc_entry("stats-rx", priv->dir_dev);
	//	remove_proc_entry("stats-ieee", priv->dir_dev);
	//	remove_proc_entry("stats-ap", priv->dir_dev);
		remove_proc_entry("registers", priv->dir_dev);
		remove_proc_entry(dev->name, rtl8180_proc);
		priv->dir_dev = NULL;
	}
}


void rtl8180_proc_init_one(struct net_device *dev)
{
	struct proc_dir_entry *e;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	priv->dir_dev = create_proc_entry(dev->name, 
					  S_IFDIR | S_IRUGO | S_IXUGO, 
					  rtl8180_proc);
	if (!priv->dir_dev) {
		DMESGE("Unable to initialize /proc/net/rtl8187/%s\n",
		      dev->name);
		return;
	}
	#if 0
	e = create_proc_read_entry("stats-hw", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_hw, dev);
				   
	if (!e) {
		DMESGE("Unable to initialize "
		      "/proc/net/rtl8187/%s/stats-hw\n",
		      dev->name);
	}
	#endif
	e = create_proc_read_entry("stats-rx", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_rx, dev);
				   
	if (!e) {
		DMESGE("Unable to initialize "
		      "/proc/net/rtl8187/%s/stats-rx\n",
		      dev->name);
	}
	
	
	e = create_proc_read_entry("stats-tx", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_tx, dev);
				   
	if (!e) {
		DMESGE("Unable to initialize "
		      "/proc/net/rtl8187/%s/stats-tx\n",
		      dev->name);
	}
	#if 0
	e = create_proc_read_entry("stats-ieee", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_ieee, dev);
				   
	if (!e) {
		DMESGE("Unable to initialize "
		      "/proc/net/rtl8187/%s/stats-ieee\n",
		      dev->name);
	}
	
	
	e = create_proc_read_entry("stats-ap", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_ap, dev);
				   
	if (!e) {
		DMESGE("Unable to initialize "
		      "/proc/net/rtl8187/%s/stats-ap\n",
		      dev->name);
	}
	#endif
	
	e = create_proc_read_entry("registers", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers, dev);
	
	if (!e) {
		DMESGE("Unable to initialize "
		      "/proc/net/rtl8187/%s/registers\n",
		      dev->name);
	}
}
/****************************************************************************
   -----------------------------MISC STUFF-------------------------
*****************************************************************************/

/* this is only for debugging */
void print_buffer(u32 *buffer, int len)
{
	int i;
	u8 *buf =(u8*)buffer;
	
	printk("ASCII BUFFER DUMP (len: %x):\n",len);
	
	for(i=0;i<len;i++)
		printk("%c",buf[i]);
		
	printk("\nBINARY BUFFER DUMP (len: %x):\n",len);
	
	for(i=0;i<len;i++)
		printk("%x",buf[i]);

	printk("\n");
}

short check_nic_enought_desc(struct net_device *dev, priority_t priority)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	int used = atomic_read((priority == NORM_PRIORITY) ? 
		&priv->tx_np_pending : &priv->tx_lp_pending);
	
	return (used < MAX_TX_URB);
}

void tx_timeout(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	//rtl8180_commit(dev);
	//schedule_work(&priv->reset_wq);
	schedule_task(&priv->reset_wq);
	//DMESG("TXTIMEOUT");
}


/* this is only for debug */
void dump_eprom(struct net_device *dev)
{
	int i;
	for(i=0; i<63; i++)
		DMESG("EEPROM addr %x : %x", i, eprom_read(dev,i));
}

/* this is only for debug */
void rtl8180_dump_reg(struct net_device *dev)
{
	int i;
	int n;
	int max=0xff;
	
	DMESG("Dumping NIC register map");	
	
	for(n=0;n<=max;)
	{
		printk( "\nD: %2x> ", n);
		for(i=0;i<16 && n<=max;i++,n++)
			printk("%2x ",read_nic_byte(dev,n));
	}
	printk("\n");
}

/****************************************************************************
      ------------------------------HW STUFF---------------------------
*****************************************************************************/


void rtl8180_irq_enable(struct net_device *dev)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);	
	//priv->irq_enabled = 1;
/*
	write_nic_word(dev,INTA_MASK,INTA_RXOK | INTA_RXDESCERR | INTA_RXOVERFLOW |\ 
	INTA_TXOVERFLOW | INTA_HIPRIORITYDESCERR | INTA_HIPRIORITYDESCOK |\ 
	INTA_NORMPRIORITYDESCERR | INTA_NORMPRIORITYDESCOK |\
	INTA_LOWPRIORITYDESCERR | INTA_LOWPRIORITYDESCOK | INTA_TIMEOUT);
*/
	write_nic_word(dev,INTA_MASK, priv->irq_mask);
}


void rtl8180_irq_disable(struct net_device *dev)
{
//	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);	

	write_nic_word(dev,INTA_MASK,0);
	force_pci_posting(dev);
//	priv->irq_enabled = 0;
}


void rtl8180_set_mode(struct net_device *dev,int mode)
{
	u8 ecmd;
	ecmd=read_nic_byte(dev, EPROM_CMD);
	ecmd=ecmd &~ EPROM_CMD_OPERATING_MODE_MASK;
	ecmd=ecmd | (mode<<EPROM_CMD_OPERATING_MODE_SHIFT);
	ecmd=ecmd &~ (1<<EPROM_CS_SHIFT);
	ecmd=ecmd &~ (1<<EPROM_CK_SHIFT);
	write_nic_byte(dev, EPROM_CMD, ecmd);
}


void rtl8180_update_msr(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	u8 msr;
	
	msr  = read_nic_byte(dev, MSR);
	msr &= ~ MSR_LINK_MASK;
	
	/* do not change in link_state != WLAN_LINK_ASSOCIATED.
	 * msr must be updated if the state is ASSOCIATING. 
	 * this is intentional and make sense for ad-hoc and
	 * master (see the create BSS/IBSS func)
	 */
	if (priv->ieee80211->state == IEEE80211_LINKED){ 
			
		if (priv->ieee80211->iw_mode == IW_MODE_INFRA)
			msr |= (MSR_LINK_MANAGED<<MSR_LINK_SHIFT);
		else if (priv->ieee80211->iw_mode == IW_MODE_ADHOC)
			msr |= (MSR_LINK_ADHOC<<MSR_LINK_SHIFT);
		else if (priv->ieee80211->iw_mode == IW_MODE_MASTER)
			msr |= (MSR_LINK_MASTER<<MSR_LINK_SHIFT);
		
	}else
		msr |= (MSR_LINK_NONE<<MSR_LINK_SHIFT);
		
	write_nic_byte(dev, MSR, msr);
}

void rtl8180_set_chan(struct net_device *dev,short ch)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	u32 tx;
	priv->chan=ch;
	#if 0
	if(priv->ieee80211->iw_mode == IW_MODE_ADHOC || 
		priv->ieee80211->iw_mode == IW_MODE_MASTER){
	
			priv->ieee80211->link_state = WLAN_LINK_ASSOCIATED;	
			priv->ieee80211->master_chan = ch;
			rtl8180_update_beacon_ch(dev); 
		}
	#endif
	
	/* this hack should avoid frame TX during channel setting*/
	tx = read_nic_dword(dev,TX_CONF);
	tx &= ~TX_LOOPBACK_MASK;

#ifndef LOOP_TEST	
	write_nic_dword(dev,TX_CONF, tx |( TX_LOOPBACK_MAC<<TX_LOOPBACK_SHIFT));

	priv->rf_set_chan(dev,priv->chan);
	mdelay(10);
	write_nic_dword(dev,TX_CONF,tx | (TX_LOOPBACK_NONE<<TX_LOOPBACK_SHIFT));
#endif
}
#if 0
void rtl8187_rx_isr(struct urb *rx_urb, struct pt_regs *regs);
#endif
void rtl8187_rx_isr(struct urb *rx_urb);
void rtl8187_rx_urbsubmit(struct net_device *dev, struct urb* rx_urb)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	int err;
	
//	u8 *rx;
#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	//DMESG("starting RX");
	/*rx = kmalloc(RX_URB_SIZE*sizeof(u8),GFP_ATOMIC);
	if(!rx){ 
		DMESGE("unable to allocate RX buffer");
		return;
	}*/
	
	usb_fill_bulk_urb(rx_urb,priv->udev,
		usb_rcvbulkpipe(priv->udev,0x81), rx_urb->transfer_buffer,
			RX_URB_SIZE,rtl8187_rx_isr,dev);
	//err = usb_submit_urb(rx_urb, GFP_ATOMIC);	
	err = usb_submit_urb(rx_urb);
	if(err && err != -EPERM){
		DMESGE("cannot submit RX command. URB_STATUS %x",rx_urb->status);
		
	}
	
}


void rtl8187_rx_initiate(struct net_device *dev)
{
	int i;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	if(!priv->rx_urb)
		DMESGE("Cannot intiate RX urb mechanism");
	for(i=0;i<MAX_RX_URB;i++) // RX_MAX_URB is 1 
		rtl8187_rx_urbsubmit(dev,priv->rx_urb[i]);
		
}

void rtl8187_set_rxconf(struct net_device *dev)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	u32 rxconf;
	
	rxconf=read_nic_dword(dev,RX_CONF);
	rxconf = rxconf &~ MAC_FILTER_MASK;
	rxconf = rxconf | (1<<ACCEPT_MNG_FRAME_SHIFT);
	rxconf = rxconf | (1<<ACCEPT_DATA_FRAME_SHIFT);
	rxconf = rxconf | (1<<ACCEPT_BCAST_FRAME_SHIFT);
	rxconf = rxconf | (1<<ACCEPT_MCAST_FRAME_SHIFT);
	rxconf = rxconf | (1<<ACCEPT_CTL_FRAME_SHIFT);	

	if (dev->flags & IFF_PROMISC) DMESG ("NIC in promisc mode");
	
	if(priv->ieee80211->iw_mode == IW_MODE_MONITOR || \
	   dev->flags & IFF_PROMISC){
		rxconf = rxconf | (1<<ACCEPT_ALLMAC_FRAME_SHIFT);
	}else{
		rxconf = rxconf | (1<<ACCEPT_NICMAC_FRAME_SHIFT);
		rxconf = rxconf | (1<<RX_CHECK_BSSID_SHIFT);
	}
	
	/*if(priv->ieee80211->iw_mode == IW_MODE_MASTER){
		rxconf = rxconf | (1<<ACCEPT_ALLMAC_FRAME_SHIFT);
		rxconf = rxconf | (1<<RX_CHECK_BSSID_SHIFT);
	}*/
	
	if(priv->ieee80211->iw_mode == IW_MODE_MONITOR){
		rxconf = rxconf | (1<<ACCEPT_ICVERR_FRAME_SHIFT);
		rxconf = rxconf | (1<<ACCEPT_PWR_FRAME_SHIFT);
	}
	
	if( priv->crcmon == 1 && priv->ieee80211->iw_mode == IW_MODE_MONITOR)
		rxconf = rxconf | (1<<ACCEPT_CRCERR_FRAME_SHIFT);
	
	
	rxconf = rxconf &~ RX_FIFO_THRESHOLD_MASK;
	rxconf = rxconf | (RX_FIFO_THRESHOLD_NONE<<RX_FIFO_THRESHOLD_SHIFT);
	
	
	rxconf = rxconf | (1<<RX_AUTORESETPHY_SHIFT);
	
	rxconf = rxconf &~ MAX_RX_DMA_MASK;
	rxconf = rxconf | (MAX_RX_DMA_2048<<MAX_RX_DMA_SHIFT);
	
	rxconf = rxconf | RCR_ONLYERLPKT;
	
//	rxconf = rxconf &~ RCR_CS_MASK;
//	rxconf = rxconf | (1<<RCR_CS_SHIFT);

	write_nic_dword(dev, RX_CONF, rxconf);	
	
	// V rtl suggested V //
//	write_nic_dword(dev, RX_CONF, 0x901ce70e);
	
	//fix_rx_fifo(dev);
// 	//set_nic_rxring(dev);
	#ifdef DEBUG_RX
	DMESG("rxconf: %x %x",rxconf ,read_nic_dword(dev,RX_CONF));
	#endif
}

void rtl8180_rx_enable(struct net_device *dev)
{
	u8 cmd;
	
	
	rtl8187_rx_initiate(dev);

	rtl8187_set_rxconf(dev);	

	cmd=read_nic_byte(dev,CMD);
	write_nic_byte(dev,CMD,cmd | (1<<CMD_RX_ENABLE_SHIFT));
#if 0
	/* In rtl8139 driver seems that DMA threshold has to be written 
	 *  after enabling RX, so we rewrite RX_CONFIG register 
	 */
	//mdelay(100);
	write_nic_dword(dev, RX_CONF, rxconf); 
	
#endif
	
}


void rtl8180_tx_enable(struct net_device *dev)
{
	u8 cmd;
	u8 byte;
	u32 txconf;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	byte = read_nic_byte(dev,CW_CONF);
	byte &= ~(1<<CW_CONF_PERPACKET_CW_SHIFT);
	byte &= ~(1<<CW_CONF_PERPACKET_RETRY_SHIFT);
	write_nic_byte(dev, CW_CONF, byte);
	
	byte = read_nic_byte(dev, TX_AGC_CTL);
	byte &= ~(1<<TX_AGC_CTL_PERPACKET_GAIN_SHIFT);
	byte &= ~(1<<TX_AGC_CTL_PERPACKET_ANTSEL_SHIFT);
	byte &= ~(1<<TX_AGC_CTL_FEEDBACK_ANT);
	write_nic_byte(dev, TX_AGC_CTL, byte);
	
	txconf= read_nic_dword(dev,TX_CONF);
	
	#if 0
	if(priv->card_8185){
		
		txconf = txconf &~ (1<<TCR_PROBE_NOTIMESTAMP_SHIFT);
	
	}else{
	
		if(priv->ieee80211->hw_seq) 
			txconf= txconf &~ (1<<TX_CONF_HEADER_AUTOICREMENT_SHIFT);
		else 
			txconf= txconf | (1<<TX_CONF_HEADER_AUTOICREMENT_SHIFT);
	}
	#endif

	txconf = txconf &~ TX_LOOPBACK_MASK;
	
#ifndef LOOP_TEST
	txconf = txconf | (TX_LOOPBACK_NONE<<TX_LOOPBACK_SHIFT);
#else
	txconf = txconf | (TX_LOOPBACK_BASEBAND<<TX_LOOPBACK_SHIFT);
#endif
	txconf = txconf &~ TCR_DPRETRY_MASK;
	txconf = txconf &~ TCR_RTSRETRY_MASK;
	
	txconf = txconf | (priv->retry_data<<TX_DPRETRY_SHIFT); // long
	txconf = txconf | (priv->retry_rts<<TX_RTSRETRY_SHIFT); // short
	
	txconf = txconf &~ (1<<TX_NOCRC_SHIFT);
	
	txconf = txconf &~ TCR_MXDMA_MASK;
	txconf = txconf | (TCR_MXDMA_2048<<TCR_MXDMA_SHIFT);
	
	txconf = txconf | TCR_CWMIN;
	txconf = txconf | TCR_DISCW;
	txconf = txconf &~ TCR_SWPLCPLEN;
	
	txconf=txconf | (1<<TX_NOICV_SHIFT);
	
	write_nic_dword(dev,TX_CONF,txconf);
	
	// V RTL suggested V //
//	write_nic_dword(dev,TX_CONF,0x00e00707);


#ifdef DEBUG_TX
	DMESG("txconf: %x %x",txconf,read_nic_dword(dev,TX_CONF));
#endif
	
	cmd=read_nic_byte(dev,CMD);
	write_nic_byte(dev,CMD,cmd | (1<<CMD_TX_ENABLE_SHIFT));		

//	mdelay(100);
	//write_nic_dword(dev,TX_CONF,txconf);
//	#endif
/*
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev, TX_DMA_POLLING, priv->dma_poll_mask);
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
	*/
}

#if 0
void rtl8180_beacon_tx_enable(struct net_device *dev)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	priv->dma_poll_mask &=~(1<<TX_DMA_STOP_BEACON_SHIFT);
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev,TX_DMA_POLLING,priv->dma_poll_mask);	
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
}


void rtl8180_
_disable(struct net_device *dev) 
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	priv->dma_poll_mask |= (1<<TX_DMA_STOP_BEACON_SHIFT);
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev,TX_DMA_POLLING,priv->dma_poll_mask);
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
}

#endif


void rtl8180_rtx_disable(struct net_device *dev)
{
	u8 cmd;
	int i;
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	cmd=read_nic_byte(dev,CMD);
	write_nic_byte(dev, CMD, cmd &~ \
		       ((1<<CMD_RX_ENABLE_SHIFT)|(1<<CMD_TX_ENABLE_SHIFT)));
	force_pci_posting(dev);
	mdelay(10);
	if(priv->rx_urb){
		for(i=0;i<MAX_RX_URB;i++)
		usb_kill_urb(priv->rx_urb[i]);
	}
	/*while (read_nic_byte(dev,CMD) & (1<<CMD_RX_ENABLE_SHIFT))
	  udelay(10); 
	*/
	
//	if(!priv->rx_skb_complete)
//		dev_kfree_skb_any(priv->rx_skb);
}


int alloc_tx_beacon_desc_ring(struct net_device *dev, int count)
{
	#if 0
	int i;
	u32 *tmp;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	priv->txbeaconring = (u32*)pci_alloc_consistent(priv->pdev,
					  sizeof(u32)*8*count, 
					  &priv->txbeaconringdma);
	if (!priv->txbeaconring) return -1;
	for (tmp=priv->txbeaconring,i=0;i<count;i++){
		*tmp = *tmp &~ (1<<31); // descriptor empty, owned by the drv 
		/*
		*(tmp+2) = (u32)dma_tmp;
		*(tmp+3) = bufsize;
		*/
		if(i+1<count)
			*(tmp+4) = (u32)priv->txbeaconringdma+((i+1)*8*4);
		else
			*(tmp+4) = (u32)priv->txbeaconringdma;
		
		tmp=tmp+8;
	}
	#endif
	return 0;
}


void rtl8180_reset(struct net_device *dev)
{
	
	u8 cr;
	
	/* make sure the analog power is on before
	 * reset, otherwise reset may fail
	 */
	rtl8180_set_anaparam(dev, RTL8225_ANAPARAM_ON);
	rtl8185_set_anaparam2(dev, RTL8225_ANAPARAM2_ON);
	
	rtl8180_irq_disable(dev);

	mdelay(200);
	write_nic_byte_E(dev,0x18,0x10);
	write_nic_byte_E(dev,0x18,0x11);
	write_nic_byte_E(dev,0x18,0x00);
	mdelay(200);
	
	cr=read_nic_byte(dev,CMD);
	cr = cr & 2;
	cr = cr | (1<<CMD_RST_SHIFT);
	write_nic_byte(dev,CMD,cr);
	
	force_pci_posting(dev);
	
	mdelay(200);
	
	if(read_nic_byte(dev,CMD) & (1<<CMD_RST_SHIFT)) 
		DMESGW("Card reset timeout!");
	else 
		DMESG("Card successfully reset");

	rtl8180_set_mode(dev,EPROM_CMD_LOAD);
	force_pci_posting(dev);
	mdelay(200);

	/* after the eeprom load cycle, make sure we have
	 * correct anaparams
	 */
	rtl8180_set_anaparam(dev, RTL8225_ANAPARAM_ON);
	rtl8185_set_anaparam2(dev, RTL8225_ANAPARAM2_ON);
	
}

inline u16 ieeerate2rtlrate(int rate)
{
	switch(rate){
	case 10:	
	return 0;
	case 20:
	return 1;
	case 55:
	return 2;
	case 110:
	return 3;
	case 60:
	return 4;
	case 90:
	return 5;
	case 120:
	return 6;
	case 180:
	return 7;
	case 240:
	return 8;
	case 360:
	return 9;
	case 480:
	return 10;
	case 540:
	return 11;
	default:
	return 3;
	
	}
}
static u16 rtl_rate[] = {10,20,55,110,60,90,120,180,240,360,480,540};
inline u16 rtl8180_rate2rate(short rate)
{
	if (rate >11) return 0;
	return rtl_rate[rate]; 
}

inline u8 rtl8180_IsWirelessBMode(u16 rate)
{
	if( ((rate <= 110) && (rate != 60) && (rate != 90)) || (rate == 220) )
		return 1;
	else return 0;
}

u16 N_DBPSOfRate(u16 DataRate)
{
	 u16 N_DBPS = 24;
	 
	 switch(DataRate)
	 {
	 case 60:
	  N_DBPS = 24;
	  break;
	 
	 case 90:
	  N_DBPS = 36;
	  break;
	 
	 case 120:
	  N_DBPS = 48;
	  break;
	 
	 case 180:
	  N_DBPS = 72;
	  break;
	 
	 case 240:
	  N_DBPS = 96;
	  break;
	 
	 case 360:
	  N_DBPS = 144;
	  break;
	 
	 case 480:
	  N_DBPS = 192;
	  break;
	 
	 case 540:
	  N_DBPS = 216;
	  break;
	 
	 default:
	  break;
	 }
	 
	 return N_DBPS;
}

u16 ComputeTxTime( 
	u16		FrameLength,
	u16		DataRate,
	u8		bManagementFrame,
	u8		bShortPreamble
)
{
	u16	FrameTime;
	u16	N_DBPS;
	u16	Ceiling;

	if( rtl8180_IsWirelessBMode(DataRate) )
	{
		if( bManagementFrame || !bShortPreamble || DataRate == 10 )
		{	// long preamble
			FrameTime = (u16)(144+48+(FrameLength*8/(DataRate/10)));		
		}
		else
		{	// Short preamble
			FrameTime = (u16)(72+24+(FrameLength*8/(DataRate/10)));
		}
		if( ( FrameLength*8 % (DataRate/10) ) != 0 ) //Get the Ceilling
				FrameTime ++;
	} else {	//802.11g DSSS-OFDM PLCP length field calculation.
		N_DBPS = N_DBPSOfRate(DataRate);
		Ceiling = (16 + 8*FrameLength + 6) / N_DBPS 
				+ (((16 + 8*FrameLength + 6) % N_DBPS) ? 1 : 0);
		FrameTime = (u16)(16 + 4 + 4*Ceiling + 6);
	}
	return FrameTime;
}		
#if 0
void rtl8187_rx_isr(struct urb *rx_urb, struct pt_regs *regs)
#endif
void rtl8187_rx_isr(struct urb *rx_urb)
{
	struct net_device *dev = (struct net_device*)rx_urb->context;
	struct r8180_priv *priv = ieee80211_priv(dev);
#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	priv->rxurb_task = rx_urb;

	if (rx_urb->status != -ENOENT) {
		tasklet_schedule(&priv->irq_rx_tasklet);
	} else {
		DMESG("RX process aborted due to explicit shutdown");
	}
}

#if 0
void rtl8180_tx_queues_stop(struct net_device *dev)
{
	//struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	u8 dma_poll_mask = (1<<TX_DMA_STOP_LOWPRIORITY_SHIFT);
	dma_poll_mask |= (1<<TX_DMA_STOP_HIPRIORITY_SHIFT);
	dma_poll_mask |= (1<<TX_DMA_STOP_NORMPRIORITY_SHIFT);
	dma_poll_mask |= (1<<TX_DMA_STOP_BEACON_SHIFT);

	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev,TX_DMA_POLLING,dma_poll_mask);
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
}
#endif

void rtl8180_data_hard_stop(struct net_device *dev)
{
	//FIXME !!
	#if 0
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	priv->dma_poll_mask |= (1<<TX_DMA_STOP_LOWPRIORITY_SHIFT);
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev,TX_DMA_POLLING,priv->dma_poll_mask);
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
	#endif
}


void rtl8180_data_hard_resume(struct net_device *dev)
{
	// FIXME !!
	#if 0
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	priv->dma_poll_mask &= ~(1<<TX_DMA_STOP_LOWPRIORITY_SHIFT);
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev,TX_DMA_POLLING,priv->dma_poll_mask);
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
	#endif
}


/* this function TX data frames when the ieee80211 stack requires this.
 * It checks also if we need to stop the ieee tx queue, eventually do it
 */
void rtl8180_hard_data_xmit(struct sk_buff *skb, struct net_device *dev, int rate)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	short morefrag = 0;	
	unsigned long flags;
	struct ieee80211_hdr *h = (struct ieee80211_hdr  *) skb->data;
#ifdef DEBUG_TX
		printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	if (le16_to_cpu(h->frame_ctl) & IEEE80211_FCTL_MOREFRAGS)
		morefrag = 1;
//	DMESG("%x %x", h->frame_ctl, h->seq_ctl);
	/*
	* This function doesn't require lock because we make
	* sure it's called with the tx_lock already acquired.
	* this come from the kernel's hard_xmit callback (trought
	* the ieee stack, or from the try_wake_queue (again trought
	* the ieee stack.
	*/
	spin_lock_irqsave(&priv->tx_lock,flags);	
			
	//DMESG("TX");
	if (!check_nic_enought_desc(dev, LOW_PRIORITY)){
		DMESG("Error: no TX slot ");
		ieee80211_stop_queue(priv->ieee80211);
	}
	
	rtl8180_tx(dev, (u32*)skb->data, skb->len, LOW_PRIORITY, morefrag,
		ieeerate2rtlrate(rate));

	priv->stats.txdatapkt++;
	
	if (!check_nic_enought_desc(dev, LOW_PRIORITY))
		ieee80211_stop_queue(priv->ieee80211);
		
	spin_unlock_irqrestore(&priv->tx_lock,flags);	
			
}
#if 0
/* This is a rough attempt to TX a frame
 * This is called by the ieee 80211 stack to TX management frames.
 * If the ring is full packet are dropped (for data frame the queue
 * is stopped before this can happen).
 */
int rtl8180_hard_start_xmit(struct sk_buff *skb,struct net_device *dev)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	int ret;
	unsigned long flags;
	
	spin_lock_irqsave(&priv->tx_lock,flags);
	
	ret = rtl8180_tx(dev, (u32*)skb->data, skb->len, NORM_PRIORITY, 0,DEFAULT_BASICRATE);
/*
	int i;
	for(i=0;i<skb->len;i++)
		printk("%x ", skb->data[i]);
	printk("--------------------\n");
*/
	priv->ieee80211->stats.tx_bytes+=skb->len;
	priv->ieee80211->stats.tx_packets++;
	
	spin_unlock_irqrestore(&priv->tx_lock,flags);	
	
	dev_kfree_skb_any(skb);
	return ret;
}
#endif

#if 0
// longpre 144+48 shortpre 72+24
u16 rtl8180_len2duration(u32 len, short rate,short* ext)
{
	u16 duration;
	u16 drift;
	*ext=0;
	
	switch(rate){
	case 0://1mbps
		*ext=0;
		duration = ((len+4)<<4) /0x2;
		drift = ((len+4)<<4) % 0x2;
		if(drift ==0 ) break;
		duration++;
		break;
		
	case 1://2mbps
		*ext=0;
		duration = ((len+4)<<4) /0x4;
		drift = ((len+4)<<4) % 0x4;
		if(drift ==0 ) break;
		duration++;
		break;
		
	case 2: //5.5mbps
		*ext=0;
		duration = ((len+4)<<4) /0xb;
		drift = ((len+4)<<4) % 0xb;
		if(drift ==0 ) 
			break;
		duration++;
		break;
		
	default:
	case 3://11mbps				
		*ext=0;
		duration = ((len+4)<<4) /0x16;
		drift = ((len+4)<<4) % 0x16;
		if(drift ==0 ) 
			break;
		duration++;
		if(drift > 6) 
			break;
		*ext=1;
		break;
	}
	
	return duration;
}
#endif

void rtl8180_try_wake_queue(struct net_device *dev, int pri);
#if 0
void rtl8187_lptx_isr(struct urb *tx_urb, struct pt_regs *regs)
#endif
void rtl8187_lptx_isr(struct urb *tx_urb)
{
	struct net_device *dev = (struct net_device*)tx_urb->context;
	struct r8180_priv *priv = ieee80211_priv(dev);
#ifdef DEBUG_TX	
	printk("%s %d\n",__FUNCTION__, __LINE__);
#endif
	if(tx_urb->status == 0)
		priv->stats.txlpokint++;
	else
		priv->stats.txlperr++;
#ifdef DEBUG_TX
	printk("%s %d txlpokint %d txlperr %d\n",__FUNCTION__, __LINE__,priv->stats.txlpokint,priv->stats.txlperr);
#endif
	kfree(tx_urb->transfer_buffer);
	usb_free_urb(tx_urb);
	atomic_dec(&priv->tx_lp_pending);
	rtl8180_try_wake_queue(dev,LOW_PRIORITY);
}


void rtl8187_beacon_stop(struct net_device *dev)
{
	u8 msr, msrm, msr2;
	msr  = read_nic_byte(dev, MSR);
	msrm = msr & MSR_LINK_MASK;
	msr2 = msr & ~MSR_LINK_MASK;
	if ((msrm == (MSR_LINK_ADHOC<<MSR_LINK_SHIFT) ||
		(msrm == (MSR_LINK_MASTER<<MSR_LINK_SHIFT)))){
		write_nic_byte(dev, MSR, msr2 | MSR_LINK_NONE);
		write_nic_byte(dev, MSR, msr);	
	}
	
	 
}


void rtl8187_net_update(struct net_device *dev)
{
	int i;
	struct r8180_priv *priv = ieee80211_priv(dev);
	struct ieee80211_network *net;
	net = & priv->ieee80211->current_network;
	
	#if 0
	write_nic_dword(dev,BSSID,((u32*)net->bssid)[0]);
	write_nic_word(dev,BSSID+4,((u16*)net->bssid)[2]);
	#endif
	for(i=0;i<ETH_ALEN;i++)
		write_nic_byte(dev,BSSID+i,net->bssid[i]);

	rtl8180_update_msr(dev);
		
//	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_word(dev, AtimWnd, 2);
	write_nic_word(dev, AtimtrItv, 100);	
	write_nic_word(dev, BEACON_INTERVAL, net->beacon_interval);
	write_nic_word(dev, BcnIntTime, 100);
	

}

void rtl8187_beacon_tx(struct net_device *dev)
{

	struct r8180_priv *priv = ieee80211_priv(dev);
	struct sk_buff *skb;
	int i = 0;
	
	rtl8187_net_update(dev);
	
	skb = ieee80211_get_beacon(priv->ieee80211);
	
	
	if(!skb){ 
		DMESG("not enought memory for allocating beacon");
		return;
	}
	
#if 0	
	while(MAX_TX_URB!=atomic_read(&priv->tx_np_pending)){
		msleep_interruptible_rtl(HZ/2);
		if(i++ > 20){
			DMESG("get stuck to wait EP3 become ready");
			return ;
		}
	}
#endif
	write_nic_byte(dev, BQREQ, read_nic_byte(dev, BQREQ) | (1<<7));
	
	i=0;
	//while(!read_nic_byte(dev,BQREQ & (1<<7)))
	while( (read_nic_byte(dev, BQREQ) & (1<<7)) == 0 )
	{
		msleep_interruptible_rtl(HZ/2);
		if(i++ > 10){
			DMESG("get stuck to wait HW beacon to be ready");
			return ;
		}
	}
		
	rtl8180_tx(dev, (u32*)skb->data, skb->len, NORM_PRIORITY,
		0, priv->ieee80211->basic_rate);
	
}
#if 0
void rtl8187_nptx_isr(struct urb *tx_urb, struct pt_regs *regs)
#endif
void rtl8187_nptx_isr(struct urb *tx_urb)
{
	struct net_device *dev = (struct net_device*)tx_urb->context;
	struct r8180_priv *priv = ieee80211_priv(dev);
	if(tx_urb->status == 0)
		priv->stats.txnpokint++;
	else
		priv->stats.txnperr++;
	kfree(tx_urb->transfer_buffer);
	usb_free_urb(tx_urb);
	atomic_dec(&priv->tx_np_pending);
	//rtl8180_try_wake_queue(dev,NORM_PRIORITY);
}


/* This function do the real dirty work: it sends a TX command 
 * descriptor plus data URB 
 */
 
short rtl8180_tx(struct net_device *dev, u32* txbuf, int len, priority_t priority,
		 short morefrag, short rate)
{
	//u32 *tx;
	u8 *tx;
//	u16 duration;
//	short ext;
	int pend ;
	int status;
	struct urb *tx_urb;
	int urb_len;
	u16	AckCtsTime;
	u16	FrameTime;
	u16 duration;	
#ifdef DEBUG_TX
		printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	struct r8180_priv *priv = ieee80211_priv(dev);
//	int rate = ieeerate2rtlrate(priv->ieee80211->rate);

	pend = atomic_read((priority == NORM_PRIORITY)? &priv->tx_np_pending : &priv->tx_lp_pending);
	/* we are locked here so the two atomic_read and inc are executed without interleaves */
	if( pend > MAX_TX_URB){
		if(priority == NORM_PRIORITY)
			priv->stats.txnpdrop++;
		else
			priv->stats.txlpdrop++;
		return -1;
	}
		
	
//	tx = kmalloc((len + 4*3), GFP_ATOMIC);
	urb_len = len + 4*3;
	if((urb_len%64 == 0)||(urb_len%512 == 0)){
  	  urb_len += 1;
	}
	
	tx = kmalloc(urb_len, GFP_ATOMIC);
	if(!tx) return -ENOMEM;
	
	//tx_urb = usb_alloc_urb(0,GFP_ATOMIC);
	tx_urb = usb_alloc_urb(0);

	if(!tx_urb){
		
		kfree(tx);
		return -ENOMEM;
	}
#if 0	
	memcpy(tx+3,txbuf,len);
	tx[0] = 0;
	tx[0] |= len & 0xfff;
	tx[0] |= (1<<15);
	
//	if(priv->shortpre)
//		tx[0] |= (1<<16);
	
	//if(len > priv->rts_threshold){
//		tx[0] |= (1<<23); //ENABLE RTS
//		tx[0] |= (1<<18); //ENABLE CTS
	//}
	if(morefrag) tx[0] |= (1<<17);
	tx[0] |= (ieeerate2rtlrate(priv->ieee80211->basic_rate) << 19); /* RTS RATE - should be basic rate */
	tx[0] |= (rate << 24);
	tx[1] = 0;
	
//	duration = rtl8180_len2duration(len,
//		rate,&ext);	
//	tx[1] |= (duration & 0x7fff) <<16;
//	if(ext) tx[1] |= (1<<31);

	
//	tx[2] = 0x303020;
	tx[2] = 3;  // CW min
	tx[2] |= (7<<4); //CW max
	tx[2] |= (11<<8);//(priv->retry_data<<8); //retry lim
	
//	printk("%x\n%x\n",tx[0],tx[1]);

	#ifdef DUMP_TX
	int i;
	printk("<Tx pkt>--rate %x---",rate);
	for (i = 0; i < (len + 3); i++)
		printk("%2x", ((u8*)tx)[i]);
	printk("---------------\n");
	#endif
#endif

	memcpy(tx+12,txbuf,len);
	tx[0] = 0;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;
	
	//tx[0] |= len & 0xfff;
	tx[0] |= len & 0xff;
	tx[1] |= (len & 0x0f00) >> 8;
	tx[1] |= (1<<7);

	if (priv->ieee80211->current_network.capability&WLAN_CAPABILITY_SHORT_PREAMBLE)
		if (priv->plcp_preamble_mode==1 && rate!=0) {	//  short mode now, not long!
			tx[2] |= 1;	

		}			

	if ( (len>priv->rts) && priv->rts && priority==LOW_PRIORITY){
			tx[2] |= (1<<7);	//enalbe RTS function
			AckCtsTime = ComputeTxTime(14, 10,0, 0);	// AckCTSLng = 14 use 1M bps send
			FrameTime = ComputeTxTime(len + 4, rtl8180_rate2rate(rate), 0, tx[2]&1);
			// RTS/CTS time is calculate as follow
			duration = FrameTime + 3*10 + 2*AckCtsTime;	//10us is the SifsTime;	
			tx[4] |= duration & 0xff; 	//Need to edit here!  ----hikaru
			tx[5] |= (duration & 0xff00) >> 8; 
			printk(KERN_INFO "duration %d!\n",duration);
	}
	else {
		tx[4] = 0;
		tx[5] = 0;
	}
	
	if(morefrag) tx[2] |= (1<<1);
	tx[2] |= (ieeerate2rtlrate(priv->ieee80211->basic_rate) << 3); /* RTS RATE - should be basic rate */
	tx[3] |= rate;

	tx[8] = 3;  // CW min
	tx[8] |= (7<<4); //CW max
	tx[9] |= 11;//(priv->retry_data<<8); //retry lim
	#ifdef DEBUG_TX
	printk("%s %d PRIORITY %d called usb function to send\n",__FUNCTION__,__LINE__,(int)priority);
	#endif
	/* FIXME check what EP is for low/norm PRI */
	usb_fill_bulk_urb(tx_urb,priv->udev,
		usb_sndbulkpipe(priv->udev,(priority == LOW_PRIORITY) ? 2:3), tx,
			urb_len, (priority == LOW_PRIORITY)?rtl8187_lptx_isr:rtl8187_nptx_isr, dev);
	//status = usb_submit_urb(tx_urb, GFP_ATOMIC);
	status = usb_submit_urb(tx_urb);
	if (!status){
		atomic_inc((priority == NORM_PRIORITY)? &priv->tx_np_pending : &priv->tx_lp_pending);
		return 0;
	}else{
	#ifdef DEBUG_TX
		printk("%s %d Error submit urb\n",__FUNCTION__,__LINE__);
	#endif

		DMESGE("Error TX URB %d, error %d",
			atomic_read((priority == NORM_PRIORITY)? &priv->tx_np_pending : &priv->tx_lp_pending),
			status);
		return -1;
	}
}

 

short rtl8187_usb_initendpoints(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int i;
	
	priv->rx_urb = (struct urb**) kmalloc (sizeof(struct urb*) * MAX_RX_URB, GFP_KERNEL);
	
	for(i=0;i<MAX_RX_URB;i++){
		//priv->rx_urb[i] = usb_alloc_urb(0,GFP_KERNEL);
		priv->rx_urb[i] = usb_alloc_urb(0);
		if(!priv->rx_urb[i]) 
			goto destroy;
		
		priv->rx_urb[i]->transfer_buffer = kmalloc(RX_URB_SIZE, GFP_KERNEL);
		if(!priv->rx_urb[i]->transfer_buffer) 
			goto destroy1;
			
		priv->rx_urb[i]->transfer_buffer_length = RX_URB_SIZE;
	}
	
	return 0;
	
destroy1:
	usb_free_urb(priv->rx_urb[i]);
	
destroy:
	while (--i >= 0){
		kfree(priv->rx_urb[i]->transfer_buffer);
		usb_free_urb(priv->rx_urb[i]);
	}
	
	kfree(priv->rx_urb);
	
	priv->rx_urb = NULL;
	DMESGE("Endpoint Alloc Failure");
	return -ENOMEM;
		
}

void rtl8187_usb_deleteendpoints(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int i;
	
	if(priv->rx_urb){
		for(i=0;i<MAX_RX_URB;i++){
			usb_kill_urb(priv->rx_urb[i]);
			kfree(priv->rx_urb[i]->transfer_buffer);
			usb_free_urb(priv->rx_urb[i]);
		}
		kfree(priv->rx_urb);
		priv->rx_urb = NULL;
		
	}
	
}


void rtl8187_set_rate(struct net_device *dev)
{
	int i;
	u16 word;
	int basic_rate,min_rr_rate,max_rr_rate;
	
//	struct r8180_priv *priv = ieee80211_priv(dev);
	
	//if (ieee80211_is_54g(priv->ieee80211->current_network) && 
//		priv->ieee80211->state == IEEE80211_LINKED){
	basic_rate = ieeerate2rtlrate(240);
	min_rr_rate = ieeerate2rtlrate(60);
	max_rr_rate = ieeerate2rtlrate(240);
	
//	
//	}else{
//		basic_rate = ieeerate2rtlrate(20);
//		min_rr_rate = ieeerate2rtlrate(10);
//		max_rr_rate = ieeerate2rtlrate(110);
//	}

	write_nic_byte(dev, RESP_RATE,
			max_rr_rate<<MAX_RESP_RATE_SHIFT| min_rr_rate<<MIN_RESP_RATE_SHIFT);

	word  = read_nic_word(dev, BRSR);
	word &= ~BRSR_MBR_8185;
		

	for(i=0;i<=basic_rate;i++)
		word |= (1<<i);

	write_nic_word(dev, BRSR, word);
	//DMESG("RR:%x BRSR: %x", read_nic_byte(dev,RESP_RATE), read_nic_word(dev,BRSR));
}


void rtl8187_link_change(struct net_device *dev)
{
//	int i;
	
	struct r8180_priv *priv = ieee80211_priv(dev);
	//write_nic_word(dev, BintrItv, net->beacon_interval);
	rtl8187_net_update(dev);
	/*update timing params*/
	rtl8180_set_chan(dev, priv->chan);

	rtl8187_set_rxconf(dev);
}

void rtl8180_irq_rx_tasklet(struct r8180_priv *priv);

short rtl8180_init(struct net_device *dev)
{
		
	struct r8180_priv *priv = ieee80211_priv(dev);
	int i, j;
	u16 word;
	int ch;
	//u16 version;
	//u8 hw_version;
	//u8 config3;
	
	//FIXME: these constants are placed in a bad pleace.

//	priv->txbuffsize = 1024;
//	priv->txringcount = 32;
//	priv->rxbuffersize = 1024;
//	priv->rxringcount = 32; 
//	priv->txbeaconcount = 3;
//	priv->rx_skb_complete = 1;
	//priv->txnp_pending.ispending=0; 
	/* ^^ the SKB does not containt a partial RXed
	 * packet (is empty)
	 */

	if(!channels){
		DMESG("No channels, aborting");
		return -1;
	}
	ch=channels;
	 // set channels 1..14 allowed in given locale
	for (i=1; i<=14; i++) {
		(priv->ieee80211->channel_map)[i] = (u8)(ch & 0x01);
		ch >>= 1;
	}
	//memcpy(priv->stats,0,sizeof(struct Stats));
	
	//priv->irq_enabled=0;
	
//	priv->stats.rxdmafail=0;
	priv->stats.txrdu=0;
//	priv->stats.rxrdu=0;
//	priv->stats.rxnolast=0;
//	priv->stats.rxnodata=0;
	//priv->stats.rxreset=0;
	//priv->stats.rxwrkaround=0;
//	priv->stats.rxnopointer=0;
	priv->stats.txnperr=0;
	priv->stats.txresumed=0;
//	priv->stats.rxerr=0;
//	priv->stats.rxoverflow=0;
//	priv->stats.rxint=0;
	priv->stats.txnpokint=0;
	/*priv->stats.txhpokint=0;
	priv->stats.txhperr=0;*/
	priv->stats.rxurberr=0;
	priv->stats.rxstaterr=0;
	priv->stats.txoverflow=0;
	priv->stats.rxok=0;
//	priv->stats.txbeaconerr=0;
	priv->stats.txlperr=0;
	priv->stats.txlpokint=0;
	
	priv->ieee80211->iw_mode = IW_MODE_INFRA;
	
	priv->retry_rts = DEFAULT_RETRY_RTS;
	priv->retry_data = DEFAULT_RETRY_DATA;
	priv->ieee80211->rate = 110; //11 mbps
	priv->ieee80211->short_slot = 1;
	priv->ieee80211->mode = IEEE_G;
	priv->promisc = (dev->flags & IFF_PROMISC) ? 1:0;
	spin_lock_init(&priv->tx_lock);
	//INIT_WORK(&priv->reset_wq,(void(*)(void*)) rtl8180_restart,dev);
	tq_init(&priv->reset_wq,(void(*)(void*)) rtl8180_restart,dev);
	sema_init(&priv->wx_sem,1);
#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	tasklet_init(&priv->irq_rx_tasklet,
		     (void(*)(unsigned long))rtl8180_irq_rx_tasklet,
		     (unsigned long)priv);

	//priv->ieee80211->func = 
	//	kmalloc(sizeof(struct ieee80211_helper_functions),GFP_KERNEL);
	//memset(priv->ieee80211->func, 0,
	  //     sizeof(struct ieee80211_helper_functions));
	priv->ieee80211->current_network.beacon_interval = DEFAULT_BEACONINTERVAL;	
	priv->ieee80211->iw_mode = IW_MODE_INFRA;
	priv->ieee80211->softmac_features  = IEEE_SOFTMAC_SCAN | 
		IEEE_SOFTMAC_ASSOCIATE | IEEE_SOFTMAC_PROBERQ | 
		IEEE_SOFTMAC_PROBERS | IEEE_SOFTMAC_TX_QUEUE |
		/*IEEE_SOFTMAC_BEACONS | */IEEE_SOFTMAC_SINGLE_QUEUE;
	
	priv->ieee80211->active_scan = 1;
	priv->ieee80211->rate = 110; //11 mbps
	priv->ieee80211->modulation = IEEE80211_CCK_MODULATION | IEEE80211_OFDM_MODULATION;
	priv->ieee80211->host_encrypt = 1;
	priv->ieee80211->host_decrypt = 1;
	priv->ieee80211->start_send_beacons = rtl8187_beacon_tx;
	priv->ieee80211->stop_send_beacons = rtl8187_beacon_stop;
	//priv->ieee80211->softmac_hard_start_xmit = rtl8180_hard_start_xmit;
	priv->ieee80211->softmac_hard_start_xmit = NULL;
	priv->ieee80211->set_chan = rtl8180_set_chan;
	priv->ieee80211->link_change = rtl8187_link_change;
	priv->ieee80211->softmac_data_hard_start_xmit = rtl8180_hard_data_xmit;
	priv->ieee80211->data_hard_stop = rtl8180_data_hard_stop;
	priv->ieee80211->data_hard_resume = rtl8180_data_hard_resume;
	//priv->ieee80211->start_send_beacons = NULL;
	//priv->ieee80211->stop_send_beacons = NULL;
	
	priv->ieee80211->fts = DEFAULT_FRAG_THRESHOLD;
	
	priv->card_8185 = 2;
	priv->phy_ver = 2;
	priv->card_type = USB;

	priv->plcp_preamble_mode = 2;
	
	#if 0
	hw_version =( read_nic_dword(dev, TCR) & TCR_HWVERID_MASK)>>TCR_HWVERID_SHIFT;
	
	switch (hw_version){
		case HW_VERID_R8185_ABC:
			DMESG("MAC controller is a RTL8185 b/g");	
			priv->card_8185 = 1;
			/* you should not find a card with 8225 PHY ver < C*/
			priv->phy_ver = 2;
			break;
			 
		case HW_VERID_R8185_D:
			DMESG("MAC controller is a RTL8185 b/g (V. D)");	
			priv->card_8185 = 2;
			/* you should not find a card with 8225 PHY ver < C*/
			priv->phy_ver = 2;
			break;
			
		case HW_VERID_R8180_ABCD:
			DMESG("MAC controller is a RTL8180");
			priv->card_8185 = 0;
			break;
		
		case HW_VERID_R8180_F:
			DMESG("MAC controller is a RTL8180 (v. F)");
			priv->card_8185 = 0;
			break;
		
		default:
			DMESGW("MAC chip not recognized: version %x. Assuming RTL8180",hw_version);
			priv->card_8185 = 0;
			break;
	}
	
	
	/* you should not found any 8185 Ver B Card */
	priv->card_8185_Bversion = 0;
	
	config3 = read_nic_byte(dev, CONFIG3);
	if(config3 & 0x8){
		priv->card_type = CARDBUS;
		DMESG("This is a CARDBUS NIC");
	}
	else if( config3 & 0x4){
		priv->card_type = MINIPCI;
		DMESG("This is a MINI-PCI NIC");
	}else{
		priv->card_type = PCI;
		DMESG("This is a PCI NIC");
	}
	#endif
	priv->enable_gpio0 = 0;

	
	/* commented out just because we already do
	   this when resetting the card
	   andrea 20050924
	 */
	#if 0
		
	u8 txcr, txreg50;
	u32 txreg54, txreg60;

	/* enable A/D D/A register */
	txcr = read_nic_byte(dev, 0x59);
//	DMESG("<txcr:%x>", txcr);
	//write_nic_word(dev, 0x59, 0x44);
	write_nic_byte(dev, 0x59, 0x44);
	//write_nic_byte(dev, 0x59, 0xea);
	txcr = read_nic_byte(dev, 0x59);
//	DMESG("<<txcr:%x>>", txcr);

        txreg50 = read_nic_byte(dev, 0x50);
  //      DMESG("<txreg50:%x>", txreg50);
        write_nic_byte(dev, 0x50, 0xc0);
        txreg50 = read_nic_byte(dev, 0x50);
    //    DMESG("<<txreg50:%x>>", txreg50);


        txreg54 = read_nic_dword(dev, 0x54);
     //   DMESG("<txreg54:%x>", txreg54);
        txreg54 = 0xa0000a59;
    //    DMESG("<<txreg54:%x>>", txreg54);
        write_nic_dword(dev, 0x54, txreg54);
        txreg54 = read_nic_dword(dev, 0x54);
   //     DMESG("<<<txreg54:%x>>>", txreg54);

        txreg60 = read_nic_dword(dev, 0x60);
     //   DMESG("<txreg60:%x>", txreg60);

        write_nic_byte(dev, 0x50, 0x0);
        txcr = read_nic_byte(dev, 0x50);
       // DMESG("<<txcr:%x>>", txcr);

#endif
	
	/*the eeprom type is stored in RCR register bit #6 */ 
	if (RCR_9356SEL & read_nic_dword(dev, RCR)){
		priv->epromtype=EPROM_93c56;
		DMESG("Reported EEPROM chip is a 93c56 (2Kbit)");
	}else{
		priv->epromtype=EPROM_93c46;
		DMESG("Reported EEPROM chip is a 93c46 (1Kbit)");
	}
	
	dev->get_stats = rtl8180_stats;
	
	dev->dev_addr[0]=eprom_read(dev,MAC_ADR) & 0xff;
	dev->dev_addr[1]=(eprom_read(dev,MAC_ADR) & 0xff00)>>8;
	dev->dev_addr[2]=eprom_read(dev,MAC_ADR+1) & 0xff;
	dev->dev_addr[3]=(eprom_read(dev,MAC_ADR+1) & 0xff00)>>8;
	dev->dev_addr[4]=eprom_read(dev,MAC_ADR+2) & 0xff;
	dev->dev_addr[5]=8/*(eprom_read(dev,MAC_ADR+2) & 0xff00)>>8 + 0x10*/;
	
	DMESG("Card MAC address is "MAC_FMT, MAC_ARG(dev->dev_addr));
		
	for(i=1,j=0; i<6; i+=2,j++){
		
		word = eprom_read(dev,EPROM_TXPW0 + j);
		priv->chtxpwr[i]=word & 0xf;
		priv->chtxpwr_ofdm[i]=(word & 0xf0)>>4;
		priv->chtxpwr[i+1]=(word & 0xf00)>>8;
		priv->chtxpwr_ofdm[i+1]=(word & 0xf000)>>12;
	}
	
	for(i=1,j=0; i<4; i+=2,j++){
			
		word = eprom_read(dev,EPROM_TXPW1 + j);
		priv->chtxpwr[i+6]=word & 0xf;
		priv->chtxpwr_ofdm[i+6]=(word & 0xf0)>>4;
		priv->chtxpwr[i+6+1]=(word & 0xf00)>>8;
		priv->chtxpwr_ofdm[i+6+1]=(word & 0xf000)>>12;
	}
	
	for(i=1,j=0; i<4; i+=2,j++){
			
		word = eprom_read(dev,EPROM_TXPW2 + j);
		priv->chtxpwr[i+6+4]=word & 0xf;
		priv->chtxpwr_ofdm[i+6+4]=(word & 0xf0)>>4;
		priv->chtxpwr[i+6+4+1]=(word & 0xf00)>>8;
		priv->chtxpwr_ofdm[i+6+4+1]=(word & 0xf000)>>12;
	}
	
		
	priv->rf_chip = 0xff & eprom_read(dev,EPROM_RFCHIPID);
	
	word = eprom_read(dev,EPROM_TXPW_BASE);
	priv->cck_txpwr_base = word & 0xf;
	priv->ofdm_txpwr_base = (word>>4) & 0xf;
	
	/* check RF frontend chipset */
	
	switch (priv->rf_chip) {
		
		case EPROM_RFCHIPID_RTL8225U:
		
		DMESG("Card reports RF frontend Realtek 8225");
		DMESGW("This driver has EXPERIMENTAL support for this chipset.");
		DMESGW("use it with care and at your own risk and");
		DMESGW("**PLEASE** REPORT SUCCESS/INSUCCESS TO andreamrl@tiscali.it");
		if(rtl8225_is_V_z2(dev)){
			priv->rf_init = rtl8225z2_rf_init;
			priv->rf_set_chan = rtl8225z2_rf_set_chan;
			priv->rf_set_sens = NULL;
			DMESG("This seems a new V2 radio");
		}else{
			priv->rf_init = rtl8225_rf_init;
			priv->rf_set_chan = rtl8225_rf_set_chan;
			priv->rf_set_sens = rtl8225_rf_set_sens;
			DMESG("This seems a legacy 1st version radio");
		}
		priv->rf_close = rtl8225_rf_close;
		
		priv->max_sens = RTL8225_RF_MAX_SENS;
		priv->sens = RTL8225_RF_DEF_SENS;
		break;
				
		default:
		DMESGW("Unknown RF module %x",priv->rf_chip);
		DMESGW("Exiting...");
		return -1;
		
	}
	
//	DMESG("Energy threshold: %x",priv->cs_treshold);
	DMESG("PAPE from CONFIG2: %x",read_nic_byte(dev,CONFIG2)&0x7);
	//DMESG("CONFIG2: %x ECONFIG2: %x",read_nic_byte(dev,CONFIG2),eprom_read(dev,EPROM_CONFIG2));
	
	if(rtl8187_usb_initendpoints(dev)!=0){ 
		DMESG("Endopoints initialization failed");
		return -ENOMEM;
	}
#if 0		
	if (0!=alloc_rx_desc_ring(dev, priv->rxbuffersize, priv->rxringcount)) 
		return -ENOMEM;

	if (0!=alloc_tx_desc_ring(dev, priv->txbuffsize, priv->txringcount,
				  TX_NORMPRIORITY_RING_ADDR))
		return -ENOMEM;

	if (0!=alloc_tx_desc_ring(dev, priv->txbuffsize, priv->txringcount,
				  TX_HIGHPRIORITY_RING_ADDR))
		return -ENOMEM;

	if (0!=alloc_tx_desc_ring(dev, priv->txbuffsize, priv->txringcount,
				  TX_LOWPRIORITY_RING_ADDR))
		return -ENOMEM;
	
	
	if (0!=alloc_tx_beacon_desc_ring(dev, priv->txbeaconcount))
		return -ENOMEM;
#endif
	

#ifdef DEBUG_EPROM
	dump_eprom(dev);
#endif 
	return 0;

}

void rtl8185_rf_pins_enable(struct net_device *dev)
{
/*	u16 tmp;
	tmp = read_nic_word(dev, RFPinsEnable);*/
	write_nic_word(dev, RFPinsEnable, 0x1ff7);// | tmp);
}


void rtl8185_set_anaparam2(struct net_device *dev, u32 a)
{
	u8 conf3;

	rtl8180_set_mode(dev, EPROM_CMD_CONFIG);

	conf3 = read_nic_byte(dev, CONFIG3);
	write_nic_byte(dev, CONFIG3, conf3 | (1<<CONFIG3_ANAPARAM_W_SHIFT));	

	write_nic_dword(dev, ANAPARAM2, a);

	conf3 = read_nic_byte(dev, CONFIG3);
	write_nic_byte(dev, CONFIG3, conf3 &~(1<<CONFIG3_ANAPARAM_W_SHIFT));

	rtl8180_set_mode(dev, EPROM_CMD_NORMAL);

}


void rtl8180_set_anaparam(struct net_device *dev, u32 a)
{
	u8 conf3;

	rtl8180_set_mode(dev, EPROM_CMD_CONFIG);

	conf3 = read_nic_byte(dev, CONFIG3);
	write_nic_byte(dev, CONFIG3, conf3 | (1<<CONFIG3_ANAPARAM_W_SHIFT));
	
	write_nic_dword(dev, ANAPARAM, a);

	conf3 = read_nic_byte(dev, CONFIG3);
	write_nic_byte(dev, CONFIG3, conf3 &~(1<<CONFIG3_ANAPARAM_W_SHIFT));

	rtl8180_set_mode(dev, EPROM_CMD_NORMAL);
	
}


void rtl8185_tx_antenna(struct net_device *dev, u8 ant)
{
	write_nic_byte(dev, TX_ANTENNA, ant); 
	force_pci_posting(dev);
	mdelay(1);
}	


void rtl8187_write_phy(struct net_device *dev, u8 adr, u32 data)
{
	//u8 phyr;
	u32 phyw;
//	int i;
	
	adr |= 0x80;
	 
	phyw= ((data<<8) | adr);
	
	
	
	// Note that, we must write 0xff7c after 0x7d-0x7f to write BB register. 
	write_nic_byte(dev, 0x7f, ((phyw & 0xff000000) >> 24));
	write_nic_byte(dev, 0x7e, ((phyw & 0x00ff0000) >> 16));
	write_nic_byte(dev, 0x7d, ((phyw & 0x0000ff00) >> 8));
	write_nic_byte(dev, 0x7c, ((phyw & 0x000000ff) ));

	//read_nic_dword(dev, PHY_ADR);
#if 0	
	for(i=0;i<10;i++){
		write_nic_dword(dev, PHY_ADR, 0xffffff7f & phyw);
		phyr = read_nic_byte(dev, PHY_READ);
		if(phyr == (data&0xff)) break;
			
	}
#endif
	/* this is ok to fail when we write AGC table. check for AGC table might be
	 * done by masking with 0x7f instead of 0xff
	 */
	//if(phyr != (data&0xff)) DMESGW("Phy write timeout %x %x %x", phyr, data, adr);
	mdelay(1);
}


inline void write_phy_ofdm (struct net_device *dev, u8 adr, u32 data)
{
	data = data & 0xff;
	rtl8187_write_phy(dev, adr, data);
}


void write_phy_cck (struct net_device *dev, u8 adr, u32 data)
{
	data = data & 0xff;
	rtl8187_write_phy(dev, adr, data | 0x10000);
}


void rtl8180_adapter_start(struct net_device *dev)
{
        struct r8180_priv *priv = ieee80211_priv(dev);
	//u32 anaparam;
	//u8 config3;
	
	//rtl8180_rtx_disable(dev);
	rtl8180_reset(dev);

	write_nic_byte(dev,0x85,0);
	write_nic_byte(dev,0x91,0);
	
	/* light blink! */
	write_nic_byte(dev,0x85,4);
	write_nic_byte(dev,0x91,1);
	write_nic_byte(dev,0x90,0);
	
	priv->irq_mask = 0xffff;
/*
	priv->dma_poll_mask = 0;
	priv->dma_poll_mask|= (1<<TX_DMA_STOP_BEACON_SHIFT);
*/	
//	rtl8180_beacon_tx_disable(dev);
	
	rtl8180_set_mode(dev, EPROM_CMD_CONFIG);
	#if 0
	write_nic_dword(dev, MAC0, ((u32*)dev->dev_addr)[0]);
	write_nic_word(dev, MAC4, ((u32*)dev->dev_addr)[1] & 0xffff );
      #endif /*hf_shi*/
	write_nic_byte(dev, MAC0, ((u8*)dev->dev_addr)[0]);
	write_nic_byte(dev, MAC1, ((u8*)dev->dev_addr)[1]);
	write_nic_byte(dev, MAC2, ((u8*)dev->dev_addr)[2]);
	write_nic_byte(dev, MAC3, ((u8*)dev->dev_addr)[3]);
	write_nic_byte(dev, MAC4, ((u8*)dev->dev_addr)[4]);
	write_nic_byte(dev, MAC5, ((u8*)dev->dev_addr)[5]);

	rtl8180_set_mode(dev, EPROM_CMD_NORMAL);
	rtl8180_update_msr(dev);
		
	rtl8180_set_mode(dev, EPROM_CMD_CONFIG);
	
	write_nic_word(dev,0xf4,0xffff);
	write_nic_byte(dev,
		       CONFIG1, (read_nic_byte(dev,CONFIG1) & 0x3f) | 0x80);	

	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
	
	write_nic_dword(dev,INT_TIMEOUT,0);	

#ifdef DEBUG_REGISTERS
	rtl8180_dump_reg(dev);	
#endif
	
	
	write_nic_byte(dev, WPA_CONFIG, 0);	

	write_nic_byte(dev, RATE_FALLBACK, 0x81);
	rtl8187_set_rate(dev);
		
	priv->rf_init(dev);	

	if(priv->rf_set_sens != NULL)
		priv->rf_set_sens(dev,priv->sens);	
	
	write_nic_word(dev,0x5e,1);

	#if 1
	//mdelay(1);
	write_nic_word(dev,0xfe,0x10);
//	mdelay(1);
	#endif
	write_nic_byte(dev, TALLY_SEL, 0x80);//Set NQ retry count

	write_nic_byte(dev, 0xff, 0x60);

	write_nic_word(dev,0x5e,0);
	
		
	rtl8180_irq_enable(dev);
	/*DMESG ("lfree %d",get_curr_tx_free_desc(dev,LOW_PRIORITY));
	
	DMESG ("nfree %d",get_curr_tx_free_desc(dev,NORM_PRIORITY));
	
	DMESG ("hfree %d",get_curr_tx_free_desc(dev,HI_PRIORITY));
	if(check_nic_enought_desc(dev,NORM_PRIORITY)) DMESG("NORM OK");
	if(check_nic_enought_desc(dev,HI_PRIORITY)) DMESG("HI OK");
	if(check_nic_enought_desc(dev,LOW_PRIORITY)) DMESG("LOW OK");*/
}



/* this configures registers for beacon tx and enables it via
 * rtl8180_beacon_tx_enable(). rtl8180_beacon_tx_disable() might
 * be used to stop beacon transmission
 */
#if 0
void rtl8180_start_tx_beacon(struct net_device *dev)
{
	int i;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	u16 word;	
	DMESG("Enabling beacon TX");
	//write_nic_byte(dev, 0x42,0xe6);// TCR
	//rtl8180_init_beacon(dev);
	//set_nic_txring(dev);
//	rtl8180_prepare_beacon(dev);
	rtl8180_irq_disable(dev);
//	rtl8180_beacon_tx_enable(dev);
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	//write_nic_byte(dev,0x9d,0x20); //DMA Poll
	//write_nic_word(dev,0x7a,0);
	//write_nic_word(dev,0x7a,0x8000);

	
	word  = read_nic_word(dev, BcnItv);
	word &= ~BcnItv_BcnItv; // clear Bcn_Itv
	write_nic_word(dev, BcnItv, word);

	write_nic_word(dev, AtimWnd, 
		       read_nic_word(dev, AtimWnd) &~ AtimWnd_AtimWnd);
	
	word  = read_nic_word(dev, BintrItv);
	word &= ~BintrItv_BintrItv;
	
	//word |= priv->ieee80211->beacon_interval * 
	//	((priv->txbeaconcount > 1)?(priv->txbeaconcount-1):1);
	// FIXME:FIXME check if correct ^^ worked with 0x3e8;
	
	write_nic_word(dev, BintrItv, word);
	
	//write_nic_word(dev,0x2e,0xe002);
	//write_nic_dword(dev,0x30,0xb8c7832e);
	for(i=0; i<ETH_ALEN; i++)
		write_nic_byte(dev, BSSID+i, priv->ieee80211->beacon_cell_ssid[i]);
	
//	rtl8180_update_msr(dev);

	
	//write_nic_byte(dev,CONFIG4,3); /* !!!!!!!!!! */
	
	rtl8180_set_mode(dev, EPROM_CMD_NORMAL);
	
	rtl8180_irq_enable(dev);
	
	/* VV !!!!!!!!!! VV*/
	/*
	rtl8180_set_mode(dev,EPROM_CMD_CONFIG);
	write_nic_byte(dev,0x9d,0x00); 	
	rtl8180_set_mode(dev,EPROM_CMD_NORMAL);
*/
}
#endif
/***************************************************************************
    -------------------------------NET STUFF---------------------------
***************************************************************************/
static struct net_device_stats *rtl8180_stats(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	return &priv->ieee80211->stats;
}


int _rtl8180_up(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	priv->up=1;
	
	//DMESG("Bringing up iface");

	rtl8180_adapter_start(dev);
	
	rtl8180_rx_enable(dev);

	rtl8180_tx_enable(dev);
	
	ieee80211_softmac_start_protocol(priv->ieee80211);
	
	ieee80211_reset_queue(priv->ieee80211);
	if(!netif_queue_stopped(dev))
		netif_start_queue(dev);
	else
		netif_wake_queue(dev);
		
	return 0;
}


int rtl8180_open(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int ret;
#ifdef CONFIG_RTL865X
	{
		int index = (char)dev->name[strlen(dev->name)-1]-'0';
		index++;
		if ((ret = rtl8187_wlanRegistration(dev, index /* WDS interface */)) != 0) {
			printk("865x flatform registration failed!\n");
			return ret;
		}
	}
#endif
	down(&priv->wx_sem);
	ret = rtl8180_up(dev);
	up(&priv->wx_sem);
	return ret;
	
}


int rtl8180_up(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	if (priv->up == 1) return -1;
	
	return _rtl8180_up(dev);
}


int rtl8180_close(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int ret;
#ifdef CONFIG_RTL865X
	{
		int index = dev->name[strlen(dev->name)-1]-'0';
		index++;
		if ((ret = rtl8187_wlanUnregistration(dev, index /* WDS interface */)) != 0) {
			printk("865x flatform registration failed!\n");
			return ret;
		}
	}
#endif	
	down(&priv->wx_sem);
	
	ret = rtl8180_down(dev);
	
	up(&priv->wx_sem);
	
	return ret;

}

int rtl8180_down(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	if (priv->up == 0) return -1;
	
	priv->up=0;

/* FIXME */
	if (!netif_queue_stopped(dev))
		netif_stop_queue(dev);
	
	rtl8180_rtx_disable(dev);
	rtl8180_irq_disable(dev);

	ieee80211_softmac_stop_protocol(priv->ieee80211);
		
	return 0;
}


void rtl8180_commit(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	if (priv->up == 0) return ;
	
	ieee80211_softmac_stop_protocol(priv->ieee80211);
	
	rtl8180_irq_disable(dev);
	rtl8180_rtx_disable(dev);
	_rtl8180_up(dev);
}

void rtl8180_restart(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	down(&priv->wx_sem);
	
	rtl8180_commit(dev);
	
	up(&priv->wx_sem);
}

static void r8180_set_multicast(struct net_device *dev)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	short promisc;

	//down(&priv->wx_sem);
	
	/* FIXME FIXME */
	
	promisc = (dev->flags & IFF_PROMISC) ? 1:0;
	
	if (promisc != priv->promisc)
	//	rtl8180_commit(dev);
	
	priv->promisc = promisc;
	
	//schedule_work(&priv->reset_wq);
	//up(&priv->wx_sem);
}


int r8180_set_mac_adr(struct net_device *dev, void *mac)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	struct sockaddr *addr = mac;
	
	down(&priv->wx_sem);
	
	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
		
	//schedule_work(&priv->reset_wq);
	schedule_task(&priv->reset_wq);

	up(&priv->wx_sem);
	
	return 0;
}


/* based on ipw2200 driver */
int rtl8180_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	down(&priv->wx_sem);
	
	struct iwreq *wrq = (struct iwreq *)rq;
        
	int ret=-1;
	switch (cmd) {
	    case RTL_IOCTL_WPA_SUPPLICANT:
		ret = ieee80211_wpa_supplicant_ioctl(priv->ieee80211, &wrq->u.data);
		break;

	    default:
		ret = -EOPNOTSUPP;
		break;
	}

	up(&priv->wx_sem);
	
	return ret;
}


void rtl8180_irq_rx_tasklet(struct r8180_priv *priv)
{
	struct urb *rx_urb = priv->rxurb_task;
	struct net_device *dev = (struct net_device*)rx_urb->context;
	int status,len,flen;
	struct sk_buff *skb;
	//u32 *desc;
	u8 *desc;
	u8 signal,quality,rate;
	
	//DMESG("rtl8187_rx_isr");
#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	struct ieee80211_rx_stats stats = {
		.signal = 0,
		.noise = -98,
		.rate = 0,
	//	.mac_time = jiffies,
		.freq = IEEE80211_24GHZ_BAND,
	};

	//DMESG("RX %d ",rx_urb->status);
	status = rx_urb->status;
	if(status == 0){

#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
//             printk("%s %d rx_urb->actual_length %d\n",__FUNCTION__,__LINE__,rx_urb->actual_length);

		len = rx_urb->actual_length;
	//	len = len - 4 - 15 - 1; /* CRC, DESC, SEPARATOR*/ 
		len -= 4*4;/* 4 dword and 4 byte CRC */
#if 0		
		desc = (u32*)(rx_urb->transfer_buffer + len);
		
		flen = desc[0] & 0xfff;
		
		if( flen <= rx_urb->actual_length){
		
			stats.signal = (desc[1] & 0x7f00)>>8;
			stats.noise = desc[1] &0xff;
			stats.rate = desc[0] >> 20 & 0xf;
			stats.mac_time[0] = desc[2];
			stats.mac_time[1] = desc[3];
		}
#endif

		desc = rx_urb->transfer_buffer + len;
		
		flen = ((desc[1] & 0x0f) << 8) + (desc[0] & 0xff);
//             printk("%s %d flen %d\n",__FUNCTION__,__LINE__,flen);

#ifdef DEBUG_RX
	printk("%s %d flen %d",__FUNCTION__,__LINE__,flen);
#endif
		if( flen <= rx_urb->actual_length){
#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
			//stats.signal = (desc[1] & 0x7f00)>>8;
			//stats.noise = desc[1] &0xff;
			signal=(desc[6]& 0xff);
			signal=(signal&0xfe)>>1;	// Modify by hikaru 6.6
				
			quality=desc[4] & 0xff;
			
			rate=(desc[2] &((1<<7)|(1<<6)|(1<<5)|(1<<4)))>>4;
		//	printk(KERN_INFO "rate is %d!\n",rate);	
			stats.rate = rtl8180_rate2rate(rate);	
		//	printk(KERN_INFO "stats.rate is %d!\n",stats.rate);
			//stats.rate = desc[0] >> 20 & 0xf;
			stats.mac_time[0] = desc[8] + (desc[9]<<8) + (desc[10]<<16) + (desc[11]<<24);   
			stats.mac_time[1] = desc[12] + (desc[13]<<8) + (desc[14]<<16) + (desc[15]<<24);   
			
			//calculate link quality begin
			if(!rtl8180_IsWirelessBMode(stats.rate) ) 
			{ // OFDM rate.
				if(signal>90)
					signal=90;
				else if(signal<25)
					signal=25;  
				signal = (90-signal)*100/65;
			}
			else
			{ // CCK rate.
				if(signal>95)
					signal = 95;
				else if(signal<30)
					signal = 30;  
				signal =(95-signal )*100/65;
			}
			priv->wstats.qual.level = signal;
		//	printk(KERN_INFO "signal is %d!\n",signal);
			if(quality > 64)
				priv ->wstats.qual.qual = 0;
			else
				priv ->wstats.qual.qual = ((64-quality) * 100) / 64;  // SQ value is the SIGNAL_QUALITY returned to IORequest, 
						//and this value only appear when STA is associated to AP or 
						// STA is in IBSS mode
		//	printk(KERN_INFO "quality is %d!\n",priv->wstats.qual.qual);
			priv->wstats.qual.noise = 100 - priv ->wstats.qual.qual;
			priv->wstats.qual.updated = 7;
			//calculate link quality end 


			skb = dev_alloc_skb(flen-4);
			//skb_reserve(skb,2);
			if(skb){ 
			memcpy(skb_put(skb,flen-4),
				rx_urb->transfer_buffer,flen -4);
			
			#ifdef DUMP_RX
			int i;
			if(flen == 34 || flen ==50 || flen==64)
			{
				for(i=0;i<flen-4;i++)
				{
					if(i % 16 == 0)
						printk("\n");
					printk("%2x ",((u8*)(rx_urb->transfer_buffer))[i]);
					
				}
			}
//			printk("------RATE %x:w---------------\n",stats.rate);
			
			#endif
			priv->stats.rxok++;
		//	priv->rxskb = skb;
		//	priv->tempstats = &stats;
 //               printk("%s %d skb->len %d\n",__FUNCTION__,__LINE__,skb->len);
			if(!ieee80211_rx(priv->ieee80211, 
				skb, &stats))
			dev_kfree_skb_any(skb);
			}
		}else priv->stats.rxurberr++;
	}else{
		priv->stats.rxstaterr++;
		priv->ieee80211->stats.rx_errors++;
		
	}
#ifdef DEBUG_RX
       printk("%s %d\n",__FUNCTION__,__LINE__);
#endif
	if(status != -ENOENT)rtl8187_rx_urbsubmit(dev,rx_urb);
	else DMESG("RX process aborted due to explicit shutdown");
}


#ifdef CONFIG_RTL865X
/* Register external device for RTL865X platform */
static int rtl8187_wlanRegistration(struct net_device *dev, int index)
{
	struct r8180_priv *priv =  ieee80211_priv(dev);
	char chipVersion[16] = {0};
      #if 0
	GetChipVersion(chipVersion, sizeof(chipVersion), NULL);
	if (chipVersion[strlen(chipVersion)-1] != 'B')
	{
		printk("This driver supports WLAN acceleration only for 865xB chips\n");
		return -EACCES;
	}
      #endif
	priv->rtl8187extPortNum = CONFIG_RTL865XB_WLAN1_PORT;
	if (priv->rtl8187linkNum[index] != 0)
	{
		printk("%s's link ID is already registered = %d.\n", dev->name, (int)(priv->rtl8187linkNum[index]));
	} else if (devglue_regExtDevice(	dev->name,
									CONFIG_RTL8185_VID,
									priv->rtl8187extPortNum,
									(unsigned long *)(&priv->rtl8187linkNum[index])) != 0)
	{
		printk("XXX Can't register a link ID for device %s on extPort 0x%x!!!\n", dev->name, priv->rtl8187extPortNum);
		priv->rtl8187extPortNum = 0;
		return -EACCES;
	} else
	{
		printk("Device %s on vlan ID %d using Link ID %d. Loopback/Ext port is %d\n", dev->name,CONFIG_RTL8185_VID, (int)(priv->rtl8187linkNum[index]), priv->rtl8187extPortNum);
	}

	return 0;
}


/* Unregister external device for RTL865X platform */
static int rtl8187_wlanUnregistration(struct net_device *dev, int index)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	#if 0
	char chipVersion[16] = {0};
	
	GetChipVersion(chipVersion, sizeof(chipVersion), NULL);
	if (chipVersion[strlen(chipVersion)-1] != 'B')
	{
		printk("This driver supports WLAN acceleration only for 865xB chips\n");
		return -EACCES;
	}
	#endif
	if ( priv->rtl8187linkNum[index] == 0 )
	{
		printk("%s's link ID already unregistered\n", dev->name);
	}else if ( devglue_unregExtDevice((unsigned long)(priv->rtl8187linkNum[index])) != 0 )
	{
		printk("XXX Can't unregister link ID %d for device %s !!!\n", (int)(priv->rtl8187linkNum[index]), dev->name);
		return -EACCES;
	}else
	{
		printk("Device %s's link ID %d unregistered.\n", dev->name, (int)(priv->rtl8187linkNum[index]));
		priv->rtl8187linkNum[index] = 0;
	}

	return 0;
}
/*if return 0 indicates that packet is consumed by fwdEngine
   if return !=-1,it should net_if RX*/
int rtl8187_wlanAccelerate(struct net_device *dev, struct sk_buff *pskb)
{
	int retval,index;
	struct r8180_priv *priv = ieee80211_priv(dev);
	if(0 == priv->rtl8187extPortNum)
	{
		return 1;/*unregistered*/
	}
#if defined(WDS) && defined(CONFIG_RTL865X)
	id = pskb->cb[0];
#endif
	index = (char)dev->name[strlen(dev->name)-1]-'0';
	index++;
	retval = rtlglue_extPortRecv(	pskb,
								pskb->data,
								pskb->len,
								CONFIG_RTL8185_VID,
								(1<<priv->rtl8187extPortNum),
								priv->rtl8187linkNum[index]);
	if (retval == 0) {
		/* 8651 fwd engine consumed the packet. */
	}
	else if (retval == -1) {
		/* exception. Drop it. */
		kfree(pskb);
	}
	return retval;
}
#endif /* CONFIG_RTL865X */

/****************************************************************************
     ---------------------------- USB_STUFF---------------------------
*****************************************************************************/

#if 0
static void * __devinit rtl8187_usb_probe(struct usb_device *dev,
	    		unsigned intf,			
	    		const struct usb_device_id *id)
#endif
static void * rtl8187_usb_probe(struct usb_device *dev,
	    		unsigned intf,			
	    		const struct usb_device_id *id)

{
//	unsigned long ioaddr = 0;
	int i; 
	struct net_device *net_dev = NULL;
	struct r8180_priv *priv= NULL;
	//struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_device *udev = dev;
//	printk("===> rtl8187_usb_probe()\n");	
	printk("rtl8187_usb_probe, intf = %d\n", intf);

	net_dev = alloc_ieee80211(sizeof(struct r8180_priv));
	
	SET_MODULE_OWNER(net_dev);
	//usb_set_intfdata(intf, dev);	
	
	//SET_NETDEV_DEV(dev, &intf->dev);

	priv = ieee80211_priv(net_dev);
	//priv->ieee80211 = netdev_priv(net_dev);
	priv->ieee80211 = (struct ieee80211_device *)net_dev->priv;
	
	priv->udev=udev;
	priv->rtl8187extPortNum =0;
	for(i=0;i<16;i++)
      		priv->rtl8187linkNum[i]=0;

	net_dev->open = rtl8180_open;
	net_dev->stop = rtl8180_close;
	//net_dev->hard_start_xmit = rtl8180_8023_hard_start_xmit;
	net_dev->tx_timeout = tx_timeout;
	net_dev->wireless_handlers = &r8180_wx_handlers_def;
	net_dev->do_ioctl = rtl8180_ioctl;
	net_dev->set_multicast_list = r8180_set_multicast;
	net_dev->set_mac_address = r8180_set_mac_adr;
	net_dev->get_wireless_stats = r8180_get_wireless_stats;
	net_dev->type=ARPHRD_ETHER;
	
	if (dev_alloc_name(net_dev, ifname) < 0){
                DMESG("Oops: devname already taken! Trying wlan%%d...\n");
		ifname = "wlan%d";
		dev_alloc_name(net_dev, ifname);
        }
	
//	net_dev->open=rtl8180_init;
	
	if(rtl8180_init(net_dev)!=0){ 
		DMESG("Initialization failed");
		goto fail;
	}
	
	netif_carrier_off(net_dev);
	netif_stop_queue(net_dev);
	
	register_netdev(net_dev);
	
	rtl8180_proc_init_one(net_dev);
	
	nat25_init();
	DMESG("Driver probe completed\n");
	//return 0;
	return net_dev;	

	
fail:
	free_ieee80211(net_dev);
		
	DMESG("wlan driver load failed\n");
	
	//return -ENODEV;
	return NULL;
	
}

#if 0
static void __devexit rtl8187_usb_disconnect(struct usb_device * dev, void * ptr)
#endif
static void rtl8187_usb_disconnect(struct usb_device * dev, void * ptr)
{
	struct r8180_priv *priv;
	//struct net_device *net_dev = usb_get_intfdata(intf);
	struct net_device *net_dev = (struct net_device *) ptr;

 	if(net_dev){
		
		unregister_netdev(net_dev);
		
		priv=ieee80211_priv(net_dev);
		
		rtl8180_proc_remove_one(net_dev);
		
		rtl8180_down(net_dev);
		priv->rf_close(net_dev);
		//rtl8180_rtx_disable(net_dev);
		rtl8187_usb_deleteendpoints(net_dev);
		rtl8180_irq_disable(net_dev);
		rtl8180_reset(net_dev);
		mdelay(10);

	}

	free_ieee80211(net_dev);
	DMESG("wlan driver removed\n");
}


static int __init rtl8187_usb_module_init(void)
{
	printk(KERN_INFO "\nLinux kernel driver for RTL8187 \
based WLAN cards\n");
	printk(KERN_INFO "Copyright (c) 2004-2005, Andrea Merello\n");
	DMESG("Initializing module");
	DMESG("Wireless extensions version %d", WIRELESS_EXT);
	rtl8180_proc_module_init();
	return usb_register(&rtl8187_usb_driver);
}


static void __exit rtl8187_usb_module_exit(void)
{
	usb_deregister(&rtl8187_usb_driver);

	rtl8180_proc_module_remove();
	DMESG("Exiting");
}


void rtl8180_try_wake_queue(struct net_device *dev, int pri)
{
	unsigned long flags;
	short enough_desc;
	struct r8180_priv *priv = (struct r8180_priv *)ieee80211_priv(dev);
	
	spin_lock_irqsave(&priv->tx_lock,flags);
	enough_desc = check_nic_enought_desc(dev,pri);
        spin_unlock_irqrestore(&priv->tx_lock,flags);	
	
	if(enough_desc)
		ieee80211_wake_queue(priv->ieee80211);
}

/*hf_shi test code*/
#ifdef  CONFIG_RTL865X

/*hf_shi. nat2.5 for test.not complete*/
/*hash functions*/
#define NAT25_HASHTBL_SIZE 32
#define NAT25_HASH_COLLISION_SIZE 1
#define HASH_IP_MASK 0x1F
#define ETH_ADRLEN 6
#define ENTRY_VALID_MASK (1<<7)
#define ENTRY_AGE_MASK 0xF
#define ENTRY_AGE(x) (x->flag & 0xF)
#define SET_ENTRY_VALID(x) ((x->flag) |= (ENTRY_VALID_MASK))
#define SET_ENTRT_INVALID(x) ((x->flag) &= (~ENTRY_VALID_MASK))

typedef struct nat25_ipMacEntry
{
  u32 ipaddr;
  u8  haddr[ETH_ADRLEN];
  u8  flag;
  u32 lastused;
  struct nat25_ipMacEntry * next;
} nat25_ipMacEntry_S;

typedef struct nat25_hashEntry
{
  nat25_ipMacEntry_S *pipMacentry;
} nat25_hashEntry_S;

nat25_hashEntry_S nat25_hashTable[NAT25_HASHTBL_SIZE];
nat25_ipMacEntry_S nat25_Entry_Global[NAT25_HASHTBL_SIZE*NAT25_HASH_COLLISION_SIZE];


u32 hash_ip(u32 ip)
{
	u32 hashval;
	hashval =ip;
	hashval ^= (hashval>>16);
	hashval ^= hashval>>8;
	hashval ^= hashval>>3;
	return hashval & HASH_IP_MASK;
}
nat25_ipMacEntry_S * find_emptyEntrybyindex(u32 index)
{
	int i;
	u32 lastused;
	nat25_ipMacEntry_S *pentry,*oldestentry;
	pentry=nat25_hashTable[index].pipMacentry;
	lastused = pentry->lastused;
	for(i=0;i<NAT25_HASH_COLLISION_SIZE;i++)
	{
		if(pentry->flag & ENTRY_VALID_MASK)
		{
			if(lastused <= pentry->lastused)
			{
				lastused= pentry->lastused;
				oldestentry=pentry;
			}
			pentry=pentry->next;
		}
		else
		{
			return pentry;
		}	
	}
	/*all Valid select the oldest*/
	SET_ENTRT_INVALID(oldestentry);
	return oldestentry;
}

nat25_ipMacEntry_S * find_matchedEntry(u32 ipaddr)
{
	u32 index;
	int i;
	nat25_ipMacEntry_S *pentry;
	index = hash_ip(ipaddr);
	//printk("find entry by %x %d\n",ipaddr,index);
	pentry=nat25_hashTable[index].pipMacentry;
	if(NULL == pentry)
		return NULL;
	for(i=0;i<NAT25_HASH_COLLISION_SIZE;i++)
	{
		//printk("%s %d %x %x",__FUNCTION__,__LINE__,ipaddr,pentry->ipaddr);
		if(0 == memcmp((void *)(&ipaddr),(void *)(&pentry->ipaddr),4))
		{
			//printk("found entry!\n");
			return pentry;
		}
		else
			pentry=pentry->next;
	}
	/*no matched*/
	//printk("not found entry!\n");
	return NULL;
}

int fill_entry(nat25_ipMacEntry_S *pentry, u32 ipaddr, u8* haddr)
{
	if(NULL == pentry)
		return -1;
	if(pentry->flag & ENTRY_VALID_MASK)
		return -1;
	//printk("%s %d %p %x",__FUNCTION__,__LINE__,pentry,ipaddr);
	memcpy((void *)(&pentry->ipaddr),(void *)(&ipaddr),4);
	memcpy(pentry->haddr,haddr,ETH_ADRLEN);
	return 0;
}
int nat25_entry_exist(u32 ipaddr,u8* haddr)
{		
	u32 index;
	int retval;
	nat25_ipMacEntry_S *pentry;
	pentry = find_matchedEntry(ipaddr);
	if(NULL == pentry)
	{
		return 0;
	}
	if((ipaddr == pentry->ipaddr) && (0== memcmp(haddr,pentry->haddr,6)))
		return 1;
	else
		return 0;
}
int nat25_record_ipmac(u32 ipaddr,u8 *haddr)
{
	u32 index;
	int retval;
	nat25_ipMacEntry_S *pentry;
	index = hash_ip(ipaddr);
	pentry = find_emptyEntrybyindex(index);
	//printk("create entry %x %d %2x:%2x:%2x:%2x:%2x:%2x\n",ipaddr,index,haddr[0],haddr[1],haddr[2],haddr[3],haddr[4],haddr[5]);
	retval = fill_entry(pentry,ipaddr,haddr);
	SET_ENTRY_VALID(pentry);
	return retval;
}

int nat25_tx_handle(struct sk_buff *skb, struct net_device *dev)
{
  int retval;
  u32 sip;
  u8 sa[6];
  /*only 802.3*/
  if(skb->data[12] == 0x08 && skb->data[13] ==0x06)
  {
    	memcpy(sa,&skb->data[6],6);
  	memcpy(&sip,&skb->data[28],4);
	/*change SMAC*/
	memcpy(&skb->data[6],dev->dev_addr,dev->addr_len);	
	/*need to change the source mac in the information field if rtl8187 won't accept the packet whose DMAC isn't his*/
	memcpy(&skb->data[22],dev->dev_addr,dev->addr_len);	
  }
  else if (skb->data[12] == 0x08 && skb->data[13] ==0x00)
  {
  	memcpy(sa,&skb->data[6],6);
  	memcpy(&sip,&skb->data[26],4);
  	/*ip packet*/
  	memcpy(&skb->data[6],dev->dev_addr,dev->addr_len);	
  } else
  {
 	printk("other packet\n");
  	return -1;
  }
  if(nat25_entry_exist(sip,sa))
  {
  	//printk("entry exist\n");
  	return 0;
  }
  retval=nat25_record_ipmac(sip,sa);
  return retval;
}
/*Rx packet from wireless lan*/
void dump_packet(struct sk_buff *skb,const char* direction)
{
	int i;
	if(skb)
	{
		printk("%s:\n",direction);
		for(i=0;i<skb->len;i++)
		{
			if(0 == (i+1)%16)
				printk("\n");
			printk("%2x ",skb->data[i]);
		}
	}
	else
		printk("%s NULL packet \n",direction);
}

int nat25_rx_handle(struct sk_buff *skb)
{
	u32 dip;
	nat25_ipMacEntry_S *pentry=NULL;

 	/*if Broadcast or Multicast packet.only need to relay it.*/
	if(skb->data[0] & 0x1)
		return 0;
	/*special deal with arp  packet*/
	if(skb->data[12] == 0x08 && skb->data[13] ==0x06)
	{
		memcpy(&dip,&skb->data[38],4);
	}else if (skb->data[12] == 0x08 && skb->data[13] ==0x00)
	{
		memcpy(&dip,&skb->data[30],4);
	} else
	{
		//printk("other packet\n");
		return -1;
	}
	pentry= find_matchedEntry(dip);
	if(NULL == pentry)
	{
		//printk("can't find matched entry\n");
		return -1;
	}
	/*change Dmac*/
	memcpy(&skb->data[0],pentry->haddr,6);
	return 0;
}


void nat25_init()
{
	int i,j;
	nat25_ipMacEntry_S *tmpentry;
	printk("nat25_init\n");
	memset((void *)nat25_hashTable,0,NAT25_HASHTBL_SIZE*sizeof(nat25_hashEntry_S));
	memset((void *)nat25_Entry_Global,0,NAT25_HASHTBL_SIZE*NAT25_HASH_COLLISION_SIZE*sizeof(nat25_ipMacEntry_S));
	tmpentry =nat25_Entry_Global;
	for(i=0;i<NAT25_HASHTBL_SIZE*NAT25_HASH_COLLISION_SIZE;i++)
	{
		tmpentry->next=tmpentry+1;
		tmpentry++;
	}
	for(i=0;i<NAT25_HASHTBL_SIZE;i++)
	{
		nat25_hashTable[i].pipMacentry =nat25_Entry_Global+i*NAT25_HASH_COLLISION_SIZE;
	}
	return;
}
void nat25_reinit()
{
	return;
}

#endif

/***************************************************************************
     ------------------- module init / exit stubs ----------------
****************************************************************************/
module_init(rtl8187_usb_module_init);
module_exit(rtl8187_usb_module_exit);
