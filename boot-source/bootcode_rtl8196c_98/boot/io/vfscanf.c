#include <stdarg.h>
#include <sys/types.h>
#include <stdlib.h>

#include <unistd.h>

int vfscanf(const char *format, va_list arg_ptr)
{
  return __v_scanf(format,arg_ptr);
}
