/*
 * meth.c -- O2 Builtin 10/100 Ethernet driver, that doesn't work at the moment
 *
 * Copyright (C) 2001 Ilya Volynets
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef __KERNEL__
#  define __KERNEL__
#endif

#ifndef MFE_DEBUG
#define MFE_DEBUG 0
#endif

#if MFE_DEBUG>=1
#define DPRINTK(str,args...) printk (KERN_DEBUG "meth: %s: " str, __FUNCTION__ , ## args)
#define MFE_RX_DEBUG 2
#else
#define DPRINTK(str,args...)
#define MFE_RX_DEBUG 0
#endif


#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h> /* printk() */
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/interrupt.h> /* mark_bh */
#include <linux/pci.h>

#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/tcp.h>         /* struct tcphdr */
#include <linux/skbuff.h>
#include <linux/mii.h> /*MII definitions */

#include <asm/ip32/crime.h>
#include <asm/ip32/mace.h>
#include <asm/ip32/ip32_ints.h>

#include "meth.h"

#include <linux/in6.h>
#include <asm/checksum.h>

static const char *version="meth.c: Ilya Volynets (ilya@theIlya.com)";
static const char *meth_str="SGI O2 Fast Ethernet";
MODULE_AUTHOR("Ilya Volynets");
MODULE_DESCRIPTION("SGI O2 Builtin Fast Ethernet driver");

/* This is a load-time options */
/*static int eth = 0;
MODULE_PARM(eth, "i");*/
/*
#ifdef HAVE_TX_TIMEOUT
static int timeout = 0;
MODULE_PARM(timeout, "i");
#endif
*/
int meth_eth;


/*
 * This structure is private to each device. It is used to pass
 * packets in and out, so there is place for a packet
 */

typedef struct meth_private {
    struct net_device_stats stats;
	volatile struct meth_regs *regs;
	u64 mode; /* in-memory copy of MAC control register */
	int  phy_addr; /* address of phy, used by mdio_* functions, initialized in mdio_probe*/
	tx_packet *tx_ring;
	dma_addr_t tx_ring_dma;
	int free_space;
	struct sk_buff *tx_skbs[TX_RING_ENTRIES];
	dma_addr_t      tx_skb_dmas[TX_RING_ENTRIES];
	int tx_read,tx_write;

	rx_packet *rx_ring[RX_RING_ENTRIES];
	dma_addr_t rx_ring_dmas[RX_RING_ENTRIES];
	int rx_write;

    spinlock_t meth_lock;
} meth_private;

extern struct net_device meth_devs[];
void meth_tx_timeout (struct net_device *dev);
void meth_interrupt(int irq, void *dev_id, struct pt_regs *pregs);
        
/* global, initialized in ip32-setup.c */
char o2meth_eaddr[8]={0,0,0,0,0,0,0,0};

static inline void load_eaddr(struct net_device *dev,
				   volatile struct meth_regs *regs)
{
	int i;
	DPRINTK("Loading MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		(int)o2meth_eaddr[0]&0xFF,(int)o2meth_eaddr[1]&0xFF,(int)o2meth_eaddr[2]&0xFF,
		(int)o2meth_eaddr[3]&0xFF,(int)o2meth_eaddr[4]&0xFF,(int)o2meth_eaddr[5]&0xFF);
	//memcpy(dev->dev_addr,o2meth_eaddr+2,6);
	for (i=0; i<6; i++)
		dev->dev_addr[i]=o2meth_eaddr[i];
	regs->mac_addr= //dev->dev_addr[0]|(dev->dev_addr[1]<<8)|
					//dev->dev_addr[2]<<16|(dev->dev_addr[3]<<24)|
					//dev->dev_addr[4]<<32|(dev->dev_addr[5]<<40);
	(*(u64*)o2meth_eaddr)>>16;
	DPRINTK("MAC, finally is %0lx\n",regs->mac_addr);
}

/*
 *Waits for BUSY status of mdio bus to clear
 */
#define WAIT_FOR_PHY(___regs, ___rval)			\
	while((___rval=___regs->phy_data)&MDIO_BUSY){	\
		udelay(25);								\
	}
