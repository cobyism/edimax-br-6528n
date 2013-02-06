/*
 * linux/drivers/char/misc.c
 *
 * Generic misc open routine by Johan Myreen
 *
 * Based on code from Linus
 *
 * Teemu Rantanen's Microsoft Busmouse support and Derrick Cole's
 *   changes incorporated into 0.97pl4
 *   by Peter Cervasio (pete%q106fm.uucp@wupost.wustl.edu) (08SEP92)
 *   See busmouse.c for particulars.
 *
 * Made things a lot mode modular - easy to compile in just one or two
 * of the misc drivers, as they are now completely independent. Linus.
 *
 * Support for loadable modules. 8-Sep-95 Philip Blundell <pjb27@cam.ac.uk>
 *
 * Fixed a failing symbol register to free the device registration
 *		Alan Cox <alan@lxorguk.ukuu.org.uk> 21-Jan-96
 *
 * Dynamic minors and /proc/mice by Alessandro Rubini. 26-Mar-96
 *
 * Renamed to misc and miscdevice to be more accurate. Alan Cox 26-Mar-96
 *
 * Handling of mouse minor numbers for kerneld:
 *  Idea by Jacques Gelinas <jack@solucorp.qc.ca>,
 *  adapted by Bjorn Ekwall <bj0rn@blox.se>
 *  corrected by Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 * Changes for kmod (from kerneld):
 *	Cyrus Durgin <cider@speakeasy.org>
 *
 * Added devfs support. Richard Gooch <rgooch@atnf.csiro.au>  10-Jan-1998
 */

#include <linux/module.h>
#include <linux/config.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/stat.h>
#include <linux/init.h>

#include <linux/tty.h>
#include <linux/selection.h>
#include <linux/kmod.h>

#include "busmouse.h"

/*
 * Head entry for the doubly linked miscdevice list
 */
static struct miscdevice misc_list = { 0, "head", NULL, &misc_list, &misc_list };
static DECLARE_MUTEX(misc_sem);

/*
 * Assigned numbers, used for dynamic minors
 */
#define DYNAMIC_MINORS 64 /* like dynamic majors */
static unsigned char misc_minors[DYNAMIC_MINORS / 8];

extern int psaux_init(void);
#ifdef CONFIG_SGI_NEWPORT_GFX
extern void gfx_register(void);
#endif
extern void streamable_init(void);
extern int rtc_DP8570A_init(void);
extern int rtc_MK48T08_init(void);
extern int ds1286_init(void);
extern int pmu_device_init(void);
extern int tosh_init(void);
extern int i8k_init(void);
extern int lcd_init(void);

static int misc_read_proc(char *buf, char **start, off_t offset,
			  int len, int *eof, void *private)
{
	struct miscdevice *p;
	int written;

	written=0;
	for (p = misc_list.next; p != &misc_list && written < len; p = p->next) {
		written += sprintf(buf+written, "%3i %s\n",p->minor, p->name ?: "");
		if (written < offset) {
			offset -= written;
			written = 0;
		}
	}
	*start = buf + offset;
	written -= offset;
	if(written > len) {
		*eof = 0;
		return len;
	}
	*eof = 1;
	return (written<0) ? 0 : written;
}


static int misc_open(struct inode * inode, struct file * file)
{
	int minor = MINOR(inode->i_rdev);
	struct miscdevice *c;
	int err = -ENODEV;
	struct file_operations *old_fops, *new_fops = NULL;
	
	down(&misc_sem);
	
	c = misc_list.next;

	while ((c != &misc_list) && (c->minor != minor))
		c = c->next;
	if (c != &misc_list)
		new_fops = fops_get(c->fops);
	if (!new_fops) {
		char modname[20];
		up(&misc_sem);
		sprintf(modname, "char-major-%d-%d", MISC_MAJOR, minor);
		request_module(modname);
		down(&misc_sem);
		c = misc_list.next;
		while ((c != &misc_list) && (c->minor != minor))
			c = c->next;
		if (c == &misc_list || (new_fops = fops_get(c->fops)) == NULL)
			goto fail;
	}

	err = 0;
	old_fops = file->f_op;
	file->f_op = new_fops;
	if (file->f_op->open) {
		err=file->f_op->open(inode,file);
		if (err) {
			fops_put(file->f_op);
			file->f_op = fops_get(old_fops);
		}
	}
	fops_put(old_fops);
fail:
	up(&misc_sem);
	return err;
}

