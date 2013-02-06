/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : parser engine for command line 
* Abstract :                                                           
* Author : Ida Yang (idayang@realtek.com.tw)               
*
* -------------------------------------------------------
* $Id: cle_parser.c,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_parser.c,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.16  2007/05/04 06:32:58  yjlou
* *: -: remove -Wno-implicit in Makefile. Fixed the compile error on model code.
*
* Revision 1.15  2007/01/09 05:13:59  michaelhuang
* *: fixed some compile flag error
*
* Revision 1.14  2007/01/09 02:49:52  michaelhuang
* *: Prepare implement test module for RTL8672
*
* Revision 1.13  2006/12/04 08:34:21  alva_zhang
* replace all cle_printf  with rtlglue_printf
*
* Revision 1.12  2006/12/04 07:29:35  alva_zhang
* replace all printk with rtlglue_printf
*
* Revision 1.11  2006/07/13 16:01:16  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.10  2006/05/03 01:46:33  yjlou
* *: little fixed for splint check
*
* Revision 1.9  2005/09/09 15:04:14  yjlou
* *: Fixed for Kernel Model Code. Including:
*    change printf() to rtlglue_printf()
*    unused variables
*    uninitialized variables
*    blah blah
*
* Revision 1.8  2005/06/10 05:32:22  yjlou
* +: Porting CLE Shell to Linux user space for model test.
*    See RTL865X_MODEL_USER compile flags.
*
* Revision 1.7  2005/01/19 05:39:36  cfliu
* fix .. cmd bug.
*
* Revision 1.6  2004/11/10 05:18:13  cfliu
* +: Move utility routines from ROME driver to cle and declare them as static
* functions.
*
* Revision 1.5  2004/06/29 07:12:38  cfliu
* *: Fix gcc3.3.3 compiler type check problem for va_list
*
* Revision 1.4  2004/03/21 09:17:20  cfliu
* no message
*
* Revision 1.3  2004/03/19 13:12:42  cfliu
* no message
*
* Revision 1.2  2004/03/03 13:07:38  cfliu
* Removed rtl-cle.h
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.85  2003/08/11 14:54:37  cfliu
* no message
*
* Revision 1.84  2003/08/06 03:33:28  hiwu
* remove unused include files
*
* Revision 1.83  2003/07/21 06:32:19  cfliu
* fix compile warnings
*
* Revision 1.82  2003/06/06 07:56:08  cfliu
* print error numbr when command failed.
*
* Revision 1.81  2003/05/28 07:11:08  danwu
* remove __mod()
*
* Revision 1.80  2003/05/15 08:24:00  jzchen
* For command everywhere equipt with cisco like parser
*
* Revision 1.79  2003/05/15 07:53:15  jzchen
* Change from CLE_MODE_PRIVILEDGE to CLE_MODE_PRIVILEGE
*
* Revision 1.78  2003/04/30 15:36:08  cfliu
* rename macros. macro moved to types.h
*
* Revision 1.77  2003/04/15 05:28:14  jzchen
* Script execution error will show whole error command
*
* Revision 1.76  2003/04/15 03:50:51  jzchen
* Avoid script to show "SUCCESS!!"
*
* Revision 1.75  2003/04/10 13:19:32  orlando
* modify implemenation of cle_cmdEveryWhereCheck to
* check return status of cmdEveryWhere[pos].execFun().
*
* Revision 1.74  2003/03/31 10:48:59  cfliu
* Eliminate unneccessary arguments in cle_init()
* Remove cle_userCli
*
* Revision 1.73  2003/03/21 08:05:58  cfliu
* Bug fixes...
*
* Revision 1.72  2003/03/18 14:31:19  cfliu
* Fix greenhill compile warnings and move run.files in rtl8650 platform compile ok.
*
* Revision 1.71  2003/03/17 09:30:15  cfliu
* bug fix for stateful command group.
*
* Revision 1.70  2003/03/13 10:32:52  cfliu
* Fix type mismatch problem
*
* Revision 1.69  2003/03/12 10:40:14  cfliu
* Add index2string array to CLE.
* Fix bugs in stateful group command processing
*
* Revision 1.68  2003/03/04 01:59:32  danwu
* clear cle_init data structure in cle_init()
*
* Revision 1.67  2003/02/07 09:56:27  cfliu
* bugfix for help mode.
*
* Revision 1.66  2003/01/21 10:34:59  hiwu
* addd in.h
*
* Revision 1.65  2003/01/21 06:01:18  cfliu
* * Remove redundant header file inclusion...still need closer look on them..
*
* Revision 1.64  2003/01/10 09:32:46  idayang
* fix bug in cle_groupCheck() about tab function
*
* Revision 1.63  2003/01/03 09:01:29  idayang
* modify tab function - fix bug
*
* Revision 1.62  2003/01/03 08:54:43  idayang
* modify tab function - about show message
*
* Revision 1.61  2003/01/02 04:44:14  cfliu
* Implement stateful group feature.
* Tokens are pushed into/poped from user's dirStack[] when entering/exiting a stateful group.
* When user's execFunc() calls cle_getNextCmdToken(), tokens saved in dirStack[] are retrieved first
* and then ciscoCmdGetNextToken() returns later tokens.
*
* Add Token auto completion feature when user presses the TAB key.
*
* Add cle_changeCmdGroup() and cle_exitCmdGroup() to centralize all cmd group switching code
* Add cle_getNextToken() to support stateful group feature. It wraps ciscoCmdGetNextToken()
*
* Revision 1.60  2002/12/30 07:03:14  idayang
* add tab function
*
* Revision 1.59  2002/12/27 04:24:39  elvis
* (Committed by cfliu)
* Fix my misinterpretation of ida's user->hierarchy and user->nowHierarchy problem
*
* Revision 1.58  2002/12/26 14:05:21  cfliu
* Add per-user based stateful directory stack.
* Add dynamic command/group changing capability
* Reorgranize cle_groupCheck function. before entering/leaving a group, user's dirStack should be checked first (working...)
* Add cle_setNextGroupName() for cisco-like cmd's check function to tell CLE parser when to switch to a new group
*
* Revision 1.57  2002/12/24 04:19:58  cfliu
* Remove cle_printToParseResult. Use clePrintf() instead
*
* Revision 1.56  2002/12/20 03:37:13  cfliu
* Use userDb[userID].flag.tabMode to identify 'mode' parameter for cle_ciscoCmdProcesser()
*
* Revision 1.55  2002/12/20 03:29:32  cfliu
* Add token auto completion in cisco parser
* Add fully best match support for ? mode
*
* Revision 1.54  2002/12/18 03:42:34  cfliu
* Add ? support for Web clients
* Add ? support for option list
*
* Revision 1.53  2002/12/17 03:49:56  cfliu
* Extend best match function with substring pattern best match.
* Ex: 'sock pkt' would match 'sock rawpkt' if no other options have the same substring 'pkt'
*
* Revision 1.52  2002/12/12 10:58:14  cfliu
* Add more help functions...
*
* Revision 1.51  2002/12/12 09:32:40  cfliu
* Web user never remembers which group he is in. user->hierachy always=0
* Fix accessLevel comparison problem
*
* Revision 1.50  2002/12/12 09:04:02  idayang
* remove warnings
*
* Revision 1.49  2002/12/12 07:05:16  idayang
* add API for telnet module
*
* Revision 1.48  2002/12/11 13:02:58  idayang
* fix a bug : in cle_addRowToOutput
*
* Revision 1.47  2002/12/11 12:46:13  cfliu
* bug fixes.
*
* Revision 1.46  2002/12/11 02:46:41  cfliu
* Add CLE_RESULT_INCOMPLETE_COMMAND so formatted output could be accepted by Web CLI.
*
* Revision 1.45  2002/12/10 10:47:33  cfliu
* CLE returns "Incomplete command" when ciscoParser is used and command not accepted.
*
* Revision 1.44  2002/12/09 10:28:04  cfliu
* no message
*
* Revision 1.43  2002/12/09 10:17:23  cfliu
* Add "?" online help support
* Accepts partial command doing best match for new Cisco-like commands
*
* Revision 1.42  2002/12/06 10:58:58  cfliu
* Merge Cisco-like commands into our CLE parser
* Add enumList[], .
* Remove displayFunc
* Remove cle_absParser
* Add cle_ciscoCmdProcessor()
*
* Revision 1.41  2002/12/04 03:25:19  idayang
* int cle_initUser() - if UI_STYLE=0 ,use default( stand IO )
*
* Revision 1.40  2002/12/03 09:59:09  idayang
* fix_bug cle_addRowToOutput about accept "\n" .
*
* Revision 1.39  2002/12/03 07:38:01  idayang
* modify cle_addRowToOutput to accept "\n" .
* "\n" mean it is end of Row
*
* Revision 1.38  2002/12/02 13:48:13  idayang
* modify cle_addItemToOutput to translate host order ip to network order ip automatically
*
* Revision 1.37  2002/12/02 06:11:08  idayang
* init userDb and user login
*
* Revision 1.36  2002/11/29 12:35:28  idayang
* fix a bug in cle_addRowToOutput
*
* Revision 1.35  2002/11/29 12:10:03  idayang
* add cle_addRowToOutput
* add cle_endRowToOutput
*
* Revision 1.34  2002/11/28 15:03:33  idayang
* add two flag to userDb about:
* - execFun's return result
* - if auto counting items in rnBuffer's API
*
* Revision 1.33  2002/11/27 10:08:25  waynelee
* add return buffer type - CLE_RNBUF_IPADDRESS_HOSTORDER
* modify return buffer API about auto counting items
*
* Revision 1.32  2002/11/21 03:43:34  idayang
* remove debug msg in cle_addStringItemToOutput
*
* Revision 1.31  2002/11/15 03:29:20  idayang
* add cle_resetEntryCount API
*
* Revision 1.30  2002/11/14 08:42:50  idayang
* no message
*
* Revision 1.29  2002/11/14 08:08:48  idayang
* add cle_addStringItemToOutput API
*
* Revision 1.28  2002/11/14 07:57:39  idayang
* add CLE_RNBUF_RESULT for cle_rnBufHeader_t data_type
*
* Revision 1.27  2002/11/14 01:50:05  idayang
* add define for cle_rnBufHeader_t data_type
*
* Revision 1.26  2002/11/11 06:57:28  idayang
* reduce code size
*
* Revision 1.25  2002/11/05 09:30:31  cfliu
* Fix Greenhill compile warnings.
*
* Revision 1.24  2002/10/25 08:18:40  idayang
* fix cle_parser return value
*
* Revision 1.23  2002/10/16 09:42:06  idayang
* remove each item's friendly string in output format
*
* Revision 1.22  2002/10/16 02:00:45  idayang
* add cle_writeOutputHeader()
* add cle_addItemToOutput()
* add cle_addSubItemHeadToOutput()
* for write fix format output
*
* Revision 1.21  2002/10/09 08:34:50  idayang
* add reent_setCleUserId(userID) to cle_parser start
* for setting userId to thread info
*
* Revision 1.20  2002/09/20 07:28:29  idayang
* add cle_absParser API
* for execution command line at absolute directory
*
*/

