#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"
#include "http-parser.h"

#include "miniz.c"

int tuni;
http_parser_t http_parser;


int tun_read_ev(int idx, dbuf_t *d, void *p) {
  http_parser_t *parser = &http_parser;
  http_parser_init(parser);
  if(http_parser_data(parser, d->buf, d->dsize)) {
    char *reply = "File not found!\n";
    char *content_type = "text/plain";
    char headers[512];
    void *xfile;
    size_t xfile_sz;

    debug(DBG_GLOBAL, 1, "Parser done. Method: '%s', URI: '%s'", parser->req_method, parser->req_uri);
    if(strstr(parser->req_uri, ".css")) {
      content_type = "text/css";
    } else if(strstr(parser->req_uri, ".html")) {
      content_type = "text/html";
    } else if(strstr(parser->req_uri, ".js")) {
      content_type = "application/javascript";
    }

    xfile = mz_zip_extract_archive_file_to_heap("data.zip", parser->req_uri+1, &xfile_sz, 0);
    if(xfile) {
      reply = xfile;
      snprintf(headers, sizeof(headers)-1, "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", (int)xfile_sz, content_type);
    } else {
      snprintf(headers, sizeof(headers)-1, "HTTP/1.0 404 Not Found\r\nConnection: keep-alive\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", (int)strlen(reply), content_type);
    }
    sock_send_data(idx, dstrcpy(headers));
    sock_send_data(idx, dstrcpy(reply));
    if(xfile) {
      free(xfile);
    }
  } else {
    return 0;
  }
  
  // printf("Got packet, sending back!\n");
  return -1;
}


int main(int argc, char *argv[]) {
  int timeout = 0;
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


