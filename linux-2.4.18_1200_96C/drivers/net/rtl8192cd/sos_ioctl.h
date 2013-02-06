#ifndef _SOS_IOCTL_
#define _SOS_IOCTL_

typedef int (*sos_ioctl_handler)(char *ifName, char *name, void *value);
typedef int (*get_sc_wlan_handler)(char *pBuf,int length);
typedef void (*sc_log_handler)(short const logType, char *fmt, ...);

typedef struct tag_SOS_IOCTL_HANDLER
{
    sos_ioctl_handler privateSetHandler;
    sos_ioctl_handler getChannel;
    sos_ioctl_handler getMacTable;

}SOS_IOCTL_HANDLER,*PSOS_IOCTL_HANDLER;

typedef struct tag_SC_WLAN_HANDLER
{
    get_sc_wlan_handler getSCWlanProfile;
    sc_log_handler sc_log_record;

}SC_WLAN_HANDLER,*PSC_WLAN_HANDLER;

PSC_WLAN_HANDLER get_SC_WLAN_handler(void);

#endif
