/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : CLE Glue Header File for Unix-like Userspace Program
* Abstract :                                                           
*   This file is included by CLE module.
*   This file provides all glue functions that are needed in user-space for CLE shell.
* Author : Yung-Chien Lo (yjlou@realtek.com.tw)
* -------------------------------------------------------
* $Id: cle_userglue.h,v 1.1 2007/12/04 11:52:00 joeylin Exp $
*/

#ifndef _CLE_USERGLUE_
#define _CLE_USERGLUE_

#if defined(RTL865X_MODEL_USER)

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	#include "rtl865x/rtl_utils.h"
	#define read_c0_status() 0
	#define write_c0_status(dummy) do {} while (0)

	#define rtl865x_pci_ioread8( addr ) 0
	#define rtl865x_pci_ioread16( addr ) 0
	#define rtl865x_pci_ioread32( addr ) 0
	#define rtl865x_pci_iowrite8( addr, val) do {} while (0)
	#define rtl865x_pci_iowrite16( addr, val) do {} while (0)
	#define rtl865x_pci_iowrite32( addr, val) do {} while (0)

	#define rtlglue_malloc malloc
	#define rtlglue_free free
	#define simple_strtol strtoul

int cle_userglue_init( void );
int cle_userglue_read( char *buf, unsigned long maxlen, char* cmd, char ch );
int cle_userglue_write( char *ubuf, unsigned long count );

#endif	/* defined(RTL865X_MODEL_USER) */

#endif/*_CLE_USERGLUE_*/
