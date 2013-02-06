/* SIP extension for UDP NAT alteration.
 *
 * (C) 2005 by Christian Hentschel <chentschel@arnet.com.ar>
 * based on RR's ip_nat_ftp.c and other modules.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 4 Apr 2008 : 
 *      atens <atens.huang@gmail.com>
 * 	 - Back ported to Linux 2.4 based on Rusty's FTP ALG
 * 	 - Add sip nat helper to help sip protocol setting up the nat 
 * 	   tunnel for incoming rtp traffic
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/udp.h>

#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_sip.h>
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
#include <asm/mips16_lib.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hentschel <chentschel@arnet.com.ar>");
MODULE_DESCRIPTION("SIP NAT helper");

#define MAX_PORTS 8

#ifdef DEBUG
#ifdef DEBUGGER
#include <linux/spinlock.h>
#endif
#endif

#ifdef CONFIG_IP_NF_ALGCTRL
extern int ip_ct_alg_sip;
#else
#if defined(CONFIG_PROC_FS)
extern int sip_alg_flag; //michael
#endif
#endif

static unsigned short ports[MAX_PORTS] = { [0 ... MAX_PORTS-1] = 0};

MODULE_PARM(ports, "1-8i");
MODULE_PARM_DESC(ports, "port number of SIP nat helper");

static char sip_names[MAX_PORTS][sizeof("sip-65535")] /*__read_mostly */;

static struct ip_nat_helper sip[MAX_PORTS];

#ifdef DEBUG /* This is used for disabling IRQ when debugging kernel. */
#ifdef DEBUGGER
static spinlock_t lock =  SPIN_LOCK_UNLOCKED;
#endif
#endif

unsigned int flags;

struct addr_map {
	struct {
		char		src[sizeof("nnn.nnn.nnn.nnn:nnnnn")];
		char		dst[sizeof("nnn.nnn.nnn.nnn:nnnnn")];
		unsigned int	srclen, srciplen;
		unsigned int	dstlen, dstiplen;
	} addr[IP_CT_DIR_MAX];
};
static unsigned int ip_nat_sdp(struct sk_buff **,
			       enum ip_conntrack_info ,
			       struct ip_conntrack_expect *,
			       const char *, int, enum sip_header_pos);

static void addr_map_init(struct ip_conntrack *ct, struct addr_map *map)
{
	struct ip_conntrack_tuple *t;
	enum ip_conntrack_dir dir;
	unsigned int n;

	for (dir = 0; dir < IP_CT_DIR_MAX; dir++) {
		t = &ct->tuplehash[dir].tuple;

		n = sprintf(map->addr[dir].src, "%u.%u.%u.%u",
			    NIPQUAD(t->src.ip));
		map->addr[dir].srciplen = n;
		n += sprintf(map->addr[dir].src + n, ":%u",
			     ntohs(t->src.u.udp.port));
		map->addr[dir].srclen = n;

		n = sprintf(map->addr[dir].dst, "%u.%u.%u.%u",
			    NIPQUAD(t->dst.ip));
		map->addr[dir].dstiplen = n;
		n += sprintf(map->addr[dir].dst + n, ":%u",
			     ntohs(t->dst.u.udp.port));
		map->addr[dir].dstlen = n;
	}
}

static int map_sip_addr(struct sk_buff **pskb, enum ip_conntrack_info ctinfo,
			struct ip_conntrack *ct, const char **dptr, size_t dlen,
			enum sip_header_pos pos, struct addr_map *map)
{
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	unsigned int matchlen, matchoff, addrlen;
	char *addr;
	struct iphdr *iph;

	if (ct_sip_get_info(ct, *dptr, dlen, &matchoff, &matchlen, pos) <= 0)
		return 1;

