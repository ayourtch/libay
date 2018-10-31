
/**************************************************************************
*
*  Copyright © 2008-2009 Andrew Yourtchenko, ayourtch@gmail.com.
*
*  Permission is hereby granted, free of charge, to any person obtaining 
* a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation 
* the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom 
* the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included 
* in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
* OR OTHER DEALINGS IN THE SOFTWARE. 
*
*****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/errno.h>
#include <execinfo.h>

#include "debug-ay.h"


struct debug_type DBG_ALL_S = { "notify", "NTFY", 0, NULL };

debug_type_t DBG_ALL = &DBG_ALL_S;

struct debug_type DBG_GLOBAL_S = { "global", "GLBL", 0, &DBG_ALL_S };
debug_type_t DBG_GLOBAL = &DBG_GLOBAL_S;

//struct debug_type DBG_SSL_S = { "ssl", "SSLD", 0, &DBG_MEMORY_S };
//debug_type_t DBG_SSL = &DBG_SSL_S;




/**
 * @defgroup debug Debugging facilities
 */

 /*
    @{ 
  */

int
set_debug_level(debug_type_t type, int level)
{
  int ret;
  ret = type->level;
  type->level = level;
  return ret;
}

int
is_debug_on(debug_type_t type, int level)
{
  return(type->level >= level);
}

static int debug_needs_erase = 0;
static debug_redraw_cb_t *debug_redraw_cb = NULL;

void debug_will_need_redraw(debug_redraw_cb_t *cb) {
  debug_needs_erase = 1;
  debug_redraw_cb = cb;
}

void debug_redraw_if_needed() {
  if (debug_redraw_cb) {
    (*debug_redraw_cb)(debug_needs_erase);
    debug_needs_erase = 1;
  }
}


static int show_timestamp_in_debugs = 1;

void 
debug_show_timestamp(int show) {
  show_timestamp_in_debugs = show;
}

/**
 * Routine to print the debug messages with timestamp.
 *
 * @param type the type of the debug
 * @param level and the level of the debug message (0=always shown)
 * @param fmt printf-style format string, followed by additional parameter
 *
 * @return the number of characters printed from the debug message
 * @see debug_dump
 */
int
debug(debug_type_t type, int level, const char *fmt, ...)
{
  va_list ap;
  int result = 0;
  //struct tm *tm;
  struct timeval tv;
  char date_buf[256];

  if (type == 0) {
    type = DBG_ALL;
  }

  if(is_debug_on(type, level)) {

    if (show_timestamp_in_debugs) {
      gettimeofday(&tv, NULL);
    } else {
      memset(&tv, 0, sizeof(struct timeval));
    }
    asctime_r(localtime(&tv.tv_sec), date_buf);
    date_buf[strlen(date_buf) - 6] = 0;

    if(debug_needs_erase) {
      fprintf(stderr, "\r");
      debug_needs_erase = 0;
    }


    fprintf(stderr, "%s.%06d LOG-%4s-%04d: ", date_buf, (int) tv.tv_usec,
            type->id, level);
    va_start(ap, fmt);
    result = vfprintf(stderr, fmt, ap);
    /* CONSOLEXXX

    if(need_console_debugs()) {
      console_printf("\n%s.%06d LOG-%4s-%04d: ", date_buf, (int) tv.tv_usec,
                     type->id, level);
      console_vprintf(fmt, ap);
      console_putstr("\n");
    }
    */
    va_end(ap);
    fprintf(stderr, "\n");
  }
  return result;
}

/**
 * Print the hexdump of a the memory if the debug level is appropriate
 *
 * @see debug
 *
 * @param type the type of the debug
 * @param level the level at which to print this dump
 * @param addr address of the block of memory to dump
 * @param len how big is the block of memory to dump
 * @return always returns 1
 */

int
debug_dump(debug_type_t type, int level, void *addr, int len)
{
  char tohex[] = "0123456789ABCDEF";
  int i = 0;
  unsigned char *pc = addr;

  char buf0[32];                // offset
  char buf1[64];                // hex
  char buf2[64];                // literal

  char *pc1;
  char *pc2;

  // if (debuglevel < level) {
  if(!is_debug_on(type, level)) {
    return 0;
  }

  while(--len >= 0) {
    if(i % 16 == 0) {
      sprintf(buf0, "%08x", i);
      buf1[0] = 0;
      buf2[0] = 0;
      pc1 = buf1;
      pc2 = buf2;
    }
    *pc1++ = tohex[*pc >> 4];
    *pc1++ = tohex[*pc & 15];
    *pc1++ = ' ';

    if(*pc >= 32 && *pc < 127) {
      *pc2++ = *pc;
    } else {
      *pc2++ = '.';
    }
    i++;
    pc++;
    if(i % 16 == 0) {
      *pc1 = 0;
      *pc2 = 0;
      debug(type, level, "%s:   %s  %s", buf0, buf1, buf2);
    }

  }
  if(i % 16 != 0) {
    while(i % 16 != 0) {
      *pc1++ = ' ';
      *pc1++ = ' ';
      *pc1++ = ' ';
      *pc2++ = ' ';
      i++;
    }
    *pc1 = 0;
    *pc2 = 0;
    debug(type, level, "%s:   %s  %s", buf0, buf1, buf2);
  }
  return 1;
}

/**
 * Simplistic error check function for fatal errors - checks 
 * the supplied value to be nonzero. 
 * If it is negative, print the supplied message, 
 * current string for errno,and die.
 *
 * @param x the value to test to be nonnegative
 * @param msg the error message to print
 */

void
notminus(int x, char *msg)
{
  if(x < 0) {
    debug(DBG_GLOBAL, 0, "ERROR: %s, system error: %s", msg, strerror(errno));
    exit(1);
  }
}

/** 
 * Put the backtrace into the memory
 * @param bt pointer to the backtrace array
 */

void
get_backtrace(backtrace_t * bt)
{
  bt->size = backtrace(bt->addresses, sizeof(bt->addresses) / sizeof(void *));
}

/**
 * return the symbolic name of the function pointer, the caller needs to free the memory!
 * param fptr function pointer
 */
char *get_symbol_name(void *fptr)
{
  char **strings;
  char *str;
  strings = backtrace_symbols(&fptr, 1);
  str = strdup(strings[0]);
  free(strings);
  return str;
}

/**
 * Obtain a backtrace and print it to stdout. 
 * Borrowed from http://www.gnu.org/software/libtool/manual/libc/Backtraces.html 
 */
void
print_backtrace_t(debug_type_t debugtype, int debuglevel, backtrace_t * bt)
{
  char **strings;
  size_t i;

  strings = backtrace_symbols(bt->addresses, bt->size);
  debug(debugtype, debuglevel, "backtrace of %zd stack frames.", bt->size);
  for(i = 0; i < bt->size; i++)
    debug(debugtype, debuglevel, "%d: %s", i, strings[i]);
  free(strings);
}

void
print_backtrace_t_hex(debug_type_t debugtype, int debuglevel, backtrace_t * bt)
{
  int i;

  debug(debugtype, debuglevel, "backtrace of %zd stack frames.", bt->size);
  for(i = 0; i < bt->size; i++)
    debug(debugtype, debuglevel, "%d: %p", i, bt->addresses[i]);
}


void
print_backtrace(void)
{
  backtrace_t bt;

  get_backtrace(&bt);
  debug(DBG_GLOBAL, 0, "Current backtrace:");
  print_backtrace_t(0, 0, &bt);
}

/*@}*/
