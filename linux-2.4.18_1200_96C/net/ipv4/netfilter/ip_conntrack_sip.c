/* SIP extension for IP connection tracking.
 *
 * (C) 2005 by Christian Hentschel <chentschel@arnet.com.ar>
 * based on RR's ip_conntrack_ftp.c and other modules.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 
 * 4 Apr 2008: 
 * 	atens <atens.huang@gmail.com>
 * 	- Back ported to Linux Kernel 2.4.
 * 	- Add sip helper for conntrack system.
 * 	- Remove 2.6 information in order to fit the architecture of 2.4.
 */

#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/net.h>
#include <linux/netfilter.h>
#if defined(CONFIG_PROC_FS)  //michael
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#endif

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_conntrack_sip.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
#include <asm/mips16_lib.h>
#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hentschel <chentschel@arnet.com.ar>");
MODULE_DESCRIPTION("SIP connection tracking helper");


#define MAX_PORTS	8
static unsigned short ports[MAX_PORTS] = {[0 ...MAX_PORTS-1] = 0};

MODULE_PARM(ports,"1-8i");
MODULE_PARM_DESC(ports, "port numbers of SIP servers");

static unsigned short sip_timeout /*__read_mostly*/ = SIP_TIMEOUT;
/* 2.6? */
/* module_param(sip_timeout, uint, 0600);*/
MODULE_PARM(sip_timeout, "h");
MODULE_PARM_DESC(sip_timeout, "timeout for the master SIP session");


static int digits_len(struct ip_conntrack *, const char *, const char *, int *);
static int epaddr_len(struct ip_conntrack *, const char *, const char *, int *);
static int skp_digits_len(struct ip_conntrack *, const char *, const char *, int *);
static int skp_epaddr_len(struct ip_conntrack *, const char *, const char *, int *);

struct sip_header_nfo {
	const char	*lname;
	const char	*sname;
	const char	*ln_str;
	size_t		lnlen;
	size_t		snlen;
	size_t		ln_strlen;
	int		case_sensitive;
	int		(*match_len)(struct ip_conntrack *, const char *,
				     const char *, int *);
};

