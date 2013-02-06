
#include	"monitor.h"

#define KEYCODE_BS			0x08
#define KEYCODE_TAB			0x09
#define KEYCODE_ESC			0x1B
#define KEYCODE_SP			0x20
#define KEYCODE_CR			0x0D
#define KEYCODE_LF			0x0A
#define KEYCODE_BS_7F		0x7F

#ifdef CONFIG_HISTORY_KEYIN
//wei add
#define KEYCODE_BRACKET	0x5B
#define KEYCODE_VKUP	0x1b5b4100  //ESC [ A
#define KEYCODE_VKDOWN	0x1b5b4200  //ESC [ B
#endif

#define TAB				8

//extern	int		GetChar(void);
#define GetChar()	serial_inc()
#define PutChar(x)	serial_outc(x)
static char*		ArgvArray[MAX_ARGV];


/*-----------------------------------------------
	Lower case string to upper case string
-----------------------------------------------*/

char* StrUpr( char* string ){
	char*		p ;
	const int	det = 'a' - 'A';

	p = string ;
	while( *p ){
		if( *p >= 'a'  &&  *p <= 'z' ){
			*p -= det ;
		}
		p++ ;
	}
	return string ;
}

/*-----------------------------------------------
	Upper case string to lower case string
-----------------------------------------------*/
#if 0
char* StrLwr( char* string ){
	char*		p ;
	const int	det = 'a' - 'A';

	p = string ;
	while( *p ){
		if( *p >= 'A'  &&  *p <= 'Z' ){
			*p += det ;
		}
		p++ ;
	}
	return string ;
}
#endif
/*
---------------------------------------------------------------------------
; char WaitKey()
:   Description:
;	    Get a character from the stdio
;	    Function not return until get a character.
;   Input:
;	    none
;   Output:
;	    character input data
;   Destroy:
---------------------------------------------------------------------------
*/
char WaitKey(void)
{
    int	    i;
    while(1)
    {
	if( (i=GetChar())!=-1 ) return(i);
    }

}

/*
---------------------------------------------------------------------------
; void GetLine(char * buffer, const unsigned int size, int EchoFlag)
:   Description:
;	    Input line from stdio
;	    Function not return until a line input complete.
;	    End input line only by <ENTER> key (0x0d)
;   Input:
;	    buffer: pointer to buffer for getline
;	    size  : maximum buffer size
;			EchoFlag: 1: Echo character when get character
;					: 0: No Echo character
;   Output:
;	    buffer: pointer to input data, end by zero character
;	    none
;   Destroy:
---------------------------------------------------------------------------
*/

#ifdef CONFIG_HISTORY_KEYIN
#define HISTORY_CMD_ELN 8
unsigned char history_cmd[HISTORY_CMD_ELN][ MAX_MONITOR_BUFFER +1 ]={0};
int hist_save_idx=0;
int hist_see_idx=0;
#endif
void GetLine( char * buffer, const unsigned int size, int EchoFlag )
{
	char		ch = 0 ;
	int		c;
	char*		p = buffer ;
	unsigned int	n = 0L ;
	int		i ;

	while( n < size )
	{
		c = GetChar() ;
		if( c == -1 )
		{
		    continue;
		}
		ch = c;
		//dprintf("%x ",ch);
		if( ch == KEYCODE_LF )
		{
		    #ifdef USE_LF
		    *--p = 0 ;
		    n-- ;
		    #endif // USE_LF
		    break ;
		}
		else if( ch == KEYCODE_CR )
		{
		    *p = 0 ;
#ifndef CONFIG_HISTORY_KEYIN
		    n-- ;
#else //#ifdef CONFIG_HISTORY_KEYIN
		if(n!=0)
		{
			strcpy(history_cmd[hist_save_idx], buffer);
			(hist_save_idx >= HISTORY_CMD_ELN-1) ? hist_save_idx=0 : hist_save_idx++;
			hist_see_idx=hist_save_idx;
		}

#endif
		    break ;
		}
		else if( ch == KEYCODE_BS_7F )
		{
		    if( p != buffer )
		    {
			    p-- ;
			    n-- ;
							if(EchoFlag)
							{
								PutChar(KEYCODE_BS);
								PutChar(' ');
								PutChar(KEYCODE_BS);
							}
		    }
		}
		else if( ch == KEYCODE_TAB )
		{
		    for( i=0 ; i < TAB ; i++ )
		    {
			    *p++ = ' ' ;
			    n++ ;
							if(EchoFlag) PutChar(' ');
		    }
		}
#ifdef CONFIG_HISTORY_KEYIN
		else if( ch == KEYCODE_ESC )
		{				
			if( GetChar()!= KEYCODE_BRACKET)		continue;
			
			c=GetChar();
			
			unsigned int  vk= ( KEYCODE_ESC <<24) | (KEYCODE_BRACKET<<16) | (c<<8) | 0 ;
			
			if( vk==KEYCODE_VKUP)
			{				
				(hist_see_idx==0) ? hist_see_idx= HISTORY_CMD_ELN-1 : hist_see_idx--;
			}
			else if(vk== KEYCODE_VKDOWN) 
			{
				(hist_see_idx==HISTORY_CMD_ELN-1) ? hist_see_idx= 0 : hist_see_idx++;
					
			}
			else 
				continue;

			//clear
			for(i=0;i<n;i++)
			{
				PutChar(KEYCODE_BS);
				PutChar(' ');
				PutChar(KEYCODE_BS);
			}
			strcpy(buffer, history_cmd[hist_see_idx]);
			n= strlen(buffer);
			p=buffer+n;
			dprintf("%s", buffer);
			
		}
#endif
		else
		{
		    *p++ = ch ;
		    n++ ;
					if(EchoFlag) PutChar(ch);
		}
	}
}