/*read phy register, return value read */
static int mdio_read(meth_private *priv,int phyreg)
{
	volatile meth_regs* regs=priv->regs;
	volatile u32 rval;
	WAIT_FOR_PHY(regs,rval)
	regs->phy_registers=(priv->phy_addr<<5)|(phyreg&0x1f);
	udelay(25);
	regs->phy_trans_go=1;
	udelay(25);
	WAIT_FOR_PHY(regs,rval)
	return rval&MDIO_DATA_MASK;
}

/*write phy register */
static void mdio_write(meth_private* priv,int pfyreg,int val)
{
	volatile meth_regs* regs=priv->regs;
	int rval;
///	DPRINTK("Trying to write value %i to reguster %i\n",val, pfyreg);
///	spin_lock_irq(&priv->meth_lock);
	WAIT_FOR_PHY(regs,rval)
	regs->phy_registers=(priv->phy_addr<<5)|(pfyreg&0x1f);
	regs->phy_data=val;
	udelay(25);
	WAIT_FOR_PHY(regs,rval)
///	spin_unlock_irq(&priv->meth_lock);
}

/* Modify phy register using given mask and value */
static void mdio_update(meth_private* priv,int phyreg, int val, int mask)
{
	int rval;
	DPRINTK("RMW value %i to PHY register %i with mask %i\n",val,phyreg,mask);
	rval=mdio_read(priv,phyreg);
	rval=(rval&~mask)|(val&mask);
	mdio_write(priv,phyreg,rval);
}

/* handle errata data on MDIO bus */
//static void mdio_errata(meth_private *priv)
//{
	/* Hmmm... what the hell is phyerrata? does it come from sys init parameters in IRIX */
//}
static int mdio_probe(meth_private *priv)
{
	int i, p2, p3;
	DPRINTK("Detecting PHY kind\n");
	/* check if phy is detected already */
	if(priv->phy_addr>=0&&priv->phy_addr<32)
		return 0;
	spin_lock_irq(priv->meth_lock);
	for (i=0;i<32;++i){
		priv->phy_addr=(char)i;
		p2=mdio_read(priv,2);
#ifdef MFE_DEBUG
		p3=mdio_read(priv,3);
		switch ((p2<<12)|(p3>>4)){
			case PHY_QS6612X:
				DPRINTK("PHY is QS6612X\n");
				break;
			case PHY_ICS1889:
				DPRINTK("PHY is ICS1889\n");
				break;
			case PHY_ICS1890:
				DPRINTK("PHY is ICS1890\n");
				break;
			case PHY_DP83840:
				DPRINTK("PHY is DP83840\n");
				break;
		}
#endif
		if(p2!=0xffff&&p2!=0x0000){
			DPRINTK("PHY code: %x\n",(p2<<12)|(p3>>4));
			break;
		}
	}
	spin_unlock_irq(priv->meth_lock);
	if(priv->phy_addr<32) {
		return 0;
	}
	DPRINTK("Oopsie! PHY is not known!\n");
	priv->phy_addr=-1;
	return -ENODEV;
}

static int meth_init_tx_ring(meth_private *priv)
{
	/* Init TX ring */
	DPRINTK("Initializing TX ring\n");
	priv->tx_ring=(tx_packet*)pci_alloc_consistent(NULL,TX_RING_BUFFER_SIZE,&(priv->tx_ring_dma));
	if(!priv->tx_ring)
		return -ENOMEM;
	memset(priv->tx_ring,0,TX_RING_BUFFER_SIZE);
	priv->regs->tx_ring_base=priv->tx_ring_dma;
	priv->free_space=TX_RING_ENTRIES;
	/* Now init ksb save area */
	memset(priv->tx_skbs,0,sizeof(priv->tx_skbs));
	memset(priv->tx_skb_dmas,0,sizeof(priv->tx_skb_dmas));
	DPRINTK("Done with TX ring init\n");
	return 0;
}

