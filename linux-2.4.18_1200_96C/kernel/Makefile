#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

O_TARGET := kernel.o

export-objs = signal.o sys.o kmod.o context.o ksyms.o pm.o exec_domain.o printk.o

ifdef CONFIG_RTL_KERNEL_MIPS16_KERNEL
obj-y     = sched.o16 dma.o16 fork.o16 exec_domain.o16 panic.o16 printk.o16 \
	    module.o16 exit.o16 itimer.o16 resource.o16 sysctl.o16 acct.o16 \
	    capability.o16 ptrace.o16 user.o16 signal.o16 \
	    timer.o kmod.o context.o

ifeq ($(CONFIG_RTL8196C),y)
obj-y += info.o time.o softirq.o sys.o
else
obj-y += info.o16 time.o16 softirq.o16 sys.o16
endif	    

else
obj-y     = sched.o dma.o fork.o exec_domain.o panic.o printk.o \
	    module.o exit.o itimer.o info.o time.o softirq.o resource.o \
	    sysctl.o acct.o capability.o ptrace.o timer.o user.o \
	    signal.o sys.o kmod.o context.o
endif

obj-$(CONFIG_UID16) += uid16.o
obj-$(CONFIG_MODULES) += ksyms.o
obj-$(CONFIG_PM) += pm.o

ifneq ($(CONFIG_IA64),y)
# According to Alan Modra <alan@linuxcare.com.au>, the -fno-omit-frame-pointer is
# needed for x86 only.  Why this used to be enabled for all architectures is beyond
# me.  I suspect most platforms don't need this, but until we know that for sure
# I turn this off for IA-64 only.  Andreas Schwab says it's also needed on m68k
# to get a correct value for the wait-channel (WCHAN in ps). --davidm
CFLAGS_sched.o := $(PROFILING) -fno-omit-frame-pointer
endif

include $(TOPDIR)/Rules.make
