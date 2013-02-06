#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>

#ifndef __ASUS_DVD__
#define RTL_WPA2_CLIENT

#define RTL_WPA2
#define AUTO_CONFIG
#define WIFI_SIMPLE_CONFIG
#endif

#include "iwcommon.h"
#include "../../include/1x_fifo.h"
#include "../../include/1x_ioctl.h"
#ifndef __ASUS_DVD__

#define INTERVAL  1
#define WLAN_CHR_MISC


//----------------------------------------------------------
// Local variables
//----------------------------------------------------------
static char *pidfile="/var/run/iwcontrol.pid";

#define MAX_WLAN_INTF	10
#define READ_BUF_SIZE	50


static Dot1x_RTLDListener RTLDListenerAuth[MAX_WLAN_INTF];
static Dot1x_RTLDListener RTLDListenerIapp;
static char dlisten_SendBuf[RWFIFOSIZE];

static int link_auth = FALSE;
static int link_iapp = FALSE;
static int  wlan_num = 0 ;
static char wlan_tbl[MAX_WLAN_INTF][20];

#ifdef AUTO_CONFIG
static int link_autoconf = FALSE;
static Dot1x_RTLDListener RTLDListenerAutoconf[MAX_WLAN_INTF];
#endif

#ifdef WIFI_SIMPLE_CONFIG
static int link_wscd = FALSE;
static Dot1x_RTLDListener RTLDListenerWscd[MAX_WLAN_INTF];
#endif

#ifdef WLAN_CHR_MISC
static int wl_chr_fd;
#endif
//----------------------------------------------------------
// Functions
//----------------------------------------------------------
int get_info()
{
	int skfd = -1;	/* generic raw socket desc */

	/* Create a channel to the NET kernel */
	if((skfd = sockets_open()) < 0)
	{
		perror("socket");
		exit(-1);
	}

	return skfd;
}

/*
  Initialize the SendBuf
  _________________________________________________
  | pid (4 bytes) | fifo type (1 byte) | data (*) |
  -------------------------------------------------
*/
void iw_init_sendBuf(char *ptr)
{
	pid_t pid;
	pid = getpid();
	iw_L2N((long)pid, ptr);
	*(ptr + sizeof(long)) = FIFO_TYPE_DLISTEN;
}
void iw_init_fifo(Dot1x_RTLDListener *listen, char *fifo_name)
{

	while(1){
		if((listen->WriteFIFO = open(fifo_name, O_WRONLY, 0)) < 0)
		{
			//iw_message(MESS_DBG_CONTROL, "open fifo %s error: %s", fifo_name, strerror(errno));
			iw_message(MESS_DBG_CONTROL, "wait %s to create", fifo_name);
			sleep(1);
			//exit(0);
		}
		else
			break;
	}
}