static struct file_operations misc_fops = {
	owner:		THIS_MODULE,
	open:		misc_open,
};


/**
 *	misc_register	-	register a miscellaneous device
 *	@misc: device structure
 *	
 *	Register a miscellaneous device with the kernel. If the minor
 *	number is set to %MISC_DYNAMIC_MINOR a minor number is assigned
 *	and placed in the minor field of the structure. For other cases
 *	the minor number requested is used.
 *
 *	The structure passed is linked into the kernel and may not be
 *	destroyed until it has been unregistered.
 *
 *	A zero is returned on success and a negative errno code for
 *	failure.
 */
 
int misc_register(struct miscdevice * misc)
{
	static devfs_handle_t devfs_handle;
	struct miscdevice *c;
	
	if (misc->next || misc->prev)
		return -EBUSY;
	down(&misc_sem);
	c = misc_list.next;

	while ((c != &misc_list) && (c->minor != misc->minor))
		c = c->next;
	if (c != &misc_list) {
		up(&misc_sem);
		return -EBUSY;
	}

	if (misc->minor == MISC_DYNAMIC_MINOR) {
		int i = DYNAMIC_MINORS;
		while (--i >= 0)
			if ( (misc_minors[i>>3] & (1 << (i&7))) == 0)
				break;
		if (i<0)
		{
			up(&misc_sem);
			return -EBUSY;
		}
		misc->minor = i;
	}
	if (misc->minor < DYNAMIC_MINORS)
		misc_minors[misc->minor >> 3] |= 1 << (misc->minor & 7);
	if (!devfs_handle)
		devfs_handle = devfs_mk_dir (NULL, "misc", NULL);
	misc->devfs_handle =
	    devfs_register (devfs_handle, misc->name, DEVFS_FL_NONE,
			    MISC_MAJOR, misc->minor,
			    S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP,
			    misc->fops, NULL);

	/*
	 * Add it to the front, so that later devices can "override"
	 * earlier defaults
	 */
	misc->prev = &misc_list;
	misc->next = misc_list.next;
	misc->prev->next = misc;
	misc->next->prev = misc;
	up(&misc_sem);
	return 0;
}

/**
 *	misc_deregister - unregister a miscellaneous device
 *	@misc: device to unregister
 *
 *	Unregister a miscellaneous device that was previously
 *	successfully registered with misc_register(). Success
 *	is indicated by a zero return, a negative errno code
 *	indicates an error.
 */

int misc_deregister(struct miscdevice * misc)
{
	int i = misc->minor;
	if (!misc->next || !misc->prev)
		return -EINVAL;
	down(&misc_sem);
	misc->prev->next = misc->next;
	misc->next->prev = misc->prev;
	misc->next = NULL;
	misc->prev = NULL;
	devfs_unregister (misc->devfs_handle);
	if (i < DYNAMIC_MINORS && i>0) {
		misc_minors[i>>3] &= ~(1 << (misc->minor & 7));
	}
	up(&misc_sem);
	return 0;
}

EXPORT_SYMBOL(misc_register);
EXPORT_SYMBOL(misc_deregister);

int __init misc_init(void)
{
	create_proc_read_entry("misc", 0, 0, misc_read_proc, NULL);
#ifdef CONFIG_MVME16x
	rtc_MK48T08_init();
#endif
#ifdef CONFIG_BVME6000
	rtc_DP8570A_init();
#endif
#ifdef CONFIG_SGI_DS1286
	ds1286_init();
#endif
#ifdef CONFIG_PMAC_PBOOK
	pmu_device_init();
#endif
#ifdef CONFIG_SGI_NEWPORT_GFX
	gfx_register ();
#endif
#ifdef CONFIG_SGI_IP22
	streamable_init ();
#endif
#ifdef CONFIG_SGI_NEWPORT_GFX
	gfx_register ();
#endif
#ifdef CONFIG_TOSHIBA
	tosh_init();
#endif
#ifdef CONFIG_COBALT_LCD
	lcd_init();
#endif
#ifdef CONFIG_I8K
	i8k_init();
#endif
	if (devfs_register_chrdev(MISC_MAJOR,"misc",&misc_fops)) {
		printk("unable to get major %d for misc devices\n",
		       MISC_MAJOR);
		return -EIO;
	}
	return 0;
}




