/*
 *   API-compatible handling routines
 *
 *
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_COMAPI_C_


#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#endif

#ifdef __LINUX_2_6__
#include <linux/initrd.h>
#include <linux/syscalls.h>
#endif

#include "./8192cd_debug.h"
#include "./8192cd_comapi.h"
#include "./8192cd_headers.h"

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
#include <linux/if_arp.h>
#include <net/iw_handler.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define IWE_STREAM_ADD_EVENT(_A, _B, _C, _D, _E)		iwe_stream_add_event(_A, _B, _C, _D, _E)
#define IWE_STREAM_ADD_POINT(_A, _B, _C, _D, _E)		iwe_stream_add_point(_A, _B, _C, _D, _E)
#define IWE_STREAM_ADD_VALUE(_A, _B, _C, _D, _E, _F)	iwe_stream_add_value(_A, _B, _C, _D, _E, _F)
#else
#define IWE_STREAM_ADD_EVENT(_A, _B, _C, _D, _E)		iwe_stream_add_event(_B, _C, _D, _E)
#define IWE_STREAM_ADD_POINT(_A, _B, _C, _D, _E)		iwe_stream_add_point(_B, _C, _D, _E)
#define IWE_STREAM_ADD_VALUE(_A, _B, _C, _D, _E, _F)	iwe_stream_add_value(_B, _C, _D, _E, _F)
#endif

typedef struct _CH_FREQ_MAP_{
	UINT16		channel;
	UINT16		freqKHz;
}CH_FREQ_MAP;

CH_FREQ_MAP CH_HZ_ID_MAP[] =
{
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
	{14, 2484},

	/*	UNII */
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
	{52, 5260},
	{56, 5280},
	{60, 5300},
	{64, 5320},
	{149, 5745},
	{153, 5765},
	{157, 5785},
	{161, 5805},
	{165, 5825},
	{167, 5835},
	{169, 5845},
	{171, 5855},
	{173, 5865},
				
	/* HiperLAN2 */
	{100, 5500},
	{104, 5520},
	{108, 5540},
	{112, 5560},
	{116, 5580},
	{120, 5600},
	{124, 5620},
	{128, 5640},
	{132, 5660},
	{136, 5680},
	{140, 5700},
				
	/* Japan MMAC */
	{34, 5170},
	{38, 5190},
	{42, 5210},
	{46, 5230},
			
	/*	Japan */
	{184, 4920},
	{188, 4940},
	{192, 4960},
	{196, 4980},
	
	{208, 5040},	/* Japan, means J08 */
	{212, 5060},	/* Japan, means J12 */	 
	{216, 5080},	/* Japan, means J16 */
};
	
int CH_HZ_ID_MAP_NUM = (sizeof(CH_HZ_ID_MAP)/sizeof(CH_FREQ_MAP));


#define     MAP_CHANNEL_ID_TO_KHZ(_ch, _khz)                 			\
		do{                                           								\
			int _chIdx;											\
			for (_chIdx = 0; _chIdx < CH_HZ_ID_MAP_NUM; _chIdx++)\
			{													\
				if ((_ch) == CH_HZ_ID_MAP[_chIdx].channel)			\
				{												\
					(_khz) = CH_HZ_ID_MAP[_chIdx].freqKHz * 1000; 	\
					break;										\
				}												\
			}													\
			if (_chIdx == CH_HZ_ID_MAP_NUM)					\
				(_khz) = 2412000;									\
		}while(0)

#define     MAP_KHZ_TO_CHANNEL_ID(_khz, _ch)                 \
			do{ 																		\
				int _chIdx; 										\
				for (_chIdx = 0; _chIdx < CH_HZ_ID_MAP_NUM; _chIdx++)\
				{													\
					if ((_khz) == CH_HZ_ID_MAP[_chIdx].freqKHz) 		\
					{												\
						(_ch) = CH_HZ_ID_MAP[_chIdx].channel;			\
						break;										\
					}												\
				}													\
				if (_chIdx == CH_HZ_ID_MAP_NUM) 				\
					(_ch) = 1;											\
			}while(0)
							
/*
struct iw_statistics *rtl8192cd_get_wireless_stats(struct net_device *net_dev)
{
	// client mode only
	return NULL;
}
*/