int ProcessRequestEvent(char *wlan_name)
{
	int outlen = 0;
	int retVal = 0;
	unsigned char szEvent[64];
	int iSend = FALSE;	// iSend = 1 for test
	int iRet = 0;
	int i;

#ifdef AUTO_CONFIG
	int isAutoconfEvt=0;
#endif

#ifdef WIFI_SIMPLE_CONFIG
	int isWscdEvt = FALSE;
#endif

	// Get message from wlan ioctl
	if(RequestIndication(RTLDListenerAuth[0].Iffd, wlan_name, &dlisten_SendBuf[5], &outlen) < 0)
	{
		iw_message(MESS_DBG_CONTROL, "RequestIndication return Fail");
		return 0;
	}

	// Process message
	if(dlisten_SendBuf[5] != 0)
	{
		memset(szEvent, 0, sizeof szEvent);
		switch(dlisten_SendBuf[5])
		{
		case	DOT11_EVENT_ASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "ASSOCIATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_REASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "REASSOCIATION_IND");
			iSend = TRUE;
			break;

		case 	DOT11_EVENT_AUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_REAUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "REAUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_DEAUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "DEAUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_DISASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "DISASSOCIATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_MIC_FAILURE:
			sprintf(szEvent, (char*)"Receive Event %s", "MIC_FAILURE");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAPOLSTART:
			sprintf(szEvent, (char*)"Receive Event %s", "EAPOL_START");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAP_PACKET:
			sprintf(szEvent, (char*)"Receive Event %s", "EAP_PACKET");
#ifdef WIFI_SIMPLE_CONFIG
			isWscdEvt = TRUE;
#endif
			iSend = TRUE;
			break;
#ifdef RTL_WPA2
		case	DOT11_EVENT_EAPOLSTART_PREAUTH:
			sprintf(szEvent, (char*)"Receive Event %s", "EAPOLSTART_PREAUTH");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAP_PACKET_PREAUTH:
			sprintf(szEvent, (char*)"Receive Event %s", "EAP_PACKET_PREAUTH");
			iSend = TRUE;
			break;
#endif

#ifdef RTL_WPA2_CLIENT
		case	DOT11_EVENT_WPA2_MULTICAST_CIPHER:
			sprintf(szEvent, (char*)"Receive Event %s", "WPA2_MULTICAST_CIPHER");
			iSend = TRUE;
			break;
#endif

		case	DOT11_EVENT_WPA_MULTICAST_CIPHER:
			sprintf(szEvent, (char*)"Receive Event %s", "WPA_MULTICAST_CIPHER");
			iSend = TRUE;
			break;

#ifdef AUTO_CONFIG
		case	DOT11_EVENT_AUTOCONF_ASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_ASSOC_IND");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_ASSOCIATION_CONFIRM:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_ASSOC_CONFIRM");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_PACKET:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_PACKET");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_LINK_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_LINK_IND");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;
#endif

#ifdef WIFI_SIMPLE_CONFIG
		case	DOT11_EVENT_WSC_PIN_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_PIN_IND");
			isWscdEvt = TRUE;
			break;
#ifdef CONFIG_IWPRIV_INTF
		case	DOT11_EVENT_WSC_START_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_START_IND");
			isWscdEvt = TRUE;
			break;
		//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
		case	DOT11_EVENT_WSC_MODE_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_MODE_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_STATUS_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_STATUS_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_METHOD_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_METHOD_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_STEP_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_STEP_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_OOB_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_OOB_IND");
			isWscdEvt = TRUE;
			break;
#endif  //ifdef CONFIG_IWPRIV_INTF
		case	DOT11_EVENT_WSC_PROBE_REQ_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_PROBE_REQ_IND");
			isWscdEvt = TRUE;
			break;

		case	DOT11_EVENT_WSC_ASSOC_REQ_IE_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_ASSOC_REQ_IE_IND");
			isWscdEvt = TRUE;
			break;
#endif

		default:
			sprintf(szEvent, (char*)"Receive Invalid or Unhandled Event %d",
				dlisten_SendBuf[5]);
			iSend = FALSE;
			break;
		}

		iw_message(MESS_DBG_CONTROL, "[iwcontrol]: %s", szEvent);
		if(iSend)
		{
#ifdef AUTO_CONFIG
			if (link_autoconf && isAutoconfEvt) {
				for(i=0; i < link_autoconf; i++){
					if(!strcmp(RTLDListenerAutoconf[i].wlanName,wlan_name)){
						if((iRet = write(RTLDListenerAutoconf[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
							iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
						else
							iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
					}
				}
				goto ret_process;
			}
#endif
			if(link_auth)
			{
				for(i=0; i < wlan_num; i++){

					if(!strcmp(RTLDListenerAuth[i].wlanName,wlan_name)){
						if((iRet = write(RTLDListenerAuth[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
							iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
						else
							iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
					}
				}
			}

			if(link_iapp)
			{
				if((iRet = write(RTLDListenerIapp.WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
					iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
				else
					iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
			}
#ifdef AUTO_CONFIG
ret_process:
#endif
			retVal = (dlisten_SendBuf[6] == TRUE)? TRUE : FALSE;	//If more event
		}
#ifdef WIFI_SIMPLE_CONFIG
		if (isWscdEvt)
		{
			if (link_wscd && isWscdEvt) {
				for(i=0; i < link_wscd; i++){
					if(!strcmp(RTLDListenerWscd[i].wlanName,wlan_name)){
						if((iRet = write(RTLDListenerWscd[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
							iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
						else
							iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
					}
				}
			}
			retVal = (dlisten_SendBuf[6] == TRUE)? TRUE : FALSE;	//If more event
		}
#endif
	}

	return retVal;
}


void RequestEvent(int sig_no)
{
	u_long	ulMoreEvent;
	int i ;
	for(i=0 ;i < wlan_num; i ++){
		do {
			ulMoreEvent = ProcessRequestEvent(wlan_tbl[i]);
		} while(ulMoreEvent);
	}
}

// david ----------------------------------------
static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0)
	{
		printf("Unable to open pidfile %s\n", pidfile);
	}
	else
	{
		lockf(pid_fd, F_LOCK, 0);
	}

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL)
	{
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}

	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}
//-----------------------------------------------
/* parsing var pid file for fifo create */
int parsing_var_pid(Dot1x_RTLDListener *auth, Dot1x_RTLDListener *autoconf, Dot1x_RTLDListener *wscd)
{
       DIR *dir;
        struct dirent *next;
        int i;
        dir = opendir("/var/run");
        if (!dir) {
                printf("Cannot open %s", "/var/run");
                return 0;
        }
        while ((next = readdir(dir)) != NULL){
                iw_message(MESS_DBG_CONTROL,"iwcontrol:/var/run/%s\n", next->d_name);
		if(!strncmp(next->d_name, "auth", strlen("auth"))){
			sscanf(next->d_name, "auth-%s.pid", auth[link_auth].wlanName);
// when repeater mode is used, its name length may be extended to 9 chars, 2005-8-8 david
//			auth[link_auth++].wlanName[5] = '\0' ;
			for (i=0; i<16; i++) {
				if (auth[link_auth].wlanName[i] == '.')
					auth[link_auth].wlanName[i] = 0;
			}
			link_auth++;
			iw_message(MESS_DBG_CONTROL,"auth[%d].wlanName=%s\n", link_auth-1, auth[link_auth-1].wlanName);
		}
		if(!strncmp(next->d_name, "iapp", strlen("iapp"))){
			link_iapp = TRUE;
			iw_message(MESS_DBG_CONTROL,"link_iapp =true\n");
		}
#ifdef AUTO_CONFIG
		if(!strncmp(next->d_name, "autoconf", strlen("autoconf"))){
			sscanf(next->d_name, "autoconf-%s.pid", autoconf[link_autoconf].wlanName);
			autoconf[link_autoconf++].wlanName[5] = '\0' ;
			iw_message(MESS_DBG_CONTROL,"autoconf[%d].wlanName=%s\n", link_autoconf-1, autoconf[link_autoconf-1].wlanName);
		}
#endif
#ifdef WIFI_SIMPLE_CONFIG
		if(!strncmp(next->d_name, "wscd", strlen("wscd"))){
			sscanf(next->d_name, "wscd-%s.pid", wscd[link_wscd].wlanName);
			//wscd[link_wscd++].wlanName[5] = '\0' ;
			for (i=0; i<16; i++) {
				if (wscd[link_wscd].wlanName[i] == '.')
					wscd[link_wscd].wlanName[i] = 0;
			}
			link_wscd++;
			iw_message(MESS_DBG_CONTROL,"wscd[%d].wlanName=%s\n", link_wscd-1, wscd[link_wscd-1].wlanName);
		}
#endif
	}
	return 0;
}

#ifdef AUTO_CONFIG
#define AUTOCONFIG_FIFO 	"/var/autoconf-%s.fifo"
#endif

#ifdef WIFI_SIMPLE_CONFIG
#define	WSCD_FIFO			"/var/wscd-%s.fifo"
#endif

int main(int argc, char *argv[])
{
	int i;
	char *iapp_fifo = "/var/iapp.fifo";
	char fifo_buf[30];
	int poll = 0; //david

	// destroy old process
	{
		FILE *fp;
		char line[20];
		pid_t pid;

		if((fp = fopen(pidfile, "r")) != NULL)
		{
			fgets(line, sizeof(line), fp);
			if(sscanf(line, "%d", &pid))
			{
				if(pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
		}
	}

	setsid(); // david, requested from abocom

	// depend on arguments to decide link fifo
	for(i=1; i<argc; i++)
	{
		if(!strncmp("wlan", argv[i], strlen("wlan"))){
			if(wlan_num >= MAX_WLAN_INTF){
				printf("Only  %d Wlan interace is supported\n", MAX_WLAN_INTF);
				return -1;
			}
			strcpy(wlan_tbl[wlan_num], argv[i]);
			wlan_num++ ;
		}
// david ------------------------------------------------------
		else if(!strncmp("poll", argv[i], strlen("poll"))){
			poll = 1;
		}
//-------------------------------------------------------------
		else
		{
			printf("Usage: %s [wlan0] [wlan1] ... \n", argv[0]);
			return -1;
		}
	}

	if(wlan_num == 0){
		printf("At least one wlan interface should be binded\n");
		return -1 ;
	}

	/* parsing /var/iapp.pid or /var/auth*.pid , /var/autoconf*.conf */
	if(parsing_var_pid(RTLDListenerAuth, RTLDListenerAutoconf, RTLDListenerWscd) < 0){
		printf("parsing pid failed\n");
		return -1;
	}
	// david ------------------------------------------
	// delete old fifo and create filo
	{
		int pid_fd;

		pid_fd = pidfile_acquire(pidfile);
		if(daemon(0,1) == -1)
		{
			printf("fork iwcontrol error!\n");
			exit(1);
		}
		pidfile_write_release(pid_fd);
	}
	//---------------------------------------------------
	// init send buffer pid header
	iw_init_sendBuf(dlisten_SendBuf);

	// init fifo and socket
	if(link_auth){
		for(i=0 ; i < link_auth; i++){
			sprintf(fifo_buf, DAEMON_FIFO, RTLDListenerAuth[i].wlanName);
			iw_message(MESS_DBG_CONTROL,"open auth fifo %s\n", fifo_buf);
			iw_init_fifo(&RTLDListenerAuth[i], fifo_buf);
		}
	}
	if(link_iapp){
		iw_message(MESS_DBG_CONTROL,"open iapp fifo %s\n", iapp_fifo);
		iw_init_fifo(&RTLDListenerIapp, iapp_fifo);
	}

#ifdef AUTO_CONFIG
	if(link_autoconf){
		for(i=0 ; i < link_autoconf; i++){
			sprintf(fifo_buf, AUTOCONFIG_FIFO, RTLDListenerAutoconf[i].wlanName);
			iw_message(MESS_DBG_CONTROL,"open autoconfig fifo %s\n", fifo_buf);
			iw_init_fifo(&RTLDListenerAutoconf[i], fifo_buf);
		}
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
	if(link_wscd){
		for(i=0 ; i < link_wscd; i++){
			sprintf(fifo_buf, WSCD_FIFO, RTLDListenerWscd[i].wlanName);
			iw_message(MESS_DBG_CONTROL,"open wscd fifo %s\n", fifo_buf);
			iw_init_fifo(&RTLDListenerWscd[i], fifo_buf);
		}
	}
#endif

	RTLDListenerAuth[0].Iffd = get_info();
	if(RTLDListenerAuth[0].Iffd <= 0)
	{
		perror("Socket fd return 0");
		exit(0);
	}

	// infinite loop
#ifdef WLAN_CHR_MISC
	if (!poll)	// david
	{
		int fdflags, idx=0, wlan0_up=0, wlan1_up=0;
		char dev_name[20];
		for(i=0; i<wlan_num; i++)
		{
			if (strncmp("wlan0", wlan_tbl[i], 5) == 0) {
				if (wlan0_up)
					continue;
				else
					wlan0_up = 1;
				idx = 0;
			}

			if (strncmp("wlan1", wlan_tbl[i], 5) == 0) {
				if (wlan1_up)
					continue;
				else
					wlan1_up = 1;
				idx = 1;
			}

			sprintf(dev_name, "/dev/wl_chr%d", idx);

			if((wl_chr_fd = open(dev_name, O_RDWR, 0)) < 0)
			{
				int retval;
				if (idx == 0)
					retval = RegisterPID(RTLDListenerAuth[0].Iffd, "wlan0");
				else
					retval = RegisterPID(RTLDListenerAuth[0].Iffd, "wlan1");

				if (retval > 0)
					signal(SIGIO, RequestEvent);
				else
				{
					printf("Warning: unable to open an wl_chr device and PID registration fail.\n");
					exit(1);
				}
			}
			else
			{
				signal(SIGIO, RequestEvent);

				fcntl(wl_chr_fd, F_SETOWN, getpid());
				fdflags = fcntl(wl_chr_fd, F_GETFL);
				fcntl(wl_chr_fd, F_SETFL, fdflags | FASYNC);
			}
		}

		while(1)
			pause();
	}
#endif
	while( 1 )
	{
		RequestEvent(SIGIO);
		sleep(INTERVAL);
	}

	return 0;
}


#else
//#include "1x_common.h"
//#include "wireless.h"

#define MAXDATALEN      1560	// jimmylin: org:256, enlarge for pass EAP packet by event queue

int read_wlan_evt(	int skfd, char *ifname, char *out)
{
	struct iwreq wrq;
	DOT11_REQUEST *req;

  	/* Get wireless name */
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
  	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

	req = (DOT11_REQUEST *)malloc(MAXDATALEN);
	wrq.u.data.pointer = (caddr_t)req;
	req->EventId = DOT11_EVENT_REQUEST;
	wrq.u.data.length = sizeof(DOT11_REQUEST);

  	if (ioctl(skfd, SIOCGIWIND, &wrq) < 0) {
    	// If no wireless name : no wireless extensions
		free(req);
		strerror(errno);
   		return(-1);
	}
  	else
		memcpy(&out[5], wrq.u.data.pointer, wrq.u.data.length);

	free(req);

	if (out[5] != 0) {
		out[4] = FIFO_TYPE_DLISTEN;
		return wrq.u.data.length+5;;
	}

	return 0;
}
#endif

