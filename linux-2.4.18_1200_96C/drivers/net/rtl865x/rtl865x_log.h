/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : rtl8651_log.h
* Abstract : 
* Creator : 
* Author :  
*/
#ifndef _RTL8651_LOG_H
#define _RTL8651_LOG_H

#if defined(__KERNEL__) && defined(__linux__)
	/* kernel mode */
	#include "rtl865x/rtl8651_tblDrv.h"
#else
	/*user mode*/
	#include <rtl8651_tblDrv.h>
#endif

#ifdef _RTL_NEW_LOGGING_MODEL

/**********************************************************
	Global variables
**********************************************************/
#define RTL8651_LOGPROC_FILE			"log_module"
#define RTL8651_MAX_LOG_MODULES		9
#define RTL8651_MAX_LOG_MSGSIZE		256
#define RTL8651_MAX_LOG_BUFFERSIZE	16000
#define RTL8651_MAX_LOG_ROUND			256		/*the type of round field is uint8*/

/**********************************************************
	Extern variable
**********************************************************/
extern int32 timezone_diff;	/* for re_core to set time zone */
/**********************************************************
	Data structure
**********************************************************/
//for circular buffer
struct circ_buf 
{	
	int size;	
	int head;	
	int tail;	
	uint8 round;
	char data[1];
};
/***************************************************************************
			External Functions for forwarding engine declaration
***************************************************************************/
int32 myLoggingFunction(uint32 moduleId, uint32 logNo, rtl8651_logInfo_t *info);
void* rtl8651_log_getCircBufPtr(uint32 moduleId, uint32* logSize);
int32 rtl8651_log_clearCircBuf(uint32 moduleId);

#endif /* _RTL_NEW_LOGGING_MODEL */
#endif /* _RTL8651_LOG_H */
