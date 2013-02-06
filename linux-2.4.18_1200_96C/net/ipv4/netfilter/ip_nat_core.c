/* NAT for netfilter; shared with compatibility layer. */

/* (c) 1999 Paul `Rusty' Russell.  Licenced under the GNU General
   Public Licence. */
#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/brlock.h>
#include <linux/vmalloc.h>
#include <net/checksum.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/tcp.h>  /* For tcp_prot in getorigdst */

#define ASSERT_READ_LOCK(x) MUST_BE_READ_LOCKED(&ip_nat_lock)
#define ASSERT_WRITE_LOCK(x) MUST_BE_WRITE_LOCKED(&ip_nat_lock)

#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_protocol.h>
#include <linux/netfilter_ipv4/ip_nat_core.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/listhelp.h>
#include <linux/ctype.h>
#ifdef CONFIG_RTL865X_HW_TABLES
#include <linux/romedrv_linux.h>
	#ifdef CONFIG_RTL865X_HW_PPTPL2TP
		#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>
	#endif
#endif

#if defined(CONFIG_PROC_FS)
#include <linux/proc_fs.h>
#endif

#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
#include <asm/mips16_lib.h>
#endif

#if defined(CONFIG_NET_QOS) || defined(CONFIG_RTL865X_HW_TABLES)
int gQosEnabled=0;

	#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	extern int auto_distribute_bandwidth;
	#define CONFIG_QOS_AUTO_SHARE_BW 1 //mark_test , for KLD auto shareBW
	#endif

#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
#define RTK_PORT_SHIFT_MAPPING 1 //for port shift mapping ,mark_dbg
#endif
#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

DECLARE_RWLOCK(ip_nat_lock);
DECLARE_RWLOCK_EXTERN(ip_conntrack_lock);

/* Calculated at init based on memory size */
static unsigned int ip_nat_htable_size;

static struct list_head *bysource;
static struct list_head *byipsproto;
LIST_HEAD(protos);
LIST_HEAD(helpers);

extern struct ip_nat_protocol unknown_nat_protocol;

#if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_QOS_UI_BY_BANDWIDTH) && !defined(CONFIG_RTL8196C_KLD)
static int checkQosSetting(u_int16_t proto, u_int32_t sip, u_int32_t dip,u_int16_t sp, u_int16_t dp);
#endif

#ifdef CONFIG_RTL865X_HW_TABLES
int gHwNatEnabled=1;

int rtl865x_handle_nat(struct ip_conntrack *ct, int act)
{
	struct ip_nat_info *info=&ct->nat.info;
	u_int32_t sip, dip, gip;
	u_int16_t sp, dp, gp, proto=0;
	u_int32_t timeval;
	int rc=0;

	if (gHwNatEnabled != 1)
		return -1;
    
	proto = (ct->tuplehash[0].tuple.dst.protonum==IPPROTO_TCP)? 1: 0;

	if (info->manips[0].direction==IP_CT_DIR_ORIGINAL &&
	   info->manips[0].maniptype==IP_NAT_MANIP_SRC)
	{ /* outbound flow */
		sip	= ct->tuplehash[0].tuple.src.ip;
		dip 	= ct->tuplehash[0].tuple.dst.ip;
		gip 	= ct->tuplehash[1].tuple.dst.ip;
		sp  	= (proto)? ct->tuplehash[0].tuple.src.u.tcp.port: ct->tuplehash[0].tuple.src.u.udp.port;
		dp  	= (proto)? ct->tuplehash[0].tuple.dst.u.tcp.port: ct->tuplehash[0].tuple.dst.u.udp.port;
		gp  	= (proto)? ct->tuplehash[1].tuple.dst.u.tcp.port: ct->tuplehash[1].tuple.dst.u.udp.port;
	} 
	else if (info->manips[0].direction == IP_CT_DIR_ORIGINAL &&
	   info->manips[0].maniptype==IP_NAT_MANIP_DST)
	{ /* inbound flow */
		sip	= ct->tuplehash[1].tuple.src.ip;
		dip 	= ct->tuplehash[1].tuple.dst.ip;
		gip 	= ct->tuplehash[0].tuple.dst.ip;
		sp  	= (proto)? ct->tuplehash[1].tuple.src.u.tcp.port: ct->tuplehash[1].tuple.src.u.udp.port;
		dp  	= (proto)? ct->tuplehash[1].tuple.dst.u.tcp.port: ct->tuplehash[1].tuple.dst.u.udp.port;
		gp  	= (proto)? ct->tuplehash[0].tuple.dst.u.tcp.port: ct->tuplehash[0].tuple.dst.u.udp.port;
	}
	else 
		return -1;

	/* do not add hardware NAPT table if protocol is UDP and source IP address is equal to gateway IP address */
	if ((act == 1) && (proto == 0) && (sip == gip))
		return -1;

	/* for TZO DDNS */
	if ((act == 1) && (proto == 1) && (dp == 21347)) {
		return -1;
	}

	if (act == 2) {
		/* query for idle */
		timeval = rtl865x_naptSync(proto, sip, sp, gip, gp, dip, dp, 0);
		if (timeval > 0)
			return 0;
		else
			return -1;
	}
	else if (act == 0) {
		/* delete */
		rc = rtl865x_delNaptConnection(proto, sip, sp, gip, gp, dip, dp);
	}
	else {
		/* add */
#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_QOS)
		if ((gQosEnabled == 0) 
			#if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_QOS_UI_BY_BANDWIDTH) && !defined(CONFIG_RTL8196C_KLD)
			|| checkQosSetting(proto, sip, dip, sp, dp)
			#endif
			)
			rc =  rtl865x_addNaptConnection(proto, sip, sp, gip, gp, dip, dp, 0);
		else
			act = 0;
#else
		rc =  rtl865x_addNaptConnection(proto, sip, sp, gip, gp, dip, dp, 0);
#endif
       }
    
	if (!rc && act == 1) /* mark it as an asic entry */
		info->hw_acc = 1;
	if (!rc && act == 0) /* unmark it */
		info->hw_acc = 0;
	
	return 0;	
}

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
unsigned char acceleratePppMac[ETH_ALEN] = { 0x00, 0xe0, 0x4c, 0x00, 0x00, 0x01 };
unsigned char accelerateDevMac[ETH_ALEN] = { 0x00, 0xe0, 0x4c, 0x00, 0x00, 0x00 };

struct pptp_info {
	int valid;
	struct net_device *wanDev;
	unsigned char ourMac[ETH_ALEN];
	__u16 ourCallID;
	__u32 ourIp;
	unsigned char peerMac[ETH_ALEN];
	__u16 peerCallID;
	__u32 peerIp;
	unsigned int tx_seqno;
	unsigned int rx_seqno;
	__u16 tx_ipID;
	__u16 ipID;
	unsigned int tx_seqno_daemon;
	unsigned int rx_seqno_daemon;
	struct net_device *pppDev;
	struct ppp_channel *pppChan;
};
static struct pptp_info pptpInfo = { 0 };

int rtl865x_addPppInfoChannel(struct net_device *pppDev, struct ppp_channel *pppChan)
{
	if (!pppDev || !pppChan)
		return -1;

	pptpInfo.pppDev = pppDev;
	pptpInfo.pppChan = pppChan;
	return 0;
}

int rtl865x_delPppInfoChannel(struct net_device *pppDev, struct ppp_channel *pppChan)
{
	if (!pppDev || !pppChan)
		return -1;

	if (pptpInfo.pppDev == pppDev) {
		pptpInfo.pppDev = NULL;
		pptpInfo.pppChan = NULL;
	}
	return 0;
}

int rtl865x_filterPptp(struct sk_buff *skb)
{
	struct ethhdr *eth = skb->mac.ethernet;
	struct iphdr *iph = skb->nh.iph;
	unsigned int iphlen = iph->ihl << 2;
	struct tcphdr *tcph = (struct tcphdr *)((char *)iph + iphlen);
	unsigned int tcphlen = tcph->doff << 2;
	unsigned int tcplen = iph->tot_len - (iph->ihl << 2);

	/* Not whole TCP header? */
	if (tcplen < sizeof(struct tcphdr) || tcplen < tcphlen) {
		return -1;
	}
	
	if (iph->protocol == IPPROTO_TCP && ntohs(tcph->source) == PPTP_TCP_PORT) {
		struct pptp_pkt_hdr	*pptph;
		struct PptpControlHeader *ctlh;
		struct PptpOutCallReply *ocack;

		if (tcph->rst || tcph->fin) {
			/* If we get a FIN or RST, this connection's going down, and so is */
			/* the GRE tunnel. Deal. */
			if (pptpInfo.ourIp == iph->daddr && pptpInfo.peerIp == iph->saddr) {
				pptpInfo.valid = 0;			
				return 0;				
			}
		}
	
		pptph = (struct pptp_pkt_hdr *)((char *)iph + iphlen + tcphlen);
		/* if it's not a control message we can't do anything with it */
        if (ntohs(pptph->packetType) != PPTP_CONTROL_PACKET ||
	    	ntohl(pptph->magicCookie) != PPTP_MAGIC_COOKIE) {
			return -1;
		}

		ctlh = (struct PptpControlHeader *)((char *)pptph + sizeof(struct pptp_pkt_hdr));
		if (ntohs(ctlh->messageType) == PPTP_OUT_CALL_REPLY) {
			ocack = (struct PptpOutCallReply *)((char *)ctlh + sizeof(struct PptpControlHeader));
			if (ocack->resultCode == PPTP_OUTCALL_CONNECT) {
				pptpInfo.valid = 1;
				pptpInfo.wanDev = skb->dev;    				
				pptpInfo.ourCallID = ocack->peersCallID;
				pptpInfo.peerCallID = ocack->callID;
				pptpInfo.ourIp = iph->daddr;
				pptpInfo.peerIp = iph->saddr;	
				memcpy(pptpInfo.ourMac, eth->h_dest, ETH_ALEN);
				memcpy(pptpInfo.peerMac, eth->h_source, ETH_ALEN);
				pptpInfo.tx_seqno = 1;
				pptpInfo.tx_ipID = iph->id + 1;    				
			}
		}
	}

	return 0;
}