#include "../rtl865x/types.h"
#include "../rtl865x/rtl_glue.h"
#include "rtl_cle.h"
#include "cle_struct.h"
#include "cle_define.h"
#include "cle_parser.h"
#include "cle_utility.h"
#include "cle_ciscoParser.h"
#include "cmd_cmdEveryWhere.h"
#include "cle_userglue.h"
#include "rtl_utils.h"

#define CISCOCMDHELP					1
#define CISCOCMDCOMPLETION			2


//#define cleVsnPrintf(a,b,c,d) _visprintf(a,c,d)
#define cleVsnPrintf(a,b,c,d) rtlglue_printf(a,c,d)
#define reent_getCleUserId()		0

#define MOD(a,b) ((a)%(b))

 

static uint32	cle_userNum;
cle_user_t *userDb;
static cle_parseResult_t parseResult;
static cle_userIdTable_t *cle_userIdTable;
//pthread_mutex_t	    cle_parserMutex; 

static cle_tokenBuffer_t	cle_tokens;
static uint32 cle_tokenGetCount;
static int8 *cle_firstCiscoCmdToken;
static int8 cle_nextGroupName[32];

struct  indexTextString {
	int8 text[sizeof"999"];
};
static struct  indexTextString *indexString;


uint32	cle_outputBufSize;
int8*	cle_outputBuffer;
uint32  cle_outputLen;
int8*	cle_hisBuffer;	
struct enumList  tokenEnum[CLE_MAXENUMENTRIES];

static void cle_tabModeReturn(uint32 userID);

#define SPRINTF(x) ((uint32)sprintf x)
static const int8 *ntop4(const uint8 *src,int8* dst, uint32 size)
{
	static const int8 fmt[] = "%u.%u.%u.%u";
	int8      tmp[sizeof "255.255.255.255"];

	if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) > size)
	{
		return (NULL);
	}
	strcpy(dst,tmp);
	return (dst);
}

#undef SPRINTF


// userNumber: Max number of users online
// tokenLen: Max number of chars of each command token
// tokenNum: Max number of command tokens in one command line
int32 cle_init( uint32 userNumber,uint32 tokenLen, uint32 tokenNum)
{
	uint32 i;

	//allocate data base for parser engine user----------------------------------------
	//userDb = (cle_user_t*) malloc(sizeof(cle_user_t)*userNumber);
	userDb = (cle_user_t*)rtlglue_malloc(sizeof(cle_user_t)*userNumber);
	memset(userDb, 0, sizeof(cle_user_t)*userNumber);
	if(!userDb)
		return CLE_INIT_ERROR;
	cle_userIdTable= (cle_userIdTable_t*) rtlglue_malloc(sizeof(cle_userIdTable_t)*userNumber);
	if(!cle_userIdTable)
		return CLE_INIT_ERROR;
	for(i=0;i<userNumber;i++)
	{
		cle_userIdTable[i].userID=CLE_userIdError;
		userDb[i].initComplete = FALSE;
	}
	
	cle_userNum = userNumber;
	//---------------------------------------------------------------------------------

	//allocate token array buffer------------------------------------------------------
	cle_tokens.x = tokenLen;
	cle_tokens.y = tokenNum;
	cle_tokens.buffer = (int8 **)rtlglue_malloc(tokenNum*sizeof(int8 *));
	if(!cle_tokens.buffer)
		return CLE_INIT_ERROR;		
	for(i=0;i<tokenNum;i++)
	{
		cle_tokens.buffer[i] = (int8 *)rtlglue_malloc(tokenLen * sizeof(int8));
		if(!cle_tokens.buffer[i])
			return CLE_INIT_ERROR;
	}
	//---------------------------------------------------------------------------------
	
	
	cle_hisBuffer = (int8 *)rtlglue_malloc(tokenLen * sizeof(int8));
	if(!cle_hisBuffer)
		return CLE_USER_INIT_ERROR;
	cle_tokenGetCount=0;
	cle_firstCiscoCmdToken=NULL;
	cle_nextGroupName[0] = '\0';

	//This strange array is used by command modules which use cisco cmd parser, 
	//to generate a string array for numbers 0~maxUnits. command engine needs them when it calls our _enumXXX() functions.
	indexString = (struct  indexTextString *)rtlglue_malloc(sizeof"999" *CLE_INDEX2STRING_SIZE);
	assert(indexString!=NULL);
	for(i=0; i < CLE_INDEX2STRING_SIZE; i++){
		sprintf((int8 *)&indexString[i], "%d", i);
	}
	
//	if (pthread_mutex_init(&cle_parserMutex, NULL) != 0)
//		return CLE_INIT_ERROR;
	
	return CLE_INIT_SUCCESS;
}


int8 *cle_index2string(uint32 index){
	assert(index<CLE_INDEX2STRING_SIZE);
	return (int8 *)&indexString[index];
}

uint32 cle_initUser(	int8*  userName,
					uint32 uiStyle,
					uint32 accessLevel,
					uint32 stackDepth,	//max number of sub command group hierachy. 
					cle_grp_t * commandEntryPoint)
{
	uint32 userId;
	
	userId=cle_getUserID(userName);
	if(userId==CLE_userIdError)
		return CLE_USER_INIT_ERROR;

	if(userId>=cle_userNum)
		return CLE_USER_INIT_ERROR;
	
	userDb[userId].stackDepth = stackDepth;
	userDb[userId].stack = (cle_grp_t ** )rtlglue_malloc(stackDepth*sizeof(cle_grp_t *));
	if(!userDb[userId].stack)
		return CLE_USER_INIT_ERROR;
	userDb[userId].stack[0]= commandEntryPoint;
	
	userDb[userId].hierarchy = 0;
	userDb[userId].nowHierarchy = 0;
	if(uiStyle==0)//use default CLE_UI_STYLE_STANDARD_IO
		uiStyle=CLE_UI_STYLE_STANDARD_IO;
	userDb[userId].uiStyle=uiStyle;
	userDb[userId].accessLevel=accessLevel;
	if(accessLevel&CLE_MODE_SUPERENG)
		userDb[userId].flag.superengMode=1;
	if(accessLevel&CLE_MODE_PRIVILEGE)
		userDb[userId].flag.priviledgeMode=1;
	
	userDb[userId].state=CLE_OUTPARSE;
	userDb[userId].initComplete = TRUE;
	memset(userDb[userId].dirStack, 0, sizeof(userDb[userId].dirStack));
	userDb[userId].dirStackUsed = 0;
	return userId;
}

//for telnet server 
//void cle_initUserData(uint32 userID,telnet_userDb_t *telnet_userDb)
//{

//	userDb[userID].hierarchy = 0;
//	userDb[userID].nowHierarchy = 0;
//	userDb[userID].accessLevel=0;
//	userDb[userID].flag.priviledgeMode=0;
//	userDb[userID].flag.superengMode=0;
//	userDb[userID].state=CLE_OUTPARSE;
//	userDb[userID].telnet_userDb=telnet_userDb;
//}


uint32 cle_returnUserID(int8* userName)
{
	int32 i;

	if(strlen(userName)>CLE_userNameSize)
		return CLE_userIdError;
	for(i=0;i<cle_userNum;i++)
		if((cle_userIdTable[i].userID!=CLE_userIdError) && strcmp(userName,cle_userIdTable[i].userName)==0)
			return cle_userIdTable[i].userID;

	return CLE_userIdError;
}

uint32 cle_getUserID(int8* userName)
{
	int32 i;

	if(strlen(userName)>CLE_userNameSize)
		return CLE_userIdError;
	for(i=0;i<cle_userNum;i++)
	{
		if((cle_userIdTable[i].userID!=CLE_userIdError)&&
			strcmp(userName,cle_userIdTable[i].userName)==0)
		{			
			if(userDb[cle_userIdTable[i].userID].initComplete==TRUE)
				return CLE_userIdError;
			else
			{
				return cle_userIdTable[i].userID;
			}
		}
	}

	for(i=0;i<cle_userNum;i++)
	{
		if(cle_userIdTable[i].userID==CLE_userIdError)
		{
			strncpy(cle_userIdTable[i].userName,userName,sizeof(cle_userIdTable[i].userName));
			cle_userIdTable[i].userID=i;
			return i;
		}
	}
	return CLE_userIdError;
}