int rtl_siwfreq(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	unsigned int chan=0;

#ifndef WIFI_HAPD
    //check if the interface is down
    if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	if (wrqu->freq.e > 1)
		return -EINVAL;

	if((wrqu->freq.e == 0) && (wrqu->freq.m <= 1000))
		chan = wrqu->freq.m;	// Setting by channel number 
	else
		MAP_KHZ_TO_CHANNEL_ID( (wrqu->freq.m /100) , chan); // Setting by frequency - search the table , like 2.412G, 2.422G, 

	priv->pmib->dot11RFEntry.dot11channel = chan;
	
#ifdef WIFI_HAPD	
    if (!netif_running(priv->dev))
		return 0;
	else		
#endif		
	SwChnl(priv, chan, priv->pshare->offset_2nd_chan);
    return 0;
}

int rtl_giwfreq(struct net_device *dev,
		   struct iw_request_info *info,
		   union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	unsigned int ch;
	//unsigned long	m = 2412000;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	ch = priv->pmib->dot11RFEntry.dot11channel;
/*
	MAP_CHANNEL_ID_TO_KHZ(ch, m);
	wrqu->freq.m = m * 100;
	wrqu->freq.e = 1;
	wrqu->freq.i = 0;
*/
	wrqu->freq.m = ch;
	wrqu->freq.e = 0;
	wrqu->freq.i = 0;
	
	return 0;
}

int rtl_siwmode(struct net_device *dev,
			   struct iw_request_info *a,
			   union iwreq_data *wrqu, char *b)
{
	int ret = 0;
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif
	
	switch(wrqu->mode)
	{
		case IW_MODE_AUTO: // 0
			OPMODE &= WIFI_AP_STATE;
			printk("set_mode = IW_MODE_AUTO\n");
			break;
		case IW_MODE_ADHOC: // 1
			OPMODE &= WIFI_ADHOC_STATE;
			printk("set_mode = IW_MODE_ADHOC\n");
			break;
		case IW_MODE_MASTER: // 3
			OPMODE &= WIFI_AP_STATE;
			printk("set_mode = IW_MODE_MASTER\n");
//			setopmode_cmd(padapter, networkType);
			break;
		case IW_MODE_INFRA: // 2
			OPMODE &= WIFI_STATION_STATE;
			printk("set_mode = IW_MODE_INFRA\n");
			break;

		default :
			ret = -EINVAL;
	}

	return ret;
	
}

int rtl_giwmode(struct net_device *dev,
		   struct iw_request_info *info,
		   union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	if (OPMODE & WIFI_AP_STATE)
		wrqu->mode = IW_MODE_MASTER;
	else if (OPMODE & WIFI_STATION_STATE)
		wrqu->mode = IW_MODE_INFRA;
	else if (OPMODE & WIFI_ADHOC_STATE)
		wrqu->mode = IW_MODE_ADHOC;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,20))
	else if (OPMODE & WIFI_SITE_MONITOR)
		wrqu->mode = IW_MODE_MONITOR;
#endif
    else
        wrqu->mode = IW_MODE_AUTO;

	return 0;
}

#define MAX_FRAG_THRESHOLD 2346
#define MIN_FRAG_THRESHOLD 256

