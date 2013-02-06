/*
 *      Realtek VLAN handler 
 *
 *      $Id: rtk_vlan.c,v 1.5.4.4 2011/05/02 07:52:13 bruce Exp $
 */
 
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <asm/string.h>
#include <linux/rtk_vlan.h>


//---------------------------------------------------------------------------

#if 0
#define DEBUG_ERR(format, args...) panic_printk("%s [%s]: "format, __FUNCTION__, dev->name, ## args)
#else
#define DEBUG_ERR(format, args...)
#endif


#if 0
#define DEBUG_TRACE(format, args...) panic_printk("%s [%s]: "format, __FUNCTION__, dev->name, ## args)
#else
#define DEBUG_TRACE(format, args...)
#endif


//---------------------------------------------------------------------------

#define VLAN_HLEN	4
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
#define MAX_IFACE_VLAN_CONFIG 11
#define WAN_IFACE_INDEX MAX_IFACE_VLAN_CONFIG-1
#define VIRTUAL_IFACE_INDEX MAX_IFACE_VLAN_CONFIG-2
static struct vlan_info_item vlan_infos[MAX_IFACE_VLAN_CONFIG];
static unsigned char wan_macaddr[6] = {0};
static unsigned char lan_macaddr[6] = {0};
#if defined(CONFIG_RTK_STB_PORT_SUPPORT) || defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
#include <linux/ip.h>
#include <linux/udp.h>

char vlan_portlist_init = 0;
char vlan_portlist = 0;

struct bootp_pkt {              /* BOOTP packet format */
	 struct iphdr iph;       /* IP header */
	 struct udphdr udph;     /* UDP header */
	 u8 op;                  /* 1=request, 2=reply */
	 u8 htype;               /* HW address type */
	 u8 hlen;                /* HW address length */
	 u8 hops;                /* Used only by gateways */
	 u32 xid;             /* Transaction ID */
	 u16 secs;            /* Seconds since we started */
	 u16 flags;           /* Just what it says */
	 u32 client_ip;               /* Client's IP address if known */
	 u32 your_ip;         /* Assigned IP address */
	 u32 server_ip;               /* (Next, e.g. NFS) Server's IP address */
	 u32 relay_ip;                /* IP address of BOOTP relay */
	 u8 hw_addr[16];         /* Client's HW address */
	 u8 serv_name[64];       /* Server host name */
	 u8 boot_file[128];      /* Name of boot file */
	 u8 exten[312];          /* DHCP options / BOOTP vendor extensions */
};