	/* Decide which IP we should use. 
	 *
	 * If the original IP is the same as the tuple, that means we should map 
	 * that IP to the related IP field in the opposite way.
	 *
	 * For instance, if the source IP of this packet is the same with 
	 * tuple[dir].src.ip, that means, we should map this packet's source
	 * IP to tuple[!dir].dst.ip (on the opposite way, the related 
	 * IP position in the IP packet) 
	 *
	 * 								--atens 
	 */
	if ((matchlen == map->addr[dir].srciplen ||
	     matchlen == map->addr[dir].srclen) &&
	    memcmp(*dptr + matchoff, map->addr[dir].src, matchlen) == 0) {
		addr    = map->addr[!dir].dst;
		addrlen = map->addr[!dir].dstlen;
	} else if ((matchlen == map->addr[dir].dstiplen ||
		    matchlen == map->addr[dir].dstlen) &&
		   memcmp(*dptr + matchoff, map->addr[dir].dst, matchlen) == 0) {
		addr    = map->addr[!dir].src;
		addrlen = map->addr[!dir].srclen;
	} else
		return 1;

	/* Then we write the result into packet */
	if (!ip_nat_mangle_udp_packet(pskb, ct, ctinfo,
				      matchoff, matchlen, addr, addrlen))
		return 0;
	#if 0
	*dptr = (*pskb)->data + ip_hdrlen(*pskb) + sizeof(struct udphdr);
	#endif
	iph = (void *)(*pskb)->data;

	/* 
	 * Recompute the new data pointer.
	 * 				--atens 
	 */
	*dptr = (*pskb)->data + iph->ihl*4 + sizeof(struct udphdr);
	return 1;

}

#if 0
static unsigned int ip_nat_sip(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       struct ip_conntrack *ct,
			       const char **dptr)
#endif			      

#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif 
static unsigned int ip_nat_sip_help(struct ip_conntrack *ct,
                         struct ip_conntrack_expect *exp,
                         struct ip_nat_info *info,
                         enum ip_conntrack_info ctinfo,
                         unsigned int hooknum,
                         struct sk_buff **pskb)
{
	struct addr_map map;
	int dataoff, datalen;

	enum sip_header_pos pos; 

	uint16_t port = 0;
	uint16_t video_port = 0;

	int ret; 

	int matchoff, matchlen;

	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *udph = (void *)iph + iph->ihl*4;

	const char *dptr = NULL;

#if 0
        struct ip_nat_info *info = &ct->nat.info; 
#endif
	int dir=-1; /* the pskb's direction */

    /* michael added start, 2008/05/28 */
#ifdef CONFIG_IP_NF_ALGCTRL
	if (!ip_ct_alg_sip) {
		return NF_ACCEPT;
	}
#else
#if defined(CONFIG_PROC_FS)
	if (!sip_alg_flag) {
		return NF_ACCEPT;
	}
#endif
#endif
    /* michael added end, 2008/05/28 */
	
#ifdef DEBUG
#ifdef DEBUGGER
	spin_lock_irqsave(&lock,flags);
#endif
#endif

#ifdef DEBUG
	printk("%s:%d: ip_nat_sip_help entered\n", __FILE__, __LINE__);
#endif

        dir = CTINFO2DIR(ctinfo);


       /* For ftp client/server model, this is good. Howerver, 
	* we are implementing the peer model of sip.
	* 						--atens
	*
	* FIXME:How about content filter in filter table?
	*       If the code listed below works, the fitler table will
	*       get confused. I think.			--atens
	*/