int rtl865x_acceleratePptpToWanSync(struct sk_buff *skb)
{
	struct iphdr /* ip,*/ *iph = (struct iphdr *)(skb->data + ETH_HLEN);
	struct pptp_gre_hdr /* gre,*/ *greh = (struct pptp_gre_hdr *)((char *)iph + (iph->ihl << 2));

	if (gHwNatEnabled != 1)
		return -1;

	if (!pptpInfo.valid || !pptpInfo.pppDev || !pptpInfo.wanDev) {
		return -1;
	}

	if (iph->protocol == IPPROTO_GRE && skb->len >= (ETH_HLEN + (iph->ihl << 2) + sizeof(struct pptp_gre_hdr) - 8)) {
		if ((greh->version & 7) == PPTP_GRE_VERSION &&
			ntohs(greh->protocol) == PPTP_GRE_PROTOCOL) {
			if (skb->dev == pptpInfo.wanDev &&
				iph->saddr == pptpInfo.ourIp && 
				iph->daddr == pptpInfo.peerIp && 
				greh->call_id == pptpInfo.peerCallID) {
				/* Matched */
				if (PPTP_GRE_IS_S(greh->flags)) {
					/* Others go head !? */
					unsigned int seq = ntohl(greh->seq);
					if (time_after(seq, pptpInfo.tx_seqno)) {
						pptpInfo.tx_seqno = seq + 1;	
					}
					else {
	   					greh->seq = htonl(pptpInfo.tx_seqno++);	
	   				}
				}			
				if (PPTP_GRE_IS_A(greh->version)) {
					/* Others go head !? */
					unsigned int ack = ntohl(greh->ack);
					if (time_after(ack, pptpInfo.rx_seqno)) {
						pptpInfo.rx_seqno = ack;	
					}
					else {
	   					greh->ack = htonl(pptpInfo.rx_seqno);	
	   				}					
				}
			}
		}
	}			

	return 0;
}

int rtl865x_acceleratePptpToWAN(struct sk_buff *skb)
{
	int	headroom;
	struct ethhdr *eth;
	struct iphdr ip, *iph;
	struct pptp_gre_hdr	gre, *greh;	
	unsigned char tos = skb->nh.iph->tos;

	headroom = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct pptp_gre_hdr);
	if (skb_headroom(skb) < headroom || skb_cloned(skb) || skb_shared(skb)) {	
		struct sk_buff *new_skb = skb_realloc_headroom(skb, headroom);				
		if (!new_skb) {
			return -1;
		}									
		dev_kfree_skb(skb);
		skb = new_skb;
	}			

	eth = (struct ethhdr *)skb_push(skb, headroom);;
	memcpy(eth->h_dest, pptpInfo.peerMac, ETH_ALEN);
	memcpy(eth->h_source, pptpInfo.ourMac, ETH_ALEN);
	eth->h_proto = htons(0x0800);

	iph = (struct iphdr *)((char *)eth + sizeof(struct ethhdr));
	if (((unsigned long)iph) & 0x03) {
		ip.version  = 4;
		ip.ihl      = sizeof(struct iphdr) >> 2;
		ip.frag_off = 0;			
		ip.protocol = IPPROTO_GRE;
		ip.tos      = tos;
		ip.daddr    = pptpInfo.peerIp;
		ip.saddr    = pptpInfo.ourIp;
		ip.ttl      = IPDEFTTL;   
		ip.tot_len  = htons(skb->len - sizeof(struct ethhdr));		
		ip.id       = htons(++pptpInfo.tx_ipID);
	   	ip.check    = 0;
	   	///- ip.check    = ip_fast_csum((unsigned char *)&ip, ip.ihl);
		memcpy(iph, &ip, sizeof(struct iphdr));
	}
	else {
		iph->version  = 4;
		iph->ihl      = sizeof(struct iphdr) >> 2;
		iph->frag_off = 0;			
		iph->protocol = IPPROTO_GRE;
		iph->tos      = tos;
		iph->daddr    = pptpInfo.peerIp;
		iph->saddr    = pptpInfo.ourIp;
		iph->ttl      = IPDEFTTL;   
		iph->tot_len  = htons(skb->len - sizeof(struct ethhdr));		
		iph->id       = htons(++pptpInfo.tx_ipID);
	   	iph->check    = 0;
	   	///- iph->check    = ip_fast_csum((unsigned char *)iph, iph->ihl);
	}		
	///- skb->ip_summed = CHECKSUM_NONE;
	skb->ip_summed = CHECKSUM_HW;

	greh = (struct pptp_gre_hdr *)((char *)iph + sizeof(struct iphdr));
	if (((unsigned long)greh) & 0x03) {
	   	gre.flags       = PPTP_GRE_FLAG_K | PPTP_GRE_FLAG_S;
	   	gre.version     = PPTP_GRE_VERSION | PPTP_GRE_FLAG_A;
	   	gre.protocol    = htons(PPTP_GRE_PROTOCOL);
	   	gre.payload_len = htons(skb->len - headroom);    	
	   	gre.call_id     = pptpInfo.peerCallID;			
	   	gre.seq         = htonl(pptpInfo.tx_seqno++);		
	   	gre.ack         = htonl(pptpInfo.rx_seqno);
		memcpy(greh, &gre, sizeof(struct pptp_gre_hdr));
	}
	else {
	   	greh->flags       = PPTP_GRE_FLAG_K | PPTP_GRE_FLAG_S;
	   	greh->version     = PPTP_GRE_VERSION | PPTP_GRE_FLAG_A;
	   	greh->protocol    = htons(PPTP_GRE_PROTOCOL);
	   	greh->payload_len = htons(skb->len - headroom);    	
	   	greh->call_id     = pptpInfo.peerCallID;			
	   	greh->seq         = htonl(pptpInfo.tx_seqno++);		
	   	greh->ack         = htonl(pptpInfo.rx_seqno);
	}		

   	skb->dev = pptpInfo.wanDev;
	///- dev_queue_xmit(skb);
	pptpInfo.wanDev->hard_start_xmit(skb, skb->dev);
	return 0;
}

#if 0
int rtl865x_acceleratePptpToLAN(struct sk_buff *skb)
{
	struct iphdr ip, *iph = skb->nh.iph;
	struct pptp_gre_hdr gre, *greh = (struct pptp_gre_hdr *)(skb->nh.raw + (iph->ihl << 2));
	int offset;
	unsigned char *p;

	if (!pptpInfo.valid || !pptpInfo.pppDev || !pptpInfo.wanDev) {
		return NET_RX_SUCCESS;
	}

	/* PPTP GRE */
	if ((greh->version & 7) == PPTP_GRE_VERSION &&
		ntohs(greh->protocol) == PPTP_GRE_PROTOCOL) {
		/* Unalignment !? */
		if (((unsigned long)iph) & 0x03) {
			memcpy(&ip, iph, sizeof(struct iphdr));
			iph = &ip;
		}
		if (((unsigned long)greh) & 0x03) {
			memcpy(&gre, greh, sizeof(struct pptp_gre_hdr));
			greh = &gre;
		}
		if (skb->dev == pptpInfo.wanDev &&
			iph->daddr == pptpInfo.ourIp && 
			iph->saddr == pptpInfo.peerIp && 
			greh->call_id == pptpInfo.ourCallID) {
			/* Matched */
			if (greh->payload_len == 0)
				return NET_RX_SUCCESS;

			offset = (iph->ihl << 2) + sizeof(struct pptp_gre_hdr) - 8;				
			if (PPTP_GRE_IS_S(greh->flags)) {	
				pptpInfo.rx_seqno = ntohl(greh->seq);						
				offset += sizeof(greh->seq);
			}	
			if (PPTP_GRE_IS_A(greh->version)) {
				/* Others go head !? */
				unsigned int ack = ntohl(greh->ack);
				if (time_after(ack, pptpInfo.tx_seqno)) {
					pptpInfo.tx_seqno = ack + 1;	
				}
				offset += sizeof(greh->ack);
			}
			
			/* strip address/control field if present */
			p = skb->nh.raw + offset;
			if (p[0] == 0xff /* PPP_ALLSTATIONS */ && p[1] == 0x03 /* PPP_UI */) {			
				/* chop off address/control */
				if (greh->payload_len < 3)
					return NET_RX_SUCCESS;
				offset += 2;
				p += 2;
			}
			/* decompress protocol field if compressed */
			if (p[0] & 1) {
				/* protocol is compressed */
				offset -= 1;
				skb_pull(skb, offset)[0] = 0;
			} 
			else {
				if (greh->payload_len < 2)
					return NET_RX_SUCCESS;
				skb_pull(skb, offset);
			}
			
			skb->cb[0] = 'P'; skb->cb[1] = 'P'; skb->cb[2] = 'P';
			ppp_input(pptpInfo.pppChan, skb);
			return NET_RX_DROP;
		}
	}
	
	return NET_RX_SUCCESS;
}
#endif