unsigned char BRCST_MAC[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
static unsigned char mcast_macaddr[3] = {0x01,0x00,0x5e};

static unsigned short eth_arp = 0x0806;
static unsigned short eth_ipv6 = 0x86dd;

void hex_dump(void *data, int size)
{
    /* dumps size bytes of *data to stdout. Looks like:
     * [0000] 75 6E 6B 6E 6F 77 6E 20
     *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
     * (in a single line of course)
     */

    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }

        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) {
            /* line completed */
            printk("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
			charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printk("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}
#endif	//CONFIG_RTK_STB_PORT_SUPPORT

#endif


#define COPY_TAG(tag, info) { \
	tag.f.tpid =  htons(ETH_P_8021Q); \
	tag.f.pci = (unsigned short) (((((unsigned char)info->pri)&0x7) << 13) | \
					((((unsigned char)info->cfi)&0x1) << 12) |((unsigned short)info->id&0xfff)); \
	tag.f.pci =  htons(tag.f.pci);	\
}


#define STRIP_TAG(skb) { \
	memmove(skb->data+VLAN_HLEN, skb->data, ETH_ALEN*2); \
	skb_pull(skb, VLAN_HLEN); \
}


//---------------------------------------------------------------------------
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
extern int is_MulticastData(struct sk_buff *skb);

struct net_device *get_wan_from_vlan(void)
{
	return vlan_infos[MAX_IFACE_VLAN_CONFIG-1].dev;
}

struct net_device *get_dev_from_bridgedvlan(void)
{
	int index=0;

	for( ;index<VIRTUAL_IFACE_INDEX;index++) {
		if( vlan_infos[index].info.forwarding == 1 )
			return vlan_infos[index].dev;
	}
	return NULL;
}


struct vlan_info_item *query_vlan_info(int target_vid)
{
	struct vlan_info_item *item = NULL;
	int index=0;

	for( ;index<VIRTUAL_IFACE_INDEX;index++) {
		item = &vlan_infos[index];
		if( item->dev && item->info.id == target_vid )
			return item;
	}
	return NULL;
}

struct vlan_info_item *search_vlan_info(struct net_device *dev)
{
	struct vlan_info_item *item = NULL;
	int index=0;

	for( ;index<MAX_IFACE_VLAN_CONFIG;index++) {
		item = &vlan_infos[index];
		if( item->dev && memcmp(dev->name,item->dev->name,16) == 0 ) {	// 16 = IFNAMSIZ
			DEBUG_TRACE("found in list id:%d, is LAN:%d, enable:%d\n",item->info.id,item->info.is_lan,item->info.vlan);
			return item;
		}
	}
	return NULL;
}

int init_vlan_port_list(void)
{
	struct vlan_info_item *pitem = NULL;
	struct net_device *pdev1 = NULL;
	struct net_device *pdev2 = NULL;
	struct net_device *pdev3 = NULL;
	struct net_device *pdev4 = NULL;
	int index = 0;

	pdev1=dev_get_by_name("eth0");
	pdev2=dev_get_by_name("eth2");
	pdev3=dev_get_by_name("eth3");
	pdev4=dev_get_by_name("eth4");

	if (pdev1 && pdev2 && pdev3 && pdev4)
	{
		vlan_portlist_init = 1;

		if((pitem = search_vlan_info(pdev1)) && pitem->info.forwarding == 1)
			vlan_portlist |= (1<<0);
		if((pitem = search_vlan_info(pdev2)) && pitem->info.forwarding == 1)
			vlan_portlist |= (1<<1);
		if((pitem = search_vlan_info(pdev3)) && pitem->info.forwarding == 1)
			vlan_portlist |= (1<<2);
		if((pitem = search_vlan_info(pdev4)) && pitem->info.forwarding == 1)
			vlan_portlist |= (1<<3);
	}

	return vlan_portlist_init;
}

int add_vlan_info(struct vlan_info *info, struct net_device *dev)
{
	struct vlan_info_item *item = NULL;
	int index=0;

	if( memcmp(dev->name,"eth0",4) == 0 ){
		memcpy(lan_macaddr, dev->dev_addr, 6);
	}


	if( memcmp(dev->name,"eth1",4) == 0 ){
		item = &vlan_infos[MAX_IFACE_VLAN_CONFIG-1];

		memset(item, 0, sizeof(struct vlan_info_item));
		memcpy(&item->info, info, sizeof(struct vlan_info));
		item->dev = dev;
		memcpy(wan_macaddr, dev->dev_addr, 6);
		DEBUG_TRACE("WAN port vlan id:%d, is LAN:%d, fowarding:%d, dev:%p\n",item->info.id,item->info.is_lan,item->info.forwarding,item->dev);
		return 0;
	}

	if( memcmp(dev->name,"eth5",4) == 0 ){
		item = &vlan_infos[VIRTUAL_IFACE_INDEX];

		memset(item, 0, sizeof(struct vlan_info_item));

		for( ;index<VIRTUAL_IFACE_INDEX;index++) {
			/* constrain: there should be only one interface bridged */
			if( vlan_infos[index].info.forwarding == 1 ) {
				memcpy(info,&vlan_infos[index].info,sizeof(*info));
				info = &vlan_infos[index].info;
				break;
			}
		}
		memcpy(&item->info, info, sizeof(struct vlan_info));
		
		item->dev = dev;
		DEBUG_TRACE("Virtual port vlan id:%d, is LAN:%d, fowarding:%d, dev:%p\n",item->info.id,item->info.is_lan,item->info.forwarding,item->dev);
		return 0;
	}

	if((item = search_vlan_info(dev)) != NULL){
                if( memcmp(dev->name,"eth5",4) == 0 ){
                        item = &vlan_infos[VIRTUAL_IFACE_INDEX];
                        memset(item, 0, sizeof(struct vlan_info_item));

                        for( ;index<VIRTUAL_IFACE_INDEX;index++) {
                                /* constrain: there should be only one interface bridged */
                                if( vlan_infos[index].info.forwarding == 1 ) {
                                        memcpy(info,&vlan_infos[index].info,sizeof(*info));
                                        info = &vlan_infos[index].info;
                                        break;
                                }
                        }
                        memcpy(&item->info, info, sizeof(struct vlan_info));
                        DEBUG_TRACE("Virtual port vlan id:%d, is LAN:%d, fowarding:%d, dev:%p\n",item->info.id,item->info.is_lan,item->info.forwarding,item->dev);
                        return 0;
        }
                memcpy(&item->info, info, sizeof(struct vlan_info));
                return 0;
        }

	for( ;index<MAX_IFACE_VLAN_CONFIG;index++) {
		if( vlan_infos[index].dev == NULL ) {
			item = &vlan_infos[index];

			memset(item, 0, sizeof(*item));
			memcpy(&item->info, info, sizeof(*info));
			item->dev = dev;

			DEBUG_TRACE("insert vlan id:%d, is LAN:%d, enable:%d\n",item->info.id,item->info.is_lan,item->info.vlan);
			return 0;
		}
	}

	if( item == NULL ) {
		DEBUG_ERR("VLAN info. list is FULL\n");
		return -1;
	}
}

int delete_vlan_info(struct net_device *dev)
{
	struct vlan_info_item *item = NULL;
	int index=0;

	if((item = search_vlan_info(dev)) != NULL){
		memset(item, 0, sizeof(struct vlan_info_item));
		return 0;
	} else {
		DEBUG_ERR("VLAN info. list is FULL\n");
		return -1;
	}
}

int discard_by_vlan(struct net_device *dev)
{
	struct vlan_info_item *item = search_vlan_info(dev);

	if( item && item->info.forwarding == 1 && item->info.global_vlan && item->info.vlan ){
		DEBUG_TRACE("<Drop> Packet from bridged VLAN, not passsed up\n");
		return 1;
	}
	DEBUG_TRACE("<== Rx packet\n");

	return 0;
}
#endif

#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
int  rx_vlan_process(struct net_device *dev, struct vlan_info *info_ori, struct sk_buff *skb,  struct sk_buff **new_skb)
#else
int  rx_vlan_process(struct net_device *dev, struct vlan_info *info, struct sk_buff *skb) 
#endif
{
	struct vlan_tag tag;
	unsigned short vid;
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	struct vlan_info_item *item = NULL;
#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
	struct vlan_info *info, info_bak;
	memcpy(&info_bak, info_ori, sizeof(struct vlan_info));
	info= &info_bak;
	if (new_skb)
		*new_skb = NULL;
#endif
#endif

	DEBUG_TRACE("==> Process Rx packet\n");

	if (!info->global_vlan) {
		DEBUG_TRACE("<== Return w/o change due to gvlan not enabled\n");
		return 0;
	}

	memcpy(&tag, skb->data+ETH_ALEN*2, VLAN_HLEN);

#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
	if (vlan_portlist_init == 0)
		if (0 == init_vlan_port_list())
			return -1;

	if (info->is_lan) {
		skb->src_info = info_ori;
	}
#endif

	// When port-vlan is disabled, discard tag packet
	if (!info->vlan) {
		if (tag.f.tpid == htons(ETH_P_8021Q)) {
			DEBUG_ERR("<Drop> due to packet w/ tag!\n");
			return 1;
		}
		DEBUG_TRACE("<== Return w/o change, and indicate not from vlan port enabled\n");
		skb->tag.f.tpid = 1; // indicate this packet come from the port w/o vlan enabled
		return 0;
	}

#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
if (info->forwarding!=0)
{
	if (info->is_lan && (info->tag&0x2))
	{
		info->tag = 1;
	}
	else if (!info->is_lan && (info->tag&0x1))
	{
		info->tag = 1;
	}
	else
		info->tag = 0;

	if(!memcmp(lan_macaddr,skb->data+6, 6) && info->forwarding==1) { //src mac == eth0
				return 1;
	}
}
#endif

	// Drop all no-tag packet if port-tag is enabled
	if (info->tag && tag.f.tpid != htons(ETH_P_8021Q)) {
		DEBUG_ERR("<Drop> due to packet w/o tag but port-tag is enabled!\n");
		return 1;
	}
	
	if (tag.f.tpid == htons(ETH_P_8021Q)) { // tag existed in incoming packet	
		if (info->is_lan) {	
			// Drop all tag packets if VID is not matched
			vid = ntohs(tag.f.pci & 0xfff);
			if (vid != (unsigned short)info->id) {
				DEBUG_ERR("<Drop> due to VID not matched!\n");
				return 1;			
			}		

#if 0 //defined(CONFIG_RTK_VLAN_ROUTETYPE)
			if (info->is_lan && (info->forwarding==1) ){
				skb->tag = tag;
				STRIP_TAG(skb);
				return -3;
			}
#endif
		
		}
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		else {	//wan interface	
			vid = ntohs(tag.f.pci & 0xfff);
			item = query_vlan_info(vid);
			
			if( item ) {
				if( item->info.forwarding == 1 ){
					skb->dev = vlan_infos[VIRTUAL_IFACE_INDEX].dev;	//return virtual interface
					DEBUG_TRACE("WAN:%02x bridge to VLAN:%02x\n",dev,skb->dev);
				} else if ( item->info.forwarding == 0 ) {
					DEBUG_TRACE("<Drop> due to VLAN is disabled\n");
					return 1;
				} else {
					DEBUG_TRACE("Recv from WAN normally\n");
				}
			}
		}
#endif
		skb->tag = tag;
		STRIP_TAG(skb);
		DEBUG_TRACE("<== Tag [%x, vid=%d] existed in Rx packet, strip it and pass up\n", 
			tag.v, (int)ntohs(tag.f.pci&0xfff));	
	}
	else	 {
#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
		//if (!info->is_lan && vlan_portlist)
		if (!info->is_lan)
		{
			if (skb->data[0] & 0x01)
			{
				if (new_skb) {
					*new_skb = skb_clone(skb, GFP_ATOMIC);
					if (*new_skb == NULL) {
						DEBUG_ERR("skb_clone() failed!\n");
					}
					else {
						struct vlan_info *new_info =  &vlan_infos[VIRTUAL_IFACE_INDEX].info;
						(*new_skb)->dev = vlan_infos[VIRTUAL_IFACE_INDEX].dev;	//return virtual interface
						COPY_TAG((*new_skb)->tag, new_info);
						(*new_skb)->src_info = new_info; //brucehou, eth5 is a lan side interface(in br0)
						(*new_skb)->src_info->idx=1;
					}
				}
			}
			else if (memcmp(wan_macaddr,skb->data,6)) //untag br
			{
				skb->dev = vlan_infos[VIRTUAL_IFACE_INDEX].dev;	//return virtual interface
				info = &vlan_infos[VIRTUAL_IFACE_INDEX].info;
			}
		}
#endif
#if defined(CONFIG_RTK_STB_PORT_SUPPORT)
		if( !info->is_lan && (memcmp(wan_macaddr,skb->data,6) && memcmp(skb->data+12,&eth_ipv6,2)) ) {
			/* special case handling for broadcast	*/
			if(!memcmp(BRCST_MAC,skb->data,6)) {
				struct bootp_pkt *btp = (struct bootp_pkt *) (skb->data+14);
				/* ARP request, receive to WAN */
				if(!memcmp(skb->data+12,&eth_arp,2))
					goto legacy_rx;

				/* DHCP */
				if( !memcmp(dev->dev_addr,btp->hw_addr,6) || !memcmp(skb->data+12,&eth_arp,2) )
					goto legacy_rx;

			} else if ( !memcmp(mcast_macaddr,skb->data,3)||(!memcmp(skb->data+12,&eth_arp,2)&&!memcmp(skb->data,dev->dev_addr,6)) )
				/* Multicast address ,ARP Response receive to WAN */
				goto legacy_rx;
			
			skb->dev = vlan_infos[VIRTUAL_IFACE_INDEX].dev;	//return virtual interface
			DEBUG_TRACE("Dest: %02x:%02x:%02x:%02x:%02x:%02x from WAN:%02x bridge to VLAN:%02x\n",
				skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5],dev,skb->dev);
			info = &vlan_infos[VIRTUAL_IFACE_INDEX].info;

			COPY_TAG(skb->tag, info);
			DEBUG_TRACE("<== No tag existed, carry forwarding port tag [%x, vid=%d] and pass up\n", 
				skb->tag.v, (int)ntohs(skb->tag.f.pci&0xfff));

			return -1;
		} else if (info->is_lan && (info->forwarding==1) ){
			return -1;
		}
#endif	//CONFIG_RTK_STB_PORT_SUPPORT
legacy_rx:
		// Store port tag to skb and then pass up
		COPY_TAG(skb->tag, info);		
		DEBUG_TRACE("<== No tag existed, carry port tag [%x, vid=%d] and pass up\n", 
			skb->tag.v, (int)ntohs(skb->tag.f.pci&0xfff));
	}
	return 0;
}