int rtl_giwrange(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;
	int i;
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	
	if (priv == NULL || range == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	wrqu->data.length = sizeof(*range);
	memset(range, 0, sizeof(*range));

	/* Let's try to keep this struct in the same order as in
	 * linux/include/wireless.h
	 */

	/* TODO: See what values we can set, and remove the ones we can't
	 * set, or fill them with some default data.
	 */

	/* ~5 Mb/s real (802.11b) */
	range->throughput = 5 * 1000 * 1000;

	// TODO: Not used in 802.11b?
//	range->min_nwid;	/* Minimal NWID we are able to set */
	// TODO: Not used in 802.11b?
//	range->max_nwid;	/* Maximal NWID we are able to set */

	/* Old Frequency (backward compat - moved lower ) */
//	range->old_num_channels;
//	range->old_num_frequency;
//	range->old_freq[6]; /* Filler to keep "version" at the same offset */

	/* signal level threshold range */


	//percent values between 0 and 100.
	range->max_qual.qual = 100;
	//range->max_qual.level = 100;
	//range->max_qual.noise = 100;
	range->max_qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_INVALID | IW_QUAL_NOISE_INVALID; /* Updated only qual b'coz not sure */


	range->avg_qual.qual = 60; 
	/* TODO: Find real 'good' to 'bad' threshol value for RSSI */
	//range->avg_qual.level = 20;
	//range->avg_qual.noise = 0;
	range->avg_qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_INVALID | IW_QUAL_NOISE_INVALID; /* Updated only qual b'coz not sure */

	range->num_bitrates = AP_BSSRATE_LEN;
	
	for(i=0; i<AP_BSSRATE_LEN && i < IW_MAX_BITRATES; i++)
	{
		if (AP_BSSRATE[i] == 0x00)
			break;
		range->bitrate[i] = (AP_BSSRATE[i]&0x7f)* 500000;
	}

	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;

	range->pm_capa = 0;

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 12;

//	range->retry_capa;	/* What retry options are supported */
//	range->retry_flags;	/* How to decode max/min retry limit */
//	range->r_time_flags;	/* How to decode max/min retry life */
//	range->min_retry;	/* Minimal number of retries */
//	range->max_retry;	/* Maximal number of retries */
//	range->min_r_time;	/* Minimal retry lifetime */
//	range->max_r_time;	/* Maximal retry lifetime */

	range->encoding_size[0]=5;
	range->encoding_size[1]=13;
	range->num_encoding_sizes = 2;
	range->max_encoding_tokens = 4;
	range->num_channels = priv->available_chnl_num;
	range->num_frequency = priv->available_chnl_num;
	
	for (i = 0; i < priv->available_chnl_num && i < IW_MAX_FREQUENCIES; i++) {
		u32 m = 0;
		range->freq[i].i = i + 1;
		//range->freq[val].m = CH_HZ_ID_MAP[i].freqKHz * 100000;
		MAP_CHANNEL_ID_TO_KHZ(priv->available_chnl[i], m);
		range->freq[i].m = m* 100;
		range->freq[i].e = 1;
	}

// Commented by Albert 2009/10/13
// The following code will proivde the security capability to network manager.
// If the driver doesn't provide this capability to network manager,
// the WPA/WPA2 routers can't be choosen in the network manager.

/*
#define IW_SCAN_CAPA_NONE		0x00
#define IW_SCAN_CAPA_ESSID		0x01
#define IW_SCAN_CAPA_BSSID		0x02
#define IW_SCAN_CAPA_CHANNEL	0x04
#define IW_SCAN_CAPA_MODE		0x08
#define IW_SCAN_CAPA_RATE		0x10
#define IW_SCAN_CAPA_TYPE		0x20
#define IW_SCAN_CAPA_TIME		0x40
*/

#if WIRELESS_EXT > 17
	range->enc_capa = IW_ENC_CAPA_WPA|IW_ENC_CAPA_WPA2|
				IW_ENC_CAPA_CIPHER_TKIP|IW_ENC_CAPA_CIPHER_CCMP;
#endif

#ifdef IW_SCAN_CAPA_ESSID
	range->scan_capa = IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE |IW_SCAN_CAPA_BSSID|
					IW_SCAN_CAPA_CHANNEL|IW_SCAN_CAPA_MODE|IW_SCAN_CAPA_RATE;
#endif
	return 0;
}

int rtl_siwap(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{

	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	if (priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
	    memset(&priv->pmib->dot11OperationEntry.hwaddr, 0, WLAN_ADDR_LEN);
		memcpy(&priv->pmib->dot11OperationEntry.hwaddr, wrqu->ap_addr.sa_data, MACADDRLEN);
		return 0;
	} else {
		//memset(&priv->pmib->dot11Bss.bssid, 0, WLAN_ADDR_LEN);
		//memcpy(&priv->pmib->dot11Bss.bssid, ap_addr->sa_data, MACADDRLEN);
		return -EOPNOTSUPP;
	}

}

int rtl_giwap(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	wrqu->ap_addr.sa_family = ARPHRD_ETHER;
	if (priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)
		memcpy(wrqu->ap_addr.sa_data, &priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
	else
		memcpy(wrqu->ap_addr.sa_data, &priv->pmib->dot11Bss.bssid, MACADDRLEN);
	
	return 0;
}

int rtl_iwaplist(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	struct sockaddr addr[IW_MAX_AP];
	struct iw_quality qual[IW_MAX_AP];
	int i;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif
	
	phead = &priv->asoc_list;
	
	i = 0;
	plist = phead->next;
	while (plist != phead && i < IW_MAX_AP) {
		pstat = list_entry(plist, struct stat_info, asoc_list);  
		addr[i].sa_family = ARPHRD_ETHER;
		memcpy(addr[i].sa_data, &pstat->hwaddr, WLAN_ADDR_LEN);

		qual[i].qual = pstat->rssi;
		//qual[i].level = pstat->sq;
		//qual[i].noise = 0
		qual[i].updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_INVALID | IW_QUAL_NOISE_INVALID;
		
		plist = plist->next;
		i++;
	}

	data->length = i;
	memcpy(extra, &addr, i*sizeof(addr[0]));
	data->flags = 1;		/* signal quality present (sort of) */
	memcpy(extra + i*sizeof(addr[0]), &qual, i*sizeof(qual[i]));

	return 0;
}

int rtl_siwessid(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *essid)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	char str[100];

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif
	sprintf(str, "ssid=%s", essid);
	return set_mib(priv, str);
		
}

int rtl_giwessid(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *essid)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif
	
	wrqu->essid.flags = 1;
	wrqu->essid.length = SSID_LEN;
	memcpy(essid, SSID, SSID_LEN);
	
	return 0;
}

int rtl_siwrate(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	UINT32 rate = wrqu->bitrate.value, fixed = wrqu->bitrate.fixed;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif
    
    //printk("rtl_siwrate::(rate = %d, fixed = %d)\n", rate, fixed);
    /* rate = -1 => auto rate
       rate = X, fixed = 1 => (fixed rate X)       
    */
    
    if (rate == -1)
    {
        //Auto Rate
        priv->pmib->dot11StationConfigEntry.autoRate = TRUE;
		priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
    }
    else
    {        
        if (fixed)
        {	
        	unsigned int txRate = rate / 10000000;
			int i, len;
			unsigned char *rateset, *p;
			rateset = AP_BSSRATE;
			len = AP_BSSRATE_LEN;

			for(i=0,p=rateset; i<len; i++,p++)
			{
				if (*p == 0x00)
					break;

				if (get_rate_from_bit_value(*p) == txRate) {
					priv->pmib->dot11StationConfigEntry.autoRate = FALSE;
					priv->pmib->dot11StationConfigEntry.fixedTxRate = *p;
					return 0;
				}
			}
			return -EOPNOTSUPP;
			
        }
        else
        {
            // TODO: rate = X, fixed = 0 => (rates <= X)
            return -EOPNOTSUPP;
        }
    }

}

int rtl_giwrate(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	unsigned int txRate;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	if (priv->pmib->dot11StationConfigEntry.autoRate)
		txRate = find_rate(priv, NULL, 1, 0);
	else
		txRate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate);

	//printk ("txRate = %d\n", txRate);
	wrqu->bitrate.value = txRate * 1000000;
    wrqu->bitrate.disabled = 0;

    return 0;
}

