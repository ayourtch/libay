
#line 1 "http-parser.rl"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "http-parser.h"
#include "debug-ay.h"



#line 123 "http-parser.rl"


  
#line 16 "http-parser.c"
static const char _httpparser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8
};

static const unsigned char _httpparser_key_offsets[] = {
	0, 0, 5, 6, 7, 8, 9, 10, 
	11, 20, 21, 22, 23, 24, 25, 26, 
	28, 31, 33, 36, 37, 53, 54, 70, 
	73, 75, 78, 88, 97, 103, 109, 119, 
	120, 121, 122, 123, 124, 125, 126, 127, 
	128, 129, 130, 132, 133
};

static const char _httpparser_trans_keys[] = {
	68, 71, 72, 79, 80, 69, 76, 69, 
	84, 69, 32, 42, 43, 47, 45, 57, 
	65, 90, 97, 122, 32, 72, 84, 84, 
	80, 47, 48, 57, 46, 48, 57, 48, 
	57, 13, 48, 57, 10, 13, 33, 124, 
	126, 35, 39, 42, 43, 45, 46, 48, 
	57, 65, 90, 94, 122, 10, 33, 58, 
	124, 126, 35, 39, 42, 43, 45, 46, 
	48, 57, 65, 90, 94, 122, 10, 13, 
	32, 10, 13, 10, 13, 32, 43, 58, 
	45, 46, 48, 57, 65, 90, 97, 122, 
	32, 37, 60, 62, 127, 0, 31, 34, 
	35, 48, 57, 65, 70, 97, 102, 48, 
	57, 65, 70, 97, 102, 32, 37, 47, 
	60, 62, 127, 0, 31, 34, 35, 69, 
	84, 69, 65, 68, 80, 84, 73, 79, 
	78, 83, 79, 85, 83, 0
};

static const char _httpparser_single_lengths[] = {
	0, 5, 1, 1, 1, 1, 1, 1, 
	3, 1, 1, 1, 1, 1, 1, 0, 
	1, 0, 1, 1, 4, 1, 4, 3, 
	2, 3, 2, 5, 0, 0, 6, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 2, 1, 0
};

static const char _httpparser_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	3, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 0, 6, 0, 6, 0, 
	0, 0, 4, 2, 3, 3, 2, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0
};

static const unsigned char _httpparser_index_offsets[] = {
	0, 0, 6, 8, 10, 12, 14, 16, 
	18, 25, 27, 29, 31, 33, 35, 37, 
	39, 42, 44, 47, 49, 60, 62, 73, 
	77, 80, 84, 91, 99, 103, 107, 116, 
	118, 120, 122, 124, 126, 128, 130, 132, 
	134, 136, 138, 141, 143
};

static const char _httpparser_indicies[] = {
	0, 2, 3, 4, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 13, 14, 13, 13, 13, 
	1, 15, 1, 16, 1, 17, 1, 18, 
	1, 19, 1, 20, 1, 21, 1, 22, 
	23, 1, 24, 1, 25, 26, 1, 27, 
	1, 28, 29, 29, 29, 29, 29, 29, 
	29, 29, 29, 1, 30, 1, 31, 32, 
	31, 31, 31, 31, 31, 31, 31, 31, 
	1, 1, 1, 34, 33, 1, 36, 35, 
	1, 36, 34, 33, 37, 38, 37, 37, 
	37, 37, 1, 15, 39, 1, 1, 1, 
	1, 1, 38, 40, 40, 40, 1, 38, 
	38, 38, 1, 15, 39, 1, 1, 1, 
	1, 1, 1, 38, 41, 1, 10, 1, 
	42, 1, 43, 1, 10, 1, 44, 1, 
	45, 1, 46, 1, 47, 1, 48, 1, 
	10, 1, 49, 41, 1, 41, 1, 1, 
	0
};

static const char _httpparser_trans_targs[] = {
	2, 0, 31, 33, 36, 42, 3, 4, 
	5, 6, 7, 8, 9, 26, 30, 10, 
	11, 12, 13, 14, 15, 16, 17, 16, 
	18, 19, 18, 20, 21, 22, 44, 22, 
	23, 24, 25, 24, 19, 26, 27, 28, 
	29, 32, 34, 35, 37, 38, 39, 40, 
	41, 43
};

static const char _httpparser_trans_actions[] = {
	1, 0, 1, 1, 1, 1, 0, 0, 
	0, 0, 0, 7, 1, 1, 1, 9, 
	0, 0, 0, 0, 0, 1, 3, 0, 
	1, 5, 0, 0, 0, 1, 17, 0, 
	11, 13, 13, 0, 15, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0
};

static const int httpparser_start = 1;
static const int httpparser_first_final = 44;
static const int httpparser_error = 0;

static const int httpparser_en_main = 1;


#line 126 "http-parser.rl"


void http_parser_init(http_parser_t *parser) {
  int cs;

  memset(parser, 0, sizeof(*parser));

  
#line 137 "http-parser.c"
	{
	cs = httpparser_start;
	}

#line 134 "http-parser.rl"

  parser->cs = cs;
  parser->end = NULL;
}

int store_acc_data(http_parser_t *parser, uint8_t *start, uint8_t *end) {
  int old_pce = 0;

  if ((sizeof(parser->buf) - parser->pce) < (end-start)) {
    return 0; /* Can't fit into the remaining part in the buffer */
  }
  memcpy(&parser->buf[parser->pce], start, end-start);
  old_pce = parser->pce;
  parser->pce += end-start;
  parser->buf[parser->pce] = 0;
  debug(DBG_GLOBAL, 0, "Storing data: '%s'", &parser->buf[old_pce]);
  
  return 1;
}

int http_parser_data(http_parser_t *parser, uint8_t *data, int data_len) {
  int cs = parser->cs;
  uint8_t *p = data;
  uint8_t *pe = p + data_len;
  uint8_t *markp;

  
#line 170 "http-parser.c"
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
	_keys = _httpparser_trans_keys + _httpparser_key_offsets[cs];
	_trans = _httpparser_index_offsets[cs];

	_klen = _httpparser_single_lengths[cs];
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

	_klen = _httpparser_range_lengths[cs];
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
	_trans = _httpparser_indicies[_trans];
	cs = _httpparser_trans_targs[_trans];

	if ( _httpparser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _httpparser_actions + _httpparser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 12 "http-parser.rl"
	{ parser->pcs = parser->pce+1; parser->pce = parser->pcs; markp = p; }
	break;
	case 1:
#line 13 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; }
	break;
	case 2:
#line 14 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; }
	break;
	case 3:
#line 15 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; parser->req_method = &parser->buf[parser->pcs]; }
	break;
	case 4:
#line 16 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; parser->req_uri = &parser->buf[parser->pcs]; }
	break;
	case 5:
#line 17 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; }
	break;
	case 6:
#line 18 "http-parser.rl"
	{ parser->hname = parser->pcs; parser->pcs = parser->pce+1; parser->pce = parser->pcs; markp = p; }
	break;
	case 7:
#line 19 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; }
	break;
	case 8:
#line 121 "http-parser.rl"
	{ parser->done = 1; parser->end = p+1; }
	break;
#line 280 "http-parser.c"
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

#line 161 "http-parser.rl"

  parser->cs = cs;
 
  return (parser->done == 1);
}




