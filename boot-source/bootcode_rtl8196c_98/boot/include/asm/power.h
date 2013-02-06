/*
 * power.h - Common power management defines
 *
 * Copyright (C) 2000 Michael Klar
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */

#ifndef __ASM_POWER_H
#define __ASM_POWER_H

#ifndef _LANGUAGE_ASSEMBLY
extern unsigned int powerevent_queued;
extern unsigned int hibernation_state;
#endif

#define LOAD_MAGIC (('L'<<24) | ('O'<<16) | ('A'<<8) | 'D')
#define HIB_MAGIC ((' '<<24) | ('H'<<16) | ('I'<<8) | 'B')
#define RUN_MAGIC ((' '<<24) | ('R'<<16) | ('U'<<8) | 'N')

#endif /* __ASM_POWER_H */
