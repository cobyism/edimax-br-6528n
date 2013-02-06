
/* Linux driver for Disk-On-Chip devices			*/
/* Probe routines common to all DoC devices			*/
/* (c) 1999 Machine Vision Holdings, Inc.			*/
/* Author: David Woodhouse <dwmw2@infradead.org>		*/
/* $Id: docprobe.c,v 1.3 2008/08/04 05:22:38 michael Exp $	*/



/* DOC_PASSIVE_PROBE:
   In order to ensure that the BIOS checksum is correct at boot time, and
   hence that the onboard BIOS extension gets executed, the DiskOnChip
   goes into reset mode when it is read sequentially: all registers
   return 0xff until the chip is woken up again by writing to the
   DOCControl register.

   Unfortunately, this means that the probe for the DiskOnChip is unsafe,
   because one of the first things it does is write to where it thinks
   the DOCControl register should be - which may well be shared memory
   for another device. I've had machines which lock up when this is
   attempted. Hence the possibility to do a passive probe, which will fail
   to detect a chip in reset mode, but is at least guaranteed not to lock
   the machine.

   If you have this problem, uncomment the following line:
#define DOC_PASSIVE_PROBE
*/


/* DOC_SINGLE_DRIVER:
   Millennium driver has been merged into DOC2000 driver.

   The newly-merged driver doesn't appear to work for writing. It's the
   same with the DiskOnChip 2000 and the Millennium. If you have a
   Millennium and you want write support to work, remove the definition
   of DOC_SINGLE_DRIVER below to use the old doc2001-specific driver.

   Otherwise, it's left on in the hope that it'll annoy someone with
   a Millennium enough that they go through and work out what the
   difference is :)
*/

#include <linux/config.h>
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

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/doc2000.h>

//#define CONFIG_MTD_DOCPROBE_ADDRESS (24*1024)

// david ----------------------
/* MXIC */
#define MANUFACTURER_MXIC	0x00C2
#define MX29LV800B		0x225B
#define MX29LV160AB		0x2249
#define MX29LV320AB		0x22A8
#define MX29LV640AB		0x22CB
#define MX29LV1280DB	        0x227a //16MB, bottom

/*AMD*/
#define MANUFACTURER_AMD	0x0001
#define AM29LV800BB		0x225B
#define AM29LV160DB		0x2249
#define AM29LV320DB		0x22F9

/*ST*/
#define MANUFACTURER_ST		0x0020
#define M29W160DB		0X2249

/* ESMT */
#define MANUFACTURER_ESMT	0x008C
#define F49L160BA		0x2249

/* SAMSUNG */
#define MANUFACTURER_SAMSUNG	0x00EC
#define K8D1716UBC		0x2277

/* ESI */
#define MANUFACTURER_ESI	0x004A
#define ES29LV320D		0x22F9

/* EON */
#define MANUFACTURER_EON	0x007F
#define EN29LV160A		0x2249

// new
#define MANUFACTURER_SPANSION   0X0001
#define S29GL064N       0x227E 

//#define FLASH_BASE 0xbe000000 //865x
//#define FLASH_BASE 0xbd000000 //8196B

struct flash_info {
	const __u16 mfr_id;
	const __u16 dev_id;
	const char *name;
	const u_long size;
	const int shift;  // shift number of chip size
	const int numeraseregions;
	const struct mtd_erase_region_info regions[4];
};

static int probeChip(struct DiskOnChip *doc, struct mtd_info *mtd);
//-----------------------------


extern void DoCMil_init(struct mtd_info *mtd);
static void __init DoC_Probe(unsigned long physadr)
{
	unsigned long docptr;
	struct DiskOnChip *this;
	struct mtd_info *mtd;
// david
//	int ChipID;
	char namebuf[15];
	char *name = namebuf;
	char *im_funcname = NULL;
	char *im_modname = NULL;
	void (*initroutine)(struct mtd_info *) = NULL;

	//docptr = (unsigned long)ioremap(physadr, DOC_IOREMAP_LEN);

// david ----------------------
#if 0
	docptr = physadr + 0xbfc00000;
	printk("docptr=%08x\n", (unsigned int)docptr);
	if (!docptr)
		return;
#endif
	//docptr = 0xbfc00000;
	docptr = FLASH_BASE;
//-----------------------------


	mtd = kmalloc(sizeof(struct DiskOnChip) + sizeof(struct mtd_info), GFP_KERNEL);

	if (!mtd) {
			printk(KERN_WARNING "Cannot allocate memory for data structures. Dropping.\n");
			iounmap((void *)docptr);
			return;
	}

	this = (struct DiskOnChip *)(&mtd[1]);

	memset((char *)mtd,0, sizeof(struct mtd_info));
	memset((char *)this, 0, sizeof(struct DiskOnChip));

	mtd->priv = this;
	this->virtadr = docptr;
	this->physadr = physadr;
	this->ChipID = DOC_ChipID_DocMil;

	name="Millennium";
	im_funcname = "DoCMil_init";
	im_modname = "doc2001";

	if ( probeChip(this, mtd) == 0) // david added,
		initroutine = &DoCMil_init;

	if (initroutine) {
		(*initroutine)(mtd);
		return;
	}
	printk(KERN_NOTICE "Cannot find driver for DiskOnChip %s at 0x%lX\n", name, physadr);
	iounmap((void *)docptr);
}



