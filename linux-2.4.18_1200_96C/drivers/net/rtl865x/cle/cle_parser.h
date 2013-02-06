/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : parser engine 
* Abstract :                                                           
* Author : Ida Yang (idayang@realtek.com.tw)               
*
* -------------------------------------------------------
*/

#ifndef cle_parser_h
#define cle_parser_h

#include "cle_struct.h"

void cle_cmdParse(uint32 userID,
				   uint32 argCnt);
void cle_helpParse(uint32 userID,
				   uint32 argCnt);
void cle_helpStringParse(uint32 userID,
				   uint32 argCnt);
void cle_listTreeNodeDesp(uint32 userID);
static int32 cle_cmdEveryWhereCheck(uint32 userID,
							uint32 argCnt,
							int32 bestMatch);
static int32 cle_execCheck(uint32 userID, 
			   uint32 argCnt,
			   int32 bestMatch);
static int32 cle_groupCheck(uint32 userID, 
			   uint32 argCnt,
			   int32 bestMatch);
static int32 cle_execHelp(uint32 userID, 
			   uint32 argCnt,
			   int32 bestMatch);
static int32 cle_grpHelp(uint32 userID, 
			   uint32 argCnt,
			   int32 bestMatch,
			   int32 mode);
uint32 cle_returnUserID(int8* userName);
uint32 cle_getUserID(int8* userName);

cle_screenInfo_t* cle_getScreenInfo(uint32 userID);

static int32 cle_cmdCount(uint32 userID, uint32 argCnt);
static int32 cle_execCount( uint32 userID, uint32 argCnt);
static int32 cle_groupCount(uint32 userID, uint32 argCnt);
static int32 cle_cmdEveryWhereCount(uint32 userID,uint32 argCnt);

#endif









