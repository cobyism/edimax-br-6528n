/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : CLE API header file
* Abstract :                                                           
* Author : Ida Yang (idayang@realtek.com.tw)               
*
* -------------------------------------------------------
* $Id: rtl_cle.h,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: rtl_cle.h,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.10  2006/12/22 04:01:39  yjlou
* *: Since we DO NOT allow the usage of 'printk', we shall NOT define 'printk' in the header file. Therefore, nobody will make this mistake.
*
* Revision 1.9  2006/12/04 08:37:47  alva_zhang
* replace all cle_printf  with rtlglue_printf
*
* Revision 1.8  2006/12/01 03:27:14  alva_zhang
* +: Add Macro to replace printk with printk_tty which is defined in printk.c
*
* Revision 1.7  2006/12/01 03:24:28  alva_zhang
*
* Revision 1.6  2006/02/27 07:49:45  ympan
* +: Add some commands for testing the driver of RTL8366.
*
* Revision 1.5  2004/11/10 05:19:55  cfliu
* +: add new cle_cmdParser() to support cmd parsing without using cleshell
*
* Revision 1.4  2004/06/23 07:33:07  yjlou
* -: remove tailing newline characters
*
* Revision 1.3  2004/03/21 09:17:20  cfliu
* no message
*
* Revision 1.2  2004/03/03 13:08:17  cfliu
* +: add cle_cmdRoot[] as command tree root
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.21  2003/07/21 06:26:58  cfliu
* no message
*
* Revision 1.20  2003/05/15 07:53:03  jzchen
* Change from CLE_MODE_PRIVILEDGE to CLE_MODE_PRIVILEGE
*
* Revision 1.19  2003/03/31 10:42:39  cfliu
* change cle_init() prototype
*
* Revision 1.18  2003/03/14 13:33:07  cfliu
* Fix word  alignment problem in ciscoParser module
*
* Revision 1.17  2003/03/12 10:35:57  cfliu
* Move index2string array to CLE.
*
* Revision 1.16  2003/02/07 09:52:59  cfliu
* Export dynamic parameter check functions to all management command modules.
*
* Revision 1.15  2003/01/02 04:32:46  cfliu
* Define grpFlags field of cle_grp_s structure
*
* Revision 1.14  2002/12/26 13:59:02  cfliu
* Remove transparent field in cle_grp_t
* Remove  CLE_GROUP_NOT_TRANSPARENT and CLE_GROUP_TRANSPARENT
* Rename accessLevel and minArg fields in cle_exec_s
* Add CLE_STATEFULGROUP flag
* Add cle_getNextCmdToken()
*
* Revision 1.13  2002/12/25 07:25:35  cfliu
* Add CLE_KEY_HELP
*
* Revision 1.12  2002/12/20 03:36:09  idayang
* add userDb flag tabMode
* add cle_setTabMode()
*
* Revision 1.11  2002/12/17 08:53:49  idayang
* add define CLE_RNBUF_MSGQUEUE and CLE_RNBUF_END-
* for some commands (ex. ping)
* modify cle_freeScreen to fix a bug
*
* Revision 1.10  2002/12/13 06:37:22  idayang
* disguise password string as '*'
*
* Revision 1.9  2002/12/13 06:03:05  idayang
* add cle_writeToScreen  cle_writeNewLine
*
* Revision 1.8  2002/12/12 07:09:00  idayang
* add API for telnet module
*
* Revision 1.7  2002/12/10 11:28:05  cfliu
* Redefine CLE_USECISCOPARSER
*
* Revision 1.6  2002/12/06 10:55:20  cfliu
*
* Redefine cle_exec_t
*
* Revision 1.5  2002/12/04 03:24:04  idayang
* redefine CLE_UI_STYLE
*
* Revision 1.4  2002/12/02 13:49:22  idayang
* modify cle_addItemToOutput to translate host order ip to network order ip automatically
* modify the defination of CLE_RNBUF_IPADDRESS_HOSTORDER
*
* Revision 1.3  2002/12/02 06:08:57  idayang
* add cle_setUserAccessLevel
*
* Revision 1.2  2002/11/29 12:10:26  idayang
* add cle_addRowToOutput
* add cle_endRowToOutput
*
* Revision 1.1  2002/11/29 07:02:46  idayang
* Moved due to include/export/mgmt -> include/export/management.
*
* Revision 1.16  2002/11/27 10:09:01  waynelee
* add return buffer type - CLE_RNBUF_IPADDRESS_HOSTORDER
* modify return buffer API about auto counting items
*
* Revision 1.15  2002/11/15 03:27:58  idayang
* add cle_resetEntryCount API
*
* Revision 1.14  2002/11/14 08:08:59  idayang
* add cle_addStringItemToOutput API
*
* Revision 1.13  2002/11/14 07:57:12  idayang
* add CLE_RNBUF_RESULT for cle_rnBufHeader_t data_type
*
* Revision 1.12  2002/11/14 01:50:25  idayang
* add define for cle_rnBufHeader_t data_type
*
* Revision 1.11  2002/11/11 07:03:15  idayang
* reduce cle code size
*
* Revision 1.10  2002/10/16 10:19:24  idayang
* add define for cle_initUser()
*
* Revision 1.9  2002/10/16 09:42:29  idayang
* remove each item's friendly string in output format
*
* Revision 1.8  2002/10/16 02:03:57  idayang
* add cle_writeOutputHeader()
* add cle_addSubItemHeadToOutput()
* add cle_addItemToOutput()
* for write fix format output
*
* Revision 1.7  2002/10/09 04:58:23  cfliu
* Add clePrintf()
*
* Revision 1.6  2002/09/20 07:28:52  idayang
* add cle_absParser API
* for execution command line at absolute directory
*
* Revision 1.5  2002/09/20 02:51:08  idayang
* define data_type for cle return buffer use
*
* Revision 1.4  2002/09/16 06:49:54  idayang
* add struct cle_rnBufHeader_s
* for parser return buffer
*
*
How to use CLE module

