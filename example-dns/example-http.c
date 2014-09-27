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

lua_State *LGL;

#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP

int join_mdns_groups(int sock) {
  char str_addr[1024];
  struct sockaddr_in6 v6_addr;
  struct sockaddr_in *pv4_addr;
  socklen_t sockaddr_sz;
  char *zErrMsg = 0;
  int rc;
  struct ipv6_mreq mreq;  /* Multicast address join structure */
  struct ip_mreq mreq4;

  inet_pton(AF_INET6, "ff02::fb", &v6_addr.sin6_addr);
  memcpy(&mreq.ipv6mr_multiaddr,
	 &((struct sockaddr_in6*)(&v6_addr))->sin6_addr,
	     sizeof(mreq.ipv6mr_multiaddr));
  /* Accept from any interface */
  mreq.ipv6mr_interface = 0;
  if ( setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) != 0 ) {
    printf("Error joining multicast group\n");
  }

  inet_pton(AF_INET6, "::ffff:224.0.0.251", &v6_addr.sin6_addr);
  memcpy(&mreq.ipv6mr_multiaddr,
	 &((struct sockaddr_in6*)(&v6_addr))->sin6_addr,
	     sizeof(mreq.ipv6mr_multiaddr));
  /* Accept from any interface */
  mreq.ipv6mr_interface = 0;
  if ( setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) != 0 ) {
    printf("Error joining multicast group\n");
  }

  inet_pton(AF_INET, "224.0.0.251", &mreq4.imr_multiaddr);
  /* Accept from any interface */
  mreq4.imr_interface.s_addr = 0;
  if ( setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq4, sizeof(mreq4)) != 0 ) {
    printf("Error joining v4 multicast group\n");
  }
  return 0;
}

int tuni;
http_parser_t http_parser;

int luaopen_lsqlite3(lua_State *L);
int luaopen_http_parser(lua_State* L);



static int lua_fn_dsend(lua_State *L) {
  int n = lua_gettop(L);
  int idx = lua_tonumber(L, 1);
  size_t len;
  int nwrote;
  char *data = (void *)lua_tolstring(L, 2, &len);
  dbuf_t *d = dalloc(len);

  memcpy(d->buf, data, len);
  d->dsize = len;
  sock_write_data(idx, d, &nwrote);
  dunlock(d);

  lua_pushnumber(L, nwrote);

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
        dunlock(sock_write_data(idx, dstrcpy(headers), NULL));
        dunlock(sock_write_data(idx, dalloc_ptr(xfile, xfile_sz), NULL));
	xfile = NULL;
      }
    } else {
      snprintf(headers, sizeof(headers)-1, "HTTP/1.0 404 Not Found\r\nConnection: keep-alive\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", (int)strlen(reply), content_type);
      dunlock(sock_send_data(idx, dstrcpy(headers), NULL));
      dunlock(sock_send_data(idx, dstrcpy(reply), NULL));
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

int dns_read_ev(int idx, dbuf_t *d, void *p) {
  struct sockaddr_in6 v6_addr;
  char v6addr_text[INET6_ADDRSTRLEN];
  socklen_t sockaddr_sz = sizeof(v6_addr);
  char *cbname = "udp5353";

  cdata_get_remote(idx, &v6_addr);
  inet_ntop(AF_INET6, &v6_addr.sin6_addr, v6addr_text, INET6_ADDRSTRLEN);
  lua_getglobal(LGL, cbname);

  if(lua_isfunction(LGL, -1)) {
    lua_pushstring(LGL, v6addr_text);
    lua_pushinteger(LGL, v6_addr.sin6_port);
    lua_pushlstring(LGL, d->buf, d->dsize);
    if (lua_pcall(LGL, 3, 0, 0) != 0) {
      fprintf(stderr, "Could not call function %s(): %s\n", cbname, lua_tostring(LGL,-1));
    }
  } else {
    fprintf(stderr, "Could not find function %s\n", cbname);
  }
  return d->size;
}



int main(int argc, char *argv[]) {
  int timeout = 0;
  sock_handlers_t *hdl;
  int udp5353;

  set_debug_level(DBG_GLOBAL, 1000);

  tuni = bind_tcp_listener(8080);
  hdl = cdata_get_handlers(tuni);
  hdl->ev_read = tun_read_ev;

  udp5353 = bind_udp_listener_specific("::", 5353, NULL);
  join_mdns_groups(sock_get_fd(udp5353));
  hdl = cdata_get_handlers(udp5353);
  hdl->ev_read = dns_read_ev;

  LGL = lua_open();
  lua_init_state(LGL);
  if(luaL_dofile(LGL, "example-dns.lua")!=0) {
    fprintf(stderr,"%s\n",lua_tostring(LGL,-1));
    return NULL;
  }


  while(1) {
    if (timeout == 0) { 
      timeout = 1000;
    }
    timeout = sock_one_cycle(timeout, NULL);
  }
}


