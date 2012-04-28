#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"

int pcap;

int pcap_read_ev(int idx, dbuf_t *d, void *p) {
  return sock_send_data(pcap, d);
}


int main(int argc, char *argv[]) {
  int sock;
  int timeout;
  htable_t *ht;
  int res;
  int tapi;
  dbuf_t *d;
  sock_handlers_t *hdl;

  set_debug_level(DBG_GLOBAL, 1000);

  ht = halloc(10, 3);
  res = hinsertss(ht, "key", "data value", NULL, NULL, NULL);
  debug(0,0, "Result of hinsertss: %d\n", res);
  debug(0,0, "Retrieve: %s", hfindss(ht, "key"));
  debug(0,0, "Delete entry: %d", hdeletess(ht, "key", NULL, NULL, NULL));
  debug(0,0, "Retrieve: %s", hfindss(ht, "key"));
  debug(0,0, "Delete entry: %d", hdeletess(ht, "key", NULL, NULL, NULL));
  debug(0,0, "Retrieve: %s", hfindss(ht, "key"));

  sock = bind_tcp_listener(2323);
  tapi = attach_tap_interface(NULL);
  pcap = attach_pcap("wlan2");
  hdl = cdata_get_handlers(pcap);
  // this handler is to cause duplicate packets on the segment
  // hdl->ev_read = pcap_read_ev;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
    debug(0,0, ".");
  }
}


