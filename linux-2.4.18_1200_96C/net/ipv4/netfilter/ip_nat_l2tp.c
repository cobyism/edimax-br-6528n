#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
#include <asm/mips16_lib.h>
#endif
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
extern unsigned int _br0_ip;
extern unsigned int _br0_mask;
#endif
#define SUCCESS 0
#define FAIL    1    
#define MaxSession 16
struct l2tp_header
{
	u_int16_t flag;
	u_int16_t length;
	u_int16_t tunnnel_id;
	u_int16_t session_id;
};
struct l2tp_session
{
	u_int32_t redirect_ip;
	u_int16_t tunnel_id;
};

static u_int32_t last_ip=0,LocalIp,AliasIp,tmp_ip;
static u_int32_t session_cnt=0;
static struct l2tp_session *session=NULL;
static char flag='1';

;
unsigned int l2tpOutbound(unsigned int hooknum,u_int32_t srcip, struct iphdr *iph,struct l2tp_header *dptr)
{
        static struct l2tp_session *tp;
	tp=session;
	if ((ntohs(dptr->flag)&0xf4ff)==0xc002)
	{
	 if(dptr->tunnnel_id==0 && dptr->session_id==0)
	  {
	  if(hooknum==0)
	    tmp_ip=iph->saddr;
	  if(hooknum==4)
	    {
            last_ip=tmp_ip;
	    AliasIp=iph->saddr;
	    return SUCCESS;
	    }

	  }
	}
	return FAIL;

}
unsigned int l2tpInbound(u_int32_t srcip, struct iphdr *iph,struct l2tp_header *dptr)
{
	int i;
	if(last_ip==0) return FAIL;
	for(i=0; i< MaxSession; i++)
	  {
	    if(session[i].tunnel_id==dptr->tunnnel_id && dptr->tunnnel_id !=0)
	      {
	      LocalIp=session[i].redirect_ip;
	      return SUCCESS;
	      }
	  }
	if((ntohs(dptr->flag) && 0xf4ff) ==0xc002)
	{
           for(i=0;i< MaxSession ;i++)
             {
             if(session[i].redirect_ip==last_ip)
	       {
               session[i].tunnel_id=dptr->tunnnel_id;
	       break;
	       }
	     }
	   if(i== MaxSession)
	     {
             session[session_cnt].tunnel_id=dptr->tunnnel_id;
             session[session_cnt].redirect_ip=last_ip;
	     session_cnt=session_cnt+1;
	     session_cnt=(session_cnt)% MaxSession;
	     }
	 }
	      LocalIp=last_ip;
	      return SUCCESS;
		     
	          		  
			  
}
static unsigned int help(struct ip_conntrack *ct,
			 struct ip_conntrack_expect *exp,
			 struct ip_nat_info *info,
			 enum ip_conntrack_info ctinfo,
			 unsigned int hooknum, 
			 struct sk_buff **pskb)
{
	struct l2tp_session *tp;
	tp = session;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *udph = (void *) iph + iph->ihl * 4;
	unsigned int ret;
	int dir = CTINFO2DIR(ctinfo);
	u_int32_t s_addr, d_addr;
        struct l2tp_header *dptr;
	if(flag=='0')
	  return NF_ACCEPT;
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
 
	if((_br0_ip !=0x0) && ((_br0_ip & _br0_mask) == (ct->tuplehash[dir].tuple.src.ip & _br0_mask))
		&& ((_br0_ip & _br0_mask) == (ct->tuplehash[!dir].tuple.src.ip & _br0_mask))){ //l2tp server is in LAN side
			return NF_ACCEPT;
	}  
#endif
	s_addr=iph->saddr;
	d_addr=iph->daddr;
	dptr = (struct l2tp_header *) ((void *) udph + sizeof(struct udphdr));
      	ret=l2tpOutbound(hooknum,ct->tuplehash[dir].tuple.src.ip,iph,dptr);
	if(ret==SUCCESS)
	  return NF_ACCEPT;
	if(ct->tuplehash[dir].tuple.dst.ip==AliasIp && hooknum==0)
	  {
      	  ret=l2tpInbound(ct->tuplehash[dir].tuple.src.ip,iph,dptr);
	  if(ret==SUCCESS) 
            {		  
	    iph->daddr=LocalIp;
	    iph->check=0;
	    iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
	    udph->check=0;
	    udph->check=csum_partial((char *)udph,ntohs(udph->len),0);
	    udph->check=csum_tcpudp_magic(iph->saddr,iph->daddr ,ntohs(udph->len),IPPROTO_UDP,udph->check);
	    }
	  }
			
	return NF_ACCEPT;
}
static int read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c\n", flag);


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
static int write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{

      if (count < 2)
	    return -EFAULT;

      if (buffer && !copy_from_user(&flag, buffer, 1)) {
	    return count;
      }
      return -EFAULT;
}



static struct ip_nat_helper ip_nat_l2tp_helpers;
static struct ip_nat_helper ip_nat_l2tp_helpers = { { NULL, NULL},
	                            "l2tp",
				    IP_NAT_HELPER_F_ALWAYS,
				    THIS_MODULE,
				    { { 0, { __constant_htons(1701) } },
			              { 0, { 0 }, IPPROTO_UDP } },
		                    { { 0, { 0xFFFF } },
		                      { 0, { 0 }, 0xFFFF } },
	                              help, NULL };

/* This function is intentionally _NOT_ defined as  __exit, because
 * it is needed by init() */
static void fini(void)
{

	ip_nat_helper_unregister(&ip_nat_l2tp_helpers);
}

static int __init init(void)
{
	int ret = 0;
	struct proc_dir_entry *res=create_proc_entry("nat_l2tp",0,NULL);
        if (res) {
	    res->read_proc=read_proc;
	    res->write_proc=write_proc;
        }

	ret = ip_nat_helper_register(&ip_nat_l2tp_helpers);

	session = kmalloc(MaxSession*(sizeof(struct l2tp_session)),GFP_KERNEL);
	memset(session,0,MaxSession*(sizeof(struct l2tp_session)));


	return ret;
}


module_init(init);
module_exit(fini);