#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
int  tx_vlan_process(struct net_device *dev, struct vlan_info *info_ori, struct sk_buff *skb, int wlan_pri)
#else 
int  tx_vlan_process(struct net_device *dev, struct vlan_info *info, struct sk_buff *skb, int wlan_pri) 
#endif
{
	struct vlan_tag tag, *adding_tag=NULL;
#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
	struct vlan_info *info, info_bak;

	if (skb->src_info != NULL && !info_ori->is_lan) {
		memcpy(&info_bak, skb->src_info, sizeof(struct vlan_info));
		info_bak.is_lan = info_ori->is_lan;
	}
	else{
		memcpy(&info_bak, info_ori, sizeof(struct vlan_info));
	}
	info= &info_bak;
#endif


	DEBUG_TRACE("==> Process Tx packet\n");

	if (wlan_pri)
		skb->cb[0] = '\0';		// for WMM priority
	
	if (!info->global_vlan) {
		DEBUG_TRACE("<== Return w/o change due to gvlan not enabled\n");
		return 0;
	}

	if (!info->vlan) {
		// When port-vlan is disabled, discard packet if packet come from source port w/ vlan enabled
		if (skb->tag.f.tpid == htons(ETH_P_8021Q)) {
			DEBUG_ERR("<Drop> due to port-vlan is disabled but Tx packet w/o vlan enabled!\n");
			return 1;
		}
		DEBUG_TRACE("<== Return w/o change because both Tx port and source vlan not enabled\n");
		return 0;
	}

	// Discard packet if packet come from source port w/o vlan enabled except from protocol stack
	if (skb->tag.f.tpid != 0) {
		if (skb->tag.f.tpid != htons(ETH_P_8021Q)) {
			DEBUG_ERR("<Drop> due to port-vlan is enabled but not from vlan enabled port!\n");
			return 1;
		}
		
		// Discard packet if its vid not matched, except it come from protocol stack or lan
		if (info->is_lan && ntohs(skb->tag.f.pci&0xfff) != ((unsigned short)info->id)) {
			DEBUG_ERR("<Drop> due to VID is not matched!\n");	
			return 1;			
		}	
	}

#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
 if (info->forwarding!=0)
{
 	if (info->is_lan && (info->tag&0x2))
	{
 		info->tag = 1;
	}
	else if (!info->is_lan)
	{
		if ((info->tag&0x1) || ntohs(skb->tag.f.pci&0xfff))
		info->tag = 1;
	}
	else {
		info->tag = 0;
	}

	if(!memcmp(lan_macaddr,skb->data+6, 6) && info->forwarding==1) { //src mac == eth0
		return 1;
	}
}
#endif
	
	if (!info->tag) {
	#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
		struct vlan_info_item *pitem = search_vlan_info(skb->dev);
	#endif

		DEBUG_TRACE("<== Return w/o tagging\n");
		if (wlan_pri) {
			if (!info->is_lan &&  skb->tag.f.tpid == htons(ETH_P_8021Q)) 
				skb->cb[0] = (unsigned char)((ntohs(skb->tag.f.pci)>>13)&0x7);
			else 
				skb->cb[0] = (unsigned char)info->pri;		

		#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
			return 0;
		#endif
		}		

	#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
		if ((skb->data[0] & 0x01) && skb->len > 34 && is_MulticastData(skb) && get_eth_snooping_portlist(skb))
		{
			skb->cb[0] &= 0x7f;
			if (skb->src_info && skb->src_info->idx == 0) {
				skb->cb[0] &= ~vlan_portlist;
			}
			else if (skb->src_info && skb->src_info->idx == 1) {
				skb->cb[0] &= vlan_portlist;
			}
			else {
				skb->cb[0] = 0;
			}
		}
	#endif /* #if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO) */

		return 0;		
	}
	
	// Add tagging
	if (!info->is_lan && skb->tag.f.tpid != 0) { // WAN port and not from local, add source tag
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		struct vlan_info_item *item = query_vlan_info(skb->tag.f.pci & 0xfff);
		if( item && item->info.forwarding == 0 ) {	//check if packet form VLAN that disable traffic to/from  WAN
			DEBUG_ERR("<Drop> due to VLAN is disabled\n");	
			return 1;
		} else 
#endif
		{
		adding_tag = &skb->tag;
		DEBUG_TRACE("<== Return w/ source port tagging [vid=%d]\n", (int)ntohs(skb->tag.f.pci&0xfff));
		}
	}
	else {			
		adding_tag = NULL;
		DEBUG_TRACE("<== Return w/ port tagging [vid=%d]\n", info->id);
	}

	memcpy(&tag, skb->data+ETH_ALEN*2, VLAN_HLEN);	
	if (tag.f.tpid !=  htons(ETH_P_8021Q)) { // tag not existed, insert tag
		if (skb_headroom(skb) < VLAN_HLEN && skb_cow(skb, VLAN_HLEN) !=0 ) {		
			panic_printk("%s-%d: error! (skb_headroom(skb) == %d < 4). Enlarge it!\n",
			__FUNCTION__, __LINE__, skb_headroom(skb));
			while (1) ;
		}
		skb_push(skb, VLAN_HLEN);
		memmove(skb->data, skb->data+VLAN_HLEN, ETH_ALEN*2);
	}	

	if (!adding_tag)	{ // add self-tag
		COPY_TAG(tag, info);
		adding_tag = &tag;		
	}

	memcpy(skb->data+ETH_ALEN*2, adding_tag, VLAN_HLEN);

	if (wlan_pri) 
		skb->cb[0] = (unsigned char)((ntohs(adding_tag->f.pci)>>13)&0x7);
		
	return 0;	
}

	

