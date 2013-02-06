#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1994, 1995, 1996 by Ralf Baechle
# DECStation modifications by Paul M. Antoine, 1996
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies. Remember to do have actions
# for "archclean" and "archdep" for cleaning up and making dependencies for
# this architecture
#

#
# Select the object file format to substitute into the linker script.
#
ifdef CONFIG_CPU_LITTLE_ENDIAN
tool-prefix	= mipsel-linux-
else
tool-prefix	= mips-linux-
endif

ifdef CONFIG_CROSSCOMPILE
CROSS_COMPILE  = $(tool-prefix)
endif

#
# GCC uses -G0 -mabicalls -fpic as default.  We don't want PIC in the kernel
# code since it only slows down the whole thing.  At some point we might make
# use of global pointer optimizations but their use of $28 conflicts with
# the current pointer optimization.
#
# The DECStation requires an ECOFF kernel for remote booting, other MIPS
# machines may also.  Since BFD is incredibly buggy with respect to
# crossformat linking we rely on the elf2ecoff tool for format conversion.
#
GCCFLAGS	:= -G 0 -mno-abicalls -fno-pic
LINKFLAGS	+= -static
MODFLAGS	+= -mlong-calls
LD		:= $(LD) -G 0

ifdef CONFIG_REMOTE_DEBUG
CFLAGS		:= $(CFLAGS) -g
endif

#
# CPU-dependent compiler/assembler options for optimization.
#
ifdef CONFIG_CPU_R3000
ifdef CONFIG_RTL_EB8186
GCCFLAGS	+= -DR3000 -mips1
endif
ifdef CONFIG_RTL865X
GCCFLAGS	+= -DR3000
endif
ifdef CONFIG_RTL8196B
GCCFLAGS	+= -DR3000
endif
endif
ifdef CONFIG_CPU_TX39XX
GCCFLAGS	+= -mcpu=r3000 -mips1
endif
ifdef CONFIG_CPU_R6000
GCCFLAGS	+= -mcpu=r6000 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_R4300
GCCFLAGS	+= -mcpu=r4300 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_VR41XX
GCCFLAGS	+= -mcpu=r4600 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_R4X00
GCCFLAGS	+= -mcpu=r4600 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_TX49XX
GCCFLAGS	+= -mcpu=r4600 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_MIPS32
GCCFLAGS	+= -mcpu=r4600 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_MIPS64
GCCFLAGS	+= -mcpu=r4600 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_R5000
GCCFLAGS	+= -mcpu=r5000 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_R5432
GCCFLAGS        += -mcpu=r5000 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_NEVADA
# Cannot use -mmad with currently recommended tools
GCCFLAGS	+= -mcpu=r5000 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_RM7000
GCCFLAGS	+= -mcpu=r5000 -mips2 -Wa,--trap
endif
ifdef CONFIG_CPU_SB1
GCCFLAGS	+= -mcpu=sb1 -mips2 -Wa,--trap
ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
MODFLAGS       += -msb1-pass1-workarounds
endif
endif

GCCFLAGS	+= -pipe

CFLAGS		:= -I $(TOPDIR)/include/asm/gcc $(CFLAGS) $(GCCFLAGS)
AFLAGS		+= $(GCCFLAGS)
ASFLAGS		+= $(GCCFLAGS)

#
# We unconditionally build the math emulator
#
ifdef CONFIG_FPU_EMU
CORE_FILES	+=arch/mips/math-emu/fpu_emulator.o
SUBDIRS		+=arch/mips/math-emu
endif

#
# ramdisk/initrd support
# You need a compressed ramdisk image, named ramdisk.gz in
# arch/mips/ramdisk
#
ifdef CONFIG_EMBEDDED_RAMDISK
CORE_FILES	+= arch/mips/ramdisk/ramdisk.o
SUBDIRS		+= arch/mips/ramdisk
endif

#
# Board-dependent options and extra files
#
ifdef CONFIG_ALGOR_P4032
CORE_FILES    += arch/mips/algor/algor.o
SUBDIRS       += arch/mips/algor
LOADADDR      += 0x80000000
endif

#
# DECstation family
#
ifdef CONFIG_DECSTATION
CORE_FILES    += arch/mips/dec/dec.o
SUBDIRS       += arch/mips/dec arch/mips/dec/prom
LIBS          += arch/mips/dec/prom/rexlib.a
LOADADDR      += 0x80040000
endif

