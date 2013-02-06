/*
 * Copyright 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * Common time service routines for MIPS machines. See 
 * Documents/MIPS/README.txt. 
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/time.h>
#include <linux/smp.h>
#include <linux/kernel_stat.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>
#include <asm/hardirq.h>
#include <asm/div64.h>
#ifdef CONFIG_RTL_EB8186
#include <asm/rtl8181.h>
#endif
#include <asm/io.h>

/* This is for machines which generate the exact clock. */
#define USECS_PER_JIFFY (1000000/HZ)
#define USECS_PER_JIFFY_FRAC ((u32)((1000000ULL << 32) / HZ))

#if defined(CONFIG_RTL8181_WTDOG) || defined(CONFIG_RTL865X_WTDOG)
int is_fault=0; // kernel fault flag
#endif

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
void (*mips_timer_ack)(void);
#endif

#define RTK_RX_TIME_SQUEEZE_CHECK 1//mark_add 
#ifdef RTK_RX_TIME_SQUEEZE_CHECK
#include <linux/netdevice.h>
extern struct softnet_data softnet_data[];
extern unsigned int rtk_rx_time_squeeze; //from dev.c = rx packet process beyond 1 tick(it 's only for single cpu)!!!
#endif
/*
 * forward reference
 */
extern rwlock_t xtime_lock;
extern volatile unsigned long wall_jiffies;

/*
 * By default we provide the null RTC ops
 */
static unsigned long null_rtc_get_time(void)
{
	return mktime(2000, 1, 1, 0, 0, 0);
}

static int null_rtc_set_time(unsigned long sec)
{
	return 0;
}

unsigned long (*rtc_get_time)(void) = null_rtc_get_time;
int (*rtc_set_time)(unsigned long) = null_rtc_set_time;


/*
 * timeofday services, for syscalls.
 */
void do_gettimeofday(struct timeval *tv)
{
	unsigned long flags;

	read_lock_irqsave (&xtime_lock, flags);
	*tv = xtime;
	tv->tv_usec += do_gettimeoffset();

	/*
	 * xtime is atomically updated in timer_bh. jiffies - wall_jiffies
	 * is nonzero if the timer bottom half hasnt executed yet.
	 */
	if (jiffies - wall_jiffies)
		tv->tv_usec += USECS_PER_JIFFY;

	read_unlock_irqrestore (&xtime_lock, flags);

	if (tv->tv_usec >= 1000000) {
		tv->tv_usec -= 1000000;
		tv->tv_sec++;
	}
}

void do_settimeofday(struct timeval *tv)
{
	write_lock_irq (&xtime_lock);

	/* This is revolting. We need to set the xtime.tv_usec
	 * correctly. However, the value in this location is
	 * is value at the last tick.
	 * Discover what correction gettimeofday
	 * would have done, and then undo it!
	 */
	tv->tv_usec -= do_gettimeoffset();

	if (tv->tv_usec < 0) {
		tv->tv_usec += 1000000;
		tv->tv_sec--;
	}
	xtime = *tv;
	time_adjust = 0;			/* stop active adjtime() */
	time_status |= STA_UNSYNC;
	time_maxerror = NTP_PHASE_LIMIT;
	time_esterror = NTP_PHASE_LIMIT;

	write_unlock_irq (&xtime_lock);
}


/*
 * Gettimeoffset routines.  These routines returns the time duration
 * since last timer interrupt in usecs.
 *
 * If the exact CPU counter frequency is known, use fixed_rate_gettimeoffset.
 * Otherwise use calibrate_gettimeoffset()
 *
 * If the CPU does not have counter register all, you can either supply
 * your own gettimeoffset() routine, or use null_gettimeoffset() routines,
 * which gives the same resolution as HZ.
 */


/* This is for machines which generate the exact clock. */
#define USECS_PER_JIFFY (1000000/HZ)

/* usecs per counter cycle, shifted to left by 32 bits */
static unsigned int sll32_usecs_per_cycle=0;

