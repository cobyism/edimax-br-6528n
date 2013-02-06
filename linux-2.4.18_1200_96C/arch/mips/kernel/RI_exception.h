/* RI_exception.h */

#ifndef __RI_exception_H__
#define __RI_exception_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define OP_MASK     0x3f
#define OP_SHIFT    26
#define R_MASK      0x1f
#define RS_SHIFT    21
#define RT_SHIFT    16
#define RD_SHIFT    11
#define OFFSET_MASK 0xffff

#define _OP_(x)     (OP_MASK & ((x) >> OP_SHIFT))
#define _OPS_(x)    (OP_MASK & (x))
#define _RS_(x)     (R_MASK & ((x) >> RS_SHIFT))
#define _RT_(x)     (R_MASK & ((x) >> RT_SHIFT))
#define _RD_(x)     (R_MASK & ((x) >> RD_SHIFT))
#define _OFFSET_(x) (OFFSET_MASK & (x))


#ifndef zero
/*
 * Symbolic register names for 32 bit ABI
 */
#define zero    $0      /* wired zero */
#define AT      $1      /* assembler temp  - uppercase because of ".set at" */
#define v0      $2      /* return value */
#define v1      $3
#define a0      $4      /* argument registers */
#define a1      $5
#define a2      $6
#define a3      $7
#define t0      $8      /* caller saved */
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12
#define t5      $13
#define t6      $14
#define t7      $15
#define s0      $16     /* callee saved */
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23
#define t8      $24     /* caller saved */
#define t9      $25
#define jp      $25     /* PIC jump register */
#define k0      $26     /* kernel scratch */
#define k1      $27
#define gp      $28     /* global pointer */
#define sp      $29     /* stack pointer */
#define fp      $30     /* frame pointer */
#define s8	$30	/* same like fp! */
#define ra      $31     /* return address */

#endif /* zero */

#define _STKFRM		24
#define SREG(reg,rno,base)	sw	reg,(_STKFRM+((rno)*4))(base)
#define LREG(reg,rno,base)	lw	reg,(_STKFRM+((rno)*4))(base)
#define MTC0			mtc0
#define MFC0			mfc0

#define	R_ZERO		0 	/* $0  (zero) */
#define	R_AT		1 	/* $1  (AT) */        
#define	R_V0		2 	/* $2  (v0) */
#define	R_V1		3 	/* $3  (v1) */
#define	R_A0		4 	/* $4  (a0) */
#define	R_A1		5 	/* $5  (a1) */
#define	R_A2		6 	/* $6  (a2) */
#define	R_A3		7 	/* $7  (a3) */
#define	R_T0		8 	/* $8  (t0) */
#define	R_T1		9 	/* $9  (t1) */
#define	R_T2		10	/* $10 (t2) */
#define	R_T3		11	/* $11 (t3) */
#define	R_T4		12	/* $12 (t4) */
#define	R_T5		13	/* $13 (t5) */
#define	R_T6		14	/* $14 (t6) */
#define	R_T7		15	/* $15 (t7) */
#define	R_S0		16	/* $16 (s0) */
#define	R_S1		17	/* $17 (s1) */
#define	R_S2		18	/* $18 (s2) */
#define	R_S3		19	/* $19 (s3) */
#define	R_S4		20	/* $20 (s4) */
#define	R_S5		21	/* $21 (s5) */
#define	R_S6		22	/* $22 (s6) */
#define	R_S7		23	/* $23 (s7) */
#define	R_T8		24	/* $24 (t8) */
#define	R_T9		25	/* $25 (t9) */
#define	R_K0		26	/* $26 (k0) */
#define	R_K1		27	/* $27 (k1) */
#define	R_GP		28	/* $28 (gp) */
#define	R_SP		29	/* $29 (sp) */
#define	R_FP		30	/* $30 (s8) */
#define	R_RA		31	/* $31 (ra) */

