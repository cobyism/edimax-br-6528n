/* util.c ....... error message utilities.
 *                C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: util.c,v 1.1 2005/03/20 05:06:37 rex Exp $
 */

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "pptp"
#endif

/* implementation of log_string, defined as extern in util.h */
char *log_string = "anon";

static void open_log(void) __attribute__ ((constructor));
static void close_log(void) __attribute__ ((destructor));

static void open_log(void) {
  openlog(PROGRAM_NAME, LOG_PID, LOG_DAEMON);
}
static void close_log(void) {
  closelog();
}

#define MAKE_STRING(label) 				\
va_list ap;						\
char buf[256], string[256];				\
va_start(ap, format);					\
vsnprintf(buf, sizeof(buf), format, ap);		\
snprintf(string, sizeof(string), "%s %s[%s:%s:%d]: %s",	\
	 log_string, label, func, file, line, buf);	\
va_end(ap)

void _log(char *func, char *file, int line, char *format, ...) {
  MAKE_STRING("log");
  syslog(LOG_NOTICE, "%s", string);
}

void _warn(char *func, char *file, int line, char *format, ...) {
  MAKE_STRING("warn");
  fprintf(stderr, "%s\n", string);
  syslog(LOG_WARNING, "%s", string);
}

void _fatal(char *func, char *file, int line, char *format, ...) {
  MAKE_STRING("fatal");
  fprintf(stderr, "%s\n", string);
  syslog(LOG_CRIT, "%s", string);
  exit(1);
}

/* connect a file to a file descriptor */
int file2fd(const char *path, const char *mode, int fd) {
  int ok = 0;
  FILE *file = NULL;

  file = fopen(path, mode);
  if (file != NULL && dup2(fileno(file), fd) != -1)
    ok = 1;

  if (file) fclose(file);

  return ok;
}
