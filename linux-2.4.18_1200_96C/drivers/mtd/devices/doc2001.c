
/*
 * Linux driver for Disk-On-Chip Millennium
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * $Id: doc2001.c,v 1.24 2010/02/09 13:33:52 bradhuang Exp $
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/reboot.h>

// david ---------------
#include <linux/config.h>
//#define NO_CHECK_REGION			// did not check region when write flash
//sc_yang
#define CALL_APP_TO_LOAD_DEFAULT	// call user program to load default
extern int flash_hw_start;
/*
#ifdef CONFIG_NINO_8MB
	#define COMPACK_SETTING
	extern int flash_hw_start;
#endif
*/ //sc_yang
#define noCONFIG_MTD_DEBUG
#define CONFIG_MTD_DEBUG_VERBOSE 3
extern int flash_hw_start, flash_hw_len, flash_ds_start, flash_ds_len, flash_write_flag;
//----------------------

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ids.h>
#include <linux/mtd/doc2000.h>
#include <linux/mtd/partitions.h>

/* #define ECC_DEBUG */

/* I have no idea why some DoC chips can not use memcop_form|to_io().
 * This may be due to the different revisions of the ASIC controller built-in or
 * simplily a QA/Bug issue. Who knows ?? If you have trouble, please uncomment
 * this:*/
#undef USE_MEMCPY

#if 0 //sc_yang
static void rtl8181_flashtimer (unsigned long data);
#endif
static int doc_read(struct mtd_info *mtd, loff_t from, size_t len,
		    size_t *retlen, u_char *buf);
static int doc_write(struct mtd_info *mtd, loff_t to, size_t len,
		     size_t *retlen, const u_char *buf);
static int doc_read_ecc(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *eccbuf);
static int doc_write_ecc(struct mtd_info *mtd, loff_t to, size_t len,
			 size_t *retlen, const u_char *buf, u_char *eccbuf);
// david
#if 0
static int doc_read_oob(struct mtd_info *mtd, loff_t ofs, size_t len,
			size_t *retlen, u_char *buf);
static int doc_write_oob(struct mtd_info *mtd, loff_t ofs, size_t len,
			 size_t *retlen, const u_char *buf);
#endif

static int doc_erase (struct mtd_info *mtd, struct erase_info *instr);

// david
static int erase_one_block(struct DiskOnChip *this, __u32 addr, __u32 len);

static struct mtd_info *docmillist = NULL;

/* Perform the required delay cycles by reading from the NOP register */
static void DoC_Delay(unsigned long docptr, unsigned short cycles)
{
	volatile char dummy;
	int i;

	for (i = 0; i < cycles; i++)
		dummy = ReadDOC(docptr, NOP);
}

/* DOC_WaitReady: Wait for RDY line to be asserted by the flash chip */
static int _DoC_WaitReady(unsigned long docptr)
{
	unsigned short c = 0xffff;

	DEBUG(MTD_DEBUG_LEVEL3,
	      "_DoC_WaitReady called for out-of-line wait\n");

	/* Out-of-line routine to wait for chip response */
	while (!(ReadDOC(docptr, CDSNControl) & CDSN_CTRL_FR_B) && --c)
		;

	if (c == 0)
		DEBUG(MTD_DEBUG_LEVEL2, "_DoC_WaitReady timed out.\n");

	return (c == 0);
}

static inline int DoC_WaitReady(unsigned long docptr)
{
	/* This is inline, to optimise the common case, where it's ready instantly */
	int ret = 0;

	/* 4 read form NOP register should be issued in prior to the read from CDSNControl
	   see Software Requirement 11.4 item 2. */
	DoC_Delay(docptr, 4);

	if (!(ReadDOC(docptr, CDSNControl) & CDSN_CTRL_FR_B))
		/* Call the out-of-line routine to wait */
		ret = _DoC_WaitReady(docptr);

	/* issue 2 read from NOP register after reading from CDSNControl register
	   see Software Requirement 11.4 item 2. */
	DoC_Delay(docptr, 2);

	return ret;
}

/* DoC_Command: Send a flash command to the flash chip through the CDSN IO register
   with the internal pipeline. Each of 4 delay cycles (read from the NOP register) is
   required after writing to CDSN Control register, see Software Requirement 11.4 item 3. */

static inline void DoC_Command(unsigned long docptr, unsigned char command,
			       unsigned char xtraflags)
{
	/* Assert the CLE (Command Latch Enable) line to the flash chip */
	WriteDOC(xtraflags | CDSN_CTRL_CLE | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);

	/* Send the command */
	WriteDOC(command, docptr, Mil_CDSN_IO);
	WriteDOC(0x00, docptr, WritePipeTerm);

	/* Lower the CLE line */
	WriteDOC(xtraflags | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);
}

