#ifndef _IP_CONNTRACK_PPTP_H
#define _IP_CONNTRACK_PPTP_H
/* PPTP tracking. */

#ifndef __KERNEL__
#error Only in kernel.
#endif

#include <linux/netfilter_ipv4/lockhelp.h>

/* Protects pptp part of conntracks */
DECLARE_LOCK_EXTERN(ip_pptp_lock);

/* We record call_id: all in host order. */
struct ip_ct_pptp
{
	/* This tells NAT that this is an pptp connection */
	int 		is_pptp;
	u_int32_t len;

	u_int16_t 		orig_call_id;		/* Client's call id */
	u_int16_t 		peer_call_id;		/* Server's call id */
	int 			pptp_magic;		/* 1723 (TCP port) */

	/* If this is a PPTP/GRE connection, point to its TCP master */
	struct ip_conntrack 	*master;		/* For GRE connections only */

	/* If this is a PPTP/TCP connection, list its child GRE connection(s).
	 * If this is a GRE connection, linked into the master's list
	 */
	struct list_head	list;
};

#define PPTP_TCP_PORT		1723
#define PPTP_GRE_VERSION	0x1
#define PPTP_GRE_PROTOCOL	0x880B

#define PPTP_GRE_FLAG_C		0x80
#define PPTP_GRE_FLAG_R		0x40
#define PPTP_GRE_FLAG_K		0x20
#define PPTP_GRE_FLAG_S		0x10
#define PPTP_GRE_FLAG_A		0x80

#define PPTP_GRE_IS_C(f)	((f)&PPTP_GRE_FLAG_C)
#define PPTP_GRE_IS_R(f)	((f)&PPTP_GRE_FLAG_R)
#define PPTP_GRE_IS_K(f)	((f)&PPTP_GRE_FLAG_K)
#define PPTP_GRE_IS_S(f)	((f)&PPTP_GRE_FLAG_S)
#define PPTP_GRE_IS_A(f)	((f)&PPTP_GRE_FLAG_A)

struct pptp_gre_hdr {
	__u8  flags;		/* bitfield */
	__u8  version;		/* should be PPTP_GRE_VER (enhanced GRE) */
	__u16 protocol;		/* should be PPTP_GRE_PROTO (ppp-encaps) */
	__u16 payload_len;	/* size of ppp payload, not inc. gre header */
	__u16 call_id;		/* peer's call_id for this session */
	__u32 seq;		/* sequence number.  Present if S==1 */
	__u32 ack;		/* seq number of highest packet recieved by */
				/*  sender in this session */
};

#define PPTP_CONTROL_PACKET	1
#define PPTP_MGMT_PACKET	2
#define PPTP_MAGIC_COOKIE	0x1a2b3c4d

struct pptp_pkt_hdr {
	__u16	packetLength;
	__u16	packetType;
	__u32	magicCookie;
};

/* PptpControlMessageType values */
#define PPTP_START_SESSION_REQUEST	1
#define PPTP_START_SESSION_REPLY	2
#define PPTP_STOP_SESSION_REQUEST	3
#define PPTP_STOP_SESSION_REPLY		4
#define PPTP_ECHO_REQUEST		5
#define PPTP_ECHO_REPLY			6
#define PPTP_OUT_CALL_REQUEST		7
#define PPTP_OUT_CALL_REPLY		8
#define PPTP_IN_CALL_REQUEST		9
#define PPTP_IN_CALL_REPLY		10
#define PPTP_IN_CALL_CONNECTED		11
#define PPTP_CALL_CLEAR_REQUEST		12
#define PPTP_CALL_DISCONNECT_NOTIFY	13
#define PPTP_WAN_ERROR_NOTIFY		14
#define PPTP_SET_LINK_INFO		15

#define PPTP_MSG_MAX			15

/* PptpGeneralError values */
#define PPTP_ERROR_CODE_NONE		0
#define PPTP_NOT_CONNECTED		1
#define PPTP_BAD_FORMAT			2
#define PPTP_BAD_VALUE			3
#define PPTP_NO_RESOURCE		4
#define PPTP_BAD_CALLID			5
#define PPTP_REMOVE_DEVICE_ERROR	6

struct PptpControlHeader {
	__u16	messageType;
	__u16	reserved;
};

/* FramingCapability Bitmap Values */
#define PPTP_FRAME_CAP_ASYNC		0x1
#define PPTP_FRAME_CAP_SYNC		0x2

/* BearerCapability Bitmap Values */
#define PPTP_BEARER_CAP_ANALOG		0x1
#define PPTP_BEARER_CAP_DIGITAL		0x2

struct PptpStartSessionRequest {
	__u16	protocolVersion;
	__u8	reserved1;
	__u8	reserved2;
	__u32	framingCapability;
	__u32	bearerCapability;
	__u16	maxChannels;
	__u16	firmwareRevision;
	__u8	hostName[64];
	__u8	vendorString[64];
};

/* PptpStartSessionResultCode Values */
#define PPTP_START_OK			1
#define PPTP_START_GENERAL_ERROR	2
#define PPTP_START_ALREADY_CONNECTED	3
#define PPTP_START_NOT_AUTHORIZED	4
#define PPTP_START_UNKNOWN_PROTOCOL	5

