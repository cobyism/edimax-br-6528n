/* --------------------------------------------------------------------
 *
 * Copyright 2002 by Realtek Semiconductor Corp.
 *
 * --------------------------------------------------------------------*/

#include <asm/io.h>
#include <linux/config.h> //JSW

#if defined(RTL8196B)
#include <asm/rtl8196x.h>
#endif
#if defined(RTL8198)
#include <asm/rtl8198.h>
#endif




#define MAX_SEC_SIZE		64*1024		// 64K bytes

#if defined(RTL8196B) || defined(RTL8198)
#define NFCR		    0xb8001100 // JSW:memory contoller register for 8672
#define FLASH_256K		0xfff88000	// 256k bytes
#define FLASH_512K		0xfff98000	// 512k bytes
#define FLASH_1M		0xfffa8000	// 1M bytes
#define FLASH_2M		0xfffb8000	// 2M bytes
#define FLASH_4M		0xfffc8000	// 4M bytes
#define FLASH_8M		0xfffd8000	// 8M bytes
#define FLASH_16M		0xfffe8000	// 8M bytes
#else //RTL8196B, RTL8198

#define FLASH_MASK		0x3fffffff

/*Cyrus Tsai*/
/*#define FLASH_1M		0x80000000	// 1M bytes*/
/*#define FLASH_2M		0xC0000000	// 2M bytes*/
/*Cyrus Tsai*/

#define FLASH_1M		0x00000000	// 1M bytes
#define FLASH_2M		0x40000000	// 2M bytes
#define FLASH_4M		0x80000000	// 4M bytes
#define FLASH_8M		0xC0000000	// 8M bytes
#endif //RTL8196B, RTL8198

//#define CPU_CLOCK		(20*1024*1024)		// CPU 20MHZ
//#define WAIT_TIME_FLASH_ERASE 	(20*CPU_CLOCK)		// 20 seconds
//#define WAIT_TIME_FLASH_WRITE 	(1*CPU_CLOCK)		// 1 seconds

/* Manufacturers */
#define MANUFACTURER_MXIC	0x00C2
#define MANUFACTURER_AMD    0x0001
#define MANUFACTURER_ST     0x0020
#define MANUFACTURER_ESI	0x004A
#define MANUFACTURER_ESMT	0x008C
#define MANUFACTURER_SAMSUNG	0x00EC
#define MANUFACTURER_INTEL  	0x0089	

/* MXIC deivce id */
#define MX29LV800B		0x225B
#define MX29LV160AB		0x2249
#define MX29LV320CT		0x22A7 //wei add, for 8651c use flash
#define MX29LV320AB		0x22A8
#define MX29LV640AB		0x22CB//Add by vin
#define MX29LV1280AB		0x22EB//Add by JSW
#define MX29LV1280DB		0x227a//Add by JSW (Bottom) 16MB
#define MX29LV1280DT		0x227E//Add by JSW (TOP) 16MB

/* AMD deivce id */
#define AM29LV800BB     0x225B
#define AM29LV160DB     0x2249
#define AM29LV320DB     0x22F9

/* ST device id */             
#define M29W160DB       0X2249 

/* Intel device id */
#define TE28F160C3      	0x88C3 

/* ESI device id */
#define ES29LV320D		0x22F9

/* ESMT device id */
#define F49L160BA		0x2249

/* SAMSUNG device id */
#define K8D1716UBC		0x2277

/* SST deivce id */
//#define MANUFACTURER_SST    0x00BF
//#define SST39VF800     0x2781
//#define SST39VF160     0x2782
//#define SST39VF320     0x2783


struct erase_region_info {
	int offset;
	int erasesize;
	int numblocks;
};

struct flash_desc {
	unsigned short mfr_id;
	unsigned short dev_id;
	int size;
	int shift;
	int numregion;
	struct erase_region_info regions[4];
};

struct flash_map {
	int totalsize;
	int chipnum;
	int chipshift;
	int numregion;
	struct erase_region_info regions[8];
};


static struct flash_desc table[]=
{
	#ifndef SPI_FLASH
	  {
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV800B,
		size: 0x00100000,
		shift: 20,/*20 bit=> that is 1 MByte size*/
		numregion: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	  },

