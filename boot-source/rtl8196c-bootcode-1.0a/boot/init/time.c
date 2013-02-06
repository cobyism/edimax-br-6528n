#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <linux/circ_buf.h>
#include <asm/rtl8181.h>
#include <asm/io.h>

unsigned int Jiffies;

typedef struct
{
	volatile unsigned long timer0tick;
	volatile unsigned long timer1tick;	
	volatile unsigned long timer2tick;
	volatile unsigned long timer3tick;
}
timer_t;

timer_t	rtl_timer;

void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{             
	unsigned int i,j;	
	unsigned char status;
	if (irq != 0)
	{
		prom_printf("in timer irq, irq!=0\n");
		for(;;);
	}	
	status = rtl_inl(TCIR);/*interrupt pending bit*/

	if (status & 0x20)
	{
		((volatile timer_t *)(dev_id))->timer1tick= ((volatile timer_t *)(dev_id))->timer1tick + 1;
		twiddle();
		Jiffies++;	
	}	
	rtl_outl(TCIR, status);/*Ack Interrupt*/
}

struct irqaction irq0 = {timer_interrupt, NULL, 0,
             "timer", NULL, NULL};
   
void time_init(void)
{
	/*Timer 1 enable*/
	rtl_outl(TCCNR, 0x0c);
	/*Timer 1 initial value*/
	rtl_outl(TC1DATA, 0xfffffff);
        /*Timer interrupt enable*/
	rtl_outl(TCIR, 0x02);
	/*Clock division base register*/
	rtl_outl(BTDATA, 0xff);		
        /*Register Timer interrupt*/
	request_IRQ(0, &irq0,&rtl_timer);	
}             


void watchdog_en(void)
{
	rtl_outl(WDTCNR, 0x700);
}