static int meth_init_rx_ring(meth_private *priv)
{
	int i;
	DPRINTK("Initializing RX ring\n");
	for(i=0;i<RX_RING_ENTRIES;i++){
		DPRINTK("\t1:\t%i\n",i);
		/*if(!(priv->rx_ring[i]=get_free_page(GFP_KERNEL)))
			return -ENOMEM;
		DPRINTK("\t2:\t%i\n",i);*/
		priv->rx_ring[i]=(rx_packet*)pci_alloc_consistent(NULL,METH_RX_BUFF_SIZE,&(priv->rx_ring_dmas[i]));
		/* I'll need to re-sync it after each RX */
		DPRINTK("\t2:\t%i\n",i);
		priv->regs->rx_fifo=priv->rx_ring_dmas[i];
	}
	DPRINTK("Done with RX ring\n");
	return 0;
}
static void meth_free_tx_ring(meth_private *priv)
{
	pci_free_consistent(NULL,TX_RING_BUFFER_SIZE,priv->tx_ring,priv->tx_ring_dma);
}
static void meth_free_rx_ring(meth_private *priv)
{
	int i;
	for(i=0;i<RX_RING_ENTRIES;i++)
		pci_free_consistent(NULL,TX_RING_BUFFER_SIZE,priv->rx_ring[i],priv->rx_ring_dmas[i]);
}

/*============End Helper Routines=====================*/

/*
 * Open and close
 */

int meth_open(struct net_device *dev)
{
	meth_private *priv=dev->priv;
	volatile meth_regs *regs=priv->regs;
    MOD_INC_USE_COUNT;
	/* Start DMA */
	regs->dma_ctrl|=
		METH_DMA_TX_EN|/*METH_DMA_TX_INT_EN|*/
		METH_DMA_RX_EN|METH_DMA_RX_INT_EN;

	if(request_irq(dev->irq,meth_interrupt,SA_SHIRQ,meth_str,dev)){
		printk(KERN_ERR "%s: Can't get irq %d\n", dev->name, dev->irq);
		return -EAGAIN;
	}
    netif_start_queue(dev);
	DPRINTK("Opened... DMA control=0x%08lx\n", regs->dma_ctrl);
    return 0;
}

int meth_release(struct net_device *dev)
{
    netif_stop_queue(dev); /* can't transmit any more */
	/* shut down dma */
	((meth_private*)(dev->priv))->regs->dma_ctrl&=
		~(METH_DMA_TX_EN|METH_DMA_TX_INT_EN|
		METH_DMA_RX_EN|METH_DMA_RX_INT_EN);
	free_irq(dev->irq, dev);
    MOD_DEC_USE_COUNT;
    return 0;
}

/*
 * Configuration changes (passed on by ifconfig)
 */
int meth_config(struct net_device *dev, struct ifmap *map)
{
    if (dev->flags & IFF_UP) /* can't act on a running interface */
        return -EBUSY;

    /* Don't allow changing the I/O address */
    if (map->base_addr != dev->base_addr) {
        printk(KERN_WARNING "meth: Can't change I/O address\n");
        return -EOPNOTSUPP;
    }

    /* Allow changing the IRQ */
    if (map->irq != dev->irq) {
        printk(KERN_WARNING "meth: Can't change IRQ\n");
        return -EOPNOTSUPP;
    }
	DPRINTK("Configured\n");

    /* ignore other fields */
    return 0;
}

/*
 * Receive a packet: retrieve, encapsulate and pass over to upper levels
 */
