#ifndef __IP_CONNTRACK_SIP_H__
#define __IP_CONNTRACK_SIP_H__

#ifdef __KERNEL__
#include <linux/netfilter_ipv4/lockhelp.h>
#endif

#define SIP_PORT	5060
#define SIP_TIMEOUT	900

enum sip_header_pos {
	POS_REG_REQ_URI,
	POS_REQ_URI,
	POS_FROM,
	POS_TO,
	POS_VIA,
	POS_CONTACT,
	POS_CONTENT,
	POS_MEDIA,
	POS_MEDIA_VIDEO,
	POS_OWNER_IP4,
	POS_CONNECTION_IP4,
	POS_OWNER_IP6,
	POS_CONNECTION_IP6,
	POS_SDP_HEADER,
};


struct ip_ct_sip_expect {
	enum ip_conntrack_dir dir;
	u_int16_t saved_port;
	u_int32_t saved_ip;
};
#if 0
extern unsigned int (*ip_nat_sip_hook)(struct sk_buff **pskb,
				       enum ip_conntrack_info ctinfo,
				       struct ip_conntrack *ct,
				       const char **dptr);
extern unsigned int (*ip_nat_sdp_hook)(struct sk_buff **pskb,
				       enum ip_conntrack_info ctinfo,
				       struct ip_conntrack_expect *exp,
				       const char *dptr);
#endif				       

extern int ct_sip_get_info(struct ip_conntrack *ct, const char *dptr, size_t dlen,
			   unsigned int *matchoff, unsigned int *matchlen,
			   enum sip_header_pos pos);
extern int ct_sip_lnlen(const char *line, const char *limit);
extern const char *ct_sip_search(const char *needle, const char *haystack,
				 size_t needle_len, size_t haystack_len,
				 int case_sensitive);
extern int sip_parse_addr(struct ip_conntrack *, const char *, const char **,
                      union ip_conntrack_address *, const char *);
#endif /* __IP_CONNTRACK_SIP_H__ */
