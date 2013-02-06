/*
 *  Header file define some tx inline functions
 *
 *  $Id: 8192cd_tx.h,v 1.2 2010/01/29 09:39:16 jimmylin Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_TX_H_
#define _8192CD_TX_H_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"

#ifdef CONFIG_RTK_MESH
#define rtl8192cd_wlantx(p,t)	rtl8192cd_firetx(p, t)
#endif

enum _TX_QUEUE_ {
	MGNT_QUEUE		= 0,
	BK_QUEUE		= 1,
	BE_QUEUE		= 2,
	VI_QUEUE		= 3,
	VO_QUEUE		= 4,
	HIGH_QUEUE		= 5,
	BEACON_QUEUE	= 6
};

#define MCAST_QNUM		HIGH_QUEUE


// the purpose if actually just to link up all the desc in the same q
static __inline__ void init_txdesc(struct rtl8192cd_priv *priv, struct tx_desc *pdesc,
				unsigned long ringaddr, unsigned int i)
{
	if (i == (NUM_TX_DESC - 1))
		(pdesc + i)->Dword10 = set_desc(ringaddr); // NextDescAddress
	else
		(pdesc + i)->Dword10 = set_desc(ringaddr + (i+1) * sizeof(struct tx_desc)); // NextDescAddress
}

static __inline__ unsigned int get_mpdu_len(struct tx_insn *txcfg, unsigned int fr_len)
{
	return (txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->icv + txcfg->mic + _CRCLNG_ + fr_len);
}

#define txdesc_rollover(ptxdesc, ptxhead)	(*ptxhead = (*ptxhead + 1) % NUM_TX_DESC)

#define txdesc_rollback(ptxhead)			(*ptxhead = (*ptxhead == 0)? (NUM_TX_DESC - 1) : (*ptxhead - 1))

static __inline__ void tx_poll(struct rtl8192cd_priv *priv, int q_num)
{
	unsigned char val = 0;

#ifdef CONFIG_RTL8671
#ifdef CONFIG_CPU_RLX4181
	r3k_flush_dcache_range(0,0);
#endif
#endif

	switch (q_num) {
	case MGNT_QUEUE:
		val = MGQ_POLL;
		break;
	case BK_QUEUE:
		val = BKQ_POLL;
		break;
	case BE_QUEUE:
		val = BEQ_POLL;
		break;
	case VI_QUEUE:
		val = VIQ_POLL;
		break;
	case VO_QUEUE:
		val = VOQ_POLL;
		break;
	case HIGH_QUEUE:
		val = HQ_POLL;
		break;
	default:
		break;
	}
	RTL_W8(PCIE_CTRL_REG, val);
}

#define desc_copy(dst, src)		memcpy(dst, src, 32)

#define descinfo_copy(d, s)										\
	do {														\
		struct tx_desc_info	*dst = (struct tx_desc_info	*)d;	\
		struct tx_desc_info	*src = (struct tx_desc_info	*)s;	\
		dst->type  = src->type;									\
		dst->len   = src->len;									\
		dst->rate  = src->rate;									\
	} while (0)


#ifdef WDS
#define DECLARE_TXINSN(A)	struct tx_insn A;	\
	do {										\
		memset(&A, 0, sizeof(struct tx_insn));	\
		A.wdsIdx = -1;							\
	} while (0)

#define DECLARE_TXCFG(P, TEMPLATE)	struct tx_insn *P = &(TEMPLATE);	\
	do {																\
		memset(P, 0, sizeof(struct tx_insn));							\
		P->wdsIdx = -1;													\
	} while (0)

#else
#define DECLARE_TXINSN(A)	struct tx_insn A;	\
	do {										\
		memset(&A, 0, sizeof(struct tx_insn));	\
	} while (0)

#define DECLARE_TXCFG(P, TEMPLATE)	struct tx_insn* P = &(TEMPLATE);	\
	do {																\
		memset(P, 0, sizeof(struct tx_insn));							\
	} while (0)

#endif // WDS

#endif // _8192CD_TX_H_