ifdef CONFIG_MIPS_ATLAS
LIBS          += arch/mips/mips-boards/atlas/atlas.o arch/mips/mips-boards/generic/mipsboards.o
SUBDIRS       += arch/mips/mips-boards/generic arch/mips/mips-boards/atlas
LOADADDR      += 0x80100000
endif

ifdef CONFIG_MIPS_MALTA
LIBS          += arch/mips/mips-boards/malta/malta.o arch/mips/mips-boards/generic/mipsboards.o
SUBDIRS       += arch/mips/mips-boards/malta arch/mips/mips-boards/generic
LOADADDR      += 0x80100000
endif

#
# Acer PICA 61, Mips Magnum 4000 and Olivetti M700.
#
ifdef CONFIG_MIPS_JAZZ
CORE_FILES    += arch/mips/jazz/jazz.o
SUBDIRS       += arch/mips/jazz arch/mips/arc
LIBS          += arch/mips/arc/arclib.a
LOADADDR      += 0x80080000
endif

ifdef CONFIG_MIPS_COBALT
SUBDIRS       += arch/mips/cobalt
CORE_FILES    += arch/mips/cobalt/cobalt.o
LOADADDR      += 0x80080000
endif

ifdef CONFIG_SNI_RM200_PCI
CORE_FILES    += arch/mips/sni/sni.o
SUBDIRS       += arch/mips/sni arch/mips/arc
LIBS          += arch/mips/arc/arclib.a
LOADADDR      += 0x80080000
endif

ifdef CONFIG_SGI_IP22
CORE_FILES    += arch/mips/sgi-ip22/ip22-kern.o
LIBS          += arch/mips/arc/arclib.a
SUBDIRS       += arch/mips/sgi-ip22 arch/mips/arc
#
# Set LOADADDR to >= 0x88069000 if you want to leave space for symmon,
# 0x88002000 for production kernels.  Note that the value must be
# 8kb aligned or the handling of the current variable will break.
#
LOADADDR      += 0x88002000
endif

#
# Baget/MIPS
#
ifdef CONFIG_BAGET_MIPS
SUBDIRS       += arch/mips/baget arch/mips/baget/prom
LIBS          += arch/mips/baget/baget.a arch/mips/baget/prom/bagetlib.a
LOADADDR      += 0x80001000
endif

#
# NEC DDB Vrc-5074
#
ifdef CONFIG_DDB5074
SUBDIRS       += arch/mips/ddb5074
LIBS          += arch/mips/ddb5074/ddb5074.a
LOADADDR      += 0x80080000
endif

#
#
# NEC DDB Vrc-5476
#
ifdef CONFIG_DDB5476
SUBDIRS       += arch/mips/ddb5xxx/common arch/mips/ddb5xxx/ddb5476
LIBS          += arch/mips/ddb5xxx/common/ddb5xxx.o \
		 arch/mips/ddb5xxx/ddb5476/ddb5476.o
LOADADDR      += 0x80080000
endif

#
#
# NEC DDB Vrc-5477
#
ifdef CONFIG_DDB5477
SUBDIRS       += arch/mips/ddb5xxx/common arch/mips/ddb5xxx/ddb5477
LIBS          += arch/mips/ddb5xxx/common/ddb5xxx.o \
                 arch/mips/ddb5xxx/ddb5477/ddb5477.o
LOADADDR      += 0x80080000
endif

#
# NEC Osprey (vr4181) board
#
ifdef CONFIG_NEC_OSPREY
SUBDIRS       += arch/mips/vr4181/common arch/mips/vr4181/osprey
LIBS          += arch/mips/vr4181/common/vr4181.o \
                 arch/mips/vr4181/osprey/osprey.o
LOADADDR      += 0x80002000
endif

#
# Galileo EV64120 Board
#
ifdef CONFIG_MIPS_EV64120
LIBS          += arch/mips/galileo-boards/ev64120/ev64120.o
SUBDIRS       += arch/mips/galileo-boards/ev64120
LOADADDR      += 0x80100000
endif

#
# Galileo EV96100 Board
#
ifdef CONFIG_MIPS_EV96100
LIBS          += arch/mips/galileo-boards/ev96100/ev96100.o arch/mips/galileo-boards/generic/galboards.o
SUBDIRS       += arch/mips/galileo-boards/generic arch/mips/galileo-boards/ev96100
LOADADDR      += 0x80100000
endif

