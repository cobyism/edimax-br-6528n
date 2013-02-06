/* This file has not been tested for ages. */

#include "hfload.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void 
read_struct(void *s, ssize_t size) 
{
        ssize_t real;

        real = read(0, s, size);
        if (size != real) {
                printf("trying to read %d, got %d; dying.\n", size, real);
                exit(1);
        }
        file_offset += real;
}

void
seek_forward(int offset) {
        int dummy, i;
        ssize_t real;

        if (offset % 4 != 0) {
                printf("Can't seek by a non-word aligned amount\n");
                exit(1);
        }
        
        if (offset < file_offset) {
                printf("can't seek backwards\n");
                exit(1);
        }

        for (; offset < file_offset; file_offset += 4) {
                real = read(0, &dummy, 4);
                if (real != 4) {
                        printf("error seeking forward at offset %d\n", file_offset);
                        exit(1);
                }
        }
}

void
copy_to_region(int *addr, ssize_t size) {
        int i, dummy, real;

        printf("copying %x bytes from file offset %x to address %08x\n",
               size, file_offset, addr);

#ifdef FAKE_COPYING
        for (i = 0; i < size; i += 4) {
                read_struct(&dummy, sizeof(int));
        }
#else
        real = read(0, addr, size);
        if (real != size) {
                printf("failed to read %d bytes, exiting\n");
                exit(1);
        }
        file_offset += real;
#endif
}

void
init_read() {
}