/* DoC_Address: Set the current address for the flash chip through the CDSN IO register
   with the internal pipeline. Each of 4 delay cycles (read from the NOP register) is
   required after writing to CDSN Control register, see Software Requirement 11.4 item 3. */

static inline void DoC_Address(unsigned long docptr, int numbytes, unsigned long ofs,
			       unsigned char xtraflags1, unsigned char xtraflags2)
{
	/* Assert the ALE (Address Latch Enable) line to the flash chip */
	WriteDOC(xtraflags1 | CDSN_CTRL_ALE | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);

	/* Send the address */
	switch (numbytes)
	    {
	    case 1:
		    /* Send single byte, bits 0-7. */
		    WriteDOC(ofs & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC(0x00, docptr, WritePipeTerm);
		    break;
	    case 2:
		    /* Send bits 9-16 followed by 17-23 */
		    WriteDOC((ofs >> 9)  & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC((ofs >> 17) & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC(0x00, docptr, WritePipeTerm);
		break;
	    case 3:
		    /* Send 0-7, 9-16, then 17-23 */
		    WriteDOC(ofs & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC((ofs >> 9)  & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC((ofs >> 17) & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC(0x00, docptr, WritePipeTerm);
		break;
	    default:
		return;
	    }

	/* Lower the ALE line */
	WriteDOC(xtraflags1 | xtraflags2 | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);
}

// david
#if 0
/* DoC_SelectChip: Select a given flash chip within the current floor */
static int DoC_SelectChip(unsigned long docptr, int chip)
{
	/* Select the individual flash chip requested */
	WriteDOC(chip, docptr, CDSNDeviceSelect);
	DoC_Delay(docptr, 4);

	/* Wait for it to be ready */
	return DoC_WaitReady(docptr);
}

/* DoC_SelectFloor: Select a given floor (bank of flash chips) */
static int DoC_SelectFloor(unsigned long docptr, int floor)
{
	/* Select the floor (bank) of chips required */
	WriteDOC(floor, docptr, FloorSelect);

	/* Wait for the chip to be ready */
	return DoC_WaitReady(docptr);
}

/* DoC_IdentChip: Identify a given NAND chip given {floor,chip} */
static int DoC_IdentChip(struct DiskOnChip *doc, int floor, int chip)
{
	int mfr, id, i;
	volatile char dummy;

	/* Page in the required floor/chip
	   FIXME: is this supported by Millennium ?? */
	DoC_SelectFloor(doc->virtadr, floor);
	DoC_SelectChip(doc->virtadr, chip);

	/* Reset the chip, see Software Requirement 11.4 item 1. */
	DoC_Command(doc->virtadr, NAND_CMD_RESET, CDSN_CTRL_WP);
	DoC_WaitReady(doc->virtadr);

	/* Read the NAND chip ID: 1. Send ReadID command */
	DoC_Command(doc->virtadr, NAND_CMD_READID, CDSN_CTRL_WP);

	/* Read the NAND chip ID: 2. Send address byte zero */
	DoC_Address(doc->virtadr, 1, 0x00, CDSN_CTRL_WP, 0x00);

	/* Read the manufacturer and device id codes of the flash device through
	   CDSN IO register see Software Requirement 11.4 item 5.*/
	dummy = ReadDOC(doc->virtadr, ReadPipeInit);
	DoC_Delay(doc->virtadr, 2);
	mfr = ReadDOC(doc->virtadr, Mil_CDSN_IO);

	DoC_Delay(doc->virtadr, 2);
	id  = ReadDOC(doc->virtadr, Mil_CDSN_IO);
	dummy = ReadDOC(doc->virtadr, LastDataRead);

	/* No response - return failure */
	if (mfr == 0xff || mfr == 0)
		return 0;

	/* FIXME: to deal with multi-flash on multi-Millennium case more carefully */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (mfr == nand_flash_ids[i].manufacture_id &&
		    id == nand_flash_ids[i].model_id) {
			printk(KERN_INFO "Flash chip found: Manufacturer ID: %2.2X, "
			       "Chip ID: %2.2X (%s)\n",
			       mfr, id, nand_flash_ids[i].name);
			doc->mfr = mfr;
			doc->id = id;
			doc->chipshift = nand_flash_ids[i].chipshift;
			break;
		}
	}

	if (nand_flash_ids[i].name == NULL)
		return 0;
	else
		return 1;
}
#endif // david

#ifdef CONFIG_RTK_MTD_ROOT
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000C0000,
                offset:         0x00000000
        },{
               name:           "root fs",                
		 size:           0x00140000,
               offset:         0xC0000,
        }
#elif defined(CONFIG_RTL865X_AC)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000C0000,
                offset:         0x00000000
        },{
               name:           "root fs",                
		 size:           0x00140000,
               offset:         0xC0000,
        }        
#elif defined(CONFIG_RTL865X_KLD) 
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000A0000,
                offset:         0x00000000
        },{
               name:           "root fs",                
			   //size:           0x00140000,
			   size:           0x00160000,
               offset:         0xA0000,
        }  
#elif defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)     
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000A0000,
                offset:         0x00000000
        },{
               name:           "root fs",                
			   size:           0x000150000,
			   	 offset:         0x000A0000,
        } ,              
        {
                name:           "webpage",
                size:           0x00010000,
                offset:         0x1f0000,
        }        
#elif defined(CONFIG_RTL865X_SC)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000C0000,
                offset:         0x00000000
        },{
                name:           "root fs",                
				size:           0x00140000,
                offset:         0xC0000,
        }
