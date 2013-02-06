/*
 * Simple NAND FLASH testing program
 * block-boundary data read/erase/write
 */
#include <asm/io.h>
#include <asm/rtl8181.h>
#include "nand.h"
#define  MCR   0x1000
#define  MTCR0 0x1004

#define save_flags(x)	 do { } while(0) 
#define restore_flags(x) do { } while(0)
#define cli(x)		 do { } while(0)
#define printk	 	prom_printf
#define NANDRDY		((rtl_inl(NCR) & CE4_RDY) >> 31) 
#define nand_no		0	

//extern union task_union sys_tasks[NUM_TASK];

static int check_id(void);  
static int erase_block(int block_no);  
static int read_block(unsigned int from_block, unsigned char *mem_buf);  
static int write_block(unsigned int to, unsigned char *mem_buf);  
int read_data(int start,int length,unsigned char *target);  
int write_data(int start,int length,unsigned char *target);  
static int open_device(int chip_num);

static unsigned int nand_addr = 0;
static volatile unsigned char *block_buf;
static unsigned char *bkup_buf;
static unsigned char *total_buf;

static unsigned char index = 0;
static int wrongchip = 0;
static unsigned char writebuffer[BLOCKSIZE];

#if 0
void nandsimple_task(void)
{
	
	int	loop = 0;
	int     bknum;
	int 	bufnum;
	int 	test = 0; 
	
	printk("This is nandtest task init speaking\n");
	
	check_id();
		
		
	for(;;)
	{
#if NAND128	
		bknum = loop & 0x3ff;
#else
		bknum = loop & 0x7ff;
#endif

// we change the SDRAM buffer address regularily 
		bufnum = loop % 60;
// 
#if NAND128
		if(bknum == 0x3ff)  { 
			// here we skip block 0
			printk("Finish testing %dth of 1023 rounds\n",++test);	
#else
		if(bknum == 0x7ff)  { 
			// here we skip block 0
			printk("Finish testing %dth of 2047 rounds\n",++test);	
#endif
			loop ++;
			continue;	
		}
		read_block(1+bknum,block_buf);
		if(erase_block(1+bknum)) {
			read_block(1+bknum,block_buf);
		}
		else  { 
			// erase fail -> skip this block
			loop ++;	
			continue;
		}

		memcpy(bkup_buf,block_buf,32*528);    
		write_block(1+bknum,block_buf);
		memset(block_buf,(unsigned char)bknum,32*528);
		read_block(1+bknum,block_buf);

		if(memcmp(bkup_buf,block_buf,32*528))
			printk("copy data error\n");    
		else {  
		//	printk("\b");
		//	printk("%c",twiddle[loop%4]);
		}

		loop++;	
		
		//printk("Hello! This is %d-th nandtest_task speaking\n",loop++);
	}
}
#endif

static int check_id(void)
{
	unsigned int id_value;
	unsigned int addr;
	
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY);
	rtl_outl(NCAR, NAND_CMD_RESET(nand_no));

	// if device is still busy, pass control to other tasks
	while(!NANDRDY);
	rtl_outl(NCAR, NAND_CMD_READID(nand_no));	
	rtl_outl(NADDR, CE4_ADEN0);	

	id_value = rtl_inl(NDR);
	printk("Read ID value = %x\n",id_value);
	id_value = id_value >> 24; 
	if( (id_value & 0xff) != NAND_MFR_SAMSUNG) {
		printk("Wrong Manufacturer ID\n");
		wrongchip = 1;
	}
	// the meaning of this write?
	rtl_outl(NCR,0xb0201211);
	return 1;
}

/*
	input 	: block_no  - the erase block number
	return 	: -1 - device is protected
		   0 - erase fail
		   1 - erase successfully
 */