/* how many counter cycles in a jiffy */
static unsigned long cycles_per_jiffy=0;

/* Cycle counter value at the previous timer interrupt.. */
static unsigned int timerhi, timerlo;

/* last time when xtime and rtc are sync'ed up */
static long last_rtc_update;

/* the function pointer to one of the gettimeoffset funcs*/
unsigned long (*do_gettimeoffset)(void) = null_gettimeoffset;

unsigned long null_gettimeoffset(void)
{
	return 0;
}

unsigned long fixed_rate_gettimeoffset(void)
{
	u32 count;
	unsigned long res;

	/* Get last timer tick in absolute kernel time */
	count = read_32bit_cp0_register(CP0_COUNT);

	/* .. relative to previous jiffy (32 bits is enough) */
	count -= timerlo;

	__asm__("multu\t%1,%2\n\t"
	        "mfhi\t%0"
	        :"=r" (res)
	        :"r" (count),
	         "r" (sll32_usecs_per_cycle));

	/*
	 * Due to possible jiffies inconsistencies, we need to check
	 * the result so that we'll get a timer that is monotonic.
	 */
	if (res >= USECS_PER_JIFFY)
		res = USECS_PER_JIFFY-1;

	return res;
}

/*
 * Cached "1/(clocks per usec)*2^32" value.
 * It has to be recalculated once each jiffy.
 */
static unsigned long cached_quotient;

/* Last jiffy when calibrate_divXX_gettimeoffset() was called. */
static unsigned long last_jiffies = 0;


/*
 * This is copied from dec/time.c:do_ioasic_gettimeoffset() by Mercij.
 */
unsigned long calibrate_div32_gettimeoffset(void)
{
	u32 count;
	unsigned long res, tmp;
	unsigned long quotient;

	tmp = jiffies;

	quotient = cached_quotient;

	if (last_jiffies != tmp) {
		last_jiffies = tmp;
		if (last_jiffies != 0) {
			unsigned long r0;
			do_div64_32(r0, timerhi, timerlo, tmp);
			do_div64_32(quotient, USECS_PER_JIFFY,
			            USECS_PER_JIFFY_FRAC, r0);
			cached_quotient = quotient;
		}
	}

	/* Get last timer tick in absolute kernel time */
	count = read_32bit_cp0_register(CP0_COUNT);

	/* .. relative to previous jiffy (32 bits is enough) */
	count -= timerlo;

	__asm__("multu  %2,%3"
	        : "=l" (tmp), "=h" (res)
	        : "r" (count), "r" (quotient));

	/*
	 * Due to possible jiffies inconsistencies, we need to check
	 * the result so that we'll get a timer that is monotonic.
	 */
	if (res >= USECS_PER_JIFFY)
		res = USECS_PER_JIFFY - 1;

	return res;
}

unsigned long calibrate_div64_gettimeoffset(void)
{
	u32 count;
	unsigned long res, tmp;
	unsigned long quotient;

	tmp = jiffies;

	quotient = cached_quotient;

	if (tmp && last_jiffies != tmp) {
		last_jiffies = tmp;
		__asm__(".set\tnoreorder\n\t"
	        ".set\tnoat\n\t"
	        ".set\tmips3\n\t"
	        "lwu\t%0,%2\n\t"
	        "dsll32\t$1,%1,0\n\t"
	        "or\t$1,$1,%0\n\t"
	        "ddivu\t$0,$1,%3\n\t"
	        "mflo\t$1\n\t"
	        "dsll32\t%0,%4,0\n\t"
	        "nop\n\t"
	        "ddivu\t$0,%0,$1\n\t"
	        "mflo\t%0\n\t"
	        ".set\tmips0\n\t"
	        ".set\tat\n\t"
	        ".set\treorder"
	        :"=&r" (quotient)
	        :"r" (timerhi),
	         "m" (timerlo),
	         "r" (tmp),
	         "r" (USECS_PER_JIFFY));
	        cached_quotient = quotient;
	}

	/* Get last timer tick in absolute kernel time */
	count = read_32bit_cp0_register(CP0_COUNT);

	/* .. relative to previous jiffy (32 bits is enough) */
	count -= timerlo;

	__asm__("multu\t%1,%2\n\t"
	        "mfhi\t%0"
	        :"=r" (res)
	        :"r" (count),
	         "r" (quotient));

	/*
	 * Due to possible jiffies inconsistencies, we need to check
	 * the result so that we'll get a timer that is monotonic.
	 */
	if (res >= USECS_PER_JIFFY)
		res = USECS_PER_JIFFY-1;

	return res;
}