int32 cle_parser(uint32 userID,int8* inputBuf,int8* outputBuf,uint32* outputLen)
{
	cle_user_t* user;
	int32 rn=0;
	
	user=&(userDb[userID]);

//	if (pthread_mutex_lock(&cle_parserMutex)!= 0) 
//		return CLE_PARSER_ERROR;

//	reent_setCleUserId(userID);
	if(cle_strToToken(inputBuf,&(cle_tokens)))
		return CLE_PARSER_ERROR;
	
	//for each command parse init----------------------
	if(ISSET(cle_getUiStyle(userID), CLE_UI_STYLE_WEB))
		user->hierarchy= 0;
	else
		user->hierarchy=user->nowHierarchy;

	cle_specialCheck(userID, inputBuf);

	cle_tokens.rowPtr=0;

	cle_outputBufSize=*outputLen;
	cle_outputBuffer=outputBuf;
	cle_outputLen=0;
	userDb[userID].outputMsg=CLE_WRITEOUTPUT_SUCCESS;
	userDb[userID].flag.funcExecResultError=0;

	memset(parseResult.msg, 0, sizeof(parseResult.msg));
	parseResult.type=0;
	if( (cle_outputBuffer!=NULL) && (cle_outputBufSize!=0) )
		memset(cle_outputBuffer,0,cle_outputBufSize*sizeof(int8));
	//----------------------------------------

	user->state=CLE_INPARSE;
	switch(cle_tokens.helpFlag){
	default:
	case CLE_EXEC_CMD:
		cle_cmdParse(userID,cle_tokens.rows);
		*outputLen=cle_outputLen;
		if((userDb[userID].uiStyle==CLE_UI_STYLE_STANDARD_IO)||
			(userDb[userID].uiStyle==CLE_UI_STYLE_TELNET))
		{
			cle_displayOutput(userID,cle_outputLen,cle_outputBuffer);
		}	
		switch(parseResult.type){					
		case CLE_RESULT_GROUP:						
			rn=1;
			break;
		case CLE_RESULT_EXECUTION:
		case CLE_RESULT_INCOMPLETE_COMMAND:
			if((userDb[userID].flag.funcExecResultError==0)&&
				(userDb[userID].outputMsg==CLE_WRITEOUTPUT_SUCCESS))
			{
				if(cle_outputLen==0 && userID != cle_returnUserID("script"))					
					rtlglue_printf("Success!!\n");
					
				rn=1;	
			}
			if(userDb[userID].flag.funcExecResultError==1) {
				uint32 i;
				int8 *errstr;
				if(userID == cle_returnUserID("script")) {
					cle_hisBuffer[0]=0;	
					for(i=0;i<cle_tokens.rows;i++)
					{
						strcat(cle_hisBuffer,cle_tokens.buffer[i]);
						strcat(cle_hisBuffer," ");
					}
					rtlglue_printf("Script: %s ",cle_hisBuffer);
				}
				errstr=cle_getParseResult(userID);		
				if(errstr[0]!=0)
					rtlglue_printf("Errno = %s\n", errstr);
				else
					rtlglue_printf("Error!!\n");
			}
			break;
		default:
			rn=0;
			break;
		}
		break;
	case CLE_HELP_CMD:
	case CLE_HELP_CMD_DESC:
	case CLE_HELP_CMD_DESC_USAGE:
		//CFLIU
		cle_helpParse(userID,cle_tokens.rows);
		if(parseResult.type==CLE_RESULT_INCOMPLETE_COMMAND)
			rn=1;
		break;
	case CLE_HELP_KEYWORD:
		//CFLIU
		cle_helpStringParse(userID,cle_tokens.rows);
		break;
	}
	user->state=CLE_OUTPARSE;

//	if (pthread_mutex_unlock(&cle_parserMutex) != 0)
//		return CLE_PARSER_ERROR;

	if(rn==1)
		return CLE_PARSER_SUCCESS;
	else
		return CLE_PARSER_ERROR;
}





void cle_cmdParse(uint32 userID,
				   uint32 argCnt)
{
	int32 cnt;

	if(!argCnt)
		return;

	if(cle_cmdEveryWhereCheck(userID,argCnt,0))
		return;
	if(cle_execCheck(userID,argCnt,0)){
		if(parseResult.type==CLE_RESULT_INCOMPLETE_COMMAND) //Specific to Cisco-like commands.
		{
			rtlglue_printf("Incomplete command\n");
		}
		return ;
	}
	if(parseResult.type==CLE_RESULT_INCOMPLETE_COMMAND){ //Specific to Cisco-like commands.
		rtlglue_printf("Incomplete command\n");
		return;
	}
	if(cle_groupCheck(userID,argCnt,0))
		return ;
	cnt=cle_cmdCount(userID,argCnt);
	if(cnt==0)
	{
		if(!userDb[userID].flag.tabMode)
			rtlglue_printf("Unknown command\n");
		return;
	}
	else if(cnt==1)
	{	
		if(cle_cmdEveryWhereCheck(userID,argCnt,1))
			return;
		if(cle_execCheck(userID,argCnt,1))
			return;
		if(cle_groupCheck(userID,argCnt,1))
			return ;	
	}
	else
	{
		if(!userDb[userID].flag.tabMode)
			rtlglue_printf("ambiguous command\n");
		return;
	}
			
	switch (parseResult.type){
	case CLE_RESULT_INCOMPLETE_COMMAND:
		rtlglue_printf("Incomplete command\n");
		break;
	case CLE_RESULT_ARGS_SCANTY:
	case CLE_RESULT_NOACTION:
		break;
	default:
		rtlglue_printf("Unknown command\n");
		break;
	}
}


void cle_helpParse(uint32 userID,
				   uint32 argCnt)
{
	int32 cnt;
	if(argCnt==0){
		cle_listTreeNodeDesp(userID);
		return;
	}

	if(cle_execHelp(userID,argCnt,0))
		return;

	if(cle_grpHelp(userID,argCnt,0,CLE_HELP_CMD))
		return;

	cnt=cle_cmdCount(userID,argCnt);
	if(cnt==0){
		rtlglue_printf("Unknown command\n");
		return;
	}else if(cnt==1){
		if(cle_execHelp(userID,argCnt,1))
			return;
		if(cle_grpHelp(userID,argCnt,1,CLE_HELP_CMD))
			return;
	}else{
		rtlglue_printf("ambiguous command\n");
		return;
	}
}

void cle_helpStringParse( uint32 userID, uint32 argCnt)
{
	int32 cnt;
	if(cle_execHelp(userID,argCnt,(argCnt==1)?1:0))
		return;
	if(cle_grpHelp(userID,argCnt,(argCnt==1)?1:0,CLE_HELP_KEYWORD))
		return;
	cnt=cle_cmdCount(userID,argCnt);
	if(cnt==0){
		rtlglue_printf("Unknown command\n");
		return;
	}else if(cnt==1){
		if(cle_execHelp(userID,argCnt,1))
			return;
		if(cle_grpHelp(userID,argCnt,1,CLE_HELP_CMD))
			return;
	}else{
		rtlglue_printf("ambiguous command\n");
		return;
	}


}

void cle_listTreeNodeDesp(uint32 userID)
{
	uint32 pos;
	cle_user_t* user;
	cle_grp_t*	grp;
	uint32 cnt;

	cnt=0;
	user=&(userDb[userID]);
	grp=user->stack[ user->hierarchy ];

	for(pos=0; pos<cmdEveryWhereNum ; pos++)
	{
		if(	cle_checkPriviledge(userID,cmdEveryWhere[pos].cmdFlags) )
		{
			switch(cle_tokens.helpFlag){
			default:
			case CLE_HELP_CMD_DESC:
				rtlglue_printf("%-20s %s\n",cmdEveryWhere[pos].cmdStr,cmdEveryWhere[pos].cmdDesc);
				break;
			case CLE_HELP_CMD:
				rtlglue_printf("%-20s",cmdEveryWhere[pos].cmdStr);
				if(MOD(cnt,3)==2)
					rtlglue_printf("\n");				
				cnt++;
				break;
			case CLE_HELP_CMD_DESC_USAGE:
				rtlglue_printf("%-20s %s\n",cmdEveryWhere[pos].cmdStr,cmdEveryWhere[pos].cmdDesc);
				rtlglue_printf("%-20s usage:%s\n","",cmdEveryWhere[pos].cmdUsage);
				break;
			}
		}		
	}
	
	for(pos=0; pos< grp->execNum ; pos++) 
	{
		if( cle_checkPriviledge(userID,grp->execPtr[pos].cmdFlags))	
		{		
			switch(cle_tokens.helpFlag){
			default:
			case CLE_HELP_CMD_DESC:
				rtlglue_printf("%-20s %s\n",grp->execPtr[pos].cmdStr,grp->execPtr[pos].cmdDesc);
				break;
			case CLE_HELP_CMD:
				rtlglue_printf("%-20s",grp->execPtr[pos].cmdStr);
				if(MOD(cnt,3)==2)
					rtlglue_printf("\n");
				cnt++;	
				break;
			case CLE_HELP_CMD_DESC_USAGE:
				rtlglue_printf("%-20s %s\n",grp->execPtr[pos].cmdStr,grp->execPtr[pos].cmdDesc);
				if(ISCLEARED(grp->execPtr[pos].cmdFlags, CLE_USECISCOCMDPARSER))
					rtlglue_printf("\t\t\tusage:%s\n",grp->execPtr[pos].cmdUsage);
				break;
			}
		}
	}

	for(pos = 0; pos<grp->grpNum; pos++)
	{
		if( cle_checkPriviledge(userID,grp->grpPtr[pos].grpFlags)&&ISCLEARED(grp->grpPtr[pos].grpFlags, CLE_STATEFULGROUP) )	
		{		
			switch(cle_tokens.helpFlag){
			default:
			case CLE_HELP_CMD_DESC:
			case CLE_HELP_CMD_DESC_USAGE:
				rtlglue_printf("%-20s %s\n",grp->grpPtr[pos].cmdStr,grp->grpPtr[pos].cmdDesc);
				break;
			case CLE_HELP_CMD:
				snprintf(cle_hisBuffer,cle_outputBufSize,"[%s]",grp->grpPtr[pos].cmdStr);
				rtlglue_printf("%-20s",cle_hisBuffer);
				if(MOD(cnt,3)==2)
					rtlglue_printf("\n");
				cnt++;	
				break;
			}
		}
	}

}

int32 cle_setNextGroupName(int8 *nextGroupName){
	assert(nextGroupName!=NULL);
	strncpy(cle_nextGroupName, nextGroupName, sizeof(cle_nextGroupName));
	return SUCCESS;
}

