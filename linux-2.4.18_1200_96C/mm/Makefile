#
# Makefile for the linux memory manager.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := mm.o

export-objs := shmem.o filemap.o

ifdef CONFIG_RTL_KERNEL_MIPS16_MM
obj-y	 := memory.o16 mmap.o16  mprotect.o16 mlock.o16 mremap.o16 \
		vmalloc.o16 slab.o16 bootmem.o16 swap.o16 vmscan.o16 page_io.o16 \
		 swap_state.o16 swapfile.o16 numa.o16 oom_kill.o16 shmem.o16

ifeq ($(CONFIG_RTL8196C),y)
obj-y += filemap.o page_alloc.o
else
obj-y += filemap.o16 page_alloc.o16
endif

else
obj-y	 := memory.o mmap.o filemap.o mprotect.o mlock.o mremap.o \
	    vmalloc.o slab.o bootmem.o swap.o vmscan.o page_io.o \
	    page_alloc.o swap_state.o swapfile.o numa.o oom_kill.o \
	    shmem.o
endif

obj-$(CONFIG_HIGHMEM) += highmem.o

include $(TOPDIR)/Rules.make
