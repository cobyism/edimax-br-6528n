#if	!defined(WAPI_WAI_H)
#define		WAPI_WAI_H	1
#include "8192cd_cfg.h"

#if defined(CONFIG_RTL_WAPI_SUPPORT)

#ifdef __LINUX_2_6__
#ifdef CONFIG_RTL8672
#include "./romeperf.h"
#else
#include <common/rtl_types.h>
#endif
#else
#include "../rtl865x/rtl_types.h"
#endif
#include <linux/timer.h>


#ifndef ETH_ALEN
#define ETH_ALEN    6
#endif

#ifdef __KERNEL__
#include <linux/spinlock.h>
#if 0
extern int	wapi_lock_cnt;
#define	WAPI_LOCK_INIT(__lock__)		do {spin_lock_init(__lock__);} while(0)
#define	WAPI_LOCK(__lock__)			do {spin_lock(__lock__); if(wapi_lock_cnt>0) printk("[%s][%d]: wapi lock cnt %d\n", wapi_lock_cnt); wapi_lock_cnt++;} while(0)
#define	WAPI_UNLOCK(__lock__)		do {spin_unlock(__lock__); wapi_lock_cnt--; if (wapi_lock_cnt<0) printk("[%s][%d]: wapi lock cnt %d\n", wapi_lock_cnt);} while(0)
#else
#define	WAPI_LOCK_INIT(__lock__)
#define	WAPI_LOCK(__lock__)	
#define	WAPI_UNLOCK(__lock__)
#endif
#endif

#define	ETH_P_WAPI			0x88B4

#if 0
#define	FL_TRACE(__FLG__, __NUM__)	printk("[%s][%d]:-[%s][0x%x]\n", __FUNCTION__, __LINE__, __FLG__, __NUM__)
#define	wapiAssert(__FLAGS__)	do {if(!(__FLAGS__)) printk("Assert Error!! ==>[%s][%d]-[%s]\n", __FUNCTION__, __LINE__, __FILE__);} while(0)
#else
#define	wapiAssert(__FLAGS__)
#define	FL_TRACE(__FLG__, __NUM__)
#endif

#ifdef	__LINUX_2_6__
#define	WAPI_ALIGNMENT_OFFSET	2
#else
#define	WAPI_ALIGNMENT_OFFSET	0
#endif
#define	WAPI_CERT_MAX_LEN		2688
#define	SMS4_MIC_LEN		16
#define	WAPI_EXT_LEN		18
#define	WAPI_PN_LEN		16
#define	WAPI_KEY_LEN		16
#define	WAPI_N_LEN			32		/*	wapi challange length	*/
#define	WAPI_KEY_MNG_PKT_LEN		4
#define	WAPI_AKM_OFFSET	4
#define	WAPI_WAI_MAX_SEND_RETRY		7
//#define	WAPI_WAI_HEADER_PADDING		(32+sizeof(wapiWaiHeader))	/*	wlan header 26, LLC header 6, ether type 2	*/
#define	WAPI_WAI_HEADER_PADDING		(34)	/*	wlan header 26, LLC header 6, ether type 2	*/

#define	WAPI_RETRY_COUNT			6	/*	max 64	*/
#define	WAPI_CERT_REQ_TIMEOUT	10*HZ
#define	WAPI_AUTH_REQ_TIMEOUT	31*HZ
#define	WAPI_GENERAL_TIMEOUT		HZ
#define	WAPI_KEY_UPDATE_TIMEOUT		6*HZ
#define	WAPI_KEY_UPDATE_PERIOD		600*HZ
#define	WAPI_KEY_UPDATE_PKTCNT		65536

#define	WAPI_RETURN_SUCCESS		0
#define	WAPI_RETURN_FAILED		-1
#define	WAPI_RETURN_DEASSOC		-2

#define	RX_QUEUE_NUM				4

typedef	enum __wapiMibType	wapiStaType;

