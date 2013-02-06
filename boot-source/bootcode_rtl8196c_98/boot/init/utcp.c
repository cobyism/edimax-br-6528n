#include <asm/system.h>
#include "etherboot.h"
#include "nic.h"
#include "rtk.h"



#if defined(RTL8196B)	
#include <asm/rtl8196.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif

#include "utcp.h"

/*httpd use*/
#if 0
static	unsigned char indexdata[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Upload Page</title>\
<META content='text/html; charset=iso-8859-1' http-equiv=Content-Type>\
<META http-equiv=Pragma content=no-cache >\
<META HTTP-EQUIV='CACHE-CONTROL' CONTENT='NO-CACHE'></head>\
<body>\
<form enctype=multipart/form-data method=post>\
Upload a image file: <input name=userfile type=file>\
<input type=submit value='Send File'>\
</form>\
</body> \
</html>";
#else
static	unsigned char indexdata[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html><head><title>System Repair</title>\
<META content='text/html; charset=iso-8859-1' http-equiv=Content-Type>\
<META http-equiv=Pragma content=no-cache >\
<META HTTP-EQUIV='CACHE-CONTROL' CONTENT='NO-CACHE'>\
</head><body><form enctype=multipart/form-data method=post><center>\
<br><br>\
<font size=5><b>Emergency Web Server</b></font>\
<br>\
<table>\
<br>\
<tr><td align=right>File</td><td><input type=file name=userfile value=></td>\
<td><input type=submit value='upload'></td></tr></table></center>\
</form></body></html>";
#endif

static	unsigned char postsuccess[]="Update successfully!<br><br>Update in progress.<br> Do not turn off or reboot the Device during this time.\
";

static	unsigned char postinvalidfile[]="Invalid image file.\
";

unsigned char postresponse[1024];

static	unsigned char postresponseheader[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Upload Complete</title></head>\
<body>";
static	unsigned char postresponsefooter[]="seconds</body></html>";

static int content_offset=0;
static int content_length=0;
static int httpd_mem_len=0;
static int upload_len=0;
static int foundboundary=0;
static unsigned short ip_seq=0;

extern void checkAutoFlashing(unsigned long startAddr, int len, int autorebootenabled);
extern int imageFileValid(unsigned long startAddr, int len);
extern void autoreboot();
extern int autoBurn;
extern struct arptable_t  arptable_tftp[3];
extern char eth0_mac_httpd[];
extern char eth0_ip_httpd[];
extern char message[];

/*write upload image to 0x80400000*/
static unsigned long httpd_mem =(IMAGE_MEM_ADDR);
static unsigned char readyToUpgrade = 0;
/*****************************************************/

static UTCPINFO_T utcp_info;

/******************************************************/
static void writeImagetoflash(char *image,int len)
{
	/*reference the upgrade firmware*/
	if(autoBurn)
	{
		/*no need to auto reboot*/
	   	checkAutoFlashing((unsigned long)(image), len,0);
 	}     
}
#if 0 //UNUSED
void dumppacket(char *packet, int len)
{
	int i;
	prom_printf("\n");
	for(i=0;i<len;i++)
	 {
	 	prom_printf("%x ",*(unsigned char*)&packet[i]);
		if(!((i+1) % 15))
			prom_printf("\n");
	 }
}
#endif
static int sameip(in_addr *ip1, in_addr *ip2)
{
	if(ip1->ip[0] == ip2->ip[0]) 
		if(ip1->ip[1] == ip2->ip[1]) 
			if(ip1->ip[2] == ip2->ip[2]) 
				if(ip1->ip[3] == ip2->ip[3])
				return 1;
	return 0;
}

static void assignip(in_addr *iptoassign, in_addr *ipaddr)
{
	iptoassign->ip[0] = ipaddr->ip[0];
	iptoassign->ip[1] = ipaddr->ip[1];
	iptoassign->ip[2] = ipaddr->ip[2];
	iptoassign->ip[3] = ipaddr->ip[3];
	return ;
}

static void assignseq(int32_num *seqtoassign, int32_num *seq)
{
	seqtoassign->numArray[0] = seq->numArray[0];
	seqtoassign->numArray[1] = seq->numArray[1];
	seqtoassign->numArray[2] = seq->numArray[2];
	seqtoassign->numArray[3] = seq->numArray[3];
	return;
}

/*since seq maybe unaligned. so inc one like below
  *BIG ENDIAN!!!
  */
static void incseq(int32_num *seq,int len)
{
	unsigned long tmpnum;
	memcpy((void *)&tmpnum,seq->numArray,4);
	tmpnum+=len;
	memcpy(seq->numArray,(void *)&tmpnum,4);
	return;
}

static int findContentlength(char *payload,int length)
{
	int i;
	int content_length=0;
	// find out the content-length
	for(i=0;i<length;i++) {
		while(1) {
			if (payload[i]=='\r'&&payload[i+1]=='\n') {
				i+=2;
				break;
			}
			i++;
		}
		
		if (payload[i]=='C'&&payload[i+1]=='o'&&payload[i+2]=='n'&&payload[i+3]=='t'&&
			payload[i+4]=='e'&&payload[i+5]=='n'&&payload[i+6]=='t'&&payload[i+7]=='-'&&
			payload[i+8]=='L'&&payload[i+9]=='e'&&payload[i+10]=='n'&&payload[i+11]=='g') {
			i += 15;
			content_length = strtol(&payload[i],NULL,10);
			break;
		}
	}
	return content_length;
}
static int findContentoffset(char *payload,int length)
{
	int i;
	for(i=0;i<length;i++) {
	while(1) {
		if (payload[i]=='\r'&&payload[i+1]=='\n') {
			i+=2;
			break;
		}
		i++;
	}
	
	while(i<length) {
		if (payload[i]=='\r'&&payload[i+1]=='\n'&&payload[i+2]=='\r'&&payload[i+3]=='\n') {
			i+=4;
			return i;
		}
		i++;
	}
}
}
static int findimagehead(char *payload, int length, int *headlen)
{
	int i;
	// find out the head
	// two 0D0A0DA encounted .means head of image.
	// but they maybe in in Two packet
	if(2==foundboundary)
	{
		/*image header already found. just return 1. copy whole packet*/
		*headlen=0;
		return 1;
	}
	
	for(i=0;i<length;i++) {
		while(1) {
			if (payload[i]=='\r'&&payload[i+1]=='\n') {
				i+=2;
				break;
			}
			i++;
		}
		
		while(i<length) {
			if (payload[i]=='\r'&&payload[i+1]=='\n'&&payload[i+2]=='\r'&&payload[i+3]=='\n') {
				foundboundary++;
				i+=4;
				if(2==foundboundary)
				{
					*headlen=i;
					 return 1;
				}
			}
			i++;
		}
	}
	return 0;
}
static int utcp_GetIpStr(unsigned char *httpIp)
{
	int i,j,shift=0;
	unsigned char	ipHttpd[4];
	
	if(*(unsigned long *)eth0_ip_httpd == 0)
	{
		unsigned char	defIp[] = {192,168,1,254};
		memcpy(ipHttpd, defIp, 4);		
	}
	else
	{
		memcpy(ipHttpd, eth0_ip_httpd, 4);
	}
	
	for(i=0;i<4;i++)
	{
		int cutNum=0;
		int firstNumFlag=0;
		
		for(j=100;j!=0;j/=10)
		{
			int num;
			char charNum;
			num = ((ipHttpd[i]&0x000000FF)-cutNum)/j;
			switch(num)
			{
				case 0:
					if(firstNumFlag || j==1)
					{
						httpIp[shift] = '0';
						shift++;
						cutNum+=num*j;
					}
					break;
				case 1:
					firstNumFlag = 1;
					httpIp[shift] = '1';
					shift++;
					cutNum+=num*j;
					break;
				case 2:
					firstNumFlag = 1;
					httpIp[shift] = '2';
					shift++;
					cutNum+=num*j;
					break;
				case 3:
					firstNumFlag = 1;
					httpIp[shift] = '3';
					shift++;
					cutNum+=num*j;
					break;
				case 4:
					firstNumFlag = 1;
					httpIp[shift] = '4';
					shift++;
					cutNum+=num*j;
					break;
				case 5:
					firstNumFlag = 1;
					httpIp[shift] = '5';
					shift++;
					cutNum+=num*j;
					break;
				case 6:
					firstNumFlag = 1;
					httpIp[shift] = '6';
					shift++;
					cutNum+=num*j;
					break;
				case 7:
					firstNumFlag = 1;
					httpIp[shift] = '7';
					shift++;
					cutNum+=num*j;
					break;
				case 8:
					firstNumFlag = 1;
					httpIp[shift] = '8';
					shift++;
					cutNum+=num*j;
					break;
				case 9:
					firstNumFlag = 1;
					httpIp[shift] = '9';
					shift++;
					cutNum+=num*j;
					break;														
			}
			//httpIp[shift]='\0';
			//prom_printf("\n httpIp=[%s]",httpIp);
		}

		if(i!=3)
		{
			httpIp[shift] = '.';
			shift++;
		}
	}
	//shift++;
	//httpIp[shift]='\0';
}
static int createResponseData(unsigned char *postData, unsigned char *postInfo, int time, int *packetlen)
{
	int len=0;
	unsigned char httpIp[16];
	unsigned char point=46;
#if 0	
	unsigned char postCnt_1[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\r\n\
<html>\
<head><title>Upload Complete</title></head>\n\
<script language=JavaScript><!--\n\
var count = 60;\n\
function do_count_down(){\n\
document.getElementById(\"show_sec\").innerHTML = count;\n\
if(count == 0) {\
location.href='http://";

unsigned char postCnt_2[]="'; return false;}\n\
if (count > 0) {\n\
count--;setTimeout('do_count_down()',1000);}}\n\
//-->\n\
</script></head>\n\
<body onload=\"do_count_down();\"><blockquote>\n\
Update successfully!<br><br>Update in progress.<br> Do not turn off or reboot the Device during this time.\
</h4>\n\
<P align=left><h4>Please wait <B><SPAN id=show_sec></SPAN></B>&nbsp;seconds ...</h4></P>\n\
</blockquote></body></html>";
#else
	unsigned char postCnt_1[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\r\n\
<html><head><title>Upgrade Successfully</title>\n\
<META content='text/html; charset=iso-8859-1' http-equiv=Content-Type>\n\
<META http-equiv=Pragma content=no-cache >\n\
<META HTTP-EQUIV='CACHE-CONTROL' CONTENT='NO-CACHE'>\n\
</head>\n\
<script language=JavaScript><!--\n\
var count = 60;\n\
function do_count_down(){\n\
document.getElementById('show_sec').innerHTML = count;\n\
if(count == 0) {location.href='http://";

	unsigned char postCnt_2[]="'; return false;}\n\
if (count > 0) {count--;setTimeout('do_count_down()',1000);}}\n\
//--></script></head><body onload='do_count_down();'>\
<br><br>\
<center><font size=4><b>Updateing File</b></font><br><p><hr>\
<table ><br><tr><td align=center><font color=red><b>Don't Power Down.</b></font></td>\
</tr><tr><td><br><P align=center>Please wait for <font color=red><B><SPAN id=show_sec></SPAN></B></font>&nbsp;seconds ...</P>\
</center></body></html>";
#endif


	memset(httpIp, 0x00, sizeof(httpIp));
	utcp_GetIpStr(httpIp);
	
	memcpy(postresponse+len,postCnt_1,sizeof(postCnt_1));
	len+=sizeof(postCnt_1);
	memcpy(postresponse+len,httpIp,sizeof(httpIp));
	len+=sizeof(httpIp);	
	memcpy(postresponse+len,postCnt_2,sizeof(postCnt_2));
	len+=sizeof(postCnt_2);

	*packetlen = len;

}
static int httpuploadfile(unsigned char *payload , int length)
{
	char *tmppd;
	int i=0;
	int lastpacket=0;
	int headlen=0;
	int retVal=0;
	int len=0;

prom_printf(".");
	
	/*since IE and firefox fragment the packet in different way.
	*IE  			|len-n*mss |+| mss| + |mss | + ... + |mss |
	*firefox		|mss|     +     |mss| +... +|mss|   +   |len-n*mss |
	*	
	*/
	if(0 == content_length)
	{
		content_length=findContentlength(payload,length);
	}

	if(0 == content_offset)
	{
		content_offset = findContentoffset(payload,length);
	}
	
	if(findimagehead(payload, length, &headlen))
	{
		/*in order to copy image aligned. we need find the image header. */
		memcpy((void *)(httpd_mem+httpd_mem_len),payload+headlen,length-headlen);
		httpd_mem_len+=(length-headlen);
	}
	upload_len+=length;
	//prom_printf("\n#upload_len=%d\n",upload_len);	
	if(upload_len >= (content_length+content_offset))
	{
		/*all image should be uploaded*/
		prom_printf("\n#web upload success!\n");
		//prom_printf("upload_len %x content_length %x content_offset %x httpd_mem %x httpd_mem_len %x\n",upload_len,content_length,content_offset,httpd_mem,httpd_mem_len);
		if((retVal=imageFileValid((unsigned char *)httpd_mem,httpd_mem_len)) >0)
		{
			tcpAck();
			//writeImagetoflash((unsigned char *)httpd_mem,httpd_mem_len);
			createResponseData(postresponse, postsuccess, 60, &len);
#if 0			
			memcpy(postresponse,postresponseheader,sizeof(postresponseheader));
			len+=sizeof(postresponseheader);
			memcpy(postresponse+len,postsuccess,sizeof(postsuccess));
			len+=sizeof(postsuccess);
			memcpy(postresponse+len,postresponsefooter,sizeof(postresponsefooter));
			len+=sizeof(postresponsefooter);
#endif
			tcpwrite(postresponse,len);		
			tcpSendFinAck();
			for(i=0;i<3000000;i++)
			{
				i=i;
			}
			readyToUpgrade = 1;
			/* When we receive FIN from PC, we will start to write image.
			writeImagetoflash((unsigned char *)httpd_mem,httpd_mem_len);
			autoreboot();
			*/
		}
		else
		{
			memcpy(postresponse,postresponseheader,sizeof(postresponseheader));
			len+=sizeof(postresponseheader);
			if(0 == retVal)
			{
				memcpy(postresponse+len,postinvalidfile,sizeof(postinvalidfile));
				len+=sizeof(postinvalidfile);
			}
			else
			{
				memcpy(postresponse+len,message,strlen(message));
				len+=strlen(message);
			}
			memcpy(postresponse+len,postresponsefooter,sizeof(postresponsefooter));
			len+=sizeof(postresponsefooter);
			tcpwrite(postresponse,len);
			tcpSendFinAck();
		}
		utcp_info.tcpi_state=TCP_FIN_WAIT1;
		return 1;
	}
	return 0;
}

/*-----------------------------------------------------------------------------------*/
/* chksum:
 *
 * Sums up all 16 bit words in a memory portion. Also includes any odd byte.
 * This function is used by the other checksum functions.
 *
 * For now, this is not optimized. Must be optimized for the particular processor
 * arcitecture on which it is to run. Preferebly coded in assembler.
 */
/*-----------------------------------------------------------------------------------*/
static unsigned int chksum(void *dataptr, Int16 len)
{
	unsigned int acc;

	for(acc = 0; len > 1; len -= 2) {
		acc += *((Int16 *)dataptr)++;
	}

	/* add up any odd byte */
	if(len == 1) {
		acc += ((Int16)((*(Int8 *)dataptr) & 0xff) << 8);
	}

	return acc;
}

/*-----------------------------------------------------------------------------------*/
/* inet_chksum_pseudo:
 *
 * Calculates the pseudo Internet checksum used by TCP and UDP for a pbuf chain.
 */
/*-----------------------------------------------------------------------------------*/
Int16
inet_chksum_pseudo(char *packet,
		   Int32 *src, Int32 *dest,
		   Int8 proto, Int32 proto_len)
{
  Int32 acc;
  Int32 i, delayTime=1;
  acc = 0;
  acc += chksum(packet,proto_len);
  acc += (*src&0xffff);
  acc += ((*src>>16)&0xffff);
  acc += (*dest&0xffff);
  acc += ((*dest>>16)&0xffff);

  acc += (Int32)htons((Int16)proto);
  acc += (Int32)htons(proto_len); 
  while(acc >> 16) {
    acc = (acc & 0xffff) + (acc >> 16);
  }
  return ~(acc & 0xffff);
}
/*now tcp write only can send tcp payload less than 1024.!!!*/
static void tcpwrite(unsigned char *packet, int length)
{
	unsigned char packet_buff[1024];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr* tcpheader;
	memset(packet_buff,0,sizeof(packet_buff));
	tcpheader= (struct tcphdr *)(tmppacket+sizeof(struct iphdr));
	if(length+sizeof(struct iphdr)+sizeof(struct tcphdr) > 1024)
	{
		prom_printf("packet too long to send!\n");
	}
	memcpy(tmppacket+sizeof(struct iphdr)+sizeof(struct tcphdr),packet,length);
	/*To ack*/
	tcpheader->tcpflags[1]=1<<ACK_OFFSET;
	/*Set data offset*/
	tcpheader->tcpflags[0]=(20>>2)<<4;
	tcpoutput(tcpheader,tmppacket,length);
}
static void tcpchecksum(unsigned char *packet, int length)
{
	struct iphdr *ipheader;
	struct tcphdr *tcpheader;
	ipheader=(struct iphdr *)packet;
	tcpheader=(struct tcphdr *)(packet+sizeof(struct iphdr));
	tcpheader->check=htons(inet_chksum_pseudo((char*)tcpheader,(Int32 *)&(ipheader->src.ip[0]),(Int32 *)&(ipheader->dest.ip[0]),IP_TCP,(length-sizeof(struct iphdr))));
	return;
}
/*BOOT_EVENT0_TCP_SYN*/
static void tcpSendSynAck()
{
	/*SYN Recived. To Send SYN ACK packet*/
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcpheader;
	unsigned char *option_field;
	memset(packet_buff,0,sizeof(packet_buff));
	
	tcpheader= (struct tcphdr *)(tmppacket+sizeof(struct iphdr));
	/*fill syn acK*/
	tcpheader->tcpflags[1] = (1<<SYN_OFFSET | 1<<ACK_OFFSET);
	
	/*set mss to 1460*/
	option_field=tmppacket+sizeof(struct iphdr)+20;
	option_field[0]=0x02;
	option_field[1]=0x04;
	option_field[2]=0x05;
	option_field[3]=0xb4;
	option_field[4]=0x01;
	option_field[5]=0x01;
	option_field[6]=0x04;
	option_field[7]=0x02;
	
	/*set tcpheaderlen*/
	tcpheader->tcpflags[0]=((28>>2) << 4);

	/*set seq=0 and ack_seq=1*/	
	tcpoutput(tcpheader,tmppacket,0);
}
/*BOOT_EVENT1_TCP_SYN_ACK_ACK*/ 

static void tcpSendFinAck()
{
//	prom_printf("%s %d\n",__FUNCTION__,__LINE__);
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcpheader;
	memset(packet_buff,0,sizeof(packet_buff));
	tcpheader= (struct tcphdr *)(tmppacket+sizeof(struct iphdr));

	/*set ack*/
	tcpheader->tcpflags[1] = (1<<ACK_OFFSET | 1<<FIN_OFFSET);
	/*set tcpheaderlen*/
	tcpheader->tcpflags[0]=((20>>2)<<4);

	tcpoutput(tcpheader,tmppacket,0);
}

static void tcpSendFin()
{
//	prom_printf("%s %d\n",__FUNCTION__,__LINE__);
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcpheader;
	memset(packet_buff,0,sizeof(packet_buff));
	tcpheader= (struct tcphdr *)(tmppacket+sizeof(struct iphdr));

	/*set ack*/
	tcpheader->tcpflags[1] = (1<<FIN_OFFSET);
	/*set tcpheaderlen*/
	tcpheader->tcpflags[0]=((20>>2)<<4);

	tcpoutput(tcpheader,tmppacket,0);
}

static void tcpAck()
{
//	prom_printf("%s %d\n",__FUNCTION__,__LINE__);
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcpheader;
	memset(packet_buff,0,sizeof(packet_buff));
	tcpheader= (struct tcphdr *)(tmppacket+sizeof(struct iphdr));

	/*set ack*/
	tcpheader->tcpflags[1] = 1<<ACK_OFFSET;
	/*set tcpheaderlen*/
	tcpheader->tcpflags[0]=((20>>2)<<4);

	tcpoutput(tcpheader,tmppacket,0);
}

static void tcpConnected()
{
	/*receive ACK of syn ack. Connected*/
	/*seq add 1*/
	//prom_printf("%s %d\n",__FUNCTION__,__LINE__);
	incseq(&utcp_info.tcpi_seq,1);
}

/*BOOT_EVENT2_TCP_DATA_ACK*/	    
static int  tcpInputData()
{
	char tmpget[]="GET";
	char tmppost[]="POST";
	struct tcphdr *tcpheader;
	struct iphdr *ipheader;
	int length;
	int doff;
	int i;
	int retval;
	unsigned char *payload;
	ipheader = (struct iphdr *)&nic.packet[ETH_HLEN];
	tcpheader = (struct tcphdr *)&nic.packet[ETH_HLEN+ sizeof(struct iphdr)];

	doff=((tcpheader->tcpflags[0] & 0xF0) >> 4) << 2;
	payload=(unsigned char *)(&nic.packet[ETH_HLEN+ sizeof(struct iphdr)]+doff);
	length=ntohs(ipheader->len)-((ipheader->verhdrlen &0xF)<<2)-doff;
	
	if(payload[0] =='G' && payload[1] == 'E' && payload[2] == 'T')
	{
		/*Get Method*/
		tcpwrite(indexdata,sizeof(indexdata));
		utcp_info.tcpi_state = TCP_FIN_WAIT1;
		tcpSendFinAck();
		retval=1;
	}
	else if(payload[0] =='P' && payload[1] == 'O' && payload[2] == 'S' && payload[3] == 'T')					
	{
		retval=httpuploadfile(payload,length);
		utcp_info.uploadstarted=1;
	}else
	{
		if(0==length)
		{
			/*it's only a simple ACK no data*/
			retval=1;
		}
		else if(utcp_info.uploadstarted)
		{
			retval=httpuploadfile(payload,length);
		}
		
	}
	/*Data Recived .Ack the Data. retval 0 means need to ack last  packet*/
	return retval;
}
/*BOOT_EVENT3_TCP_RST*/	   
static void tcpReset()
{
	unsigned char packet_buff[128];
	unsigned char *tmppacket=(unsigned char *)BYTES_ALIGN((unsigned long)packet_buff);
	struct tcphdr *tcpheader;
	memset(packet_buff,0,sizeof(packet_buff));
	tcpheader= (struct tcphdr *)(tmppacket+sizeof(struct iphdr));

	/*set rst*/
	tcpheader->tcpflags[1]= (1<<RST_OFFSET);
	/*set tcpheaderlen*/
	tcpheader->tcpflags[0]=(20>>2)<<4;

	tcpoutput(tcpheader,tmppacket,0);
}

static void tcpoutput(struct tcphdr *tcpheader,unsigned char *packet, int length)
{	
	/*fill tcp header*/
	tcpheader->source =htons(utcp_info.tcpi_dport);
	tcpheader->dest = htons(utcp_info.tcpi_sport);
	assignseq(&tcpheader->ack_seq,&utcp_info.tcpi_ack);
	assignseq(&tcpheader->seq,&utcp_info.tcpi_seq);		
	incseq(&utcp_info.tcpi_seq,length);
	/*set window size 2048 . reduce peer's burst sending*/
	tcpheader->window =htons(2048);
	/*checksum*/
	//tcpheader->check=0;
	length+=(((tcpheader->tcpflags[0] & 0xF0)>>4)<<2);
	ipoutput(packet,length,IP_TCP);
}

static void ipoutput(unsigned char *packet, int length, Int8 protocol)
{
	/*fix ip header field*/
	struct iphdr *ipheader;
	ipheader = (struct iphdr *)packet;
	int i;
	/*header without options*/
	ipheader->verhdrlen=0x45;
	//ipheader->service=0x0
	ipheader->len=htons(length+sizeof(struct iphdr));
	length=length+sizeof(struct iphdr);
	ipheader->ident=htons(ip_seq);
	ip_seq++;
	//ipheader->frags=htons(0x0);
	ipheader->ttl =0x80;
	ipheader->protocol=protocol;
	/*ipchecksum*/
	ipheader->chksum = 0;
	ipheader->frags =htons(0x4000);
	assignip(&ipheader->src,&utcp_info.dst);
	assignip(&ipheader->dest,&utcp_info.src);

	ipheader->chksum = ipheader_chksum((Int16 *)packet, sizeof(struct iphdr));

	if(IP_TCP==protocol)
	{
		tcpchecksum(packet, length);
	}	
	/*send it out*/
	prepare_txpkt(0, FRAME_IP,utcp_info.node, packet, length);
}

static void tcpSaveSession(char* Packet)
{
//	prom_printf("%s %d\n",__FUNCTION__,__LINE__);
	/*Save sip dip sport dport*/	
	struct iphdr *ipheader;
	struct tcphdr *tcpheader;
	/*Save client's mac*/
	memcpy(utcp_info.node,nic.packet+ETH_ALEN,ETH_ALEN);
	ipheader = (struct iphdr *)&nic.packet[ETH_HLEN];
	tcpheader = (struct tcphdr *)&nic.packet[ETH_HLEN+ sizeof(struct iphdr)];
	assignip(&utcp_info.src,&ipheader->src);
	assignip(&utcp_info.dst,&ipheader->dest);
	utcp_info.tcpi_dport = ntohs(tcpheader->dest);
	utcp_info.tcpi_sport = ntohs(tcpheader->source);
	utcp_info.sessionsaved =1;
	assignseq(&utcp_info.tcpi_ack,&tcpheader->seq);
	incseq(&utcp_info.tcpi_ack,1);
	return;
}
static int isthisSession(char *packet)
{
	struct iphdr *ipheader;
	struct tcphdr *tcpheader;
	int tcplen;
	if(utcp_info.sessionsaved)
	{
		/*check pakcet belongs to the same session.*/
		ipheader = (struct iphdr *)&nic.packet[ETH_HLEN];
		tcpheader = (struct tcphdr *)&nic.packet[ETH_HLEN+ sizeof(struct iphdr)];
		if(utcp_info.tcpi_dport == ntohs(tcpheader->dest))
		{
			if(utcp_info.tcpi_sport == ntohs(tcpheader->source))
			{
				if(sameip(&(utcp_info.src),&(ipheader->src)))
				{
					if(sameip(&(utcp_info.dst),&(ipheader->dest)))
					{
						/*remember tcp seq*/
						tcplen=ntohs(ipheader->len)-((ipheader->verhdrlen &0xF)<<2)-(((tcpheader->tcpflags[0] & 0xF0) >> 4)<< 2);
						incseq(&utcp_info.tcpi_ack,tcplen);
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 1;
	}
}
static void tcpClearSession()
{
	memset(&utcp_info,0,sizeof(utcp_info));
	utcp_info.tcpi_state=TCP_CLOSE;

	/*Post Session Clear need*/
	content_length=0;
	content_offset=0;
	upload_len=0;
	httpd_mem_len=0;
	foundboundary=0;
}
/*reference user defined IP and ELAN Mac
  *if IP address all zero (not set), then use hard code ip and mac.
  *if IP address set, but mac all zero not setted, then use hw setting nic0 mac,
  *if hw setting mac all zero, then use user define ip and  hardcode mac.
  *if current setting error (checksum error),use hard code ip and mac.
  */
void httpd_entry(void)
{
	memset(&utcp_info,0,sizeof(utcp_info));
	utcp_info.tcpi_state=TCP_CLOSE;
	if(memcmp(eth0_ip_httpd,"\x0\x0\x0\x0",4))
	{
		arptable_tftp[HTTPD_ARPENTRY].ipaddr.s_addr =*(unsigned long *)eth0_ip_httpd;
		if(memcmp(eth0_mac_httpd,"\x0\x0\x0\x0\x0\x0",4))
		{
			arptable_tftp[HTTPD_ARPENTRY].node[5]=eth0_mac_httpd[5];
			arptable_tftp[HTTPD_ARPENTRY].node[4]=eth0_mac_httpd[4];
			arptable_tftp[HTTPD_ARPENTRY].node[3]=eth0_mac_httpd[3];
			arptable_tftp[HTTPD_ARPENTRY].node[2]=eth0_mac_httpd[2];
			arptable_tftp[HTTPD_ARPENTRY].node[1]=eth0_mac_httpd[1];
			arptable_tftp[HTTPD_ARPENTRY].node[0]=eth0_mac_httpd[0];
		}
		else
		{
			memcpy(arptable_tftp[HTTPD_ARPENTRY].node,arptable_tftp[TFTP_SERVER].node,6);
		}
	}
	else
	{
		memcpy(&arptable_tftp[HTTPD_ARPENTRY],&arptable_tftp[TFTP_SERVER],sizeof(struct arptable_t));
	}
}

void tcpinput()
{
	int i;
	struct iphdr *ipheader;
	 struct tcphdr *tcpheader;
//prom_printf("tcpinput start\n");
	 ipheader = (struct iphdr *)&nic.packet[ETH_HLEN];
   tcpheader = (struct tcphdr *)&nic.packet[ETH_HLEN+ sizeof(struct iphdr)];	
 	if(inet_chksum_pseudo((char*)tcpheader,(Int32 *)&(ipheader->src.ip[0]),(Int32 *)&(ipheader->dest.ip[0]),
		IP_TCP,(ntohs(ipheader->len)-((ipheader->verhdrlen &0xF)<<2))))
 	{
		return;
 	}
	//prom_printf("packet len %d tcpheader %x reqPort %d\n",nic.packetlen,tcpheader,htons(tcpheader->source));
        if(80==htons(tcpheader->dest))
        {
        	   /*we only support one session*/
		   if(!isthisSession(nic.packet))
		   {
		   	return;
		   }
                /*get tcp flags*/
		if((tcpheader->tcpflags[1]) & SYN_MASK)
             {
                        //reply with syn ack and window =2048
                        if(TCP_CLOSE==utcp_info.tcpi_state)
                        {
                                utcp_info.tcpi_state=TCP_SYN_RECV;
                                tcpSaveSession(nic.packet);
				     										tcpSendSynAck();
                        }else
                        {
                        	      tcpReset();
                        }
                }
		   else if(tcpheader->tcpflags[1] & FIN_MASK)
                {
									if(readyToUpgrade == 1)
									{                		 	
										writeImagetoflash((unsigned char *)httpd_mem,httpd_mem_len);
										autoreboot();
									}
                		 
                	   if( TCP_FIN_WAIT2 == utcp_info.tcpi_state)
                	   {
                	   		incseq(&utcp_info.tcpi_ack,1);
					incseq(&utcp_info.tcpi_seq,1);
					tcpAck();
					utcp_info.tcpi_state=TCP_CLOSE;
					tcpClearSession();
                	   }
			   else if(TCP_FIN_WAIT1 == utcp_info.tcpi_state)
			   {
			   	if(tcpheader->tcpflags[1] & ACK_MASK)
			   	{
			   		incseq(&utcp_info.tcpi_ack,1);
					incseq(&utcp_info.tcpi_seq,1);
					tcpAck();
					utcp_info.tcpi_state=TCP_CLOSE;
					tcpClearSession();
			   	}else
			   	{
			   		/*Two FIN Send together.CLOSING same time*/
					incseq(&utcp_info.tcpi_ack,1);
					incseq(&utcp_info.tcpi_seq,1);
					tcpAck();
					utcp_info.tcpi_state=TCP_CLOSING;
			   	}
			   }
			   else if(TCP_CLOSE_WAIT == utcp_info.tcpi_state)
			   {
			              /*never happen ?*/
			   }
               	   else if(TCP_SYN_RECV == utcp_info.tcpi_state)
                	   {
                	   		/*never happen ?*/
                	   }else if(TCP_ESTABLISHED == utcp_info.tcpi_state)
			   {
			   	utcp_info.tcpi_state = TCP_CLOSE_WAIT;
				//send FIN ACK bk
				//increase 1 for FIN ack
				incseq(&utcp_info.tcpi_ack,1);
				incseq(&utcp_info.tcpi_seq,1);
				tcpSendFin();
			   }
                	
                }
                else if((tcpheader->tcpflags[1]) & ACK_MASK)
                {
                      if(TCP_SYN_RECV == utcp_info.tcpi_state)
                      {
                                utcp_info.tcpi_state=TCP_ESTABLISHED;
			    		tcpConnected();
                      }
                      else if(TCP_ESTABLISHED == utcp_info.tcpi_state)
                      {
                      		  //Data Received
                        		  if(0 ==tcpInputData())
					  	tcpAck();
								  
                      }else if(TCP_CLOSE_WAIT == utcp_info.tcpi_state)
                      {
                      		/*inactive close*/
					utcp_info.tcpi_state = TCP_CLOSE;
					tcpClearSession();
				
                      } else if(TCP_CLOSE ==utcp_info.tcpi_state)
                      {
	                      	/*Ack to FIN ACK. clear Session*/
					tcpClearSession();
                      }
			  else if( TCP_FIN_WAIT1 == utcp_info.tcpi_state)
			  {
			  		utcp_info.tcpi_state = TCP_FIN_WAIT2;
			  }
			  else if(TCP_CLOSING == utcp_info.tcpi_state)
			  {
			  		/*Ack to pre send FIN. and peer's FIN received */
					utcp_info.tcpi_state = TCP_CLOSE;
					tcpClearSession();
			  }
			  else
                      {
                                utcp_info.tcpi_state=TCP_CLOSE;
				     tcpReset();
                      }
                }
		   else
                {
                		/*Just Reset the tcp connection*/
			      if(TCP_CLOSE != utcp_info.tcpi_state)
					tcpReset();
                }
	}
	return;
}

