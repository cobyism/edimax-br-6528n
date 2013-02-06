/******************************************************************************
 *
 * Name:	skgedrv.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.1.1.1 $
 * Date:	$Date: 2007/08/06 10:04:51 $
 * Purpose:	Interface with the driver
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998,1999 SysKonnect,
 *	a business unit of Schneider & Koch & Co. Datensysteme GmbH.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * History:
 *
 *	$Log: skgedrv.h,v $
 *	Revision 1.1.1.1  2007/08/06 10:04:51  root
 *	Initial import source to CVS
 *	
 *	Revision 1.1.1.1  2004/07/28 06:27:27  ysc
 *	
 *	Initial version
 *	
 *	Revision 1.1.1.1  2004/07/14 13:34:11  ysy
 *	
 *	
 *	Revision 1.1.1.1  2004/07/13 21:58:20  ysy
 *	
 *	
 *	Revision 1.1.1.1  2004/07/13 15:57:35  ysy
 *	no message
 *	
 *	Revision 1.1.1.2  2003/04/04 01:16:19  david
 *	rtl8181 project
 *	
 *	Revision 1.1.1.1  2003/03/13 07:57:15  david
 *	
 *	:
 *	VS: ----------------------------------------------------------------------
 *	
 *	Revision 1.1.1.1  2003/03/13 07:57:15  john
 *	rtl8181_prj initially build_up
 *	
 *	Revision 1.1.1.1  2003/03/12 06:25:10  john
 *	rtl8181_prj initially build_up
 *	
 *	Revision 1.1.1.1  2003/03/12 06:19:21  john
 *	rtl8181_prj initially build_up
 *	
 *	Revision 1.1.1.1  2003/03/11 06:59:05  john
 *	RTL8181 Initical Check In
 *	
 *	Revision 1.1.1.1  2002/03/04 11:12:52  carstenl
 *	Linux kernel version 2.4.18 from cvs@oss.sgi.com, 4 March, 2002
 *	
 *	Revision 1.4  1999/11/22 13:52:46  cgoos
 *	Changed license header to GPL.
 *	
 *	Revision 1.3  1998/12/01 13:31:39  cgoos
 *	SWITCH INTERN Event added.
 *	
 *	Revision 1.2  1998/11/25 08:28:38  gklug
 *	rmv: PORT SWITCH Event
 *	
 *	Revision 1.1  1998/09/29 06:14:07  gklug
 *	add: driver events (initial version)
 *	
 *
 ******************************************************************************/

#ifndef __INC_SKGEDRV_H_
#define __INC_SKGEDRV_H_

/* defines ********************************************************************/

/*
 * Define the driver events.
 * Usually the events are defined by the destination module. In case of the
 * driver we put the definition of the events here.
 */
#define SK_DRV_PORT_RESET	1	/* The port needs to be reset */
#define SK_DRV_NET_UP   	2	/* The net is now operational */
#define SK_DRV_NET_DOWN		3	/* The net is now down */
#define	SK_DRV_SWITCH_SOFT	4	/* Ports switch with both links conn */
#define	SK_DRV_SWITCH_HARD	5	/* Port switch due to link failure */
#define	SK_DRV_RLMT_SEND	6	/* Send a RLMT packet */
#define	SK_DRV_ADAP_FAIL	7	/* The whole adapter fails */
#define	SK_DRV_PORT_FAIL	8	/* One port fails */
#define	SK_DRV_SWITCH_INTERN	9	/* Port switch from driver to itself */

#endif	/* __INC_SKGEDRV_H_ */
