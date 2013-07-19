#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"

int tuni;


int tun_read_ev(int idx, dbuf_t *d, void *p) {
  printf("Got packet, sending back!\n");
  return sock_send_data(tuni, d);
}


int main(int argc, char *argv[]) {
  int timeout;
  sock_handlers_t *hdl;

  set_debug_level(DBG_GLOBAL, 1000);

  tuni = attach_tun_interface(NULL);
  hdl = cdata_get_handlers(tuni);
  hdl->ev_read = tun_read_ev;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
    debug(0,0, ".");
  }
}


