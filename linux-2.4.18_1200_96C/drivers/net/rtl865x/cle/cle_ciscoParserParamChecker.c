/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Global command check functions for ciscoParser.c
* Abstract: 
* Author  : Chun-Feng Liu (cfliu@realtek.com.tw)
* $Id: cle_ciscoParserParamChecker.c,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_ciscoParserParamChecker.c,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.11  2007/06/01 09:14:57  michaelhuang
* -: removed the rtl867x compiler flag.
*
* Revision 1.10  2007/01/09 05:13:59  michaelhuang
* *: fixed some compile flag error
*
* Revision 1.9  2007/01/09 02:49:52  michaelhuang
* *: Prepare implement test module for RTL8672
*
* Revision 1.8  2006/07/13 16:01:16  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.7  2006/05/03 01:46:33  yjlou
* *: little fixed for splint check
*
* Revision 1.6  2005/06/10 05:32:22  yjlou
* +: Porting CLE Shell to Linux user space for model test.
*    See RTL865X_MODEL_USER compile flags.
*
* Revision 1.5  2004/12/21 13:17:55  yjlou
* +: support cle_checkDynCmdParam_Net(), but still buggy: slash(/) will be stripped.
*
* Revision 1.4  2004/11/10 05:16:22  cfliu
* +: Move utility routines from ROME driver to cle and declare them as static
* functions.
*
* Revision 1.3  2004/03/21 09:17:20  cfliu
* no message
*
* Revision 1.2  2004/03/19 13:12:42  cfliu
* no message
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.19  2003/08/06 03:33:28  hiwu
* remove unused include files
*
* Revision 1.18  2003/07/21 06:33:03  cfliu
* eliminate dependency of reentrant module
*
* Revision 1.17  2003/04/30 15:36:20  cfliu
* rename macros. macro moved to types.h
*
* Revision 1.16  2003/03/31 10:51:19  cfliu
* Remove compiler warning.
*
* Revision 1.15  2003/03/21 08:05:58  cfliu
* Bug fixes...
*
* Revision 1.14  2003/03/17 09:29:24  cfliu
* no message
*
* Revision 1.13  2003/03/14 13:31:08  cfliu
* Fix word  alignment problem in ciscoParser module
*
* Revision 1.12  2003/03/03 09:12:41  hiwu
* change ip4a to ipaddr_t
*
* Revision 1.11  2003/02/19 09:32:43  cfliu
* Use strtol insetead of atoi so octal and hexadecimal numbers can be accepted with %d type.
*
* Revision 1.10  2003/02/07 09:57:19  cfliu
* Export dynamic parameter check functions to all management command modules.
*
* Revision 1.9  2003/01/21 06:01:18  cfliu
* * Remove redundant header file inclusion...still need closer look on them..
*
* Revision 1.8  2003/01/02 04:44:29  cfliu
* no message
*
*/

#include "../rtl865x/rtl_types.h"

#include "rtl_cle.h"
#include "cle_define.h"
#include "rtl_utils.h"

#if defined(RTL865X_MODEL_USER)
#include <ctype.h>
#include "rtl_glue.h"
#include "cle_userglue.h"
#endif


/*********************************************************************************
	Section 6:	Type checking utility functions registered in paramTypes structure. More to be added
**********************************************************************************/
//Accept any text string 'input'. No check is applied.
 int32 cle_checkDynCmdParam_String(void **token, int8 *input, int32 size, cle_exec_t **nextCmd){
	if(input){
		*token = input;
		return SUCCESS;
	}
	*nextCmd=NULL;
	return FAILED;
}	


//Accept only text strings begins with a alphabetic character. ie. A-Z, a-z
 int32 cle_checkDynCmdParam_AlphabeticString(void **token, int8 *input, int32 size, cle_exec_t **nextCmd){
	if(input && isalpha(*input)){
		*token = input;
		return SUCCESS;
	}
	*nextCmd=NULL;
	return FAILED;
}	



 int32 cle_checkDynCmdParam_Number(void **token, int8 *input, int32 size, cle_exec_t **nextCmd){
	int32 number;
	if(isalpha(input[0]))
		goto failed;
	if(strlen(input)==1 && *input=='0')
		number = 0;
	else if(strchr(input, '.')) //we don't allow any dot in a decimal number.
		goto failed;
	else{
		//number = strtoul(input, NULL, 0); //chhuang
		number = simple_strtol(input, NULL, 0);
		if(number==0)
			goto failed;
	}
	assert(IS4BYTEALIGNED(I32_value(token)));
	*((uint32 *) *((memaddr *)token))	= number;
	return sizeof(int32);
failed:
	*nextCmd=NULL;
	return FAILED;
}	

//converted to network order
int32 cle_checkDynCmdParam_Ip(void **token, int8 *input, int32 size,cle_exec_t **nextCmd){
	assert(IS4BYTEALIGNED((memaddr)*((struct in_addr **) token)));
	if(IpStrToAscii(input, *((ipaddr_t **) token)))
		return sizeof(ipaddr_t);
	*nextCmd=NULL;
	return FAILED;
}	

// 192.168.1.0/24
int32 cle_checkDynCmdParam_Net(void **token, int8 *input, int32 size,cle_exec_t **nextCmd){
	char *first, *second;
	char *store;
	
	assert(IS4BYTEALIGNED((memaddr)*((struct in_addr **) token)));

	first = strtok( input, "/" );
	if ( first==NULL ) return FAILED;
	second = strtok( NULL, "/" );
	if ( second==NULL ) return FAILED;
	
	if(IpStrToAscii(first, *((ipaddr_t **) token))==FALSE)
		return FAILED;
	
	for( ; *second; second++ )
		if (!isdigit(*second)) return FAILED;
	store = ((char*)*token)+4;
	*store = simple_strtol( second, NULL, 10 );

	return sizeof(ipaddr_t)+1;
}	

int32 cle_checkDynCmdParam_Mac(void **token, int8 *input, int32 size, cle_exec_t **nextCmd){
	ether_addr_t ether;
	if(ether_aton_r(input, &ether)){
		*nextCmd=NULL;
		return FAILED;
 	}
	memcpy(*token, &ether, ETHER_ADDR_LEN);
	return ETHER_ADDR_LEN;
}
 


////////////////////////////////////////////////////////////////////////
 
 cle_paramType_t systemParamTypes[] ={
	{"%s", 	0, cle_checkDynCmdParam_String, 	NULL, 	"Any text string"},
	{"%str", 	0, cle_checkDynCmdParam_AlphabeticString, 	NULL, 	"A text string begins with A-Z,a-z"},
	{"%d", 	4, cle_checkDynCmdParam_Number, 	NULL,	"A numeric number"},
	{"%ip", 	4, cle_checkDynCmdParam_Ip, 		NULL,	"An IP address. X.X.X.X (X=0~255)"},
	{"%net", 5, cle_checkDynCmdParam_Net,	NULL,	"An IP address and CIDR. X.X.X.X/Y (X=0~255,Y=0~32)"},	
	{"%mac", 6, cle_checkDynCmdParam_Mac,	NULL,	"A MAC address. 00:11:22:33:44:55"},	
};

int32 sysParamTypesNum = sizeof(systemParamTypes)/sizeof(cle_paramType_t);