typedef	enum __wapiMibEncryptAlgorithm	wapiStaEncryptAlgorithm;

typedef	struct	__wapiKey {
	unsigned char		dataKey[WAPI_KEY_LEN];	/*	used as key for BK		*/
											/*	used as MCastKeyEncrypt Key for WAI */
	unsigned char		micKey[WAPI_KEY_LEN];	/*	used as keyID for BK	*/
											/*	used as UCast MIC Key for WAI */
#if defined(WAPI_SUPPORT_MULTI_ENCRYPT)
	wapiStaEncryptAlgorithm		keyType;
#endif
}	wapiKey;

#define	WAPI_IOCTL_TYPE_ACTIVEAUTH	0
#define	WAPI_IOCTL_TYPE_SETBK			1
#define	WAPI_IOCTL_TYPE_AUTHRSP		2

#define	WAPI_IOCTL_TYPE_REQ_ACTIVE	3
#define	WAPI_IOCTL_TYPE_CA_AUTH		4

typedef	struct	__wapiCAAppPara {
	uint8			eventID;
	uint8			moreData;
	uint16			type;
	void				*ptr;
	char				name[IFNAMSIZ];
	uint8			data[0];
}	wapiCAAppPara;

typedef	struct	__wapiStaPN {
	unsigned char		rxUCast[RX_QUEUE_NUM][WAPI_PN_LEN];
	unsigned char		txUCast[WAPI_PN_LEN];
	unsigned short		rxSeq[RX_QUEUE_NUM];
}	wapiStaPN;

/*	AE STATE during WAI Handshake	*/
typedef enum __WAPI_AE_STATE { 
	ST_WAPI_AE_IDLE=0, 
	ST_WAPI_AE_ACTIVE_AUTHENTICATION_REQ,
	ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT,
	ST_WAPI_AE_ACCESS_AUTHENTICATE_REQ_RCVD,
	ST_WAPI_AE_ACCESS_CERTIFICATE_REQ_SNT,
	ST_WAPI_AE_BKSA_ESTABLISH,
	ST_WAPI_AE_USK_AGGREMENT_REQ_SNT,
	ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD,
	ST_WAPI_AE_USKA_ESTABLISH,
	ST_WAPI_AE_MSK_NOTIFICATION_SNT,
	ST_WAPI_AE_MSK_RSP_RCVD,
	ST_WAPI_AE_MSKA_ESTABLISH
}	WAPI_AE_STATE;

typedef	struct	__wapiStaInfo {
	uint8			asueWapiIE[256];
	uint8			aeWapiIE[256];
	uint8			waiASUEChallange[WAPI_N_LEN];
	uint8			waiAEChallange[WAPI_N_LEN];
	uint8			waiAuthFlag[WAPI_N_LEN];
	uint8			asueWapiIELength;
	uint8			aeWapiIELength;
	uint8			wapiRxFragSeq;
	uint8			wapiUCastKeyId:1;
	uint8			wapiUCastRxEnable:1;
	uint8			wapiUCastTxEnable:1;
	uint8			wapiUCastKeyUpdate:1;
	uint8			wapiMCastEnable:1;
	uint8			wapiMCastKeyUpdateDone:1;
	uint8			wapiRetry:6;
	uint16			waiTxSeq;
	uint16			waiRxSeq;
	wapiStaType		wapiType;
	WAPI_AE_STATE	wapiState;
	wapiStaPN		wapiPN;
	wapiKey			wapiBK;
	wapiKey			wapiUCastKey[2];
	wapiKey			wapiWaiKey;
	uint32			wapiRxFragLen;
	uint32			waiCertRspDataLen;
	uint32			waiCertCachedDataLen;
	unsigned long		wapiUCastKeyUpdateCnt;
	struct timer_list	waiResendTimer;
	struct timer_list	waiUCastKeyUpdateTimer;
	struct sk_buff		*wapiRxFragPskb;
	struct rtl8192cd_priv	*priv;
	uint8			*waiCertCachedData;
	uint8			*waiCertRspData;
#ifdef __KERNEL__
	spinlock_t				lock;
#endif
}	wapiStaInfo;