#define MAX_RTS_THRESHOLD 2347

int rtl_siwrts(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	u16 val;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	if (wrqu->rts.disabled)
		val = MAX_RTS_THRESHOLD;
	else if (wrqu->rts.value < 0 || wrqu->rts.value > MAX_RTS_THRESHOLD)
		return -EINVAL;
	else if (wrqu->rts.value == 0)
	    val = MAX_RTS_THRESHOLD;
	else
		val = wrqu->rts.value;
	
	if (val != RTSTHRSLD)
		RTSTHRSLD = val;

	return 0;
}

int rtl_giwrts(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	wrqu->rts.value = RTSTHRSLD;
	wrqu->rts.disabled = (wrqu->rts.value == MAX_RTS_THRESHOLD);
	wrqu->rts.fixed = 1;

	return 0;
}

int rtl_siwfrag(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	u16 val;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

	if (wrqu->frag.disabled)
		val = MAX_FRAG_THRESHOLD;
	else if (wrqu->frag.value >= MIN_FRAG_THRESHOLD || wrqu->frag.value <= MAX_FRAG_THRESHOLD)
        val = __cpu_to_le16(wrqu->frag.value & ~0x1); /* even numbers only */
	else if (wrqu->frag.value == 0)
	    val = MAX_FRAG_THRESHOLD;
	else
		return -EINVAL;

	FRAGTHRSLD = val;
	
	return 0;
}

