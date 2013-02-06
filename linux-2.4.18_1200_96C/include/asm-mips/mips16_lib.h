/*
 *      Headler file of MIPS16 library
 *
 *      $Id: mips16_lib.h,v 1.1 2009/03/17 02:31:59 davidhsu Exp $
 */

#ifndef _MIPS16_LIB_H
#define _MIPS16_LIB_H

#define rtattr_strcmp(rta, str)	_rtattr_strcmp(rta, str)
#define atomic_sub_return(i, v)	_atomic_sub_return(i, v)
#define test_and_clear_bit(nr, addr)	_test_and_clear_bit(nr, addr)
#define get_fd_set(nr, ufdset, fdset)	_get_fd_set(nr, ufdset, fdset)
#define __test_and_change_bit(nr, addr)	__test_and_change_bit__(nr, addr)
#define test_and_set_bit(nr, addr)	_test_and_set_bit(nr, addr)
#define set_bit(nr, addr)	_set_bit(nr, addr)
#define test_bit(nr, addr)	_test_bit(nr, addr)
#define clear_bit(nr, addr)	_clear_bit(nr, addr)
#define activate_mm(prev, next)	_activate_mm(prev, next)
#define switch_mm(prev, next, tsk, cpu)	_switch_mm(prev, next, tsk, cpu)
#define get_cycles()	_get_cycles()
#define __list_cmp_name(i, name)	__list_cmp_name__(i, name)
#define tcp_v4_check(th, len, saddr, daddr, base)	_tcp_v4_check(th, len, saddr, daddr, base)
#define tcp_checksum_complete(skb)	_tcp_checksum_complete(skb)
#define __tcp_checksum_complete(skb) __tcp_checksum_complete__(skb)
#define ip_compute_csum(buff, len)	_ip_compute_csum(buff, len)
#define ip_fast_csum(iph, ihl) _ip_fast_csum(iph, ihl)
#define csum_tcpudp_nofold(saddr, daddr, len, proto, sum)	_csum_tcpudp_nofold(saddr, daddr, len, proto, sum)
#define csum_tcpudp_magic(saddr, daddr, en, proto, sum)	_csum_tcpudp_magic(saddr, daddr, en, proto, sum)
#define csum_fold(sum)	_csum_fold(sum)
#define atomic_add(i, v)	_atomic_add(i, v)
#define atomic_sub(i, v)	_atomic_sub(i, v)
#define atomic_dev(i, v)	_atomic_sub(i, v)

#define strcpy(dest, src)	_strcpy(dest, src)
#define strncpy(dest, src, count)	_strncpy(dest, src, count)
#define strcmp(cs, ct)	_strcmp(cs, ct)
#define strncmp(cs, ct, len)	_strncmp(cs, ct, len)

extern int _rtattr_strcmp(struct rtattr *rta, char *str);
extern int _atomic_sub_return(int i, atomic_t * v);
extern int _test_and_clear_bit(int nr, volatile void *addr);
extern int _get_fd_set(unsigned long nr, void *ufdset, unsigned long *fdset);
extern int __test_and_change_bit__(int nr, volatile void * addr);
extern int _test_and_set_bit(int nr, volatile void *addr);
extern void _set_bit(int nr, volatile void *addr);
extern int _test_bit(int nr, volatile void *addr);
extern void _clear_bit(int nr, volatile void *addr);
extern void _activate_mm(struct mm_struct *prev, struct mm_struct *next);
extern void _switch_mm(struct mm_struct *prev, struct mm_struct *next,
                             struct task_struct *tsk, unsigned cpu);
extern cycles_t _get_cycles(void);
extern int __list_cmp_name__(const void *i, const char *name);
extern unsigned short _ip_compute_csum(unsigned char * buff, int len);
extern int _tcp_checksum_complete(struct sk_buff *skb);
extern u16 _tcp_v4_check(struct tcphdr *th, int len,unsigned long saddr, unsigned long daddr, unsigned long base);
extern int __tcp_checksum_complete__(struct sk_buff *skb);
extern unsigned short _ip_fast_csum(unsigned char *iph, unsigned int ihl);
extern unsigned long _csum_tcpudp_nofold(unsigned long saddr, unsigned long daddr, unsigned short len, unsigned short proto, unsigned int sum);
extern unsigned short int _csum_tcpudp_magic(unsigned long saddr, unsigned long daddr, unsigned short len, unsigned short proto, unsigned int sum);
extern unsigned short int _csum_fold(unsigned int sum);
extern void _atomic_sub(int i, atomic_t *v);
extern void _atomic_add(int i, atomic_t *v);

extern char *_strcpy(char *dest, __const__ char *src);
extern char * _strncpy(char * dest,const char *src,size_t count);
extern int _strcmp(__const__ char *__cs, __const__ char *__ct);
extern int _strncmp(__const__ char *__cs, __const__ char *__ct, size_t __count);

#endif
