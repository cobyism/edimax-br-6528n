
#define WP_PORT_BASE    0xBBF00000

#define wpreg_bread(val, addr)  \
	do {  \
	val = (*(volatile unsigned char *)(addr+WP_PORT_BASE));\
	}  \
	while (0)