/*
 * high-level timer interrupt service routines.  This function
 * is set as irqaction->handler and is invoked through do_IRQ.
 */
void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	
//	 int status;
//Brad add for update flash check 20080711	 
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL8197B_PANA) && !defined(CONFIG_RTL8196C_KLD) && !defined(CONFIG_RTL8196C_EC)
	 static int update_flash_time=0;
	 extern int start_count_time;
	 extern int Reboot_Wait;
#endif	 
	// clear watch dog first
#ifdef CONFIG_RTL8181_WTDOG	 
	// if kernel fault is happened, do not reset watchdog timer, rather reset
	// watchdog immediately
    if (!is_fault)
    	outl(0x700, WDTCNR);
    else {
		outl(0,GIMR0);
		cli();
		*(volatile unsigned long *)(0xBD010058)=0x10; // this is to enable watchdog
		*(volatile unsigned long *)(0xBD01005c)=0x00; // this is to trigger timer to reset system immediately
		for(;;);
	}   
#endif    

#ifdef CONFIG_RTL865X_WTDOG	 
    if (!is_fault) {
#if defined(CONFIG_RTL865X_PANAHOST) || defined(CONFIG_RTL8197B_PANA)
        *(volatile unsigned long *)(0xB800311c) |=  1 << 23;
#else
		int reset_timer = 0;
		static unsigned int pre_context_swtch = 0;
		static unsigned int pre_cpu_time_squeeze= 0; //mark_add
		static unsigned int pre_rtk_nat_fwd_cnt= 0; //mark_dbg
#ifdef RTK_RX_TIME_SQUEEZE_CHECK		
		struct softnet_data *queue;
#endif
		if (kstat.context_swtch > 0) {
			if (pre_context_swtch != kstat.context_swtch) {
				pre_context_swtch = kstat.context_swtch;
				reset_timer = 1;				
			}
#ifdef RTK_RX_TIME_SQUEEZE_CHECK
			else
			{
				//mark_add
				queue = &softnet_data[0]; 
				if (pre_cpu_time_squeeze != rtk_rx_time_squeeze) 
				{
					pre_cpu_time_squeeze = rtk_rx_time_squeeze;
					reset_timer = 1;	
				}						
				else if( queue->input_pkt_queue.qlen > 0) //mark_dbg
				{
					reset_timer = 1;	
				}
			}
#endif			
		}
		else
			reset_timer = 1;
		
		if (reset_timer)
			*(volatile unsigned long *)(0xB800311c) |=  1 << 23;
#endif
	}
	else {
		// quick fix for warn reboot fail issue
#if defined(CONFIG_RTL8192SE) || defined(CONFIG_RTL8192CD)
#if !defined(CONFIG_RTL865X_PANAHOST) && !defined(CONFIG_RTL8197B_PANA)
		extern void force_stop_wlan_hw();
		force_stop_wlan_hw();
#endif
#endif
#ifdef CONFIG_RTL8196_RTL8366
		extern void force_reset_nic_8366();
		 force_reset_nic_8366();
#endif
		cli();		
		*(volatile unsigned long *)(0xB800311c)=0; /*this is to enable 865xc watch dog reset*/
		for(;;);
	}
