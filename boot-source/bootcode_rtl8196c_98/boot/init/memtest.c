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
#include <asm/rtl8181.h>
#include <asm/io.h>


#define printf	prom_printf

#define COLUMN_S		8
#define	RAW_S			12
#define BANK_S			2
#define	DIE_S				1

#define COLUMN_ADDR			(1<<COLUMN_S)
#define	RAW_ADDR			(1<<RAW_S)
#define BANK_ADDR			(1<<BANK_S)
#define	DIE_ADDR			(1<<DIE_S)

int START_ADDR;
int LENGTH;

int errlog (int addr,int val1, int val2)
{
	int i;
*(volatile unsigned long *)(0xbd012000);
	prom_printf("Mem Test Errors! at addr=%X (read)%X!=(expected)%X\n",addr,val1, val2);
	cli();
	for(;;);
	
}


void cache_bmem_test(unsigned long addr, int length,char seed)
{
	int i,tmp,seed_s;
	
	seed_s	= 0xff - seed;
/*-------------------------------------------------
	increasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=0x0; i< length; i++)
	{
		*(unsigned char *)(addr+i)=seed;	
		//flush_dcache();
	}
	//Read seed, and write seed's
	for(i=0x00; i< length; i++)
	{
		//flush_dcache();
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*(unsigned char *)(addr+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*(unsigned char *)(addr+i)=seed;
	}
	// Read seed
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}

/*-------------------------------------------------
	Decreasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		*(unsigned char *)(addr+i)=seed;	
	}
	//Read seed, and write seed's
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*(unsigned char *)(addr+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*(unsigned char *)(addr+i)=seed;
	}
	// Read seed
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}	
	
}


void byte_mem_test(unsigned long addr, int length,char seed)
{
	int i,tmp,seed_s;
	
	seed_s	= 0xff - seed;
/*-------------------------------------------------
	increasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=0x0; i< length; i++)
	{
		*(unsigned char *)(addr+i)=seed;	
		//*(unsigned long *)(0xbd013000);	
	}	
	//Read seed, and write seed's
	for(i=0x00; i< length; i++)
	{
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*(unsigned char *)(addr+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	{
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*(unsigned char *)(addr+i)=seed;
	}
	// Read seed
	for(i=0; i< length; i++)
	{
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}

/*-------------------------------------------------
	Decreasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=length-1; i>= 0; i--)
		*(unsigned char *)(addr+i)=seed;	
	//Read seed, and write seed's
	for(i=length-1; i>= 0; i--)
	{
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*(unsigned char *)(addr+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=length-1; i>= 0; i--)
	{
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*(unsigned char *)(addr+i)=seed;
	}
	// Read seed
	for(i=length-1; i>= 0; i--)
	{
		tmp=*(unsigned char *)(addr+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}	
	
}


void cache_hwmem_test(unsigned long addr, int length,short seed)
{
	int i,tmp,seed_s;
	
	seed_s	= 0xffff - seed;
/*-------------------------------------------------
	increasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		*((unsigned short *)(addr)+i)=seed;	
	}
	//Read seed, and write seed's
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned short *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned short *)(addr)+i)=seed;
	}
	// Read seed
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}

/*-------------------------------------------------
	Decreasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		*((unsigned short *)(addr)+i)=seed;	
	}
	//Read seed, and write seed's
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned short *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned short *)(addr)+i)=seed;
	}
	// Read seed
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}	
	
}


void halfw_mem_test(unsigned long addr, int length,short seed)
{
	int i,tmp,seed_s;
	
	seed_s	= 0xffff - seed;
/*-------------------------------------------------
	increasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=0; i< length; i++)
		*((unsigned short *)(addr)+i)=seed;	
	//Read seed, and write seed's
	for(i=0; i< length; i++)
	{
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned short *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	{
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned short *)(addr)+i)=seed;
	}
	// Read seed
	for(i=0; i< length; i++)
	{
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}

/*-------------------------------------------------
	Decreasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=length-1; i>= 0; i--)
		*((unsigned short *)(addr)+i)=seed;	
	//Read seed, and write seed's
	for(i=length-1; i>= 0; i--)
	{
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned short *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=length-1; i>= 0; i--)
	{
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned short *)(addr)+i)=seed;
	}
	// Read seed
	for(i=length-1; i>= 0; i--)
	{
		tmp=*((unsigned short *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}	
	
}


void cache_wmem_test(unsigned long addr, int length,int seed)
{
	int i,tmp,seed_s;
	
	seed_s	= 0xffffffff - seed;
/*-------------------------------------------------
	increasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=0; i< length; i++)
	{	
		//flush_dcache();
		*((unsigned long *)(addr)+i)=seed;	
	}
	//Read seed, and write seed's
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned long *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned long *)(addr)+i)=seed;
	}
	// Read seed
	for(i=0; i< length; i++)
	{
		//flush_dcache();
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}

/*-------------------------------------------------
	Decreasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		*((unsigned long *)(addr)+i)=seed;	
	}	
	//Read seed, and write seed's
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned long *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned long *)(addr)+i)=seed;
	}
	// Read seed
	for(i=length-1; i>= 0; i--)
	{
		//flush_dcache();
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}	
	
}


void word_mem_test(unsigned long addr, int length,int seed)
{
	int i,tmp,seed_s;
	
	seed_s	= 0xffffffff - seed;
/*-------------------------------------------------
	increasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=0; i< length; i++)
		*((unsigned long *)(addr)+i)=seed;	
	//Read seed, and write seed's
	for(i=0; i< length; i++)
	{
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned long *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=0; i< length; i++)
	{
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned long *)(addr)+i)=seed;
	}
	// Read seed
	for(i=0; i< length; i++)
	{
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}

/*-------------------------------------------------
	Decreasing address
--------------------------------------------------*/
	
	// Write seed	
	for(i=length-1; i>= 0; i--)
		*((unsigned long *)(addr)+i)=seed;	
	//Read seed, and write seed's
	for(i=length-1; i>= 0; i--)
	{
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
			*((unsigned long *)(addr)+i)=seed_s;
	}
	//Read seed_s, and write seed
	for(i=length-1; i>= 0; i--)
	{
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed_s)   errlog(i,tmp,seed_s);
			*((unsigned long *)(addr)+i)=seed;
	}
	// Read seed
	for(i=length-1; i>= 0; i--)
	{
		tmp=*((unsigned long *)(addr)+i);	
		if (tmp != seed)   errlog(i,tmp,seed);
	}	
	
}


