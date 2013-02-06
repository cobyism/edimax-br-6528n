/* $Id: tpanel.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * Touch Panel Data Structures
 * by Michael Klar, wyldfier@iname.com
 *
 * Created for the touch panel interface built into the NEC VR41xx family of
 * CPUs, but should be applicable to other touch panel hardware
 */

#ifndef _LINUX_TPANEL_H
#define _LINUX_TPANEL_H

#include <linux/ioctl.h>

/*
 * The scan interval and settling time can be set via IOCTL, but the defualt
 * of .01 sec and 480us should suffice for most applications.  The argument to
 * these IOCTL calls is a struct of the values in microseconds, but note that
 * the hardware may have a less precise interval.  If so, the value will be
 * rounded and the actual value returned back.
 */

struct scanparam {
	unsigned int	interval;
	unsigned int	settletime;
};

#define TPGETSCANPARM _IOR( 0xB0, 0x00, struct scanparam )
#define TPSETSCANPARM _IOW( 0xB0, 0x01, struct scanparam )

#endif /* _LINUX_TPANEL_H */
