/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*
 * ISP116x virtual root hub
 * Roman Weissgaerber weissg@vienna.at
 * based on usb-ohci.c by R. Weissgaerber et al.
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

/*-------------------------------------------------------------------------*
 * Virtual Root Hub
 *-------------------------------------------------------------------------*/

/* Device descriptor */
static __u8 root_hub_dev_des[] =
{
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x10,	    /*  __u16 bcdUSB; v1.1 */
	0x01,
	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x00,       /*  __u8  bDeviceProtocol; */
	0x08,       /*  __u8  bMaxPacketSize0; 8 Bytes */
	0x00,       /*  __u16 idVendor; */
	0x00,
	0x00,       /*  __u16 idProduct; */
 	0x00,
	0x00,       /*  __u16 bcdDevice; */
 	0x00,
	0x00,       /*  __u8  iManufacturer; */
	0x02,       /*  __u8  iProduct; */
	0x01,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};


/* Configuration descriptor */
static __u8 root_hub_config_des[] =
{
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19,       /*  __u16 wTotalLength; */
	0x00,
	0x01,       /*  __u8  bNumInterfaces; */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0x40,       /*  __u8  bmAttributes;
                 Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup, 4..0: resvd */
	0x00,       /*  __u8  MaxPower; */

	/* interface */
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; */
	0x00,       /*  __u8  if_iInterface; */

	/* endpoint */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
 	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
 	0x02,       /*  __u16 ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
 	0x00,
	0xff        /*  __u8  ep_bInterval; 255 ms */
};

/* Hub class-specific descriptor is constructed dynamically */


/*-------------------------------------------------------------------------*/

/* prepare Interrupt pipe data; HUB INTERRUPT ENDPOINT */

