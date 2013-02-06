CC		= mips-uclibc-gcc
COPT		= -Os -Wall
CFLAGS		= ${COPT}
LDFLAGS		= 
STRIP		= mips-uclibc-strip

ifeq ($(RTL8186_TR),1)
CFLAGS+=-DCONFIG_RTL8186_TR 
endif

ifeq ($(RTL8196C_EC),1)
CFLAGS+=-DCONFIG_RTL8196C_EC
endif

ifeq ($(RTL865X_AC),1)
CFLAGS+=-DCONFIG_RTL865X_AC 
endif

ifeq ($(RTL865X_KLD),1)
CFLAGS+=-DCONFIG_RTL865X_KLD
endif

ifeq ($(CONFIG_RTL865X_SC),1)
CFLAGS+=-DCONFIG_RTL865X_SC 
endif

ifeq ($(AP),1)
CFLAGS+=-DCONFIG_RTL865X_AP 
endif

ifeq ($(RTL8196B),1)
CFLAGS+=-DCONFIG_RTL8196B 
endif
ifeq ($(RTL8196C),1)
CFLAGS+=-DCONFIG_RTL8196C
endif

ifeq ($(RTL8198),1)
CFLAGS+=-DCONFIG_RTL8198
endif

all:    reload

clean:
	rm -f reload tags *.o 

sources = reload.c

reload:	$(sources:.c=.o)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@	
	$(STRIP) $@