       if (!((hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
             || (hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY))) {
	       ret = NF_ACCEPT;
	       goto out;
       }

       

#if 0
	for (i=0; i<info->num_manips; i++) 
		if (info->manips[i].direction == dir && info->manips[i].hooknum == hooknum) {
			/* 
			 * Make sure that it's appropriate time to handle this skb.
			 * 							--atens
			 */
			helper_doable = 1;
			break;
		}

	/* 
	 * Don't do the manipulation since it's not a good time to do so.
	 *						          --atens
	 */

#ifdef DEBUG
	printk("helper_doable:%d\n", helper_doable);
#endif
	if (!helper_doable) {
#ifdef DEBUG
		printk("%s:%d:Accept packet, but not handled.\n", __FILE__, __LINE__);
#endif
		ret = NF_ACCEPT;
		goto out;
	}
#endif
	

#if 0
	 if (info->initialized  != ((1 << IP_NAT_MANIP_SRC) | (1 << IP_NAT_MANIP_DST)) || info->num_manips != 0)     return NF_ACCEPT;

#endif


	/* dataoff = ip_hdrlen(*pskb) + sizeof(struct udphdr); */
	dataoff = iph->ihl*4  + sizeof(struct udphdr);
	datalen = (*pskb)->len - dataoff;
	if (datalen < sizeof("SIP/2.0") - 1)  {
#ifdef DEBUG
		printk("%s:%d:Accept packet, but not handled.\n", __FILE__, __LINE__);
#endif
		ret = NF_ACCEPT; 
		goto out;
	}

	addr_map_init(ct, &map);

	dptr = (char *)udph + sizeof(struct udphdr);
	
	/* Basic rules: requests and responses. */
	if (strncmp(dptr, "SIP/2.0", sizeof("SIP/2.0") - 1) != 0) {
		/* 10.2: Constructing the REGISTER Request:
		 *
		 * The "userinfo" and "@" components of the SIP URI MUST NOT
		 * be present.
		 */
		if (datalen >= sizeof("REGISTER") - 1 &&
		    strncmp(dptr, "REGISTER", sizeof("REGISTER") - 1) == 0)
			pos = POS_REG_REQ_URI;
		else
			pos = POS_REQ_URI;

		if (!map_sip_addr(pskb, ctinfo, ct, (void *)&dptr, datalen, pos, &map)) {
#ifdef DEBUG
			printk("%s:%d:Drop packet\n", __FILE__, __LINE__);
#endif 
			ret = NF_ACCEPT;
			goto out;
		}
	}

	/* FIXME: These might crash since the datalen is not updated. --atens */
	if (!map_sip_addr(pskb, ctinfo, ct, (void *)&dptr, datalen, POS_FROM, &map) ||
	    !map_sip_addr(pskb, ctinfo, ct, (void *)&dptr, datalen, POS_TO, &map) ||
	    !map_sip_addr(pskb, ctinfo, ct, (void *)&dptr, datalen, POS_VIA, &map) ||
	    !map_sip_addr(pskb, ctinfo, ct, (void *)&dptr, datalen, POS_CONTACT, &map)) {

#ifdef DEBUG
		printk("%s:%d:Error in replacing ip addr.\n", __FILE__, __LINE__);
#endif
		}

	
        /* RTP info only in some SDP pkts */
	       if ( !exp || (memcmp(dptr, "INVITE", sizeof("INVITE") - 1) != 0 &&
		    memcmp(dptr, "UPDATE", sizeof("UPDATE") - 1) != 0 &&
		    memcmp(dptr, "SIP/2.0 180", sizeof("SIP/2.0 180") - 1) != 0 &&
		    memcmp(dptr, "SIP/2.0 183", sizeof("SIP/2.0 183") - 1) != 0 &&
		    memcmp(dptr, "SIP/2.0 200", sizeof("SIP/2.0 200") - 1) != 0)) {

		#ifdef DEBUG
		printk("%s:%d:Cannot find RTP info or exp null\n", __FILE__, __LINE__);
		#endif

		ret = NF_ACCEPT;
		goto out;

		}

	if (dir == exp->help.exp_sip_info.dir) {
		ret = NF_ACCEPT;
		goto out;
	}

	if (ct_sip_get_info(ct, dptr, datalen, &matchoff, &matchlen, 
				POS_MEDIA) > 0 ) {

		port = simple_strtoul(dptr + matchoff, NULL, 10);
	} 

	if (ct_sip_get_info(ct, dptr, datalen, &matchoff, &matchlen, 
				POS_MEDIA_VIDEO) > 0 ) {
		video_port = simple_strtoul(dptr + matchoff, NULL, 10);
	}

	/* Check the exp to see if the saved ip and saved 
	 * port is the same as this packet.
	 * 					--atens
	 * FIXME: Ignore the saved_ip? It's because that the sdp's connection
	 *        ip will be overwritten to the outboud ip of the nat box. Thus
	 *        the second check will fail (probably the audio expectation).
	 *
	 */
		if (exp->help.exp_sip_info.saved_port == htons(port) && port !=0) {
		   pos = POS_MEDIA; 
		} else if (exp->help.exp_sip_info.saved_port == htons(video_port) && video_port != 0) {
		   pos = POS_MEDIA_VIDEO;
		} else {
			/* There is no meida info in this exp? --atens */
			ret = NF_ACCEPT;
			goto out;
		}

#ifdef DEBUG
	printk("%s:%d:Now enter ip nat sdp helper for masquerading advertisement packet. Hook number:%d \n", __FILE__, __LINE__, hooknum);
#endif

	/* If we have any SDP payload, then we parse it and create
	 * the tunnel for it. 				   --atens
	 */
	if (!ip_nat_sdp(pskb, ctinfo, exp, dptr, hooknum, pos)) {

#ifdef DEBUG
		printk("%s:%d:Drop packet\n", __FILE__, __LINE__);
#endif
		ret = NF_DROP;
		goto out;
	}
#ifdef DEBUG
	printk("%s:%d:sdp mangling succeed.\n", __FILE__, __LINE__);
#endif

	ret = NF_ACCEPT;

out:
#ifdef DEBUG
#ifdef DEBUGGER
	spin_unlock_irqrestore(&lock, flags);
#endif
#endif
	return ret;
}

static unsigned int mangle_sip_packet(struct sk_buff **pskb,
				      enum ip_conntrack_info ctinfo,
				      struct ip_conntrack *ct,
				      const char **dptr, size_t dlen,
				      char *buffer, int bufflen,
				      enum sip_header_pos pos)
{
	int i;
	unsigned int matchlen, matchoff;
	struct iphdr *iph = (*pskb)->nh.iph;

