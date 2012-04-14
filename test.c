#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"

int main(int argc, char *argv[]) {
  int sock;
  int timeout;
  htable_t *ht;
  int res;

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
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
    debug(0,0, ".");
  }
}


