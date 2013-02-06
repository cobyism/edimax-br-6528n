/*
 *	Spanning tree protocol; timer-related code
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_stp_timer.c,v 1.10 2009/08/10 12:22:45 chris Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/if_bridge.h>
#include <linux/smp_lock.h>
#include <asm/uaccess.h>
#include "br_private.h"
#include "br_private_stp.h"

static void dump_bridge_id(bridge_id *id)
{
	printk("%.2x%.2x.%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", id->prio[0],
	       id->prio[1], id->addr[0], id->addr[1], id->addr[2], id->addr[3],
	       id->addr[4], id->addr[5]);
}

/* called under bridge lock */
static int br_is_designated_for_some_port(struct net_bridge *br)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		if (p->state != BR_STATE_DISABLED &&
		    !memcmp(&p->designated_bridge, &br->bridge_id, 8))
			return 1;

		p = p->next;
	}

	return 0;
}

/* called under bridge lock */
static void br_hello_timer_expired(struct net_bridge *br)
{
	br_config_bpdu_generation(br);
	br_timer_set(&br->hello_timer, jiffies);
}

/* called under bridge lock */
static void br_message_age_timer_expired(struct net_bridge_port *p)
{
	struct net_bridge *br;
	int was_root;

	br = p->br;
	printk(KERN_INFO "%s: ", br->dev.name);
	printk("neighbour ");
	dump_bridge_id(&p->designated_bridge);
	printk(" lost on port %i(%s)\n", p->port_no, p->dev->name);

#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
	br_reset_cost(p);
#endif
	
	/*
	 * According to the spec, the message age timer cannot be
	 * running when we are the root bridge. So..  this was_root
	 * check is redundant. I'm leaving it in for now, though.
	 */
	was_root = br_is_root_bridge(br);

	br_become_designated_port(p);
	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !was_root)
		br_become_root_bridge(br);
}

/* called under bridge lock */
static void br_forward_delay_timer_expired(struct net_bridge_port *p)
{
	if (p->state == BR_STATE_LISTENING) {
		printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
		       p->br->dev.name, p->port_no, p->dev->name, "learning");

		p->state = BR_STATE_LEARNING;
		br_timer_set(&p->forward_delay_timer, jiffies);
	} else if (p->state == BR_STATE_LEARNING) {
		printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
		       p->br->dev.name, p->port_no, p->dev->name, "forwarding");

		p->state = BR_STATE_FORWARDING;
		if (br_is_designated_for_some_port(p->br))
			br_topology_change_detection(p->br);

#ifdef MULTICAST_BWCTRL
		br_timer_set(&p->bwcrtl_timer, jiffies);
#endif
	}
}

/* called under bridge lock */
static void br_tcn_timer_expired(struct net_bridge *br)
{
	printk(KERN_INFO "%s: retransmitting tcn bpdu\n", br->dev.name);
	br_transmit_tcn(br);
	br_timer_set(&br->tcn_timer, jiffies);
}

/* called under bridge lock */
static void br_topology_change_timer_expired(struct net_bridge *br)
{
	br->topology_change_detected = 0;
	br->topology_change = 0;
}

/* called under bridge lock */
static void br_hold_timer_expired(struct net_bridge_port *p)
{
	if (p->config_pending)
		br_transmit_config(p);
}



