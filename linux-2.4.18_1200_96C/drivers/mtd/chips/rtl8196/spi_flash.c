#include <linux/config.h>
#include "spi_flash.h"
#if 0
#include <linux/module.h>
#include <linux/kmod.h>
//#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

#include <asm/io.h>
#endif
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/spinlock.h>
//#include <linux/wait.h>
#define SCCR	0xb8001200
unsigned char ICver=0;
#define IC8672 	0
#define IC8196b 	1
#define IC8196b_costdown 	2
/* SPI Flash Controller */
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)
//#define RTL8196C 1
/*
 * SPI Flash Info
 */
#if 0 
const struct spi_flash_db   spi_flash_known[] =
{
   {0x01, 0x02,   1}, /* Spansion */
   {0xC2, 0x20,   0}, /* MXIC */
   {0xC2, 0x5e,   0}, /* MXIC */ //for SPI Dual/Qual  SPI chip series
   {0xC2, 0x24,   0}, /* MXIC */ // /* MXIC,MX25L1635D */ //JSW :20090121 for SPI Dual/Qual  SPI chip series ,104MHZ single I/O available
   
};
#endif

const struct spi_flash_db   spi_flash_known[] =
{
    {0x01, 0x02,1},       /* Spansion <= 8MB*/
		{0xC2, 0x20,0},       /* MXIC */
    {0xC2, 0x5e,0}, 			/* MXIC,MX25L3235D */ //JSW :20090119 for SPI Dual/Qual  SPI chip series ,104MHZ single I/O available
    {0xC2, 0x24,0}, 			/* MXIC,MX25L1635D */ //JSW :20090121 for SPI Dual/Qual  SPI chip series ,104MHZ single I/O available
    {0xBF, 0x25,0},      	/* SST */    
    {0x01, 0x20,0}, 			/* Spansion 128Mbit(16MB)*/ //0x18(24),2^24=16MB
    											/*  List of supported Multi I/O chip    */
    											/*  Verified OK*/
													/*  List of supported Multi I/O chip    */
    											/*  Not Verified yet*/
    {0xBF, 0x26,0},    		/*  SST Flash       */
    {0xEF, 0x40,0},   		/*  WindBond Flash  */

};


//#define SPI_DEBUG

/*
 * SPI Flash Info
 */
struct spi_flash_type   spi_flash_info[2];


/*
 * SPI Flash APIs
 */

/*
 * This function shall be called when switching from MMIO to PIO mode
 */
// #define __TESSPIIRAM		__attribute__ ((__section__ (".iram")))
//__TESSPIIRAM 
#if defined(CONFIG_RTL8196C)||defined(CONFIG_RTL8198)
void spi_pio_init_8198(void)
{
  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
}
#endif

void spi_pio_init(void)
{
#if defined(CONFIG_RTL8196C)||defined(CONFIG_RTL8198)
   spi_pio_init_8198();
#else
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
#endif
}
void spi_pio_init_ready(void)
{
   spi_ready();
}
void spi_pio_toggle1(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
void spi_pio_toggle2(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
}
void spi_read(unsigned int chip, unsigned int address, unsigned int *data_out)
{
   /* De-Select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* READ Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = (0x03 << 24) | (address & 0xFFFFFF);

   /* Read Data Out */
   *data_out = *(volatile unsigned int *) SFDR;

   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

#if 0
__TESSPIIRAM void spi_write(unsigned int chip, unsigned int address, unsigned int data_in)
{
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* PP Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
   *(volatile unsigned int *) SFDR = data_in;
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
#endif

 void spi_erase_chip(unsigned int chip)
{
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      /* RDSR Command */
      if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* BE Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0xC7 << 24);
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

 void spi_ready(void)
{
   while (1)
   {
      if ( (*(volatile unsigned int *) SFCSR) & READY(1))
         break;
   }
}

#if 0
 void spi_erase_sector(int sector){
	int chip=0;
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (sector * 65536);
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}
#endif
void spi_cp_probe(void)
{
   unsigned int cnt, i;
   unsigned int temp;

   for (cnt = 0; cnt < 1; cnt++)
   {
#if defined(CONFIG_RTL8196C)||defined(CONFIG_RTL8198)
      CHECK_READY;
      SPI_REG(SFCSR_8198) = SPI_CS_INIT;      //deactive CS0, CS1
      CHECK_READY;
      SPI_REG(SFCSR_8198) = 0;                //active CS0,CS1
      CHECK_READY;
      SPI_REG(SFCSR_8198) = SPI_CS_INIT;      //deactive CS0, CS1
      CHECK_READY;
      //unsigned int read_data;
      // Read Flash ID, JSW@20090304: only for 8198 new design
      SPI_REG(SFCSR_8198) = (CS_8198(cnt) | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0x9f));
      temp = (SPI_REG(SFDR2_8198) >> 8);
      *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);
      //JSW:Pre-init 8198 SPI-controller for Set Quad Mode and QE bit
      //__spi_flash_preinit_8198(read_data, cnt);
		
      spi_flash_info[cnt].maker_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].type_id = (temp >> 8) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp) & 0xFF;
      CHECK_READY;
#else
      /* Here set the default setting */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* One More Toggle (May not Necessary) */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* RDID Command */
      *(volatile unsigned int *) SFDR = 0x9F << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);
      temp = *(volatile unsigned int *) SFDR;

      spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
      spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;
      spi_ready();