	  {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV800BB,
		size: 0x00100000,
		shift: 20,/*20 bit=> that is 1 MByte size*/
		numregion: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	  },
	  {
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV160AB,
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numregion: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},	  {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV160DB,
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numregion: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	        {                                                               
		mfr_id: MANUFACTURER_ST,                                        
		dev_id: M29W160DB,                                              
	        size: 0x00200000,                                               
		shift: 21,/*21 bit=> that is 2 MByte size*/                     
		numregion: 4,                                                   
		regions: {                                                      
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 } 
		}                                                               
	},                                                                      
	        {
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV320AB,
		size: 0x00400000,
		shift: 22,/*22 bit=> that is 4 MByte size*/
		numregion: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  63}
		}
	  },
	  		  {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV320DB,
		size: 0x00400000,
		shift: 22,/*22 bit=> that is 4 MByte size*/
		numregion: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  63}
		}
	  }
		//Intel table
	       ,{                                                               
		mfr_id: MANUFACTURER_INTEL,                                        
		dev_id: TE28F160C3,                                              
	        size: 0x00200000,                                               
		shift: 21,/*21 bit=> that is 2 MByte size*/                     
		numregion: 2,                                                   
		regions: {                                                      
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 } 
		}                                                               
	}       
		//ESI table
	       ,{                                                               
		mfr_id: MANUFACTURER_ESI,
		dev_id: ES29LV320D,
		size: 0x00400000,
		shift: 22,/*22 bit=> that is 4 MByte size*/
		numregion: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  63}
		}                                                              
	}     	
		//SAMSUNG table
	       ,{                                                               
		mfr_id: MANUFACTURER_SAMSUNG,
		dev_id: K8D1716UBC,
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numregion: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  31}
		}                                                              
	}     
		//ESMT table
	       ,{                                                               
		mfr_id: MANUFACTURER_ESMT,
		dev_id: F49L160BA,
		size: 0x00200000,
		shift: 21,/*22 bit=> that is 4 MByte size*/
		numregion: 4,
		regions: {                                                      
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 } 
		}                                                              
	}     
	
	  #if 0
	  ,
	    {
		mfr_id: MANUFACTURER_SST,
		dev_id: SST39VF800,
		size: 0x00100000,
		shift: 20,/*22 bit=> that is 1 MByte size*/
		numregion: 1,
		regions: {
			{ offset: 0x000000, erasesize: 0x00800, numblocks:  512 }
		}
	  },
	  	  		  {
		mfr_id: MANUFACTURER_SST,
		dev_id: SST39VF160,
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numregion: 1,
		regions: {
			{ offset: 0x000000, erasesize: 0x00800, numblocks:  1024 }
		}
	  },
	  {
		mfr_id: MANUFACTURER_SST,
		dev_id: SST39VF320,
		size: 0x00400000,
		shift: 22,/*22 bit=> that is 4 MByte size*/
		numregion: 1,
		regions: {
			{ offset: 0x000000, erasesize: 0x00800, numblocks:  2048 }
		}
	  }
     #endif

		,{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV640AB,
		size: 0x00800000,
		shift: 23,/*22 bit=> that is 8 MByte size*/
		numregion: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  127}
		}
		}
#if defined(RTL8196B) || defined(RTL8198)
	 	,{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV1280DB,
		size: 0x01000000,
		shift: 24,/*24 bit=> that is 16 MByte size*/
		numregion: 2,
		regions: {			
			{ offset: 0x00000000, erasesize: 0x02000, numblocks:  8 }, //8KB //JSW
			{ offset: 0x00010000, erasesize: 0x10000, numblocks:  255} //
		}
		}
#endif
#endif
};



static struct flash_map map;
static unsigned short mfid, devid;
unsigned int flash_buf[MAX_SEC_SIZE>>2];

static unsigned long  get_sector(unsigned long addr);
static unsigned int get_size_by_sector(unsigned long addr);
static int flash_sector_erase(unsigned long addr);
static int flash_sector_program(unsigned long addr , unsigned char *buf, unsigned int size);
static int program_word(unsigned long addr, unsigned short prog_data, unsigned int offset);

