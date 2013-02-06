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

static int START_ADDR;
static int LENGTH;

#define 	byte_write(addr, val)	*(unsigned char *)(addr)=(val)
#define 	hw_write(addr, val)		*(unsigned short *)(addr) = (val)
#define  	word_write(addr, val)	*(unsigned long *)(addr) = (val)

#define byte_read(addr)		(*(unsigned char*)(addr))
#define hw_read(addr)		(*(unsigned short *)(addr))
#define word_read(addr)		(*(unsigned long *)(addr))

#define addr2seed_16(addr)	(((addr&0xff) << 8) | ((addr+1) & 0xff))
#define addr2seed_32(addr)	(((addr&0xff) << 24) | (((addr+1) & 0xff) << 16)  \
				| (((addr+2) & 0xff) << 8) | ((addr+3) & 0xff) )



void byte_intf(unsigned long Addr, int Length)
{
	unsigned int i;	
	unsigned long addr;
	
	unsigned char   tmp_8, seed_8;
	unsigned short 	tmp_16,seed_16;
	unsigned long	tmp_32,seed_32;
	
	
	// sequential mode
	for(i=0,addr=Addr; i< Length; i+=4)
	{
			
		byte_write(addr, 	(addr));
		byte_write(addr+1, 	(addr+1));
		byte_write(addr+2, 	(addr+2));
		byte_write(addr+3, 	(addr+3));	
		seed_16 = addr2seed_16(addr);	
		if ((tmp_16=hw_read(addr)) != seed_16) 
				errlog(addr, tmp_16, seed_16);	

		seed_16 = addr2seed_16((addr+2));	
		if ((tmp_16=hw_read(addr+2)) != seed_16)
				errlog(addr, tmp_16, seed_16);	
		seed_32 = addr2seed_32((addr));	
		if ((tmp_32=word_read(addr)) != seed_32)
				errlog(addr, tmp_32, seed_32);				
		
		addr += 4;
	}

	memset(Addr, Length, 0xff);
	// scanning mode
	for(i=0,addr=Addr; i< Length; i+=4)
	{
			
		byte_write(addr, 	(addr));
		byte_write(addr+1, 	(addr+1));
		byte_write(addr+2, 	(addr+2));
		byte_write(addr+3, 	(addr+3));	
		addr += 4;
	}
	
	
	for(i=0,addr=Addr; i< Length; i+=4)
	{
			
		seed_16 = addr2seed_16(addr);	
		if ((tmp_16=hw_read(addr)) != seed_16) 
				errlog(addr, tmp_16, seed_16);	

		seed_16 = addr2seed_16((addr+2));	
		if ((tmp_16=hw_read(addr+2)) != seed_16)
				errlog(addr, tmp_16, seed_16);	
		seed_32 = addr2seed_32((addr));	
		if ((tmp_32=word_read(addr)) != seed_32)
				errlog(addr, tmp_32, seed_32);				
		
		addr += 4;
	}
	
}



void hw_intf(unsigned long Addr, int Length)
{
	unsigned int i;	
	unsigned long addr;
	
	unsigned char   tmp_8, seed_8;
	unsigned short 	tmp_16,seed_16;
	unsigned long	tmp_32,seed_32;
	
	
	//sequentail mode:
	for(i=0,addr = (Addr); i< Length; i+=4)
	{
		
		seed_16 = 	addr2seed_16(addr);	
		hw_write(addr, 	seed_16);
		
		seed_16 = 	addr2seed_16(addr+2);	
		hw_write(addr+2, 	seed_16);
		

		tmp_8 = byte_read(addr);
		if (tmp_8 != ((addr) & 0xff))
			errlog(addr, tmp_8, ((addr) & 0xff));
			
		tmp_8 = byte_read(addr+1);
		if (tmp_8 != ((addr+1) & 0xff))
			errlog(addr, tmp_8, ((addr+2) & 0xff));
			
			
		tmp_8 = byte_read(addr+2);
		if (tmp_8 != ((addr+2) & 0xff))
			errlog(addr, tmp_8, ((addr+2) & 0xff));
			
			
		tmp_8 = byte_read(addr+3);
		if (tmp_8 != ((addr+3) & 0xff))
			errlog(addr, tmp_8, ((addr+3) & 0xff));
					
		tmp_32 = word_read(addr);
		
		
		seed_16 = hw_read(addr);
		if ( (tmp_32 >> 16) != seed_16)
			errlog(addr, (tmp_32 >> 16), seed_16);
			
			
		seed_16 = hw_read(addr+2);
		if ( (tmp_32 & 0xffff) != seed_16)
			errlog(addr, (tmp_32 & 0xffff), seed_16);
				
		addr += 4;
	
	}
	
	
	memset(Addr, 0x00, Length);

	// scanning  mode
	for(i=0,addr = (Addr); i< Length; i+=4)
	{
		
		seed_16 = 	addr2seed_16(addr);	
		hw_write(addr, 	seed_16);
		
		seed_16 = 	addr2seed_16(addr+2);	
		hw_write(addr+2, 	seed_16);
	
		addr += 4;	
	}
	
	for(i=0,addr = (Addr); i< Length; i+=4)
	{	

		tmp_8 = byte_read(addr);
		if (tmp_8 != ((addr) & 0xff))
			errlog(addr, tmp_8, ((addr) & 0xff));
			
		tmp_8 = byte_read(addr+1);
		if (tmp_8 != ((addr+1) & 0xff))
			errlog(addr, tmp_8, ((addr+2) & 0xff));
			
			
		tmp_8 = byte_read(addr+2);
		if (tmp_8 != ((addr+2) & 0xff))
			errlog(addr, tmp_8, ((addr+2) & 0xff));
			
			
		tmp_8 = byte_read(addr+3);
		if (tmp_8 != ((addr+3) & 0xff))
			errlog(addr, tmp_8, ((addr+3) & 0xff));
					
		tmp_32 = word_read(addr);
		
		
		seed_16 = hw_read(addr);
		if ( (tmp_32 >> 16) != seed_16)
			errlog(addr, (tmp_32 >> 16), seed_16);
			
			
		seed_16 = hw_read(addr+2);
		if ( (tmp_32 & 0xffff) != seed_16)
			errlog(addr, (tmp_32 & 0xffff), seed_16);
				
		addr += 4;
	
	}
	
	
}