int rtl865x_pppAcceleratePptpL2tpToWAN(struct sk_buff *skb, struct net_device *pppDev)
{
	if (gHwNatEnabled != 1)
		return -1;

	if (!pptpInfo.valid || !pptpInfo.pppDev || !pptpInfo.wanDev || pptpInfo.pppDev != pppDev) {
		return -1;
	}

	return (rtl865x_acceleratePptpToWAN(skb));
}

int rtl865x_pppAcceleratePptpL2tpToLAN(struct sk_buff *skb, struct net_device *pppDev)
{
	struct ethhdr *eth;

	if (gHwNatEnabled != 1)
		return -1;

	if (!pptpInfo.valid || !pptpInfo.pppDev || !pptpInfo.wanDev || pptpInfo.pppDev != skb->dev) {
		return -1;
	}

	if (skb_headroom(skb) < sizeof(struct ethhdr) || skb_cloned(skb) || skb_shared(skb)) {
		struct sk_buff *new_skb = skb_realloc_headroom(skb, sizeof(struct ethhdr));				
		if (!new_skb) {				
			return -1;
		}
		dev_kfree_skb(skb);
		skb = new_skb;
	}
		
	skb_push(skb, sizeof(struct ethhdr));
	eth = (struct ethhdr *)skb->data;
	memcpy(eth->h_dest, accelerateDevMac, ETH_ALEN);
	memcpy(eth->h_source, acceleratePppMac, ETH_ALEN);
	eth->h_proto = htons(0x0800);
	rtl865x_pptpl2tp_extport_xmit(skb);
	return 0;
}

int rtl865x_acceleratePptpL2tp(struct sk_buff **pskb) 
{
	struct sk_buff *skb = *pskb;
 	/* struct iphdr *iph;*/
	
	if (gHwNatEnabled != 1)
		return NET_RX_SUCCESS;
		
	if (ntohs(skb->mac.ethernet->h_proto) != 0x0800) {
		return NET_RX_SUCCESS;
	}

	skb->h.raw = skb->nh.raw = skb->data;
	if (skb->cb[0] == 'P' && skb->cb[1] == 'P' && skb->cb[2] == 'P') {
		if (pptpInfo.pppDev) {
			pptpInfo.pppDev->hard_start_xmit(skb, pptpInfo.pppDev);
			return NET_RX_DROP;
		}
		memset(skb->cb, '\x0', 3);
	}
	else if (skb->cb[0] == 'N' && skb->cb[1] == 'A' && skb->cb[2] == 'T') {
		memset(skb->cb, '\x0', 3);
		if (pptpInfo.pppDev) {
			skb->dev = pptpInfo.pppDev;
			return NET_RX_SUCCESS;
		}
	}
#if 0    
	else {
		iph = skb->nh.iph;
		if (iph->protocol == IPPROTO_GRE && skb->len >= ((iph->ihl << 2) + sizeof(struct pptp_gre_hdr) - 8)) {
			return (rtl865x_acceleratePptpToLAN(skb));
		}
	}
#endif	
	return NET_RX_SUCCESS;	
}	
#endif /* CONFIG_RTL865X_HW_PPTPL2TP */

#endif

/* We keep extra hashes for each conntrack, for fast searching. */
static inline size_t
hash_by_ipsproto(u_int32_t src, u_int32_t dst, u_int16_t proto)
{
	/* Modified src and dst, to ensure we don't create two
           identical streams. */
	return (src + dst + proto) % ip_nat_htable_size;
}

static inline size_t
hash_by_src(const struct ip_conntrack_manip *manip, u_int16_t proto)
{
	/* Original src, to ensure we map it consistently if poss. */
	return (manip->ip + manip->u.all + proto) % ip_nat_htable_size;
}

/* Noone using conntrack by the time this called. */
static void ip_nat_cleanup_conntrack(struct ip_conntrack *conn)
{
	struct ip_nat_info *info = &conn->nat.info;
	unsigned int hs, hp;

	if (!info->initialized)
		return;

	IP_NF_ASSERT(info->bysource.conntrack);
	IP_NF_ASSERT(info->byipsproto.conntrack);

	hs = hash_by_src(&conn->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src,
	                 conn->tuplehash[IP_CT_DIR_ORIGINAL]
	                 .tuple.dst.protonum);

	hp = hash_by_ipsproto(conn->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip,
	                      conn->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip,
	                      conn->tuplehash[IP_CT_DIR_REPLY]
	                      .tuple.dst.protonum);

	WRITE_LOCK(&ip_nat_lock);
	LIST_DELETE(&bysource[hs], &info->bysource);
	LIST_DELETE(&byipsproto[hp], &info->byipsproto);
	WRITE_UNLOCK(&ip_nat_lock);
}

/* We do checksum mangling, so if they were wrong before they're still
 * wrong.  Also works for incomplete packets (eg. ICMP dest
 * unreachables.) */
u_int16_t
ip_nat_cheat_check(u_int32_t oldvalinv, u_int32_t newval, u_int16_t oldcheck)
{
	u_int32_t diffs[] = { oldvalinv, newval };
	return csum_fold(csum_partial((char *)diffs, sizeof(diffs),
				      oldcheck^0xFFFF));
}

static inline int cmp_proto(const struct ip_nat_protocol *i, int proto)
{
	return i->protonum == proto;
}

struct ip_nat_protocol *
find_nat_proto(u_int16_t protonum)
{
	struct ip_nat_protocol *i;

	MUST_BE_READ_LOCKED(&ip_nat_lock);
	i = LIST_FIND(&protos, cmp_proto, struct ip_nat_protocol *, protonum);
	if (!i)
		i = &unknown_nat_protocol;
	return i;
}

/* Is this tuple already taken? (not by us) */
int
ip_nat_used_tuple(const struct ip_conntrack_tuple *tuple,
		  const struct ip_conntrack *ignored_conntrack)
{
	/* Conntrack tracking doesn't keep track of outgoing tuples; only
	   incoming ones.  NAT means they don't have a fixed mapping,
	   so we invert the tuple and look for the incoming reply.

	   We could keep a separate hash if this proves too slow. */
	struct ip_conntrack_tuple reply;

	invert_tuplepr(&reply, tuple);
	return ip_conntrack_tuple_taken(&reply, ignored_conntrack);
}

/* Does tuple + the source manip come within the range mr */
static int
in_range(const struct ip_conntrack_tuple *tuple,
	 const struct ip_conntrack_manip *manip,
	 const struct ip_nat_multi_range *mr)
{
	struct ip_nat_protocol *proto = find_nat_proto(tuple->dst.protonum);
	unsigned int i;
	struct ip_conntrack_tuple newtuple = { *manip, tuple->dst };

	for (i = 0; i < mr->rangesize; i++) {
		/* If we are allowed to map IPs, then we must be in the
		   range specified, otherwise we must be unchanged. */
		if (mr->range[i].flags & IP_NAT_RANGE_MAP_IPS) {
			if (ntohl(newtuple.src.ip) < ntohl(mr->range[i].min_ip)
			    || (ntohl(newtuple.src.ip)
				> ntohl(mr->range[i].max_ip)))
				continue;
		} else {
			if (newtuple.src.ip != tuple->src.ip)
				continue;
		}

		if ((mr->range[i].flags & IP_NAT_RANGE_PROTO_SPECIFIED)
		    && proto->in_range(&newtuple, IP_NAT_MANIP_SRC,
				       &mr->range[i].min, &mr->range[i].max))
			return 1;
	}
	return 0;
}

static inline int
src_cmp(const struct ip_nat_hash *i,
	const struct ip_conntrack_tuple *tuple,
	const struct ip_nat_multi_range *mr)
{
	return (i->conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum
		== tuple->dst.protonum
		&& i->conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip
		== tuple->src.ip
		&& i->conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all
		== tuple->src.u.all
		&& in_range(tuple,
			    &i->conntrack->tuplehash[IP_CT_DIR_ORIGINAL]
			    .tuple.src,
			    mr));
}