typedef enum	__wapiWaiProtocolVersion {
	WAI_V1 = 1
}	wapiWaiProtocolVersion;
typedef enum	__wapiWaiType {
	WAI_TYPE_WAI = 1
}	wapiWaiType;
typedef enum	__wapiWaiSubType {
	WAI_SUBTYPE_PREAUTH = 1,
	WAI_SUBTYPE_STA_KEY_REQ = 2,
	WAI_SUBTYPE_AUTH_ACTIVE = 3,
	WAI_SUBTYPE_AUTH_REQ = 4,
	WAI_SUBTYPE_AUTH_RSP = 5,
	WAI_SUBTYPE_CERT_REQ = 6,
	WAI_SUBTYPE_CERT_RSP =7,
	WAI_SUBTYPE_UCAST_KEY_REQ = 8,
	WAI_SUBTYPE_UCAST_KEY_RSP = 9,
	WAI_SUBTYPE_UCAST_KEY_ACK = 10,
	WAI_SUBTYPE_MCAST_KEY_NOTIFY = 11,
	WAI_SUBTYPE_MCAST_KEY_RSP = 12
}	wapiWaiSubType;

#define	WAI_HEADER_MF		0x1
#define	WAI_HEADER_LEN	12
#define	WAI_MIC_LEN			20

typedef struct __wapiTLV {
	uint16	id;
	uint16	len;
	uint8	data[0];
}	wapiTLV;

typedef struct __wapiTLV1 {
	uint8	id;
	uint16	len;
	uint8	data[0];
}	__attribute__ ((packed))	wapiTLV1;

typedef struct __wapiWaiHeader {
	uint16	protocolVersion;
	uint8	type;
	uint8	subType;
	uint16	reserved;
	uint16	length;
	uint16	sequenceNum;
	uint8	fragmentNum;
	uint8	flags;
}	wapiWaiHeader;

typedef struct	__wapiWaiCertActivPkt {
	/*	8Bit flags	*/
	uint8	reserved:6;
	uint8	preAuth:1;
	uint8	updateBK:1;
	/*	8Bit flags over	*/
	uint8	authFlag[WAPI_N_LEN];
	uint8	data[0];
}	wapiWaiCertActivPkt;

typedef struct	__wapiWaiCertAuthReqPkt {
	/*	8Bit flags	*/
	uint8	reserved:4;
	uint8	options:1;
	uint8	AEAuthReq:1;
	uint8	preAuth:1;
	uint8	updateBK:1;
	/*	8Bit flags over	*/
	uint8	authFlag[WAPI_N_LEN];
	uint8	ASUEChallange[WAPI_N_LEN];
	uint8	data[0];
}	wapiWaiCertAuthReqPkt;

typedef struct	__wapiWaiCertAuthRspPkt {
	/*	8Bit flags	*/
	uint8	reserved1:4;
	uint8	options:1;
	uint8	reserved2:1;
	uint8	preAuth:1;
	uint8	updateBK:1;
	uint8	ASUEChallange[WAPI_N_LEN];
	uint8	AEChallange[WAPI_N_LEN];
	uint8	CAResult;
	uint8	data[0];
}	wapiWaiCertAuthRspPkt;

typedef struct	__wapiWaiUCastReqPkt {
	/*	8Bit flags	*/
	uint8	reserved1:3;
	uint8	uskUpdate:1;
	uint8	reserved2:4;
	/*	8Bit flags over	*/
	uint8	bkId[WAPI_KEY_LEN];
	uint8	uskId;
	uint8	mac1[ETH_ALEN];
	uint8	mac2[ETH_ALEN];
	uint8	AEChallange[WAPI_N_LEN];
}	wapiWaiUCastReqPkt;

