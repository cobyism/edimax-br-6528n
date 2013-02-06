#
# Makefile for the netfilter modules on top of IPv4.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := netfilter.o

ifdef CONFIG_RTL_KERNEL_MIPS16_NETFILTER
export-objs = ip_conntrack_standalone.o16 ip_fw_compat.o16 ip_nat_standalone.o16 ip_tables.o16 arp_tables.o16  ip_conntrack_core.o16 iptable_mangle.o16 ip_nat_core.o16 ip_conntrack_pptp.o16

# Multipart objects.
list-multi		:= ip_conntrack.o iptable_nat.o ipfwadm.o ipchains.o

# objects for the conntrack and NAT core (used by standalone and backw. compat)
ip_nf_conntrack-objs	:= ip_conntrack_core.o16 ip_conntrack_proto_generic.o16 ip_conntrack_proto_tcp.o16 ip_conntrack_proto_udp.o16 ip_conntrack_proto_icmp.o16
ip_nf_nat-objs		:= ip_nat_core.o16 ip_nat_helper.o16 ip_nat_proto_unknown.o16 ip_nat_proto_tcp.o16 ip_nat_proto_udp.o16 ip_nat_proto_icmp.o16

# objects for the standalone - connection tracking / NAT
ip_conntrack-objs	:= ip_conntrack_standalone.o16 $(ip_nf_conntrack-objs)
iptable_nat-objs	:= ip_nat_standalone.o16 ip_nat_rule.o16 $(ip_nf_nat-objs)

# objects for backwards compatibility mode
ip_nf_compat-objs	:= ip_fw_compat.o16 ip_fw_compat_redir.o16 ip_fw_compat_masq.o16 $(ip_nf_conntrack-objs) $(ip_nf_nat-objs)

ipfwadm-objs		:= $(ip_nf_compat-objs) ipfwadm_core.o16
ipchains-objs		:= $(ip_nf_compat-objs) ipchains_core.o16

# connection tracking
obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o

# L2TP
obj-$(CONFIG_IP_NF_NAT_L2TP) += ip_nat_l2tp.o16

# IPSEC 
obj-$(CONFIG_IP_NF_IPSEC) += ip_conntrack_ipsec.o16
ifdef CONFIG_IP_NF_IPSEC
	export-objs += ip_conntrack_ipsec.o16
endif
obj-$(CONFIG_IP_NF_NAT_IPSEC) += ip_nat_ipsec.o16

# H.323 support
obj-$(CONFIG_IP_NF_H323) += ip_conntrack_h323.o16
ifdef CONFIG_IP_NF_H323
	export-objs += ip_conntrack_h323.o16
endif
obj-$(CONFIG_IP_NF_NAT_H323) += ip_nat_h323.o16

# sip support
obj-$(CONFIG_IP_NF_SIP) += ip_conntrack_sip.o16
ifdef CONFIG_IP_NF_SIP
	export-objs += ip_conntrack_sip.o16
endif
obj-$(CONFIG_IP_NF_NAT_SIP) += ip_nat_sip.o16

# connection tracking helpers
obj-$(CONFIG_IP_NF_AMANDA) += ip_conntrack_amanda.o16
ifdef CONFIG_IP_NF_AMANDA
	export-objs += ip_conntrack_amanda.o16
endif

obj-$(CONFIG_IP_NF_TFTP) += ip_conntrack_tftp.o16
obj-$(CONFIG_IP_NF_FTP) += ip_conntrack_ftp.o16
ifdef CONFIG_IP_NF_FTP
	export-objs += ip_conntrack_ftp.o16
endif

obj-$(CONFIG_IP_NF_IRC) += ip_conntrack_irc.o16
ifdef CONFIG_IP_NF_IRC
	export-objs += ip_conntrack_irc.o16
endif
obj-$(CONFIG_IP_NF_PPTP) += ip_conntrack_pptp.o16

obj-$(CONFIG_IP_NF_PPTP) += ip_nat_pptp.o16
# NAT helpers 
obj-$(CONFIG_IP_NF_NAT_AMANDA) += ip_nat_amanda.o16
obj-$(CONFIG_IP_NF_NAT_TFTP) += ip_nat_tftp.o16
obj-$(CONFIG_IP_NF_NAT_FTP) += ip_nat_ftp.o16
obj-$(CONFIG_IP_NF_NAT_IRC) += ip_nat_irc.o16

# generic IP tables 
obj-$(CONFIG_IP_NF_IPTABLES) += ip_tables.o16

