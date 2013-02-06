/*  *********************************************************************
    *  SB1250 Board Support Package
    *  
    *  Global constants and macros		File: sb1250_defs.h	
    *  
    *  This file contains macros and definitions used by the other
    *  include files.
    *
    *  Author:  Mitch Lichtenberg (mitch@sibyte.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This program is free software; you can redistribute it and/or 
    *  modify it under the terms of the GNU General Public License as 
    *  published by the Free Software Foundation; either version 2 of 
    *  the License, or (at your option) any later version.
    *
    *  This program is distributed in the hope that it will be useful,
    *  but WITHOUT ANY WARRANTY; without even the implied warranty of
    *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    *  GNU General Public License for more details.
    *
    *  You should have received a copy of the GNU General Public License
    *  along with this program; if not, write to the Free Software
    *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
    *  MA 02111-1307 USA
    ********************************************************************* */


/*  *********************************************************************
    *  Naming schemes for constants in these files:
    *  
    *  M_xxx           MASK constant (identifies bits in a register). 
    *                  For multi-bit fields, all bits in the field will
    *                  be set.
    *
    *  K_xxx           "Code" constant (value for data in a multi-bit
    *                  field).  The value is right justified.
    *
    *  V_xxx           "Value" constant.  This is the same as the 
    *                  corresponding "K_xxx" constant, except it is
    *                  shifted to the correct position in the register.
    *
    *  S_xxx           SHIFT constant.  This is the number of bits that
    *                  a field value (code) needs to be shifted 
    *                  (towards the left) to put the value in the right
    *                  position for the register.
    *
    *  A_xxx           ADDRESS constant.  This will be a physical 
    *                  address.  Use the PHYS_TO_K1 macro to generate
    *                  a K1SEG address.
    *
    *  R_xxx           RELATIVE offset constant.  This is an offset from
    *                  an A_xxx constant (usually the first register in
    *                  a group).
    *  
    *  G_xxx(X)        GET value.  This macro obtains a multi-bit field
    *                  from a register, masks it, and shifts it to
    *                  the bottom of the register (retrieving a K_xxx
    *                  value, for example).
    *
    *  V_xxx(X)        VALUE.  This macro computes the value of a
    *                  K_xxx constant shifted to the correct position
    *                  in the register.
    ********************************************************************* */




#ifndef _SB1250_DEFS_H
#define _SB1250_DEFS_H

/*
 * Cast to 64-bit number.  Presumably the syntax is different in 
 * assembly language.
 *
 * Note: you'll need to define uint32_t and uint64_t in your headers.
 */

#if !defined(__ASSEMBLER__)
#define _SB_MAKE64(x) ((uint64_t)(x))
#define _SB_MAKE32(x) ((uint32_t)(x))
#else
#define _SB_MAKE64(x) (x)
#define _SB_MAKE32(x) (x)
#endif


/*
 * Make a mask for 1 bit at position 'n'
 */

#define _SB_MAKEMASK1(n) (_SB_MAKE64(1) << _SB_MAKE64(n))
#define _SB_MAKEMASK1_32(n) (_SB_MAKE32(1) << _SB_MAKE32(n))

/*
 * Make a mask for 'v' bits at position 'n'
 */

#define _SB_MAKEMASK(v,n) (_SB_MAKE64((_SB_MAKE64(1)<<(v))-1) << _SB_MAKE64(n))
#define _SB_MAKEMASK_32(v,n) (_SB_MAKE32((_SB_MAKE32(1)<<(v))-1) << _SB_MAKE32(n))

/*
 * Make a value at 'v' at bit position 'n'
 */

#define _SB_MAKEVALUE(v,n) (_SB_MAKE64(v) << _SB_MAKE64(n))
#define _SB_MAKEVALUE_32(v,n) (_SB_MAKE32(v) << _SB_MAKE32(n))

#define _SB_GETVALUE(v,n,m) ((_SB_MAKE64(v) & _SB_MAKE64(m)) >> _SB_MAKE64(n))
#define _SB_GETVALUE_32(v,n,m) ((_SB_MAKE32(v) & _SB_MAKE32(m)) >> _SB_MAKE32(n))

/*
 * Macros to read/write on-chip registers
 * XXX should we do the PHYS_TO_K1 here?
 */


#if !defined(__ASSEMBLER__)
#define SBWRITECSR(csr,val) *((volatile uint64_t *) PHYS_TO_K1(csr)) = (val)
#define SBREADCSR(csr) (*((volatile uint64_t *) PHYS_TO_K1(csr)))
#endif /* __ASSEMBLER__ */

#endif
