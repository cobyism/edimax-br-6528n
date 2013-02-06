include ../../make.def
include ../../define/FUNCTION_COMPILER
include ../../define/PATH
include ../../target.def

EXEC = epi_ttcp

all: $(EXEC)

$(EXEC): $(EXEC).c
	$(CC) -o $@ $@.c $(LDLIBS)
	$(STRIP) $@
romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.o

