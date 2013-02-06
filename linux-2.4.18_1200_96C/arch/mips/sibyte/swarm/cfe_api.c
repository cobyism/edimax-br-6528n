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
    *  Device Function stubs			File: cfe_api.c
    *  
    *  This module contains device function stubs (small routines to
    *  call the standard "iocb" interface entry point to CFE).
    *  There should be one routine here per iocb function call.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    ********************************************************************* */


#include "cfe_xiocb.h"
#include "cfe_api.h"
#include <linux/string.h>

static long cfe_console_handle = -1;
static int (*cfe_dispfunc)(long handle,cfe_xiocb_t *xiocb) = 0;
static cfe_xuint_t cfe_handle = 0;

/*
 * This macro makes a "signed 64-bit pointer" - basically extending a regular
 * pointer to its 64-bit compatibility space equivalent. 
 */
#define BIGPTR(x) (long long) (long) (x)

typedef unsigned long intptr_t;

int cfe_init(cfe_xuint_t handle)
{
	unsigned int *sealloc = (unsigned int *) (intptr_t) (int)  CFE_APISEAL;
	if (*sealloc != CFE_EPTSEAL) return -1;
	cfe_dispfunc = (void *) (cfe_xptr_t) (int) CFE_APIENTRY;
	if (handle) cfe_handle = handle;
	return 0;
}

int cfe_iocb_dispatch(cfe_xiocb_t *xiocb)
{
	if (!cfe_dispfunc)
		return -1;

	return (*cfe_dispfunc)(cfe_handle,xiocb);
}

static int cfe_strlen(char *name)
{
	int count = 0;
	
	while (*name) {
		count++;
		name++;
	}
	
	return count;
}

int cfe_open(char *name)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_OPEN;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_buffer_t);
	xiocb.plist.xiocb_buffer.buf_offset = 0;
	xiocb.plist.xiocb_buffer.buf_ptr = BIGPTR(name);
	xiocb.plist.xiocb_buffer.buf_length = cfe_strlen(name);
	
	cfe_iocb_dispatch(&xiocb);
	
	return (xiocb.xiocb_status < 0) ? xiocb.xiocb_status : xiocb.xiocb_handle;
}

int cfe_close(int handle)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_CLOSE;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = handle;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = 0;
	
	cfe_iocb_dispatch(&xiocb);
	
	return (xiocb.xiocb_status);
	
}

int cfe_readblk(int handle,cfe_xint_t offset,unsigned char *buffer,int length)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_READ;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = handle;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_buffer_t);
	xiocb.plist.xiocb_buffer.buf_offset = offset;
	xiocb.plist.xiocb_buffer.buf_ptr = BIGPTR(buffer);
	xiocb.plist.xiocb_buffer.buf_length = length;
	
	cfe_iocb_dispatch(&xiocb);
	
	return (xiocb.xiocb_status < 0) ? xiocb.xiocb_status : xiocb.plist.xiocb_buffer.buf_retlen;
}

int cfe_read(int handle,unsigned char *buffer,int length)
{
	return cfe_readblk(handle,0,buffer,length);
}


int cfe_writeblk(int handle,cfe_xint_t offset,unsigned char *buffer,int length)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_WRITE;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = handle;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_buffer_t);
	xiocb.plist.xiocb_buffer.buf_offset = offset;
	xiocb.plist.xiocb_buffer.buf_ptr = BIGPTR(buffer);
	xiocb.plist.xiocb_buffer.buf_length = length;
	
	cfe_iocb_dispatch(&xiocb);
	
	return (xiocb.xiocb_status < 0) ? xiocb.xiocb_status : xiocb.plist.xiocb_buffer.buf_retlen;
}

int cfe_write(int handle,unsigned char *buffer,int length)
{
	return cfe_writeblk(handle,0,buffer,length);
}


int cfe_ioctl(int handle,unsigned int ioctlnum,unsigned char *buffer,int length,int *retlen)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_IOCTL;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = handle;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_buffer_t);
	xiocb.plist.xiocb_buffer.buf_ioctlcmd = (cfe_xint_t) ioctlnum;
	xiocb.plist.xiocb_buffer.buf_ptr = BIGPTR(buffer);
	xiocb.plist.xiocb_buffer.buf_length = length;
	
	cfe_iocb_dispatch(&xiocb);
	
	if (retlen) *retlen = xiocb.plist.xiocb_buffer.buf_retlen;
	return xiocb.xiocb_status;
}

int cfe_inpstat(int handle)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_INPSTAT;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = handle;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_inpstat_t);
	xiocb.plist.xiocb_inpstat.inp_status = 0;
	
	cfe_iocb_dispatch(&xiocb);
	
	if (xiocb.xiocb_status < 0) return xiocb.xiocb_status;
	
	return xiocb.plist.xiocb_inpstat.inp_status;
	
}

long long cfe_getticks(void)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_FW_GETTIME;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_time_t);
	xiocb.plist.xiocb_time.ticks = 0;
	
	cfe_iocb_dispatch(&xiocb);
	
	return xiocb.plist.xiocb_time.ticks;
	
}