	if (ct_sip_get_info(ct, *dptr, dlen, &matchoff, &matchlen, pos) <= 0)
		return 0;

	
#ifdef DEBUG
	printk("%s:%d:mangle udp packet.\nBefore:\n\t", __FILE__, __LINE__);

	for (i= 0; i<matchlen; i++) {
		printk("%c", *(*dptr+i+matchoff));
	}

		printk("\nAfter:\n\t");
	
	for (i=0; i<bufflen; i++) {
		printk("%c", buffer[i]);
	}
		printk("\n");

#endif
	if (!ip_nat_mangle_udp_packet(pskb, ct, ctinfo,
				      matchoff, matchlen, buffer, bufflen))
		return 0;

	/* We need to reload this. Thanks Patrick. */

	*dptr = (*pskb)->data + iph->ihl*4 + sizeof(struct udphdr);
	return 1;
}

static int mangle_content_len(struct sk_buff **pskb,
			      enum ip_conntrack_info ctinfo,
			      struct ip_conntrack *ct,
			      const char *dptr)
{
	unsigned int dataoff, matchoff, matchlen;
	char buffer[sizeof("65536")];
	int bufflen;

	struct iphdr *iph = (*pskb)->nh.iph;

#if 0
	dataoff = ip_hdrlen(*pskb) + sizeof(struct udphdr);
#endif
	dataoff = iph->ihl*4  + sizeof(struct udphdr);

	/* Get actual SDP lenght */
	if (ct_sip_get_info(ct, dptr, (*pskb)->len - dataoff, &matchoff,
			    &matchlen, POS_SDP_HEADER) > 0) {

		/* since ct_sip_get_info() give us a pointer passing 'v='
		   we need to add 2 bytes in this count. */
		int c_len = (*pskb)->len - dataoff - matchoff + 2;

		/* Now, update SDP length */
		if (ct_sip_get_info(ct, dptr, (*pskb)->len - dataoff, &matchoff,
				    &matchlen, POS_CONTENT) > 0) {

			bufflen = sprintf(buffer, "%u", c_len);
			return ip_nat_mangle_udp_packet(pskb, ct, ctinfo,
							matchoff, matchlen,
							buffer, bufflen);
		}
	}
	return 0;
}

/* called by another function */ 
static unsigned int mangle_sdp(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       struct ip_conntrack *ct,
			       u32 newip, u16 port,
			       const char *dptr, enum sip_header_pos pos)
{
	char buffer[sizeof("nnn.nnn.nnn.nnn")];
	unsigned int dataoff, bufflen;

	struct iphdr *iph = (*pskb)->nh.iph;

#if 0
	dataoff = ip_hdrlen(*pskb) + sizeof(struct udphdr);
#endif
	dataoff = iph->ihl*4  + sizeof(struct udphdr);

	/* Mangle owner and contact info. */
	bufflen = sprintf(buffer, "%u.%u.%u.%u", NIPQUAD(newip));
	if (!mangle_sip_packet(pskb, ctinfo, ct,(void *) &dptr, (*pskb)->len - dataoff,
			       buffer, bufflen, POS_OWNER_IP4))
		return 0;

	if (!mangle_sip_packet(pskb, ctinfo, ct, (void *)&dptr, (*pskb)->len - dataoff,
			       buffer, bufflen, POS_CONNECTION_IP4))
		return 0;

	/* Mangle media port. */
	bufflen = sprintf(buffer, "%u", port);
	if (!mangle_sip_packet(pskb, ctinfo, ct, (void *)&dptr, (*pskb)->len - dataoff,
			       buffer, bufflen, pos))
		return 0;

	return mangle_content_len(pskb, ctinfo, ct, dptr);
}

#if 0
static unsigned int ip_nat_sdp_expect(struct ip_conntrack *ct,
			      struct ip_conntrack_expect *exp)
#endif
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif 
static unsigned int ip_nat_sdp_expect(struct sk_buff **pskb,
	unsigned int hooknum,
	struct ip_conntrack *ct,
	struct ip_nat_info *info)
{
	int i;
	struct ip_nat_multi_range mr;
	struct ip_ct_sip_expect *exp_sip_info;

