#include "spi_flash.h"
#include <linux/interrupt.h>
#include <asm/system.h>
#include "monitor.h"
#include "etherboot.h"
#include "nic.h"
#include <asm/mipsregs.h>                           //wei add

#include <linux/config.h> //JSW


        //#if defined(RTL8196B)
#include <asm/rtl8196.h>
        //#endif

        //#if defined(RTL8198)||	defined(RTL8196C)
        //#include <asm/rtl8198.h>
        //#endif

extern char _bootimg_start, _bootimg_end;

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define CS_8198(i)           SPI_CS_8198(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)
#define printf dprintf
#define SPI_DUMMY 0xff

unsigned int get_ctimestamp( void );
void check_spi_clk_div(void);
extern int check_dram_freq_reg(void) ;
void set_spi_clk_div(unsigned int spi_clock_div_num);

        //end of SST's SPI
void spi_sector_erase(unsigned int cnt,unsigned int i);
void EWSR(unsigned short cnt);
void WRSR(unsigned short cnt);
void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in);

        /*JSW@20091008: Add for 8196C/8198 New SPI Memory Controller*/
#ifdef SUPPORT_SPI_MIO_8198_8196C
void Set_QE_bit(unsigned short QE_bit, unsigned short cnt, unsigned int  SPI_ID);
unsigned long __spi_flash_preinit_8198(unsigned int read_data1 , unsigned int  cnt);
void spi_sector_erase_8198(unsigned int cnt, unsigned short i);
void auto_spi_memtest(unsigned long DRAM_starting_addr, unsigned int spi_clock_div_num);
void auto_spi_memtest_8198(unsigned long DRAM_starting_addr, unsigned int spi_clock_div_num);
void spi_pio_init_8198(void);
#endif
extern unsigned int rand2(void);

int flashread (unsigned long dst, unsigned int src, unsigned long length)
{
    Int16 tmp=0;
    if (length==0)
        return 1;
        /* It is the first byte              */
    if (src & 0x01)                                 /* FLASH should be 16Bit alignment   */
    {
        /* FLASH 16bit alignment             */
        tmp = rtl_inw(FLASH_BASE + src - 1);        /* FLASH 16bit access                */
        tmp = tmp & 0x00ff;
        *(Int8*)(dst) = (Int8)tmp;                  /* Byte access SDRAM                 */

        //prom_printf("%X,%X,%X,%X\n",dst,src,length,*(Int8*)(dst));

        dst=dst+1;                                  /* Finish the First Byte             */
        src=src+1;                                  /* Finish the First Byte             */
        length=length-1;                            /* Finish the First Byte             */
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

        tmp=rtl_inw(FLASH_BASE + src);              // From now on, src 16Bit aligned
        // FLASH 16bit access
        //if(src&0x01)
        //prom_printf("error");
        //if(length<2)
        //prom_printf("error");
        memcpy((Int8*)dst,&tmp,2);                  //use memcpy, avoid SDRAM alignement

        //prom_printf("%X,%X,%X,%X\n",dst,src,length,*(unsigned short*)(dst));

        dst=dst+2;
        src=src+2;
        length=length-2;
    }
    return 1;
}


        /*
         *  :SPI Flash Info  @20080613
         */
        /*
         * JSW :SPI Flash Info  @20080805
         * Note1:SST just only has single-byte program,but SST25VF064C(8MB/80MHZ) gets page-program just like MXIC
         */
const struct spi_flash_db   spi_flash_known[] =
{
        /*  List of supported SingleI/O chip    */
        /*  Verified OK*/
    {                                               /* MXIC(50MHZ): MX25L3235D/MX25L3205D(4MB),MX25L1635D/MX25L1605D(2MB) */
        0xC2, 0x20,   0
    },
    {                                               /* SST(50MHZ): SST25VF016B(2MB)/SST25VF032B(4MB)/SST25VF064C(8MB/80MHZ) */
        0xBF, 0x25,   0
    },
    {                                               /* Spansion(104MHZ):S25FL064P(8MB)*/
        0x01, 0x02,   1
    },
    {                                               /* Spansion(104MHZ):S25FL128P(16MB)*/
        0x01, 0x20,   0
    },

        /*  List of supported Multi I/O chip    */
        /*  Verified OK*/
    {                                               /* MXIC(104MHZ):MX25L3235D(4MB) */
        0xC2, 0x5e,   0
    },
    {                                               /* MXIC(104MHZ):MX25L1635D(2MB) */
        0xC2, 0x24,   0
    },
    {                                               /* WindBond(80MHZ):W25Q16(2MB)/W25Q32(4MB)/W25Q64(8MB)*/
        0xEF, 0x40,   0
    },
    {                                               /*  EON(100MHZ/Sector4KB) :EN25F32(4MB)/:EN25F16(2MB) */
        0x1C, 0x31,   0
    },
        /*  Multi I/O chip    */
        /*  Not Verified yet*/
    {                                               /*  SST*/
        0xBF, 0x26,   0
    },
    {                                               /*  EON(100MHZ/Sector64KB) :EN25P64(8MB) */
        0x1C, 0x20,   0
    },
    {                                               /*  EON(100MHZ/Sector64KB) :EN25P64(8MB) */
        0x1F, 0x47,   0
    },

};

#if 1
static unsigned int spi_flash_total_size;
static unsigned int spi_flash_num_of_chips = 0;
#endif

        /*
         * SPI Flash Info  //3
         */
struct spi_flash_type   spi_flash_info[5];          //3

        /*
         * SPI Flash APIs
         */

        /*
         * This function shall be called when switching from MMIO to PIO mode
         */
void spi_pio_init(void)
{
    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}


        //unsigned short FAST_READ=0; //1:enable,0:disable ,global variable for spi_read command
#define SPI_DUMMY 0xff

        //JSW add :Sector Erase
void spi_sector_erase(unsigned int cnt,unsigned int i)
{

        /* WREN Command */
    spi_ready();
        //*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x06 << 24;
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        /* SE Command */
    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);

        /*JSW@20090714 Note:
         Instruction code :
        =======================
        For Old 50MHZ SPI:
        Sector Erase(20h)~
        1.MXIC: 4KB
        2.Spansion : Not support
        3.SST: 4KB

        SE(D8h)~
        1.MXIC: 64KB
        2.Spansion : 64KB
        3.SST:  64KB

        ========================
        For New 104MHZ SPI:
        Sector Erase(20h)~
        1.MXIC: 4KB
        2.Spansion : 4KB
        3.SST:  4KB

        SE(D8h)~
        1.MXIC: 64KB
        2.Spansion : 64KB
        3.SST: 64KB

        */
#ifdef Set_SECTOR_ERASE_64KB
    *(volatile unsigned int *) SFDR = ( 0xD8 << 24) | (i * 65536);
#endif

#ifdef Set_SECTOR_ERASE_4KB
    *(volatile unsigned int *) SFDR = ( 0x20 << 24) | (i * 4096);
#endif

    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

        /* RDSR Command */
    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x05 << 24;

    while (1)
    {
        /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
            break;
    }

    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#if 1
#ifdef Set_SECTOR_ERASE_64KB
    printf("Erase Sector:%d->%d,Addr:0x%x->0x%x\n" ,i,\
        i+1,(0xbd000000+(i*0x10000)),(0xbd000000+(((i+1)*0x10000))-1));
#endif

#ifdef Set_SECTOR_ERASE_4KB
    printf("Erase Sector:%d->%d,Addr:0x%x->0x%x\n" ,i,\
        i+1,(0xbd000000+(i*0x1000)),(0xbd000000+(((i+1)*0x1000))-1));
