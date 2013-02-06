        
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>        

        .text
        .set noreorder
		.globl __start              
__start:
		//setup stack pointer
		la	t0, __bss_end
		nop
		addi 	t0, t0, 4096
		nop
		move 	sp, t0
		nop
test:
		jal	boot_entry