/* Only called for SRC manip */
static struct ip_conntrack_manip *
find_appropriate_src(const struct ip_conntrack_tuple *tuple,
		     const struct ip_nat_multi_range *mr)
{
	unsigned int h = hash_by_src(&tuple->src, tuple->dst.protonum);
	struct ip_nat_hash *i;

	MUST_BE_READ_LOCKED(&ip_nat_lock);
	i = LIST_FIND(&bysource[h], src_cmp, struct ip_nat_hash *, tuple, mr);
	if (i)
		return &i->conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src;
	else
		return NULL;
}

#ifdef CONFIG_IP_NF_NAT_LOCAL
/* If it's really a local destination manip, it may need to do a
   source manip too. */
static int
do_extra_mangle(u_int32_t var_ip, u_int32_t *other_ipp)
{
	struct rtable *rt;

	/* FIXME: IPTOS_TOS(iph->tos) --RR */
	if (ip_route_output(&rt, var_ip, 0, 0, 0) != 0) {
		DEBUGP("do_extra_mangle: Can't get route to %u.%u.%u.%u\n",
		       NIPQUAD(var_ip));
		return 0;
	}

	*other_ipp = rt->rt_src;
	ip_rt_put(rt);
	return 1;
}
#endif

/* Simple way to iterate through all. */
static inline int fake_cmp(const struct ip_nat_hash *i,
			   u_int32_t src, u_int32_t dst, u_int16_t protonum,
			   unsigned int *score,
			   const struct ip_conntrack *conntrack)
{
	/* Compare backwards: we're dealing with OUTGOING tuples, and
           inside the conntrack is the REPLY tuple.  Don't count this
           conntrack. */
	if (i->conntrack != conntrack
	    && i->conntrack->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip == dst
	    && i->conntrack->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip == src
	    && (i->conntrack->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum
		== protonum))
		(*score)++;
	return 0;
}

static inline unsigned int
count_maps(u_int32_t src, u_int32_t dst, u_int16_t protonum,
	   const struct ip_conntrack *conntrack)
{
	unsigned int score = 0;
	unsigned int h;

	MUST_BE_READ_LOCKED(&ip_nat_lock);
	h = hash_by_ipsproto(src, dst, protonum);
	LIST_FIND(&byipsproto[h], fake_cmp, struct ip_nat_hash *,
	          src, dst, protonum, &score, conntrack);

	return score;
}

/* For [FUTURE] fragmentation handling, we want the least-used
   src-ip/dst-ip/proto triple.  Fairness doesn't come into it.  Thus
   if the range specifies 1.2.3.4 ports 10000-10005 and 1.2.3.5 ports
   1-65535, we don't do pro-rata allocation based on ports; we choose
   the ip with the lowest src-ip/dst-ip/proto usage.

   If an allocation then fails (eg. all 6 ports used in the 1.2.3.4
   range), we eliminate that and try again.  This is not the most
   efficient approach, but if you're worried about that, don't hand us
   ranges you don't really have.  */
static struct ip_nat_range *
find_best_ips_proto(struct ip_conntrack_tuple *tuple,
		    const struct ip_nat_multi_range *mr,
		    const struct ip_conntrack *conntrack,
		    unsigned int hooknum)
{
	unsigned int i;
	struct {
		const struct ip_nat_range *range;
		unsigned int score;
		struct ip_conntrack_tuple tuple;
	} best = { NULL,  0xFFFFFFFF };
	u_int32_t *var_ipp, *other_ipp, saved_ip, orig_dstip;
	static unsigned int randomness = 0;

	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC) {
		var_ipp = &tuple->src.ip;
		saved_ip = tuple->dst.ip;
		other_ipp = &tuple->dst.ip;
	} else {
		var_ipp = &tuple->dst.ip;
		saved_ip = tuple->src.ip;
		other_ipp = &tuple->src.ip;
	}
	/* Don't do do_extra_mangle unless neccessary (overrides
           explicit socket bindings, for example) */
	orig_dstip = tuple->dst.ip;

	IP_NF_ASSERT(mr->rangesize >= 1);
	for (i = 0; i < mr->rangesize; i++) {
		/* Host order */
		u_int32_t minip, maxip, j;

		/* Don't do ranges which are already eliminated. */
		if (mr->range[i].flags & IP_NAT_RANGE_FULL) {
			continue;
		}

		if (mr->range[i].flags & IP_NAT_RANGE_MAP_IPS) {
			minip = ntohl(mr->range[i].min_ip);
			maxip = ntohl(mr->range[i].max_ip);
		} else
			minip = maxip = ntohl(*var_ipp);

		randomness++;
		for (j = 0; j < maxip - minip + 1; j++) {
			unsigned int score;

			*var_ipp = htonl(minip + (randomness + j) 
					 % (maxip - minip + 1));

			/* Reset the other ip in case it was mangled by
			 * do_extra_mangle last time. */
			*other_ipp = saved_ip;

#ifdef CONFIG_IP_NF_NAT_LOCAL
			if (hooknum == NF_IP_LOCAL_OUT
			    && *var_ipp != orig_dstip
			    && !do_extra_mangle(*var_ipp, other_ipp)) {
				DEBUGP("Range %u %u.%u.%u.%u rt failed!\n",
				       i, NIPQUAD(*var_ipp));
				/* Can't route?  This whole range part is
				 * probably screwed, but keep trying
				 * anyway. */
				continue;
			}
#endif

			/* Count how many others map onto this. */
			score = count_maps(tuple->src.ip, tuple->dst.ip,
					   tuple->dst.protonum, conntrack);
			if (score < best.score) {
				/* Optimization: doesn't get any better than
				   this. */
				if (score == 0)
					return (struct ip_nat_range *)
						&mr->range[i];

				best.score = score;
				best.tuple = *tuple;
				best.range = &mr->range[i];
			}
		}
	}
	*tuple = best.tuple;

	/* Discard const. */
	return (struct ip_nat_range *)best.range;
}

/* Fast version doesn't iterate through hash chains, but only handles
   common case of single IP address (null NAT, masquerade) */
static struct ip_nat_range *
find_best_ips_proto_fast(struct ip_conntrack_tuple *tuple,
			 const struct ip_nat_multi_range *mr,
			 const struct ip_conntrack *conntrack,
			 unsigned int hooknum)
{
	if (mr->rangesize != 1
	    || (mr->range[0].flags & IP_NAT_RANGE_FULL)
	    || ((mr->range[0].flags & IP_NAT_RANGE_MAP_IPS)
		&& mr->range[0].min_ip != mr->range[0].max_ip))
		return find_best_ips_proto(tuple, mr, conntrack, hooknum);

	if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS) {
		if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC)
			tuple->src.ip = mr->range[0].min_ip;
		else {
			/* Only do extra mangle when required (breaks
                           socket binding) */
#ifdef CONFIG_IP_NF_NAT_LOCAL
			if (tuple->dst.ip != mr->range[0].min_ip
			    && hooknum == NF_IP_LOCAL_OUT
			    && !do_extra_mangle(mr->range[0].min_ip,
						&tuple->src.ip))
				return NULL;
#endif
			tuple->dst.ip = mr->range[0].min_ip;
		}
	}

	/* Discard const. */
	return (struct ip_nat_range *)&mr->range[0];
}

#ifdef RTK_PORT_SHIFT_MAPPING	
static int find_port_shift_mapping(unsigned int hooknum,
     		 struct ip_nat_range *range,
		     struct ip_conntrack_tuple *tuple)
{
	unsigned short *portptr;
	signed short map_port;
	enum ip_nat_manip_type maniptype = HOOK2MANIP(hooknum);

	if (maniptype == IP_NAT_MANIP_SRC)
		portptr = &tuple->src.u.tcp.port;
	else
		portptr = &tuple->dst.u.tcp.port;
	map_port = ( (*portptr) + (signed short)(range->shift));

	//check man_port illegal ??  range.min.port <map_port <range.max.port	
	*portptr = htons(map_port);
	return 1;
}
#endif