void word_intf(unsigned long Addr, int Length)
{
	unsigned int i;	
	unsigned long addr;
	
	unsigned char   tmp_8, seed_8;
	unsigned short 	tmp_16,seed_16;
	unsigned long	tmp_32,seed_32;
	
	
	// sequential mode:
	for(i=0,addr = Addr; i< Length; i+=4)
	{
		
		seed_32 = 	addr2seed_32(addr);	
		word_write(addr, 	seed_32);
		

		tmp_8 = byte_read(addr);
		if (tmp_8 != ((addr) & 0xff))
			errlog(addr, tmp_8, ((addr) & 0xff));
			
		tmp_8 = byte_read(addr+1);
		if (tmp_8 != ((addr+1) & 0xff))
			errlog(addr, tmp_8, ((addr+1) & 0xff));
			
			
		tmp_8 = byte_read(addr+2);
		if (tmp_8 != ((addr+2) & 0xff))
			errlog(addr, tmp_8, ((addr+2) & 0xff));
			
			
		tmp_8 = byte_read(addr+3);
		if (tmp_8 != ((addr+3) & 0xff))
			errlog(addr, tmp_8, ((addr+3) & 0xff));
					
		tmp_16 = hw_read(addr);
		
		
		seed_32 = word_read(addr);
		if ( (seed_32 >> 16) != tmp_16)
			errlog(addr, (seed_32 >> 16), tmp_16);
			
			
		tmp_16 = hw_read(addr+2);
		if ( (seed_32 & 0xffff) != tmp_16)
			errlog(addr, (seed_32 & 0xffff), tmp_16);
				
		addr += 4;
	
	}
	memset(Addr, 0x55, Length);	
	
	
	for(i=0,addr = Addr; i< Length; i+=4)
	{
		seed_32 = 	addr2seed_32(addr);	
		word_write(addr, 	seed_32);
		addr += 4;
	}
	
	for(i=0,addr = Addr; i< Length; i+=4)
	{	

		tmp_8 = byte_read(addr);
		if (tmp_8 != ((addr) & 0xff))
			errlog(addr, tmp_8, ((addr) & 0xff));
			
		tmp_8 = byte_read(addr+1);
		if (tmp_8 != ((addr+1) & 0xff))
			errlog(addr, tmp_8, ((addr+1) & 0xff));
			
			
		tmp_8 = byte_read(addr+2);
		if (tmp_8 != ((addr+2) & 0xff))
			errlog(addr, tmp_8, ((addr+2) & 0xff));
			
			
		tmp_8 = byte_read(addr+3);
		if (tmp_8 != ((addr+3) & 0xff))
			errlog(addr, tmp_8, ((addr+3) & 0xff));
					
		tmp_16 = hw_read(addr);
		
		
		seed_32 = word_read(addr);
		if ( (seed_32 >> 16) != tmp_16)
			errlog(addr, (seed_32 >> 16), tmp_16);
			
			
		tmp_16 = hw_read(addr+2);
		if ( (seed_32 & 0xffff) != tmp_16)
			errlog(addr, (seed_32 & 0xffff), tmp_16);
				
		addr += 4;
	
	}
	
}

void memintf()
{

	extern unsigned int free_memory_start;
	extern unsigned int free_memory_size;
	extern unsigned int loop;

	unsigned int i;
	unsigned char   tmp_8, seed_8;
	unsigned short 	tmp_16,seed_16;
	unsigned long	tmp_32,seed_32;
	unsigned long addr;
	int	regvalue;
	
	
	LENGTH = free_memory_size;

	START_ADDR = free_memory_start;	
	
	prom_printf("Beginning of MemInterface Test...\n  START_ADDR=%X, LENGTH=%X\n", START_ADDR,LENGTH); 
	
	// byte write, and hw word read , word read for each address


	byte_intf(START_ADDR,LENGTH);
	
	memset(START_ADDR, 0x12, LENGTH);

	byte_intf((START_ADDR | 0x20000000),LENGTH);
	memset(START_ADDR, 0x34, LENGTH);
	

	prom_printf("Byte_Interface Test finished!\n");
	
	hw_intf( START_ADDR,  LENGTH);
	memset(START_ADDR, 0x56, LENGTH);
	
	hw_intf(( START_ADDR | 0x20000000),  LENGTH);
	memset(START_ADDR, 0x78, LENGTH);
	
	
	prom_printf("HWord_Interface Test finished!\n");
	

	word_intf(START_ADDR, LENGTH);
	memset(START_ADDR, 0x89, LENGTH);
	
	word_intf(( START_ADDR | 0x20000000),  LENGTH);


	printf("MemInterface Test ok...\n");



}



