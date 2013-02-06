/* 
   This file contains wireless extension handlers.

   This is part of rtl8180 OpenSource driver.
   Copyright (C) Andrea Merello 2004-2005  <andreamrl@tiscali.it> 
   Released under the terms of GPL (General Public Licence)
   
   Parts of this driver are based on the GPL part 
   of the official realtek driver.
   
   Parts of this driver are based on the rtl8180 driver skeleton 
   from Patric Schenke & Andres Salomon.

   Parts of this driver are based on the Intel Pro Wireless 2100 GPL driver.
   
   We want to tanks the Authors of those projects and the Ndiswrapper 
   project Authors.
*/



#include "r8187.h"
#include "r8180_hw.h"


#define RATE_COUNT 4
u32 rtl8180_rates[] = {1000000,2000000,5500000,11000000,
	6000000,9000000,12000000,18000000,24000000,36000000,48000000,54000000};
	

static int r8180_wx_get_freq(struct net_device *dev,
			     struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	return ieee80211_wx_get_freq(priv->ieee80211,a,wrqu,b);
}


#if 0

static int r8180_wx_set_beaconinterval(struct net_device *dev, struct iw_request_info *aa,
			  union iwreq_data *wrqu, char *b)
{
	int *parms = (int *)b;
	int bi = parms[0];
	
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	down(&priv->wx_sem);
	DMESG("setting beacon interval to %x",bi);
	
	priv->ieee80211->beacon_interval=bi;
	rtl8180_commit(dev);
	up(&priv->wx_sem);
		
	return 0;	
}


static int r8180_wx_set_forceassociate(struct net_device *dev, struct iw_request_info *aa,
			  union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv=ieee80211_priv(dev);	
	int *parms = (int *)extra;
	
	priv->ieee80211->force_associate = (parms[0] > 0);
	

	return 0;
}

#endif
static int r8180_wx_get_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	struct r8180_priv *priv=ieee80211_priv(dev);	

	return ieee80211_wx_get_mode(priv->ieee80211,a,wrqu,b);
}



static int r8180_wx_get_rate(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	return ieee80211_wx_get_rate(priv->ieee80211,info,wrqu,extra);
}



static int r8180_wx_set_rate(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	int ret;
	struct r8180_priv *priv = ieee80211_priv(dev);	
	
	down(&priv->wx_sem);

	ret = ieee80211_wx_set_rate(priv->ieee80211,info,wrqu,extra);
	
	up(&priv->wx_sem);
	
	return ret;
}

static int r8180_wx_set_rawtx(struct net_device *dev, 
			       struct iw_request_info *info, 
			       union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int ret;
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_set_rawtx(priv->ieee80211, info, wrqu, extra);
	
	up(&priv->wx_sem);
	
	return ret;
	 
}

static int r8180_wx_set_crcmon(struct net_device *dev, 
			       struct iw_request_info *info, 
			       union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int *parms = (int *)extra;
	int enable = (parms[0] > 0);
	short prev = priv->crcmon;

	down(&priv->wx_sem);
	
	if(enable) 
		priv->crcmon=1;
	else 
		priv->crcmon=0;

	DMESG("bad CRC in monitor mode are %s", 
	      priv->crcmon ? "accepted" : "rejected");

	if(prev != priv->crcmon && priv->up){
		rtl8180_down(dev);
		rtl8180_up(dev);
	}
	
	up(&priv->wx_sem);
	
	return 0;
}

static int r8180_wx_set_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int ret;
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_set_mode(priv->ieee80211,a,wrqu,b);
	
	rtl8187_set_rxconf(dev);
	
	up(&priv->wx_sem);
	return ret;
}