	struct ip_conntrack *master = master_ct(ct);

    struct iphdr *iph = (*pskb)->nh.iph; //michael

    /* michael added start, 2008/05/28 */
#ifdef CONFIG_IP_NF_ALGCTRL
	if (!ip_ct_alg_sip) {
		return NF_ACCEPT;
	}
#else
#if defined(CONFIG_PROC_FS)
	if (!sip_alg_flag) {
		return NF_ACCEPT;
	}
#endif
#endif
    /* michael added end, 2008/05/28 */
	
#ifdef DEBUG
#ifdef DEBUGGER
	spin_lock_irqsave(&lock, flags);
#endif
#endif

	IP_NF_ASSERT(master);
	IP_NF_ASSERT(master->nat.info.initialized);
	IP_NF_ASSERT(info);

	if (info->initialized){
#ifdef DEBUG
#ifdef DEBUGGER
	spin_unlock_irqrestore(&lock, flags);
#endif
#endif
		return NF_ACCEPT;
	}

#ifdef DEBUG
	printk("%s:%s:%d RTP arrived, hooknum=%d\n", __FILE__, __func__, __LINE__, hooknum);
#endif

#if 0 /* 2.6 */
	/* This must be a fresh one. */
	BUG_ON(ct->status & IPS_NAT_DONE_MASK);
#endif


	/* Change src to where master sends to */

	#if 0
	range.min_ip = range.max_ip
		= ct->master->tuplehash[!exp->dir].tuple.dst.ip;
	#endif

	/* Fetch the ip_conntrack_expect help structure from this conntrack 
	 *
	 * 							    --atens
	 */
	exp_sip_info = &ct->master->help.exp_sip_info;
	

	/* FIXME: Don't use master's connection, it's not master but the peer's address.
	 * 	  We should use the contact address, however, the contact address might
	 * 	  contain domain name that we could not resolve to ip address.
	 * 	  So, the first candidate ip is contact address. We must check if the 
	 * 	  incoming source address is the same with the contact address. If not,
	 * 	  this is a security breach except that the contact address is FQDN.
	 * 									--atens 
	 */
	
	/* FIXME: Phenomenon but good for Hardware NAT
	 * 	  Since there are two expectations, however, if we saw one expectation and
	 * 	  establish a conntrack for it, then the another expectation will be absorbed
	 * 	  by the first expectation and we will leave up one expectation unseen.
	 * 	  However, this is good for Hardware NAT. 	 
	 *									--atens
	 */
	#if 0
	mr.range[0].min_ip = mr.range[0].max_ip = master->tuplehash[!exp_sip_info->dir].tuple.dst.ip;
	#endif

