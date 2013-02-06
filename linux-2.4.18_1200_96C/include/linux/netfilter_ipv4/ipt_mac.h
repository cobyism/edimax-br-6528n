#ifndef _IPT_MAC_H
#define _IPT_MAC_H

struct ipt_mac_info {
    unsigned char srcaddr[ETH_ALEN];
    int invert;
    int type;
};
#endif /*_IPT_MAC_H*/
