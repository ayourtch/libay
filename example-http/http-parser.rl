#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "http-parser.h"
#include "debug-ay.h"


%%{
  machine httpparser;
  alphtype char;

  action mark      { markp = p; }
  action http_version_major { store_acc_data(parser, markp, p); markp = NULL; }
  action http_version_minor { store_acc_data(parser, markp, p); markp = NULL; }
  action http_method { store_acc_data(parser, markp, p); markp = NULL; }
  action http_uri { store_acc_data(parser, markp, p); markp = NULL; printf("URI: %s\n", &parser->buf[parser->pcs]); }
  action http_header_name { store_acc_data(parser, markp, p); markp = NULL; }
  action mark_value { parser->hname = parser->pcs; parser->pcs = parser->pce+1; parser->pce = parser->pcs; markp = p; }
  action http_header_value { store_acc_data(parser, markp, p); markp = NULL; }
  
  htp_octet = (any);
  htp_uint8_t = (ascii);
  htp_upalpha = (upper);
  htp_loalpha = (lower);
  htp_alpha = (htp_loalpha | htp_upalpha);
  htp_digit = (digit);
  htp_ctl = (cntrl | 127);
  htp_cr = ( 13 );
  htp_lf = ( 10 );
  htp_sp = ( ' ' );
  htp_ht = ( 9 );
  htp_quote = ( '"' );

  htp_crlf = ( htp_cr htp_lf? ); # // Accomodate for unix NLs ?
  # htp_crlf = ( htp_cr htp_lf ); # // do NOT accomodate for unix NLs ?

  htp_lws = ( htp_crlf? (htp_sp | htp_ht)+ ); 

  htp_not_ctl = (htp_octet - htp_ctl);

  htp_text = (htp_not_ctl | htp_lws); # (htp_cr | htp_lf | htp_sp | htp_ht));

  htp_hex = (xdigit);

  htp_tspecials = (
      '(' | ')' | '<' | '>' | '@' |
      ',' | ';' | ':' | '\\' | htp_quote |
      '/' | '[' | ']' | '?' | '=' |
      '{' | '}' | htp_sp | htp_ht);

  htp_token_uint8_t = ((htp_uint8_t - htp_tspecials) - htp_ctl);
  htp_token = (htp_token_uint8_t+);

  # comments not supported yet - they require a sub-machine
  # htp_comment_uint8_t = htp_text - ('(' | ')');
  # htp_comment = ( '(' (htp_comment_uint8_t+ | htp_comment) ')' );

  htp_quoted_uint8_t = (htp_text - '"');
  htp_quoted_string = ( '"' htp_quoted_uint8_t* '"' );
  
  htp_quoted_pair = '\\' htp_uint8_t;

  htp_http_ver_major = htp_digit+ >mark %http_version_major;
  htp_http_ver_minor = htp_digit+ >mark %http_version_minor;

  htp_http_version = ("HTTP" "/" htp_http_ver_major "." htp_http_ver_minor);

  htp_escape = ('%' htp_hex htp_hex);
  htp_reserved = (';' | '/' | '?' | ':' | '@' | '&' | '=' | '+');
  htp_extra = ('!' | '*' | '\'' | '(' | ')' | ',');
  htp_safe = ('$' | '-' | '_' | '.');
  htp_unsafe = (htp_ctl | htp_sp | htp_quote | '#' | '%' | '<' | '>');
  htp_national = (htp_octet - (htp_alpha | htp_digit | htp_reserved | htp_extra | htp_safe | htp_unsafe));
  
  htp_unreserved = (htp_alpha | htp_digit | htp_safe | htp_extra | htp_national);
  htp_uuint8_t = (htp_unreserved | htp_escape);
  htp_puint8_t = (htp_uuint8_t | ':' | '@' | '&' | '=' | '+');

  htp_fragment = ( (htp_uuint8_t | htp_reserved)* );
  htp_query = ( (htp_uuint8_t | htp_reserved)* );

  htp_net_loc = ( (htp_puint8_t | ';' | '?' )* );
  htp_scheme = ( (htp_alpha | htp_digit | '+' | '-' | '.')+ );

  htp_param = ( (htp_puint8_t | '/')* );
  htp_params = (htp_param (';' htp_param)* ); 

  htp_segment = (htp_puint8_t*);
  htp_fsegment = (htp_puint8_t+);
  htp_path = (htp_fsegment ('/' htp_fsegment)*);

  htp_rel_path = ( htp_path? (';' htp_params)? ('?' htp_query)? );
  htp_abs_path = ('/' htp_rel_path);
  htp_net_path = ("//" htp_net_loc htp_abs_path?);

  htp_relative_uri = (htp_net_path | htp_abs_path | htp_rel_path);
  htp_absolute_uri = (htp_scheme ':' (htp_uuint8_t | htp_reserved)*);
  htp_uri = ((htp_absolute_uri | htp_relative_uri) ('#' htp_fragment)?);

  htp_host = (htp_alpha);
  htp_port = (htp_digit+);

  htp_http_url = ("http://" htp_host (':' htp_port)? (htp_abs_path)?);
  
  htp_method = ("OPTIONS" | "GET" | "HEAD" | "POST" | "PUT" | "DELETE") >mark %http_method;


  htp_request_uri = ('*' | htp_absolute_uri | htp_abs_path) >mark %http_uri;

  htp_request_line = (htp_method htp_sp htp_request_uri htp_sp htp_http_version htp_crlf);

  htp_header_name = htp_token+ >mark %http_header_name;
  # fixme.
  htp_header_value_uint8_t = htp_octet - htp_cr - htp_lf;
  htp_header_value = htp_header_value_uint8_t+ >mark_value %http_header_value;

  htp_some_header = (htp_header_name ':' htp_sp* htp_header_value htp_crlf);
  htp_last_crlf = htp_crlf; #  >{ printf("Last CRLF!\n"); eof = pe; };
  htp_request = htp_request_line (htp_some_header)* htp_last_crlf;

  main := (htp_request) @{ parser->done = 1; };

}%%

  %%write data;


void http_parser_init(http_parser_t *parser) {
  int cs;

  memset(parser, 0, sizeof(*parser));

  %%write init;

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

  %%write exec;

  parser->cs = cs;
 
  return (parser->done == 1);
}