/********************************************************************
 *	Move from WLAN driver to flexibly create file system            *
 *******************************************************************/
//#ifdef CONFIG_RTL8185
#ifdef CONFIG_NET_WIRELESS_AG

#define MAX_NUM_WLANIF	4
#define WLAN_MISC_MAJOR	13
//#define DEBUG

#ifdef DEBUG
	#define DEBUG_OUT(fmt, args...)		printk(fmt, ## args)
#else
	#define DEBUG_OUT(fmt, args...)		{}
#endif

#ifdef CONFIG_DEVFS_FS
static devfs_handle_t devfs_handle1, devfs_handle2;
#endif

struct rtk8185_chr_priv {
	unsigned int			major;
	unsigned int			minor;
	struct rtk8185_priv*	wlan_priv;
	struct fasync_struct*	asoc_fasync;	// asynch notification
};


extern struct rtk8185_priv *rtk8185_chr_reg(unsigned int minor, struct rtk8185_chr_priv *priv);
extern void rtk8185_chr_unreg(unsigned int minor);

// for wlan driver module, 2005-12-26 ---
struct rtk8185_priv* (*rtk8185_chr_reg_hook)(unsigned int minor, struct rtk8185_chr_priv *priv) = NULL;
void (*rtk8185_chr_unreg_hook)(unsigned int minor) = NULL;
//---------------------------------------

static int wlanchr_fasync(int fd, struct file *filp, int mode)
{
	struct rtk8185_chr_priv *pchrdev;
	pchrdev = (struct rtk8185_chr_priv *)filp->private_data;

  	return fasync_helper(fd, filp, mode, &pchrdev->asoc_fasync);
}


static int wlanchr_open(struct inode *inode, struct file *filp)
{
	struct rtk8185_chr_priv *pchrdev;

	int minor = MINOR(inode->i_rdev);

	MOD_INC_USE_COUNT;

	if (minor >= MAX_NUM_WLANIF)
	{
		DEBUG_OUT("Sorry, try to open %dth wlan_chr dev is not allowed\n", minor);
		return -ENODEV;
	}

	pchrdev = kmalloc(sizeof(struct rtk8185_chr_priv), GFP_KERNEL);
	if (pchrdev == NULL)
	{
		DEBUG_OUT("Sorry, allowd privdata for %dth chr_dev fails\n", minor);
		return 	-ENODEV;
	}

	memset((void *)pchrdev, 0, sizeof (struct rtk8185_chr_priv));
	pchrdev->minor = minor;
	pchrdev->major = MAJOR(inode->i_rdev);

// for wlan driver module, 2005-12-26 ---
//	pchrdev->wlan_priv = rtk8185_chr_reg(minor, pchrdev);
	if (rtk8185_chr_reg_hook)
		pchrdev->wlan_priv = rtk8185_chr_reg_hook(minor, pchrdev);
	else
		pchrdev->wlan_priv = NULL;	
//---------------------------------------	
	
	filp->private_data = (void *)pchrdev;

	DEBUG_OUT("Open wlan_chr dev%d success!\n", minor);

	return  0;
}


static int wlanchr_close(struct inode *inode, struct file *filp)
{
	struct rtk8185_chr_priv *pchrdev;

	int minor = MINOR(inode->i_rdev);

	unsigned long flags;

	save_flags(flags);cli();

	// below is to un-register process queue!
	wlanchr_fasync(-1, filp, 0);

	pchrdev = (struct rtk8185_chr_priv *)filp->private_data;

	kfree(pchrdev);

// for wlan driver module, 2005-12-26 ---
//	rtk8185_chr_unreg(minor);
	if (rtk8185_chr_unreg_hook)
		rtk8185_chr_unreg_hook(minor);
//----------------------------------------

	restore_flags(flags);

	MOD_DEC_USE_COUNT;

	DEBUG_OUT("Close wlan_chr dev%d success!\n", minor);

	return 0;

}