#
# Momentum Ocelot board
#
ifdef CONFIG_MOMENCO_OCELOT
# The Ocelot setup.o must be linked early - it does the ioremap() for the
# mips_io_port_base.
CORE_FILES    += arch/mips/gt64120/common/gt64120.o arch/mips/gt64120/momenco_ocelot/momenco_ocelot.o
SUBDIRS       += arch/mips/gt64120/common arch/mips/gt64120/momenco_ocelot
LOADADDR      += 0x80100000
endif

#
# Philips Nino
#
ifdef CONFIG_NINO
CORE_FILES    += arch/mips/philips/nino/nino.o
SUBDIRS       += arch/mips/philips/nino
LOADADDR      += 0x80000000
endif

#
# Realtek 865x
#
ifdef CONFIG_RTL865X
CORE_FILES    += arch/mips/philips/nino/nino.o
SUBDIRS       += arch/mips/philips/nino
#LOADADDR      := 0x80080000
LOADADDR      := 0x80000000
endif

#
# Realtek 8196B
#
ifdef CONFIG_RTL8196B
ifdef CONFIG_RTL8198
CORE_FILES    += arch/mips/realtek/rtl8198/rtl8196b.o
SUBDIRS       += arch/mips/realtek/rtl8198
LOADADDR      := 0x80000000
else
CORE_FILES    += arch/mips/realtek/rtl8196b/rtl8196b.o
SUBDIRS       += arch/mips/realtek/rtl8196b
LOADADDR      := 0x80000000
endif
endif

#
# ITE 8172 eval board with QED 5231 CPU
#
ifdef CONFIG_MIPS_ITE8172
LIBS          += arch/mips/ite-boards/qed-4n-s01b/ite.o arch/mips/ite-boards/generic/it8172.o
SUBDIRS       += arch/mips/ite-boards/generic arch/mips/ite-boards/qed-4n-s01b
LOADADDR      += 0x80100000
endif

#
# Globespan IVR eval board with QED 5231 CPU
#
ifdef CONFIG_MIPS_IVR
LIBS          += arch/mips/ite-boards/ivr/ivr.o arch/mips/ite-boards/generic/it8172.o
SUBDIRS       += arch/mips/ite-boards/generic arch/mips/ite-boards/ivr
LOADADDR      += 0x80100000
endif

#
# Au1000 eval board
#
ifdef CONFIG_MIPS_PB1000
LIBS          += arch/mips/au1000/pb1000/pb1000.o \
                 arch/mips/au1000/common/au1000.o
SUBDIRS       += arch/mips/au1000/pb1000 arch/mips/au1000/common
LOADADDR      += 0x80100000
endif

ifdef CONFIG_MIPS_PB1500
LIBS          += arch/mips/au1000/pb1500/pb1500.o \
                 arch/mips/au1000/common/au1000.o
SUBDIRS       += arch/mips/au1000/pb1500 arch/mips/au1000/common
LOADADDR      += 0x80100000
endif

#
# Sibyte SB1250 SOC
#
ifdef CONFIG_SIBYTE_SB1250
# This is a LIB so that it links at the end, and initcalls are later
# the sequence; but it is built as an object so that modules don't get
# removed (as happens, even if they have __initcall/module_init)
LIBS          += arch/mips/sibyte/sb1250/sb1250.o
SUBDIRS       += arch/mips/sibyte/sb1250 
endif

#
# Sibyte SWARM board
#
ifdef CONFIG_SB1_CACHE_ERROR
LIBS          += arch/mips/sibyte/sb1/sb1kern.a
SUBDIRS       += arch/mips/sibyte/sb1
endif

ifdef CONFIG_SIBYTE_SWARM
LIBS          += arch/mips/sibyte/swarm/sbswarm.a
SUBDIRS       += arch/mips/sibyte/swarm
LOADADDR      += 0x80100000
endif

#
# HP LaserJet
#
ifdef CONFIG_HP_LASERJET
SUBDIRS       += arch/mips/hp-lj
LIBS          += arch/mips/hp-lj/hp-lj.o
LOADADDR      := 0x80030000
endif