int rtl_giwfrag(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif

		
	wrqu->frag.value = FRAGTHRSLD;
	wrqu->frag.disabled = (wrqu->frag.value == MAX_FRAG_THRESHOLD);
	wrqu->frag.fixed = 1;

	return 0;
}


int rtl_siwretry(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = wrqu->retry.value;

	return 0;
}


int rtl_giwretry(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	wrqu->retry.value = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	wrqu->retry.fixed = 0;	/* no auto select */
	wrqu->retry.flags |= (IW_RETRY_LIMIT |IW_RETRY_ON);
	wrqu->retry.disabled = 0;

	return 0;
}

int rtl_siwencode(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *keybuf)
{
	u32 key, ret = 0;
	u32 keyindex_provided;
	int i;
//	NDIS_802_11_WEP	 wep;	
//	NDIS_802_11_AUTHENTICATION_MODE authmode;

	struct iw_point *erq = &(wrqu->encoding);
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	
	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}
	
	printk("rtl_siwencode: erq->flags=0x%x erq->length=%d keybuf=%02x%02x%02x%02x%02x\n", erq->flags, erq->length, 
		keybuf[0],keybuf[1],keybuf[2],keybuf[3],keybuf[4]);	

	if (erq->flags & IW_ENCODE_DISABLED)
	{
		printk("rtl_siwencode: EncryptionDisabled\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0; //open system
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
		goto exit;
	}

	key = erq->flags & IW_ENCODE_INDEX;

	if (key) {
		if (key > 4)
			return -EINVAL;
		key--;
		keyindex_provided = 1;
	} 
	else
	{
		keyindex_provided = 0;
		key = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		printk("rtl_siwencode, key=%d\n", key);
	}

	//set authentication mode
	if (erq->flags & IW_ENCODE_OPEN)
	{
		printk("rtl_siwencode: IW_ENCODE_OPEN\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0; //open system
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	}
	else if (erq->flags & IW_ENCODE_RESTRICTED)
	{
		printk("rtl_siwencode: IW_ENCODE_RESTRICTED\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 1; //shared system
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	}
	else
	{
		printk("rtl_siwencode: IW_ENCODE_OPEN\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0; //open system
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	}

	priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = key;

	if (erq->length > 0) {
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = erq->length <= 5 ? _WEP_40_PRIVACY_ : _WEP_104_PRIVACY_;
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = erq->length <= 5 ? 8 : 16;
	} else {
	
		if (keyindex_provided == 1)// set key_id only, no given KeyMaterial(erq->length==0).
		{
			printk("rtl_siwencode: keyindex provided, keyid=%d, key_len=%d\n", key, erq->length);

			switch (priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen)
			{
				case 5:
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_40_PRIVACY_;
					break;
				case 13:
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_104_PRIVACY_;
					break;
				default:
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _NO_PRIVACY_;
					break;
			}

			goto exit;
		}
	}

	for (i=0; i<4; i++) {
		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[i].skey[0], keybuf, erq->length);
	}

exit:

	return ret;
}


int rtl_giwencode(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *key)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	struct iw_point *erq = &(wrqu->encoding);

	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef WIFI_HAPD
	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
#endif 
	
	if ((OPMODE & (WIFI_AP_STATE|WIFI_STATION_STATE|WIFI_ADHOC_STATE)) &&
		!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
				priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		int i;
		priv->pmib->dot11GroupKeysTable.dot11Privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			i = 13;

		// copy wep key
		erq->flags = (priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex + 1) & IW_ENCODE_INDEX;
		erq->length = i; 
		memcpy(key, &priv->pmib->dot11DefaultKeysTable.keytype[priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);

		if (priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm == 1)
			erq->flags |= IW_ENCODE_RESTRICTED;		/* XXX */
		else
			erq->flags |= IW_ENCODE_OPEN;		/* XXX */
		
		erq->flags |= IW_ENCODE_ENABLED;	/* XXX */
	}
	else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _NO_PRIVACY_) {
		erq->length = 0;
		erq->flags = IW_ENCODE_DISABLED;
	} 
	
	return 0;
}

int rtl_giwpower(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = netdev_priv(dev);

	wrqu->power.value = 0;
	wrqu->power.fixed = 0;	/* no auto select */
	wrqu->power.disabled = 1;

	return 0;
}


int rtl_siwscan(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	
	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}
	
	return rtl8192cd_ss_req(priv, (unsigned char *) &wrqu->data, 0);
}