1. system init:

	call cle_init
	
	ex.
	#include <management/rtl_cle.h>
   	cle_init(   2,	// Init command line engine user space allocation
				20,20, 	//max token len,max token count
				1024,	//size of parser result buffer 
				128,	//size of input data (the max string length of command line)
				1024,	//size of ouput buffer 
			);


2. module user init

	a.  #include <cmd/cmd_rtlRoot.h>
	b.  call cle_initUser

	ex. 	
	int32 userId;
	userId=cle_initUser("User",				//userName or module name
			CLE_UI_STYLE_STANDARD_IO,	//uiStyle
			0,				//access level
			10,	   			//depth of command tree
			rtlRootGrp); 			//commandEntryPoint ,cle_grp_t


3. command line parse

	call cle_parser

	ex.	
	uint32 outLen;	
	int8 outbuf[1024];

	//put command line into input[]
	outLen=1024; 	
	cle_parser(userId,input,&outbuf,&outLen);
	
4. call CLI(command line interface) thread

	call cle_console

	ex.
	static pthread_t  cle_thread;
	void    **status = NULL;

	cliThread=cle_console(rtlRootGrp);
	pthread_join(cliThread, (void **) &status);	

5. add command to RTL command set	

	a. open uisrc/cmd/cmd_rtlRoot.c
	b. add your commands in the cle_exec_t array

	ex. 

	if your command is : set ifconfig .....
	please add "ifconfig" command in cmdSet[]

	{
		"ifconfig",			//cmdStr
		"interface coifiguration",	//cmdDesc
		0,				//access level
		NULL,			//exist function
		"ifconfig <interface name>",			//cmdUsage
		0,				//minimum argument name + parameter
		NULL,				//args checking function string
		NULL,				//default value string
		ifconfig_main	//execution function
	},

	if your command is : set dhcp .....
	please add "dhcp" command in cmdSet[]

	if your command is : get dhcp .....
	please add "dhcp" command in cmdGet[]

	and so on...
	
	c. include your execution function's header file in "uisrc/cmd/cmd_rtlRoot.c"
	ex. add command "set ifconfig ..." 
	this command will call ifconfig_main()
	you must include ifconfig_main() 's header file ...
	so... 
	add "#include <proto/netinet/ifconfighlpapi.h" in "uisrc/cmd/cmd_rtlRoot.c"
