/*
################################################################################
# 
# Copyright(c) Realtek Semiconductor Corp. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation; either version 2 of the License, or (at your option) 
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
# The full GNU General Public License is included in this distribution in the
# file called LICENSE.
# 
################################################################################
*/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/irq.h>

#include "r8168.h"

#include "rtl_ioctl.h"

static u32 rtl8168_vpd_read(struct net_device *dev, unsigned int read_addr);
static void rtl8168_vpd_write(struct net_device *dev, unsigned int write_addr, u32 write_data);
static int rtl8168_eeprom_size(struct net_device *dev);
static u32 rtl8168_eeprom_read_dword(struct net_device *dev, unsigned int read_addr);
static void rtl8168_eeprom_write_dword(struct net_device *dev, u32 eeprom_cont, unsigned int write_addr);
static int rtl8168_eeprom_read(struct net_device *dev, u32 *eeprom_cont, int eeprom_size);
static void rtl8168_eeprom_write(struct net_device *dev, u32 *eeprom_cont, int eeprom_size);
void rtl8168_set_loopback(struct net_device *dev, int type, int state);
int rtl8168_loopback_test(struct net_device *dev);
static int rtl8168_interrupt_test(struct net_device *dev, u32 *isr_content);
static int rtl8168_report_device_type(struct net_device *dev, char *device_type);
static int rtl8168_report_device_id(struct net_device *dev, char *device_id);
int rtl8168_priv_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);


static u32
rtl8168_vpd_read(struct net_device *dev, unsigned int read_addr)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;

	u32 tmp;
	int vpd_addr = VPD_ADDRESS;
	int vpd_data = VPD_DATA;

	read_addr |= VPD_READ;
	pci_write_config_word(pdev, vpd_addr, read_addr);
	udelay(10000);
	pci_read_config_dword(pdev, vpd_data, &tmp);

	return tmp;
}

static void
rtl8168_vpd_write(struct net_device *dev, unsigned int write_addr, u32 write_data)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;

	int vpd_addr = VPD_ADDRESS;
	int vpd_data = VPD_DATA;

	write_addr |= VPD_WRITE;
	pci_write_config_dword(pdev, vpd_data, write_data);
	pci_write_config_word(pdev, vpd_addr, write_addr);
	udelay(10000);
}

static int
rtl8168_eeprom_size(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int eeprom_size;
	u32 offset_0x00;
	u32 offset_0x100;
	u32 test_cont1;

	if (RTL_R32(RxConfig) & EEPROM_9356) {
		offset_0x00 = rtl8168_eeprom_read_dword(dev, 0x00);
		offset_0x100 = rtl8168_eeprom_read_dword(dev, 0x100);

		rtl8168_eeprom_write_dword(dev, 0x56565656, 0x8000);
		rtl8168_eeprom_write_dword(dev, 0x66666666, 0x8100);

		test_cont1 = rtl8168_eeprom_read_dword(dev, 0x00);
		printk("test_cont1: %X\n", test_cont1);

		if (test_cont1 == 0x66666666)
			eeprom_size = EEPROM_SZ_9356;
		else if (test_cont1 == 0x56565656)
			eeprom_size = EEPROM_SZ_9366;
		else
			eeprom_size = EEPROM_SZ_UNKNOWN;

		rtl8168_eeprom_write_dword(dev, offset_0x00, 0x8000);
		rtl8168_eeprom_write_dword(dev, offset_0x100, 0x8100);
	} else {
		eeprom_size = EEPROM_SZ_9346;
	}

	return eeprom_size;
}

static u32
rtl8168_eeprom_read_dword(struct net_device *dev, unsigned int read_addr)
{
	return rtl8168_vpd_read(dev, read_addr);
}

static void
rtl8168_eeprom_write_dword(struct net_device *dev, u32 eeprom_cont, unsigned int write_addr)
{
	rtl8168_vpd_write(dev, write_addr, eeprom_cont);
}


static int
rtl8168_eeprom_read(struct net_device *dev, u32 *eeprom_cont, int eeprom_size)
{
	int i;
	unsigned int read_addr;

	for (i = 0, read_addr = 0; i < eeprom_size / 4; i++)
		*(eeprom_cont + i) = rtl8168_vpd_read(dev, read_addr + i * 4);

	return 0;
}

static void
rtl8168_eeprom_write(struct net_device *dev, u32 *eeprom_cont, int eeprom_size)
{
	int i;
	unsigned int write_addr = 0x00;

	for (i = 0; i < eeprom_size / 4; i++)
		rtl8168_vpd_write(dev, write_addr + i * 4, *(eeprom_cont + i));
}