ifdef CONFIG_TOSHIBA_JMR3927
CORE_FILES    += arch/mips/jmr3927/rbhma3100/jmr3927.o arch/mips/jmr3927/common/tx3927.o
SUBDIRS       += arch/mips/jmr3927/rbhma3100  arch/mips/jmr3927/common
LOADADDR      += 0x80050000 
endif                                                        
STACK_SIZE = $(CONFIG_STACK_SIZE)
#
# Choosing incompatible machines durings configuration will result in
# error messages during linking.  Select a default linkscript if
# none has been choosen above.
#
vmlinux: arch/$(ARCH)/ld.script

ifdef CONFIG_RTL_EB8186
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.in arch/$(ARCH)/Makefile
	sed -e 's/@@LOADADDR@@/$(LOADADDR)/' <$< >$@
endif

ifdef CONFIG_RTL865X
ifdef CONFIG_RTK_VOIP
ifdef CONFIG_AUDIOCODES_VOIP
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.voip.ac.in arch/$(ARCH)/Makefile
else
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.voip.in arch/$(ARCH)/Makefile
endif	
else # !CONFIG_RTK_VOIP
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.in.865x arch/$(ARCH)/Makefile include/linux/autoconf.h
endif # CONFIG_RTK_VOIP
	sed -e 's/@@LOADADDR@@/$(LOADADDR)/' -e 's/@@IRAMSIZE@@/16384/' -e 's/@@DRAMSIZE@@/8192/' -e 's/@@STACKSIZE@@/$(STACK_SIZE)/' <$< >$@
endif

ifdef CONFIG_RTL8196B
ifdef CONFIG_RTL8196C
ifdef CONFIG_RTL8198
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.in.8198 arch/$(ARCH)/Makefile include/linux/autoconf.h
else
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.in.8196c arch/$(ARCH)/Makefile include/linux/autoconf.h
endif
else
ifdef CONFIG_RTK_VOIP
ifdef CONFIG_AUDIOCODES_VOIP
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.voip.ac.in arch/$(ARCH)/Makefile
else
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.voip.in arch/$(ARCH)/Makefile
endif	
else # !CONFIG_RTK_VOIP
arch/$(ARCH)/ld.script: arch/$(ARCH)/ld.script.in.8196b arch/$(ARCH)/Makefile include/linux/autoconf.h
endif # CONFIG_RTK_VOIP
endif
ifdef CONFIG_RTL8198
	sed -e 's/@@LOADADDR@@/$(LOADADDR)/' -e 's/@@IRAMSIZE@@/8192/' -e 's/@@DRAMSIZE@@/8192/' -e 's/@@STACKSIZE@@/$(STACK_SIZE)/' <$< >$@
else
	sed -e 's/@@LOADADDR@@/$(LOADADDR)/' -e 's/@@IRAMSIZE@@/16384/' -e 's/@@DRAMSIZE@@/8192/' -e 's/@@STACKSIZE@@/$(STACK_SIZE)/' <$< >$@
endif
endif

LINKFLAGS	+= -T arch/$(ARCH)/ld.script

HEAD := arch/mips/kernel/head.o arch/mips/kernel/init_task.o

SUBDIRS := $(addprefix arch/mips/, tools) $(SUBDIRS) $(addprefix arch/mips/, kernel mm lib)
CORE_FILES := arch/mips/kernel/kernel.o arch/mips/mm/mm.o $(CORE_FILES)
LIBS := arch/mips/lib/lib.a $(LIBS)

ifdef CONFIG_BAGET_MIPS

BAGETBOOT = $(MAKE) -C arch/$(ARCH)/baget

balo:  vmlinux
	$(BAGETBOOT) balo

endif

ifdef CONFIG_MIPS_EV64120
GALILEOBOOT = $(MAKE) -C arch/$(ARCH)/galileo-boards/ev64120

gboot: vmlinux
	$(MAKE) -C arch/$(ARCH)/galileo-boards/ev64120/compressed 
endif

MAKEBOOT = $(MAKE) -C arch/$(ARCH)/boot

vmlinux.ecoff: vmlinux
	@$(MAKEBOOT) $@

archclean:
	@$(MAKEBOOT) clean
	rm -f arch/$(ARCH)/ld.script
	$(MAKE) -C arch/$(ARCH)/tools clean

archmrproper:
	@$(MAKEBOOT) mrproper
	$(MAKE) -C arch/$(ARCH)/tools mrproper

archdep:
	if [ ! -f $(TOPDIR)/include/asm-$(ARCH)/offset.h ]; then \
	 touch $(TOPDIR)/include/asm-$(ARCH)/offset.h; \
	fi;
	@$(MAKEBOOT) dep