/* called under bridge lock */
static void br_check_port_timers(struct net_bridge_port *p)
{
	if (br_timer_has_expired(&p->message_age_timer, p->br->max_age)) {
		br_timer_clear(&p->message_age_timer);
		br_message_age_timer_expired(p);
	}

	if (br_timer_has_expired(&p->forward_delay_timer, p->br->forward_delay)) {
		br_timer_clear(&p->forward_delay_timer);
		br_forward_delay_timer_expired(p);
	}

	if (br_timer_has_expired(&p->hold_timer, BR_HOLD_TIME)) {
		br_timer_clear(&p->hold_timer);
		br_hold_timer_expired(p);
	}


#if 0
#ifdef STP_DISABLE_ETH
//Chris: stp+mesh
	if (br_timer_has_expired(&p->eth_disable_timer, ETH_CHK_INTVL)) {
		br_timer_clear(&p->eth_disable_timer);
		printk(KERN_INFO "%s: enabled for MESH-STP configuration\n", p->br->dev.name);
		br_stp_enable_port(p);
	}
#endif
#endif	//CONFIG_RTK_MESH


#ifdef MULTICAST_BWCTRL
	if ((p->bandwidth != 0) && br_timer_has_expired(&p->bwcrtl_timer, 1*HZ)) {
		br_timer_clear(&p->bwcrtl_timer);
		p->accumulation = 0;
		br_timer_set(&p->bwcrtl_timer, jiffies);
	}
#endif

#ifdef RTL_BRIDGE_MAC_CLONE
	if (p->enable_mac_clone && p->mac_clone_completed && br_timer_has_expired(&p->macClone_timer, 1*HZ)) {
		br_timer_clear(&p->macClone_timer);
		p->macClone_expire_to--;
		if (p->macClone_expire_to == 0) {
			p->mac_clone_completed = 0;
			TRACE("Cloned MAC %02x-%02x-%02x-%02x-%02x-%02x expired\n",
				clone_pair.mac.addr[0], clone_pair.mac.addr[1], clone_pair.mac.addr[2],
				clone_pair.mac.addr[3], clone_pair.mac.addr[4], clone_pair.mac.addr[5]);
		}
		else
			br_timer_set(&p->macClone_timer, jiffies);
	}
#endif
}

/* called under bridge lock */
static void br_check_timers(struct net_bridge *br)
{
	struct net_bridge_port *p;

#ifdef CONFIG_RTK_MESH
	if (br_timer_has_expired(&br->eth0_monitor_timer, br->eth0_monitor_interval)) {
		br_timer_clear(&br->eth0_monitor_timer);
		br->eth0_received = 0;
		br->stp_enabled = 0;
		br_signal_pathsel(br);
		//printk(KERN_INFO,"90 seconds never receive packet from eth0 \n");
	}
	
#if 0	//chris modified for plug detection

#ifdef STP_DISABLE_ETH
//chris: stp + mesh
	if (br_timer_has_expired(&br->eth0_autostp_timer, br->eth0_monitor_interval)) {
		br_timer_set(&br->eth0_autostp_timer, jiffies);
		br->stp_enabled = 0;
	}
#endif
#endif

#endif	//CONFIG_RTK_MESH

	if (br_timer_has_expired(&br->gc_timer, br->gc_interval)) {
		br_timer_set(&br->gc_timer, jiffies);
		br_fdb_cleanup(br);
	}

	if (br_timer_has_expired(&br->hello_timer, br->hello_time)) {
		br_timer_clear(&br->hello_timer);
		br_hello_timer_expired(br);
	}

	if (br_timer_has_expired(&br->tcn_timer, br->bridge_hello_time)) {
		br_timer_clear(&br->tcn_timer);
		br_tcn_timer_expired(br);
	}

	if (br_timer_has_expired(&br->topology_change_timer, br->bridge_forward_delay + br->bridge_max_age)) {
		br_timer_clear(&br->topology_change_timer);
		br_topology_change_timer_expired(br);
	}

	p = br->port_list;
	while (p != NULL) {
		if (p->state != BR_STATE_DISABLED)
			br_check_port_timers(p);
#if 0
#ifdef STP_DISABLE_ETH
//Chris: stp + mesh
		else if (p->disable_by_mesh == 1)
			br_check_port_timers(p);
#endif
#endif 
		p = p->next;
	}
}

void br_tick(unsigned long __data)
{
	struct net_bridge *br = (struct net_bridge *)__data;

	read_lock(&br->lock);
	br_check_timers(br);
	read_unlock(&br->lock);

	br->tick.expires = jiffies + 1;
	add_timer(&br->tick);
}
