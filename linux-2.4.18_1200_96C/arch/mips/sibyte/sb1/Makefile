.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

L_TARGET = sb1kern.a

obj-$(CONFIG_SB1_CACHE_ERROR) += cache_err_handler.o cache_error.o

include $(TOPDIR)/Rules.make
