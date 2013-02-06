/***************************************************************************
*  Copyright (c) 2009 Realtek, Inc. All rights reserved.
*  Contains licensed, confidential, and proprietary information.
*  Use and distribution are restricted, refer to license agreement.
***************************************************************************/
/***************************************************************************
*
*  File: lexraregs.h
*
*  Description:
*    lexra cp3 register access helper macro & etc
*
***************************************************************************/

#ifndef _LEXRAREGS_H
#define _LEXRAREGS_H

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 * Coprocessor 3 gereral register names
 */
#define CP3_IWBASE $0
#define CP3_IWTOP $1
#if defined(__m5281) || defined(__m4281)
#define CP3_IWBASE1 $2
#define CP3_IWTOP1 $3
#endif
#define CP3_DWBASE $4
#define CP3_DWTOP $5
#if defined(__m5281) || defined(__m4281)
#define CP3_DWBASE1 $6
#define CP3_DWTOP1 $7
#endif
#define CP3_MONCNT0LO $8
#define CP3_MONCNT0HI $9
#define CP3_MONCNT1LO $10
#define CP3_MONCNT1HI $11
#define CP3_MONCNT2LO $12
#define CP3_MONCNT2HI $13
#define CP3_MONCNT3LO $14
#define CP3_MONCNT3HI $15
#if defined(__m5281) || defined(__m4281)
#define CP3_L2MEM_BASE $16
#define CP3_L2MEM_TOP $17
#endif

/*
 * Macros to access the general register of  coprocessor 3
 */
#define read_32bit_cp3_general_register(source)                 \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
        "mfc3\t%0,"STR(source)"\n\t"                            \
	".set\tpop"						\
        : "=r" (__res));                                        \
        __res;})


#define write_32bit_cp3_general_register(register,value)        \
        __asm__ __volatile__(                                   \
        "mtc3\t%0,"STR(register)"\n\t"				\
	"nop"							\
        : : "r" (value));

#ifndef _LANGUAGE_ASSEMBLY
/* NOTE: must stop counter first */
static inline uint64_t cp3_counter0_get_64bit( void )
{
	uint64_t __value64;
	__asm__ __volatile__(
	".set\tpush\n\t"
	".set\treorder\n\t"
        "mfc3\t%M0,\t$9\n\t"
        "mfc3\t%L0,\t$8\n\t"
	".set\tpop"
        : "=r" (__value64));

	return __value64;
}
static inline uint64_t cp3_counter1_get_64bit( void )
{
	uint64_t __value64;
	__asm__ __volatile__(
	".set\tpush\n\t"
	".set\treorder\n\t"
        "mfc3\t%M0,\t$11\n\t"
        "mfc3\t%L0,\t$10\n\t"
	".set\tpop"
        : "=r" (__value64));

	return __value64;
}
/* NOTE: auto stop/start counter */
static inline uint64_t cp3_counter0_get_64bit_auto( void )
{
	uint64_t __value64;
	__asm__ __volatile__(
	".set\tpush\n\t"
	"cfc3\t$8,\t$0\n\t"
	"ctc3\t$0,\t$0\n\t"
	"nop\n\t"
	"nop\n\t"
	".set\treorder\n\t"
        "mfc3\t%M0,\t$9\n\t"
        "mfc3\t%L0,\t$8\n\t"
	"ctc3\t$8,\t$0\n\t"
	".set\tpop"
        : "=r" (__value64)
        :
        :"$8");

	return __value64;
}
	

#endif /* !defined (_LANGUAGE_ASSEMBLY) */

/*
 * Coprocessor 3 control register names
 */
#define CP3_CONTROL0 $0
#if defined(__m5281) || defined(__m4281)
	#define CP3_CONTROL1 $1 /* dual counter mode control */
	#define CP3_CONTROL2 $2 /* enable/disable dual counter mode control */
#endif
/*
 * Macros to access the control register of  coprocessor 3
 */
#define read_32bit_cp3_control_register(source)                 \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
        "cfc3\t%0,"STR(source)"\n\t"                            \
	".set\tpop"						\
        : "=r" (__res));                                        \
        __res;})

#define write_32bit_cp3_control_register(register,value)        \
        __asm__ __volatile__(                                   \
        "ctc3\t%0,"STR(register)"\n\t"				\
	"nop"							\
        : : "r" (value));

/*
 * Macros to access the system control coprocessor 0 register
 */
#ifndef CP0_STATUS
#define CP0_STATUS $12
#endif
#define CP0_CCTL $20
#define CP0_DATA_LO $28
#define CP0_DATA_HI $29

#define CCTL_DCACHE_INVAL (1<<0)
#define CCTL_ICACHE_INVAL (1<<1)

#define CCTL_L2MEM_OFF (1<<27)
#define CCTL_L2MEM_ON (1<<26)


#define CCTL_WBUF_OFF (1<<23)
#define CCTL_WBUF_ON (1<<22)
#define CCTL_WBUFMG_OFF (1<<21)
#define CCTL_WBUFMG_ON (1<<20)

#define CCTL_DMEM_OFF (1<<11)
#define CCTL_DMEM_ON (1<<10)

#define CCTL_DCACHE_WBINVAL (1<<9)
#define CCTL_DCACHE_WB (1<<8)
#define CCTL_DCACHE_WALLOCON (1<<7)
#define CCTL_IMEM_ON (1<<6)
#define CCTL_IMEM_OFF (1<<5)
#define CCTL_IMEM_FILL (1<<4)