#endif
#else
    prom_printf(".");
#endif
}


void spi_ready(void)
{
    while (1)
    {
        if ( (*(volatile unsigned int *) SFCSR) & READY(1))
            break;
    }
}


unsigned short SST_1Byte_SPI_Flash=1;               //20090903 added:"1"=SST single-byte,"0"=SST 4-byte style SPI
void spi_probe(void)
{
    unsigned int cnt, i;
    unsigned int temp;

    for (cnt = 0; cnt < 1; cnt++)
    {

#ifdef SUPPORT_SPI_MIO_8198_8196C
        CHECK_READY;
        SPI_REG(SFCSR_8198) = SPI_CS_INIT;          //deactive CS0, CS1
        CHECK_READY;
        SPI_REG(SFCSR_8198) = 0;                    //active CS0,CS1
        CHECK_READY;
        SPI_REG(SFCSR_8198) = SPI_CS_INIT;          //deactive CS0, CS1
        CHECK_READY;
        unsigned int read_data;
        // Read Flash ID, JSW@20090304: only for 8198 new design
        SPI_REG(SFCSR_8198) = (CS_8198(cnt) | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0x9f));
        read_data = (SPI_REG(SFDR2_8198) >> 8);
        *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

	#if SPI_DBG_MESSAGE
	        dprintf("\n===========(cnt=%d)===========\n", cnt);
	        dprintf("8198_SFCR(b8001200) =%x\n", *(volatile unsigned int *) SFCR_8198);
	        dprintf("8198_SFCR2(b8001204) =%x\n", *(volatile unsigned int *) SFCR2_8198);
	        dprintf("8198_SFCSR(b8001208) =%x\n", *(volatile unsigned int *) SFCSR_8198);
	#endif
	
        //JSW:Pre-init 8198 SPI-controller for Set Quad Mode and QE bit
        __spi_flash_preinit_8198(read_data, cnt);

        spi_flash_info[cnt].maker_id = (read_data >> 16) & 0xFF;
        spi_flash_info[cnt].type_id = (read_data >> 8) & 0xFF;
        spi_flash_info[cnt].capacity_id = (read_data) & 0xFF;
        CHECK_READY;

#else                                       //JSW:for old 8196 SPI

        /* Here set the default setting */
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1 + cnt) | READY(1);

        /* One More Toggle (Necessary) */
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
        /*20081002: JSW  For spi_probe OK, add spi_ready() */
        spi_ready();
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1 + cnt) | READY(1);
        spi_ready();

        /* RDID Command */
        *(volatile unsigned int *) SFDR = 0x9F << 24;
        *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1 + cnt) | READY(1);

	#if SPI_DBG_MESSAGE
	        dprintf("\n===========(cnt=%d)===========\n", cnt);
	        dprintf("8196_SFCR(b8001200) =%x\n", *(volatile unsigned int *) SFCR);
	        dprintf("8196_SFCSR(b8001204) =%x\n", *(volatile unsigned int *) SFCSR);
	#endif

	temp = *(volatile unsigned int *) SFDR;

        //dprintf("temp =%x\n", temp);
        

        spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
        spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
        spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;
        spi_ready();
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
#endif

        /* Iterate Each Maker ID/Type ID Pair */
        for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
        {

            if ((spi_flash_info[cnt].maker_id == spi_flash_known[i].maker_id) &&
                (spi_flash_info[cnt].type_id == spi_flash_known[i].type_id))
            {
               // dprintf("\n=============================\n");
                //dprintf("<Probe SPI #%d >\n", cnt);
                //dprintf("1.Manufacture ID=0x%x\n2.Device ID=0x%x\n3.Capacity ID=0x%x\n", spi_flash_info[cnt].maker_id, spi_flash_info[cnt].type_id, spi_flash_info[cnt].capacity_id);

                switch (spi_flash_info[cnt].capacity_id)
                {

                    case 0x8d:
                        //dprintf("4.SST SPI (0.5 MByte)!\n");
        //JSW: Modify SST size_shift for calculation
                        spi_flash_info[cnt].capacity_id -= 122;

        //All SST product need to do this first : Free lock-area in advance
                        EWSR(cnt);
                        WRSR(cnt);
        //RDSR(cnt);
                        break;

                    case 0x8e:
                        //dprintf("4.SST SPI (1 MByte)!\n");
        //JSW: Modify SST size_shift for convenience of calculation later
                        spi_flash_info[cnt].capacity_id -= 122;

        //All SST product need to do this first : Free lock-area in advance
                        EWSR(cnt);
                        WRSR(cnt);
        // RDSR(cnt);
                        break;

                    case 0x4b:
                        //dprintf("4.SST SPI (8 MByte)!\n");

                        spi_flash_info[cnt].capacity_id -= 52;

                        EWSR(cnt);
                        WRSR(cnt);
                        SST_1Byte_SPI_Flash=0;      //It's 4-Byte page program style SPI
        //RDSR(cnt);
                        break;

                    case 0x4a:
                        //dprintf("4.SST SPI (4 MByte)!\n");

                        spi_flash_info[cnt].capacity_id -= 52;

                        EWSR(cnt);
                        WRSR(cnt);
        //RDSR(cnt);
                        break;

                    case 0x41:
                        //dprintf("4.SST SPI (2 MByte)!\n");
        //JSW: Modify  SST size_shift for convenience of calculation later
                        spi_flash_info[cnt].capacity_id -= 44;

        //All SST product need to do this first : Free lock-area in advance
                        EWSR(cnt);
                        WRSR(cnt);
        //RDSR(cnt);
                        break;

                    case 0x20:
                        //dprintf("4.MXIC SPI (4 MByte)!\n");
                        break;

                    case 0x18:
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            //dprintf("4.MXIC SPI (16 MByte)!\n");
                        }
                        if (spi_flash_info[cnt].maker_id == 0x01)
                        {
                            //dprintf("4.Spansion SPI (16 MByte)!\n");
                        }
                        break;

                    case 0x17:
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            //dprintf("4.MXIC SPI (8 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0xef)
                        {
                            //dprintf("4.Winbond SPI (8 MByte)!\n");
                        }
                                                    //EON
                        else if (spi_flash_info[cnt].maker_id == 0x1c)
                        {
                            //dprintf("4.EON SPI (8 MByte)!\n");
                        }
                        break;

                    case 0x16:
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            prom_printf("4.MXIC SPI (4 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0x01)
                        {
                            prom_printf("4.Spansion SPI (8 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0xef)
                        {
                            prom_printf("4.Winbond SPI (4 MByte)!\n");
                        }
                                                    //EON
                        else if (spi_flash_info[cnt].maker_id == 0x1c)
                        {
                            //dprintf("4.EON SPI (4 MByte)!\n");
                        }
                        break;

                    case 0x15:
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            //dprintf("4.MXIC SPI (2 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0x01)
                        {
                            //dprintf("4.Spansion SPI (4 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0xef)
                        {
                            //dprintf("4.Winbond SPI (2 MByte)!\n");
                        }
                                                    //EON
                        else if (spi_flash_info[cnt].maker_id == 0x1c)
                        {
                            //dprintf("4.EON SPI (2 MByte)!\n");
                        }

                        break;

                    case 0x14:
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            //dprintf("4.MXIC SPI (1 MByte)!\n");
                        }
                        else
                        {
                            //dprintf("4.Spansion SPI (2 MByte)!\n");
                        }

                        break;

                    case 0x13:
                        //dprintf("4.Spansion SPI (1 MByte)!\n");

                        break;

                    case 0x12:
                        //dprintf("4.Spansion SPI (0.5 MByte)!\n");

                        break;

                    case 0x01:
                        //dprintf("4.ATMEL SPI (4 MByte)!\n");

                        break;

                    default :
                        //dprintf("4.Unknow type/size!\n");
                        break;
                }

                spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + spi_flash_known[i].size_shift);

#ifdef Set_SECTOR_ERASE_4KB
                spi_flash_info[cnt].sector_cnt = (1 << (spi_flash_info[cnt].device_size - 16)) * 16;
                //dprintf("5.Total sector-counts = %d(sector=4KB)\n", spi_flash_info[cnt].sector_cnt);
#else
                spi_flash_info[cnt].sector_cnt = (1 << (spi_flash_info[cnt].device_size - 16));
                //dprintf("5.Total sector-counts = %d(sector=64KB)\n", spi_flash_info[cnt].sector_cnt);
#endif

                //dprintf("6.spi_flash_info[%d].device_size = %d\n", cnt, spi_flash_info[cnt].device_size);

        //Set 8198 SPI flash size and parameter
#if defined  (SUPPORT_SPI_MIO_8198_8196C)
        //Set SPI Flash size
        //REG32(SFCR2_8198) &= 0xFF0FFFFF; //RD_OPT=0
                REG32(SFCR2_8198) &= 0xFF1FFFFF;    //RD_OPT=1
                REG32(SFCR2_8198) |= SPI_SFSIZE_8198(spi_flash_info[0].device_size - 17) ;
#if defined (CONFIG_BOOT_SIO_8198 )
#if defined (CONFIG_DDR_SDRAM)      //JSW:DDR 200/2=100MHZ will over SPI Spec,so we divide by 4 here
                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));
#elif defined (CONFIG_SDRAM)        //Set SPI clock divide Mem_clock by 2
                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));
