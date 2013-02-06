/*
 *      MIPS16 library
 *
 *      $Id: mips16_lib.c,v 1.1 2009/03/17 02:20:18 davidhsu Exp $
 */

#include <linux/config.h>
#include <linux/string.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <asm/system.h>
#include <asm/atomic.h>
#include <asm/timex.h>
#include <asm/mmu_context.h>
#include <asm/bitops.h>
#include <asm/semaphore.h>
#include <asm/mipsregs.h>
#include <linux/netfilter_ipv4/listhelp.h>

int _rtattr_strcmp(struct rtattr *rta, char *str)
{
	return rtattr_strcmp(rta, str);
}

int _atomic_sub_return(int i, atomic_t * v)
{
	return atomic_sub_return(i, v);
}

int _test_and_clear_bit(int nr, volatile void *addr)
{
	return test_and_clear_bit(nr, addr);
}

int _get_fd_set(unsigned long nr, void *ufdset, unsigned long *fdset)
{
	return get_fd_set(nr, ufdset, fdset);
}

int __test_and_change_bit__(int nr, volatile void * addr)
{
	return __test_and_change_bit(nr, addr);
}

int _test_and_set_bit(int nr, volatile void *addr)
{
	return test_and_set_bit(nr, addr);
}

void _set_bit(int nr, volatile void *addr)
{
	return set_bit(nr, addr);
}

int _test_bit(int nr, volatile void *addr)
{
	return test_bit(nr, addr);
}

void _clear_bit(int nr, volatile void *addr)
{
	clear_bit(nr, addr);
}

void _activate_mm(struct mm_struct *prev, struct mm_struct *next)
{
	activate_mm(prev, next);
}

void _switch_mm(struct mm_struct *prev, struct mm_struct *next,
                             struct task_struct *tsk, unsigned cpu)
{
	switch_mm(prev, next, tsk, cpu);
}                             

cycles_t _get_cycles(void)
{
	return get_cycles();
}

int __list_cmp_name__(const void *i, const char *name)
{
	return __list_cmp_name(i, name);
}

int _tcp_checksum_complete(struct sk_buff *skb)
{
	return tcp_checksum_complete(skb);
}

int __tcp_checksum_complete__(struct sk_buff *skb)
{
	return __tcp_checksum_complete(skb);
}

void _atomic_sub(int i, atomic_t * v)
{
	atomic_sub(i, v);
}

void _atomic_add(int i, atomic_t * v)
{
	atomic_add(i, v);
}

int _strcmp(__const__ char *cs, __const__ char *ct)
{
	return strcmp(cs, ct);
}

int _strncmp(__const__ char *__cs, __const__ char *__ct, size_t __count)
{
	return strncmp(__cs, __ct, __count);
}

char *_strcpy(char *dest, __const__ char *src)
{
	return strcpy(dest, src);
}

char *_strncpy(char * dest,const char *src,size_t count)
{
	return strncpy(dest, src, count);
}

unsigned short _ip_compute_csum(unsigned char * buff, int len)
{
	return ip_compute_csum(buff, len);
}

unsigned short _ip_fast_csum(unsigned char *iph, unsigned int ihl)
{
	return ip_fast_csum(iph,ihl);
}

unsigned long _csum_tcpudp_nofold(unsigned long saddr, unsigned long daddr, unsigned short len, unsigned short proto, unsigned int sum)
{
	return csum_tcpudp_nofold(saddr, daddr, len, proto, sum);
}

unsigned short int _csum_tcpudp_magic(unsigned long saddr, unsigned long daddr, unsigned short len, unsigned short proto, unsigned int sum)
{
	return csum_tcpudp_magic(saddr, daddr, len, proto, sum);
}	

unsigned short int _csum_fold(unsigned int sum)
{
	return csum_fold(sum);
}

u16 _tcp_v4_check(struct tcphdr *th, int len,unsigned long saddr, unsigned long daddr, unsigned long base)
{
	return tcp_v4_check(th, len, saddr, daddr, base);
}

void __sti__(void)
{
	__sti();	
}

void __cli__(void)
{
	__cli();	
}

void __save_flags__(unsigned long *x)
{
	unsigned long flags;
	
	__save_flags(flags);
	*x = flags;	
}

void __save_and_cli__(unsigned long *x)
{
	unsigned long flags;
	
	__save_and_cli(flags);
	*x = flags;			
}

void __restore_flags__(unsigned long x)
{
	__restore_flags(x);	
}