static int erase_block(int block_no)
{
	unsigned int in_addr;
	unsigned int row1, row2;
	unsigned int value;
	unsigned int flags;	
	
	// block = 32 page x 528 bytes
	row1 = (block_no << 5) & 0xff; // page number A0~A7
	row2 = (block_no >> 3) & 0xff; // page number A9~A16

	// STEP 1
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY);
	// get into command mode
	save_flags(flags); cli();
	rtl_outl(NCAR, NAND_CMD_RESET(nand_no));
	restore_flags(flags);
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY); 
	// STEP 2  			
	// check if write-protection
	save_flags(flags); cli();
	rtl_outl(NCAR, NAND_CMD_STATUS(nand_no));		
	if (!(rtl_inl (NDR) & WRITEPROTECT)) {
		printk("nand_write_ecc: Device is write protected!!!\n");
		restore_flags(flags);
		return -1;
	}		
	restore_flags(flags);


	// STEP 3  			
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY); 

	save_flags(flags); cli();
	in_addr = (CE4_ADEN1 | (row2 << 8)) | (CE4_ADEN0 | row1);
	rtl_outl(NCAR, NAND_CMD_ERASE1(nand_no));
	rtl_outl(NADDR,in_addr ); // 2 cycle address in
	restore_flags(flags);
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY); 
	save_flags(flags); cli();
	rtl_outl(NCAR, NAND_CMD_ERASE2(nand_no));
	restore_flags(flags);

	// STEP 4  			
	// wait for finish of erasing
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY);  

	// STEP 5  			
	// check if successful
	save_flags(flags); cli();
	rtl_outl(NCAR, NAND_CMD_STATUS(nand_no));		
	value = rtl_inl(NDR);
	restore_flags(flags);
	if(!(value & 1))
	{
		//printk("Successfully erase block%d\n",block_no);
		//while(1);
		return 1;
	}
	else 
	{
		printk("Failed to erase block%d with data=%x\n",block_no,value);		
		//printk("Row1 = %x, Row2 = %x\n",row1,row2);	
		return 0;
	}
	
}

/*
 * input: from  - start block number (must block-alignment)
 *	  *mem_buf  - data buffer (1 block size)
 */
static int read_block(unsigned int from_block,unsigned char *mem_buf)
{
	int page_num;
	unsigned int index;
	unsigned int i;
	unsigned int row1,row2;
	unsigned int rowx,rowy;
	unsigned int flags;

	// read a block, 32 pages, 528 bytes per page	
	nand_addr = 0;
	index = 0;
	
	rowx = (from_block * PAGENUM) & 0xff;  // page number A0~A7
	rowy = (from_block >> 3) & 0xff;  // page number A9~A16
	row2 = (rowy << 16) & 0xff0000;	
	for(page_num=0;page_num<32;page_num++) {

		// STEP 1		
		// if device is still in busy, pass control to other tasks
		while(!NANDRDY);
		// get into command mode
		save_flags(flags); cli();
		rtl_outl(NCAR, NAND_CMD_RESET(nand_no));
		restore_flags(flags);
		// if device is still in busy, pass control to other tasks
		while(!NANDRDY); 
		// 
		row1 = ((rowx | page_num) << 8) & 0xff00;
		//printk("rowx=%x rowy=%x row1=%x row2=%x\n",rowx,rowy,row1,row2);
		// always start from column 0
		nand_addr = (CE4_ADEN2 | row2) | (CE4_ADEN1 | row1) | (CE4_ADEN0);
		// STEP 2		
		//printk("read_block:read block %d address %x\n",from_block,nand_addr);
		save_flags(flags); cli();
		rtl_outl(NCAR, NAND_CMD_READ0(nand_no));
//		rtl_outl(NCAR, NAND_CMD_SEQIN(nand_no));
		rtl_outl(NADDR, nand_addr);
		restore_flags(flags);
		// STEP 3
		// if device is still in busy, pass control to other tasks
		while(!NANDRDY); 
		// STEP 4		
		// read 528 bytes(1 page) per loop
		for(index;index<(528+page_num*528);index+=4)
                {
			*(volatile unsigned int *)(mem_buf+index) = rtl_inl(NDR);	
		 	if(!(index % 12))
			{
	
			}
			else
				nand_addr = rtl_inl(MCR);
		}
	}			

	return 1;
}

/*
 * input: to    - start block number (must block-alignment)
 *	  *mem_buf  - data buffer (1 block size)
 */