static int
get_unique_tuple(struct ip_conntrack_tuple *tuple,
		 const struct ip_conntrack_tuple *orig_tuple,
		 const struct ip_nat_multi_range *mrr,
		 struct ip_conntrack *conntrack,
		 unsigned int hooknum)
{
	struct ip_nat_protocol *proto
		= find_nat_proto(orig_tuple->dst.protonum);
	struct ip_nat_range *rptr;
	unsigned int i;
	int ret;

	/* We temporarily use flags for marking full parts, but we
	   always clean up afterwards */
	struct ip_nat_multi_range *mr = (void *)mrr;

	/* 1) If this srcip/proto/src-proto-part is currently mapped,
	   and that same mapping gives a unique tuple within the given
	   range, use that.

	   This is only required for source (ie. NAT/masq) mappings.
	   So far, we don't do local source mappings, so multiple
	   manips not an issue.  */
	if (hooknum == NF_IP_POST_ROUTING) {
		struct ip_conntrack_manip *manip;

		manip = find_appropriate_src(orig_tuple, mr);
		if (manip) {
			/* Apply same source manipulation. */
			*tuple = ((struct ip_conntrack_tuple)
				  { *manip, orig_tuple->dst });
			DEBUGP("get_unique_tuple: Found current src map\n");
			return 1;
		}
	}

	/* 2) Select the least-used IP/proto combination in the given
	   range.
	*/
	*tuple = *orig_tuple;
	while ((rptr = find_best_ips_proto_fast(tuple, mr, conntrack, hooknum))
	       != NULL) {
		DEBUGP("Found best for "); DUMP_TUPLE(tuple);
		/* 3) The per-protocol part of the manip is made to
		   map into the range to make a unique tuple. */
#ifdef RTK_PORT_SHIFT_MAPPING	
#define IP_NAT_PORT_SHIFT_MAPPING 0x80 //it's a special RTK's flag from iptables.(also define in libipt_DNAT.c)
		//this patch will find the port shift mapping nat!!		
		if( (rptr->flags & IP_NAT_PORT_SHIFT_MAPPING) && (rptr->shift!=0) )
		{
			if( ((rptr->shift) < 65535) && ((rptr->shift) > -65535) )
			{	
				if( find_port_shift_mapping(hooknum,rptr,tuple) ) 
				{
	     			//if we find port shift mapping(tuple will be changed)
	     			//then check if it is in used . (Should we go next step??)     		
					if(!ip_nat_used_tuple(tuple, conntrack))
					{
	    				ret = 1;
			    		goto clear_fulls;	
					}	
				}
		    }

		}
#endif		
		/* Only bother mapping if it's not already in range
		   and unique */
		if ((!(rptr->flags & IP_NAT_RANGE_PROTO_SPECIFIED)
		     || proto->in_range(tuple, HOOK2MANIP(hooknum),
					&rptr->min, &rptr->max))
		    && !ip_nat_used_tuple(tuple, conntrack)) {
			ret = 1;
			goto clear_fulls;
		} else {
			if (proto->unique_tuple(tuple, rptr,
						HOOK2MANIP(hooknum),
						conntrack)) {
				/* Must be unique. */
				IP_NF_ASSERT(!ip_nat_used_tuple(tuple,
								conntrack));
				ret = 1;
				goto clear_fulls;
			} else if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_DST) {
				/* Try implicit source NAT; protocol
                                   may be able to play with ports to
                                   make it unique. */
				struct ip_nat_range r
					= { IP_NAT_RANGE_MAP_IPS, 
					    tuple->src.ip, tuple->src.ip,
					    { 0 }, { 0 } };
				DEBUGP("Trying implicit mapping\n");
				if (proto->unique_tuple(tuple, &r,
							IP_NAT_MANIP_SRC,
							conntrack)) {
					/* Must be unique. */
					IP_NF_ASSERT(!ip_nat_used_tuple
						     (tuple, conntrack));
					ret = 1;
					goto clear_fulls;
				}
			}
			DEBUGP("Protocol can't get unique tuple %u.\n",
			       hooknum);
		}

		/* Eliminate that from range, and try again. */
		rptr->flags |= IP_NAT_RANGE_FULL;
		*tuple = *orig_tuple;
	}

	ret = 0;

 clear_fulls:
	/* Clear full flags. */
	IP_NF_ASSERT(mr->rangesize >= 1);
	for (i = 0; i < mr->rangesize; i++)
		mr->range[i].flags &= ~IP_NAT_RANGE_FULL;

	return ret;
}

static inline int
helper_cmp(const struct ip_nat_helper *helper,
	   const struct ip_conntrack_tuple *tuple)
{
	return ip_ct_tuple_mask_cmp(tuple, &helper->tuple, &helper->mask);
}

/* Where to manip the reply packets (will be reverse manip). */
static unsigned int opposite_hook[NF_IP_NUMHOOKS]
= { [NF_IP_PRE_ROUTING] = NF_IP_POST_ROUTING,
    [NF_IP_POST_ROUTING] = NF_IP_PRE_ROUTING,
#ifdef CONFIG_IP_NF_NAT_LOCAL
    [NF_IP_LOCAL_OUT] = NF_IP_LOCAL_IN,
    [NF_IP_LOCAL_IN] = NF_IP_LOCAL_OUT,
#endif
};

unsigned int
ip_nat_setup_info(struct ip_conntrack *conntrack,
		  const struct ip_nat_multi_range *mr,
		  unsigned int hooknum)
{
	struct ip_conntrack_tuple new_tuple, inv_tuple, reply;
	struct ip_conntrack_tuple orig_tp;
	struct ip_nat_info *info = &conntrack->nat.info;
	int in_hashes = info->initialized;

	MUST_BE_WRITE_LOCKED(&ip_nat_lock);
	IP_NF_ASSERT(hooknum == NF_IP_PRE_ROUTING
		     || hooknum == NF_IP_POST_ROUTING
		     || hooknum == NF_IP_LOCAL_OUT);
	IP_NF_ASSERT(info->num_manips < IP_NAT_MAX_MANIPS);
	IP_NF_ASSERT(!(info->initialized & (1 << HOOK2MANIP(hooknum))));

	/* What we've got will look like inverse of reply. Normally
	   this is what is in the conntrack, except for prior
	   manipulations (future optimization: if num_manips == 0,
	   orig_tp =
	   conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple) */
	invert_tuplepr(&orig_tp,
		       &conntrack->tuplehash[IP_CT_DIR_REPLY].tuple);

#if 0
	{
	unsigned int i;

	DEBUGP("Hook %u (%s), ", hooknum,
	       HOOK2MANIP(hooknum)==IP_NAT_MANIP_SRC ? "SRC" : "DST");
	DUMP_TUPLE(&orig_tp);
	DEBUGP("Range %p: ", mr);
	for (i = 0; i < mr->rangesize; i++) {
		DEBUGP("%u:%s%s%s %u.%u.%u.%u - %u.%u.%u.%u %u - %u\n",
		       i,
		       (mr->range[i].flags & IP_NAT_RANGE_MAP_IPS)
		       ? " MAP_IPS" : "",
		       (mr->range[i].flags
			& IP_NAT_RANGE_PROTO_SPECIFIED)
		       ? " PROTO_SPECIFIED" : "",
		       (mr->range[i].flags & IP_NAT_RANGE_FULL)
		       ? " FULL" : "",
		       NIPQUAD(mr->range[i].min_ip),
		       NIPQUAD(mr->range[i].max_ip),
		       mr->range[i].min.all,
		       mr->range[i].max.all);
	}
	}
#endif

	do {
		if (!get_unique_tuple(&new_tuple, &orig_tp, mr, conntrack,
				      hooknum)) {
			DEBUGP("ip_nat_setup_info: Can't get unique for %p.\n",
			       conntrack);
			return NF_DROP;
		}

#if 0
		DEBUGP("Hook %u (%s) %p\n", hooknum,
		       HOOK2MANIP(hooknum)==IP_NAT_MANIP_SRC ? "SRC" : "DST",
		       conntrack);
		DEBUGP("Original: ");
		DUMP_TUPLE(&orig_tp);
		DEBUGP("New: ");
		DUMP_TUPLE(&new_tuple);
#endif

		/* We now have two tuples (SRCIP/SRCPT/DSTIP/DSTPT):
		   the original (A/B/C/D') and the mangled one (E/F/G/H').

		   We're only allowed to work with the SRC per-proto
		   part, so we create inverses of both to start, then
		   derive the other fields we need.  */

		/* Reply connection: simply invert the new tuple
                   (G/H/E/F') */
		invert_tuplepr(&reply, &new_tuple);

		/* Alter conntrack table so it recognizes replies.
                   If fail this race (reply tuple now used), repeat. */
	} while (!ip_conntrack_alter_reply(conntrack, &reply));

	/* FIXME: We can simply used existing conntrack reply tuple
           here --RR */
	/* Create inverse of original: C/D/A/B' */
	invert_tuplepr(&inv_tuple, &orig_tp);

	/* Has source changed?. */
	if (!ip_ct_tuple_src_equal(&new_tuple, &orig_tp)) {
		/* In this direction, a source manip. */
		info->manips[info->num_manips++] =
			((struct ip_nat_info_manip)
			 { IP_CT_DIR_ORIGINAL, hooknum,
			   IP_NAT_MANIP_SRC, new_tuple.src });

		IP_NF_ASSERT(info->num_manips < IP_NAT_MAX_MANIPS);

		/* In the reverse direction, a destination manip. */
		info->manips[info->num_manips++] =
			((struct ip_nat_info_manip)
			 { IP_CT_DIR_REPLY, opposite_hook[hooknum],
			   IP_NAT_MANIP_DST, orig_tp.src });
		IP_NF_ASSERT(info->num_manips <= IP_NAT_MAX_MANIPS);
	}

	/* Has destination changed? */
	if (!ip_ct_tuple_dst_equal(&new_tuple, &orig_tp)) {
		/* In this direction, a destination manip */
		info->manips[info->num_manips++] =
			((struct ip_nat_info_manip)
			 { IP_CT_DIR_ORIGINAL, hooknum,
			   IP_NAT_MANIP_DST, reply.src });

		IP_NF_ASSERT(info->num_manips < IP_NAT_MAX_MANIPS);

		/* In the reverse direction, a source manip. */
		info->manips[info->num_manips++] =
			((struct ip_nat_info_manip)
			 { IP_CT_DIR_REPLY, opposite_hook[hooknum],
			   IP_NAT_MANIP_SRC, inv_tuple.src });
		IP_NF_ASSERT(info->num_manips <= IP_NAT_MAX_MANIPS);
	}

	/* If there's a helper, assign it; based on new tuple. */
	if (!conntrack->master)
		info->helper = LIST_FIND(&helpers, helper_cmp, struct ip_nat_helper *,
					 &reply);

	/* It's done. */
	info->initialized |= (1 << HOOK2MANIP(hooknum));

	if (in_hashes) {
		IP_NF_ASSERT(info->bysource.conntrack);
		replace_in_hashes(conntrack, info);
	} else {
		place_in_hashes(conntrack, info);
	}

	return NF_ACCEPT;
}

