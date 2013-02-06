/*
 *  Handle routines for proc file system
 *
 *  $Id: 8192cd_proc.c,v 1.34.2.21 2011/04/19 09:16:07 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_PROC_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/compile.h>
#include <linux/init.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./ieee802_mib.h"
#include "./8192cd_headers.h"

#ifdef CONFIG_RTK_MESH
#include "./mesh_ext/mesh_route.h"
#include "./mesh_ext/mesh_util.h"
#endif // CONFIG_RTK_MESH
#ifdef _INCLUDE_PROC_FS_
#include <asm/uaccess.h>

/*
 *	Note : These define someone copy to ./mesh_ext/mesh_proc.c !!
*/
#define PRINT_ONE(val, format, line_end) { 		\
	pos += sprintf(&buf[pos], format, val);		\
	if (line_end)					\
		strcat(&buf[pos++], "\n");		\
}

#define PRINT_ARRAY(val, format, len, line_end) { 	\
	int index;					\
	for (index=0; index<len; index++)		\
		pos += sprintf(&buf[pos], format, val[index]); \
	if (line_end)					\
		strcat(&buf[pos++], "\n");		\
							\
}

#define PRINT_SINGL_ARG(name, para, format) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ONE(para, format, 1); \
}

#define PRINT_ARRAY_ARG(name, para, format, len) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ARRAY(para, format, len, 1); \
}

#define CHECK_LEN { \
	len += size; \
	pos = begin + len; \
	if (pos < offset) { \
		len = 0; \
		begin = pos; \
	} \
	if (pos > offset + length) \
		goto _ret; \
}


/*const unsigned short MCS_DATA_RATE[2][2][16] =
{
	{{13, 26, 39, 52, 78, 104, 117, 130, 26, 52, 78 ,104, 156, 208, 234, 260},		// Long GI, 20MHz
	 {14, 29, 43, 58, 87, 116, 130, 144, 29, 58, 87, 116, 173, 231, 260, 289}},		// Short GI, 20MHz
	{{27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540},	// Long GI, 40MHz
	 {30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600}}	// Short GI, 40MHz
};*/


const unsigned char* MCS_DATA_RATEStr[2][2][16] =
{
	{{"6.5", "13", "19.5", "26", "39", "52", "58.5", "65", "13", "26", "39" ,"52", "78", "104", "117", "130"},		// Long GI, 20MHz
	 {"7.2", "14.4", "21.7", "28.9", "43.3", "57.8", "65", "72.2", "14.4", "28.9", "43.3", "57.8", "86.7", "115.6", "130", "144.5"}},		// Short GI, 20MHz
	{{"13.5", "27", "40.5", "54", "81", "108", "121.5", "135", "27", "54", "81", "108", "162", "216", "243", "270"},	// Long GI, 40MHz
	 {"15", "30", "45", "60", "90", "120", "135", "150", "30", "60", "90", "120", "180", "240", "270", "300"}}	// Short GI, 40MHz
};


static int rtl8192cd_proc_mib_staconfig(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0, i;
	unsigned char tmpbuf[100];

	PRINT_ONE("  Dot11StationConfigEntry...", "%s", 1);
	PRINT_ARRAY_ARG("    dot11Bssid: ",
			priv->pmib->dot11StationConfigEntry.dot11Bssid, "%02x", 6);

	memcpy(tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen);
	tmpbuf[priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen] = '\0';
	PRINT_ONE("    dot11DesiredSSID:(Len ", "%s", 0);
	PRINT_ONE(priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen, "%d) ", 0);
	PRINT_ONE(tmpbuf, "%s", 1);

	memcpy(tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DefaultSSID, priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen);
	tmpbuf[priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen] = '\0';
	PRINT_ONE("    dot11DefaultSSID:(Len ", "%s", 0);
	PRINT_ONE(priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen, "%d) ", 0);
	PRINT_ONE(tmpbuf, "%s", 1);

	memcpy(tmpbuf, priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan, priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen);
	tmpbuf[priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen] = '\0';
	PRINT_ONE("    dot11SSIDtoScan:(Len ", "%s", 0);
	PRINT_ONE(priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen, "%d) ", 0);
	PRINT_ONE(tmpbuf, "%s", 1);

	PRINT_ARRAY_ARG("    dot11DesiredBssid: ",
			priv->pmib->dot11StationConfigEntry.dot11DesiredBssid, "%02x", 6);
	PRINT_ARRAY_ARG("    dot11OperationalRateSet: ",
			priv->pmib->dot11StationConfigEntry.dot11OperationalRateSet, "%02x",
			priv->pmib->dot11StationConfigEntry.dot11OperationalRateSetLen);
	PRINT_SINGL_ARG("    dot11OperationalRateSetLen: ",
			priv->pmib->dot11StationConfigEntry.dot11OperationalRateSetLen, "%d");
	PRINT_SINGL_ARG("    dot11BeaconPeriod: ",
			priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod, "%d");
	PRINT_SINGL_ARG("    dot11DTIMPeriod: ",
			priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod, "%d");
	PRINT_SINGL_ARG("    dot11swcrypto: ",
			priv->pmib->dot11StationConfigEntry.dot11swcrypto, "%d");
	PRINT_SINGL_ARG("    dot11AclMode: ",
			priv->pmib->dot11StationConfigEntry.dot11AclMode, "%d");
	PRINT_SINGL_ARG("    dot11AclNum: ",
			priv->pmib->dot11StationConfigEntry.dot11AclNum, "%d");

	for (i=0; i<priv->pmib->dot11StationConfigEntry.dot11AclNum; i++) {
		sprintf(tmpbuf, "    dot11AclAddr[%d]: ", i);
		PRINT_ARRAY_ARG(tmpbuf,	priv->pmib->dot11StationConfigEntry.dot11AclAddr[i], "%02x", 6);
	}

	PRINT_SINGL_ARG("    dot11SupportedRates: ",
			priv->pmib->dot11StationConfigEntry.dot11SupportedRates, "0x%x");
	PRINT_SINGL_ARG("    dot11BasicRates: ",
			priv->pmib->dot11StationConfigEntry.dot11BasicRates, "0x%x");
	PRINT_SINGL_ARG("    dot11RegDomain: ",
			priv->pmib->dot11StationConfigEntry.dot11RegDomain, "%d");
	PRINT_SINGL_ARG("    autoRate: ",
			priv->pmib->dot11StationConfigEntry.autoRate, "%d");
	PRINT_SINGL_ARG("    fixedTxRate: ",
			priv->pmib->dot11StationConfigEntry.fixedTxRate, "0x%x");
	PRINT_SINGL_ARG("    swTkipMic: ",
			priv->pmib->dot11StationConfigEntry.swTkipMic, "%d");
	PRINT_SINGL_ARG("    protectionDisabled: ",
			priv->pmib->dot11StationConfigEntry.protectionDisabled, "%d");
	PRINT_SINGL_ARG("    olbcDetectDisabled: ",
			priv->pmib->dot11StationConfigEntry.olbcDetectDisabled, "%d");
	PRINT_SINGL_ARG("    legacySTADeny: ",
			priv->pmib->dot11StationConfigEntry.legacySTADeny, "%d");
#ifdef CLIENT_MODE
	PRINT_SINGL_ARG("    fastRoaming: ",
			priv->pmib->dot11StationConfigEntry.fastRoaming, "%d");
#endif
	PRINT_SINGL_ARG("    lowestMlcstRate: ",
			priv->pmib->dot11StationConfigEntry.lowestMlcstRate, "%d");
	PRINT_SINGL_ARG("    supportedStaNum: ",
			priv->pmib->dot11StationConfigEntry.supportedStaNum, "%d");

	return pos;
}


static int rtl8192cd_proc_mib_auth(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  Dot1180211AuthEntry...", "%s", 1);
	PRINT_SINGL_ARG("    dot11AuthAlgrthm: ",
			priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm, "%d");
	PRINT_SINGL_ARG("    dot11PrivacyAlgrthm: ",
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm, "%d");
	PRINT_SINGL_ARG("    dot11PrivacyKeyIndex: ",
			priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex, "%d");
	PRINT_SINGL_ARG("    dot11PrivacyKeyLen: ",
			priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen, "%d");
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	PRINT_SINGL_ARG("    dot11EnablePSK: ",
			priv->pmib->dot1180211AuthEntry.dot11EnablePSK, "%d");
	PRINT_SINGL_ARG("    dot11WPACipher: ",
			priv->pmib->dot1180211AuthEntry.dot11WPACipher, "%d");
#ifdef RTL_WPA2
	PRINT_SINGL_ARG("    dot11WPA2Cipher: ",
			priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher, "%d");
#endif
	PRINT_SINGL_ARG("    dot11PassPhrase: ",
			priv->pmib->dot1180211AuthEntry.dot11PassPhrase, "%s");
	PRINT_SINGL_ARG("    dot11PassPhraseGuest: ",
			priv->pmib->dot1180211AuthEntry.dot11PassPhraseGuest, "%s");
	PRINT_SINGL_ARG("    dot11GKRekeyTime: ",
			priv->pmib->dot1180211AuthEntry.dot11GKRekeyTime, "%ld");
#endif

	PRINT_ONE("  Dot118021xAuthEntry...", "%s", 1);
	PRINT_SINGL_ARG("    dot118021xAlgrthm: ",
			priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm, "%d");
	PRINT_SINGL_ARG("    dot118021xDefaultPort: ",
			priv->pmib->dot118021xAuthEntry.dot118021xDefaultPort, "%d");
	PRINT_SINGL_ARG("    dot118021xcontrolport: ",
			priv->pmib->dot118021xAuthEntry.dot118021xcontrolport, "%d");

	PRINT_ONE("  Dot11RsnIE...", "%s", 1);
	PRINT_ARRAY_ARG("    rsnie: ",
			priv->pmib->dot11RsnIE.rsnie, "%02x", priv->pmib->dot11RsnIE.rsnielen);
	PRINT_SINGL_ARG("    rsnielen: ", priv->pmib->dot11RsnIE.rsnielen, "%d");

#ifdef CONFIG_RTL_WAPI_SUPPORT
	PRINT_ONE("  Dot1180211WAPIEntry...", "%s", 1);
	PRINT_SINGL_ARG("    dot11EnableWAPI: ",
			priv->pmib->wapiInfo.wapiType, "%d");
#ifdef WAPI_SUPPORT_MULTI_ENCRYPT
	PRINT_SINGL_ARG("    dot11wapiUCastEncodeType: ",
			priv->pmib->wapiInfo.wapiUCastEncodeType, "%d");
	PRINT_SINGL_ARG("    dot11wapiMCastEncodeType: ",
			priv->pmib->wapiInfo.wapiMCastEncodeType, "%d");
#endif
	PRINT_ARRAY_ARG("    dot11wapiPsk: ",
			priv->pmib->wapiInfo.wapiPsk.octet, "%02x", WAPI_PSK_LEN);
	PRINT_SINGL_ARG("    dot11wapiwapiPsklen: ",
			priv->pmib->wapiInfo.wapiPsk.len, "%d");
	PRINT_SINGL_ARG("    dot11wapiUpdateUCastKeyType: ",
			priv->pmib->wapiInfo.wapiUpdateUCastKeyType, "%u");
	PRINT_SINGL_ARG("    dot11wapiUpdateUCastKeyTimeout: ",
			(unsigned int)priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout, "%u");
	PRINT_SINGL_ARG("    dot11wapiUpdateUCastKeyPktNum: ",
			(unsigned int)priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum, "%u");
	PRINT_SINGL_ARG("    dot11wapiUpdateMCastKeyType: ",
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType, "%u");
	PRINT_SINGL_ARG("    dot11wapiUpdateMCastKeyTimeout: ",
			(unsigned int)priv->pmib->wapiInfo.wapiUpdateMCastKeyTimeout, "%u");
	PRINT_SINGL_ARG("    dot11wapiUpdateMCastKeyPktNum: ",
			(unsigned int)priv->pmib->wapiInfo.wapiUpdateMCastKeyPktNum, "%u");
	PRINT_ARRAY_ARG("    dot11wapiTimeout: ",
			priv->pmib->wapiInfo.wapiTimeout, "%08x", wapiTimeoutTotalNum);
#endif

	return pos;
}


static int rtl8192cd_proc_mib_dkeytbl(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  Dot11DefaultKeysTable...", "%s", 1);
	PRINT_ARRAY_ARG("    keytype[0].skey: ",
			priv->pmib->dot11DefaultKeysTable.keytype[0].skey, "%02x", 16);
	PRINT_ARRAY_ARG("    keytype[1].skey: ",
			priv->pmib->dot11DefaultKeysTable.keytype[1].skey, "%02x", 16);
	PRINT_ARRAY_ARG("    keytype[2].skey: ",
			priv->pmib->dot11DefaultKeysTable.keytype[2].skey, "%02x", 16);
	PRINT_ARRAY_ARG("    keytype[3].skey: ",
			priv->pmib->dot11DefaultKeysTable.keytype[3].skey, "%02x", 16);
	return pos;
}


static int rtl8192cd_proc_mib_gkeytbl(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;
	unsigned char *ptr;

	PRINT_ONE("  Dot11GroupKeysTable...", "%s", 1);
	PRINT_SINGL_ARG("    dot11Privacy: ",
			priv->pmib->dot11GroupKeysTable.dot11Privacy, "%d");
	PRINT_SINGL_ARG("    keyInCam: ", (priv->pmib->dot11GroupKeysTable.keyInCam? "yes" : "no"), "%s");
	PRINT_SINGL_ARG("    dot11EncryptKey.dot11TTKeyLen: ",
			priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen, "%d");
	PRINT_SINGL_ARG("    dot11EncryptKey.dot11TMicKeyLen: ",
			priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen, "%d");
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TTKey.skey: ",
			priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey, "%02x", 16);
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TMicKey1.skey: ",
			priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey1.skey, "%02x", 16);
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TMicKey2.skey: ",
			priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey2.skey, "%02x", 16);
	ptr = (unsigned char *)&priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48.val48;
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TXPN48.val48: ", ptr, "%02x", 8);
	ptr = (unsigned char *)&priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11RXPN48.val48;
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11RXPN48.val48: ", ptr, "%02x", 8);

	return pos;
}


