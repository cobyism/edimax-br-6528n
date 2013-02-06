/*
	This is the easy irda control for rtl8181/rtl8186 
	media client/adaptor project
	jackson@realtek.com.tw
	2004/3/18

	2005/05/12 not to direct access i2c controller, via rtli2c
	kfchang@realtek.com.tw
*/


#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/smp_lock.h>

#include <linux/circ_buf.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/current.h>
#include <asm/fcntl.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <asm/semaphore.h>
#include <asm/rtl8181.h>

#define IRDA_MAJOR		90	
#define POLL_PERIOD		(8)
#define	I2C_BUFSIZE		64

#define SIO_CLK_DIV32		0x00
#define SIO_CLK_DIV64		0x01
#define SIO_CLK_DIV128		0x02
#define SIO_CLK_DIV256		0x03
#define SIO_ADDMODE00   0x0000
#define SIO_ADDMODE01   0x0004
#define SIO_ADDMODE02   0x0008
//#define DEBUG	1

/*
	Please refer to spec,
	if a key is pressed repeatly, then you will get 0xfe.
	previous = 0xfe else previous = 0;
*/

struct irda_client {
	struct	file		*filp;
	unsigned char 		readbuf[I2C_BUFSIZE];
	int			head;
	int 			tail;	//head and tail are used to tracking the
					//usage of readbuf
	struct  semaphore 	sem;	
	unsigned char		previous;  
	unsigned char		last;
	struct timer_list 	rx_timer;
	wait_queue_head_t 	rx_wait;
	spinlock_t		lock;
};

#define RTL_W32(reg,value32)	rtl_outl(reg,value32)	
#define RTL_R32(reg)		rtl_inl(reg)	

/* struct file_operations changed too often in the 2.1 series for nice code */
static ssize_t irda_read (struct file *filp, char *buf, size_t count, loff_t *offset);
static ssize_t irda_write (struct file *filp, const char *buf, size_t count, loff_t *offset);
static int irda_open (struct inode *inode, struct file *filp);
static int irda_release (struct inode *inode, struct file *filp);

static struct file_operations irda_fops = {
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,4,0)
	owner:		THIS_MODULE,
#endif /* LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,4,0) */
	read:		irda_read,
	open:		irda_open,
	release:	irda_release,
};

static unsigned char code_mapping(unsigned char code)
{
	
}

static unsigned int get_irdadata()
{
	//unsigned long i;
	unsigned int value;	
		
	/* marked by kfchang */
	//printk("enter get_irdadata \n");
	i2c_read_multi(0x60, 0, 0, &value, SIO_ADDMODE00);
#if 0
	RTL_W32(SIO_ADDR, SIO_RWC_R | 0x60);
	
	RTL_W32(SIO_CNR, (READ_1BYTE<<4)|SIO_ADDMODE00|ENABLE_SIO|ENGAGED_SIO);//one word address

	for(i=0;i<0xffffff;i++)
	{
		if( ((RTL_R32(SIO_CNR)) & 0x02) ==0)
			break;
	}
	if(i==0xffffff) printk("reading timeout\n");
	value = RTL_R32(SIO_DATA0);
	//printk("read value in get_irdadata=%x in %x\n",value,i);
#endif
	return value;
	//return (RTL_R32(SIO_DATA0) & 0xff);
}

/*
	We keep on polling the i2c for just one byte
*/
static void rx_timer_func (unsigned long data)
{
	unsigned char	input;
	unsigned int 	val8;
	unsigned long	flags;
	struct irda_client *client = (struct irda_client *)data;
	int i;
	unsigned char	previous_input;
#ifdef DEBUG
	printk("in timer func\n");
#endif
	//spin_lock_irqsave(&client->lock, flags);
	// keep on reading until we got 0xff
	previous_input = 0xab;
	while (1)
	{
		//for(i=0;i<0xffffff;i++);
		val8 = get_irdadata();
#ifdef DEBUG
		printk("read data=%x\n",val8); 	
#endif
		val8 >>= 24;
		//for(i=0; i<4; i++,val32 = val32 >> 8)
		{
			input = val8 & 0xff;
			if ( input == 0xff || input == 0 ) 
				goto end_readirda;
			else if (input == 0xfe)
				continue;		
			else if (input == previous_input)
				continue;
			else {
				client->last = input;
				previous_input = input;
#ifdef DEBUG
			printk("read data=%8x\n",input);
#endif
			}
			//input = code_mapping(client->last);
			
			// here we get valid input			
				
			if (CIRC_SPACE(client->head, client->tail, I2C_BUFSIZE) > 0)
			{
				client->readbuf[client->head] = input;
				client->head = (client->head + 1) & (I2C_BUFSIZE - 1);
			}	
			else
			{
				printk("I2C Buf size is not enough! head=%x, tail=%x\n", 
					client->head, client->tail);
				break;
			}
		}
	}

end_readirda:			
#ifdef DEBUG
	printk("leaving timer func\n");	
#endif
	//spin_unlock_irqrestore(&client->lock, flags);
	
	if (CIRC_CNT(client->head, client->tail, I2C_BUFSIZE))
	{
		if (!(client->filp->f_flags & O_NONBLOCK))
			wake_up_interruptible(&client->rx_wait);	
	}	

	mod_timer(&client->rx_timer, jiffies + POLL_PERIOD);
}


