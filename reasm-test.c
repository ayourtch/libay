#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"

int stdi;

int stdin_read_ev(int idx, dbuf_t *d, void *p) {
  if (d->buf[0] == 'q' ) { 
    detach_stdin();
    printf("\n\n\n");
    exit(1);
  }
  return d->dsize;
}

int main(int argc, char *argv[]) {
  int sock;
  int timeout;
  htable_t *ht;
  int res;
  dbuf_t *d;
  sock_handlers_t *hdl;

  set_debug_level(DBG_GLOBAL, 0);

  stdi = attach_stdin(1); 
  hdl = cdata_get_handlers(stdi); 
  hdl->ev_read = stdin_read_ev;
  printf("Press q to quit\n");

  timeout = 1000;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}


