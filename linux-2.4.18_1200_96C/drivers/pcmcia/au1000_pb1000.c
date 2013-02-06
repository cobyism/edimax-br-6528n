/*
 *
 * Alchemy Semi PB1000 board specific pcmcia routines.
 *
 * Copyright 2001 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *         	ppopov@mvista.com or source@mvista.com
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 * 
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/config.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/tqueue.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/types.h>

#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/ss.h>
#include <pcmcia/bulkmem.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/bus_ops.h>
#include "cs_internal.h"

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>

#include <asm/au1000.h>
#include <asm/au1000_pcmcia.h>
#include <asm/pb1000.h>


extern struct pcmcia_x_table x_table;

static int pb1000_pcmcia_init(struct pcmcia_init *init)
{
	u32 pcr;
	pcr = PCR_SLOT_0_RST | PCR_SLOT_1_RST;

	writel(0x8000, PB1000_MDR); /* clear pcmcia interrupt */
	au_sync_delay(100);
	writel(0x4000, PB1000_MDR); /* enable pcmcia interrupt */
	au_sync();

	pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,0);
	pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,1);
	writew(pcr, PB1000_PCR);
	au_sync_delay(20);
	  
	/* There's two sockets, but only the first one, 0, is used and tested */
	return 1;
}

static int pb1000_pcmcia_shutdown(void)
{
	u16 pcr;
	pcr = PCR_SLOT_0_RST | PCR_SLOT_1_RST;
	pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,0);
	pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,1);
	writew(pcr, PB1000_PCR);
	au_sync_delay(20);
	return 0;
}

static int 
pb1000_pcmcia_socket_state(unsigned sock, struct pcmcia_state *state)
{
	u16 levels, pcr;
	unsigned char vs;

	levels = readw(PB1000_ACR1);
	pcr = readw(PB1000_PCR);

	state->ready = 0;
	state->vs_Xv = 0;
	state->vs_3v = 0;
	state->detect = 0;

	/* 
	 * This is tricky. The READY pin is also the #IRQ pin.  We'll treat
	 * READY as #IRQ and set state->ready to 1 whenever state->detect 
	 * is true.
	 */

	/* 
	 * CD1/2 are active low; so are the VSS pins; Ready is active high
	 */
	if (sock == 0) {
		if (!(levels & (ACR1_SLOT_0_CD1 | ACR1_SLOT_0_CD2))) {
			state->detect = 1;
			vs =  (levels >> 4) & 0x3;
			switch (vs) {
				case 0:
				case 1:
				case 2:
					state->vs_3v=1;
					break;
				case 3:
				default:
					break;
			}
		}
	}
	else if (sock == 1) {
		if (!(levels & (ACR1_SLOT_1_CD1 | ACR1_SLOT_1_CD2))) {
			state->detect = 1;
			vs =  (levels >> 12) & 0x3;
			switch (vs) {
				case 0:
				case 1:
				case 2:
					state->vs_3v=1;
					break;
				case 3:
				default:
					break;
			}
		}
	}
	else  {
		printk(KERN_ERR "pb1000 socket_state bad sock %d\n", sock);
	}

	if (state->detect)
		state->ready = 1;

	state->bvd1=1;
	state->bvd2=1;
	state->wrprot=0; 
	return 1;
}


static int pb1000_pcmcia_get_irq_info(struct pcmcia_irq_info *info)
{

	if(info->sock > PCMCIA_MAX_SOCK) return -1;

	if(info->sock == 0)
		info->irq = AU1000_GPIO_15;
	else 
		info->irq = -1;

	return 0;
}


static int 
pb1000_pcmcia_configure_socket(const struct pcmcia_configure *configure)
{
	u16 pcr;

	if(configure->sock > PCMCIA_MAX_SOCK) return -1;

	pcr = readw(PB1000_PCR);

	if (configure->sock == 0) {
		pcr &= ~(PCR_SLOT_0_VCC0 | PCR_SLOT_0_VCC1 | 
				PCR_SLOT_0_VPP0 | PCR_SLOT_0_VPP1);
	}
	else  {
		pcr &= ~(PCR_SLOT_1_VCC0 | PCR_SLOT_1_VCC1 | 
				PCR_SLOT_1_VPP0 | PCR_SLOT_1_VPP1);
	}

	pcr &= ~PCR_SLOT_0_RST;
	/*
	writew(pcr, PB1000_PCR);
	au_sync_delay(200);
	*/
	DEBUG(KERN_INFO "Vcc %dV Vpp %dV, pcr %x\n", 
			configure->vcc, configure->vpp, pcr);
	switch(configure->vcc){
		case 0:  /* Vcc 0 */
			switch(configure->vpp) {
				case 0:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_GND,
							configure->sock);
					break;
				case 12:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_12V,
							configure->sock);
					break;
				case 50:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_5V,
							configure->sock);
					break;
				case 33:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_3V,
							configure->sock);
					break;
				default:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,
							configure->sock);
					printk("%s: bad Vcc/Vpp (%d:%d)\n", 
							__FUNCTION__, 
							configure->vcc, 
							configure->vpp);
					break;
			}
			break;
		case 50: /* Vcc 5V */
			switch(configure->vpp) {
				case 0:
					pcr |= SET_VCC_VPP(VCC_5V,VPP_GND,
							configure->sock);
					break;
				case 50:
					pcr |= SET_VCC_VPP(VCC_5V,VPP_5V,
							configure->sock);
					break;
				case 12:
					pcr |= SET_VCC_VPP(VCC_5V,VPP_12V,
							configure->sock);
					break;
				case 33:
					pcr |= SET_VCC_VPP(VCC_5V,VPP_3V,
							configure->sock);
					break;
				default:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,
							configure->sock);
					printk("%s: bad Vcc/Vpp (%d:%d)\n", 
							__FUNCTION__, 
							configure->vcc, 
							configure->vpp);
					break;
			}
			break;
		case 33: /* Vcc 3.3V */
			switch(configure->vpp) {
				case 0:
					pcr |= SET_VCC_VPP(VCC_3V,VPP_GND,
							configure->sock);
					break;
				case 50:
					pcr |= SET_VCC_VPP(VCC_3V,VPP_5V,
							configure->sock);
					break;
				case 12:
					pcr |= SET_VCC_VPP(VCC_3V,VPP_12V,
							configure->sock);
					break;
				case 33:
					pcr |= SET_VCC_VPP(VCC_3V,VPP_3V,
							configure->sock);
					break;
				default:
					pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,
							configure->sock);
					printk("%s: bad Vcc/Vpp (%d:%d)\n", 
							__FUNCTION__, 
							configure->vcc, 
							configure->vpp);
					break;
			}
			break;
		default: /* what's this ? */
			pcr |= SET_VCC_VPP(VCC_HIZ,VPP_HIZ,configure->sock);
			printk(KERN_ERR "%s: bad Vcc %d\n", 
					__FUNCTION__, configure->vcc);
			break;
	}

	pcr &= ~(PCR_SLOT_0_RST);
	if (configure->reset) {
		pcr |= PCR_SLOT_0_RST;
	}
	writew(pcr, PB1000_PCR);
	au_sync_delay(300);
	return 0;
}

struct pcmcia_low_level pb1000_pcmcia_ops = { 
	pb1000_pcmcia_init,
	pb1000_pcmcia_shutdown,
	pb1000_pcmcia_socket_state,
	pb1000_pcmcia_get_irq_info,
	pb1000_pcmcia_configure_socket
};

