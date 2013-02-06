/*
 *  a4 sta functions
 *
 *  $Id: 8192cd_a4_sta.c,v 1.1.2.1 2010/10/13 06:42:52 davidhsu Exp $
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_A4_STA_C_

#ifdef __KERNEL__
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/timer.h>
#endif

#include "./8192cd_cfg.h"

#ifdef A4_STA

#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"


//#define A4_STA_DEBUG

static struct a4_sta_db_entry *alloc_entry(struct rtl8192cd_priv *priv)
{
	int i;

	for (i=0; i<MAX_A4_TBL_NUM; i++) {
		if (!priv->a4_ent[i].used) {
			priv->a4_ent[i].used = 1;
			return &priv->a4_ent[i].entry;
		}		
	}
	return NULL;
}

static void free_entry(struct rtl8192cd_priv *priv,  struct a4_sta_db_entry *entry)
{
	int i;

	for (i=0; i<MAX_A4_TBL_NUM; i++) {
		if (priv->a4_ent[i].used && (entry == &priv->a4_ent[i].entry)) {
			priv->a4_ent[i].used = 0;
			break;				
		}		
	}
}

static int mac_hash(unsigned char *networkAddr)
{	
	unsigned long x;

	x = networkAddr[0] ^ networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^ networkAddr[4] ^ networkAddr[5];

	return x & (A4_STA_HASH_SIZE - 1);
}

static void mac_hash_link(struct rtl8192cd_priv *priv, struct a4_sta_db_entry *ent, int hash)
{
	ent->next_hash = priv->machash[hash];
	
	if (ent->next_hash != NULL)
		ent->next_hash->pprev_hash = &ent->next_hash;
	priv->machash[hash] = ent;
	ent->pprev_hash = &priv->machash[hash];
}

static void mac_hash_unlink(struct a4_sta_db_entry *ent)
{
	*(ent->pprev_hash) = ent->next_hash;
	if (ent->next_hash != NULL)
		ent->next_hash->pprev_hash = ent->pprev_hash;
	ent->next_hash = NULL;
	ent->pprev_hash = NULL;
}

static unsigned long _timeout(struct rtl8192cd_priv *priv)
{
	unsigned long timeout;

	timeout = jiffies - A4_STA_AGEING_TIME*HZ;
	return timeout;
}

static  int  has_expired(struct rtl8192cd_priv *priv, struct a4_sta_db_entry *fdb)
{
	if (time_before_eq(fdb->ageing_timer, _timeout(priv)))
		return 1;

	return 0;
}

void a4_sta_cleanup(struct rtl8192cd_priv *priv)
{
	int i;

	if (priv->pshare->rf_ft_var.a4_enable) {
		for (i=0; i<A4_STA_HASH_SIZE; i++) {	
			struct a4_sta_db_entry *f;
			f = priv->machash[i];

			while (f != NULL) {
				struct a4_sta_db_entry *g;

				g = f->next_hash;
				mac_hash_unlink(f);
				free_entry(priv, f);
				f = g;
			}
		}
	}
}

void a4_sta_expire(struct rtl8192cd_priv *priv)
{
	int i;

	if (priv->pshare->rf_ft_var.a4_enable) {	
		for (i=0; i<A4_STA_HASH_SIZE; i++) {
			struct a4_sta_db_entry *f;
			f = priv->machash[i];

			while (f != NULL) {			
				struct a4_sta_db_entry *g;
				g = f->next_hash;

				if (has_expired(priv, f)) {				
#ifdef A4_STA_DEBUG
					panic_printk("A4 STA Expire (%02d) emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",
						i,
						f->mac[0],
						f->mac[1],
						f->mac[2],
						f->mac[3],
						f->mac[4],
						f->mac[5],
						f->stat->hwaddr[0],
						f->stat->hwaddr[1],
						f->stat->hwaddr[2],
						f->stat->hwaddr[3],
						f->stat->hwaddr[4],
						f->stat->hwaddr[5]);
#endif
					mac_hash_unlink(f);
					free_entry(priv, f);
				}
				f = g;
			}
		}
	}
}

void a4_sta_add(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *mac)
{
	struct a4_sta_db_entry *db;
	int hash;

	ASSERT(mac);

	hash = mac_hash(mac);
	db = priv->machash[hash];

	while (db != NULL) {	
		if (!memcmp(db->mac, mac, ETH_ALEN)) {		
			db->stat = pstat;
			db->ageing_timer = jiffies;
			return;
		}
		db = db->next_hash;
	}

	db = alloc_entry(priv);
	if (db == NULL) {
		DEBUG_ERR("alloc_entry() failed for a4_sta_db_entry!\n");
		return;
	}
	memcpy(db->mac, mac, ETH_ALEN);
	db->stat = pstat;
	db->ageing_timer = jiffies;

#ifdef A4_STA_DEBUG
	panic_printk("A4 STA Add emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",		
		db->mac[0],
		db->mac[1],
		db->mac[2],
		db->mac[3],
		db->mac[4],
		db->mac[5],
		db->stat->hwaddr[0],
		db->stat->hwaddr[1],
		db->stat->hwaddr[2],
		db->stat->hwaddr[3],
		db->stat->hwaddr[4],
		db->stat->hwaddr[5]);
#endif

	mac_hash_link(priv, db, hash);
}

struct stat_info *a4_sta_lookup(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	struct a4_sta_db_entry *db;

	ASSERT(mac);
	
	db = priv->machash[mac_hash(mac)];

	while (db != NULL) {	
		if (!memcmp(db->mac, mac, ETH_ALEN)) {

#ifdef A4_STA_DEBUG
			panic_printk("A4 STA LOOKUP emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",		
				db->mac[0],
				db->mac[1],
				db->mac[2],
				db->mac[3],
				db->mac[4],
				db->mac[5],
				db->stat->hwaddr[0],
				db->stat->hwaddr[1],
				db->stat->hwaddr[2],
				db->stat->hwaddr[3],
				db->stat->hwaddr[4],
				db->stat->hwaddr[5]);
#endif			
			return db->stat;	
		}

		db = db->next_hash;
	}
	return NULL;
}

#endif /* A4_STA */