void 
rtl8168_set_loopback(struct net_device *dev, int type, int state)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int i;

	if (state == LOOPBACK_ON) {
		tp->loopback_tx_ok = 0;
		tp->loopback_rx_ok = 0;
		tp->loopback_tx_err = 0;
		tp->loopback_rx_err = 0;
	}

	if ((type == MAC_LOOPBACK) && (state == LOOPBACK_ON)) {
		//turn on MAC loopback
		RTL_W32(TxConfig, RTL_R32(TxConfig) | TxMACLoopBack);

		tp->loopback_tx_schedule = LOOPBACK_TX_SCHEDULE_ON;
		tp->loopback_state = LOOPBACK_ON;
		tp->loopback_type = MAC_LOOPBACK;

		tasklet_schedule(&tp->loopback_tasklet);
	} else if ((type == MAC_LOOPBACK) && (state == LOOPBACK_OFF)) {
		tp->loopback_tx_schedule = LOOPBACK_TX_SCHEDULE_OFF;

		//turn off MAC loopback
		RTL_W32(TxConfig, RTL_R32(TxConfig) & ~TxMACLoopBack);

		tp->loopback_state = LOOPBACK_OFF;
	} else if ((type == PHY_LOOPBACK) && (state == LOOPBACK_ON)) {
		//turn on PHY loopback
		RTL_W32(PHYAR, PHY_LB_ON);

		//Since PHY is analog, waite for a while to link.
		for (i = 0; i < 10000; i++)
			if (RTL_R8(PHYstatus) & LinkStatus)
				break;

		for (i = 0; i < 50 ; i++)
			udelay(10000);

		tp->loopback_tx_schedule = LOOPBACK_TX_SCHEDULE_ON;
		tp->loopback_state = LOOPBACK_ON;
		tp->loopback_type = PHY_LOOPBACK;

		tasklet_schedule(&tp->loopback_tasklet);
	} else if ((type == PHY_LOOPBACK) && (state == LOOPBACK_OFF)) {	
		tp->loopback_tx_schedule = LOOPBACK_TX_SCHEDULE_OFF;

		//turn off PHY loopback
		RTL_W32(PHYAR, PHY_LB_OFF);

		udelay(10000);

		tp->loopback_state = LOOPBACK_OFF;
	}
}

int 
rtl8168_loopback_test(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct sk_buff *skb = NULL;
	unsigned char lp_des_mac[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char lp_protocol[MAC_PROTOCOL_LEN] = {0x09, 0x00};

	skb = dev_alloc_skb(tp->loopback_pkt_payload_len + MAC_ADDR_LEN * 2 + MAC_PROTOCOL_LEN);
	if (!skb) {
		printk("skb allocation fail.\n");
		return -ENOMEM;
	}
	tp->loopback_skb = skb;

	skb_reserve(skb, MAC_ADDR_LEN * 2 + MAC_PROTOCOL_LEN);//reserve the space for MAC header
	memcpy(skb->data, tp->loopback_pkt_payload, tp->loopback_pkt_payload_len);//copy the payload of packet
	skb_put(skb, tp->loopback_pkt_payload_len);
	skb_push(skb, MAC_PROTOCOL_LEN);
	memcpy(skb->data, lp_protocol, MAC_PROTOCOL_LEN);//copy the protocol
	skb_push(skb, MAC_ADDR_LEN);
	memcpy(skb->data, dev->dev_addr, MAC_ADDR_LEN);//copy the src MAC address
	skb_push(skb, MAC_ADDR_LEN);
	memcpy(skb->data, lp_des_mac, MAC_ADDR_LEN);//copy the dest MAC address
	skb->dev = dev;

	if (tp->loopback_tx_schedule == LOOPBACK_TX_SCHEDULE_ON) {
		dev->hard_start_xmit(skb, dev);
	}

	return 0;
}

static int 
rtl8168_interrupt_test(struct net_device *dev, u32 *isr_content)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	RTL_W8(TxPoll, FSWInt);	//tirgger software interrupt
	*isr_content = RTL_R16(IntrStatus);
	RTL_W16(IntrStatus, 0xFFFF);

	return 0;
}

static int 
rtl8168_report_device_type(struct net_device *dev, char *device_type)
{
	strcpy(device_type, "PCI-E Gigabit Ethernet");

	return 0;
}

static int 
rtl8168_report_device_id(struct net_device *dev, char *device_id)
{
	strcpy(device_id, "8168");

	return 0;
}


