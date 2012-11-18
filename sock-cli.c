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
#include "lists-ay.h"
#include "os-ay.h"
#include "sock-pcap-ay.h"
/*
#include "timers-ay.h"
*/


#define LINE_SIZE 2000

typedef struct cli_data_t {
  dbuf_t *cmd;
  int upstream_idx;
} cli_data_t;



int cli_transport_read_ev(int idx, dbuf_t *d, void *p) {
  int i;
  int ret = d->dsize;
  dbuf_t *dout = dalloc(d->dsize);
  cli_data_t *cli = cdata[idx].private;

  for(i=0; i<d->dsize; i++) {
    char c = d->buf[i];
    if (c == 0x7f) {
      if(cli->cmd->dsize > 0) {
	cli->cmd->buf[--cli->cmd->dsize] = 0;
        dappendfill(dout, 8, 1);
        dappendfill(dout, ' ', 1);
        dappendfill(dout, 8, 1);
      }
    } else if ((c == '\r') || (c == '\n') || (c == '\x03')) {
      dappendfill(dout, '\n', 1);
      sock_send_data(idx, dout);
      dout = NULL;
      if (c == '\x03') {
        cli->cmd->buf[cli->cmd->dsize++] = c;
        cli->cmd->buf[cli->cmd->dsize] = 0;
      }
      ret = ev_read(cli->upstream_idx, cli->cmd, p);
      dunlock(cli->cmd);
      cli->cmd = dalloc(LINE_SIZE);
      cli->cmd->buf[0] = 0;
      cli->cmd->dsize = 0;
      if (i < d->dsize - 1) {
        dout = dalloc(d->dsize - i);
      } 
    } else {
      dappendfill(dout, c, 1);
      cli->cmd->buf[cli->cmd->dsize++] = c;
      cli->cmd->buf[cli->cmd->dsize] = 0;
    }
  }
  if (dout) {
    if (dout->dsize > 0) { 
      sock_send_data(idx, dout);
    } else {
      dunlock(dout);
    }
  }
  return ret;
}

int 
cli_recv(int idx, dbuf_t *d, void *private) 
{
  debug(DBG_GLOBAL, -1, "CLI recv called! Error!");
  return 0;
}

int 
cli_send(int idx, dbuf_t *d, void *private) 
{
  int child_idx = (int) private;
  return sock_send_data(child_idx, d);
}



int attach_cli(int iosk) 
{
  sock_handlers_t *hdl;
  int idx;
  cli_data_t *cli = malloc(sizeof(*cli));
  memset(cli, 0, sizeof(*cli));
  cli->cmd = dalloc(LINE_SIZE);
  

  // downstream setup
  cdata[iosk].private = cli;
  hdl = cdata_get_handlers(iosk); 
  hdl->ev_read = cli_transport_read_ev;

  // upstream setup
  idx = sock_make_new(-2, (void*) iosk);
  sock_set_hooks(idx, cli_send, cli_recv);

  // link upstream to downstream
  cli->upstream_idx = idx;

  return idx;
}