/*-----------------------------------------------------------------------
  flasinit(): probe flash memory by reading manufacture and device id. If
	found, try to probe 2nd flash to see if it exists or not. Last,
	map the flash table to determine size, offset, and erase block.
-------------------------------------------------------------------------*/
#ifndef SPI_FLASH
int flashinit()    
{
	int i, j, k, interleave=1, chipsize;
	unsigned short mfid_2nd, devid_2nd;
	unsigned long tmp;


	// issue reset and auto-selection command
	rtl_outw(FLASH_BASE, 0xf0);

	rtl_outw(FLASH_BASE + 0x555 * 2, 0xaa);
	rtl_outw(FLASH_BASE + 0x2aa * 2, 0x55);
	rtl_outw(FLASH_BASE + 0x555 * 2, 0x90);

	mfid = rtl_inw(FLASH_BASE);
	devid = rtl_inw(FLASH_BASE + 1*2);

	prom_printf("mfid=%x devid=%x\n",mfid,devid);
	rtl_outw(FLASH_BASE, 0xf0);


	for (i=0; i< sizeof(table)/sizeof(table[0]); i++) {
		if ( mfid==table[i].mfr_id && devid==table[i].dev_id)
			break;
	}
	if ( i == sizeof(table)/sizeof(table[0]) ) {
		prom_printf("Probe flash memory failed!\n");
		return 0;
	}

#if defined(RTL8196B) || defined(RTL8198)
	//JSW: Set flash size into memory controller
	tmp = *((volatile unsigned long *)NFCR);
        
	switch (table[i].size) {	
	case (256*1024):
		tmp =  FLASH_256K;
		break;
	case (512*1024):
		tmp =  FLASH_512K;
		break;
	case (1*1024*1024):
		tmp =  FLASH_1M;
		break;
	case (2*1024*1024):
		tmp =  FLASH_2M;
		break;
	case (4*1024*1024):
		tmp =  FLASH_4M;
		break;
	case (8*1024*1024):
		tmp =  FLASH_8M;
		break;
	case (16*1024*1024):
		tmp =  FLASH_16M;
		break;
			}	

	*((volatile unsigned long *)NFCR) = tmp;
#else //RTL8196B, RTL8198

	// Set flash size into memory controller
	tmp = *((volatile unsigned long *)MEM_CONTROLLER_REG);
        
        /*Cyrus Tsai change to switch case for higher flash size*/
#if 0
	if ( table[i].size == 0x100000 )
		tmp = (tmp & FLASH_MASK) | FLASH_1M;
	else
		tmp = (tmp & FLASH_MASK) | FLASH_2M;
#endif

	switch (table[i].size) {	
	case (1*1024*1024):
		tmp = (tmp & FLASH_MASK) | FLASH_1M;
		break;
	case (2*1024*1024):
		tmp = (tmp & FLASH_MASK) | FLASH_2M;
		break;
	case (4*1024*1024):
		tmp = (tmp & FLASH_MASK) | FLASH_4M;
		break;
	case (8*1024*1024):
		tmp = (tmp & FLASH_MASK) | FLASH_8M;
		break;
			}	

	//*((volatile unsigned long *)MEM_CONTROLLER_REG) = tmp;
#endif //RTL8196B, RTL8198

	// Look for 2nd flash
	
//	prom_printf("The flash size: %x \n", table[i].size);	
//	prom_printf("2nd flash address: %x \n", FLASH_BASE + table[i].size);
	rtl_outw(FLASH_BASE + table[i].size, 0xf0);

	rtl_outw(FLASH_BASE + table[i].size + 0x555 * 2, 0xaa);
	rtl_outw(FLASH_BASE + table[i].size + 0x2aa * 2, 0x55);
	rtl_outw(FLASH_BASE + table[i].size + 0x555 * 2, 0x90);

	mfid_2nd = rtl_inw(FLASH_BASE + table[i].size);
	devid_2nd = rtl_inw(FLASH_BASE + table[i].size + 1*2);

//	prom_printf("mfid_2nd=%x devid_2nd=%x\n",mfid_2nd,devid_2nd);
	rtl_outw(FLASH_BASE + table[i].size, 0xf0);

	if ( mfid_2nd==table[i].mfr_id && devid_2nd==table[i].dev_id) {
		interleave++;
	}
        /*here add a return for an un-proper case*/
        //else
        //return 0;

	prom_printf("Found %d x %dM flash memory\n", interleave, table[i].size/0x100000);

	map.totalsize = table[i].size*interleave;
	map.numregion = table[i].numregion*interleave;
	map.chipnum = interleave;
	map.chipshift = table[i].shift;

	for (k=0, chipsize=0; interleave>0; interleave--, chipsize+=table[i].size) {
		for (j=0; j<table[i].numregion; j++, k++) {
			map.regions[k].offset = table[i].regions[j].offset+chipsize;
			map.regions[k].erasesize = table[i].regions[j].erasesize;
			map.regions[k].numblocks = table[i].regions[j].numblocks;
		}
	}
	return 1;
}
#endif