static int write_block(unsigned int block_no, unsigned char *mem_buf)
{	
	unsigned int addr;
	unsigned int page_num;
	unsigned int rowx;
	unsigned int rowy;	
	unsigned int row1;
	unsigned int row2;
	unsigned int i;
	unsigned int flags;

	rowx = (block_no << 5) & 0xff;  // page number A0~A7
	rowy = (block_no >> 3) & 0xff;  // page number A9~A16
	row2 = (rowy << 16) & 0xff0000;

	// read a block, 32 pages, 528 bytes per page	
	addr = 0;
	// check if write-protection
	save_flags(flags);cli();	
	rtl_outl(NCAR, NAND_CMD_STATUS(nand_no));		
	if (!(rtl_inl (NDR) & WRITEPROTECT)) {
		printk("nand_write_ecc: Device is write protected!!!\n");
		return -1;
	}		
	restore_flags(flags);
	printk("write block %dth from %x\n",block_no,(unsigned long)mem_buf);
	for(page_num=0;page_num<32;page_num++) {

		// STEP 1
		// if device is still in busy, pass control to other tasks
		while(!NANDRDY); 
		// get into command mode
		save_flags(flags);cli();
		rtl_outl(NCAR, NAND_CMD_RESET(nand_no));
		restore_flags(flags);
		// if device is still in busy, pass control to other tasks
		while(!NANDRDY);
		// STEP 2		
		// wait for device to be ready
		row1 = ((rowx | page_num) << 8) & 0xff00;
		addr = (CE4_ADEN2 | row2) | (CE4_ADEN1 | row1) | CE4_ADEN0;
		// if device is still in busy, pass control to other tasks
		while(!NANDRDY); 
		// STEP 3
		// issue command and address
		save_flags(flags);cli();
		rtl_outl(NCAR, NAND_CMD_SEQIN(nand_no));
		rtl_outl(NADDR, addr);
		for(i=0;i<528;i+=4)
		{
			rtl_outl(NDR,*(volatile unsigned int *)(mem_buf+page_num*528+i));
		 	rtl_inl(MTCR0);	
		}

		rtl_outl(NCAR, NAND_CMD_PAGEPROG(nand_no));
		restore_flags(flags);
		// STEP 4
		// wait for data write to be finished 
		while(!NANDRDY); 
		
		// STEP 5  			
		// check if successful
		save_flags(flags); cli();
		rtl_outl(NCAR, NAND_CMD_STATUS(nand_no));		
		if(rtl_inl(NDR) & 1)
		{
			printk("Failed to program block%d at page%d\n",block_no,page_num);		
			restore_flags(flags);
			return 0;
		}
		else 
			;//printk("Finish programming block%d at page%d\n",to,page_num);
	}				
	restore_flags(flags);
	return 1;	
}

/*
	start 	: start address (NAND address 0~32M)
	length 	: total data size (Bytes)
	*target	: SDRAM buffer address
	method	: if the start address is not BLOCK-alignment
		  then read data until the next boundary
		  then read block size data
		  then read the left segment
 */
int read_data(int start, int length, unsigned char *target)
{
	int i;
	unsigned int tmp_address;
	unsigned int tmp_length;
	unsigned int block_number;
	unsigned int newstart; 
	static unsigned int blockno;
	
	if(wrongchip) {
		printk("Chip is not Samsung, exit!\n");
		return 0;
	}	
	tmp_length = 0;
	// if the start read address is not aligned to the block boundary	
	printk("start from %x to %x for length=%d\n",start,target,length);
	blockno = start/BLOCKSIZE;
	if(start % BLOCKSIZE) {
		newstart = BLOCKSIZE*(blockno+1); 
		if(newstart > start+length)
			tmp_length = length;
		else
			tmp_length = newstart-start; 
		start -= (BLOCKSIZE*blockno);	
		printk("read block %d from %x with %d to %x new(%x)\n",
			blockno,start,tmp_length,target,newstart);
		read_seg(blockno,
			 start,
			 tmp_length,
			 target);
		tmp_address += (unsigned int)(target + tmp_length);
		block_number = tmp_length/BLOCKSIZE;
		blockno ++;
	}
	else {
		newstart = 0;
		tmp_address = (unsigned int)target;	
		block_number = length/BLOCKSIZE;
	}
	//
	printk("read %d blocks from %d block, start from %x to %x\n",
		block_number, blockno, newstart,tmp_address);
	for(i=0;i<block_number;i++) {
		read_block(blockno+i,
			   tmp_address);   
		tmp_address += BLOCKSIZE;	
	}
	i += blockno;	
	// check if we still need to read the left segment
	tmp_length += (block_number * BLOCKSIZE); // total read length
	
	if(tmp_length < length) {
		// i = last read block	
		printk("read block %d from 0 with %d to %x\n",
			i,length-tmp_length,tmp_address);
		read_seg(i,	// next block
			 0,	// start from 0
			 length-tmp_length, // left data length
			 tmp_address 
			);
		tmp_address += (length-tmp_length);
	}
	return tmp_address;	
}

static int open_device(int chip_num)
{

	unsigned int flags;
	
	save_flags(flags); cli();
	rtl_outl(NCAR, NAND_CMD_RESET(chip_num));
	restore_flags(flags);
	
	while(!NANDRDY);
	save_flags(flags); cli();
	if(chip_num == 0)	
		rtl_outl(NCAR, CE4_EN);  
	else
		rtl_outl(NCAR, CE5_EN); 
	restore_flags(flags);
	return 1; 
}

/*
 * input: from  - start block number (must block-alignment)
 *	  *mem_buf  - data buffer (1 block size)
 */
