/*
 *	Spanning tree protocol; generic parts
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_stp.c,v 1.8 2009/08/24 03:38:23 button Exp $
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



/* called under ioctl_lock or bridge lock */
int br_is_root_bridge(struct net_bridge *br)
{
	//bert trace
	/*printk("receiving bpdu port bridge id:%c%c%c%c%c%c%c%c, designated root:%c%c%c%c%c%c%c%c \n ",
		   br->bridge_id.prio[0],br->bridge_id.prio[1],br->bridge_id.addr[0],br->bridge_id.prio[1],br->bridge_id.prio[2],
		   br->bridge_id.prio[3],br->bridge_id.prio[4],br->bridge_id.prio[5],
		   br->designated_root.prio[0],br->designated_root.prio[1],br->designated_root.addr[0],br->designated_root.prio[1],br->designated_root.prio[2],
		   br->designated_root.prio[3],br->designated_root.prio[4],br->designated_root.prio[5]); */
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}

/* called under bridge lock */
int br_is_designated_port(struct net_bridge_port *p)
{
	return !memcmp(&p->designated_bridge, &p->br->bridge_id, 8) &&
		(p->designated_port == p->port_id);
}

/* called under ioctl_lock or bridge lock */
struct net_bridge_port *br_get_port(struct net_bridge *br, int port_no)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		if (p->port_no == port_no)
			return p;

		p = p->next;
	}

	return NULL;
}

/* called under bridge lock */
static int br_should_become_root_port(struct net_bridge_port *p, int root_port)
{
	struct net_bridge *br;
	struct net_bridge_port *rp;
	int t;

	//char const port_name[16] = {"wlan0-msh"};	//16 = IFNAMSIZ

	br = p->br;
	if (p->state == BR_STATE_DISABLED ||
	    br_is_designated_port(p))
		return 0;

#ifdef CONFIG_RTL865X_CMO
	if(p->designated_bridge.prio[0] < br->bridge_id.prio[0])
		return 1;
	else if((p->designated_bridge.prio[0] == br->bridge_id.prio[0]) && (p->designated_bridge.prio[1] < br->bridge_id.prio[1]))
		return 1;
	else if((p->designated_bridge.prio[0] != br->bridge_id.prio[0]) && (p->designated_bridge.prio[1] != br->bridge_id.prio[1]))
		return 0;
#endif		
	if (memcmp(&br->bridge_id, &p->designated_root, 8) <= 0)
		return 0;

	if (!root_port)
		return 1;

	rp = br_get_port(br, root_port);

	//brian modify for enforcing spanning tree to block some port except mesh
	/*if( !memcmp(&(p->dev->name), port_name, strlen(port_name)) ){	//greater than wlan0-msh, means "wlan0-mshX"
		printk("%s is No.%d set to root\n",p->dev->name, p->port_no);
		return 1;
	}
	else
		return 0;*/

	t = memcmp(&p->designated_root, &rp->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->designated_cost + p->path_cost <
	    rp->designated_cost + rp->path_cost)
		return 1;
	else if (p->designated_cost + p->path_cost >
		 rp->designated_cost + rp->path_cost)
		return 0;

	t = memcmp(&p->designated_bridge, &rp->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->designated_port < rp->designated_port)
		return 1;
	else if (p->designated_port > rp->designated_port)
		return 0;

	if (p->port_id < rp->port_id)
		return 1;

	return 0;
}

/* called under bridge lock */
static void br_root_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;
	int root_port;
#ifdef CONFIG_RTL865X_CMO
	int root_init=0;
#endif
	root_port = 0;

	p = br->port_list;
	while (p != NULL) {
		if (br_should_become_root_port(p, root_port))
			root_port = p->port_no;

		p = p->next;
	}

	br->root_port = root_port;

	if (!root_port) {
#ifdef CONFIG_RTL865X_CMO
		if (!br_is_root_bridge(br)) root_init = 1;
#endif
		br->designated_root = br->bridge_id;
		br->root_path_cost = 0;
#ifdef CONFIG_RTL865X_CMO
	       	if(root_init) br_become_root_bridge(br);
#endif
	} else {
		p = br_get_port(br, root_port);
		br->designated_root = p->designated_root;
		br->root_path_cost = p->designated_cost + p->path_cost;
	}
}

