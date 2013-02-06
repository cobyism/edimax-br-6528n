#include <asm/asm.h>
#include <asm/bootinfo.h>
#include <asm/cachectl.h>
#include <asm/io.h>
#include <asm/stackframe.h>
#include <asm/system.h>
#include <asm/cpu.h>
#include <asm/mipsregs.h>

asmlinkage void init_arch(int argc, char **argv, char **envp, int *prom_vec)
{
	unsigned int s;
	/* Disable coprocessors */
	s = read_32bit_cp0_register(CP0_STATUS);
	s &= ~(ST0_CU1|ST0_CU2|ST0_CU3|ST0_KX|ST0_SX);
	s |= ST0_CU0;
	write_32bit_cp0_register(CP0_STATUS, s);
	s = read_32bit_cp0_register(CP0_STATUS);

        start_kernel();
}


void  setup_arch(void)
{
	unsigned long s;
        s = read_32bit_cp0_register(CP0_STATUS);
        s |= ST0_BEV;
        s ^= ST0_BEV;
        //s |= IE_IRQ0 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4  | IE_IRQ5;	//wei del
        s |= IE_IRQ0 | IE_IRQ1|IE_IRQ2 | IE_IRQ3 | IE_IRQ4  | IE_IRQ5; //wei add, david teach for use timer IRQ 3
	write_32bit_cp0_register(CP0_STATUS, s);        
	return ;
}

