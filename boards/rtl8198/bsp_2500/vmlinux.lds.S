#include <asm/asm-offsets.h>
#include <asm-generic/vmlinux.lds.h>

#undef mips
#define mips mips
OUTPUT_ARCH(mips)
ENTRY(kernel_entry)
PHDRS {
	text PT_LOAD FLAGS(7);	/* RWX */
	note PT_NOTE FLAGS(4);	/* R__ */
}
jiffies = JIFFIES;

SECTIONS
{
	. = LOADADDR;
	/* read-only */
	_text = .;	/* Text and read-only data */
	.text : {
		TEXT_TEXT
#ifdef CONFIG_RTK_VOIP
		. = ALIGN(8);
		*(.AllPoleFilter_fixed16s)
		*(.AllZeroFilter_fixed16s)
		*(.getCBvec_fixed16s)

		. = ALIGN(ZZ_IRAMSIZE_ZZ);
		__ILBCENC_IMEM_start = .;
		*(.iLBC_Lsp_Azs)
		*(.iLBC_Get_lsp_pols)
		*(.iLBC_crossconvs)
		*(.iLBC_Autocorrs)
		*(.iLBC_Levinsons)
		*(.iCBSearch_hbsrchlps)
		*(.iCBSearch_mainlps)
		*(.iCBSearch_hbsrchstg0s)
		*(.iCBSearch_Loop4s)
		*(.ilbcenc_oper_32bs)
		*(.iLBC_Chebps_1xs)
		*(.searchAugmentedCB_fixeds)
		*(.createAugmentedVec_fixed16s)
		*(.iCBSearch_scal_bufs)
		*(.filteredCBvecs_fixed16s)
		__ILBCENC_IMEM_end = .;

		. = ALIGN(ZZ_IRAMSIZE_ZZ);
		__ILBCDEC_IMEM_start = .;
		*(.doThePLC_fixed16s)
		*(.enhancerInterface_fixed16s)
		*(.smath_fixed16s)
		*(.ilbcdec_oper_32bs)
		*(.compCorr_fixed16s)
		*(.hpOutput_fixed16s)
		*(.xCorrCoef_fixed16s)
		*(.iLBC_Iirs)
		*(.iLBC_Syn_filts)
		*(.mycorr1_fixed16s)
		*(.mycorr2_fixed16s)
		*(.refiner_fixed16)
		*(.enh_upsample_fixed16)
		*(.iLBC_decode)
		__ILBCDEC_IMEM_end = .;

#if 0 
		/* g729 codec */
		*(.Pre_Process2s)
		*(.Cor_hs)
		*(.D4i40_17_fasts)
		*(.Corr_xy2s)
		*(.imem8K)
		*(.imem16K)
		*(.imem32K)
		*(.utility)
		*(.memset)
		*(.preemphasiss)
		*(.g729_utils)
		*(.dspfuncs)
#endif
#endif
		SCHED_TEXT
		LOCK_TEXT
		KPROBES_TEXT
		*(.text.*)
		*(.fixup)
		*(.gnu.warning)
/*#ifdef CONFIG_RTL8192SE*/
#if 1
		/* start of wlan f/w definition */
		__fw_start = . ;
		*(.rtl8192sfw.bin)
		__fw_end = . ;

		__AGC_TAB_start = . ;
		*(.AGC_TAB.txt)
		__AGC_TAB_end = . ;

		__phy_reg_start = . ;
		*(.phy_reg.txt)
		__phy_reg_end = . ;

		__phy_reg_MP_start = . ;
		*(.phy_reg_MP.txt)
		__phy_reg_MP_end = . ;

		__MACPHY_REG_start = . ;
		*(.MACPHY_REG.txt)
		__MACPHY_REG_end = . ;

		__radio_a_start = . ;
		*(.radio_a.txt)
		__radio_a_end = . ;                                                       

		__radio_b_start = . ;
		*(.radio_b.txt)
		__radio_b_end = . ;

		__PHY_REG_PG_start = .;
		*(.PHY_REG_PG.txt)
		__PHY_REG_PG_end = .;

		__PHY_to1T2R_start = .;
		*(.PHY_to1T2R.txt)
		__PHY_to1T2R_end = .;

		__PHY_to1T2R_b_start = .;
		*(.PHY_to1T2R_b.txt)
		__PHY_to1T2R_b_end = .;

		__PHY_to1T1R_start = .;
		*(.PHY_to1T1R.txt)
		__PHY_to1T1R_end = .;

		__PHY_to1T1R_b_start = .;
		*(.PHY_to1T1R_b.txt)
		__PHY_to1T1R_b_end = .;

		__radio_a_hp_start = . ;
		*(.radio_a_hp.txt)
		__radio_a_hp_end = . ;

		/* end of wlan f/w definition */
#endif

	} :text = 0
	_etext = .;	/* End of text section */

	/* Exception table */
	. = ALIGN(16);
	__ex_table : {
		__start___ex_table = .;
		*(__ex_table)
		__stop___ex_table = .;
	}

	/* Exception table for data bus errors */
	__dbe_table : {
		__start___dbe_table = .;
		*(__dbe_table)
		__stop___dbe_table = .;
	}
	/* jasonwang0413*/
   	. = ALIGN(8192*4);
   	__iram = . ;
   	.iram        :
   	{
#ifdef CONFIG_RTK_VOIP
		__imem_G729_START = .;
		*(.Cor_h_Xs)		/* G.729enc only start*/
		*(.g729_crossconvs)
		*(.Chebps_1xs)		/* G.729enc only end*/
		
		*(.agcs)		/* G.729dec only start*/
		*(.preemphasiss)	/* G.729dec only end*/
		
		*(.g729_crosscorrs)	/* G.729 comm start*/
		*(.Pred_lt_3s)
		*(.filters)		/* G.729 comm end*/

#if 1
		/* g729 codec */
		*(.Pre_Process2s)
		*(.Cor_hs)
		*(.D4i40_17_fasts)
		*(.Corr_xy2s)
		*(.imem8K)
		*(.imem16K)
		*(.imem32K)
		*(.utility)
		*(.memset)
		*(.preemphasiss)
		*(.g729_utils)
		*(.dspfuncs)
#endif
#if 1
		*(.Decod_ACELPs)
		*(.Decod_ld8a_sub1s)
		*(.Decod_ld8a_sub2s)
		*(.bit2parm)
		*(.Post_Processs)
		*(.Post_Filters)
		*(.Int_qlpcs)
#endif
		__imem_G729_END = .;
		
		__imem_common_start = .;
		*(.basic_ops)
		*(.oper_32bs)
		__imem_common_end = .;

#if 0
		__imem_LEC_START = .;
		*(.lec_fir_asm)
		*(.lec_lec_adapt_asm)
		*(.lec_lec_process)
		__imem_LEC_END = .;
#endif

#if 0
		__imem_ISR_START = .;
		*(.bus_pcm_isr)
		*(.bus_iis_isr)
		__imem_ISR_END = .;
#endif

#if 0
		*(.dtmf_dec)
		*(.dtmf_dec1)
		*(.fsk_dec)
		*(.fsk_dec1)
#endif
		
		__imem_G7231_START = .;
		*(.crossconv2s)		/* G.723enc only start */
		*(.crossconvs)
		*(.convols)
		*(.EvalPolys)		/* G.723enc only end */
		
		*(.Comp_Ens)		/* G.723dec only */
		
		*(.Get_Rezs)
		*(.Sqrt_lbcs)
		*(.Vec_Norms)
		*(.crosscorrs)
		__imem_G7231_END = .;
		
#endif
		*(.iram-gen)		/* general usage (essential) */
		*(.iram-fwd)            /* general packet forwarding used */
		*(.iram-rtkwlan)	/* realtek wlan driver */
		*(.iram-l2-fwd)         /* L2 packet forwarding */
		*(.iram-l34-fwd)        /* L34 packet forwarding */
		*(.iram-tx)		/* protocol stack TX */
		*(.iram-extdev)		/* ROMEDRV extension device fwd */
		*(.iram-crypto)		/* authetication / crypto-engine */
		*(.iram-rtkusb)		/* usb */	
		*(.iram-voip)		/* voip */
		*(.iram)		/* other usage */
		*(.iram.1)
		__iram_end = .;		/* help programmer know the iram size */
   	}


  
   	. = ALIGN(8192);
   	__dram = . ;
   	__dram_start = . ;
#ifdef CONFIG_RTK_VOIP
	__dram0_start = . ;
	. = . + 4k;	/* this 4k is placed on DMEM0 for voip use */
	__dram0_end = . ;
	__dram1_start = . ;
	__dram_voip_data_start = .;
	.dram_voip_data :	/* we will copy external memory data to dram in this section */
	{
#if 0 
		*(.con_desc_data)
		*(.snd_desc_data) 
		*(.bus_desc_data)
		*(.dsp_desc_data)
		*(.gpio_dmem_data)
		*(.v21_tab_data)
		*(.rtk_debug_data) 
#endif
	}
	__dram_voip_data_end = .;
	__dram_voip_bss_start = .;
	.dram_voip_bss :	/* we will clean this section */
	{
#if 0
		*(.con_desc_bss)
		*(.snd_desc_bss)
		*(.bus_desc_bss)
		*(.dsp_desc_bss)
		*(.spi_dmem_data)
		*(.fsk_det_bss)
		*(.agc_bss)
#endif
	}
	__dram_voip_bss_end = .;
#endif
   	.dram	:
   	{
		*(.dram-wapi)           /*wapi encryption/decryption used*/
   		*(.dram-gen)		/* general usage (essential) */
		*(.dram-fwd)            /* general packet forwarding used */
   		*(.dram-l2-fwd)		/* L2 packet forwarding */
		*(.dram-l34-fwd)        /* L34 packet forwarding */
   		*(.dram-extdev)		/* ROMEDRV extension device fwd */
   		*(.dram-rtkwlan)	/* realtek wlan driver */
		*(.dram-crypto)		/* authetication / crypto-engine */
   		*(.dram-voip)		/* voip */
		*(.dram-tx)		/* protocol stack TX */
		*(.dram)		/* other usage */
		*(.dram.1)
   	}
   	. = ALIGN(8192);
#ifdef CONFIG_RTK_VOIP
	__dram1_end = . ;
#endif
   	__dram_end = . ;

	NOTES :text :note
	.dummy : { *(.dummy) } :text

	RODATA

	/* writeable */
	.data : {	/* Data */
		. = . + DATAOFFSET;		/* for CONFIG_MAPPED_KERNEL */
		/*
		 * This ALIGN is needed as a workaround for a bug a
		 * gcc bug upto 4.1 which limits the maximum alignment
		 * to at most 32kB and results in the following
		 * warning:
		 *
		 *  CC      arch/mips/kernel/init_task.o
		 * arch/mips/kernel/init_task.c:30: warning: alignment
		 * of ?˜init_thread_union??is greater than maximum
		 * object file alignment.  Using 32768
		 */
		. = ALIGN(_PAGE_SIZE);
		*(.data.init_task)

		DATA_DATA
		CONSTRUCTORS
#ifdef CONFIG_RTK_VOIP
		.  = ALIGN(8);
		*(.g7231_dmem_bss.1)
#endif
	}
	_gp = . + 0x8000;
	.lit8 : {
		*(.lit8)
	}
	.lit4 : {
		*(.lit4)
	}
	/* We want the small data sections together, so single-instruction offsets
	   can access them all, and initialized data all before uninitialized, so
	   we can shorten the on-disk segment size.  */
	.sdata : {
		*(.sdata)
	}

	. = ALIGN(_PAGE_SIZE);
	.data_nosave : {
		__nosave_begin = .;
		*(.data.nosave)
	}
	. = ALIGN(_PAGE_SIZE);
	__nosave_end = .;

	. = ALIGN(1 << 5);
	.data.cacheline_aligned : {
		*(.data.cacheline_aligned)
	}
	_edata =  .;			/* End of data section */

	/* will be freed after init */
	. = ALIGN(_PAGE_SIZE);		/* Init code and data */
	__init_begin = .;
	.init.text : {
		_sinittext = .;
		INIT_TEXT
		_einittext = .;
	}
	.init.data : {
		INIT_DATA
	}
	. = ALIGN(16);
	.init.setup : {
		__setup_start = .;
		*(.init.setup)
		__setup_end = .;
	}

	.initcall.init : {
		__initcall_start = .;
		INITCALLS
#ifdef CONFIG_RTK_VOIP
        *(.initcall98.init) 
		*(.initcall98a.init) *(.initcall98b.init) *(.initcall98c.init) *(.initcall98d.init) *(.initcall98e.init) 
		*(.initcall98f.init) *(.initcall98g.init) *(.initcall98h.init) *(.initcall98i.init) *(.initcall98j.init)
		*(.initcall99.init) 
		*(.initcall99a.init) *(.initcall99b.init) *(.initcall99c.init) *(.initcall99d.init) *(.initcall99e.init) 
		*(.initcall99f.init) *(.initcall99g.init)
		*(.initcall99s.init)
#endif
		__initcall_end = .;
	}

	.con_initcall.init : {
		__con_initcall_start = .;
		*(.con_initcall.init)
		__con_initcall_end = .;
	}
	SECURITY_INIT

	/* .exit.text is discarded at runtime, not link time, to deal with
	 * references from .rodata
	 */
	.exit.text : {
		EXIT_TEXT
	}
	.exit.data : {
		EXIT_DATA
	}
#if defined(CONFIG_BLK_DEV_INITRD)
	. = ALIGN(_PAGE_SIZE);
	.init.ramfs : {
		__initramfs_start = .;
		*(.init.ramfs)
		__initramfs_end = .;
	}
#endif
	PERCPU(_PAGE_SIZE)
	. = ALIGN(_PAGE_SIZE);
	__init_end = .;
	/* freed after init ends here */

#ifdef CONFIG_RTK_VOIP
  /* VoIP DMEM start */
  	. = . + 2k; /* reserve 2k avoid voip stack grow to here. */
   	/*. = ALIGN( MAX(8k, ZZ_DRAMSIZE_ZZ) );*/
	__dmem_start = .;
	/*__gsmfr_dmem_start = .;*/
	/* rtk_voip start*/
	/*.DMEM_GSMFR	:
							{
	*/							/* *(.gsmfr_dummy_topstack) */
	/*						. = . + 4k;
							. = ALIGN(4k);
								*(.gsmfr_dmem_stack1K)
								. = ALIGN(8);
								*(.gsmfr_dmem_bss)
								__gsmfr_dmem_end = .;
							}
	*/
	. = ALIGN( MAX(8k, ZZ_DRAMSIZE_ZZ) );
	__g729_dmem_start = .;
	.DMEM_G729	:
							{
								/* *(.g729_dummy_topstack) */
    							. = . + 4k;
    							. = ALIGN(4k);
								*(.g729_dmem_stack1K)
								. = ALIGN(8);
								*(.g729_dmem_bss.0)
								*(.g729_dmem_bss.1)
								__g729_dmem_end = .;
							}

	/*. = ALIGN( MAX(8k, ZZ_DRAMSIZE_ZZ) );
	__g726_dmem_start = .;
	.DMEM_G726	:
							{
	*/							/* *(.g726_dummy_topstack) */
	/*						. = . + 4k;
							. = ALIGN(4k);
								*(.g726_dmem_stack1K)
								*(.dmem_g726)
								__g726_dmem_end = .;
							}
	*/

	. = ALIGN( MAX(8k, ZZ_DRAMSIZE_ZZ) );
	__lec_dmem_start = .;
	.DMEM_LEC	:
							{
							. = . + 4k;
							. = ALIGN(4k);
								/* *(.lec_dummy_topstack) */
								*(.lec_dmem_stack1K)
								. = ALIGN(8);
								*(.dmem_lec)
								__lec_dmem_end = .;
							}
	. = ALIGN( MAX(8k, ZZ_DRAMSIZE_ZZ) );
	__sys_dmem_start = .;
	.DMEM_SYS	:
							{
							. = . + 4k;
							. = ALIGN(4k);
								/* *(.sys_dummy_topstack) */
								*(.sys_dmem_stack)
								__sys_dmem_end = .;
							}
	/* . = ALIGN(ZZ_DRAMSIZE_ZZ);*/
	. = ALIGN( MAX(8k, ZZ_DRAMSIZE_ZZ) );
	__codec_dmem_start = .;
	.DMEM_CODEC	:
							{
							. = . + 4k;
							. = ALIGN( MIN(4k, ZZ_DRAMSIZE_ZZ));
							__codec_dmem_4k_start = .;
								
								*(.codec_dmem_area)
								__codec_dmem_end = .;
   }
  
	. = ALIGN( 4k );
	__g7231_dmem_start = .;
	.DMEM_G7231	__codec_dmem_4k_start : AT (__g7231_dmem_start)
							{
								*(.g7231_dmem_stack1K)
								. = ALIGN(8);
								*(.g7231_dmem_bss.0)
								__g7231_dmem_end = .;
							}
	. = ADDR(.DMEM_CODEC) + MAX(8k, ZZ_DRAMSIZE_ZZ) + 4k; /* skip overlays address*/

	. = . + 6k; /* reserve 6k avoid ilbc iLBCTmpVct grow here. */

  	__voip_dmem_end = .;
  /* VoIP DMEM end */
#endif

	__bss_start = .;	/* BSS */
	.sbss  : {
		*(.sbss)
		*(.scommon)
	}
	.bss : {
		*(.bss)
		*(COMMON)
#ifdef CONFIG_RTK_VOIP
		.  = ALIGN(8);

		*(.g7231_dmem_bss.1)
#endif
	}
	__bss_stop = .;

	_end = . ;

	/* Sections to be discarded */
	/DISCARD/ : {
		*(.exitcall.exit)

		/* ABI crap starts here */
		*(.MIPS.options)
		*(.options)
		*(.pdr)
		*(.reginfo)
	}

	/* These mark the ABI of the kernel for debuggers.  */
	.mdebug.abi32 : {
		KEEP(*(.mdebug.abi32))
	}
	.mdebug.abi64 : {
		KEEP(*(.mdebug.abi64))
	}

	/* This is the MIPS specific mdebug section.  */
	.mdebug : {
		*(.mdebug)
	}

	STABS_DEBUG
	DWARF_DEBUG

	/* These must appear regardless of  .  */
	.gptab.sdata : {
		*(.gptab.data)
		*(.gptab.sdata)
	}
	.gptab.sbss : {
		*(.gptab.bss)
		*(.gptab.sbss)
	}
}
