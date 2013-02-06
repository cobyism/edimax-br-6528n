/*
 *	Ioctl handler
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_ioctl.c,v 1.6 2009/07/24 13:28:30 chris Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/if_bridge.h>
#include <linux/inetdevice.h>
#include <asm/uaccess.h>
#include "br_private.h"

static int br_ioctl_device(struct net_bridge *br,
			   unsigned int cmd,
			   unsigned long arg0,
			   unsigned long arg1,
			   unsigned long arg2)
{
	if (br == NULL)
		return -EINVAL;

	switch (cmd)
	{
	case BRCTL_ADD_IF:
	case BRCTL_DEL_IF:
	{
		struct net_device *dev;
		int ret;

		dev = dev_get_by_index(arg0);
		if (dev == NULL)
			return -EINVAL;

		if (cmd == BRCTL_ADD_IF)
			ret = br_add_if(br, dev);
		else
			ret = br_del_if(br, dev);

		dev_put(dev);
		return ret;
	}

	case BRCTL_GET_BRIDGE_INFO:
	{
		struct __bridge_info b;

		memset(&b, 0, sizeof(struct __bridge_info));
		memcpy(&b.designated_root, &br->designated_root, 8);
		memcpy(&b.bridge_id, &br->bridge_id, 8);
		b.root_path_cost = br->root_path_cost;
		b.max_age = br->max_age;
		b.hello_time = br->hello_time;
		b.forward_delay = br->forward_delay;
		b.bridge_max_age = br->bridge_max_age;
		b.bridge_hello_time = br->bridge_hello_time;
		b.bridge_forward_delay = br->bridge_forward_delay;
		b.topology_change = br->topology_change;
		b.topology_change_detected = br->topology_change_detected;
		b.root_port = br->root_port;
		b.stp_enabled = br->stp_enabled;
		b.ageing_time = br->ageing_time;
		b.gc_interval = br->gc_interval;
		b.hello_timer_value = br_timer_get_residue(&br->hello_timer);
		b.tcn_timer_value = br_timer_get_residue(&br->tcn_timer);
		b.topology_change_timer_value = br_timer_get_residue(&br->topology_change_timer);
		b.gc_timer_value = br_timer_get_residue(&br->gc_timer);

		if (copy_to_user((void *)arg0, &b, sizeof(b)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_GET_PORT_LIST:
	{
		int i;
		int indices[256];

		for (i=0;i<256;i++)
			indices[i] = 0;

		br_get_port_ifindices(br, indices);
		if (copy_to_user((void *)arg0, indices, 256*sizeof(int)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_SET_BRIDGE_FORWARD_DELAY:
		br->bridge_forward_delay = arg0;
		if (br_is_root_bridge(br))
			br->forward_delay = arg0;
		return 0;

	case BRCTL_SET_BRIDGE_HELLO_TIME:
		br->bridge_hello_time = arg0;
		if (br_is_root_bridge(br))
			br->hello_time = arg0;
		return 0;

	case BRCTL_SET_BRIDGE_MAX_AGE:
		br->bridge_max_age = arg0;
		if (br_is_root_bridge(br))
			br->max_age = arg0;
		return 0;

	case BRCTL_SET_AGEING_TIME:
		br->ageing_time = arg0;
		return 0;

	case BRCTL_SET_GC_INTERVAL:
		br->gc_interval = arg0;
		return 0;

	case BRCTL_GET_PORT_INFO:
	{
		struct __port_info p;
		struct net_bridge_port *pt;

		if ((pt = br_get_port(br, arg1)) == NULL)
			return -EINVAL;

		memset(&p, 0, sizeof(struct __port_info));
		memcpy(&p.designated_root, &pt->designated_root, 8);
		memcpy(&p.designated_bridge, &pt->designated_bridge, 8);
		p.port_id = pt->port_id;
		p.designated_port = pt->designated_port;
		p.path_cost = pt->path_cost;
		p.designated_cost = pt->designated_cost;
		p.state = pt->state;
		p.top_change_ack = pt->topology_change_ack;
		p.config_pending = pt->config_pending;
		p.message_age_timer_value = br_timer_get_residue(&pt->message_age_timer);
		p.forward_delay_timer_value = br_timer_get_residue(&pt->forward_delay_timer);
		p.hold_timer_value = br_timer_get_residue(&pt->hold_timer);

		if (copy_to_user((void *)arg0, &p, sizeof(p)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_SET_BRIDGE_STP_STATE:
		br->stp_enabled = arg0?1:0;
		return 0;

	case BRCTL_SET_BRIDGE_PRIORITY:
		br_stp_set_bridge_priority(br, arg0);
		return 0;

	case BRCTL_SET_PORT_PRIORITY:
	{
		struct net_bridge_port *p;

		if ((p = br_get_port(br, arg0)) == NULL)
			return -EINVAL;
		br_stp_set_port_priority(p, arg1);
		return 0;
	}

	case BRCTL_SET_PATH_COST:
	{
		struct net_bridge_port *p;

		if ((p = br_get_port(br, arg0)) == NULL)
			return -EINVAL;
		br_stp_set_path_cost(p, arg1);
		return 0;
	}

	case BRCTL_GET_FDB_ENTRIES:
#ifdef CONFIG_RTK_GUEST_ZONE
		return br_fdb_get_entries(br, (void *)arg0, arg1, arg2, 0);
#else		
		return br_fdb_get_entries(br, (void *)arg0, arg1, arg2);
#endif

#ifdef MULTICAST_FILTER
	case 101:
		printk(KERN_INFO "%s: clear port list of multicast filter\n", br->dev.name);
		br->fltr_portlist_num = 0;
		return 0;

	case 102:
	{
		int i;

		if (br->fltr_portlist_num == MLCST_FLTR_ENTRY) {
			printk(KERN_INFO "%s: set port num of multicast filter, entries full!\n", br->dev.name);
			return 0;
		}
		for (i=0; i<br->fltr_portlist_num; i++)
			if (br->fltr_portlist[i] == (unsigned short)arg0)
				return 0;
		printk(KERN_INFO "%s: set port num [%d] of multicast filter\n", br->dev.name, (unsigned short)arg0);
		br->fltr_portlist[br->fltr_portlist_num] = (unsigned short)arg0;
		br->fltr_portlist_num++;
		return 0;
	}
#endif

#ifdef MULTICAST_BWCTRL
	case 103:
	{
		struct net_bridge_port *p;

		if ((p = br_get_port(br, arg0)) == NULL)
			return -EINVAL;
		if (arg1 == 0) {
			p->bandwidth = 0;
			printk(KERN_INFO "%s: port %i(%s) multicast bandwidth all\n",
			       p->br->dev.name, p->port_no, p->dev->name);
		}
		else {
			p->bandwidth = arg1 * 1000 / 8;
			printk(KERN_INFO "%s: port %i(%s) multicast bandwidth %dkbps\n",
			       p->br->dev.name, p->port_no, p->dev->name, (unsigned int)arg1);
		}
		return 0;
	}
#endif

#ifdef RTL_BRIDGE_MAC_CLONE
	case 104:	// MAC Clone enable/disable
	{
		struct net_bridge_port *p;
		unsigned char nullmac[] = {0, 0, 0, 0, 0, 0};

		if ((p = br_get_port(br, arg0)) == NULL)
			return -EINVAL;

		if ((p->macCloneTargetPort = br_get_port(br, arg1)) == NULL)
			return -EINVAL;

		p->enable_mac_clone = 1;
		p->mac_clone_completed = 0;

		if (clone_pair.port != p->macCloneTargetPort)
		{
			TRACE("clone_pair.port [%x] != p->macCloneTargetPort [%x], don't clone\n", (unsigned int)clone_pair.port, (unsigned int)p->macCloneTargetPort);
			clone_pair.port = p->macCloneTargetPort;
			TRACE("clone_pair.port = %x\n", (unsigned int)clone_pair.port);
			memset(clone_pair.mac.addr, 0, ETH_ALEN);
		}
		else
		{
			if(!memcmp(clone_pair.mac.addr, nullmac, ETH_ALEN))
			{
				TRACE("clone_pair.mac.addr == nullmac, don't clone\n");
			}
			else
			{
				TRACE("Clone MAC from previous one\n");
				br_mac_clone(p->macCloneTargetPort, clone_pair.mac.addr);
			}
		}

		TRACE("device %s, Enable MAC Clone to device %s\n", p->dev->name, p->macCloneTargetPort->dev->name);
		return 0;
	}
#endif

#ifdef CONFIG_RTK_GUEST_ZONE
	case 105:	// set zone
	{
		struct net_bridge_port *p;
		if ((p = br_get_port(br, arg0)) == NULL)
			return -EINVAL;
		p->is_guest_zone = arg1;
#ifdef DEBUG_GUEST_ZONE
		panic_printk("set device=%s is_guest_zone=%d\n", p->dev->name, p->is_guest_zone);
#endif
		return 0;
	}

	case 106:	// set zone isolation
		br->is_zone_isolated = arg0;
#ifdef DEBUG_GUEST_ZONE
		panic_printk("set zone isolation=%d\n",	br->is_zone_isolated);		
#endif
		return 0;
		
	case 107:	// set guest isolation
		br->is_guest_isolated = arg0;
#ifdef DEBUG_GUEST_ZONE
		panic_printk("set guest isolation=%d\n",	br->is_guest_isolated);		
#endif		
		return 0;

	case 108:	// set lock mac list
	{
		unsigned char mac[6];
		int i;
		if (copy_from_user(mac, (unsigned long*)arg0, 6))
			return -EFAULT;
#ifdef DEBUG_GUEST_ZONE
		panic_printk("set lock client list=%02x:%02x:%02x:%02x:%02x:%02x\n", 
							mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);	
#endif
		if (!memcmp(mac, "\x0\x0\x0\x0\x0\x0", 6)) {  // reset list
#ifdef DEBUG_GUEST_ZONE		
			panic_printk("reset lock list!\n");
#endif
			br->lock_client_num = 0;
			return 0;		
		}
		for (i=0; i<br->lock_client_num; i++) {
			if (!memcmp(mac, br->lock_client_list[i], 6)) {
#ifdef DEBUG_GUEST_ZONE				
				panic_printk("duplicated lock entry!\n");
#endif
				return 0;
			}			
		}
		if (br->lock_client_num >= MAX_LOCK_CLIENT) {
#ifdef DEBUG_GUEST_ZONE			
			panic_printk("Add failed, lock list table full!\n");
#endif
			return 0;
		}
		memcpy(br->lock_client_list[br->lock_client_num], mac, 6);
		br->lock_client_num++;
		return 0;
	}		
	case 109:	// show guest info
	{
		int i;
		panic_printk("\n");
		panic_printk("  zone isolation: %d\n", br->is_zone_isolated);
		panic_printk("  guest isolation: %d\n", br->is_guest_isolated);
		i = 1;
		while (1) {
			struct net_bridge_port *p;
			if ((p = br_get_port(br, i++)) == NULL)
				break;
			panic_printk("  %s: %s\n", p->dev->name, (p->is_guest_zone ? "guest" : "host"));			
		}		
		panic_printk("  locked client no: %d\n", br->lock_client_num);
		for (i=0; i< br->lock_client_num; i++) {
			unsigned char *mac;
			mac = br->lock_client_list[i];
			panic_printk("    mac=%02x:%02x:%02x:%02x:%02x:%02x\n", 
							mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);	
		}			
		panic_printk("\n");		
		return 0;
	}	

	case 110:
		return br_fdb_get_entries(br, (void *)arg0, arg1, arg2, 1);	
#endif	// CONFIG_RTK_GUEST_ZONE
	}

	return -EOPNOTSUPP;
}

static int br_ioctl_deviceless(unsigned int cmd,
			       unsigned long arg0,
			       unsigned long arg1)
{
	switch (cmd)
	{
	case BRCTL_GET_VERSION:
		return BRCTL_VERSION;

	case BRCTL_GET_BRIDGES:
	{
		int i;
		int indices[64];

		for (i=0;i<64;i++)
			indices[i] = 0;

		if (arg1 > 64)
			arg1 = 64;
		arg1 = br_get_bridge_ifindices(indices, arg1);
		if (copy_to_user((void *)arg0, indices, arg1*sizeof(int)))
			return -EFAULT;

		return arg1;
	}

	case BRCTL_ADD_BRIDGE:
	case BRCTL_DEL_BRIDGE:
	{
		char buf[IFNAMSIZ];

		if (copy_from_user(buf, (void *)arg0, IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;

		if (cmd == BRCTL_ADD_BRIDGE)
			return br_add_bridge(buf);

		return br_del_bridge(buf);
	}
#ifdef CONFIG_RTK_MESH
	//by brian, dynamic portal enable
	case BRCTL_SET_MESH_PATHSELPID:
		return br_set_meshpathsel_pid(arg0);

	case BRCTL_GET_PORTSTAT:
		{
		struct net_bridge *br;
	
		br = find_br_by_name("br0");
		if (br == NULL)
			return -1;
		else	
			return br->eth0_received;
		}
#endif
	}

	return -EOPNOTSUPP;
}

DECLARE_MUTEX(ioctl_mutex);

int br_ioctl_deviceless_stub(unsigned long arg)
{
	int err;
	unsigned long i[3];

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (copy_from_user(i, (void *)arg, 3*sizeof(unsigned long)))
		return -EFAULT;

	down(&ioctl_mutex);
	err = br_ioctl_deviceless(i[0], i[1], i[2]);
	up(&ioctl_mutex);

	return err;
}

int br_ioctl(struct net_bridge *br, unsigned int cmd, unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	down(&ioctl_mutex);
	err = br_ioctl_deviceless(cmd, arg0, arg1);
	if (err == -EOPNOTSUPP)
		err = br_ioctl_device(br, cmd, arg0, arg1, arg2);
	up(&ioctl_mutex);

	return err;
}

void br_call_ioctl_atomic(void (*fn)(void))
{
	down(&ioctl_mutex);
	fn();
	up(&ioctl_mutex);
}
