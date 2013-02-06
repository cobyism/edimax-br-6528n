/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Cisco CLI Command parser header file
* Abstract :
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)
* $Id: cle_ciscoParser.h,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_ciscoParser.h,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.3  2006/07/13 16:01:16  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.2  2004/03/21 09:17:20  cfliu
* no message
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.7  2003/03/12 10:42:58  cfliu
* Add CISCOCMD_ACCEPT_SYMBOL,_STRING...
*
* Revision 1.6  2003/01/02 04:44:52  cfliu
* Add ciscoCmdParserFlushTmpBuffer()
*
* Revision 1.5  2002/12/26 14:06:07  cfliu
* ciscoCmdGetNextToken becomes internal API.
*
* Revision 1.4  2002/12/18 03:43:28  cfliu
* Add ciscoCmdParserCandidate() and change ciscoCmdParserGetCandidateTokenList() prototype
*
* Revision 1.3  2002/12/06 11:01:12  cfliu
* Make this header file private to local modules.
* All APIs needed only by CLE parser are kept here.
*
* Revision 1.4  2002/12/05 07:21:12  cfliu
* Add CVS header.
* Add enumFunc()
*
*/

#ifndef _MGMT_CLE_CISCOCMDPARSER_H
#define _MGMT_CLE_CISCOCMDPARSER_H

extern  cle_paramType_t systemParamTypes[];
extern int32 sysParamTypesNum;


#define CISCOCMD_ACCEPT_SYMBOL	"<cr>"
#define CISCOCMD_ACCEPT_STRING	"Acceptable command. Press <Return> to commit command..."
#define CISCOCMD_HELPMSG_FORMAT	"%-16s %s\n"
/*********************************************************************************
Exported API prototype
**********************************************************************************/
void ciscoCmdParserGetAllCandidates(void);
void ciscoCmdParserGetCandidate(void *lastTokenNode);
int32 ciscoCmdGetNextToken(int8 **nextToken, int32 *size, int8 **savedData);
int32 ciscoCmdParserGetCandidateTokenList(struct enumList *list, int32 totalEntries);

//If ciscoCmdParser() returns -1, pass the savedData returned by ciscoCmdParser() to print help message
int32 ciscoCmdParserShowHelp(struct enumList * list, int32 totalEntries);

//Returns the number of valid tokens, if returns -1, command is invalid.
//If command is invalid, *savedData returns the last valid token data structure.
//If commans is valid, *savedData saves the address of candidateNode structure.
int32 ciscoCmdParser( int32 argc, int8 **argv, cle_exec_t **thisCmd, int8 **savedData, int8 **errorToken);

//Release temp resource after users store all state info in his dirStack upon entering a stateful command group
int32 ciscoCmdParserFlushTmpBuffer(void);

//Remember to release all state infomation before any new call to ciscoCmdParser
int32 ciscoCmdParserFree(void);

//cfliu: (2002.11.25) Before ciscoCmdParser module is integrated into our CLE, ciscoCmdParser can 
//be called independently from Ida's current argc,argv style CLE engine. Ref. the dhcp_client.c module for example.
//Returns -1 if command is not valid, otherwise returns 0.
//Modules's command handler is called inside. 
int32 ciscoCmdChecker(uint32 userId, int32 argc, int8 **argv, cle_exec_t *cmd);

//Print Command format string (without help message)
void ciscoCmdPrintSynopsis(int8 *help, int8 *formatString);

#endif
