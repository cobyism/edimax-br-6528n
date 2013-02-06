


#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>
#include <asm/addrspace.h>

	.text
	.set    noreorder
	.align  5
	NESTED(IA7546RIexception, PT_SIZE, ra)
	.set    noat
	SAVE_ALL
	CLI                             # TEST: interrupts should be off
	.set    at
	.set    noreorder

	//addiu   a0, sp, _STKFRM
	addiu   a0, sp, 24 
        jal     emulateIA7546RI 
        nop
	#j       ret_from_sys_call
	j       restore_all
         nop
	
	END(IA7546RIexception)
