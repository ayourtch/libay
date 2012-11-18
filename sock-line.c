/**************************************************************************
*
*  Copyright © 2012 Andrew Yourtchenko, ayourtch@gmail.com.
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

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <poll.h>
#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip6.h>

#include "lib_sock_intern.h"
#include "dbuf-ay.h"
#include "debug-ay.h"
#include "sock-line.h"


#define LINE_SIZE 2000

typedef struct line_data_t {
  dbuf_t *line;
  int upstream_idx;
} line_data_t;



int line_transport_read_ev(int idx, dbuf_t *d, void *p) {
  int i;
  int ret = d->dsize;
  line_data_t *cli = cdata[idx].private;

  for(i=0; i<d->dsize; i++) {
    char c = d->buf[i];
    if (c == '\r') {
      ret = ev_read(cli->upstream_idx, cli->line, p);
      dunlock(cli->line);
      cli->line = dalloc(LINE_SIZE);
      cli->line->buf[0] = 0;
      cli->line->dsize = 0;
    } else if (c != '\n') {
      cli->line->buf[cli->line->dsize++] = c;
      cli->line->buf[cli->line->dsize] = 0;
    }
  }
  return ret;
}

int 
line_recv(int idx, dbuf_t *d, void *private) 
{
  debug(DBG_GLOBAL, -1, "line recv called! Error!");
  return 0;
}

int 
line_send(int idx, dbuf_t *d, void *private) 
{
  int child_idx = (int) private;
  return sock_send_data(child_idx, d);
}



int attach_line(int iosk) 
{
  sock_handlers_t *hdl;
  int idx;
  line_data_t *cli = malloc(sizeof(*cli));
  memset(cli, 0, sizeof(*cli));
  cli->line = dalloc(LINE_SIZE);
  

  // downstream setup
  cdata[iosk].private = cli;
  hdl = cdata_get_handlers(iosk); 
  hdl->ev_read = line_transport_read_ev;

  // upstream setup
  idx = sock_make_new(-2, (void*) iosk);
  sock_set_hooks(idx, line_send, line_recv);

  // link upstream to downstream
  cli->upstream_idx = idx;

  return idx;
}