#endif

        /*Auto Probe Low-speed SPI Flash and Set clock < 40MHZ 	*/
                                                    //Limited SPI clock <=40MHZ for older SPI flash
#ifdef CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ

        /*List table for MXIC Low-speed SPI flash:Add your older SPI Flash ID below*/
        //#define MX25L0805D  0x00C22014   /*MXIC 1MB*/
        //#define MX25L1605D  0x00C22015   /*MXIC 2MB*/
        //#define MX25L1605D  0x00C22016   /*MXIC 4MB*/
                if (((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x14)\
                    || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x15) \
                    || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x16))
                {
        	      dprintf("spi_flash.c: Set SPI clock < 40MHZ for low-speed SPI Flash\n");
                    WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));

        	      dprintf("spi_flash.c: Set No dummy cycle\n");
                    REG32(SFCR2_8198)=READ_MEM32(SFCR2_8198)&0xFFFF1FFF;
                }
#endif

                                                    /*For DIO and QIO*/
#elif defined(CONFIG_BOOT_DIO_8198) ||defined(CONFIG_BOOT_QIO_8198)
        //Set SPI clock divide Mem_clock by 4
                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));
#endif
#else   
	#ifdef CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ
	 /*List table for MXIC Low-speed SPI flash:Add your older SPI Flash ID below*/
        //#define MX25L0805D  0x00C22014   /*MXIC 1MB*/
        //#define MX25L1605D  0x00C22015   /*MXIC 2MB*/
	if ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x14)
        {
		 dprintf("spi_flash.c: Set SPI clock < 40MHZ for low-speed SPI Flash\n");
		 WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));
        	dprintf("spi_flash.c: Set No dummy cycle\n");
        	REG32(SFCR2_8198)=READ_MEM32(SFCR2_8198)&0xFFFF1FFF;
	}
	else
	#endif
	{
	                //for old 8196 //SPI clock divide Mem_clock by 6
                WRITE_MEM32(SFCR, SPI_SFSIZE(spi_flash_info[0].device_size - 17) \
                    | SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(2) \
                    | SPI_TCS(15) | SPI_RD_MODE(0) | SPI_RD_OPT(0));
	}
#endif

            }
        }

    }

}


        //#ifdef SUPPORT_SST_SPI

void EWSR(unsigned short cnt)
{
        //printf("\nEWSR\n ");

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
        //	Send_Byte(0x50);		/* enable writing to the status register */
    *(volatile unsigned int *) SFDR = 0x50 << 24;   //swap

        //	CE_High();			/* disable device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

}


        /* SST WRSR */
void WRSR(unsigned short cnt)
{
        //printf("\nWRSR\n");
    spi_ready();

        // CE_Low();			/* enable device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

        //Send_Byte(0x01);		/* select write to status register */
    *(volatile unsigned int *) SFDR = (0x01 << 24);

        //Send_Byte(byte);		/* data that will change the status of BPx  or BPL (only bits 2,3,4,5,7 can be written) */
    *(volatile unsigned int *) SFDR = (0x0<<24);    //swap ,still 0

        //CE_High();			/* disable the device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

}


void RDSR(unsigned short cnt)
{
        //printf("\nRDSR\n ");
    unsigned char byte ;
    spi_ready();

        // CE_Low();			/* enable device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

        //Send_Byte(0x05);		/* select write to status register */
    *(volatile unsigned int *) SFDR = (0x05 << 24);

    while (1)
    {
        /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
        {
            break;
        }
    }

    byte = *(volatile unsigned int *) SFDR;
        //printf("\nSTATUS(char)=%x\n", byte);

        //CE_High();			/* disable the device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

}


#if 1
void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in)
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
#endif

        //#endif
        //end of SUPPORT_SST_SPI

        // 20070827 auto-test-porgram for SPI and SDRAM

#define MIN(i, j)                ((i) < (j) ? (i) : (j))
#define MAX(i, j)                ((i) > (j) ? (i) : (j))

        //void auto_memtest(unsigned char *start,unsigned char *end)
