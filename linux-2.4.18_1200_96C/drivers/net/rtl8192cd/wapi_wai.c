#define _WAPI_WAI_C_
#include "8192cd.h"
#include "wapi_wai.h"

#include "8192cd_cfg.h"
#include <linux/config.h>
#ifdef CONFIG_RTL_WAPI_SUPPORT

#include "wapiCrypto.h"
#ifdef __LINUX_2_6__
#ifdef CONFIG_RTL8672
#include "./romeperf.h"
#else
#include <common/rtl_types.h>
#endif
#include "8192cd_tx.h"
#else
#include "../rtl865x/rtl_types.h"
#include "8192cd_headers.h"
#endif
#include "8192cd_rx.h"



#define	WAPI_KM_OUI_LEN	3

/*int	wapi_lock_cnt=0;*/

/*struct timer_list	waiMCastKeyUpdateTimer;*/
static unsigned char		WAPI_KM_OUI[WAPI_KM_OUI_LEN] = {0x00, 0x14, 0x72};
static void wapiResendTimeout(unsigned long task_psta);
static struct sk_buff * wapiDefragement(struct sk_buff *pskb, struct stat_info *pstat, int waiOffset);
static int32 wapiFragementSend(struct sk_buff *pskb, struct rtl8192cd_priv *priv);
static int WapiRecvAccessAuthenticateRequest(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status);
static int wapiRecvUnicastKeyAgreementResponse(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status);
static int wapiRecvMulticastKeyResponse(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status);
static int wapiRecvKeyUpdateResponse(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status);
static int SecIsWAIPacket(struct sk_buff *pskb, int *waiOffset);
static inline void wapiSetWaiHeader(wapiWaiHeader *wai_hdr, uint8 subType);
static int	WapiSendActivateAuthenticationPacket(struct rtl8192cd_priv *priv, struct stat_info *pstat, int len, uint8 *data);

static int	wapiFreeAllSta(struct rtl8192cd_priv *priv, int forceFree)
{
	struct list_head		*plist;
	struct stat_info		*pstat1;
	unsigned int			index;

	for(index=0;index<NUM_STAT;index++)
	{
		plist = &priv->stat_hash[index];
		do
		{
			pstat1 = list_entry(plist, struct stat_info ,hash_list);
			if (pstat1 == NULL)
			{
				break;
			}
			plist = plist->next;
			if (forceFree==TRUE||pstat1->wapiInfo->wapiMCastKeyUpdateDone!=TRUE)
			{
			del_timer(&pstat1->wapiInfo->waiResendTimer);
			issue_deauth(priv, pstat1->hwaddr, _RSON_USK_HANDSHAKE_TIMEOUT_);
			}
		} while (plist!= &(priv->stat_hash[index]));
	}

	return WAPI_RETURN_SUCCESS;
}

static void wapiMCastUpdateKeyTimeout(unsigned long task_psta)
{
	struct rtl8192cd_priv 		*priv;
#if 0
	struct list_head		*plist;
	struct stat_info		*pstat1;
	unsigned int			index;
#endif

	priv = (struct rtl8192cd_priv *)task_psta;
	wapiAssert(priv);
	if (priv->wapiMCastKeyUpdate)
	{
		WAPI_LOCK(&priv->pshare->lock);
		if (priv->wapiMCastKeyUpdateAllDone)
		{
			del_timer(&priv->waiMCastKeyUpdateTimer);
			init_timer(&priv->waiMCastKeyUpdateTimer);
			priv->waiMCastKeyUpdateTimer.data = (unsigned long)priv;
			priv->waiMCastKeyUpdateTimer.function = wapiMCastUpdateKeyTimeout;
			if (priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_time_update||
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_all_update)
			{
				mod_timer(&priv->waiMCastKeyUpdateTimer, jiffies + priv->pmib->wapiInfo.wapiUpdateMCastKeyTimeout*HZ);
			}
			WAPI_LOCK(&priv->pshare->lock);
			priv->wapiMCastKeyUpdate = 0;
			WAPI_UNLOCK(&priv->pshare->lock);
		}
		else
		{
			/* during update time out */
			del_timer(&priv->waiMCastKeyUpdateTimer);
/*
			init_timer(&waiMCastKeyUpdateTimer);
			waiMCastKeyUpdateTimer.data = (unsigned long)priv;
			waiMCastKeyUpdateTimer.function = wapiMCastUpdateKeyTimeout;
*/
			priv->wapiMCastKeyUpdateAllDone = 0;
			priv->wapiMCastKeyId = 0;
			priv->wapiMCastKeyUpdate = 0;
			wapiFreeAllSta(priv, FALSE);
		}
		WAPI_UNLOCK(&priv->pshare->lock);
	}
	else
	{
		/* update time out */
		wapiUpdateMSK(priv, NULL);
	}
}

static void wapiUCastUpdateKeyTimeout(unsigned long task_psta)
{
	struct stat_info 		*pstat;
	struct rtl8192cd_priv 		*priv;
	wapiStaInfo			*wapiInfo;

	pstat = (struct stat_info *)task_psta;
	wapiAssert(pstat);
	wapiInfo = pstat->wapiInfo;
	wapiAssert(wapiInfo);
	priv = wapiInfo->priv;
	wapiAssert(priv);

	if (wapiInfo->wapiUCastKeyUpdate)
	{
		WAPI_LOCK(&wapiInfo->lock);
		/* during update time out */
		del_timer(&wapiInfo->waiUCastKeyUpdateTimer);
		del_timer(&wapiInfo->waiResendTimer);

		wapiAssert(wapiInfo->wapiUCastKeyUpdate);
		WAPI_UNLOCK(&wapiInfo->lock);
		issue_deauth(priv, pstat->hwaddr, _RSON_USK_HANDSHAKE_TIMEOUT_);
	}
	else
	{
		/* update time out */
		wapiUpdateUSK(priv, pstat);
	}
}

static void wapiResendTimeout(unsigned long task_psta)
{
	struct stat_info 	*pstat;
	struct rtl8192cd_priv *priv;
	wapiStaInfo		*wapiInfo;

	pstat = (struct stat_info *)task_psta;

	wapiAssert(pstat);
	wapiInfo = pstat->wapiInfo;
	wapiAssert(wapiInfo);
	priv = wapiInfo->priv;
	wapiAssert(priv);

	wapiInfo->wapiRetry++;
	if (wapiInfo->wapiRetry>WAPI_RETRY_COUNT)
	{
		WAPI_LOCK(&wapiInfo->lock);
		wapiInfo->wapiState = ST_WAPI_AE_IDLE;
		del_timer(&wapiInfo->waiResendTimer);
		WAPI_UNLOCK(&wapiInfo->lock);
		issue_deauth(priv, pstat->hwaddr, _RSON_USK_HANDSHAKE_TIMEOUT_);
		return;
	}

	switch (wapiInfo->wapiState)
	{
		case ST_WAPI_AE_IDLE:
			{
				if (wapiInfo->wapiType==wapiTypeCert)
				{
					wapiReqActiveCA(pstat);
				}
				break;
			}
		case ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT:
			{
				if (wapiInfo->wapiType==wapiTypeCert)
				{
					WapiSendActivateAuthenticationPacket(priv, pstat, wapiInfo->waiCertCachedDataLen, wapiInfo->waiCertCachedData);
				}
				break;
			}
		case ST_WAPI_AE_ACCESS_AUTHENTICATE_REQ_RCVD:
			{
				if (wapiInfo->wapiType==wapiTypeCert)
				{
					WAPI_LOCK(&wapiInfo->priv->pshare->lock);
					/* update timer	*/
					mod_timer(&wapiInfo->waiResendTimer,jiffies + WAPI_CERT_REQ_TIMEOUT);
					WAPI_UNLOCK(&wapiInfo->priv->pshare->lock);

					#if 0
					para = (wapiCAAppPara*)wapiInfo->waiCertCachedData;
					memset(data, 0, WAPI_CERT_MAX_LEN);
					para->type = WAPI_IOCTL_TYPE_CA_AUTH;
					para->ptr = (void*)pstat;
					memcpy(para->data, wapiInfo->waiCertCachedData, wapiInfo->waiCertCachedDataLen);
					#endif
					DOT11_EnQueue((unsigned long)wapiInfo->priv, wapiInfo->priv->wapiEvent_queue, wapiInfo->waiCertCachedData, wapiInfo->waiCertCachedDataLen);
					wapi_event_indicate(wapiInfo->priv);
				}
				break;
			}
		case ST_WAPI_AE_USK_AGGREMENT_REQ_SNT:
			{
				if (wapiInfo->wapiType==wapiTypeCert)
				{
					wapiAssert(wapiInfo->waiCertCachedData);
					wapiAssert(wapiInfo->waiCertCachedDataLen>0);
					WapiSendAuthenticationRspPacket(priv, pstat, wapiInfo->waiCertCachedDataLen, wapiInfo->waiCertCachedData);
				}

				wapiSendUnicastKeyAgrementRequeset(priv, pstat);
				break;
			}
		case ST_WAPI_AE_ACCESS_CERTIFICATE_REQ_SNT:
			{
				break;
			}
		case ST_WAPI_AE_MSK_NOTIFICATION_SNT:
			{
				wapiSendUnicastKeyAgrementConfirm(priv, pstat);
				wapiSendMulticastKeyNotification(priv, pstat);
				break;
			}
		case ST_WAPI_AE_MSKA_ESTABLISH:
			{
				if (priv->wapiMCastKeyUpdate)
				{
					wapiSendMulticastKeyNotification(priv, pstat);
				}
				else if (wapiInfo->wapiUCastKeyUpdate)
				{
					wapiSendUnicastKeyAgrementRequeset(priv, pstat);
				}
				else
				{
					wapiSendUnicastKeyAgrementConfirm(wapiInfo->priv, pstat);
					{
						WAPI_LOCK(&wapiInfo->lock);
						del_timer(&wapiInfo->waiResendTimer);
						init_timer(&wapiInfo->waiResendTimer);
						wapiInfo->waiResendTimer.data = (unsigned long)pstat;
						wapiInfo->waiResendTimer.function = wapiResendTimeout;
						WAPI_UNLOCK(&wapiInfo->lock);
					}
				}
				break;
			}

		case ST_WAPI_AE_MSK_RSP_RCVD:
		case ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD:				
		case ST_WAPI_AE_BKSA_ESTABLISH:
		default:
			break;
	 }
}