static int rtl8192cd_proc_mib_operation(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;
	char tmpbuf[4];
	int idx = 0;

	PRINT_ONE("  Dot11OperationEntry...", "%s", 1);
	PRINT_ARRAY_ARG("    hwaddr: ",	priv->pmib->dot11OperationEntry.hwaddr, "%02x", 6);
	PRINT_SINGL_ARG("    opmode: ", priv->pmib->dot11OperationEntry.opmode, "0x%x");
	PRINT_SINGL_ARG("    hiddenAP: ", priv->pmib->dot11OperationEntry.hiddenAP, "%d");
	PRINT_SINGL_ARG("    dot11RTSThreshold: ", priv->pmib->dot11OperationEntry.dot11RTSThreshold, "%d");
	PRINT_SINGL_ARG("    dot11FragmentationThreshold: ", priv->pmib->dot11OperationEntry.dot11FragmentationThreshold, "%d");
	PRINT_SINGL_ARG("    dot11ShortRetryLimit: ", priv->pmib->dot11OperationEntry.dot11ShortRetryLimit, "%d");
	PRINT_SINGL_ARG("    dot11LongRetryLimit: ", priv->pmib->dot11OperationEntry.dot11LongRetryLimit, "%d");
	PRINT_SINGL_ARG("    expiretime: ", priv->pmib->dot11OperationEntry.expiretime, "%d");
	PRINT_SINGL_ARG("    led_type: ", priv->pmib->dot11OperationEntry.ledtype, "%d");
#ifdef RTL8190_SWGPIO_LED
	PRINT_SINGL_ARG("    led_route: ", priv->pmib->dot11OperationEntry.ledroute, "0x%x");
#endif
	PRINT_SINGL_ARG("    iapp_enable: ", priv->pmib->dot11OperationEntry.iapp_enable, "%d");
	PRINT_SINGL_ARG("    block_relay: ", priv->pmib->dot11OperationEntry.block_relay, "%d");
	PRINT_SINGL_ARG("    deny_any: ", priv->pmib->dot11OperationEntry.deny_any, "%d");
	PRINT_SINGL_ARG("    crc_log: ", priv->pmib->dot11OperationEntry.crc_log, "%d");
	PRINT_SINGL_ARG("    wifi_specific: ", priv->pmib->dot11OperationEntry.wifi_specific, "%d");
#ifdef WIFI_WMM
	PRINT_SINGL_ARG("    qos_enable: ", priv->pmib->dot11QosEntry.dot11QosEnable, "%d");
#ifdef WMM_APSD
	PRINT_SINGL_ARG("    apsd_enable: ", priv->pmib->dot11QosEntry.dot11QosAPSD, "%d");
#endif
#endif

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
		tmpbuf[idx++] = 'A';
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)
		tmpbuf[idx++] = 'B';
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)
		tmpbuf[idx++] = 'G';
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
		tmpbuf[idx++] = 'N';
	tmpbuf[idx] = '\0';
	PRINT_SINGL_ARG("    net_work_type: ", tmpbuf, "%s");

#ifdef TX_SHORTCUT
	PRINT_SINGL_ARG("    disable_txsc: ", priv->pmib->dot11OperationEntry.disable_txsc, "%d");
#endif

#ifdef RX_SHORTCUT
	PRINT_SINGL_ARG("    disable_rxsc: ", priv->pmib->dot11OperationEntry.disable_rxsc, "%d");
#endif

#ifdef BR_SHORTCUT
	PRINT_SINGL_ARG("    disable_brsc: ", priv->pmib->dot11OperationEntry.disable_brsc, "%d");
#endif

	PRINT_SINGL_ARG("    guest_access: ", priv->pmib->dot11OperationEntry.guest_access, "%d");

	return pos;
}


#ifdef DFS
static int rtl8192cd_proc_mib_DFS(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  Dot11DFSEntry...", "%s", 1);

	PRINT_SINGL_ARG("    disable_DFS: ", priv->pmib->dot11DFSEntry.disable_DFS, "%d");
	PRINT_SINGL_ARG("    DFS_timeout: ", priv->pmib->dot11DFSEntry.DFS_timeout, "%d");
	PRINT_SINGL_ARG("    DFS_detected: ", priv->pmib->dot11DFSEntry.DFS_detected, "%d");
	PRINT_SINGL_ARG("    NOP_timeout: ", priv->pmib->dot11DFSEntry.NOP_timeout, "%d");
	PRINT_SINGL_ARG("    rs1_threshold: ", priv->pmib->dot11DFSEntry.rs1_threshold, "%d");
	PRINT_SINGL_ARG("    disable_tx: ", priv->pmib->dot11DFSEntry.disable_tx, "%d");
	PRINT_SINGL_ARG("    Throughput_Threshold: ", priv->pmib->dot11DFSEntry.Throughput_Threshold, "%d");
	PRINT_SINGL_ARG("    RecordHistory_sec: ", priv->pmib->dot11DFSEntry.RecordHistory_sec, "%d");
	PRINT_SINGL_ARG("    temply_disable_DFS: ", priv->pmib->dot11DFSEntry.temply_disable_DFS, "%d");
	PRINT_SINGL_ARG("    Dump_Throughput: ", priv->pmib->dot11DFSEntry.Dump_Throughput, "%d");
	PRINT_SINGL_ARG("    disable_DtmDFS: ", priv->pmib->dot11DFSEntry.disable_DetermineDFSDisable, "%d");

	return pos;
}
#endif