int rtl_giwscan(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	RTL_PRIV *priv = (RTL_PRIV *)dev->priv;
	int i=0;
	char * current_ev = extra;
	char * previous_ev = extra;
	char * end_buf;
	//char * current_val;
#ifndef IWEVGENIE
	unsigned char idx;
#endif // IWEVGENIE //
	struct iw_event iwe;
	
	if (priv == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
	if (!netif_running(priv->dev))
	{
		//printk("\nFail: interface not opened\n");
		return -ENETDOWN;
	}

	if (priv->site_survey.count == 0)
	{
		wrqu->data.length = 0;
		return 0;
	}
	
#if WIRELESS_EXT >= 17
    if (wrqu->data.length > 0)
        end_buf = extra + wrqu->data.length;
    else
        end_buf = extra + IW_SCAN_MAX_DATA;
#else
    end_buf = extra + IW_SCAN_MAX_DATA;
#endif

	for (i = 0; i < priv->site_survey.count; i++) 
	{
		if (current_ev >= end_buf)
        {
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
        }
		
		//MAC address
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
				memcpy(iwe.u.ap_addr.sa_data, &priv->site_survey.bss[i].bssid, MACADDRLEN);

        previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev,end_buf, &iwe, IW_EV_ADDR_LEN);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif

		/* 
		Protocol:
			it will show scanned AP's WirelessMode .
			it might be
					802.11a
					802.11a/n
					802.11g/n
					802.11b/g/n
					802.11g
					802.11b/g
		*/
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWNAME;

	{
		struct bss_desc *pBss=&priv->site_survey.bss[i];
		//int rateCnt=0;

		if (pBss->network==WIRELESS_11B)
				strcpy(iwe.u.name, "802.11b");
		else if (pBss->network==WIRELESS_11G)
				strcpy(iwe.u.name, "802.11g");
		else if (pBss->network==(WIRELESS_11G|WIRELESS_11B))
				strcpy(iwe.u.name, "802.11b/g");
		else if (pBss->network==(WIRELESS_11N))
				strcpy(iwe.u.name, "802.11n");
		else if (pBss->network==(WIRELESS_11G|WIRELESS_11N))
				strcpy(iwe.u.name, "802.11g/n");
		else if (pBss->network==(WIRELESS_11G|WIRELESS_11B | WIRELESS_11N))
				strcpy(iwe.u.name, "802.11b/g/n");
		else if(pBss->network== WIRELESS_11A)
				strcpy(iwe.u.name, "802.11a");
		else
				strcpy(iwe.u.name, "---");
	}

		previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev,end_buf, &iwe, IW_EV_ADDR_LEN);
		if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
	   		return -E2BIG;
#else
			break;
#endif

		//ESSID	
		//================================
		memset(&iwe, 0, sizeof(iwe));

		//printk("ESSID %s %d\n",priv->site_survey.bss[i].ssid, priv->site_survey.bss[i].ssidlen);
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.length = priv->site_survey.bss[i].ssidlen;
		iwe.u.data.flags = 1;
 
        previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe, (char *)priv->site_survey.bss[i].ssid);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
		
		//Network Type 
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWMODE;
		if (priv->site_survey.bss[i].bsstype & WIFI_ADHOC_STATE)
		{
			iwe.u.mode = IW_MODE_ADHOC;
		}
		else if (priv->site_survey.bss[i].bsstype & WIFI_STATION_STATE)
		{
			iwe.u.mode = IW_MODE_INFRA;
		}
		else if (priv->site_survey.bss[i].bsstype & WIFI_AP_STATE)
		{
			iwe.u.mode = IW_MODE_MASTER;
		}
		else
		{
			iwe.u.mode = IW_MODE_AUTO;
		}
		iwe.len = IW_EV_UINT_LEN;

        previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev, end_buf, &iwe,  IW_EV_UINT_LEN);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif

		//Channel and Frequency
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWFREQ;
		{
			u8 ch = priv->site_survey.bss[i].channel;
			//u32	m = 0;
			//MAP_CHANNEL_ID_TO_KHZ(ch, m);
			//iwe.u.freq.m = m * 100;
			//iwe.u.freq.e = 1;
			iwe.u.freq.m = ch;
			iwe.u.freq.e = 0;
		iwe.u.freq.i = 0;
		previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev,end_buf, &iwe, IW_EV_FREQ_LEN);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
		}	    

        //Add quality statistics
        //================================
        memset(&iwe, 0, sizeof(iwe));
    	iwe.cmd = IWEVQUAL;
		iwe.u.qual.qual = priv->site_survey.bss[i].rssi;

		// not sure about signal level and noise level
		//iwe.u.qual.level = (u8) priv->site_survey.bss[i].sq;  
		//iwe.u.qual.noise = signal_todbm((u8)(100-priv->site_survey.bss[i].rssi)) -25;
		iwe.u.qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_INVALID | IW_QUAL_NOISE_INVALID;
		
    	current_ev = IWE_STREAM_ADD_EVENT(info, current_ev, end_buf, &iwe, IW_EV_QUAL_LEN);
	if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
