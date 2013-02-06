/******************************************************************************
 *
 * Name:	version.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.1.1.1 $
 * Date:	$Date: 2007/08/06 10:04:51 $
 * Purpose:	SK specific Error log support
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
 *	$Log: skversion.h,v $
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
 *	Revision 1.1  2001/03/06 09:25:00  mlindner
 *	first version
 *	
 *	
 *
 ******************************************************************************/
 
 
static const char SysKonnectFileId[] = "@(#)" __FILE__ " (C) SysKonnect GmbH.";
static const char SysKonnectBuildNumber[] =
	"@(#)SK-BUILD: 4.06 PL: 01"; 

#define BOOT_STRING	"sk98lin: Network Device Driver v4.06\n" \
			"Copyright (C) 2000-2001 SysKonnect GmbH."

#define VER_STRING	"4.06"