#elif defined(CONFIG_RTK_VOIP)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x00150000,
                offset:         0x00000000
        },{
                name:           "root fs",                
				size:           0x00240000,
                offset:         0x00150000,
        }
#elif defined(CONFIG_WIRELESS_LAN_MODULE)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000C0000,
                offset:         0x00000000
        },{
                name:           "root fs",                
				size:           0x00140000,
                offset:         0xC0000,
        }
#elif defined(CONFIG_RTL865X_CMO)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000C0000,
                offset:         0x00000000
        },{
                name:           "root fs",                
				size:           0x00140000,
                offset:         0xC0000,
        }
#elif defined(CONFIG_RTL8196C)
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000
        },{
                name:           "root fs",                
								size:           (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
#else // CONFIG_RTL8196B
static struct mtd_partition rtl8186_partitions[] = {
        {
                name:           "boot+cfg+linux",
                size:           0x000E0000,
                offset:         0x00000000
        },{
                name:           "root fs",                
#ifdef CONFIG_RTL_WAPI_SUPPORT 
                size:           0x00300000,
                offset:         0xE0000,
#elif defined(CONFIG_CWMP_TR069)
								size:           0x00300000,
                offset:         0xE0000,
#else               	         		      
								size:           0x00120000,
                offset:         0xE0000,
#endif            
        }
        
#endif   
          
};
#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))
#endif

static const char im_name[] = "DoCMil_init";

/* This routine is made available to other mtd code via
 * inter_module_register.  It must only be accessed through
 * inter_module_get which will bump the use count of this module.  The
 * addresses passed back in mtd are valid as long as the use count of
 * this module is non-zero, i.e. between inter_module_get and
 * inter_module_put.  Keith Owens <kaos@ocs.com.au> 29 Oct 2000.
 */
void DoCMil_init(struct mtd_info *mtd)
{
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
// david
//	struct DiskOnChip *old = NULL;

	/* We must avoid being called twice for the same device. */

	mtd->name = "DiskOnChip Millennium";
// david
//	printk(KERN_NOTICE "DiskOnChip Millennium found at address 0x%lX\n",
//	       this->physadr);

	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
// david
//	mtd->size = 0;

	/* FIXME: erase size is not always 8kB */
// david
//	mtd->erasesize = 0x2000;

	mtd->oobblock = 512;
	mtd->oobsize = 16;
	mtd->module = THIS_MODULE;
	mtd->erase = doc_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = doc_read;
	mtd->write = doc_write;
	mtd->read_ecc = doc_read_ecc;
	mtd->write_ecc = doc_write_ecc;
// david
//	mtd->read_oob = doc_read_oob;
//	mtd->write_oob = doc_write_oob;
	mtd->sync = NULL;

#if 0 //sc_yang , move to driver/char/rtl_gio.c

	init_timer (&this->probe_timer);
	this->probe_counter = 0;
	this->probe_timer.expires=jiffies + 100;
        this->probe_timer.data = (unsigned long) mtd;
        this->probe_timer.function = &rtl8181_flashtimer;
        mod_timer(&this->probe_timer, jiffies + 100);

#endif

// marked by david ----------------------
#if 0
	this->totlen = 0;
	this->numchips = 0;
	this->curfloor = -1;
	this->curchip = -1;

	/* Ident all the chips present. */
	//DoC_ScanChips(this);

    this->numchips = 1;
    this->chipshift = 13; //  8k = 1 << 13

   this->totlen = this->numchips * ( 1 << this->chipshift);

    this->chips = kmalloc(sizeof(struct Nand) * this->numchips, GFP_KERNEL);
    if (!this->chips){
// david
//        printk("No memory for allocating chip info structures\n");
        printk(KERN_WARNING "No memory for allocating chip info structures\n");
        return;
    }

    this->chips[0].floor = 0;
    this->chips[0].chip = 0;
    this->chips[0].curadr = 0;
    this->chips[0].curmode = 0x50;

#endif
//-----------------------------------------


// david
//	printk("totlen=%08x\n", this->totlen);

	if (!this->totlen) {
		kfree(mtd);
		iounmap((void *)this->virtadr);
	} else {
		this->nextdoc = docmillist;
		docmillist = mtd;
		mtd->size  = this->totlen;
#ifdef CONFIG_RTK_MTD_ROOT
		add_mtd_partitions(mtd, rtl8186_partitions,  NB_OF(rtl8186_partitions));
#else
		add_mtd_device(mtd);
#endif
		return;
	}
}

static int doc_read (struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, u_char *buf)
{
	/* Just a special case of doc_read_ecc */
	return doc_read_ecc(mtd, from, len, retlen, buf, NULL);
}

static int doc_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			 size_t *retlen, u_char *buf, u_char *eccbuf)
{
// david
//	int i, ret;
	int i;
	unsigned short tmp;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
// david
//	unsigned long docptr = this->virtadr;
	unsigned long docptr = this->virtadr+from;

	/* Don't allow read past end of device */
	if (from >= this->totlen)
		return -EINVAL;
	for(i=0; i< len; i+=2)
	{
		tmp = *(volatile unsigned short *)(docptr);
#if 0
		*(unsigned char *)buf = (tmp >> 8) & 0xff;
		buf++;
		*(unsigned char *)buf = (tmp ) & 0xff;
		buf++;
#endif
		*(unsigned short *)buf = tmp;
		buf += 2;
		docptr +=2;
	}
	if (len & 0x01)
	{
		tmp = *(volatile unsigned long *)(docptr);
        *(unsigned char *)buf = (tmp >> 8) & 0xff;
	}

	/* Let the caller know we completed it */
	*retlen = len;

	return 0;
}

static int doc_write (struct mtd_info *mtd, loff_t to, size_t len,
		      size_t *retlen, const u_char *buf)
{
	char eccbuf[6];
	return doc_write_ecc(mtd, to, len, retlen, buf, eccbuf);
}

static int doc_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const u_char *buf, u_char *eccbuf)
{
// david
//	int i,ret,j = 0;
	int i,ret;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr = this->virtadr;
	unsigned int ofs;
	unsigned short val,val1;

// david ------------
unsigned long timeo, offset;
unsigned long flags; 
//-------------------

	/* Don't allow write past end of device */
	if (to >= this->totlen)
	{
// david
//		printk("write to >= total len\n");
	printk(KERN_WARNING "write to >= total len\n");
		return -EINVAL;
	}
	DEBUG(MTD_DEBUG_LEVEL1,"going to write len=0x%x,to =0x%x\n", (int)len, (int)to);

	*retlen = len;
	ofs = docptr + to;
	for(i=0; i< len; i+=2)
	{
// david -----------------------------------------------------
#if 0
		val = *(unsigned short *)buf;

		*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2)= 0xaa;
		*(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2)= 0x55;
		*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2)= 0xa0;
		*(volatile unsigned short *)(ofs )= val;

	       	j=0xfffff1;
        	do{
                	val1=*(volatile unsigned short *)(ofs);
  	            	if( ((val1^val) & 0x80)==0 )
 				break;

       		}while(j--!=1);
		if (j <= 2)
			printk("program fails\n");
