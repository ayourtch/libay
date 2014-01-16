#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"
#include "http-parser.h"

int tuni;


int tun_read_ev(int idx, dbuf_t *d, void *p) {
  http_parser_t parser;
  http_parser_init(&parser);
  if(http_parser_data(&parser, d->buf, d->dsize)) {
    char *reply = "This is a test!\n";
    char headers[512];

    debug(DBG_GLOBAL, 1, "Parser done!");

    snprintf(headers, sizeof(headers)-1, "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-length: %d\r\nContent-type: text/plain\r\n\r\n", (int)strlen(reply));
    sock_send_data(idx, dstrcpy(headers));
    sock_send_data(idx, dstrcpy(reply));
  }
  
  // printf("Got packet, sending back!\n");
  return -1;
}


int main(int argc, char *argv[]) {
  int timeout;
  sock_handlers_t *hdl;

  set_debug_level(DBG_GLOBAL, 1000);

  tuni = bind_tcp_listener(8080);
  hdl = cdata_get_handlers(tuni);
  hdl->ev_read = tun_read_ev;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}

