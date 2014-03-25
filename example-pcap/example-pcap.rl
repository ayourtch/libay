#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"
#include "sock-pcap-ay.h"

uint32_t acc;
uint32_t offset;
int nchars;

void reset_acc() {
  acc = 0;
  offset = 0;
  nchars = 0;
}

uint8_t tohex(char ac) {
  char c = tolower(ac);
  if (c >= '0' && c <= '9') {
    return (c - '0');
  } else if (c >= 'a' && c <= 'f') {
    return (c - 'a' + 10);
  }
  assert(0);
  return 0;
}

void store_bytes(dbuf_t *d, uint32_t acc, int nbytes) {
  union {
    uint32_t u32;
    char c[4];
  } data;

  data.u32 = htonl(acc);
  for(; nbytes > 0; nbytes--) {
    dappendfill(d, data.c[4-nbytes], 1);
  }
}

%%{
machine parsehex;
alphtype char;

action dgt      { acc = (acc << 4) + tohex(fc); nchars++; }
action store      { store_bytes(d, acc, nchars/2); reset_acc(); }
action off_digit      { offset = (offset << 4) + tohex(fc); }

bytes = ( space+ [0-9a-fA-F]+ $dgt ) %store;

dataline = space* '0' 'x' xdigit+ $off_digit ':' (bytes)+ '\n';

emptyline = '\n';

main :=  ( dataline | emptyline) +;

}%%


%%write data;

int pcapi;


int main(int argc, char *argv[]) {
  dbuf_t *d;

  if (argc < 2) {
    printf("Usage: cat packet.txt | %s <intf>\n", argv[0]);
    exit(1);
  }

  set_debug_level(DBG_GLOBAL, 1);

  pcapi = attach_pcap(argv[1]);
  if (pcapi < 0) {
    fprintf(stderr, "Could not attach pcap to %s\n", argv[1]);
    exit(1);
  }

  int cs = 0;
  int nread;
  char buf[1000];
  char *p = buf;
  char *pe = buf + strlen(buf); 

  %%write init;
  d = dalloc(2000);

  while(1) {
    nread = read(0, buf, 1000);
    p = buf;
    pe = buf + nread;
    reset_acc();
    
    %%write exec;
    if (pe-p  > 0) {
      debug(DBG_GLOBAL, -1, "Accumulated dlen: %d", d->dsize);
      debug_dump(DBG_GLOBAL, -1, d->buf, d->dsize);
      if (d->dsize > 0) {
        sock_send_data(pcapi, d);
      }
      d->dsize = 0;
    }
    cs = parsehex_first_final;
  }

}