void replace_in_hashes(struct ip_conntrack *conntrack,
		       struct ip_nat_info *info)
{
	/* Source has changed, so replace in hashes. */
	unsigned int srchash
		= hash_by_src(&conntrack->tuplehash[IP_CT_DIR_ORIGINAL]
			      .tuple.src,
			      conntrack->tuplehash[IP_CT_DIR_ORIGINAL]
			      .tuple.dst.protonum);
	/* We place packet as seen OUTGOUNG in byips_proto hash
           (ie. reverse dst and src of reply packet. */
	unsigned int ipsprotohash
		= hash_by_ipsproto(conntrack->tuplehash[IP_CT_DIR_REPLY]
				   .tuple.dst.ip,
				   conntrack->tuplehash[IP_CT_DIR_REPLY]
				   .tuple.src.ip,
				   conntrack->tuplehash[IP_CT_DIR_REPLY]
				   .tuple.dst.protonum);

	IP_NF_ASSERT(info->bysource.conntrack == conntrack);
	MUST_BE_WRITE_LOCKED(&ip_nat_lock);

	list_del(&info->bysource.list);
	list_del(&info->byipsproto.list);

	list_prepend(&bysource[srchash], &info->bysource);
	list_prepend(&byipsproto[ipsprotohash], &info->byipsproto);
}

void place_in_hashes(struct ip_conntrack *conntrack,
		     struct ip_nat_info *info)
{
	unsigned int srchash
		= hash_by_src(&conntrack->tuplehash[IP_CT_DIR_ORIGINAL]
			      .tuple.src,
			      conntrack->tuplehash[IP_CT_DIR_ORIGINAL]
			      .tuple.dst.protonum);
	/* We place packet as seen OUTGOUNG in byips_proto hash
           (ie. reverse dst and src of reply packet. */
	unsigned int ipsprotohash
		= hash_by_ipsproto(conntrack->tuplehash[IP_CT_DIR_REPLY]
				   .tuple.dst.ip,
				   conntrack->tuplehash[IP_CT_DIR_REPLY]
				   .tuple.src.ip,
				   conntrack->tuplehash[IP_CT_DIR_REPLY]
				   .tuple.dst.protonum);

	IP_NF_ASSERT(!info->bysource.conntrack);

	MUST_BE_WRITE_LOCKED(&ip_nat_lock);
	info->byipsproto.conntrack = conntrack;
	info->bysource.conntrack = conntrack;

	list_prepend(&bysource[srchash], &info->bysource);
	list_prepend(&byipsproto[ipsprotohash], &info->byipsproto);
}

static void
manip_pkt(u_int16_t proto, struct iphdr *iph, size_t len,
	  const struct ip_conntrack_manip *manip,
	  enum ip_nat_manip_type maniptype,
	  __u32 *nfcache)
{
	*nfcache |= NFC_ALTERED;
	find_nat_proto(proto)->manip_pkt(iph, len, manip, maniptype);

	if (maniptype == IP_NAT_MANIP_SRC) {
		iph->check = ip_nat_cheat_check(~iph->saddr, manip->ip,
						iph->check);
		iph->saddr = manip->ip;
	} else {
		iph->check = ip_nat_cheat_check(~iph->daddr, manip->ip,
						iph->check);
		iph->daddr = manip->ip;
	}
#if 0
	if (ip_fast_csum((u8 *)iph, iph->ihl) != 0)
		DEBUGP("IP: checksum on packet bad.\n");

	if (proto == IPPROTO_TCP) {
		void *th = (u_int32_t *)iph + iph->ihl;
		if (tcp_v4_check(th, len - 4*iph->ihl, iph->saddr, iph->daddr,
				 csum_partial((char *)th, len-4*iph->ihl, 0)))
			DEBUGP("TCP: checksum on packet bad\n");
	}
#endif
}

static inline int exp_for_packet(struct ip_conntrack_expect *exp,
			         struct sk_buff **pskb)
{
	struct ip_conntrack_protocol *proto;
	int ret = 1;

	MUST_BE_READ_LOCKED(&ip_conntrack_lock);
	proto = __ip_ct_find_proto((*pskb)->nh.iph->protocol);
	if (proto->exp_matches_pkt)
		ret = proto->exp_matches_pkt(exp, pskb);

	return ret;
}

/* Do packet manipulations according to binding. */
unsigned int
do_bindings(struct ip_conntrack *ct,
	    enum ip_conntrack_info ctinfo,
	    struct ip_nat_info *info,
	    unsigned int hooknum,
	    struct sk_buff **pskb)
{
	unsigned int i;
	struct ip_nat_helper *helper;
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	int is_tcp = (*pskb)->nh.iph->protocol == IPPROTO_TCP;

	/* Need nat lock to protect against modification, but neither
	   conntrack (referenced) and helper (deleted with
	   synchronize_bh()) can vanish. */
	READ_LOCK(&ip_nat_lock);
	for (i = 0; i < info->num_manips; i++) {
		/* raw socket (tcpdump) may have clone of incoming
                   skb: don't disturb it --RR */
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb) {
				READ_UNLOCK(&ip_nat_lock);
				return NF_DROP;
			}
			kfree_skb(*pskb);
			*pskb = nskb;
		}

		if (info->manips[i].direction == dir
		    && info->manips[i].hooknum == hooknum) {
			DEBUGP("Mangling %p: %s to %u.%u.%u.%u %u\n",
			       *pskb,
			       info->manips[i].maniptype == IP_NAT_MANIP_SRC
			       ? "SRC" : "DST",
			       NIPQUAD(info->manips[i].manip.ip),
			       htons(info->manips[i].manip.u.all));
			manip_pkt((*pskb)->nh.iph->protocol,
				  (*pskb)->nh.iph,
				  (*pskb)->len,
				  &info->manips[i].manip,
				  info->manips[i].maniptype,
				  &(*pskb)->nfcache);
		}
	}
	helper = info->helper;
	READ_UNLOCK(&ip_nat_lock);

	if (helper) {
		struct ip_conntrack_expect *exp = NULL;
		struct list_head *cur_item;
		int ret = NF_ACCEPT;
		int helper_called = 0;

		DEBUGP("do_bindings: helper existing for (%p)\n", ct);

		/* Always defragged for helpers */
		IP_NF_ASSERT(!((*pskb)->nh.iph->frag_off
			       & htons(IP_MF|IP_OFFSET)));

		/* Have to grab read lock before sibling_list traversal */
		READ_LOCK(&ip_conntrack_lock);
		list_for_each_prev(cur_item, &ct->sibling_list) { 
			exp = list_entry(cur_item, struct ip_conntrack_expect, 
					 expected_list);
					 
			/* if this expectation is already established, skip */
			if (exp->sibling)
				continue;

			if (exp_for_packet(exp, pskb)) {
				/* FIXME: May be true multiple times in the
				 * case of UDP!! */
				DEBUGP("calling nat helper (exp=%p) for	packet\n", exp);
				ret = helper->help(ct, exp, info, ctinfo, 
						   hooknum, pskb);
				if (ret != NF_ACCEPT) {
					READ_UNLOCK(&ip_conntrack_lock);
					return ret;
				}
				helper_called = 1;
			}
		}
		/* Helper might want to manip the packet even when there is no
		 * matching expectation for this packet */
		if (!helper_called && helper->flags & IP_NAT_HELPER_F_ALWAYS) {
			DEBUGP("calling nat helper for packet without expectation\n");
			ret = helper->help(ct, NULL, info, ctinfo, 
					   hooknum, pskb);
			if (ret != NF_ACCEPT) {
				READ_UNLOCK(&ip_conntrack_lock);
				return ret;
			}
		}
		READ_UNLOCK(&ip_conntrack_lock);
		
		/* Adjust sequence number only once per packet 
		 * (helper is called at all hooks) */
		if (is_tcp && (hooknum == NF_IP_POST_ROUTING
			       || hooknum == NF_IP_LOCAL_IN)) {
			DEBUGP("ip_nat_core: adjusting sequence number\n");
			/* future: put this in a l4-proto specific function,
			 * and call this function here. */
			ip_nat_seq_adjust(*pskb, ct, ctinfo);
		}

		return ret;

	} else 
		return NF_ACCEPT;

	/* not reached */
}