#if 0
#define R_CAUSE 	32	/* CAUSE */
#define R_EPC   	33	/* EPC   */
#define R_STATUS 	34  /* STATUS */
#define R_SR		34
#define R_HI		35	/* hi */
#define R_LO		36  /* Lo */
#endif
#define R_LO         	32     
#define R_HI           	33    
#define R_EPC        	34  
#define R_BVADDR        35 
#define R_STATUS        36      
#define R_SR            36      
#define R_CAUSE         37     

#if 0
#define PT_LO     152
#define PT_HI     156
#define PT_EPC    160
#define PT_BVADDR 164
#define PT_STATUS 168
#define PT_CAUSE  172
#define PT_SIZE   176
#endif

#define NONFPREGS  	38


//#define EXCFRAMESIZE	((NONFPREGS*4)+_STKFRM)

#define _STKOFFSET(rno)	(*(ULONG *)(pEXCdata+((rno)*4)))
#define _GPR_STKOFFSET(x)	_STKOFFSET(x)

#define EXC_DATA_CAUSE	R_CAUSE /* cause register */
//#define EXC_DATA_BADVADDR	R_BADVADDR /* bad virtual address reg */

#define EXC_DATA_SR		R_SR /* status register */
#define EXC_DATA_EPC		R_EPC /* calculated epc */

#define EXC_DATA_LO		R_LO /* mul/div low   */
#define EXC_DATA_HI		R_HI /* mul/div hi   */

#define EXC_DATA_ZERO	R_ZERO /* just for kicks, always zero */
#define EXC_DATA_AT		R_AT /* assembler temporary */
#define EXC_DATA_V0		R_V0 /* return value 1 */
#define EXC_DATA_V1		R_V1 /* return value 2 */
#define EXC_DATA_A0		R_A0 /* passed parameter 1 */
#define EXC_DATA_A1		R_A1 /* passed parameter 2 */
#define EXC_DATA_A2		R_A2 /* passed parameter 3 */
#define EXC_DATA_A3		R_A3 /* passed parameter 4 */
#define EXC_DATA_T0		R_T0 /* temp reg t0 */
#define EXC_DATA_T1		R_T1 /* temp reg t1 */
#define EXC_DATA_T2		R_T2 /* temp reg t2 */
#define EXC_DATA_T3		R_T3 /* temp reg t3 */
#define EXC_DATA_T4		R_T4 /* temp reg t4 */
#define EXC_DATA_T5		R_T5 /* temp reg t5 */
#define EXC_DATA_T6		R_T6 /* temp reg t6 */
#define EXC_DATA_T7		R_T7 /* temp reg t7 */
#define EXC_DATA_S0		R_S0 /* saved reg s0 */
#define EXC_DATA_S1		R_S1 /* saved reg s1 */
#define EXC_DATA_S2		R_S2 /* saved reg s2 */
#define EXC_DATA_S3		R_S3 /* saved reg s3 */
#define EXC_DATA_S4		R_S4 /* saved reg s4 */
#define EXC_DATA_S5		R_S5 /* saved reg s5 */
#define EXC_DATA_S6		R_S6 /* saved reg s6 */
#define EXC_DATA_S7		R_S7 /* saved reg s7 */
#define EXC_DATA_T8		R_T8 /* temp reg t8 */
#define EXC_DATA_T9		R_T9 /* temp reg t9 */
#define EXC_DATA_K0		R_K0 /* kernel reg 0 */
#define EXC_DATA_K1		R_K1 /* kernel reg 1, not saved */
#define EXC_DATA_GP		R_GP /* global pointer */
#define EXC_DATA_SP		R_SP /* stack pointer */
#define EXC_DATA_FP		R_FP /* saved reg */
#define EXC_DATA_RA		R_RA /* return addr reg */

#define C0_SR           $12             /* status register */
#define C0_CAUSE        $13             /* exception cause */
#define C0_EPC          $14             /* exception pc */
#define C0_PRID         $15             /* revision identifier */



#define CAUSE_BD  0x80000000

#ifdef  __cplusplus
}
#endif

#endif /*__RI_exception_H__ */