#ifndef __read_32bit_c0_register
#define __read_32bit_c0_register(source, sel)				\
({ int __res;								\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mfc0\t%0, " STR(source) "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " STR(source) ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})

#define __write_32bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtc0\t%z0, " STR(register) "\n\t"			\
			: : "Jr" ((unsigned int)(value)));		\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%z0, " STR(register) ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" ((unsigned int)(value)));		\
} while (0)
#endif

#if defined(__m5280)
	#define CP3_COUNT_STOP					0
	#define CP3_COUNT_CYCLE					0x10
	#define CP3_COUNT_NEW_INST_FETCH		0x11
	#define CP3_COUNT_INST_CACHE_MISS		0x12
	#define CP3_COUNT_INST_CACHE_MISS_BUSY	0x13
	#define CP3_COUNT_DATA_STORE			0x14
	#define CP3_COUNT_DATA_LOAD				0x15
	#define CP3_COUNT_LOAD_STORE			0x16
	#define CP3_COUNT_LOAD_STORE_MISS		0x1a
	#define CP3_COUNT_LOAD_STORE_MISS_BUSY	0x1b

#endif

#if defined(__m5181) || defined(__m4181) || defined(__m5280)
	#define CP3_COUNT_STOP					0
	#define CP3_COUNT_CYCLE					0x10
	#define CP3_COUNT_NEW_INST_FETCH		0x11
	#define CP3_COUNT_INST_CACHE_MISS		0x12
	#define CP3_COUNT_INST_CACHE_MISS_BUSY	0x13
	#define CP3_COUNT_DATA_STORE			0x14
	#define CP3_COUNT_DATA_LOAD				0x15
	#define CP3_COUNT_LOAD_STORE			0x16

#if defined(__m5280)
	#define CP3_COUNT_EXACT_RETIRED_INST		0x17
	#define CP3_COUNT_RETIRED_INST_FOR_PIPE_A	0x18
	#define CP3_COUNT_RETIRED_INST_FOR_PIPE_B	0x19
#endif

	#define CP3_COUNT_LOAD_STORE_MISS		0x1a
	#define CP3_COUNT_LOAD_STORE_MISS_BUSY	0x1b
#endif

#if defined(__m5281) || defined(__m4281)
	#define CP3_COUNT_STOP					0
	#define CP3_COUNT_NEW_INST_FETCH		0x1
	#define CP3_COUNT_INST_CACHE_MISS		0x2
	#define CP3_COUNT_INST_CACHE_MISS_BUSY		0x3
	#define CP3_COUNT_DATA_STORE			0x4
	#define CP3_COUNT_DATA_LOAD			0x5
	#define CP3_COUNT_LOAD_STORE			0x6
	#define CP3_COUNT_COMPLETE_INST			0X7 /* COMPLETE INSTRUCTIONS */
	#define CP3_COUNT_CYCLE				0x8
	#define CP3_COUNT_INST_CACHE_SOFT_MISS		0x9
	#define CP3_COUNT_LOAD_STORE_MISS		0xa
	#define CP3_COUNT_LOAD_STORE_MISS_BUSY		0xb
	#define CP3_COUNT_L2CACHE_HITS			0xC
	#define CP3_COUNT_L2CACHE_HIT_CYCLES		0xD
	#define CP3_COUNT_L2CACHE_MISS			0xE
	#define CP3_COUNT_L2CACHE_MISS_CYCLES		0xF

	#define CP3_COUNT_WBUF_MERGE			0x1D
	#define CP3_COUNT_WBUF_UC_MERGE			0x1E
	#define CP3_COUNT_WBUF_XFROM			0x1F
	#define CP3_COUNT_EXCEPTION			0x2D
	#define CP3_COUNT_HARD_INTERRUPT		0x2E
	#define CP3_COUNT_VECTOR_INTERRUPT		0x2F
#endif


#ifndef _LANGUAGE_ASSEMBLY
/*
 * Manipulate the cp3 control 0/1 counter register.
 * Mostly used to stop/start counter, set conuter mode.
 */
#define __BUILD_SET_CP3_CON(name,register)			\
static inline unsigned int					\
set_cp3_con_##name(unsigned int set)				\
{								\
	unsigned int res;					\
								\
	res = read_32bit_cp3_control_register(register);	\
	res |= set;						\
	write_32bit_cp3_control_register(register, res);	\
								\
	return res;						\
}								\
								\
static inline unsigned int					\
clear_cp3_con_##name(unsigned int clear)			\
{								\
	unsigned int res;					\
								\
	res = read_32bit_cp3_control_register(register);	\
	res &= ~clear;						\
	write_32bit_cp3_control_register(register, res);	\
								\
	return res;						\
}								\
								\
static inline unsigned int					\
change_cp3_con_##name(unsigned int change, unsigned int new)	\
{								\
	unsigned int res;					\
								\
	res = read_32bit_cp3_control_register(register);	\
	res &= ~change;						\
	res |= (new & change);					\
	write_32bit_cp3_control_register(register, res);	\
								\
	return res;						\
}

__BUILD_SET_CP3_CON(control0,CP3_CONTROL0)
#if defined(__m5281) || defined(__m4281)
__BUILD_SET_CP3_CON(control1,CP3_CONTROL1)
__BUILD_SET_CP3_CON(control2,CP3_CONTROL2)
#endif
#endif /* !defined (_LANGUAGE_ASSEMBLY) */

#endif /* _LEXRAREGS_H */
