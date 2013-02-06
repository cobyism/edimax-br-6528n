/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*
 * isp1161 USB HCD for Linux Version 0.8 (9/3/2001)
 *
 * requires (includes) hc_simple.[hc] simple generic HCD frontend
 *
 * Roman Weissgaerber weissg@vienna.at (C) 2001
 *
 *-------------------------------------------------------------------------*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *-------------------------------------------------------------------------*/

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/malloc.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/smp_lock.h>
#include <linux/list.h>
#include <linux/ioport.h>
#include <asm/io.h>

#include <linux/usb.h>

#include "hc_isp116x.h"
#include "hc_simple.h"

static int hc_verbose = 1;
static int hc_error_verbose = 0;
static int urb_debug = 0;

#include "hc_simple.c"
#include "hc_isp116x_rh.c"

#ifdef CONFIG_YAZU
static int irq = 27;
static int hcport = 0xd0000000;
static int wuport = 0xd0000240;
#else
static int irq = 10;
static int hcport = 0x290;
static int wuport = 0x240;
#endif

MODULE_PARM(hc_verbose,"i");
MODULE_PARM_DESC(hc_verbose,"verbose startup messages, default is 1 (yes)");
MODULE_PARM(hc_error_verbose,"i");
MODULE_PARM_DESC(hc_error_verbose,"verbose error/warning messages, default is 0 (no)");
MODULE_PARM(urb_debug,"i");
MODULE_PARM_DESC(urb_debug,"debug urb messages, default is 0 (no)");

MODULE_PARM(irq,"i");
MODULE_PARM_DESC(irq,"IRQ 3, 10 (default)");
MODULE_PARM(hcport,"i");
MODULE_PARM_DESC(hcport,"ISP116x PORT 0x290");
MODULE_PARM(wuport,"i");
MODULE_PARM_DESC(wuport,"WAKEUP PORT 0x240");

static inline int READ_REG32 (hci_t * hci, int regindex)
{
	hcipriv_t * hp = &hci->hp;
	int val16, val;

	outw (regindex, hp->hcport + 2);
	val16 = inw (hp->hcport);
	val = val16;
	val16 = inw (hp->hcport);
	val += val16 << 16;

	return val;
}

static inline int READ_REG16 (hci_t * hci, int regindex)
{
	hcipriv_t * hp = &hci->hp;
	int val = 0;

	outw (regindex, hp->hcport + 2);
	val = inw (hp->hcport);

	return val;
}

static inline void READ_REGn16 (hci_t * hci, int regindex, int length, __u8 * buffer)
{
	hcipriv_t * hp = &hci->hp;
	int i;
	int val = 0;

	outw (regindex, hp->hcport + 2);

	for (i = 0; i < length; i += 2) {
		val = inw (hp->hcport);
		buffer [i] = val;
		buffer [i+1] = val >> 8;
	}
	if (length & 1) {
		val = inw (hp->hcport);
		buffer [length - 1] = val;
	}
}

static inline void WRITE_REG32 (hci_t * hci, unsigned int value, int regindex)
{
	hcipriv_t * hp = &hci->hp;

	outw (regindex | 0x80, hp->hcport + 2);
	outw (value, hp->hcport);
	outw (value >> 16, hp->hcport);
}

static inline void WRITE_REG16 (hci_t * hci, unsigned int value, int regindex)
{
	hcipriv_t * hp = &hci->hp;

	outw (regindex | 0x80, hp->hcport + 2);
	outw (value, hp->hcport);
}

static inline void WRITE_REG0 (hci_t * hci, int regindex)
{
	hcipriv_t * hp = &hci->hp;

	outw (regindex | 0x80, hp->hcport + 2);
}

static inline void WRITE_REGn16 (hci_t * hci, int regindex, int length, __u8 * buffer)
{
	hcipriv_t * hp = &hci->hp;
	int i;

	outw (regindex | 0x80, hp->hcport + 2);

	for (i = 0; i < length; i+=2) {
		outw (buffer [i] + (buffer [i+1] << 8), hp->hcport);
	}
	if (length & 1) {
		outw (buffer [length - 1], hp->hcport);
	}
}

