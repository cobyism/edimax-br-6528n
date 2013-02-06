/*
* RTL8187 USB Wireless LAN Driver (for RTL865xC FT2 Only)
* Copyright (c) 2007 Realtek Semiconductor Corporation.
*
* Program : Source File of RTL8187 Driver (for RTL865xC FT2 Only)
* Abstract :
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: rtl8187.c,v 1.1.1.1 2007/08/06 10:04:45 root Exp $
*/

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include "rtl8187.h"

/* Version Information */
#define DRIVER_AUTHOR	"Michael Huang <michaelhuang@realtek.com.tw>"
#define DRIVER_DESC	"RTL8187 based USB-Wireless Lan Driver"
#define DRIVER_VERSION	"v0.0.2 (2007-02/27)"

/* Flag RTL8187_FT2: Just for RTL8187_FT2 */
#define	RTL8187_FT2
#define	DUMP_RX
#define	DUMP_TX
#define	DEBUGP(format, args...)

/* table of devices that work with this driver */
static struct usb_device_id rtl8187_table[] = {
	{ USB_DEVICE(VENDOR_ID_REALTEK, PRODUCT_ID_RTL8187) },
	{ }
};

MODULE_DEVICE_TABLE(usb, rtl8187_table);

struct rtl8187 {
	struct semaphore sem;
	struct usb_device *udev;
	struct usb_interface *interface;
	struct urb *rx_urb, *tx_urb;
	struct usb_ctrlrequest dr;
	struct net_device *ndev;
	struct net_device_stats ndev_stats;
	unsigned int flags;
//	u8 phy;

	/* EEPROM */
	u8 hw_verid;		/* 1: RTL8187L */
	u8 hw_version;
	u8 eprom_type;
	u8 rf_chip;
	u16 irq_mask;
	
#ifdef	RTL8187_FT2
	u8 ft2_flag;		/* 0x01:DUMP_RX, 0x02: DUMP_TX, 0x08: Testing */
	u8 ft2_result;		/* FT2 test result */
	u32 ft2_times;		/* FT2 test times */
	u32 ft2_target;		/* FT2 test target */
#endif
};

typedef struct rtl8187 rtl8187_t;


static void * rtl8187_probe(struct usb_device *dev, unsigned int ifnum, const struct usb_device_id *id);
static void rtl8187_disconnect(struct usb_device *dev, void *ptr);

static struct usb_driver rtl8187_driver = {
	name:		RTL8187_MODULE_NAME,
	probe:		rtl8187_probe,
	disconnect:	rtl8187_disconnect,
	id_table:		rtl8187_table,
};

/*
**	device related part of the code
*/
static int get_register(rtl8187_t * priv, u16 val, u16 idx, u16 size, void *data)
{
	return usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       val, idx, data, size, HZ / 2);
}
static int set_register(rtl8187_t * priv, u16 val, u16 idx, u16 size, void *data)
{
	return usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       val, idx, data, size, HZ / 2);
}

/* ------------------------------------------------------------------------------------------- */
u16 eprom_read(rtl8187_t *priv, u8 addr);
void eprom_set_mode(rtl8187_t *priv, u8 mode);
void dump_eprom(rtl8187_t *priv);
void rtl8180_dump_reg(rtl8187_t *priv);
void rtl8180_set_anaparam(rtl8187_t *priv, u32 addr);
void rtl8185_set_anaparam2(rtl8187_t *priv, u32 addr);

/* RTL8187 Function prototype */
static void read_bulk_callback(struct urb *urb);
static void write_bulk_callback(struct urb *urb);



/* ----------------------------------------------------------------------------------------------- */
/* kernel-to-kernel system call */
int k2kOpen(const char *path, int flags, int mode )
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_open( path, flags, mode );
	set_fs(fs);        /* recover value*/
	return ret;
}

int k2kRead(int fd, void* buf, int nbytes)
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_read( fd, buf, nbytes );
	set_fs(fs);        /* recover value*/
	return ret;
}

int k2kWrite(int fd, void* buf, int nbytes)
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_write( fd, buf, nbytes );
	set_fs(fs);        /* recover value*/
	return ret;
}

int k2kClose(int fd)
{
	int ret;
	mm_segment_t fs;
	fs = get_fs();     /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	ret = sys_close( fd );
	set_fs(fs);        /* recover value*/
	return ret;
}

/* ----------------------------------------------------------------------------------------------- */






