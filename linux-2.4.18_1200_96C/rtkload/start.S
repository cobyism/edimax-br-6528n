#include <linux/config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>  

#if defined(CONFIG_RTL865X) || defined(CONFIG_RTL8196B)
#define CONFIG_DONOT_ADJUST_SYSCLK_MEMSP
#endif

#include "start.h"      
#ifndef boot32        
#define NEWSTACK        (0x80670000)
#else
#define NEWSTACK        (0x809FFF00)
#endif
	.globl old_stack_pointer
        
	.extern __start_dram
 
        .text
        .globl __start              
__start:
		 
#ifdef CONFIG_DONOT_ADJUST_SYSCLK_MEMSP
		nop
#else
		// Set PLLMNR and SYSCLKR
		jal		hfclock_setting
		nop
#endif		

	move s0, zero
	mtc0 s0, $12
	nop
	nop

#if 0
	/* Clock */
	li  a0,0xbd010108
	li  a1,0x0000000b
	sw  a1,0(a0)
	nop
	nop
	nop
	nop
#endif

#ifdef CONFIG_RTL_EB8186
	/* Memory Controllor */
	li  a0,0xbd011008
	li  a1,0x00000884
	li  a1,0x000008c7
	li  a1,0x00000cda
	li  a1,0x00000886
	li  a1,0x000008c7   // <victor> for 187/124
	/*sw  a1,0(a0) */
	nop
	nop
	nop
	nop




        /*--- initialize and start COP3*/
        mfc0    $8,$12
        nop
        nop
        or      $8,0x80000000
        mtc0    $8,$12
        nop
        nop


#if 1
	la   a0, 0x90000000
	mtc0 a0, $12
	nop
	nop
	mfc0 t0, $20
	nop
	nop
	ori  t0, 0x1
	mtc0 t0, $20
	nop
	nop 
#if 0
	la   a0, 0x10000000
	mtc3 a0, $4
	la   a0, 0x10000fff
	nop
	mtc3 a0, $5
	nop
	la   a0, 0x90000000
	sw   zero, 0(a0)
	lw   t0, 0(a0)
	nop
	la   a0, 0x90000fff
	sb   zero, 0(a0)
        lb   t0, 0(a0)
#endif
#endif
#endif // CONFIG_RTL_EB8186


        nop


#if 0 
        /*la              t0, __start_dram */
        /*and             t0, 0x0fffffff*/                          # translate to physical address
        la              t0, 0x81000000  
        and             t0, 0x0fffffff                          # translate to physical address
        mtc3    t0, $4                                          # $4: d-ram base
        nop
        addi    t0, t0, 0x1000 - 1				# total 2k bytes used//2003-07-04//TKIP_encryption
        mtc3    t0, $5                                          # $5: d-ram top
        nop
#endif



	la s0, _bstart
        la s1, _bend

        move t0, s0

1:
        sw zero, 0(t0)
        addi t0, 4
        bne t0, s1, 1b

        //sw sp, old_stack_pointer
// david ------------------------
//	 li sp, NEWSTACK
//sc_yang
//#ifndef BZ2_COMPRESS
//        li sp, NEWSTACK
//#else
	move t0, s1
	addi t0, t0, 4096
 	move sp, t0
 	move a0, t0
//#endif
//-------------------------------
                
        j main
        nop

        .globl exit                
exit:
        j exit
        nop

        .globl start_kernel
start_kernel:
        move t0, a0
        li a0, 0
        li a1, 0
        li a2, 0
		jr t0
       	nop 


hfclock_setting:
		// Check chip version to set divider register
		li		t1, REG_REVISION
		lw		t0, 0(t1)
		li		t1, 0xf0000000
		and		t0, t0, t1
		srl		t0, t0, 0x1c		// Right shift 28 bits
		
		beq		t0, VERSION_D, version_8186_C //set D cut keep 108MHz
		nop
		
//		bne		t0, VERSION_D, 1f
//		nop
//		j		ra			// 8186 D cut doesn't need watch dog reset.
//		nop

1:
		beq		t0, VERSION_C, version_8186_C
		nop


version_8186_B:
		// Check PLLMNR and SYSCLKR Registers.
		la   	a0, REG_PLLMNR
		lw		t0, 0(a0)
		and		t0, t0, 0x3FFFF	//You must mask other bits!!
		li   	t1, VAL_PLLMNR_8186B
		bne		t0, t1, set_B
		nop

		la   	a0, REG_SYSCLKR
		lw		t0, 0(a0)
		and		t0, t0, 0xFFF		//You must mask other bits!!
		li   	t1, VAL_SYSCLKR_8186B
		bne		t0, t1, set_B
		nop

		j		ra			//PLLMNR and SYSCLKR are set already.
		nop

set_B:
		// Set CPU/MEM Clock and Watch Dog Reset.
		li		t0, VAL_PLLMNR_8186B
		la		a0, REG_PLLMNR
		sw		t0, 0(a0)

		li		t0, VAL_SYSCLKR_8186B
		la		a0, REG_SYSCLKR
		sw		t0, 0(a0)
		
		j		start_watch_dog
		nop

version_8186_C:
		// Check PLLMNR and SYSCLKR Registers.
		la   	a0, REG_PLLMNR
		lw		t0, 0(a0)
		and		t0, t0, 0x3FFFF		//You must mask other bits!!
		li   	t1, VAL_PLLMNR_8186C
		bne		t0, t1, set_C
		nop

		la   	a0, REG_SYSCLKR
		lw		t0, 0(a0)
		and		t0, t0, 0xFFF			//You must mask other bits!!
		li   	t1, VAL_SYSCLKR_8186C
		bne		t0, t1, set_C
		nop

		j		ra
		nop

set_C:
		// Set CPU/MEM Clock and Watch Dog Reset.
		li		t0, VAL_PLLMNR_8186C
		la		a0, REG_PLLMNR
		sw		t0, 0(a0)

		li		t0, VAL_SYSCLKR_8186C
		la		a0, REG_SYSCLKR
		sw		t0, 0(a0)
		
		j		start_watch_dog
		nop

start_watch_dog:
		// start WDTDOG
		la		a0, REG_CDBR
		la		t0, 0x2
		sw		t0, 0(a0)
		la		t0, 0x100
		la		a0, REG_WDTCNR
		sw		t0, 0(a0)
1:
		b		1b
		nop