#endif

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* Iterate Each Maker ID/Type ID Pair */
      for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
      {
         if ( (spi_flash_info[cnt].maker_id == spi_flash_known[i].maker_id) &&
              (spi_flash_info[cnt].type_id == spi_flash_known[i].type_id) )
         {
            spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + spi_flash_known[i].size_shift);
         }
      }

      spi_flash_info[cnt].sector_cnt = 1 << (spi_flash_info[cnt].device_size - 16);
   }
   for(i=0;i<2;i++){
	printk("get SPI CS%d\n\r",i);
	printk("maker:%x  type:%x  sector_cnt:%d\n",spi_flash_info[i].maker_id,spi_flash_info[i].type_id,spi_flash_info[i].sector_cnt);
   }
}

#if 0
 void spi_burn_image(unsigned int chip, unsigned char *image_addr, unsigned int image_size)
{
   unsigned int temp;
   unsigned int i, j, k;
   unsigned char *cur_addr;
   unsigned int cur_size;
   unsigned int cnt;

   cur_addr = image_addr;
   cur_size = image_size;

   /* Iterate Each Sector */
   for (i = 0; i < spi_flash_info[chip].sector_cnt; i++)
   {
      //unsigned int spi_data;

      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (i * 65536);
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#if 1
      printk("Erase Sector: %d\n", i);
#endif

      /* Iterate Each Page */
      for (j = 0; j < 256; j++)
      {
         if (cur_size == 0)
            break;

         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (i * 65536) | (j * 256);

         for (k = 0; k < 64; k++)
         {
            temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

            spi_ready();
            if (cur_size >= 4)
            {
               *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
               cur_size -= 4;
            }
            else
            {
               *(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1+chip) | READY(1);
               cur_size = 0;
            }

            *(volatile unsigned int *) SFDR = temp;

            cur_addr += 4;

            if (cur_size == 0)
               break;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 2000)
            {
               printk("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536+j*256);
busy:
               goto busy;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

         /* Verify Burned Image */
         /* READ Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = (0x03 << 24) | (i * 65536) | (j * 256);

         for (k = 0; k < 64; k++)
         {
            unsigned int data;

            temp = (*(cur_addr -256 + (k<<2) )) << 24 | (*(cur_addr -256 + (k<<2) + 1)) << 16 | (*(cur_addr - 256 + (k<<2) + 2)) << 8 | (*(cur_addr - 256 + (k<<2) + 3));

            data = *(volatile unsigned int *) SFDR;

            if ((data != temp))
            {
               printk("\nVerify Error at 0x%08X: Now 0x%08X, Expect 0x%08X",
                      i*65536+j*256+(k<<2), data, temp);
halt_here:
               goto halt_here;
            }
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }

      if (cur_size == 0)
         break;
   } /* Iterate Each Sector */
}
#endif
//new SPI driver


//#define MTD_SPI_SUZAKU_DEBUG

#if defined(MTD_SPI_SUZAKU_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

#define write32(a, v)       __raw_writel(v, a)
#define read32(a)           __raw_readl(a)

/* Proto-type declarations */
static u8 spi_read_status(void);
static void spi_set_cs(u32);

#define SPI_ERASING 1

static int spi_state = 0;
static spinlock_t spi_mutex = SPIN_LOCK_UNLOCKED;
//static wait_queue_head_t spi_wq;

//spinlock_t spi_lock = SPIN_LOCK_UNLOCKED;


/**
 * select which cs (chip select) line to activate
 */
inline static void spi_set_cs(u32 cs)
{
	;//write32(REG_SPISSR, !cs);
}


 static u32 spi_copy_to_dram(const u32 from, const u32 to, const u32 size)
{
	memcpy(to,from|0xbd000000,size);
	
	return 0;
}




static u32 do_spi_read(u32 from, u32 to, u32 size)
{
	//DECLARE_WAITQUEUE(wait, current);
	//unsigned long timeo;
	u32 ret;
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	ret = spi_copy_to_dram(from, to, size);
#if !defined(CONFIG_RTL8196C)|| !defined(CONFIG_RTL8198)
	spi_pio_init();
#endif
	//spin_unlock_irqrestore(spi_lock,flags);
	return ret;
}


static u32 do_spi_write(u32 from, u32 to, u32 size)
{
   unsigned int temp;
	unsigned int  remain;
   unsigned int cur_addr;
   unsigned int cur_size ,flash_addr;
   unsigned int cnt;
   unsigned int next_page_addr;
 
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
   cur_addr = from;
   flash_addr = to;
   cur_size = size;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#endif

   	   spi_pio_init();
      next_page_addr = ((flash_addr >> 8) +1) << 8;

      while (cur_size > 0)
      {
	   /* De-select Chip */
	   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	  
         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (flash_addr & 0xFFFFFF);

	   while (flash_addr != next_page_addr)
	   {
		remain = (cur_size > 4)?4:cur_size;		
		temp = *((int*)cur_addr);
		
            spi_ready();
			
            *(volatile unsigned int *) SFCSR = LENGTH(remain-1) | CS(1) | READY(1);                     
            *(volatile unsigned int *) SFDR = temp;
		
		cur_size -= remain;
		cur_addr += remain;
		flash_addr+=remain;
		
            if (cur_size == 0)
               break;;
	   }
		next_page_addr = flash_addr + 256;
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 200000)
            {
	//spin_unlock_irqrestore(spi_lock,flags);

            		return -EINVAL;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }
	//spin_unlock_irqrestore(spi_lock,flags);

	return 0;
}
static void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in)
{
        /* RDSR Command */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x05 << 24;

    while (1)
    {
        /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x0)
        {
            break;
        }
    }

        //1.release CS
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        /* WREN Command */
    spi_ready();                                    //2.waiting release OK
        //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x06 << 24;   //4.instr code
        //1.release CS
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        /* BP Command */
    spi_ready();                                    //2.waiting release OK
        //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);

        //JSW: for SST Byte Program,be aware of LENGTH @2007/9/26
        //4.instr code
    *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
        //  *(volatile unsigned int *) SFDR = (0x02<<24 ) ;  //4.instr code

        //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = (data_in<<24);
        //5.CS HIGH
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

}
static u32 do_spi_write_SST(u32 from, u32 to, u32 size)
{
	unsigned int temp;
	unsigned int  remain;
	unsigned char *cur_addr;
	unsigned int cur_size ,flash_addr;
	unsigned int cnt;
	unsigned int next_page_addr;
	unsigned int byte_cnt=0;
	short shift_cnt8=0;
	unsigned int SST_Single_Byte_Data=0,SST_Flash_Offset=0;

	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	cur_addr =(unsigned char*) from;
	flash_addr = to;
	cur_size = size;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#endif

   	   spi_pio_init();
      next_page_addr = ((flash_addr >> 8) +1) << 8;

      while (cur_size > 0)
      {
	   /* De-select Chip */
	   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	  
         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
         //*(volatile unsigned int *) SFDR = (0x02 << 24) | (flash_addr & 0xFFFFFF);

	   while (flash_addr != next_page_addr)
	   {
		remain = (cur_size > 4)?4:cur_size;
		temp =(unsigned int)(((*cur_addr) << 24 )| ((*(cur_addr + 1)) << 16) | ((*(cur_addr + 2)) << 8) | ((*(cur_addr + 3))));
		for(shift_cnt8=24;shift_cnt8>=0;shift_cnt8-=8)
		{
			 byte_cnt%=4;                        
			 SST_Single_Byte_Data=(temp>>shift_cnt8)&0xff;                        
			 SST_Flash_Offset=flash_addr+byte_cnt;
			 /* RDSR Command */                        
			 spi_ready();                        
			 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);                       
			 *(volatile unsigned int *) SFDR = 0x05 << 24;                       
			 while (1)
			 {                            
			 	if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)                            
				{                                
					break;                            
				}                        
			  }
			  sst_spi_write(0,SST_Flash_Offset,SST_Single_Byte_Data);                        
			  byte_cnt+=1;
		}
		cur_size -= remain;
		cur_addr += remain;
		flash_addr+=remain;
		
            if (cur_size == 0)
               break;;
	   }
		next_page_addr = flash_addr + 256;
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 200000)
            {
	//spin_unlock_irqrestore(spi_lock,flags);

            		return -EINVAL;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }
	//spin_unlock_irqrestore(spi_lock,flags);

	return 0;
}