#if !defined(SUPPORT_SPI_MIO_8198_8196C)
int spi_flw_image(unsigned int chip, unsigned int flash_addr_offset ,unsigned char *image_addr, unsigned int image_size)
{
#if 0
#ifdef Set_SECTOR_ERASE_4KB
    dprintf("\nSingle Sector : 4KB \n");
#else
    dprintf("\nSingle Sector : 64KB \n");
#endif
#endif
    unsigned int temp;
    unsigned short i, j, k;
    unsigned char *cur_addr;
    unsigned int cur_size;
    unsigned int SST_Single_Byte_Data,SST_Flash_Offset;
    short shift_cnt8;
    unsigned int byte_cnt=0;

    cur_addr = image_addr;
    cur_size = image_size;

#ifdef Set_SECTOR_ERASE_64KB
        //Sector:64KB
    unsigned int sector_start_cnt = flash_addr_offset / 65535;

    unsigned int sector_end_cnt = sector_start_cnt + (image_size / 0x10000);
#endif

#ifdef Set_SECTOR_ERASE_4KB
        //Sector:4KB
    unsigned int sector_start_cnt = flash_addr_offset / 4095;

    unsigned int sector_end_cnt = sector_start_cnt + (image_size / 0x1000);
#endif

        /* Iterate Each Sector */
    for (i = sector_start_cnt; i <= sector_end_cnt; i++)
    {
        //unsigned int spi_data;
        spi_pio_init();
        spi_sector_erase(chip,i);

        *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

        /* Iterate Each Page */
#ifdef Set_SECTOR_ERASE_64KB
        for (j = 0; j < 256; j++)                   //64KB(Sector)/256B(PageProgram)=256
#endif
#ifdef Set_SECTOR_ERASE_4KB
            for (j = 0; j < 16; j++)                //4KB(Sector)/256B(PageProgram)=16
#endif
        {
            if (cur_size == 0)
                break;

            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
            *(volatile unsigned int *) SFDR = 0x05 << 24;

            while (1)
            {

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

        /* PP Command */
            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
#ifdef Set_SECTOR_ERASE_64KB
        //Sector:64KB
            *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (i * 65536) | (j * 256);
#endif

#ifdef Set_SECTOR_ERASE_4KB
            if ((spi_flash_info[0].type_id==0x25 ))
                {}
                else
            {
        //Sector:4KB , j:page Program size
                *(volatile unsigned int *) SFDR = (0x02 << 24) | (i * 4096) | (j * 256);
            }
#endif

            for (k = 0; k < 64; k++)                //k:write 4Byte once
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

        //for SST SPI Type:Single-Byte Burning
                if ((spi_flash_info[0].type_id==0x25 ))
                {
                    for(shift_cnt8=24;shift_cnt8>=0;shift_cnt8-=8)
                    {
                        byte_cnt%=4;
                        SST_Single_Byte_Data=(temp>>shift_cnt8)&0xff;
                        SST_Flash_Offset=(i*4096)+(j*256)+(k*4)+byte_cnt;
        //Write 1 byte each time

#if 1
        /* RDSR Command */
                        spi_ready();
                        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
                        *(volatile unsigned int *) SFDR = 0x05 << 24;

                        while (1)
                        {
                            if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
                            {
                                break;
                            }
                        }
#endif

                        sst_spi_write(chip,SST_Flash_Offset,SST_Single_Byte_Data);
                        byte_cnt+=1;

                    }
                }
                else                                //for MXIC and Spansion 4 byte  burning
                {
                    *(volatile unsigned int *) SFDR = temp;
                }

                cur_addr += 4;

                if (cur_size == 0)
                    break;
            }

            *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

        /* RDSR Command */
            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
            *(volatile unsigned int *) SFDR = 0x05 << 24;

            while (1)
            {
                unsigned int status = *(volatile unsigned int *) SFDR;

        /* RDSR Command */
                if ((status & 0x01000000) == 0x00000000)
                {
                    break;
                }
#if 0                               //JSW@20090714 :Delete for code size
                unsigned int cnt=0;
                if (cnt > (1000*1000*200))
                {

                    busy:
#ifdef Set_SECTOR_ERASE_64KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536+j*256);
#endif

#ifdef Set_SECTOR_ERASE_4KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*4096+j*256);
#endif
                    goto busy;
                }
                cnt++;
#endif
            }

            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        }

        if (cur_size == 0)
            break;
    }                                               /* Iterate Each Sector */
    return 1;

}
#endif

        /*JSW@20091007: For RTL8196C/8198 New SPI architecture*/
#ifdef SUPPORT_SPI_MIO_8198_8196C

        //JSW add :Sector Erase
void spi_sector_erase_8198(unsigned int cnt, unsigned short i)
{
    spi_pio_init_8198();
        /* WREN Command */
    CHECK_READY;
        //*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
    *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        /* SE Command */
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1 + cnt) | READY(1);

#ifdef Set_SECTOR_ERASE_64KB
    *(volatile unsigned int *) SFDR_8198 = (0xD8 << 24) | (i * 65536);
#endif

#ifdef Set_SECTOR_ERASE_4KB
    *(volatile unsigned int *) SFDR_8198 = (0x20 << 24) | (i * 4096);
#endif
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

        /* RDSR Command */
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
    *(volatile unsigned int *) SFDR_8198 = 0x05 << 24;

    while (1)
    {
        /* RDSR Command */
        if (((*(volatile unsigned int *) SFDR_8198) & 0x01000000) == 0x00000000)
            break;
    }

    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

#if 0
        //dprintf("Erase Sector: %d\n", i);
#else
    prom_printf(".");
#endif

}


void Set_QE_bit(unsigned short QE_bit, unsigned short cnt, uint32 SPI_ID)
{
        //1.release CS
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        /* WREN Command */
    CHECK_READY;
        //3.CS low
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
        //4.instr code
    *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
        //1.release CS;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        //WRSR
    SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x01));

    if (QE_bit)                                     //QE=1
    {
#ifdef MXIC_SPI_QPP_Instruction_code
        SPI_REG(SFDR2_8198) = 0x40000000;           //Enable QE Bit for MXIC
#endif                                      //end of MXIC_SPI_QPP_Instruction_code

#ifdef Spansion_SPI_QPP_Instruction_code
        SPI_REG(SFDR2_8198) = 0x00020000;           //Enable QE Bit for Spansion
#endif                                      //end of Spansion_SPI_QPP_Instruction_code
    }
    else                                            //QE=0
    {
        SPI_REG(SFDR2_8198) = 0x0;
    }

}


void spi_pio_init_8198(void)
{
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
}


        /*JSW @20090416 :Pre-init 8198 SPI-controller for Set Quad Mode and QE bit
           SFCR2_CMDIO(?) depends on Vendor's Spec

        */
unsigned long __spi_flash_preinit_8198(uint32 read_data, uint32 cnt)
{
    uint32 RDSR_data;

    switch (read_data&0x00FFFFFF)
    {
        case MX25L1635D:
        case MX25L3235D:
        case MX25L6445E:
        case MX25L12845E:

#if 0
        // Read EQ bit
        //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
            dprintf("Before WRSR, RDSR_data =%x\n", RDSR_data);

        //WRSR
        //WRSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x01));
            SPI_REG(SFDR2_8198) = 0x40000000;

        // Read EQ bit
        //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
        //SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(3)|SFCSR_CMD_BYTE(0x05));//RDSR
            RDSR_data = SPI_REG(SFDR2_8198);
            dprintf("After WRSR, RDSR_data =%x\n", RDSR_data);

            if (RDSR_data&0x40000000)               // MXIC Quad Enable Bit
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
        //dprintf(" RDSR_data =%x\n",RDSR_data);
            }
            else
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
        //dprintf("RDSR_data =%x\n",RDSR_data);
            }
#endif

#if 1
#ifdef CONFIG_BOOT_SIO_8198
        //0x0b,FAST READ
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
        //Unset QUAD Enable Bit
            Set_QE_bit(0, cnt, read_data);
        //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
            //dprintf("\nSIO RDSR_data =%x\n", RDSR_data);
            //dprintf("MXIC: Set Single I/O Fast Read\n");
#endif

#ifdef CONFIG_BOOT_DIO_8198
        //0xbb,Dual READ
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xbb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(1) | SFCR2_DUMMYCYCLE(2) | SFCR2_DATAIO(1));
        //Unset QUAD Enable Bit
            Set_QE_bit(0, cnt, read_data);
        //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
            //dprintf("DIO RDSR_data =%x\n", RDSR_data);
            //dprintf("MXIC: Set DUAL I/O Read\n");
#endif

#ifdef CONFIG_BOOT_QIO_8198
        //Set QUAD Enable Bit
            Set_QE_bit(1, cnt, read_data);
        //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
            //dprintf("QIO RDSR_data =%x\n", RDSR_data);

            if (RDSR_data&0x40000000)               // MXIC Quad Enable Bit
            {
                CHECK_READY;
        //default OK
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
                CHECK_READY;
              //  dprintf("MXIC: Set QUAD I/O Read\n");
            }
            else
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
        //dprintf("RDSR_data =%x\n",RDSR_data);
                //dprintf("\n\nMXIC: Set QUAD I/O Fail!!\n\n");
            }