void async_byte_mem_test(unsigned long addr, int length,char seed)
{
	int i,j,k,p,tmp,seed_s;
	unsigned char *pAddr;
	
	seed_s	= 0xff - seed;
	

	// Write Seed	
	for (i=0; i<COLUMN_ADDR; i++)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						*(unsigned char *)(pAddr)=seed;
					}
	
	// Read Seed and Write seed_s	
	for (i=0; i<COLUMN_ADDR; i++)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp = *(unsigned char *)pAddr;
						if (tmp != seed)   errlog(i,tmp,seed);
						*(unsigned char *)(pAddr)=seed_s;
					}				

	// Read seed_s and write seed	
	for (i=0; i<COLUMN_ADDR; i++)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp = *(unsigned char *)pAddr;
						if (tmp != seed_s)   errlog(i,tmp,seed_s);
						*(unsigned char *)(pAddr)=seed_s;
					}					
	
	// Write Seed	
	for (i=COLUMN_ADDR-1; i>=0; i--)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						*(unsigned char *)(pAddr)=seed;
					}

	// Read Seed and Write seed_s
	for (i=COLUMN_ADDR-1; i>=0; i--)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned char *)(pAddr);
						if (tmp != seed) errlog(i,tmp,seed);
						*(unsigned char *)(pAddr)=seed_s;
					}								

	// Read Seed_s and Write seed
	for (i=COLUMN_ADDR-1; i>=0; i--)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned char *)(pAddr);
						if (tmp != seed_s) errlog(i,tmp,seed_s);
						*(unsigned char *)(pAddr)=seed;
					}							
	// Read Seed and write seed_s
	for (i=COLUMN_ADDR-1; i>=0; i--)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned char *)(pAddr);
						if (tmp != seed) errlog(i,tmp,seed);
						*(unsigned char *)(pAddr)=seed_s;
					}			
}


