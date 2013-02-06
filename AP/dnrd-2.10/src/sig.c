/*
 * sig.c - signal handling.
 *
 * Copyright (C) 1998 Brad M. Garcia <garsh@home.com>
 *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "sig.h"
#include "common.h"

/*
 * sig_handler()
 *
 * In:       signo - the type of signal that has been recieved.
 *
 * Abstract: If we receive SIGUSR1, we toggle debugging mode.
 *           Otherwise, we assume that we should die.
 */
#if defined(_WanAutoDetect_) || defined(_IQSETUP_)
extern int always_fake;
#endif
void sig_handler(int signo)
{
    switch(signo) {
      case SIGUSR1:
	  opt_debug = opt_debug ? 0 : 1;
	  break;
#if defined(_WanAutoDetect_) || defined(_IQSETUP_)
      case SIGUSR2:
    	  always_fake=1;
    	  break;
#endif
      default:
	  cleanexit(0);
    }
}