static const struct sip_header_nfo ct_sip_hdrs[] = {
	[POS_REG_REQ_URI] = { 	/* SIP REGISTER request URI */
		.lname		= "sip:",
		.lnlen		= sizeof("sip:") - 1,
		.ln_str		= ":",
		.ln_strlen	= sizeof(":") - 1,
		.match_len	= epaddr_len,
	},
	[POS_REQ_URI] = { 	/* SIP request URI */
		.lname		= "sip:",
		.lnlen		= sizeof("sip:") - 1,
		.ln_str		= "@",
		.ln_strlen	= sizeof("@") - 1,
		.match_len	= epaddr_len,
	},
	[POS_FROM] = {		/* SIP From header */
		.lname		= "From:",
		.lnlen		= sizeof("From:") - 1,
		.sname		= "\r\nf:",
		.snlen		= sizeof("\r\nf:") - 1,
		.ln_str		= "sip:",
		.ln_strlen	= sizeof("sip:") - 1,
		.match_len	= skp_epaddr_len,
	},
	[POS_TO] = {		/* SIP To header */
		.lname		= "To:",
		.lnlen		= sizeof("To:") - 1,
		.sname		= "\r\nt:",
		.snlen		= sizeof("\r\nt:") - 1,
		.ln_str		= "sip:",
		.ln_strlen	= sizeof("sip:") - 1,
		.match_len	= skp_epaddr_len
	},
	[POS_VIA] = { 		/* SIP Via header */
		.lname		= "Via:",
		.lnlen		= sizeof("Via:") - 1,
		.sname		= "\r\nv:",
		.snlen		= sizeof("\r\nv:") - 1, /* rfc3261 "\r\n" */
		.ln_str		= "UDP ",
		.ln_strlen	= sizeof("UDP ") - 1,
		.match_len	= epaddr_len,
	},
	[POS_CONTACT] = { 	/* SIP Contact header */
		.lname		= "Contact:",
		.lnlen		= sizeof("Contact:") - 1,
		.sname		= "\r\nm:",
		.snlen		= sizeof("\r\nm:") - 1,
		.ln_str		= "sip:",
		.ln_strlen	= sizeof("sip:") - 1,
		.match_len	= skp_epaddr_len
	},
	[POS_CONTENT] = { 	/* SIP Content length header */
		.lname		= "Content-Length:",
		.lnlen		= sizeof("Content-Length:") - 1,
		.sname		= "\r\nl:",
		.snlen		= sizeof("\r\nl:") - 1,
		.ln_str		= ":",
		.ln_strlen	= sizeof(":") - 1,
		.match_len	= skp_digits_len
	},
	[POS_MEDIA] = {		/* SDP media info */
		.case_sensitive	= 1,
		.lname		= "\nm=",
		.lnlen		= sizeof("\nm=") - 1,
		.sname		= "\rm=",
		.snlen		= sizeof("\rm=") - 1,
		.ln_str		= "audio ",
		.ln_strlen	= sizeof("audio ") - 1,
		.match_len	= digits_len
	},
	[POS_MEDIA_VIDEO] = {		/* SDP media info */
		.case_sensitive	= 1,
		.lname		= "\nm=",
		.lnlen		= sizeof("\nm=") - 1,
		.sname		= "\rm=",
		.snlen		= sizeof("\rm=") - 1,
		.ln_str		= "video ",
		.ln_strlen	= sizeof("video ") - 1,
		.match_len	= digits_len
	},
	[POS_OWNER_IP4] = {	/* SDP owner address*/
		.case_sensitive	= 1,
		.lname		= "\no=",
		.lnlen		= sizeof("\no=") - 1,
		.sname		= "\ro=",
		.snlen		= sizeof("\ro=") - 1,
		.ln_str		= "IN IP4 ",
		.ln_strlen	= sizeof("IN IP4 ") - 1,
		.match_len	= epaddr_len
	},
	[POS_CONNECTION_IP4] = {/* SDP connection info */
		.case_sensitive	= 1,
		.lname		= "\nc=",
		.lnlen		= sizeof("\nc=") - 1,
		.sname		= "\rc=",
		.snlen		= sizeof("\rc=") - 1,
		.ln_str		= "IN IP4 ",
		.ln_strlen	= sizeof("IN IP4 ") - 1,
		.match_len	= epaddr_len
	},
	[POS_OWNER_IP6] = {	/* SDP owner address*/
		.case_sensitive	= 1,
		.lname		= "\no=",
		.lnlen		= sizeof("\no=") - 1,
		.sname		= "\ro=",
		.snlen		= sizeof("\ro=") - 1,
		.ln_str		= "IN IP6 ",
		.ln_strlen	= sizeof("IN IP6 ") - 1,
		.match_len	= epaddr_len
	},
	[POS_CONNECTION_IP6] = {/* SDP connection info */
		.case_sensitive	= 1,
		.lname		= "\nc=",
		.lnlen		= sizeof("\nc=") - 1,
		.sname		= "\rc=",
		.snlen		= sizeof("\rc=") - 1,
		.ln_str		= "IN IP6 ",
		.ln_strlen	= sizeof("IN IP6 ") - 1,
		.match_len	= epaddr_len
	},
	[POS_SDP_HEADER] = { 	/* SDP version header */
		.case_sensitive	= 1,
		.lname		= "\nv=",
		.lnlen		= sizeof("\nv=") - 1,
		.sname		= "\rv=",
		.snlen		= sizeof("\rv=") - 1,
		.ln_str		= "=",
		.ln_strlen	= sizeof("=") - 1,
		.match_len	= digits_len
	}
};

/* get line lenght until first CR or LF seen. */
int ct_sip_lnlen(const char *line, const char *limit)
{
	const char *k = line;

	while ((line <= limit) && (*line == '\r' || *line == '\n'))
		line++;

	while (line <= limit) {
		if (*line == '\r' || *line == '\n')
			break;
		line++;
	}
	return line - k;
}
EXPORT_SYMBOL(ct_sip_lnlen);

/* Linear string search, case sensitive. */
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif 
const char *ct_sip_search(const char *needle, const char *haystack,
			  size_t needle_len, size_t haystack_len,
			  int case_sensitive)
{
	const char *limit = haystack + (haystack_len - needle_len);

	while (haystack <= limit) {
		if (case_sensitive) {
			if (strncmp(haystack, needle, needle_len) == 0)
				return haystack;
		} else {
			if (strnicmp(haystack, needle, needle_len) == 0)
				return haystack;
		}
		haystack++;
	}
	return NULL;
}