/* called under bridge lock */
void br_become_root_bridge(struct net_bridge *br)
{
	br->max_age = br->bridge_max_age;
	br->hello_time = br->bridge_hello_time;
	br->forward_delay = br->bridge_forward_delay;
	br_topology_change_detection(br);
	br_timer_clear(&br->tcn_timer);
	br_config_bpdu_generation(br);
	br_timer_set(&br->hello_timer, jiffies);
}

/* called under bridge lock */
void br_transmit_config(struct net_bridge_port *p)
{
	struct br_config_bpdu bpdu;
	struct net_bridge *br;

	if (br_timer_is_running(&p->hold_timer)) {
		p->config_pending = 1;
		return;
	}

	br = p->br;

	bpdu.topology_change = br->topology_change;
	bpdu.topology_change_ack = p->topology_change_ack;
	bpdu.root = br->designated_root;
	bpdu.root_path_cost = br->root_path_cost;
	bpdu.bridge_id = br->bridge_id;
	bpdu.port_id = p->port_id;
	bpdu.message_age = 0;
	if (!br_is_root_bridge(br)) {
		struct net_bridge_port *root;
		unsigned long age;

		root = br_get_port(br, br->root_port);
		age = br_timer_get_residue(&root->message_age_timer) + 1;
		bpdu.message_age = age;
	}
	bpdu.max_age = br->max_age;
	bpdu.hello_time = br->hello_time;
	bpdu.forward_delay = br->forward_delay;

	br_send_config_bpdu(p, &bpdu);

	p->topology_change_ack = 0;
	p->config_pending = 0;
	br_timer_set(&p->hold_timer, jiffies);
}

/* called under bridge lock */
static void br_record_config_information(struct net_bridge_port *p, struct br_config_bpdu *bpdu)
{
	p->designated_root = bpdu->root;
	p->designated_cost = bpdu->root_path_cost;
	p->designated_bridge = bpdu->bridge_id;
	p->designated_port = bpdu->port_id;

	br_timer_set(&p->message_age_timer, jiffies - bpdu->message_age);
}

/* called under bridge lock */
static void br_record_config_timeout_values(struct net_bridge *br, struct br_config_bpdu *bpdu)
{
	br->max_age = bpdu->max_age;
	br->hello_time = bpdu->hello_time;
	br->forward_delay = bpdu->forward_delay;
	br->topology_change = bpdu->topology_change;
	//printk("record br information from bpdu \n");
}

/* called under bridge lock */
void br_transmit_tcn(struct net_bridge *br)
{
	//printk("Required to send TCN BPDU \n");
	br_send_tcn_bpdu(br_get_port(br, br->root_port));
}

/* called under bridge lock */
static int br_should_become_designated_port(struct net_bridge_port *p)
{
	struct net_bridge *br;
	int t;

	br = p->br;
	if (br_is_designated_port(p))
		return 1;

	if (memcmp(&p->designated_root, &br->designated_root, 8))
		return 1;

	if (br->root_path_cost < p->designated_cost)
		return 1;
	else if (br->root_path_cost > p->designated_cost)
		return 0;

	t = memcmp(&br->bridge_id, &p->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->port_id < p->designated_port)
		return 1;

	return 0;
}

/* called under bridge lock */
static void br_designated_port_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		if (p->state != BR_STATE_DISABLED &&
		    br_should_become_designated_port(p))
			br_become_designated_port(p);

		p = p->next;
	}
}

/* called under bridge lock */
static int br_supersedes_port_info(struct net_bridge_port *p, struct br_config_bpdu *bpdu)
{
	int t;

	t = memcmp(&bpdu->root, &p->designated_root, 8);
	if (t < 0){
		//printk("root described by bpdu is less than receive port designated root. \n");
		return 1;
		}
	else if (t > 0){
		//printk("root described by bpdu is greater than receive port designated root. \n");
		return 0;
		}

	if (bpdu->root_path_cost < p->designated_cost){
		//printk("root path cost described by bpdu is less than receive port designated cost. \n");
		return 1;
		}
	else if (bpdu->root_path_cost > p->designated_cost){
		//printk("root path cost described by bpdu is greater than receive port designated cost. \n");
		return 0;
		}

	t = memcmp(&bpdu->bridge_id, &p->designated_bridge, 8);
	if (t < 0){
		//printk("bridge id described by bpdu is less than receive port designated bridge id. \n");
		return 1;
		}
	else if (t > 0){
		//printk("bridge id described by bpdu is greater than receive port designated bridge id. \n");
		return 0;
		}

	if (memcmp(&bpdu->bridge_id, &p->br->bridge_id, 8)){
		//printk("bridge id described by bpdu is not equal to receive port bridge id. \n");
		return 1;
		}

	if (bpdu->port_id <= p->designated_port){
		//printk("port id described by bpdu is not equal to receive port designated port id. \n");
		return 1;
		}

	return 0;
}

