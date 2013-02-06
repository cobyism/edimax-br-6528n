include ../../../make.def

ifeq ($(MODEL),-D_7206GA_)
	CFLAGS = --static
else
	CFLAGS =
endif

all:			brctl brctld

brctl:			brctl.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
	$(CC)  -s -Wall $(CFLAGS) -o brctl brctl.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
	$(STRIP) $@
	
brctld:			brctld.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
	$(CC) -s -Wall $(CFLAGS) -o brctld brctld.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
	
clean:
			rm -f *.o
			rm -f brctl
			rm -f brctld
			rm -f core

brctl.o:		brctl.c brctl.h
			$(CC) -s $(CFLAGS) -I../libbridge -Wall -c -o brctl.o brctl.c

brctld.o:		brctld.c brctl.h
			$(CC) -s $(CFLAGS) -I../libbridge -Wall -c -o brctld.o brctld.c
brctl_cmd.o:		brctl_cmd.c brctl.h
			$(CC) -s $(CFLAGS) -I../libbridge -Wall -c -o brctl_cmd.o brctl_cmd.c
brctl_disp.o:		brctl_disp.c brctl.h
			$(CC) -s $(CFLAGS) -I../libbridge -Wall -c -o brctl_disp.o brctl_disp.c