# the three instances of ip_tables
obj-$(CONFIG_IP_NF_FILTER) += iptable_filter.o16
obj-$(CONFIG_IP_NF_MANGLE) += iptable_mangle.o16
obj-$(CONFIG_IP_NF_NAT) += iptable_nat.o

# matches
obj-$(CONFIG_IP_NF_MATCH_HELPER) += ipt_helper.o16
obj-$(CONFIG_IP_NF_MATCH_LIMIT) += ipt_limit.o16
obj-$(CONFIG_IP_NF_MATCH_IPRANGE) += ipt_iprange.o16
obj-$(CONFIG_IP_NF_MATCH_MARK) += ipt_mark.o16
obj-$(CONFIG_IP_NF_MATCH_MAC) += ipt_mac.o16

obj-$(CONFIG_IP_NF_MATCH_PKTTYPE) += ipt_pkttype.o16
obj-$(CONFIG_IP_NF_MATCH_MULTIPORT) += ipt_multiport.o16
obj-$(CONFIG_IP_NF_MATCH_MPORT) += ipt_mport.o16
obj-$(CONFIG_IP_NF_MATCH_OWNER) += ipt_owner.o16
obj-$(CONFIG_IP_NF_MATCH_TOS) += ipt_tos.o16
obj-$(CONFIG_IP_NF_MATCH_TIME) += ipt_time.o16
obj-$(CONFIG_IP_NF_MATCH_RECENT) += ipt_recent.o16

obj-$(CONFIG_IP_NF_MATCH_ECN) += ipt_ecn.o16
obj-$(CONFIG_IP_NF_MATCH_DSCP) += ipt_dscp.o16
obj-$(CONFIG_IP_NF_MATCH_AH_ESP) += ipt_ah.o16 ipt_esp.o16

obj-$(CONFIG_IP_NF_MATCH_LENGTH) += ipt_length.o16

obj-$(CONFIG_IP_NF_MATCH_TTL) += ipt_ttl.o16
obj-$(CONFIG_IP_NF_MATCH_STATE) += ipt_state.o16
obj-$(CONFIG_IP_NF_MATCH_CONNTRACK) += ipt_conntrack.o16
obj-$(CONFIG_IP_NF_MATCH_UNCLEAN) += ipt_unclean.o16
obj-$(CONFIG_IP_NF_MATCH_TCPMSS) += ipt_tcpmss.o16

# targets
obj-$(CONFIG_IP_NF_TARGET_REJECT) += ipt_REJECT.o16
obj-$(CONFIG_IP_NF_TARGET_MIRROR) += ipt_MIRROR.o16
obj-$(CONFIG_IP_NF_TARGET_TOS) += ipt_TOS.o16
obj-$(CONFIG_IP_NF_TARGET_ECN) += ipt_ECN.o16
obj-$(CONFIG_IP_NF_TARGET_DSCP) += ipt_DSCP.o16
obj-$(CONFIG_IP_NF_TARGET_MARK) += ipt_MARK.o16
obj-$(CONFIG_IP_NF_TARGET_MASQUERADE) += ipt_MASQUERADE.o16
obj-$(CONFIG_IP_NF_TARGET_REDIRECT) += ipt_REDIRECT.o16
obj-$(CONFIG_IP_NF_NAT_SNMP_BASIC) += ip_nat_snmp_basic.o16
obj-$(CONFIG_IP_NF_TARGET_LOG) += ipt_LOG.o16
obj-$(CONFIG_IP_NF_TARGET_ULOG) += ipt_ULOG.o16
obj-$(CONFIG_IP_NF_TARGET_TCPMSS) += ipt_TCPMSS.o
obj-$(CONFIG_IP_NF_TARGET_TRIGGER) += ipt_TRIGGER.o16

# generic ARP tables
obj-$(CONFIG_IP_NF_ARPTABLES) += arp_tables.o16
obj-$(CONFIG_IP_NF_ARP_MANGLE) += arpt_mangle.o16

# just filtering instance of ARP tables for now
obj-$(CONFIG_IP_NF_ARPFILTER) += arptable_filter.o16

# backwards compatibility 
obj-$(CONFIG_IP_NF_COMPAT_IPCHAINS) += ipchains.o16
obj-$(CONFIG_IP_NF_COMPAT_IPFWADM) += ipfwadm.o16

obj-$(CONFIG_IP_NF_QUEUE) += ip_queue.o16

else
export-objs = ip_conntrack_standalone.o ip_fw_compat.o ip_nat_standalone.o ip_tables.o arp_tables.o  ip_conntrack_core.o iptable_mangle.o ip_nat_core.o ip_conntrack_pptp.o