EXPORT_SYMBOL(ct_sip_search);

static int digits_len(struct ip_conntrack *ct, const char *dptr,
		      const char *limit, int *shift)
{
	int len = 0;
	while (dptr <= limit && isdigit(*dptr)) {
		dptr++;
		len++;
	}
	return len;
}

/* get digits lenght, skiping blank spaces. */
static int skp_digits_len(struct ip_conntrack *ct, const char *dptr,
			  const char *limit, int *shift)
{
	for (; dptr <= limit && *dptr == ' '; dptr++)
		(*shift)++;

	return digits_len(ct, dptr, limit, shift);
}

int sip_parse_addr(struct ip_conntrack *ct, const char *cp, const char **endp,
		      union ip_conntrack_address *addr, const char *limit)
{
	const char *end;
	int ret = 0;
	int family = AF_INET;

	/* But we just leave the original switching code unchange. */
	switch (family) {
	case AF_INET:
		ret = in4_pton(cp, limit - cp, (void *)&(addr->ip), -1, &end);
		break;
#if 0
	case AF_INET6:
		ret = in6_pton(cp, limit - cp, (void *)&(addr->ip6), -1, &end);
		break;
	default:
		BUG();
#endif
		
	}

	if (ret == 0 || end == cp)
		return 0;
	if (endp)
		*endp = end;
	return 1;
}

/* skip ip address. returns its length. */
static int epaddr_len(struct ip_conntrack *ct, const char *dptr,
		      const char *limit, int *shift)
{
	union ip_conntrack_address addr;
	const char *aux = dptr;

	if (!sip_parse_addr(ct, dptr, &dptr, &addr, limit)) {
		pr_debug("ip: %s parse failed.!\n", dptr);
		return 0;
	}

	/* Port number */
	if (*dptr == ':') {
		dptr++;
		dptr += digits_len(ct, dptr, limit, shift);
	}
	return dptr - aux;
}

/* get address length, skiping user info. */
static int skp_epaddr_len(struct ip_conntrack *ct, const char *dptr,
			  const char *limit, int *shift)
{
	const char *start = dptr;
	int s = *shift;

	/* Search for @, but stop at the end of the line.
	 * We are inside a sip: URI, so we don't need to worry about
	 * continuation lines. */
	while (dptr <= limit &&
	       *dptr != '@' && *dptr != '\r' && *dptr != '\n') {
		(*shift)++;
		dptr++;
	}

	if (dptr <= limit && *dptr == '@') {
		dptr++;
		(*shift)++;
	} else {
		dptr = start;
		*shift = s;
	}

	return epaddr_len(ct, dptr, limit, shift);
}

/* Returns 0 if not found, -1 error parsing. */
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
__NOMIPS16
#endif 
int ct_sip_get_info(struct ip_conntrack *ct,
		    const char *dptr, size_t dlen,
		    unsigned int *matchoff,
		    unsigned int *matchlen,
		    enum sip_header_pos pos)
{
	const struct sip_header_nfo *hnfo = &ct_sip_hdrs[pos];
	const char *limit, *aux, *k = dptr;
	int shift = 0;

	limit = dptr + (dlen - hnfo->lnlen);

	while (dptr <= limit) {
		if ((strncmp(dptr, hnfo->lname, hnfo->lnlen) != 0) &&
		    (hnfo->sname == NULL ||
		     strncmp(dptr, hnfo->sname, hnfo->snlen) != 0)) {
			dptr++;
			continue;
		}
		aux = ct_sip_search(hnfo->ln_str, dptr, hnfo->ln_strlen,
				    ct_sip_lnlen(dptr, limit),
				    hnfo->case_sensitive);
		if (!aux) {
#ifdef DEBUG
			printk("'%s' not found in '%s'.\n", hnfo->ln_str,
				 hnfo->lname);
#endif
			dptr++;
			continue;
		}
		aux += hnfo->ln_strlen;

		*matchlen = hnfo->match_len(ct, aux, limit, &shift);
		if (!*matchlen)
			return -1;

		*matchoff = (aux - k) + shift;

#ifdef DEBUG
		printk("%s match succeeded! - len: %u\n", hnfo->lname,
			 *matchlen);
#endif
		return 1;
	}
#ifdef DEBUG
	printk("%s header not found.\n", hnfo->lname);
#endif
	return 0;
}
EXPORT_SYMBOL(ct_sip_get_info);
EXPORT_SYMBOL(sip_parse_addr);

