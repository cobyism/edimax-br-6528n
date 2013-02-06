#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <net/udp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_irc.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
struct isakmphdr {
	__u16	source;
	__u16	dest;
	__u16	len;
	__u16	chksum;
	__u64	icookie;
	__u64	rcookie;
};

#ifdef CONFIG_IP_NF_ALGCTRL
int ip_ct_alg_ipsec = 1;
#endif

#define IPPROTO_ESP 0x32

#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int ports_n_c = 0;
extern char ipsec_flag;

unsigned long ip_ct_esp_timeout = 30*HZ;
unsigned long ip_ct_esp_timeout_stream = 180*HZ;

static int esp_pkt_to_tuple(const void *datah, size_t datalen,
			    struct ip_conntrack_tuple *tuple)
{
	const struct udphdr *hdr = datah;

	tuple->src.u.udp.port = hdr->source;
	tuple->dst.u.udp.port = hdr->dest;

	return 1;
}

static int esp_invert_tuple(struct ip_conntrack_tuple *tuple,
			    const struct ip_conntrack_tuple *orig)
{
	tuple->src.u.udp.port = orig->dst.u.udp.port;
	tuple->dst.u.udp.port = orig->src.u.udp.port;
	return 1;
}

/* Print out the per-protocol part of the tuple. */
static unsigned int esp_print_tuple(char *buffer,
				    const struct ip_conntrack_tuple *tuple)
{
	return sprintf(buffer, "sport=%hu dport=%hu ",
		       ntohs(tuple->src.u.udp.port),
		       ntohs(tuple->dst.u.udp.port));
}

/* Print out the private part of the conntrack. */
static unsigned int esp_print_conntrack(char *buffer,
					const struct ip_conntrack *conntrack)
{
	return 0;
}

/* Returns verdict for packet, and may modify conntracktype */
static int esp_packet(struct ip_conntrack *conntrack,
		      struct iphdr *iph, size_t len,
		      enum ip_conntrack_info conntrackinfo)
{
	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */
	if (test_bit(IPS_SEEN_REPLY_BIT, &conntrack->status)) {
		ip_ct_refresh(conntrack, ip_ct_esp_timeout_stream);
		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &conntrack->status);
	} else
		ip_ct_refresh(conntrack, ip_ct_esp_timeout);

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static int esp_new(struct ip_conntrack *conntrack,
			     struct iphdr *iph, size_t len)
{
	return 1;
}
struct ip_conntrack_protocol ip_conntrack_protocol_esp
= { { NULL, NULL }, IPPROTO_ESP, "esp",
    esp_pkt_to_tuple, esp_invert_tuple, esp_print_tuple, esp_print_conntrack,
    esp_packet, esp_new, NULL, NULL, NULL };




/* FIXME: This should be in userspace.  Later. */
static int help(const struct iphdr *iph, size_t len,
		struct ip_conntrack *ct, enum ip_conntrack_info ctinfo)

{
#ifdef CONFIG_IP_NF_ALGCTRL
	if (!ip_ct_alg_ipsec) {
		return NF_ACCEPT;
	}
#endif
	
	if(ipsec_flag == '1')
		/* tcplen not negative guaranteed by ip_conntrack_tcp.c */
		ip_conntrack_protocol_register(&ip_conntrack_protocol_esp);

	
	
	return NF_ACCEPT;

}

static struct ip_conntrack_helper ipsec_helpers[MAX_PORTS];

static void fini(void);

static int __init init(void)
{
	int i, ret;

	/* If no port given, default to standard irc port */

	if (ports[0] == 0)
		ports[0] = 500;

	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		memset(&ipsec_helpers[i], 0,
		       sizeof(struct ip_conntrack_helper));
		ipsec_helpers[i].tuple.src.u.udp.port = htons(ports[i]);
		ipsec_helpers[i].tuple.dst.protonum = IPPROTO_UDP;
		ipsec_helpers[i].mask.src.u.udp.port = 0xFFFF;
		ipsec_helpers[i].mask.dst.protonum = 0xFFFF;
		ipsec_helpers[i].help = help;


		ret = ip_conntrack_helper_register(&ipsec_helpers[i]);

		if (ret) {
			printk("ip_conntrack_ipsec: ERROR registering port %d\n",
				ports[i]);
			fini();
			return -EBUSY;
		}
		ports_n_c++;
	}

	return 0;
}

/* This function is intentionally _NOT_ defined as __exit, because 
 * it is needed by the init function */
static void fini(void)
{
	int i;
	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		ip_conntrack_helper_unregister(&ipsec_helpers[i]);
	}
}

module_init(init);
module_exit(fini);
