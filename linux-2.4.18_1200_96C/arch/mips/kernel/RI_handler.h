#ifndef __RI_HANDLER_H__
#define __RI_HANDLER_H__

#ifdef  __cplusplus
extern "C" {
#endif


#ifndef Branch_t
typedef enum {
  BRANCH_T_NONE,
  BRANCH_T_OFFSET,
  BRANCH_T_TARGET,
  BRANCH_T_REGISTER
} Branch_t;
#endif /* Branch_t */

typedef signed int LONG;
typedef signed int INT;
typedef unsigned int ULONG;
typedef unsigned int UINT;

#ifdef  __cplusplus
}
#endif


#endif
