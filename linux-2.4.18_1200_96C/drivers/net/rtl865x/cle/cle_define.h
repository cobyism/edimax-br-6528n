/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : CLE module defined value  
* Abstract :                                                           
* Author : Ida Yang (idayang@realtek.com.tw)               
*
* $Id: cle_define.h,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_define.h,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.3  2006/07/13 16:01:16  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.2  2005/02/25 05:27:31  cfliu
* *** empty log message ***
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.7  2002/12/30 07:03:14  idayang
* add tab function
*
* -------------------------------------------------------
*/


#ifndef cle_define_h
#define cle_define_h


#define CLE_EXEC_CMD				0
#define CLE_HELP_CMD				1
#define CLE_HELP_CMD_DESC			2
#define CLE_HELP_CMD_DESC_USAGE		3
#define CLE_HELP_KEYWORD			4	

//Successful results
#define CLE_RESULT_EXECUTION				1
#define CLE_RESULT_GROUP					2
#define CLE_RESULT_DESC_MSG				3
#define CLE_RESULT_QUERY_REPORT			4
#define CLE_RESULT_CONFLICT_LIST			5
#define CLE_RESULT_UNKNOWN				6
//Error results
#define CLE_RESULT_DESC_OVERFLOW			7
#define CLE_RESULT_QUERY_OVERFLOW		8
#define CLE_RESULT_CONFLICT_OVERFLOW		9
#define CLE_RESULT_UNEXPECTED_LEAVE		10
#define CLE_RESULT_STACK_OVERFLOW		11
#define CLE_RESULT_ARGS_SCANTY			12
#define CLE_RESULT_INCOMPLETE_COMMAND	13

#define CLE_RESULT_EXEC_EXIST				14
#define CLE_RESULT_EXEC_NOTEXIST			15
#define CLE_RESULT_GROUP_EXIST				16
#define CLE_RESULT_GROUP_NOTEXIST			17
#define CLE_RESULT_NOACTION					18

#define assert(x)\
if (!(x)) { \
        rtlglue_printf("\n%s:%s:%d:'%s'", __FILE__, __FUNCTION__, __LINE__, #x);\
	while(1){};\
}
#endif