/*-----------------------------------------------------------------------
flashread: read from flash (src) to (dst) with byte counts==length
since flash can only be accessed by 16-bits mode, therefore, we set limits
on dst/src to be 16-bits alignment.
------------------------------------------------------------------------*/

/*Cyrus Tsai*/
/*dst at SDRAM, src at FLASH, when using SDRAM, only Byte access,or memcpy*/
/*dst at SDRAM, src at FLASH, when using FLASH, only 16Bit access         */
#ifndef SPI_FLASH

unsigned int nor_get_ctimestamp( void );
void nor_srand2( unsigned int seed );
unsigned int nor_rand2( void );
void auto_nor_memtest(unsigned long dram_test_starting_addr );



int flashread (unsigned long dst, unsigned int src, unsigned long length)
{
 Int16 tmp=0;
 if (length==0)
 return 1;
                                           /* It is the first byte              */
 if (src & 0x01)                          /* FLASH should be 16Bit alignment   */
    {
  					  /* FLASH 16bit alignment             */
     tmp = rtl_inw(FLASH_BASE + src - 1); /* FLASH 16bit access                */
     tmp = tmp & 0x00ff;
     *(Int8*)(dst) = (Int8)tmp;           /* Byte access SDRAM                 */  
     
     //prom_printf("%X,%X,%X,%X\n",dst,src,length,*(Int8*)(dst));

     dst=dst+1;                           /* Finish the First Byte             */
     src=src+1;                           /* Finish the First Byte             */
     length=length-1;                     /* Finish the First Byte             */
    }

   while(length)
      {
       if(length == 1)                    
         {
	  tmp = rtl_inw(FLASH_BASE + src);
	  *(Int8*)(dst)=(Int8)( ((tmp >> 8) & 0x00ff) );
	//  prom_printf("%X,%X,%X,%X\n",dst,src,length,*(Int8*)(dst));
          break;
         }

       tmp=rtl_inw(FLASH_BASE + src);     // From now on, src 16Bit aligned
                                          // FLASH 16bit access                 
                                          //if(src&0x01)
                                          //prom_printf("error");
                                          //if(length<2)
                                          //prom_printf("error");                                            
       memcpy((Int8*)dst,&tmp,2);         //use memcpy, avoid SDRAM alignement 
     
       //prom_printf("%X,%X,%X,%X\n",dst,src,length,*(unsigned short*)(dst));
     
       dst=dst+2;
       src=src+2;
       length=length-2;
      }
   return 1;
}


/*-----------------------------------------------------------------------
For flash_write operation, we don't set any limit on dst/src to be 16-bits
alignment.
------------------------------------------------------------------------*/
/*dst at FLASH, src at SDRAM*/
/*when using SDRAM, we must use Byte access or memcpy  */
/*when using FLASH, we must use 16Bit access           */

//int flashwrite(unsigned long FLASH, unsigned long SDRAM, unsigned long length)