/*-------------------------------------------------------------------------*/
/* tl functions */

static inline void hc_mark_last_trans (hci_t * hci)
{
	hcipriv_t * hp = &hci->hp;
	__u8 * ptd = hp->tl;

	if (hp->tlp > 0)
		*(ptd + hp->tl_last) |= (1 << 3);
}

static inline int hc_add_trans (hci_t * hci, int len, void * data,
		int toggle, int maxps, int slow, int endpoint, int address, int pid, int format)
{
	hcipriv_t * hp = &hci->hp;
	int last = 0;
	__u8 * ptd = hp->tl;

	/* just send 4 packets of each kind at a frame,
	 * other URBs also want some bandwitdh, and a NACK is cheaper
	 * with less packets */
	if (len > maxps * 1)
			len = maxps * 1;

	if (hp->units_left < len + 8)
		return 0;
	else
		hp->units_left -= len + 8;


	ptd += hp->tlp;
	hp->tl_last = hp->tlp + 3;
	ptd [0] = 0;
	ptd [1] = (toggle << 2) | (1 << 3) | (0xf << 4);
	ptd [2] = maxps;
	ptd [3] = ((maxps >> 8) & 0x3) | (slow << 2) | (last << 3) | (endpoint << 4);
	ptd [4] = len;
	ptd [5] = ((len >> 8) & 0x3) | (pid << 2);
	ptd [6] = address | (format << 7);
    ptd [7] = 0;

	memcpy (ptd + 8, data, len);
	hp->tlp += ((len + 8 + 3) & ~0x3);

	return 1;
}

static inline int hc_parse_trans (hci_t * hci, int * actbytes, void ** data,
			int * cc, int * toggle)
{
	hcipriv_t * hp = &hci->hp;
	int last = 0;
	int totbytes;
	__u8 *ptd = hp->tl + hp->tlp;


	*cc = (ptd [1] >> 4) & 0xf;
	last = (ptd [3] >> 3) & 0x1;
	*actbytes = ((ptd [1] & 0x3) << 8) | ptd [0];
	totbytes = ((ptd [5] & 0x3) << 8) | ptd [4];
	*data = ptd + 8;

	*toggle = !(ptd [1] >> 2 & 1);
/*	*active = (ptd[1] >> 3) & 0x1;
	*endpoint = ptd [3] >> 4;
	*address = ptd [6] & 0x7f;
	*pid = (ptd [5] >> 2) & 0x3;
*/
	hp->tlp += ((totbytes + 8 + 3) & ~0x3);

	return !last;
}


/*-------------------------------------------------------------------------*/

/*
static void hc_start_list (hci_t * hci, ed_t * ed)
{
	hcipriv_t * hp = hci->hcipriv;
	int units_left = hp->atl_buffer_len;

// printk("hc_start_list atl_buffer_len: %d atl_len: %d\n", hp->atl_buffer_len, hp->atl_len);
	switch (usb_pipetype (ed->pipe)) {
	case PIPE_CONTROL:
	case PIPE_BULK:
	case PIPE_INTERRUPT:
		if (hp->atl_len == 0) {
			hp->tlp = 0;
			sh_schedule_trans (hci, TR_INTR | TR_CTRL | TR_BULK, &units_left);
			mark_last_trans (hci);
			hp->atl_len = hp->tlp;
			if (hp->atl_len > 0) {
// printk("hc_start_list: send %d bytes\n", hp->atl_len);
				WRITE_REG16 (hci, hp->atl_len, HcTransferCounter);
				WRITE_REGn16 (hci, HcATLBufferPort, hp->atl_len, hp->tl);
			}
		}
	}
}
*/

