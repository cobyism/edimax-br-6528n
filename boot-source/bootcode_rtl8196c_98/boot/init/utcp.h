/*Realsil 20081031 hf_shi*/
#ifndef __UTCP_H__
#define __UTCP_H__

#define IMAGE_MEM_ADDR 0x80500000
#define BYTES_ALIGN(x) ((x+3) & (~3))
/*only support one http client*/
/*for simply !!!we just assume there are no packet loss,no congestion,no dis-order*/
/*Only One Session Exist. if two web browser connected together. first one success*/
/*************************************************************/
typedef struct utcpinfo
{
	Int8 	sessionsaved;
	Int8		tcpi_state;
	Int8 	node[6];
	Int16	tcpi_sport;
	Int16     tcpi_dport;
	int32_num	tcpi_ack;
	int32_num	tcpi_acked;
	int32_num	tcpi_seq;
	in_addr dst;
	in_addr src;
	Int8		uploadstarted;
} UTCPINFO_T, UTPCINFO_Tp;

static void tcpSendSynAck();
static void tcpConnected();
static int tcpInputData();
static void tcpReset();
void tcpinput();
static void tcpAck();
static void tcpSendFinAck();
static void tcpwrite(unsigned char *packet, int length);
static void ipoutput(unsigned char *packet, int length, Int8 protocol);
static void tcpoutput(struct tcphdr*tcpheader,unsigned char *packet, int length);

#endif /*__UTCP_H__*/