	for (i=0; i<master->nat.info.num_manips; i++) {

		if (master->nat.info.manips[i].direction == exp_sip_info->dir 
			&& HOOK2MANIP(master->nat.info.manips[i].hooknum) == IP_NAT_MANIP_SRC) {

				mr.rangesize=1;
				mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
				mr.range[0].min_ip = mr.range[0].max_ip = 
					master->nat.info.manips[i].manip.ip;
			break;
		}
	}

	if (i != master->nat.info.num_manips) {
		/* If we cannot find the manips for this exp, that means we 
		 * don't need to do src ip man 
		 * 						--atens */

		/* hook doesn't matter, but it has to do source manip */
		ip_nat_setup_info(ct, &mr, NF_IP_POST_ROUTING);
	}

    /* realtek added start, michael, 2008/05/28, @SIP_LAN2LAN */
    //printk("dev.name=%s\n", (*pskb)->dev->name);
    //printk("saddr=%u.%u.%u.%u  daddr=%u.%u.%u.%u\n",  NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
    if (strncmp((*pskb)->dev->name, "br0", sizeof("br0") - 1) == 0) {
        mr.rangesize=1;
        mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
        mr.range[0].min_ip = mr.range[0].max_ip = iph->daddr; //WAN ip address
        /* hook doesn't matter, but it has to do source manip */
        ip_nat_setup_info(ct, &mr, NF_IP_POST_ROUTING);
    }
    /* realtek added end, michael, 2008/05/28, @SIP_LAN2LAN */

	/* For DST manip, map port here to where it's expected. */
	mr.range[0].flags = (IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED);

	/* 
	 * We don't have these structures, however, we could use the
	 * legacy structure used by 2.4 ftp alg.
	 * 				--atens. 
	 */

	#if 0
	range.min = range.max = exp->saved_proto; 
	range.min_ip = range.max_ip = exp->saved_ip;
	#endif
	mr.rangesize=1;
	mr.range[0].min.udp.port = mr.range[0].max.udp.port = exp_sip_info->saved_port;
	mr.range[0].min_ip = mr.range[0].max_ip = exp_sip_info->saved_ip;