#if 0
		//Encyption key
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWENCODE;
		if (priv->site_survey.bss[i].capability & 0x0010)
			iwe.u.data.flags =IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;

        previous_ev = current_ev;		
        current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf,&iwe, (u8 *)priv->pmib->dot11DefaultKeysTable.keytype[priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0]);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
#endif 
	}

	wrqu->data.length = current_ev - extra;
	return 0;
}

#endif

#ifdef CONFIG_RTL_COMAPI_CFGFILE

#define CFG_FILE_PATH			"/etc/Wireless/RTL8192CD.dat"

void del_mib_list(struct net_device *dev);
int CfgFileSetMib(struct net_device *dev, char *buf);

int CfgFileProc(struct net_device *dev)
{
	//RTL_PRIV *priv = dev->priv;
	unsigned char *mem_ptr;
	int ret = 0;
	
	printk("-------> Set MIB from " CFG_FILE_PATH "\n");	
	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("%s: not enough memory\n", __FUNCTION__);
		return -1;
	}
	
	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE);
	
	ret = CfgFileRead(dev, mem_ptr);

	if (ret < 0)
	{
		printk("%s: ReadCfgFile failed (%d)\n", __FUNCTION__, ret);
		goto proc_exit;
	}

	//printk("%s\n", mem_ptr);

	del_mib_list(dev);

	CfgFileSetMib(dev, mem_ptr);

	printk("<------- Set MIB from " CFG_FILE_PATH " Success\n");

proc_exit: 
	kfree(mem_ptr);
	return ret;
	
}

int CfgFileRead(struct net_device *dev, char *buf)
{
	//RTL_PRIV *priv = dev->priv;
	struct file *fp;
	mm_segment_t oldfs;
	//size_t len;

	//int read_bytes = 0;
	int ret = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	fp = filp_open(CFG_FILE_PATH, O_RDONLY, 0);
	if(IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		printk("Fail to open configuration file. (%d)\n", ret);
		goto err_exit;
	}
	
	if (!(fp->f_op && fp->f_op->read)) {	
		printk("Fail to support file ops: read\n");
		ret = -1;
		goto err_close;
	}	
	
	if ((ret = fp->f_op->read(fp, buf, MAX_CONFIG_FILE_SIZE, &fp->f_pos))< 0){
		printk("Fail to read file. (%d)\n", ret);
		goto err_close;
	}

err_close:
	filp_close(fp, NULL);
err_exit:	
	set_fs(oldfs);
	return ret;
	
}

static int rewrite_line (unsigned char **dst, unsigned char **src)
{
	int cnt=0;
	char *s = *src;
	char *d = *dst;
	char *loc=NULL, *vl_s=NULL, *vl_e=NULL;
	unsigned char quoted = 0;

	//printk("src = %s(%d)\n", *src, strlen(*src));
	loc=strchr(s, '"');
	if (loc) {
		unsigned int i = strlen(*src);
		vl_s=loc;
		while (i>0){
			char *t = (char *)((unsigned int)s+i-1);
			if (*t=='"' && t > vl_s ){
					vl_e = t;
					quoted = 1;
					break;
			}
			i--;
		}
	}
	
	while (*s) {
		u8 noop = 0;
		if (quoted ==1 && (vl_s < s && s < vl_e))
			noop = 1;
			
		if ( noop == 0 ) {
			if ((*s=='\r') || (*s=='\n') || (*s=='#') || (*s=='\0'))
				break;
			if ((*s == '\t')||(*s == ' ')||(*s == '"')){
				s++;
				continue;
			}
		}

		*d = *s;		
		s++;
		d++;
		cnt++;

	}
	*d = '\0';
	//printk("   dst = %s\n", *dst);
	return cnt;
}


