/* Kernel module to match MAC address parameters. */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>

#include <linux/netfilter_ipv4/ipt_mac.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netdevice.h>
#include <net/dst.h>

static int compare_with_header_cache_dest_mac(const struct sk_buff *skb, char *macaddr)
{
	struct dst_entry *dst = skb->dst;
	struct hh_cache *hh;
	int ret = 0;
	
	if (dst) {
    	hh = dst->hh;
    	if (hh && (hh->hh_type==ETH_P_IP)) {
    		read_lock_bh(&hh->hh_lock);
      		memcpy(skb->data - 16, hh->hh_data, 16);
      		if (memcmp((((u8*)hh->hh_data) + 2), macaddr, ETH_ALEN) == 0)
      		    ret = 1;
    		read_unlock_bh(&hh->hh_lock);
    	}
    }
	return ret;
}

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      const void *hdr,
      u_int16_t datalen,
      int *hotdrop)
{
    const struct ipt_mac_info *info = matchinfo;

    /* Is mac pointer valid? */
    switch(info->type) {
        case 0:
            return (skb->mac.raw >= skb->head
        	    && (skb->mac.raw + ETH_HLEN) <= skb->data
        	    /* If so, compare... */
        	    && ((memcmp(skb->mac.ethernet->h_source, info->srcaddr, ETH_ALEN)
        		== 0) ^ info->invert));
        case 1:
#if 1			
/* fix the issue: the fragmented UDP packets will not be controlled by kernel scheduling mechanism */
            return ((compare_with_header_cache_dest_mac(skb, (char*)info->srcaddr) ^ info->invert));
#else
            return (skb->mac.raw >= skb->head
        	    && (skb->mac.raw + ETH_HLEN) <= skb->data
        	    /* If so, compare... */
        	    && (compare_with_header_cache_dest_mac(skb, (char*)info->srcaddr) ^ info->invert));
#endif
        default:
            return 0;
    }
}

static int
ipt_mac_checkentry(const char *tablename,
		   const struct ipt_ip *ip,
		   void *matchinfo,
		   unsigned int matchsize,
		   unsigned int hook_mask)
{
	/* FORWARD isn't always valid, but it's nice to be able to do --RR */
    if (((struct ipt_mac_info *)matchinfo)->type==0) {
        if (hook_mask
            & ~((1 << NF_IP_PRE_ROUTING) | (1 << NF_IP_LOCAL_IN)
        	| (1 << NF_IP_FORWARD))) {
        	printk("ipt_mac: only valid for PRE_ROUTING, LOCAL_IN or FORWARD.\n");
        	return 0;
        }
    }
    else {
        if (hook_mask
            & ~(1 << NF_IP_POST_ROUTING)) {
        	printk("ipt_mac: only valid for POST_ROUTING.\n");
        	return 0;
        }
    }

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_mac_info)))
		return 0;

	return 1;
}

static struct ipt_match mac_match
= { { NULL, NULL }, "mac", &match, &ipt_mac_checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_match(&mac_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&mac_match);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