static int set_expected_rtp(struct ip_conntrack *ct,
			    enum ip_conntrack_info ctinfo,
			    union ip_conntrack_address *addr,
			    u16 port,
			    const char *dptr)
{
	struct ip_conntrack_expect expect, *exp=&expect;
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	/* No IPV6 now --atens */
	#if 0
	int family = ct->tuplehash[!dir].tuple.src.l3num;   
	#endif
	int ret;
	/*  gcc 2.x does not support typeof operator --atens */
	#if 0
	typeof(ip_nat_sdp_hook) ip_nat_sdp;
	#endif

	/* realtek added start, michael, 2008/05/28, @SIP_LAN2LAN */
	/* not to set expectation for reply direction */
    if (dir != IP_CT_DIR_ORIGINAL)
        return NF_ACCEPT;
	/* realtek added end, michael, 2008/05/28 */

	memset(exp, 0, sizeof (expect));

#if 0
	exp = ip_ct_expect_alloc(ct);
	if (exp == NULL)
		return NF_DROP;

	
	ip_ct_expect_init(exp, family,
			  &ct->tuplehash[!dir].tuple.src.u3, addr,
			  IPPROTO_UDP, NULL, &port);

#endif
        exp->tuple = (struct ip_conntrack_tuple) { 
		/* FIXME: The server's address or the peer's address? 
		 *
		 * 	  Since the media issuer is not expected to 
		 * 	  restrict the source ip, we, the NAT gateway,
		 * 	  won't restrict it too.
		 * 	  				--atens
		 */
		/* src.ip & source port (don't care) */
		#if 0  
		{ ct->tuplehash[!dir].tuple.src.ip, {0}	},
		#endif
		{ 0, {0}},
		/* dst.ip & dsp.port */
		{ (u32)addr->ip , { port }, 
		IPPROTO_UDP
		}
	};

	exp->mask = ((struct ip_conntrack_tuple) { 
		/*	{ 0xFFFFFFFF, { 0 } }, */
			{ 0x0, { 0 } },
	                { 0xFFFFFFFF, { .udp = { 0xFFFF } }, 0xFFFF }
		    });

	/* We want expectfn callback in nat, not now.
	 * 				     -- atens
	 */
	exp->expectfn = NULL;

	exp->help.exp_sip_info.dir=!dir;
	exp->help.exp_sip_info.saved_ip=exp->tuple.dst.ip;
	exp->help.exp_sip_info.saved_port=exp->tuple.dst.u.udp.port;

#ifdef DEBUG
	printk("%s:%d:ip_conntrack_expect_related, expected structure\n", __FILE__,__LINE__);
	printk("%s:%d:source ip: %u.%u.%u.%u source port: %u destination ip: %u.%u.%u.%u\n destination port: %u\n protocol:%u\n", __FILE__, __LINE__,
	NIPQUAD(exp->tuple.src.ip), ntohs(exp->tuple.src.u.udp.port), 
	NIPQUAD(exp->tuple.dst.ip), ntohs(exp->tuple.dst.u.udp.port), exp->tuple.dst.protonum);
#endif

	if (ip_conntrack_expect_related(ct, &expect) != 0) {
	#ifdef DEBUG
		printk("Failed to relate the expect\n");
	#endif
		ret = NF_DROP;
	} else {
	#ifdef DEBUG
		printk("Succeed to relate the expect\n");
	#endif
		ret = NF_ACCEPT;
	}

	/* No NAT here */
	#if 0
	nf_nat_sdp = rcu_dereference(nf_nat_sdp_hook);
	if (nf_nat_sdp && ct->status & IPS_NAT_MASK)
		ret = nf_nat_sdp(pskb, ctinfo, exp, dptr);
	else {
		if (nf_ct_expect_related(exp) != 0)
			ret = NF_DROP;
		else
			ret = NF_ACCEPT;
	}
	nf_ct_expect_put(exp);
	#endif

	return NF_ACCEPT;
}

/* michael added start, 2008/05/28 */
#ifdef CONFIG_IP_NF_ALGCTRL
int ip_ct_alg_sip = 1;
#else
#if defined(CONFIG_PROC_FS)
static struct proc_dir_entry *sip_alg_entry=NULL;
int sip_alg_flag=1;
EXPORT_SYMBOL(sip_alg_flag);

