/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : utility for parser engine
* Abstract :                                                           
* Author : Ida Yang (idayang@realtek.com.tw)               
*
* $Id: cle_utility.c,v 1.1 2007/12/04 11:52:00 joeylin Exp $
* $Log: cle_utility.c,v $
* Revision 1.1  2007/12/04 11:52:00  joeylin
* for Command Line Engine
*
* Revision 1.11  2007/06/01 09:14:57  michaelhuang
* -: removed the rtl867x compiler flag.
*
* Revision 1.10  2007/01/09 05:14:00  michaelhuang
* *: fixed some compile flag error
*
* Revision 1.9  2007/01/09 02:49:52  michaelhuang
* *: Prepare implement test module for RTL8672
*
* Revision 1.8  2006/12/25 08:44:03  nickwu
* mark <linux/socket.h> and <linux/in.h> for x86 compile
*
* Revision 1.7  2006/07/13 16:01:17  chenyl
* *: modify for RTL865xC compilation
* *: dos2unix
*
* Revision 1.6  2005/06/10 05:32:22  yjlou
* +: Porting CLE Shell to Linux user space for model test.
*    See RTL865X_MODEL_USER compile flags.
*
* Revision 1.5  2004/03/21 09:17:20  cfliu
* no message
*
* Revision 1.4  2004/02/26 04:48:40  cfliu
* Move all non cle related utility routines into table driver directory
*
* Revision 1.3  2004/02/24 06:44:08  cfliu
* Move memDump to glue module...
*
* Revision 1.2  2003/12/09 15:16:13  cfliu
* add memDump function
*
* Revision 1.1  2003/12/03 14:18:55  cfliu
* Initial version. Part of Command Line Engine.
*
* Revision 1.10  2002/12/12 09:03:45  idayang
* remove warnings
*
* Revision 1.9  2002/12/05 06:53:42  idayang
* modify cle_strToToken() to accept ""  for parameter.
* if an argument contain blank character .. you can use "" to mark it.
* ex. command -- tftp get "this is a test"
* you will get three arguments: [tftp],[get],[this is a test]
*
* -------------------------------------------------------
*/
#include "../rtl865x/rtl_types.h"

#ifndef RTK_X86_CLE//RTK-CNSD2-NickWu-20061222: for x86 compile
#include <linux/socket.h>
#include <linux/in.h>
#endif

//#include "strutils.h" // for strtok_r()

#include "cle_define.h"
#include "cle_utility.h"

#if defined(RTL865X_MODEL_USER)
#include <string.h>
#endif

/*
#ifndef CYGWIN
static int8 *save_ptr;
#endif
int32 cle_strToToken(int8 *inputStr,cle_tokenBuffer_t *tokenBuf) 
{
int8 * tokenPtr;
uint32 len;
uint32 i;

	tokenBuf->rows = 0;
	tokenBuf->helpFlag=CLE_EXEC_CMD;

	if(strlen(inputStr)==0)
		return 0;

#ifdef CYGWIN	
	tokenPtr = strtok(inputStr, " \n");
#else
	tokenPtr = strtok_r(inputStr, " \n", &save_ptr);
#endif

	while(tokenPtr != NULL && tokenBuf->rows<tokenBuf->y)
	{
		len=strlen(tokenPtr);
		if(len < tokenBuf->x )
		{
			for(i=0;i<cle_helpKeywordNum;i++)
			{
				if(strcmp(tokenPtr,cle_helpKeyword[i])==0)
				{
					switch(i){
					default:
					case 0:
						tokenBuf->helpFlag=CLE_HELP_CMD_DESC;
						break;
					case 1:
						tokenBuf->helpFlag=CLE_HELP_CMD;
						break;
					case 2:
						tokenBuf->helpFlag=CLE_HELP_CMD_DESC_USAGE;
						break;
					}
					return 0;
				}
			}
			if(tokenPtr[len-1]=='?')
			{
				tokenBuf->helpFlag=CLE_HELP_KEYWORD;
				strncpy(tokenBuf->buffer[tokenBuf->rows], tokenPtr,tokenBuf->x);
				tokenBuf->buffer[tokenBuf->rows][len-1]=0;
				tokenBuf->rows ++;
				return 0;
			}
			strncpy(tokenBuf->buffer[tokenBuf->rows], tokenPtr,tokenBuf->x);
			tokenBuf->rows ++;
		}
		else
			return -1;
#ifdef CYGWIN
		tokenPtr = strtok(NULL, " ");
#else
		tokenPtr = strtok_r(NULL, " ", &save_ptr);
#endif
	}
	if(tokenPtr != NULL)
		return -1;
	return 0;
}
*/