struct PptpStartSessionReply {
	__u16	protocolVersion;
	__u8	resultCode;
	__u8	generalErrorCode;
	__u32	framingCapability;
	__u32	bearerCapability;
	__u16	maxChannels;
	__u16	firmwareRevision;
	__u8	hostName[64];
	__u8	vendorString[64];
};

/* PptpStopReasons */
#define PPTP_STOP_NONE			1
#define PPTP_STOP_PROTOCOL		2
#define PPTP_STOP_LOCAL_SHUTDOWN	3

struct PptpStopSessionRequest {
	__u8	reason;
};

/* PptpStopSessionResultCode */
#define PPTP_STOP_OK			1
#define PPTP_STOP_GENERAL_ERROR		2

struct PptpStopSessionReply {
	__u8	resultCode;
	__u8	generalErrorCode;
};

struct PptpEchoRequest {
	__u32 identNumber;
};

/* PptpEchoReplyResultCode */
#define PPTP_ECHO_OK			1
#define PPTP_ECHO_GENERAL_ERROR		2

struct PptpEchoReply {
	__u32	identNumber;
	__u8	resultCode;
	__u8	generalErrorCode;
	__u16	reserved;
};

/* PptpFramingType */
#define PPTP_ASYNC_FRAMING		1
#define PPTP_SYNC_FRAMING		2
#define PPTP_DONT_CARE_FRAMING		3

/* PptpCallBearerType */
#define PPTP_ANALOG_TYPE		1
#define PPTP_DIGITAL_TYPE		2
#define PPTP_DONT_CARE_BEARER_TYPE	3

struct PptpOutCallRequest {
	__u16	callID;
	__u16	callSerialNumber;
	__u32	minBPS;
	__u32	maxBPS;
	__u32	bearerType;
	__u32	framingType;
	__u16	packetWindow;
	__u16	packetProcDelay;
	__u16	reserved1;
	__u16	phoneNumberLength;
	__u16	reserved2;
	__u8	phoneNumber[64];
	__u8	subAddress[64];
};
/* PptpCallResultCode */
#define PPTP_OUTCALL_CONNECT		1
#define PPTP_OUTCALL_GENERAL_ERROR	2
#define PPTP_OUTCALL_NO_CARRIER		3
#define PPTP_OUTCALL_BUSY		4
#define PPTP_OUTCALL_NO_DIAL_TONE	5
#define PPTP_OUTCALL_TIMEOUT		6
#define PPTP_OUTCALL_DONT_ACCEPT	7

struct PptpOutCallReply {
	__u16	callID;
	__u16	peersCallID;
	__u8	resultCode;
	__u8	generalErrorCode;
	__u16	causeCode;
	__u32	connectSpeed;
	__u16	packetWindow;
	__u16	packetProcDelay;
	__u32	physChannelID;
};

struct PptpInCallRequest {
	__u16	callID;
	__u16	callSerialNumber;
	__u32	callBearerType;
	__u32	physChannelID;
	__u16	dialedNumberLength;
	__u16	dialingNumberLength;
	__u8	dialedNumber[64];
	__u8	dialingNumber[64];
	__u8	subAddress[64];
};

/* PptpInCallResultCode */
#define PPTP_INCALL_ACCEPT		1
#define PPTP_INCALL_GENERAL_ERROR	2
#define PPTP_INCALL_DONT_ACCEPT		3

struct PptpInCallReply {
	__u16	callID;
	__u16	peersCallID;
	__u8	resultCode;
	__u8	generalErrorCode;
	__u16	packetWindow;
	__u16	packetProcDelay;
	__u16	reserved;
};

struct PptpInCallConnected {
	__u16	callID;
	__u16	peersCallID;
	__u32	connectSpeed;
	__u16	packetWindow;
	__u16	packetProcDelay;
	__u32	callFramingType;
};

struct PptpClearCallRequest {
	__u16	callID;
	__u16	reserved;
};

struct PptpCallDisconnectNotify {
	__u16	callID;
	__u8	resultCode;
	__u8	generalErrorCode;
	__u16	causeCode;
	__u16	reserved;
	__u8	callStatistics[128];
};

struct PptpWanErrorNotify {
	__u16	peersCallID;
	__u16	reserved;
	__u32	crcErrors;
	__u32	framingErrors;
	__u32	hardwareOverRuns;
	__u32	bufferOverRuns;
	__u32	timeoutErrors;
	__u32	alignmentErrors;
};

struct PptpSetLinkInfo {
	__u16	peersCallID;
	__u16	reserved;
	__u32	sendAccm;
	__u32	recvAccm;
};


struct pptp_priv_data {
	__u16	call_id;
	__u16	mcall_id;
	__u16	pcall_id;
};

//#define GRE_TIMEOUT		(60*HZ)		/* after initial packet */
//#define GRE_CONNECTED_TIMEOUT	(600*HZ)	/* after bidirectional traffic */

//#define GRE_TIMEOUT		(3*HZ)		/* after initial packet */
//#define GRE_CONNECTED_TIMEOUT	(3*HZ)	/* after bidirectional traffic */

//Brad modify 2008-02-18
#define GRE_TIMEOUT		(30*HZ)		/* after initial packet */
#define GRE_CONNECTED_TIMEOUT	(180*HZ)	/* after bidirectional traffic */


#endif /* _IP_CONNTRACK_PPTP_H */
