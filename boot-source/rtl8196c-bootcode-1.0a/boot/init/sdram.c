#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <linux/circ_buf.h>


#include <asm/io.h>
#include <asm/rtl8181.h>

#define COLUMN_S		8
#define	RAW_S			12
#define BANK_S			2
#define	DIE_S				1

#define COLUMN_ADDR			(1<<COLUMN_S)
#define	RAW_ADDR			(1<<RAW_S)
#define BANK_ADDR			(1<<BANK_S)
#define	DIE_ADDR			(1<<DIE_S)

#define MAIN_MEMORY_SIZE	(16*1024*1024)

/*SDRAM area to test*/
Int32 free_memory_start;
Int32 free_memory_size;

void SDRAM_init(void);
void SDRAM_init_test(void);
void byte4_mem_test(Int32 addr, Int32 length,Int32 seed);

extern void prom_printf(char *, ...);
extern void flush_dcache();
#define printf	prom_printf

int errlog (int addr,int val1, int val2)
{
 prom_printf("Mem Test Errors! at addr=%X (read)%X!=(expected)%X\n",addr,val1, val2);
}



/*can be found in ld.script*/
extern char _stext, _etext;
extern char _end;
extern char _ftext;
void SDRAM_init(void)
{
 Int32 i=0;

 /*First initialize SDRM Memory register*/
 rtl_outw(1008,0x884);
 for(i=0x0000;i<=0xffff0000;i=i+0x01) ;
 if(rtl_inw(1008)!=0x0884)
   {
    prom_printf("SDRAM memory control register fail.");
    }
 /*The region required to check.*/
 /*From the 0x80000000 to end, already be occupied boot code.*/
 free_memory_start = (((unsigned int)&(_end) + 4) &(~0x03));
 /*From _ftext to _end, already occupied by bootcode*/
 free_memory_size = MAIN_MEMORY_SIZE - (free_memory_start - (unsigned int)&(_ftext));

 flush_dcache();
 SDRAM_init_test();
}


void SDRAM_init_test(void)
{
/*we have total 16MB memory, and started at free_memory_start*/	
/*this function will be simple test.*/
/*More complex test will be at the FULL-SET-TEST in CTRLX*/

/*Mostly used is cacheable, 32 bit access, such as ethernet descriptor*/
/*Test if Basic Read/Write is OK*/
/*Make sure it is cacheable region*/
/*free_memory_start must be virtual address began at 8xxx or axxx that is boot code*/
/*and never go to bxxx or 9xxx*/
byte4_mem_test(free_memory_start&0x8ffffffff,(free_memory_size>>2),0x12345678);
/*Make sure it is non-cacheable region*/
byte4_mem_test(Virtual2NonCache(free_memory_start),(free_memory_size>>2),0xabcdef01);
}

void byte4_mem_test(Int32 addr, Int32 length, Int32 seed)
{
	Int32 i,tmp,seed_s;     
	seed_s	= 0xffffffff - seed;
	// Write seed	
	for(i=0; i< length; i++)
	    *((Int32 *)(addr)+i)=seed;	
	//Read seed, and write seed's
	for(i=0; i< length; i++)
	   {
	    tmp=*((Int32 *)(addr)+i);	
	    if (tmp != seed)
	        errlog(i,tmp,seed);
  	    *((Int32 *)(addr)+i)=seed_s;
	   } 
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	   {
	    tmp=*((Int32 *)(addr)+i);	
	    if (tmp != seed_s)
	        errlog(i,tmp,seed_s);
 	    *((Int32 *)(addr)+i)=seed;
	   }
	// Read seed
	for(i=0; i< length; i++)
	{
	 tmp=*((Int32 *)(addr)+i);	
	 if (tmp != seed)   
	     errlog(i,tmp,seed);
	}
}
//---------------------------------------------------------------------