
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/rtl8181.h>


#Cyrus Tsai
#This INT dispatch routine is triggered via Exception->CAUSE:EXC-CODE:INT
	.text
	.set	noreorder
	.set	noat
	.align	5
	NESTED(IRQ_finder, PT_SIZE, sp)
	SAVE_ALL
	CLI
	.set	at
	mfc0    t0, CP0_CAUSE       #get pending interrupts
        nop
        nop
        
        mfc0    t2, CP0_STATUS
        nop
        nop
        
        and     t0, t2              #isolate allowed ones
        andi    t2, t0, 0xfc00      #CT: just look at the Hardware INT bit 10-15
        
        la      t0, (GISR  + 0xb8000000)
        la      t1, (GIMR0 + 0xb8000000)
        lw      a0, 0(t0)  	    #Passing argument a0,a1, pass to handler. 
        lw      a1, 0(t1)	    #ISR: INT status, IMR: INT mask
        nop
        nop
        
        and     a0, a1		    #like t0,t2, isolate allowed ones.
        and     a0, 0xffffffff	    #extend to 32 interrupt 2007/3/23
        bne     t2, zero, handle_it #it shows there are pending interrupt, jump to Handler.
        nop
        nop
1:        
        li      t0,( UART_THR + 0xb8000000)
        li      t1,'m'		    #it shows m, that is not reasonable to come here.
        nop
        sb      t1, 0(t0)
     	nop
     	nop
	 		            #just hang here.
      	b	1b 
        nop
        nop

handle_it:  
#if 0	//irq happend, will display
	//---------------------------------------------------------- 
        li      t0,( UART_THR + 0xb8000000)
        li      t1,'i'		   
        nop
        sb      t1, 0(t0)
	//----------------------------------------------------------
#endif
        
	jal irq_dispatch        #this will lead to real ISR 
	nop
	nop
	
        move   a1,sp
	RESTORE_ALL_AND_RET
	nop
	nop
	END(IRQ_finder)

	.set    at