void meth_rx(struct net_device* dev)
{
    struct sk_buff *skb;
    struct meth_private *priv = (struct meth_private *) dev->priv;
	rx_packet *rxb;
	DPRINTK("RX...\n");
	while((rxb=priv->rx_ring[priv->rx_write])->status.raw&0x8000000000000000){
		int len=rxb->status.parsed.rx_len;
		DPRINTK("\tpriv->tx_write=%i\n",priv->rx_write);
		if(!(rxb->status.raw&METH_RX_STATUS_ERRORS)){
			DPRINTK("Packet had no errors...\n");
			skb=alloc_skb(len+2,GFP_ATOMIC);/* Should be atomic -- we are in interrupt */
			if(!skb){
				/* Ouch! No memory! Drop packet on the floor */
				DPRINTK("!!!>>>Ouch! Not enough Memory for RX buffer!\n");
				priv->stats.tx_dropped++;
			} else {
				DPRINTK("And there was no problem getting new skb...\n");
				skb_reserve(skb, 2); /* align IP on 16B boundary */  
    			memcpy(skb_put(skb, len), rxb->buf, len);
			    /* Write metadata, and then pass to the receive level */
			    skb->dev = dev;
			    skb->protocol = eth_type_trans(skb, dev);
				//skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
			    priv->stats.rx_packets++;
				DPRINTK("We are about to pass packet up\n");
    			netif_rx(skb);
				DPRINTK("There we go... Whew...\n");
			}
		}
		rxb->status.raw=0;
		priv->regs->rx_fifo=priv->rx_ring_dmas[priv->rx_write];
		priv->rx_write=(priv->rx_write+1)&(RX_RING_ENTRIES-1);
	}
}

void meth_tx_cleanup(struct net_device* dev, int rptr)
{
	meth_private *priv=dev->priv;
	//volatile meth_regs* regs=priv->regs;
	tx_packet* status;
	struct sk_buff* skb;
	DPRINTK("TX...\n");
	spin_lock(&priv->meth_lock);
	priv->regs->dma_ctrl&=~(METH_DMA_TX_INT_EN);
	while(priv->tx_read!=rptr){
		skb=priv->tx_skbs[priv->tx_read];
		status=&priv->tx_ring[priv->tx_read];
		if(!status->header.res.sent)
			break;
		if(status->header.raw&METH_TX_STATUS_DONE){
			priv->stats.tx_packets++;
			priv->stats.tx_bytes+=skb->len;
		}
        dev_kfree_skb(skb);
		status->header.raw=0;
		priv->tx_read=(priv->tx_read+1)&(TX_RING_ENTRIES-1);
	}
	spin_unlock(priv->meth_lock);
	netif_wake_queue(dev);
}

/*
 * The typical interrupt entry point
 */
void meth_interrupt(int irq, void *dev_id, struct pt_regs *pregs)
{
    struct meth_private *priv;
	//volatile meth_regs* regs;
	union {
		u32	reg; /*Whole status register */
		struct {
			u32			:	2,
				rx_seq	:	5,
				tx_read	:	9,
				
				rx_read	:	8,
				int_mask:	8;
		} parsed;
	} status;
    /*
     * As usual, check the "device" pointer for shared handlers.
     * Then assign "struct device *dev"
     */
    struct net_device *dev = (struct net_device *)dev_id;
    /* ... and check with hw if it's really ours */

    if (!dev /*paranoid*/ ) return;

    /* Lock the device */
    priv = (struct meth_private *) dev->priv;
    spin_lock(&priv->meth_lock);

    status.reg = priv->regs->int_flags;
	priv->regs->int_flags=status.reg&0xff; /* clear interrupts */
	DPRINTK("Interrupt, status %08x...\n",status.reg);
    if (status.parsed.int_mask & METH_INT_RX_THRESHOLD) {
        /* send it to meth_rx for handling */
        meth_rx(dev);
    }
    if (status.parsed.int_mask & (METH_INT_TX_EMPTY|METH_INT_TX_PKT)) {
        /* a transmission is over: free the skb */
		meth_tx_cleanup(dev,status.parsed.tx_read);
    }
	/* check for errors too... */

	DPRINTK("Interrupt handling done...\n");
    /* Unlock the device and we are done */
    spin_unlock(&priv->meth_lock);
}

/*
 * Transmits packets that fit into TX describtor (are <=120B)
 */