#endif
#endif
            break;                                  //End of MXIC

        case SST26VF016:
        case SST26VF032:
        // Set MMIO Controller Register
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_CMDIO(2) | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(1) | SFCR2_DATAIO(2) | SFCR2_HOLD_TILL_SFDR2);

        // Enter Quad Mode
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(0) | SFCSR_CMD_BYTE(0x38));
            SPI_REG(SFDR2_8198) = 0x02000000;
            break;
        //End of SST

        case W25Q80:
        case W25Q16:
        case W25Q32:
        // Read EQ bit
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x35));
            read_data = SPI_REG(SFDR2_8198);
            if (read_data&0x02000000)               // MXIC Quad Enable Bit
            {
        // Enter High Performane Mode
                SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0xA3));
                SPI_REG(SFDR2_8198) = 0x00000000;
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
            }
            else
            {
#ifdef CONFIG_BOOT_DIO_8198
                SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0xA3));
                SPI_REG(SFDR2_8198) = 0x00000000;
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xbb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_CMDIO(0) | SFCR2_ADDRIO(1) | SFCR2_DUMMYCYCLE(2) | SFCR2_DATAIO(1));
                dprintf("=>Winbond:Set Dual READ I/O SPI \n");
#else
        //SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(3)|SFCSR_CMD_BYTE(0xA3));
        //SPI_REG(SFDR2) = 0x00000000;
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
        //SPI_REG(SFCR2) = (SFCR2_SFCMD(0x0b)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(0)|SFCR2_DUMMYCYCLE(4)|SFCR2_DATAIO(0)|SFCR2_HOLD_TILL_SFDR2);
#endif

            }
            break;                                  //End of Winbond

        case S25FL032P:
#ifdef CONFIG_BOOT_QIO_8198
            Set_QE_bit(1, cnt, read_data);
        //Read Configuration Register
        //Spansion RCR(0x35)
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x35));
            read_data = SPI_REG(SFDR2_8198);

            dprintf("Spansion RCR =%x\n", read_data);

            if (read_data&0x02000000)               //If Quad enable bit set.
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
                dprintf("=>Spansion:Set QUAD READ I/O SPI \n");

            }
#endif

#ifdef CONFIG_BOOT_DIO_8198
            Set_QE_bit(0, cnt, read_data);
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xbb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(1) | SFCR2_DUMMYCYCLE(2) | SFCR2_DATAIO(1));
            dprintf("=>Spansion:Set Dual READ I/O SPI \n");
#endif

#ifdef CONFIG_BOOT_SIO_8198
            Set_QE_bit(0, cnt, read_data);
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
            dprintf("=>Spansion:Set Single READ I/O SPI \n");
#endif

            break;                                  //End of Spansion

        default:
            break;
    }
    read_data = SPI_REG(SFCR2_8198) & (~SFCR2_HOLD_TILL_SFDR2);
    SPI_REG(SFCR2_8198) = read_data;

}


int spi_flw_image_mio_8198(unsigned int cnt, unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size)
{
    unsigned int temp;
    unsigned int i, j, k;
    unsigned char *cur_addr;
    unsigned int cur_size;
    unsigned int RDSR_data;

    cur_addr = image_addr;
    cur_size = image_size;

#ifdef Set_SECTOR_ERASE_64KB
        //Sector:64KB
    unsigned int sector_start_cnt = flash_addr_offset / 0x10000;

    unsigned int sector_end_cnt = sector_start_cnt + (image_size / 0x10000);
#endif

#ifdef Set_SECTOR_ERASE_4KB
        //Sector:4KB
    unsigned int sector_start_cnt = flash_addr_offset / 0x1000;

    unsigned int sector_end_cnt = sector_start_cnt + (image_size / 0x1000);
    if ((image_size %0x1000)!=0)
        sector_end_cnt+=1;
#endif

    spi_pio_init_8198();
        //No dummy cycle
    *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1BFF;
        //1.release CS
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        /* Iterate Each Sector */
        //i:sector
    for (i = sector_start_cnt; i <= sector_end_cnt; i++)
    {
        unsigned int spi_data;

        spi_pio_init_8198();
        spi_sector_erase_8198(cnt,i);

        *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

#ifdef Set_SECTOR_ERASE_64KB
        //printf("Erase Sector:%d->%d,Addr:0x%x->0x%x\n" , i, \
            i + 1, (0xbd000000 + (i*0x10000)), (0xbd000000 + (((i + 1)*0x10000)) - 1));
	printf(".");
#endif

#ifdef Set_SECTOR_ERASE_4KB
        printf(".");
	//printf("Erase Sector:%d->%d,Addr:0x%x->0x%x\n" , i, \
            i + 1, (0xbd000000 + (i*0x1000)), (0xbd000000 + (((i + 1)*0x1000)) - 1));
#endif

        /* Iterate Each Page */
#ifdef Set_SECTOR_ERASE_64KB
        for (j = 0; j < 256; j++)                   //64KB(Sector)/256B(PageProgram)=256
#endif
#ifdef Set_SECTOR_ERASE_4KB
            for (j = 0; j < 16; j++)                //4KB(Sector)/256B(PageProgram)=16
#endif
        {
            if (cur_size == 0)
                break;

            CHECK_READY;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
            *(volatile unsigned int *) SFDR_8198 = 0x05 << 24;
            while (1)
            {
        //write in progress loop
                if (((*(volatile unsigned int *) SFDR_8198) & 0x01000000) == 0x00000000)
                {
                    break;
                }
            }

        /*JSW @ 20090414: For 8198 SPI QIO Write*/
#ifdef CONFIG_BOOT_QIO_8198

            CHECK_READY;
            SPI_REG(SFCSR_8198) = SPI_CS_INIT;      //deactive CS0, CS1
            CHECK_READY;
            SPI_REG(SFCSR_8198) = 0;                //active CS0,CS1
            CHECK_READY;
            SPI_REG(SFCSR_8198) = SPI_CS_INIT;      //deactive CS0, CS1
            CHECK_READY;

        //No dummy cycle
            *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1BFF;
        //1.release CS
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        /* WREN Command */
            CHECK_READY;
        //3.CS low
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
        //4.instr code
            *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
        //1.release CS
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

            CHECK_READY;
        //3.CS low and IO_WIDTH must =0
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(0) | CS(1 + cnt) | READY(1) | SFCSR_IO_WIDTH(0));

        //4.QPP instr code for MXIC
#ifdef MXIC_SPI_QPP_Instruction_code
            *(volatile unsigned int *) SFDR_8198 = 0x38 << 24;

            CHECK_READY;
        //for 3 Byte address and IO_WIDTH must =2
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(2) | CS(1 + cnt) | SFCSR_IO_WIDTH(2));
#endif

        //4.QPP instr code Spansion
#ifdef Spansion_SPI_QPP_Instruction_code
            *(volatile unsigned int *) SFDR_8198 = 0x32 << 24;

        //for 3 Byte address and IO_WIDTH must = 0
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(2) | CS(1 + cnt) | SFCSR_IO_WIDTH(0));
#endif

#ifdef Set_SECTOR_ERASE_64KB
        // 3 Byte address to SFDR
            *(volatile unsigned int *) SFDR_8198 = ((((i * 65535) | (j * 256)) & 0xFFFFFF) << 8);
#endif

#ifdef Set_SECTOR_ERASE_4KB
        // 3 Byte address to SFDR
            *(volatile unsigned int *) SFDR_8198 = ((((i * 4096) | (j * 256)) & 0xFFFFFF) << 8);
