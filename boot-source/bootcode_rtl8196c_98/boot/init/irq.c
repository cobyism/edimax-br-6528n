#include <linux/init.h>
//#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/mipsregs.h>



#if defined(RTL8196B)
#include <asm/rtl8196x.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif

/*Cyrus Tsai*/
unsigned long exception_handlers[32];
void set_except_vector(int n, void *addr);
asmlinkage void do_reserved(struct pt_regs *regs);
void __init exception_init(void);
/*Cyrus Tsai*/


static struct irqaction *irq_action[NR_IRQS] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


//#define ALLINTS (IE_IRQ0 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)	//wei del
#define ALLINTS (IE_IRQ0|IE_IRQ1 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)	//wei add, david talk

static void  unmask_irq(unsigned int irq)
{
    outl((inl(GIMR0) | (1 << irq)),GIMR0);
    inl(GIMR0);
}
static void  mask_irq(unsigned int irq)
{
    outl(inl(GIMR0) & (~(1 << irq)),GIMR0);
    inl(GIMR0);
}

extern asmlinkage void do_IRQ(int irq, struct pt_regs *regs);

void irq_dispatch(int irq_nr, struct pt_regs *regs)
{
	int i,irq=0;
	//prom_printf("irq.c : irq_nr=%x\n",irq_nr);
    for (i=0; i<=31; i++)
    {
        if (irq_nr & 0x01)
		{
			//prom_printf("do irq=%x\n",irq);
			do_IRQ(irq, regs);

		}  
        irq++;
        irq_nr = irq_nr >> 1;
    }

}

inline unsigned int					
clear_cp0_status(unsigned int clear)				
{								
	unsigned int res;					
								
	res = read_32bit_cp0_register(CP0_STATUS);		
	res &= ~clear;						
	write_32bit_cp0_register(CP0_STATUS, res);									
}								

inline unsigned int					
change_cp0_status(unsigned int change, unsigned int newvalue)	
{								
	unsigned int res;					
								
	res = read_32bit_cp0_register(CP0_STATUS);		
	res &= ~change;						
	res |= (newvalue & change);					
	write_32bit_cp0_register(CP0_STATUS, res);		
							
	return res;						
}


void __init ExceptionToIrq_setup(void)
{
	extern asmlinkage void IRQ_finder(void);

	unsigned int i;

	/* Disable all hardware interrupts */
	change_cp0_status(ST0_IM, 0x00);

	/* Set up the external interrupt exception vector */
	/* First exception is Interrupt*/
	set_except_vector(0, IRQ_finder);

	/* Enable all interrupts */
	change_cp0_status(ST0_IM, ALLINTS);
//	prom_printf("###");
}


void __init init_IRQ(void)
{
	ExceptionToIrq_setup();
}


// below is adopted from kernel/irq.c


int setup_IRQ(int irq, struct irqaction *new)
{
    int shared = 0;
    struct irqaction *old, **p;
    unsigned long flags;

    p = irq_action + irq;
//    prom_printf("IRQ action=%x,%x\n",irq_action,irq);
    save_and_cli(flags);
    *p = new;
    
    restore_flags(flags);
    
    return 0;
}

int request_IRQ(unsigned long irq, struct irqaction *action, void* dev_id)
{

    int retval;
      
 //   prom_printf("IRQ No=%x,%x\n",irq,NR_IRQS);
    if (irq >= NR_IRQS)
		return -EINVAL;

	action->dev_id = dev_id;
	
    retval = setup_IRQ(irq, action);
 //   prom_printf("devid & retval =%x,%x\n",dev_id,retval);
	unmask_irq(irq);

    if (retval)

	    return retval;
}


int	free_IRQ(unsigned long irq)
{
	mask_irq(irq);	
	
}


asmlinkage void do_IRQ(int irqnr, struct pt_regs *regs)
{
    struct irqaction *action;
	unsigned long i;

    action = *(irqnr + irq_action);
        	
	if (action) 
    {
	    action->handler(irqnr, action->dev_id, regs);
    }
	else
	{    
		prom_printf("you got irq=%X\n", irqnr);
		for(;;);
	}			
}	


//------------------------------------------------------------------------------
/*Cyrus Tsai*/
void set_except_vector(int n, void *addr)
{
	unsigned handler = (unsigned long) addr;
	exception_handlers[n] = handler;
}

asmlinkage void do_reserved(struct pt_regs *regs)
{
	/*fatal hard/software error*/
	int i;
	prom_printf("Undefined Exception happen.");	
	for(;;);
	/*Just hang here.*/
}

/*In Head.S, as sw boot up, set vector program path.*/
extern char exception_matrix;
void __init exception_init(void)
{
	unsigned long i;
	clear_cp0_status(ST0_BEV);
    /*this is for default exception handlers: NULL*/	 
	for (i = 0; i <= 31; i++)
		set_except_vector(i, do_reserved);
	//KSEG0 8000 0000 and remember it is cacheable, 
	//remember here we set BEV=0, and vector base is 80000000, offset 0x80
    memcpy((void *)(KSEG0 + 0x80), &exception_matrix, 0x80);
    flush_cache();
    
}
/*Cyrus Tsai*/
//------------------------------------------------------------------------------
