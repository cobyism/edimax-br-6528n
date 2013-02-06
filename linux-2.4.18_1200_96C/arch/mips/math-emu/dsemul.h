typedef long gpreg_t;
typedef u_int8_t *vaddr_t;

#define REG_TO_VA (vaddr_t)
#define VA_TO_REG (gpreg_t)
#define LINUX_FPU_EMUL 1

/* Instruction inserted following delay slot instruction to force trap */
#define CP1UNDEF 0x44400001    /* cfc1 $0,$0 undef  */

u32 mips_get_word(struct pt_regs *xcp, vaddr_t va, int *perr);
u64 mips_get_dword(struct pt_regs *xcp, vaddr_t va, int *perr);
int mips_put_word(struct pt_regs *xcp, vaddr_t va, u32 val);
int mips_put_dword(struct pt_regs *xcp, vaddr_t va, u64 val);
int mips_dsemul(struct pt_regs *regs, mips_instruction ir, vaddr_t cpc);
int do_dsemulret(struct pt_regs *xcp);
