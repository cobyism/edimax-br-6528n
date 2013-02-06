#ifndef MISC_H
#define MISC_H

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif

#define	FIFO_HEADER_LEN	5
#define MacAddrLen		6
#define MAXRSNIELEN		128
#define DAEMON_FIFO		"/var/iapp.fifo"
#define FILE_MODE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef enum
{
        DOT11_EVENT_NO_EVENT = 1,
        DOT11_EVENT_REQUEST = 2,
        DOT11_EVENT_ASSOCIATION_IND = 3,
        DOT11_EVENT_ASSOCIATION_RSP = 4,
        DOT11_EVENT_AUTHENTICATION_IND = 5,
        DOT11_EVENT_REAUTHENTICATION_IND = 6,
        DOT11_EVENT_DEAUTHENTICATION_IND = 7,
        DOT11_EVENT_DISASSOCIATION_IND = 8,
        DOT11_EVENT_DISCONNECT_REQ = 9,
        DOT11_EVENT_SET_802DOT11 = 10,
        DOT11_EVENT_SET_KEY = 11,
        DOT11_EVENT_SET_PORT = 12,
        DOT11_EVENT_DELETE_KEY = 13,
        DOT11_EVENT_SET_RSNIE = 14,
        DOT11_EVENT_GKEY_TSC = 15,
        DOT11_EVENT_MIC_FAILURE = 16,
        DOT11_EVENT_ASSOCIATION_INFO = 17,
        DOT11_EVENT_INIT_QUEUE = 18,
        DOT11_EVENT_EAPOLSTART = 19,

        DOT11_EVENT_ACC_SET_EXPIREDTIME = 31,
        DOT11_EVENT_ACC_QUERY_STATS = 32,
        DOT11_EVENT_ACC_QUERY_STATS_ALL = 33,
        DOT11_EVENT_REASSOCIATION_IND = 34,
        DOT11_EVENT_REASSOCIATION_RSP = 35,
        DOT11_EVENT_STA_QUERY_BSSID = 36,

        DOT11_EVENT_EAP_PACKET = 41,

        DOT11_EVENT_MAX = 50,
} DOT11_EVENT;

typedef struct _DOT11_ASSOCIATION_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MacAddrLen];
        unsigned short  RSNIELen;
        char            RSNIE[MAXRSNIELEN];
}DOT11_ASSOCIATION_IND;

typedef struct _DOT11_REASSOCIATION_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MacAddrLen];
        unsigned short  RSNIELen;
        char            RSNIE[MAXRSNIELEN];
        char            OldAPaddr[MacAddrLen];
}DOT11_REASSOCIATION_IND;


enum
{
	HOSTAPD_DEBUG_NO = 0,
	HOSTAPD_DEBUG_MINIMAL = 1,
	HOSTAPD_DEBUG_VERBOSE = 2,
	HOSTAPD_DEBUG_MSGDUMPS = 3
} ;

// debug printing
#define HOSTAPD_DEBUG(level, args...) \
do { \
	if (hapd->debug >= (level)) \
		printf(args); \
} while (0)


#endif /* MISC_H */
