#KERNEL_INCLUDE=/usr/src/linux/include
KERNEL_INCLUDE=../../../linux-2.4.18/include

C=mips-uclibc-gcc
#CC=gcc
CCOPT=-O2 -Wstrict-prototypes -Wall -Werror -fno-strict-aliasing -fno-common
CFLAGS=-DMODULE -D__KERNEL__ -I$(KERNEL_INCLUDE) $(CCOPT)


all: pg3.o

clean:
	rm -f *.o