static struct file_operations wlanchr_fops = {
   //     read:           wlanchr_read,
   //     poll:           wlanchr_poll,
   //     ioctl:          wlanchr_ioctl,
          fasync:         wlanchr_fasync,
          open:           wlanchr_open,
		  release:		  wlanchr_close,
};


int __init rtk8185_chr_init(void)
{
	// here we are gonna to register all the wlan_chr dev
#ifdef CONFIG_DEVFS_FS
	if (devfs_register_chrdev(WLAN_MISC_MAJOR, "wlan/chr", &wlanchr_fops)) {
		printk(KERN_NOTICE "Can't allocate major number %d for wl_char Devices.\n", WLAN_MISC_MAJOR);
		return -EAGAIN;
	}

	devfs_handle1 = devfs_register(NULL, "wl_chr0",
								DEVFS_FL_DEFAULT, WLAN_MISC_MAJOR, 0,
								S_IFCHR | S_IRUGO | S_IWUGO,
								&wlanchr_fops, NULL);
	devfs_handle2 = devfs_register(NULL, "wl_chr1",
								DEVFS_FL_DEFAULT, WLAN_MISC_MAJOR, 1,
								S_IFCHR | S_IRUGO | S_IWUGO,
								&wlanchr_fops, NULL);
#else
	int ret;

	if ((ret = register_chrdev(WLAN_MISC_MAJOR, "wlan/chr", &wlanchr_fops)) < 0) {
		printk(KERN_NOTICE "Can't allocate major number %d for wl_char Devices.\n", WLAN_MISC_MAJOR);
        return ret;
	}
#endif

	return 0;
}


void __exit rtk8185_chr_exit(void)
{
#ifdef CONFIG_DEVFS_FS
	devfs_unregister(devfs_handle1);
	devfs_unregister(devfs_handle2);
	devfs_unregister_chrdev(WLAN_MISC_MAJOR, "wlan/chr");
#else
	unregister_chrdev(WLAN_MISC_MAJOR, "wlan/chr");
#endif
}

// for wlan driver module, 2005-12-26 ---
EXPORT_SYMBOL(rtk8185_chr_reg_hook);
EXPORT_SYMBOL(rtk8185_chr_unreg_hook);
//---------------------------------------
#endif // CONFIG_NET_WIRELESS_AG

#ifndef CONFIG_RTL8196B

#ifdef CONFIG_NET_WIRELESS_AGN
#define MAX_NUM_WLANIF	4
#define WLAN_MISC_MAJOR	13
//#define DEBUG

#ifdef DEBUG
	#define DEBUG_OUT(fmt, args...)		printk(fmt, ## args)
#else
	#define DEBUG_OUT(fmt, args...)		{}
#endif

#ifdef CONFIG_DEVFS_FS
static devfs_handle_t devfs_handle1, devfs_handle2;
#endif

struct rtl8190_chr_priv {
	unsigned int			major;
	unsigned int			minor;
	struct rtl8190_priv*	wlan_priv;
	struct fasync_struct*	asoc_fasync;	// asynch notification
};


extern struct rtl8190_priv *rtl8190_chr_reg(unsigned int minor, struct rtl8190_chr_priv *priv);
extern void rtl8190_chr_unreg(unsigned int minor);

// for wlan driver module, 2005-12-26 ---
struct rtl8190_priv* (*rtl8190_chr_reg_hook)(unsigned int minor, struct rtl8190_chr_priv *priv) = NULL;
void (*rtl8190_chr_unreg_hook)(unsigned int minor) = NULL;
//---------------------------------------

static int wlanchr_fasync(int fd, struct file *filp, int mode)
{
	struct rtl8190_chr_priv *pchrdev;
	pchrdev = (struct rtl8190_chr_priv *)filp->private_data;

  	return fasync_helper(fd, filp, mode, &pchrdev->asoc_fasync);
}