# Multipart objects.
list-multi		:= ip_conntrack.o iptable_nat.o ipfwadm.o ipchains.o

# objects for the conntrack and NAT core (used by standalone and backw. compat)
ip_nf_conntrack-objs	:= ip_conntrack_core.o ip_conntrack_proto_generic.o ip_conntrack_proto_tcp.o ip_conntrack_proto_udp.o ip_conntrack_proto_icmp.o
ip_nf_nat-objs		:= ip_nat_core.o ip_nat_helper.o ip_nat_proto_unknown.o ip_nat_proto_tcp.o ip_nat_proto_udp.o ip_nat_proto_icmp.o

# objects for the standalone - connection tracking / NAT
ip_conntrack-objs	:= ip_conntrack_standalone.o $(ip_nf_conntrack-objs)
iptable_nat-objs	:= ip_nat_standalone.o ip_nat_rule.o $(ip_nf_nat-objs)

# objects for backwards compatibility mode
ip_nf_compat-objs	:= ip_fw_compat.o ip_fw_compat_redir.o ip_fw_compat_masq.o $(ip_nf_conntrack-objs) $(ip_nf_nat-objs)

ipfwadm-objs		:= $(ip_nf_compat-objs) ipfwadm_core.o
ipchains-objs		:= $(ip_nf_compat-objs) ipchains_core.o

# connection tracking
obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o

# L2TP
obj-$(CONFIG_IP_NF_NAT_L2TP) += ip_nat_l2tp.o

# IPSEC 
obj-$(CONFIG_IP_NF_IPSEC) += ip_conntrack_ipsec.o
ifdef CONFIG_IP_NF_IPSEC
	export-objs += ip_conntrack_ipsec.o
endif
obj-$(CONFIG_IP_NF_NAT_IPSEC) += ip_nat_ipsec.o

# H.323 support
obj-$(CONFIG_IP_NF_H323) += ip_conntrack_h323.o
ifdef CONFIG_IP_NF_H323
	export-objs += ip_conntrack_h323.o
endif
obj-$(CONFIG_IP_NF_NAT_H323) += ip_nat_h323.o

# sip support
obj-$(CONFIG_IP_NF_SIP) += ip_conntrack_sip.o
ifdef CONFIG_IP_NF_SIP
	export-objs += ip_conntrack_sip.o
endif
obj-$(CONFIG_IP_NF_NAT_SIP) += ip_nat_sip.o

# connection tracking helpers
obj-$(CONFIG_IP_NF_AMANDA) += ip_conntrack_amanda.o
ifdef CONFIG_IP_NF_AMANDA
	export-objs += ip_conntrack_amanda.o
endif

obj-$(CONFIG_IP_NF_TFTP) += ip_conntrack_tftp.o
obj-$(CONFIG_IP_NF_FTP) += ip_conntrack_ftp.o
ifdef CONFIG_IP_NF_FTP
	export-objs += ip_conntrack_ftp.o
endif

obj-$(CONFIG_IP_NF_IRC) += ip_conntrack_irc.o
ifdef CONFIG_IP_NF_IRC
	export-objs += ip_conntrack_irc.o
endif

obj-$(CONFIG_IP_NF_PPTP) += ip_conntrack_pptp.o
ifdef CONFIG_IP_NF_PPTP
export-objs +=ip_conntrack_pptp.o
endif

obj-$(CONFIG_IP_NF_PPTP) += ip_nat_pptp.o
ifdef CONFIG_IP_NF_PPTP
export-objs +=ip_nat_pptp.o
endif

# NAT helpers 
obj-$(CONFIG_IP_NF_NAT_AMANDA) += ip_nat_amanda.o
obj-$(CONFIG_IP_NF_NAT_TFTP) += ip_nat_tftp.o
obj-$(CONFIG_IP_NF_NAT_FTP) += ip_nat_ftp.o
obj-$(CONFIG_IP_NF_NAT_IRC) += ip_nat_irc.o

# generic IP tables 
obj-$(CONFIG_IP_NF_IPTABLES) += ip_tables.o

# the three instances of ip_tables
obj-$(CONFIG_IP_NF_FILTER) += iptable_filter.o
obj-$(CONFIG_IP_NF_MANGLE) += iptable_mangle.o
obj-$(CONFIG_IP_NF_NAT) += iptable_nat.o