/* an interrupt happens */
static void hc_interrupt (int irq, void * __hci, struct pt_regs * r)
{
	hci_t * hci = __hci;
	hcipriv_t * hp = &hci->hp;
	int ints_uP, ints = 0, bstat = 0;

	if ((ints_uP = (READ_REG16 (hci, HcuPInterrupt) & READ_REG16 (hci, HcuPInterruptEnable))) == 0) {
// printk("int1 %x %x bstat %x\n", ints_uP, ints, bstat);
		return;
	}
	WRITE_REG16 (hci, ints_uP, HcuPInterrupt);
	if ((ints_uP & OPR_Reg) &&
 		(ints = (READ_REG32 (hci, HcInterruptStatus)))) {

		if (ints & OHCI_INTR_SO) {
			dbg("USB Schedule overrun");
			WRITE_REG32 (hci, OHCI_INTR_SO, HcInterruptEnable);
		}

		if (ints & OHCI_INTR_SF) {
			WRITE_REG32 (hci, OHCI_INTR_SF, HcInterruptEnable);
		}
		WRITE_REG32 (hci, ints, HcInterruptStatus);
		WRITE_REG32 (hci, OHCI_INTR_MIE, HcInterruptEnable);
	}

	bstat = READ_REG16 (hci, HcBufferStatus);
	if (ints_uP & SOFITLInt) {
/*		READ_REGn16 (hci,reg, len, hci->tl);
		hp->tlp = 0;
		done_list (hci);
		schedule_trans (hci, TR_ISO);
		WRITE_REG16 (hci, hp->tlp, HcTransferCounter);
		WRITE_REGn16 (hci, HcITLBufferPort, hp->tlp, hp->tl);
*/	}

	hci->frame_no = GET_FRAME_NUMBER (hci);
//	if (hci->frame_no == 0)
//		printk("*** INTERRUPT frame %u intuP %x ints %x bstat %x \n", hci->frame_no, ints_uP, ints, bstat);

	if (ints_uP & ATLInt) {
// printk("int2 %x %x bstat %x\n", ints_uP, ints, bstat);
		if ((bstat & ATLBufferFull) && (bstat & ATLBufferDone)) {
			if (hp->atl_len > 0) {
				READ_REGn16 (hci, HcATLBufferPort, hp->atl_len, hp->tl);
				hp->tlp = 0;
				sh_done_list (hci);
			}
			hp->tlp = 0;
			hci->trans = 0;
		}
	}
	sh_del_list (hci);
	if (hci->trans == 0 && !(bstat & ATLBufferFull)) {
		hp->units_left = hp->atl_buffer_len;
		hp->tlp = 0;
		sh_schedule_trans (hci);
		hc_mark_last_trans (hci);
		hp->atl_len = hp->tlp;
		if (hp->atl_len > 0) {
			WRITE_REG16 (hci, hp->atl_len, HcTransferCounter);
			WRITE_REGn16 (hci, HcATLBufferPort, hp->atl_len, hp->tl);
		}

	}
}



/*-------------------------------------------------------------------------*
 * HC functions
 *-------------------------------------------------------------------------*/

/* reset the HC and BUS */