static int wlanchr_open(struct inode *inode, struct file *filp)
{
	struct rtl8190_chr_priv *pchrdev;

	int minor = MINOR(inode->i_rdev);

	MOD_INC_USE_COUNT;

	if (minor >= MAX_NUM_WLANIF)
	{
		DEBUG_OUT("Sorry, try to open %dth wlan_chr dev is not allowed\n", minor);
		return -ENODEV;
	}

	pchrdev = kmalloc(sizeof(struct rtl8190_chr_priv), GFP_KERNEL);
	if (pchrdev == NULL)
	{
		DEBUG_OUT("Sorry, allowd privdata for %dth chr_dev fails\n", minor);
		return 	-ENODEV;
	}

	memset((void *)pchrdev, 0, sizeof (struct rtl8190_chr_priv));
	pchrdev->minor = minor;
	pchrdev->major = MAJOR(inode->i_rdev);

// for wlan driver module, 2005-12-26 ---
//	pchrdev->wlan_priv = rtl8190_chr_reg(minor, pchrdev);
	if (rtl8190_chr_reg_hook)
		pchrdev->wlan_priv = rtl8190_chr_reg_hook(minor, pchrdev);
	else
		pchrdev->wlan_priv = NULL;	
//---------------------------------------	
	
	filp->private_data = (void *)pchrdev;

	DEBUG_OUT("Open wlan_chr dev%d success!\n", minor);

	return  0;
}


static int wlanchr_close(struct inode *inode, struct file *filp)
{
	struct rtl8190_chr_priv *pchrdev;

	int minor = MINOR(inode->i_rdev);

	unsigned long flags;

	save_flags(flags);cli();

	// below is to un-register process queue!
	wlanchr_fasync(-1, filp, 0);

	pchrdev = (struct rtl8190_chr_priv *)filp->private_data;

	kfree(pchrdev);

// for wlan driver module, 2005-12-26 ---
//	rtl8190_chr_unreg(minor);
	if (rtl8190_chr_unreg_hook)
		rtl8190_chr_unreg_hook(minor);
//----------------------------------------

	restore_flags(flags);

	MOD_DEC_USE_COUNT;

	DEBUG_OUT("Close wlan_chr dev%d success!\n", minor);

	return 0;

}


static struct file_operations wlanchr_fops = {
   //     read:           wlanchr_read,
   //     poll:           wlanchr_poll,
   //     ioctl:          wlanchr_ioctl,
          fasync:         wlanchr_fasync,
          open:           wlanchr_open,
		  release:		  wlanchr_close,
};


int __init rtl8190_chr_init(void)
{
	// here we are gonna to register all the wlan_chr dev
#ifdef CONFIG_DEVFS_FS
	if (devfs_register_chrdev(WLAN_MISC_MAJOR, "wlan/chr", &wlanchr_fops)) {
		printk(KERN_NOTICE "Can't allocate major number %d for wl_char Devices.\n", WLAN_MISC_MAJOR);
		return -EAGAIN;
	}

	devfs_handle1 = devfs_register(NULL, "wl_chr0",
								DEVFS_FL_DEFAULT, WLAN_MISC_MAJOR, 0,
								S_IFCHR | S_IRUGO | S_IWUGO,
								&wlanchr_fops, NULL);
	devfs_handle2 = devfs_register(NULL, "wl_chr1",
								DEVFS_FL_DEFAULT, WLAN_MISC_MAJOR, 1,
								S_IFCHR | S_IRUGO | S_IWUGO,
								&wlanchr_fops, NULL);
#else
	int ret;

	if ((ret = register_chrdev(WLAN_MISC_MAJOR, "wlan/chr", &wlanchr_fops)) < 0) {
		printk(KERN_NOTICE "Can't allocate major number %d for wl_char Devices.\n", WLAN_MISC_MAJOR);
        return ret;
	}
#endif

	return 0;
}


void __exit rtl8190_chr_exit(void)
{
#ifdef CONFIG_DEVFS_FS
	devfs_unregister(devfs_handle1);
	devfs_unregister(devfs_handle2);
	devfs_unregister_chrdev(WLAN_MISC_MAJOR, "wlan/chr");
#else
	unregister_chrdev(WLAN_MISC_MAJOR, "wlan/chr");
#endif
}

// for wlan driver module, 2005-12-26 ---
EXPORT_SYMBOL(rtl8190_chr_reg_hook);
EXPORT_SYMBOL(rtl8190_chr_unreg_hook);
//---------------------------------------

#endif // CONFIG_NET_WIRELESS_AGN

#endif // CONFIG_RTL8196B
