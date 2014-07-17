#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"

int tapi;
int pcapi;


int tap_read_ev(int idx, dbuf_t *d, void *p) {
  return sock_send_data(pcapi, d);
}

int pcap_read_ev(int idx, dbuf_t *d, void *p) {
  return sock_send_data(tapi, d);
}

int main(int argc, char *argv[]) {
  int timeout;
  sock_handlers_t *hdl;

 //  set_debug_level(DBG_GLOBAL, 1000);

  tapi = attach_tap_interface(NULL);
  pcapi = attach_pcap(argv[1]);

  hdl = cdata_get_handlers(tapi);
  hdl->ev_read = tap_read_ev;

  hdl = cdata_get_handlers(pcapi);
  hdl->ev_read = pcap_read_ev;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}


