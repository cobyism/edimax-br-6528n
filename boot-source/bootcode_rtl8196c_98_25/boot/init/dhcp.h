/*Realsil 20081105 hf*/
#ifndef __DHCP_H__
#define __DHCP_H__

/*moved for in.h*/
#define	INADDR_BROADCAST	((unsigned long int) 0xffffffff)

/* DHCP protocol -- see RFC 2131 */
#define SERVER_PORT		67
#define CLIENT_PORT		68

#define DHCP_MAGIC		0x63825363

/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d

#define DHCP_END		0xFF

#define BOOTREQUEST		1
#define BOOTREPLY		2

#define ETH_10MB		1
#define ETH_10MB_LEN		6

#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8

#define BROADCAST_FLAG		0x8000

#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

/* miscellaneous defines */
#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2

#define MAX_LEASE 2

#define TYPE_MASK	0x0F

#define LEASE_TIME              (60*60*24*10) /* 10 days of seconds */

#ifdef SYSLOG
#define LOG(level, str, args...) do { prom_printf(str, ## args); \
				prom_printf("\n"); } while(0)

#define LOG_EMERG	"EMERGENCY!"
#define LOG_ALERT	"ALERT!"
#define LOG_CRIT	"critical!"
#define LOG_WARNING	"warning"
#define LOG_ERR	"error"
#define LOG_INFO	"info"
#define LOG_DEBUG	"debug"
#else
#define LOG(level, str, args...) do {;}while(0) 
#endif

#ifdef _DEBUG
#undef DEBUG
#define DEBUG(level, str, args...) LOG(level, str, ## args)
#else
#define DEBUG(level, str, args...) do {;} while(0)
#endif


enum {
	OPTION_IP=1,
	OPTION_IP_PAIR,
	OPTION_STRING,
	OPTION_BOOLEAN,
	OPTION_U8,
	OPTION_U16,
	OPTION_S16,
	OPTION_U32,
	OPTION_S32
};

#define OPTION_REQ	0x10 /* have the client request this option */
#define OPTION_LIST	0x20 /* There can be a list of 1 or more of these */

struct option_set {
	unsigned char *data;
	struct option_set *next;
};

struct dhcp_option {
	char name[10];
	char flags;
	unsigned char code;
};

struct dhcpMessage {
        Int8 op;
        Int8 htype;
        Int8 hlen;
        Int8 hops;
        Int32 xid;
        Int16 secs;
        Int16 flags;
        Int32 ciaddr;
        Int32 yiaddr;
        Int32 siaddr;
        Int32 giaddr;
        Int8 chaddr[16];
        Int8 sname[64];
        Int8 file[128];
        Int32 cookie;
        Int8 options[308]; /* 312 - cookie */
};
 
struct udp_dhcp_packet {
        struct iphdr ip;
        struct udphdr udp;
        struct dhcpMessage data;
};

struct dhcpOfferedAddr {
	Int8 chaddr[16];
	Int32 yiaddr;	/* network order */
	Int32 expires;	/* host order */
};

struct server_config_t {
	Int32 server;		/* Our IP, in network order */
	Int32 start;		/* Start address of leases, network order */
	Int32 end;			/* End of leases, network order */

	unsigned char arp[6];		/* Our arp address */
	unsigned long lease;		/* lease time in seconds (host order) */
	unsigned long max_leases; 	/* maximum number of leases (including reserved address) */
	unsigned long decline_time; 	/* how long an address is reserved if a client returns a
				    	 * decline message */
	unsigned long offer_time; 	/* how long an offered address is reserved */
	unsigned long min_lease; 	/* minimum lease a client can request*/
};
#endif /*__DHCP_H__*/