static int read_seg(unsigned int from_block, // block number
		    unsigned int start,	     // start address
		    unsigned int length,     // read length (< 528 bytes)
		    unsigned char *mem_buf)
{
	unsigned int page_num;
	unsigned int i;
	unsigned int row1,row2;
	unsigned int rowx,rowy;
	unsigned int flags;
	unsigned int area;

	// read a block, 32 pages, 528 bytes per page	
	nand_addr = 0;
	index = 0;
	
	page_num = start / 528;
	//printk("read_seg:block=%d,start=%x,dst=%x,length=%d\n",
	//	from_block,start,(unsigned int)mem_buf,length);
//	if(start > 512) {
//		prom_printf("Oversize of start at read_seg\n");
//		while(1);
//	} 
	// identifiy first 256 bytes or second 256 bytes
	
	if((start%528) < 256)  
		area = 0;
	else if((start%528) < 512) 
		area = 1;
	else
		area = 2;
	
	// get A0~A7 only
	start = start & 0xff;

	rowx = (from_block << 5) & 0xff;  // page number A0~A7
	rowy = (from_block >> 3) & 0xff;  // page number A9~A11
	row2 = (rowy << 16) & 0xff0000;	

	// STEP 1		
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY);
	// get into command mode
	save_flags(flags); cli();
	rtl_outl(NCAR, NAND_CMD_RESET(nand_no));
	restore_flags(flags);
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY); 
	// 
	row1 = ((rowx | page_num) << 8) & 0xff00;
	
	nand_addr = (CE4_ADEN2 | row2) | (CE4_ADEN1 | row1) | (CE4_ADEN0 | start);
	//printk("read_seg:read block %d address %x\n",from_block,nand_addr);
	// STEP 2		
	save_flags(flags); cli();
	if(area==0)		// first block
		rtl_outl(NCAR, NAND_CMD_READ0(nand_no));  
	else if(area==1)	// second block
		rtl_outl(NCAR, NAND_CMD_READ1(nand_no)); 
	else			// OOB
		rtl_outl(NCAR, NAND_CMD_READOOB(nand_no)); 
		
//	rtl_outl(NCAR, NAND_CMD_SEQIN(nand_no));
	rtl_outl(NADDR, nand_addr);
	restore_flags(flags);
	// STEP 3
	// if device is still in busy, pass control to other tasks
	while(!NANDRDY); 
	// STEP 4		
	// read 528 bytes(1 page) per loop
	for(i=0;i<length;i+=4)
        {
		*(volatile unsigned int *)(mem_buf+i) = rtl_inl(NDR);	
	}

	return 1;
}

int write_data(int start, int length, unsigned char *target)
{
	int i;
	unsigned char *tmp_address;
	unsigned int x;
	unsigned int tmp_length;
	unsigned int block_number;
	unsigned int newstart; 
	static unsigned int blockno;
	
	if(wrongchip) {
		printk("Chip is not Samsung, exit!\n");
		return 0;
	}	
	tmp_length = 0;
	// if the start read address is not aligned to the block boundary	
	printk("start from %x to %x for length=%d\n",target,start,length);
	blockno = start/BLOCKSIZE;
	if(start % BLOCKSIZE) {
		newstart = BLOCKSIZE*(blockno+1); 
		if(newstart > start+length)
			tmp_length = length;
		else
			tmp_length = newstart-start; 
		start -= (BLOCKSIZE*blockno);	
		printk("write block %d from %x with %d to %x new(%x)\n",
			blockno,target,tmp_length,start,newstart);

		read_block(blockno+1,writebuffer);

		if(!erase_block(blockno+1)) 
			return -1;		
		for(i=0;i<tmp_length;i++)
			writebuffer[i+start]=target[i];
		write_block(blockno+1,writebuffer);
		x = (unsigned int)target + tmp_length;
		tmp_address += x; 
		block_number = tmp_length/BLOCKSIZE;
		blockno ++;
	}
	else {
		newstart = 0;
		tmp_address = target;	
		block_number = length/BLOCKSIZE;
	}
	//
	printk("write %d blocks from %d block, start from %x to %x\n",
		block_number, blockno, tmp_address, tmp_address);
	for(i=0;i<block_number;i++) {
		if(!erase_block(blockno+i)) {
			printk("Erase %dth block fail\n",blockno+i);
			return -1;
		}
		write_block(blockno+i,
			   tmp_address);   
		tmp_address += BLOCKSIZE;	
	}
	blockno += i;	
	// check if we still need to read the left segment
	tmp_length += (block_number * BLOCKSIZE); // total read length
	
	if(tmp_length < length) {
		// i = last read block	
		printk("write block %d from %x with %d to 0\n",
			blockno,tmp_address,length-tmp_length);
		read_block(blockno,writebuffer);
		if(!erase_block(blockno))
			return -1;
		for(i=0;i<length-tmp_length;i++)
			writebuffer[i]=tmp_address[i];
		write_block(blockno,writebuffer);		
		tmp_address += (length-tmp_length);
	}
	return tmp_address;	
}