static void meth_tx_short_prepare(meth_private* priv, struct sk_buff* skb)
{
	tx_packet *desc=&priv->tx_ring[priv->tx_write];
	int len=(skb->len<ETH_ZLEN)?ETH_ZLEN:skb->len;
	DPRINTK("Short packet... \n");
	/* amybe I should set whole thing to 0 first... */
	memcpy(desc->data.dt+(120-len),skb->data,skb->len);
	if(skb->len<len)
		memset(desc->data.dt+120-len+skb->len,0,len-skb->len);
	desc->header.raw=METH_TX_CMD_INT_EN|(len-1)|((128-len)<<16);
	DPRINTK("desc=%016lx\n",desc->header.raw);
}
#define TX_CATBUF1 BIT(25)
static void meth_tx_1page_prepare(meth_private* priv, struct sk_buff* skb)
{
	tx_packet *desc=&priv->tx_ring[priv->tx_write];
	void *data=skb->data;
	int len=(u64)data&0x7;
	dma_addr_t catbuf;
	desc->header.raw=METH_TX_CMD_INT_EN|TX_CATBUF1|(skb->len-1);
	if(len){
		memcpy(desc->data.dt+(120-len),data,len);
		desc->header.raw|=(128-len)<<16;
	}
	data+=len;
	/* I'll just pretend I know for sure that pci_unmap_single is NOP */
	catbuf=pci_map_single(NULL,data,skb->len-len,PCI_DMA_TODEVICE);
	desc->data.cat_buf[0].raw=catbuf|((skb->len-len-1)<<31);
}
#define TX_CATBUF2 BIT(26)
static void meth_tx_2page_prepare(meth_private* priv, struct sk_buff* skb)
{
	tx_packet *desc=&priv->tx_ring[priv->tx_write];
	void *data=skb->data;
	int len=(u64)data&0x7;
	dma_addr_t catbuf1,catbuf2;
	desc->header.raw=METH_TX_CMD_INT_EN|TX_CATBUF1|TX_CATBUF2|(skb->len-1);
	if(len){
		memcpy(desc->data.dt+(120-len),data,len);
		desc->header.raw|=(128-len)<<16;
	}
	data+=len;
	/* I'll just pretend I know for sure that pci_unmap_single is NOP */
	catbuf1=pci_map_single(NULL,data,skb->len-len,PCI_DMA_TODEVICE);
	desc->data.cat_buf[0].raw=catbuf1|((skb->len-len-1)<<31);
	len=(u64)data&0xFFF;
	data+=len;
	catbuf2=pci_map_single(NULL,data,skb->len-((u64)data-(u64)skb->data),PCI_DMA_TODEVICE);
	desc->data.cat_buf[1].raw=catbuf2|((skb->len-((u64)data-(u64)skb->data))<<31);
}
/*
 * Transmit a packet (called by the kernel)
 */
int meth_tx(struct sk_buff *skb, struct net_device *dev)
{
    struct meth_private *priv = (struct meth_private *) dev->priv;

    spin_lock(&priv->meth_lock);
	DPRINTK("Transmitting data...\n");
	if(skb->len<=120) /* Whole thing fits into descriptor */
		meth_tx_short_prepare(priv,skb);
	else if(((u64)(skb->data)&0xFFF)<skb->len) /* Packet crosses page boundary */
		meth_tx_2page_prepare(priv,skb);
	else /* Packet is in one page */
		meth_tx_1page_prepare(priv,skb);
    dev->trans_start = jiffies; /* save the timestamp */

    /* Remember the skb, so we can free it at interrupt time */
    priv->tx_skbs[priv->tx_write] = skb;
	priv->regs->tx_info.wptr=priv->tx_write;
	priv->tx_write=(priv->tx_write+1)&(TX_RING_ENTRIES-1);

	priv->regs->dma_ctrl|=METH_DMA_TX_INT_EN;

	spin_unlock(&priv->meth_lock);

    return 0;
}

/*
 * Deal with a transmit timeout.
 */
 /*
void meth_tx_timeout (struct net_device *dev)
{
    struct meth_private *priv = (struct meth_private *) dev->priv;

    DPRINTK("Transmit timeout at %ld, latency %ld\n", jiffies,
                    jiffies - dev->trans_start);
    priv->status = meth_TX_INTR;
    meth_interrupt(0, dev, NULL);
    priv->stats.tx_errors++;
    netif_wake_queue(dev);
    return;
}
*/
/*
 * Ioctl commands 
 */
int meth_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
 
    DPRINTK("ioctl\n");
    return 0;
}