int CfgFileSetMib(struct net_device *dev, char *buf)
{
	RTL_PRIV *priv = dev->priv;
	unsigned char *line_head, *next_head;
	unsigned char *cmd_buf, *mibstr, *valstr, *mibstart;
	//struct mib_cfg_func *tmp_mibcfg;
	int ret = 0;
#ifdef VENDOR_PARAM_COMPATIBLE
	int arg_num = sizeof(RTL_SUPPORT_MIBCFG)/sizeof(struct mib_cfg_func);
#endif //VENDOR_PARAM_COMPATIBLE

	if((cmd_buf = (unsigned char *)kmalloc(MAX_PARAM_BUF_SIZE, GFP_ATOMIC)) == NULL) {
		printk("%s(%d): not enough memory\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if((mibstr = (unsigned char *)kmalloc(20, GFP_ATOMIC)) == NULL) {
		printk("%s(%d): not enough memory\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if((valstr = (unsigned char *)kmalloc(MAX_PARAM_BUF_SIZE, GFP_ATOMIC)) == NULL) {
		printk("%s(%d): not enough memory\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	next_head = buf;
	
	do {
		char *loc;
		int len = 0, miblen = 0, vallen = 0;
		//int i=0;
		
		memset(cmd_buf, 0, MAX_PARAM_BUF_SIZE);
		memset(mibstr, 0, 20);
		memset(valstr, 0, MAX_PARAM_BUF_SIZE);
		
		line_head = next_head;
		next_head = get_line(&line_head);
		if (line_head == NULL)
			break;
		
		if (line_head[0] == '#')
			continue;
		
		len = rewrite_line(&cmd_buf, &line_head);
		//printk("%s (%d)\n", cmd_buf, len);
		
#ifdef VENDOR_PARAM_COMPATIBLE	
		/* To compatible with other vendor's parameters, each parameter must have its own process function - chris*/
		loc = strchr(mibstart, '=');
		miblen = (u32)loc - (u32)mibstart;
		vallen = len - miblen -1;
		if (vallen>0) {			
			for (i=0; i<arg_num; i++) {
				if (strcmp(mibstr, RTL_SUPPORT_MIBCFG[i].name) == 0) {
					if(!RTL_SUPPORT_MIBCFG[i].set_proc(priv, valstr)) {
						printk("CFGFILE set %s failed \n", mibstr);
						return -1;
					}
					break;
				}
			}
		}
#else
			
		//printk(">>>>>>>> cmd=%s , %s, %c \n",cmd_buf, dev->name, cmd_buf[strlen(dev->name)]);
		if (!strncmp(dev->name, cmd_buf, strlen(dev->name))&&(cmd_buf[strlen(dev->name)]!='-')) {
			mibstart = cmd_buf + strlen(dev->name)+1;
		} else
			continue;
		
		loc = strchr(mibstart, '=');
		miblen = (u32)loc - (u32)mibstart;
		vallen = len - (strlen(dev->name)+1) - (miblen+1);
		
		if (vallen>0) {
			
			ret = set_mib(priv, mibstart);
			if (ret < 0) {
				strncpy(mibstr, mibstart, miblen);
				strncpy(valstr, (char*)((u32)loc+1), vallen);
				//printk("(%s) = (%s) (%d)\n", mibstr, valstr, vallen);
				printk("CFGFILE set_mib \"%s\" failed \n", mibstart);
				//return -1;
			}

#endif // VENDOR_PARAM_COMPATIBLE
		}
		
	} while (1);

	kfree(cmd_buf);
	kfree(mibstr);
	kfree(valstr);
	
	return ret;
}

void del_mib_list(struct net_device *dev)
{
	RTL_PRIV *priv = dev->priv;
	
	if (priv->pmib) {
#ifdef WDS
		priv->pmib->dot11WdsInfo.wdsNum = 0;
#endif
		priv->pmib->dot11StationConfigEntry.dot11AclNum=0;
	}
	
}

#endif //CONFIG_RTL_COMAPI_CFGFILE



