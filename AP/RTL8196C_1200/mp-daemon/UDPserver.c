//=============================================================================
// Copyright (c) 2006 Realtek Semiconductor Corporation.	All Rights Reserved.
//
//	Title:
//		UDPserver.c
//	Desc:
//		UDP server : accepts MP commands from the client
//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <fcntl.h>

#ifndef WIN32
#define __PACK__			__attribute__ ((packed))
#else
#define __PACK__
#endif


#define MYPORT 9034                    // the port users will be connecting to
#define BUFLEN 1024                      // length of the buffer
#define MP_TX_PACKET 0x8B71
#define MP_BRX_PACKET 0x8B73
#define MP_QUERY_STATS 	0x8B6D
#define RTL8190_IOCTL_WRITE_REG				0x89f3
#define RTL8190_IOCTL_READ_REG				0x89f4
#define MP_CONTIOUS_TX	0x8B66
#define MP_TXPWR_TRACK	0x8B6E
#define MP_QUERY_TSSI	0x8B6F
#define MP_QUERY_THER 0x8B77

#define FLASH_DEVICE_NAME		("/dev/mtd")
#define FLASH_DEVICE_NAME1		("/dev/mtdblock1")
#define HW_SETTING_HEADER_TAG		((char *)"hs")
#define HW_SETTING_OFFSET		0x6000
#define DEFAULT_SETTING_OFFSET		0x8000
#define CURRENT_SETTING_OFFSET		0xc000

#if 1
#define TAG_LEN					2
#define SIGNATURE_LEN			4
#define HW_SETTING_VER			3	// hw setting version
/* Config file header */
typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;  // Tag + version
	unsigned short len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;
PARAM_HEADER_T hsHeader;
#endif
#if 0
/* Do checksum and verification for configuration data */
#ifndef WIN32
static inline unsigned char CHECKSUM(unsigned char *data, int len)
#else
__inline unsigned char CHECKSUM(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}
#ifndef WIN32
static inline int CHECKSUM_OK(unsigned char *data, int len)
#else
__inline int CHECKSUM_OK(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}
/////////////////////////////////////////////////////////////////////////////////
static int flash_read(char *buf, int offset, int len)
{
	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);
	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( read(fh, buf, len) != len)
		ok = 0;

	close(fh);

	return ok;
}


////////////////////////////////////////////////////////////////////////////////
static int flash_write(char *buf, int offset, int len)
{
	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);

	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( write(fh, buf, len) != len)
		ok = 0;

	close(fh);
	sync();

	return ok;
}

int ReadSinguture(void)
{
	int ver;
	char *buff;
	// Read hw setting
	if ( flash_read((char *)&hsHeader, HW_SETTING_OFFSET, sizeof(hsHeader))==0 ) {
		printf("Read hw setting header failed!\n");
		return NULL;
	}

	if ( sscanf(&hsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != HW_SETTING_VER)  ) { // length is less than current
		printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n", hsHeader.signature[0],
			hsHeader.signature[1], ver, hsHeader.len);
		return NULL;
	}
	//printf("hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n", hsHeader.signature[0],	hsHeader.signature[1], ver, hsHeader.len);
	buff = calloc(1, hsHeader.len);
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return NULL;
	}
	if ( flash_read(buff, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
		printf("Read hw setting failed!\n");
		free(buff);
		return NULL;
	}
	if ( !CHECKSUM_OK(buff, hsHeader.len) ) {
		printf("Invalid checksum of hw setting!\n");
		free(buff);
		return NULL;
	}
	//printf("CS=%x\n",buff[hsHeader.len-1]);
}
#endif
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int iw_get_ext(int skfd,    /* Socket to the kernel */
           			char *ifname,        	/* Device name */
           			int request,        		/* WE ID */
           			struct iwreq *pwrq)    /* Fixed part of the request */
{
  	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);	/* Set device name */
  	return(ioctl(skfd, request, pwrq));			/* Do the request */
}

int MP_get_ext(char *ifname, char *buf, unsigned int ext_num)
{
    	int skfd;
    	struct iwreq wrq;

    	skfd = socket(AF_INET, SOCK_DGRAM, 0);
    	wrq.u.data.pointer = (caddr_t)buf;
    	wrq.u.data.length = strlen(buf);

    	if (iw_get_ext(skfd, ifname, ext_num, &wrq) < 0) {
    		printf("MP_get_ext failed\n");
		return -1;
    	}
	
    	close(skfd);
    	return 0;
}