static int hc_reset (hci_t * hci)
{
	int timeout = 30;

	/* Disable HC interrupts */
	WRITE_REG32 (hci, OHCI_INTR_MIE, HcInterruptDisable);

	dbg ("USB HC reset_hc usb-: ctrl = 0x%x ;",
		READ_REG32 (hci, HcControl));

  	/* Reset USB (needed by some controllers) */
	WRITE_REG32 (hci, 0, HcControl);

	WRITE_REG0 (hci, HcSoftwareReset);

	/* HC Reset requires max 10 us delay */
	WRITE_REG32 (hci, OHCI_HCR, HcCommandStatus);
	while ((READ_REG32 (hci, HcCommandStatus) & OHCI_HCR) != 0) {
		if (--timeout == 0) {
			err ("USB HC reset timed out!");
			return -1;
		}
		udelay (1);
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* Start an host controller, set the BUS operational
 * enable interrupts
 * connect the virtual root hub */

static int hc_alloc_trans_buffer (hci_t * hci)
{
	hcipriv_t * hp = &hci->hp;
	int maxlen;

	hp->itl0_len = 0;
	hp->itl1_len = 0;
	hp->atl_len = 0;

	hp->itl_buffer_len = 1024;
	hp->atl_buffer_len = 4096 - 2 * hp->itl_buffer_len; /* 2048 */

	WRITE_REG16 (hci, hp->itl_buffer_len, HcITLBufferLength);
	WRITE_REG16 (hci, hp->atl_buffer_len, HcATLBufferLength);
	WRITE_REG16 (hci,
		InterruptPinEnable |
		InterruptPinTrigger |
		InterruptOutputPolarity |
		DataBusWidth16 |
		AnalogOCEnable,
		HcHardwareConfiguration);
	WRITE_REG16 (hci, 0, HcDMAConfiguration);

	maxlen = (hp->itl_buffer_len > hp->atl_buffer_len) ? hp->itl_buffer_len : hp->atl_buffer_len;

	hp->tl = kmalloc (maxlen, GFP_KERNEL);

	if (!hp->tl)
		return -ENOMEM;

	memset (hp->tl, 0, maxlen);
	return 0;
}

static int hc_start (hci_t * hci)
{
	hcipriv_t * hp = &hci->hp;
  	__u32 mask;
  	unsigned int fminterval;

  	fminterval = 0x2edf;
	fminterval |= ((((fminterval - 210) * 6) / 7) << 16);
	WRITE_REG32 (hci, fminterval, HcFmInterval);
	WRITE_REG32 (hci, 0x628, HcLSThreshold);


 	/* start controller operations */
 	hp->hc_control = OHCI_USB_OPER;
 	WRITE_REG32 (hci, hp->hc_control, HcControl);

	/* Choose the interrupts we care about now, others later on demand */
	mask = OHCI_INTR_MIE |
//	OHCI_INTR_ATD |
	OHCI_INTR_SO |
	OHCI_INTR_SF;

	WRITE_REG32 (hci, mask, HcInterruptEnable);
	WRITE_REG32 (hci, mask, HcInterruptStatus);

	mask = SOFITLInt | ATLInt | OPR_Reg;
	WRITE_REG16 (hci, mask, HcuPInterrupt);
	WRITE_REG16 (hci, mask, HcuPInterruptEnable);


#ifdef	OHCI_USE_NPS
	WRITE_REG32 ((READ_REG32 (hci, HcRhDescriptorA) | RH_A_NPS) & ~RH_A_PSM,
		HcRhDescriptorA);
	WRITE_REG32 (hci, RH_HS_LPSC, HcRhStatus);
#endif	/* OHCI_USE_NPS */

	// POTPGT delay is bits 24-31, in 2 ms units.
	mdelay ((READ_REG32 (hci, HcRhDescriptorA) >> 23) & 0x1fe);

	rh_connect_rh (hci);

	return 0;
}


/*-------------------------------------------------------------------------*/

/* allocate HCI */

static hci_t * __devinit hc_alloc_hci (void)
{
	hci_t * hci;
	hcipriv_t * hp;

	struct usb_bus * bus;

	hci = (hci_t *) kmalloc (sizeof (hci_t), GFP_KERNEL);
	if (!hci)
		return NULL;

	memset (hci, 0, sizeof (hci_t));

	hp = &hci->hp;

	hp->irq = -1;
	hp->hcport = -1;

	INIT_LIST_HEAD (&hci->hci_hcd_list);
	list_add (&hci->hci_hcd_list, &hci_hcd_list);
	init_waitqueue_head (&hci->waitq);

	INIT_LIST_HEAD (&hci->ctrl_list);
	INIT_LIST_HEAD (&hci->bulk_list);
	INIT_LIST_HEAD (&hci->iso_list);
	INIT_LIST_HEAD (&hci->intr_list);
	INIT_LIST_HEAD (&hci->del_list);

	bus = usb_alloc_bus (&hci_device_operations);
	if (!bus) {
		kfree (hci);
		return NULL;
	}

	hci->bus = bus;
	bus->hcpriv = (void *) hci;

	return hci;
}


/*-------------------------------------------------------------------------*/

/* De-allocate all resources.. */

static void hc_release_hci (hci_t * hci)
{
	hcipriv_t * hp = &hci->hp;
	dbg ("USB HC release hci %d", hci->regs);



	/* disconnect all devices */
	if (hci->bus->root_hub)
		usb_disconnect (&hci->bus->root_hub);

	if (hp->hcport > 0) {
		WRITE_REG16 (hci, 0, HcHardwareConfiguration);
		WRITE_REG16 (hci, 0, HcDMAConfiguration);
		WRITE_REG16 (hci, 0, HcuPInterruptEnable);
	}

//	if (!hci->disabled)
		hc_reset (hci);
	if (hp->tl)
		kfree (hp->tl);

	if (hp->hcport > 0) {
		release_region (hp->hcport, 4);
		hp->hcport = 0;
	}

	if (hp->wuport > 0) {
		release_region (hp->wuport, 2);
		hp->wuport = 0;
	}

	if (hp->irq >= 0) {
		free_irq (hp->irq, hci);
		hp->irq = -1;
	}

	usb_deregister_bus (hci->bus);
	usb_free_bus (hci->bus);

	list_del (&hci->hci_hcd_list);
	INIT_LIST_HEAD (&hci->hci_hcd_list);

	kfree (hci);
}

/*-------------------------------------------------------------------------*/

/* Increment the module usage count, start the control thread and
 * return success. */


static int __devinit hc_found_hci (int addr, int wuaddr, int irq, int dma)
{
	hci_t * hci;
	hcipriv_t * hp;

	hci = hc_alloc_hci ();
	if (!hci) {
		return -ENOMEM;
	}

	hp = &hci->hp;
#ifndef CONFIG_YAZU
	if (!request_region (addr, 4, "ISP116x USB HOST")) {
		err ("request address %d-%d failed", addr, addr+4);
		hc_release_hci (hci);
		return -EBUSY;
	}
#endif
	hp->hcport = addr;

	if ((READ_REG16(hci, HcChipID) & 0xff00) != 0x6100) {
		hc_release_hci (hci);
		return -ENODEV;
	}
#ifndef CONFIG_YAZU
	if (!request_region (wuaddr, 2, "ISP116x USB HOST")) {
		err ("request address %d-%d failed", wuaddr, wuaddr+2);
		hc_release_hci (hci);
		return -EBUSY;
	}
#endif
	hp->wuport = wuaddr;

	if (hc_reset (hci) < 0) {
		hc_release_hci (hci);
		return -ENODEV;
	}

	if (hc_alloc_trans_buffer (hci)) {
		hc_release_hci (hci);
		return -ENOMEM;
	}

	printk(KERN_INFO __FILE__ ": USB ISP116x at %x/%x IRQ %d Rev. %x ChipID: %x\n",
		addr, wuaddr, irq, READ_REG32(hci, HcRevision), READ_REG16(hci, HcChipID));

	/* FIXME this is a second HC reset; why?? */
	WRITE_REG32 (hci, hp->hc_control = OHCI_USB_RESET, HcControl);
	wait_ms (1000);

	usb_register_bus (hci->bus);

	if (request_irq (irq, hc_interrupt, 0,
			"ISP116x", hci) != 0) {
		err ("request interrupt %d failed", irq);
		hc_release_hci (hci);
		return -EBUSY;
	}
	hp->irq = irq;
#if 0
	if (dma && !request_dma (dma, "ISP116x USB HOST")) {
		err ("request dma %d failed", dma);
		dma = 0;
	}

	hp->dmahci->dmaci->dma = dma;
#endif
	if (hc_start (hci) < 0) {
		err ("can't start usb-%x", addr);
		hc_release_hci (hci);
		return -EBUSY;
	}

#ifdef	DEBUG
//	hci_dump (hci, 1);
#endif
	return 0;
}


/*-------------------------------------------------------------------------*/

static int __init hci_hcd_init (void)
{
	int ret;

	ret = hc_found_hci (hcport, wuport, irq, 0);

	return ret;
}

/*-------------------------------------------------------------------------*/

static void __exit hci_hcd_cleanup (void)
{
	struct list_head *  hci_l;
	hci_t * hci;
	for (hci_l = hci_hcd_list.next; hci_l != &hci_hcd_list;) {
		hci = list_entry (hci_l, hci_t, hci_hcd_list);
		hci_l = hci_l->next;
		hc_release_hci(hci);
	}
}

module_init (hci_hcd_init);
module_exit (hci_hcd_cleanup);


MODULE_AUTHOR ("Roman Weissgaerber <weissg@vienna.at>");
MODULE_DESCRIPTION ("USB ISP116x Host Controller Driver");