//returns new hierachy number
static int32 cle_changeCmdGroup(int8 *newGrpName, int32 bestMatch, int8 **saved, uint32 *saved_tokens){
	uint32 pos, match = 0, count=0;
	cle_user_t* user = NULL;
	cle_grp_t*	grp;
	uint32 hierarchy;
	int32 retval= FAILED;

	user=&(userDb[reent_getCleUserId()]);
	hierarchy=user->hierarchy;
	grp=user->stack[ hierarchy ];

	if(hierarchy+1 ==user->stackDepth){
		retval = hierarchy; //return the same value back to caller. This implies stack overflow.
		goto out;
	}

	for(pos = 0; pos<grp->grpNum; pos++){
		if(bestMatch?
			(strncmp(grp->grpPtr[pos].cmdStr, newGrpName,strlen(newGrpName)) == 0):
			(strcmp(grp->grpPtr[pos].cmdStr, newGrpName) == 0)
		  ){
			if( cle_checkPriviledge(reent_getCleUserId(),grp->grpPtr[pos].grpFlags)==0 )
				continue;

			match = pos;
			count++;
		}
	}
	if(count!=1)
		goto out;

	grp = &grp->grpPtr[match];

	if(ISSET(grp->grpFlags, CLE_STATEFULGROUP)&&saved){
		//dirStack is used to store accepted parameters in each stateful command group.
		//Format: Len: Length of this parameter (32 bit, if <0, succedding Value is a string and abs(Len) is its string length
		//		 Value: Content of parameter (4 byte aligned)
		//		 Level, Total: each 16 bits. 'Level' is the value of user->hierarchy in this command group. 'Total' is the total length (in bytes) of all Len, Value pairs in this command group level.
		//When pushing/pop elements into dirStack, always start from the end of dirStack[]
		//+---+----+---+----+-----+-----------+
		//|Len|Value|Len|Value| ... ...| Level, Total |
		//+---+----+---+----+-----+-----------+

		
		//If this cmd group is stateful, dump all accepted tokens in this group into user's dirStack.
		int8 *nextToken;
		int32 size, used =user->dirStackUsed, required, count=0;
		assert(saved!=NULL);

//#ifndef NDEBUG
//		if(used)
//			assert(used && (used&&0x3==0));//make sure it is 4 byte aligned
//#endif
		if(saved_tokens)
			*saved_tokens=0;
		while(ciscoCmdGetNextToken(&nextToken,& size,saved)!=FAILED){
			uint32 copy=(!size)?(int32)strlen(nextToken): size;
			//If this assertion fails, it means Not enough space in user's dirStack....we need to enlarge CLE_MAXDIRSTACKDEPTH. 
			required = ROUNDUP4(copy);
			assert(CLE_MAXDIRSTACKDEPTH - used >=required );
			*((int32 *)&user->dirStack[used+count]) = (!size)? -copy: copy;
			count+=sizeof(uint32);
			memcpy(&user->dirStack[used+count], nextToken, copy); //If nextToken is a string pointer, NULL char IS copied..
			if(!size)//this is a string. Add trailing null character.
				user->dirStack[used+count+copy]='\0';
			count+=required;
			if(saved_tokens)
				*saved_tokens+=1;
		}
		if(count>0){
			uint16 *p = (uint16 *)&user->dirStack[used+count];
			*p= user->hierarchy+1;  //Level
			p++;
			*p= count;  //Total length
			user->dirStackUsed +=(count+sizeof(uint32));
			ciscoCmdParserFlushTmpBuffer();
		}
	}

	if(user->flag.tabMode&&ISCLEARED(grp->grpFlags, CLE_STATEFULGROUP)){
		strcpy(cle_tokens.buffer[cle_tokens.rowPtr],grp->cmdStr);
		//cle_tabModeReturn(reent_getCleUserId());		
		cle_tabModeReturn(0);	
	}
	
	user->hierarchy =  ++hierarchy;
	user->stack[hierarchy] = &(user->stack[hierarchy-1]->grpPtr[match]);
	retval = hierarchy; //returns new hierarchy back to caller.
out:
	return retval; //new group not found
}


//Return to indicated directory hierarchy number.
//If 'previousHierarchy' is -N, go back N levels
static int32 cle_exitCmdGroup(int32 previousHierarchy){
	cle_user_t* user;
	cle_grp_t*  grp;
	uint32 curHierarchy;
	uint32 used, i, max;
	uint16 *level, *totalLen;
	//user=&(userDb[reent_getCleUserId()]);
	user=&(userDb[0]);
	curHierarchy = user->hierarchy;
	if(previousHierarchy<0)
		max = (curHierarchy> -previousHierarchy)? curHierarchy: -previousHierarchy;
	else if(curHierarchy > previousHierarchy)
		max = curHierarchy - previousHierarchy; //max number of groups to backoff
	else
		return FAILED;
	
	used = user->dirStackUsed; //byte count of dirStack[] used now.
	for(i=0; i < max; i++, curHierarchy--){
		assert(curHierarchy>0);
		grp=user->stack[curHierarchy];
		if(ISSET(grp->grpFlags, CLE_STATEFULGROUP)&&used>=sizeof(uint32)){
			//If this cmd group is stateful, pop all accepted tokens out of user's dirStack.
			totalLen = level = (uint16 *)&user->dirStack[used-sizeof(uint32)];
			assert(curHierarchy==(uint32)*level);
			totalLen++;
			assert((*totalLen+sizeof(uint32))<=used);
			used-= (*totalLen+sizeof(uint32));
		}
	}
	user->hierarchy = previousHierarchy;
	user->dirStackUsed = used;
	return SUCCESS;
}


int32 cle_getNextCmdToken(int8 **nextToken, int32 *size, int8 **savedData){
	cle_user_t* user=&(userDb[reent_getCleUserId()]);
	uint32 used=user->dirStackUsed;
	assert(nextToken && size);
	assert(savedData!=NULL);

	if(!used){
ciscoParserToken:
		return ciscoCmdGetNextToken(nextToken,size, savedData);
	}else{
		int8 *head = (int8 *)&user->dirStack[0];
		int8 *last, *this;
		uint16 *total;

		last = (cle_tokenGetCount)? (*savedData): head;
		cle_tokenGetCount++;

nextLevel:
		this =(int8 *)&user->dirStack[used-sizeof(uint32)];
		assert( ((memaddr)this & 0x1) ==0); //make sure it is 2 byte aligned.
		total = (uint16 *)this;
		total++;
		if(!last)
			last = this - (*total);
		assert(used>=3*sizeof(uint32));
		assert(last >= head &&last<= this);
		if(last<this){
			int32 tokenSize;
			int32 absTokenSize;
			assert(IS4BYTEALIGNED((memaddr)last));
			tokenSize=I32_value(last);
			absTokenSize= (tokenSize>0)? tokenSize: -tokenSize;
			*size = (tokenSize>0)? tokenSize: 0;
			*nextToken = last+sizeof(uint32);
			if(tokenSize<0)
				*savedData = last+ROUNDUP4(absTokenSize)+sizeof(uint32);				
				//*savedData = last+(((absTokenSize>>2)+1)<<2)+sizeof(uint32);				
			else
				*savedData = last+absTokenSize+sizeof(uint32);
			user->dirStackUsed = used;
		}else{
			assert(last==this);
			used-=(*total+sizeof(uint32));
			user->dirStackUsed = used;
			if(used!=0){
				last = NULL;
				goto nextLevel;
			}
			*savedData=cle_firstCiscoCmdToken;
			goto ciscoParserToken;
		}
	}
	return SUCCESS;
}


