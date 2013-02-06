/* read_memory.c
 * 
 * This file is subject to the terms and conditions of the GNU
 * General Public License.  See the file "COPYING" in the main
 * directory of this archive for more details.
 *
 * Copyright (C) 2000, Jay Carlson
 */

/*   read_memory is the memory-based back end for the image-reading
 * functions.
 * 
 * Support for non-compressed kernels has probably rotted.  
 */


#include <linux/types.h>
#include "hfload.h"

#ifndef EMBEDDED
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

char *source_memory;

#ifdef EMBEDDED
#ifndef COMPRESSED_KERNEL
extern char source_memory_start;
#endif
#endif

// david -----------------------
#include <linux/config.h>

#ifdef BZ2_COMPRESS //sc_yang
void * memcpy(void * dest,const void *src,size_t count)
{
        char *tmp = (char *) dest, *s = (char *) src;

        while (count--)
                *tmp++ = *s++;

        return dest;
}
void * memset(void * s, int c, size_t count)
{
        char *xs = (char *) s;

        while (count--)
                *xs++ = (char) c;

        return s;
}
#endif // CONFIG_NINO_8MB
//------------------------------


void
read_struct(void *s, ssize_t size)
{
	memcpy(s, source_memory+file_offset, size);
    file_offset += size;
}

void
seek_forward(int offset) {
        if (offset % 4 != 0) {
         #ifndef __DO_QUIET__       
				printf("Can't seek by a non-word aligned amount\n");
        #endif 
		       exit(1);
        }
        
        if (offset < file_offset) {
            #ifndef __DO_QUIET__    
			printf("can't seek backwards\n");
			#endif
                exit(1);
        }

        file_offset = offset;
}

void
copy_to_region(int *addr, ssize_t size) {
        int i, dummy;

        int *source, *dest;
#ifndef __DO_QUIET__
        printf("copying 0x%x bytes from file offset 0x%x to address 0x%08x\n",
               size, file_offset, addr);
#endif
#ifndef FAKE_COPYING
        memcpy(addr, source_memory+file_offset, size);
#endif
        
        file_offset += size;
}

void
init_read() {
#ifndef EMBEDDED
        struct stat buf;
        if (fstat(0, &buf)) {
                perror("stat");
                exit(1);
        }
        source_memory = mmap(0, buf.st_size & ~(4095), PROT_READ, MAP_PRIVATE, 0, 0);
        if (!source_memory) {
                perror("mmap");
                exit(1);
        }
#else
#ifdef COMPRESSED_KERNEL
        source_memory = UNCOMPRESS_OUT;
#else
        source_memory = &source_memory_start;
#endif
#endif
}