//JSW PS.Max single write is 64KB(0x0001 0000)
int flashwrite(unsigned long dst, unsigned long src, unsigned long length)
{
	unsigned int sec_size,offset;
	unsigned long sec_addr;
	int res=0;
        
        if (length==0)
 	return 1;
/**********************************************************************************/	
	while(length)
	{
 	    sec_addr = get_sector(dst);/*Now dst must equal sec_addr*/
  	    sec_size = get_size_by_sector(sec_addr);

            if (dst != sec_addr)
  	       offset = dst - sec_addr;
 	    else
	       offset = 0;

	   //prom_printf("program offset at %X\n", offset);

            if ((flashread ((unsigned long)(flash_buf), sec_addr, sec_size)) == 0)
			goto write_end;

 	    if (length < (sec_size-offset))
		{
	         memcpy((void *)((int)flash_buf+(int)offset), (unsigned char *)src, length);
		 length = 0; /*last sector to deal with*/
		}
   	    else
		{/*still need to deal with next sector*/
 		 memcpy((void *)((int)flash_buf+(int)offset), (unsigned char *)src, sec_size-offset);
		 length = length-(sec_size-offset);;
		 src += sec_size-offset;
		 dst += sec_size-offset;
		}
		
		if (!(flash_sector_erase(sec_addr)))
		{
			//prom_printf("sector_erase fails! at sec_addr=%X\n");			
			goto write_end;
		}	
		if (!(flash_sector_program(sec_addr , (unsigned char *)flash_buf, sec_size)))
		{
			//prom_printf("sector_program fails! at sec_addr=%X, sec_size=%X\n", sec_addr, sec_size);
			goto write_end;
		}
	}

	res=1;	
	
write_end:
        return res;	

}




/*-------------------------------------------------------------
 *  Local routines
 *-------------------------------------------------------------*/

static unsigned long  get_sector(unsigned long addr)
{
	int i,j;

	unsigned long saddr = 0;

	if (addr >= map.totalsize)
		goto sect_find;

	i=0;
	while(i < map.numregion)
	{
		for(j=0; j< map.regions[i].numblocks; j++)
		{
			if ((addr >= (map.regions[i].offset +
			    ((j) * map.regions[i].erasesize))) &&
			    (addr < (map.regions[i].offset +
			    ((j+1) * map.regions[i].erasesize))))
				goto sect_find;

			saddr += map.regions[i].erasesize;
		}
		i++;
	}


	if ( i == map.numregion )
		saddr = (unsigned long)-1;
sect_find:
	return saddr;
}

#if 1
static unsigned int get_size_by_sector(unsigned long addr)
{

	int i,j;
	unsigned int saddr ;
	for(i=0,saddr=0; i< map.numregion; i++)
	{
		for(j=0; j< map.regions[i].numblocks;j++)
		{
			if (saddr == addr)
				return	map.regions[i].erasesize;
			saddr += map.regions[i].erasesize;
		}
	}

	return 0;
}
#endif

