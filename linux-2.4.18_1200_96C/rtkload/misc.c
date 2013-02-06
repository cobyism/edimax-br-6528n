/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 *
 * Modified for ARM Linux by Russell King
 *
 * Nicolas Pitre <nico@visuaide.com>  1999/04/14 :
 *  For this code to run directly from Flash, all constant variables must
 *  be marked with 'const' and all other variables initialized at run-time 
 *  only.  This way all non constant variables will end up in the bss segment,
 *  which should point to addresses in RAM and cleared to 0 on start.
 *  This allows for a much quicker boot time.
 *
 * Stolen for mips loader to handle decompression by Jay Carlson
 * <nop@nop.com> 2000/04/17
 */

// david ------------------
#ifdef BZ2_COMPRESS
#define  __DO_QUIET__
#endif
//-------------------------


unsigned int __machine_arch_type;

#ifndef __DO_QUIET__
#define puts prom_printf
#else
#define puts
#endif
#define arch_decomp_wdog()
#define arch_decomp_setup()
#define proc_decomp_setup()  /* this is where cache flushing, clock, etc lived */

#define memzero(d, count) (memset((d), 0, (count)))

#define __ptr_t void *
#define NULL ((void *) 0)

typedef unsigned long  __u32;
#define ___swab32(x) \
 	({ \
		__u32 __x = (x); \
		((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
			(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
			(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
			(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
	})

#define le32_to_cpu(x) ___swab32(x)


/*
 * gzip declarations
 */
#define OF(args)  args
#define STATIC static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */

static uch *inbuf;		/* input buffer */
static uch window[WSIZE];	/* Sliding window buffer */
static unsigned insize;		/* valid bytes in inbuf */
static unsigned inptr;		/* index of next byte to be processed in inbuf */
static unsigned outcnt;		/* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* Diagnostic functions */
#ifdef DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);
static void gzip_mark(void **);
static void gzip_release(void **);

extern char __vmlinux_start[];
extern char __vmlinux_end[];

static uch *output_data;
static ulg output_ptr;
static ulg bytes_out;

static void *malloc(int size);
static void free(void *where);
static void error(char *m);
static void gzip_mark(void **);
static void gzip_release(void **);

/* static void puts(const char *); */

/* extern int end; */
static ulg free_mem_ptr;
static ulg free_mem_ptr_end;

#define HEAP_SIZE 0x2000

/* #include "../../../../lib/inflate.c" */
/* #include "../lib/inflate.c"*/

// david ----------------
//#include <inflate.c>

#ifndef BZ2_COMPRESS
#include <inflate.c>
#else
#include "../lib/bzip2_inflate.c"
#endif
//----------------------

#if defined(CONFIG_RTL8197B_PANA) || defined(CONFIG_RTL_8198_NFBI_BOARD)
int is_vmlinux_checksum_ok(void)
{
	unsigned long sum = 0;
	unsigned long *buf = (unsigned long *)(((long)__vmlinux_start)+4);
	unsigned long image_len = (unsigned long)(__vmlinux_end -__vmlinux_start-4);	
	
	for (; ((unsigned long)buf)<((unsigned long)__vmlinux_end);  buf++)
		sum += *buf;
	
	return ((sum == 0) ? 1 : 0);
}
#endif

#ifndef STANDALONE_DEBUG
static void *malloc(int size)
{
	void *p;

	if (size <0) error("Malloc error\n");
	if (free_mem_ptr <= 0) error("Memory error\n");

	free_mem_ptr = (free_mem_ptr + 3) & ~3;	/* Align */

	p = (void *)free_mem_ptr;
	free_mem_ptr += size;

	if (free_mem_ptr >= free_mem_ptr_end)
		error("Out of memory");
	return p;
}

static void free(void *where)
{ /* gzip_mark & gzip_release do the free */
}

static void gzip_mark(void **ptr)
{
	arch_decomp_wdog();
	*ptr = (void *) free_mem_ptr;
}

static void gzip_release(void **ptr)
{
	arch_decomp_wdog();
	free_mem_ptr = (long) *ptr;
}
#else
static void gzip_mark(void **ptr)
{
}

static void gzip_release(void **ptr)
{
}
#endif

// david
#ifndef BZ2_COMPRESS
/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
int fill_inbuf(void)
{
	if (insize != 0)
		error("ran out of input data\n");

	inbuf = __vmlinux_start;
	insize = &__vmlinux_end[0] - &__vmlinux_start[0];

	inptr = 1;
	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
void flush_window(void)
{
	ulg c = crc;
	unsigned n;
	uch *in, *out, ch;

	in = window;
	out = &output_data[output_ptr];
	for (n = 0; n < outcnt; n++) {
		ch = *out++ = *in++;
		c = crc_32_tab[((int)c ^ ch) & 0xff] ^ (c >> 8);
	}
	crc = c;
	bytes_out += (ulg)outcnt;
	output_ptr += (ulg)outcnt;
	outcnt = 0;
	//puts(".");
}
#endif // BZ2_COMPRESS, david


static void error(char *x)
{
	int ptr;

	puts("\n\n");
	puts(x);
	puts("\n\n -- System halted");

	while(1);	/* Halt */
}

#ifndef STANDALONE_DEBUG

ulg
decompress_kernel(ulg output_start, ulg free_mem_ptr_p, ulg free_mem_ptr_end_p,
		  int arch_id)
{
	output_data		= (uch *)output_start;	/* Points to kernel start */
	free_mem_ptr		= free_mem_ptr_p;
	free_mem_ptr_end	= free_mem_ptr_end_p;
	__machine_arch_type	= arch_id;

	proc_decomp_setup();
	arch_decomp_setup();

// david
//	makecrc();
	puts("Uncompressing Linux...");
// david ---------------------------------
// 	gunzip();
#ifdef LZMA_COMPRESS
{
	#include "LzmaDecode.h"
	
	unsigned char *startBuf=__vmlinux_start;
	unsigned char *outBuf = (unsigned char *)output_start;
	unsigned int inLen=__vmlinux_end - __vmlinux_start;	

	SizeT compressedSize;
	unsigned char *inStream; 
	UInt32 outSize = 0;
	UInt32 outSizeHigh = 0;	
	SizeT outSizeFull;
	int res;
    SizeT inProcessed;
    SizeT outProcessed;
	char tmpbuf[100];
	CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
	unsigned char properties[LZMA_PROPERTIES_SIZE];
	
#if defined(CONFIG_RTL8197B_PANA) || defined(CONFIG_RTL_8198_NFBI_BOARD)
    unsigned long pending_len = *((unsigned long *)__vmlinux_start);
	startBuf += 4;
	inLen -= (4+pending_len);
#endif

	compressedSize = (SizeT)(inLen - (LZMA_PROPERTIES_SIZE + 8));

	memcpy(properties, startBuf, sizeof(properties));
	startBuf += sizeof(properties);

	memcpy((char *)&outSize, startBuf, 4);
	outSize = le32_to_cpu(outSize);	

	memcpy((char *)&outSizeHigh, startBuf+4, 4);	
	outSizeHigh = le32_to_cpu(outSizeHigh);

    outSizeFull = (SizeT)outSize;
    if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize) {
		puts("LZMA: Too big uncompressed stream\n");
		return 0;
   	}	
	startBuf += 8;    
	
	/* Decode LZMA properties and allocate memory */  
	if (LzmaDecodeProperties(&state.Properties, properties, LZMA_PROPERTIES_SIZE) != LZMA_RESULT_OK) {
		puts("LZMA: Incorrect stream properties\n");
		return 0;
	}
	state.Probs = (CProb *)malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
    
    res = LzmaDecode(&state, startBuf, compressedSize, &inProcessed,
					      outBuf, outSizeFull, &outProcessed);    
    if (res != 0) {
      sprintf(tmpbuf, "LZMA: Decoding error = %d\n", res);
      puts(tmpbuf);
      return 0;
    }    
}

#else // !LZMA_COMPRESS
#ifndef BZ2_COMPRESS
	makecrc();
	gunzip();
#else

// david ------------
{
        unsigned char *startBuf=__vmlinux_start;
	unsigned char *outBuf = (unsigned char *)output_start;
	unsigned int inLen=__vmlinux_end - __vmlinux_start;
	int outLen=-1;

	BZ2_bzBuffToBuffDecompress(outBuf, &outLen, startBuf, inLen, 0, 0);
}
#endif // BZ2_COMPRESS
//----------------------------------------

#endif // LZMA_COMPRESS

	puts(" done, booting the kernel.\n");
	return output_ptr;
}
#else

char output_buffer[1500*1024];

int main()
{
	output_data = output_buffer;

	makecrc();
	puts("Uncompressing Linux...");
	gunzip();
	puts("done.\n");
	return 0;
}
#endif
	