/*Notice !!!
 * To comply current design, the erase function will implement sector erase
*/
static int do_spi_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_erase : [%x] ", addr);
#endif
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0x20 << 24) | addr;
//	  *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	//spin_unlock_irqrestore(spi_lock,flags);

	return 0;
}
/*
 The Block Erase function
*/
static int do_spi_block_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	//spin_unlock_irqrestore(spi_lock,flags);

	return 0;
}

static u32 do_spi_block_write(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,0xbd000000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}


//type of SPI flash we support
static const struct spi_flash_info flash_tables[] = {

	{
		mfr_id: SPANSION,
		dev_id: SPI,
		name: "spansion",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
// Support for MX2fL series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x20,
		name: "mxic",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
   {
		mfr_id: 0xC2,
		dev_id: 0x5e,
		name: "mxic",
		DeviceSize: 0x400000,
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0xC2,
		dev_id: 0x24,
		name: "mxic",
		DeviceSize: 0x200000,
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0xbf,
		dev_id: 0x25,
		name: "sst",
		DeviceSize: 0x200000,
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0x01,
		dev_id: 0x20,
		name: "spansion",
		DeviceSize: 0x1000000,
		EraseSize: SIZE_64KiB,//104MHz
	},
	{
		mfr_id: 0xbf,
		dev_id: 0x26,
		name: "sst",
		DeviceSize: 0x200000,
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0xef,
		dev_id: 0x40,
		name: "winbond",
		DeviceSize: 0x200000,
		EraseSize: 4096,//104MHz
	},


};
static struct spi_chip_info *spi_suzaku_setup(struct map_info *map)
{
	struct spi_chip_info *chip_info;

	chip_info = kmalloc(sizeof(*chip_info), GFP_KERNEL);
	if (!chip_info) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}

	memset(chip_info, 0, sizeof(struct spi_chip_info));

	return chip_info;
}
static void spi_suzaku_destroy(struct spi_chip_info *chip_info)
{
	printk("spi destroy!\n");
}