unsigned int
icmp_reply_translation(struct sk_buff *skb,
		       struct ip_conntrack *conntrack,
		       unsigned int hooknum,
		       int dir)
{
	struct iphdr *iph = skb->nh.iph;
	struct icmphdr *hdr = (struct icmphdr *)((u_int32_t *)iph + iph->ihl);
	struct iphdr *inner = (struct iphdr *)(hdr + 1);
	size_t datalen = skb->len - ((void *)inner - (void *)iph);
	unsigned int i;
	struct ip_nat_info *info = &conntrack->nat.info;

	IP_NF_ASSERT(skb->len >= iph->ihl*4 + sizeof(struct icmphdr));
	/* Must be RELATED */
	IP_NF_ASSERT(skb->nfct - (struct ip_conntrack *)skb->nfct->master
		     == IP_CT_RELATED
		     || skb->nfct - (struct ip_conntrack *)skb->nfct->master
		     == IP_CT_RELATED+IP_CT_IS_REPLY);

	/* Redirects on non-null nats must be dropped, else they'll
           start talking to each other without our translation, and be
           confused... --RR */
	if (hdr->type == ICMP_REDIRECT) {
		/* Don't care about races here. */
		if (info->initialized
		    != ((1 << IP_NAT_MANIP_SRC) | (1 << IP_NAT_MANIP_DST))
		    || info->num_manips != 0)
			return NF_DROP;
	}

	DEBUGP("icmp_reply_translation: translating error %p hook %u dir %s\n",
	       skb, hooknum, dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY");
	/* Note: May not be from a NAT'd host, but probably safest to
	   do translation always as if it came from the host itself
	   (even though a "host unreachable" coming from the host
	   itself is a bit weird).

	   More explanation: some people use NAT for anonymizing.
	   Also, CERT recommends dropping all packets from private IP
	   addresses (although ICMP errors from internal links with
	   such addresses are not too uncommon, as Alan Cox points
	   out) */

	READ_LOCK(&ip_nat_lock);
	for (i = 0; i < info->num_manips; i++) {
		DEBUGP("icmp_reply: manip %u dir %s hook %u\n",
		       i, info->manips[i].direction == IP_CT_DIR_ORIGINAL ?
		       "ORIG" : "REPLY", info->manips[i].hooknum);

		if (info->manips[i].direction != dir)
			continue;

		/* Mapping the inner packet is just like a normal
		   packet, except it was never src/dst reversed, so
		   where we would normally apply a dst manip, we apply
		   a src, and vice versa. */
		if (info->manips[i].hooknum == hooknum) {
			DEBUGP("icmp_reply: inner %s -> %u.%u.%u.%u %u\n",
			       info->manips[i].maniptype == IP_NAT_MANIP_SRC
			       ? "DST" : "SRC",
			       NIPQUAD(info->manips[i].manip.ip),
			       ntohs(info->manips[i].manip.u.udp.port));
			manip_pkt(inner->protocol, inner,
				  skb->len - ((void *)inner - (void *)iph),
				  &info->manips[i].manip,
				  !info->manips[i].maniptype,
				  &skb->nfcache);
			/* Outer packet needs to have IP header NATed like
	                   it's a reply. */

			/* Use mapping to map outer packet: 0 give no
                           per-proto mapping */
			DEBUGP("icmp_reply: outer %s -> %u.%u.%u.%u\n",
			       info->manips[i].maniptype == IP_NAT_MANIP_SRC
			       ? "SRC" : "DST",
			       NIPQUAD(info->manips[i].manip.ip));
			manip_pkt(0, iph, skb->len,
				  &info->manips[i].manip,
				  info->manips[i].maniptype,
				  &skb->nfcache);
		}
	}
	READ_UNLOCK(&ip_nat_lock);

	/* Since we mangled inside ICMP packet, recalculate its
	   checksum from scratch.  (Hence the handling of incorrect
	   checksums in conntrack, so we don't accidentally fix one.)  */
	hdr->checksum = 0;
	hdr->checksum = ip_compute_csum((unsigned char *)hdr,
					sizeof(*hdr) + datalen);

	return NF_ACCEPT;
}

#if defined(CONFIG_PROC_FS) && defined(CONFIG_RTL865X_HW_TABLES)
static struct proc_dir_entry *proc_hw_nat=NULL;
static char gHwNatSetting[16];

static int hw_nat_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%s\n", gHwNatSetting);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif
static int hw_nat_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	if (count < 2) 
		return -EFAULT;
      
	if (buffer && !copy_from_user(&gHwNatSetting, buffer, 8)) {
		WRITE_LOCK(&ip_nat_lock);
		if (gHwNatSetting[0] == '0') { /* hardware NAT disabled, operation mode = gateway */
			rtl8651_setAsicOperationLayer2(2);
			gHwNatEnabled = 0;
		}
		else if (gHwNatSetting[0] == '1') { /* hardware NAT enabled, operation mode = gateway */
			if (gHwNatEnabled == 2) {
				re_init_vlanTable(0);	/* Re-Configure hardware tables */
			}
			rtl8651_setAsicOperationLayer2(4);
			gHwNatEnabled = 1;
		}
		else if (gHwNatSetting[0] == '2') { /* hardware NAT disabled, operation mode = bridge/WISP */

			#ifdef CONFIG_RTL8196B
			re_init_vlanTable(1);
			#else
			if (gHwNatEnabled != 2) {
				re_init_vlanTable(1);	/* Re-Configure hardware tables */
			}
			#endif
			rtl8651_setAsicOperationLayer2(2);
			gHwNatEnabled = 2;
		}
		else if (gHwNatSetting[0] == '8') {
			if (gHwNatSetting[1] == '6') /* L2TP */
				gHwNatEnabled = 0;
		}		
		WRITE_UNLOCK(&ip_nat_lock);
		return count;
	}
	return -EFAULT;
}
#endif

#if defined(CONFIG_PROC_FS) && !defined(CONFIG_RTL865X_HW_TABLES)
static struct proc_dir_entry *proc_sw_nat=NULL;
#if defined(CONFIG_RTL865X_SUPPORT_IPV6_MLD) || defined(_ARSWITCH_) || defined(_SOFT_ARSWITCH_)
char gSwNatSetting[16];
#else
static char gSwNatSetting[16];
#endif

static int sw_nat_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%s\n", gSwNatSetting);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif 
static int sw_nat_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	if (count < 2) 
		return -EFAULT;      
#ifdef CONFIG_RTL865X_ETH      
	if (buffer && !copy_from_user(&gSwNatSetting, buffer, 8)) {
		WRITE_LOCK(&ip_nat_lock);
		if (gSwNatSetting[0] == '0'){  /* operation mode = gateway */
			SoftNAT_OP_Mode(2);
		}
		else if (gSwNatSetting[0] == '1'){  /* operation mode = bridge/WISP */
			SoftNAT_OP_Mode(1);
		}
		else if(gSwNatSetting[0] == '2'){ /* operation mode = bridge/WISP */
			SoftNAT_OP_Mode(1);
		}
		WRITE_UNLOCK(&ip_nat_lock);
		return count;
	}
#endif	
	return -EFAULT;
}
#endif

#ifdef _PPPOE_PASSTHROUGH_
static struct proc_dir_entry *proc_pppoe_pass=NULL;
char pppoe_pass[16];
static int pppoe_pass_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data) {
	int len;
	len = sprintf(page, "%s\n", pppoe_pass);
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int pppoe_pass_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) {
	if (count < 2) return -EFAULT;
	if (buffer && !copy_from_user(&pppoe_pass, buffer, 8)) return count;
	return -EFAULT;
}
#endif

#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_QOS)
static struct proc_dir_entry *proc_qos=NULL;
#define MAX_QOS_RULE_NUM		10

static char gQosSetting[1024];

#ifdef CONFIG_QOS_AUTO_SHARE_BW
#define MAX_QOS_ACTIVE_FLOW 16
static struct proc_dir_entry *proc_qos_flow=NULL;
static char gQosTotalFlowSrcIp[1024];
#endif
//int gQosEnabled=0;
#ifdef CONFIG_FAST_PATH_MODULE
EXPORT_SYMBOL(gQosEnabled);
#endif
typedef struct qos_entry {
	unsigned int protocol;
	struct in_addr local_ip_start;
	struct in_addr  local_ip_end;
	struct in_addr  remote_ip_start;
	struct in_addr  remote_ip_end;
	unsigned short lo_port_start;
	unsigned short lo_port_end;
	unsigned short re_port_start;
	unsigned short re_port_end;
} QOS_T, *QOS_Tp;

