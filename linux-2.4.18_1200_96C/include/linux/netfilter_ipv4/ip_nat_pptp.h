#ifndef _IP_NAT_PPTP_H
#define _IP_NAT_PPTP_H
/* PPTP extension for TCP NAT alteration. */

#ifndef __KERNEL__
#error Only in kernel.
#endif

/* Protects pptp part of conntracks */
DECLARE_LOCK_EXTERN(ip_pptp_lock);

struct ip_nat_pptp_info
{
	u_int16_t 	call_id;	/* original, before masq */
	u_int16_t	mcall_id;	/* masq call id */
	int		serv_to_client;	/* server sent GRE first */
};

#endif /* _IP_NAT_PPTP_H */