// david -------------------------------------------------------------------
static int probeChip(struct DiskOnChip *doc, struct mtd_info *mtd)
{
	/* Keep this table on the stack so that it gets deallocated after the
	 * probe is done.
	 */
	const struct flash_info table[] = {
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV800B,
		name: "MXIC MX29LV800B",
		size: 0x00100000,
		shift: 20,
		numeraseregions: 4,
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
		name: "MXIC MX29LV160AB",
		size: 0x00200000,
		shift: 21,
		numeraseregions: 4,
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
		name: "MXIC MX29LV320AB",
		size: 0x00400000,
		shift: 22,
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 63 }
		}
	},
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV800BB,
		name: "AMD AM29LV800BB",
		size: 0x00100000,
		shift: 20,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	},
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV160DB,
		name: "AMD AM29LV160DB",
		size: 0x00200000,
		shift: 21,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV320DB,
		name: "AMD AM29LV320DB",
		size: 0x00400000,
		shift: 22,
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 63 }
		}
	},
	{
		mfr_id: MANUFACTURER_ST,
		dev_id: M29W160DB,
		name: "ST M29W160DB",
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV640AB,
		name: "MXIC MX29LV640AB",
		size: 0x00800000,
		shift: 23,/*23 bit=> that is 8 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:   8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 127 }
		}
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV1280DB,
		name: "MXIC MX29LV1280DB",
		size: 0x01000000,
		shift: 24,/*24 bit=> that is 16 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:   8 }, //8KB 
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 255 }
		}
	},	
	{
		mfr_id: MANUFACTURER_SAMSUNG,
		dev_id: K8D1716UBC,
		name: "SAMSUNG K8D1716UBC",
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_ESMT,
		dev_id: F49L160BA,
		name: "ESMT F49L160BA",
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_ESI,
		dev_id: ES29LV320D,
		name: "ESI ES29LV320D",
		size: 0x00400000,
		shift: 22,/*22 bit=> that is 4 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 63 }
		}
	},
	{
		mfr_id: MANUFACTURER_EON,
		dev_id: EN29LV160A,
		name: "EON EN29LV160A",
		size: 0x00200000,
		shift: 21,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}
	,
	{
		mfr_id: MANUFACTURER_SPANSION,
		dev_id: S29GL064N,
		name: "SPANSION S29GL064N",
		size: 0x00800000,
		shift: 23,/*22 bit=> that is 8 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  127}
		}
	}
	};

	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr = this->virtadr;
	__u16 mfid, devid;
	int i, j, k, interleave=1, chipsize;

	// issue reset and auto-selection command
	*(volatile unsigned short *)(FLASH_BASE) = 0xf0;

	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0x90;

	mfid = *((volatile unsigned short *)docptr);
	devid = *((volatile unsigned short *)(docptr + 1*2));

        *(volatile unsigned short *)(FLASH_BASE) = 0xf0;

	for (i=0; i< sizeof(table)/sizeof(table[0]); i++) {
		if ( mfid==table[i].mfr_id && devid==table[i].dev_id)
			break;
	}
	if ( i == sizeof(table)/sizeof(table[0]) )
		return -1;

	// Look for 2nd flash
	*(volatile unsigned short *)(FLASH_BASE + table[i].size) = 0xf0;
	*(volatile unsigned short *)(FLASH_BASE + table[i].size + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(FLASH_BASE + table[i].size + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(FLASH_BASE + table[i].size + 0x555 * 2) = 0x90;

	mfid = *((volatile unsigned short *)(docptr + table[i].size));
	devid = *((volatile unsigned short *)(docptr + table[i].size + 1*2));

	*(volatile unsigned short *)(FLASH_BASE+table[i].size) = 0xf0;
	if ( mfid==table[i].mfr_id && devid==table[i].dev_id) {
		interleave++;
	}

	printk(KERN_NOTICE "Found %d x %ldM Byte %s at 0x%lx\n",
		interleave, (table[i].size)/(1024*1024), table[i].name, docptr);

	mtd->size = table[i].size*interleave;
	mtd->numeraseregions = table[i].numeraseregions*interleave;

	mtd->eraseregions = kmalloc(sizeof(struct mtd_erase_region_info) *
				    mtd->numeraseregions*interleave, GFP_KERNEL);
	if (!mtd->eraseregions) {
		printk(KERN_WARNING "Failed to allocate "
		       "memory for MTD erase region info\n");
		kfree(mtd);
		return -1;
	}

	for (k=0, chipsize=0; interleave>0; interleave--, chipsize+=table[i].size) {
		for (j=0; j<table[i].numeraseregions; j++, k++) {
			mtd->eraseregions[k].offset = table[i].regions[j].offset+chipsize;
			mtd->eraseregions[k].erasesize = table[i].regions[j].erasesize;
			mtd->eraseregions[k].numblocks = table[i].regions[j].numblocks;
			if (mtd->erasesize < mtd->eraseregions[k].erasesize)
				mtd->erasesize = mtd->eraseregions[k].erasesize;
		}
	}

	this->totlen = mtd->size;
	this->numchips = interleave;
	this->chipshift = table[i].shift;

	return 0;
}
//---------------------------------------------------------------------------




/****************************************************************************
 *
 * Module stuff
 *
 ****************************************************************************/

int __init init_doc(void)
{
// david
//	int i;

	printk(KERN_NOTICE "RealTek E-Flash System Driver. (C) 2002 RealTek Corp.\n");
	DoC_Probe(CONFIG_MTD_DOCPROBE_ADDRESS);
	/* So it looks like we've been used and we get unloaded */
	MOD_INC_USE_COUNT;
	MOD_DEC_USE_COUNT;
	return 0;

}

module_init(init_doc);