# matches
obj-$(CONFIG_IP_NF_MATCH_HELPER) += ipt_helper.o
obj-$(CONFIG_IP_NF_MATCH_LIMIT) += ipt_limit.o
obj-$(CONFIG_IP_NF_MATCH_IPRANGE) += ipt_iprange.o
obj-$(CONFIG_IP_NF_MATCH_MARK) += ipt_mark.o
obj-$(CONFIG_IP_NF_MATCH_MAC) += ipt_mac.o

obj-$(CONFIG_IP_NF_MATCH_PKTTYPE) += ipt_pkttype.o
obj-$(CONFIG_IP_NF_MATCH_MULTIPORT) += ipt_multiport.o
obj-$(CONFIG_IP_NF_MATCH_MPORT) += ipt_mport.o
obj-$(CONFIG_IP_NF_MATCH_OWNER) += ipt_owner.o
obj-$(CONFIG_IP_NF_MATCH_TOS) += ipt_tos.o
obj-$(CONFIG_IP_NF_MATCH_TIME) += ipt_time.o
obj-$(CONFIG_IP_NF_MATCH_RECENT) += ipt_recent.o

obj-$(CONFIG_IP_NF_MATCH_ECN) += ipt_ecn.o
obj-$(CONFIG_IP_NF_MATCH_DSCP) += ipt_dscp.o
obj-$(CONFIG_IP_NF_MATCH_AH_ESP) += ipt_ah.o ipt_esp.o

obj-$(CONFIG_IP_NF_MATCH_LENGTH) += ipt_length.o

obj-$(CONFIG_IP_NF_MATCH_TTL) += ipt_ttl.o
obj-$(CONFIG_IP_NF_MATCH_STATE) += ipt_state.o
obj-$(CONFIG_IP_NF_MATCH_CONNTRACK) += ipt_conntrack.o
obj-$(CONFIG_IP_NF_MATCH_UNCLEAN) += ipt_unclean.o
obj-$(CONFIG_IP_NF_MATCH_TCPMSS) += ipt_tcpmss.o
obj-$(CONFIG_IP_NF_MATCH_WEBSTR) += ipt_webstr.o

# targets
obj-$(CONFIG_IP_NF_TARGET_REJECT) += ipt_REJECT.o
obj-$(CONFIG_IP_NF_TARGET_MIRROR) += ipt_MIRROR.o
obj-$(CONFIG_IP_NF_TARGET_TOS) += ipt_TOS.o
obj-$(CONFIG_IP_NF_TARGET_ECN) += ipt_ECN.o
obj-$(CONFIG_IP_NF_TARGET_DSCP) += ipt_DSCP.o
obj-$(CONFIG_IP_NF_TARGET_MARK) += ipt_MARK.o
obj-$(CONFIG_IP_NF_TARGET_MASQUERADE) += ipt_MASQUERADE.o
obj-$(CONFIG_IP_NF_TARGET_REDIRECT) += ipt_REDIRECT.o
obj-$(CONFIG_IP_NF_NAT_SNMP_BASIC) += ip_nat_snmp_basic.o
obj-$(CONFIG_IP_NF_TARGET_LOG) += ipt_LOG.o
obj-$(CONFIG_IP_NF_TARGET_TTL) += ipt_TTL.o
obj-$(CONFIG_IP_NF_TARGET_ULOG) += ipt_ULOG.o
obj-$(CONFIG_IP_NF_TARGET_TCPMSS) += ipt_TCPMSS.o
obj-$(CONFIG_IP_NF_TARGET_TRIGGER) += ipt_TRIGGER.o

# generic ARP tables
obj-$(CONFIG_IP_NF_ARPTABLES) += arp_tables.o
obj-$(CONFIG_IP_NF_ARP_MANGLE) += arpt_mangle.o

# just filtering instance of ARP tables for now
obj-$(CONFIG_IP_NF_ARPFILTER) += arptable_filter.o

# backwards compatibility 
obj-$(CONFIG_IP_NF_COMPAT_IPCHAINS) += ipchains.o
obj-$(CONFIG_IP_NF_COMPAT_IPFWADM) += ipfwadm.o

obj-$(CONFIG_IP_NF_QUEUE) += ip_queue.o

endif

include $(TOPDIR)/Rules.make

ip_conntrack.o: $(ip_conntrack-objs)
	$(LD) -r -o $@ $(ip_conntrack-objs)

iptable_nat.o: $(iptable_nat-objs)
	$(LD) -r -o $@ $(iptable_nat-objs)

ipfwadm.o: $(ipfwadm-objs)
	$(LD) -r -o $@ $(ipfwadm-objs)

ipchains.o: $(ipchains-objs)
	$(LD) -r -o $@ $(ipchains-objs)