int
rtl8168_priv_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	struct mac_reg_rw *mac_rw;
	void *user_data = rq->ifr_data;
	int eeprom_size;
	u32 *eeprom_cont;
	u32 isr_content;
	char device_type[DEVICE_TYPE_LEN];
	char device_id[DEVICE_ID_LEN];
	struct loopback loopback_test;
	int i;

	switch (cmd) {
	case RTL_READ_MAC:
		mac_rw = kmalloc(sizeof(struct mac_reg_rw), GFP_ATOMIC);
		if (!mac_rw)
			return -ENOMEM;
		memset(mac_rw, 0, sizeof(struct mac_reg_rw));

		if (copy_from_user(mac_rw, user_data, sizeof(struct mac_reg_rw))) {
			kfree(mac_rw);
			return -EFAULT;
		}

		if (mac_rw->len == 8) {
			mac_rw->value = RTL_R8(mac_rw->offset);
		} else if (mac_rw->len == 16) {
			mac_rw->value = RTL_R16(mac_rw->offset);
		} else if (mac_rw->len == 32) {
			mac_rw->value = RTL_R32(mac_rw->offset);
		}

		if (copy_to_user(user_data, mac_rw, sizeof(struct mac_reg_rw))) {
			kfree(mac_rw);
			return -EFAULT;
		}

		return 0;
	case RTL_WRITE_MAC:
		mac_rw = kmalloc(sizeof(struct mac_reg_rw), GFP_ATOMIC);
		if (!mac_rw)
			return -ENOMEM;
		memset(mac_rw, 0, sizeof(struct mac_reg_rw));

		if (copy_from_user(mac_rw, user_data, sizeof(struct mac_reg_rw))) {
			kfree(mac_rw);
			return -EFAULT;
		}

		RTL_W8(Cfg9346, Cfg9346_Unlock);

		if (mac_rw->len == 8) {
			RTL_W8(mac_rw->offset, mac_rw->value);
		} else if (mac_rw->len == 16) {
			RTL_W16(mac_rw->offset, mac_rw->value);
		} else if (mac_rw->len == 32) {
			RTL_W32(mac_rw->offset, mac_rw->value);
		}

		RTL_W8(Cfg9346, Cfg9346_Lock);

		return 0;
	case RTL_EEPROM_SIZE:
		eeprom_size = rtl8168_eeprom_size(dev);
		if (copy_to_user(user_data, &eeprom_size, sizeof(int)))
			return -EFAULT;
		return 0;
	case RTL_READ_EEPROM:
		eeprom_size = rtl8168_eeprom_size(dev);
		eeprom_cont = kmalloc(eeprom_size, GFP_ATOMIC);
		if (!eeprom_cont)
			return -ENOMEM;
		memset(eeprom_cont, 0, eeprom_size);
		rtl8168_eeprom_read(dev, eeprom_cont, eeprom_size);
		if (copy_to_user(user_data, eeprom_cont, eeprom_size)) {
			kfree(eeprom_cont);
			return -EFAULT;
		}
		kfree(eeprom_cont);
		return 0;
	case RTL_WRITE_EEPROM:
		eeprom_size = rtl8168_eeprom_size(dev);
		eeprom_cont = kmalloc(eeprom_size, GFP_ATOMIC);
		if (!eeprom_cont)
			return -ENOMEM;
		memset(eeprom_cont, 0, eeprom_size);
		if (copy_from_user(eeprom_cont, user_data, eeprom_size)) {
			kfree(eeprom_cont);
			return -EFAULT;
		}
		rtl8168_eeprom_write(dev, eeprom_cont, eeprom_size);
		kfree(eeprom_cont);
		return 0;
	case RTL_LOOPBACK:
		if (copy_from_user(&loopback_test, user_data, sizeof(struct loopback)))
			return -EFAULT;

		if (loopback_test.state == LOOPBACK_ON) {
			tp->loopback_pkt_payload = kmalloc(loopback_test.len, GFP_KERNEL);
			tp->loopback_pkt_payload_len = loopback_test.len;
			for (i = 0; i < loopback_test.len; i++)
				*(tp->loopback_pkt_payload + i) = *(loopback_test.payload + i);
		}

		rtl8168_set_loopback(dev, loopback_test.type, loopback_test.state);

		if (loopback_test.state == LOOPBACK_OFF) {
			loopback_test.loopback_tx_ok = tp->loopback_tx_ok;
			loopback_test.loopback_rx_ok = tp->loopback_rx_ok;
			loopback_test.loopback_tx_err = tp->loopback_tx_err;
			loopback_test.loopback_rx_err = tp->loopback_rx_err;

			if (copy_to_user(user_data, &loopback_test, sizeof(struct loopback)))
				return -EFAULT;
		}
		return 0;
	case RTL_INTERRUPT:
		rtl8168_interrupt_test(dev, &isr_content);
		if (copy_to_user(user_data, &isr_content, sizeof(u32)))
			return -EFAULT;
		return 0;
	case RTL_DEVICE_TYPE:
		if (copy_from_user(device_type, user_data, DEVICE_TYPE_LEN))
			return -EFAULT;

		rtl8168_report_device_type(dev, device_type);

		if (copy_to_user(user_data, device_type, DEVICE_TYPE_LEN))
			return -EFAULT;

		return 0;
	case RTL_DEVICE_ID:
		if (copy_from_user(device_id, user_data, DEVICE_ID_LEN))
			return -EFAULT;

		rtl8168_report_device_id(dev, device_id);

		if (copy_to_user(user_data, device_id, DEVICE_ID_LEN))
			return -EFAULT;

		return 0;
	default:
		return -EOPNOTSUPP;		
	}
}

