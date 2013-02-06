/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : data struct for parser engine 
* Abstract :                                                           
* Author : Ida Yang (idayang@realtek.com.tw)               
* $Id: cle_struct.h,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_struct.h,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.3  2006/07/13 16:01:17  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.2  2004/11/10 05:19:00  cfliu
* 	+: Use 	cle_newCmdRoot as new cmd tree root
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.26  2003/07/21 06:30:01  cfliu
* remove unused fields
*
* Revision 1.25  2003/03/31 10:47:38  cfliu
* no message
*
* Revision 1.24  2003/03/13 10:30:26  cfliu
* Fix type mismatch problem
*
* Revision 1.23  2003/01/02 04:34:14  cfliu
* Implement dirStack to support stateful group
*
* Revision 1.22  2002/12/26 14:00:10  cfliu
* Add CLE_MAXDIRSTACKDEPTH to support per-used stateful directory stack
*
* Revision 1.21  2002/12/20 03:46:44  idayang
* add userDb flag tabMode
* add cle_setTabMode()
*
* Revision 1.20  2002/12/12 09:04:37  idayang
* remove warnings
*
* Revision 1.19  2002/12/12 07:06:55  idayang
* add telnet_userDb_t in cle_user_t for telnet data
*
* Revision 1.18  2002/12/06 10:56:06  cfliu
* Remove displayFunc()...
*
* -------------------------------------------------------
*/

#ifndef CLE_STRUCT_H
#define CLE_STRUCT_H

#include "rtl_cle.h"

typedef struct cle_tokenBuffer_s 
{

uint32	x, y, rows, rowPtr;
int8**	buffer;

uint8	helpFlag; //CLE_execCmd , CLE_helpCmd , CLE_helpKeyword;
} cle_tokenBuffer_t;

#define CLE_INPARSE 1
#define CLE_OUTPARSE 0
#define CLE_MAXDIRSTACKDEPTH	64   	//may be too conservative...

typedef struct cle_user_s
{
	uint32	hierarchy;
	uint32  nowHierarchy;
	uint32	accessLevel;
	uint32	uiStyle;
	uint32  state;
	uint32	initComplete;
	uint32	stackDepth;
	cle_grp_t	**stack;
	struct {
	uint32 priviledgeMode:1;    /* for the customer's system manager used only */
	uint32 superengMode:1;       /* for the field enginner and interla use only */
	uint32 tabMode:1;
	uint32 isTelnet:1;    /* indicate telent session */
	uint32 isCli:1;    /* indicate local console session */
	uint32 isHttp:1;     /* indicate web server session */
	uint32 useItemCount:1;/* item auto counting for return buffer*/
	uint32 funcExecResultError:1;/* recorded function execution result (0:no error)(1:error)*/ 
	}flag;	
	uint32 itemCount;//for counting how much items adding to outputBuf
	uint32 fieldCount;//for recorded outputBuf's field count
	int32 outputMsg;//for recorded any error msg when write to outputBuf
//	telnet_userDb_t *telnet_userDb;

	uint8 dirStack[CLE_MAXDIRSTACKDEPTH];
	uint32 dirStackUsed;
	//dirStack is used to store accepted parameters in each stateful command group.
	//Format: Len: Length of this parameter (32 bit, if <0, succedding Value is a string and abs(Len) is its string length
	//		 Value: Content of parameter (4 byte aligned)
	//		 Level, Total: each 16 bits. 'Level' is the value of user->hierarchy in this command group. 'Total' is the total length (in bytes) of all Len, Value pairs in this command group level.
	//When pushing/pop elements into dirStack, always start from the end of dirStack[]
	//+---+----+---+----+-----+-----------+
	//|Len|Value|Len|Value| ... ...| Level, Total |
	//+---+----+---+----+-----+-----------+

}cle_user_t;

typedef struct cle_parseResult_s
{
	int32 type;
	int8 msg[16];
} cle_parseResult_t;

#define CLE_userNameSize 16
#define CLE_userIdError 9999

typedef struct cle_userIdTable_s
{
	int8 userName[CLE_userNameSize];
	uint32 userID;
} cle_userIdTable_t;

typedef struct cle_consoleSrc_s
{
int8* outbuf;
uint32 outbufSize;
uint32 userID;
cle_screenInfo_t screen;		
} cle_consoleSrc_t;

extern int8 * cle_exitKeyword[];
extern int32 cle_exitKeywordNum;

extern int8 * cle_leaveGrpKeyword[];
extern int32 cle_leaveGrpKeywordNum;

extern int8 * cle_helpKeyword[];
extern int32 cle_helpKeywordNum;

int32 cle_checkPriviledge(uint32 userID,uint16 accessLevel);
int32 cle_getTelnetSocket(uint32 userID);

int8 cle_ioReadFromTelnet(uint32 userID,uint32 wait);
int8 cle_ioWriteToTelnet(uint32 userID,int8* buf,int32 len);

extern cle_grp_t cle_newCmdRoot[];
#endif

