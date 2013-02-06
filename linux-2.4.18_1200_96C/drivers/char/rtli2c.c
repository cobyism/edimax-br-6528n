/*
	This is the easy i2c control for rtl8181/rtl8186 
	only support I2C write one bye and Read Multy-byte(1~4)
	the following is reading result
	1 byte (XX------)
	2 byte (XXXX----)
	3 byte (XXXXXX--)
	4 byte (XXXXXXXX)
	you should deal it carefully.
	writing method is (------XX)
	XX --> mean the data you want to read or write
	-- --> mean don't care
	kfchang@realtek.com.tw
	2005/4/21
*/


#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>        // error codes
#include <linux/types.h>        // size_t
#include <linux/delay.h>        // udelay
#include <linux/interrupt.h>    // probe_irq_on, probe_irq_off
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/smp_lock.h>
#include <linux/ioctl.h>
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/current.h>
#include <asm/fcntl.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <asm/semaphore.h>
#include <asm/rtl8181.h>

#define SIO_CNR         0x80
#define SIO_STR         0x84
#define SIO_CKDIV       0x88
#define SIO_ADDR        0x8c
#define SIO_DATA0       0x90
#define SIO_DATA1       0x94
#define SIO_DATA2       0x98
#define SIO_DATA3       0x9c

#define SIO_SIZE0       0x0000
#define SIO_SIZE1       0x0010
#define SIO_SIZE2       0x0020
#define SIO_SIZE3       0x0030
#define SIO_SIZE4       0x0040
#define SIO_SIZE5       0x0050
#define SIO_ADDMODE00   0x0000
#define SIO_ADDMODE01   0x0004
#define SIO_ADDMODE02   0x0008
#define SIO_READISR_EN  0x0100
#define SIO_WRITEISR_EN 0x0200

#define SIO_RWC_R       0x0001
#define SIO_RWC_W       0x0000

#define ENABLE_SIO      0x0001
#define ENGAGED_SIO     0x0002

#define SIO_CLK_DIV32   0x00
#define SIO_CLK_DIV64   0x01
#define SIO_CLK_DIV128  0x02
#define SIO_CLK_DIV256  0x03

//for I2C
#define I2C_MAJOR		89

#define DEBUG	1

#define POLL_PERIOD		(8)

#define	I2C_BUFSIZE		64
void i2c_byte_write(int i2c_addr, int addr,int data);
void i2c_read_multi(int i2c_addr, int addr, int byte_count,volatile unsigned long *data0, unsigned long mode);
int rtli2c_open (struct inode *inode, struct file *filp);
int rtli2c_release (struct inode *inode, struct file *filp);
int rtli2c_ioctl(struct inode *inode, struct file *filp,  unsigned int cmd, unsigned long arg);

struct i2c_transaction{
	unsigned int i2c_addr;		/*target i2c device's address */
	unsigned int i2c_rw;		/*read=0, write =1*/
	unsigned int len;		/*tranction len*/
	unsigned int addr;		/*register address*/
	unsigned int data;		
};

struct i2c_private{
	struct	file		*filp;
/*
	unsigned char 		readbuf[I2C_BUFSIZE];
	int			head;
	int 			tail;	// head and tail are used to tracking the 
					// usage of readbuf
	unsigned char		previous;  
	unsigned char		last;
	struct timer_list 	rx_timer;
	wait_queue_head_t 	rx_wait;
*/
//	struct  semaphore 	sem_rx;	
//	struct  semaphore 	sem_tx;	
	unsigned long		slave_i2c_addr;
//	unsigned long		ioaddr;			
	spinlock_t		lock;
};
struct i2c_private *prv;
unsigned long mysem_read, mysem_write;

#define RTL_W32(reg,value32)	rtl_outl(reg,value32)	
				
#define RTL_R32(reg)		rtl_inl(reg)	

/* struct file_operations changed too often in the 2.1 series for nice code */
/*
ssize_t i2c_read (struct file *filp, char *buf, size_t count, 
                            loff_t *offset);
ssize_t i2c_write (struct file *filp, const char *buf, size_t count, 
                             loff_t *offset);
*/

int i2c_open (struct inode *inode, struct file *filp);

int i2c_release (struct inode *inode, struct file *filp);

struct file_operations rtli2c_fops = {
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,4,0)
	owner:		THIS_MODULE,
#endif /* LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,4,0) */
	open:		rtli2c_open,
	release:	rtli2c_release,
	ioctl:		rtli2c_ioctl,
};


#define READ_1BYTE	0 // 1 bytes per read
#define READ_2BYTE	1 // 2 bytes per read
#define READ_3BYTE	2 // 3 bytes per read
#define READ_4BYTE	3 // 4 bytes per read
#define READ 	0
#define WRITE	1


int rtli2c_ioctl(struct inode *inode, struct file *filp,  unsigned int cmd, unsigned long arg)
{
        int retval = 0;
	struct i2c_transaction i2c_req;
	int byte_count;

        switch(cmd)
        {
                case READ:
			copy_from_user(&i2c_req, (void *)arg, sizeof(struct i2c_transaction));
			i2c_read_multi(i2c_req.i2c_addr, i2c_req.addr, i2c_req.len, &(i2c_req.data), SIO_ADDMODE01);
			copy_to_user((void *)arg,&i2c_req,sizeof(struct i2c_transaction));
                        break;
                case WRITE:
			copy_from_user(&i2c_req, (void *)arg, sizeof(struct i2c_transaction));
			i2c_byte_write(i2c_req.i2c_addr, i2c_req.addr, i2c_req.data);
                        break;
        }

        return retval;
}



