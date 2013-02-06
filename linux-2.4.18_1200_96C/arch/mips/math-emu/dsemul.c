#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>

#include <asm/asm.h>
#include <asm/bootinfo.h>
#include <asm/byteorder.h>
#include <asm/cpu.h>
#include <asm/inst.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/branch.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <asm/pgtable.h>

#include <asm/fpu_emulator.h>

#include "ieee754.h"
#include "dsemul.h"

/* Strap kernel emulator for full MIPS IV emulation */

#ifdef __mips
#undef __mips
#endif
#define __mips 4

extern struct mips_fpu_emulator_private fpuemuprivate;


/*
 * Emulate the arbritrary instruction ir at xcp->cp0_epc.  Required when
 * we have to emulate the instruction in a COP1 branch delay slot.  Do
 * not change cp0_epc due to the instruction
 *
 * According to the spec:
 * 1) it shouldnt be a branch :-)
 * 2) it can be a COP instruction :-(
 * 3) if we are tring to run a protected memory space we must take
 *    special care on memory access instructions :-(
 */

/*
 * "Trampoline" return routine to catch exception following
 *  execution of delay-slot instruction execution.
 */

/* Instruction inserted following the CP1UNDEF to further tag the sequence */
#define BD_COOKIE 0x0000bd36 /* tne $0,$0 with baggage */


int mips_dsemul(struct pt_regs *regs, mips_instruction ir, vaddr_t cpc)
{
	vaddr_t dsemul_insns;
	extern asmlinkage void handle_dsemulret(void);

	if (ir == 0) {		/* a nop is easy */
		regs->cp0_epc = VA_TO_REG(cpc);
		regs->cp0_cause &= ~CAUSEF_BD;
		return 0;
	}
#ifdef DSEMUL_TRACE
	printk("desemul %08lx %08lx\n", REG_TO_VA(regs->cp0_epc), cpc);
#endif

	/* 
	 * The strategy is to push the instruction onto the user stack 
	 * and put a trap after it which we can catch and jump to 
	 * the required address any alternative apart from full 
	 * instruction emulation!!.
	 *
	 * Algorithmics used a system call instruction, and
	 * borrowed that vector.  MIPS/Linux version is a bit
	 * more heavyweight in the interests of portability and
	 * multiprocessor support.  We flag the thread for special
	 * handling in the unaligned access handler and force an
	 * address error excpetion.
	 */

	dsemul_insns =  REG_TO_VA(regs->regs[29] & ~0x7); /* 8-byte alignment */
	dsemul_insns -= 20;

	/* Verify that the stack pointer is not completely insane */
	if (verify_area
	    (VERIFY_WRITE, dsemul_insns, 20))
		return SIGBUS;

	if (mips_put_word(regs, dsemul_insns+0, ir)) {
		fpuemuprivate.stats.errors++;
		return (SIGBUS);
	}

	if (mips_put_word(regs, dsemul_insns+4, (mips_instruction)CP1UNDEF)) {
		fpuemuprivate.stats.errors++;
		return (SIGBUS);
	}
	
	if (mips_put_word(regs, dsemul_insns+8, 
			  (mips_instruction)BD_COOKIE)) {
		fpuemuprivate.stats.errors++;
		return (SIGBUS);
	}

	if (mips_put_dword(regs, dsemul_insns+12, VA_TO_REG cpc)) {
		fpuemuprivate.stats.errors++;
		return (SIGBUS);
	}

	regs->cp0_epc = VA_TO_REG dsemul_insns;

	/* Invalidate this and the next cache line */
	flush_cache_sigtramp((gpreg_t)dsemul_insns);
	flush_cache_sigtramp((gpreg_t)dsemul_insns+16);
	return SIGILL;		/* force out of emulation loop */
}


int do_dsemulret(struct pt_regs *xcp)
{
	vaddr_t pinst, cpc;
	u32 stackitem;
	int err = 0;

	/* See if this trap was deliberate. First check the instruction */

	pinst = REG_TO_VA(xcp->cp0_epc);

	/* 
	 * If we can't even access the area, something is very wrong, but we'll
	 * leave that to the default handling
	 */
	if (verify_area(VERIFY_READ, pinst, 16))
		return 0;

	/* Is the instruction pointed to by the EPC an CP1UNDEF? */
	stackitem = mips_get_word(xcp, pinst, &err);
	if (err || (stackitem != CP1UNDEF))
		return 0;

	/* Is the following memory word the BD_COOKIE? */

	stackitem = mips_get_word(xcp, pinst+4, &err);
	if (err || (stackitem != BD_COOKIE))
		return 0;

	/* 
	 * At this point, we are satisfied that it's a BD emulation trap.  Yes,
	 * a user might have deliberately put two malformed and useless
	 * instructions in a row in his program, in which case he's in for a
	 * nasty surprise - the next instruction will be treated as a
	 * continuation address!  Alas, this seems to be the only way that we
	 * can handle signals, recursion, and longjmps() in the context of
	 * emulating the branch delay instruction.
	 */

#ifdef DSEMUL_TRACE
	printk("desemulret\n");
#endif
	/* Fetch the Saved EPC to Resume */

	cpc = REG_TO_VA (gpreg_t)mips_get_dword(xcp, pinst+8, &err);
	if (err) {
		/* This is not a good situation to be in */
		fpuemuprivate.stats.errors++;
		force_sig(SIGBUS, current);
		return 1;
	}

	/* Set EPC to return to post-branch instruction */
	xcp->cp0_epc = VA_TO_REG cpc;

	return SIGILL;
}


#ifdef LINUX_FPU_EMUL
u32
mips_get_word(struct pt_regs *xcp, vaddr_t va, int *perr)
{
	u32 temp;

	if (!user_mode(xcp)) {
		*perr = 0;
		return (*(u32 *) va);
	} else {
		/* Use kernel get_user() macro */
		*perr = (int) get_user(temp, (u32 *) va);
		return temp;
	}
}


u64
mips_get_dword(struct pt_regs *xcp, vaddr_t va, int *perr)
{
	u64 temp;

	if (!user_mode(xcp)) {
		*perr = 0;
		return (*(u64 *) va);
	} else {
		/* Use kernel get_user() macro */
		*perr = (int) get_user(temp, (u64 *) va);
		return temp;
	}
}


int mips_put_word(struct pt_regs *xcp, vaddr_t va, u32 val)
{
	if (!user_mode(xcp)) {
		*(u32 *) va = val;
		return 0;
	} else {
		/* Use kernel get_user() macro */
		return (int) put_user(val, (u32 *) va);
	}
}


int mips_put_dword(struct pt_regs *xcp, vaddr_t va, u64 val)
{
	if (!user_mode(xcp)) {
		*(u64 *) va = val;
		return 0;
	} else {
		/* Use kernel get_user() macro */
		return (int) put_user(val, (u64 *) va);
	}
}
#endif
