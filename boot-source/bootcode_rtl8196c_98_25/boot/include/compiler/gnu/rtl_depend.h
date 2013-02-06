/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
*
* Abstract: Header file for gnu platform.
*
* ---------------------------------------------------------------
*/

#ifndef _RTL_DEPEND_H_
#define _RTL_DEPEND_H_


/*
 * many standard C library API references "size_t"
 * cygwin's stdio.h and ghs's stdio.h will test
 * _SIZE_T symbol before typedef ...
 */
#ifndef _SIZE_T
#define _SIZE_T
	typedef unsigned int size_t;
#endif /* _SIZE_T */

#define RTL_EXTERN_INLINE
#define RTL_STATIC_INLINE   static __inline__

#define malloc(x) osk_malloc(x)
#define free(x) osk_free(x)

//Our own assert macro, when condition not met, forces cygwin to create a stackdump file.
#undef assert
#define assert(x)\
if (!(x)) { \
        int *p=NULL;\
        printf("\nAssertion fail at File %s, In function %s, Line number %d:\nExpression '%s'", __FILE__, __FUNCTION__, __LINE__, #x);\
	*p = 123;\
	while(1){};\
}\


#endif   /* _RTL_DEPEND_H_ */