int cfe_getenv(char *name,char *dest,int destlen)
{
	cfe_xiocb_t xiocb;
	
	*dest = 0;
	
	xiocb.xiocb_fcode = CFE_CMD_ENV_GET;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_envbuf_t);
	xiocb.plist.xiocb_envbuf.enum_idx = 0;
	xiocb.plist.xiocb_envbuf.name_ptr = BIGPTR(name);
	xiocb.plist.xiocb_envbuf.name_length = cfe_strlen(name);
	xiocb.plist.xiocb_envbuf.val_ptr = BIGPTR(dest);
	xiocb.plist.xiocb_envbuf.val_length = destlen;
	
	cfe_iocb_dispatch(&xiocb);
	
	return xiocb.xiocb_status;
}

int cfe_setenv(char *name,char *val)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_ENV_SET;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_envbuf_t);
	xiocb.plist.xiocb_envbuf.enum_idx = 0;
	xiocb.plist.xiocb_envbuf.name_ptr = BIGPTR(name);
	xiocb.plist.xiocb_envbuf.name_length = cfe_strlen(name);
	xiocb.plist.xiocb_envbuf.val_ptr = BIGPTR(val);
	xiocb.plist.xiocb_envbuf.val_length = cfe_strlen(val);
	
	cfe_iocb_dispatch(&xiocb);
	
	return xiocb.xiocb_status;
}

int cfe_enummem(long idx, unsigned long long *addr, unsigned long long *size, long *type)
{
	cfe_xiocb_t xiocb;
	xiocb.xiocb_fcode = CFE_CMD_FW_MEMENUM;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_meminfo_t);
	xiocb.plist.xiocb_meminfo.mi_idx = idx;
	
	cfe_iocb_dispatch(&xiocb);
	
	(*addr) = xiocb.plist.xiocb_meminfo.mi_addr;
	(*size) = xiocb.plist.xiocb_meminfo.mi_size;
	(*type) = xiocb.plist.xiocb_meminfo.mi_type;

	return xiocb.xiocb_status;
}


int cfe_enumenv(int idx,char *name,int namelen,char *val,int vallen)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_ENV_SET;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = 0;
	xiocb.xiocb_psize = sizeof(xiocb_envbuf_t);
	xiocb.plist.xiocb_envbuf.enum_idx = idx;
	xiocb.plist.xiocb_envbuf.name_ptr = BIGPTR(name);
	xiocb.plist.xiocb_envbuf.name_length = namelen;
	xiocb.plist.xiocb_envbuf.val_ptr = BIGPTR(val);
	xiocb.plist.xiocb_envbuf.val_length = vallen;
	
	cfe_iocb_dispatch(&xiocb);
	
	return xiocb.xiocb_status;
}

int cfe_exit(int warm, int status)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_FW_RESTART;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = warm ? CFE_FLG_WARMSTART : 0;
	xiocb.xiocb_psize = sizeof(xiocb_exitstat_t);
	xiocb.plist.xiocb_exitstat.status = (cfe_xint_t) status;
	
	cfe_iocb_dispatch(&xiocb);
	
	return (xiocb.xiocb_status);
}

int cfe_flushcache(int flg)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_FW_FLUSHCACHE;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = flg;
	xiocb.xiocb_psize = 0;
	
	cfe_iocb_dispatch(&xiocb);
	
	return xiocb.xiocb_status;
}

int cfe_getstdhandle(int flg)
{
	cfe_xiocb_t xiocb;
	
	xiocb.xiocb_fcode = CFE_CMD_DEV_GETHANDLE;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags = flg;
	xiocb.xiocb_psize = 0;
	
	cfe_iocb_dispatch(&xiocb);
	
	return (xiocb.xiocb_status < 0) ? xiocb.xiocb_status : xiocb.xiocb_handle;
	
}

int cfe_start_cpu(int cpu, void (*fn)(void), long sp, long gp, long a1)
{
	cfe_xiocb_t xiocb;

	xiocb.xiocb_fcode = CFE_CMD_FW_CPUCTL;
	xiocb.xiocb_status = 0;
	xiocb.xiocb_handle = 0;
	xiocb.xiocb_flags  = 0;
	xiocb.xiocb_psize = sizeof(xiocb_cpuctl_t);
	xiocb.plist.xiocb_cpuctl.cpu_number = cpu;
	xiocb.plist.xiocb_cpuctl.cpu_command = CFE_CPU_CMD_START;
	xiocb.plist.xiocb_cpuctl.gp_val = gp;
	xiocb.plist.xiocb_cpuctl.sp_val = sp;
	xiocb.plist.xiocb_cpuctl.a1_val = a1;
	xiocb.plist.xiocb_cpuctl.start_addr = (long)fn;

	cfe_iocb_dispatch(&xiocb);

	return xiocb.xiocb_status;
}


void cfe_open_console()
{
	cfe_console_handle = cfe_getstdhandle(CFE_STDHANDLE_CONSOLE);
}

void cfe_console_print(char *str)
{
	int len = strlen(str);
	int res;

	if (cfe_console_handle != -1) {
		cfe_write(cfe_console_handle, str, strlen(str));
		do {
			res = cfe_writeblk(cfe_console_handle, 0, str, len);
			if (res < 0)
				break;
			str += res;
			len -= res;
		} while (len);
	}
}
