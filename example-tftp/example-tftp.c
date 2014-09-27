#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"

int tftpi;


int tftp_read_ev(int idx, dbuf_t *d, void *p) {
  printf("Got packet, sending back!\n");
  return -1;
}


int main(int argc, char *argv[]) {
  int timeout;
  sock_handlers_t *hdl;

  set_debug_level(DBG_GLOBAL, 1000);

  tftpi = bind_udp_listener(69);
  hdl = cdata_get_handlers(tftpi);
  hdl->ev_read = tftp_read_ev;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}