static int sip_read_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%d\n", sip_alg_flag);


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
static int sip_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char tmp;
      if (count < 2)
	    return -EFAULT;
      
      if (buffer && !copy_from_user(&tmp, buffer, 1)) {
	    sip_alg_flag = tmp - '0';
	    return count;
      }
      return -EFAULT;
}
#endif //CONFIG_PROC_FS
#endif //CONFIG_IP_NF_ALGCTRL
/* michael added end, 2008/05/28 */

/* 
 * The help function accept the iph, len, conntrack structure 
 * and conntrack info for this packet as argments. Using iph, 
 * len instead of skb means you cannot modify the packet info.
 *
 * For 2.6, the helper function combine conntrack helper and
 * nat helper into one, though you could separate into two as
 * 2.4.
 * 						--atens
 */ 
#if 0
static int sip_help(struct sk_buff **pskb,
		    unsigned int protoff,
		    struct ip_conntrack *ct,
		    enum ip_conntrack_info ctinfo)
#endif		 
static int sip_help(const struct iphdr *iph, size_t len,
                struct ip_conntrack *ct,
                enum ip_conntrack_info ctinfo)
{
#if 0
	int family = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
#endif
	union ip_conntrack_address addr;
	unsigned int dataoff, datalen;
	const char *dptr;
	int ret = NF_ACCEPT;
	int matchoff, matchlen;
	u_int16_t port;
	enum sip_header_pos pos;
	
	/* 
	 * We don't do nat helping in 2.4 now,but leave it to ip_nat_sip.c.
	 *
	 *   				   			--atens 
	 */


	unsigned int protoff = iph->ihl * 4;
	dataoff = protoff + sizeof(struct udphdr);

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

	/* No Data? */
	if (len <= iph->ihl*4) {
		return NF_ACCEPT;
	}

	/* We have seen this kind of packet, update the timeout value 
	 * 						     --atens
	 */
	ip_ct_refresh(ct, sip_timeout * HZ);

	/* Calculate the dptr here */
	dptr = (const char *)iph + dataoff;

	/* datalen = (*pskb)->len - dataoff; */
	datalen = len - dataoff;

	if (datalen < sizeof("SIP/2.0 200") - 1) {
	#ifdef DEBUG
		printk("%s:%d:not enough data\n", __FILE__, __LINE__);
	#endif
		goto out;
	}

	/* RTP info only in some SDP pkts */
	if (memcmp(dptr, "INVITE", sizeof("INVITE") - 1) != 0 &&
	    memcmp(dptr, "UPDATE", sizeof("UPDATE") - 1) != 0 &&
	    memcmp(dptr, "SIP/2.0 180", sizeof("SIP/2.0 180") - 1) != 0 &&
	    memcmp(dptr, "SIP/2.0 183", sizeof("SIP/2.0 183") - 1) != 0 &&
	    memcmp(dptr, "SIP/2.0 200", sizeof("SIP/2.0 200") - 1) != 0) {

#ifdef DEBUG
		printk("%s:%d:Cannot find RTP info\n", __FILE__, __LINE__);
#endif

		goto out;
	}
	/* Fetch address and port from SDP packet. --atens */

	pos = POS_CONNECTION_IP4; /* IPV6? Not today. */

	if (ct_sip_get_info(ct, dptr, datalen, &matchoff, &matchlen, pos) > 0) {

#ifdef DEBUG
		printk("Parse addr inside sip packet.\n");
#endif

		/* We'll drop only if there are parse problems. */
		if (!sip_parse_addr(ct, dptr + matchoff, NULL, &addr,
				dptr + datalen)) {
			ret = NF_DROP;
			goto out;
		}

		

#ifdef DEBUG
		printk("%s:%d:Get rtp listen address as %u.%u.%u.%u\n", __FILE__, __LINE__, NIPQUAD(addr.ip));
#endif

		if (ct_sip_get_info(ct, dptr, datalen, &matchoff, &matchlen,
				    POS_MEDIA) > 0) {

			port = simple_strtoul(dptr + matchoff, NULL, 10);
			if (port < 1024) {
				ret = NF_DROP;
				goto out;
			}

#ifdef DEBUG
		printk("%s:%d:Get rtp listen port for audio as %u\n", __FILE__, __LINE__,port);
#endif

			ret = set_expected_rtp(ct, ctinfo, &addr,
					       htons(port), dptr);

			if (ret == NF_DROP) {
				goto out;
			}

		if (ct_sip_get_info(ct, dptr, datalen, &matchoff, &matchlen, 
				POS_MEDIA_VIDEO) > 0) {

			port = simple_strtoul(dptr + matchoff, NULL, 10);
			if (port < 1024) {
				ret = NF_DROP;
				goto out;
			}

#ifdef DEBUG
		printk("%s:%d:Get rtp listen port for video as %u\n", __FILE__, __LINE__,port);
#endif

			ret = set_expected_rtp(ct, ctinfo, &addr, htons(port), dptr);
			
		}

		}
	}
out:
	/* We have done. The SIP conntrack system is back ported to 2.4 now */
	return ret;
}

