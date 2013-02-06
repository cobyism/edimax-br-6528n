#include <linux/slab.h>
#include <linux/types.h>  

// david ----------------------
/* Manufacturers */
#define MANUFACTURER_MXIC	0x00C2

/* MXIC */
#define MX29LV800B		0x225B
#define MX29LV160AB		0x2249
#define MX29LV320AB		0x22A8
#define MX29LV640AB             0x22CB


#define MANUFACTURER_AMD    0x0001
/*AMD*/
#define AM29LV800BB     0x225B
#define AM29LV160DB     0x2249
#define AM29LV320DB     0x22F9

/*ST*/
#define MANUFACTURER_ST     0x0020
#define M29W160DB       0X2249

/*INTEL*/
#define MANUFACTURER_INTEL  	0x0089		 
#define TE28F160C3      	0x88C3 		 

struct erase_region_info_t {
      u32  offset;
      u32  erasesize;
      u32  numblocks;
};

struct flash_info_t {
      __u16 mfr_id;
      __u16 dev_id;
      char *name;
      u_long size;
      int shift;  // shift number of chip size
      int numeraseregions;
      struct erase_region_info_t regions[4];
};

unsigned short g_mfid, g_devid;

// david -------------------------------------------------------------------
int locate_flash_info(struct flash_info_t* flash_info, int mfid, int devid)
{
      /* Keep this table on the stack so that it gets deallocated after the
       * probe is done.
       */
      const struct flash_info_t table[] = {
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
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  63}
		  }
	    },
	    {
		  mfr_id: MANUFACTURER_MXIC,
		  dev_id: MX29LV640AB,
		  name: "MX29LV640AB-8MB",
		  size: 8*1024*1024, //8MB
		  shift: 23,
		  numeraseregions:2,
		  regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  127}
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
			{ offset: 0x010000, erasesize: 0x10000, numblocks:  63}
		  }
	    },
	    /*ST*/
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
	    }	  
	    /*INTEL*/
	    ,{                                                               
		mfr_id: MANUFACTURER_INTEL,                                        
		dev_id: TE28F160C3,                                              
		name: "Intel TE28F160C3",                                      
	        size: 0x00200000,                                               
		shift: 21,/*21 bit=> that is 2 MByte size*/                     
		numeraseregions: 2,                                                   
		regions: {                                                      
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 } 
		}                                                               
	}       
      };

      int i;
#ifndef FAKE
      for (i=0; i< sizeof(table)/sizeof(table[0]); i++) {
	    if ( mfid==table[i].mfr_id && devid==table[i].dev_id)
		  break;
      }
      if ( i == sizeof(table)/sizeof(table[0]) )
	    return -1;
      g_mfid = mfid; g_devid = devid;
#else
      i = 0;
#endif
      memcpy(flash_info, &table[i], sizeof(struct flash_info_t));
      return 0;
}