typedef struct	__wapiWaiUCastRspPkt {
	/*	8Bit flags	*/
	uint8	reserved1:3;
	uint8	uskUpdate:1;
	uint8	reserved2:4;
	/*	8Bit flags over	*/
	uint8	bkId[WAPI_KEY_LEN];
	uint8	uskId;
	uint8	mac1[ETH_ALEN];
	uint8	mac2[ETH_ALEN];
	uint8	ASUEChallange[WAPI_N_LEN];
	uint8	AEChallange[WAPI_N_LEN];
	uint8	WIEasue[0];
}	wapiWaiUCastRspPkt;

typedef struct	__wapiWaiUCastAckPkt {
	/*	8Bit flags	*/
	uint8	reserved1:3;
	uint8	uskUpdate:1;
	uint8	reserved2:4;
	/*	8Bit flags over	*/
	uint8	bkId[WAPI_KEY_LEN];
	uint8	uskId;
	uint8	mac1[ETH_ALEN];
	uint8	mac2[ETH_ALEN];
	uint8	ASUEChallange[WAPI_N_LEN];
	uint8	WIEae[0];
}	wapiWaiUCastAckPkt;

typedef struct	__wapiWaiMCastNotiPkt {
	/*	8Bit flags	*/
	uint8	reserved1:1;
	uint8	delKeyFlag:1;
	uint8	staKeyFlag:1;
	uint8	reserved2:5;
	/*	8Bit flags over	*/
	uint8	mskId;
	uint8	uskId;
	uint8	mac1[ETH_ALEN];
	uint8	mac2[ETH_ALEN];
	uint8	dataPN[WAPI_PN_LEN];
	uint8	keyPN[WAPI_PN_LEN];
	uint8	keyData[0];
}	wapiWaiMCastNofiPkt;

typedef struct	__wapiWaiMCastRspPkt {
	/*	8Bit flags	*/
	uint8	reserved1:1;
	uint8	delKeyFlag:1;
	uint8	staKeyFlag:1;
	uint8	reserved2:5;
	/*	8Bit flags over	*/
	uint8	mskId;
	uint8	uskId;
	uint8	mac1[ETH_ALEN];
	uint8	mac2[ETH_ALEN];
	uint8	keyPN[WAPI_PN_LEN];
	uint8	mic[0];
}	wapiWaiMCastRspPkt;

void wapiInit(struct rtl8192cd_priv *priv);
void wapiExit(struct rtl8192cd_priv *priv);
void wapiStationInit(struct stat_info *pstat);
void wapiSetIE(struct rtl8192cd_priv	*priv);
int	wapiIEInfoInstall(struct rtl8192cd_priv *priv, struct stat_info *pstat);
void wapiReleaseFragementQueue(wapiStaInfo *wapiInfo);
void wapiReqActiveCA(struct stat_info *pstat);
void	wapiSetBK(struct stat_info *pstat);
int32 WapiComparePN(uint8 *PN1, uint8 *PN2);
int32 WapiIncreasePN(uint8 *PN, uint8 AddCount);
int wapiHandleRecvPacket(struct rx_frinfo *pfrinfo, struct stat_info *pstat);
int	wapiSendActiveAuthenticationPacket(struct rtl8192cd_priv *priv, struct stat_info *pstat, int len, uint8 *data);
int	wapiSendUnicastKeyAgrementRequeset(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int	wapiSendUnicastKeyAgrementConfirm(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int	wapiSendMulticastKeyNotification(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int	wapiUpdateUSK(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int	wapiUpdateMSK(struct rtl8192cd_priv *priv, struct stat_info *pstat);

extern struct timer_list	waiMCastKeyUpdateTimer;
#endif	/*	CONFIG_RTL_WAPI_SUPPORT	*/

#endif	/*	WAPI_WAI_H			*/