/* No IPV6 now. --atens */
static struct ip_conntrack_helper sip[MAX_PORTS] /*__read_mostly */;
static char sip_names[MAX_PORTS][sizeof("sip-65535")] /*__read_mostly */;

static void ip_conntrack_sip_fini(void)
{
	int i;

    /* michael added start, 2008/05/28 */
#ifndef CONFIG_IP_NF_ALGCTRL
#if defined(CONFIG_PROC_FS)
	if (sip_alg_entry) {
		remove_proc_entry("sip_alg", sip_alg_entry);		
		sip_alg_entry = NULL;
	}
#endif
#endif
    /* michael added end, 2008/05/28 */

	for (i = 0; ports[i] != 0; i++) {
		if (sip[i].me == NULL)
			continue;
#ifdef DEBUG
		printk("Unregister conntrack helper for ports[%d]=%d\n", i, ports[i]);
#endif
		ip_conntrack_helper_unregister(&sip[i]);
	}
}

static int __init ip_conntrack_sip_init(void)
{
	int i, ret;
	char *tmpname;

    /* michael added start, 2008/05/28 */
#ifndef CONFIG_IP_NF_ALGCTRL
#if defined(CONFIG_PROC_FS)
    sip_alg_entry=create_proc_entry("sip_alg",0,NULL);
    if (sip_alg_entry) {
	    sip_alg_entry->read_proc = sip_read_proc;
	    sip_alg_entry->write_proc = sip_write_proc;
    }
#endif //CONFIG_PROC_FS
#endif
    /* michael added end, 2008/05/28 */

	if (ports[0] == 0)
		ports[0] = SIP_PORT;

	for (i = 0; ports[i] != 0; i++) {
		memset(&sip[i], 0, sizeof(sip[i]));

		sip[i].tuple.dst.protonum = IPPROTO_UDP;
		sip[i].tuple.src.u.udp.port = htons(ports[i]);
                sip[i].mask.src.u.udp.port = htons(0xFFFF);
		sip[i].mask.dst.protonum = 0xff;
		sip[i].max_expected = 4;
		#if 0
		sip[i].timeout = 3 * 60; /* 3 minutes */
		#endif
		/* 
		 * Actually, this timeout is used for expectation instead of SIP timeout.
		 * SIP timeout is specified whenever the helper sees the SIP packet. 
		 * 								--atens */
		sip[i].timeout = 60; /* To avoid too long time expectation */
		sip[i].me = THIS_MODULE;
		sip[i].help = sip_help;

		/* Reuse expectation to avoid inconsistency of the helper state machine */
		sip[i].flags= IP_CT_HELPER_F_REUSE_EXPECT; 

		tmpname = &sip_names[i][0];
		if (ports[i] == SIP_PORT)
			sprintf(tmpname, "sip");
			else
				sprintf(tmpname, "sip-%u", i);
			sip[i].name = tmpname;


			ret = ip_conntrack_helper_register(&sip[i]);
			if (ret) {
#ifdef DEBUG
				printk("nf_ct_sip: failed to register helper "
				       "for pf: %u port: %u\n",
				       AF_INET, ports[i]);
#endif
				ip_conntrack_sip_fini();
				return ret;
			}

#ifdef DEBUG
			printk("ports[%d]=%d registered as sip alg\n", i, ports[i]);
#endif
	}
	return 0;
}

module_init(ip_conntrack_sip_init);
module_exit(ip_conntrack_sip_fini);
