/*
 *	Linux NET3:	Internet Group Management Protocol  [IGMP]
 *
 *	Authors:
 *		Alan Cox <Alan.Cox@linux.org>
 *
 *	Extended to talk the BSD extended IGMP protocol of mrouted 3.6
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _LINUX_IGMP_H
#define _LINUX_IGMP_H

/*
 *	IGMP protocol structures
 */

/*
 *	Header in on cable format
 */

struct igmphdr
{
	__u8 type;
	__u8 code;		/* For newer IGMP */
	__u16 csum;
	__u32 group;
};

#define IGMPV3_MODE_IS_INCLUDE		1
#define IGMPV3_MODE_IS_EXCLUDE		2

#define IGMPV3_CHANGE_TO_INCLUDE	3	//leave
#define IGMPV3_CHANGE_TO_EXCLUDE	4	//join

#define IGMPV3_ALLOW_NEW_SOURCES	5
#define IGMPV3_BLOCK_OLD_SOURCES	6




/*group record*/
struct igmpv3_grec {
	__u8	grec_type;		//Record type
	__u8	grec_auxwords;	//aux data length
	__u16	grec_nsrcs;		//number of source
	__u32	grec_mca;		//multicast address
	__u32	grec_src[0];	
};


#if	0
struct igmpv3_report {
	__u8 type;
	__u8 resv1;
	__u16 csum;
	__u16 resv2;
	__u16 ngrec;
	struct igmpv3_grec grec[0];
};

struct igmpv3_query {
	__u8 type;
	__u8 code;
	__u16 csum;
	__u32 group;

	/*__BIG_ENDIAN_BITFIELD*/
	__u8 resv:4,
	     suppress:1,
	     qrv:3;
	/*__BIG_ENDIAN_BITFIELD*/
	
	__u8 qqic;
	__u16 nsrcs;
	__u32 srcs[0];
};

#endif

#define IGMP_HOST_MEMBERSHIP_QUERY	0x11	/* From RFC1112 */
#define IGMP_HOST_MEMBERSHIP_REPORT	0x12	/* Ditto */

#define IGMP_DVMRP			0x13	/* DVMRP routing */
#define IGMP_PIM			0x14	/* PIM routing */
#define IGMP_TRACE			0x15

#define IGMP_HOST_NEW_MEMBERSHIP_REPORT 0x16	/* New version of 0x11 */
#define IGMPV2_HOST_MEMBERSHIP_REPORT	0x16	/* V2 version of 0x11 */

#define IGMP_HOST_LEAVE_MESSAGE 	0x17
#define IGMPV3_HOST_MEMBERSHIP_REPORT	0x22	/* V3 version of 0x11 */


#define IGMP_MTRACE_RESP		0x1e
#define IGMP_MTRACE			0x1f


/*
 *	Use the BSD names for these for compatibility
 */

#define IGMP_DELAYING_MEMBER		0x01
#define IGMP_IDLE_MEMBER		0x02
#define IGMP_LAZY_MEMBER		0x03
#define IGMP_SLEEPING_MEMBER		0x04
#define IGMP_AWAKENING_MEMBER		0x05

#define IGMP_MINLEN			8

#define IGMP_MAX_HOST_REPORT_DELAY	10	/* max delay for response to */
						/* query (in seconds)	*/

#define IGMP_TIMER_SCALE		10	/* denotes that the igmphdr->timer field */
						/* specifies time in 10th of seconds	 */

#define IGMP_AGE_THRESHOLD		400	/* If this host don't hear any IGMP V1	*/
						/* message in this period of time,	*/
						/* revert to IGMP v2 router.		*/

#define IGMP_ALL_HOSTS		htonl(0xE0000001L)
#define IGMP_ALL_ROUTER 	htonl(0xE0000002L)
#define IGMPV3_ALL_MCR	 	htonl(0xE0000016L)
#define IGMP_LOCAL_GROUP	htonl(0xE0000000L)
#define IGMP_LOCAL_GROUP_MASK	htonl(0xFFFFFF00L)

/*
 * struct for keeping the multicast list in
 */

#ifdef __KERNEL__

/* ip_mc_socklist is real list now. Speed is not argument;
   this list never used in fast path code
 */

struct ip_mc_socklist
{
	struct ip_mc_socklist	*next;
	int			count;
	struct ip_mreqn		multi;
};

struct ip_mc_list
{
	struct in_device	*interface;
	unsigned long		multiaddr;
	struct ip_mc_list	*next;
	struct timer_list	timer;
	int			users;
	atomic_t		refcnt;
	spinlock_t		lock;
	char			tm_running;
	char			reporter;
	char			unsolicit_count;
	char			loaded;
};

extern int ip_check_mc(struct in_device *dev, u32 mc_addr);
extern int igmp_rcv(struct sk_buff *);
extern int ip_mc_join_group(struct sock *sk, struct ip_mreqn *imr);
extern int ip_mc_leave_group(struct sock *sk, struct ip_mreqn *imr);
extern void ip_mc_drop_socket(struct sock *sk);
extern void ip_mr_init(void);
extern void ip_mc_init_dev(struct in_device *);
extern void ip_mc_destroy_dev(struct in_device *);
extern void ip_mc_up(struct in_device *);
extern void ip_mc_down(struct in_device *);
extern int ip_mc_dec_group(struct in_device *in_dev, u32 addr);
extern void ip_mc_inc_group(struct in_device *in_dev, u32 addr);
#endif
#endif