/* called under bridge lock */
static void br_topology_change_acknowledged(struct net_bridge *br)
{
	br->topology_change_detected = 0;
	br_timer_clear(&br->tcn_timer);
}

/* called under bridge lock */
void br_topology_change_detection(struct net_bridge *br)
{
	printk(KERN_INFO "%s: topology change detected", br->dev.name);

	if (br_is_root_bridge(br)) {
		printk(", propagating");
		br->topology_change = 1;
		br_timer_set(&br->topology_change_timer, jiffies);
	} else if (!br->topology_change_detected) {
		printk(", sending tcn bpdu");
		br_transmit_tcn(br);
		br_timer_set(&br->tcn_timer, jiffies);
	}

	printk("\n");
	br->topology_change_detected = 1;
}

/* called under bridge lock */
void br_config_bpdu_generation(struct net_bridge *br)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		if (p->state != BR_STATE_DISABLED &&
		    br_is_designated_port(p))
			br_transmit_config(p);

		p = p->next;
	}
}

/* called under bridge lock */
static void br_reply(struct net_bridge_port *p)
{
	br_transmit_config(p);
}

/* called under bridge lock */
void br_configuration_update(struct net_bridge *br)
{
	br_root_selection(br);
	br_designated_port_selection(br);
}

/* called under bridge lock */
void br_become_designated_port(struct net_bridge_port *p)
{
	struct net_bridge *br;

	br = p->br;
	p->designated_root = br->designated_root;
	p->designated_cost = br->root_path_cost;
	p->designated_bridge = br->bridge_id;
	p->designated_port = p->port_id;
}

/* called under bridge lock */
static void br_make_blocking(struct net_bridge_port *p)
{
#if 0
#ifdef STP_DISABLE_ETH 
//Chris:  stp+mesh
	struct net_bridge_port *tp;
#endif
#endif //CONFIG_RTK_MESH

	if (p->state != BR_STATE_DISABLED &&
	    p->state != BR_STATE_BLOCKING) {
		if (p->state == BR_STATE_FORWARDING ||
		    p->state == BR_STATE_LEARNING)
			br_topology_change_detection(p->br);

		printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
		       p->br->dev.name, p->port_no, p->dev->name, "blocking");

		
#ifdef CONFIG_RTK_MESH
#if 0
#ifdef STP_DISABLE_ETH
// Chris:  stp+mesh. if the blocking port is mesh, we disable the eth0 to avoid isolated MP
// 		 NOTE: MUST resume the interface after loop disapeared
		if (!strncmp(p->dev->name, "wlan0-msh0", 10)){
			tp = p->br->port_list;
			while (tp != NULL) {
				if (tp!=p && (tp->state != BR_STATE_DISABLED)){
					tp->disable_by_mesh = 1;
					br_stp_disable_port(tp);
					printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
	  				    tp->br->dev.name, tp->port_no, tp->dev->name, "disabled if mesh blocked");
				}
				tp = tp->next;
			}
		}else {
			p->state = BR_STATE_BLOCKING;
			br_timer_clear(&p->forward_delay_timer);
		}
#endif
#endif

#ifdef STP_ADDCOST_ETH
// Chris:  stp+mesh. Add cost on eth to block it. 
//		 NOTE: should set to default cost when topology changed
		
		if (!strncmp(p->dev->name, "wlan0-msh0", 10)){
			struct net_bridge_port *tp;
			struct net_bridge *b;

			b = p->br;
			
			p->state = BR_STATE_BLOCKING;
			br_timer_clear(&p->forward_delay_timer);
		
			//printk(KERN_INFO "%s-------\n", p->dev->name);
			tp = b->port_list;
			while (tp != NULL) {
				
				//printk(KERN_INFO "%s-------\n", tp->dev->name);
				if (tp!=p && (tp->state != BR_STATE_DISABLED)){
					printk(KERN_INFO "%s: port %i(%s) path_cost %d set to %d\n",
	  				    b->dev.name, tp->port_no, tp->dev->name, tp->path_cost, b->root_path_cost+p->path_cost+100);
					if (tp->path_cost <= (b->root_path_cost+p->path_cost) ) {
						br_stp_set_path_cost(tp, b->root_path_cost+p->path_cost+100);
						b->is_cost_changed = 1;
					}
				}
				tp = tp->next;
			}
			
		}
		else {
			p->state = BR_STATE_BLOCKING;
			br_timer_clear(&p->forward_delay_timer);
		}
		