#define CLE_STATE_N 1
#define CLE_STATE_QM 2
#define CLE_STATE_BK 3

static int32 cle_checkHelpKeyword(cle_tokenBuffer_t *tokenBuf);


int32 cle_strToToken(int8 *inputStr,cle_tokenBuffer_t *tokenBuf) 
{
	uint32 i;
	int8 fch;
	uint32 state;

	tokenBuf->rows = 0;
	tokenBuf->helpFlag=CLE_EXEC_CMD;

	state=CLE_STATE_BK;
	i=0;
	while( (fch=*inputStr++)!=0 )
	{
		if(i>=tokenBuf->x) return FAILED;
		switch(state){
		case CLE_STATE_N:
			if(fch==' '||fch==0x09)
			{
				if(cle_checkHelpKeyword(tokenBuf))return 0;
				state=CLE_STATE_BK;
			}
			else if(fch=='"')
			{
				i=0;				
				state=CLE_STATE_QM;
			}
			else
			{
				tokenBuf->buffer[tokenBuf->rows][i++]=fch;
				tokenBuf->buffer[tokenBuf->rows][i]=0;
			}
			break;
		case CLE_STATE_BK:
			if(fch==' '||fch==0x09)
			{}
			else if(fch=='"')
			{
				i=0;
				state=CLE_STATE_QM;
			}
			else
			{
				i=0;
				tokenBuf->buffer[tokenBuf->rows][i++]=fch;
				tokenBuf->buffer[tokenBuf->rows][i]=0;
				state=CLE_STATE_N;
			}
			break;		
		case CLE_STATE_QM:
			if(fch=='"')
			{
				tokenBuf->buffer[tokenBuf->rows][i]=0;
				if(cle_checkHelpKeyword(tokenBuf))return 0;
				state=CLE_STATE_BK;
			}
			else
			{
				tokenBuf->buffer[tokenBuf->rows][i++]=fch;
				tokenBuf->buffer[tokenBuf->rows][i]=0;
			}
			break;
		default:
			break;
		}
	}
	if(state==CLE_STATE_N)
		cle_checkHelpKeyword(tokenBuf);

	return 0;
}


static int32 cle_checkHelpKeyword(cle_tokenBuffer_t *tokenBuf)
{
	uint32 len,i;
	int8* str;

	if(tokenBuf->rows>=tokenBuf->y-1) return 1;

	str=tokenBuf->buffer[tokenBuf->rows];
	
	len=strlen(str);
	for(i=0;i<cle_helpKeywordNum;i++)
	{
		if(strcmp(str,cle_helpKeyword[i])==0)
		{
			switch(i){
			default:
			case 0:
				tokenBuf->helpFlag=CLE_HELP_CMD_DESC;
				break;
			case 1:
				tokenBuf->helpFlag=CLE_HELP_CMD;
				break;
			case 2:
				tokenBuf->helpFlag=CLE_HELP_CMD_DESC_USAGE;
				break;
			}
			return 1;
		}
	}
	if(str[len-1]=='?')
	{
		tokenBuf->helpFlag=CLE_HELP_KEYWORD;
		str[len-1]=0;
		tokenBuf->rows ++;
		return 1;
	}
	tokenBuf->rows ++;
	return 0;
}

#ifdef __linux__

/*
 * reentrant version of strtok.
 * source ported from uClibC.
 * This routine will call strspn() and strpbrk(), which are reentrant
 * routines in green hills library. As for on the cygwin platform,
 * strspn()/strpbrk() are also available, and the reentrancy problem
 * should not matter.
 * Note that prototype has been changed from 'char' to 'int8'.
 * original prototype: 
 *   char *strtok_r(char *s, const char *delim, char **save_ptr)
 *
 */
int8 *
strtok_r (int8 * s, const int8 * delim, int8 ** save_ptr)
{
  int8 *token;

  token = 0;			/* Initialize to no token. */

  if (s == 0)
    {				/* If not first time called... */
      s = *save_ptr;		/* restart from where we left off. */
    }

  if (s != 0)
    {				/* If not finished... */
      *save_ptr = 0;

      s += strspn (s, delim);	/* Skip past any leading delimiters. */
      if (*s != '\0')
	{			/* We have a token. */
	  token = s;
	  *save_ptr = strpbrk (token, delim);	/* Find token's end. */
	  if (*save_ptr != 0)
	    {
	      /* Terminate the token and make SAVE_PTR point past it.  */
	      *(*save_ptr)++ = '\0';
	    }
	}
    }

  return token;
}

#endif /* __linux__ */