//tylo, for 8196b, test IC version
void checkICver(void){
	
		SFCR = 0xB8001200;
#if defined(CONFIG_RTL8196C)||defined(CONFIG_RTL8198)
		SFCSR= 0xB8001208;
		SFDR = 0xB800120C;
#else
		SFCSR= 0xB8001204;
		SFDR = 0xB8001208;
#endif	
}

struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp)
{
	int i;
	struct spi_chip_info *chip_info = NULL;
	
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	checkICver();
	spi_pio_init();
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);//
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR & 0x1fffffff;
#if !defined(CONFIG_RTL8196C) || !defined(CONFIG_RTL8198)
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(1);
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));
#endif
	spi_cp_probe();
	for (i=0; i < (sizeof(flash_tables)/sizeof(struct spi_flash_info)); i++) {
		//printk("%x   %x",spi_flash_info[0].maker_id ,spi_flash_info[0].type_id );
		if ( (spi_flash_info[0].maker_id == spi_flash_known[i].maker_id) &&
              		(spi_flash_info[0].type_id == spi_flash_known[i].type_id) ) {
			chip_info = spi_suzaku_setup(map);
			if (chip_info) {
				chip_info->flash      = &flash_tables[i];
				if (spi_flash_info[0].maker_id == 0xC2){
					printk("\r\nMXIC matched!!");
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
				}
				
				chip_info->destroy    = spi_suzaku_destroy;

				chip_info->read       = do_spi_read;
				
				if (flash_tables[i].EraseSize == 4096) //sector or block erase
				{
					chip_info->erase      = do_spi_erase;
					if(spi_flash_info[0].maker_id == 0xBF)
					{
						chip_info->write      = do_spi_write_SST;
					}
					else
					chip_info->write      = do_spi_write;
				}
				else
				{
					chip_info->erase      = do_spi_block_erase;
					chip_info->write      = do_spi_block_write;
				}
			}		
			printk("get SPI chip driver!\n");
			//spin_unlock_irqrestore(spi_lock,flags);
			
			return chip_info;
		}
		else{
			//printk("can not get SPI chip driver!\n");
		}
	}
	//spin_unlock_irqrestore(spi_lock,flags);

	return NULL;
}
EXPORT_SYMBOL(spi_probe_flash_chip);
//module_exit(spi_suzaku_destroy);
