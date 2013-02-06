#
# Makefile for the common code of TOSHIBA JMR-TX3927 board
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

O_TARGET:= tx3927.o

obj-y	 += prom.o puts.o rtc_ds1742.o

include $(TOPDIR)/Rules.make