#endif

#ifdef CONFIG_RTL_EB8186
    status = inl(TCIR);
    outl(status, TCIR);
#endif

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
	mips_timer_ack();
#endif	

#if 0
	if (mips_cpu.options & MIPS_CPU_COUNTER) {
		unsigned int count;

		/*
		 * The cycle counter is only 32 bit which is good for about
		 * a minute at current count rates of upto 150MHz or so.
		 */
		count = read_32bit_cp0_register(CP0_COUNT);
		timerhi += (count < timerlo);   /* Wrap around */
		timerlo = count;

		/*
		 * set up for next timer interrupt - no harm if the machine
		 * is using another timer interrupt source.
		 * Note that writing to COMPARE register clears the interrupt
		 */
		write_32bit_cp0_register (CP0_COMPARE,
					  count + cycles_per_jiffy);

	}
#endif
	if(!user_mode(regs)) {
		if (prof_buffer && current->pid) {
			extern int _stext;
			unsigned long pc = regs->cp0_epc;

			pc -= (unsigned long) &_stext;
			pc >>= prof_shift;
			/*
			 * Dont ignore out-of-bounds pc values silently,
			 * put them into the last histogram slot, so if
			 * present, they will show up as a sharp peak.
			 */
			if (pc > prof_len-1)
			pc = prof_len-1;
			atomic_inc((atomic_t *)&prof_buffer[pc]);
		}
	}

	/*
	 * call the generic timer interrupt handling
	 */
	do_timer(regs);

	/*
	 * If we have an externally synchronized Linux clock, then update
	 * CMOS clock accordingly every ~11 minutes. rtc_set_time() has to be
	 * called as close as possible to 500 ms before the new second starts.
	 */
#if 0
	read_lock (&xtime_lock);
	if ((time_status & STA_UNSYNC) == 0 &&
	    xtime.tv_sec > last_rtc_update + 660 &&
	    xtime.tv_usec >= 500000 - ((unsigned) tick) / 2 &&
	    xtime.tv_usec <= 500000 + ((unsigned) tick) / 2) {
		if (rtc_set_time(xtime.tv_sec) == 0) {
			last_rtc_update = xtime.tv_sec;
		} else {
			last_rtc_update = xtime.tv_sec - 600; 
			/* do it again in 60 s */
		}
	}
	read_unlock (&xtime_lock);
#endif
	/*
	 * If jiffies has overflowed in this timer_interrupt we must
	 * update the timer[hi]/[lo] to make fast gettimeoffset funcs
	 * quotient calc still valid. -arca
	 */
	if (!jiffies) {
		timerhi = timerlo = 0;
	}
//Brad add for update flash check 20080711	 	
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196B_KLD) && !defined(CONFIG_RTL865X_SC) && !defined(CONFIG_RTL8197B_PANA) && !defined(CONFIG_RTL8196C_KLD) &&!defined(CONFIG_RTL8196C_EC)
	if(start_count_time == 1){
		update_flash_time = jiffies;
		start_count_time = 0;
	}
#if defined(CONFIG_RTL8181_WTDOG) || defined(CONFIG_RTL865X_WTDOG)
	if (update_flash_time && (jiffies - update_flash_time) > Reboot_Wait){ 			
		is_fault=1;
	}
#endif	
#endif	
#ifdef CONFIG_RTL8186_KB
	#define GPCDDATA	0x130
	extern int disable_power_led_blink;
	static int led_cnt=0;
	
	if (disable_power_led_blink == 0) {
		if ((jiffies%50) == 0) {
			if (led_cnt++%2)
				outl(1|inl(GPCDDATA), GPCDDATA);
			else
				outl(0xfffffffe&inl(GPCDDATA), GPCDDATA);
		}
	}
	else if (disable_power_led_blink == 1) { // ready
		outl(0xfffffffe&inl(GPCDDATA), GPCDDATA);
		disable_power_led_blink = 2;
	}
	else if (disable_power_led_blink == 3) { // power-off	
		outl(1|inl(GPCDDATA), GPCDDATA);
		disable_power_led_blink = 2;
	}	
