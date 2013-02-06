#include <linux/module.h>
#include <sos_ioctl.h>
static SOS_IOCTL_HANDLER IOCTLHandler = {NULL, NULL, NULL};
static volatile PSC_WLAN_HANDLER pWLANHANDLER = NULL;


PSOS_IOCTL_HANDLER sos_get_ioctl_handler(void)
{
    return (&IOCTLHandler);
}
void register_SOS_IOCTL_handler(void)
{
    IOCTLHandler.privateSetHandler = sos_ioctl_priv_set;
    IOCTLHandler.getChannel = NULL;//get_current_channel;
    IOCTLHandler.getMacTable = NULL; //get_mac_table;

}
void register_SC_WLAN_handler(PSC_WLAN_HANDLER p)
{
    pWLANHANDLER = p;
}
PSC_WLAN_HANDLER get_SC_WLAN_handler(void)
{
    return pWLANHANDLER;
}
EXPORT_SYMBOL(sos_get_ioctl_handler);
EXPORT_SYMBOL(register_SOS_IOCTL_handler);
EXPORT_SYMBOL(register_SC_WLAN_handler);