	/* hook doesn't matter, but it has to do destination manip */
	ip_nat_setup_info(ct, &mr, NF_IP_PRE_ROUTING);


#ifdef DEBUG
#ifdef DEBUGGER
	spin_unlock_irqrestore(&lock, flags);
#endif
#endif

	
	return NF_ACCEPT;

}

/* 
 * This function should be translated to be nat helper function 
 * in Linux kernel 2.4. Note that the most important thing is altering
 * the expection since we separate nat helper from conntrack helper.   
 * 								--atens
 */
static unsigned int ip_nat_sdp(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       struct ip_conntrack_expect *exp,
			       const char *dptr, int hooknum, enum sip_header_pos pos)
{
	struct ip_conntrack *ct = exp->expectant;
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	u32 newip;
	/* __be32 newip; */
	u16 port;

	struct ip_conntrack_tuple new_tuple;


	/* Hey, be careful! If we are mainpulating destination IP
	 * then we don't need to change expectation at all.
	 * 
	 * Expectation is expecting the session coming from the
	 * the opposited way in contrary to the original advertisement packet. 
	 * Thus, no matter what the desination of the advertisement packet is, 
	 * we are still expecting the packet whose destination is what we 
	 * expect.
	 * 								--atens
	 */

	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC) {

	/* Why do we use expectation instead of reply? 
	 * The nated session should match the second condition.
	 * Thus, in case of non-nat condition, there might be 
	 * a situation that the sender use different address instead
	 * of being the same with sip channel. 
	 *
	 * For instance, assume that the sip packet was sent from A 
	 * to B, however, sdp channel elected by A uses C as source 
	 * IP, then we should use exp->tuple.dst.ip as newip
	 *
	 * For nated session, because we are symmetric nat system, there
	 * are no problems using the dst ip of the reply session.
	 * 						    --atens
	 *
	 * FIXME: Why would nat helper code has non-nat case?						
	 * 	  Maybe it's due to the difference between 2.4 and 2.6. 
	 * 	  Currently, I don't care about the first case...and I
	 * 	  supppose that it will not happen in my Linux Box.
	 * 	  					    --atens
	 */

	/* Connection will come from reply. */
	if (ct->tuplehash[dir].tuple.src.ip ==
	    ct->tuplehash[!dir].tuple.dst.ip)
		newip = exp->tuple.dst.ip;
	else
		newip = ct->tuplehash[!dir].tuple.dst.ip;

	#if 0
	exp->saved_ip = exp->tuple.dst.u3.ip;
	exp->tuple.dst.u3.ip = newip;
	exp->saved_proto.udp.port = exp->tuple.dst.u.udp.port;
	exp->dir = !dir;

	/* When you see the packet, we need to NAT it the same as the
	   this one. */
	exp->expectfn = ip_nat_sdp_expect;
	#endif

	/* We don't have the the structure listed above, therefore, we 
	 * use legacy structure used by ftp alg in Linux 2.4, and make
	 * sure that we will not exceed the size of the already existing
	 * structure in order to keep the vendor's driver statble.
	 * 						 	--atens
	 */


	/* FIXME:
	 * Race condition would happen here. Assume that
	 * we have already establish a expectation and we
	 * want to re-establilsh a brand new one since the
	 * expected rtp session did not response.
	 *
	 * What we did was sending new sdp that expects the 
	 * same port & the same IP. However, the expectation
	 * estatlishment will fail, and will end up with 
	 * exp->tuple.dst.ip as already translated dst.ip. 
	 * 
	 * It caused the the exp->help.exp_sip_info.saved_ip fail
	 * to setup nat info since the saved ip was wrong (the 
	 * already translated ip). We move the saved_ip to conntrack
	 * code in order to avoid this race condition and I hope that
	 * it will work as I wish.
	 *
	 * 					--atens*/
	IP_NF_ASSERT(exp->help.exp_sip_info.dir == !dir);
	
	/* It's time to change our expectation .
	 * For 2.4, it's really "change" the expectation.
	 * For 2.6, the expectation is brand new one.
	 * No need to change expectation in 2.6.
	 *
	 * 				-- atens 
	 */

	/* We don't care about source port and source ip of the expectation 
	 * at all.
	 *							--atens  
	 */
	new_tuple.src.ip = 0;
	new_tuple.src.u.udp.port = 0;
	new_tuple.dst.ip = newip;                               	
	new_tuple.dst.u.udp.port = exp->tuple.dst.u.udp.port; 
	/* We don't care, we leave the kernel to choose port. */	
	new_tuple.dst.protonum  = IPPROTO_UDP;                  	

#ifdef DEBUG
	printk("%s:%d:new ip %u.%u.%u.%u\n", __FILE__, __LINE__, NIPQUAD(newip));
	printk("%s:%d:new tuple src ip %u.%u.%u.%u  port:%d dsp ip %u.%u.%u.%u port:%d\n", __FILE__, __LINE__, NIPQUAD(new_tuple.src.ip),ntohs(new_tuple.src.u.udp.port), NIPQUAD(new_tuple.dst.ip), ntohs(new_tuple.dst.u.udp.port));
#endif	

	/* find a new port, if the original is not accepted.
	 * 						--atens */
	
	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(exp->tuple.dst.u.udp.port); port != 0; port++) {
	#if 0 	/* For 2.6, not for 2.4. You don't need to create a brand
		   new one expectation   
		   						--atens */
		exp->tuple.dst.u.udp.port = htons(port);
	#endif
		new_tuple.dst.u.udp.port = htons(port);

		if (ip_conntrack_change_expect(exp, &new_tuple) == 0) {
		
			break;
		}
	#if 0   /* This is for 2.6. --atens */
		if (ip_ct_expect_related(exp) == 0)
			break;
	#endif
	}

