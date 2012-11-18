#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"
#include "reasm-ay.h"

#define CMDSZ 200

void *pile;

char cmdbuf[CMDSZ];
int cmdlen = 0;


int cli_read_ev(int idx, dbuf_t *di, void *p) {
  int i;
  char *op;
  dbuf_t *d;
  char *cmd = di->buf;
  debug(0,0, "Cmd: '%s'", cmd);
  if (NULL == cmd) {
    return;
  }
  op = strtok(cmd, " ");
  if (NULL == op) {
    return;
  }

  if (0 == strcmp(op, "h")) {
    debug(0,0, "d <level>");
    debug(0,0, "r <xid> <offs> <mf> <data> [now]");
  }  
  if (0 == strcmp(op, "d")) {
    set_debug_level(DBG_REASM, atoi(strtok(NULL, " ")));
  }
  if (0 == strcmp(op, "q")) {
    detach_stdin();
    printf("\n\n\n");
    exit(1);
  }

  if (0 == strcmp(op, "r")) {
    uint32_t xid = atol(strtok(NULL, " "));
    uint16_t offs = atoi(strtok(NULL, " "));
    int mf = atoi(strtok(NULL, " "));
    char *data = strtok(NULL, " ");
    char *now_s = strtok(NULL, " ");
    if (now_s) {
      d = dtry_reasm_timed(pile, xid, data, strlen(data), offs, mf, atoi(now_s));
    } else {
      d = dtry_reasm(pile, xid, data, strlen(data), offs, mf);
    }
    if(d) {
      debug(0,0, "Reassembly complete");
      debug_dump(DBG_REASM, 0, d->buf, d->dsize);
    } else {
      debug(0,0, "Reassembly in progress");
    }
  }
  return d->dsize || 1;
}

int main(int argc, char *argv[]) {
  int sock;
  int timeout;
  int stdi, cli;
  htable_t *ht;
  int res;
  dbuf_t *d;
  sock_handlers_t *hdl;

  // set_debug_level(DBG_GLOBAL, 1000);

  pile = make_reasm_pile();

  stdi = attach_stdin(1); 
  cli = attach_cli(stdi);
  hdl = cdata_get_handlers(cli); 
  hdl->ev_read = cli_read_ev;
  debug(0,0, "Press Ctrl-C to quit");


  timeout = 1000;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}