#else

//		if ( ofs < (docptr+CONFIG_MTD_DOCPROBE_ADDRESS) )
//			goto next_word;

		offset = (to >> this->chipshift)*(1 << this->chipshift);

#if !defined(COMPACK_SETTING) && !defined(NO_CHECK_REGION)
		if ( flash_write_flag != 0x8000
#ifdef CONFIG_RTK_MTD_ROOT
			|| offset < (rtl8186_partitions[0].size+ rtl8186_partitions[0].offset)
#endif
			)
		{

			if ( (flash_write_flag & 1) && (ofs < (docptr+flash_hw_start)) )
				goto next_word;

			if ( (flash_write_flag & 2) && (ofs < (docptr+flash_ds_start)) )
				goto next_word;

			if ( (flash_write_flag & 4) && (ofs < (docptr+flash_ds_start+flash_ds_len)) )
				goto next_word;
		}
#endif // COMPACK_SETTING && NO_CHECK_REGION

		val = *(unsigned short *)buf;

		save_flags(flags);cli(); // david
		
		*(volatile unsigned short *)(FLASH_BASE + offset + 0x555 * 2)= 0xaa;
		*(volatile unsigned short *)(FLASH_BASE + offset + 0x2aa * 2)= 0x55;
		*(volatile unsigned short *)(FLASH_BASE + offset + 0x555 * 2)= 0xa0;
		*(volatile unsigned short *)(ofs )= val;
		
		restore_flags(flags); // david

		timeo = jiffies + (HZ * 50);
        	do{    		
#if 0
			val1=*(volatile unsigned short *)(ofs);
			if ( val1 == val )
				break;
#endif
			unsigned short val2;
      	
			val2=*(volatile unsigned short *)(ofs);
			val1=*(volatile unsigned short *)(ofs);	
			
			if (((val1^val2) & 0x40) != 0)
				continue;	 	        
	 	        if (((val1^val) & 0x80) != 0)
 				continue;
 			if ( val1 == val )		
				break;
//--------------	      	    	
		} while ( !time_after(jiffies, timeo) );

		if ( time_after(jiffies, timeo)) {
			printk(KERN_WARNING "program timeout!");
			printk(KERN_WARNING " write: %x, read:%x, addr: %x\n", val, val1, ofs);
			return -1;
		}

