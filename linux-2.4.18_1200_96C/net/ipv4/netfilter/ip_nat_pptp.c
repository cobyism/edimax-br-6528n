/* PPTP extension for TCP and GRE NAT alteration. 
 * Brian Kuschak <bkuschak@yahoo.com> with some help from
 * Galen Hazelwood <galenh@esoft.com>
 */
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_nat_pptp.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
#include <asm/mips16_lib.h>
#endif

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#if defined(CONFIG_RTK_IPTABLES_FAST_PATH)
	#define FAST_PPTP
#endif

#if defined(FAST_PPTP) && !defined(CONFIG_FAST_PATH_MODULE)
extern void fast_pptp_map_rx_callID(struct pptp_gre_hdr *check_greh, unsigned int LANHost);
#endif

#if defined(FAST_PPTP) && defined(CONFIG_FAST_PATH_MODULE)
void (*fast_pptp_map_rx_callID_hook)(struct pptp_gre_hdr *check_greh, unsigned int LANHost)=NULL;
#endif
#if defined(FAST_PPTP) && defined(CONFIG_FAST_PATH_MODULE)
EXPORT_SYMBOL(fast_pptp_map_rx_callID_hook);
#endif
/* FIXME: Time out? --RR */
#if 0	/* bk not sure if we really need this... */
/*
 *	this function will get called in response to receiving an expected GRE packet, as long
 *	as there exists no NAT setup for this connection.  so it will be called twice when a 
 *	new PPTP/GRE connection is opened.
 */
static int
pptp_nat_expected(struct sk_buff **pskb,
		 unsigned int hooknum,
		 struct ip_conntrack *ct,
		 struct ip_nat_info *info,
		 struct ip_conntrack *master,
		 struct ip_nat_info *masterinfo,
		 unsigned int *verdict)
{
	struct ip_nat_multi_range mr;
	u_int32_t newdstip, newsrcip, newip;
	struct ip_ct_pptp *pptpinfo;
	struct ip_nat_pptp_info *natinfo;

	IP_NF_ASSERT(info);
	IP_NF_ASSERT(master);
	IP_NF_ASSERT(masterinfo);

	IP_NF_ASSERT(!(info->initialized & (1<<HOOK2MANIP(hooknum))));

	/* This function gets called for *ALL* registered expectations, not just
	 * PPTP.  We use the 1723 port number in the ip_ct_pptp union as an identifier.
	 */
	pptpinfo = &master->help.ct_pptp_info;

	/* TODO - also make sure it is IPPROTO_GRE */

	LOCK_BH(&ip_pptp_lock);
	if (!(pptpinfo->pptp_magic == PPTP_TCP_PORT)) {
		UNLOCK_BH(&ip_pptp_lock);
		DEBUGP("nat_expected: master not pptp\n");
		return 0;
	}
	natinfo = (struct ip_nat_pptp_info*) masterinfo;

	DEBUGP("nat_expected: We have a pptp connection!\n");
	DEBUGP("nat_expected: CID=%d, MCID=%d\n", 
		ntohs(natinfo->call_id), ntohs(natinfo->mcall_id));

	/* ip_nat_setup_info() sets up manipulations in both directions, but only
	 * initialized for one hook.  For the PREROUTING hook, setup a manipulation
	 * that makes no changes - will prevent this fn from being called continuously
	 */
	if(hooknum == NF_IP_POST_ROUTING)
	{
		newdstip = master->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
		newsrcip = master->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
		DEBUGP("nat_expected: GRE %u.%u.%u.%u -> %u.%u.%u.%u\n",
		       	NIPQUAD(newsrcip), NIPQUAD(newdstip));
	}
	else if(hooknum == NF_IP_PRE_ROUTING)
	{
		newsrcip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
		newdstip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
		DEBUGP("nat_expected: no changes GRE %u.%u.%u.%u -> %u.%u.%u.%u\n",
		       	NIPQUAD(newsrcip), NIPQUAD(newdstip));
	}
	UNLOCK_BH(&ip_pptp_lock);

	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC)
		newip = newsrcip;
	else
		newip = newdstip;

	DEBUGP("nat_expected: IP to %u.%u.%u.%u\n", NIPQUAD(newip));

	mr.rangesize = 1;
	/* We don't want to manip the per-protocol, just the IPs... */
	mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
	mr.range[0].min_ip = mr.range[0].max_ip = newip;

	*verdict = ip_nat_setup_info(ct, &mr, hooknum);

	return 1;
}
#endif