/*
 * Return statistics to the caller
 */
struct net_device_stats *meth_stats(struct net_device *dev)
{
    struct meth_private *priv = (struct meth_private *) dev->priv;
    return &priv->stats;
}

/*
 * The init function (sometimes called probe).
 * It is invoked by register_netdev()
 */
int meth_init(struct net_device *dev)
{
	meth_private *priv;
	volatile meth_regs* regs;
	int ret;
    /* 
     * Then, assign other fields in dev, using ether_setup() and some
     * hand assignments
     */
    ether_setup(dev); /* assign some of the fields */

    dev->open            = meth_open;
    dev->stop            = meth_release;
    dev->set_config      = meth_config;
    dev->hard_start_xmit = meth_tx;
    dev->do_ioctl        = meth_ioctl;
    dev->get_stats       = meth_stats;
/*#ifdef HAVE_TX_TIMEOUT
    dev->tx_timeout     = meth_tx_timeout;
    dev->watchdog_timeo = timeout;
#endif*/
	dev->irq			= MACE_ETHERNET_IRQ;
    SET_MODULE_OWNER(dev);

    /*
     * Then, allocate the priv field. This encloses the statistics
     * and a few private fields.
     */
    priv = kmalloc(sizeof(struct meth_private), GFP_KERNEL);
    if (priv == NULL)
        return -ENOMEM;
	dev->priv=priv;
    memset(priv, 0, sizeof(struct meth_private));
    spin_lock_init(& (struct meth_private *) (dev->priv)->meth_lock);
    /*
     * Make the usual checks: check_region(), probe irq, ...  -ENODEV
     * should be returned if no device found.  No resource should be
     * grabbed: this is done on open(). 
     */
	priv->regs=(meth_regs*)SGI_MFE;
	dev->base_addr=SGI_MFE;

	regs=(meth_regs*)SGI_MFE;

	/* Reset card */
	regs->mac_ctrl=SGI_MAC_RESET;
	regs->mac_ctrl=0;
	udelay(25);
	DPRINTK("MAC control after reset: %016lx\n",regs->mac_ctrl);
	printk("SGI O2 Fast Ethernet rev. %ld\n",regs->mac_ctrl>>29);

	load_eaddr(dev,regs);
	priv->phy_addr=-1; /* No phy is known yet... */
	if(mdio_probe(priv)<0){
		DPRINTK("Unable to find PHY\n");
		return -ENODEV;
	}
	/* Should load some "errata", but later */
	
	/* Initial mode -- 100|Half-duplex|Acept normal packets */
	priv->mode=METH_ACCEPT_MCAST|METH_100MBIT|METH_DEFAULT_IPG;
	if(dev->flags|IFF_PROMISC)
		priv->mode|=METH_PROMISC;
	regs->mac_ctrl=priv->mode;

	if((ret=meth_init_tx_ring(priv))<0||(ret=meth_init_rx_ring(priv))<0){
		meth_free_tx_ring(priv);
		meth_free_rx_ring(priv);
		return ret;
	}
	/* Now set dma control, but don't enable DMA, yet */
	priv->regs->dma_ctrl=(4<<METH_RX_OFFSET_SHIFT)|(RX_RING_ENTRIES<<METH_RX_DEPTH_SHIFT);
    return 0;
}

/*
 * The devices
 */

struct net_device meth_devs[1] = {
    { init: meth_init, }  /* init, nothing more */
};

/*
 * Finally, the module stuff
 */

int meth_init_module(void)
{

    int result, device_present = 0;

	strcpy(meth_devs[0].name, "eth%d");

	if ( (result = register_netdev(meth_devs)) )
        printk("meth: error %i registering device \"%s\"\n",
                   result, meth_devs->name);
	else device_present++;
#ifndef METH_DEBUG
    EXPORT_NO_SYMBOLS;
#endif

    return device_present ? 0 : -ENODEV;
}

void meth_cleanup(void)
{
    kfree(meth_devs->priv);
    unregister_netdev(meth_devs);
    return;
}

module_init(meth_init_module);
module_exit(meth_cleanup);
