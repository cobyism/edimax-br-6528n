#
# Makefile for the linux squashfs routines.
#

O_TARGET := squashfs.o

obj-y  := inode.o

ifeq ($(CONFIG_SQUASHFS_LZMA),y)
obj-y += LzmaDecode.o
endif

obj-m := $(O_TARGET)

CFLAGS_inode.o := -I $(TOPDIR)/fs/inflate_fs
include $(TOPDIR)/Rules.make
