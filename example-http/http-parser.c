
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
	28, 31, 33, 36, 53, 69, 85, 88, 
	90, 93, 103, 112, 118, 124, 136, 148, 
	154, 160, 171, 172, 173, 174, 175, 176, 
	177, 178, 179, 180, 181, 182, 184, 185, 
	186
};

static const char _httpparser_trans_keys[] = {
	68, 71, 72, 79, 80, 69, 76, 69, 
	84, 69, 32, 42, 43, 47, 45, 57, 
	65, 90, 97, 122, 32, 72, 84, 84, 
	80, 47, 48, 57, 46, 48, 57, 48, 
	57, 13, 48, 57, 10, 13, 33, 124, 
	126, 35, 39, 42, 43, 45, 46, 48, 
	57, 65, 90, 94, 122, 13, 33, 124, 
	126, 35, 39, 42, 43, 45, 46, 48, 
	57, 65, 90, 94, 122, 33, 58, 124, 
	126, 35, 39, 42, 43, 45, 46, 48, 
	57, 65, 90, 94, 122, 10, 13, 32, 
	10, 13, 10, 13, 32, 43, 58, 45, 
	46, 48, 57, 65, 90, 97, 122, 32, 
	37, 60, 62, 127, 0, 31, 34, 35, 
	48, 57, 65, 70, 97, 102, 48, 57, 
	65, 70, 97, 102, 32, 37, 47, 59, 
	60, 62, 63, 127, 0, 31, 34, 35, 
	32, 37, 47, 59, 60, 62, 63, 127, 
	0, 31, 34, 35, 48, 57, 65, 70, 
	97, 102, 48, 57, 65, 70, 97, 102, 
	37, 47, 127, 0, 32, 34, 35, 59, 
	60, 62, 63, 69, 84, 69, 65, 68, 
	80, 84, 73, 79, 78, 83, 79, 85, 
	83, 10, 0
};

static const char _httpparser_single_lengths[] = {
	0, 5, 1, 1, 1, 1, 1, 1, 
	3, 1, 1, 1, 1, 1, 1, 0, 
	1, 0, 1, 5, 4, 4, 3, 2, 
	3, 2, 5, 0, 0, 8, 8, 0, 
	0, 3, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 2, 1, 1, 
	0
};

static const char _httpparser_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	3, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 6, 6, 6, 0, 0, 
	0, 4, 2, 3, 3, 2, 2, 3, 
	3, 4, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0
};

static const unsigned char _httpparser_index_offsets[] = {
	0, 0, 6, 8, 10, 12, 14, 16, 
	18, 25, 27, 29, 31, 33, 35, 37, 
	39, 42, 44, 47, 59, 70, 81, 85, 
	88, 92, 99, 107, 111, 115, 126, 137, 
	141, 145, 153, 155, 157, 159, 161, 163, 
	165, 167, 169, 171, 173, 175, 178, 180, 
	182
};

static const char _httpparser_indicies[] = {
	0, 2, 3, 4, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 13, 14, 13, 13, 13, 
	1, 15, 1, 16, 1, 17, 1, 18, 
	1, 19, 1, 20, 1, 21, 1, 22, 
	23, 1, 24, 1, 25, 26, 1, 27, 
	28, 29, 29, 29, 29, 29, 29, 29, 
	29, 29, 1, 28, 29, 29, 29, 29, 
	29, 29, 29, 29, 29, 1, 30, 31, 
	30, 30, 30, 30, 30, 30, 30, 30, 
	1, 1, 1, 33, 32, 1, 35, 34, 
	1, 35, 33, 32, 36, 37, 36, 36, 
	36, 36, 1, 15, 38, 1, 1, 1, 
	1, 1, 37, 39, 39, 39, 1, 37, 
	37, 37, 1, 15, 41, 1, 37, 1, 
	1, 37, 1, 1, 1, 40, 15, 41, 
	42, 37, 1, 1, 37, 1, 1, 1, 
	40, 43, 43, 43, 1, 40, 40, 40, 
	1, 41, 1, 1, 1, 1, 1, 1, 
	40, 44, 1, 10, 1, 45, 1, 46, 
	1, 10, 1, 47, 1, 48, 1, 49, 
	1, 50, 1, 51, 1, 10, 1, 52, 
	44, 1, 44, 1, 53, 1, 1, 0
};

static const char _httpparser_trans_targs[] = {
	2, 0, 34, 36, 39, 45, 3, 4, 
	5, 6, 7, 8, 9, 25, 29, 10, 
	11, 12, 13, 14, 15, 16, 17, 16, 
	18, 19, 18, 20, 47, 21, 21, 22, 
	23, 24, 23, 19, 25, 26, 27, 28, 
	30, 31, 33, 32, 35, 37, 38, 40, 
	41, 42, 43, 44, 46, 48
};

static const char _httpparser_trans_actions[] = {
	1, 0, 1, 1, 1, 1, 0, 0, 
	0, 0, 0, 7, 1, 1, 1, 9, 
	0, 0, 0, 0, 0, 1, 3, 0, 
	1, 5, 0, 0, 17, 1, 0, 11, 
	13, 13, 0, 15, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 17
};

static const int httpparser_start = 1;
static const int httpparser_first_final = 47;
static const int httpparser_error = 0;

static const int httpparser_en_main = 1;


#line 126 "http-parser.rl"


void http_parser_init(http_parser_t *parser) {
  int cs;

  memset(parser, 0, sizeof(*parser));

  
#line 152 "http-parser.c"
	{
	cs = httpparser_start;
	}

#line 134 "http-parser.rl"

  parser->cs = cs;
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

  
#line 184 "http-parser.c"
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
	{ markp = p; }
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
	{ store_acc_data(parser, markp, p); markp = NULL; }
	break;
	case 4:
#line 16 "http-parser.rl"
	{ store_acc_data(parser, markp, p); markp = NULL; printf("URI: %s\n", &parser->buf[parser->pcs]); }
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
	{ parser->done = 1; }
	break;
#line 294 "http-parser.c"
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

#line 160 "http-parser.rl"

  parser->cs = cs;
 
  return (parser->done == 1);
}