*/

#ifndef CLE_H
#define CLE_H


#define CLE_CSI_HEAD              "\x1B\x5B"
#define CLE_VT100_COLUMN		80
#define CLE_INIT_ERROR  -1
#define CLE_INIT_SUCCESS 0

#define CLE_EXERN_OK_NOSCRIPT	0
#define CLE_EXERN_OK_SCRIPT		1
#define CLE_EXERN_ERROR			-1


/*************************/
/* Key code definition   */
/*************************/
enum CLE_KEYCODE_MAP
{
	CLE_KEY_UP,
	CLE_KEY_DOWN,
	CLE_KEY_RIGHT,
	CLE_KEY_LEFT,
	CLE_KEY_ENTER,
	CLE_KEY_HELP,
	CLE_KEY_BS,
	CLE_KEY_DEL,
	CLE_KEY_ESCAPE,
	CLE_KEY_CTRL_A,
	CLE_KEY_CTRL_C,
	CLE_KEY_CTRL_O,
	CLE_KEY_CTR_N,
	CLE_KEY_CTRL_P,
	CLE_KEY_CTRL_D,
	CLE_KEY_CTRL_U,
	CLE_KEY_CTRL_T,
	CLE_KEY_CTRL_R,
	CLE_KEY_CTRL_B,
	CLE_KEY_CTRL_L,
	CLE_KEY_CTRL_G,
	CLE_KEY_SPACE,
	CLE_KEY_PRINTABLE,
	CLE_KEY_UNKNOWN,
	CLE_KEY_TAB	
};

#ifdef CYGWIN
#define CLE_HISTORY 10
#define CLE_SCREEN_BUFLEN 128
#else
#define CLE_HISTORY 5
#define CLE_SCREEN_BUFLEN 128
#endif

typedef struct cle_uiHistory_s {
	int8		data[CLE_HISTORY][CLE_SCREEN_BUFLEN];
	int32	ptr;
	int32	top;
	int32	bottom;
}cle_uiHistory_t;


//define for screen status
#define CLE_SCR_STATUS_NORMAL 0
#define CLE_SCR_STATUS_PASSWORD 1
typedef struct cle_screenInfo_s
{
	uint32 userID;
	uint32 uiStyle;
	uint32 x,y;
	int8 sendBuf[CLE_SCREEN_BUFLEN];
	uint32 sendBufCnt;
	cle_uiHistory_t history;
	struct {
	uint32 password:1;/* input password status*/ 
	}status;	
}cle_screenInfo_t;


#define CLE_MODE_TURNON		1
#define CLE_MODE_TURNOFF	0


struct cle_exec_s;
typedef struct cle_exec_s cle_exec_t;



struct enumList {
	int8 *tokenName;
	int8 *helpString;
};


//checkFunc(), provided by user, checks whether 'input' is acceptable. If it is, 'input' is converted to designated type
//at '*token'. If 'input' is not acceptable, -1 is returned. 
//
//If checkFunc() decides that a new command should be used to handle rested tokens in 'input' command string, -1 is returned and the new command structure pointer
// is returned via *nextCmd. 
//
//Otherwise, checkFUnc() always returns NULL for *nextCmd.
//
//enumFunc(), also provided by user, prints all possible vlues for this specific type in a string (separated by whitespace). 
//cle_ciscoParser.c will print it to UI. If enumFunc is NULL, 'formatString is printed to UI
//
typedef struct cle_paramType_s {
	int8 *formatString;
	uint32 maxSize;
	int32 (*checkFunc)(void **token, int8 *input, int32 size, cle_exec_t **nextCmd);
	int32 (*enumFunc)(int32 entries, struct enumList *list);
		#define CLE_MAXENUMENTRIES	128
	int8 *helpStr;
} cle_paramType_t  ;


