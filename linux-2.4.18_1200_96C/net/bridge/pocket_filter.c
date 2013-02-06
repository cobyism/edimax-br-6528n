#include <linux/config.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/brlock.h>
#include <linux/net.h>
#include <linux/socket.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <net/udp.h>
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2
/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_STATIC_ROUTE	0x21 
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d
#define DHCP_NETBIOS_NODETYPE 0x2e
#define DHCP_NETBIOS_SCOPE 0x2F
#define DHCP_END		0xFF
int br_filter_mangle_udp_packet(struct sk_buff **skb, unsigned int match_offset,unsigned int match_len,unsigned char *rep_buffer, unsigned int rep_len);
struct proc_dir_entry *filter_root=NULL;
static struct proc_dir_entry *res1=NULL;
static struct proc_dir_entry *res2=NULL;
int enable_filter=0;
static unsigned char filter_config[256];
int dut_br0_ip=0;
unsigned char dut_br0_mac[6]={0};
unsigned char Filter_State=0;
struct dhcpMessage {
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned int xid;
	unsigned short secs;
	unsigned short flags;
	unsigned int ciaddr;
	unsigned int yiaddr;
	unsigned int siaddr;
	unsigned int giaddr;
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
	unsigned int cookie;
	unsigned char options[308]; /* 312 - cookie */ 
};

struct udp_dhcp_packet {
	struct iphdr ip;
	struct udphdr udp;
	struct dhcpMessage data;
};

static int br_filter_resize_packet(struct sk_buff **skb, int new_size)
{
	struct iphdr *iph;
	


	if (new_size > (*skb)->len + skb_tailroom(*skb)) {
		struct sk_buff *newskb;
		newskb = skb_copy_expand(*skb, skb_headroom(*skb), new_size - (*skb)->len,GFP_ATOMIC);

		if (!newskb) {
			return 0;
		} else {
			kfree_skb(*skb);
			*skb = newskb;
		}
	}
	
	return 1;
}

int br_filter_mangle_udp_packet(struct sk_buff **skb, unsigned int match_offset,unsigned int match_len,unsigned char *rep_buffer, unsigned int rep_len)
{
	struct iphdr *iph = (*skb)->nh.iph;
	struct udphdr *udph = (void *)iph + iph->ihl * 4;
	unsigned char *data;
	u_int32_t udplen, newlen, newudplen;

	udplen = (*skb)->len - iph->ihl*4;
	newudplen = udplen - match_len + rep_len;
	newlen = iph->ihl*4 + newudplen;

	/* UDP helpers might accidentally mangle the wrong packet */
	if (udplen < sizeof(*udph) + match_offset + match_len) {
		return 0;
	}

	if (newlen > 65535) {
		return 0;
	}

	if ((*skb)->len != newlen) {
		if (!br_filter_resize_packet(skb, newlen)) {
			return 0;
		}
	}

	/* skb may be copied !! */
	iph = (*skb)->nh.iph;
	udph = (void *)iph + iph->ihl*4;
	data = (void *)udph + sizeof(struct udphdr);

	if (rep_len != match_len)
		/* move post-replacement */
		memmove(data + match_offset + rep_len,
			data + match_offset + match_len,
			(*skb)->tail - (data + match_offset + match_len));

	/* insert data from buffer */
	memcpy(data + match_offset, rep_buffer, rep_len);

	/* update skb info */
	if (newlen > (*skb)->len) {
		
		skb_put(*skb, newlen - (*skb)->len);
	} else {
		
		skb_trim(*skb, newlen);
	}

	/* update the length of the UDP and IP packets to the new values*/
	udph->len = htons((*skb)->len - iph->ihl*4);
	iph->tot_len = htons(newlen);

	/* fix udp checksum if udp checksum was previously calculated */
	if (udph->check != 0) {
		udph->check = 0;
		udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						newudplen, IPPROTO_UDP,
						csum_partial((char *)udph,
						             newudplen, 0));
	}
	ip_send_check(iph);

	return 1;
}
unsigned char *br_filter_get_dhcp_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	unsigned char *optionptr=NULL;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	optionptr = packet->options;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			return optionptr + i + 2;
		}			
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}
int br_filter_enter(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct udphdr *udph;
	unsigned char *data_start;
	struct udp_dhcp_packet *dhcp_packet=NULL;
	iph = skb->nh.iph;
	int i;
	unsigned int match_offset=0;
	unsigned int match_len=0;
	unsigned char replace_buffer[6]={0};
	unsigned char option_len=0,*temp;
	if(enable_filter==0)
		return 0;
	else{
		
		udph=(void *) iph + iph->ihl*4;
		//panic_printk("start to trace dhcp packet, dst port=%d\n",udph->dest);
		if(iph->protocol==IPPROTO_UDP &&(udph->dest ==67 || udph->dest ==68)){
			if(udph->dest ==67){//from Client host in dhcp 
				//panic_printk("start to trace dhcp packet:client packet from :%s\n",skb->dev->name);
				if(!strcmp(skb->dev->name, "eth0")){
					if(Filter_State==0){ 
						//panic_printk("in this state, we drop client packet:Filter_State=%d\n",Filter_State);
						return 1;
					}
				}
				
			}
			
			if(udph->dest ==68){//from server host in dhcp
				if(Filter_State==1 || Filter_State==0){ //our dut has got ip address
					dhcp_packet =(struct udp_dhcp_packet *)skb->data; 
					if ((temp = br_filter_get_dhcp_option(&(dhcp_packet->data), DHCP_DNS_SERVER))) {
						option_len = (temp-1)[0];
						match_offset=(temp-2)-(unsigned char *)&(dhcp_packet->data);
						match_len = option_len+2;
						
						replace_buffer[0]= DHCP_DNS_SERVER;
						replace_buffer[1]=4;
						replace_buffer[2]=((unsigned char *)&dut_br0_ip)[0];
						replace_buffer[3]=((unsigned char *)&dut_br0_ip)[1];
						replace_buffer[4]=((unsigned char *)&dut_br0_ip)[2];
						replace_buffer[5]=((unsigned char *)&dut_br0_ip)[3];
						if(replace_buffer[2] != 0 || replace_buffer[3] != 0 || replace_buffer[4] != 0 || replace_buffer[5] != 0){
							//panic_printk("in this state, we mangle dhcp server packet, dns ip =%02X%02X%02X%02X\n",replace_buffer[2],replace_buffer[3],replace_buffer[4],replace_buffer[5]);
							br_filter_mangle_udp_packet(&skb, match_offset, match_len, replace_buffer, 6);
						}
					}
				}
					
					
			}
		}
		return 0;
	}
}


