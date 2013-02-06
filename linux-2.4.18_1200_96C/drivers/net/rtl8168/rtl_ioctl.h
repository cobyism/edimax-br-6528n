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

#define EEPROM_9356			(1 << 6)

#define EEPROM_SZ_9346			128//byte
#define EEPROM_SZ_9356			256//byte
#define EEPROM_SZ_9366			512//byte
#define EEPROM_SZ_UNKNOWN		-1

#define VPD_WRITE			0x8000
#define VPD_READ			0x0000

#define VPD_ADDRESS			0x4A
#define VPD_DATA			0x4C

#define PHY_LB_ON			0x800B0002
#define PHY_LB_OFF			0x800B0000

#define DEVICE_TYPE_LEN			200
#define DEVICE_ID_LEN			10

#define LOOPBACK_TX_SCHEDULE_ON		1 
#define LOOPBACK_TX_SCHEDULE_OFF	0

#define LOOPBACK_ON			1
#define LOOPBACK_OFF			0

#define MAC_LOOPBACK	1
#define PHY_LOOPBACK	2

#define RTL_HELP		0x0000
#define RTL_READ_MAC		0x89F1
#define RTL_WRITE_MAC		0x89F2
#define RTL_EEPROM_SIZE		0x89F3
#define RTL_READ_EEPROM		0x89F4
#define RTL_WRITE_EEPROM	0x89F5
#define RTL_LOOPBACK		0x89F6
#define RTL_INTERRUPT		0x89F7
#define RTL_DEVICE_TYPE		0x89F8
#define RTL_DEVICE_ID		0x89F9

struct loopback {
	int type;//1: mac loopback; 2: phy loopback
	int state;//1: on; 0: off
	int loopback_tx_ok;
	int loopback_rx_ok;
	int loopback_tx_err;
	int loopback_rx_err;
	unsigned char *payload;	//payload is assigned by the application.
	int len;
};

struct mac_reg_rw {
	int offset;
	int len;
	unsigned long long value;
};


int rtl8168_priv_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
int rtl8168_loopback_test(struct net_device *dev);
void rtl8168_set_loopback(struct net_device *dev, int type, int state);