static int flash_sector_erase(unsigned long addr)
{
	unsigned long i, j;
	int res=0, offset;

	if (addr >= map.totalsize)
		goto erase_end;

	offset = (addr >> map.chipshift) * (1 << map.chipshift);

	prom_printf(".");

	if(mfid==MANUFACTURER_INTEL && devid==TE28F160C3) {
		//do unlock for all blocks 
		rtl_outw(FLASH_BASE+offset, 0x60);
		rtl_outw(FLASH_BASE+addr, 0xd0);
		//block erase command
		rtl_outw(FLASH_BASE + offset, 0x20);
		rtl_outw(FLASH_BASE + addr, 0xd0);
	
	// + start + this is for INTEL flash
	        i=0xffffff0;
		while(!(rtl_inw(FLASH_BASE+addr) & 0x80)) 
			i -- ;
		if(!i)
			return 0;
		// issue read command
		rtl_outw(FLASH_BASE + addr, 0xff);
	// - end - 
	} else {

		rtl_outw(FLASH_BASE + offset + 0x555*2, 0xf0); // for reset
	for(i=0; i< 0x100; i++);
		
		rtl_outw(FLASH_BASE + offset + 0x555*2, 0xaa);
		rtl_outw(FLASH_BASE + offset + 0x2aa*2, 0x55);
		rtl_outw(FLASH_BASE + offset + 0x555*2, 0x80);
#ifdef CONFIG_RTL8198
		asm volatile ("nop\n\t");// add nop fix test chip cpu 5281 bug, formal chip is ok. advised by yen@rtk
#endif
		rtl_outw(FLASH_BASE + offset + 0x555*2, 0xaa);
		rtl_outw(FLASH_BASE + offset + 0x2aa*2, 0x55);

		rtl_outw(FLASH_BASE + addr, 0x30);
	}
        i=0xffffff0;
	while (i)
	{
		j=rtl_inw(FLASH_BASE +  addr);
  		if (j == 0xffff)
			break;
	}
	if (i)
		res = 1;
erase_end:

	return res;
}
#if 1
/*--------------------------------------------------------/
addr is the starting addr of a given sector,
and buf will be the source address for programming.
size is the byte count to be programmed.
---------------------------------------------------------*/
static int flash_sector_program(unsigned long addr , unsigned char *buf, unsigned int size)
{
	unsigned int  i;
	unsigned int status;
	unsigned int res=0, offset;

	if (addr >= map.totalsize)
		goto program_end;

	offset = (addr >> map.chipshift) * (1 << map.chipshift);

//	rtl_outw(FLASH_BASE + offset + 0x555*2, 0xf0); // for reset
//	for(i=0; i< 0x100; i++);

	for(i=0;i< size; i+=2,addr+=2,buf+=2) // our flash is implemented as 16-bits mode
	{

		status=program_word(addr,*(unsigned short*)(buf), offset);
		if(!status)
		{
//			prom_printf("program_word fails! at addr=%X, data=%X\n", addr,*(unsigned short*)(buf));
			goto program_end;
		}

	}
	res=1;
program_end:
	return res;
}
#endif
static int program_word(unsigned long addr, unsigned short prog_data, unsigned int offset)
{
	unsigned short j;
	unsigned long flags,i;
         

	if(mfid==MANUFACTURER_INTEL && devid==TE28F160C3) {
		//program command
		rtl_outw(FLASH_BASE + offset , 0x40);
		rtl_outw(FLASH_BASE + addr, prog_data);
	// + start + this is for INTEL flash
		i=0xffffff0;
		while(i) {
			if(rtl_inw(FLASH_BASE+offset) & 0x80)
				break;
			i --;
        }
		if(!i)
			return 0;
		// issue read command
       		rtl_outw(FLASH_BASE + offset , 0xff);
	// - end -
	} else {
		rtl_outw(FLASH_BASE + offset + 0x555*2, 0xaa);
		rtl_outw(FLASH_BASE + offset + 0x2aa*2, 0x55);
		rtl_outw(FLASH_BASE + offset + 0x555*2, 0xa0);
		rtl_outw(FLASH_BASE + addr, prog_data);
	}

	i=0xffffff0;
	
	while(i)
	{
		j=rtl_inw(FLASH_BASE	+addr);

		if (j == prog_data )
		{
			//prom_printf("i=%X\n",i);
			break;
		}
          i--;
	}

	if (i)
		return 1;
	else
		return 0; //fail
}




//==================================
//JSW :Below 3 functions for generating SPI/DRAM random data source,just using for verification
unsigned int nor_rtl_seed = 0xDeadC0de;

#define nor_using_timer0_as_seed 1

unsigned int nor_get_ctimestamp( void )
{
      unsigned int temp;
#if nor_using_timer0_as_seed	
	temp= (READ_MEM32(0xb8003108));//TC0CNT;
	return temp;
	//return 0xe8a00;
#else
	return 0x90900;//if verify error,change this value as temp seed's data  
	/*error data record 
	    1.(0x3c2a00)
	*/
#endif
}

//Set rseed
void nor_srand2( unsigned int seed )
{
	nor_rtl_seed = seed;
}

//Get Random function
unsigned int nor_rand2( void )
{
	unsigned int hi32, lo32;

	hi32 = (nor_rtl_seed>>16)*19;
	lo32 = (nor_rtl_seed&0xffff)*19+37;
	hi32 = hi32^(hi32<<16);
	return ( nor_rtl_seed = (hi32^lo32) );
}
//==================================


#endif


