/*
 *	Generic parts
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br.c,v 1.13 2010/03/15 12:12:32 bradhuang Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/if_bridge.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include "br_private.h"

#if defined(CONFIG_ATM_LANE) || defined(CONFIG_ATM_LANE_MODULE)
#include "../atm/lec.h"
#endif
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
extern int __init br_filter_init(void);
extern void __exit br_filter_exit(void);
#endif
struct proc_dir_entry *resFDB=NULL;
//unsigned int fdb_max=1024;
unsigned int fdb_max=2048;
unsigned char fdb_count[8];
void br_dec_use_count()
{
	MOD_DEC_USE_COUNT;
}

void br_inc_use_count()
{
	MOD_INC_USE_COUNT;
}
static int fdb_read_proc(char *page, char **start, off_t off,
		                     int count, int *eof, void *data)
{

      int len;
      len = sprintf(page, "%s\n", fdb_count);

      if (len <= off+count) *eof = 1;
	*start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}
#ifdef CONFIG_RTL_KERNEL_MIPS16_BRIDGE
__NOMIPS16
#endif 
static int fdb_write_proc(struct file *file, const char *buffer,
		                      unsigned long count, void *data)
{
//        char tmpbuf[8];
	if (count < 2)
   	   return -EFAULT;
	if (buffer && !copy_from_user(&fdb_count, buffer,8)) 
	{
	  fdb_max=simple_strtol(buffer,NULL,0);
	  return count;
   	}
   	return -EFAULT;
}

#ifdef IGMP_SNOOPING
struct proc_dir_entry *procigmp=NULL;
int igmpsnoopenabled=0;

static int br_igmpread_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c\n", igmpsnoopenabled + '0');


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}
#ifdef CONFIG_RTL_KERNEL_MIPS16_BRIDGE
__NOMIPS16
#endif 
static int br_igmpwrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char br_tmp; 
      if (count < 2) 
	    return -EFAULT;
      
      if (buffer && !copy_from_user(&br_tmp, buffer, 1)) {
	    igmpsnoopenabled = br_tmp - '0';
	    return count;
      }
      return -EFAULT;
}

#endif
#ifdef	IGMP_SNOOPING
struct proc_dir_entry *procIgmpProxy = NULL;

int IGMPProxyOpened = 0;

//#define IGMP_Proxy_DBG

#if !defined(CONFIG_RTL8196B_AP_ROOT) && !defined(CONFIG_RTL8196C_AP_ROOT) && !defined(CONFIG_RTL8198_AP_ROOT) && !defined(CONFIG_RTL8196C_CLIENT_ONLY)
static int br_igmpProxyRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{
#ifdef	IGMP_Proxy_DBG
	  printk("br_igmpProxyRead_proc()\n");	
#endif	  
      int len;
      len = sprintf(page, "%c\n", IGMPProxyOpened + '0');

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}
#ifdef CONFIG_RTL_KERNEL_MIPS16_BRIDGE
__NOMIPS16
#endif 
static int br_igmpProxyWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	#ifdef	IGMP_Proxy_DBG
	printk("\nbr_igmpProxyWrite_proc()\n");	
	printk("count=%d\n",count);
	printk("buffer=%s\n",buffer);	  	  
	#endif	
    unsigned char chartmp; 
	  
    if (count > 1) {	//call from shell
      	if (buffer && !copy_from_user(&chartmp, buffer, 1)) {
	    	IGMPProxyOpened = chartmp - '0';
			#ifdef	IGMP_Proxy_DBG			
			printk("IGMPProxyOpened=%d\n",IGMPProxyOpened);
			#endif
			
	    }
	}else if(count==1){//call from demon(demon direct call br's ioctl)
			//memcpy(&chartmp,buffer,1);
			if(buffer){
				get_user(chartmp,buffer);	
		    	IGMPProxyOpened = chartmp - '0';
			}else
				return -EFAULT;
			#ifdef	IGMP_Proxy_DBG			
			printk("IGMPProxyOpened=%d\n",IGMPProxyOpened);
			#endif			

	}else{
		#ifdef	IGMP_Proxy_DBG			
		printk("br_igmpProxyWrite_proc fail\n");
		#endif			
		return -EFAULT;
	}
	return count;
}
#endif
#endif

static int __init br_init(void)
{
	printk(KERN_INFO "NET4: Ethernet Bridge 008 for NET4.0\n");
	
	resFDB=create_proc_entry("fdb_max",0,NULL);
	if (resFDB) 
	{
	          resFDB->read_proc=fdb_read_proc;
	          resFDB->write_proc=fdb_write_proc;
	}
		 
#ifdef IGMP_SNOOPING	
		procigmp = create_proc_entry("br_igmpsnoop", 0, NULL);
		if (procigmp) {
		    procigmp->read_proc = br_igmpread_proc;
		    procigmp->write_proc = br_igmpwrite_proc;
		}
#endif

#ifdef MCAST_TO_UNICAST	
#if !defined(CONFIG_RTL8196B_AP_ROOT) && !defined(CONFIG_RTL8196C_AP_ROOT) && !defined(CONFIG_RTL8198_AP_ROOT) && !defined(CONFIG_RTL8196C_CLIENT_ONLY)
		procIgmpProxy = create_proc_entry("br_igmpProxy", 0, NULL);
		if (procIgmpProxy) {
		    procIgmpProxy->read_proc = br_igmpProxyRead_proc;
		    procIgmpProxy->write_proc = br_igmpProxyWrite_proc;
		}
#endif		
#endif
	br_handle_frame_hook = br_handle_frame;
	br_ioctl_hook = br_ioctl_deviceless_stub;
#if defined(CONFIG_ATM_LANE) || defined(CONFIG_ATM_LANE_MODULE)
	br_fdb_get_hook = br_fdb_get;
	br_fdb_put_hook = br_fdb_put;
#endif
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	br_filter_init();
#endif
	register_netdevice_notifier(&br_device_notifier);

	return 0;
}

static void __br_clear_frame_hook(void)
{
	br_handle_frame_hook = NULL;
}

static void __br_clear_ioctl_hook(void)
{
	br_ioctl_hook = NULL;
}

static void __exit br_deinit(void)
{
	if (resFDB) {
	          remove_proc_entry("fdb_max", resFDB);
	          resFDB = NULL;
	          }
#ifdef IGMP_SNOOPING
	if (procigmp) {
		remove_proc_entry("br_igmpsnoop", procigmp);		
		procigmp = NULL;
	}
#endif		
	unregister_netdevice_notifier(&br_device_notifier);
	br_call_ioctl_atomic(__br_clear_ioctl_hook);
	net_call_rx_atomic(__br_clear_frame_hook);
#if defined(CONFIG_ATM_LANE) || defined(CONFIG_ATM_LANE_MODULE)
	br_fdb_get_hook = NULL;
	br_fdb_put_hook = NULL;
#endif
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	br_filter_exit();
#endif
}

EXPORT_NO_SYMBOLS;

module_init(br_init)
module_exit(br_deinit)
MODULE_LICENSE("GPL");