#if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_QOS_UI_BY_BANDWIDTH) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL8196C_KLD)
static QOS_T gQosTbl[MAX_QOS_RULE_NUM];

static unsigned long atoi_dec(char *s)
{
	unsigned long k = 0;

	k = 0;
	while (*s != '\0' && *s >= '0' && *s <= '9') {
		k = 10 * k + (*s - '0');
		s++;
	}
	return k;
}

/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
typedef unsigned long in_addr_t; 
static int inet_aton(const char *cp, struct in_addr *addr)
{
	register in_addr_t val;
	register int base, n;
	register char c;
	unsigned int parts[4];
	register unsigned int *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		val <<=24;
		break;

	case 2:				/* a.b -- 8.24 bits */
		val <<=16;
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		val <<=8;
		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}
#endif
    
static int qos_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
      int len;

	#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	extern int active_client_num(void);
      len = sprintf(page, "%d %d %d\n", gQosEnabled, auto_distribute_bandwidth, 
	  	(gQosEnabled && auto_distribute_bandwidth) ? active_client_num() : 0);
	#else
      len = sprintf(page, "%s\n", gQosSetting);
	#endif

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}

#ifdef CONFIG_QOS_AUTO_SHARE_BW
extern int active_flow_ip(unsigned int *);
static int qos_flow_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
      int len,flow_num,i;
	  unsigned int srcIp[MAX_QOS_ACTIVE_FLOW];
	  char *buf= gQosTotalFlowSrcIp;
	  
	  flow_num = active_flow_ip(&srcIp[0]);	
	  for(i=0;i<flow_num;i++)
	  {
		if(i == (flow_num-1) )
		len = sprintf(buf, "%d",srcIp[i]);	
		else
		len = sprintf(buf, "%d ",srcIp[i]);

		buf += len;
	  }  

	  //len = sprintf(page, "%s\n", gQosTotalFlowSrcIp);      
	  len = sprintf(page, "%s", gQosTotalFlowSrcIp);   
      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}
#endif

#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif 
static int qos_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      #if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_QOS_UI_BY_BANDWIDTH) && !defined(CONFIG_RTL8196C_KLD)
      int i;
      char *str, *token;
      char tmpbuf[1024];
      #endif
	
      if (count < 2) 
	    return -EFAULT;
      
      if (buffer && !copy_from_user(&gQosSetting, buffer, count)) {
          gQosSetting[count-1] = 0; /* remove 0x0a */
          if (gQosSetting[0] == '0')
                gQosEnabled = 0;
          else
                gQosEnabled = 1;

		#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		if (gQosSetting[2] == '0')
			auto_distribute_bandwidth = 0;
		else
			auto_distribute_bandwidth = 1;
		#endif
		
          #if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_QOS_UI_BY_BANDWIDTH) && !defined(CONFIG_RTL8196C_KLD)
          strcpy(tmpbuf, gQosSetting);

          str = tmpbuf+2;
          memset(gQosTbl, 0, MAX_QOS_RULE_NUM * sizeof(QOS_T));
          token = strtok(str,",");
          for (i=0 ; (i<MAX_QOS_RULE_NUM) && (token != NULL); i++, token = strtok(NULL,","))
          {
             gQosTbl[i].protocol = atoi_dec(token);
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
		if ( !inet_aton(token, &(gQosTbl[i].local_ip_start)))
                    break;             
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
		if ( !inet_aton(token, &(gQosTbl[i].local_ip_end)))
                    break;
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
		if ( !inet_aton(token, &(gQosTbl[i].remote_ip_start)))
                    break;
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
		if ( !inet_aton(token, &(gQosTbl[i].remote_ip_end)))
                    break;        

	      	if((token = strtok(NULL,","))==NULL)
	        	break;
             gQosTbl[i].lo_port_start = atoi_dec(token);
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
             gQosTbl[i].lo_port_end = atoi_dec(token);
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
             gQosTbl[i].re_port_start = atoi_dec(token);
	      	if((token = strtok(NULL,","))==NULL)
	        	break;
             gQosTbl[i].re_port_end = atoi_dec(token);
          }

          /* let gQosEnabled = 0 if no any QoS rule be set */
          if (gQosTbl[0].protocol == 0) 
              gQosEnabled = 0;
          #endif

	    return count;
      }
      return -EFAULT;
}

#if !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_QOS_UI_BY_BANDWIDTH) && !defined(CONFIG_RTL8196C_KLD)
/* proto: 1 TCP, 0 UDP */
static int checkQosSetting( u_int16_t proto,u_int32_t sip, u_int32_t dip, u_int16_t sp, u_int16_t dp)
{
    int i;
    unsigned long v1, v2, v3, v4;   
    
    for (i=0; i<MAX_QOS_RULE_NUM; i++)
    {
        if (gQosTbl[i].protocol == 0) 
            break;
    
        v1 = *((unsigned long *)&(gQosTbl[i].local_ip_start));
        v2 = *((unsigned long *)&(gQosTbl[i].local_ip_end));
        v3 = *((unsigned long *)&(gQosTbl[i].remote_ip_start));
        v4 = *((unsigned long *)&(gQosTbl[i].remote_ip_end));
    
        if (((proto == 1 && gQosTbl[i].protocol != IPPROTO_UDP) || (proto == 0 && gQosTbl[i].protocol != IPPROTO_TCP)) &&
            (sip >= v1 && sip <= v2) && (dip >= v3 && dip <= v4) ) {
            if ((sp >= gQosTbl[i].lo_port_start && sp <= gQosTbl[i].lo_port_end) && (dp >= gQosTbl[i].re_port_start && dp <= gQosTbl[i].re_port_end)) {
                return 0; /* do not add to H/W NAT table */
            }
        }        
    }
    return 1; /* can be added to H/W NAT table */
}
#endif
#endif

int __init ip_nat_init(void)
{
	size_t i;

	/* Leave them the same for the moment. */
	ip_nat_htable_size = ip_conntrack_htable_size;

	/* One vmalloc for both hash tables */
	bysource = vmalloc(sizeof(struct list_head) * ip_nat_htable_size*2);
	if (!bysource) {
		return -ENOMEM;
	}
	byipsproto = bysource + ip_nat_htable_size;

	/* Sew in builtin protocols. */
	WRITE_LOCK(&ip_nat_lock);
	list_append(&protos, &ip_nat_protocol_tcp);
	list_append(&protos, &ip_nat_protocol_udp);
	list_append(&protos, &ip_nat_protocol_icmp);
	WRITE_UNLOCK(&ip_nat_lock);

	for (i = 0; i < ip_nat_htable_size; i++) {
		INIT_LIST_HEAD(&bysource[i]);
		INIT_LIST_HEAD(&byipsproto[i]);
	}

	/* FIXME: Man, this is a hack.  <SIGH> */
	IP_NF_ASSERT(ip_conntrack_destroyed == NULL);
	ip_conntrack_destroyed = &ip_nat_cleanup_conntrack;

#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_QOS)
	proc_qos = create_proc_entry("qos", 0, NULL);
	if (proc_qos) {
	    proc_qos->read_proc = qos_read_proc;
	    proc_qos->write_proc = qos_write_proc;
      }  
#endif
#ifdef CONFIG_QOS_AUTO_SHARE_BW
	proc_qos_flow = create_proc_entry("qos_flow", 0, NULL);
	if (proc_qos_flow) {
	    proc_qos_flow->read_proc = qos_flow_read_proc;	   
      }  
#endif

#if defined(CONFIG_PROC_FS) && defined(CONFIG_RTL865X_HW_TABLES)
	proc_hw_nat = create_proc_entry("hw_nat", 0, NULL);
	if (proc_hw_nat) {
	    proc_hw_nat->read_proc = hw_nat_read_proc;
	    proc_hw_nat->write_proc = hw_nat_write_proc;
      }  
#endif

#if defined(CONFIG_PROC_FS) && !defined(CONFIG_RTL865X_HW_TABLES)
	proc_sw_nat = create_proc_entry("sw_nat", 0, NULL);
	if (proc_sw_nat) {
		proc_sw_nat->read_proc = sw_nat_read_proc;
		proc_sw_nat->write_proc = sw_nat_write_proc;
	}  
#endif

#ifdef _PPPOE_PASSTHROUGH_
	proc_pppoe_pass = create_proc_entry("pppoe_pass", 0, NULL);
	if (proc_pppoe_pass) {
		proc_pppoe_pass->read_proc = pppoe_pass_read_proc;
		proc_pppoe_pass->write_proc = pppoe_pass_write_proc;
	}
#endif

	return 0;
}

/* Clear NAT section of all conntracks, in case we're loaded again. */
static int clean_nat(const struct ip_conntrack *i, void *data)
{
	memset((void *)&i->nat, 0, sizeof(i->nat));
	return 0;
}

/* Not __exit: called from ip_nat_standalone.c:init_or_cleanup() --RR */
void ip_nat_cleanup(void)
{
	ip_ct_selective_cleanup(&clean_nat, NULL);
	ip_conntrack_destroyed = NULL;
	vfree(bysource);
}
