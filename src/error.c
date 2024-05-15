#include <stdarg.h>
#include <stdio.h>

#include "error.h"

// Currently reporting an error just prints it to standard error.
void report_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}
