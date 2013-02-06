
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : Core types used in source tree.
* Abstract :  Types such as uint32 are not defined here. They are included via <sys/types.h>
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)               
* $Id: types.h,v 1.1 2007/12/04 11:57:45 joeylin Exp $
* $Log: types.h,v $
* Revision 1.1  2007/12/04 11:57:45  joeylin
* add hardware NAT feature
*
* Revision 1.7  2005/04/29 13:34:59  yjlou
* +: add NIPQUAD2STR2 to support formatted IP address output.
*
* Revision 1.6  2004/12/17 13:11:10  yjlou
* +: add NIPQUAD2() macro to support print non-memory type IP address
*
* Revision 1.5  2004/08/09 02:15:55  chhuang
* *: code re-architecture--phase 1
*
* Revision 1.4  2004/03/19 13:13:35  cfliu
* Reorganize ROME driver local header files. Put all private data structures into different .h file corrsponding to its layering
* Rename printf, printk, malloc, free with rtlglue_XXX prefix
*
* Revision 1.3  2004/03/05 07:46:03  cfliu
* fix htonl, htons,..etc compile redefinition problem..
*
* Revision 1.2  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.8  2004/02/24 04:13:22  cfliu
* no message
*
* Revision 1.7  2004/02/19 02:20:04  chenyl
* *** empty log message ***
*
* Revision 1.6  2004/02/19 02:14:30  chenyl
* *** empty log message ***
*
* Revision 1.5  2004/02/18 13:54:37  chhuang
* *** empty log message ***
*
* Revision 1.4  2003/10/21 08:07:02  tony
* add DoS attack log routine
*
* Revision 1.3  2003/09/30 06:07:50  orlando
* check in RTL8651BLDRV_V20_20030930
*
* Revision 1.10  2003/09/04 17:48:11  cfliu
* fix GET_UINT32_BIG_ENDIAN_UNALIGNED macro
*
* Revision 1.9  2003/08/15 07:06:53  hiwu
* add __linux__ compiler flag
*
* Revision 1.8  2003/08/11 08:34:39  hiwu
* add __linux__
*
* Revision 1.7  2003/05/19 10:06:53  cfliu
* Undo change to ISSET. For  ISSET( a, X) , any bit in X set returns TRUE
*
* Revision 1.6  2003/05/17 08:47:31  cfliu
* Change definition of ISSET()
*
* Revision 1.5  2003/04/30 15:34:18  cfliu
* add macros from rtl_types.h and add new macros to solve alignment, endian issues
*
* Revision 1.4  2003/04/02 15:39:39  cfliu
* Move ASSERT_ISP and ASSERT_ISR to rtl_depend.h
*
* Revision 1.3  2003/03/13 10:29:55  cfliu
* Remove unused symbols.
* Add ASSERT_CSP/ISR
*
* Revision 1.2  2002/04/29 10:04:28  hiwu
* remove ipaddr typedefine
*
* Revision 1.1  2002/04/29 05:45:25  cfliu
* Moved to new source tree
*
* Revision 1.5  2002/04/25 03:59:34  cfliu
* no message
*
* Revision 1.4  2002/04/04 10:47:29  cfliu
* Use 'indent' tool to unify coding style....
* The options used could be found on /product/cygtest/.indent.pro
*
* Revision 1.3  2002/03/28 06:46:54  cfliu
* no message
*
* Revision 1.2  2002/03/25 11:36:28  cfliu
* no message
*
* Revision 1.1  2002/03/21 05:44:07  cfliu
* Primitive data types changed to uint32, uint16, ...etc.
*
*/
#ifndef _TYPES_H
#define _TYPES_H

#include "rtl_types.h"


#ifndef NIPQUAD
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#endif	

#ifndef NIPQUAD2STR
/* Example: printf( "intIp="NIPQUAD2STR"\n", NIPQUAD2(ntohl(intIp)) ); */
#define NIPQUAD2STR "%d.%d.%d.%d"
#define NIPQUAD2STR2 "%3d.%3d.%3d.%3d" /* aligned format */
#endif
#ifndef NIPQUAD2
#define NIPQUAD2(addr) \
	((addr&0xff000000)>>24), \
	((addr&0x00ff0000)>>16), \
	((addr&0x0000ff00)>> 8), \
	((addr&0x000000ff)>> 0)
#endif	

#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))


//Get/Set host-order value from/to little endian buffer. 
//first byte ptr points to Little endian system's LSB.
#define GET_UINT16_LITTLE_ENDIAN_UNALIGNED( ptr )  ((uint16)  (*(((uint8*)(ptr))+1)<<8) | *((uint8*)(ptr)))
#define SET_UINT16_LITTLE_ENDIAN_UNALIGNED(  u16value, ptr )	do{\
		*((uint8*)(ptr)) =  (uint8)((u16value));/* get and set LSB */\
		*(((uint8*)(ptr))+1) =  (uint8)((u16value)>>8);/*get and set MSB*/\
}while(0)
#define GET_UINT32_LITTLE_ENDIAN_UNALIGNED( ptr )  ((uint32)  (*(((uint8*)(ptr))+3)<<24)|(*(((uint8*)(ptr))+2)<<16)|(*(((uint8*)(ptr))+1)<<8) | *((uint8*)(ptr)))
#define SET_UINT32_LITTLE_ENDIAN_UNALIGNED(  u32value, ptr )	do{\
		*((uint8*)(ptr)) =  (uint8)((u32value));/* get and set LSB */\
		*(((uint8*)(ptr))+1) =  (uint8)((u32value)>>8);\
		*(((uint8*)(ptr))+2) =  (uint8)((u32value)>>16);\
		*(((uint8*)(ptr))+3) =  (uint8)((u32value)>>24);/*get and set MSB*/\
}while(0)


