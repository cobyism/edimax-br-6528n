/******************************************************************************
 *
 * Name:	skgehwt.c
 * Project:	PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.1.1.1 $
 * Date:	$Date: 2007/08/06 10:04:51 $
 * Purpose:	Hardware Timer.
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
 *	$Log: skgehwt.c,v $
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
 *	Revision 1.1.1.1  2003/03/12 06:25:09  john
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
 *	Revision 1.12  1998/10/15 15:11:34  gklug
 *	fix: ID_sccs to SysKonnectFileId
 *	
 *	Revision 1.11  1998/10/08 15:27:51  gklug
 *	chg: correction factor is host clock dependent
 *	
 *	Revision 1.10  1998/09/15 14:18:31  cgoos
 *	Changed more BOOLEANs to SK_xxx
 *
 *	Revision 1.9  1998/09/15 14:16:06  cgoos
 *	Changed line 107: FALSE to SK_FALSE
 *	
 *	Revision 1.8  1998/08/24 13:04:44  gklug
 *	fix: typo
 *	
 *	Revision 1.7  1998/08/19 09:50:49  gklug
 *	fix: remove struct keyword from c-code (see CCC) add typedefs
 *	
 *	Revision 1.6  1998/08/17 09:59:02  gklug
 *	fix: typos
 *	
 *	Revision 1.5  1998/08/14 07:09:10  gklug
 *	fix: chg pAc -> pAC
 *	
 *	Revision 1.4  1998/08/10 14:14:52  gklug
 *	rmv: unneccessary SK_ADDR macro
 *	
 *	Revision 1.3  1998/08/07 12:53:44  gklug
 *	fix: first compiled version
 *	
 *	Revision 1.2  1998/08/07 09:19:29  gklug
 *	adapt functions to the C coding conventions
 *	rmv unneccessary functions.
 *	
 *	Revision 1.1  1998/08/05 11:28:36  gklug
 *	first version: adapted from SMT/FDDI
 *	
 *	
 *	
 *
 ******************************************************************************/


/*
	Event queue and dispatcher
*/
static const char SysKonnectFileId[] =
	"$Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/sk98lin/skgehwt.c,v 1.1.1.1 2007/08/06 10:04:51 root Exp $" ;

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

#ifdef __C2MAN__
/*
	Hardware Timer function queue management.

	General Description:

 */
intro()
{}
#endif

/*
 * Prototypes of local functions.
 */
#define	SK_HWT_MAX	(65000)

/* correction factor */
#define	SK_HWT_FAC	(1000 * (SK_U32)pAC->GIni.GIHstClkFact / 100)

/*
 * Initialize hardware timer.
 *
 * Must be called during init level 1.
 */
void	SkHwtInit(
SK_AC	*pAC,	/* Adapters context */
SK_IOC	Ioc)	/* IoContext */
{
	pAC->Hwt.TStart = 0 ;
	pAC->Hwt.TStop	= 0 ;
	pAC->Hwt.TActive = SK_FALSE ;

	SkHwtStop(pAC,Ioc) ;
}

/*
 *
 * Start hardware timer (clock ticks are 16us).
 *
 */
void	SkHwtStart(
SK_AC	*pAC,	/* Adapters context */
SK_IOC	Ioc,	/* IoContext */
SK_U32	Time)	/* Time in units of 16us to load the timer with. */
{
	SK_U32	Cnt ;

	if (Time > SK_HWT_MAX)
		Time = SK_HWT_MAX ;

	pAC->Hwt.TStart = Time ;
	pAC->Hwt.TStop = 0L ;

	Cnt = Time ;

	/*
	 * if time < 16 us
	 *	time = 16 us
	 */
	if (!Cnt) {
		Cnt++ ;
	}

	SK_OUT32(Ioc, B2_TI_INI, Cnt * SK_HWT_FAC) ;
	SK_OUT16(Ioc, B2_TI_CRTL, TIM_START) ;	/* Start timer. */

	pAC->Hwt.TActive = SK_TRUE ;
}

/*
 * Stop hardware timer.
 * and clear the timer IRQ
 */
void	SkHwtStop(
SK_AC	*pAC,	/* Adapters context */
SK_IOC	Ioc)	/* IoContext */
{
	SK_OUT16(Ioc, B2_TI_CRTL, TIM_STOP) ;
	SK_OUT16(Ioc, B2_TI_CRTL, TIM_CLR_IRQ) ;

	pAC->Hwt.TActive = SK_FALSE ;
}


/*
 *	Stop hardware timer and read time elapsed since last start.
 *
 * returns
 *	The elapsed time since last start in units of 16us.
 *
 */
SK_U32	SkHwtRead(
SK_AC	*pAC,	/* Adapters context */
SK_IOC	Ioc)	/* IoContext */
{
	SK_U32	TRead ;
	SK_U32	IStatus ;

	if (pAC->Hwt.TActive) {
		SkHwtStop(pAC,Ioc) ;

		SK_IN32(Ioc, B2_TI_VAL, &TRead);
		TRead /= SK_HWT_FAC;

		SK_IN32(Ioc, B0_ISRC, &IStatus);

		/* Check if timer expired (or wraparound). */
		if ((TRead > pAC->Hwt.TStart) || (IStatus & IS_TIMINT)) {
			SkHwtStop(pAC,Ioc) ;
			pAC->Hwt.TStop = pAC->Hwt.TStart ;
		} else {
			pAC->Hwt.TStop = pAC->Hwt.TStart - TRead ;
		}
	}
	return (pAC->Hwt.TStop) ;
}

/*
 * interrupt source= timer
 */
void	SkHwtIsr(
SK_AC	*pAC,	/* Adapters context */
SK_IOC	Ioc)	/* IoContext */
{
	SkHwtStop(pAC,Ioc);
	pAC->Hwt.TStop = pAC->Hwt.TStart;
	SkTimerDone(pAC,Ioc) ;
}

/* End of file */