#endif

            CHECK_READY;
        //4Byte Data
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(3) | CS(1 + cnt) | SFCSR_IO_WIDTH(2));

#else                                   /*JSW @ 20090414: For 8198 SPI SIO or DIO Write*/

            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        /* WREN Command */
            CHECK_READY;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
            *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

        /* PP Command (0x02)*/
            CHECK_READY;
        //default OK
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1 + cnt) | READY(1);

#ifdef Set_SECTOR_ERASE_64KB
        //Sector:64KB
            *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (i * 65536) | (j * 256);
#endif

#ifdef Set_SECTOR_ERASE_4KB
        //Sector:4KB , j:page Program size
            *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (i * 4096) | (j * 256);
#endif
#endif                                  //end of CONFIG_BOOT_QIO_8198

            for (k = 0; k < 64; k++)                //k:write 4Byte once
            {
                temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

                CHECK_READY;
                if (cur_size >= 4)
                {
#ifdef CONFIG_BOOT_QIO_8198
        //4Byte Data
                    *(volatile unsigned int *) SFCSR_8198 = (LENGTH(3) | CS(1 + cnt) | SFCSR_IO_WIDTH(2));
#else
                    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1 + cnt) | READY(1);
#endif
                    cur_size -= 4;
                }
                else
                {
#ifdef CONFIG_BOOT_QIO_8198
                    *(volatile unsigned int *) SFCSR_8198 = LENGTH(cur_size - 1) | CS(1 + cnt) | SFCSR_IO_WIDTH(2);
#else
                    *(volatile unsigned int *) SFCSR_8198 = LENGTH(cur_size - 1) | CS(1 + cnt) | READY(1);
#endif
                    cur_size = 0;
                }

                *(volatile unsigned int *) SFDR_8198 = temp;
                cur_addr += 4;

                if (cur_size == 0)
                    break;
            }                                       //end of k-loop

            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

        /* RDSR Command */
            CHECK_READY;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
            *(volatile unsigned int *) SFDR_8198 = 0x05 << 24;

            unsigned int timeout = 0;
            while (1)
            {
                unsigned int status = *(volatile unsigned int *) SFDR_8198;

        /* RDSR Command */
                if ((status & 0x01000000) == 0x00000000)
                {
                    break;
                }

                if (timeout  > (1000*1000*200))
                {

                    busy:
#ifdef Set_SECTOR_ERASE_64KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536 + j*256);
#endif
#ifdef Set_SECTOR_ERASE_4KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*4096 + j*256);
#endif
                    goto busy;
                }
                timeout ++;
            }

            CHECK_READY;
#ifdef CONFIG_BOOT_SIO_8198
        //CS high
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

        //Recovery READ mode dummy cycle for SIO that SPI clcok > 40MHZ
       	 *(volatile unsigned int *) SFCR2_8198 |= SFCR2_DUMMYCYCLE(4);
        
        /*Auto Probe Low-speed SPI Flash and Set clock < 40MHZ 	*/
                                                    //Limited SPI clock <=40MHZ for older SPI flash
#ifdef CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ

        /*List table for MXIC Low-speed SPI flash:Add your older SPI Flash ID below*/
        //#define MX25L0805D  0x00C22014   /*MXIC 1MB*/
        //#define MX25L1605D  0x00C22015   /*MXIC 2MB*/
            if (((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x14)\
                || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x15))
            {
        //dprintf("spi_flash.c: Set SPI clock < 40MHZ for low-speed SPI Flash\n");
                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));

        //dprintf("spi_flash.c: Set No dummy cycle\n");
                REG32(SFCR2_8198)=READ_MEM32(SFCR2_8198)&0xFFFF1FFF;
            }
#endif
#endif

#ifdef CONFIG_BOOT_DIO_8198
        //CS high
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
        //dummy_cycle(0)
            *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1FFF;
        //Recovery READ mode 4 dummy cycle for DIO
                                                    //4
            *(volatile unsigned int *) SFCR2_8198 |= SFCR2_DUMMYCYCLE(2);
#endif

#ifdef CONFIG_BOOT_QIO_8198
        //CS high
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
        //*(volatile unsigned int *) SFCR2_8198 &=0xFFF31FFF;//dummy_cycle(0) and CMD_IO(0)
        //*(volatile unsigned int *) SFCR2_8198 = (SFCR2_DUMMYCYCLE(3)|SFCR2_CMDIO(2));//Recovery READ mode dummy cycle for QIO
        //dummy_cycle(0)
            *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1FFF;
        //Recovery READ mode 6 dummy cycle for QIO
            *(volatile unsigned int *) SFCR2_8198 |= SFCR2_DUMMYCYCLE(3);
#endif
            CHECK_READY;

        }                                           //end of j-loop

        if (cur_size == 0)
            break;
    }
printf("\n");                                               /* Iterate Each Sector */

    return 1;
}
#endif                                              //end of SUPPORT_SPI_MIO_8198_8196C

#if SPI_DBG_MESSAGE
void auto_spi_memtest_8198(unsigned long DRAM_starting_addr, unsigned int spi_clock_div_num)
{
    unsigned int i, j;
    unsigned int spi_data, rseed_timer;

        /*JSW 20070720 :
            Must set 1st SPI size to SFCR , but 2nd SPI is unnecessary.
            If total size of SPI is over 16MB,then remember 1st SPI must be the bigger size one.
        */

        /*
         JSW 20090121 :
         Modify Set SPI Parameter for SPI FAST_READ_MODE (104 MHZ)

        1.0x5e and 0x24 are MXIC's SPI_Flash_104MHZ
        */

#if SPI_DBG_MESSAGE
        /*Set SPI Parameter */
    if ((spi_flash_info[0].type_id == 0x5e) || (spi_flash_info[0].type_id == 0x24)|| (spi_flash_info[0].type_id == 0x20))
    {

        WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(7) | SPI_TCS_8198(15));

        dprintf("\nPS.ASIC mode \n");

        set_spi_clk_div(spi_clock_div_num);
    }
    else
    {

        //set 8198 SPI timing parameter
        WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(7) | SPI_TCS_8198(15));
        /*For most Single I/O SPI Flash(40MHZ),our CPU_CLK(DDR200 or SDR166) over 6 is still a safe setting.
         */
        //REG32(SFCR_8198) &=0x5FFFFFFF; //set SPI_CLK_DIV(6) and SPI_RD_MODE(1)
        // dprintf("\nPS.ASIC Normal_READ mode\n");
    }
    check_spi_clk_div();
#endif

#if FPGA_8196
        /*Set SPI Parameter */
    if ((spi_flash_info[0].type_id == 0x5e) || (spi_flash_info[0].type_id == 0x24) || (FAST_READ == 1))
    {

        WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(7) | SPI_TCS_8198(15));

        //dprintf("\nPS.It's SPI_FAST_READ_104MHZ Single I/O MODE \n" );
        dprintf("\nPS.FPGA mode FAST_READ\n");
        //REG32(SFCR_8198) &=0x1BFFFFFF; //set SPI_CLK_DIV(0) and SPI_RD_MODE(0)
        //FAST_READ=1;
    }
    else
    {

        //set 8198 SPI timing parameter
        WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(7) | SPI_TCS_8198(15));
        /*For most Single I/O SPI Flash(40MHZ),our CPU_CLK(DDR200 or SDR166) over 6 is still a safe setting.
         */
        //REG32(SFCR_8198) &=0x5FFFFFFF; //set SPI_CLK_DIV(6) and SPI_RD_MODE(1)
        dprintf("\nPS.FPGA Normal_READ mode\n");
    }