static void eprom_ck_cycle(rtl8187_t *priv)
{
	u8 udata8 = 0;
	
	get_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	udata8 |= (1<<EPROM_CK_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	udelay(EPROM_DELAY);
	
	get_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	udata8 &= ~(1<<EPROM_CK_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	udelay(EPROM_DELAY);
}

static void eprom_w(rtl8187_t *priv, u8 bit)
{
	u8 udata8;
	
	get_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	
	if (bit)
		udata8 |= (1<<EPROM_W_SHIFT);
	else
		udata8 &= ~(1<<EPROM_W_SHIFT);
	
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	
	udelay(EPROM_DELAY);
}

static short eprom_r(rtl8187_t *priv)
{
	u8 bit;
	
	get_register(priv, REG_EEPROMCR, 0, 1, &bit);
	bit &= (1<<EPROM_R_SHIFT);
	
	udelay(EPROM_DELAY);
	
	if (bit)
		return 1;
	else
		return 0;
}

u16 eprom_read(rtl8187_t *priv, u8 addr)
{
	int i;
	u32 ret = 0;
	u8 udata8;
	
	/* Enable EPROM programming */
	udata8 = (EPROM_CMD_PROGRAM << EPROM_CMD_OPERATING_MODE_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	udelay(EPROM_DELAY);
	
	/* Enable EEPROM */
	get_register(priv, REG_EEPROMCR, 0, 1, &udata8);	
	udata8 |= (1<<EPROM_CS_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);	
	udelay(EPROM_DELAY);
	eprom_ck_cycle(priv);
	
	/* EEPROM - Read CMD (1, 1, 0) */
	eprom_w(priv, 1); eprom_ck_cycle(priv);
	eprom_w(priv, 1); eprom_ck_cycle(priv);
	eprom_w(priv, 0); eprom_ck_cycle(priv);
	
	/* EEPROM - Read ADDR (MSB First) */
	if (priv->eprom_type == EPROM_93c56) {
		/* EEPROM (93c56) 2KB */
		for (i=7; i>=0; i--) {
			eprom_w(priv, addr & (1 << i));
			eprom_ck_cycle(priv);
		}
	} else {
		/* EEPROM (93c46) 1KB */
		for (i=5; i>=0; i--) {
			eprom_w(priv, addr & (1 << i));
			eprom_ck_cycle(priv);
		}
	}
	
	/* keep chip pin D to low state while reading I'm unsure if it is
	   necessary, but anyway shouldn't hurt */
	eprom_w(priv, 0);
	
	for (i = 0; i < 16; i++) {
		/* eeprom needs a clk cycle between writing opcode &adr and
		reading data. (eeprom outs a dummy 0) */
		eprom_ck_cycle(priv);
		ret |= (eprom_r(priv) << (15 - i));
	}
	
	/* Disable EEPROM */
	get_register(priv, REG_EEPROMCR, 0, 1, &udata8);	
	udata8 &= ~(1<<EPROM_CS_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);	
	udelay(EPROM_DELAY);
	eprom_ck_cycle(priv);
	
	/* Disable EPROM programming */
	udata8 = (EPROM_CMD_NORMAL << EPROM_CMD_OPERATING_MODE_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &udata8);
	udelay(EPROM_DELAY);
	
	return ret;
}

void eprom_set_mode(rtl8187_t *priv, u8 mode)
{
	u8 ecmd;
	
	get_register(priv, REG_EEPROMCR, 0, 1, &ecmd);
	ecmd = ecmd & ~(EPROM_CMD_OPERATING_MODE_MASK);
	ecmd = ecmd | (mode << EPROM_CMD_OPERATING_MODE_SHIFT);
	ecmd = ecmd & ~(1<<EPROM_CS_SHIFT);
	ecmd = ecmd & ~(1<<EPROM_CK_SHIFT);
	set_register(priv, REG_EEPROMCR, 0, 1, &ecmd);
}

void dump_eprom(rtl8187_t *priv)
{
	int i;
	for(i=0; i<63; i++)
		printk("EEPROM addr %x : %x\n", i, eprom_read(priv,i));
}

/* this is only for debug */
void rtl8180_dump_reg(rtl8187_t *priv)
{
	int i;
	int n;
	int max=0xff;
	u8 udata8 = 0;
			
	printk("Dumping NIC register map");	
	
	for(n=0;n<=max;)
	{
		printk( "\nD: %02x> ", n);
		for(i=0;i<16 && n<=max;i++,n++) {
			get_register(priv, 0xff00 + n, 0, 1, &udata8);
			printk("%02x ", udata8);
		}
	}
	printk("\n");
}

/* =========================================================================================== */

void rtl8180_set_anaparam(rtl8187_t *priv, u32 addr)
{
	u8 conf3;

	eprom_set_mode(priv, EPROM_CMD_CONFIG);

	get_register(priv, REG_CONFIG3, 0, 1, &conf3);
	conf3 |= (1<<6);
	set_register(priv, REG_CONFIG3, 0, 1, &conf3);
	
	set_register(priv, 0xff54, 0, 4, &addr);
	
	get_register(priv, REG_CONFIG3, 0, 1, &conf3);
	conf3 &= ~(1<<6);
	set_register(priv, REG_CONFIG3, 0, 1, &conf3);

	eprom_set_mode(priv, EPROM_CMD_NORMAL);	
}

void rtl8185_set_anaparam2(rtl8187_t *priv, u32 addr)
{
	u8 conf3;

	eprom_set_mode(priv, EPROM_CMD_CONFIG);

	get_register(priv, REG_CONFIG3, 0, 1, &conf3);
	conf3 |= (1<<6);
	set_register(priv, REG_CONFIG3, 0, 1, &conf3);

	set_register(priv, 0xff60, 0, 4, &addr);

	get_register(priv, REG_CONFIG3, 0, 1, &conf3);
	conf3 &= ~(1<<6);
	set_register(priv, REG_CONFIG3, 0, 1, &conf3);
	
	eprom_set_mode(priv, EPROM_CMD_NORMAL);
}

static inline void set_ethernet_addr(rtl8187_t * priv)
{
	struct net_device *ndev = priv->ndev;
	
	((u16 *)(ndev->dev_addr))[0] = le16_to_cpu(eprom_read(priv, EPR_MACADDR + 0));
	((u16 *)(ndev->dev_addr))[1] = le16_to_cpu(eprom_read(priv, EPR_MACADDR + 1));
	((u16 *)(ndev->dev_addr))[2] = le16_to_cpu(eprom_read(priv, EPR_MACADDR + 2));
}

static int rtl8187_set_mac_address(struct net_device *ndev, void *ptr)
{
#ifdef	RTL8187_FT2
	rtl8187_t *priv = ndev->priv;
	struct sockaddr *addr = ptr;
	u8 cmd = addr->sa_data[0];
	u8 opt1 = addr->sa_data[1];
	u8 opt2 = addr->sa_data[2];
	u8 opt3 = addr->sa_data[3];
	u8 opt4 = addr->sa_data[4];
	u8 opt5 = addr->sa_data[5];
	
	if (priv == NULL)
		return -ENODEV;

	printk("rtl8187_set_mac_address(%01x:%01x:%01x:%01x:%01x:%01x);\n",
		cmd, opt1, opt2, opt3, opt4, opt5);
	
	switch(cmd) {
	case 0x00:
		rtl8180_dump_reg(priv);
		break;
	case 0x01:	/* setting dump rx flag */
		//rtl8187_dump_rx(priv);
		if (opt1) {
			printk("Dump Rx: ON\n");
			priv->ft2_flag |= 0x01;
		} else {
			printk("Dump Rx: OFF\n");
			priv->ft2_flag &= ~(0x01);
		}
		break;
	case 0x02:	/* setting dump tx flag */
		//rtl8187_dump_tx(priv);
		if (opt1) {
			printk("Dump Tx: ON\n");
			priv->ft2_flag |= 0x02;
		} else {
			printk("Dump Tx: OFF\n");
			priv->ft2_flag &= ~(0x02);
		}
		break;
	case 0xf0:	/* FT2 Flag Config */
		priv->ft2_flag = opt1;
		priv->ft2_target = opt2<<24;
		priv->ft2_target |= opt3<<16;
		priv->ft2_target |= opt4<<8;
		priv->ft2_target |= opt5;
		break;
	case 0xf1:
		printk("ft2_flag = %02x\n", priv->ft2_flag);
		printk("ft2_result = %02x\n", priv->ft2_result);
		printk("ft2_times = %08x\n", priv->ft2_times);
		printk("ft2_target = %08x\n", priv->ft2_target);
		break;
	default:
		printk("Unknow command!\n");
	}
#endif	/* RTL8187_FT2 */
#if 0	/* uncomment: FT2 don't need */	
	rtl8187_t *priv = ndev->priv;
	struct sockaddr *addr = ptr;
	
	if (netif_running(ndev))
		return -EBUSY;
	if (priv == NULL)
		return -ENODEV;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);
	dbg("%s: Setting MAC address to ", ndev->name);
	
	/* Set the IDR registers */
	{
		u32 udata32;
		eprom_set_mode(priv, EPROM_CMD_CONFIG);
		udata32 = ndev->dev_addr[0] << 0;
		udata32 |= ndev->dev_addr[1] << 8;
		udata32 |= ndev->dev_addr[2] << 16;
		udata32 |= ndev->dev_addr[3] << 24;
		udata32 = cpu_to_le32(udata32);
		set_register(priv, REG_IDR0, 0, 4, &udata32);
		udata32 = ndev->dev_addr[4] << 0;
		udata32 |= ndev->dev_addr[5] << 8;
		udata32 = cpu_to_le32(udata32);
		set_register(priv, REG_IDR4, 0, 4, &udata32);
		eprom_set_mode(priv, EPROM_CMD_NORMAL);
	}
#endif
	return 0;
}

static int rtl8187_reset(rtl8187_t * priv)
{
	u8 data = 0x00;
	int i = HZ;
	u8 udata8 = 0;

#if 0
	mdelay(200);
	udata8 = 0x10;
	set_register(dev, 0xfe18, 0, 1, &udata8);
	udata8 = 0x11;
	set_register(dev, 0xfe18, 0, 1, &udata8);
	udata8 = 0x00;
	set_register(dev, 0xfe18, 0, 1, &udata8);
	mdelay(200);
#endif 

	rtl8180_set_anaparam(priv, 0xa0000a59);		/* RTL8225_ANAPARAM_ON */
	rtl8185_set_anaparam2(priv, 0x860c7312);	/* RTL8225_ANAPARAM2_ON */

	priv->irq_mask = 0x0000;
	{	/* RTL8187 IRQ Disable */
		set_register(priv, REG_IMR, 0, 2, &priv->irq_mask);
	}
	
	udata8 = 0x10;
	get_register(priv, REG_CR, 0, 1, &udata8);
	udata8 |= (1 << 4);
	set_register(priv, REG_CR, 0, 1, &udata8);
	do {
		get_register(priv, REG_CR, 0, 1, &data);
	} while ((data & 0x10) && --i);
	
	eprom_set_mode(priv, EPROM_CMD_LOAD);
	mdelay(200);
	
	rtl8180_set_anaparam(priv, 0xa0000a59);		/* RTL8225_ANAPARAM_ON */
	rtl8185_set_anaparam2(priv, 0x860c7312);	/* RTL8225_ANAPARAM2_ON */
	
	return (i > 0) ? 0 : -1;
}

static int alloc_all_urbs(rtl8187_t * priv)
{
	/* RX_URB Alloc */
	priv->rx_urb = usb_alloc_urb(0);
	if (!priv->rx_urb) {
		return 0;
	}
	priv->rx_urb->transfer_buffer = kmalloc(RTL8187_URB_RX_MAX, GFP_KERNEL);
	if (!priv->rx_urb->transfer_buffer) {
		usb_free_urb(priv->rx_urb);
		return 0;
	}
	priv->rx_urb->transfer_buffer_length = RTL8187_URB_RX_MAX;

	/* TX_URB Alloc */
	priv->tx_urb = usb_alloc_urb(0);
	if (!priv->tx_urb) {
		kfree(priv->rx_urb->transfer_buffer);
		usb_free_urb(priv->rx_urb);
		return 0;
	}
	priv->tx_urb->transfer_buffer = kmalloc(RTL8187_URB_TX_MAX, GFP_KERNEL);
	if (!priv->tx_urb->transfer_buffer) {
		kfree(priv->rx_urb->transfer_buffer);
		usb_free_urb(priv->rx_urb);
		usb_free_urb(priv->tx_urb);
		return 0;
	}
	priv->tx_urb->transfer_buffer_length = RTL8187_URB_TX_MAX;

	return 1;
}

static void free_all_urbs(rtl8187_t * priv)
{
	kfree(priv->rx_urb->transfer_buffer);
	kfree(priv->tx_urb->transfer_buffer);
	usb_free_urb(priv->rx_urb);
	usb_free_urb(priv->tx_urb);
}

static void unlink_all_urbs(rtl8187_t * priv)
{
	usb_unlink_urb(priv->rx_urb);
	usb_unlink_urb(priv->tx_urb);
}

static void rtl8187_dump_rx(rtl8187_t * priv)
{
#ifdef	DUMP_RX
	if (priv->ft2_flag & 0x01) {
		u16 i;
		printk("[%s-RX]: ", priv->ndev->name);
		for (i = 0; i < (priv->rx_urb->actual_length); i++)
			printk("%02X ", ((u8*)priv->rx_urb->transfer_buffer)[i]);
		printk(" \n");
	}
#endif
}

static void rtl8187_dump_tx(rtl8187_t * priv)
{
#ifdef	DUMP_TX
	if (priv->ft2_flag & 0x02) {
		u16 i;
		printk("[%s-TX]: ", priv->ndev->name);
		for (i = 0; i < (priv->tx_urb->actual_length); i++)
			printk("%02X ", ((u8*)priv->tx_urb->transfer_buffer)[i]);
		printk(" \n");
	}
#endif
}

static void read_bulk_callback(struct urb *urb)
{
	rtl8187_t *priv;
	int pkt_len, res;
//	struct sk_buff *skb;
	struct net_device *ndev;
	
	DEBUGP("read_bulk_callback(stats=%d);\n", urb->status);
	
	priv = urb->context;
	if (!priv) {
		warn("!priv");
		return;
	}
	
	ndev = priv->ndev;
	if (!netif_device_present(ndev)) {
		warn("netdev is not present");
		return;
	}
	switch (urb->status) {
	case 0:
		break;
	case -ENOENT:
		return;
	case -ETIMEDOUT:
		warn("need a device reset?..");
		goto goon;
	default:
		warn("Rx status %d", urb->status);
		goto goon;
	}
	
	rtl8187_dump_rx(priv);
	
	pkt_len = urb->actual_length -30 -4 -16;	/* -30: Pseudo80211Hdr -4: FCS, -16: Descriptor */
	
#if 0	/* DONT to CPU Now */
	if (!(skb = dev_alloc_skb(urb->actual_length -30 -4 -16)))
		goto goon;
	skb->dev = ndev;
	skb_reserve(skb, 2);
	eth_copy_and_sum(skb, urb->transfer_buffer, pkt_len, 0);
	skb_put(skb, pkt_len);
	skb->protocol = eth_type_trans(skb, ndev);
	netif_rx(skb);
#endif

	priv->ndev_stats.rx_packets++;
	priv->ndev_stats.rx_bytes += (pkt_len +30 +4);	/* 802.11 Hdr, CRC32 */
	
#ifdef	RTL8187_FT2
	if (priv->ft2_flag & 0x08) {	/* in ft2 testing mode */
		/* Verification */
		if(memcmp(priv->rx_urb->transfer_buffer +30, priv->tx_urb->transfer_buffer +12 +30, pkt_len) == 0) {
			if (priv->ft2_result == 0) {
				char fpath[16] = "/var/ft2_wlanX";
				char buff[8] = "1";
				int fd1 = -1;
				fpath[13] = ndev->name[4];
				
				fd1 = k2kOpen(fpath, O_WRONLY|O_CREAT|O_TRUNC, 0644 );
				if (fd1 != -1) {
					k2kWrite(fd1, buff, 1);
					k2kClose(fd1);
				}
				priv->ft2_result = 1;	/* OK */
			}
		} else {
				char fpath[16] = "/var/ft2_wlanX";
				char buff[8] = "2";
				int fd2 = -1;
				fpath[13] = ndev->name[4];
				
				fd2 = k2kOpen(fpath, O_WRONLY|O_CREAT|O_TRUNC, 0644 );
				if (fd2 != -1) {
					k2kWrite(fd2, buff, 1);
					k2kClose(fd2);
				}
			priv->ft2_result = 2;	/* Error */
		}
	}
goon:
	usb_fill_bulk_urb(priv->rx_urb, priv->udev, usb_rcvbulkpipe(priv->udev, 0x81), 
                priv->rx_urb->transfer_buffer, RTL8187_URB_RX_MAX, read_bulk_callback, priv);
	if ((res = usb_submit_urb(priv->rx_urb)))
		warn("%s: Rx urb submission failed %d", ndev->name, res);
	
	if ((priv->ft2_flag & 0x80) && (priv->ft2_times < priv->ft2_target)) {	/* in ft2 loop mode */
		priv->ft2_times++;
		priv->tx_urb->actual_length = pkt_len +12 +30;
		usb_fill_bulk_urb(priv->tx_urb, priv->udev, usb_sndbulkpipe(priv->udev, 2),
			priv->tx_urb->transfer_buffer, pkt_len + 12 + 30, write_bulk_callback, priv);

		if ((res = usb_submit_urb(priv->tx_urb))) {
			warn("failed tx_urb %d\n", res);
			priv->ndev_stats.tx_errors++;
		} else {
			rtl8187_dump_tx(priv);
			priv->ndev_stats.tx_packets++;
			priv->ndev_stats.tx_bytes += (pkt_len +30 +4);	/* 802.11 Hdr, CRC32 */
			ndev->trans_start = jiffies;
		}
	}
#endif	/* RTL8187_FT2 */
#ifndef	RTL8187_FT2
goon:
	usb_fill_bulk_urb(priv->rx_urb, priv->udev, usb_rcvbulkpipe(priv->udev, 0x81), 
                priv->rx_urb->transfer_buffer, RTL8187_URB_RX_MAX, read_bulk_callback, priv);
	if ((res = usb_submit_urb(priv->rx_urb)))
		warn("%s: Rx urb submission failed %d", ndev->name, res);
#endif	/* !RTL8187_FT2 */
}

static void write_bulk_callback(struct urb *urb)
{
	rtl8187_t *priv = urb->context;

	DEBUGP("write_bulk_callback(stats=%d);\n", urb->status);

	if (!priv)
		return;
	if (!netif_device_present(priv->ndev))
		return;
	if (urb->status)
		info("%s: Tx status %d", priv->ndev->name, urb->status);
	
	priv->ndev->trans_start = jiffies;
	netif_wake_queue(priv->ndev);
}


/*
**	network related part of the code
*/

static int rtl8187_traffic_enable(rtl8187_t * priv)
{
	u8 udata8 = 0x00;
	
	udata8 = 0x0c;	/* Enable Rx,Tx */
	set_register(priv, REG_CR, 0, 1, &udata8);
	
	return 0;
}

static void rtl8187_traffic_disable(rtl8187_t * priv)
{
	u8 udata8 = 0x00;
	
	get_register(priv, REG_CR, 0, 1, &udata8);
	udata8 &= 0xf3;	/* Disable Rx,Tx */
	set_register(priv, REG_CR, 0, 1, &udata8);
}

static struct net_device_stats *rtl8187_netdev_stats(struct net_device *ndev)
{
	return &((rtl8187_t *) ndev->priv)->ndev_stats;
}

static void rtl8187_tx_timeout(struct net_device *ndev)
{
	rtl8187_t *priv = ndev->priv;

	if (!priv)
		return;
	
	warn("%s: Tx timeout.", ndev->name);
	
	priv->tx_urb->transfer_flags |= USB_ASYNC_UNLINK;
	usb_unlink_urb(priv->tx_urb);
	priv->ndev_stats.tx_errors++;
}

static void rtl8187_set_multicast(struct net_device *ndev)
{
	//rtl8187_t *priv = ndev->priv;
	
	netif_stop_queue(ndev);
	
	/* FIX ME */
	
	netif_wake_queue(ndev);
}

static int rtl8187_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	rtl8187_t *priv;
	int res;
	
	u32 TxDesc[3] = { 0x00000000, 0x00000000, 0x00000000 };	
	/* Pseudo 802.11 Header (2+2+6+6+6+2+6 = 30Bytes) */
	u16 FrameCtrl = 0x0000;
	u16 DurationID = 0x0000;
	u8 Addr1[6] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
	u8 Addr2[6] = { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 };
	u8 Addr3[6] = { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33 };
	u16 SeqCtrl = 0x0000;
	u8 Addr4[6] = { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 };
	/*u32 FCS = 0x00000000;*/
	
	netif_stop_queue(ndev);
	priv = ndev->priv;
	
	/* Tx Descriptor */
	TxDesc[0] = cpu_to_le32(0x00008000 | ((skb->len + 30) & 0x00000fff));
	TxDesc[1] = cpu_to_le32(0x00000000);
	TxDesc[2] = cpu_to_le32((3 | (7<<4) | (11<<8)));
	
	/* Packet Pack */
	memset(priv->tx_urb->transfer_buffer, 0, RTL8187_URB_TX_MAX);
	memcpy(priv->tx_urb->transfer_buffer + 0, &TxDesc[0], 4);
	memcpy(priv->tx_urb->transfer_buffer + 4, &TxDesc[1], 4);
	memcpy(priv->tx_urb->transfer_buffer + 8, &TxDesc[2], 4);
	memcpy(priv->tx_urb->transfer_buffer + 12, &FrameCtrl, 2);
	memcpy(priv->tx_urb->transfer_buffer + 14, &DurationID, 2);
	memcpy(priv->tx_urb->transfer_buffer + 16, &Addr1, 6);
	memcpy(priv->tx_urb->transfer_buffer + 22, &Addr2, 6);
	memcpy(priv->tx_urb->transfer_buffer + 28, &Addr3, 6);
	memcpy(priv->tx_urb->transfer_buffer + 34, &SeqCtrl, 2);
	memcpy(priv->tx_urb->transfer_buffer + 36, &Addr4, 6);
	memcpy(priv->tx_urb->transfer_buffer + 42, skb->data, skb->len);
	priv->tx_urb->actual_length = skb->len +12 +30;	/* Tx Descriptor, Pseudo80211Hdr */
	
	usb_fill_bulk_urb(priv->tx_urb, priv->udev, usb_sndbulkpipe(priv->udev, 2),
			priv->tx_urb->transfer_buffer, skb->len + 12 + 30, write_bulk_callback, priv);
	
	if ((res = usb_submit_urb(priv->tx_urb))) {
		warn("failed tx_urb %d\n", res);
		priv->ndev_stats.tx_errors++;
		netif_start_queue(ndev);
	} else {
		rtl8187_dump_tx(priv);
		priv->ndev_stats.tx_packets++;
		priv->ndev_stats.tx_bytes += (skb->len +30 +4);	/* 802.11 Hdr, CRC32 */
		ndev->trans_start = jiffies;
	}
	dev_kfree_skb(skb);

	return 0;
}

static int rtl8187_open(struct net_device *ndev)
{
	rtl8187_t *priv = ndev->priv;
	int res;

	if (priv == NULL) {
		return -ENODEV;
	}

	down(&priv->sem);
	
	rtl8187_reset(priv);
	
	{	/* RF & GPIO */
		u8 udata8 = 0;
		
		udata8 = 0;	/* RF Pins Select */
		set_register(priv, 0xff85, 0, 1, &udata8);
		udata8 = 0;	/* GPIO Disable */
		set_register(priv, 0xff91, 0, 1, &udata8);
		
		udata8 = 4;	/* RF Pins Select */
		set_register(priv, 0xff85, 0, 1, &udata8);
		udata8 = 1;	/* GPIO Enable */
		set_register(priv, 0xff91, 0, 1, &udata8);
		udata8 = 0;	/* GPIO Output */
		set_register(priv, 0xff90, 0, 1, &udata8);
	}
	
	{	/* Setting Mac Address */
		u32 udata32 = 0;
		
		eprom_set_mode(priv, EPROM_CMD_CONFIG);
		
		udata32 = ndev->dev_addr[0] << 0;
		udata32 |= ndev->dev_addr[1] << 8;
		udata32 |= ndev->dev_addr[2] << 16;
		udata32 |= ndev->dev_addr[3] << 24;
		udata32 = cpu_to_le32(udata32);
		set_register(priv, REG_IDR0, 0, 4, &udata32);
		udata32 = ndev->dev_addr[4] << 0;
		udata32 |= ndev->dev_addr[5] << 8;
		udata32 = cpu_to_le32(udata32);
		set_register(priv, REG_IDR4, 0, 4, &udata32);
		
		eprom_set_mode(priv, EPROM_CMD_NORMAL);
	}

	{	/* Update MSR */
		u8 msr;

		get_register(priv, REG_MSR, 0, 1, &msr);
		msr &= ~( (1<<2) | (1<<3));
		msr |= (2<<2);	/* Link OK on Infrastructure network */
		set_register(priv, REG_MSR, 0, 1, &msr);
	}

	{	/* CONFIG1: LED Config */
		u16 udata16 = 0xffff;
		u8 udata8 = 0;
		
		eprom_set_mode(priv, EPROM_CMD_CONFIG);
		
		set_register(priv, 0xfff4, 0, 2, &udata16);
		
		get_register(priv, REG_CONFIG1, 0, 1, &udata8);
		udata8 = ((udata8 & 0x3f) | 0x80);
		set_register(priv, REG_CONFIG1, 0, 1, &udata8);
		
		eprom_set_mode(priv, EPROM_CMD_NORMAL);		
	}

	{	/* Timer Interrupt Register */
		u32 udata32 = 0;
		
		set_register(priv, REG_TIMERINT, 0, 4, &udata32);
	}

	{	/* WPA CONFIG */
		u8 udata8 = 0;

		set_register(priv, REG_WPACONFIG, 0, 1, &udata8);
	}

	{	/* RATE_FALLBACK */
		u8 udata8 = 0x81;	/* Enable Auto Rate Fallback */

		set_register(priv, REG_RFC, 0, 1, &udata8);

		udata8 = (8<<4) | (4<<0);
		set_register(priv, REG_RR, 0, 1, &udata8);

		get_register(priv, REG_BRSR, 0, 1, &udata8);
		udata8 &= ~(((1<< 11)|(1<< 10)|(1<< 9)|(1<< 8)|(1<< 7)|(1<< 6)|(1<< 5)|(1<< 4)|(1<< 3)|(1<< 2)|(1<< 1)|(1<< 0)));
		udata8 |= (((1<< 8)|(1<< 7)|(1<< 6)|(1<< 5)|(1<< 4)|(1<< 3)|(1<< 2)|(1<< 1)|(1<< 0)));
		set_register(priv, REG_BRSR, 0, 1, &udata8);
	}

/* RF_INIT(DEV); */

	{	/* PSR: Page set to 1 */
		u16 udata16 = 0;
		u8 udata8 = 0;

		udata16 = 0x0001;		/* PSR: Page set to 1 */
		udata16 = cpu_to_le16(udata16);
		set_register(priv, REG_PSR, 0, 2, &udata16);
		udata16 = 0x0010;
		udata16 = cpu_to_le16(udata16);
		set_register(priv, 0xfffe, 0, 2, &udata16);

		udata8 = 0x80;
		set_register(priv, REG_TSEL, 0, 1, &udata8);
		udata8 = 0x60;
		set_register(priv, 0xffff, 0, 1, &udata8);

		udata16 = 0x0000;
		udata16 = cpu_to_le16(udata16);
		set_register(priv, REG_PSR, 0, 2, &udata16);
	}

	priv->irq_mask = cpu_to_le16(0xffff);
	{	/* RTL8187 IRQ Enable */
		set_register(priv, REG_IMR, 0, 2, &priv->irq_mask);
	}


	/* Rx,Tx Config */
	{ /* RCR Config */
		u32 udata32 = 0;

		//udata32 = 0x90fc102f;
		udata32 = cpu_to_le32(0xe07c102f);
		set_register(priv, REG_RCR, 0, 4, &udata32);
	}
	{ /* TCR Config */
		u32 udata32 = 0;

		//udata32 = 0x18fa0707;
		udata32 = cpu_to_le32(0x98fa0707);	/* CRC32 (4Bytes) appended */
		//udata32 = 0x98fb0707;	/* NO CRC32 (4Bytes) appended */
		set_register(priv, REG_TCR, 0, 4, &udata32);
	}
	
	usb_fill_bulk_urb(priv->rx_urb, priv->udev, usb_rcvbulkpipe(priv->udev, 0x81), 
                priv->rx_urb->transfer_buffer, RTL8187_URB_RX_MAX, read_bulk_callback, priv);
	if ((res = usb_submit_urb(priv->rx_urb)))
		warn("%s: rx_urb submit failed: %d", __FUNCTION__, res);
	
	netif_start_queue(ndev);
	rtl8187_traffic_enable(priv);
	
	up(&priv->sem);

	return res;
}

static int rtl8187_close(struct net_device *ndev)
{
	rtl8187_t *priv = ndev->priv;
	int res = 0;

	if (!priv)
		return -ENODEV;
	
	down(&priv->sem);
	netif_stop_queue(ndev);
	priv->irq_mask = 0x0000;
	{	/* RTL8187 IRQ Disable */
		set_register(priv, REG_IMR, 0, 2, &priv->irq_mask);
	}
	
	{	/* Update MSR */
		u8 msr;

		get_register(priv, REG_MSR, 0, 1, &msr);
		msr &= ~( (1<<2) | (1<<3));
		msr |= (0<<2);	/* No Link */
		set_register(priv, REG_MSR, 0, 1, &msr);
	}
	
	if (!test_bit(RTL8187_UNPLUG, &priv->flags))
		rtl8187_traffic_disable(priv);
	
	unlink_all_urbs(priv);
	up(&priv->sem);

	return res;
}

#if 0
static int rtl8187_ethtool_ioctl(struct net_device *ndev, void *uaddr)
{
	rtl8187_t *dev;
	int cmd;
	char tmp[128];

	dev = ndev->priv;
	if (get_user(cmd, (int *) uaddr))
		return -EFAULT;

	switch (cmd) {
	case ETHTOOL_GDRVINFO:{
			struct ethtool_drvinfo info = { ETHTOOL_GDRVINFO };

			strncpy(info.driver, DRIVER_DESC, ETHTOOL_BUSINFO_LEN);
			strncpy(info.version, DRIVER_VERSION,
				ETHTOOL_BUSINFO_LEN);
			sprintf(tmp, "usb%d:%d", dev->udev->bus->busnum,
				dev->udev->devnum);
			strncpy(info.bus_info, tmp, ETHTOOL_BUSINFO_LEN);
			if (copy_to_user(uaddr, &info, sizeof(info)))
				return -EFAULT;
			return 0;
		}
	case ETHTOOL_GSET:{
			struct ethtool_cmd ecmd;
			short lpa, bmcr;

			if (copy_from_user(&ecmd, uaddr, sizeof(ecmd)))
				return -EFAULT;
			ecmd.supported = (SUPPORTED_10baseT_Half |
					  SUPPORTED_10baseT_Full |
					  SUPPORTED_100baseT_Half |
					  SUPPORTED_100baseT_Full |
					  SUPPORTED_Autoneg |
					  SUPPORTED_TP | SUPPORTED_MII);
			ecmd.port = PORT_TP;
			ecmd.transceiver = XCVR_INTERNAL;
			ecmd.phy_address = dev->phy;
			get_registers(dev, BMCR, 2, &bmcr);
			get_registers(dev, ANLP, 2, &lpa);
			if (bmcr & BMCR_ANENABLE) {
				ecmd.autoneg = AUTONEG_ENABLE;
				ecmd.speed =
				    (lpa & (LPA_100HALF | LPA_100FULL)) ?
				    SPEED_100 : SPEED_10;
				if (ecmd.speed == SPEED_100)
					ecmd.duplex = (lpa & LPA_100FULL) ?
					    DUPLEX_FULL : DUPLEX_HALF;
				else
					ecmd.duplex = (lpa & LPA_10FULL) ?
					    DUPLEX_FULL : DUPLEX_HALF;
			} else {
				ecmd.autoneg = AUTONEG_DISABLE;
				ecmd.speed = (bmcr & BMCR_SPEED100) ?
				    SPEED_100 : SPEED_10;
				ecmd.duplex = (bmcr & BMCR_FULLDPLX) ?
				    DUPLEX_FULL : DUPLEX_HALF;
			}
			if (copy_to_user(uaddr, &ecmd, sizeof(ecmd)))
				return -EFAULT;
			return 0;
		}
	case ETHTOOL_SSET:
		return -ENOTSUPP;
	case ETHTOOL_GLINK:{
			struct ethtool_value edata = { ETHTOOL_GLINK };

			edata.data = netif_carrier_ok(ndev);
			if (copy_to_user(uaddr, &edata, sizeof(edata)))
				return -EFAULT;
			return 0;
		}
	default:
		return -EOPNOTSUPP;
	}
}
#endif

static int rtl8187_ioctl(struct net_device *ndev, struct ifreq *rq, int cmd)
{
	rtl8187_t *priv = ndev->priv;
	u16 *data;
	int res;

	data = (u16 *) & rq->ifr_data;
	res = 0;

	down(&priv->sem);
	switch (cmd) {
/*
	case SIOCETHTOOL:
		res = rtl8187_ethtool_ioctl(ndev, rq->ifr_data);
		break;
*/
	default:
		res = -EOPNOTSUPP;
	}
	up(&priv->sem);

	return res;
}

static void *rtl8187_probe(struct usb_device *udev, unsigned int ifnum,
			   const struct usb_device_id *id)
{
	rtl8187_t *priv;
	struct net_device *ndev;
	u32 udata32 = 0;
	
	udev->config[0].bConfigurationValue = 1;
	if (usb_set_configuration(udev, udev->config[0].bConfigurationValue)) {
		err("usb_set_configuration() failed");
		return NULL;
	}
	
	priv = kmalloc(sizeof(rtl8187_t), GFP_KERNEL);
	if (!priv) {
		err("Out of memory");
		goto exit;
	} else {
		memset(priv, 0, sizeof(rtl8187_t));
	}
	
	ndev = init_etherdev(NULL, 0);
	if (!ndev) {
		kfree(priv);
		err("Oh boy, out of memory again?!?");
		priv = NULL;
		goto exit;
	}
	
	init_MUTEX(&priv->sem);
	priv->udev = udev;
	priv->ndev = ndev;
#ifdef	RTL8187_FT2
	priv->ft2_flag |= 0x08;	/* Testing mode On */
#endif	/* RTL8187_FT2 */
	SET_MODULE_OWNER(ndev);
	ndev->priv = priv;
	ndev->open = rtl8187_open;
	ndev->stop = rtl8187_close;
	ndev->do_ioctl = rtl8187_ioctl;
	ndev->watchdog_timeo = RTL8187_TX_TIMEOUT;
	ndev->tx_timeout = rtl8187_tx_timeout;
	ndev->hard_start_xmit = rtl8187_start_xmit;
	ndev->set_multicast_list = rtl8187_set_multicast;
	ndev->set_mac_address = rtl8187_set_mac_address;
	ndev->get_stats = rtl8187_netdev_stats;
	ndev->mtu = RTL8187_MTU;
	strcpy(ndev->name, "wlan%d");
	
	set_ethernet_addr(priv);
	
	if (!alloc_all_urbs(priv)) {
		err("couldn't alloc_all_urbs");
		free_all_urbs(priv);
		unregister_netdev(ndev);
		kfree(ndev);
		kfree(priv);
		priv = NULL;
		goto exit;
	}

	get_register(priv, REG_TCR, 0, 4, &udata32);
	priv->hw_version = (udata32 & 0x0e000000) >> 25;
	get_register(priv, REG_RCR, 0, 4, &udata32);
	priv->eprom_type = udata32 & (1<<6);
	
	register_netdev(ndev);

	info("%s: rtl8187 is detected", ndev->name);
	
exit:
	return priv;
}

static void rtl8187_disconnect(struct usb_device *udev, void *ptr)
{
	rtl8187_t *priv = ptr;
	
	printk("%s: rtl8187 is removed", priv->ndev->name);
	set_bit(RTL8187_UNPLUG, &priv->flags);
	unregister_netdev(priv->ndev);
	unlink_all_urbs(priv);
	free_all_urbs(priv);
	kfree(priv->ndev);
	kfree(priv);
	priv->ndev = NULL;
	priv = NULL;
}

static int __init usb_rtl8187_init(void)
{
	/* delay for USB initialize ready */
	u32 delay = 50000000;
	while(delay>0) {delay--;}

	info(DRIVER_DESC " " DRIVER_VERSION);
	info(DRIVER_AUTHOR);
	
	info("usb_rtl8187_init();");
	return usb_register(&rtl8187_driver);
}

static void __exit usb_rtl8187_exit(void)
{
	usb_deregister(&rtl8187_driver);
	info("usb_rtl8187_exit();");
}

module_init(usb_rtl8187_init);
module_exit(usb_rtl8187_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

