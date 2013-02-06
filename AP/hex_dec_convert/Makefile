include ../../make.def
include ../../define/FUNCTION_COMPILER
include ../../define/PATH
include ../../target.def

EXEC = hex_dec_convert

CFLAGS += -I$(LINUXDIR)/drivers/char -I$(LINUXDIR)/include
CFLAGS += $(MODEL) $(FUNCTION)
all: $(EXEC)

$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS)
	$(STRIP) $@
romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