static int rtl8192cd_proc_mib_rf(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;
	unsigned char tmpbuf[16];

	PRINT_ONE("  Dot11RFEntry...", "%s", 1);
	PRINT_SINGL_ARG("    dot11channel: ", priv->pmib->dot11RFEntry.dot11channel, "%d");
	PRINT_SINGL_ARG("    dot11ch_low: ", priv->pmib->dot11RFEntry.dot11ch_low, "%d");
	PRINT_SINGL_ARG("    dot11ch_hi: ", priv->pmib->dot11RFEntry.dot11ch_hi, "%d");
	PRINT_ARRAY_ARG("    pwrlevelCCK_A: ", priv->pmib->dot11RFEntry.pwrlevelCCK_A, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_ARRAY_ARG("    pwrlevelCCK_B: ", priv->pmib->dot11RFEntry.pwrlevelCCK_B, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_ARRAY_ARG("    pwrlevelHT40_1S_A: ", priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_ARRAY_ARG("    pwrlevelHT40_1S_B: ", priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_ARRAY_ARG("    pwrdiffHT40_2S: ", priv->pmib->dot11RFEntry.pwrdiffHT40_2S, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_ARRAY_ARG("    pwrdiffHT20: ", priv->pmib->dot11RFEntry.pwrdiffHT20, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_ARRAY_ARG("    pwrdiffOFDM: ", priv->pmib->dot11RFEntry.pwrdiffOFDM, "%02x", MAX_2G_CHANNEL_NUM);
	PRINT_SINGL_ARG("    shortpreamble: ", priv->pmib->dot11RFEntry.shortpreamble, "%d");
	PRINT_SINGL_ARG("    trswitch: ", priv->pmib->dot11RFEntry.trswitch, "%d");
	PRINT_SINGL_ARG("    disable_ch14_ofdm: ", priv->pmib->dot11RFEntry.disable_ch14_ofdm, "%d");
	PRINT_SINGL_ARG("    xcap: ", priv->pmib->dot11RFEntry.xcap, "%d");
	PRINT_SINGL_ARG("    tssi1: ", priv->pmib->dot11RFEntry.tssi1, "%d");
	PRINT_SINGL_ARG("    tssi2: ", priv->pmib->dot11RFEntry.tssi2, "%d");
	PRINT_SINGL_ARG("    ther: ", priv->pmib->dot11RFEntry.ther, "%d");

	switch (priv->pshare->phw->MIMO_TR_hw_support) {
	case MIMO_1T2R:
		sprintf(tmpbuf, "1T2R");
		break;
	case MIMO_1T1R:
		sprintf(tmpbuf, "1T1R");
		break;
	case MIMO_2T2R:
		sprintf(tmpbuf, "2T2R");
		break;
	default:
		sprintf(tmpbuf, "2T4R");
		break;
	}
	PRINT_SINGL_ARG("    MIMO_TR_hw_support: ", tmpbuf, "%s");

	switch (priv->pmib->dot11RFEntry.MIMO_TR_mode) {
	case MIMO_1T2R:
		sprintf(tmpbuf, "1T2R");
		break;
	case MIMO_1T1R:
		sprintf(tmpbuf, "1T1R");
		break;
	case MIMO_2T2R:
		sprintf(tmpbuf, "2T2R");
		break;
	default:
		sprintf(tmpbuf, "2T4R");
		break;
	}
	PRINT_SINGL_ARG("    MIMO_TR_mode: ", tmpbuf, "%s");

	if (GET_CHIP_VER(priv) == VERSION_8188C) {
		if(IS_UMC_B_CUT_88C(priv))
			sprintf(tmpbuf, "RTL6195B");
		else if(IS_88RE(priv))
			sprintf(tmpbuf, "RTL8188R");
		else
			sprintf(tmpbuf, "RTL8188C");
	}
	else {
		sprintf(tmpbuf, "RTL8192C");
	}
	if (IS_TEST_CHIP(priv))
		strcat(tmpbuf, "t");
	else
		strcat(tmpbuf, "n");

	if(IS_UMC_A_CUT(priv))
		strcat(tmpbuf, "u");

	PRINT_SINGL_ARG("    chipVersion: ", tmpbuf, "%s");

#ifdef EN_EFUSE
	if(priv->pmib->efuseEntry.enable_efuse)	{
		int k;
		PRINT_SINGL_ARG("    autoload fail: ", priv->AutoloadFailFlag, "%d");
		PRINT_SINGL_ARG("    efuse used bytes: ", priv->EfuseUsedBytes, "%d");

		PRINT_ONE("efuse init map...", "%s", 1);
		for(k=0; k<HWSET_MAX_SIZE_92C; k+=16)
			PRINT_ARRAY_ARG("    ", (priv->EfuseMap[EFUSE_INIT_MAP]+k), "%02x", 16);
		PRINT_ONE("efuse modify map...", "%s", 1);
		for(k=0; k<HWSET_MAX_SIZE_92C; k+=16)
			PRINT_ARRAY_ARG("    ", (priv->EfuseMap[EFUSE_MODIFY_MAP]+k), "%02x", 16);
	}
#endif

#ifdef SW_ANT_SWITCH
	PRINT_SINGL_ARG("    SW Ant switch enable: ", (priv->pshare->rf_ft_var.antSw_enable ? "enable" : "disable"), "%s");
	PRINT_SINGL_ARG("    SW Diversity Antenna : ", priv->pshare->DM_SWAT_Table.CurAntenna, "%d");
#endif

#ifdef HW_ANT_SWITCH
	PRINT_SINGL_ARG("    HW Ant switch enable: ", (priv->pshare->rf_ft_var.antHw_enable ? "enable" : "disable"), "%s");
	PRINT_SINGL_ARG("    RxIdle Antenna : ", (priv->pshare->rf_ft_var.CurAntenna==0 ? 2 : 1), "%d");
#endif

	return pos;
}


static int rtl8192cd_proc_mib_bssdesc(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;
	unsigned char tmpbuf[33];

	PRINT_ONE("  bss_desc...", "%s", 1);
	PRINT_ARRAY_ARG("    bssid: ", priv->pmib->dot11Bss.bssid, "%02x", MACADDRLEN);

	memcpy(tmpbuf, priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen);
	tmpbuf[priv->pmib->dot11Bss.ssidlen] = '\0';
	PRINT_SINGL_ARG("    ssid: ", tmpbuf, "%s");

	PRINT_SINGL_ARG("    ssidlen: ", priv->pmib->dot11Bss.ssidlen, "%d");
	PRINT_SINGL_ARG("    bsstype: ", priv->pmib->dot11Bss.bsstype, "%x");
	PRINT_SINGL_ARG("    beacon_prd: ", priv->pmib->dot11Bss.beacon_prd, "%d");
	PRINT_SINGL_ARG("    dtim_prd: ", priv->pmib->dot11Bss.dtim_prd, "%d");
	PRINT_ARRAY_ARG("    t_stamp(hex): ", priv->pmib->dot11Bss.t_stamp, "%08x", 2);
	PRINT_SINGL_ARG("    ibss_par.atim_win: ", priv->pmib->dot11Bss.ibss_par.atim_win, "%d");
	PRINT_SINGL_ARG("    capability(hex): ", priv->pmib->dot11Bss.capability, "%02x");
	PRINT_SINGL_ARG("    channel: ", priv->pmib->dot11Bss.channel, "%d");
	PRINT_SINGL_ARG("    basicrate(hex): ", priv->pmib->dot11Bss.basicrate, "%x");
	PRINT_SINGL_ARG("    supportrate(hex): ", priv->pmib->dot11Bss.supportrate, "%x");
	PRINT_ARRAY_ARG("    bdsa: ", priv->pmib->dot11Bss.bdsa, "%02x", MACADDRLEN);
	PRINT_SINGL_ARG("    rssi: ", priv->pmib->dot11Bss.rssi, "%d");
	PRINT_SINGL_ARG("    sq: ", priv->pmib->dot11Bss.sq, "%d");

	return pos;
}


static int rtl8192cd_proc_mib_erp(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  ERP info...", "%s", 1);
	PRINT_SINGL_ARG("    protection: ", priv->pmib->dot11ErpInfo.protection, "%d");
	PRINT_SINGL_ARG("    nonErpStaNum: ", priv->pmib->dot11ErpInfo.nonErpStaNum, "%d");
	PRINT_SINGL_ARG("    olbcDetected: ", priv->pmib->dot11ErpInfo.olbcDetected, "%d");
	PRINT_SINGL_ARG("    olbcExpired: ", priv->pmib->dot11ErpInfo.olbcExpired, "%d");
	PRINT_SINGL_ARG("    shortSlot: ", priv->pmib->dot11ErpInfo.shortSlot, "%d");
	PRINT_SINGL_ARG("    ctsToSelf: ", priv->pmib->dot11ErpInfo.ctsToSelf, "%d");
	PRINT_SINGL_ARG("    longPreambleStaNum: ", priv->pmib->dot11ErpInfo.longPreambleStaNum, "%d");

	return pos;
}


static int rtl8192cd_proc_cam_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0, i;
	unsigned char TempOutputMac[6];
	unsigned char TempOutputKey[16];
	unsigned short TempOutputCfg=0;

	PRINT_ONE("  CAM info...", "%s", 1);
	PRINT_ONE("    CAM occupied: ", "%s", 0);
	PRINT_ONE(priv->pshare->CamEntryOccupied, "%d", 1);
	for (i=0; i<32; i++)
	{
		PRINT_ONE("    Entry", "%s", 0);
		PRINT_ONE(i, " %2d:", 0);
		CAM_read_entry(priv,i,TempOutputMac,TempOutputKey,&TempOutputCfg);
		PRINT_ARRAY_ARG(" MAC addr: ", TempOutputMac, "%02x", 6);
		PRINT_SINGL_ARG("              Config: ", TempOutputCfg, "%x");
		PRINT_ARRAY_ARG("              Key: ", TempOutputKey, "%02x", 16);
	}

	return pos;
}


#ifdef WDS
static int rtl8192cd_proc_mib_wds(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0, i;
	unsigned char tmpbuf[100];

	PRINT_ONE("  WDS info...", "%s", 1);
	PRINT_SINGL_ARG("    wdsEnabled: ", priv->pmib->dot11WdsInfo.wdsEnabled, "%d");
	PRINT_SINGL_ARG("    wdsPure: ", priv->pmib->dot11WdsInfo.wdsPure, "%d");
	PRINT_SINGL_ARG("    wdsPriority: ", priv->pmib->dot11WdsInfo.wdsPriority, "%d");
	PRINT_SINGL_ARG("    wdsNum: ", priv->pmib->dot11WdsInfo.wdsNum, "%d");
	for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
		sprintf(tmpbuf, "    wdsMacAddr[%d]: ", i);
		PRINT_ARRAY_ARG(tmpbuf,	priv->pmib->dot11WdsInfo.entry[i].macAddr, "%02x", 6);
		PRINT_SINGL_ARG("    wdsTxRate: ", priv->pmib->dot11WdsInfo.entry[i].txRate, "0x%x");
	}
	PRINT_SINGL_ARG("    wdsPrivacy: ", priv->pmib->dot11WdsInfo.wdsPrivacy, "%d");
	PRINT_ARRAY_ARG("    wdsWepKey: ",
			priv->pmib->dot11WdsInfo.wdsWepKey, "%02x", 16);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	PRINT_SINGL_ARG("    wds_passphrase: ",
			priv->pmib->dot11WdsInfo.wdsPskPassPhrase, "%s");
#endif

	return pos;
}
#endif // WDS


#ifdef RTK_BR_EXT
static int rtl8192cd_proc_mib_brext(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  BR Ext info...", "%s", 1);
	PRINT_SINGL_ARG("    nat25_disable: ", priv->pmib->ethBrExtInfo.nat25_disable, "%d");
	PRINT_SINGL_ARG("    macclone_enable: ", priv->pmib->ethBrExtInfo.macclone_enable, "%d");
	PRINT_SINGL_ARG("    dhcp_bcst_disable: ", priv->pmib->ethBrExtInfo.dhcp_bcst_disable, "%d");
	PRINT_SINGL_ARG("    addPPPoETag: ", priv->pmib->ethBrExtInfo.addPPPoETag, "%d");
	PRINT_SINGL_ARG("    nat25sc_disable: ", priv->pmib->ethBrExtInfo.nat25sc_disable, "%d");

	return pos;
}
#endif


static int rtl8192cd_proc_txdesc_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	struct rtl8192cd_hw *phw;
	unsigned long *txdescptr;
	unsigned int q_num = priv->txdesc_num;

	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int i, size;

	phw = GET_HW(priv);
	size = sprintf(buf, "  Tx queue %d descriptor ..........\n", q_num);
	CHECK_LEN;
	if (get_txdesc(phw, q_num)) {
		size = sprintf(buf+len, "  tx_desc%d/physical: 0x%.8x/0x%.8x\n", q_num, (UINT)get_txdesc(phw, q_num),
						*(UINT *)(((UINT)&phw->tx_ring0_addr)+sizeof(unsigned long)*q_num));
		CHECK_LEN;
		size = sprintf(buf+len, "  head/tail: %3d/%-3d  DW0      DW1      DW2      DW3      DW4\n",
			get_txhead(phw, q_num), get_txtail(phw, q_num));
		CHECK_LEN;
		for (i=0; i<NUM_TX_DESC; i++) {
			txdescptr = (unsigned long *)(get_txdesc(phw, q_num) + i);
			size = sprintf(buf+len, "      txdesc%d[%3d]: %.8x %.8x %.8x %.8x %.8x \n", q_num, i,
					(UINT)get_desc(txdescptr[0]), (UINT)get_desc(txdescptr[1]),
					(UINT)get_desc(txdescptr[2]), (UINT)get_desc(txdescptr[3]),
					(UINT)get_desc(txdescptr[4]));
			CHECK_LEN;
		}
	}
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return len;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int rtl8192cd_proc_txdesc_info_write(struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	char tmp[32];

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		int num = sscanf(tmp, "%d", &priv->txdesc_num);

		if (num != 1)
			panic_printk("Invalid tx desc number!\n");
		else if (priv->txdesc_num > 5) {
			panic_printk("Invalid tx desc number!\n");
			priv->txdesc_num = 0;
		}
		else
			panic_printk("Ready to dump tx desc %d\n", priv->txdesc_num);
	}
	return count;
}


static int rtl8192cd_proc_rxdesc_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	struct rtl8192cd_hw *phw;
	unsigned long *rxdescptr;

	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int i, size;

	phw = GET_HW(priv);
	size = sprintf(buf+len, "  Rx queue descriptor ..........\n");
	CHECK_LEN;
	if(phw->rx_descL){
		size = sprintf(buf+len, "  rx_descL/physical: 0x%.8x/0x%.8x\n", (UINT)phw->rx_descL, (UINT)phw->rx_ring_addr);
		CHECK_LEN;
		size = sprintf(buf+len, "  cur_rx: %d\n", phw->cur_rx);
		CHECK_LEN;
		for(i=0; i<NUM_RX_DESC; i++) {
			rxdescptr = (unsigned long *)(phw->rx_descL+i);
			size = sprintf(buf+len, "      rxdesc[%02d]: 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x\n", i,
					(UINT)get_desc(rxdescptr[0]), (UINT)get_desc(rxdescptr[1]),
					(UINT)get_desc(rxdescptr[2]), (UINT)get_desc(rxdescptr[3]),
					(UINT)get_desc(rxdescptr[4]), (UINT)get_desc(rxdescptr[5]),
					(UINT)get_desc(rxdescptr[6]), (UINT)get_desc(rxdescptr[7]));
			CHECK_LEN;
		}
	}
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return len;
}


static int rtl8192cd_proc_desc_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	int pos = 0;

	PRINT_ONE("  descriptor info...", "%s", 1);
	PRINT_ONE("    RX queue:", "%s", 1);
	PRINT_ONE("      rx_descL/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->rx_descL, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->rx_ring_addr, "0x%.8x", 1);
	PRINT_ONE("      RDSAR: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(RX_DESA), "0x%.8x", 0);
	PRINT_ONE("  cur_rx: ", "%s", 0);
	PRINT_ONE((UINT)phw->cur_rx, "%d", 1);

	PRINT_ONE("    queue 0:", "%s", 1);
	PRINT_ONE("      tx_desc0/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc0, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring0_addr, "0x%.8x", 1);
	PRINT_ONE("      TMGDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(MGQ_DESA), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead0, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail0, "%d", 1);

	PRINT_ONE("    queue 1:", "%s", 1);
	PRINT_ONE("      tx_desc1/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc1, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring1_addr, "0x%.8x", 1);
	PRINT_ONE("      TBKDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(BKQ_DESA), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead1, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail1, "%d", 1);

	PRINT_ONE("    queue 2:", "%s", 1);
	PRINT_ONE("      tx_desc2/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc2, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring2_addr, "0x%.8x", 1);
	PRINT_ONE("      TBEDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(BEQ_DESA), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead2, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail2, "%d", 1);

	PRINT_ONE("    queue 3:", "%s", 1);
	PRINT_ONE("      tx_desc3/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc3, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring3_addr, "0x%.8x", 1);
	PRINT_ONE("      TLPDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(VIQ_DESA), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead3, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail3, "%d", 1);

	PRINT_ONE("    queue 4:", "%s", 1);
	PRINT_ONE("      tx_desc4/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc4, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring4_addr, "0x%.8x", 1);
	PRINT_ONE("      TNPDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(VOQ_DESA), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead4, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail4, "%d", 1);

	PRINT_ONE("    queue 5:", "%s", 1);
	PRINT_ONE("      tx_desc5/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc5, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring5_addr, "0x%.8x", 1);
	PRINT_ONE("      THPDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(HQ_DESA), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead5, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail5, "%d", 1);
#if 0
	PRINT_ONE("    RX cmd queue:", "%s", 1);
	PRINT_ONE("      rxcmd_desc/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->rxcmd_desc, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->rxcmd_ring_addr, "0x%.8x", 1);
	PRINT_ONE("      RCDSA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_RCDSA_), "0x%.8x", 0);
	PRINT_ONE("  cur_rx: ", "%s", 0);
	PRINT_ONE((UINT)phw->cur_rxcmd, "%d", 1);

	PRINT_ONE("    TX cmd queue:", "%s", 1);
	PRINT_ONE("      txcmd_desc/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->txcmd_desc, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->txcmd_ring_addr, "0x%.8x", 1);
	PRINT_ONE("      TCDA:  ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TCDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txcmdhead, "%d/", 0);
	PRINT_ONE((UINT)phw->txcmdtail, "%d", 1);
#endif

	return pos;
}


static int rtl8192cd_proc_buf_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  buf info...", "%s", 1);
	PRINT_ONE("    hdr poll:", "%s", 1);
	PRINT_ONE("      head: ", "%s", 0);
	PRINT_ONE((UINT)&priv->pshare->wlan_hdrlist, "0x%.8x", 0);
	PRINT_ONE("    count: ", "%s", 0);
	PRINT_ONE(priv->pshare->pwlan_hdr_poll->count, "%d", 1);

	PRINT_ONE("    hdrllc poll:", "%s", 1);
	PRINT_ONE("      head: ", "%s", 0);
	PRINT_ONE((UINT)&priv->pshare->wlanllc_hdrlist, "0x%.8x", 0);
	PRINT_ONE("    count: ", "%s", 0);
	PRINT_ONE(priv->pshare->pwlanllc_hdr_poll->count, "%d", 1);

	PRINT_ONE("    mgmtbuf poll:", "%s", 1);
	PRINT_ONE("      head: ", "%s", 0);
	PRINT_ONE((UINT)&priv->pshare->wlanbuf_list, "0x%.8x", 0);
	PRINT_ONE("    count: ", "%s", 0);
	PRINT_ONE(priv->pshare->pwlanbuf_poll->count, "%d", 1);

	PRINT_ONE("    icv poll:", "%s", 1);
	PRINT_ONE("      head: ", "%s", 0);
	PRINT_ONE((UINT)&priv->pshare->wlanicv_list, "0x%.8x", 0);
	PRINT_ONE("    count: ", "%s", 0);
	PRINT_ONE(priv->pshare->pwlanicv_poll->count, "%d", 1);

	PRINT_ONE("    mic poll:", "%s", 1);
	PRINT_ONE("      head: ", "%s", 0);
	PRINT_ONE((UINT)&priv->pshare->wlanmic_list, "0x%.8x", 0);
	PRINT_ONE("    count: ", "%s", 0);
	PRINT_ONE(priv->pshare->pwlanmic_poll->count, "%d", 1);

	return pos;
}


#ifdef ENABLE_RTL_SKB_STATS
static int rtl8192cd_proc_skb_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  skb counter...", "%s", 1);
	PRINT_SINGL_ARG("    skb_tx_cnt: ", rtl_atomic_read(&priv->rtl_tx_skb_cnt) , "%d");
	PRINT_SINGL_ARG("    skb_rx_cnt: ", rtl_atomic_read(&priv->rtl_rx_skb_cnt) , "%d");

	return pos;
}
#endif


static int rtl8192cd_proc_mib_11n(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  dot11nConfigEntry...", "%s", 1);

	PRINT_SINGL_ARG("    supportedmcs: ", priv->pmib->dot11nConfigEntry.dot11nSupportedMCS, "%08x");
	PRINT_SINGL_ARG("    basicmcs: ", priv->pmib->dot11nConfigEntry.dot11nBasicMCS, "%08x");
	PRINT_SINGL_ARG("    use40M: ", priv->pmib->dot11nConfigEntry.dot11nUse40M, "%d");
	PRINT_SINGL_ARG("    currBW: ", priv->pshare->is_40m_bw ? "40M":"20M", "%s");
	PRINT_ONE("    2ndchoffset: ", "%s", 0);
	switch (priv->pshare->offset_2nd_chan) {
	case HT_2NDCH_OFFSET_BELOW:
		PRINT_ONE("below", "%s", 1);
		break;
	case HT_2NDCH_OFFSET_ABOVE:
		PRINT_ONE("above", "%s", 1);
		break;
	default:
		PRINT_ONE("dontcare", "%s", 1);
		break;
	}

	PRINT_SINGL_ARG("    shortGI20M: ", priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M, "%d");
	PRINT_SINGL_ARG("    shortGI40M: ", priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M, "%d");
	PRINT_SINGL_ARG("    stbc: ", priv->pmib->dot11nConfigEntry.dot11nSTBC, "%d");
	PRINT_SINGL_ARG("    ampdu: ", priv->pmib->dot11nConfigEntry.dot11nAMPDU, "%d");
	PRINT_SINGL_ARG("    amsdu: ", priv->pmib->dot11nConfigEntry.dot11nAMSDU, "%d");
	PRINT_SINGL_ARG("    ampduSndSz: ", priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz, "%d");
	PRINT_SINGL_ARG("    amsduMax: ", priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax, "%d");
	PRINT_SINGL_ARG("    amsduTimeout: ", priv->pmib->dot11nConfigEntry.dot11nAMSDUSendTimeout, "%d");
	PRINT_SINGL_ARG("    amsduNum: ", priv->pmib->dot11nConfigEntry.dot11nAMSDUSendNum, "%d");
	PRINT_SINGL_ARG("    lgyEncRstrct: ", priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct, "%d");
#ifdef WIFI_11N_2040_COEXIST
	PRINT_SINGL_ARG("    coexist: ", priv->pmib->dot11nConfigEntry.dot11nCoexist, "%d");
	if (priv->pmib->dot11nConfigEntry.dot11nCoexist) {
		PRINT_SINGL_ARG("    bg_ap_timeout: ", priv->bg_ap_timeout, "%d");
		PRINT_SINGL_ARG("    force_20_sta: ", priv->force_20_sta, "0x%08x");
#ifdef STA_EXT
		PRINT_SINGL_ARG("    force_20_sta_ext: ", priv->force_20_sta_ext, "0x%08x");
#endif
		PRINT_SINGL_ARG("    switch_20_sta: ", priv->switch_20_sta, "0x%08x");
#ifdef STA_EXT
		PRINT_SINGL_ARG("    switch_20_sta_ext: ", priv->switch_20_sta_ext, "0x%08x");
#endif
	}
#endif

	if (priv->ht_cap_len) {
		unsigned char *pbuf = (unsigned char *)&priv->ht_cap_buf;
		PRINT_ARRAY_ARG("    ht_cap: ", pbuf, "%02x", priv->ht_cap_len);
	}
	else {
		PRINT_ONE("    ht_cap: none", "%s", 1);
	}
	if (priv->ht_ie_len) {
		unsigned char *pbuf = (unsigned char *)&priv->ht_ie_buf;
		PRINT_ARRAY_ARG("    ht_ie: ", pbuf, "%02x", priv->ht_ie_len);
	}
	else {
		PRINT_ONE("    ht_ie: none", "%s", 1);
	}

	PRINT_SINGL_ARG("    legacy_obss_to: ", priv->ht_legacy_obss_to, "%d");
	PRINT_SINGL_ARG("    legacy_sta_num: ", priv->ht_legacy_sta_num, "%d");
	PRINT_SINGL_ARG("    11nProtection: ", priv->ht_protection, "%d");
//	PRINT_SINGL_ARG("    has_2r_sta: ", priv->pshare->has_2r_sta, "0x%08x");

	return pos;
}


#ifdef MERGE_FW
static int rtl8192cd_proc_phypara_file_read(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int len;

	if (priv->phypara_file_end <= (priv->phypara_file_start + offset + length)) {
		*eof = 1;
		len = priv->phypara_file_end - priv->phypara_file_start - offset;
	}
	else
		len = length;

	memcpy(buf, &priv->phypara_file_start[offset], len);
	*start = buf;
	return len;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int rtl8192cd_proc_phypara_file_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	char tmp[64];
	int len = 0;

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 64)) {
		len = strlen(tmp);
		tmp[len-1] = '\0';

#ifdef TESTCHIP_SUPPORT
		if (!strcmp(tmp, "AGC_TAB.txt")) {
			priv->phypara_file_start = data_AGC_TAB_start;
			priv->phypara_file_end = data_AGC_TAB_end;
		}
		else if (!strcmp(tmp, "PHY_REG_1T.txt")) {
			priv->phypara_file_start = data_PHY_REG_1T_start;
			priv->phypara_file_end = data_PHY_REG_1T_end;
		}
		else if (!strcmp(tmp, "PHY_REG_2T.txt")) {
			priv->phypara_file_start = data_PHY_REG_2T_start;
			priv->phypara_file_end = data_PHY_REG_2T_end;
		}
		else if (!strcmp(tmp, "radio_a_1T.txt")) {
			priv->phypara_file_start = data_radio_a_1T_start;
			priv->phypara_file_end = data_radio_a_1T_end;
		}
		else if (!strcmp(tmp, "radio_a_2T.txt")) {
			priv->phypara_file_start = data_radio_a_2T_start;
			priv->phypara_file_end = data_radio_a_2T_end;
		}
		else if (!strcmp(tmp, "radio_b_2T.txt")) {
			priv->phypara_file_start = data_radio_b_2T_start;
			priv->phypara_file_end = data_radio_b_2T_end;
		}
		else
#endif

		if (!strcmp(tmp, "AGC_TAB_n.txt")) {
			priv->phypara_file_start = data_AGC_TAB_n_start;
			priv->phypara_file_end = data_AGC_TAB_n_end;
		}
		else if (!strcmp(tmp, "PHY_REG_1T_n.txt")) {
			priv->phypara_file_start = data_PHY_REG_1T_n_start;
			priv->phypara_file_end = data_PHY_REG_1T_n_end;
		}
		else if (!strcmp(tmp, "PHY_REG_2T_n.txt")) {
			priv->phypara_file_start = data_PHY_REG_2T_n_start;
			priv->phypara_file_end = data_PHY_REG_2T_n_end;
		}
		else if (!strcmp(tmp, "radio_a_2T_n.txt")) {
			priv->phypara_file_start = data_radio_a_2T_n_start;
			priv->phypara_file_end = data_radio_a_2T_n_end;
		}
		else if (!strcmp(tmp, "radio_b_2T_n.txt")) {
			priv->phypara_file_start = data_radio_b_2T_n_start;
			priv->phypara_file_end = data_radio_b_2T_n_end;
		}
		else if (!strcmp(tmp, "radio_a_1T_n.txt")) {
			priv->phypara_file_start = data_radio_a_1T_n_start;
			priv->phypara_file_end = data_radio_a_1T_n_end;
		}
		else if (!strcmp(tmp, "PHY_REG_PG.txt")) {
			priv->phypara_file_start = data_PHY_REG_PG_start;
			priv->phypara_file_end = data_PHY_REG_PG_end;
		}
		else if (!strcmp(tmp, "MACPHY_REG.txt")) {
			priv->phypara_file_start = data_MACPHY_REG_start;
			priv->phypara_file_end = data_MACPHY_REG_end;
		}
		else if (!strcmp(tmp, "PHY_REG_MP_n.txt")) {
			priv->phypara_file_start = data_PHY_REG_MP_n_start;
			priv->phypara_file_end = data_PHY_REG_MP_n_end;
		}
		else if (!strcmp(tmp, "AGC_TAB_n_hp.txt")) {
			priv->phypara_file_start = data_AGC_TAB_n_hp_start;
			priv->phypara_file_end = data_AGC_TAB_n_hp_end;
		}
		else if (!strcmp(tmp, "PHY_REG_2T_n_hp.txt")) {
			priv->phypara_file_start = data_PHY_REG_2T_n_hp_start;
			priv->phypara_file_end = data_PHY_REG_2T_n_hp_end;
		}
		else if (!strcmp(tmp, "radio_a_2T_n_lna.txt")) {
			priv->phypara_file_start = data_radio_a_2T_n_lna_start;
			priv->phypara_file_end = data_radio_a_2T_n_lna_end;
		}
		else if (!strcmp(tmp, "radio_b_2T_n_lna.txt")) {
			priv->phypara_file_start = data_radio_b_2T_n_lna_start;
			priv->phypara_file_end = data_radio_b_2T_n_lna_end;
		}

#ifdef HIGH_POWER_EXT_PA
		else if (!strcmp(tmp, "radio_a_2T_n_hp.txt")) {
			priv->phypara_file_start = data_radio_a_2T_n_hp_start;
			priv->phypara_file_end = data_radio_a_2T_n_hp_end;
		}
		else if (!strcmp(tmp, "radio_b_2T_n_hp.txt")) {
			priv->phypara_file_start = data_radio_b_2T_n_hp_start;
			priv->phypara_file_end = data_radio_b_2T_n_hp_end;
		}
		else if (!strcmp(tmp, "PHY_REG_PG_hp.txt")) {
			priv->phypara_file_start = data_PHY_REG_PG_hp_start;
			priv->phypara_file_end = data_PHY_REG_PG_hp_end;
		}		
#endif
		else {
			panic_printk("No file of \"%s\"\n", tmp);
			panic_printk("PHY parameter file name list:\n"
#ifdef TESTCHIP_SUPPORT
				"\tAGC_TAB.txt\n"
				"\tPHY_REG_1T.txt\n"
				"\tPHY_REG_2T.txt\n"
				"\tradio_a_1T.txt\n"
				"\tradio_a_2T.txt\n"
				"\tradio_b_2T.txt\n"
#endif
				"\tAGC_TAB_n.txt\n"
				"\tPHY_REG_1T_n.txt\n"
				"\tPHY_REG_2T_n.txt\n"
				"\tradio_a_2T_n.txt\n"
				"\tradio_b_2T_n.txt\n"
				"\tradio_a_1T_n.txt\n"
				"\tPHY_REG_PG.txt\n"
				"\tMACPHY_REG.txt\n"
				"\tPHY_REG_MP_n.txt\n"
				"\tAGC_TAB_n_hp.txt\n"
				"\tPHY_REG_2T_n_hp.txt\n"
				"\tradio_a_2T_n_lna.txt\n"
				"\tradio_b_2T_n_lna.txt\n"
#ifdef HIGH_POWER_EXT_PA
				"\tradio_a_2T_n_hp.txt\n"
				"\tradio_b_2T_n_hp.txt\n"
				"\tPHY_REG_PG_hp.txt\n"
#endif
				);
			return count;
		}
	}

	panic_printk("Ready to dump \"%s\"\n", tmp);
	return count;
}

#endif


#ifdef AUTO_TEST_SUPPORT
static int rtl8192cd_proc_SSR_read(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;
	int i1;
	unsigned char tmp[6];

	//wait SiteSurvey completed
	if(priv->ss_req_ongoing)	{
		PRINT_ONE("waitting", "%s", 1);
	}else{
		PRINT_ONE(" SiteSurvey result : ", "%s", 1);
		PRINT_ONE("    ====================", "%s", 1);
		if(priv->site_survey.count_backup==0){
			PRINT_ONE("none", "%s", 1);
		}else{
			for(i1=0; i1<priv->site_survey.count_backup ;i1++){

				memcpy(tmp,priv->site_survey.bss_backup[i1].bssid,MACADDRLEN);
				/*
				panic_printk("Mac=%02X%02X%02X:%02X%02X%02X ;Channel=%02d ;SSID:%s  \n",
					tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]
					,priv->site_survey.bss_backup[i1].channel
					,priv->site_survey.bss_backup[i1].ssid
					);
				*/
				PRINT_ARRAY_ARG("    HwAddr: ",	tmp, "%02x", MACADDRLEN);
				PRINT_SINGL_ARG("    Channel: ",	priv->site_survey.bss_backup[i1].channel, "%d");
				PRINT_SINGL_ARG("    SSID: ", priv->site_survey.bss_backup[i1].ssid, "%s");
				PRINT_SINGL_ARG("    Type: ", ((priv->site_survey.bss_backup[i1].bsstype == 16) ? "AP" : "Ad-Hoc"), "%s");
				//PRINT_SINGL_ARG("    Type: ", priv->site_survey.bss_backup[i1].bsstype, "%d");
				PRINT_ONE("    ====================", "%s", 1);
			}
		}


	}


	return pos;

}
#endif


#ifdef CONFIG_RTK_VLAN_SUPPORT
static int rtl8192cd_proc_vlan_read(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  vlan setting...", "%s", 1);
	PRINT_SINGL_ARG("    global_vlan: ",
			priv->pmib->vlan.global_vlan, "%d");
	PRINT_SINGL_ARG("    is_lan: ",
			priv->pmib->vlan.is_lan, "%d");
	PRINT_SINGL_ARG("    vlan_enable: ",
			priv->pmib->vlan.vlan_enable, "%d");
	PRINT_SINGL_ARG("    vlan_tag: ",
			priv->pmib->vlan.vlan_tag, "%d");
	PRINT_SINGL_ARG("    vlan_id: ",
			priv->pmib->vlan.vlan_id, "%d");
	PRINT_SINGL_ARG("    vlan_pri: ",
			priv->pmib->vlan.vlan_pri, "%d");
	PRINT_SINGL_ARG("    vlan_cfi: ",
			priv->pmib->vlan.vlan_cfi, "%d");
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	PRINT_SINGL_ARG("    vlan_forwarding: ",
			priv->pmib->vlan.forwarding, "%d");
#endif

	return pos;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int rtl8192cd_proc_vlan_write(struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	char tmp[128];

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		int num = sscanf(tmp, "%d %d %d %d %d %d %d %d",
#else
		int num = sscanf(tmp, "%d %d %d %d %d %d %d",
#endif
			&priv->pmib->vlan.global_vlan, &priv->pmib->vlan.is_lan,
			&priv->pmib->vlan.vlan_enable, &priv->pmib->vlan.vlan_tag,
			&priv->pmib->vlan.vlan_id, &priv->pmib->vlan.vlan_pri,
			&priv->pmib->vlan.vlan_cfi
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
			,&priv->pmib->vlan.forwarding
#endif
			);

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		if (num != 8) {
#else
		if (num !=  7) {
#endif
			panic_printk("invalid vlan parameter!\n");
		}
	}
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	add_vlan_info(&priv->pmib->vlan,dev);
#endif
	return count;
}
#endif // CONFIG_RTK_VLAN_SUPPORT


static int rtl8192cd_proc_mib_all(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int size;

#ifdef __KERNEL__
	size = sprintf(buf, "  Make info: %s by %s, v%d.%d (%s)\n", UTS_VERSION, LINUX_COMPILE_BY,
		DRV_VERSION_H, DRV_VERSION_L, DRV_RELDATE);
	CHECK_LEN;
#endif
	size = sprintf(buf+len, "  RTL8192 firmware version: %d.%d,  built time: %02x-%02x %02x:%02x\n", priv->pshare->fw_version,
		priv->pshare->fw_sub_version, priv->pshare->fw_date_month, priv->pshare->fw_date_day,
		priv->pshare->fw_date_hour, priv->pshare->fw_date_minute);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_rf(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_operation(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_staconfig(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_dkeytbl(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_auth(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_gkeytbl(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_bssdesc(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

	size = rtl8192cd_proc_mib_erp(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

#ifdef WDS
	size = rtl8192cd_proc_mib_wds(buf+len, start, offset, length, eof, data);
	CHECK_LEN;
#endif

#ifdef RTK_BR_EXT
	size = rtl8192cd_proc_mib_brext(buf+len, start, offset, length, eof, data);
	CHECK_LEN;
#endif

#ifdef DFS
	size = rtl8192cd_proc_mib_DFS(buf+len, start, offset, length, eof, data);
	CHECK_LEN;
#endif

	size = rtl8192cd_proc_mib_11n(buf+len, start, offset, length, eof, data);
	CHECK_LEN;

#ifdef CONFIG_RTK_VLAN_SUPPORT
	size = rtl8192cd_proc_vlan_read(buf+len, start, offset, length, eof, data);
	CHECK_LEN;
#endif

	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	return len;
}

#define MAX_CHAR_IN_A_LINE		80
// macro "CHECK_LEN_B" is used in dump_one_stainfo() only
#define CHECK_LEN_B { \
	if (pos > (remained_len - MAX_CHAR_IN_A_LINE)) {\
		return pos; \
	} \
}

//static int dump_one_stainfo(int num, struct stat_info *pstat, char *buf, char **start,
//			off_t offset, int length, int *eof, void *data)
static int dump_one_stainfo(int num, struct stat_info *pstat, char *buf,
			off_t offset, int length, int remained_len, int *rc)
{
	int pos = 0, idx = 0;
	unsigned int m, n;
	char tmp[32];
	//unsigned short rate;
	unsigned char tmpbuf[9];
	unsigned char *rate;

	PRINT_ONE(num,  " %d: stat_info...", 1);
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    state: ", pstat->state, "%x");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    AuthAlgrthm: ", pstat->AuthAlgrthm, "%d");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    ieee8021x_ctrlport: ", pstat->ieee8021x_ctrlport, "%d");
	CHECK_LEN_B;
	PRINT_ARRAY_ARG("    hwaddr: ",	pstat->hwaddr, "%02x", MACADDRLEN);
	CHECK_LEN_B;
	PRINT_ARRAY_ARG("    bssrateset: ", pstat->bssrateset, "%02x", pstat->bssratelen);
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    aid: ", pstat->aid, "%d");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    tx_bytes: ", pstat->tx_bytes, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rx_bytes: ", pstat->rx_bytes, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    tx_pkts: ", pstat->tx_pkts, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rx_pkts: ", pstat->rx_pkts, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    tx_fail: ", pstat->tx_fail, "%u");
	CHECK_LEN_B;
	PRINT_ONE("    rssi: ", "%s", 0);
	PRINT_ONE(pstat->rssi, "%u", 0);
	PRINT_ONE(pstat->rf_info.mimorssi[0], " (%u", 0);
	PRINT_ONE(pstat->rf_info.mimorssi[1], " %u", 0);
	PRINT_ONE(")", "%s", 1);
	CHECK_LEN_B;

#ifdef WDS
	if (pstat->state & WIFI_WDS
#ifdef LAZY_WDS
			&&  !(pstat->state & WIFI_WDS_LAZY)
#endif
		) {
		PRINT_SINGL_ARG("    idle_time: ", pstat->idle_time, "%d");
		CHECK_LEN_B;
	}
	else
#endif
	{
		PRINT_SINGL_ARG("    expired_time: ", pstat->expire_to, "%d");
		CHECK_LEN_B;
	}

	PRINT_SINGL_ARG("    sleep: ", (!list_empty(&pstat->sleep_list) ? "yes" : "no"), "%s");
	CHECK_LEN_B;
#ifdef STA_EXT
	PRINT_SINGL_ARG("    sta_in_firmware_mem: ",(pstat->sta_in_firmware == -1 ? "removed" : ((pstat->sta_in_firmware) == 0 ? "no":"yes")), "%s" );
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    remapped_aid: ", pstat->remapped_aid, "%d");
	CHECK_LEN_B;
#endif

	if (is_MCS_rate(pstat->current_tx_rate)) {
		PRINT_ONE("    current_tx_rate: MCS", "%s", 0);
//		PRINT_ONE(pstat->current_tx_rate&0x7f, "%d", 0);
		PRINT_ONE(pstat->current_tx_rate&0xf, "%d", 0);		
//		rate = (unsigned char *)MCS_DATA_RATEStr[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][pstat->current_tx_rate&0x7f];
		rate = (unsigned char *)MCS_DATA_RATEStr[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][pstat->current_tx_rate&0xf];
		PRINT_ONE(rate, " %s", 1);
		CHECK_LEN_B;
	}
	else
	{
		PRINT_SINGL_ARG("    current_tx_rate: ", pstat->current_tx_rate/2, "%d");
		CHECK_LEN_B;
	}

	if (is_MCS_rate(pstat->rx_rate)) {
		PRINT_ONE("    current_rx_rate: MCS", "%s", 0);
//		PRINT_ONE(pstat->rx_rate&0x7f, "%d", 0);
		PRINT_ONE(pstat->rx_rate&0xf, "%d", 0);
//		rate = (unsigned char *)MCS_DATA_RATEStr[pstat->rx_bw][pstat->rx_splcp][pstat->rx_rate&0x7f];
		rate = (unsigned char *)MCS_DATA_RATEStr[pstat->rx_bw][pstat->rx_splcp][pstat->rx_rate&0xf];
		PRINT_ONE(rate, " %s", 1);
		CHECK_LEN_B;
	}
	else
	{
		PRINT_SINGL_ARG("    current_rx_rate: ", pstat->rx_rate/2, "%d");
		CHECK_LEN_B;
	}

	PRINT_SINGL_ARG("    rx_bw: ", (pstat->rx_bw? "40M":"20M"), "%s");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    hp_level: ", pstat->hp_level, "%d");
	CHECK_LEN_B;

#ifdef WIFI_WMM
	PRINT_SINGL_ARG("    QoS Enable: ", pstat->QosEnabled, "%d");
	CHECK_LEN_B;
#ifdef WMM_APSD
	PRINT_SINGL_ARG("    APSD bitmap: ", pstat->apsd_bitmap, "0x%01x");
	CHECK_LEN_B;
#endif
#endif

	if (pstat->is_realtek_sta)
		sprintf(tmpbuf, "Realtek");
	else if (pstat->is_broadcom_sta)
		sprintf(tmpbuf, "Broadcom");
	else if (pstat->is_marvell_sta)
		sprintf(tmpbuf, "Marvell");
	else if (pstat->is_intel_sta)
		sprintf(tmpbuf, "Intel");
	else if (pstat->is_ralink_sta)
		sprintf(tmpbuf, "Ralink");
	else
		sprintf(tmpbuf, "--");
	PRINT_SINGL_ARG("    Chip Vendor: ", tmpbuf, "%s");
	CHECK_LEN_B;
#ifdef RTK_WOW
	if (pstat->is_realtek_sta) {
		PRINT_SINGL_ARG("    is_rtk_wow_sta: ", (pstat->is_rtk_wow_sta? "yes":"no"), "%s");
		CHECK_LEN_B;
	}
#endif

	m = pstat->link_time / 86400;
	n = pstat->link_time % 86400;
	if (m)	idx += sprintf(tmp, "%d day ", m);
	m = n / 3600;
	n = n % 3600;
	if (m)	idx += sprintf(tmp+idx, "%d hr ", m);
	m = n / 60;
	n = n % 60;
	if (m)	idx += sprintf(tmp+idx, "%d min ", m);
	idx += sprintf(tmp+idx, "%d sec ", n);
	PRINT_SINGL_ARG("    link_time: ", tmp, "%s");
	CHECK_LEN_B;

	if (pstat->private_ie_len) {
		PRINT_ARRAY_ARG("    private_ie: ", pstat->private_ie, "%02x", pstat->private_ie_len);
		CHECK_LEN_B;
	}
	if (pstat->ht_cap_len) {
		unsigned char *pbuf = (unsigned char *)&pstat->ht_cap_buf;
		PRINT_ARRAY_ARG("    ht_cap: ", pbuf, "%02x", pstat->ht_cap_len);
		CHECK_LEN_B;

		PRINT_ONE("    11n MIMO ps: ", "%s", 0);
		if (!(pstat->MIMO_ps)) {
			PRINT_ONE("no limit", "%s", 1);
		}
		else {
			PRINT_ONE(((pstat->MIMO_ps&BIT(0))?"static":"dynamic"), "%s", 1);
		}
		CHECK_LEN_B;

		PRINT_SINGL_ARG("    Is_8K_AMSDU: ", pstat->is_8k_amsdu, "%d");
		CHECK_LEN_B;
		PRINT_SINGL_ARG("    amsdu_level: ", pstat->amsdu_level, "%d");
		CHECK_LEN_B;
		PRINT_SINGL_ARG("    diffAmpduSz: 0x", pstat->diffAmpduSz, "%x");
		CHECK_LEN_B;

		switch (pstat->aggre_mthd) {
		case AGGRE_MTHD_MPDU:
			sprintf(tmp, "AMPDU");
			break;
		case AGGRE_MTHD_MSDU:
			sprintf(tmp, "AMSDU");
			break;
		default:
			sprintf(tmp, "None");
			break;
		}
		PRINT_SINGL_ARG("    aggre mthd: ", tmp, "%s");
		CHECK_LEN_B;

#ifdef _DEBUG_RTL8192CD_
		PRINT_ONE("    ch_width: ", "%s", 0);
		PRINT_ONE((pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))?"40M":"20M", "%s", 1);
		CHECK_LEN_B;
		PRINT_ONE("    ampdu_mf: ", "%s", 0);
		PRINT_ONE(pstat->ht_cap_buf.ampdu_para & 0x03, "%d", 1);
		CHECK_LEN_B;
		PRINT_ONE("    ampdu_amd: ", "%s", 0);
		PRINT_ONE((pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_, "%d", 1);
		CHECK_LEN_B;
#endif
	}
	else {
		PRINT_ONE("    ht_cap: none", "%s", 1);
		CHECK_LEN_B;
	}

#ifdef SUPPORT_TX_MCAST2UNI
	PRINT_SINGL_ARG("    ipmc_num: ", pstat->ipmc_num, "%d");
	CHECK_LEN_B;
	for (idx=0; idx<MAX_IP_MC_ENTRY; idx++) {
		if (pstat->ipmc[idx].used) {
			PRINT_ARRAY_ARG("    mcmac: ",	pstat->ipmc[idx].mcmac, "%02x", MACADDRLEN);
			CHECK_LEN_B;
		}
	}
#endif

#ifdef CLIENT_MODE
	if (pstat->ht_ie_len) {
		unsigned char *pbuf = (unsigned char *)&pstat->ht_ie_buf;
		PRINT_ARRAY_ARG("    ht_ie: ", pbuf, "%02x", pstat->ht_ie_len);
		CHECK_LEN_B;
	}
	else {
		PRINT_ONE("    ht_ie: none", "%s", 1);
		CHECK_LEN_B;
	}
#endif

#if defined(HW_ANT_SWITCH)
	PRINT_SINGL_ARG("    Ant 1 packet count:  ", pstat->hwRxAntSel[1], "%u");
	PRINT_SINGL_ARG("    Ant 2 packet count:  ", pstat->hwRxAntSel[0], "%u");
	PRINT_SINGL_ARG("    Antenna select :  ", (pstat->CurAntenna==0 ? 2 : 1), "%u");
#endif

#ifdef _DEBUG_RTL8192CD_
	PRINT_SINGL_ARG("    amsdu err:  ", pstat->rx_amsdu_err, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    amsdu 1pkt: ", pstat->rx_amsdu_1pkt, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    amsdu 2pkt: ", pstat->rx_amsdu_2pkt, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    amsdu 3pkt: ", pstat->rx_amsdu_3pkt, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    amsdu 4pkt: ", pstat->rx_amsdu_4pkt, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    amsdu 5pkt: ", pstat->rx_amsdu_5pkt, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    amsdu gt 5pkt: ", pstat->rx_amsdu_gt5pkt, "%u");
	CHECK_LEN_B;

	PRINT_SINGL_ARG("    rc drop1:     ", pstat->rx_rc_drop1, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc passup2:   ", pstat->rx_rc_passup2, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc drop3:     ", pstat->rx_rc_drop3, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc reorder3:  ", pstat->rx_rc_reorder3, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc passup3:   ", pstat->rx_rc_passup3, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc drop4:     ", pstat->rx_rc_drop4, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc reorder4:  ", pstat->rx_rc_reorder4, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc passup4:   ", pstat->rx_rc_passup4, "%u");
	CHECK_LEN_B;
	PRINT_SINGL_ARG("    rc passupi:   ", pstat->rx_rc_passupi, "%u");
	CHECK_LEN_B;
#endif

	PRINT_ONE("", "%s", 1);

	*rc = 1; //finished, assign okay to return code.
	return pos;
}

static int read_sta_info_down;

static int rtl8192cd_proc_stainfo(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int len = 0, rc=1;
	off_t begin = 0;
	off_t pos = 0;
	int size, num=1;
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	unsigned long flags=0;

	if (offset == 0) // first calling, reset read_sta_info_down variable
		read_sta_info_down = 0;

	// first, kernel call me with length=3072; second, kernel call me with length=1024,
	// third, , kernel call me with length < 1024, I just return 0 when length < 1024 to avoid wasting time.
	if (length < 1024) {
		return 0;
	}

	// do not waste time again
	// I sent *eof=1 last time, I do not know why the kernel will call me again.
	if (read_sta_info_down) {		
		*eof = 1;		
		return 0;		
	}

	SAVE_INT_AND_CLI(flags);

	// !!! it REALLY waste the cpu time ==> it will do from beginning when kernel call rtl8192cd_proc_stainfo() function again and again.
	size = sprintf(buf, "-- STA info table -- (active: %d)\n", priv->assoc_num);
	CHECK_LEN;

	phead = &priv->asoc_list;
	if (!netif_running(dev) || list_empty(phead)) {
		*eof = 1;		
		goto _ret;
	}

	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);

		// (length + offset) : the total length can be used in this rtl8192cd_proc_stainfo() calling.
		//  (length + offset - -begin - len) : the remained length can be used.
		if ((length + offset - begin - len) < MAX_CHAR_IN_A_LINE)
			goto _ret;

		rc = 0; // return code, assume error by default
		//size = dump_one_stainfo(num++, pstat, buf+len, start, offset, length, eof, data);
		size = dump_one_stainfo(num++, pstat, buf+len, offset, length, (length + offset - begin- len), &rc);	
		CHECK_LEN;
		
#ifdef CONFIG_RTK_MESH
		if (rc == 0)
			break;
		
		// 3 line for Throughput statistics (sounder)
		size = dump_mesh_one_mpflow_sta(num, pstat, buf+len, start, offset, length,
					eof, data);
		CHECK_LEN;
#endif

		plist = plist->next;
	}

	if (rc == 1) { // return code is okay. if return code is 0, it means the dump_one_stainfo() is not finished.
		read_sta_info_down = 1;
		*eof = 1;
	}

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	RESTORE_INT(flags);
	return len;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int dump_one_sta_keyinfo(int num, struct stat_info *pstat, char *buf, char **start,
			off_t offset, int length, int *eof, void *data)
{
	int pos = 0;
	unsigned char *ptr;

	PRINT_ONE(num,  " %d: stat_keyinfo...", 1);
	PRINT_ARRAY_ARG("    hwaddr: ",	pstat->hwaddr, "%02x", MACADDRLEN);
	PRINT_SINGL_ARG("    keyInCam: ", (pstat->dot11KeyMapping.keyInCam? "yes" : "no"), "%s");
	PRINT_SINGL_ARG("    dot11Privacy: ",
			pstat->dot11KeyMapping.dot11Privacy, "%d");
	PRINT_SINGL_ARG("    dot11EncryptKey.dot11TTKeyLen: ",
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen, "%d");
	PRINT_SINGL_ARG("    dot11EncryptKey.dot11TMicKeyLen: ",
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen, "%d");
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TTKey.skey: ",
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey, "%02x", 16);
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TMicKey1.skey: ",
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey1.skey, "%02x", 16);
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TMicKey2.skey: ",
			pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey2.skey, "%02x", 16);
	ptr = (unsigned char *)&pstat->dot11KeyMapping.dot11EncryptKey.dot11TXPN48.val48;
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11TXPN48.val48: ", ptr, "%02x", 8);
	ptr = (unsigned char *)&pstat->dot11KeyMapping.dot11EncryptKey.dot11RXPN48.val48;
	PRINT_ARRAY_ARG("    dot11EncryptKey.dot11RXPN48.val48: ", ptr, "%02x", 8);

	PRINT_ONE("", "%s", 1);

	return pos;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int rtl8192cd_proc_sta_keyinfo(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int size, num=1;
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	unsigned long flags=0;
	SAVE_INT_AND_CLI(flags);	

	strcpy(buf, "-- STA key info table --\n");
	size = strlen(buf);
	CHECK_LEN;

	phead = &priv->asoc_list;
	if (!netif_running(dev) || list_empty(phead))
		goto _ret;

	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		size = dump_one_sta_keyinfo(num++, pstat, buf+len, start, offset, length,
					eof, data);
		CHECK_LEN;

		plist = plist->next;
	}

	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	RESTORE_INT(flags);
	return len;
}

#ifdef PCIE_POWER_SAVING
char pwr_state_str[][20] = {"L0", "L1", "L2", "ASPM_L0s_L1" };
#endif

static int rtl8192cd_proc_stats(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0, idx = 0;
	unsigned int m, n, print=0;
	char tmp[32];
	unsigned char *rate;
	unsigned long flags=0;
	SAVE_INT_AND_CLI(flags);
	
	PRINT_ONE("  Statistics...", "%s", 1);

	m = priv->up_time / 86400;
	n = priv->up_time % 86400;
	if (m) {
		idx += sprintf(tmp, "%d day ", m);
		print = 1;
	}
	m = n / 3600;
	n = n % 3600;
	if (m || print) {
		idx += sprintf(tmp+idx, "%d hr ", m);
		print = 1;
	}
	m = n / 60;
	n = n % 60;
	if (m || print) {
		idx += sprintf(tmp+idx, "%d min ", m);
		print = 1;
	}
	idx += sprintf(tmp+idx, "%d sec ", n);
	PRINT_SINGL_ARG("    up_time: ", tmp, "%s");

	PRINT_SINGL_ARG("    tx_packets:    ", priv->net_stats.tx_packets, "%lu");
	PRINT_SINGL_ARG("    tx_bytes:      ", priv->net_stats.tx_bytes, "%lu");
	PRINT_SINGL_ARG("    tx_retrys:     ", priv->ext_stats.tx_retrys, "%lu");
	PRINT_SINGL_ARG("    tx_fails:      ", priv->net_stats.tx_errors, "%lu");
	PRINT_SINGL_ARG("    tx_drops:      ", priv->ext_stats.tx_drops, "%lu");
	PRINT_SINGL_ARG("    rx_packets:    ", priv->net_stats.rx_packets, "%lu");
	PRINT_SINGL_ARG("    rx_bytes:      ", priv->net_stats.rx_bytes, "%lu");
	PRINT_SINGL_ARG("    rx_retrys:     ", priv->ext_stats.rx_retrys, "%lu");
	PRINT_SINGL_ARG("    rx_crc_errors: ", priv->net_stats.rx_crc_errors, "%lu");
	PRINT_SINGL_ARG("    rx_errors:     ", priv->net_stats.rx_errors, "%lu");
	PRINT_SINGL_ARG("    rx_data_drops: ", priv->ext_stats.rx_data_drops, "%lu");
	PRINT_SINGL_ARG("    rx_decache:    ", priv->ext_stats.rx_decache, "%lu");
	PRINT_SINGL_ARG("    rx_fifoO:      ", priv->ext_stats.rx_fifoO, "%lu");
	PRINT_SINGL_ARG("    rx_rdu:        ", priv->ext_stats.rx_rdu, "%lu");
	PRINT_SINGL_ARG("    beacon_ok:     ", priv->ext_stats.beacon_ok, "%lu");
	PRINT_SINGL_ARG("    beacon_er:     ", priv->ext_stats.beacon_er, "%lu");

	PRINT_SINGL_ARG("    freeskb_err:   ", priv->ext_stats.freeskb_err, "%lu");

#ifdef CHECK_HANGUP
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef CHECK_TX_HANGUP
		PRINT_SINGL_ARG("    check_cnt_tx:  ", priv->check_cnt_tx, "%d");
#endif
#ifdef CHECK_RX_HANGUP
		PRINT_SINGL_ARG("    check_cnt_rx:  ", priv->check_cnt_rx, "%d");
#endif
#ifdef CHECK_BEACON_HANGUP
		PRINT_SINGL_ARG("    check_cnt_bcn: ", priv->check_cnt_bcn, "%d");
#endif
		PRINT_SINGL_ARG("    check_cnt_rst: ", priv->check_cnt_rst, "%d");
#ifdef CHECK_BB_HANGUP
		PRINT_SINGL_ARG("    check_cnt_bb:  ", priv->check_cnt_bb, "%d");
		PRINT_SINGL_ARG("    check_cnt_cca: ", priv->check_cnt_cca, "%d");
#endif
	}
#endif

#if defined(CONFIG_RTL_NOISE_CONTROL_92C)
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		PRINT_SINGL_ARG("    FA_total_cnt:  ", priv->pshare->FA_total_cnt, "%d");
		PRINT_SINGL_ARG("    DNC_on:        ", (priv->pshare->DNC_on ? 1 : 0), "%d");
		PRINT_SINGL_ARG("    tp_avg_pre:    ", priv->ext_stats.tp_average_pre, "%lu");
	}
#endif

		PRINT_SINGL_ARG("    reused_skb:    ", priv->ext_stats.reused_skb, "%lu");

#ifdef CONFIG_RTL8190_PRIV_SKB
		{
			extern int skb_free_num;
			PRINT_SINGL_ARG("    skb_free_num:  ", skb_free_num+priv->pshare->skb_queue.qlen, "%d");
		}
#endif

	PRINT_SINGL_ARG("    tx_avarage:    ", priv->ext_stats.tx_avarage, "%lu");
	PRINT_SINGL_ARG("    rx_avarage:    ", priv->ext_stats.rx_avarage, "%lu");

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
	PRINT_SINGL_ARG("    tx_peak:       ", priv->ext_stats.tx_peak, "%lu");
	PRINT_SINGL_ARG("    rx_peak:       ", priv->ext_stats.rx_peak, "%lu");
#endif

	if (is_MCS_rate(priv->pshare->current_tx_rate)) {
		PRINT_ONE("    cur_tx_rate:   MCS", "%s", 0);
		PRINT_ONE(priv->pshare->current_tx_rate&0x7f, "%d", 0);
		rate = (unsigned char *)MCS_DATA_RATEStr[(priv->pshare->ht_current_tx_info&BIT(0))?1:0][(priv->pshare->ht_current_tx_info&BIT(1))?1:0][priv->pshare->current_tx_rate&0x7f];
		PRINT_ONE(rate, " %s", 1);
	}
	else
	{
		PRINT_SINGL_ARG("    cur_tx_rate:   ", priv->pshare->current_tx_rate/2, "%d");
	}
#ifdef PCIE_POWER_SAVING
	PRINT_SINGL_ARG("    pcie pwr state:   ", pwr_state_str[priv->pwr_state], "%s");
#endif

#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	if (priv->pmib->miscEntry.rsv_txdesc) {
		PRINT_SINGL_ARG("    bkq_used_desc:   ", (UINT)priv->use_txdesc_cnt[BK_QUEUE], "%d");
		PRINT_SINGL_ARG("    beq_used_desc:   ", (UINT)priv->use_txdesc_cnt[BE_QUEUE], "%d");
		PRINT_SINGL_ARG("    viq_used_desc:   ", (UINT)priv->use_txdesc_cnt[VI_QUEUE], "%d");
		PRINT_SINGL_ARG("    voq_used_desc:   ", (UINT)priv->use_txdesc_cnt[VO_QUEUE], "%d");
	}
#endif
	RESTORE_INT(flags);

	return pos;
}


static int rtl8192cd_proc_stats_clear(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;

	memset(&priv->net_stats, 0, sizeof(struct net_device_stats));
	memset(&priv->ext_stats, 0, sizeof(struct extra_stats));
#ifdef CONFIG_RTK_MESH
	memset(&priv->mesh_stats, 0, sizeof(struct net_device_stats));
#endif
#ifdef _11s_TEST_MODE_
	memset(priv->rvTestPacket, 0, 3000);
#endif
	return count;
}


static int rtl8192cd_proc_mib_misc(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  miscEntry...", "%s", 1);

	PRINT_SINGL_ARG("    show_hidden_bss: ", priv->pmib->miscEntry.show_hidden_bss, "%d");
	PRINT_SINGL_ARG("    ack_timeout: ", (unsigned char)priv->pmib->miscEntry.ack_timeout, "%d");
	PRINT_ARRAY_ARG("    private_ie: ", priv->pmib->miscEntry.private_ie, "%02x", priv->pmib->miscEntry.private_ie_len);
	PRINT_SINGL_ARG("    rxInt: ", priv->pmib->miscEntry.rxInt_thrd, "%d");
#ifdef DRVMAC_LB
	PRINT_SINGL_ARG("    dmlb: ", priv->pmib->miscEntry.drvmac_lb, "%d");
	PRINT_ARRAY_ARG("    lb_da: ", priv->pmib->miscEntry.lb_da, "%02x", MACADDRLEN);
	PRINT_SINGL_ARG("    lb_tps: ", priv->pmib->miscEntry.lb_tps, "%d");
#endif
	PRINT_SINGL_ARG("    groupID: ", priv->pmib->miscEntry.groupID, "%d");
	PRINT_SINGL_ARG("    rc_enable: ", priv->pmib->reorderCtrlEntry.ReorderCtrlEnable, "%d");
	PRINT_SINGL_ARG("    rc_winsz: ", priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz, "%d");
	PRINT_SINGL_ARG("    rc_timeout: ", priv->pmib->reorderCtrlEntry.ReorderCtrlTimeout, "%d");
//#ifdef FW_SW_BEACON
	PRINT_SINGL_ARG("    vap_enable: ", priv->pmib->miscEntry.vap_enable, "%d");
//#endif
#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	PRINT_SINGL_ARG("    rsv_txdesc: ", priv->pmib->miscEntry.rsv_txdesc, "%d");
#endif
	PRINT_SINGL_ARG("    func_off: ", priv->pmib->miscEntry.func_off, "%d");

	return pos;
}


#ifdef WIFI_SIMPLE_CONFIG
static int rtl8192cd_proc_mib_wsc(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  wscEntry...", "%s", 1);
	PRINT_SINGL_ARG("    wsc_enable: ", priv->pmib->wscEntry.wsc_enable, "%d");
	PRINT_ARRAY_ARG("    beacon_ie: ",
			priv->pmib->wscEntry.beacon_ie, "%02x", priv->pmib->wscEntry.beacon_ielen);
	PRINT_SINGL_ARG("    beacon_ielen: ", priv->pmib->wscEntry.beacon_ielen, "%d");
	PRINT_ARRAY_ARG("    probe_rsp_ie: ",
			priv->pmib->wscEntry.probe_rsp_ie, "%02x", priv->pmib->wscEntry.probe_rsp_ielen);
	PRINT_SINGL_ARG("    probe_rsp_ielen: ", priv->pmib->wscEntry.probe_rsp_ielen, "%d");
	PRINT_ARRAY_ARG("    probe_req_ie: ",
			priv->pmib->wscEntry.probe_req_ie, "%02x", priv->pmib->wscEntry.probe_req_ielen);
	PRINT_SINGL_ARG("    probe_req_ielen: ", priv->pmib->wscEntry.probe_req_ielen, "%d");
	PRINT_ARRAY_ARG("    assoc_ie: ",
			priv->pmib->wscEntry.assoc_ie, "%02x", priv->pmib->wscEntry.assoc_ielen);
	PRINT_SINGL_ARG("    assoc_ielen: ", priv->pmib->wscEntry.assoc_ielen, "%d");

	return pos;
}
#endif


#ifdef RF_FINETUNE
static int rtl8192cd_proc_rfft(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  RF fine tune variables...", "%s", 1);

	PRINT_SINGL_ARG("    rssi: ", priv->pshare->rf_ft_var.rssi_dump, "%d");
	PRINT_SINGL_ARG("    rxfifoO: ", priv->pshare->rf_ft_var.rxfifoO, "%x");
	PRINT_SINGL_ARG("    raGoDownUpper: ", priv->pshare->rf_ft_var.raGoDownUpper, "%d");
	PRINT_SINGL_ARG("    raGoDown20MLower: ", priv->pshare->rf_ft_var.raGoDown20MLower, "%d");
	PRINT_SINGL_ARG("    raGoDown40MLower: ", priv->pshare->rf_ft_var.raGoDown40MLower, "%d");
	PRINT_SINGL_ARG("    raGoUpUpper: ", priv->pshare->rf_ft_var.raGoUpUpper, "%d");
	PRINT_SINGL_ARG("    raGoUp20MLower: ", priv->pshare->rf_ft_var.raGoUp20MLower, "%d");
	PRINT_SINGL_ARG("    raGoUp40MLower: ", priv->pshare->rf_ft_var.raGoUp40MLower, "%d");
	PRINT_SINGL_ARG("    dig_enable: ", priv->pshare->rf_ft_var.dig_enable, "%d");
	PRINT_SINGL_ARG("    digGoLowerLevel: ", priv->pshare->rf_ft_var.digGoLowerLevel, "%d");
	PRINT_SINGL_ARG("    digGoUpperLevel: ", priv->pshare->rf_ft_var.digGoUpperLevel, "%d");
	PRINT_SINGL_ARG("    rssiTx20MUpper: ", priv->pshare->rf_ft_var.rssiTx20MUpper, "%d");
	PRINT_SINGL_ARG("    rssiTx20MLower: ", priv->pshare->rf_ft_var.rssiTx20MLower, "%d");
	PRINT_SINGL_ARG("    rssi_expire_to: ", priv->pshare->rf_ft_var.rssi_expire_to, "%d");

	PRINT_SINGL_ARG("    cck_pwr_max: ", priv->pshare->rf_ft_var.cck_pwr_max, "%d");
	PRINT_SINGL_ARG("    cck_tx_pathB: ", priv->pshare->rf_ft_var.cck_tx_pathB, "%d");

	PRINT_SINGL_ARG("    tx_pwr_ctrl: ", priv->pshare->rf_ft_var.tx_pwr_ctrl, "%d");

	// 11n ap AES debug
	PRINT_SINGL_ARG("    aes_check_th: ", priv->pshare->rf_ft_var.aes_check_th, "%d KB");

	// Tx power tracking
	PRINT_SINGL_ARG("    tpt_period: ", priv->pshare->rf_ft_var.tpt_period, "%d");

	// TXOP enlarge
	PRINT_SINGL_ARG("    txop_enlarge_upper: ", priv->pshare->rf_ft_var.txop_enlarge_upper, "%d");
	PRINT_SINGL_ARG("    txop_enlarge_lower: ", priv->pshare->rf_ft_var.txop_enlarge_lower, "%d");

	// 2.3G support
	PRINT_SINGL_ARG("    frq_2_3G: ", priv->pshare->rf_ft_var.use_frq_2_3G, "%d");

	//Support IP multicast->unicast
#ifdef SUPPORT_TX_MCAST2UNI
	PRINT_SINGL_ARG("    mc2u_disable: ", priv->pshare->rf_ft_var.mc2u_disable, "%d");
#endif
#ifdef	HIGH_POWER_EXT_PA
	PRINT_SINGL_ARG("    use_ext_pa: ", priv->pshare->rf_ft_var.use_ext_pa, "%d");
#endif
	PRINT_SINGL_ARG("    use_ext_lna: ", priv->pshare->rf_ft_var.use_ext_lna, "%d");
	PRINT_SINGL_ARG("    NDSi_support: ", priv->pshare->rf_ft_var.NDSi_support, "%d");
	PRINT_SINGL_ARG("    EDCCA threshold: ", priv->pshare->rf_ft_var.edcca_thd, "%d");
	PRINT_SINGL_ARG("    1rcca: ", priv->pshare->rf_ft_var.one_path_cca, "%d");

	return pos;
}
#endif


#ifdef GBWC
static int rtl8192cd_proc_mib_gbwc(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0, i;

	PRINT_ONE("  miscGBWC...", "%s", 1);

	PRINT_SINGL_ARG("    GBWCMode: ", priv->pmib->gbwcEntry.GBWCMode, "%d");
	PRINT_SINGL_ARG("    GBWCThrd: ", priv->pmib->gbwcEntry.GBWCThrd, "%d kbps");
	PRINT_ONE("    Address List:", "%s", 1);
	for (i=0; i<priv->pmib->gbwcEntry.GBWCNum; i++) {
		PRINT_ARRAY_ARG("      ", priv->pmib->gbwcEntry.GBWCAddr[i], "%02x", MACADDRLEN);
	}

	return pos;
}
#endif


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static int rtl8192cd_proc_led(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	char tmpbuf[100];
	int flag;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {
		sscanf(tmpbuf, "%d", &flag);
		if (flag == 0) // disable
			control_wireless_led(priv, 0);
		else if (flag == 1) // enable
			control_wireless_led(priv, 1);
		else if (flag == 2) // restore
			control_wireless_led(priv, 2);
		else
			printk("flag [%d] not supported!\n", flag);
    }
	return count;
}


#ifdef RTL_MANUAL_EDCA
static int rtl8192cd_proc_mib_edca(char *buf, char **start, off_t offset,
                     int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int pos = 0;
	char *queue[] = {"", "BK", "BE", "VI", "VO"};

	PRINT_SINGL_ARG("  Manually config EDCA : ", priv->pmib->dot11QosEntry.ManualEDCA, "%d");
	PRINT_ONE("  EDCA for AP...", "%s", 1);
	PRINT_SINGL_ARG("  [BE]slot number (AIFS): ", priv->pmib->dot11QosEntry.AP_manualEDCA[BE].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[BE].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[BE].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.AP_manualEDCA[BE].TXOPlimit, "%d");
	PRINT_SINGL_ARG("  [BK]slot number (AIFS): ", priv->pmib->dot11QosEntry.AP_manualEDCA[BK].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[BK].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[BK].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.AP_manualEDCA[BK].TXOPlimit, "%d");
	PRINT_SINGL_ARG("  [VI]slot number (AIFS)= ", priv->pmib->dot11QosEntry.AP_manualEDCA[VI].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[VI].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[VI].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.AP_manualEDCA[VI].TXOPlimit, "%d");
	PRINT_SINGL_ARG("  [VO]slot number (AIFS): ", priv->pmib->dot11QosEntry.AP_manualEDCA[VO].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[VO].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.AP_manualEDCA[VO].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.AP_manualEDCA[VO].TXOPlimit, "%d");
	PRINT_ONE("  EDCA for Wireless client...", "%s", 1);
	PRINT_SINGL_ARG("  [BE]ACM: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BE].ACM, "%d");
	PRINT_SINGL_ARG("      slot number (AIFS): ", priv->pmib->dot11QosEntry.STA_manualEDCA[BE].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BE].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BE].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BE].TXOPlimit, "%d");
	PRINT_SINGL_ARG("  [BK]ACM:", priv->pmib->dot11QosEntry.STA_manualEDCA[BK].ACM, "%d");
	PRINT_SINGL_ARG("      slot number (AIFS): ", priv->pmib->dot11QosEntry.STA_manualEDCA[BK].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BK].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BK].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.STA_manualEDCA[BK].TXOPlimit, "%d");
	PRINT_SINGL_ARG("  [VI]ACM: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VI].ACM, "%d");
	PRINT_SINGL_ARG("      slot number (AIFS): ", priv->pmib->dot11QosEntry.STA_manualEDCA[VI].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VI].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VI].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit:", priv->pmib->dot11QosEntry.STA_manualEDCA[VI].TXOPlimit, "%d");
	PRINT_SINGL_ARG("  [VO]ACM: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VO].ACM, "%d");
	PRINT_SINGL_ARG("      slot number (AIFS): ", priv->pmib->dot11QosEntry.STA_manualEDCA[VO].AIFSN, "%d");
	PRINT_SINGL_ARG("      Maximal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VO].ECWmax, "%d");
	PRINT_SINGL_ARG("      Minimal contention window period: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VO].ECWmin, "%d");
	PRINT_SINGL_ARG("      TXOP limit: ", priv->pmib->dot11QosEntry.STA_manualEDCA[VO].TXOPlimit, "%d");

	PRINT_SINGL_ARG("      TID0 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[0]], "%s");
	PRINT_SINGL_ARG("      TID1 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[1]], "%s");
	PRINT_SINGL_ARG("      TID2 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[2]], "%s");
	PRINT_SINGL_ARG("      TID3 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[3]], "%s");
	PRINT_SINGL_ARG("      TID4 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[4]], "%s");
	PRINT_SINGL_ARG("      TID5 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[5]], "%s");
	PRINT_SINGL_ARG("      TID6 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[6]], "%s");
	PRINT_SINGL_ARG("      TID7 mapping: ", queue[priv->pmib->dot11QosEntry.TID_mapping[7]], "%s");

	return pos;
}
#endif //RTL_MANUAL_EDCA


#ifdef __KERNEL__
void MDL_DEVINIT rtl8192cd_proc_init(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = dev->priv;
	struct proc_dir_entry *rtl8192cd_proc_root = NULL ;
	struct proc_dir_entry *res;

	rtl8192cd_proc_root = proc_mkdir(dev->name, NULL);
	priv->proc_root = rtl8192cd_proc_root ;
	if (rtl8192cd_proc_root == NULL) {
		printk("create proc root failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_all", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_all, (void *)dev) == NULL ) {
		printk("create proc mib_all failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_rf", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_rf, (void *)dev) == NULL ) {
		printk("create proc mib_rf failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_operation", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_operation, (void *)dev) == NULL ) {
		printk("create proc mib_operation failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_staconfig", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_staconfig, (void *)dev) == NULL ) {
		printk("create proc mib_staconfig failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_dkeytbl", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_dkeytbl, (void *)dev) == NULL ) {
		printk("create proc mib_dkeytbl failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_auth", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_auth, (void *)dev) == NULL ) {
		printk("create proc mib_auth failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_gkeytbl", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_gkeytbl, (void *)dev) == NULL ) {
		printk("create proc mib_gkeytbl failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mib_bssdesc", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_bssdesc, (void *)dev) == NULL ) {
		printk("create proc mib_bssdesc failed!\n");
		return;
	}
	if ( create_proc_read_entry ("sta_info", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_stainfo, (void *)dev) == NULL ) {
		printk("create proc sta_info failed!\n");
		return;
	}
	if ( create_proc_read_entry ("sta_keyinfo", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_sta_keyinfo, (void *)dev) == NULL ) {
		printk("create proc sta_keyinfo failed!\n");
		return;
	}
	if ( (res = create_proc_read_entry ("stats", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_stats, (void *)dev)) == NULL ) {
		printk("create proc stats failed!\n");
		return;
	}
	res->write_proc = rtl8192cd_proc_stats_clear;

	if ( create_proc_read_entry ("mib_erp", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_erp, (void *)dev) == NULL ) {
		printk("create proc mib_erp failed!\n");
		return;
	}

#ifdef WDS
	if ( create_proc_read_entry ("mib_wds", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_wds, (void *)dev) == NULL ) {
		printk("create proc mib_wds failed!\n");
		return;
	}
#endif

#ifdef RTK_BR_EXT
	if ( create_proc_read_entry ("mib_brext", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_brext, (void *)dev) == NULL ) {
		printk("create proc mib_brext failed!\n");
		return;
	}
#endif

#ifdef DFS
	if ( create_proc_read_entry ("mib_dfs", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_DFS, (void *)dev) == NULL ) {
		printk("create proc mib_DFS failed!\n");
		return;
	}
#endif

	if ( create_proc_read_entry ("mib_misc", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_misc, (void *)dev) == NULL ) {
		printk("create proc mib_misc failed!\n");
		return;
	}

#ifdef WIFI_SIMPLE_CONFIG
	if ( create_proc_read_entry ("mib_wsc", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_wsc, (void *)dev) == NULL ) {
		printk("create proc mib_wsc failed!\n");
		return;
	}
#endif

#ifdef GBWC
	if ( create_proc_read_entry ("mib_gbwc", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_gbwc, (void *)dev) == NULL ) {
		printk("create proc mib_gbwc failed!\n");
		return;
	}
#endif

	if ( create_proc_read_entry ("mib_11n", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_11n, (void *)dev) == NULL ) {
		printk("create proc mib_11n failed!\n");
		return;
	}

#ifdef RTL_MANUAL_EDCA
	if ( create_proc_read_entry ("mib_EDCA", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_mib_edca, (void *)dev) == NULL ) {
		printk("create proc mib_EDCA failed!\n");
		return;
	}
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if ( (res = create_proc_read_entry("mib_vlan", 0644, rtl8192cd_proc_root,
			rtl8192cd_proc_vlan_read, (void *)dev)) == NULL) {
		printk("create_proc vlan failed!\n");
		return;
	}
	res->write_proc = rtl8192cd_proc_vlan_write;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
	{
		if ( (res = create_proc_read_entry ("txdesc", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_txdesc_info, (void *)dev)) == NULL ) {
			printk("create proc txdesc failed!\n");
			return;
		}
		res->write_proc = rtl8192cd_proc_txdesc_info_write;
		if ( create_proc_read_entry ("rxdesc", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_rxdesc_info, (void *)dev) == NULL ) {
			printk("create proc rxdesc failed!\n");
			return;
		}
		if ( create_proc_read_entry ("desc_info", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_desc_info, (void *)dev) == NULL ) {
			printk("create proc desc_info failed!\n");
			return;
		}
		if ( create_proc_read_entry ("buf_info", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_buf_info, (void *)dev) == NULL ) {
			printk("create proc buf_info failed!\n");
			return;
		}
		if ( create_proc_read_entry ("cam_info", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_cam_info, (void *)dev) == NULL ) {
			printk("create proc cam_info failed!\n");
			return;
		}
#ifdef ENABLE_RTL_SKB_STATS
		if ( create_proc_read_entry ("skb_info", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_skb_info, (void *)dev) == NULL ) {
			printk("create proc skb_info failed!\n");
			return;
		}
#endif
#ifdef RF_FINETUNE
		if ( create_proc_read_entry ("rf_finetune", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_rfft, (void *)dev) == NULL ) {
			printk("create proc rf_finetune failed!\n");
			return;
		}
#endif

#ifdef MERGE_FW
		if ( (res = create_proc_read_entry ("phypara_file", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_phypara_file_read, (void *)dev)) == NULL ) {
			printk("create proc stats failed!\n");
			return;
		}
		res->write_proc = rtl8192cd_proc_phypara_file_write;
#endif

		if ( (res = create_proc_entry("led", 0644, rtl8192cd_proc_root)) == NULL ) {
			printk("create proc led failed!\n");
		}
		res->write_proc = rtl8192cd_proc_led;
		res->data = (void *)dev;

#ifdef AUTO_TEST_SUPPORT
		if ( create_proc_read_entry("SS_Result", 0644, rtl8192cd_proc_root,
				rtl8192cd_proc_SSR_read, (void *)dev) == NULL ) {
			printk("create_proc SS_Result failed!\n");
			return;
		}
#endif
	}

#ifdef CONFIG_RTK_MESH
#ifdef MESH_BOOTSEQ_AUTH
	if ( create_proc_read_entry ("mesh_auth_mpinfo", 0644, rtl8192cd_proc_root,
			rtl8190_mesh_auth_mpinfo, (void *)dev) == NULL ) {
		printk("create proc mesh_auth_mpinfo failed!\n");
		return;
	}
#endif

	if ( create_proc_read_entry ("mesh_unestablish_mpinfo", 0644, rtl8192cd_proc_root,
			rtl8190_mesh_unEstablish_mpinfo, (void *)dev) == NULL ) {
		printk("create proc mesh_unestablish_mpinfo failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_assoc_mpinfo", 0644, rtl8192cd_proc_root,
			rtl8190_mesh_assoc_mpinfo, (void *)dev) == NULL ) {
		printk("create proc mesh_assoc_mpinfo failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_stats", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_stats, (void *)dev) == NULL ) {
		printk("create proc mesh_stats failed!\n");
		return;
	}

	// 6 line for Throughput statistics (sounder)
	if ( (res = create_proc_read_entry ("flow_stats", 0644, rtl8192cd_proc_root,
			rtk8190_proc_flow_stats, (void *)dev)) == NULL ) {
		printk("create proc flow_stats failed!\n");
		return;
	}
	res->write_proc = rtk8190_proc_flow_stats_write;

	if ( create_proc_read_entry ("mesh_pathsel_routetable", 0644, rtl8192cd_proc_root,
			rtl8190_pathsel_routetable_info, (void *)dev) == NULL ) {
		printk("create proc mesh_pathsel_routetable failed!\n");
		return;
	}
	if ( create_proc_read_entry ("mesh_proxy_table", 0644, rtl8192cd_proc_root,
			rtl8190_proxy_table_info, (void *)dev) == NULL ) {
		printk("create proc mesh_proxy_table failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_portal_table", 0644, rtl8192cd_proc_root,
			rtl8190_portal_table_info, (void *)dev) == NULL ) {
		printk("create proc mesh_portal_table failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_root_info", 0644, rtl8192cd_proc_root,
			rtl8190_root_info, (void *)dev) == NULL ) {
		printk("create proc mesh_root_info failed!\n");
		return;
	}
#ifdef MESH_USE_METRICOP
	{ // change metric method
		struct proc_dir_entry *proc_file1;
		proc_file1 = create_proc_entry( "mesh_metric",
						0644,
						rtl8192cd_proc_root);
		if(proc_file1 == NULL)
		{
			printk("can't create proc: %s\r\n", "mesh_metric");
			return;
		}
		proc_file1->write_proc = rtl8190_mesh_metric_w;
		proc_file1->read_proc = rtl8190_mesh_metric_r;
		proc_file1->data = (void *)dev;
	}
#endif

#if DBG_NCTU_MESH
	if ( create_proc_read_entry ("mesh_skb", 0644, rtl8192cd_proc_root,
			showAllSkbs, (void *)dev) == NULL ) {
		printk("create proc mesh_skb failed!\n");
		return;
	}
	{
		struct proc_dir_entry *proc_file1;

		proc_file1 = create_proc_entry(
					"mesh_dbg",
					0644,
					rtl8192cd_proc_root);
		if(proc_file1 == NULL)
		{
			printk("can't create proc: %s\r\n", "mesh_dbg");
			return;
		}
		proc_file1->write_proc = rtl8190_setMeshDebugLevel;
	}
#endif
#ifdef _MESH_DEBUG_
	if ( create_proc_read_entry ("mesh_clearalltable", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_clear_table, (void *)dev) == NULL ) {
		printk("create proc mesh_clearalltable failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_issueauthreq", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_issueAuthReq, (void *)dev) == NULL ) {
		printk("create proc mesh_issueauthreq failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_issueauthrsp", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_issueAuthRsp, (void *)dev) == NULL ) {
		printk("create proc mesh_issueauthrsp failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_issuedeauth", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_issueDeAuth, (void *)dev) == NULL ) {
		printk("create proc mesh_issuedeauth failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_openconnect", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_openConnect, (void *)dev) == NULL ) {
		printk("create proc mesh_openconnect failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_issueopen", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_issueOpen, (void *)dev) == NULL ) {
		printk("create proc mesh_issueopen failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_issueconfirm", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_issueConfirm, (void *)dev) == NULL ) {
		printk("create proc mesh_issueconfirm failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_issueclose", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_issueClose, (void *)dev) == NULL ) {
		printk("create proc mesh_issueclose failed!\n");
		return;
	}

	if ( create_proc_read_entry ("mesh_closeconnect", 0644, rtl8192cd_proc_root,
			rtl8190_proc_mesh_closeConnect, (void *)dev) == NULL ) {
		printk("create proc mesh_closeconnect failed!\n");
		return;
	}

	{
	struct proc_dir_entry *proc_file1;

	proc_file1 = create_proc_entry(
					"mesh_setmacaddr",
					0644,
					rtl8192cd_proc_root);
	if(proc_file1 == NULL)
	{
		printk("can't create proc: %s\r\n", "mesh_setmacaddr");
		return;
	}
	proc_file1->write_proc = rtl8190_mesh_setMACAddr;
	}


#if (MESH_DBG_LV & MESH_DBG_COMPLEX)
	{
		struct proc_dir_entry *proc_file;

		proc_file = create_proc_entry(
						MESH_PROC_SME_TEST_FILENAME,
						0644,
						rtl8192cd_proc_root);
		if(proc_file == NULL)
		{
			printk("can't create proc: %s\r\n", MESH_PROC_SME_TEST_FILENAME);
			return;
		}
		proc_file->read_proc = mesh_test_sme_proc_read;
		proc_file->write_proc = mesh_test_sme_proc_write;
	}



/*	{
		struct proc_dir_entry *proc_file;

		proc_file = create_proc_entry(
						MESH_PROC_FILENAME,
						0644,
						rtl8192cd_proc_root);
		if(proc_file == NULL)
		{
			printk("can't create proc: %s\r\n", MESH_PROC_FILENAME);
			return;
		}
		proc_file->read_proc = mesh_proc_read;
		proc_file->write_proc = mesh_proc_write;
	}*/
#endif // (MESH_DBG_LV & MESH_DBG_COMPLEX)
#endif	// _MESH_DEBUG_
#endif	// CONFIG_RTK_MESH
}


void __devexit rtl8192cd_proc_remove (struct net_device *dev)
{
	struct rtl8192cd_priv *priv = dev->priv;
	struct proc_dir_entry *rtl8192cd_proc_root = priv->proc_root;

	if (rtl8192cd_proc_root != NULL) {
		remove_proc_entry( "mib_all", rtl8192cd_proc_root );
		remove_proc_entry( "mib_rf", rtl8192cd_proc_root );
		remove_proc_entry( "mib_operation", rtl8192cd_proc_root );
		remove_proc_entry( "mib_staconfig", rtl8192cd_proc_root );
		remove_proc_entry( "mib_dkeytbl", rtl8192cd_proc_root );
		remove_proc_entry( "mib_auth", rtl8192cd_proc_root );
		remove_proc_entry( "mib_gkeytbl", rtl8192cd_proc_root );
		remove_proc_entry( "mib_bssdesc", rtl8192cd_proc_root );
		remove_proc_entry( "sta_info", rtl8192cd_proc_root );
		remove_proc_entry( "sta_keyinfo", rtl8192cd_proc_root );
		remove_proc_entry( "stats", rtl8192cd_proc_root );
		remove_proc_entry( "mib_erp", rtl8192cd_proc_root );

#ifdef WDS
		remove_proc_entry( "mib_wds", rtl8192cd_proc_root );
#endif

#ifdef RTK_BR_EXT
		remove_proc_entry( "mib_brext", rtl8192cd_proc_root );
#endif


#ifdef DFS
		remove_proc_entry( "mib_dfs", rtl8192cd_proc_root );
#endif

		remove_proc_entry( "mib_misc", rtl8192cd_proc_root );

#ifdef WIFI_SIMPLE_CONFIG
		remove_proc_entry( "mib_wsc", rtl8192cd_proc_root );
#endif


#ifdef GBWC
		remove_proc_entry( "mib_gbwc", rtl8192cd_proc_root );
#endif

		remove_proc_entry( "mib_11n", rtl8192cd_proc_root );

#ifdef CONFIG_RTK_VLAN_SUPPORT
		remove_proc_entry( "mib_vlan", rtl8192cd_proc_root );
#endif


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
		{
			remove_proc_entry( "txdesc", rtl8192cd_proc_root );
			remove_proc_entry( "rxdesc", rtl8192cd_proc_root );
			remove_proc_entry( "desc_info", rtl8192cd_proc_root );
			remove_proc_entry( "buf_info", rtl8192cd_proc_root );
			remove_proc_entry( "cam_info", rtl8192cd_proc_root );
#ifdef ENABLE_RTL_SKB_STATS
			remove_proc_entry( "skb_info", rtl8192cd_proc_root );
#endif
#ifdef RF_FINETUNE
			remove_proc_entry( "rf_finetune", rtl8192cd_proc_root );
#endif
#ifdef MERGE_FW
			remove_proc_entry( "phypara_file", rtl8192cd_proc_root );
#endif
			remove_proc_entry( "led", rtl8192cd_proc_root );

#ifdef AUTO_TEST_SUPPORT
			remove_proc_entry( "SS_Result", rtl8192cd_proc_root );
#endif
		}

#ifdef CONFIG_RTK_MESH
		remove_proc_entry( "mesh_auth_mpinfo", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_unestablish_mpinfo", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_assoc_mpinfo", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_stats", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_pathsel_routetable", rtl8192cd_proc_root );
		remove_proc_entry( "rtl8190_proxy_table_info", rtl8192cd_proc_root );
		remove_proc_entry( "rtl8190_root_info", rtl8192cd_proc_root );
		remove_proc_entry( "rtl8190_portal_table_info", rtl8192cd_proc_root );

#ifdef MESH_USE_METRICOP // remove proc file
                remove_proc_entry( "mesh_metric", rtl8192cd_proc_root );
#endif

#if DBG_NCTU_MESH
		remove_proc_entry( "mesh_skb", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_dbg", rtl8192cd_proc_root );
#endif
#ifdef _MESH_DEBUG_
		remove_proc_entry( "mesh_clearalltable", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_setmacaddr", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_issueauthreq", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_issueauthrsp", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_issuedeauth", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_openconnect", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_issueopen", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_issueconfirm", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_issueclose", rtl8192cd_proc_root );
		remove_proc_entry( "mesh_closeconnect", rtl8192cd_proc_root );
#if (MESH_DBG_LV & MESH_DBG_COMPLEX)
		remove_proc_entry( MESH_PROC_SME_TEST_FILENAME, rtl8192cd_proc_root );
#endif
#endif	// _MESH_DEBUG_
#endif	// CONFIG_RTK_MESH

		remove_proc_entry( dev->name, NULL );
		rtl8192cd_proc_root = NULL;
	}
}

#endif

#if defined(_SINUX_) || !defined(__KERNEL__)

struct _proc_table_
{
	char *cmd;
	int (*func)(char *buf, char **start, off_t offset,
			int length, int *eof, void *data);
};

static struct _proc_table_ proc_table[] =
{
	{"mib_all",				rtl8192cd_proc_mib_all},
	{"mib_rf",				rtl8192cd_proc_mib_rf},
	{"mib_operation",		rtl8192cd_proc_mib_operation},
	{"mib_staconfig",		rtl8192cd_proc_mib_staconfig},
	{"mib_dkeytbl",			rtl8192cd_proc_mib_dkeytbl},
	{"mib_auth",			rtl8192cd_proc_mib_auth},
	{"mib_gkeytbl",			rtl8192cd_proc_mib_gkeytbl},
	{"mib_bssdesc",			rtl8192cd_proc_mib_bssdesc},
	{"sta_info",			rtl8192cd_proc_stainfo},
	{"sta_keyinfo",			rtl8192cd_proc_sta_keyinfo},
	{"txdesc",				rtl8192cd_proc_txdesc_info},
	{"rxdesc",				rtl8192cd_proc_rxdesc_info},
	{"desc_info",			rtl8192cd_proc_desc_info},
	{"buf_info",			rtl8192cd_proc_buf_info},
	{"stats",				rtl8192cd_proc_stats},
	{"mib_erp",				rtl8192cd_proc_mib_erp},
	{"cam_info",			rtl8192cd_proc_cam_info},
#ifdef WDS
	{"mib_wds",				rtl8192cd_proc_mib_wds},
#endif
#ifdef RTK_BR_EXT
	{"mib_brext",			rtl8192cd_proc_mib_brext},
#endif
#ifdef ENABLE_RTL_SKB_STATS
	{"skb_info",			rtl8192cd_proc_skb_info},
#endif
#ifdef DFS
	{"mib_dfs",				rtl8192cd_proc_mib_DFS},
#endif
	{"mib_misc",			rtl8192cd_proc_mib_misc},

#ifdef AUTO_TEST_SUPPORT
	{"SS_Result",			rtl8192cd_proc_SSR_read},
#endif

#ifdef _MESH_DEBUG_ // 802.11s output debug information
	{"mesh_unestablish_mpinfo",	rtl8190_mesh_unEstablish_mpinfo},
	{"mesh_assoc_mpinfo",		rtl8190_mesh_assoc_mpinfo},
	{"mesh_stats",			rtl8190_proc_mesh_stats}
#endif	// _MESH_DEBUG_
};

#define NUM_CMD_TABLE_ENTRY		(sizeof(proc_table) / sizeof(struct _proc_table_))

void rtl8192cd_proc_debug(struct net_device *dev, char *cmd)
{
	int i, j, eof, len;
	char *tmpbuf, *start;

	start = tmpbuf = (char *)kmalloc(4096, 0);
	for (i=0; i<NUM_CMD_TABLE_ENTRY; i++) {
		if (!strcmp(cmd, proc_table[i].cmd)) {
			memset(tmpbuf, 0, 4096);
			len = proc_table[i].func(tmpbuf, &start, 0, 4096, &eof, dev);
			for(j=0; j<len; j++)
				printk("%c", tmpbuf[j]);
		}
	}
	kfree(tmpbuf);
}
#ifdef CONFIG_MSC
int rtl8192cd_show_wifi_debug(char *dev_name, char *cmd, char *str)
{
	int i, j, eof, len;
	char *tmpbuf, *start;
	struct net_device *dev;
	
	printk("dev_name=%s, cmd=%s.\n", dev_name, cmd);

    dev = dev_get_by_name(dev_name);
    if (dev) {
    	start = tmpbuf = (char *)kmalloc(4096, 0);
    	for (i=0; i<NUM_CMD_TABLE_ENTRY; i++) {
    		if (!strcmp(cmd, proc_table[i].cmd)) {
    			memset(tmpbuf, 0, 4096);
    			len = proc_table[i].func(tmpbuf, &start, 0, 4096, &eof, dev);
    			strcpy(str, tmpbuf);
    			for(j=0; j<len; j++)
    				printk("%c", tmpbuf[j]);
    		}
    	}
	    kfree(tmpbuf);
	    dev_put(dev);
	}
	else
	    return 1;
	    
	return 0;
}

EXPORT_SYMBOL(rtl8192cd_show_wifi_debug);
EXPORT_SYMBOL(rtl8192cd_proc_debug);
#endif
#endif // __KERNEL__

#endif // __INCLUDE_PROC_FS__