#endif

#ifdef CONFIG_RTL865X_SC
{
	// Under constructing.	
}
#endif

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
{
	//#define GPABDDATA	0x120
	#define  RTL_W32TR(addr, l)	((*(volatile unsigned long*)(addr)) = (l))
	#define  RTL_R32TR(addr)		(*(volatile unsigned long *)(addr))
	#define GPABDDATA	0xB800350C //Brad modify for tr 11n
#if defined(CONFIG_RTL8196C_EC)	
	#define RESET_LED_PIN		6
#else
	#define RESET_LED_PIN		13
#endif	
	static int led_cnt=0;	
	extern int system_led_blink;
#if 0 //Brad disable, since the check is not precise for system status
	extern unsigned int sys_keep_alive;
	static unsigned int last_sys_keep_alive=0;
	static unsigned int check_count=0;	
	static unsigned int start_check=0;
	static unsigned int check_interval=5000;
#endif
	int interval;	
	static int last_system_led_blink=0;	
	static int update_flash_time=0;
	extern int flash_write_flag;
#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	extern int kld_model;
       int update_flash_interval = (flash_write_flag == 0x8000 ? 7000 : 7000);

#else
	int update_flash_interval = (flash_write_flag == 0x8000 ? 7000 : 5150);
#endif		
#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if(	kld_model==0){
#endif		
//	if (system_led_blink == 3)
//		return;
	if (system_led_blink == 2)	{ // disbale	
		//RTL_W32TR(0x4|RTL_R32TR(GPABDDATA), GPABDDATA);
		RTL_W32TR(GPABDDATA, (RTL_R32TR(GPABDDATA) | ((1 << RESET_LED_PIN))));	
		system_led_blink = 3;	
	}
	else {
		if (system_led_blink )
			interval = 3; 	// 3ms
		else
			interval = 100;	 // 1s
		if ((jiffies%interval) == 0) {
			//printk("the led_cnt=%d\n", led_cnt);
			if (led_cnt++%2){
				RTL_W32TR(GPABDDATA, (RTL_R32TR(GPABDDATA) | ((1 << RESET_LED_PIN))));
				//RTL_W32TR(0x4|RTL_R32TR(GPABDDATA), GPABDDATA);
			}else{
				//RTL_W32TR((~0x4)&RTL_R32TR(GPABDDATA), GPABDDATA);	
				RTL_W32TR(GPABDDATA, (RTL_R32TR(GPABDDATA) & (~(1 << RESET_LED_PIN))));					
			}
		}
	}
#if defined(CONFIG_RTL865X_KLD)	|| defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
}
#endif
	// add reset mechanism when update flash is too long		
#ifndef CONFIG_RTL865X_CMO		
	if (system_led_blink==1 && last_system_led_blink==0)			
		update_flash_time = jiffies;		
	if (system_led_blink==0 && last_system_led_blink)			
		update_flash_time = 0;		
	last_system_led_blink = system_led_blink;				
	//if (update_flash_time && (jiffies - update_flash_time) > 4000) 	
#ifdef CONFIG_RTL865X_WTDOG			
	if (update_flash_time && (jiffies - update_flash_time) > update_flash_interval) //sync from rtl8186 11g tnet code
		is_fault=1;
#endif
#endif		
#if 0		//Brad disable, since the check is not precise for system status
	if ((jiffies%check_interval) == 0)  {
		if(update_flash_time ==0){
			if((sys_keep_alive > last_sys_keep_alive) || (sys_keep_alive < last_sys_keep_alive)) {
				last_sys_keep_alive =  sys_keep_alive;
				check_count = 0;
			}
			else if(sys_keep_alive == last_sys_keep_alive){
				check_count++;
				if(check_count==2){
					printk("System hang!\n");
					is_fault=1;
				}
			}
		}
	if(start_check <2){
		start_check++;
		check_interval=5000;
	}else
		check_interval=1000;	  
	}