static int32 cle_ciscoCmdProcesser(int32 argc, cle_exec_t *thisCmd, int32 mode){
	int8 *saved=NULL;
	cle_exec_t *newCmd;
	cle_user_t* user;	
	int32 count=0;
	uint32 tokens_saved;
	uint32 newHierachy;	
	int8 *token;
	int8 **argv;  
	int32 retval = FAILED;
	uint32 uiStyle;
	int32 previousHierarchy;
	uiStyle = cle_getUiStyle(reent_getCleUserId());
	assert(ISSET(thisCmd->cmdFlags, CLE_USECISCOCMDPARSER));

	argc--; //Drop first token since we already know which command to process via 'thisCmd'
	argv = &(cle_tokens.buffer[cle_tokens.rowPtr+1]);
	newCmd = thisCmd;
	user=&(userDb[reent_getCleUserId()]);
	previousHierarchy = user->nowHierarchy;

parse_again:
	tokens_saved=0;
	//Parse command argument and check validity.
	count=ciscoCmdParser(argc, argv, &newCmd,&saved,&token);
	if(cle_nextGroupName[0]){
		//change to next level command group
		uint32 oldHierarchy;
		int8 *first = (count<0)? NULL: saved;
		oldHierarchy = user->hierarchy;
		newHierachy= cle_changeCmdGroup(cle_nextGroupName, FALSE, &first, &tokens_saved);

		if(oldHierarchy+1==newHierachy){
			if(!mode)
				count -= tokens_saved;
			//group changed to next level...
			parseResult.type = CLE_RESULT_GROUP; //this may be overrided later if operation not successful
			cle_nextGroupName[0]='\0'; //reset
			if(!token){
				//No more succeding tokens.
				user->nowHierarchy = user->hierarchy;
			}
			retval = SUCCESS;
		}
	}
	else
		newHierachy = 0; /* just for initilize a value. */

	if(newCmd!=thisCmd&&newCmd){
		//If a new command is specified, switch to another new command.
		int32 count_abs = (count>0) ? count: -count;
		if(token){
			argc = argc - count_abs;
			if(tokens_saved && !mode &&previousHierarchy < newHierachy){ 
				argc--; //we just changed directory and about to execute a new command, skip next token.
				assert(argc>=0);
				argv =&argv[count_abs+1]; //First token to parse from.
			}else
				argv = &argv[count_abs]; //First token to parse from.
		}else{
			argc = 0;
			argv = NULL;//strcpy(working, "");
		}
		ciscoCmdParserFree();
		cle_tokenGetCount=0;
		cle_firstCiscoCmdToken=NULL;
		thisCmd=newCmd;
		//Now reparse the new command with new argc, argv.
		goto parse_again;
	}


	if(count>0){
		//some arguments accepted
accepted:
		//count may <=0 if jumped to here via 'accepted' label
		if(!mode){
			if(thisCmd->execFun){
				cle_firstCiscoCmdToken = saved;
				retval = thisCmd->execFun(0, count, &saved);
				if(parseResult.type==CLE_RESULT_GROUP && retval==SUCCESS){
					//we have changed group, now go back to where we started..
					cle_exitCmdGroup(previousHierarchy);
					user->nowHierarchy = previousHierarchy;
				}
				parseResult.type = CLE_RESULT_EXECUTION;
				IntToAscii(retval,parseResult.msg);//save error number
			}
		}else{
			int8 *dummyToken;
			int32 size;
			//Help mode. But command arguments given alone is acceptable. No more tokens required
			if(count){
				while((count--) >1)
					ciscoCmdGetNextToken(&dummyToken, &size, &saved);
				assert(saved!=NULL);
				ciscoCmdParserGetCandidate(saved);
			}
			token = NULL;
			goto showHelp;
		}		
	}else if(count==0){
		//No tokens accepted...
		if(!mode){
			if(!saved){
				if(!token)
					goto accepted;	//Accepted without any arguments.
				else{
					ciscoCmdParserGetAllCandidates();
					token = NULL;
				}
				goto showHelp;
			}
		}else{
			if(parseResult.type==CLE_RESULT_GROUP ){
				cle_exitCmdGroup(previousHierarchy);
				user->nowHierarchy = previousHierarchy;
			}
			parseResult.type = CLE_RESULT_INCOMPLETE_COMMAND;
			rtlglue_printf(CISCOCMD_HELPMSG_FORMAT, CISCOCMD_ACCEPT_SYMBOL, CISCOCMD_ACCEPT_STRING);
		}
	}else{
		//	abs(count) means the number of arguments accepted. Get help for the count+1-th argument 
		int32 total;
showHelp:
		assert(thisCmd!=NULL);
		total = ciscoCmdParserGetCandidateTokenList( tokenEnum, CLE_MAXENUMENTRIES);
		if(total){
			if(argc>0){ 
				int32 i,j;
				int8 *match = NULL;
				if(token){
					for(i=0, j=0; i < total; i++)	
						if(strncmp(tokenEnum[i].tokenName, argv[-count-1], strlen(argv[-count-1])) == 0){
							match = tokenEnum[i].tokenName; j++;
						}
					if(j==1){
						//Found the best match
						strcpy(argv[-count-1], match);
						if(mode==CISCOCMDCOMPLETION){
							cle_tabModeReturn(reent_getCleUserId());
							parseResult.type = CLE_RESULT_NOACTION;
							goto out;
						}else if(ISSET(uiStyle ,CLE_UI_STYLE_TERMINAL)){
							rtlglue_printf("%dth token best matched for '%s'\n", -count, match);
							goto parse_again;
						}
					}else if (j>1){
						//More than one token match.
						for(i=0; i < total; i++)	
							if(strncmp(tokenEnum[i].tokenName, argv[-count-1], strlen(argv[-count-1])) != 0)
								 tokenEnum[i].tokenName = NULL;
					}else {
						total = ciscoCmdParserGetCandidateTokenList( tokenEnum, CLE_MAXENUMENTRIES);
						assert(total>0);
						//Do substring pattern match
						for(i=0, j=0; i < total; i++)	
							if(strstr(tokenEnum[i].tokenName, argv[-count-1])) {
								match = tokenEnum[i].tokenName; j++;
							}
						if(j==1){
							//Found the best match
							strcpy(argv[-count-1], match);
							if(mode==CISCOCMDCOMPLETION){
								cle_tabModeReturn(reent_getCleUserId());
								parseResult.type = CLE_RESULT_NOACTION;
								goto out;
							}else if(ISSET(uiStyle ,CLE_UI_STYLE_TERMINAL)&&mode!=CISCOCMDCOMPLETION)
								rtlglue_printf("\n%dth token best matched for '%s'\n", -count, match);
							goto parse_again;
						}else if (j>1){
							//More than one token match.
							for(i=0; i < total; i++)	
								if(strncmp(tokenEnum[i].tokenName, argv[-count-1], strlen(argv[-count-1])) != 0)
									 tokenEnum[i].tokenName = NULL;
						}else if(ISSET(uiStyle ,CLE_UI_STYLE_TERMINAL)){					
							//Nothing matched.
							rtlglue_printf("\nError on %dth token '%s'\n\n", -count, token);
						}
					}
				}
			}
			if(mode!=CISCOCMDCOMPLETION){
				if(thisCmd->cmdUsage)
					ciscoCmdPrintSynopsis(NULL, thisCmd->cmdUsage);
				if(total>0)
					ciscoCmdParserShowHelp(tokenEnum, total);
			}
			//If we have changed group, revoke to original group
			if(parseResult.type==CLE_RESULT_GROUP ){
				cle_exitCmdGroup(previousHierarchy);
				user->nowHierarchy = previousHierarchy;
			}
			parseResult.type = CLE_RESULT_INCOMPLETE_COMMAND;
		}else if(mode){
			if(parseResult.type==CLE_RESULT_GROUP ){
				cle_exitCmdGroup(previousHierarchy);
				user->nowHierarchy = previousHierarchy;
			}
			parseResult.type = CLE_RESULT_INCOMPLETE_COMMAND;
		}
	}

out:	
	ciscoCmdParserFree();
	cle_tokenGetCount=0;
	cle_firstCiscoCmdToken=NULL;	
	if(!mode)
		return retval;
	//parseResult.type = CLE_RESULT_EXECUTION;
	return SUCCESS;
}
		