void wapiInit(struct rtl8192cd_priv *priv)
{
	int	i;
	char *		MskLabelSrc="multicast or station key expansion for station unicast and multicast and broadcast";
	UCHAR		derivedKey[32];

	/*	set multicast key	*/
	KD_hmac_sha256(priv->wapiNMK, WAPI_KEY_LEN, 
		MskLabelSrc, strlen(MskLabelSrc), 
		derivedKey, WAPI_KEY_LEN<<1);
	memcpy(&priv->wapiMCastKey[priv->wapiMCastKeyId].dataKey, 
		derivedKey, WAPI_KEY_LEN);
	memcpy(&priv->wapiMCastKey[priv->wapiMCastKeyId].micKey, 
		&derivedKey[WAPI_KEY_LEN], WAPI_KEY_LEN);
//	priv->wapiMCastKeyId = 0;
//	priv->wapiMCastKeyUpdate = 0;
	/*	set pn	*/
#ifdef LITTLE_ENDIAN
	for (i=0;i<WAPI_PN_LEN;i+=2)
	{
		priv->txMCast[i] = 0x5c;
		priv->txMCast[i] = 0x36;
	}
#else
	for (i=WAPI_PN_LEN;i>0;i-=2)
	{
		priv->txMCast[i] = 0x5c;
		priv->txMCast[i] = 0x36;
	}
#endif
	memcpy(priv->rxMCast, priv->txMCast, WAPI_PN_LEN);

	{
		del_timer(&priv->waiMCastKeyUpdateTimer);
		init_timer(&priv->waiMCastKeyUpdateTimer);
		priv->waiMCastKeyUpdateTimer.data = (unsigned long)priv;
		priv->waiMCastKeyUpdateTimer.function = wapiMCastUpdateKeyTimeout;
		if (priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_time_update||
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_all_update)
		{
			mod_timer(&priv->waiMCastKeyUpdateTimer, jiffies + priv->pmib->wapiInfo.wapiUpdateMCastKeyTimeout*HZ);
		}
	}
	
	/* always set the number */
	priv->wapiMCastKeyUpdateCnt = priv->pmib->wapiInfo.wapiUpdateMCastKeyPktNum;
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
	init_SMS4_CK_Sbox();
#endif
	
}
void wapiExit(struct rtl8192cd_priv *priv)
{
	del_timer(&priv->waiMCastKeyUpdateTimer);
/*
	init_timer(&priv->waiMCastKeyUpdateTimer);
	priv->waiMCastKeyUpdateTimer.data = (unsigned long)priv;
	priv->waiMCastKeyUpdateTimer.function = wapiMCastUpdateKeyTimeout;
*/
	
}