#if 1
//Get/Set host-order value from/to big endian buffer. 
//first byte ptr points to Big endian system's the MSB
#define GET_UINT16_BIG_ENDIAN_UNALIGNED( ptr )  ((uint16)  (*((uint8*)(ptr))<<8) | *(((uint8*)(ptr))+1))
#define SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )	do{\
	*((uint8*)(ptr)) = (uint8)(((uint16)u16value)>>8);  /*get and set MSB*/\
	*(((uint8*)(ptr))+1) = (uint8)(u16value); /* get and set LSB */\
}while(0)
#define GET_UINT32_BIG_ENDIAN_UNALIGNED( ptr )  ((uint32)  (*((uint8*)(ptr))<<24)|(*(((uint8*)(ptr))+1)<<16)|(*(((uint8*)(ptr))+2)<<8) | *(((uint8*)(ptr))+3))
#define SET_UINT32_BIG_ENDIAN_UNALIGNED( u32value, ptr )	do{\
	*((uint8*)(ptr)) =  (uint8)((u32value)>>24);/* get and set MSB */\
	*(((uint8*)(ptr))+1) =  (uint8)((u32value)>>16);\
	*(((uint8*)(ptr))+2) =  (uint8)((u32value)>>8);\
	*(((uint8*)(ptr))+3) =  (uint8)((u32value));/*get and set LSB*/\
}while(0)
#else
#define GET_UINT16_BIG_ENDIAN_UNALIGNED( ptr ) 			(*((uint16 *)(ptr)))
#define SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr ) 	(*((uint16 *)(ptr))) = (uint16)(u16value)
#define GET_UINT32_BIG_ENDIAN_UNALIGNED( ptr )			(*((uint32 *)(ptr)))
#define SET_UINT32_BIG_ENDIAN_UNALIGNED( u32value, ptr )	(*((uint32 *)(ptr))) = (uint32)(u32value)
#endif


#ifdef _LITTLE_ENDIAN

#ifndef __linux__
	#define ntohs(x)   (swaps16(x))
	#define ntohl(x)   (swapl32(x))
	#define htons(x)   (swaps16(x))
	#define htonl(x)   (swapl32(x))
#endif /* __linux__ */	
	#define NTOHL(d) ((d) = ntohl((d)))
	#define NTOHS(d) ((d) = ntohs((uint16)(d)))
	#define HTONL(d) ((d) = htonl((d)))
	#define HTONS(d) ((d) = htons((uint16)(d)))

	#define GET_UINT16_UNALIGNED( ptr )  				GET_UINT16_LITTLE_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT16_UNALIGNED(  u16value, ptr )		SET_UINT16_LITTLE_ENDIAN_UNALIGNED(  u16value, ptr )
	#define GET_UINT32_UNALIGNED( ptr )  				GET_UINT32_LITTLE_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT32_UNALIGNED(  u32value, ptr )		SET_UINT32_LITTLE_ENDIAN_UNALIGNED(  u32value,ptr)
	#define PKTGET_UINT16_UNALIGNED( ptr )  				GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT16_UNALIGNED(  u16value, ptr 	)	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define PKTGET_UINT32_UNALIGNED( ptr )  				GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT32_UNALIGNED(  u32value, ptr )	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)

#else /*Big endian system */

/* chhuang */
#ifdef __linux__
	#undef ntohs
	#undef ntohl
	#undef htons
	#undef htonl
	#define ntohs(x)   (x)
	#define ntohl(x)   (x) 
	#define htons(x)   (x)
	#define htonl(x)   (x)
	#define _LINUX_BYTEORDER_GENERIC_H
#endif /* __linux__ */	
	#define NTOHL(d) ((d) = ntohl(d))
	#define NTOHS(d) ((d) = ntohs((uint16)(d)))
	#define HTONL(d) ((d) = htonl(d))
	#define HTONS(d) ((d) = htons((uint16)(d)))
	#define GET_UINT16_UNALIGNED( ptr )  			GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT16_UNALIGNED(  u16value, ptr )	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define GET_UINT32_UNALIGNED( ptr )  			GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT32_UNALIGNED(  u32value, ptr )	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)
	#define PKTGET_UINT16_UNALIGNED( ptr )  			GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT16_UNALIGNED(  u16value, ptr)	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define PKTGET_UINT32_UNALIGNED( ptr )  			GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT32_UNALIGNED(  u32value, ptr)	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)	


#endif




#endif

