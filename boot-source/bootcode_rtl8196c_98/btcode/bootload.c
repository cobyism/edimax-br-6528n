#include "../boot/init/ver.h"
#define NULL ((void *)0)
#ifdef CONFIG_NFBI
#define DECOMP_ADDR 0x80600000
#define LZMA_STATUS_ADDR 0x80500000
#else
#define DECOMP_ADDR 0x80400000
#define LZMA_STATUS_ADDR 0x80300000
#endif

#ifdef LZMA_COMPRESS
typedef unsigned int UINT32;
#define ___swab32(x) \
 	({ \
		UINT32 __x = (x); \
		((UINT32)( \
		(((UINT32)(__x) & (UINT32)0x000000ffUL) << 24) | \
			(((UINT32)(__x) & (UINT32)0x0000ff00UL) <<  8) | \
			(((UINT32)(__x) & (UINT32)0x00ff0000UL) >>  8) | \
			(((UINT32)(__x) & (UINT32)0xff000000UL) >> 24) )); \
	})
#define le32_to_cpu(x) ___swab32(x)
void * memcpy(void * dest,const void *src,int count)
{
        char *tmp = (char *) dest, *s = (char *) src;

        while (count--)
                *tmp++ = *s++;

        return dest;
}
#endif
/* Extern VARIABLE DECLARATIONS
 */
extern char  __boot_start[];
extern char  __boot_end[];
void boot_entry(void);
///////////////////////////////////////////////////////////////////
//DEBUG ISSUE
///////////////////////////////////////////////////////////////////
#ifdef DEBUG_MODE
#define IO_BASE		0xB8000000
#define UART_RBR	(0x2000+IO_BASE)
#define UART_THR	(0x2000+IO_BASE)
#define UART_DLL	(0x2000+IO_BASE)
#define	UART_IER	(0x2004+IO_BASE)
#define	UART_DLM	(0x2004+IO_BASE)
#define	UART_IIR	(0x2008+IO_BASE)
#define	UART_FCR	(0x2008+IO_BASE)
#define UART_LCR	(0x200c+IO_BASE)
#define	UART_MCR	(0x2010+IO_BASE)
#define	UART_LSR	(0x2014+IO_BASE)
#define	UART_MSR	(0x2018+IO_BASE)
#define	UART_SCR	(0x201c+IO_BASE)
#if 0
#define REG32(reg) (*(volatile unsigned int *)(reg))
#endif
#define _SYSTEM_HEAP_SIZE	1024*64	//wei add

/* DATA STRUCTURE DECLARATIONS
typedef unsigned long  __u32;
*/

typedef struct hdr {
        struct hdr *    ptr;
        UINT32          size;
} HEADER;

char dl_heap[_SYSTEM_HEAP_SIZE];	//wei add
/* STATIC VARIABLE DECLARATIONS
 */
static HEADER * frhd; /* free list head */
static UINT32   memleft; /* memory left */
void console_init(void)
{

	int i;
	unsigned long dl;
	unsigned long dll;     
	unsigned long dlm;       
           
  	REG32( UART_LCR)=0x03000000;		//Line Control Register  8,n,1
  		
  	REG32( UART_FCR)=0xc7000000;		//FIFO Ccontrol Register
  	REG32( UART_IER)=0x00000000;
  	dl = (200000000 /16)/38400-1;
  	*(volatile unsigned long *)(0xa1000000) = dl ; 
  	dll = dl & 0xff;
  	dlm = dl / 0x100;
  	REG32( UART_LCR)=0x83000000;		//Divisor latch access bit=1
  	REG32( UART_DLL)=dll*0x1000000;
   	REG32( UART_DLM)=dlm*0x1000000; 
   	REG32( UART_LCR)=0x83000000& 0x7fffffff;	//Divisor latch access bit=0

}

