
#ifndef cle_utility_h
#define cle_utility_h


#include "cle_struct.h"


int32 cle_strToToken(int8 *inputStr,cle_tokenBuffer_t *tokenBuf);
#ifndef strtok_r /* Compiler might have build-in. */
int8 *strtok_r(int8 * s, const int8 * delim, int8 ** save_ptr);
#endif
#endif