	/* FIXME: Shall we unexpected the expectation? --atens */
	if (port == 0) {

#ifdef DEBUG
		printk("%s:%d: Change expectation failed.\n", __FILE__, __LINE__);
#endif
		/* We cannot change expectation, that means, we have the same expectation 
		 * before manipulation.
		 * 								--atens
		 */

		return NF_ACCEPT;
	}

#ifdef DEBUG
	printk("%s:%d:Change expectation succeeded.\n", __FILE__, __LINE__);
	printk("%s:%d:new tuple: src %u.%u.%u.%u:%u", __FILE__, __LINE__,NIPQUAD(new_tuple.src.ip), ntohs(new_tuple.src.u.udp.port));
	printk(" dst:%u.%u.%u.%u:%u\n", NIPQUAD(new_tuple.dst.ip), ntohs(new_tuple.dst.u.udp.port));
	printk("%s:%d:Mangling sdp to inform that the listening IP & port\n", __FILE__, __LINE__);
#endif

	if (!mangle_sdp(pskb, ctinfo, ct, newip, port, dptr, pos)) {

		/* To give up the expection changing .
		 *				-- atens
		 *
		 * FIXME: In 2.4.27, unrelated expect is a static function. 
		 * 	  Thus, I cannot call it directly.
		 */

		#if 0
		ip_ct_unexpect_related(exp);
		/* since the unexpect_related is static, we cannot
		 * call it directly.
		 * 				--atens
		 */
		unexpect_related(exp);
		#endif

#ifdef DEBUG
		printk("%s:%d: failed to mangle SDP \n", __FILE__, __LINE__);
		printk("%s:%d:Drop packet\n", __FILE__, __LINE__);
#endif
		return NF_ACCEPT;
	}
	}
	return NF_ACCEPT;
}


/* Not __exit: called from init() */
//static void __exit ip_nat_sip_fini(void)
static void ip_nat_sip_fini(void)
{
	/* no rcu technology now */
	#if 0
	rcu_assign_pointer(ip_nat_sip_hook, NULL);
	rcu_assign_pointer(ip_nat_sdp_hook, NULL);
	synchronize_rcu();
	#endif

	/* trying to unregister all nat helper since the 
	 * module is about to be remove */
	int i;

	for (i = 0; ports[i]!=0; i++) {
		ip_nat_helper_unregister(&sip[i]);
	}

}

static int __init ip_nat_sip_init(void)
{
	/* No hook now, we are using Linux Kernel 2.4 */
	/*
	BUG_ON(rcu_dereference(nf_nat_sip_hook));
	BUG_ON(rcu_dereference(nf_nat_sdp_hook));
	rcu_assign_pointer(nf_nat_sip_hook, ip_nat_sip);
	rcu_assign_pointer(nf_nat_sdp_hook, ip_nat_sdp);
	*/

	/* Register nat helper for sip and prepare 
	 * to manipluating nat address translation 
	 * for expecting traffic */

	int i;
	int ret;
	char *tmpname;
		
	if (ports[0]==0) {
		ports[0]=SIP_PORT;
	}

	for (i=0; ports[i]!=0; i++) {
		/* Register the helper */
		/* Firstly, initialize the helper by memset */
		memset(&sip[i], 0, sizeof(sip[i]));

		sip[i].tuple.dst.protonum = IPPROTO_UDP;
		sip[i].tuple.src.u.udp.port = htons(ports[i]);
		sip[i].mask.src.u.udp.port = htons(0xFFFF);
		sip[i].mask.dst.protonum = 0xff;
		sip[i].help = ip_nat_sip_help;
		sip[i].me = THIS_MODULE;
		/* For REGISTER, we should toggle this flags */ 
		sip[i].flags = IP_NAT_HELPER_F_ALWAYS;  
		sip[i].expect = ip_nat_sdp_expect;
		
		tmpname=&sip_names[i][0];

		if (i==0) {
			sprintf(tmpname, "sip");
		} else {
			sprintf(tmpname, "sip-%d", ports[i]);
		}
		sip[i].name = tmpname;

		ret = ip_nat_helper_register(&sip[i]);

		if (ret) {
#ifdef DEBUG
			printk("ip_nat_sip: error registering sip nat helper for port %d.\n", ports[i]);
#endif

			ip_nat_sip_fini();
			return ret;

		}

	}

	
	return 0;
}

module_init(ip_nat_sip_init);
module_exit(ip_nat_sip_fini);
