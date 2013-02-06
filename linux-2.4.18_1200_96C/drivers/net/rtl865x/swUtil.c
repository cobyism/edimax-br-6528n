/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/swUtil.c,v 1.2 2009/01/13 13:36:03 bradhuang Exp $
*
* Abstract: Utilities for switch core drivers.
*
* $Author: bradhuang $
*
* $Log: swUtil.c,v $
* Revision 1.2  2009/01/13 13:36:03  bradhuang
* modify include path for new tool chain
*
* Revision 1.1  2007/08/06 10:04:52  root
* Initial revision
*
* Revision 1.4  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:25  alva
* initial import for add TFTP server
*
* Revision 1.3  2004/07/14 02:16:09  yjlou
* +: add '#ifdef FAT_CODE' to remove un-used functions
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:55  tony
*  initial loader tree 
*
* Revision 1.2  2003/06/19 05:28:51  danwu
* replace s2Mac() with strToMac()
*
* Revision 1.1.1.1  2003/05/07 08:16:06  danwu
* no message
*
* ---------------------------------------------------------------
*/

#include <linux/string.h>
#include "rtl_types.h"

/*
 * Convert an ASCII string to a
 * binary representation of mac address
*/
int32 strToMac(uint8 *pMac, int8 *pStr)
{
    int8 *ptr;
    uint32 k;
       
    bzero(pMac,6);
    ptr = pStr;
    for(k=0;*ptr;ptr++)
    {
        if( (*ptr==':') || (*ptr=='-') )
            k++;
        else if( ('0'<=*ptr) && (*ptr<='9') )
            pMac[k]=(pMac[k]<<4)+(*ptr-'0');
        else if( ('a'<=*ptr) && (*ptr<='f') )
            pMac[k]=(pMac[k]<<4)+(*ptr-'a'+10);
        else if( ('A'<=*ptr) && (*ptr<='F') )
            pMac[k]=(pMac[k]<<4)+(*ptr-'A'+10);
        else
            break;
    }
    if(k!=5)
        return -1;
    
    return 0;
}