/*
 *	called from PRE/POSTROUING hook for TCP/1723 packets.
 *	masquerade the outgoing call id, demasq the incoming call id.
 */
static unsigned int pptp_help(	struct ip_conntrack *ct,
		                struct ip_conntrack_expect *exp,
			 	struct ip_nat_info *info,
			 	enum ip_conntrack_info ctinfo,
			 	unsigned int hooknum,
			 	struct sk_buff **pskb)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	unsigned int tcplen = (*pskb)->len - iph->ihl * 4;
	struct pptp_pkt_hdr		*pptph;
	struct PptpControlHeader	*ctlh;
        union {
                void				*rawreq;
                struct PptpOutCallRequest       *ocreq;
                struct PptpOutCallReply         *ocack;
                struct PptpInCallRequest        *icreq;
                struct PptpInCallReply          *icack;
		struct PptpClearCallRequest	*clrreq;
                struct PptpCallDisconnectNotify *disc;
                struct PptpWanErrorNotify       *wanerr;
                struct PptpSetLinkInfo          *setlink;
        } pptpReq;
	__u16                           msg, pcid;
	unsigned int 			datalen;
	int				dir;
	struct ip_nat_pptp_info *nat = &ct->nat.help.pptp_info;
#if 1

	/* Only mangle things once: original direction in POST_ROUTING
	   and reply direction on PRE_ROUTING. */
	dir = CTINFO2DIR(ctinfo);
	if (!((hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
	      || (hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY))) {
		DEBUGP("nat_pptp: Not touching dir %s at hook %s\n",
		       dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
		       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
		       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
		       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???");
		return NF_ACCEPT;
	}

	pptph = (struct pptp_pkt_hdr *) 
		((char *) iph + sizeof(struct iphdr) + sizeof(struct tcphdr));
	ctlh = (struct PptpControlHeader *) ((char *) pptph + sizeof(struct pptp_pkt_hdr));
	pptpReq.rawreq = (void *) ((char*) ctlh + sizeof(struct PptpControlHeader));

	LOCK_BH(&ip_pptp_lock);

	/* 
	 *	for original direction (outgoing), masquerade the CID 
	 * 	select the masqueraded id on the call_request cmd 
	 *	use tcp source port as masq call id
	 */
	if(hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
	{
		nat->mcall_id = tcph->source;

		switch (msg = htons(ctlh->messageType)) {
			case PPTP_OUT_CALL_REQUEST:
				/* masq'd client initiating connection to server */
				pptpReq.ocreq->callID = nat->mcall_id;
				break;
	
			case PPTP_IN_CALL_REQUEST:
				/* masq'd client initiating connection to server */
				pptpReq.icreq->callID = nat->mcall_id;
				break;	
	
			case PPTP_CALL_CLEAR_REQUEST:
				/* masq'd client sending to server */
				pptpReq.clrreq->callID = nat->mcall_id;
				break;
	
			case PPTP_CALL_DISCONNECT_NOTIFY:
				/* masq'd client sending to server */
				pptpReq.disc->callID = nat->mcall_id;
				break;
	
			default:
				DEBUGP("UNKNOWN inbound packet\n");
				/* fall through */
	
			case PPTP_SET_LINK_INFO:
			case PPTP_START_SESSION_REQUEST:
			case PPTP_START_SESSION_REPLY:
			case PPTP_STOP_SESSION_REQUEST:
			case PPTP_STOP_SESSION_REPLY:
			case PPTP_ECHO_REQUEST:
			case PPTP_ECHO_REPLY:
				/* no need to alter packet */
				UNLOCK_BH(&ip_pptp_lock);
				return NF_ACCEPT;
		}
		DEBUGP("pptp_nat_help: Masq   original CID=%d as MCID=%d\n", 
			ntohs(nat->call_id), ntohs(nat->mcall_id));
	}
	

	/*
	 *	for reply direction (incoming), demasquerade the peer CID 
	 *	lookup original CID in NAT helper struct
	 */
	if(hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY)
	{
		switch (msg = htons(ctlh->messageType)) {
			case PPTP_OUT_CALL_REPLY:
				/* server responding to masq'd client */
				//cid = &pptpReq.ocack->callID;
				pcid = pptpReq.ocack->peersCallID;
				DEBUGP("Changing incoming peer call ID from %d to %d\n", 
					ntohs(pcid), ntohs(nat->call_id));
				pptpReq.ocack->peersCallID = nat->call_id;
				break;
	
			case PPTP_IN_CALL_REPLY:
				/* server responding to masq'd client */
				//cid = &pptpReq.icack->callID;
				pcid = pptpReq.icack->peersCallID;
				pptpReq.icack->peersCallID = nat->call_id;
				break;
	
			case PPTP_WAN_ERROR_NOTIFY:
				/* server notifying masq'd client */
				pptpReq.wanerr->peersCallID = nat->call_id;
				break;
	
			case PPTP_SET_LINK_INFO:
				/* server notifying masq'd client */
				pptpReq.setlink->peersCallID = nat->call_id;
				break;
	
			default:
				DEBUGP("UNKNOWN inbound packet\n");
				/* fall through */
	
			case PPTP_START_SESSION_REQUEST:
			case PPTP_STOP_SESSION_REQUEST:
			case PPTP_ECHO_REQUEST:
				/* no need to alter packet */
				UNLOCK_BH(&ip_pptp_lock);
				return NF_ACCEPT;
		}
		DEBUGP("pptp_nat_help: Demasq original MPCID=%d as PCID=%d\n", 
			ntohs(pcid), ntohs(nat->call_id));
	}
	
	UNLOCK_BH(&ip_pptp_lock);

	/* recompute checksum */
	datalen = (*pskb)->len - iph->ihl * 4 - tcph->doff * 4;
	(*pskb)->csum = csum_partial((char *)tcph + tcph->doff*4, datalen, 0);
	tcph->check = 0;
	tcph->check = tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
			csum_partial((char *)tcph, tcph->doff*4, (*pskb)->csum));
#endif
	return NF_ACCEPT;
}

/*
 *	called from PRE/POSTROUING hook for GRE packets.
 *	demasq the incoming call id
 */
static unsigned int gre_help(	struct ip_conntrack *ct,
		                struct ip_conntrack_expect *exp,
			 	struct ip_nat_info *info,
			 	enum ip_conntrack_info ctinfo,
			 	unsigned int hooknum,
			 	struct sk_buff **pskb)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct pptp_gre_hdr *greh;
	int dir;
	struct ip_conntrack *master;
	struct ip_nat_pptp_info *nat;
	u_int32_t newdst, newsrc;
#if 1
	dir = CTINFO2DIR(ctinfo);

	LOCK_BH(&ip_pptp_lock);
	master = ct->help.ct_pptp_info.master;
	if(master == NULL || master->help.ct_pptp_info.pptp_magic != PPTP_TCP_PORT) {
		DEBUGP("gre_help: Not part of an established PPTP connection\n");
		UNLOCK_BH(&ip_pptp_lock);
		return NF_ACCEPT;
	} else {
		nat = &master->nat.help.pptp_info;
		greh = (struct pptp_gre_hdr*) ((char *) iph + sizeof(struct iphdr));
	}

	if(!ct->nat.help.pptp_info.serv_to_client)
	{
		/* 
	 	 *	for original direction (outgoing), do nothing
	 	 */
		if(hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL) {
			DEBUGP("gre_help: outgoing CID=%d\n", ntohs(greh->call_id));
		}
	
		/*
	 	 *	for reply direction (incoming), demasquerade the call id
	 	 *	lookup original CID in master's NAT helper struct
	 	 */
		else if(hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY) {
			if(greh->call_id != nat->mcall_id)
				DEBUGP("Whoops!  Incoming call ID isn't what we expect "
					"(expected %d, recv %d)!\n", 
					ntohs(nat->mcall_id), ntohs(greh->call_id));
	
			DEBUGP("gre_help: CT=%lx, master CT=%lx, MCID=%d, demasq CID=%d\n", 
				(unsigned long) ct, (unsigned long) master, 
				ntohs(greh->call_id), ntohs(nat->call_id));
	
			greh->call_id = nat->call_id;
             #if defined(FAST_PPTP) 
             #if defined(CONFIG_FAST_PATH_MODULE)
			if( fast_pptp_map_rx_callID_hook != NULL)
				fast_pptp_map_rx_callID_hook(greh,master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
		#else
			fast_pptp_map_rx_callID(greh,master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
		#endif
		#endif

		}
	}
	else
	{
		/*
	 	 *	if the server sent us packets first, orig and reply are reversed...
	 	 *	for orig direction (incoming), demasquerade the call id
	 	 *	lookup original CID in master's NAT helper struct
	 	 */
		if(hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_ORIGINAL) {
			DEBUGP("Fixing up packets from server first!\n");
	
			if(greh->call_id != nat->mcall_id)
				DEBUGP("Whoops!  Incoming call ID isn't what we expect "
					"(expected %d, recv %d)!\n", 
					ntohs(nat->mcall_id), ntohs(greh->call_id));
	
			DEBUGP("gre_help: CT=%lx, master CT=%lx, MCID=%d, demasq CID=%d\n", 
				(unsigned long) ct, (unsigned long) master, 
				ntohs(greh->call_id), ntohs(nat->call_id));
	
			greh->call_id = nat->call_id;
	             #if defined(FAST_PPTP) 
	             #if defined(CONFIG_FAST_PATH_MODULE)
				if( fast_pptp_map_rx_callID_hook != NULL)
					fast_pptp_map_rx_callID_hook(greh,master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
			#else
				fast_pptp_map_rx_callID(greh,master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
			#endif
			#endif

			/* We might have to demasquerade the IP address also...
		 	*/
			newdst = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
			iph->check = ip_nat_cheat_check(~iph->daddr, newdst, iph->check);
			iph->daddr = newdst;

			DEBUGP("PPTP Mangling %p: DST to %u.%u.%u.%u\n",
				*pskb, NIPQUAD(newdst));

		}
		else if(hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_REPLY) {
			
			/* Masquerade the source IP
			 */
			newsrc = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
			iph->check = ip_nat_cheat_check(~iph->saddr, newsrc, iph->check);
			iph->saddr = newsrc;

			DEBUGP("PPTP Mangling %p: SRC to %u.%u.%u.%u\n",
				*pskb, NIPQUAD(newsrc));
		}
	}
	UNLOCK_BH(&ip_pptp_lock);
#endif
	return NF_ACCEPT;
}

static struct ip_nat_helper pptp = { { NULL, NULL },
	                             "pptp",
				     0,
				     THIS_MODULE,
				    { { 0, { __constant_htons(1723) } },
				      { 0, { 0 }, IPPROTO_TCP } },
				    { { 0, { 0xFFFF } },
				      { 0, { 0 }, 0xFFFF } },
				    pptp_help, NULL };

static struct ip_nat_helper gre =  { { NULL, NULL },
	                             "gre",
				     IP_NAT_HELPER_F_ALWAYS,
				     THIS_MODULE,
				    { { 0, { 0 } },
				      { 0, { 0 }, IPPROTO_GRE } },
				    { { 0, { 0 } },
				      { 0, { 0 }, 0xFFFF } },
				    gre_help, NULL };

//static struct ip_nat_expect pptp_expect
//= { { NULL, NULL }, pptp_nat_expected };

static int __init init(void)
{
	int ret;

	//ret = ip_nat_expect_register(&pptp_expect);
	ret = ip_nat_helper_register(&pptp);
	if (ret == 0) {
		ret = ip_nat_helper_register(&gre);
		if (ret != 0)
			ip_nat_helper_unregister(&pptp);
		else
			printk("PPTP netfilter NAT helper: registered\n");
	}
	return ret;
}

static void __exit fini(void)
{
	ip_nat_helper_unregister(&gre);
	ip_nat_helper_unregister(&pptp);
}

module_init(init);
module_exit(fini);