struct cle_exec_s
{
	int8*	cmdStr;			//command string
	int8*	cmdDesc;		//command description
	int8*	cmdUsage;		//command format string

	int32	(*execFun)(uint32,int32, int8 **);  //execution function  
	uint8	cmdFlags;
		#define CLE_USECISCOCMDPARSER	0x1
		#define CLE_MODE_PRIVILEGE      0x40	//Priviledged mode. Can access commands requires CLE_MODE_PRIVILEGE priviledge
		#define CLE_MODE_SUPERENG 	    0x80	//Engineering mode. Can access any command.
	uint8	cmdParamTypeNum;
		//REMOVEME:  Remove '__minArg' after all commands moved to new parser
		#define __minCmdArgs	cmdParamTypeNum
	cle_paramType_t *cmdParamType;
};


typedef struct cle_grp_s
{
	int8*	cmdStr;			
	int8*	cmdDesc;		//command description
	int8*	cmdPrompt;		//prompt string -- (NULL for show directory path)
	cle_exec_t*	execPtr;
	struct cle_grp_s*	grpPtr;
	uint32	execNum:8, 
			grpNum:8, 
			grpFlags:8;
				#define CLE_STATEFULGROUP		0x1
				#define CLE_PRIVILEDGEDGROUP	CLE_MODE_PRIVILEGE
				#define CLE_SUPERENGGROUP		CLE_MODE_SUPERENG
}cle_grp_t;

//-------------------------------------------------------------
//parser return buffer header structure: 
//-------------------------------------------------------------
typedef struct cle_rnBufHeader_s
{
	uint8 	version;
	uint8 	format;
	uint16	len;
	uint16	entryCount;
	uint8	fieldCount;
	uint8	ctrlFlag;
}cle_rnBufHeader_t;

//define for cle_rnBufHeader_t.format  
#define CLE_RNBUF_SCALAR 	0 //stream format=scalar
#define CLE_RNBUF_TABLE  	1 //stream format=table
#define CLE_RNBUF_RAWDATA 	0 //data format=Raw data
#define CLE_RNBUF_ENUM 		2 //data format=Enum data
//set format ....ex. CLE_RNBUF_SCALAR|CLE_RNBUF_ENUM

//define for cle_rnBufHeader_t data_type
#define CLE_RNBUF_UINT32		1		//%ud
#define CLE_RNBUF_INT32  		2		//%d
#define CLE_RNBUF_IPADDRESS 	3		//network order //%p
#define CLE_RNBUF_DISPLAYSTR 	4		//%[length]s
#define CLE_RNBUF_NUMERICSTR	5		//%n
#define CLE_RNBUF_MACADDRESS	6		//%m
#define CLE_RNBUF_UINT16		7		//%uh
#define CLE_RNBUF_INT16			8		//%h
#define CLE_RNBUF_SUBITEM		9		//not support for cle_addRawToOutput
#define CLE_RNBUF_RESULT		10		//%r  //will remove 
#define CLE_RNBUF_MSGQUEUE		11		//%k  //need a message queue
//---------------------------------------------------------------------
//translation - cle_rnBufHeader_t data_type
//it will do (host order) -> (nertwork order) translation frist by CLE
//and it's cle_rnBufHeader_t data_type will be CLE_RNBUF_IPADDRESS
#define CLE_RNBUF_IPADDRESS_HOSTORDER	201//host order //%q
//---------------------------------------------------------------------

//for CLE_RNBUF_RESULT message 
#define CLE_RNBUF_RESULT_OK		0
#define CLE_RNBUF_RESULT_ERROR	1

//define for cle_rnBufHeader_t.ctrlFlag
#define CLE_RNBUF_END	1	//identify this is the last data for message queue use

#define CLE_INDEX2STRING_SIZE	100


//-------------------------------------------------

//-------------------------------------------------
//parser return buffer's item structue :
//-------------------------------------------------
typedef struct cle_rnBufItem_s
{
	uint8 	type;
	uint8	dataLen;
}cle_rnBufItem_t;

typedef struct cle_rnBufSubitem_s{
	uint8 type;
	uint8 entryC;
	uint8 fieldC;
}cle_rnBufSubitem_t;

