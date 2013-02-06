#include <linux/types.h>
#include <linux/config.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include "struct.h"
/*INTEL*/
#define MANUFACTURER_INTEL  	0x0089		 
#define TE28F160C3      	0x88C3 		 
extern unsigned short g_mfid, g_devid;

void issue_cmd_write(unsigned int chip_addr, unsigned int op_addr, u16 value) 
{
	int i;
      if(g_mfid == MANUFACTURER_INTEL && g_devid == TE28F160C3) {
      	*(volatile unsigned short *)(chip_addr )	= 0x40;      
      	*(volatile unsigned short *)(op_addr )	= value;      
      	i = 0xffffff;
	while(!(*(volatile unsigned short *)(chip_addr) & 0x80))
		i -- ;
      	if(!i)
		printk("program word timeout\n");

      	*(volatile unsigned short *)(op_addr) = 0xff;

      }
      else {
      	*(volatile unsigned short *)(chip_addr + 0x555 * 2)= 0xaa;
      	*(volatile unsigned short *)(chip_addr + 0x2aa * 2)= 0x55;
      	*(volatile unsigned short *)(chip_addr + 0x555 * 2)= 0xa0;
      	*(volatile unsigned short *)(op_addr )= value;      
      }
}



void issue_cmd_erase(unsigned int chip_addr, unsigned int op_addr) 
{
	int i;

      if(g_mfid == MANUFACTURER_INTEL && g_devid == TE28F160C3) {
      	// unlock blocks
      	*(volatile unsigned short *)(chip_addr) 	= 0x60;
      	*(volatile unsigned short *)(op_addr) 	= 0xd0;
      	// block erase command	
      	*(volatile unsigned short *)(chip_addr) 	= 0x20;
      	*(volatile unsigned short *)(op_addr) 	= 0xd0;
	
      	i = 0xffffff;
	while(!(*(volatile unsigned short *)(op_addr) & 0x80))
		i --;
      	if(!i)
		printk("erase flash timeout\n");
      	*(volatile unsigned short *)(op_addr) = 0xff;
      }
      else {
      	*(volatile unsigned short *)(chip_addr + 0x555 * 2) = 0xaa;
      	*(volatile unsigned short *)(chip_addr + 0x2aa * 2) = 0x55;
      	*(volatile unsigned short *)(chip_addr + 0x555 * 2) = 0x80;
      	*(volatile unsigned short *)(chip_addr + 0x555 * 2) = 0xaa;
      	*(volatile unsigned short *)(chip_addr + 0x2aa * 2) = 0x55;
      	*(volatile unsigned short *)(op_addr ) = 0x30;
      }		
}

void issue_cmd_reset(unsigned int chip_addr) 
{
      if(g_mfid == MANUFACTURER_INTEL && g_devid == TE28F160C3) 
	      *(volatile unsigned short *)(chip_addr) = 0xff;
      else
	      *(volatile unsigned short *)(chip_addr) = 0xf0;
}

void issue_cmd_probe(unsigned int chip_addr) 
{
      *(volatile unsigned short *)(chip_addr + 0x555 * 2) = 0xaa;
      *(volatile unsigned short *)(chip_addr + 0x2aa * 2) = 0x55;
      *(volatile unsigned short *)(chip_addr + 0x555 * 2) = 0x90;
}

/* Description:
 *   Device Driver will use the function 
 *   to protect some sections from erase 
 *   and write operations.
 */
int check_protection(unsigned int offset)
{
      /* Do not allow erase and write on the first 24K size */
      if ( offset < 24*1024 ) return -1;
      return 0;
}


	

