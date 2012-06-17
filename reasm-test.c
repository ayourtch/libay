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


void handle_command(char *cmd) {
  char *op;
  dbuf_t *d;
  if (NULL == cmd) {
    return;
  }
  op = strtok(cmd, " ");
  if (NULL == op) {
    return;
  }

  if (0 == strcmp(op, "h")) {
    debug(0,0, "d <level>");
    debug(0,0, "r <xid> <offs> <mf> <data>");
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
    d = dtry_reasm(pile, xid, data, strlen(data), offs, mf);
    if(d) {
      debug(0,0, "Reassembly complete");
      debug_dump(DBG_REASM, 0, d->buf, d->dsize);
    } else {
      debug(0,0, "Reassembly in progress");
    }
  }

}

void debug_redraw_cb(int idlecall) {
  if (!idlecall) {
    fprintf(stderr, "> %s", cmdbuf);
  }
}

void handle_char(char c) {
  if (c == 3 ) { 
    detach_stdin();
    printf("\n\n\n");
    exit(1);
  }
  if (c == 0x7f) {
    if(cmdlen>0) {
      cmdbuf[--cmdlen] = 0;
      fprintf(stderr, "%c %c", 8, 8);
    }
  } else if ((c == '\r') || (c == '\n')) {
    fprintf(stderr, "\n");
    debug(0,0, "Cmd: '%s'", cmdbuf);
    handle_command(cmdbuf);
    cmdlen = 0;
    cmdbuf[cmdlen] = 0;
  } else {
    fprintf(stderr, "%c", c);
    cmdbuf[cmdlen++] = c;
    cmdbuf[cmdlen] = 0;
  }
}

int stdi;

int stdin_read_ev(int idx, dbuf_t *d, void *p) {
  int i;
  for(i=0; i<d->dsize; i++) {
    handle_char(d->buf[i]);
  }
  return d->dsize;
}

int main(int argc, char *argv[]) {
  int sock;
  int timeout;
  htable_t *ht;
  int res;
  dbuf_t *d;
  sock_handlers_t *hdl;

  // set_debug_level(DBG_GLOBAL, 1000);

  pile = make_reasm_pile();

  stdi = attach_stdin(1); 
  hdl = cdata_get_handlers(stdi); 
  hdl->ev_read = stdin_read_ev;
  debug_will_need_redraw(debug_redraw_cb);
  debug(0,0, "Press Ctrl-C to quit");


  timeout = 1000;
  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}


