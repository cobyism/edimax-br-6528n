#
# Makefile for the Linux proc filesystem routines.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile.

O_TARGET := proc.o

export-objs := root.o

ifdef CONFIG_RTL_KERNEL_MIPS16_PROC
obj-y    := inode.o16 root.o16 array.o16 \
		kmsg.o16 proc_tty.o16 proc_misc.o16 kcore.o16
		
ifeq ($(CONFIG_RTL8196C),y)
obj-y += base.o generic.o
else
obj-y += base.o16 generic.o16
endif

else
obj-y    := inode.o root.o base.o generic.o array.o \
		kmsg.o proc_tty.o proc_misc.o kcore.o
endif

ifeq ($(CONFIG_PROC_DEVICETREE),y)
obj-y += proc_devtree.o
endif

include $(TOPDIR)/Rules.make