int main(void) {
	int sockfd;                     				// socket descriptors
	struct sockaddr_in my_addr;     		// my address information
	struct sockaddr_in their_addr;  			// connector¡¦s address information
	int addr_len, numbytes;
	FILE *fp;
	char buf[BUFLEN], buf_tmp[BUFLEN], 
	pre_result[BUFLEN];				// buffer that stores message
	static char cmdWrap[500];
	static int rwHW=0;
	static int ret_value=0;
	// create a socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;         		// host byte order
	my_addr.sin_port = htons(MYPORT);     	// short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; 	// automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); 	// zero the rest of the struct

	// bind the socket with the address
	if (bind(sockfd, (struct sockaddr *)&my_addr,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		close(sockfd);
		exit(1);
	}

	addr_len = sizeof(struct sockaddr);

	printf("MP AUTOMATION daemon (ver 1.2)\n");
	//Self reading flash!!!
	#if 	0
	if(!ReadSinguture())
	{
		printf("HW Settting Error!!\n");
	}
	#endif	
	// main loop : wait for the client
	while (1) {
		//receive the command from the client
		memset(buf, 0, BUFLEN);
		memset(cmdWrap, 0, 500);
		rwHW = 0;
		if ((numbytes = recvfrom(sockfd, buf, BUFLEN, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			fprintf(stderr,"Receive failed!!!\n");
			close(sockfd);
			exit(1);
		}
		
		//printf("received command (%s) from IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));

		if(!memcmp(buf, "orf", 3)){
			strcat(buf, " > /tmp/MP.txt");
                        system(buf);
		}
		if(!memcmp(buf, "irf", 3)){
			strcat(buf, " > /tmp/MP.txt");
                        system(buf);
		}
		if (!memcmp(buf, "ther", 4)) {
                        strcpy(buf, pre_result);
                }
		if (!memcmp(buf, "tssi", 4)) {
			strcpy(buf, pre_result);
		}
		if (!memcmp(buf, "query", 5)) {

			strcpy(buf, pre_result);
		}
		if(!memcmp(buf, "cat", 3)){

			strcat(buf, " > /tmp/MP.txt");
			system(buf);		
		}
	#if 1	
		if (!memcmp(buf, "iwpriv wlan0 mp_tssi", 20)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan0", buf_tmp, MP_QUERY_TSSI);
			strcpy(buf, buf_tmp);
			printf("buf= %s\n",buf);
			usleep(1000);
		}
		else if (!memcmp(buf, "iwpriv wlan0 mp_ther", 20)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan0", buf_tmp, MP_QUERY_THER);
			strcpy(buf, buf_tmp);
			printf("buf= %s\n",buf);
			usleep(1000);
		}
		else if (!memcmp(buf, "iwpriv wlan0 mp_query", 21)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan0", buf_tmp, MP_QUERY_STATS);
			strcpy(buf, buf_tmp);
			usleep(1000);
			printf("2b= %s\n",buf);
		}
	#endif
		else {


			if ( (!memcmp(buf, "flash read", 10)) ){
				if ((fp = fopen("/tmp/MP.txt", "r")) == NULL)
					fprintf(stderr, "opening MP.txt failed !\n");
	
				if (fp) {
					fgets(buf, BUFLEN, fp);
					buf[BUFLEN-1] = '\0';
					fclose(fp);
				}	
				sprintf(pre_result, "data:%s", buf);
			}
			//ack to the client
			else if (!memcmp(buf, "flash get", 9))
				sprintf(pre_result, "%s > /tmp/MP.txt ok", buf);
			else {
				sprintf(pre_result, "%s ok", buf);;
			}
			
			if (!memcmp(buf, "iwpriv wlan0 mp_brx stop", 24)) {
				strcpy(buf, "stop");
				MP_get_ext("wlan0", buf, MP_BRX_PACKET);
			}
			else if (!memcmp(buf, "iwpriv wlan0 mp_tx", 18) && buf[18] == ' ') {
				memcpy(buf_tmp, buf+19, strlen(buf)-19);
				MP_get_ext("wlan0", buf_tmp, MP_TX_PACKET);
				strcpy(buf, buf_tmp);
			}
			
			else if (!memcmp(buf, "iwpriv wlan0 mp_ctx", 19) && buf[19] == ' ') {
				memcpy(buf_tmp, buf+20, strlen(buf)-20);
				MP_get_ext("wlan0", buf_tmp, MP_CONTIOUS_TX);
				strcpy(buf, buf_tmp);;
			}
			else if(!memcmp(buf, "iwpriv wlan0 read_reg", 21)){
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
				
			}
			else if (!memcmp(buf, "probe", 5))
				strcpy(buf, "ack");
			else if (!memcmp(buf, "verify_flw", 10)) {
				if ((fp = fopen("/tmp/MP.txt", "r")) == NULL)
					fprintf(stderr, "opening MP.txt failed !\n");
	
				if (fp) {
					fgets(buf, BUFLEN, fp);
					buf[BUFLEN-1] = '\0';
					fclose(fp);
				}
			}
			else {
#if 0
				if (!memcmp(buf, "flash get", 9))
					strcat(buf, " > /tmp/MP.txt");
#endif
					if (!memcmp(buf, "flash get", 9)){
					sprintf(cmdWrap, "flash gethw %s", buf+10);
					rwHW = 1;
					////strcat(buf, " > /tmp/MP.txt");
					strcat(cmdWrap, " > /tmp/MP.txt");
				}
				if (!memcmp(buf, "flash set", 9)) {
					sprintf(cmdWrap, "flash sethw %s", buf+10);
					rwHW = 1;
					//printf("1 sent command (%s) to IP:%s\n", pre_result, inet_ntoa(their_addr.sin_addr));
					if ((numbytes = sendto(sockfd, pre_result, strlen(pre_result), 0,
						(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
						fprintf(stderr, "send failed\n");
						close(sockfd);
						exit(1);
					}
					//printf("2 sent command (%s) to IP:%s\n", pre_result, inet_ntoa(their_addr.sin_addr));
				}
				if(rwHW == 1){
					system(cmdWrap);
				}else{
				system(buf);
				}
				
				//delay
				//open(/tmp/abc.txt)
				
			}
			
			strcpy(buf_tmp, pre_result);
			strcpy(pre_result, buf);
			strcpy(buf, buf_tmp);
		}

		if (memcmp(buf, "flash set", 9) != 0) {
			//printf("1 sent command (%s) to IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));
			if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
				(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
				fprintf(stderr, "send failed\n");
				close(sockfd);
				exit(1);
			}
			//printf("2 sent command (%s) to IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));
		}
      }

	return 0;
}
