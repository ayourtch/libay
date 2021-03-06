#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"

int pcap;
int tapi;

int pcap_read_ev(int idx, dbuf_t *d, void *p) {
  int nwrote = 0;
  sock_send_data(pcap, d, &nwrote);
  return nwrote;
}

int tun_read_ev(int idx, dbuf_t *d, void *p) {
  int nwrote = 0;
  printf("Got packet, sending back!\n");
  sock_send_data(tapi, d, &nwrote);
  return nwrote;
}


int main(int argc, char *argv[]) {
  int sock;
  int timeout = 1000;
  htable_t *ht;
  int res;
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
#ifdef WITH_TUNTAP
  tapi = attach_tun_interface(NULL);
#endif
  pcap = attach_pcap("en0");
  hdl = cdata_get_handlers(pcap);
  // this handler is to cause duplicate packets on the segment
  // hdl->ev_read = pcap_read_ev;
  hdl = cdata_get_handlers(tapi);
  hdl->ev_read = tun_read_ev;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
    debug(0,0, ".");
  }
}


