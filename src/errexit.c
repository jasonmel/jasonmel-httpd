/* errexit.c - errexit */

// #include <varargs.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * errexit - print an error message and exit
 *------------------------------------------------------------------------
 */
/*VARARGS1*/
int errexit(char *format, ...)
{
  va_list  args;

  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  exit(1);
}
