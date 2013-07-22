
#line 1 "example-pcap.rl"
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


#line 61 "example-pcap.rl"




#line 54 "example-pcap.c"
static const char _parsehex_actions[] = {
	0, 1, 0, 1, 1, 1, 2
};

static const char _parsehex_key_offsets[] = {
	0, 0, 5, 9, 10, 16, 23, 26, 
	35, 45, 55, 66, 77
};

static const char _parsehex_trans_keys[] = {
	10, 32, 48, 9, 13, 32, 48, 9, 
	13, 120, 48, 57, 65, 70, 97, 102, 
	58, 48, 57, 65, 70, 97, 102, 32, 
	9, 13, 32, 9, 13, 48, 57, 65, 
	70, 97, 102, 10, 32, 9, 13, 48, 
	57, 65, 70, 97, 102, 32, 48, 9, 
	13, 49, 57, 65, 70, 97, 102, 10, 
	32, 120, 9, 13, 48, 57, 65, 70, 
	97, 102, 10, 32, 48, 9, 13, 49, 
	57, 65, 70, 97, 102, 10, 32, 48, 
	9, 13, 0
};

static const char _parsehex_single_lengths[] = {
	0, 3, 2, 1, 0, 1, 1, 1, 
	2, 2, 3, 3, 3
};

static const char _parsehex_range_lengths[] = {
	0, 1, 1, 0, 3, 3, 1, 4, 
	4, 4, 4, 4, 1
};

static const char _parsehex_index_offsets[] = {
	0, 0, 5, 9, 11, 15, 20, 23, 
	29, 36, 43, 51, 59
};

static const char _parsehex_indicies[] = {
	2, 0, 3, 0, 1, 0, 3, 0, 
	1, 4, 1, 5, 5, 5, 1, 6, 
	5, 5, 5, 1, 7, 7, 1, 7, 
	7, 8, 8, 8, 1, 10, 9, 9, 
	8, 8, 8, 1, 11, 12, 11, 8, 
	8, 8, 1, 10, 9, 4, 9, 8, 
	8, 8, 1, 13, 11, 12, 11, 8, 
	8, 8, 1, 2, 0, 3, 0, 1, 
	0
};

static const char _parsehex_trans_targs[] = {
	2, 0, 12, 3, 4, 5, 6, 7, 
	8, 7, 11, 9, 10, 11
};

static const char _parsehex_trans_actions[] = {
	0, 0, 0, 0, 0, 5, 0, 0, 
	1, 3, 3, 0, 1, 0
};

static const int parsehex_start = 1;
static const int parsehex_first_final = 11;
static const int parsehex_error = 0;

static const int parsehex_en_main = 1;


#line 65 "example-pcap.rl"

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

  
#line 150 "example-pcap.c"
	{
	cs = parsehex_start;
	}

#line 92 "example-pcap.rl"
  d = dalloc(2000);

  while(1) {
    nread = read(1, buf, 1000);
    p = buf;
    pe = buf + nread;
    reset_acc();
    
    
#line 165 "example-pcap.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _parsehex_trans_keys + _parsehex_key_offsets[cs];
	_trans = _parsehex_index_offsets[cs];

	_klen = _parsehex_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _parsehex_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _parsehex_indicies[_trans];
	cs = _parsehex_trans_targs[_trans];

	if ( _parsehex_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _parsehex_actions + _parsehex_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 49 "example-pcap.rl"
	{ acc = (acc << 4) + tohex((*p)); nchars++; }
	break;
	case 1:
#line 50 "example-pcap.rl"
	{ store_bytes(d, acc, nchars/2); reset_acc(); }
	break;
	case 2:
#line 51 "example-pcap.rl"
	{ offset = (offset << 4) + tohex((*p)); }
	break;
#line 251 "example-pcap.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 101 "example-pcap.rl"
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
