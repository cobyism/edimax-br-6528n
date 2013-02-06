/*
 *	Generic address resultion entity
 *
 *	Authors:
 *	net_random Alan Cox
 *	net_ratelimit Andy Kleen
 *
 *	Created by Alexey Kuznetsov <kuznet@ms2.inr.ac.ru>
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>

static unsigned long net_rand_seed = 152L;

unsigned long net_random(void)
{
	net_rand_seed=net_rand_seed*69069L+1;
        return net_rand_seed^jiffies;
}

void net_srandom(unsigned long entropy)
{
	net_rand_seed ^= entropy;
	net_random();
}

int net_msg_cost = 5*HZ;
int net_msg_burst = 10*5*HZ;

/* 
 * This enforces a rate limit: not more than one kernel message
 * every 5secs to make a denial-of-service attack impossible.
 *
 * All warning printk()s should be guarded by this function. 
 */ 
int net_ratelimit(void)
{
	static spinlock_t ratelimit_lock = SPIN_LOCK_UNLOCKED;
	static unsigned long toks = 10*5*HZ;
	static unsigned long last_msg; 
	static int missed;
	unsigned long flags;
	unsigned long now = jiffies;

	spin_lock_irqsave(&ratelimit_lock, flags);
	toks += now - last_msg;
	last_msg = now;
	if (toks > net_msg_burst)
		toks = net_msg_burst;
	if (toks >= net_msg_cost) {
		int lost = missed;
		missed = 0;
		toks -= net_msg_cost;
		spin_unlock_irqrestore(&ratelimit_lock, flags);
		if (lost)
			printk(KERN_WARNING "NET: %d messages suppressed.\n", lost);
		return 1;
	}
	missed++;
	spin_unlock_irqrestore(&ratelimit_lock, flags);
	return 0;
}
#if 0
/*
 * Convert an ASCII string to binary IP.
 * This is outside of net/ipv4/ because various code that uses IP addresses
 * is otherwise not dependent on the TCP/IP stack.
 */
__u32 in_aton(const char *str)
{
        unsigned long l;
        unsigned int val;
        int i;

        l = 0;
        for (i = 0; i < 4; i++)
        {
                l <<= 8;
                if (*str != '\0')
                {
                        val = 0;
                        while (*str != '\0' && *str != '.' && *str != '\n')
                        {
                                val *= 10;
                                val += *str - '0';
                                str++;
                        }
                        l |= val;
                        if (*str != '\0')
                                str++;
                }
        }
        return(htonl(l));
}
#endif

#define IN6PTON_XDIGIT          0x00010000
#define IN6PTON_DIGIT           0x00020000
#define IN6PTON_COLON_MASK      0x00700000
#define IN6PTON_COLON_1         0x00100000      /* single : requested */
#define IN6PTON_COLON_2         0x00200000      /* second : requested */
#define IN6PTON_COLON_1_2       0x00400000      /* :: requested */
#define IN6PTON_DOT             0x00800000      /* . */
#define IN6PTON_DELIM           0x10000000
#define IN6PTON_NULL            0x20000000      /* first/tail */
#define IN6PTON_UNKNOWN         0x40000000


static inline int digit2bin(char c, int delim)
{
        if (c == delim || c == '\0')
                return IN6PTON_DELIM;
        if (c == '.')
                return IN6PTON_DOT;
        if (c >= '0' && c <= '9')
                return (IN6PTON_DIGIT | (c - '0'));
        return IN6PTON_UNKNOWN;
}

static inline int xdigit2bin(char c, int delim)
{
        if (c == delim || c == '\0')
                return IN6PTON_DELIM;
        if (c == ':')
                return IN6PTON_COLON_MASK;
        if (c == '.')
                return IN6PTON_DOT;
        if (c >= '0' && c <= '9')
                return (IN6PTON_XDIGIT | IN6PTON_DIGIT| (c - '0'));
        if (c >= 'a' && c <= 'f')
                return (IN6PTON_XDIGIT | (c - 'a' + 10));
        if (c >= 'A' && c <= 'F')
                return (IN6PTON_XDIGIT | (c - 'A' + 10));
        if (delim == -1)
                return IN6PTON_DELIM;
        return IN6PTON_UNKNOWN;
}

