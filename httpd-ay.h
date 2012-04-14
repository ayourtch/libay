
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

#ifndef _APP_HTTP_H_
#define _APP_HTTP_H_

#include "dbuf-ay.h"
#include "sock-ay.h"

enum {
  HTTP_REQ_INCOMPLETE = 0,
  HTTP_REQ_UNKNOWN,
  HTTP_REQ_GET,
  HTTP_REQ_POST,
  HTTP_REQ_CONNECT,
  HTTP_L7_INIT,
  HTTP_L7_SHOWTIME,
  HTTP_L7_READ_POST_DATA,
};

extern char *http_appdata_sig;

struct appdata_http_t_tag;

typedef int (*http_handler_func_t)(dbuf_t *dad, dbuf_t *dh, dbuf_t *dd);
typedef http_handler_func_t (*http_dispatcher_func_t)(dbuf_t *dad);

typedef struct appdata_http_t_tag {
  int l7state;			/* l7 state */
  dbuf_t *rdb;			/* request dbuf - the data that is sent from the browser */
  char *http_path;              /* requested path */
  char *http_querystring;       /* query string */
  char *http_referer;
  char *content_type;
  int http_11;                  /* is it http_1.1 ? */
  uint32_t post_content_length;
  dbuf_t *post_content_buf;
  uint32_t post_content_got_length;
  http_dispatcher_func_t dispatcher;
  char *lua_handler_name;
  void *L; // Lua state, if needed
} appdata_http_t;

dbuf_t *alloc_appdata_http(int idx);
appdata_http_t *http_dbuf_get_appdata(dbuf_t *d);



int ev_http_read(int idx, dbuf_t * d, void *u_ptr);
int ev_http_connect_read(int idx, dbuf_t * d, void *u_ptr);

/* the one that is exported */
int http_start_listener(char *addr, int port, http_dispatcher_func_t dispatcher);


#endif