#endif			
	
}
#endif
}

#if 0
asmlinkage void ll_timer_interrupt(int irq, struct pt_regs *regs)
{
	int cpu = smp_processor_id();

	irq_enter(cpu, irq);
	kstat.irqs[cpu][irq]++;

	/* we keep interrupt disabled all the time */
	timer_interrupt(irq, NULL, regs);
	
	irq_exit(cpu, irq);

	if (softirq_pending(cpu))
		do_softirq();
}
#endif

/*
 * time_init() - it does the following things.
 *
 * 1) board_time_init() - 
 * 	a) (optional) set up RTC routines, 
 *      b) (optional) calibrate and set the mips_counter_frequency
 *	    (only needed if you intended to use fixed_rate_gettimeoffset
 *	     or use cpu counter as timer interrupt source)
 * 2) setup xtime based on rtc_get_time().
 * 3) choose a appropriate gettimeoffset routine.
 * 4) calculate a couple of cached variables for later usage
 * 5) board_timer_setup() - 
 *	a) (optional) over-write any choices made above by time_init().
 *	b) machine specific code should setup the timer irqaction.
 *	c) enable the timer interrupt
 */ 

void (*board_time_init)(void) = NULL;
void (*board_timer_setup)(struct irqaction *irq) = NULL;

unsigned int mips_counter_frequency = 0;

static struct irqaction timer_irqaction = {
	timer_interrupt,
	SA_INTERRUPT,
	0,
	"timer",
	NULL,
	NULL
};

void __init time_init(void)
{
	if (board_time_init)
		board_time_init();

	xtime.tv_sec = rtc_get_time();
	xtime.tv_usec = 0;

	/* choose appropriate gettimeoffset routine */
	do_gettimeoffset = null_gettimeoffset;


	/* 
	 * Call board specific timer interrupt setup.
	 *
	 * this pointer must be setup in machine setup routine. 
	 *
	 * Even if the machine choose to use low-level timer interrupt,
	 * it still needs to setup the timer_irqaction.
	 * In that case, it might be better to set timer_irqaction.handler 
	 * to be NULL function so that we are sure the high-level code
	 * is not invoked accidentally.
	 */
	board_timer_setup(&timer_irqaction);
}

#define FEBRUARY		2
#define STARTOFTIME		1970
#define SECDAY			86400L
#define SECYR			(SECDAY * 365)
#define leapyear(year)		((year) % 4 == 0)
#define days_in_year(a)		(leapyear(a) ? 366 : 365)
#define days_in_month(a)	(month_days[(a) - 1])

static int month_days[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void to_tm(unsigned long tim, struct rtc_time * tm)
{
	long hms, day, gday;
	int i;

	gday = day = tim / SECDAY;
	hms = tim % SECDAY;

	/* Hours, minutes, seconds are easy */
	tm->tm_hour = hms / 3600;
	tm->tm_min = (hms % 3600) / 60;
	tm->tm_sec = (hms % 3600) % 60;

	/* Number of years in days */
	for (i = STARTOFTIME; day >= days_in_year(i); i++)
	day -= days_in_year(i);
	tm->tm_year = i;

	/* Number of months in days left */
	if (leapyear(tm->tm_year))
	days_in_month(FEBRUARY) = 29;
	for (i = 1; day >= days_in_month(i); i++)
	day -= days_in_month(i);
	days_in_month(FEBRUARY) = 28;
	tm->tm_mon = i-1;	/* tm_mon starts from 0 to 11 */

	/* Days are what is left over (+1) from all that. */
	tm->tm_mday = day + 1;

	/*
	 * Determine the day of week
	 */
	tm->tm_wday = (gday + 4) % 7; /* 1970/1/1 was Thursday */
}
