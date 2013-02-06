/******************************************************************************
 *
 * Name:	skhwt.h
 * Project:	Genesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.1.1.1 $
 * Date:	$Date: 2007/08/06 10:04:51 $
 * Purpose:	Defines for the hardware timer functions
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1989-1998 SysKonnect,
 *	a business unit of Schneider & Koch & Co. Datensysteme GmbH.
 *	All Rights Reserved
 *
 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SYSKONNECT
 *	The copyright notice above does not evidence any
 *	actual or intended publication of such source code.
 *
 *	This Module contains Proprietary Information of SysKonnect
 *	and should be treated as Confidential.
 *
 *	The information in this file is provided for the exclusive use of
 *	the licensees of SysKonnect.
 *	Such users have the right to use, modify, and incorporate this code
 *	into products for purposes authorized by the license agreement
 *	provided they include this notice and the associated copyright notice
 *	with any such product.
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * History:
 *
 *	$Log: skgehwt.h,v $
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
 *	Revision 1.4  1998/08/19 09:50:58  gklug
 *	fix: remove struct keyword from c-code (see CCC) add typedefs
 *	
 *	Revision 1.3  1998/08/14 07:09:29  gklug
 *	fix: chg pAc -> pAC
 *	
 *	Revision 1.2  1998/08/07 12:54:21  gklug
 *	fix: first compiled version
 *	
 *	Revision 1.1  1998/08/07 09:32:58  gklug
 *	first version
 *	
 *	
 *	
 *	
 *
 ******************************************************************************/

/*
 * SKGEHWT.H	contains all defines and types for the timer functions
 */

#ifndef	_SKGEHWT_H_
#define _SKGEHWT_H_

/*
 * SK Hardware Timer
 * - needed wherever the HWT module is used
 * - use in Adapters context name pAC->Hwt
 */
typedef	struct s_Hwt {
	SK_U32		TStart ;	/* HWT start */
	SK_U32		TStop ;		/* HWT stop */
	int		TActive ;	/* HWT: flag : active/inactive */
} SK_HWT;

extern void SkHwtInit(SK_AC *pAC, SK_IOC Ioc);
extern void SkHwtStart(SK_AC *pAC, SK_IOC Ioc, SK_U32 Time);
extern void SkHwtStop(SK_AC *pAC, SK_IOC Ioc);
extern SK_U32 SkHwtRead(SK_AC *pAC,SK_IOC Ioc);
extern void SkHwtIsr(SK_AC *pAC, SK_IOC Ioc);
#endif	/* _SKGEHWT_H_ */