//-------------------------------------------------------------
//input : setup the max number of user for using CLE
//-------------------------------------------------------------
int32 cle_init( uint32 concurrentUserNumbers, uint32 cmdTokenLen, uint32 cmdTokenNum); //tokens array length ,max count


//for cle_initUser()
#define CLE_USER_INIT_ERROR 9999
#define CLE_UI_STYLE_STANDARD_IO	1
#define CLE_UI_STYLE_TELNET 		2
#define CLE_UI_STYLE_WEB 			4
#define CLE_UI_STYLE_SNMP 			8

//for determin UI_STYLE 
#define CLE_UI_STYLE_TERMINAL	(CLE_UI_STYLE_STANDARD_IO|CLE_UI_STYLE_TELNET)


//-------------------------------------------------------------
//return : success- userId ; error- CLE_USER_INIT_ERROR
//-------------------------------------------------------------
uint32 cle_initUser(	int8*  userName,//user name or module name
					uint32 uiStyle,//user command line Interface style
					uint32 accessLevel,
					uint32 stackDepth,//group depth and parse result buffer
					cle_grp_t * commandEntryPoint);//command tree root

//void cle_initUserData(uint32 userID,telnet_userDb_t *telnet_userDb);



#define CLE_PARSER_SUCCESS	 0
#define CLE_PARSER_ERROR	-1
//-------------------------------------------------------------
//input: userID - return from cle_initUser
//		 inputBuf - command line string
//		 outputBuf - output data
//-------------------------------------------------------------
int32 cle_parser(uint32 userID,int8* inputBuf,int8* outputBuf,uint32* outputLen);

//for absolute directory execution command line
int32 cle_absParser(uint32 userID,int8* inputBuf,int8* outputBuf,uint32* outputLen);

int32 cle_cmdParser( int8 *cmdString, cle_exec_t *CmdFmt, int8 *delimiter);
#define CLE_WRITEOUTPUT_ERROR 	-1
#define CLE_WRITEOUTPUT_SUCCESS  0
int32 cle_writeOutput(uint32 userID,int8* buf,uint32 bufLen);
int32 cle_writeOutputHeader(uint32 userID,uint8 format,uint16 itemCount,uint8 fieldCount);
int32 cle_addItemToOutput(uint32 userID,uint8 type,int8* buf,uint32 bufLen);
int32 cle_addStringItemToOutput(uint32 userID,int8* fmt,...);
int32 cle_setOutputLen(uint32 userID);
int32 cle_addSubItemHeadToOutput(uint32 userID,uint32 entryCnt,uint32 fieldCnt);
int32 cle_resetEntryCount(uint32 userID,uint16 entryCount);
void cle_addRowToOutput(uint32 channel,int8* fmt,...);
void cle_endRowToOutput(uint32 channel);



//-------------------------------------------------------------
//for execFun use -- if you want to show message to all user's 
//					 parser result, use cle_printToALL
//this function will copy message to all user's parser result
//-------------------------------------------------------------
int32 cle_printToAll(int8* fmt,...);
void cli_printf(int8* fmt,...);

//for set mode
void cle_setMode(uint32 userID,uint32 mode,uint32 action);
int32 cle_setUserAccessLevel(uint32 userID,uint16 accessLevel);
void cle_getUserAccessLevel(uint32 userID,uint16 *accessLevel);
void cle_setTabMode(uint32 userID,uint32 on);

//------------------------------------------------------
//get mode status
//return : CLE_MODE_TURNON or CLE_MODE_TURNOFF
uint32 cle_getMode(uint32 userID,uint32 mode);
//------------------------------------------------------

void cle_getPrompt(uint32 userID,int8* prompt,int32 maxLen);
int8* cle_getParseResult(uint32 userID);
uint32 cle_getUiStyle(uint32 userID);

int32 cle_exitCheck(int8* inputStr);
int32 cle_specialCheck(uint32 userID, int8 * inputStr);

//---------------------------------------------------------------
int8 cle_ioReadFromUart(uint32 wait);
int8 cle_ioWriteToUart(int8* buf,int32 len);
//---------------------------------------------------------------