#ifdef CONFIG_NOR_TEST
//JSW:for NOR/SDRAM auto-memory-test
//void auto_memtest(unsigned char *start,unsigned char *end)
void auto_nor_memtest( unsigned long dram_test_starting_addr )
{

	int i, j;
	unsigned int nor_data,nor_rseed_timer;
       static unsigned short probe_cnt=0;
	dprintf("\n=============================\n");
	dprintf("\n====> NOR Device Size (MB): %d\n", table[probe_cnt].size/0X100000);

	//printf("probe_cnt=%d\n",probe_cnt);
	//printf("table[probe_cnt].size=%x\n",table[probe_cnt].size);


	//srand2(get_ctimestamp());
	nor_rseed_timer=nor_get_ctimestamp();
	printf("nor_rseed_timer=0x%x\n",nor_rseed_timer);
	nor_srand2(nor_rseed_timer);

	#if 0
	// Set NOR Parameter
	WRITE_MEM32(NFCR, SPI_SFSIZE(spi_flash_info[0].device_size - 17) \
					| SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) \
					| SPI_TCS(15) | SPI_RD_MODE(1) | SPI_RD_OPT(0) );

	//printf("After WRITE_MEM32....\n");
	#endif
	dprintf("Check NFCR(0xb8001100) =%x\n",*(volatile unsigned int *) NFCR); 

	dprintf("\n=============================\n");



	   /*JSW :SFCR setting reference table
		
#define FLASH_256K		0xfff88000	// 256k bytes
#define FLASH_512K		0xfff98000	// 512k bytes
#define FLASH_1M			0xfffa8000	// 1M bytes
#define FLASH_2M			0xfffb8000	// 2M bytes
#define FLASH_4M			0xfffc8000	// 4M bytes
#define FLASH_8M			0xfffd8000	// 8M bytes
#define FLASH_16M		0xfffe8000	// 16M bytes

	   */

	 