int in4_pton(const char *src, int srclen,
             u8 *dst,
             int delim, const char **end)
{
        const char *s;
        u8 *d;
        u8 dbuf[4];
        int ret = 0;
        int i;
        int w = 0;

        if (srclen < 0)
                srclen = strlen(src);
        s = src;
        d = dbuf;
        i = 0;
        while(1) {
                int c;
                c = xdigit2bin(srclen > 0 ? *s : '\0', delim);
                if (!(c & (IN6PTON_DIGIT | IN6PTON_DOT | IN6PTON_DELIM | IN6PTON_COLON_MASK))) {
                        goto out;
                }
                if (c & (IN6PTON_DOT | IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
                        if (w == 0)
                                goto out;
                        *d++ = w & 0xff;
                        w = 0;
                        i++;
                        if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
                                if (i != 4)
                                        goto out;
                                break;
                        }
                        goto cont;
                }
                w = (w * 10) + c;
                if ((w & 0xffff) > 255) {
                        goto out;
                }
cont:
                if (i >= 4)
                        goto out;
                s++;
                srclen--;
        }
        ret = 1;
        memcpy(dst, dbuf, sizeof(dbuf));
out:
        if (end)
                *end = s;
        return ret;
}


int in6_pton(const char *src, int srclen,
             u8 *dst,
             int delim, const char **end)
{
        const char *s, *tok = NULL;
        u8 *d, *dc = NULL;
        u8 dbuf[16];
        int ret = 0;
        int i;
        int state = IN6PTON_COLON_1_2 | IN6PTON_XDIGIT | IN6PTON_NULL;
        int w = 0;

        memset(dbuf, 0, sizeof(dbuf));

        s = src;
        d = dbuf;
        if (srclen < 0)
                srclen = strlen(src);

        while (1) {
                int c;

                c = xdigit2bin(srclen > 0 ? *s : '\0', delim);
                if (!(c & state))
                        goto out;
                if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
                        /* process one 16-bit word */
                        if (!(state & IN6PTON_NULL)) {
                                *d++ = (w >> 8) & 0xff;
                                *d++ = w & 0xff;
                        }
                        w = 0;
                        if (c & IN6PTON_DELIM) {
                                /* We've processed last word */
                                break;
                        }
                        /*
                         * COLON_1 => XDIGIT
                         * COLON_2 => XDIGIT|DELIM
                         * COLON_1_2 => COLON_2
                         */
                        switch (state & IN6PTON_COLON_MASK) {
                        case IN6PTON_COLON_2:
                                dc = d;
                                state = IN6PTON_XDIGIT | IN6PTON_DELIM;
                                if (dc - dbuf >= sizeof(dbuf))
                                        state |= IN6PTON_NULL;
                                break;
                        case IN6PTON_COLON_1|IN6PTON_COLON_1_2:
                                state = IN6PTON_XDIGIT | IN6PTON_COLON_2;
                                break;
                        case IN6PTON_COLON_1:
                                state = IN6PTON_XDIGIT;
                                break;
                        case IN6PTON_COLON_1_2:
                                state = IN6PTON_COLON_2;
                                break;
                        default:
                                state = 0;
                        }
                        tok = s + 1;
                        goto cont;
                }

                if (c & IN6PTON_DOT) {
                        ret = in4_pton(tok ? tok : s, srclen + (int)(s - tok), d, delim, &s);
                        if (ret > 0) {
                                d += 4;
                                break;
                        }
                        goto out;
                }

                w = (w << 4) | (0xff & c);
                state = IN6PTON_COLON_1 | IN6PTON_DELIM;
                if (!(w & 0xf000)) {
                        state |= IN6PTON_XDIGIT;
                }
                if (!dc && d + 2 < dbuf + sizeof(dbuf)) {
                        state |= IN6PTON_COLON_1_2;
                        state &= ~IN6PTON_DELIM;
                }
                if (d + 2 >= dbuf + sizeof(dbuf)) {
                        state &= ~(IN6PTON_COLON_1|IN6PTON_COLON_1_2);
                }
cont:
                if ((dc && d + 4 < dbuf + sizeof(dbuf)) ||
                    d + 4 == dbuf + sizeof(dbuf)) {
                        state |= IN6PTON_DOT;
                }
                if (d >= dbuf + sizeof(dbuf)) {
                        state &= ~(IN6PTON_XDIGIT|IN6PTON_COLON_MASK);
                }
                s++;
                srclen--;
        }

        i = 15; d--;

        if (dc) {
                while(d >= dc)
                        dst[i--] = *d--;
                while(i >= dc - dbuf)
                        dst[i--] = 0;
                while(i >= 0)
                        dst[i--] = *d--;
        } else
                memcpy(dst, dbuf, sizeof(dbuf));

        ret = 1;
out:
        if (end)
                *end = s;
        return ret;
}

