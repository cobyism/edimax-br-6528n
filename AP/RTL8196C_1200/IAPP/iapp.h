#ifndef IAPP_H
#define IAPP_H

#define IAPP_PORT_D3 2313
#define IAPP_PORT_D5 3517
#define IAPP_MULTICAST_ADDR	"224.0.1.178"

struct iapp_hdr {
	u8 version;
	u8 command;
	u16 identifier;
	u16 length;
	/* followed by length-6 octets of data */
} __attribute__ ((packed));

#define IAPP_VERSION 0

enum IAPP_COMMAND {
	IAPP_CMD_ADD_notify = 0,
	IAPP_CMD_MOVE_notify = 1,
	IAPP_CMD_MOVE_response = 2,
	IAPP_CMD_Send_Security_Block = 3,
	IAPP_CMD_ACK_Security_Block = 4
};

struct iapp_add_notify {
	u8 addr_len;
	u8 reserved;
	u8 mac_addr[6];
	u16 seq_num;
} __attribute__ ((packed));


/* Layer 2 Update frame (802.2 Type 1 LLC XID Update response) */
struct iapp_layer2_update {
	u8 da[6]; /* broadcast */
	u8 sa[6]; /* STA addr */
	u16 len; /* 8 */
	u8 dsap; /* 0 */
	u8 ssap; /* 0 */
	u8 control;
	u8 xid_info[3];
	u8 pad[10];		// Fix TKIP MIC error issue. david+2006-11-09
};


#define MAX_MSG_SIZE	1600
#define APPLY_D3		0x01
#define APPLY_D5		0x02

#define DRIVER_8180		0x01
#define DRIVER_8185		0x02

// context
struct iapp_context {
	int				driver_ver;
	u16				iapp_identifier; /* next IAPP identifier */
	struct in_addr	iapp_own, iapp_broadcast;
	int				iapp_udp_sock;
	int				iapp_mltcst_sock;
	int				iapp_packet_sock;

	char			iapp_iface[16];
	int				readfifo;
	u8				spcApply;
	u8				RecvBuf[MAX_MSG_SIZE];

	char			wlan_iface[10][16];
	int				wlan_sock[10];

	int				debug; /* debug verbosity level */
};

#define FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif /* IAPP_H */