int32 cle_displayOutput(uint32 userID,int32 outputLen,int8* output);
uint32 cle_interpretOutput(uint32 userID,uint32 entryCnt,uint32 fieldCnt,int8* output);


void cle_writeByte(cle_screenInfo_t* screenInfo,int8 ch);

void cle_writeString(cle_screenInfo_t* screenInfo,int8* str);

int8 cle_readByte(cle_screenInfo_t* screenInfo,uint32 wait);

void cle_cursorUp(cle_screenInfo_t* screenInfo,uint32 cnt);
void cle_cursorLeft(cle_screenInfo_t* screenInfo,uint32 cnt);
void cle_cursorRight(cle_screenInfo_t* screenInfo,uint32 cnt);
uint32 cle_getKey(cle_screenInfo_t* screenInfo,int8* ch);

void cle_backAWord(cle_screenInfo_t* screenInfo);

void cle_cursorReverseLineFeed(cle_screenInfo_t* screenInfo);

void cle_initScreen(uint32 userID,cle_screenInfo_t* screenInfo);
void cle_freeScreen(cle_screenInfo_t* screenInfo);
void cle_writeToScreen(cle_screenInfo_t* screenInfo,int8* str);
void cle_writeNewLine(cle_screenInfo_t* screenInfo);

int32 cle_addHistory(cle_screenInfo_t* screenInfo,int8* cmdStr);
int8* cle_getHistoryUp(cle_screenInfo_t* screenInfo);
int8* cle_getHistoryDown(cle_screenInfo_t* screenInfo);
void cle_flush(cle_screenInfo_t* screenInfo);

void *cle_ioControl(void *arg);
void cle_script(uint32 userId,int8* fileName);

//----------------------------------------------------
int8 cle_ioRead(uint32 channel,uint32 wait);
int8 cle_ioWrite(uint32 channel,int8* buf,int32 len);
//----------------------------------------------------

//----------------------------------------------------
int8 cle_getch(uint32 channel);
void cle_gets(uint32 channel,int8* str,uint32 strLen);
void cle_getInt(uint32 channel,uint32* num);
//----------------------------------------------------
void cle_getString(cle_screenInfo_t* screenInfo,int8* str,uint32 strLen);


int32 cle_raw_mode (int32 on);

//To be used in Cisco command check functions. Tells parser to unconditionally switch to next level of command group 
//named 'nextGroupname' when check function is returned.
int32 cle_setNextGroupName(int8 *nextGroupName);

//Returns -1 if no more tokens.
//nextToken returns next token, may be a keyword or a converted data specific to previous related keyword
//savedData is value-result parameter.
int32 cle_getNextCmdToken(int8 **nextToken, int32 *size, int8 **savedData);

//converts an index to string. used by command modules which use the Cisco command parser.
int8 *cle_index2string(uint32 index);


#define CLE_UI_WAIT 1
#define CLE_UI_NOTWAIT 0
#define U8_value(x)		((uint8)*((memaddr *)x))
#define I8_value(x)		((int8)*((memaddr *)x))
#define U16_value(x)		((uint16)*((memaddr *)x))
#define I16_value(x)		((int16)*((memaddr *)x))
#define U32_value(x)		((uint32)*((memaddr *)x))
#define I32_value(x)		((int32)*((memaddr *)x))


int32 cle_checkDynCmdParam_String(void **token, int8 *input, int32 size, cle_exec_t **nextCmd);
int32 cle_checkDynCmdParam_AlphabeticString(void **token, int8 *input, int32 size, cle_exec_t **nextCmd);
int32 cle_checkDynCmdParam_Number(void **token, int8 *input, int32 size, cle_exec_t **nextCmd);
int32 cle_checkDynCmdParam_Ip(void **token, int8 *input, int32 size, cle_exec_t **nextCmd);
int32 cle_checkDynCmdParam_Mac(void **token, int8 *input, int32 size, cle_exec_t **nextCmd);


extern cle_grp_t  cle_cmdRoot[];

#endif
