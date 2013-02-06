#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

int scanf(const char *format, ...) {
  int n;
  va_list arg_ptr;
  va_start(arg_ptr, format);
  n=vfscanf(format,arg_ptr);
  va_end (arg_ptr);
  return n;
}