/*
---------------------------------------------------------------------------
; int GetArgc(const char* string)
:   Description:
;			Get argument number for a input string
;			Input string separate by space key
;   Input:
;	    string : pointer to buffer
;   Output:
;			argument number
;   Destroy:
---------------------------------------------------------------------------
*/
int GetArgc( const char* string )
{
	int			argc;
	char*		p ;

	argc = 0 ;
	p = (char* )string ;
	while( *p )
	{
		if( *p != ' '  &&  *p )
		{
			argc++ ;
			while( *p != ' '  &&  *p ) p++ ;
			continue ;
		}
		p++ ;
	}
	if (argc >= MAX_ARGV) argc = MAX_ARGV - 1;
	return argc ;
}

/*
---------------------------------------------------------------------------
; int GetArgv(const char* string)
:   Description:
;			Get argument for a input string
;			Input string separate by space key
;   Input:
;	    string : pointer to buffer
;   Output:
;			argument string array, argv[0], argv[1],...
;			argument string end by zero character
;   Destroy:
---------------------------------------------------------------------------
*/
char** GetArgv(const char* string)
{
	char*			p ;
	int				n;

	n = 0 ;
	memset( ArgvArray, 0, MAX_ARGV*sizeof(char *) );
	p = (char* )string ;
	while( *p )
	{
		ArgvArray[n] = p ;
		while( *p != ' '  &&  *p ) p++ ;
		*p++ = '\0';
		while( *p == ' '  &&  *p ) p++ ;
		n++ ;
		if (n == MAX_ARGV) break;
	}
	return (char** )&ArgvArray ;
}

/*
---------------------------------------------------------------------------
; int Hex2Val(const char* HexStr, unsigned long* PVal)
:   Description:
;			Convert hex string to value
;   Input:
;	    HexStr : pointer to hex string
;	    PVal   : Address to converted value
;   Output:
;			return:	 0: false, 1: success
;			PVal   : if convert success ,pointer to converted value
;   Destroy:
---------------------------------------------------------------------------
*/
int Hex2Val(char* HexStr, unsigned long* PVal)
{
	unsigned char				*ptrb;
	unsigned long				sum,csum;
	register unsigned char		c,HexValue;

	sum = csum = 0;
	ptrb = HexStr;
	while(c=*ptrb++)
	{
		if( c>='0' && c<='9')		HexValue = c - '0';
		else if( c>='a' && c<='f')	HexValue = c - 'a' + 10;
		else if( c>='A' && c<='F')  HexValue = c - 'A' + 10;
		else return FALSE;				// character invalid
		sum = csum*16 + HexValue;
		if(sum < csum) return FALSE;	// value overlow
		csum = sum;
	}
	*PVal = csum;
	return TRUE;
}