static int en_filter_proc_read(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{

      int len=0;

      len = sprintf(page, "%d\n", enable_filter);

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
   	
      return len;

}

static int filter_conf_proc_read(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{

      int len=0;

      len = sprintf(page, "%s\n", filter_config);


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int _is_hex_br_filter(char c)
{
	if(((c >= '0') && (c <= '9')) ||((c >= 'A') && (c <= 'F')) ||((c >= 'a') && (c <= 'f')))
		return 1;
	else
    		return 0;
}
static int en_filter_proc_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char tmpbuf[80];


      if (count < 2)
	    return -EFAULT;
	    
	if (buffer && !copy_from_user(tmpbuf, buffer, 80))  {
		if (tmpbuf[0] == '0'){ 
			enable_filter = 0;	
		}else if (tmpbuf[0] == '1'){
			enable_filter = 1;
		}	    
		return count;
	}	
      return -EFAULT;
}
int br_filter_string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex_br_filter(tmpBuf[0]) || !_is_hex_br_filter(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) simple_strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}
static int filter_conf_proc_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char tmpbuf[1024];
      char *tokptr, *strptr=tmpbuf;
	u_int8_t idx=1;
	u_int32_t val;

      if (count < 2)
	    return -EFAULT;
	memset(filter_config, 0x00, 256);
      if (buffer && !copy_from_user(&filter_config, buffer, count)) {
	      strncpy(tmpbuf,filter_config,count);
	      while ((tokptr = strtok(strptr," ")) !=NULL)
		{
			strptr=NULL;
			val=simple_strtol(tokptr,NULL,0);
			switch(idx)
			{
			case 1:
				val=simple_strtol(tokptr,NULL,16);
				dut_br0_ip=val;
				break;
			case 2:
				br_filter_string_to_hex(tokptr, dut_br0_mac, 12);
				//panic_printk("dut mac=%02X:%02X:%02X:%02X:%02X:%02X\n",dut_br0_mac[0], dut_br0_mac[1], dut_br0_mac[2], dut_br0_mac[3],dut_br0_mac[4],dut_br0_mac[5]); 
				break;
			case 3:
				val=simple_strtol(tokptr,NULL,10);
				Filter_State=val;
				//panic_printk("Filter_State=%d\n",Filter_State);
				break;	
			default:
				break;
			}
			idx++;

		}
	      
	    return count;
      }
      return -EFAULT;
}


int __init br_filter_init(void)
{
#if defined(CONFIG_PROC_FS)

	struct proc_dir_entry *devices = NULL,*pvc = NULL,*svc = NULL;
	filter_root = proc_mkdir("pocket",NULL);
	if (!filter_root){
		panic_printk("create folder fail\n");
		return -ENOMEM;
	}
	res1 = create_proc_entry("en_filter", 0, filter_root);
	if (res1) {
		res1->read_proc = en_filter_proc_read;
		res1->write_proc = en_filter_proc_write;
	}

	res2 = create_proc_entry("filter_conf", 0, filter_root);
	if (res2) {
		res2->read_proc = filter_conf_proc_read;
		res2->write_proc = filter_conf_proc_write;
	}
	
#endif // CONFIG_PROC_FS
	return 0;
}

void __exit br_filter_exit(void)
{
#if defined(CONFIG_PROC_FS)
	if (res1) {
		remove_proc_entry("en_filter", filter_root);
		res2 = NULL;
	}
	if (res2) {
		remove_proc_entry("filter_conf", filter_root);
		res2 = NULL;
	}

	remove_proc_entry("pocket",NULL);
#endif // CONFIG_PROC_FS
}


