/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef __ASM_ELF_H
#define __ASM_ELF_H

/* ELF register definitions */
#define ELF_NGREG	45
#define ELF_NFPREG	33

/* ELF header e_flags defines. MIPS architecture level. */
#define EF_MIPS_ARCH_1      0x00000000  /* -mips1 code.  */
#define EF_MIPS_ARCH_2      0x10000000  /* -mips2 code.  */
#define EF_MIPS_ARCH_3      0x20000000  /* -mips3 code.  */
#define EF_MIPS_ARCH_4      0x30000000  /* -mips4 code.  */
#define EF_MIPS_ARCH_5      0x40000000  /* -mips5 code.  */
#define EF_MIPS_ARCH_32     0x60000000  /* MIPS32 code.  */
#define EF_MIPS_ARCH_64     0x70000000  /* MIPS64 code.  */
#define EF_MIPS_ARCH_32R2   0x80000000  /* MIPS32 code.  */
#define EF_MIPS_ARCH_64R2   0x90000000  /* MIPS64 code.  */

typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef double elf_fpreg_t;
typedef elf_fpreg_t elf_fpregset_t[ELF_NFPREG];

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(hdr)						\
({									\
	int __res = 1;							\
	struct elfhdr *__h = (hdr);					\
									\
	if (__h->e_machine != EM_MIPS)					\
		__res = 0;						\
	if (((__h->e_flags & EF_MIPS_ARCH) != EF_MIPS_ARCH_1) &&       	\
	    ((__h->e_flags & EF_MIPS_ARCH) != EF_MIPS_ARCH_2) &&       	\
	    ((__h->e_flags & EF_MIPS_ARCH) != EF_MIPS_ARCH_32) &&      	\
	    ((__h->e_flags & EF_MIPS_ARCH) != EF_MIPS_ARCH_32R2))      	\
		__res = 0;						\
									\
	__res;								\
})

/* This one accepts IRIX binaries.  */
#define irix_elf_check_arch(hdr)	((hdr)->e_machine == EM_MIPS)

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#ifdef __MIPSEB__
#define ELF_DATA	ELFDATA2MSB
#elif __MIPSEL__
#define ELF_DATA	ELFDATA2LSB
#endif
#define ELF_ARCH	EM_MIPS
#ifndef CONFIG_ELF_CORE_OFF //sc_yang
#define USE_ELF_CORE_DUMP
#endif
#define ELF_EXEC_PAGESIZE	4096

#define ELF_CORE_COPY_REGS(_dest,_regs)				\
	memcpy((char *) &_dest, (char *) _regs,			\
	       sizeof(struct pt_regs));

/* This yields a mask that user programs can use to figure out what
   instruction set this cpu supports.  This could be done in userspace,
   but it's not easy, and we've already done it here.  */

#define ELF_HWCAP       (0)

/* This yields a string that ld.so will use to load implementation
   specific libraries for optimization.  This is more specific in
   intent than poking at uname or /proc/cpuinfo.

   For the moment, we have only optimizations for the Intel generations,
   but that could change... */

#define ELF_PLATFORM  (NULL)

/*
 * See comments in asm-alpha/elf.h, this is the same thing
 * on the MIPS.
 */
#define ELF_PLAT_INIT(_r)	do { \
	_r->regs[1] = _r->regs[2] = _r->regs[3] = _r->regs[4] = 0;	\
	_r->regs[5] = _r->regs[6] = _r->regs[7] = _r->regs[8] = 0;	\
	_r->regs[9] = _r->regs[10] = _r->regs[11] = _r->regs[12] = 0;	\
	_r->regs[13] = _r->regs[14] = _r->regs[15] = _r->regs[16] = 0;	\
	_r->regs[17] = _r->regs[18] = _r->regs[19] = _r->regs[20] = 0;	\
	_r->regs[21] = _r->regs[22] = _r->regs[23] = _r->regs[24] = 0;	\
	_r->regs[25] = _r->regs[26] = _r->regs[27] = _r->regs[28] = 0;	\
	_r->regs[30] = _r->regs[31] = 0;				\
} while (0)

/* This is the location that an ET_DYN program is loaded if exec'ed.  Typical
   use of this is to invoke "./ld.so someprog" to test out a new version of
   the loader.  We need to make sure that it is out of the way of the program
   that it will "exec", and that there is sufficient room for the brk.  */

#define ELF_ET_DYN_BASE         (TASK_SIZE / 3 * 2)

#ifdef __KERNEL__
#define SET_PERSONALITY(ex, ibcs2) set_personality((ibcs2)?PER_SVR4:PER_LINUX)
#endif

#endif /* __ASM_ELF_H */