static int rtl8180_wx_get_range(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;
	struct r8180_priv *priv = ieee80211_priv(dev);
	u16 val;
	int i;

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
	if(priv->rf_set_sens != NULL)
		range->sensitivity = priv->max_sens;	/* signal level threshold range */
	
	range->max_qual.qual = 100;
	/* TODO: Find real max RSSI and stick here */
	range->max_qual.level = 0;
	range->max_qual.noise = -98;
	range->max_qual.updated = 7; /* Updated all three */

	range->avg_qual.qual = 92; /* > 8% missed beacons is 'bad' */
	/* TODO: Find real 'good' to 'bad' threshol value for RSSI */
	range->avg_qual.level = 20 + -98;
	range->avg_qual.noise = 0;
	range->avg_qual.updated = 7; /* Updated all three */

	range->num_bitrates = RATE_COUNT;
	
	for (i = 0; i < RATE_COUNT && i < IW_MAX_BITRATES; i++) {
		range->bitrate[i] = rtl8180_rates[i];
	}
	
	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;
	
	range->pm_capa = 0;

	range->we_version_compiled = WIRELESS_EXT;
	//range->we_version_source = 16;
	range->we_version_source = 15;

//	range->retry_capa;	/* What retry options are supported */
//	range->retry_flags;	/* How to decode max/min retry limit */
//	range->r_time_flags;	/* How to decode max/min retry life */
//	range->min_retry;	/* Minimal number of retries */
//	range->max_retry;	/* Maximal number of retries */
//	range->min_r_time;	/* Minimal retry lifetime */
//	range->max_r_time;	/* Maximal retry lifetime */

        range->num_channels = 14;

	for (i = 0, val = 0; i < 14; i++) {
		
		// Include only legal frequencies for some countries
		if ((priv->challow)[i+1]) {
		        range->freq[val].i = i + 1;
			range->freq[val].m = ieee80211_wlan_frequencies[i] * 100000;
			range->freq[val].e = 1;
			val++;
		} else {
			// FIXME: do we need to set anything for channels
			// we don't use ?
		}
		
		if (val == IW_MAX_FREQUENCIES)
		break;
	}

	range->num_frequency = val;
	return 0;
}


static int r8180_wx_set_scan(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int ret;
	
	if(!priv->up) return -1;
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_set_scan(priv->ieee80211,a,wrqu,b);
	
	up(&priv->wx_sem);
	return ret;
}


static int r8180_wx_get_scan(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{

	int ret;
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	if(!priv->up) return -1;
			
	down(&priv->wx_sem);

	ret = ieee80211_wx_get_scan(priv->ieee80211,a,wrqu,b);
		
	up(&priv->wx_sem);
	
	return ret;
}


static int r8180_wx_set_essid(struct net_device *dev, 
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *b)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	int ret;
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_set_essid(priv->ieee80211,a,wrqu,b);
	
	up(&priv->wx_sem);
	return ret;
}


static int r8180_wx_get_essid(struct net_device *dev, 
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *b)
{
	int ret;
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_get_essid(priv->ieee80211, a, wrqu, b);

	up(&priv->wx_sem);
	
	return ret;
}


static int r8180_wx_set_freq(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	int ret;
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_set_freq(priv->ieee80211, a, wrqu, b);
	
	up(&priv->wx_sem);
	return ret;
}

static int r8180_wx_get_name(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	return ieee80211_wx_get_name(priv->ieee80211, info, wrqu, extra);
}


static int r8180_wx_set_frag(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	if (wrqu->frag.disabled)
		priv->ieee80211->fts = DEFAULT_FRAG_THRESHOLD;
	else {
		if (wrqu->frag.value < MIN_FRAG_THRESHOLD ||
		    wrqu->frag.value > MAX_FRAG_THRESHOLD)
			return -EINVAL;
		
		priv->ieee80211->fts = wrqu->frag.value & ~0x1;
	}

	return 0;
}


static int r8180_wx_get_frag(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);

	wrqu->frag.value = priv->ieee80211->fts;
	wrqu->frag.fixed = 0;	/* no auto select */
	wrqu->frag.disabled = (wrqu->frag.value == DEFAULT_FRAG_THRESHOLD);

	return 0;
}


static int r8180_wx_set_wap(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *awrq,
			 char *extra)
{
	int ret;
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	down(&priv->wx_sem);
	
	ret = ieee80211_wx_set_wap(priv->ieee80211,info,awrq,extra);
	
	up(&priv->wx_sem);
	return ret;
	
}
	

static int r8180_wx_get_wap(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	return ieee80211_wx_get_wap(priv->ieee80211,info,wrqu,extra);
}


static int r8180_wx_get_enc(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *key)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	return ieee80211_wx_get_encode(priv->ieee80211, info, wrqu, key);
}

