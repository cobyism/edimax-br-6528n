/*
 * Copyright (C) 2000, 2001 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _LIB_HSSUBR_H
#define _LIB_HSSUBR_H

/*
 * If __long64 we can do this via macros.  Otherwise, call
 * the magic functions.
 */

#if __long64

typedef long hsaddr_t;

#define hs_write8(a,b) *((volatile uint8_t *) (a)) = (b)
#define hs_write16(a,b) *((volatile uint16_t *) (a)) = (b)
#define hs_write32(a,b) *((volatile uint32_t *) (a)) = (b)
#define hs_write64(a,b) *((volatile uint32_t *) (a)) = (b)
#define hs_read8(a) *((volatile uint8_t *) (a))
#define hs_read16(a) *((volatile uint16_t *) (a))
#define hs_read32(a) *((volatile uint32_t *) (a))
#define hs_read64(a) *((volatile uint64_t *) (a))

#else	/* not __long64 */

typedef long hsaddr_t;

extern void hs_write8(hsaddr_t a,uint8_t b);
extern void hs_write16(hsaddr_t a,uint16_t b);
extern void hs_write32(hsaddr_t a,uint32_t b);
extern void hs_write64(hsaddr_t a,uint64_t b);
extern uint8_t hs_read8(hsaddr_t a);
extern uint16_t hs_read16(hsaddr_t a);
extern uint32_t hs_read32(hsaddr_t a);
extern uint64_t hs_read64(hsaddr_t a);
#endif

#endif
