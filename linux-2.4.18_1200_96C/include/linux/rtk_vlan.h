/*
 *      Headler file of Realtek VLAN
 *
 *      $Id: rtk_vlan.h,v 1.5.2.3 2011/04/29 09:12:58 bruce Exp $
 */

#ifndef _RTK_VLAN_H
#define _RTK_VLAN_H

struct vlan_info {
	int global_vlan;	// 0/1 - global vlan disable/enable
	int is_lan;				// 1: eth-lan/wlan port, 0: wan port	
	int vlan;					// 0/1: disable/enable vlan
	int tag;					// 0/1: disable/enable tagging
	int id;						// 1~4090: vlan id
	int pri;						// 0~7: priority;
	int cfi;						// 0/1: cfi
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	int forwarding;				// 0:disabled/1: bridge/2:NAT
	int idx;
#endif
};

struct _vlan_tag {
	unsigned short tpid;	// protocol id
	unsigned short pci;	// priority:3, cfi:1, id:12
};

struct vlan_tag {
	union
	{	
		unsigned long v;
		struct _vlan_tag f;
	};	
};

struct vlan_info_item {
	struct vlan_info info;
	struct net_device *dev;
};

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
struct net_device *get_wan_from_vlan(void);
int add_vlan_info(struct vlan_info *info, struct net_device *dev);
#endif

#endif // _RTK_VLAN_H