static int rh_send_irq (hci_t * hci, void * rh_data, int rh_len)
{
	int num_ports;
	int i;
	int ret;
	int len;

	__u8 data[8];

	num_ports = READ_REG32 (hci, HcRhDescriptorA) & RH_A_NDP;

	*(__u8 *) data = (READ_REG32 (hci, HcRhStatus) & (RH_HS_LPSC | RH_HS_OCIC))
		? 1: 0;
	ret = *(__u8 *) data;

	for ( i = 0; i < num_ports; i++) {
		*(__u8 *) (data + (i + 1) / 8) |=
			((READ_REG32 (hci, HcRhPortStatus + i) &
				(RH_PS_CSC | RH_PS_PESC | RH_PS_PSSC | RH_PS_OCIC | RH_PS_PRSC))
			    ? 1: 0) << ((i + 1) % 8);
		ret += *(__u8 *) (data + (i + 1) / 8);
	}
	len = i/8 + 1;

	if (ret > 0) {
		memcpy (rh_data, data, min (len, min (rh_len, sizeof(data))));
		return len;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* Virtual Root Hub INTs are polled by this timer every "interval" ms */

static void rh_int_timer_do (unsigned long ptr)
{
	int len;

	urb_t * urb = (urb_t *) ptr;
	hci_t * hci = urb->dev->bus->hcpriv;

#if 0
	if (hci->disabled)
//		return;

	/* ignore timers firing during PM suspend, etc */
	if ((hci->hc_control & OHCI_CTRL_HCFS) != OHCI_USB_OPER)
		goto out;
#endif

	if(hci->rh.send) {
		len = rh_send_irq (hci, urb->transfer_buffer, urb->transfer_buffer_length);
		if (len > 0) {
			urb->actual_length = len;
			if (urb_debug == 2)
				urb_print (urb, "RET-t(rh)", usb_pipeout (urb->pipe));

			if (urb->complete)
				urb->complete (urb);
		}
	}
// out:
	rh_init_int_timer (urb);
}

/*-------------------------------------------------------------------------*/

/* Root Hub INTs are polled by this timer */

static int rh_init_int_timer (urb_t * urb)
{
	hci_t * hci = urb->dev->bus->hcpriv;

	hci->rh.interval = urb->interval;
	init_timer (&hci->rh.rh_int_timer);
	hci->rh.rh_int_timer.function = rh_int_timer_do;
	hci->rh.rh_int_timer.data = (unsigned long) urb;
	hci->rh.rh_int_timer.expires =
			jiffies + (HZ * (urb->interval < 30? 30: urb->interval)) / 1000;
	add_timer (&hci->rh.rh_int_timer);

	return 0;
}

/*-------------------------------------------------------------------------*/

#define OK(x) 			len = (x); break
#define WR_RH_STAT(x) 		WRITE_REG32 (hci, (x), HcRhStatus)
#define WR_RH_PORTSTAT(x) 	WRITE_REG32 (hci, (x), HcRhPortStatus + wIndex - 1)
#define RD_RH_STAT		READ_REG32 (hci, HcRhStatus)
#define RD_RH_PORTSTAT		READ_REG32 (hci, HcRhPortStatus + wIndex - 1)

/* request to virtual root hub */

static int rh_submit_urb (urb_t * urb)
{
	struct usb_device * usb_dev = urb->dev;
	hci_t * hci = usb_dev->bus->hcpriv;
	unsigned int pipe = urb->pipe;
	devrequest * cmd = (devrequest *) urb->setup_packet;
	void * data = urb->transfer_buffer;
	int leni = urb->transfer_buffer_length;
	int len = 0;
	int status = TD_CC_NOERROR;

	__u32 datab[4];
	__u8  * data_buf = (__u8 *) datab;

 	__u16 bmRType_bReq;
	__u16 wValue;
	__u16 wIndex;
	__u16 wLength;

	if (usb_pipeint(pipe)) {
		hci->rh.urb =  urb;
		hci->rh.send = 1;
		hci->rh.interval = urb->interval;
		rh_init_int_timer(urb);
		urb->status = cc_to_error [TD_CC_NOERROR];

		return 0;
	}

	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);
	wValue        = le16_to_cpu (cmd->value);
	wIndex        = le16_to_cpu (cmd->index);
	wLength       = le16_to_cpu (cmd->length);

	dbg ("rh_submit_urb, req = %d(%x) len=%d", bmRType_bReq,
		bmRType_bReq, wLength);

	switch (bmRType_bReq) {
	/* Request Destination:
	   without flags: Device,
	   RH_INTERFACE: interface,
	   RH_ENDPOINT: endpoint,
	   RH_CLASS means HUB here,
	   RH_OTHER | RH_CLASS  almost ever means HUB_PORT here
	*/

		case RH_GET_STATUS:
				*(__u16 *) data_buf = cpu_to_le16 (1); OK (2);
		case RH_GET_STATUS | RH_INTERFACE:
				*(__u16 *) data_buf = cpu_to_le16 (0); OK (2);
		case RH_GET_STATUS | RH_ENDPOINT:
				*(__u16 *) data_buf = cpu_to_le16 (0); OK (2);
		case RH_GET_STATUS | RH_CLASS:
				*(__u32 *) data_buf = cpu_to_le32 (
					RD_RH_STAT & ~(RH_HS_CRWE | RH_HS_DRWE));
				OK (4);
		case RH_GET_STATUS | RH_OTHER | RH_CLASS:
				*(__u32 *) data_buf = cpu_to_le32 (RD_RH_PORTSTAT); OK (4);

		case RH_CLEAR_FEATURE | RH_ENDPOINT:
			switch (wValue) {
				case (RH_ENDPOINT_STALL): OK (0);
			}
			break;

		case RH_CLEAR_FEATURE | RH_CLASS:
			switch (wValue) {
				case RH_C_HUB_LOCAL_POWER:
					OK(0);
				case (RH_C_HUB_OVER_CURRENT):
						WR_RH_STAT(RH_HS_OCIC); OK (0);
			}
			break;

		case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
			switch (wValue) {
				case (RH_PORT_ENABLE):
						WR_RH_PORTSTAT (RH_PS_CCS ); OK (0);
				case (RH_PORT_SUSPEND):
						WR_RH_PORTSTAT (RH_PS_POCI); OK (0);
				case (RH_PORT_POWER):
						WR_RH_PORTSTAT (RH_PS_LSDA); OK (0);
				case (RH_C_PORT_CONNECTION):
						WR_RH_PORTSTAT (RH_PS_CSC ); OK (0);
				case (RH_C_PORT_ENABLE):
						WR_RH_PORTSTAT (RH_PS_PESC); OK (0);
				case (RH_C_PORT_SUSPEND):
						WR_RH_PORTSTAT (RH_PS_PSSC); OK (0);
				case (RH_C_PORT_OVER_CURRENT):
						WR_RH_PORTSTAT (RH_PS_OCIC); OK (0);
				case (RH_C_PORT_RESET):
						WR_RH_PORTSTAT (RH_PS_PRSC); OK (0);
			}
			break;

		case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
			switch (wValue) {
				case (RH_PORT_SUSPEND):
						WR_RH_PORTSTAT (RH_PS_PSS ); OK (0);
				case (RH_PORT_RESET): /* BUG IN HUB CODE *********/
						if (RD_RH_PORTSTAT & RH_PS_CCS)
							WR_RH_PORTSTAT (RH_PS_PRS);
							OK (0);
				case (RH_PORT_POWER):
						WR_RH_PORTSTAT (RH_PS_PPS ); OK (0);
				case (RH_PORT_ENABLE): /* BUG IN HUB CODE *********/
						if (RD_RH_PORTSTAT & RH_PS_CCS)
							WR_RH_PORTSTAT (RH_PS_PES );
							OK (0);
			}
			break;

		case RH_SET_ADDRESS: hci->rh.devnum = wValue; OK(0);

		case RH_GET_DESCRIPTOR:
			switch ((wValue & 0xff00) >> 8) {
				case (0x01): /* device descriptor */
					len = min (leni, min (sizeof (root_hub_dev_des), wLength));
					data_buf = root_hub_dev_des; OK(len);
				case (0x02): /* configuration descriptor */
					len = min (leni, min (sizeof (root_hub_config_des), wLength));
					data_buf = root_hub_config_des; OK(len);
				case (0x03): /* string descriptors */
					len = usb_root_hub_string (wValue & 0xff,
						(int)(long) 0, "ISP116x",
						data, wLength);
					if (len > 0) {
						data_buf = data;
						OK (min (leni, len));
					}
					// else fallthrough
				default:
					status = TD_CC_STALL;
			}
			break;

		case RH_GET_DESCRIPTOR | RH_CLASS:
		    {
			    __u32 temp = READ_REG32 (hci, HcRhDescriptorA);

			    data_buf [0] = 9;		// min length;
			    data_buf [1] = 0x29;
			    data_buf [2] = temp & RH_A_NDP;
			    data_buf [3] = 0;
			    if (temp & RH_A_PSM) 	/* per-port power switching? */
				data_buf [3] |= 0x1;
			    if (temp & RH_A_NOCP)	/* no overcurrent reporting? */
				data_buf [3] |= 0x10;
			    else if (temp & RH_A_OCPM)	/* per-port overcurrent reporting? */
				data_buf [3] |= 0x8;

			    datab [1] = 0;
			    data_buf [5] = (temp & RH_A_POTPGT) >> 24;
			    temp = READ_REG32 (hci, HcRhDescriptorB);
			    data_buf [7] = temp & RH_B_DR;
			    if (data_buf [2] < 7) {
				data_buf [8] = 0xff;
			    } else {
				data_buf [0] += 2;
				data_buf [8] = (temp & RH_B_DR) >> 8;
				data_buf [10] = data_buf [9] = 0xff;
			    }

			    len = min (leni, min (data_buf [0], wLength));
			    OK (len);
			}

		case RH_GET_CONFIGURATION: 	*(__u8 *) data_buf = 0x01; OK (1);

		case RH_SET_CONFIGURATION: 	WR_RH_STAT (0x10000); OK (0);

		default:
			dbg ("unsupported root hub command");
			status = TD_CC_STALL;
	}

#ifdef	DEBUG
	// hci_dump_roothub (ohci, 0);
#endif

	len = min(len, leni);
	if (data != data_buf)
	    memcpy (data, data_buf, len);
  	urb->actual_length = len;
	urb->status = cc_to_error [status];

	// hci_return_urb (hci, urb, NULL);
	urb->hcpriv = NULL;
//	usb_dec_dev_use (usb_dev);
	urb->dev = NULL;
	if (urb->complete)
	    	urb->complete (urb);

	return 0;
}

/*-------------------------------------------------------------------------*/

static int rh_unlink_urb (urb_t * urb)
{
	hci_t * hci = urb->dev->bus->hcpriv;

	if (hci->rh.urb == urb) {
		hci->rh.send = 0;
		del_timer (&hci->rh.rh_int_timer);
		hci->rh.urb = NULL;

		urb->hcpriv = NULL;
		usb_dec_dev_use(urb->dev);
		urb->dev = NULL;
		if (urb->transfer_flags & USB_ASYNC_UNLINK) {
			urb->status = -ECONNRESET;
			if (urb->complete)
				urb->complete (urb);
		} else
			urb->status = -ENOENT;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/
/* connect the virtual root hub */

static int rh_connect_rh (hci_t * hci) {

        struct usb_device  * usb_dev;

        hci->rh.devnum = 0;
        usb_dev = usb_alloc_dev (NULL, hci->bus);
        if (!usb_dev)
            return -ENOMEM;

        hci->bus->root_hub = usb_dev;
        usb_connect (usb_dev);
        if (usb_new_device (usb_dev) != 0) {
                usb_free_dev (usb_dev);
                return -ENODEV;
        }

        return 0;
}