void print_msg(char x)
{
	REG32(UART_THR)=x<<24;
}
void    i_alloc(void * _heapstart, void * _heapend)
/*
        _heapstart is the first byte allocated to the heap.
        _heapend is the last.
*/
{
        /* initialize the allocator */
        frhd = (HEADER *) _heapstart;
        frhd->ptr = NULL;
        frhd->size = (_heapend - _heapstart) / sizeof(HEADER);
        memleft = frhd->size; /* initial size in 4-byte units */
}
void * malloc(UINT32 nbytes)                /* bytes to allocate */
{
        HEADER      *nxt, *prev;
        UINT32      nunits;

        nunits = (nbytes+sizeof(HEADER)-1)
                        / sizeof(HEADER) + 1;
        /*
                Change above divide to shift (for speed) only if                        
                the compiler doesn't do this for you and you                            
                don't require portability and you know that                             
                sizeof (HEADER) is a power of 2.
        
                Allocate the space requested + space for the                            
                header of the block.
        
                Search the free space queue for a block large                           
                enough. If block is larger than needed, break                           
                the block into two pieces and allocate the                              
                portion higher up in memory. Otherwise, just
                allocate the entire block.
        */
        for (prev=NULL,nxt=frhd; nxt; nxt = nxt->ptr)
        {
                if (nxt->size >= nunits) /* big enough */
                {
                        if (nxt->size > nunits)
#if 0
                        /* allocate tail end */
                        {
                                nxt->size -= nunits;
                                nxt += nxt->size;       
                                /* nxt now == ptr to alloc */
                                nxt->size = nunits;
                        }
#else
                        /* allocate head end */
                        {
                                HEADER  *seg = nxt + nunits;
                                if (prev==NULL) frhd = seg;
                                else prev->ptr = seg;
                                seg->ptr = nxt->ptr;
                                seg->size = nxt->size - nunits;
                                
                                /* nxt now == ptr to alloc */
                                nxt->size = nunits;
                        }
#endif
                        else
                        {
                                if (prev==NULL) frhd = nxt->ptr;
                                else prev->ptr = nxt->ptr;
                        }
                        memleft -= nunits;
                        return((void *)(nxt+1));
                        /*
                                Return a pointer past the header to                                     
                                the actual space requested.
                        */
                }
        }
        //printf("\nAllocation Failed!");
        while(1);
}
#endif
///////////////////////////////////////////////////////////////////
//END of DEBUG ISSUE
///////////////////////////////////////////////////////////////////


void boot_entry(void)
{


	unsigned char *inbuf;
	unsigned long insize;
	unsigned char *outbuf;
	unsigned long outsize;
	unsigned int i ;
	void (*jumpF)(void);
	
#ifdef DEBUG_MODE
	UINT32 heap_addr=((UINT32)dl_heap&(~7))+8 ;
  	i_alloc((void *)heap_addr, heap_addr+sizeof(dl_heap)-8);
#endif
	inbuf = (unsigned char *)__boot_start;
	outbuf = (unsigned char *)(DECOMP_ADDR);
	insize = 0x7fffffff;
	outsize = 0x7fffffff;
	//print_msg('Q');	
#ifndef LZMA_COMPRESS
	gunzip(inbuf, &insize, outbuf, &outsize);
#else
	{
	#include "LzmaDecode.h"
	
	unsigned char *startBuf=inbuf;//__vmlinux_start;
	unsigned char *outBuf = outbuf;//(unsigned char *)output_start;
								//_bootimg_end
	unsigned int inLen=__boot_end-__boot_start;//__vmlinux_end - __vmlinux_start;		
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
	compressedSize = (SizeT)(inLen - (LZMA_PROPERTIES_SIZE + 8));

	memcpy(properties, startBuf, sizeof(properties));
	startBuf += sizeof(properties);

	memcpy((char *)&outSize, startBuf, 4);
	outSize = le32_to_cpu(outSize);	

	memcpy((char *)&outSizeHigh, startBuf+4, 4);	
	outSizeHigh = le32_to_cpu(outSizeHigh);

    outSizeFull = (SizeT)outSize;
    if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize) {
		//printf("LZMA: Too big uncompressed stream\n");
		return 0;
   	}	
	startBuf += 8;    
	
	/* Decode LZMA properties and allocate memory */  
	if (LzmaDecodeProperties(&state.Properties, properties, LZMA_PROPERTIES_SIZE) != LZMA_RESULT_OK) {
		//puts("LZMA: Incorrect stream properties\n");
		return 0;
	}
	//state.Probs = (CProb *)malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
	state.Probs = (CProb *)((void*)(LZMA_STATUS_ADDR));//malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
	//signed int iproc, oproc;
      //signed char probs[LzmaGetNumProbs(&state.Properties) * sizeof(CProb)];

      //state.Probs = (CProb *) probs;
	
    res = LzmaDecode(&state, startBuf, compressedSize, &inProcessed,
					      outBuf, outSizeFull, &outProcessed);    
    if (res != 0) {
     // sprintf(tmpbuf, "LZMA: Decoding error = %d\n", res);
      //puts(tmpbuf);
      return 0;	 
    } 

}
#endif
	jumpF = (void (*)(void))(DECOMP_ADDR);
	(*jumpF)();
}

