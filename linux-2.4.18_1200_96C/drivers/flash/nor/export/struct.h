#ifndef _RTL_STRUCT_H_
#define _RTL_STRUCT_H_


#include <asm/semaphore.h>
#include <linux/timer.h>



struct Flash_Dev {
      int usage;
      int do_not_schedule;
      struct semaphore sem;


#ifdef  CONFIG_RTL8181_GPIO
      struct timer_list gp_timer;
      unsigned int gp_counter;
#endif      

};


#endif
