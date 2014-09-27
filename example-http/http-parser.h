typedef struct http_parser_t {
  int cs;
  int done;

  int hname;
  int pcs, pce;
  char buf[8192];

  /* These are the pointers to the above buf where everything is stored */
  char *req_method;
  char *req_uri; 

  /* This is the pointer to the content after successful parsing */
  char *end;
} http_parser_t;

void http_parser_init(http_parser_t *parser);
int http_parser_data(http_parser_t *parser, uint8_t *data, int data_len); 