void i2c_byte_write(int i2c_addr, int addr,int data)
{
	unsigned long comm;
	unsigned long i;
	unsigned long flags;
	//WaitKey();
	comm= (0xff & addr)<<24 | (data & 0xff)<<16;
	//printk("comm=%x\n",comm); //kfchang
	spin_lock_irqsave(prv->lock, flags);
	
	//down_interruptible(&prv->sem_tx);

	rtl_outl(SIO_ADDR,SIO_RWC_W | i2c_addr ); 
	rtl_outl(SIO_DATA0,comm);//
	rtl_outl(SIO_CNR, SIO_SIZE1 |SIO_ADDMODE00|ENABLE_SIO | ENGAGED_SIO);//one word address
	for(i=0;i<0xfff;i++)
	{
		if( (rtl_inl(SIO_CNR) & 0x02)==0) 
		{
			//flush_dcache();
			//prom_printf("i=%x\n",i); //kfchang
			break;
		}	
		//schedule((unsigned long)(get_ctxt_buf(&(sys_tasks[I2C_TASK]))));	
	}

	spin_unlock_irqrestore(prv->lock, flags);
	//up(&prv->sem_tx);

}

void i2c_read_multi(int i2c_addr, int addr, int byte_count,volatile unsigned long *data0,unsigned long mode)
{
	unsigned long i;
	unsigned long flags;
	if (byte_count>4) {
		//prom_printf("The read string too much.\n");
		return NULL;
	}
	spin_lock_irqsave(prv->lock, flags);
	//down_interruptible(&prv->sem_rx);

	//WaitKey();
	rtl_outl(SIO_ADDR,SIO_RWC_R | (i2c_addr&0xff) | (addr<<8) ); // 0xE3 read
	rtl_outl(SIO_CNR,(byte_count<<4)|mode|ENABLE_SIO | ENGAGED_SIO);//one word address
	for(i=0;i<0xfff;i++)
	{
		if( (rtl_inl(SIO_CNR) & 0x02)==0)
		{
			//flush_dcache();
			//prom_printf("i=%x\n",i); 
			break;
		}
		//schedule((unsigned long)(get_ctxt_buf(&(sys_tasks[I2C_TASK]))));	
	}
	
	*data0 =rtl_inl(SIO_DATA0);

	spin_unlock_irqrestore(prv->lock, flags);
	//up(&prv->sem_rx);
	//return tmp;
}

int rtli2c_open (struct inode *inode, struct file *filp)
{
	unsigned int minor = MINOR(inode->i_rdev);

	/*not to open multiple time */

	if (minor > 0)
	{
		printk("rtli2c.o: Trying to open unattached adapter rtli2c-%d\n", minor);
		return -ENODEV;
	}
//	filp->private_data = prv;
//	prv->filp = filp;
	spin_lock_init (&prv->lock);
	printk("rtli2c.o: opened rtli2c-%d\n",minor);
//	MOD_INC_USE_COUNT;

	return 0;
}

int rtli2c_release (struct inode *inode, struct file *filp)
{

	unsigned int minor = MINOR(inode->i_rdev);
	struct i2c_private *prv= (struct i2c_private *)filp->private_data;
/*
	del_timer(&client->rx_timer);
*/
	kfree(filp->private_data);
	filp->private_data=NULL;
#ifdef DEBUG
	printk("rtli2c.o: Closed: rtli2c-%d\n", minor);
#endif

//	MOD_DEC_USE_COUNT;

	return 0;
}
void i2c_init(int clock_mode)
{
        rtl_outl(SIO_CNR,ENABLE_SIO ); //enable SIO
        rtl_outl(SIO_CKDIV,clock_mode); //set clock speed
        rtl_outl(SIO_STR,0xffffffff);
}

int __init rtli2c_init(void)
{
	int res;

	printk("rtli2c.o: i2c entries driver module\n");

	if (register_chrdev(I2C_MAJOR,"rtli2c",&rtli2c_fops)) {
		printk("rtli2c.o: unable to get major %d for i2c bus\n", I2C_MAJOR);
		return -EIO;
	}

	// below is for hw init!
	i2c_init(SIO_CLK_DIV256);
	prv = (struct i2c_private *)kmalloc(sizeof (struct i2c_private), GFP_KERNEL);
        if(!prv)
                return -ENOMEM;
        memset((void *)prv, 0, sizeof (struct i2c_private));
//        sema_init(&prv->sem_rx,1);
//        sema_init(&prv->sem_tx,1);
	mysem_read=0;
	mysem_write=0;

	return 0;
}

void rtli2c_cleanup(void){

	unregister_chrdev(I2C_MAJOR, "rtli2c");
	kfree(prv);
}

__initcall (rtli2c_init);
EXPORT_SYMBOL(i2c_byte_write);
EXPORT_SYMBOL(i2c_read_multi);