void
wapiStationInit(struct stat_info *pstat)
{
	wapiStaInfo	*wapiInfo;
	int			idx, idx2;
	
	wapiInfo = pstat->wapiInfo;

	init_timer(&wapiInfo->waiResendTimer);
	wapiInfo->waiResendTimer.data = (unsigned long)pstat;
	wapiInfo->waiResendTimer.function = wapiResendTimeout;

	init_timer(&wapiInfo->waiUCastKeyUpdateTimer);
	wapiInfo->waiUCastKeyUpdateTimer.data = (unsigned long)pstat;
	wapiInfo->waiUCastKeyUpdateTimer.function = wapiUCastUpdateKeyTimeout;

	/*	set rx/tx seqnumber	*/
	wapiInfo->waiRxSeq = 0;
	wapiInfo->waiTxSeq = 0;

	/*	set pn	*/
#ifdef LITTLE_ENDIAN
	for (idx=0;idx<WAPI_PN_LEN;idx+=2)
	{
		wapiInfo->wapiPN.rxUCast[0][idx] = 0x5c;
		wapiInfo->wapiPN.rxUCast[0][idx+1] = 0x36;
	}
#else
	for (idx=WAPI_PN_LEN;idx>0;idx-=2)
	{
		wapiInfo->wapiPN.rxUCast[0][idx-1] = 0x5c;
		wapiInfo->wapiPN.rxUCast[0][idx-2] = 0x36;
	}
#endif
	for(idx2=1;idx2<RX_QUEUE_NUM;idx2++)
	{
		memcpy(&wapiInfo->wapiPN.rxUCast[idx2][0], wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
	}
	memcpy(wapiInfo->wapiPN.txUCast, wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
	memcpy(wapiInfo->priv->txMCast, wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
	memcpy(wapiInfo->priv->rxMCast, wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
	memcpy(wapiInfo->priv->keyNotify, wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
	for(idx2=0;idx2<RX_QUEUE_NUM;idx2++)
	{
		wapiInfo->wapiPN.rxUCast[idx2][idx] = 0x37;
	}
	wapiInfo->wapiPN.txUCast[idx] = 0x37;

	wapiInfo->wapiUCastKeyId = 0;
	wapiInfo->wapiUCastKeyUpdate = 0;

#if 0
	memDump(wapiInfo->priv->wapiNMK, 16, "NMK");
	memDump(wapiInfo->priv->wapiMCastKey[0].micKey, 16, "MIK");
	memDump(wapiInfo->priv->wapiMCastKey[0].dataKey, 16, "DAK");
#endif
	wapiInfo->wapiUCastRxEnable = 0;
	wapiInfo->wapiUCastTxEnable = 0;
	wapiInfo->wapiMCastEnable = 0;
	
	/*	set frag info	*/
	wapiInfo->wapiRxFragSeq = 0;
	wapiInfo->wapiRxFragPskb = NULL;

	/*	set status		*/
	wapiInfo->wapiState=ST_WAPI_AE_IDLE;

	wapiInfo->waiCertCachedData = NULL;
	wapiInfo->waiCertCachedDataLen = 0;
	
	WAPI_LOCK_INIT(&wapiInfo->lock);
}

void
wapiSetIE(struct rtl8192cd_priv	*priv)
{
	unsigned short	protocolVer = cpu_to_le16(0x1);		/*	little endian 1	*/
	unsigned short	akmCnt = cpu_to_le16(0x1);			/*	little endian 1	*/
	unsigned short	suiteCnt = cpu_to_le16(0x1);		/*	little endian 1	*/
	unsigned short	capability = 0;
	unsigned char		*buf;
	
	wapiAssert(priv->wapiCachedBuf);

	priv->wapiCachedLen = 0;
	buf = priv->wapiCachedBuf;
	/*	set protocol version	*/
	memcpy(buf, &protocolVer, 2);
	buf += 2;
	priv->wapiCachedLen += 2;
	
	/*	set akm	*/
	memcpy(buf, &akmCnt, 2);
	buf += 2;
	priv->wapiCachedLen += 2;

	memcpy(buf, WAPI_KM_OUI, WAPI_KM_OUI_LEN);
	buf[3] = priv->pmib->wapiInfo.wapiType;
	buf += 4;
	priv->wapiCachedLen += 4;

#if 0		
	if (priv->pmib->wapiInfo.wapiType ==wapiTypePSK)
	{
		/*	psk	*/
		memcpy(buf, WAPI_KM_OUI, WAPI_KM_OUI_LEN);
		buf[3] = 0x2;
		buf += 4;
		priv->wapiCachedLen += 4;
	}
	else
	{
		/*	cert	*/
		wapiAssert(priv->pmib->wapiInfo.wapiType ==wapiTypeCert);
		memcpy(buf, WAPI_KM_OUI, WAPI_KM_OUI_LEN);
		buf[3] = 0x1;
		buf += 4;
		priv->wapiCachedLen += 4;
	}
#endif

	/*	usk	*/
	memcpy(buf, &suiteCnt, 2);
	memcpy(&buf[2], WAPI_KM_OUI, WAPI_KM_OUI_LEN);
#if defined(WAPI_SUPPORT_MULTI_ENCRYPT)
	buf[5] = priv->pmib->wapiInfo.wapiUCastEncodeType;
#else
	buf[5] = 1;	/*	wapiUCastEncodeType	*/
#endif
	buf += 6;
	priv->wapiCachedLen += 6;

	/*	msk	*/
	memcpy(buf, WAPI_KM_OUI, WAPI_KM_OUI_LEN);
#if defined(WAPI_SUPPORT_MULTI_ENCRYPT)
	buf[5] = priv->pmib->wapiInfo.wapiMCastEncodeType;
#else
	buf[3] = 1;	/*	wapiMCastEncodeType	*/
#endif
	buf += 4;
	priv->wapiCachedLen += 4;

	/*	Capbility	*/
	memcpy(buf, &capability, 2);
	buf += 2;
	priv->wapiCachedLen += 2;	
}

void wapiReleaseFragementQueue(wapiStaInfo *wapiInfo)
{
	struct sk_buff *tmp, *pskb;

	if (wapiInfo->wapiRxFragPskb==NULL)
		return;
	
	pskb = wapiInfo->wapiRxFragPskb;
	do {
		tmp = pskb;
		pskb = pskb->next;
		tmp->next = tmp->prev = NULL;
		rtl_kfree_skb(wapiInfo->priv, tmp, _SKB_RX_);
	} while(pskb!=wapiInfo->wapiRxFragPskb);

	wapiInfo->wapiRxFragPskb = NULL;
	/*	the following to filed should be re-init when new fragment pkt rcved	*/
#if 1
	wapiInfo->wapiRxFragSeq = 0;
	wapiInfo->wapiRxFragLen = 0;
#endif
}

static struct sk_buff * wapiDefragement(struct sk_buff *pskb, struct stat_info *pstat, int waiOffset)
{
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader		*waihdr;
	struct sk_buff		*total, *tmpSkb;
	
	waihdr = (wapiWaiHeader*)(pskb->data+waiOffset);

	wapiInfo = pstat->wapiInfo;
#if 0
	/*	All the fragement pkt has the same seq	
	*	Since we add seq number when do 
	*	sanity check, it should be reduce here
	*/
	if (waihdr->fragmentNum!=0)
		wapiInfo->waiRxSeq--;
#endif
	WAPI_LOCK(&wapiInfo->lock);
	if (waihdr->fragmentNum!=wapiInfo->wapiRxFragSeq)
	{
		wapiReleaseFragementQueue(wapiInfo);
		WAPI_UNLOCK(&wapiInfo->lock);
		return NULL;
	}
	else
		wapiInfo->wapiRxFragSeq++;

	if ((waihdr->flags&WAI_HEADER_MF)==0)
	{
		/* all frag pkt received	*/
		wapiAssert(wapiInfo->wapiRxFragLen<MAXDATALEN);
		total = rtl_dev_alloc_skb(wapiInfo->priv, wapiInfo->wapiRxFragLen, _SKB_RX_, TRUE);
		if (total == NULL)
		{
			wapiInfo->waiRxSeq++;
			wapiReleaseFragementQueue(wapiInfo);
			rtl_kfree_skb(wapiInfo->priv, pskb, _SKB_RX_);
			WAPI_UNLOCK(&wapiInfo->lock);
			return NULL;
		}

		tmpSkb = wapiInfo->wapiRxFragPskb;
		wapiAssert(tmpSkb);
		wapiAssert(total->len==0);
		do{
			memcpy(total->data+total->len, tmpSkb->data, tmpSkb->len);
			skb_put(total, tmpSkb->len);
			
			tmpSkb = tmpSkb->next;
		}while(tmpSkb!=wapiInfo->wapiRxFragPskb);

		memcpy(total->data+total->len, pskb->data+(WAI_HEADER_LEN+waiOffset), pskb->len-(WAI_HEADER_LEN+waiOffset));
		skb_put(total, pskb->len-(WAI_HEADER_LEN+waiOffset));

		wapiAssert(total->len==(wapiInfo->wapiRxFragLen+pskb->len-(WAI_HEADER_LEN+waiOffset)));
		total->dev = pskb->dev;
		wapiReleaseFragementQueue(wapiInfo);
		rtl_kfree_skb(wapiInfo->priv, pskb, _SKB_RX_);

		WAPI_UNLOCK(&wapiInfo->lock);
		return total;
	}
	else
	{
		tmpSkb = wapiInfo->wapiRxFragPskb;
		if (tmpSkb)
		{
			tmpSkb->next->prev = pskb;
			pskb->next = tmpSkb->next;
			tmpSkb->next = pskb;
			pskb->prev = tmpSkb;

#if 1			
			/* remove wai header length	*/
			pskb->data += (WAI_HEADER_LEN+waiOffset);
			pskb->len -= (WAI_HEADER_LEN+waiOffset);
#endif
			wapiInfo->wapiRxFragLen += pskb->len;
		}
		else
		{
			/*	the first skb	*/
			wapiAssert(waihdr->fragmentNum==0);
			wapiAssert(wapiInfo->wapiRxFragPskb==NULL);
			wapiAssert((pskb->next==pskb->prev)&&(pskb->next==NULL));
			wapiInfo->wapiRxFragPskb = pskb;
			pskb->next = pskb->prev = pskb;

			wapiInfo->wapiRxFragLen = pskb->len;
		}
		
		/*	queue it 	*/
		WAPI_UNLOCK(&wapiInfo->lock);
		return NULL;
	}
}

static int32 wapiFragementSend(struct sk_buff *pskb, struct rtl8192cd_priv *priv)
{
	int	fragthreshold, fraglen, fragnum, fragIdx;
	int	len, datalen;
	struct sk_buff *fragSkb;
	uint8		*data;
	wapiWaiHeader	*waiHeader;
		
	fragthreshold = priv->dev->mtu - (priv->dev->mtu%8);		/* 8 bytes align	*/
	fraglen = fragthreshold - WAPI_WAI_HEADER_PADDING;
	wapiAssert(fraglen>(WAI_HEADER_LEN+sizeof(struct ethhdr)));
	fragnum = (pskb->len-WAI_HEADER_LEN-sizeof(struct ethhdr))/(fraglen-WAI_HEADER_LEN-sizeof(struct ethhdr));
	if ( (pskb->len-WAI_HEADER_LEN-sizeof(struct ethhdr))%(fraglen-WAI_HEADER_LEN-sizeof(struct ethhdr)) )
		fragnum++;
	/*	store waiHeader	*/
	data = pskb->data+WAI_HEADER_LEN+sizeof(struct ethhdr);
	len = pskb->len-WAI_HEADER_LEN-sizeof(struct ethhdr);

	for(fragIdx=0;fragIdx<fragnum;fragIdx++)
	{
		fragSkb = rtl_dev_alloc_skb(priv, 2400, _SKB_TX_, TRUE);
		if (fragSkb==NULL)
			return FAILED;

		skb_reserve(fragSkb, WAPI_WAI_HEADER_PADDING);
		wapiAssert(fragSkb->len == 0);
		wapiAssert(fragSkb->data == fragSkb->tail);

		skb_put(fragSkb, len>(fraglen-WAI_HEADER_LEN-sizeof(struct ethhdr))?fraglen:len+WAI_HEADER_LEN+sizeof(struct ethhdr));
		datalen = (fragSkb->len-WAI_HEADER_LEN-sizeof(struct ethhdr));
		waiHeader = (wapiWaiHeader*)(fragSkb->data+sizeof(struct ethhdr));
		memcpy(fragSkb->data, pskb->data, WAI_HEADER_LEN+sizeof(struct ethhdr));		/* ether & wai header */
		memcpy(fragSkb->data+WAI_HEADER_LEN+sizeof(struct ethhdr), data, datalen);	/* data */
		
		data += datalen;
		len -= datalen;
		fragSkb->dev = pskb->dev;
		fragSkb->protocol = htonl(ETH_P_WAPI);
		waiHeader->fragmentNum = fragIdx;
		waiHeader->flags = (len==0?0:WAI_HEADER_MF);
		waiHeader->length = datalen+WAI_HEADER_LEN;
		if (rtl8192cd_start_xmit(fragSkb, priv->dev))
		{
			rtl_kfree_skb(priv, fragSkb, _SKB_TX_);
			return FAILED;
		}
	}
	/*	do wapi fragement	*/

	wapiAssert(len==0);
	rtl_kfree_skb(priv, pskb, _SKB_TX_);
	return SUCCESS;
}


/*  PN1 > PN2, return WAPI_RETURN_SUCCESS,
 *  else return WAPI_RETURN_FAILED.
 */
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__MIPS16 
__IRAM_WLAN_HI 
int32 WapiComparePN(uint8 *PN1, uint8 *PN2)
#else
int32 WapiComparePN(uint8 *PN1, uint8 *PN2)
#endif
{
	int8 i;

	/* overflow case	*/
	if ((PN2[15] - PN1[15]) & 0x80)
	    return WAPI_RETURN_SUCCESS;

	for (i=16; i>0; i--)
	{
		if(PN1[i-1] == PN2[i-1])
		    	continue;
		else if(PN1[i-1] > PN2[i-1])
			return WAPI_RETURN_SUCCESS;
		else
			return WAPI_RETURN_FAILED;			
	}

	return WAPI_RETURN_FAILED;
}


/* AddCount: 1 or 2. 
 *  If overflow, return WAPI_RETURN_SUCCESS,
 *  else return WAPI_RETURN_FAILED.
 */
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__MIPS16 
__IRAM_WLAN_HI 
int32 WapiIncreasePN(uint8 *PN, uint8 AddCount)
#else
int32 WapiIncreasePN(uint8 *PN, uint8 AddCount)
#endif
{
    uint8  i;

    for (i=0; i<16; i++)
    {
        if (PN[i] + AddCount <= 0xff)
        {
            PN[i] += AddCount;
            return WAPI_RETURN_FAILED;
        }
        else
        {
            PN[i] += AddCount;
            AddCount = 1;
        }
    }

    return WAPI_RETURN_SUCCESS;
}

int DOT11_Process_WAPI_Info(struct rtl8192cd_priv *priv, uint8 *data, int32 len)
{
	struct stat_info	*pstat;
	wapiCAAppPara	*caPara;
	wapiWaiCertAuthRspPkt	*rsp;
	unsigned long		flags;

	caPara = (wapiCAAppPara*)data;
	pstat = (struct stat_info*)caPara->ptr;
	wapiAssert(priv);
	wapiAssert(pstat);
	wapiAssert(pstat->wapiInfo);
	//Patch: several times into this function may lead to "pstat->wapiInfo==NULL"
	if(pstat->wapiInfo==NULL)
		return WAPI_RETURN_FAILED;
	//End patch
	
	wapiAssert(pstat->wapiInfo->priv==priv);

	if (memcmp(priv->dev->name, caPara->name, strlen(caPara->name)))
		return WAPI_RETURN_FAILED;
	
	switch (caPara->type)
	{
		case WAPI_IOCTL_TYPE_ACTIVEAUTH:	/* active auth	*/
			if (ST_WAPI_AE_ACTIVE_AUTHENTICATION_REQ==pstat->wapiInfo->wapiState)
			WapiSendActivateAuthenticationPacket(priv, pstat, len-sizeof(wapiCAAppPara), caPara->data);
			break;
		case WAPI_IOCTL_TYPE_SETBK:	/* set bk	*/
			if (ST_WAPI_AE_ACCESS_AUTHENTICATE_REQ_RCVD==pstat->wapiInfo->wapiState)
			{
				SAVE_INT_AND_CLI(flags);
				priv->wapiCachedBuf = caPara->data;
				wapiSetBK(pstat);
				priv->wapiCachedBuf = NULL;
				RESTORE_INT(flags);
			}
			break;
		case WAPI_IOCTL_TYPE_AUTHRSP:	/* auth response */
			if (ST_WAPI_AE_BKSA_ESTABLISH==pstat->wapiInfo->wapiState)
			{
				rsp = (wapiWaiCertAuthRspPkt *)(caPara->data);
				WapiSendAuthenticationRspPacket(priv, pstat, len-sizeof(wapiCAAppPara), caPara->data);
				if (rsp->CAResult==0)
				{
					wapiSendUnicastKeyAgrementRequeset(priv, pstat);
				}
				else
				{
					wapiAssert(0);
					//delay_ms(100);
					WAPI_LOCK(&pstat->wapiInfo->lock);
					pstat->wapiInfo->wapiState = ST_WAPI_AE_IDLE;
					del_timer(&pstat->wapiInfo->waiResendTimer);
					WAPI_UNLOCK(&pstat->wapiInfo->lock);
					issue_deauth(priv, pstat->hwaddr, _RSON_PMK_NOT_AVAILABLE_);

				}
			}
			break;
		default:
			break;
	}

	return WAPI_RETURN_SUCCESS;
}

static int WapiRecvAccessAuthenticateRequest(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status)
{
	wapiWaiCertAuthReqPkt	*waiCertReq;
	wapiStaInfo			*wapiInfo;
	wapiTLV				*rxData, *lastData;
	wapiTLV1				*rxData1, *lastData1;
	wapiCAAppPara		*para;

	wapiInfo = pstat->wapiInfo;

	/*	deliver the pkt to appliation	*/
	if (wapiInfo->wapiState==ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT)
	{
		wapiAssert(waihdr!=NULL);
		{
			if (waihdr->subType!=WAI_SUBTYPE_AUTH_REQ)
				return WAPI_RETURN_FAILED;
			waiCertReq = (wapiWaiCertAuthReqPkt*)(((uint8*)waihdr)+(sizeof(wapiWaiHeader)));

			if ((waiCertReq->updateBK==1||waiCertReq->AEAuthReq==0)
				||waiCertReq->preAuth==1)
			{
				wapiAssert(0);
				return WAPI_RETURN_FAILED;
			}

			/*	bypass ASUE keydata	*/
				rxData = (wapiTLV*)(waiCertReq->data+(waiCertReq->data[0])+1);
			/*	bypass AE ID	*/
			rxData = (wapiTLV*)(&rxData->data[rxData->len]);
			/*	bypass ASUE CA	*/
				rxData1 = (wapiTLV1*)(&rxData->data[rxData->len]);

				lastData = (wapiTLV*)(wapiInfo->waiCertCachedData);	/* it's currently cached the active info from AP */
			/*	bypass ASU ID	*/
			lastData = (wapiTLV*)(&lastData->data[lastData->len]);
			/*	bypass AE CA	*/
				lastData1 = (wapiTLV1*)(&lastData->data[lastData->len]);

				if (rxData1->len!=lastData1->len ||
					memcmp(rxData1, lastData1, sizeof(wapiTLV1)+rxData1->len))
			{
				wapiAssert(0);
				return WAPI_RETURN_FAILED;
			}
		}

		WAPI_LOCK(&wapiInfo->priv->pshare->lock);
		wapiInfo->wapiState = ST_WAPI_AE_ACCESS_AUTHENTICATE_REQ_RCVD;
		wapiInfo->wapiRetry = 0;
		
		wapiAssert(wapiInfo->waiCertCachedData);
		para = wapiInfo->waiCertCachedData;
		para->type = WAPI_IOCTL_TYPE_CA_AUTH;
		para->ptr = (void*)pstat;
		memset(para->name, 0, IFNAMSIZ);
		memcpy(para->name, wapiInfo->priv->dev->name, strlen(wapiInfo->priv->dev->name));
		wapiInfo->waiCertCachedDataLen = waihdr->length-(sizeof(wapiWaiHeader));
		memcpy(para->data, (((uint8*)waihdr)+(sizeof(wapiWaiHeader))), wapiInfo->waiCertCachedDataLen);
		wapiInfo->waiCertCachedDataLen += sizeof(wapiCAAppPara);

		wapiAssert(wapiInfo->waiCertCachedDataLen<WAPI_CERT_MAX_LEN);
		WAPI_UNLOCK(&wapiInfo->priv->pshare->lock);
	}
	
	WAPI_LOCK(&wapiInfo->priv->pshare->lock);
	/* update timer	*/
	mod_timer(&wapiInfo->waiResendTimer,jiffies + WAPI_CERT_REQ_TIMEOUT);
	WAPI_UNLOCK(&wapiInfo->priv->pshare->lock);

#if 0
	memset(data, 0, WAPI_CERT_MAX_LEN);
	para->type = WAPI_IOCTL_TYPE_CA_AUTH;
	para->ptr = (void*)pstat;
	memcpy(para->data, wapiInfo->waiCertCachedData, wapiInfo->waiCertCachedDataLen);
#endif

	DOT11_EnQueue((unsigned long)wapiInfo->priv, wapiInfo->priv->wapiEvent_queue, wapiInfo->waiCertCachedData, wapiInfo->waiCertCachedDataLen);
	wapi_event_indicate(wapiInfo->priv);
	
	return WAPI_RETURN_SUCCESS;
}

#define	WAPI_UCAST_KEYS_LEN		96
static int wapiRecvUnicastKeyAgreementResponse(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status)
{
	wapiWaiUCastRspPkt	*waiCastRsp;
	wapiStaInfo *wapiInfo;
	uint8	text[156];
	uint8	textLen;
	uint8	derivedKeys[WAPI_UCAST_KEYS_LEN];
	char *	uskKeyLabelSrc="pairwise key expansion for unicast and additional keys and nonce";
	uint8	keyIdx;

	if (waihdr->subType!=WAI_SUBTYPE_UCAST_KEY_RSP)
		return WAPI_RETURN_FAILED;

	wapiInfo = pstat->wapiInfo;
	waiCastRsp = (wapiWaiUCastRspPkt*)(((uint8*)waihdr)+(sizeof(wapiWaiHeader)));

	if (waiCastRsp->uskUpdate==1)
	{
		if (waiCastRsp->uskId==wapiInfo->wapiUCastKeyId)
			return WAPI_RETURN_FAILED;
	}

	if (wapiInfo->wapiUCastKeyUpdate)
	{
		keyIdx = !wapiInfo->wapiUCastKeyId;
	}
	else
	{
		keyIdx = wapiInfo->wapiUCastKeyId;
	}

	if (memcmp(waiCastRsp->WIEasue, wapiInfo->asueWapiIE, wapiInfo->asueWapiIELength)!=0)
	{
		*status = _RSON_IE_NOT_CONSISTENT_;
		return WAPI_RETURN_DEASSOC;
	}
	
	/*	sanity check	*/
	if (memcmp(waiCastRsp->bkId, wapiInfo->wapiBK.micKey,WAPI_KEY_LEN)!=0||
		(waiCastRsp->uskId!=keyIdx&&!wapiInfo->wapiUCastKeyUpdate)||
		memcmp(waiCastRsp->AEChallange, wapiInfo->waiAEChallange, WAPI_N_LEN)!=0
		)
	{
		return WAPI_RETURN_FAILED;
	}

	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->waiCertCachedData)
	{
		kfree(wapiInfo->waiCertCachedData);
		wapiInfo->waiCertCachedData = NULL;
		wapiInfo->waiCertCachedDataLen = 0;
	}
	
	/*	record ASUE Chanllange	*/
	memcpy(wapiInfo->waiASUEChallange, waiCastRsp->ASUEChallange, WAPI_N_LEN);

	/*	Calc Keys		*/
	memcpy(text, waiCastRsp->mac1, ETH_ALEN<<1);	/*	addID	*/
	textLen = ETH_ALEN<<1;
	memcpy(text+textLen, wapiInfo->waiAEChallange, WAPI_N_LEN);
	textLen += WAPI_N_LEN;
	memcpy(text+textLen, wapiInfo->waiASUEChallange, WAPI_N_LEN);
	textLen += WAPI_N_LEN;
	memcpy(text+textLen, uskKeyLabelSrc, strlen(uskKeyLabelSrc));
	textLen += strlen(uskKeyLabelSrc);

	KD_hmac_sha256(wapiInfo->wapiBK.dataKey, WAPI_KEY_LEN, 
		text, textLen, derivedKeys, WAPI_UCAST_KEYS_LEN);

	/*	UCast data key	*/
	memcpy(&wapiInfo->wapiUCastKey[keyIdx].dataKey, derivedKeys, WAPI_KEY_LEN);
	/*	UCast mic key		*/
	memcpy(&wapiInfo->wapiUCastKey[keyIdx].micKey, derivedKeys+WAPI_KEY_LEN, WAPI_KEY_LEN);
	/*	WAI Encrypt key	*/
	memcpy(wapiInfo->wapiWaiKey.micKey, derivedKeys+(WAPI_KEY_LEN<<1), WAPI_KEY_LEN);
	/*	WAI MCast Encrypt key	*/
	memcpy(wapiInfo->wapiWaiKey.dataKey, derivedKeys+(WAPI_KEY_LEN*3), WAPI_KEY_LEN);

	sha2(derivedKeys+(WAPI_KEY_LEN<<2), 32, wapiInfo->waiAEChallange, 0);

	if (wapiInfo->wapiState==ST_WAPI_AE_USK_AGGREMENT_REQ_SNT)
	{
		wapiInfo->wapiState = ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD;
		wapiInfo->wapiUCastRxEnable = 1;
		wapiInfo->wapiUCastTxEnable = 1;
	}

	if (wapiInfo->priv->pmib->wapiInfo.wapiUpdateUCastKeyType==wapi_time_update||
		wapiInfo->priv->pmib->wapiInfo.wapiUpdateUCastKeyType==wapi_all_update)
	{
		mod_timer(&wapiInfo->waiUCastKeyUpdateTimer, 
			jiffies+wapiInfo->priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout*HZ);
	}
	else
	{
		del_timer(&wapiInfo->waiUCastKeyUpdateTimer);
		init_timer(&wapiInfo->waiUCastKeyUpdateTimer);
		wapiInfo->waiUCastKeyUpdateTimer.data = (unsigned long)pstat;
		wapiInfo->waiUCastKeyUpdateTimer.function = wapiUCastUpdateKeyTimeout;
	}
	wapiInfo->wapiRetry = 0;

	{
		/* always set */
		wapiInfo->wapiUCastKeyUpdateCnt = wapiInfo->priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum;
	}

	WAPI_UNLOCK(&wapiInfo->lock);
	return WAPI_RETURN_SUCCESS;
}

static int wapiRecvMulticastKeyResponse(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status)
{
	wapiWaiMCastRspPkt	*waiMCastRsp;
	wapiStaInfo 			*wapiInfo;
	uint8				mic[WAI_MIC_LEN];

	if (waihdr->subType!=WAI_SUBTYPE_MCAST_KEY_RSP)
		return WAPI_RETURN_FAILED;

	wapiInfo = pstat->wapiInfo;
	waiMCastRsp = (wapiWaiMCastRspPkt*)(((uint8*)waihdr)+(sizeof(wapiWaiHeader)));

	/*	sanity check	*/
	if ( (waiMCastRsp->uskId!=wapiInfo->wapiUCastKeyId)||
		((waiMCastRsp->mskId!=wapiInfo->priv->wapiMCastKeyId)&&(!wapiInfo->priv->wapiMCastKeyUpdate))||
		(memcmp(waiMCastRsp->keyPN, wapiInfo->priv->keyNotify, WAPI_PN_LEN))||
		(memcmp(waiMCastRsp->mac1, wapiInfo->priv->dev->dev_addr, ETH_ALEN))||
		(memcmp(waiMCastRsp->mac2, pstat->hwaddr, ETH_ALEN)))
	{
		return WAPI_RETURN_FAILED;
	}
	sha256_hmac(wapiInfo->wapiWaiKey.micKey, WAPI_KEY_LEN, 
		(unsigned char*)waiMCastRsp, sizeof(wapiWaiMCastRspPkt),
 		(unsigned char*)mic, WAI_MIC_LEN);

	if (memcmp(mic, waiMCastRsp->mic, WAI_MIC_LEN))
	{
		return WAPI_RETURN_FAILED;
	}

	WAPI_LOCK(&wapiInfo->priv->pshare->lock);
	wapiInfo->wapiMCastEnable = 1;
	wapiInfo->wapiState = ST_WAPI_AE_MSKA_ESTABLISH;

	/* update timer	*/
	del_timer(&wapiInfo->waiResendTimer);
	init_timer(&wapiInfo->waiResendTimer);
	wapiInfo->waiResendTimer.data = (unsigned long)pstat;
	wapiInfo->waiResendTimer.function = wapiResendTimeout;
	wapiInfo->wapiRetry = 0;

	if (wapiInfo->priv->wapiMCastKeyUpdateCnt==0)
		wapiInfo->priv->wapiMCastKeyUpdateCnt = wapiInfo->priv->pmib->wapiInfo.wapiUpdateMCastKeyPktNum;
	WAPI_UNLOCK(&wapiInfo->priv->pshare->lock);
	return WAPI_RETURN_SUCCESS;
}

static int wapiRecvKeyUpdateResponse(wapiWaiHeader *waihdr, struct stat_info *pstat, int *status)
{
	wapiStaInfo 			*wapiInfo;
	int					ret;
	int					idx, idx2;
	int					allDone;
	struct list_head		*plist;
	struct stat_info		*pstat1;
	unsigned int			index;
	wapiWaiUCastRspPkt	*waiCastRsp;

	ret = WAPI_RETURN_FAILED;
	
	wapiInfo = pstat->wapiInfo;

	if (waihdr->subType==WAI_SUBTYPE_MCAST_KEY_RSP)
	{
		wapiAssert(wapiInfo->priv->wapiMCastKeyUpdate);
		if (wapiInfo->priv->wapiMCastKeyUpdate)
		{
			/*	The status already be: ST_WAPI_AE_MSKA_ESTABLISH */
			ret = wapiRecvMulticastKeyResponse(waihdr, pstat, status);
			if (ret==WAPI_RETURN_SUCCESS)
			{
				wapiInfo->wapiMCastKeyUpdateDone = TRUE;
				allDone = TRUE;
				for(index=0;index<NUM_STAT;index++)
				{
					plist = &wapiInfo->priv->stat_hash[index];
					while (plist->next != &(wapiInfo->priv->stat_hash[index]))
					{
						plist = plist->next;
						pstat1 = list_entry(plist, struct stat_info ,hash_list);

						if (pstat1 == NULL) {
							break;
						}

						if (pstat1->wapiInfo->wapiState<ST_WAPI_AE_MSKA_ESTABLISH)
						{
							issue_deauth(wapiInfo->priv, pstat1->hwaddr, _RSON_USK_HANDSHAKE_TIMEOUT_);
							continue;
						}
						
						if (!pstat1->wapiInfo->wapiMCastKeyUpdateDone)
						{
							allDone = FALSE;
							break;
						}
						
						if (plist == plist->next)
							break;
					}
					if (allDone==FALSE)
						break;
				}
				
				if (allDone==TRUE)
				{
					WAPI_LOCK(&wapiInfo->priv->pshare->lock);
					wapiInfo->priv->wapiMCastKeyId = !wapiInfo->priv->wapiMCastKeyId;
					wapiInfo->priv->wapiMCastKeyUpdateAllDone = 1;
					wapiInit(wapiInfo->priv);
					mod_timer(&wapiInfo->priv->waiMCastKeyUpdateTimer, jiffies + HZ);
					WAPI_UNLOCK(&wapiInfo->priv->pshare->lock);
				}
			}
		}
	}
	else if (waihdr->subType==WAI_SUBTYPE_UCAST_KEY_RSP)
	{
		if (wapiInfo->wapiUCastKeyUpdate)
		{
			ret = wapiRecvUnicastKeyAgreementResponse(waihdr, pstat, status);
			if (ret!=WAPI_RETURN_SUCCESS)
			{
				return ret;
			}

			ret = wapiSendUnicastKeyAgrementConfirm(wapiInfo->priv, pstat);
			if (ret==WAPI_RETURN_SUCCESS)
			{
				WAPI_LOCK(&wapiInfo->lock);

				/*	prevent duplicate USK confirm	*/
				del_timer(&wapiInfo->waiResendTimer);
				init_timer(&wapiInfo->waiResendTimer);
				wapiInfo->waiResendTimer.data = (unsigned long)pstat;
				wapiInfo->waiResendTimer.function = wapiResendTimeout;

				/*	toggle the keyID	*/
				wapiInfo->wapiUCastKeyId = !wapiInfo->wapiUCastKeyId;
				/*	set pn	*/
#ifdef LITTLE_ENDIAN
				for (idx=0;idx<WAPI_PN_LEN;idx+=2)
				{
					wapiInfo->wapiPN.rxUCast[0][idx] = 0x5c;
					wapiInfo->wapiPN.rxUCast[0][idx+1] = 0x36;
				}
#else
				for (idx=WAPI_PN_LEN;idx>0;idx-=2)
				{
					wapiInfo->wapiPN.rxUCast[0][idx-1] = 0x5c;
					wapiInfo->wapiPN.rxUCast[0][idx-2] = 0x36;
				}
#endif
				for(idx2=1;idx2<RX_QUEUE_NUM;idx2++)
				{
					memcpy(&wapiInfo->wapiPN.rxUCast[idx2][0], wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
				}
				memcpy(wapiInfo->wapiPN.txUCast, wapiInfo->wapiPN.rxUCast, WAPI_PN_LEN);
				for(idx2=0;idx2<RX_QUEUE_NUM;idx2++)
				{
					wapiInfo->wapiPN.rxUCast[idx2][idx] = 0x37;
				}
				wapiInfo->wapiPN.txUCast[idx] = 0x37;
				memset(wapiInfo->wapiPN.rxSeq, 0, RX_QUEUE_NUM*sizeof(unsigned short));
				
				wapiInfo->wapiUCastKeyUpdate = 0;
				WAPI_UNLOCK(&wapiInfo->lock);
			}

		}
		else
		{
			waiCastRsp = (wapiWaiUCastRspPkt*)(((uint8*)waihdr)+(sizeof(wapiWaiHeader)));
			if (waiCastRsp->uskUpdate==1)
				wapiUpdateUSK(wapiInfo->priv, pstat);
		}
	}

	return ret;
}

static int SecIsWAIPacket(struct sk_buff *pskb, int *waiOffset)
{
	int		Offset_TypeWAI;

	if (is_qos_data(pskb->data))
	{
		Offset_TypeWAI = WLAN_HDR_A3_QOS_LEN+WLAN_LLC_HEADER_SIZE;
	}
	else
	{
		Offset_TypeWAI = WLAN_HDR_A3_LEN+WLAN_LLC_HEADER_SIZE;
	}

	if (pskb->len<(Offset_TypeWAI+2))
	{
		return FAILED;
	}

	if (*((uint16*)&pskb->data[Offset_TypeWAI])!=htons(ETH_P_WAPI))
	{
		return FAILED;
	}

	/*	data to wai header	*/
	*waiOffset = Offset_TypeWAI+2;
#if 0
	pskb->data += Offset_TypeWAI+2;	/*	2 for ether type	*/
	pskb->len -= Offset_TypeWAI+2;
#endif
	return SUCCESS;
}

int wapiHandleRecvPacket(struct rx_frinfo *pfrinfo, struct stat_info *pstat)
{
	struct sk_buff		*pskb;
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader	*waihdr;
	struct rtl8192cd_priv *priv;
	int				status;
	int				waiOffset;

	wapiInfo = pstat->wapiInfo;

	if (wapiInfo==NULL)
		return FAILED;
	
	priv = wapiInfo->priv;

	if (wapiInfo->wapiType==wapiDisable)
		return FAILED;

	pskb = pfrinfo->pskb;

	if (SecIsWAIPacket(pskb, &waiOffset)==FAILED)
	{
		if (wapiInfo->wapiState==ST_WAPI_AE_MSKA_ESTABLISH)
		{
			return FAILED;
		}
		else
			goto release_out;
	}

	/*	after SecIsWAIPacket() check , the pskb->data point to wai header	*/
	waihdr = (wapiWaiHeader*)(pskb->data+waiOffset);
	
	/*	wai sanity check	*/
	wapiAssert(waihdr->sequenceNum>(wapiInfo->waiRxSeq));

#if 0
	if (waihdr->sequenceNum!=(wapiInfo->waiRxSeq+1))	/* add rx sequence */
#else
	if (waihdr->sequenceNum<=wapiInfo->waiRxSeq)
#endif
	{
		WAPI_LOCK(&wapiInfo->lock);
		wapiReleaseFragementQueue(wapiInfo);
		WAPI_UNLOCK(&wapiInfo->lock);
		goto release_out;
	}
	
	if (waihdr->protocolVersion!=WAI_V1 ||
		waihdr->type!=WAI_TYPE_WAI ||
		waihdr->length>pskb->len)
	{
		goto release_out;
	}

	if (((waihdr->flags&WAI_HEADER_MF)!=0)||
		(waihdr->fragmentNum!=0))
	{
		if ((pskb=wapiDefragement(pskb, pstat, waiOffset))==NULL)
		{
			wapiAssert(!pskb);
			return SUCCESS;
		}
		else
			waihdr = (wapiWaiHeader*)(pskb->data+waiOffset);
	}

	wapiInfo->waiRxSeq++;

#if defined(CLIENT_MODE)
	if (OPMODE & WIFI_AP_STATE)
#endif
	{
		switch (wapiInfo->wapiState)
		{
			case ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT:
			{
#if 0
				WAPI_LOCK(&wapiInfo->lock);
				wapiInfo->wapiRetry = 0;
				mod_timer(&wapiInfo->waiResendTimer,0);
				WAPI_UNLOCK(&wapiInfo->lock);
#endif
				switch (WapiRecvAccessAuthenticateRequest(waihdr, pstat, &status))
				{
					case WAPI_RETURN_FAILED:
						goto release_out;
				}
				goto release_out;
			}
			case ST_WAPI_AE_USK_AGGREMENT_REQ_SNT:
			{
#if 0
				WAPI_LOCK(&wapiInfo->lock);
				wapiInfo->wapiRetry = 0;
				mod_timer(&wapiInfo->waiResendTimer,0);
				WAPI_UNLOCK(&wapiInfo->lock);
#endif
				switch (wapiRecvUnicastKeyAgreementResponse(waihdr, pstat, &status))
				{
					case WAPI_RETURN_FAILED:
						goto release_out;
					case WAPI_RETURN_DEASSOC:
						goto deAuth_out;
				}

				wapiSendUnicastKeyAgrementConfirm(priv, pstat);
				wapiSendMulticastKeyNotification(priv, pstat);
				goto release_out;
			}
			case ST_WAPI_AE_MSK_NOTIFICATION_SNT:
			{
#if 0
				WAPI_LOCK(&wapiInfo->lock);
				wapiInfo->wapiRetry = 0;
				mod_timer(&wapiInfo->waiResendTimer,0);
				WAPI_UNLOCK(&wapiInfo->lock);
#endif
				switch (wapiRecvMulticastKeyResponse(waihdr, pstat, &status))
				{
					case WAPI_RETURN_FAILED:
						goto release_out;
				}
				goto release_out;
			}
			case ST_WAPI_AE_MSKA_ESTABLISH:
			{
				/*	key update	*/
#if 0
				WAPI_LOCK(&wapiInfo->lock);
				wapiInfo->wapiRetry = 0;
				mod_timer(&wapiInfo->waiResendTimer,0);
				WAPI_UNLOCK(&wapiInfo->lock);
#endif
				switch (wapiRecvKeyUpdateResponse(waihdr, pstat, &status))
				{
					case WAPI_RETURN_FAILED:
						goto release_out;
				}

				goto release_out;
			}
			case ST_WAPI_AE_ACCESS_CERTIFICATE_REQ_SNT:
			case ST_WAPI_AE_ACCESS_AUTHENTICATE_REQ_RCVD:
			case ST_WAPI_AE_USKA_ESTABLISH:
			case ST_WAPI_AE_MSK_RSP_RCVD:
			case ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD:				
			case ST_WAPI_AE_BKSA_ESTABLISH:
				goto release_out;
			default:
				goto release_out;
		}
	}
#if 0
#if defined(CLIENT_MODE)
	else if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
	{
	}
#endif
#endif

deAuth_out:
	wapiAssert(wapiInfo->wapiState);
	wapiInfo->wapiState = ST_WAPI_AE_IDLE;
	issue_deauth(priv, pstat->hwaddr, status);
release_out:
	wapiAssert(wapiInfo->wapiState);
	rtl_kfree_skb(priv, pskb, _SKB_RX_);
	return SUCCESS;
}

int	wapiIEInfoInstall(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int 			akmNum;
	uint8		*akm;

	WAPI_LOCK(&pstat->wapiInfo->lock);
	akm = &pstat->wapiInfo->asueWapiIE[WAPI_AKM_OFFSET];
	akmNum = le16_to_cpu(*((uint16*)&akm[0]));
	wapiAssert(akmNum==1);
	akm += 2;

	/*	AKM	sanity check	*/
	if(!memcmp(akm, WAPI_KM_OUI, WAPI_KM_OUI_LEN)
		&&(akm[WAPI_KM_OUI_LEN]&priv->pmib->wapiInfo.wapiType))
	{
		pstat->wapiInfo->wapiType=akm[WAPI_KM_OUI_LEN];
	}
	else
	{
		WAPI_UNLOCK(&pstat->wapiInfo->lock);
		return _RSON_INVALID_WAPI_CAPABILITY_;
	}
	akm += WAPI_KM_OUI_LEN+1;

	akmNum = le16_to_cpu(*((uint16*)&akm[0]));
	wapiAssert(akmNum==1);
	akm += 2;
	
	/*	UKM	sanity check	*/
	if(!memcmp(akm, WAPI_KM_OUI, WAPI_KM_OUI_LEN)
		&&(akm[WAPI_KM_OUI_LEN]==wapi_SMS4))
	{
#if defined(WAPI_SUPPORT_MULTI_ENCRYPT)
		pstat->wapiInfo->wapiUCastKey[pstat->wapiInfo->wapiUCastKeyId].keyType=akm[WAPI_KM_OUI_LEN];
#endif	
	}
	else
	{
		WAPI_UNLOCK(&pstat->wapiInfo->lock);
		return _RSON_INVALID_USK_;
	}
	akm += WAPI_KM_OUI_LEN+1;

	/*	MKM sanity check	*/
	if(!memcmp(akm, WAPI_KM_OUI, WAPI_KM_OUI_LEN)
		&&(akm[WAPI_KM_OUI_LEN]==wapi_SMS4))
	{
#if defined(WAPI_SUPPORT_MULTI_ENCRYPT)
		pstat->wapiInfo->wapiMCastKey[priv->wapiMCastKeyId].keyType=akm[WAPI_KM_OUI_LEN];
#endif
	}else
	{
		WAPI_UNLOCK(&pstat->wapiInfo->lock);
		return _RSON_INVALID_MSK_;
	}

	/*	record AE WAPI IE	*/
	priv->wapiCachedBuf = pstat->wapiInfo->aeWapiIE+2;
	wapiSetIE(priv);
	pstat->wapiInfo->aeWapiIE[0] = _EID_WAPI_;
	pstat->wapiInfo->aeWapiIE[1] = priv->wapiCachedLen;
	pstat->wapiInfo->aeWapiIELength = priv->wapiCachedLen+2;

	WAPI_UNLOCK(&pstat->wapiInfo->lock);
	return _STATS_SUCCESSFUL_;
}

void wapiReqActiveCA(struct stat_info *pstat)
{
#define	REQ_ACTIVE_CA_LEN	64
	static uint8			data[REQ_ACTIVE_CA_LEN];
	wapiCAAppPara	*para;
	wapiStaInfo	*wapiInfo;
	struct rtl8192cd_priv	*priv;

	wapiInfo = pstat->wapiInfo;
	priv = wapiInfo->priv;
	para = (wapiCAAppPara*)data;
	memset(data, 0, REQ_ACTIVE_CA_LEN);

	para->type = WAPI_IOCTL_TYPE_REQ_ACTIVE;
	para->ptr = (void*)pstat;
	memset(para->name, 0, IFNAMSIZ);
	memcpy(para->name, priv->dev->name, strlen(priv->dev->name));
	memcpy(para->data, priv->dev->dev_addr, ETH_ALEN);
	memcpy(&para->data[ETH_ALEN], pstat->hwaddr, ETH_ALEN);
	DOT11_EnQueue((unsigned long)wapiInfo->priv, wapiInfo->priv->wapiEvent_queue, (unsigned char*)para, sizeof(wapiCAAppPara)+(ETH_ALEN<<1));
	wapi_event_indicate(wapiInfo->priv);

	WAPI_LOCK(&wapiInfo->lock);
	if (ST_WAPI_AE_IDLE==wapiInfo->wapiState)
		wapiInfo->wapiState = ST_WAPI_AE_ACTIVE_AUTHENTICATION_REQ;
	mod_timer(&wapiInfo->waiResendTimer, jiffies + WAPI_CERT_REQ_TIMEOUT);
	WAPI_UNLOCK(&wapiInfo->lock);
}
/*void	wapiSetBKByPreshareKey(struct stat_info *pstat)*/
void	wapiSetBK(struct stat_info *pstat)
{
	wapiStaInfo	*wapiInfo;
	struct rtl8192cd_priv	*priv;
	uint8		addrID[ETH_ALEN*2];
	uint8 		*preSharedKeyLabelSrc="preshared key expansion for authentication and key negotiation";
	uint8		*baseKeyLabelSrc="base key expansion for key and additional nonce";
	uint8		text[128];
	uint8		textLen;
	
	wapiInfo = pstat->wapiInfo;
	priv = wapiInfo->priv;

	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->wapiType==wapiTypePSK)
	{
		wapiAssert((wapiInfo->priv->pmib->wapiInfo.wapiType&wapiTypePSK));
		/*	BK	*/
		KD_hmac_sha256(
			priv->pmib->wapiInfo.wapiPsk.octet, priv->pmib->wapiInfo.wapiPsk.len, 
			preSharedKeyLabelSrc, strlen(preSharedKeyLabelSrc),
			wapiInfo->wapiBK.dataKey, WAPI_KEY_LEN);
	}
	else if (wapiInfo->wapiType==wapiTypeCert)
	{
		wapiAssert((priv->pmib->wapiInfo.wapiType&wapiTypeCert));
		wapiAssert(priv->wapiCachedBuf);
		textLen = WAPI_N_LEN<<1;
		memcpy(text, priv->wapiCachedBuf+24, textLen);
		memcpy(text+textLen, baseKeyLabelSrc, strlen(baseKeyLabelSrc));
		textLen += strlen(baseKeyLabelSrc);
		/*	BK	*/
		KD_hmac_sha256(
			priv->wapiCachedBuf, 24, 
			text, textLen,
			wapiInfo->wapiBK.dataKey, WAPI_KEY_LEN);
	}
	else
		wapiAssert(0);

	{
		/*	BKID		*/
		memcpy(addrID, priv->dev->dev_addr, ETH_ALEN);
		memcpy(&addrID[ETH_ALEN], pstat->hwaddr, ETH_ALEN);
		KD_hmac_sha256(
			wapiInfo->wapiBK.dataKey, WAPI_KEY_LEN, 
			addrID, ETH_ALEN*2, 
			wapiInfo->wapiBK.micKey, WAPI_KEY_LEN);	
	}

	/*	set default AE Challange	*/
	GenerateRandomData(wapiInfo->waiAEChallange, WAPI_N_LEN);
	wapiInfo->wapiState = ST_WAPI_AE_BKSA_ESTABLISH;
	WAPI_UNLOCK(&wapiInfo->lock);
}

void	wapiSetBKByCA(struct stat_info *pstat, uint8 *BKBase)
{
	wapiStaInfo	*wapiInfo;
	struct rtl8192cd_priv	*priv;
	uint8		addrID[ETH_ALEN*2];
	uint8 		*preSharedKeyLabelSrc="preshared key expansion for authentication and key negotiation";

	wapiInfo = pstat->wapiInfo;
	priv = wapiInfo->priv;

	WAPI_LOCK(&wapiInfo->lock);
	wapiAssert((wapiInfo->wapiType==wapiTypePSK)&&(wapiInfo->priv->pmib->wapiInfo.wapiType&wapiTypePSK));
	{
		/*	BK	*/
		KD_hmac_sha256(
			priv->pmib->wapiInfo.wapiPsk.octet, priv->pmib->wapiInfo.wapiPsk.len, 
			preSharedKeyLabelSrc, strlen(preSharedKeyLabelSrc),
			wapiInfo->wapiBK.dataKey, WAPI_KEY_LEN);
	}

	{
		/*	BKID		*/
		memcpy(addrID, priv->dev->dev_addr, ETH_ALEN);
		memcpy(&addrID[ETH_ALEN], pstat->hwaddr, ETH_ALEN);
		KD_hmac_sha256(
			wapiInfo->wapiBK.dataKey, WAPI_KEY_LEN, 
			addrID, ETH_ALEN*2, 
			wapiInfo->wapiBK.micKey, WAPI_KEY_LEN);	
	}

	/*	set default AE Challange	*/
	GenerateRandomData(wapiInfo->waiAEChallange, WAPI_N_LEN);
	pstat->wapiInfo->wapiState = ST_WAPI_AE_BKSA_ESTABLISH;
	WAPI_UNLOCK(&wapiInfo->lock);
}

static inline void wapiSetWaiHeader(wapiWaiHeader *wai_hdr, uint8 subType)
{
	wai_hdr->protocolVersion = WAI_V1;
	wai_hdr->type = WAI_TYPE_WAI;
	wai_hdr->subType = subType;
	wai_hdr->reserved = 0;
	wai_hdr->length = WAI_HEADER_LEN;
	wai_hdr->fragmentNum = 0;
	wai_hdr->flags = 0;
}

static int	WapiSendActivateAuthenticationPacket(struct rtl8192cd_priv *priv, struct stat_info *pstat, int len, uint8 *data)
{
	struct sk_buff		*pskb;
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader		*wai_hdr;
	wapiWaiCertActivPkt	*wai_cert_active;
	unsigned long		timeout;
	wapiTLV			*tlvHeader;
	wapiTLV1			*tlv1Hdr;
	int				tlvLen;

	wapiInfo = pstat->wapiInfo;
	wapiAssert(wapiInfo->wapiState==ST_WAPI_AE_ACTIVE_AUTHENTICATION_REQ||wapiInfo->wapiState==ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT);

	tlvHeader = (wapiTLV*)data;
	/*	ASU ID	*/
	tlvLen= sizeof(wapiTLV)+tlvHeader->len;
	tlvHeader = (wapiTLV*)(&tlvHeader->data[tlvHeader->len]);
	/*	AE CA	*/
	tlvLen += sizeof(wapiTLV)+tlvHeader->len;
	tlv1Hdr = (wapiTLV1*)(&tlvHeader->data[tlvHeader->len]);
	/*	ECDH para	*/
	tlvLen += sizeof(wapiTLV1)+tlv1Hdr->len;
	if (tlvLen !=len)
	{
		return WAPI_RETURN_FAILED;
	}
	
	timeout = jiffies + WAPI_GENERAL_TIMEOUT;
	pskb = rtl_dev_alloc_skb(priv, MAXDATALEN, _SKB_TX_, TRUE);
	if (pskb==NULL)
		goto updateTimer;

	if (wapiInfo->waiCertCachedData==NULL)
	{
		WAPI_LOCK(&wapiInfo->lock);
		wapiInfo->waiCertCachedData = kmalloc(WAPI_CERT_MAX_LEN, GFP_ATOMIC);
		WAPI_UNLOCK(&wapiInfo->lock);
		if (wapiInfo->waiCertCachedData==NULL)
			goto updateTimer;
	}

	pskb->protocol = htonl(ETH_P_WAPI);
	memcpy(pskb->data, pstat->hwaddr, ETH_ALEN);
	memcpy(&pskb->data[ETH_ALEN], priv->dev->dev_addr, ETH_ALEN);
	*((uint16*)&pskb->data[ETH_ALEN<<1]) = htons(ETH_P_WAPI);
	pskb->dev = priv->dev;
	skb_put(pskb, 14);		/*	DA|SA|ETHER_TYPE|	*/
	
	/*	set wai header	*/
	wai_hdr = (wapiWaiHeader*)(pskb->data+pskb->len);
	wapiSetWaiHeader(wai_hdr, WAI_SUBTYPE_AUTH_ACTIVE);
	wai_hdr->sequenceNum = ++wapiInfo->waiTxSeq;

	/*	set unicast request pkt	*/
	wai_cert_active = (wapiWaiCertActivPkt*)(((unsigned char*)wai_hdr) + wai_hdr->length);
	wai_cert_active->reserved = 0;
	/*	does NOT support pre-auth	*/
	wai_cert_active->preAuth= 0;
	if (wapiInfo->wapiState==ST_WAPI_AE_ACTIVE_AUTHENTICATION_REQ ||
		wapiInfo->wapiState==ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT)
	{	/*	first time	*/
		wai_cert_active->updateBK= 0;

		WAPI_LOCK(&wapiInfo->lock);
		/*	set authFlag	*/
		GenerateRandomData(wapiInfo->waiAuthFlag, WAPI_N_LEN);
		WAPI_UNLOCK(&wapiInfo->lock);
	}
	else
	{
		/*	update bk key	*/
		wapiAssert(0);
		wai_cert_active->updateBK= 1;
	}

	memcpy(wai_cert_active->authFlag, wapiInfo->waiAuthFlag, WAPI_N_LEN);
	
	memcpy(wai_cert_active->data, data, len);
	wai_hdr->length += sizeof(wapiWaiCertActivPkt)+len;
	wapiAssert(wai_hdr->length<MAXDATALEN);
	skb_put(pskb, wai_hdr->length);

	/*	14 = DA|SA|ETHER_TYPE|	*/
	if (pskb->len-14>(priv->dev->mtu-WLAN_HDR_A3_QOS_LEN))
	{
		if (wapiFragementSend(pskb, priv)!=SUCCESS)
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	else
	{
		if (rtl8192cd_start_xmit(pskb, priv->dev))
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}

	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->wapiState==ST_WAPI_AE_ACTIVE_AUTHENTICATION_REQ)
	{
		/*	cache data	*/
		memcpy(wapiInfo->waiCertCachedData, data, len);
		wapiInfo->waiCertCachedDataLen = len;
		wapiInfo->wapiState = ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT;
	}
	WAPI_UNLOCK(&wapiInfo->lock);
updateTimer:	
	WAPI_LOCK(&wapiInfo->lock);
#if 0
	wapiInfo->waiResendTimer.expires = jiffies + WAPI_GENERAL_TIMEOUT;
	add_timer(&(wapiInfo->waiResendTimer));
#else
	mod_timer(&wapiInfo->waiResendTimer, timeout);
#endif
	WAPI_UNLOCK(&wapiInfo->lock);
	return WAPI_RETURN_SUCCESS;
}

int	WapiSendAuthenticationRspPacket(struct rtl8192cd_priv *priv, struct stat_info *pstat, int len, uint8 *data)
{
	struct sk_buff		*pskb;
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader		*wai_hdr;
	wapiWaiCertAuthRspPkt	*wai_cert_rsp;
	unsigned long		timeout;

	wapiInfo = pstat->wapiInfo;
	wapiAssert(wapiInfo->wapiState==ST_WAPI_AE_USK_AGGREMENT_REQ_SNT 
		||wapiInfo->wapiState==ST_WAPI_AE_BKSA_ESTABLISH);
	timeout = jiffies + WAPI_GENERAL_TIMEOUT;
	
	pskb = rtl_dev_alloc_skb(priv, WAPI_CERT_MAX_LEN, _SKB_TX_, TRUE);
	if (pskb==NULL)
		goto updateTimer;
	if (wapiInfo->waiCertCachedData==NULL)
	{
		wapiAssert(0);
		goto updateTimer;
	}

	pskb->protocol = htonl(ETH_P_WAPI);
	memcpy(pskb->data, pstat->hwaddr, ETH_ALEN);
	memcpy(&pskb->data[ETH_ALEN], priv->dev->dev_addr, ETH_ALEN);
	*((uint16*)&pskb->data[ETH_ALEN<<1]) = htons(ETH_P_WAPI);
	pskb->dev = priv->dev;
	skb_put(pskb, 14);		/*	DA|SA|ETHER_TYPE|	*/
	
	/*	set wai header	*/
	wai_hdr = (wapiWaiHeader*)(pskb->data+pskb->len);
	wapiSetWaiHeader(wai_hdr, WAI_SUBTYPE_AUTH_RSP);
	wai_hdr->sequenceNum = ++wapiInfo->waiTxSeq;

	/*	set unicast request pkt	*/
	wai_cert_rsp = (wapiWaiCertAuthRspPkt*)(((unsigned char*)wai_hdr) + wai_hdr->length);
	memcpy((uint8*)wai_cert_rsp, data, len);
#if 0
	/*	That's application's responsibility to preprare all the data
	*	including the flags.
	*/
	wai_cert_rsp->reserved1 = wai_cert_rsp->reserved2 = 0;
	wai_cert_rsp->preAuth = 0;
	wai_cert_rsp->updateBK = 0;
#endif
	wai_hdr->length += len;
	wapiAssert(wai_hdr->length<WAPI_CERT_MAX_LEN);
	skb_put(pskb, wai_hdr->length);

	/*	14 = DA|SA|ETHER_TYPE|	*/
	if (pskb->len-14>(priv->dev->mtu-WLAN_HDR_A3_QOS_LEN))
	{
		if (wapiFragementSend(pskb, priv)!=SUCCESS)
		{
			wapiAssert(0);
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	else
	{
		if (rtl8192cd_start_xmit(pskb, priv->dev))
		{
			wapiAssert(0);
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	

	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->wapiState==ST_WAPI_AE_BKSA_ESTABLISH)
	{
		memcpy(wapiInfo->waiCertCachedData, data, len);
		wapiInfo->waiCertCachedDataLen = len;
		/*wapiInfo->wapiState = ST_WAPI_AE_BKSA_ESTABLISH;*/
	}
	WAPI_UNLOCK(&wapiInfo->lock);
updateTimer:
	WAPI_LOCK(&wapiInfo->lock);
#if 0
	wapiInfo->waiResendTimer.expires = jiffies + WAPI_GENERAL_TIMEOUT;
	add_timer(&(wapiInfo->waiResendTimer));
#else
	mod_timer(&wapiInfo->waiResendTimer, timeout);
#endif
	WAPI_UNLOCK(&wapiInfo->lock);
	return WAPI_RETURN_SUCCESS;
}

int	wapiSendUnicastKeyAgrementRequeset(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct sk_buff		*pskb;
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader	*wai_hdr;
	wapiWaiUCastReqPkt	*wai_ucast_req;
	unsigned long		timeout;

	timeout = jiffies + WAPI_GENERAL_TIMEOUT;
	wapiInfo = pstat->wapiInfo;
	pskb = rtl_dev_alloc_skb(priv, MAXDATALEN, _SKB_TX_, TRUE);
	if (pskb==NULL)
		goto updateTimer;

	pskb->protocol = htonl(ETH_P_WAPI);
	memcpy(pskb->data, pstat->hwaddr, ETH_ALEN);
	memcpy(&pskb->data[ETH_ALEN], priv->dev->dev_addr, ETH_ALEN);
	*((uint16*)&pskb->data[ETH_ALEN<<1]) = htons(ETH_P_WAPI);
	skb_put(pskb, 14);		/*	DA|SA|ETHER_TYPE|	*/
	pskb->dev = priv->dev;

	/*	set wai header	*/
	wai_hdr = (wapiWaiHeader*)(pskb->data+pskb->len);
	wapiSetWaiHeader(wai_hdr, WAI_SUBTYPE_UCAST_KEY_REQ);
	wai_hdr->sequenceNum = ++wapiInfo->waiTxSeq;

	/*	set unicast request pkt	*/
	wai_ucast_req = (wapiWaiUCastReqPkt*)(((unsigned char*)wai_hdr) + wai_hdr->length);
	wai_ucast_req->reserved1 = wai_ucast_req->reserved2 = 0;
	if (wapiInfo->wapiState==ST_WAPI_AE_BKSA_ESTABLISH ||
		wapiInfo->wapiState==ST_WAPI_AE_USK_AGGREMENT_REQ_SNT)
	{	/*	first time	*/
		wai_ucast_req->uskUpdate = FALSE;
		wai_ucast_req->uskId = 0;
		wapiAssert(wapiInfo->wapiUCastKeyId==0);
		WAPI_LOCK(&wapiInfo->lock);
		wapiInfo->wapiUCastKeyId=0;
		wapiInfo->wapiUCastKeyUpdate=0;
		WAPI_UNLOCK(&wapiInfo->lock);
	}
	else
	{
		/*	update key	*/
		wapiAssert(wapiInfo->wapiUCastKeyUpdate);
		wai_ucast_req->uskUpdate = TRUE;
		wai_ucast_req->uskId = !wapiInfo->wapiUCastKeyId;
	}
	
	memcpy(wai_ucast_req->bkId, wapiInfo->wapiBK.micKey, WAPI_KEY_LEN);
	memcpy(wai_ucast_req->mac1, priv->dev->dev_addr, ETH_ALEN);
	memcpy(wai_ucast_req->mac2, pstat->hwaddr, ETH_ALEN);
	memcpy(wai_ucast_req->AEChallange, wapiInfo->waiAEChallange, WAPI_N_LEN);
	wai_hdr->length += sizeof(wapiWaiUCastReqPkt);

	wapiAssert(wai_hdr->length<MAXDATALEN);
	skb_put(pskb, wai_hdr->length);

	/*	14 = DA|SA|ETHER_TYPE|	*/
	if (pskb->len-14>(priv->dev->mtu-WLAN_HDR_A3_QOS_LEN))
	{
		if (wapiFragementSend(pskb, priv)!=SUCCESS)
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	else
	{
		if (rtl8192cd_start_xmit(pskb, priv->dev))
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}

	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->wapiState==ST_WAPI_AE_BKSA_ESTABLISH)
		wapiInfo->wapiState = ST_WAPI_AE_USK_AGGREMENT_REQ_SNT;
	WAPI_UNLOCK(&wapiInfo->lock);
updateTimer:
	WAPI_LOCK(&wapiInfo->lock);
#if 0
	wapiInfo->waiResendTimer.expires = timeout;
	add_timer(&(wapiInfo->waiResendTimer));
#else
	mod_timer(&wapiInfo->waiResendTimer, timeout);
#endif
	WAPI_UNLOCK(&wapiInfo->lock);

	return WAPI_RETURN_SUCCESS;
}

int	wapiSendUnicastKeyAgrementConfirm(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct sk_buff		*pskb;
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader	*wai_hdr;
	wapiWaiUCastAckPkt	*wai_ucast_ack;

	pskb = rtl_dev_alloc_skb(priv, MAXDATALEN, _SKB_TX_, TRUE);
	if (pskb==NULL)
		goto updateTimer;

	pskb->protocol = htonl(ETH_P_WAPI);
	memcpy(pskb->data, pstat->hwaddr, ETH_ALEN);
	memcpy(&pskb->data[ETH_ALEN], priv->dev->dev_addr, ETH_ALEN);
	*((uint16*)&pskb->data[ETH_ALEN<<1]) = htons(ETH_P_WAPI);
	skb_put(pskb, 14);		/*	DA|SA|ETHER_TYPE	*/
	pskb->dev = priv->dev;

	wapiInfo = pstat->wapiInfo;
	/*	set wai header	*/
	wai_hdr = (wapiWaiHeader*)(pskb->data+pskb->len);
	wapiSetWaiHeader(wai_hdr, WAI_SUBTYPE_UCAST_KEY_ACK);
	wai_hdr->sequenceNum = ++wapiInfo->waiTxSeq;

	/*	set unicast response pkt	*/
	wai_ucast_ack = (wapiWaiUCastAckPkt*)(((unsigned char*)wai_hdr) + wai_hdr->length);
	wai_ucast_ack->reserved1 = wai_ucast_ack->reserved2 = 0;
	if (wapiInfo->wapiState==ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD
		||wapiInfo->wapiState==ST_WAPI_AE_USKA_ESTABLISH)
	{	/*	first time	*/
		wai_ucast_ack->uskUpdate = FALSE;
		wai_ucast_ack->uskId = 0;
		wapiAssert(wapiInfo->wapiUCastKeyId==0);
	}
	else
	{
		/*	update key	*/
		wai_ucast_ack->uskUpdate = TRUE;
		wai_ucast_ack->uskId = !wapiInfo->wapiUCastKeyId;
	}

	memcpy(wai_ucast_ack->bkId, wapiInfo->wapiBK.micKey, WAPI_KEY_LEN);
	memcpy(wai_ucast_ack->mac1, priv->dev->dev_addr, ETH_ALEN);
	memcpy(wai_ucast_ack->mac2, pstat->hwaddr, ETH_ALEN);
	memcpy(wai_ucast_ack->ASUEChallange, wapiInfo->waiASUEChallange, WAPI_N_LEN);
	memcpy(wai_ucast_ack->WIEae, wapiInfo->aeWapiIE, wapiInfo->aeWapiIELength);

	/*	MIC Calc	*/
	sha256_hmac(wapiInfo->wapiWaiKey.micKey, WAPI_KEY_LEN, (uint8*)wai_ucast_ack, 
		sizeof(wapiWaiUCastAckPkt)+wapiInfo->aeWapiIELength, 
		(((uint8*)wai_ucast_ack)+sizeof(wapiWaiUCastAckPkt)+wapiInfo->aeWapiIELength), 
		WAI_MIC_LEN);

	wai_hdr->length += sizeof(wapiWaiUCastAckPkt)+wapiInfo->aeWapiIELength+WAI_MIC_LEN;
	wapiAssert(wai_hdr->length<MAXDATALEN);
	skb_put(pskb, wai_hdr->length);

	/*	14 = DA|SA|ETHER_TYPE|	*/
	if (pskb->len-14>(priv->dev->mtu-WLAN_HDR_A3_QOS_LEN))
	{
		if (wapiFragementSend(pskb, priv)!=SUCCESS)
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	else
	{
		if (rtl8192cd_start_xmit(pskb, priv->dev))
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}

	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->wapiState==ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD)
		wapiInfo->wapiState = ST_WAPI_AE_USKA_ESTABLISH;
/*	mod_timer(&wapiInfo->waiResendTimer, jiffies + WAPI_GENERAL_TIMEOUT); */
	WAPI_UNLOCK(&wapiInfo->lock);
updateTimer:
	return WAPI_RETURN_SUCCESS;
}

int	wapiSendMulticastKeyNotification(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct sk_buff		*pskb;
	wapiStaInfo		*wapiInfo;
	wapiWaiHeader	*wai_hdr;
	wapiWaiMCastNofiPkt	*wai_mcast_noti;
	unsigned long			timeout;

#if 0
	struct list_head	*plist;
	struct stat_info	*pstat1;
	unsigned int	index;
#endif

	wapiInfo = pstat->wapiInfo;
	timeout = jiffies + WAPI_GENERAL_TIMEOUT;
	if (WapiIncreasePN(priv->keyNotify, 1)==WAPI_RETURN_SUCCESS)
	{
		WAPI_LOCK(&priv->pshare->lock);
		wapiFreeAllSta(priv, TRUE);
		WAPI_UNLOCK(&priv->pshare->lock);
		wapiInfo->wapiState = ST_WAPI_AE_IDLE;
		return WAPI_RETURN_SUCCESS;
	}
	pskb = rtl_dev_alloc_skb(priv, MAXDATALEN, _SKB_TX_, TRUE);

	if (pskb==NULL)
		goto updateTimer;

	pskb->protocol = htonl(ETH_P_WAPI);		
	memcpy(pskb->data, pstat->hwaddr, ETH_ALEN);
	memcpy(&pskb->data[ETH_ALEN], priv->dev->dev_addr, ETH_ALEN);
	*((uint16*)&pskb->data[ETH_ALEN<<1]) = htons(ETH_P_WAPI);
	skb_put(pskb, 14);		/*	DA|SA|ETHER_TYPE|	*/
	pskb->dev = priv->dev;

	/*	set wai header	*/
	wai_hdr = (wapiWaiHeader*)(pskb->data+pskb->len);
	wapiSetWaiHeader(wai_hdr, WAI_SUBTYPE_MCAST_KEY_NOTIFY);
	wai_hdr->sequenceNum = ++wapiInfo->waiTxSeq;

	/*	set unicast request pkt	*/
	wai_mcast_noti = (wapiWaiMCastNofiPkt*)(((unsigned char*)wai_hdr) + wai_hdr->length);
	wai_mcast_noti->reserved1 = wai_mcast_noti->reserved2 = 0;
	wai_mcast_noti->delKeyFlag = wai_mcast_noti->staKeyFlag = 0;
	if (wapiInfo->wapiState==ST_WAPI_AE_USKA_ESTABLISH
		||wapiInfo->wapiState==ST_WAPI_AE_MSK_NOTIFICATION_SNT)
	{	/*	first time	*/
		wai_mcast_noti->mskId = priv->wapiMCastKeyId;
	}
	else
	{
		wapiAssert(wapiInfo->priv->wapiMCastKeyUpdate==1);
		/*	update key	*/
		wai_mcast_noti->mskId = !priv->wapiMCastKeyId;
	}
	
	wai_mcast_noti->uskId = wapiInfo->wapiUCastKeyId;
	
	memcpy(wai_mcast_noti->mac1, priv->dev->dev_addr, ETH_ALEN);
	memcpy(wai_mcast_noti->mac2, pstat->hwaddr, ETH_ALEN);
	memcpy(wai_mcast_noti->dataPN, priv->txMCast, WAPI_PN_LEN);
	memcpy(wai_mcast_noti->keyPN, priv->keyNotify, WAPI_PN_LEN);
	wai_hdr->length += sizeof(wapiWaiMCastNofiPkt);
	wapiAssert(wai_hdr->length<MAXDATALEN);

	WAPI_LOCK(&priv->pshare->lock);
	
	WapiSMS4ForMNKEncrypt(wapiInfo->wapiWaiKey.dataKey, 
		priv->keyNotify, 
		priv->wapiNMK, WAPI_KEY_LEN, 
		wai_mcast_noti->keyData+1, wai_mcast_noti->keyData, 	/* the first byet was used to record len	*/
		ENCRYPT);

	WAPI_UNLOCK(&priv->pshare->lock);
	
	wapiAssert(*wai_mcast_noti->keyData==WAPI_KEY_LEN);
	wai_hdr->length += (*wai_mcast_noti->keyData)+1;
	wapiAssert(wai_hdr->length<MAXDATALEN);

	sha256_hmac(wapiInfo->wapiWaiKey.micKey, WAPI_KEY_LEN, 
		(uint8*)wai_mcast_noti, wai_hdr->length-WAI_HEADER_LEN,
		(((uint8*)wai_mcast_noti)+wai_hdr->length-WAI_HEADER_LEN), 
		WAI_MIC_LEN);

	wai_hdr->length += WAI_MIC_LEN;
	wapiAssert(wai_hdr->length<MAXDATALEN);
	skb_put(pskb, wai_hdr->length);

	/*	14 = DA|SA|ETHER_TYPE|	*/
	if (pskb->len-14>(priv->dev->mtu-WLAN_HDR_A3_QOS_LEN))
	{
		if (wapiFragementSend(pskb, priv)!=SUCCESS)
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	else
	{
		if (rtl8192cd_start_xmit(pskb, priv->dev))
		{
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}
	}
	WAPI_LOCK(&wapiInfo->lock);
	if (wapiInfo->wapiState==ST_WAPI_AE_USKA_ESTABLISH)
		wapiInfo->wapiState = ST_WAPI_AE_MSK_NOTIFICATION_SNT;
	WAPI_UNLOCK(&wapiInfo->lock);

updateTimer:
	WAPI_LOCK(&wapiInfo->lock);
#if 0
	wapiInfo->waiResendTimer.expires = timeout;
	printk("[%s][%d] timer [0x%p]\n", __FUNCTION__, __LINE__, &(wapiInfo->waiResendTimer));
	add_timer(&(wapiInfo->waiResendTimer));
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);
#else
	mod_timer(&wapiInfo->waiResendTimer, timeout);
#endif
	WAPI_UNLOCK(&wapiInfo->lock);
	return WAPI_RETURN_SUCCESS;
}

int	wapiUpdateUSK(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	wapiStaInfo		*wapiInfo;

	WAPI_LOCK(&wapiInfo->lock);
	wapiInfo = pstat->wapiInfo;
	wapiInfo->wapiUCastKeyUpdate = 1;
	wapiInfo->waiRxSeq = 0;
	wapiReleaseFragementQueue(wapiInfo);

	mod_timer(&wapiInfo->waiUCastKeyUpdateTimer, jiffies + WAPI_KEY_UPDATE_TIMEOUT);
	WAPI_UNLOCK(&wapiInfo->lock);
	return wapiSendUnicastKeyAgrementRequeset(priv, pstat);
}

int	wapiUpdateMSK(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct list_head	*plist;
	struct stat_info	*pstat1;
	unsigned int	index;
	unsigned long		timeout;

	wapiAssert(pstat==NULL);
	WAPI_LOCK(&priv->pshare->lock);

	timeout = priv->pmib->wapiInfo.wapiUpdateMCastKeyTimeout*HZ;
	if (list_empty(&priv->asoc_list))
		goto updateTimer;

	priv->wapiMCastKeyUpdate  = 0;
	
	for(index=0;index<NUM_STAT;index++)
	{
		plist = &priv->stat_hash[index];

		while (plist->next != &(priv->stat_hash[index]))
		{
			plist = plist->next;
			pstat1 = list_entry(plist, struct stat_info ,hash_list);

			if (pstat1 == NULL)
			{
				break;
			}

			if (pstat1->wapiInfo->wapiState<ST_WAPI_AE_MSKA_ESTABLISH)
			{
				issue_deauth(priv, pstat1->hwaddr, _RSON_USK_HANDSHAKE_TIMEOUT_);
				continue;
			}

			priv->wapiMCastKeyUpdate = 1;
			pstat1->wapiInfo->wapiMCastKeyUpdateDone = 0;
			pstat1->wapiInfo->waiRxSeq = 0;
			wapiReleaseFragementQueue(pstat1->wapiInfo);
			wapiSendMulticastKeyNotification(priv, pstat1);

			if (plist == plist->next)
				break;
		}
	}

	if (priv->wapiMCastKeyUpdate==1)
	{
		timeout = WAPI_KEY_UPDATE_TIMEOUT;
		/*	set NMK	*/
		GenerateRandomData(priv->wapiNMK, WAPI_KEY_LEN);
		priv->wapiMCastKeyUpdateAllDone = 0;
	}
updateTimer:
	del_timer(&priv->waiMCastKeyUpdateTimer);
	init_timer(&priv->waiMCastKeyUpdateTimer);
	priv->waiMCastKeyUpdateTimer.data = (unsigned long)priv;
	priv->waiMCastKeyUpdateTimer.function = wapiMCastUpdateKeyTimeout;

	if (priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_time_update||
		priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_all_update)
	{
		mod_timer(&priv->waiMCastKeyUpdateTimer, jiffies + timeout);
	}

	WAPI_UNLOCK(&priv->pshare->lock);
	return SUCCESS;
}
#endif	/*CONFIG_RTL_WAPI_SUPPORT*/