static int32 cle_cmdEveryWhereCheck(uint32 userID,
							uint32 argCnt,
							int32 bestMatch)
{
	uint32 pos;
	cle_user_t* user;

	user=&(userDb[userID]);

	for(pos=0; pos<cmdEveryWhereNum ; pos++)
	{
		if(	bestMatch?
					(strncmp(cmdEveryWhere[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr],
						strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0):
					(strcmp(cmdEveryWhere[pos].cmdStr, cle_tokens.buffer[cle_tokens.rowPtr]) == 0)
		)
		{
			if(ISSET(cmdEveryWhere[pos].cmdFlags, CLE_USECISCOCMDPARSER)){
				int32 mode = 0, ret;
				if(userDb[userID].flag.tabMode){
					if(bestMatch)
						goto tabBestMatch;
					mode = CISCOCMDCOMPLETION;
				}
				ret=cle_ciscoCmdProcesser(argCnt, &cmdEveryWhere[pos], mode);
				if(ret!=SUCCESS){
					if(parseResult.type != CLE_RESULT_INCOMPLETE_COMMAND)
						user->flag.funcExecResultError=1;
					IntToAscii(ret,parseResult.msg);//save error number
				}
			}else{

				//for tab key-------------------------------------------------------------
				if(userDb[userID].flag.tabMode)
				{
tabBestMatch:
					strcpy(cle_tokens.buffer[cle_tokens.rowPtr],cmdEveryWhere[pos].cmdStr);
					cle_tabModeReturn(userID);					
					parseResult.type = CLE_RESULT_NOACTION;
					return 1;
				}
				//------------------------------------------------------------------------
				if(cmdEveryWhere[pos].__minCmdArgs<=argCnt )
				{
					int32 ret=cmdEveryWhere[pos].execFun(userID, argCnt, &(cle_tokens.buffer[cle_tokens.rowPtr]));

					if ( ret== SUCCESS)
						parseResult.type = CLE_RESULT_EXECUTION;
					else
						IntToAscii(ret,parseResult.msg);//save error number
				}
				else
				{
					if(bestMatch)
					{
						rtlglue_printf("%-20s usage:%s\n",cmdEveryWhere[pos].cmdStr,cmdEveryWhere[pos].cmdUsage);
						parseResult.type = CLE_RESULT_ARGS_SCANTY;
					}
					return 0;
				}
			}
			return 1;
		}
	}
	return 0;
}

static int32 cle_execCheck(uint32 userID, 
			   uint32 argCnt,
			   int32 bestMatch)
{
	uint32 pos;
	cle_user_t* user;
	cle_grp_t*	grp;

	user=&(userDb[userID]);
	grp=user->stack[ user->hierarchy ];

	for(pos=0; pos< grp->execNum ; pos++) 
	{
		if( cle_checkPriviledge(userID,grp->execPtr[pos].cmdFlags) &&	
			(	bestMatch?
					(strncmp(grp->execPtr[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr],
						strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0):
					(strcmp(grp->execPtr[pos].cmdStr, cle_tokens.buffer[cle_tokens.rowPtr]) == 0)
			)
		  )
		{
			if(ISSET(grp->execPtr[pos].cmdFlags, CLE_USECISCOCMDPARSER)){
				int32 mode = 0, ret;
				if(userDb[userID].flag.tabMode){
					if(bestMatch)
						goto tabBestMatch;
					mode = CISCOCMDCOMPLETION;
				}
				ret = cle_ciscoCmdProcesser(argCnt, &grp->execPtr[pos], mode);
				if(ret!=SUCCESS){
					if(parseResult.type != CLE_RESULT_INCOMPLETE_COMMAND)
						user->flag.funcExecResultError=1;
						IntToAscii(ret,parseResult.msg);//save error number				
				}
			}else{

				//for tab key-------------------------------------------------------------
				if(userDb[userID].flag.tabMode)
				{
tabBestMatch:
					strcpy(cle_tokens.buffer[cle_tokens.rowPtr],grp->execPtr[pos].cmdStr);
					cle_tabModeReturn(userID);					
					parseResult.type = CLE_RESULT_NOACTION;
					return 1;
				}
				//------------------------------------------------------------------------
				
				if(grp->execPtr[pos].__minCmdArgs<=argCnt )
				{
					int32 ret=grp->execPtr[pos].execFun(userID, argCnt, &(cle_tokens.buffer[cle_tokens.rowPtr]));
					if(ret!=SUCCESS){
						user->flag.funcExecResultError=1;
						IntToAscii(ret,parseResult.msg);//save error number
					}
					//user->displayFun=grp->execPtr[pos].displayFun;

					parseResult.type = CLE_RESULT_EXECUTION;

				}
				else
				{
					if(bestMatch)
					{
						rtlglue_printf("%-20s usage:%s\n",grp->execPtr[pos].cmdStr,grp->execPtr[pos].cmdUsage);
						parseResult.type = CLE_RESULT_ARGS_SCANTY;
					}
					return 0;
				}
			}			
			return 1;
		}
	}
	return 0;
}


static int32 cle_groupCheck(uint32 userID, 
							uint32 argCnt,
							int32 bestMatch)
{
	cle_user_t* user;
	int32 hierarchy;
	int32 retval;

	user=&(userDb[userID]);
	hierarchy = user->hierarchy;

	retval = cle_changeCmdGroup(cle_tokens.buffer[cle_tokens.rowPtr], bestMatch, NULL, NULL);
	if(retval == hierarchy+1){
		//New group found and entered.
		if(user->flag.tabMode)
			parseResult.type = CLE_RESULT_NOACTION;
		else
			parseResult.type = CLE_RESULT_GROUP;
		cle_tokens.rowPtr++;
		assert(argCnt!=0);
		if(argCnt > 1){
			cle_cmdParse(userID,argCnt-1);
			switch(parseResult.type){					
			case CLE_RESULT_GROUP:				
			case CLE_RESULT_EXECUTION:
				user->hierarchy = user->nowHierarchy;
				break;
			case CLE_RESULT_NOACTION:
				user->nowHierarchy = hierarchy;
				break;
			default:
				cle_exitCmdGroup(hierarchy);
				user->nowHierarchy = hierarchy;
				break;
			}
		}else{
			if(!user->flag.tabMode)
				user->nowHierarchy =hierarchy+1;
		}
		return 1;
	}else if(retval == hierarchy){
		//User's stack overflowed. Stay in the same group
		parseResult.type = CLE_RESULT_STACK_OVERFLOW;
		return 1;
	}else //if(retval == FAILED) //can't find the new group 
		return 0;
	
}


static int32 cle_execHelp( uint32 userID, 
						   uint32 argCnt,
						   int32 bestMatch)
{
	uint32 pos;
	cle_user_t* user;
	cle_grp_t*	grp;
	int32 find=0;

	user=&(userDb[userID]);
	grp=user->stack[ user->hierarchy ];

	for(pos=0; pos<cmdEveryWhereNum ; pos++)
	{
		if(	bestMatch?
					(strncmp(cmdEveryWhere[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr],
						strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0):
					(strcmp(cmdEveryWhere[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr])== 0)
		)
		{
			parseResult.type = CLE_RESULT_EXEC_EXIST;
			rtlglue_printf("%-20s %s\n",cmdEveryWhere[pos].cmdStr,cmdEveryWhere[pos].cmdDesc);
			rtlglue_printf("%20s usage: %s\n","",cmdEveryWhere[pos].cmdUsage);
			if(!bestMatch) 
				return 1;
			else 
				find=1;
		}
	}

	for(pos=0; pos< grp->execNum ; pos++) 
	{
		if( cle_checkPriviledge(userID,grp->execPtr[pos].cmdFlags) &&	
			(	bestMatch?
					(strncmp(grp->execPtr[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr],
						strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0):
					(strcmp(grp->execPtr[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr])== 0)
			)
		  )
		{
			parseResult.type = CLE_RESULT_EXEC_EXIST;
			if(ISSET(grp->execPtr[pos].cmdFlags, CLE_USECISCOCMDPARSER)){
				int32 ret=cle_ciscoCmdProcesser(argCnt, &grp->execPtr[pos], CISCOCMDHELP);
				if(ret!=SUCCESS){
					IntToAscii(ret,parseResult.msg);//save error number
					return 0;
				}
			}else{
				rtlglue_printf("%-20s %s\n",grp->execPtr[pos].cmdStr,grp->execPtr[pos].cmdDesc);
				rtlglue_printf("%20s usage: %s\n","",grp->execPtr[pos].cmdUsage);
			}
			if(!bestMatch) 
				return 1;
			else 
				find=1;
		}
	}
	return find;
}

static int32 cle_grpHelp(uint32 userID, 
			   uint32 argCnt,
			   int32 bestMatch,
			   int32 mode)
{
	uint32 pos;
	cle_user_t* user;
	cle_grp_t*	grp;
	int find=0;

	user=&(userDb[userID]);
	grp=user->stack[ user->hierarchy ];
	
	for(pos = 0; pos<grp->grpNum; pos++)
	{
		if( cle_checkPriviledge(userID,grp->grpPtr[pos].grpFlags) &&	
			(bestMatch?
				(strncmp(grp->grpPtr[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr],
						strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0):
				(strcmp(grp->grpPtr[pos].cmdStr, 
						cle_tokens.buffer[cle_tokens.rowPtr])== 0)
			)
		  )
		{
			user->hierarchy++;
			if(user->hierarchy ==user->stackDepth)
			{
				parseResult.type = CLE_RESULT_STACK_OVERFLOW;
				user->hierarchy--;
			}
			else
			{
				if(mode==CLE_HELP_CMD)
				{
					cle_tokens.rowPtr++;
					user->stack[user->hierarchy] = &(user->stack[user->hierarchy-1]->grpPtr[pos]);
					if(argCnt == 1)
					{
						cle_listTreeNodeDesp(userID);
						parseResult.type = CLE_RESULT_GROUP_EXIST;
						return 1;
					}					
					else
					{
						cle_helpParse(userID,argCnt-1);

						switch(parseResult.type){					
						case CLE_RESULT_GROUP_EXIST:
						case CLE_RESULT_EXEC_EXIST:
						case CLE_RESULT_INCOMPLETE_COMMAND:
							return 1;
						default:
							user->hierarchy--;
							break;
						}
					}
				}
				else//mode==CLE_HELP_STRING_CMD
				{
					if(argCnt == 1)
					{
						rtlglue_printf("%-20s %s\n",grp->grpPtr[pos].cmdStr,grp->grpPtr[pos].cmdDesc);
						find=1;
					}
					else
					{
						cle_tokens.rowPtr++;
						user->stack[user->hierarchy] = &(user->stack[user->hierarchy-1]->grpPtr[pos]);
						cle_helpStringParse(userID,argCnt-1);
					}
				}
			}
		}
	}
	if(find==1)
	{
		parseResult.type = CLE_RESULT_GROUP_EXIST;
		return 1;
	}
	return 0;
}

int32 cle_writeOutput(uint32 userID,int8* buf,uint32 bufLen)
{
//	cle_user_t* user;

//	user=&(userDb[userID]);
	if(cle_outputLen+bufLen>=cle_outputBufSize)
		return CLE_WRITEOUTPUT_ERROR;	
	memcpy(cle_outputBuffer+cle_outputLen,buf,bufLen);
	cle_outputLen+=bufLen;
	return CLE_WRITEOUTPUT_SUCCESS;
}

int32 cle_writeOutputHeader(uint32 userID,uint8 format,uint16 entryCount,uint8 fieldCount)
{
	cle_rnBufHeader_t rnHeader;

	//reset item count
	userDb[userID].itemCount=0;
	userDb[userID].outputMsg=CLE_WRITEOUTPUT_SUCCESS;

	if(entryCount==0)//enable auto item count function		
		userDb[userID].flag.useItemCount=1;
	else//disable auto item count function
		userDb[userID].flag.useItemCount=0;
		
	memset(&rnHeader,0,sizeof(cle_rnBufHeader_t));	
	if(sizeof(cle_rnBufHeader_t)>=cle_outputBufSize)
	{
		userDb[userID].outputMsg=CLE_WRITEOUTPUT_ERROR;
		return CLE_WRITEOUTPUT_ERROR;	
	}
	rnHeader.version=1;
	rnHeader.format=format;
	rnHeader.len=sizeof(cle_rnBufHeader_t);
	rnHeader.entryCount=entryCount;
	if( (format&(uint8)0x01) == CLE_RNBUF_TABLE )
		rnHeader.fieldCount=fieldCount;	
	else
		rnHeader.fieldCount=1;	
	
	//recorded fieldCount
	userDb[userID].fieldCount=rnHeader.fieldCount;

	memcpy(cle_outputBuffer,&rnHeader,sizeof(cle_rnBufHeader_t));
	cle_outputLen=sizeof(cle_rnBufHeader_t);
	return CLE_WRITEOUTPUT_SUCCESS;
}

int32 cle_setOutputLen(uint32 userID)
{
	uint16 tmp;

	if(cle_outputBufSize<=4)
	{
		userDb[userID].outputMsg=CLE_WRITEOUTPUT_ERROR;	
		return CLE_WRITEOUTPUT_ERROR;	
	}
	tmp=(uint16)cle_outputLen;
	memcpy(cle_outputBuffer+2,&tmp,sizeof(uint16));

	//reset entry count
	if((userDb[userID].flag.useItemCount)&&(userDb[userID].fieldCount!=0))
		cle_resetEntryCount(userID,userDb[userID].itemCount/userDb[userID].fieldCount);

	return CLE_WRITEOUTPUT_SUCCESS;
}

int32 cle_setUserAccessLevel(uint32 userID,uint16 accessLevel)
{
	if(accessLevel&CLE_MODE_PRIVILEGE && !userDb[userID].flag.priviledgeMode)
		return FAILED;
	if(accessLevel&CLE_MODE_SUPERENG && !userDb[userID].flag.superengMode)
		return FAILED;
	userDb[userID].accessLevel=accessLevel;
	return SUCCESS;
}

void cle_getUserAccessLevel(uint32 userID,uint16 *accessLevel)
{
	*accessLevel = userDb[userID].accessLevel;
	return;
}

int32 cle_resetEntryCount(uint32 userID,uint16 entryCount)
{
	uint16 tmp;

	if(cle_outputBufSize<=6)
	{
		userDb[userID].outputMsg=CLE_WRITEOUTPUT_ERROR;	
		return CLE_WRITEOUTPUT_ERROR;	
	}
	tmp=(uint16)entryCount;
	memcpy(cle_outputBuffer+4,&tmp,sizeof(uint16));
	return CLE_WRITEOUTPUT_SUCCESS;
}

int32 cle_addSubItemHeadToOutput(uint32 userID,uint32 entryCnt,uint32 fieldCnt)
{
	cle_rnBufSubitem_t head;
	
	if((cle_outputLen+3)>=cle_outputBufSize)
	{
		userDb[userID].outputMsg=CLE_WRITEOUTPUT_ERROR;	
		return CLE_WRITEOUTPUT_ERROR;
	}
	head.type=CLE_RNBUF_SUBITEM;
	head.entryC=(uint8)entryCnt;
	head.fieldC=(uint8)fieldCnt;	
	memcpy(cle_outputBuffer+cle_outputLen,&head,3);	
	cle_outputLen+=3;
	return CLE_WRITEOUTPUT_SUCCESS;
}

int32 cle_addItemToOutput(uint32 userID,uint8 type,int8* buf,uint32 bufLen)
{
	cle_rnBufItem_t item;
	uint16 message;
	uint32 ip_tmp;

	item.type=type;
	switch(type){
	case CLE_RNBUF_UINT32:
	case CLE_RNBUF_INT32:
		item.dataLen=4;
		break;
	case CLE_RNBUF_IPADDRESS:		
	case CLE_RNBUF_IPADDRESS_HOSTORDER:
		memcpy(&ip_tmp,buf,sizeof(uint32));
		item.dataLen=4;
		if(type==CLE_RNBUF_IPADDRESS_HOSTORDER)
		{
			ip_tmp=htonl(ip_tmp);
			item.type=CLE_RNBUF_IPADDRESS;
		}
		break;
	case CLE_RNBUF_DISPLAYSTR:
	case CLE_RNBUF_NUMERICSTR:
		item.dataLen=(uint8)bufLen;
		break;
	case CLE_RNBUF_MACADDRESS:
		item.dataLen=6;
		break;
	case CLE_RNBUF_UINT16:
	case CLE_RNBUF_INT16:
	case CLE_RNBUF_RESULT:
		item.dataLen=2;
		break;
	default:			
		item.dataLen=0;
		break;
	}	
	
	if((cle_outputLen+sizeof(cle_rnBufItem_t)+item.dataLen)>=cle_outputBufSize)
	{
		userDb[userID].outputMsg=CLE_WRITEOUTPUT_ERROR;	
		return CLE_WRITEOUTPUT_ERROR;
	}
	
	//counting items
	userDb[userID].itemCount++;

	memcpy(cle_outputBuffer+cle_outputLen,&item,sizeof(cle_rnBufItem_t));
	cle_outputLen+=sizeof(cle_rnBufItem_t);


	if(item.type==CLE_RNBUF_RESULT)
	{
		message=(uint16)bufLen;
		memcpy(cle_outputBuffer+cle_outputLen,&message,item.dataLen);
	}
	else if(item.type==CLE_RNBUF_IPADDRESS)
	{
		memcpy(cle_outputBuffer+cle_outputLen,&ip_tmp,item.dataLen);
	}
	else
		memcpy(cle_outputBuffer+cle_outputLen,buf,item.dataLen);
	cle_outputLen+=item.dataLen;
	
	return CLE_WRITEOUTPUT_SUCCESS;
}

int32 cle_addStringItemToOutput(uint32 userID,int8* fmt,...)
{
	va_list     args;

	va_start(args,fmt);
	cleVsnPrintf(cle_hisBuffer,cle_outputBufSize,fmt, args);
	va_end(args);
	
	cle_addItemToOutput(userID,CLE_RNBUF_DISPLAYSTR,cle_hisBuffer,strlen(cle_hisBuffer));
	return CLE_WRITEOUTPUT_SUCCESS;
}

void cle_addRowToOutput(uint32 userID,int8* fmt,...)
{
    va_list     args;
    uint32 i;
    int8   fch;         /* format char */
    uint32 isUnsigned;
    uint32 fieldC;
	union {
		int8*  d_buf;
		int8* d_ip;
		uint32 d_uint32;
		int32  d_int32;
		uint16 d_uint16;
		int16  d_int16;
	} value;

	//if this sub-function is the first time called by API:
	//must calculate the field count	
	if(cle_outputLen==0)
	{
		fieldC=0;
		for(i=0;i<strlen(fmt);i++)
		{
			if(fmt[i]=='%')
				fieldC++;
		}
		if(fieldC==0)
			return;
		cle_writeOutputHeader(userID,CLE_RNBUF_TABLE,0,fieldC);
	}		

	fieldC=0;

    va_start(args,fmt);
	
    while ( (fch = *fmt++) != 0  )
    {        
        if (fch == '%')
        {
			if(fieldC>=userDb[userID].fieldCount)
				continue;
            isUnsigned = 0;       /* default signed */            
            while ((*fmt >= '0') && (*fmt <= '9'))
                fmt++;
cle_ismod:
            fch = *fmt++;
            switch( fch ){
            case 'u': //unsigned flag
                isUnsigned = 1;
                goto cle_ismod;         /* modifier character */
                
            case 'd': //type = int32 or uint32
                if(isUnsigned)
                {
                    value.d_uint32 = (uint32)va_arg(args, uint32); /* integer argument */
                    cle_addItemToOutput(userID,CLE_RNBUF_UINT32,(int8*)&value.d_uint32,0);
                }
                else
                {
                    value.d_int32 = (int32)va_arg(args, int32);    /* integer argument */
                    cle_addItemToOutput(userID,CLE_RNBUF_INT32,(int8*)&value.d_int32,0);
                }
                fieldC++;
                break;
            case 'h': //type = int16 or uint16
                if(isUnsigned)
                {
                    value.d_uint16 = (uint16)va_arg(args, int); /* integer argument */
                    cle_addItemToOutput(userID,CLE_RNBUF_UINT16,(int8*)&value.d_uint16,0);
                }
                else
                {
                    value.d_int16 = (int16)va_arg(args, int);    /* integer argument */
                    cle_addItemToOutput(userID,CLE_RNBUF_INT16,(int8*)&value.d_int16,0);
                }
                fieldC++;
                break;                                
            case 'q': //type = ip address (host order)
                value.d_ip=(int8*) va_arg(args, uint8*);
                cle_addItemToOutput(userID,CLE_RNBUF_IPADDRESS_HOSTORDER,value.d_ip,0);
                fieldC++;
                break;                
            case 'p': //type = ip address (network order)
                value.d_ip=(int8*) va_arg(args, uint8*);
                cle_addItemToOutput(userID,CLE_RNBUF_IPADDRESS,value.d_ip,0);
                fieldC++;
                break;
            case 'm': //type = ip address (network order)
                value.d_ip=(int8*) va_arg(args, uint8*);
                cle_addItemToOutput(userID,CLE_RNBUF_MACADDRESS,value.d_ip,0);
                fieldC++;
                break;
            case 's': //type = display string
                value.d_buf = va_arg(args,int8 *);
                if (!value.d_buf)
                    value.d_buf = "NULL";             /* null pointer passed for %s */
                cle_addItemToOutput(userID,CLE_RNBUF_DISPLAYSTR,value.d_buf,strlen(value.d_buf));
                fieldC++;
                break;
            case 'n': //type = numeric string  
                value.d_buf = va_arg(args,int8 *);
                if (!value.d_buf)
                    value.d_buf = "NULL";             /* null pointer passed for %s */
                cle_addItemToOutput(userID,CLE_RNBUF_NUMERICSTR,value.d_buf,strlen(value.d_buf));
                fieldC++;
                break;
            default:
                break;
            }
        }
        else if(fch=='\n')
        {
           	cle_endRowToOutput(userID);
            return;
        }
    }

    for(i=fieldC;i<userDb[userID].fieldCount;i++)
	{
    	cle_addItemToOutput(userID,CLE_RNBUF_DISPLAYSTR,"",0);	
	}
    return;
}

void cle_endRowToOutput(uint32 userID)
{
	cle_setOutputLen(userID);
}

int32 cle_printToAll(int8* fmt,...)
{
	cle_user_t* user;
	va_list     args;
	uint32 i;
	uint32 userID;

	for(i=0;i<cle_userNum;i++)
	{
		userID=cle_userIdTable[i].userID;
		if( (userID!=CLE_userIdError) && (userDb[userID].initComplete==TRUE) )
		{	
			va_start(args,fmt);
			user=&(userDb[ userID ]);
//			vsprintf(user->hisBuffer, fmt, args);
			cleVsnPrintf(cle_hisBuffer,cle_outputBufSize,fmt, args);
			va_end(args);

			/* yjlou note: Is it a bug ?
			 * The first argument of cle_printf(), said printf(), should be 'char*', not 'uint32'.
			 * For compilation, I cast to 'void*'.
			 */
			rtlglue_printf((void*)user->uiStyle,cle_hisBuffer);
		}
	}
	return 0;
}


int32 cle_exitCheck(int8* inputStr)
{
	int32 i;

	for(i=0;i<cle_exitKeywordNum;i++)
		if(strcmp(inputStr,cle_exitKeyword[i])==0)
		return 1;
	return 0;
}

int32 cle_specialCheck(uint32 userID, int8 * inputStr)
{
	int32 i;
	cle_user_t* user;

	user=&(userDb[userID]);

	for(i=0;i<cle_leaveGrpKeywordNum;i++)
		if(strcmp(inputStr,cle_leaveGrpKeyword[i])==0 && (user->nowHierarchy>0)){
			cle_exitCmdGroup(user->nowHierarchy-1);
			user->nowHierarchy--;
			strcpy(inputStr, "");
			return 1;
		}
	return 0;
}

void cle_setMode(uint32 userID,uint32 mode,uint32 action)
{
	switch(mode){
	case CLE_MODE_PRIVILEGE:
		userDb[userID].flag.priviledgeMode=action;
		break;
	case CLE_MODE_SUPERENG:
		userDb[userID].flag.superengMode=action;
		break;
	default:
		break;
	}
	return;
}


void cle_setTabMode(uint32 userID,uint32 on)
{
	userDb[userID].flag.tabMode=on?1:0;
}

uint32 cle_getMode(uint32 userID,uint32 mode)
{
	switch(mode){
	case CLE_MODE_PRIVILEGE:
		return userDb[userID].flag.priviledgeMode;
	case CLE_MODE_SUPERENG:
		return userDb[userID].flag.superengMode;
	default:
		break;
	}
	return 0;
}


void cle_getPrompt(uint32 userID,int8* prompt,int32 maxLen)
{
	cle_user_t* user;
	cle_grp_t*	grp;
	int32 i;
	int32 cnt;


	user=&(userDb[userID]);
	grp=user->stack[ user->nowHierarchy ];

	memset(prompt,0,(size_t)maxLen);
	cnt=0;
	if(grp->cmdPrompt==NULL)
	{
		//count length
		for(i=0;i<=user->nowHierarchy;i++)
		{
			cnt+=strlen(user->stack[i]->cmdStr)+1;
		}
		cnt+=2; // <>

		strcat(prompt,"<");
		if(cnt>=maxLen)
			strcat(prompt,"prompt error");
		else
		{
			for(i=0;i<=user->nowHierarchy;i++)
			{
				strcat(prompt,user->stack[i]->cmdStr);
				strcat(prompt,"/");
			}
		}
		strcat(prompt,">");
	}
	else
	{
		strcat(prompt,"<");
		if((int32)strlen(grp->cmdPrompt)+2>=maxLen)
			strcat(prompt,"prompt error");
		else
			strcat(prompt,grp->cmdPrompt);
		strcat(prompt,">");
	}
	return;
}

int8* cle_getParseResult(uint32 userID)
{
	return parseResult.msg;
}
int8* cle_getOutputBuffer(uint32 userID)
{
	return cle_outputBuffer;
}

uint32 cle_getUiStyle(uint32 userID)
{
	return userDb[userID].uiStyle;
}

int32 cle_getTelnetSocket(uint32 userID)
{
//	if(userDb[userID].uiStyle!=CLE_UI_STYLE_TELNET)
		return 0;
//	if(userDb[userID].telnet_userDb->in!=1)
//		return 0;	
//	return userDb[userID].telnet_userDb->sockfd;
}

cle_screenInfo_t* cle_getScreenInfo(uint32 userID)
{
//	return userDb[userID].screenInfo;
return NULL;
}

static int32 cle_cmdCount(uint32 userID,
				   uint32 argCnt)
{
int32 cnt=0;

	if(!argCnt)
		return 0;
	cnt+=cle_cmdEveryWhereCount(userID,argCnt);
	cnt+=cle_execCount(userID,argCnt);
	cnt+=cle_groupCount(userID,argCnt);
	return cnt;
}


static int32 cle_cmdEveryWhereCount(uint32 userID,
							uint32 argCnt )
{
	uint32 pos;
//	cle_user_t* user;
	int32 cnt;
	
//	user=&(userDb[userID]);
	cnt=0;

	for(pos=0; pos<cmdEveryWhereNum ; pos++)
	{
		if(strncmp(cmdEveryWhere[pos].cmdStr, cle_tokens.buffer[cle_tokens.rowPtr],strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0)		
		{
			cnt++;
		}
	}
	return cnt;
}

static int32 cle_execCount( uint32 userID, uint32 argCnt)
{
	uint32 pos;
	cle_user_t* user;
	cle_grp_t*	grp;
	int32 	cnt;

	user=&(userDb[userID]);
	grp=user->stack[ user->hierarchy ];
	cnt=0;

	for(pos=0; pos< grp->execNum ; pos++) 
	{
		if(	cle_checkPriviledge(userID,grp->execPtr[pos].cmdFlags)  &&
			(	strncmp(grp->execPtr[pos].cmdStr, cle_tokens.buffer[cle_tokens.rowPtr],strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0)
		  )
		{
			cnt++;
		}
	}
	return cnt;
}

static int32 cle_groupCount(uint32 userID, 
							uint32 argCnt)
{
	uint32 pos;
	cle_user_t* user;
	cle_grp_t*	grp;
//	uint32 hierarchy;
	int32 cnt;

	user=&(userDb[userID]);
	grp=user->stack[ user->hierarchy ];
	cnt=0;

	for(pos = 0; pos<grp->grpNum; pos++)
	{
		if(strncmp(grp->grpPtr[pos].cmdStr, 
					cle_tokens.buffer[cle_tokens.rowPtr],
					strlen(cle_tokens.buffer[cle_tokens.rowPtr])) == 0)
		{
			if( !cle_checkPriviledge(userID,grp->grpPtr[pos].grpFlags) )
				continue;
			
			//record old hierarchy--for CLE_GROUP_TRANSPARENT recover 
			//hierarchy=user->nowHierarchy;
			
			user->hierarchy++;
			if(user->hierarchy ==user->stackDepth){
				parseResult.type = CLE_RESULT_STACK_OVERFLOW;
			}else{
				cle_tokens.rowPtr++;
				user->stack[user->hierarchy] = &(user->stack[user->hierarchy-1]->grpPtr[pos]);
				
				if( (argCnt == 1) )
					cnt++;
				else
					cnt+=cle_cmdCount(userID,argCnt-1);
				cle_tokens.rowPtr--;
			}
			user->hierarchy--;
		}
	}
	return cnt;
}

int32 cle_checkPriviledge(uint32 userID,uint16 AccessLevel)
{
	if(AccessLevel&CLE_MODE_PRIVILEGE)
		if(userDb[userID].flag.priviledgeMode!=1)
				return 0;

	if(AccessLevel&CLE_MODE_SUPERENG)
		if(userDb[userID].flag.superengMode!=1)
				return 0;
	return 1;
}

int32 cle_displayOutput(uint32 userID,int32 outputLen,int8* output)
{
	cle_rnBufHeader_t *rnHeader;
	uint32 pos;

	if(outputLen==0) return -1;
	pos=sizeof(cle_rnBufHeader_t);
	rnHeader=(cle_rnBufHeader_t *)output;
	if(rnHeader->version!=1)
		return -1;
	cle_interpretOutput(userID,rnHeader->entryCount,rnHeader->fieldCount,output+pos);
	return 0;
}

uint32 cle_interpretOutput(uint32 userID,uint32 entryCnt,uint32 fieldCnt,int8* output)
{
	uint32 i,j,pos;
	uint8 subEntry,subField;
	int8 *data;
	cle_rnBufItem_t item;
	union {
		int8*  d_buf;
		uint32 d_uint32;
		int32  d_int32;
		uint16 d_uint16;
		int16  d_int16;
	} value;

	int8 tmp[128];
	int8 iptmp[16];	
	int8 ethstring[18];
	pos=0;
	
	for(i=0;i<entryCnt;i++)
	{		
		for(j=0;j<fieldCnt;j++)
		{
			memcpy(&item,output+pos,sizeof(cle_rnBufItem_t));
			pos+=sizeof(cle_rnBufItem_t);
			data=output+pos;

			switch(item.type){
			case CLE_RNBUF_UINT32:
				memcpy(&value.d_uint32,data,sizeof(uint32));
				rtlglue_printf("%6d ",value.d_uint32);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_INT32:
				memcpy(&value.d_int32,data,sizeof(int32));
				rtlglue_printf("%6d ",value.d_int32);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_IPADDRESS:				
				value.d_buf = (int8*)ntop4((uint8 *)data, iptmp, sizeof(iptmp));
				rtlglue_printf("%16s ",value.d_buf);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_IPADDRESS_HOSTORDER:
				memcpy(&value.d_uint32,data,sizeof(uint32));
				value.d_uint32=htonl(value.d_uint32);
				value.d_buf = (int8*)ntop4((uint8 *)&value.d_uint32, iptmp, sizeof(iptmp));
				rtlglue_printf("%16s ",value.d_buf);
				pos+=item.dataLen;				
				break;
			case CLE_RNBUF_DISPLAYSTR:
			case CLE_RNBUF_NUMERICSTR:
				memcpy(tmp,data,item.dataLen);
				tmp[item.dataLen]=0;
				rtlglue_printf("%s ",tmp);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_MACADDRESS:
				//cfliu: will we have a bug for this reentrancy call? ethstring is a local variable...
				//also ether_ntoa_r may return FAILED.
				ether_ntoa_r((ether_addr_t *)data, (uint8*)ethstring);
				value.d_buf=ethstring;
				rtlglue_printf("%s ",value.d_buf);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_UINT16:
				memcpy(&value.d_uint16,data,sizeof(uint16));
				rtlglue_printf("%6hd",value.d_uint16);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_INT16:
				memcpy(&value.d_int16,data,sizeof(int16));
				rtlglue_printf("%6hd",value.d_int16);
				pos+=item.dataLen;
				break;
			case CLE_RNBUF_SUBITEM:
				subEntry=item.dataLen;
				subField=data[0];
				pos++;
				pos+=cle_interpretOutput(userID,subEntry,subField,output+pos);
				break;
			case CLE_RNBUF_RESULT:
				memcpy(&value.d_uint16,data,sizeof(uint16));
				switch(value.d_uint16){
				case CLE_RNBUF_RESULT_OK:
					rtlglue_printf("Succeed!! ");
					break;
			case CLE_RNBUF_RESULT_ERROR:
				{
					int8 *errstr=cle_getParseResult(userID);
					if(errstr[0]!=0)
						rtlglue_printf("Errno = %s", errstr);
					else
						rtlglue_printf("Error!!");
				}
					break;
				default:
					break;
				}
				pos+=item.dataLen;
				break;
			default:			
				pos+=item.dataLen;
				break;
			}
		}
		rtlglue_printf("\n");
	}
	return pos;
}

static void cle_tabModeReturn(uint32 userID)
{
	uint32 i;

	cle_hisBuffer[0]=0;	
	for(i=0;i<cle_tokens.rows;i++)
	{
		strcat(cle_hisBuffer,cle_tokens.buffer[i]);
		strcat(cle_hisBuffer," ");
	}
	cle_hisBuffer[strlen(cle_hisBuffer)-1]=0;
	cle_outputLen=0;
	cle_writeOutput(userID,cle_hisBuffer,strlen(cle_hisBuffer)+1);
}	
	