#endif

    dprintf("\n====> SPI #0 Device Size (KB): %d\n", 1 << (spi_flash_info[0].device_size - 10));

        /*JSW :SFCR setting reference table
          SFCR(0xb8001200)=0xfc780000=128KB
          SFCR(0xb8001200)=0xfcf80000=256KB
        SFCR(0xb8001200)=0xfd780000=512KB
        SFCR(0xb8001200)=0xfdf80000=1MB
        SFCR(0xb8001200)=0xfe780000=2MB
        SFCR(0xb8001200)=0xfef80000=4MB
        SFCR(0xb8001200)=0xff780000=8MB
        SFCR(0xb8001200)=0xfff80000=16MB
        */

        // Set SDRAM
    for (i = 0; i < (1 << spi_flash_info[0].device_size); i += 4)
    {
        if ((i + 4) % 0x10000 == 4)
        {
            dprintf("==> DRAM 0x%08X\n", DRAM_starting_addr   + i);
        }

        WRITE_MEM32(DRAM_starting_addr   + i, rand2());
        //JSW:Write the flash-size's(ex:2MB) random data into DRAM

    }

        /*
           for (i = 0; i < spi_flash_info[0].sector_cnt; i++)
           {
        spi_pio_init();
        spi_sector_erase(0,i);
        }

        */

        /*  JSW Note:
         *  MXIC/Spansion and SST have different instruction-code of verification
         *  So you have to mask/unmask either (MXIC/Spansion) or (SST) for 1st/2nd SPI
         *  BTW,you shall change DRAM's offset (like DRAM_starting_addr) for coverage test
         */

        //==========================================
        //1st SPI flash
        //===========================================
        // "1" for MXIC or Spansion , 0 for SST
    dprintf("\n1. Erase chip#0 and Burn Code\n");
    dprintf("\nSpi_flash_info[0].device_size=%d\n", spi_flash_info[0].device_size);
    spi_pio_init_8198();                            //Must have before burn data

    spi_flw_image_mio_8198(0, 0 , (unsigned char *)DRAM_starting_addr , (1 << spi_flash_info[0].device_size));

        //dprintf("After WRITE_MEM32....\n");
        //#ifdef CONFIG_BOOT_SIO_8198 //only pass @
#if 0
    dprintf("\n2. Code Verification by Programmed I/O\n\n");
    spi_pio_init_8198();
    for (i = 0; i < (1 << spi_flash_info[0].device_size); i += 4)
    {
        spi_read_8198(0, i, &spi_data);

        if (READ_MEM32(DRAM_starting_addr + i) != spi_data)
        {
        //dprintf("Verify Failed! Address: 0x%08X, Data: 0x%08X\n", i, spi_data);
            dprintf("Verify Fail! Flash Address: 0x%x, Burned SPI-data:0x%x \
     \n", 0xbd000000 + i, spi_data);
            dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
       \n", (DRAM_starting_addr + i), READ_MEM32(DRAM_starting_addr   + i));

        }

    }
#endif
    dprintf("\n3. Code Verification by Memory Mapped I/O\n\n");

#ifdef CONFIG_BOOT_QIO_8198
        //default OK
    SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
    dprintf("\n => SPI Enter QIO READ mode\n");
