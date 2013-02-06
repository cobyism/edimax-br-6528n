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

/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Device function prototypes		File: cfe_api.h
    *  
    *  This module contains prototypes for cfe_devfuncs.c, a set
    *  of wrapper routines to the IOCB interface.  This file,
    *  along with cfe_api.c, can be incorporated into programs
    *  that need to call CFE.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    ********************************************************************* */


#define CFE_EPTSEAL 0x43464531
#define CFE_APIENTRY 0x9FC00500
#define CFE_APISEAL  0x9FC00508

#ifndef __ASSEMBLER__
int cfe_init(cfe_xuint_t handle);
int cfe_open(char *name);
int cfe_close(int handle);
int cfe_readblk(int handle,cfe_xint_t offset,unsigned char *buffer,int length);
int cfe_read(int handle,unsigned char *buffer,int length);
int cfe_writeblk(int handle,cfe_xint_t offset,unsigned char *buffer,int length);
int cfe_write(int handle,unsigned char *buffer,int length);
int cfe_ioctl(int handle,unsigned int ioctlnum,unsigned char *buffer,int length,int *retlen);
int cfe_inpstat(int handle);
int cfe_enumenv(int idx,char *name,int namelen,char *val,int vallen);
int cfe_enummem(long idx, unsigned long long *addr, unsigned long long *size, long *type);
int cfe_setenv(char *name,char *val);
int cfe_getenv(char *name,char *dest,int destlen);
long long cfe_getticks(void);
int cfe_exit(int warm, int status);
int cfe_flushcache(int flg);
int cfe_getstdhandle(int flg);
int cfe_start_cpu(int cpu, void (*fn)(void), long sp, long gp, long a1);

void cfe_open_console(void);
void cfe_console_print(char *);
#endif