#endif

#else
		p->state = BR_STATE_BLOCKING;
		br_timer_clear(&p->forward_delay_timer);
			
#endif //CONFIG_RTK_MESH

		
	}

}

/* called under bridge lock */
static void br_make_forwarding(struct net_bridge_port *p)
{
	if (p->state == BR_STATE_BLOCKING) {
		printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
		       p->br->dev.name, p->port_no, p->dev->name, "listening");

		p->state = BR_STATE_LISTENING;
		br_timer_set(&p->forward_delay_timer, jiffies);

	}
}

/* called under bridge lock */
void br_port_state_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		if (p->state != BR_STATE_DISABLED) {
			if (p->port_no == br->root_port) {
				p->config_pending = 0;
				p->topology_change_ack = 0;
				br_make_forwarding(p);
			} else if (br_is_designated_port(p)) {
				br_timer_clear(&p->message_age_timer);
				br_make_forwarding(p);
			} else {
				p->config_pending = 0;
				p->topology_change_ack = 0;
				br_make_blocking(p);
			}
		}

		p = p->next;
	}
}

/* called under bridge lock */
static void br_topology_change_acknowledge(struct net_bridge_port *p)
{
	p->topology_change_ack = 1;
	br_transmit_config(p);
}

/* lock-safe */
void br_received_config_bpdu(struct net_bridge_port *p, struct br_config_bpdu *bpdu)
{
	struct net_bridge *br;
	int was_root;

	if (p->state == BR_STATE_DISABLED)
		return;

	br = p->br;
	read_lock(&br->lock);

	was_root = br_is_root_bridge(br);
	if (br_supersedes_port_info(p, bpdu)) {
		br_record_config_information(p, bpdu);
		br_configuration_update(br);
		br_port_state_selection(br);

		if (!br_is_root_bridge(br) && was_root) {
			br_timer_clear(&br->hello_timer);
			if (br->topology_change_detected) {
				br_timer_clear(&br->topology_change_timer);
				br_transmit_tcn(br);
				br_timer_set(&br->tcn_timer, jiffies);
			}
		}

		if (p->port_no == br->root_port) {
			br_record_config_timeout_values(br, bpdu);
			br_config_bpdu_generation(br);
			if (bpdu->topology_change_ack)
				br_topology_change_acknowledged(br);
		}
	} else if (br_is_designated_port(p)) {		
		br_reply(p);		
	}

	read_unlock(&br->lock);
}

/* lock-safe */
void br_received_tcn_bpdu(struct net_bridge_port *p)
{
	read_lock(&p->br->lock);
	if (p->state != BR_STATE_DISABLED &&
	    br_is_designated_port(p)) {
		printk(KERN_INFO "%s: received tcn bpdu on port %i(%s)\n",
		       p->br->dev.name, p->port_no, p->dev->name);
#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
		br_reset_cost(p);
#endif
		br_topology_change_detection(p->br);
		br_topology_change_acknowledge(p);
	}
	read_unlock(&p->br->lock);
}

#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)

/* called under bridge lock */
void br_reset_cost(struct net_bridge_port *p){
	struct net_bridge_port *tp;
	
	if (p->br->is_cost_changed) {
		tp = p->br->port_list;
		while (tp != NULL) {
			if (strncmp(tp->dev->name, "wlan0-msh0", 10)){
				tp->path_cost = br_initial_port_cost(tp->dev);
				printk(KERN_INFO "%s: port %i(%s) set default path_cost %d \n",
					    tp->br->dev.name, tp->port_no, tp->dev->name, tp->path_cost);
			}
			tp = tp->next;
		}
		p->br->is_cost_changed = 0;
	}
}
#endif