#ifndef COMPACK_SETTING
next_word:
#endif

#endif
//---------------------------------------------------------
		ofs += 2;
		buf += 2;

		//*retlen = *retlen - 2;
	}

	ret = 0 ;
// david
//	printk("in doc_write_ecc ret=%08x\n", ret);
	return ret;
}

// marked by david
#if 0
static int doc_read_oob(struct mtd_info *mtd, loff_t ofs, size_t len,
			size_t *retlen, u_char *buf)
{
#ifndef USE_MEMCPY
	int i;
#endif
	volatile char dummy;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr = this->virtadr;
	struct Nand *mychip = &this->chips[ofs >> this->chipshift];

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* disable the ECC engine */
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_DIS, docptr, ECCConf);

	/* issue the Read2 command to set the pointer to the Spare Data Area.
	   Polling the Flash Ready bit after issue 3 bytes address in
	   Sequence Read Mode, see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, NAND_CMD_READOOB, CDSN_CTRL_WP);
	DoC_Address(docptr, 3, ofs, CDSN_CTRL_WP, 0x00);
	DoC_WaitReady(docptr);

	/* Read the data out via the internal pipeline through CDSN IO register,
	   see Pipelined Read Operations 11.3 */
	dummy = ReadDOC(docptr, ReadPipeInit);
