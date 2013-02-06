/* $Id: head.S,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * arch/mips/kernel/head.S
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995 Waldorf Electronics
 * Written by Ralf Baechle and Andreas Busse
 * Copyright (C) 1995 - 1999 Ralf Baechle
 * Copyright (C) 1996 Paul M. Antoine
 * Modified for DECStation and hence R3000 support by Paul M. Antoine
 * Further modifications by David S. Miller and Harald Koerfgen
 * Copyright (C) 1999 Silicon Graphics, Inc.
 *
 * Head.S contains the MIPS exception handler and startup code.
 */

#include <linux/config.h>
/*Cyrus Tsai*/
#include <linux/init.h>
/*Cyrus Tsai*/

#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/cacheops.h>
#include <asm/current.h>
#include <asm/offset.h>
#include <asm/processor.h>
#include <asm/regdef.h>
#include <asm/cachectl.h>
#include <asm/mipsregs.h>
#include <asm/lexraregs.h>
#include <asm/stackframe.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#ifdef CONFIG_PM_SUSPEND_WAKEUP
#include <asm/power.h>
#endif

__INIT
	.text

	b check_self_exist
	//b after_move

		
	/*
	 * Reserved space for exception handlers.
	 * Necessary for machines which link their kernels at KSEG0.
	 * FIXME: Use the initcode feature to get rid of unused handler
	 * variants.
	 */
	.fill	0x400



//-------------------------------------------------------------------------
	NESTED(start, 16, sp)
	.set	noreorder
	/* The following two symbols are used for kernel profiling. */
	EXPORT(stext)
	EXPORT(_stext)

check_self_exist:
//#include "weiboot.s"
//#include "uart.s"
after_move:


	//add initial code from 8672 ADSL project
	//---------------------------initial status & cause
	mtc0	zero,CP0_STATUS 	# load status register 
	mfc0	t1,CP0_STATUS     
	nop                
	srl	t1,t1,0x2      
	sll	t1,t1,0x2     	# clear KUc and IEc
	mtc0	t1,CP0_STATUS   # set status register
	//--------------------------------------------
	mfc0	t1,CP0_CAUSE      
	mtc0	zero,CP0_CAUSE   # clear cause register


	
	jal		_rom_flush_cache             
	nop        


	/*----------------------------- flush the write buffer*/       
	jal		_rom_wbflush                 
	nop                                  
		
	//-----------------------------------------setting sp
	la	$28, init_task_union                              
	addiu	t0, $28, KERNEL_STACK_SIZE-32                 
	subu	sp, t0, 4*SZREG                      
	sw	t0, kernelsp                                      

 	#--- initialize and start COP3             
	mfc0	$4,CP0_STATUS                     
	nop                                
	or	$4,0x80000000              
	mtc0	$4,CP0_STATUS                     
	nop                                
	nop     
                           
 #if 0  //close DMEM, kernel will open it.
	# set kernelsp into DRAM                                                      
	addiu	t1, t0, 0x0FFF                                                
	and		t1, 0x0ffff000    # translate to physical address             
	subu	t0, t1, 4096                                                  
	subu	t1, t1, 1	                                                  
	mtc3 	t0, $4      # $0: d-ram base                                  
	nop                                                                   
	nop                                                                   
	mtc3 	t1, $5      # $1: d-ram top                                   
	nop                                                                   
	nop   

#endif

	
		
	//-----------------------------------------clear bss
	/* The firmware/bootloader passes argc/argp/envp                   
	 * to us as arguments.  But clear bss first because                
	 * the romvec and other important info is stored there             
	 * by prom_init().                                                 
	 */                                                                
	la	t0, _edata  	//la	t0, _fbss                                                   
	sw	zero, (t0)                                                     
	la	t1, (_end - 4)                                                 
1:                                                                         
	addiu	t0, 4                                                      
	bne	t0, t1, 1b                                                     
	sw	zero, (t0)                                                     
                                                                           
		                                                                   
	jal	init_arch                                                      
	nop  
	nop
	
//==========================================

	END(start)

//-------------------------------------------------------------------------
//==============================================================================
#if 1 
		.text
		LEAF(_rom_flush_cache)
		.set noreorder
	/*--- transition to kseg1 from undetermined kernel segment*/
	la	$9,2f
	or	$9,0xa0000000
	jr	$9

	.set	noreorder


2:	
	#--- initialize and start COP3
	mfc0	$8,$12
	nop
	nop
	or		$8,0x80000000
	mtc0	$8,$12
	nop
	nop
	
	#--- invalidate the IRAM with a 0->1 transition
	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000020 # IRAM Off
	mtc0	$8, $20
	nop
	nop
	
	#--- invalidate the icache and dcache with a 0->1 transition
	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000202 # Invalid ICACHE and DCACHE
	mtc0	$8, $20
	nop
	nop
#ifdef DDR_SDRAM #ddr cali need iram
	#--- load iram base and top
	la		$8,__iram	
	//la		$8,0x0ffff000
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,CP3_IWBASE								# IW bas
	nop
	nop
#ifdef CONFIG_RTL8196B
	addiu	$8,$8,0x3fff
#elif defined(CONFIG_RTL8198)
	addiu	$8,$8,0xfff
#else
#error "no config"
#endif
	mtc3	$8,CP3_IWTOP								# IW top
	nop
	nop
#endif
	#--- Refill the IRAM with a 0->1 transition
	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000010 # IRAM Fill
	mtc0	$8, $20
	nop
	nop
#if 0	
	#--- load dram base and top
#	//la		$8,__dram
#ifdef CONFIG_DEFAULTS_REALTEK_RTL8196B
	la		$8,0x90000000
#else
	la		$8,0x0ffff000
#endif
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,CP3_DWBASE								# DW bas
	nop
	nop
	addiu	$8,$8,0x0fff
	mtc3	$8,CP3_DWTOP								# DW top
	nop
	nop
#endif
	#--- enable icache and dcache
	mtc0	$0, $20	# CCTL
	nop
	nop

	.set	reorder
	j		$31
	nop


	END(_rom_flush_cache)
//--------------------------------------------------------------------------------------------


	LEAF(_rom_wbflush)
_rom_wbflush:
	li		$8, 0xa0000000				#/* load uncached address	*/
	lw		$8, 0($8)					#/* read in order to flush 	*/
	j		$31							#/* return to caller		*/
	nop
	END(_rom_wbflush)


#endif



//-------------------------------------------------------------------------
	/* General exception vector. */
		/* TLB refill, EXL == 0, R[23]00 version */

	.set	at
	.set	reorder	   
	/* General exception vector.*/
	/*Cyrus Tsai*/
	/*This is the direction of exception toward their handlers*/
		NESTED(exception_matrix, 0, sp)

		mfc0	k1, CP0_CAUSE	
		nop
		nop
		la	k0, exception_handlers
		andi	k1, k1, 0x7c
		addu	k0, k0, k1
		lw	k0, (k0)
		jr	k0
		nop
		nop
		END(exception_matrix)
/*Cyrus Tsai*/




/*
 * This buffer is reserved for the use of the cache error handler.
 */

__FINIT