static int r8180_wx_set_enc(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *key)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int ret;
	
	down(&priv->wx_sem);
	
		DMESG("Setting SW wep key");
		ret = ieee80211_wx_set_encode(priv->ieee80211,info,wrqu,key);
			
	up(&priv->wx_sem);
	return ret;
}


static int r8180_wx_set_scan_type(struct net_device *dev, struct iw_request_info *aa, union
 iwreq_data *wrqu, char *p){
  
 	struct r8180_priv *priv = ieee80211_priv(dev);
	int *parms=(int*)p;
	int mode=parms[0];
	
	priv->ieee80211->active_scan = mode;
	
	return 1;
}



static int r8180_wx_set_retry(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	int err = 0;
	
	down(&priv->wx_sem);
	
	if (wrqu->retry.flags & IW_RETRY_LIFETIME || 
	    wrqu->retry.disabled){
		err = -EINVAL;
		goto exit;
	}
	if (!(wrqu->retry.flags & IW_RETRY_LIMIT)){
		err = -EINVAL;
		goto exit;
	}

	if(wrqu->retry.value > R8180_MAX_RETRY){
		err= -EINVAL;
		goto exit;
	}
	if (wrqu->retry.flags & IW_RETRY_MAX) {
		priv->retry_rts = wrqu->retry.value;
		DMESG("Setting retry for RTS/CTS data to %d", wrqu->retry.value);
	
	}else {
		priv->retry_data = wrqu->retry.value;
		DMESG("Setting retry for non RTS/CTS data to %d", wrqu->retry.value);
	}
	
	/* FIXME ! 
	 * We might try to write directly the TX config register
	 * or to restart just the (R)TX process.
	 * I'm unsure if whole reset is really needed
	 */

 	rtl8180_commit(dev);
	/*
	if(priv->up){
		rtl8180_rtx_disable(dev);
		rtl8180_rx_enable(dev);
		rtl8180_tx_enable(dev);
			
	}
	*/
exit:
	up(&priv->wx_sem);
	
	return err;
}

static int r8180_wx_get_retry(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	

	wrqu->retry.disabled = 0; /* can't be disabled */

	if ((wrqu->retry.flags & IW_RETRY_TYPE) == 
	    IW_RETRY_LIFETIME) 
		return -EINVAL;
	
	if (wrqu->retry.flags & IW_RETRY_MAX) {
		wrqu->retry.flags = IW_RETRY_LIMIT & IW_RETRY_MAX;
		wrqu->retry.value = priv->retry_rts;
	} else {
		wrqu->retry.flags = IW_RETRY_LIMIT & IW_RETRY_MIN;
		wrqu->retry.value = priv->retry_data;
	}
	//DMESG("returning %d",wrqu->retry.value);
	

	return 0;
}

static int r8180_wx_get_sens(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	struct r8180_priv *priv = ieee80211_priv(dev);
	if(priv->rf_set_sens == NULL) 
		return -1; /* we have not this support for this radio */
	wrqu->sens.value = priv->sens;
	return 0;
}


static int r8180_wx_set_sens(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	
	struct r8180_priv *priv = ieee80211_priv(dev);
	
	short err = 0;
	down(&priv->wx_sem);
	//DMESG("attempt to set sensivity to %ddb",wrqu->sens.value);
	if(priv->rf_set_sens == NULL) {
		err= -1; /* we have not this support for this radio */
		goto exit;
	}
	if(priv->rf_set_sens(dev, wrqu->sens.value) == 0)
		priv->sens = wrqu->sens.value;
	else
		err= -EINVAL;

exit:
	up(&priv->wx_sem);
	
	return err;
}


static int dummy(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu,char *b)
{
	return -1;
}


