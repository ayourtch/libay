
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

#ifndef __LIB_DEBUG_H__
#define __LIB_DEBUG_H__

#define soft_assert(x) { if (!(x)) { debug(DBG_GLOBAL, 1, "Soft assert failed at %s:%d: %s", __FILE__, __LINE__, __STRING(x)); } }

typedef struct debug_type {
  char *label;
  char *id;
  int level;
  struct debug_type *next;
} *debug_type_t;

extern struct debug_type DBG_GLOBAL_S;
extern debug_type_t DBG_GLOBAL;
extern debug_type_t DBG_SSL;
extern debug_type_t DBG_TIMERS;
extern debug_type_t DBG_REASM;

/**
 * structure to store the backtrace
 */
typedef struct {
  int size;
  void *addresses[10];
} backtrace_t;

typedef void debug_redraw_cb_t(int idlecall);

void debug_will_need_redraw(debug_redraw_cb_t *cb);
void debug_redraw_if_needed();


void debug_show_timestamp(int show);
int debug(debug_type_t type, int level, const char *fmt, ...);
int set_debug_level(debug_type_t type, int level);
int get_debug_level();
int is_debug_on(debug_type_t type, int level);
int debug_dump(debug_type_t type, int level, void *addr, int len);
char *get_symbol_name(void *fptr);
void print_backtrace(void);
void print_backtrace_t(debug_type_t debugtype, int debuglevel, backtrace_t * bt);
void get_backtrace(backtrace_t * bt);
void notminus(int x, char *msg);




#endif
