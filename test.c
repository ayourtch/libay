#include "debug-ay.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int sock;
  int timeout;
  set_debug_level(DBG_GLOBAL, 1000);
  sock = bind_tcp_listener(2323);
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
    debug(0,0, ".");
  }
}