#endif

    for (i = 0; i < (1 << spi_flash_info[0].device_size); i += 4)
    {
        unsigned int dram_addr = (DRAM_starting_addr   + i);
        unsigned int spi_addr = 0xBD000000 + i;

        for (j = 0; j < 4; j++)
        {
        // 32 Bit Access
            if (j == 0)
            {

        //JSW:Check by eyes if you want
#if 0
                while (i <= 20)
                {
                    dprintf("32 Bit Access, i=%d,j=%d\n\n", i, j);
                    dprintf("Verify OK! SPI_addr: 0x%x, Burned SPI-data:0x%x \
       \n\n", 0xbd000000 + i, READ_MEM32(spi_addr + j));
                    dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
         \n\n", DRAM_starting_addr  + i, READ_MEM32(dram_addr + j));
                    break;
                }
#endif

                if (READ_MEM32(dram_addr) != READ_MEM32(spi_addr))
                {
                    dprintf("32-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
                        spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));

                    return 0;
                }
            }

        // 16 Bit Access
            if ((j == 0) || (j == 2))
            {
#if 0                               //JSW:Check by eyes if you want
                while (i <= 20)
                {
                    dprintf("16 Bit Access, i=%d,j=%d\n\n", i, j);
                    dprintf("Verify OK! SPI_addr: 0x%x, Burned SPI-data:0x%x \
       \n\n", 0xbd000000 + i, READ_MEM16(spi_addr + j));
                    dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
         \n\n", DRAM_starting_addr  + i, READ_MEM16(dram_addr + j));
                    break;
                }
#endif

                if (READ_MEM16(dram_addr + j) != READ_MEM16(spi_addr + j))
                {
                    dprintf("16-bit Verify Failed! Address: 0x%x, Data: 0x%x, Expected 0x%x\n",
                        spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));

                    return 0;
                }
            }

        // 8 Bit Access
            if ((j == 0) || (j == 1) || (j == 2) || (j == 3))
            {

#if 0                               //JSW:Check by eyes if you want
                while (i <= 20)
                {
                    dprintf("8 Bit Access, i=%d,j=%d\n\n", i, j);
                    dprintf("Verify OK! SPI_addr: 0x%x, Burned SPI-data:0x%x \
        \n\n", 0xbd000000 + i, READ_MEM8(spi_addr + j));
                    dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \
          \n\n", DRAM_starting_addr  + i, READ_MEM8(dram_addr + j));
                    break;
                }
#endif

                if (READ_MEM8(dram_addr + j) != READ_MEM8(spi_addr + j))
                {

                    dprintf("8-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
                        spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));

                    return 0;
                }
            }                                       //end if j==0/1/2/3
        }                                           //end
    }

        /*SPI #2 Auto Test*/

        /*
        dprintf("====> SPI #2 Device Size (KB): %d\n", 1 << (spi_flash_info[1].device_size - 10));

        // Set DRAM
        for (i = 0; i < (1 << spi_flash_info[1].device_size); i += 4)
        {
        if ((i + 4) % 0x10000 == 4)
        {
        dprintf("==> DRAM 0x%08X\n", DRAM_starting_addr   + i);
        }

        WRITE_MEM32(DRAM_starting_addr   + i, rand2());
        }

        dprintf("\n1. Erase Whole chip-2 and Burn Code\n");

        //spi_pio_init();
        //spi_erase_cnt(1);
        spi_pio_init();

        //==========================================
        //2nd SPI flash
        //===========================================
        // "1" for MXIC or Spansion , 0 for SST
        #if 1

        spi_flw_image(1,0 ,(unsigned char *)DRAM_starting_addr , (1 << spi_flash_info[1].device_size));

        #else
        sst_spi_burn_image(1, (unsigned char *)DRAM_starting_addr  , (1 << spi_flash_info[1].device_size));
        //Burning DRAM's data into flash
        #endif

        dprintf("2. Code Verification by Programmed I/O\n");
        spi_pio_init();
        for (i = 0; i < (1 << spi_flash_info[1].device_size); i += 4)
        {
        spi_pio_init();
        spi_read(1, i, &spi_data);

        if (READ_MEM32(DRAM_starting_addr   + i) != spi_data) //JSW:Comparing DRAM's data to flash
        {

        //dprintf("Verify Failed! Address: 0x%08X, Data: 0x%08X\n", i, spi_data);
        dprintf("Verify Fail! Flash Address: 0x%x, Burned SPI-data:0x%x \ 
        \n",(0xbd000000+i+ (1 << spi_flash_info[0].device_size)), spi_data);
        dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \ 
        \n",DRAM_starting_addr  +i,READ_MEM32(DRAM_starting_addr   + i));

        //JSW:Check by eyes if you want
        #if 1
        while(i<=20)
        {
        dprintf("i=%d\n",i);
        dprintf("Verify OK! Flash Address: 0x%x, Burned SPI-data:0x%x \ 
        \n",(0xbd000000+i+ (1 << spi_flash_info[0].device_size)), spi_data);
        dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \ 
        \n",DRAM_starting_addr  +i,READ_MEM32(DRAM_starting_addr   + i));
        break;
        }
        #endif

        }
        }

        dprintf("3. Code Verification by Memory Mapped I/O\n");
        dprintf("From 0x%08X ~ 0x%08X\n",
        0xBD000000 + (1 << spi_flash_info[0].device_size),
        0xBD000000 + (1 << spi_flash_info[0].device_size) \ 
        + MIN((1 << spi_flash_info[1].device_size), (1 << spi_flash_info[0].device_size)) - 1);

        for (i = 0; i < MIN((1 << spi_flash_info[1].device_size), (1 << spi_flash_info[0].device_size)); i += 4)
        {
        unsigned int dram_addr = DRAM_starting_addr   + i;
        unsigned int spi_addr = 0xBD000000 + (1 << spi_flash_info[0].device_size) + i;

        for (j = 0; j < 4; j++)
        {
        // 32 Bit Access
        if (j == 0)
        {

        //JSW:Check by eyes if you want
        #if 1
        while(i<=20)
        {
        dprintf("32 Bit Access, i=%d,j=%d\n\n",i,j);
        dprintf("Verify OK! SPI_addr: 0x%x, Burned SPI-data:0x%x \ 
        \n\n",(0xbd000000+i+ (1 << spi_flash_info[0].device_size)), READ_MEM32(spi_addr+j));
        dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \ 
        \n\n",DRAM_starting_addr  +i,READ_MEM32(dram_addr +j));
        break;
        }
        #endif

        if (READ_MEM32(dram_addr) != READ_MEM32(spi_addr))
        {
        dprintf("32-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
        spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));

        }
        }

        // 16 Bit Access
        if ((j == 0) || (j == 2))
        {

        #if 1//JSW:Check by eyes if you want
        while(i<=20)
        {
        dprintf("16 Bit Access, i=%d,j=%d\n\n",i,j);
        dprintf("Verify OK! SPI_addr: 0x%x, Burned SPI-data:0x%x \ 
        \n\n",(0xbd000000+i+ (1 << spi_flash_info[0].device_size)), READ_MEM16(spi_addr+j));
        dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \ 
        \n\n",DRAM_starting_addr  +i,READ_MEM16(dram_addr +j));
        break;
        }
        #endif

        if (READ_MEM16(dram_addr + j) != READ_MEM16(spi_addr + j))
        {
        dprintf("16-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
        spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));

        }
        }

        // 8 Bit Access
        if ((j == 0) || (j == 1)|| (j == 2)|| (j == 3))
        {
        #if 1 //JSW:Check by eyes if you want
        while(i<=20)
        {
        dprintf("8 Bit Access, i=%d,j=%d\n\n",i,j);
        dprintf("Verify OK! SPI_addr: 0x%x, Burned SPI-data:0x%x \ 
        \n\n",(0xbd000000+i+ (1 << spi_flash_info[0].device_size)), READ_MEM8(spi_addr+j));
        dprintf("DRAM Address: 0x%x, Correct DRAM-data:0x%x \ 
        \n\n",DRAM_starting_addr  +i,READ_MEM8(dram_addr + j));
        break;
        }
        #endif

        if (READ_MEM8(dram_addr + j) != READ_MEM8(spi_addr + j))
        {

        dprintf("8-bit Verify Failed! Address: 0x%08X, Data: 0x%08X, Expected 0x%08X\n",
        spi_addr, READ_MEM32(spi_addr), READ_MEM32(dram_addr));

        }
        }//end of j=0/1/2/3
        }
        }

        */
        //End of 2nd SPI Flash test
    dprintf("\nSPI/DRAM verify Pass! \n\n");
}


void check_spi_clk_div(void)
{

    unsigned int SPI_DIV_31_29, SPI_CLK_MHZ;
#ifdef SUPPORT_SPI_MIO_8198_8196C
    CHECK_READY;
    SPI_DIV_31_29 = READ_MEM32(SFCR_8198) >> 29;
#else
    spi_ready();
    SPI_DIV_31_29 = READ_MEM32(SFCR) >> 29;
#endif

        //dprintf("1X DRAM_CLK=%d\n",check_dram_freq_reg() );
    dprintf("\n====================================\n");

    switch (SPI_DIV_31_29)
    {
        case 0:
            dprintf("\nSPI_CLK_DIV : 2\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 2;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 1:
            dprintf("\nSPI_CLK_DIV : 4\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 4;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 2:
            dprintf("\nSPI_CLK_DIV : 6\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 6;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 3:
            dprintf("\nSPI_CLK_DIV : 8\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 8;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 4:
            dprintf("\nSPI_CLK_DIV : 10\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 10;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 5:
            dprintf("\nSPI_CLK_DIV : 12\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 12;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 6:
            dprintf("\nSPI_CLK_DIV : 14\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 14;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        case 7:
            dprintf("\nSPI_CLK_DIV : 16\n");
            SPI_CLK_MHZ = check_dram_freq_reg() / 16;
            dprintf("\nSPI_CLK : %dMHZ\n", SPI_CLK_MHZ);
            break;

        default :
            dprintf("Error SPI_CLK_DIV number,should be 0~7");
            break;
    }
    dprintf("\n====================================\n\n");

}


void set_spi_clk_div(unsigned int spi_clock_div_num)
{
#ifdef SUPPORT_SPI_MIO_8198_8196C
    CHECK_READY;
    REG32(SFCR_8198) &= 0x1FFFFFFF;                 //DIV by 2
#else
    spi_ready();
    REG32(SFCR) &= 0x1FFFFFFF;                      //DIV by 2
#endif

    switch (spi_clock_div_num)
    {
        case 0:
            REG32(SFCR_8198) |= 0x1FFFFFFF;         //DIV by 2
            break;

        case 1:
            REG32(SFCR_8198) |= 0x3FFFFFFF;         //DIV by 4
            break;

        case 2:
            REG32(SFCR_8198) |= 0x5FFFFFFF;         //DIV by 6
            break;

        case 3:
            REG32(SFCR_8198) |= 0x7FFFFFFF;         //DIV by 8
            break;

        case 4:
            REG32(SFCR_8198) |= 0x7FFFFFFF;         //DIV by 10
            break;

        case 5:
            REG32(SFCR_8198) |= 0x7FFFFFFF;         //DIV by 12
            break;

        case 6:
            REG32(SFCR_8198) |= 0x7FFFFFFF;         //DIV by 14
            break;

        case 7:
            REG32(SFCR_8198) |= 0x7FFFFFFF;         //DIV by 16
            break;

        default :
            dprintf("Error SPI_CLK_DIV number,should be 0~7");
            break;
    }
    dprintf("\n====================================\n\n");
    prom_printf("\nSFCR_8198(0xb8001200)=%x\n", READ_MEM32(SFCR_8198));

}

#endif