void async_word_mem_test(unsigned long addr, int length,int seed)
{
	int i,j,k,p,tmp,seed_s;
	unsigned short *pAddr;
	
	seed_s	= 0xffffffff - seed;
	
	// Write Seed	
	for (i=0; i<COLUMN_ADDR; i+=4)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						*(unsigned long *)(pAddr)=seed;
					}
	
	// Read Seed and Write seed_s	
	for (i=0; i<COLUMN_ADDR; i+=4)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp = *(unsigned long *)pAddr;
						if (tmp != seed)   errlog(i,tmp,seed);
						*(unsigned long *)(pAddr)=seed_s;
					}				

	// Read seed_s and write seed	
	for (i=0; i<COLUMN_ADDR; i+=4)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp = *(unsigned long *)pAddr;
						if (tmp != seed_s)   errlog(i,tmp,seed_s);
						*(unsigned long *)(pAddr)=seed_s;
					}					
	
	// Write Seed	
	for (i=COLUMN_ADDR-4; i>=0; i-=4)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						*(unsigned long *)(pAddr)=seed;
					}

	// Read Seed and Write seed_s
	for (i=COLUMN_ADDR-4; i>=0; i-=4)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned long *)(pAddr);
						if (tmp != seed) errlog(i,tmp,seed);
						*(unsigned long *)(pAddr)=seed_s;
					}								

	// Read Seed_s and Write seed
	for (i=COLUMN_ADDR-4; i>=0; i-=4)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned long *)(pAddr);
						if (tmp != seed_s) errlog(i,tmp,seed_s);
						*(unsigned long *)(pAddr)=seed;
					}							
	// Read Seed and write seed_s
	for (i=COLUMN_ADDR-4; i>=0; i-=4)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned long *)(pAddr);
						if (tmp != seed) errlog(i,tmp,seed);
						*(unsigned long *)(pAddr)=seed_s;
					}			
}

void async_halfw_mem_test(unsigned long addr, int length,short seed)
{
	int i,j,k,p,tmp,seed_s;
	unsigned short *pAddr;
	
	seed_s	= 0xffff - seed;
	
	// Write Seed	
	for (i=0; i<COLUMN_ADDR; i+=2)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						*(unsigned short *)(pAddr)=seed;
					}
	
	// Read Seed and Write seed_s	
	for (i=0; i<COLUMN_ADDR; i+=2)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp = *(unsigned short *)pAddr;
						if (tmp != seed)   errlog(i,tmp,seed);
						*(unsigned short *)(pAddr)=seed_s;
					}				

	// Read seed_s and write seed	
	for (i=0; i<COLUMN_ADDR; i+=2)
		for (j=0; j<RAW_ADDR; j++)
			for(k=0; k<BANK_ADDR; k++)
				for(p=0; p<DIE_ADDR;p++)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp = *(unsigned short *)pAddr;
						if (tmp != seed_s)   errlog(i,tmp,seed_s);
						*(unsigned short *)(pAddr)=seed_s;
					}					
	
	// Write Seed	
	for (i=COLUMN_ADDR-2; i>=0; i-=2)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						*(unsigned short *)(pAddr)=seed;
					}

	// Read Seed and Write seed_s
	for (i=COLUMN_ADDR-2; i>=0; i-=2)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned short *)(pAddr);
						if (tmp != seed) errlog(i,tmp,seed);
						*(unsigned short *)(pAddr)=seed_s;
					}								

	// Read Seed_s and Write seed
	for (i=COLUMN_ADDR-2; i>=0; i-=2)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned short *)(pAddr);
						if (tmp != seed_s) errlog(i,tmp,seed_s);
						*(unsigned short *)(pAddr)=seed;
					}							
	// Read Seed and write seed_s
	for (i=COLUMN_ADDR-2; i>=0; i-=2)
		for (j=RAW_ADDR-1; j>=0; j--)
			for(k=BANK_ADDR-1; k>=0; k--)
				for(p=DIE_ADDR-1; p>=0;p--)
					{
						pAddr = addr + i + (j << COLUMN_S) +(k << (RAW_S+COLUMN_S)) + (p<<(COLUMN_S+RAW_S+BANK_S));
						tmp  = *(unsigned short *)(pAddr);
						if (tmp != seed) errlog(i,tmp,seed);
						*(unsigned short *)(pAddr)=seed_s;
					}			
}

