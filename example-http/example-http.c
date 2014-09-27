#include <stdio.h>
#include <stdlib.h>
#include "debug-ay.h"
#include "hash-ay.h"
#include "dbuf-ay.h"
#include "sock-ay.h"
#include "http-parser.h"

#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lstate.h"
#include "lauxlib.h"


#include "miniz.c"

int tuni;
http_parser_t http_parser;

int luaopen_lsqlite3(lua_State *L);
int luaopen_http_parser(lua_State* L);



static int lua_fn_dsend(lua_State *L) {
  int n = lua_gettop(L);
  int idx = lua_tonumber(L, 1);
  size_t len;
  char *data = (void *)lua_tolstring(L, 2, &len);
  dbuf_t *d = dalloc(len);

  memcpy(d->buf, data, len);
  d->dsize = len;
  sock_write_data(idx, d);

  lua_pushnumber(L, len);

  return 1;
}


void lua_init_state(lua_State *L) {
  luaL_openlibs(L);

  lua_getglobal(L,"package");
  lua_getfield(L,-1,"preload");
  lua_pushcfunction(L,luaopen_lsqlite3);
  lua_setfield(L,-2,"lsqlite3");

  lua_register(L, "dsend", lua_fn_dsend);
}



int tun_read_ev(int idx, dbuf_t *d, void *p) {
  lua_State *L;
  http_parser_t *parser = &http_parser;
  http_parser_init(parser);
  L = lua_open();
  lua_init_state(L);
  if(http_parser_data(parser, d->buf, d->dsize)) {
    char *reply = "File not found!\n";
    char *content_type = "text/plain";
    char headers[512];
    char trailer[1024];
    char *xfile;
    size_t xfile_sz;

    debug(DBG_GLOBAL, 1, "Parser done. Method: '%s', URI: '%s'", parser->req_method, parser->req_uri);
    if(strstr(parser->req_uri, ".css")) {
      content_type = "text/css";
    } else if(strstr(parser->req_uri, ".html")) {
      content_type = "text/html";
    } else if(strstr(parser->req_uri, ".js")) {
      content_type = "application/javascript";
    } else if(strstr(parser->req_uri, ".png")) {
      content_type = "image/png";
    }
    memcpy(trailer, parser->end, parser->content_length);
    trailer[parser->content_length] = 0;
    debug(0,0, "Parser trailer: '%s'", trailer);

    xfile = mz_zip_extract_archive_file_to_heap("data.zip", parser->req_uri+1, &xfile_sz, 0);
    if(xfile) {
      debug(0, 0, "File '%s' => size is %d", parser->req_uri+1, xfile_sz);
      xfile[xfile_sz] = 0;
      if(strstr(parser->req_uri, ".lua")) {
	debug(0, 0, "Lua file load : '%s'", xfile);
        if(luaL_loadstring(L, xfile)) {
	  debug(0, 0, "Lua file load error: %s", lua_tostring(L,-1));
        } else {
          lua_pcall(L, 0, LUA_MULTRET, 0);
          lua_getglobal(L, "request");
	  lua_pushinteger(L, idx);
	  int err = lua_pcall(L, 1, 0, 0);
          if (err != 0) {
            debug(0,0,"%d: LUA error %s\n",getpid(), lua_tostring(L,-1));
          }
        }
      } else {
        snprintf(headers, sizeof(headers)-1, "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", (int)xfile_sz, content_type);
        sock_write_data(idx, dstrcpy(headers));
        sock_write_data(idx, dalloc_ptr(xfile, xfile_sz));
	xfile = NULL;
      }
    } else {
      snprintf(headers, sizeof(headers)-1, "HTTP/1.0 404 Not Found\r\nConnection: keep-alive\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", (int)strlen(reply), content_type);
      sock_send_data(idx, dstrcpy(headers));
      sock_send_data(idx, dstrcpy(reply));
    }
    if(xfile) {
      free(xfile);
    }
    lua_close(L);
  } else {
    lua_close(L);
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


