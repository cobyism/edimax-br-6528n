#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#if defined(RTL8196B)
#include <asm/rtl8196x.h>
#endif


//Matt
#define Context_Switch_Bound	0xff
#define Context_Switch_Max_Bound 0xfff
extern unsigned long Context_Switch_Count;
extern int Lock_Time_Ctxt_Switch;
extern int free_IRQ(unsigned long irq);
extern int request_IRQ(unsigned long irq, struct irqaction *action, void* dev_id);
extern void memtest();
int taskdata[6]={0,0,0,0,0,0};
volatile int abc=0;

//############################

// extern union task_union sys_tasks[NUM_TASK];
unsigned long Jiffies;

typedef struct
{
	volatile unsigned long timer0tick;
	volatile unsigned long timer1tick;	
	volatile unsigned long timer2tick;
	volatile unsigned long timer3tick;
}
timer_t;

timer_t	rtl_timer;

static unsigned char*timer_test_buf;
static unsigned int *timer_buf;
static unsigned int *timer_read_buf;

#if 0 //UNUSED
//Matt
void errorlog(int i){
	prom_printf("Task %d Test Errors!" ,i );
	cli();
	for(;;);
}
//##################
#endif

/*TestBufferFree if test over*/



void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{             
	//unsigned int i,j;	
	unsigned char status;
	static int light=0;

//	prom_printf("in timer interrupt\n");
	
	prom_printf("IRQ=%x\n",irq);
	if (irq != 8)
	{
		prom_printf("in timer irq, irq!=0\n");
		for(;;);
	}	
	status = rtl_inl(TCIR);
	if (status & 0x40000000)
	{
		((volatile timer_t *)(dev_id))->timer0tick= ((volatile timer_t *)(dev_id))->timer0tick + 1;
		#if (WATCHDOG_TEST == 1) 
			    /*Cyrus Tsai*/
                /*this should be longer */		
		if (Jiffies <=0x19)
		   {
		    rtl_outl(WDTCNR,0x100);	
		   } 
		#endif
	}
	if (status & 0x20)
	{
		((volatile timer_t *)(dev_id))->timer1tick= ((volatile timer_t *)(dev_id))->timer1tick + 1;
		//serial_outc('T');
		
		//wlan_tx_list_generate(0xFF);
		flush_cache();


		//twiddle(); 
		Jiffies++;	

		if(Jiffies>1000)
		{
		prom_printf("----\n");
		Jiffies=0;
		}



	}	
	if (status & 0x40)
		((volatile timer_t *)(dev_id))->timer2tick= ((volatile timer_t *)(dev_id))->timer2tick + 1;	

}

struct irqaction irq8 = {timer_interrupt, 0, 8, "timer", NULL, NULL};
             
struct timer_default
{
	char *regname;
	unsigned long offset;
	unsigned int val;	
};



struct timer_default timerreg[] =
{
	{
		regname : "TCCNR",
		offset:	   TCCNR,
		val:	   0xFFFFF000,
	},
	
	{
		regname : "TCIR",
		offset:	   TCIR,
		val:	   0xFFFFFF00,
	},
	
	{
		regname:  "WDTCNR",
		offset:    WDTCNR,
		val	:	   0xFFFFF8A5,
	},
};

void  timer_pos(void)
{
	
	unsigned int val,i;
	for(i=0; i < ( (sizeof (timerreg)) / (sizeof (timerreg[0]))); i++)
	{
		
		val = rtl_inl(timerreg[i].offset);
		if (val != timerreg[i].val)
			prom_printf("READ(%s) %X != EXPECTED %X for timer pos\n", 
						timerreg[i].regname, val, timerreg[i].val);
	}						
}     
             

void time_init(void)
{
	//set up 2 timer interrupt
	/*timer 0 WDG*/
    /*timer 1 Twiddle */
	rtl_outl(TCCNR, 0x0f);
	rtl_outl(TC0DATA, 0xfff);
	rtl_outl(TC1DATA, 0xfffffff);
	//rtl_outl(TC2DATA, 0x7988);
	//rtl_outl(TC3DATA, 0xf9788);
	rtl_outl(TCIR, 0x03);
	#if ((WATCHDOG_TEST == 1))
	rtl_outl(BTDATA, 0xff);		
	#endif
	request_IRQ(8, &irq8,&rtl_timer);		
}             

void time1_init(unsigned long time)
{
	//set up 1 timer interrupt
    /*timer 1 Twiddle */
	rtl_outl(TCCNR, 0x0c);
	rtl_outl(TC1DATA, time);
	rtl_outl(TCIR, 0x02);
	request_IRQ(8, &irq8,&rtl_timer);		
}             

void time_init_2(void)
{
	rtl_outl(TCCNR, 0xf0000000);
	rtl_outl(TC0DATA, 0xfff000);
	rtl_outl(TC1DATA, 0x55f000);
	rtl_outl(TCIR, 0xc0000000);
	rtl_outl(BTDATA, 0x40000);
	rtl_outl(IRR1, 0x44);
	sti();
	request_IRQ(8, &irq8,&rtl_timer);	
}

        
void request_timer(unsigned long time){
	prom_printf("%d!!!!!!\n",time);
	time1_init(time);
	//	time1_init(time);
	}
void stop_timer(){
	unsigned long status;
	status=	rtl_inl(TCCNR);
	status&=0xfffffff3;
	rtl_outl(TCCNR,status);
	}

void time_stop(void)
{
	/*here add a function to stop the timer*/
	/*i just want to see the twiddle in SDRAM test.*/
	rtl_outl(TCCNR, 0x00);
	rtl_outl(TC1DATA, 0x0);
	rtl_outl(TCIR, 0x0);
	free_IRQ(0);
}

void watchdog_en(void)
{
	rtl_outl(WDTCNR, 0x700);
}


timer_t	rtl_timer;


void timer_isr(int, void *, struct pt_regs *);


struct irqaction timer_irq = { timer_isr,0,0, "TIMER_ISR", 0, 0};
	

void timer_isr(int irq_nr, void * dev_id, struct pt_regs *reg)
{
}

#if 0 //UNUSED
void timer_task(void)
{/*
	unsigned char *buf;
	unsigned int i;
	
	// timer_init	
	//request_irq(0, NULL, NULL);
	
	int	loop=0;
	*/
	static unsigned long count=0;
	
	prom_printf("This is timer task init speaking\n");
	//############################
	//Matt
	//############################
//    time1_init();	
 	time_init_2();
	//matt
//	for(;;)
//	{
//	printk("Hello! This is %d-th timer_task speaking\n",12);
		
		count++;
		//printk("Hello! This is %d-th timer_task speaking\n",loop++);
		if(count>=0x100)
		{
//		 prom_printf(".");
		 count=0;
		} 
	
//		schedule((unsigned long)(get_ctxt_buf(&(sys_tasks[TIMER_TASK]))));		
//	}
}
#endif