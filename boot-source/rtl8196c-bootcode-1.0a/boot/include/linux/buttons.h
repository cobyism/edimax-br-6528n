/* $Id: buttons.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * Data defines for button input
 *
 * This was created for the NEC VR41xx GPIO button driver, but should be
 * general enough that it applies to other platforms as well.
 *
 * Copyright (c) 2000 Michael Klar <wyldfier@iname.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef _LINUX_BUTTONS_H
#define _LINUX_BUTTONS_H


#define BTN_PRESS	0x8000
#define BTN_RELEASE	0

#define BTN_STATE_MASK	0x8000
#define BTN_DATA_MASK	0x0fff

/*
 * The button definitions:
 *
 * Feel free to add more.  With 4096 potential vlaues, we're not going to
 * run out any time soon.  Note that the application types listed for the
 * BTN_AP buttons are only suggestions for which AP buttons to assign to
 * which physical button based on button icon, and may not be applicable
 * to platforms that are not productivity-oriented.
 */

#define BTN_POWER		0	// Button driver might not queue
					// BTN_POWER releases (e.g. on VR41xx).
#define BTN_ACTION		1
#define BTN_EXIT		2
#define BTN_UP			3
#define BTN_DOWN		4
#define BTN_CONTRAST		5
#define BTN_BACKLIGHT		6
#define BTN_NORTH		7
#define BTN_SOUTH		8
#define BTN_EAST		9
#define BTN_WEST		10
#define BTN_NOTIFICATION	11
#define BTN_SYNC		12
#define BTN_POWER_GPIO		13	// Power switch also hooked to GPIO to
					// detect power button releases.
#define BTN_BATT_DOOR		14
#define BTN_PEN				15

#define BTN_AP1			256	// Task list
#define BTN_AP2			257	// Address book
#define BTN_AP3			258	// Notepad
#define BTN_AP4			259	// Calendar
#define BTN_AP5			260	// Voice recorder
#define BTN_AP6			261
#define BTN_AP7			262
#define BTN_AP8			263
#define BTN_AP9			264
#define BTN_AP10		265
#define BTN_AP11		266
#define BTN_AP12		267
#define BTN_AP13		268
#define BTN_AP14		269
#define BTN_AP15		270
#define BTN_AP16		271

#define BTN_AP17		272
#define BTN_AP18		273
#define BTN_AP19		274
#define BTN_AP20		275
#define BTN_AP21		276
#define BTN_AP22		277
#define BTN_AP23		278
#define BTN_AP24		279
#define BTN_AP25		280
#define BTN_AP26		281
#define BTN_AP27		282
#define BTN_AP28		283
#define BTN_AP29		284
#define BTN_AP30		285
#define BTN_AP31		286
#define BTN_AP32		287

#define BTN_AP33		288
#define BTN_AP34		289
#define BTN_AP35		290
#define BTN_AP36		291
#define BTN_AP37		292

#endif /* _LINUX_BUTTONS_H */