#if 1
	// Writing random data to DRAM
	for (i = 0; i < (table[probe_cnt].size); i += 4)
	{
		if ((i + 4) % 0x10000 == 4)
		{
			printf("==> DRAM 0x%08X\n", dram_test_starting_addr + i);
		}

		//JSW:Write the flash-size's(ex:2MB) random data into DRAM 
		//unsigned int nor_rand3=nor_rand2();
		//printf("nor_rand3=%08X\n", nor_rand2());
		WRITE_MEM32(dram_test_starting_addr + i, nor_rand2());

		//printf(" DRAM dram_test_starting_addr + (i)%d=%x\n",i, *(volatile unsigned long*)dram_test_starting_addr + i);

		
		
				
	}
	
	
	printf("1. Erase Whole Chip and Burn Code\n");

	flashwrite(0, (unsigned long)dram_test_starting_addr,(table[probe_cnt].size));

	//Comparing DRAM data to burning NOR data
	printf("\n2.Comparing DRAM to burning NOR's data\n");
	
	for (i = 0; i < (table[probe_cnt].size); i += 4)
	{
		if (READ_MEM32(dram_test_starting_addr + i) != READ_MEM32(0xbd000000 + i) )
			{
				printf("Verify Fail! Flash Address: 0x%x, Burned NOR-data:0x%x \
				 \n",0xbd000000+i, READ_MEM32(0xbd000000 + i));		
				printf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
				   \n",dram_test_starting_addr+i,READ_MEM32(dram_test_starting_addr + i));
				return 0;//halt();
			}
	}

	
	
	//spi_burn_image(0, (unsigned char *)dram_test_starting_addr, (table[probe_cnt].size));

	printf("3. Re-Comparing burning NOR to DRAM's data by 32/16/8 Bit Access \n\n"); 
	
	for (i = 0; i < (table[probe_cnt].size); i += 4)
	{
	
		unsigned int dram_addr = dram_test_starting_addr + i;
		unsigned int nor_addr = 0xBD000000 + i;

		for (j = 0; j < 4; j++)
		{
			// 32 Bit Access
			if (j == 0)
			{
	//JSW:Check by eyes if you want
				#if 0
					while(i<=20)
					{
					printf("32 Bit Access, i=%d,j=%d\n\n",i,j);
					printf("Verify OK! NOR_addr: 0x%x, Burned NOR-data:0x%x \
						 \n\n",0xbd000000+i, READ_MEM32(nor_addr+j));		
					printf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
						   \n\n",dram_test_starting_addr+i,READ_MEM32(dram_addr +j));
					break;
					}
				#endif

					if (READ_MEM32(dram_addr) != READ_MEM32(nor_addr))
					{
						printf("32-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
							 nor_addr, READ_MEM32(nor_addr), READ_MEM32(dram_addr));
						return 0;//halt();
					}
			}

			// 16 Bit Access
			if ((j == 0) || (j == 2))
			{

				#if 0//JSW:Check by eyes if you want
				while(i<=20)
				{
					printf("16 Bit Access, i=%d,j=%d\n\n",i,j);
					printf("Verify OK! NOR_addr: 0x%x, Burned NOR-data:0x%x \
						 \n\n",0xbd000000+i, READ_MEM16(nor_addr+j));		
					printf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
						   \n\n",dram_test_starting_addr+i,READ_MEM16(dram_addr +j));
					break;
				}
				#endif
				if (READ_MEM16(dram_addr + j) != READ_MEM16(nor_addr + j))
				{
					printf("16-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
							 nor_addr, READ_MEM32(nor_addr), READ_MEM32(dram_addr));
					return 0;//halt();
				}
			}

			// 8 Bit Access
			if ((j == 0) || (j == 1)|| (j == 2)|| (j == 3))
			{

		#if 0 //JSW:Check by eyes if you want
		
				while(i<=20)
				{
					printf("8 Bit Access, i=%d,j=%d\n\n",i,j);
					printf("Verify OK! NOR_addr: 0x%x, Burned NOR-data:0x%x \
							 \n\n",0xbd000000+i, READ_MEM8(nor_addr+j));		
					printf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
							   \n\n",dram_test_starting_addr+i,READ_MEM8(dram_addr + j));
					break;
				}
		#endif
				if (READ_MEM8(dram_addr + j) != READ_MEM8(nor_addr + j))
				{
		
				printf("8-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
						 nor_addr, READ_MEM32(nor_addr), READ_MEM32(dram_addr));
				return 0;//halt();
				}
			}//end if j==0/1/2/3
		}
	}

	#if 0 //JSW:if get 2nd SPI flash,modify below
	printf("====> SPI #2 Device Size (KB): %d\n", 1 << (spi_flash_info[1].device_size - 10));

	// Set SDRAM
	for (i = 0; i < (1 << spi_flash_info[1].device_size); i += 4)
	{
		if ((i + 4) % 0x10000 == 4)
		{
			printf("==> DRAM 0x%08X\n", dram_test_starting_addr + i);
		}

		WRITE_MEM32(dram_test_starting_addr + i, rand2());
	}


	printf("1. Erase Whole Chip and Burn Code\n");
	spi_pio_init();	
	spi_burn_image(1, (unsigned char *)dram_test_starting_addr, (1 << spi_flash_info[1].device_size));
	//JSW:Burning DRAM's data into flash


	printf("2. Code Verification by Programmed I/O\n");
	spi_pio_init();
	for (i = 0; i < (1 << spi_flash_info[1].device_size); i += 4)
	{
		spi_read(1, i, &spi_data);
		
		if (READ_MEM32(dram_test_starting_addr + i) != spi_data) //JSW:Comparing DRAM's data to flash
		{
			printf("Verify Failed! Address: 0x%08X, Data: 0x%08X\n", i, spi_data);
			printf("SFCR Value: 0x%08X\n", READ_MEM32(SFCR));

			halt();
		}
	}

	printf("3. Code Verification by Memory Mapped I/O\n");
	printf("From 0x%08X ~ 0x%08X\n",
			 0xBD000000 + (1 << spi_flash_info[0].device_size),
			 0xBD000000 + (1 << spi_flash_info[0].device_size) \
			 + MIN((1 << spi_flash_info[1].device_size), (1 << spi_flash_info[0].device_size)) - 1);

	for (i = 0; i < MIN((1 << spi_flash_info[1].device_size), (1 << spi_flash_info[0].device_size)); i += 4)
	{
		unsigned int dram_addr = dram_test_starting_addr + i;
		unsigned int spi_addr = 0xBD000000 + (1 << spi_flash_info[0].device_size) + i;

		for (j = 0; j < 4; j++)
		{
			// 32 Bit Access
			if (j == 0)
			{
			  if (READ_MEM32(dram_addr) != READ_MEM32(spi_addr))
				{
					printf("32-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
							 spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));
					halt();
				}
			}

			// 16 Bit Access
			if ((j == 0) || (j == 2))
			{
				if (READ_MEM16(dram_addr + j) != READ_MEM16(spi_addr + j))
				{
					printf("16-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
							 spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));
					halt();
				}
			}

			// 8 Bit Access
			if (READ_MEM8(dram_addr + j) != READ_MEM8(spi_addr + j))
			{
				printf("8-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
						 spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));
				halt();
			}
		}
	}

	
}

#endif
	printf("\n=======> PASS! NOR/DRAM verify OK! \n\n");
#endif
}

#endif //end if CONFIG_NOR_TEST