#ifndef USE_MEMCPY
	for (i = 0; i < len-1; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		buf[i] = ReadDOC(docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_fromio(buf, docptr + DoC_Mil_CDSN_IO, len - 1);
#endif
	buf[len - 1] = ReadDOC(docptr, LastDataRead);

	*retlen = len;

	return 0;
}

static int doc_write_oob(struct mtd_info *mtd, loff_t ofs, size_t len,
			 size_t *retlen, const u_char *buf)
{
#ifndef USE_MEMCPY
	int i;
#endif
	volatile char dummy;
	int ret = 0;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr = this->virtadr;
	struct Nand *mychip = &this->chips[ofs >> this->chipshift];

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* disable the ECC engine */
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_DIS, docptr, ECCConf);

	/* Reset the chip, see Software Requirement 11.4 item 1. */
	DoC_Command(docptr, NAND_CMD_RESET, CDSN_CTRL_WP);
	DoC_WaitReady(docptr);
	/* issue the Read2 command to set the pointer to the Spare Data Area. */
	DoC_Command(docptr, NAND_CMD_READOOB, CDSN_CTRL_WP);

	/* issue the Serial Data In command to initial the Page Program process */
	DoC_Command(docptr, NAND_CMD_SEQIN, 0x00);
	DoC_Address(docptr, 3, ofs, 0x00, 0x00);

	/* Write the data via the internal pipeline through CDSN IO register,
	   see Pipelined Write Operations 11.2 */
#ifndef USE_MEMCPY
	for (i = 0; i < len; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		WriteDOC(buf[i], docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_toio(docptr + DoC_Mil_CDSN_IO, buf, len);
#endif
	WriteDOC(0x00, docptr, WritePipeTerm);

	/* Commit the Page Program command and wait for ready
	   see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, NAND_CMD_PAGEPROG, 0x00);
	DoC_WaitReady(docptr);

	/* Read the status of the flash device through CDSN IO register
	   see Software Requirement 11.4 item 5.*/
	DoC_Command(docptr, NAND_CMD_STATUS, 0x00);
	dummy = ReadDOC(docptr, ReadPipeInit);
	DoC_Delay(docptr, 2);
	if (ReadDOC(docptr, Mil_CDSN_IO) & 1) {
		printk("Error programming oob data\n");
		/* FIXME: implement Bad Block Replacement (in nftl.c ??) */
		*retlen = 0;
		ret = -EIO;
	}
	dummy = ReadDOC(docptr, LastDataRead);

	*retlen = len;

	return ret;
}
#endif // end of comment, david


int doc_erase (struct mtd_info *mtd, struct erase_info *instr)
{
// david --------------------------------------------------------
#if 0

	int i;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	__u32 ofs = instr->addr;
	__u32 len = instr->len;
	unsigned long docptr = this->virtadr;

	if (len != mtd->erasesize)
		printk(KERN_WARNING "Erase not right size (%x != %x)n",
		       len, mtd->erasesize);
	printk("going to erase sector addr=%08x, docptr=%08x, len=%08x\n",
		(instr->addr),docptr, instr->len);

	instr->state = MTD_ERASING;

	// issue erase command!
	ofs = docptr + ofs;
	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0x80;
	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(ofs ) = 0x30;

     i=0xffffff1;
     do{
    		if ((*(volatile unsigned short *)(ofs))==0xffff)
			{
				printk("Erase success!\n");
				break;
			}
     }while(i--!=1);
	if (i<=2)
		printk("erase fails!\n");
	instr->state = MTD_ERASE_DONE;

	if (instr->callback)
		instr->callback(instr);

	return 0;
#else
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long adr, len;
	int i;
	int first;
	struct mtd_erase_region_info *regions = mtd->eraseregions;

	DEBUG(MTD_DEBUG_LEVEL1, "going to erase sector addr=%08x,len=%08x\n",
		instr->addr, instr->len);

	if (instr->addr > mtd->size) {
		printk(KERN_WARNING "Erase addr greater than max size (0x%x > 0x%x)\n",
			instr->addr, mtd->size );
		return -EINVAL;
	}

	if ((instr->len + instr->addr) > mtd->size) {
		printk(KERN_WARNING "Erase size greater than max size (0x%x + 0x%x > 0x%x)\n",
		       instr->addr, instr->len, mtd->size );
		return -EINVAL;
	}

	/* Check that both start and end of the requested erase are
	 * aligned with the erasesize at the appropriate addresses.
	 */

	i = 0;

        /* Skip all erase regions which are ended before the start of
           the requested erase. Actually, to save on the calculations,
           we skip to the first erase region which starts after the
           start of the requested erase, and then go back one.
        */

        while ((i < mtd->numeraseregions) &&
	       (instr->addr >= regions[i].offset)) {
               i++;
	}
        i--;

	/* OK, now i is pointing at the erase region in which this
	 * erase request starts. Check the start of the requested
	 * erase range is aligned with the erase size which is in
	 * effect here.
	 */

	if (instr->addr & (regions[i].erasesize-1)) {
		return -EINVAL;
	}

	/* Remember the erase region we start on. */

	first = i;

	/* Next, check that the end of the requested erase is aligned
	 * with the erase region at that address.
	 */

	while ((i < mtd->numeraseregions) &&
	       ((instr->addr + instr->len) >= regions[i].offset)) {
                i++;
	}

	/* As before, drop back one to point at the region in which
	 * the address actually falls.
	 */

	i--;

	if ((instr->addr + instr->len) & (regions[i].erasesize-1)) {
                return -EINVAL;
	}


	adr = instr->addr;
	len = instr->len;

	i = first;
	instr->state = MTD_ERASING;

	while (len) {
//		if (adr >= CONFIG_MTD_DOCPROBE_ADDRESS) {

#if defined(COMPACK_SETTING) || defined(NO_CHECK_REGION)
		if ( erase_one_block(this, adr, regions[i].erasesize) )
			return -1;

#else
		if ( ((flash_write_flag & 1) && (adr == flash_hw_start)) ||
				((flash_write_flag & 2) &&(adr >= flash_ds_start && adr < (flash_ds_start+flash_ds_len))) 
				|| ((flash_write_flag & 4) && (adr >= (flash_ds_start+flash_ds_len))) 
#ifdef CONFIG_RTK_MTD_ROOT
				|| (adr >= (rtl8186_partitions[0].size+ rtl8186_partitions[0].offset))
#endif			
				|| (flash_write_flag == 0x8000)
				) {
			if ( erase_one_block(this, adr, regions[i].erasesize) )
				return -1;
		}
			
#endif // COMPACK_SETTING || NO_CHECK_REGION

		adr += regions[i].erasesize;
		if (len < regions[i].erasesize)
			len = 0;
		else
			len -= regions[i].erasesize;

		if ( adr >= (regions[i].offset + regions[i].erasesize*regions[i].numblocks))
			i++;
	}

	instr->state = MTD_ERASE_DONE;
	if (instr->callback) {
		instr->callback(instr);
	}

	return 0;
#endif
}

static int erase_one_block(struct DiskOnChip *this, __u32 addr, __u32 len)
{
	unsigned long timeo;
	unsigned long docptr = this->virtadr;
	__u32 ofs, offset;
	unsigned long flags; // david


	DEBUG(MTD_DEBUG_LEVEL1, "Erase sector, addr=0x%x, docptr=0x%x, len=0x%x\n",
		(int)addr, (int)docptr, (int)len);

	// issue erase command!
	ofs = docptr + addr;

	offset = (addr >> this->chipshift)*(1 << this->chipshift);

	save_flags(flags);cli(); // david
	*(volatile unsigned short *)(docptr + offset + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(docptr + offset + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(docptr + offset + 0x555 * 2) = 0x80;
	*(volatile unsigned short *)(docptr + offset + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(docptr + offset + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(ofs ) = 0x30;
	restore_flags(flags); // david

	timeo = jiffies + (HZ * 40);

	while (1) {
     		if ((*(volatile unsigned short *)(ofs))==0xffff) {
			DEBUG(MTD_DEBUG_LEVEL1, "Erase success!\n");
			break;
		}
		if (time_after(jiffies, timeo)) {
			printk(KERN_WARNING "Erase timeout!\n");
			return -1;
		}		
		udelay(1);
     	}
	return 0;
}
//-----------------------------------------------------------------

/****************************************************************************
 *
 * Module stuff
 *
 ****************************************************************************/

int __init init_doc2001(void)
{

	//inter_module_register(im_name, THIS_MODULE, &DoCMil_init);
	return 0;
}

static void __exit cleanup_doc2001(void)
{
	struct mtd_info *mtd;
	struct DiskOnChip *this;

	while ((mtd=docmillist)) {
		this = (struct DiskOnChip *)mtd->priv;
		docmillist = this->nextdoc;

		del_mtd_device(mtd);

		iounmap((void *)this->virtadr);
		kfree(this->chips);
		kfree(mtd);
	}
	inter_module_unregister(im_name);
}

#if 0
static void reset_flash_default()
{
	char *argv[3], *envp[1] = {NULL};
	int i = 0;

	argv[i++] = "/bin/flash";
	argv[i++] = "reset";
	argv[i] = NULL;

	if (call_usermodehelper(argv[0], argv, envp, 1))
		printk("failed to Reset to default\n");

	machine_restart(0);
}
#endif // CALL_APP_TO_LOAD_DEFAULT


#if 0 //sc_yang
unsigned int led0enable;
#define PROBE_NULL	0
#define PROBE_ACTIVE	1
#define PROBE_RESET	2
#define PROBE_RELOAD	3


unsigned int flash_buf[2048];
static void rtl8181_flashtimer (unsigned long data)
{
	unsigned long flags;
	unsigned int  retlen;
	struct mtd_info *mtd = (struct mtd_info *)data;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned int pressed=1;

	// probe GPIO to see if pressed.
	*(volatile unsigned long *)(0xbd010040)= *(volatile unsigned long *)(0xbd010040) & (~(1 << 10));
	//*(volatile unsigned char *)(0xbd400050)= 0xc0;
	//*(volatile unsigned char *)(0xbd400051)= 0x10;

	if ((*(volatile unsigned long *)(0xbd010044)) & (1 << 10))
		pressed = 0;
	else
	{
		//printk("Key pressed!\n");
	}

	if (this->probe_state == PROBE_NULL)
	{
		if (pressed)
		{
			this->probe_state = PROBE_ACTIVE;
			this->probe_counter++;
		}
		else
			this->probe_counter = 0;
	}
	else if (this->probe_state == PROBE_ACTIVE)
	{
		if (pressed)
		{
			this->probe_counter++;
			if ((this->probe_counter >=2 ) && (this->probe_counter <=5))
			{
				led0enable=1;
				// turn on LED0
				*(volatile unsigned char *)(0xbd400050)= 0xc0;
				*(volatile unsigned char *)(0xbd400051)= 0x10;
				*(volatile unsigned char *)(0xbd40005e) &= 0xEF;
			}
			else if (this->probe_counter >= 5)
			{
				led0enable=1;
				*(volatile unsigned char *)(0xbd400050)= 0xc0;
				*(volatile unsigned char *)(0xbd400051)= 0x10;
				// sparkling LED0
				if (this->probe_counter & 1)
					*(volatile unsigned char *)(0xbd40005e) |= 0x10;
				else
					*(volatile unsigned char *)(0xbd40005e) &= 0xEF;

			}
		}
		else
		{
			if (this->probe_counter < 2)
			{
				this->probe_state = PROBE_NULL;
				this->probe_counter = 0;
			}
			else if (this->probe_counter >=5)
			{
				led0enable=1;
				//reload default
				//printk("Going to Reload Default\n");
#ifdef CALL_APP_TO_LOAD_DEFAULT
				kernel_thread(reset_flash_default, 1, SIGCHLD);
				return;
#else
#ifdef COMPACK_SETTING
				 if ((doc_read_ecc (mtd, flash_hw_start, (2048*4), &(retlen), (unsigned char *)flash_buf, NULL)))
			         {
			         	printk("doc read error in reload default!\n");
			         	return;
				 }
				 memcpy( ((unsigned char *)flash_buf)+flash_cs_start-flash_hw_start,
					 ((unsigned char *)flash_buf)+flash_ds_start-flash_hw_start,
					 flash_ds_len );
 				 *(((unsigned char *)flash_buf)+flash_cs_start-flash_hw_start) =0x43;

#else
				 if ((doc_read_ecc (mtd, 0x6400, (2048*4), &(retlen), (unsigned char *)flash_buf, NULL)))
			         {
			         	printk("doc read error in reload default!\n");
			         	return;
				 }
		                 //printk("flash_buf[0]=%x\n", flash_buf[0]);
		                 *(unsigned char *)flash_buf=0x43;
#endif
				 flash_write_flag=0;
				 save_flags(flags);cli();
				 *(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0xaa;
				 *(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2) = 0x55;
				 *(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0x80;
			         *(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0xaa;
			         *(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2) = 0x55;

#ifdef COMPACK_SETTING
				 *(volatile unsigned short *)(FLASH_BASE + flash_hw_start ) = 0x30;
#else
			         *(volatile unsigned short *)(FLASH_BASE + 0x8000 ) = 0x30;
#endif
			   	 for(retlen=0 ; retlen <=0xffffff0; retlen++)
			         {
#ifdef COMPACK_SETTING
					if (*(volatile unsigned short *)(FLASH_BASE + flash_hw_start)==0xffff)
#else
			         	if (*(volatile unsigned short *)(FLASH_BASE + 0x8000 )==0xffff)
#endif
				        	break;
			         }

#ifdef COMPACK_SETTING
				 if (doc_write_ecc (mtd, flash_hw_start, (2048*4), &retlen, (const u_char *)flash_buf, NULL))
#else
				 if (doc_write_ecc (mtd, 0x8000, (2048*4), &retlen, (const u_char *)flash_buf, NULL))
#endif
		                 {
		                 	printk("doc write error in reload default!\n");
				        return;
			         }
				 restore_flags(flags);

				 this->probe_counter = 0;
				*(volatile unsigned char *)(0xbd40005e) |= 0x10;
				*(volatile unsigned char *)(0xbd400051) &= (~0x10);
				machine_restart(0);
#endif	// CALL_APP_TO_LOAD_DEFAULT

			}
			else
			{
				*(volatile unsigned char *)(0xbd40005e) |= 0x10;
				*(volatile unsigned char *)(0xbd400051) &= (~0x10);
				machine_restart(0);
			}
		}

	}
	mod_timer(&this->probe_timer, jiffies + 100);
}

#endif
module_exit(cleanup_doc2001);
module_init(init_doc2001);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org> et al.");
MODULE_DESCRIPTION("Alternative driver for DiskOnChip Millennium");