static iw_handler r8180_wx_handlers[] =
{
        NULL,                     /* SIOCSIWCOMMIT */
        r8180_wx_get_name,   	  /* SIOCGIWNAME */
        dummy,                    /* SIOCSIWNWID */
        dummy,                    /* SIOCGIWNWID */
        r8180_wx_set_freq,        /* SIOCSIWFREQ */
        r8180_wx_get_freq,        /* SIOCGIWFREQ */
        r8180_wx_set_mode,        /* SIOCSIWMODE */
        r8180_wx_get_mode,        /* SIOCGIWMODE */
        r8180_wx_set_sens,        /* SIOCSIWSENS */
        r8180_wx_get_sens,        /* SIOCGIWSENS */
        NULL,                     /* SIOCSIWRANGE */
        rtl8180_wx_get_range,	  /* SIOCGIWRANGE */
        NULL,                     /* SIOCSIWPRIV */
        NULL,                     /* SIOCGIWPRIV */
        NULL,                     /* SIOCSIWSTATS */
        NULL,                     /* SIOCGIWSTATS */
        dummy,                    /* SIOCSIWSPY */
        dummy,                    /* SIOCGIWSPY */
        NULL,                     /* SIOCGIWTHRSPY */
        NULL,                     /* SIOCWIWTHRSPY */
        r8180_wx_set_wap,      	  /* SIOCSIWAP */
        r8180_wx_get_wap,         /* SIOCGIWAP */
        NULL,                     /* -- hole -- */
        dummy,                     /* SIOCGIWAPLIST -- depricated */
        r8180_wx_set_scan,        /* SIOCSIWSCAN */
        r8180_wx_get_scan,        /* SIOCGIWSCAN */
        r8180_wx_set_essid,       /* SIOCSIWESSID */
        r8180_wx_get_essid,       /* SIOCGIWESSID */
        dummy,                    /* SIOCSIWNICKN */
        dummy,                    /* SIOCGIWNICKN */
        NULL,                     /* -- hole -- */
        NULL,                     /* -- hole -- */
        r8180_wx_set_rate,        /* SIOCSIWRATE */
        r8180_wx_get_rate,        /* SIOCGIWRATE */
        dummy,                    /* SIOCSIWRTS */
        dummy,                    /* SIOCGIWRTS */
        r8180_wx_set_frag,        /* SIOCSIWFRAG */
        r8180_wx_get_frag,        /* SIOCGIWFRAG */
        dummy,                    /* SIOCSIWTXPOW */
        dummy,                    /* SIOCGIWTXPOW */
        r8180_wx_set_retry,       /* SIOCSIWRETRY */
        r8180_wx_get_retry,       /* SIOCGIWRETRY */
        r8180_wx_set_enc,         /* SIOCSIWENCODE */
        r8180_wx_get_enc,         /* SIOCGIWENCODE */
        dummy,                    /* SIOCSIWPOWER */
        dummy,                    /* SIOCGIWPOWER */
}; 


static const struct iw_priv_args r8180_private_args[] = { 
	
	{
		SIOCIWFIRSTPRIV + 0x0, 
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "badcrc" 
	}, 
	
	{
		SIOCIWFIRSTPRIV + 0x1,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "activescan"
	
	},
	{
		SIOCIWFIRSTPRIV + 0x2, 
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "rawtx" 
	}
};


static iw_handler r8180_private_handler[] = {
//	r8180_wx_set_monitor,  /* SIOCIWFIRSTPRIV */
	r8180_wx_set_crcmon,   /*SIOCIWSECONDPRIV*/
//	r8180_wx_set_forceassociate,
//	r8180_wx_set_beaconinterval,
//	r8180_wx_set_monitor_type,
	r8180_wx_set_scan_type,
	r8180_wx_set_rawtx,
};

#if WIRELESS_EXT >= 17	
static struct iw_statistics *r8180_get_wireless_stats(struct net_device *dev)
{
       struct r8180_priv *priv = ieee80211_priv(dev);

       return &priv->wstats;
}
#endif


struct iw_handler_def  r8180_wx_handlers_def={
	.standard = r8180_wx_handlers,
	.num_standard = sizeof(r8180_wx_handlers) / sizeof(iw_handler),
	.private = r8180_private_handler,
	.num_private = sizeof(r8180_private_handler) / sizeof(iw_handler),
 	.num_private_args = sizeof(r8180_private_args) / sizeof(struct iw_priv_args),
#if WIRELESS_EXT >= 17	
	.get_wireless_stats = r8180_get_wireless_stats,
#endif
	.private_args = (struct iw_priv_args *)r8180_private_args,	
};