void memtest()
{

	extern unsigned int free_memory_start;
	extern unsigned int free_memory_size;
	extern unsigned int loop;


	unsigned int i;
	unsigned long addr;
	int	regvalue;
	
	
	LENGTH = free_memory_size;

	START_ADDR = free_memory_start;	
	
	prom_printf("Beginning of SDRAM Test...\n  START_ADDR=%X, LENGTH=%X\n", START_ADDR,LENGTH); 
	
#if 0	
	for(i=0; i< 1024*1024; i++)
	{
		*(unsigned char *)(0x80500000+i) = i & 0xff;		
	}
	
	
	memset(0x80500000, 0x00, 1024*1024);
	
	
	for(i=0; i < 1024*1024; i++)
	{
		if ( (*(unsigned char *)(0x80500000 + i)) != 0x00)
			prom_printf("memset error at i=%X\n", i);	
	}
	
	
	prom_printf("memset test pass!\n");
	
#endif	
	//for(loop=0x0; loop<=0xfffffff0; loop++)
	{
		printf("Loop=%X\n", loop);
#if 1
	// Test non cache address first
	addr = ( START_ADDR | 0xa0000000);  // to make the compiler happy

	printf("For Non-Cacheable Address Testing ...\n");

	printf("Begining of byte_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	byte_mem_test(addr,LENGTH,loop);

	printf("Begining of halfword_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH>>1);
	halfw_mem_test(addr,(LENGTH>>1), loop); 


	
	
	printf("Begining of word_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH>>2);
	word_mem_test(addr, (LENGTH>>2), loop);
#endif
#if 1	
	printf("For Cacheable Address Testing ...\n");
	flush_dcache();
	//Test cacheable address
	addr = (unsigned char *)(START_ADDR & 0x8fffffff);  // to make the compiler happy

	printf("Begining of cache_bmem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	
	cache_bmem_test(addr,LENGTH,loop);

	printf("Begining of byte_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);

	

	byte_mem_test(addr,LENGTH,loop);
	
	flush_dcache();


	printf("Begining of halfword_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	halfw_mem_test(addr,(LENGTH>>1), loop); 

		printf("Begining of cache_wmem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);

		cache_hwmem_test(addr,(LENGTH>>1), loop); 		
	printf("Begining of cache_wmem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	cache_wmem_test(addr, (LENGTH>>2), loop);

	
	printf("Begining of word_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	flush_dcache();
	word_mem_test(addr, (LENGTH>>2), loop);
	
	addr = START_ADDR; 
	printf("Begining of async_byte_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	
	async_byte_mem_test(addr,LENGTH, loop);

	flush_dcache();
	printf("Begining of async_halfword_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	async_halfw_mem_test(addr,(LENGTH), loop); 

	flush_dcache();
	printf("Begining of async_word_mem_test... starting addr=%X, length=%X bytes\n"
	,addr,LENGTH);
	async_word_mem_test(addr, (LENGTH), loop);
	#endif
	}
	

	printf("SDRAM Testing ok...\n");
}