// for the rea
static ssize_t irda_read (struct file *filp, char *buf, size_t count,
                            loff_t *offset)
{

	int 	ret;
	int	size;
	int 	loop=0;
	int	ret_value;

	struct irda_client *client = (struct irda_client *)filp->private_data;

	if(down_interruptible(&client->sem))
		return -ERESTARTSYS;

	if (!buf)
		return 0;

	while(1)
	{
		size = CIRC_CNT(client->head, client->tail, I2C_BUFSIZE);
		if (size == 0)
		{
			if (filp->f_flags & O_NONBLOCK) // non blocking mode...
				return -EAGAIN;
			else {
				interruptible_sleep_on(&client->rx_wait);
				break;
			}
		#if 0
			if (current->signal & ~current->blocked)
				return -ERESTARTSYS;
		#endif
		}
		else
			break;
	}
	
	size = (size < count) ? size : count;
	
	if (copy_to_user((void *)buf,(void *)(client->readbuf + client->tail),size))
		ret_value = -EFAULT;
	else 
		ret_value = size;

	client->tail = (client->tail + size) & (I2C_BUFSIZE - 1);

	up(&client->sem);	

	return ret_value;
}


static void rtl_irda_init(int clock_mode)
{
#ifdef DEBUG
	printk("initialize by irda_init\n");
#endif
	i2c_init(SIO_CLK_DIV256);
}

int irda_open (struct inode *inode, struct file *filp)
{
	unsigned int minor = MINOR(inode->i_rdev);
	struct irda_client *client = NULL;

	if (minor > 0)
	{
		printk("irda.o: Trying to open unattached adapter irda-%d\n",
		       minor);
		return -ENODEV;
	}

	client = (struct irda_client *)kmalloc(sizeof (struct irda_client), GFP_KERNEL);

	if(!client)
		return -ENOMEM;

	memset((void *)client, 0, sizeof (struct irda_client));

	filp->private_data = client;
	client->filp = filp;
	sema_init(&client->sem,1);
	// below is for hw init!
	rtl_irda_init(SIO_CLK_DIV256);
	
	spin_lock_init (&client->lock);
 	init_waitqueue_head (&client->rx_wait);
	
	init_timer (&client->rx_timer);
	client->rx_timer.expires = jiffies + POLL_PERIOD;
   	client->rx_timer.data = (unsigned long) client;
    	client->rx_timer.function = &rx_timer_func;
	client->head = 0;
	client->tail = 0;
	client->last = 0;
	client->previous = 0;
	
	mod_timer(&client->rx_timer, jiffies + POLL_PERIOD);
	
	printk("irda-dev.o: opened irda-%d\n",minor);

	MOD_INC_USE_COUNT;

	return 0;
}

static int irda_release (struct inode *inode, struct file *filp)
{

	unsigned int minor = MINOR(inode->i_rdev);
	struct irda_client *client = (struct irda_client *)filp->private_data;

	del_timer(&client->rx_timer);
	kfree(filp->private_data);
	filp->private_data=NULL;
#ifdef DEBUG
	printk("irda-dev.o: Closed: irda-%d\n", minor);
#endif

	MOD_DEC_USE_COUNT;

	return 0;
}

int __init irda_dev_init(void)
{
	int res;

	printk("irda-dev.o: irda /dev entries driver module\n");

	if (register_chrdev(IRDA_MAJOR,"irda",&irda_fops)) {
		printk("irda-dev.o: unable to get major %d for i2c bus\n",
		       IRDA_MAJOR);
		return -EIO;
	}

	return 0;
}

int irda_cleanup(void)
{
	return 0;
}
__initcall (irda_dev_init);
EXPORT_NO_SYMBOLS;
