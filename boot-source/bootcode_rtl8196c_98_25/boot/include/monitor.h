
/*
==========================================================================
; Copyright (C) 2000,2010 Global View Corporation
;
; Module Name:
;		monitor.h
;
; Description:
;		Monitor global constant define
;
; author:
;	Steve Chen
;
; Revision History:
;	2000/12/26		: Initial release
===========================================================================
*/
#ifndef _MONITOR_H_
#define _MONITOR_H_

#define P_STACKSIZE	0x8000	 /* sets stack size to 32K */

#define MAX_ARGV				20	
#define MAX_MONITOR_BUFFER			128
#define PAGE_ECHO_HEIGHT						18

#ifndef TRUE
#define TRUE			1
#endif	//TRUE
#ifndef NULL
#define NULL			0
#endif	//NULL

#ifndef FALSE
#define FALSE			0
#endif	//FALSE

#define CONST	const



typedef					unsigned	char	  UCHAR;
typedef					unsigned	short	  USHORT;
typedef					unsigned	int	  UINT;
typedef					unsigned	long	  ULONG;
typedef volatile			char	  VCHAR;
typedef volatile	unsigned	char	  VUCHAR;
typedef volatile			short	  VSHORT;
typedef volatile	unsigned	short	  VUSHORT;
typedef volatile			int	  VINT;
typedef volatile	unsigned	int	  VUINT;
typedef volatile			long	  VLONG;
typedef volatile	unsigned	long	  VULONG;

// Command Table
typedef struct {
	const char*	cmd ;			// Input command string
	int	n_arg;
	int		(*func)( int argc, char* argv[] );
	const char*	msg ;			// Help message
} COMMAND_TABLE ;


extern int dprintf(char *fmt, ...);


#endif
