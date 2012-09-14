

/**************************************************************************
*
*  Copyright © 2008-2009 Andrew Yourtchenko, ayourtch@gmail.com.
*
*  Permission is hereby granted, free of charge, to any person obtaining 
* a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation 
* the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom 
* the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included 
* in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
* OR OTHER DEALINGS IN THE SOFTWARE. 
*
*****************************************************************************/

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>

#include "lib_sock_intern.h"
#include "dbuf-ay.h"
#include "debug-ay.h"
#include "lists-ay.h"
#include "os-ay.h"
/*
#include "timers-ay.h"
*/

/**
 * @defgroup socket Socket handling
 */

/*@{*/


struct pollfd ufds[MAX_FDS];
conndata_t cdata[MAX_FDS];
int nfds = 0;

int nclients = 0;               /* number of active inbound connections */

long nconnections = 0;          /* counter for incoming connections */

int biggest_udp_idx = 0;        /* the biggest index of the UDP sockets */

/* L5 stuff */


void
print_cdata(int i, int debuglevel)
{
  debug(DBG_GLOBAL, debuglevel, "CDATA for %d:", i);
  debug(DBG_GLOBAL, debuglevel, "  xmit_list: %x", cdata[i].xmit_list);
  debug(DBG_GLOBAL, debuglevel, "  written: %d", cdata[i].written);
  debug(DBG_GLOBAL, debuglevel, "  listener: %d", cdata[i].listener);
  debug(DBG_GLOBAL, debuglevel, "  listener_port: %d", cdata[i].listen_port);
  debug(DBG_GLOBAL, debuglevel, "  is_udp: %d", cdata[i].is_udp);
  debug(DBG_GLOBAL, debuglevel, "  do_ssl: %d", cdata[i].do_ssl);
  debug(DBG_GLOBAL, debuglevel, "  is_ssl: %d", cdata[i].is_ssl);
  debug(DBG_GLOBAL, debuglevel, "  ssl: %x", cdata[i].ssl);
  debug(DBG_GLOBAL, debuglevel, "  apptype: %d", cdata[i].apptype);
  debug(DBG_GLOBAL, debuglevel, "  appdata: %x", cdata[i].appdata);
  debug(DBG_GLOBAL, debuglevel, "  l7state: %d", cdata[i].l7state);
  debug(DBG_GLOBAL, debuglevel, "  recv_list: %x", cdata[i].recv_list);
}

/* Dump all we know about sockets */

void print_socks(void)
{
  int i;
  for(i = 0; i < nfds; i++) {
    print_cdata(i, 0);
  }
}

void
pkt_dprint_cdata(int i, dbuf_t * d)
{
  dxprintf(d,
          "  %d:%s  do_ssl:%d,ssl:%d,listen:%d,lport:%-5d fd:%-5d revents:%x\n",
          i, cdata[i].is_udp ? "udp" : "tcp", cdata[i].do_ssl,
          cdata[i].is_ssl, cdata[i].listener, cdata[i].listen_port,
          ufds[i].fd, ufds[i].revents);
}

void
pkt_dprint_cdata_all(dbuf_t * d)
{
  int i;

  for(i = 0; i < nfds; i++) {
    pkt_dprint_cdata(i, d);
  }
}


void
dsend(int idx, dbuf_t * d)
{
  debug(DBG_GLOBAL, 5, "pushing %d bytes for index %d", d->dsize, idx);
  debug_dump(DBG_GLOBAL, 100, d->buf, d->dsize);
  lpush(&cdata[idx].xmit_list, d);
  ufds[idx].events |= POLLOUT;
  // Will be unlocked by the send mechanism
  dlock(d);
}

int
sock_get_fd(int idx)
{
  return ufds[idx].fd;
}

void
sock_set_hooks(int idx, sock_io_t do_send, sock_io_t do_recv)
{
  cdata[idx].do_send = do_send;
  cdata[idx].do_recv = do_recv;
}

dbuf_t *
cdata_get_appdata_dbuf(int idx, const char *appdata_sig)
{
  if(cdata[idx].appdata == NULL) {
    return NULL;
  } else {
    if(dcheckusig(cdata[idx].appdata, appdata_sig)) {
      return cdata[idx].appdata;
    } else {
      return NULL;
    }
  }
}

void
cdata_set_appdata_dbuf(int idx, dbuf_t *d)
{
  cdata[idx].appdata = d; 
}

sock_handlers_t *
cdata_get_handlers(int idx)
{
  return &cdata[idx].handlers;
}

int 
cdata_get_remote4(int idx, uint32_t *addr, uint16_t *port) {
  debug(DBG_GLOBAL, -1, "Fixme cdata_get_remote4");
#ifdef FIXME
  if (addr) {
    *addr = *((uint32_t *) (&cdata[idx].remote.sin_addr)); 
  }
  if (port) {
    *port = (uint16_t) cdata[idx].remote.sin_port;
  }
#endif
  return 1;
}

int 
cdata_set_remote4(int idx, uint32_t *addr, uint16_t *port) {
  debug(DBG_GLOBAL, -1, "Fixme cdata_get_remote4");
#ifdef FIXME
  if (addr) {
    *((uint32_t *) (&cdata[idx].remote.sin_addr)) = *addr; 
  }
  if (port) {
    cdata[idx].remote.sin_port = *port;
  }
#endif
  return 1;
}

int 
cdata_check_remote4(int idx, uint32_t addr, uint16_t port) {
  uint32_t a;
  uint16_t p;
  cdata_get_remote4(idx, &a, &p);
  return ((a == addr) && (p == port));
}


/*@}*/

/**
 * @defgroup ssl SSL and other "crypto" functionality
 *
 * @{
 */

SSL_METHOD *ssl_client_meth;
SSL_METHOD *ssl_server_meth;
SSL_CTX *ssl_client_ctx;
SSL_CTX *ssl_server_ctx;


int
load_ssl_ctx_certkey(SSL_CTX * ssl_ctx, char *cert_file, char *key_file)
{
  debug(DBG_SSL, 3, "Loading PEM-encoded cert from file %s...", cert_file);
  if(SSL_CTX_use_certificate_file(ssl_ctx, cert_file, SSL_FILETYPE_PEM)
     <= 0) {
    ERR_print_errors_fp(stderr);
    return -1;
  }
  debug(DBG_SSL, 3, "%s successfully loaded", cert_file);

  debug(DBG_SSL, 3, "Loading PEM-encoded private key from file %s...",
        key_file);
  if(SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    return -2;
  }
  debug(DBG_SSL, 3, "%s successfully loaded", key_file);
  if(!SSL_CTX_check_private_key(ssl_ctx)) {
    debug(DBG_SSL, 0,
          "Private key from %s does not match the certificate public key from %s",
          key_file, cert_file);
    return -3;
  }
  return 0;
}


int
init_ssl(char *cert_file_svr, char *key_file_svr, char *cert_file_clt,
         char *key_file_clt)
{
  int rc = 0;

  SSLeay_add_ssl_algorithms();
  //ssl_client_meth = SSLv2_client_method();
  ssl_client_meth = SSLv3_client_method();
  ssl_client_ctx = SSL_CTX_new(ssl_client_meth);

  ssl_server_meth = SSLv23_server_method();
  ssl_server_ctx = SSL_CTX_new(ssl_server_meth);
  SSL_load_error_strings();

  if(!ssl_client_ctx || !ssl_server_ctx) {
    ERR_print_errors_fp(stderr);
    return -1;
  }
  debug(DBG_SSL, 1, "Loading the key/cert for the client SSL context...");
  rc = load_ssl_ctx_certkey(ssl_client_ctx, cert_file_clt, key_file_clt);
  if(rc < 0) {
    return rc - 10;
  }
  debug(DBG_SSL, 1, "SSL client context successfully loaded");
  debug(DBG_SSL, 1, "Loading the key/cert for the server SSL context...");

  rc = load_ssl_ctx_certkey(ssl_server_ctx, cert_file_svr, key_file_svr);
  if(rc < 0) {
    return rc - 20;
  }
  debug(DBG_SSL, 1, "SSL server context successfully loaded");

  return 0;
}

int
test_verify(int ok, X509_STORE_CTX * ctx)
{
  debug(DBG_SSL, 0, "Verify: %d", ok);
  return 1;
}

/** 
 * called on an cleartext socket after accept, 
 * if we want to do the SSL nego on this socket 
 */
int
ssl_negotiate(int idx, int is_server)
{
  int err = 0;
  X509 *client_cert;
  char *str;

  if(cdata[idx].is_ssl) {
    debug(DBG_SSL, 0,
          "ERROR: trying to start SSL for index %d, when already started - assuming success",
          idx);
    print_cdata(idx, 0);
    return 1;
  }
  if(cdata[idx].is_udp) {
    debug(DBG_SSL, 0, "ERROR: trying to start SSL for index UDP %d", idx);
    print_cdata(idx, 0);
    return 0;
  }

  if(cdata[idx].ssl == NULL) {
    cdata[idx].ssl = SSL_new(ssl_server_ctx);
    if(cdata[idx].ssl == NULL) {
      debug(DBG_SSL, 0, "Could not create SSL for index %d", idx);
      return -100;
    }
    err = SSL_set_fd(cdata[idx].ssl, ufds[idx].fd);
    if(err == 0) {
      debug(DBG_SSL, 0, "Could not assign socket to SSL for index %d", idx);
      SSL_free(cdata[idx].ssl);
      cdata[idx].ssl = NULL;
      return -101;
    }
    SSL_set_verify(cdata[idx].ssl,
                   SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, test_verify);
  }
  if(is_server) {
    err = SSL_accept(cdata[idx].ssl);
    debug(DBG_SSL, 2, "Error from SSL_accept: %d", err);
  } else {
    err = SSL_connect(cdata[idx].ssl);
    debug(DBG_SSL, 2, "Error from SSL_connect: %d", err);
  }
  if(err <= 0) {
    ERR_print_errors_fp(stderr);
    return err;
  }
  debug(DBG_SSL, 2, "Index %d: SSL connection with cipher %s",
        idx, SSL_get_cipher(cdata[idx].ssl));
  // Get the client cert and compare it if needed. (NB: dynamic allocation) 
  client_cert = SSL_get_peer_certificate(cdata[idx].ssl);
  if(client_cert != NULL) {
    debug(DBG_SSL, 10, "Peer certificate:");
    str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
    if(str != NULL) {
      debug(DBG_SSL, 10, "  subject: %s", str);
      free(str);
    }
    str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
    if(str != NULL) {
      debug(DBG_SSL, 10, "  issuer: %s", str);
      free(str);
    }
    // some checks on client cert go here 
    X509_free(client_cert);
  } else {
    debug(DBG_SSL, 1, "Peer does not have a certificate");
  }
  // all SSL things done, now consider socket as SSL 
  cdata[idx].is_ssl = 1;
  return 1;
}

/*@}*/


/**
 * @addtogroup socket 
 */

/*@{*/

/*
int
do_l7_reset(int idx)
{
  dbuf_t *d;

  while((d = rpop(&cdata[idx].recv_list))) {
    dunlock(d);
  }
  debug(DBG_GLOBAL, 1, "L7 reset");
  cdata[idx].l7state = L7_INIT;
  return 1;
}
*/

/**
 * Gets called when the channel with the idx is ready - once.
 */
void
ev_channel_ready(int idx, void *u_ptr)
{
  if (cdata[idx].handlers.ev_channel_ready) {
    cdata[idx].handlers.ev_channel_ready(idx, u_ptr);
  }
}

/**
 * Event function which fires when 
 * the data in the dbuf has been read from the socket with index idx 
 */
int
ev_read(int idx, dbuf_t * d, void *u_ptr)
{
  int out = 0;

  debug(DBG_GLOBAL, 1, "ev_read for index %d idx, apptype: %d\n",
        idx, cdata[idx].apptype);
  debug_dump(DBG_GLOBAL, 100, d->buf, d->dsize);

  if (cdata[idx].handlers.ev_read) {
    out = cdata[idx].handlers.ev_read(idx, d, u_ptr);
  } else {
    /* 
     * make the absent event handler act like a sink,
     * rather than forcing the local close by returning zero
     */
    out = d->dsize;
  }
  return out;
}

/**
 * the socket in index idx got closed 
 */
int
ev_closed(int idx, void *u_ptr)
{
  dbuf_t *d;

  if(cdata[idx].is_ssl) {
    SSL_shutdown(cdata[idx].ssl);
    SSL_free(cdata[idx].ssl);
    cdata[idx].ssl = NULL;
  }
  while((d = rpop(&cdata[idx].xmit_list))) {
    dunlock(d);
  }
  if(cdata[idx].handlers.ev_closed) {
    cdata[idx].handlers.ev_closed(idx, u_ptr);
  }
  dunlock(cdata[idx].appdata);

  if(cdata[idx].inbound) {
    nclients--;
    /*
       if(nclients == 0) {
       debug(DBG_GLOBAL, 1, "Last client went away, close the outbound connection");
       close(ufds[delayed_idx].fd);
       ufds[delayed_idx].fd = -1;
       }
     */
  }
  return 1;
}

/**
 * the connect initiated for socket idx has failed 
 */
int
ev_connectfailed(int idx)
{
  return 1;
}

/**
 * init the data for an index. 
 */
int
init_idx(int idx)
{
  ufds[idx].events = POLLIN;
  memset(&cdata[idx], 0, sizeof(cdata[idx]));
#if 0
  cdata[idx].xmit_list = NULL;
  cdata[idx].written = 0;
  cdata[idx].recv_list = NULL;
  cdata[idx].apptype = 0;
  cdata[idx].http_path = NULL;
  cdata[idx].http_querystring = NULL;
  cdata[idx].http_referer = NULL;
  cdata[idx].listener = 0;
  cdata[idx].http_11 = 0;
#endif
  return 1;

}

void
close_idx(int idx, void *u_ptr)
{
  ev_closed(idx, u_ptr);
  close(ufds[idx].fd);
  ufds[idx].fd = -1;
}


/**
 * get a new free index
 */
int
sock_get_free_index(int minimum)
{
  int i;

  for(i = minimum; i < nfds; i++) {
    if(ufds[i].fd == -1) {
      return i;
    }
  }
  i = nfds;
  nfds++;
  return i;
}

int sock_make_new(int fd, void *private) {
  int idx = sock_get_free_index(0);
  init_idx(idx);
  ufds[idx].fd = fd;
  ufds[idx].events |= POLLIN;
  cdata[idx].private = private;
  cdata[idx].connected = 1;
  return idx;
}


/** 
 * initiate the connect to a given remote addr, return the index 
 */
int
initiate_connect(char *addr, int port)
{
  int s;
  struct sockaddr_in sin;
  int result;
  int idx = -1;
  struct hostent *hp;

  assert(nfds <= MAX_FDS);      /* we should never go _above_ */
  if(nfds == MAX_FDS) {
    printf
      ("Too many sockets opened, can not initiate outbound connection to %s:%d!",
       addr, port);
    return -1;
  }
  s = socket(PF_INET, SOCK_STREAM, 6);
  if(s < 0) {
    printf("Could not allocate socket!");
    return -1;
  }
  bzero(&sin, sizeof(sin));
  sin.sin_port = htons(port);
  sin.sin_family = AF_INET;
  inet_aton(addr, &sin.sin_addr);
  hp = gethostbyname(addr);
  sin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;

  fcntl(s, F_SETFL, O_NONBLOCK);
  result = connect(s, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  if(result == 0) {
    printf("Successful connect to %s:%d on a nonblocking socket ??",
           addr, port);
    return -1;
  } else {
    if(errno == EINPROGRESS) {
      idx = sock_make_new(s, NULL);
      /*
         the socket connection once completed, appears to cause the POLLIN rather than pollout event 
       */
      ufds[idx].events |= POLLOUT;     /* we want to know when we get connected */
      cdata[idx].connected = 0;        /* we did not connect yet */
      cdata[idx].inbound = 0;  /* outbound connection */
      debug(DBG_GLOBAL, 1,
            "Outbound connection in progress for index %d to %s:%d",
            idx, addr, port);

    } else {
      debug(DBG_GLOBAL, 1,
            "Unexpected error on connection attempt for index %d: %s",
            idx, strerror(errno));
      return -1;
    }

  }
  return idx;
}


/**
 * new connection accepted, the index of it is idx 
 */
int
ev_newconn(int idx, int parent, void *u_ptr)
{
  bzero(&cdata[idx], sizeof(cdata[idx]));

  cdata[idx].connected = 1;
  cdata[idx].inbound = 1;
  cdata[idx].l7state = 0;
  cdata[idx].recv_list = 0;
  nconnections++;


  debug(DBG_GLOBAL, 1,
        "New connection on index %d, connected: %d, inbound: %d", idx,
        cdata[idx].connected, cdata[idx].inbound);

  if(cdata[parent].handlers.ev_newconn) {
    cdata[parent].handlers.ev_newconn(idx, parent, u_ptr); 
  } 
  if (cdata[parent].handlers.ev_read) {
    cdata[idx].handlers.ev_read = cdata[parent].handlers.ev_read;
  }
  if (cdata[parent].handlers.ev_closed) {
    cdata[idx].handlers.ev_closed = cdata[parent].handlers.ev_closed;
  }
  if(cdata[idx].do_ssl == 0) {
    if(cdata[idx].handlers.ev_channel_ready) {
      cdata[idx].handlers.ev_channel_ready(idx, u_ptr);
    }
  }
  return 0;
}

/* outbound connection succeeded */
int
ev_newoutconn(int idx, int parent, void *u_ptr)
{
  /* the struct was zeroed out before - when we started connect
      bzero(&cdata[idx], sizeof(cdata[idx])); */

  cdata[idx].connected = 1;
  cdata[idx].inbound = 0;
  cdata[idx].l7state = 0;
  cdata[idx].recv_list = 0;
  nconnections++;


  debug(DBG_GLOBAL, 1,
        "New connection on index %d, connected: %d, inbound: %d", idx,
        cdata[idx].connected, cdata[idx].inbound);

  if(cdata[idx].handlers.ev_newconn) {
     cdata[idx].handlers.ev_newconn(idx, parent, u_ptr); 
  }
  if(cdata[idx].do_ssl == 0) {
    if(cdata[idx].handlers.ev_channel_ready) {
      cdata[idx].handlers.ev_channel_ready(idx, u_ptr);
    }
  }
  return 0;
}




int
create_tcp_socket()
{
  int s;
  long yes = 1;

  s = socket(PF_INET6, SOCK_STREAM, 0);
  notminus(s, "could not create socket");
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  return s;
}

int
create_udp_socket()
{
  int s;
  long yes = 1;

  s = socket(PF_INET6, SOCK_DGRAM, 0);
  notminus(s, "could not create socket");
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  return s;
}

/**
 * Bind the socket s to local address and port
 *
 * @param s socket fd
 * @param addr local address to bind to
 * @param port local port to bind to
 *
 * @return index of the newly bound socket within cdata/ufds structures
 *
 * @see bind_udp_listener_specific
 */

int
bind_socket_listener_specific(int newidx, int s, char *addr, int port)
{
  struct sockaddr_in6 sin6;
  int idx = newidx;
  char addrbuf[INET6_ADDRSTRLEN];
  int on = 1;

  bzero(&sin6, sizeof(sin6));
  sin6.sin6_port = htons(port);
  sin6.sin6_family = AF_INET6;

  if(addr != NULL) {
    if (strcmp(addr, "*") == 0) {
      notminus(setsockopt(s, SOL_IP, IP_TRANSPARENT, &on, sizeof on), "setsockopt IP_TRANSPARENT");
      addr = "::";
    } 
    inet_pton(AF_INET6, addr, &sin6.sin6_addr);
  }

  notminus(bind(s, (struct sockaddr *) &sin6, sizeof(sin6)), "bind failed");

  init_idx(idx);
  cdata[idx].listener = 1;
  cdata[idx].listen_port = port;

  ufds[idx].fd = s;
  ufds[idx].events = POLLIN;
  debug(DBG_GLOBAL, 1,
        "Index %d - starting listening on port %d for address %s!", idx,
        port, inet_ntop(AF_INET6, &sin6.sin6_addr, addrbuf, sizeof(sin6)));
  return idx;
}

int
bind_udp_listener_specific(char *addr, int port, char *remote)
{
  struct sockaddr_in6 sin6;
  struct hostent *hp;
  int s = create_udp_socket();
  int idx =
    bind_socket_listener_specific(sock_get_free_index(biggest_udp_idx), s,
                                  addr, port);

  biggest_udp_idx = idx;

  cdata[idx].is_udp = 1;

  bzero(&sin6, sizeof(sin6));
  sin6.sin6_port = 0;             //7777; // htons(port);
  sin6.sin6_family = AF_INET;

  fcntl(s, F_SETFL, O_NONBLOCK);

  debug(DBG_GLOBAL, 1, "UDP listener on %s:%d started with index %d!",
        addr, port, idx);
  if(remote != NULL) {
    inet_pton(AF_INET6, remote, &sin6.sin6_addr);
    notminus(connect
             (s, (struct sockaddr *) &sin6, sizeof(sin6)),
             "Connect of UDP");
  }
  // even though it is UDP, we assume it is "connected"
  cdata[idx].connected = 1;
  cdata[idx].inbound = 1;
  return idx;
}



int
bind_tcp_listener_specific(char *addr, int port)
{
  int s = create_tcp_socket();
  int idx =
    bind_socket_listener_specific(sock_get_free_index(0), s, addr, port);

  cdata[idx].is_udp = 0;
  notminus(listen(s, 3), "listen() failed");
  debug(DBG_GLOBAL, 1, "TCP listener on %s:%d started with index %d!",
        addr, port, idx);
  return idx;
}

int
bind_tcp_listener(int port)
{
  return bind_tcp_listener_specific(NULL, port);
}

int attach_tuntap_interface(char *dev, int s) 
{
  if (s > 0) {
    int idx = sock_make_new(s, NULL);
    debug(DBG_GLOBAL, 1, "TUN/TAP device (%s) added to index %d", dev, idx);
    return idx;
  } else {
    debug(DBG_GLOBAL, 0, "Could not get TUN/TAP device: %s", strerror(errno));
    return -1;
  }
}

int attach_tap_interface(char *dev) 
{
  return attach_tuntap_interface(dev, tap_alloc(dev));
}

int attach_tun_interface(char *dev) 
{
  return attach_tuntap_interface(dev, tun_alloc(dev));
}

struct termios ti_saved;
int fd_stdin = 0;

int attach_stdin(int raw)
{
  struct termios ti;
  struct termios *termios_p = &ti;
  int idx = sock_make_new(fd_stdin, NULL);
  tcgetattr(fd_stdin, &ti_saved);
  fcntl(fd_stdin, F_SETFL, O_NONBLOCK);

  if (raw) {
    tcgetattr(fd_stdin, &ti);
    termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    // termios_p->c_oflag &= ~OPOST;
    termios_p->c_oflag |= ONLCR;
    termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    termios_p->c_cflag &= ~(CSIZE | PARENB);
    termios_p->c_cflag |= CS8;
    tcsetattr(fd_stdin, 0, &ti);
  }
  return idx;
}

int detach_stdin() 
{
  tcsetattr(fd_stdin, 0, &ti_saved);
  return 1;
}

/*@}*/


/**
 * POLLIN event on the TCP listening socket - means there is a new
 * incoming connection, that we need to accept, allocate a new index 
 * for it, and call the appropriate handlers.
 *
 * @param i index of the listening socket
 *
 * @see sock_unconnected_pollin
 */

void
sock_tcp_listener_pollin(int i, void *u_ptr)
{
  struct sockaddr_in sin1;
  unsigned int sin1sz = sizeof(sin1);
  //int revents_save = ufds[i].revents;

  ufds[i].revents = 0;
  debug(DBG_GLOBAL, 1, "before accept");
  int s1 = accept(ufds[i].fd, (struct sockaddr *) &sin1,
                  &sin1sz);
  int newidx = sock_get_free_index(0);

  debug(DBG_GLOBAL, 1, "after accept");
  debug(DBG_GLOBAL, 1,
        "New inbound connection on index %d from %s:%d -> trying to use index %d",
        i, inet_ntoa(sin1.sin_addr), ntohs(sin1.sin_port), newidx);
  debug(DBG_GLOBAL, 10, "%d:LISTEN-POLLIN", i);
  if(nfds < MAX_FDS) {
    fcntl(s1, F_SETFL, O_NONBLOCK);
    init_idx(newidx);
    ufds[newidx].fd = s1;
    if(ev_newconn(newidx, i, u_ptr) < 0) {
      debug(DBG_GLOBAL, 1,
            "Application-specific error, closing the connection");
      close(s1);
      ufds[newidx].fd = -1;
      ufds[newidx].revents = 0;
    }
  } else {
    debug(DBG_GLOBAL, 1,
          "Could not accept new connection, too many open sockets!");
    close(s1);
  }
  //ufds[i].revents = revents_save;
}

/**
 * POLLIN event on the unconnected socket - this is supposed to 
 * be the connection result for the async outbound connection.
 *
 * @param i index that experienced POLLIN event
 *
 * @see sock_tcp_listener_pollin
 */
void
sock_unconnected_pollinout(int i, void *u_ptr)
{
  int err;
  socklen_t l = sizeof(err);

  if(getsockopt(ufds[i].fd, SOL_SOCKET, SO_ERROR, &err, &l) == -1) {
    debug(DBG_GLOBAL, 1, "Getsockopt failure (%s)!", strerror(errno));
  }
  if(err != 0) {
    debug(DBG_GLOBAL, 1,
          "Outbound connection error for index %d: %s", i, strerror(err));
    ev_connectfailed(i);
    close(ufds[i].fd);
    ufds[i].fd = -1;
  } else {
    debug(DBG_GLOBAL, 1, "Index %d successfully connected", i);
    cdata[i].connected = 1;
    ev_newoutconn(i, -1, u_ptr);
  }
}

void
sock_ssl_check_error(int i, int ret, void *u_ptr)
{
  int err;
  int err2;

  switch (err = SSL_get_error(cdata[i].ssl, ret)) {
  case SSL_ERROR_NONE:
    ufds[i].events = POLLIN;
    break;
  case SSL_ERROR_ZERO_RETURN:
    close_idx(i, u_ptr);
    break;
  case SSL_ERROR_WANT_READ:
    ufds[i].events |= POLLIN;
    break;
  case SSL_ERROR_WANT_WRITE:
    ufds[i].events |= POLLOUT;
    break;
  case SSL_ERROR_SYSCALL:
    err2 = ERR_get_error();
    debug(DBG_SSL, 0, "SSL error: syscall error %d on index %d", err2, i);
    close_idx(i, u_ptr);
    break;
  default:
    debug(DBG_SSL, 0, "SSL error: %d, closing index %d", err, i);
    ERR_print_errors_fp(stderr);
    close_idx(i, u_ptr);
  }
}

/**
 * POLLIN event for the SSL socket that is in the process of negotiation
 *
 * @param i index of the socket
 */
void
sock_ssl_pollinout(int i, void *u_ptr)
{
  int ret;

  if((ret = ssl_negotiate(i, cdata[i].inbound)) == 1) {
    cdata[i].do_ssl = 0;
    debug(DBG_GLOBAL, 11, "SSL done!! (idx %d)", i);
    ufds[i].events = POLLIN;
    ev_channel_ready(i, u_ptr);
  } else {
    sock_ssl_check_error(i, ret, u_ptr);
  }

}

int
sock_receive_data(int i, dbuf_t * d)
{
  if (cdata[i].do_recv) {
    cdata[i].do_recv(i, d, cdata[i].private);
  } else {
    if(cdata[i].is_udp == 1) {
      socklen_t remote_len = sizeof(cdata[i].remote);
      d->dsize =
	recvfrom(ufds[i].fd, d->buf, d->size, 0,
		 (struct sockaddr *) &cdata[i].remote, &remote_len);
    } else {
      if(cdata[i].is_ssl) {
	d->dsize = SSL_read(cdata[i].ssl, d->buf, d->size);
      } else {
	d->dsize = read(ufds[i].fd, d->buf, d->size);
      }
    }
  }
  cdata[i].rx_count++;
  return d->dsize;
}

/**
 * POLLIN event on connected socket *or* on a UDP socket
 *
 * @arg i index of the socket
 *
 * @see sock_connected_pollout
 * @see sock_unconnected_pollinout
 * @see sock_tcp_listener_pollin
 */

void
sock_connected_pollin(int i, void *u_ptr)
{
  if(cdata[i].do_ssl) {
    sock_ssl_pollinout(i, u_ptr);
  } else {
    dbuf_t *d = NULL;

    debug(DBG_GLOBAL, 11, "..on a Xconnected socket (idx %d)", i);
    d = dalloc(CHUNK_SZ);
    debug(DBG_GLOBAL, 11, "sock_connected_pollin: new dbuf %x (idx %d)", d,
          i);
    if(d != NULL) {
      sock_receive_data(i, d);
      debug(DBG_GLOBAL, 11,
            "..read %d bytes - max %d (idx %d)", d->dsize, d->size, i);
      if(d->dsize == 0) {
        debug(DBG_GLOBAL, 1, "Index %d closed by remote host", i);
        close_idx(i, u_ptr);
      } else if (d->dsize > 0 && ev_read(i, d, u_ptr) == 0) {
        debug(DBG_GLOBAL, 1, "Index %d closed by local host (zero return from ev_read()", i);
        close_idx(i, u_ptr);
      } 
      // those who needed this have copied it already 
      debug(DBG_GLOBAL, 11, "sock_connected_pollin: freeing buffer %x", d);
      dunlock(d);
    } else {
      debug(DBG_GLOBAL, 11, ".. NOT allocated chunk (idx %d)", i);
    }
  }
}

/**
 * Attempt to send some data and return the number of bytes written.
 *
 */
int
sock_send_data(int i, dbuf_t * d)
{
  int nwrote;
  if (cdata[i].do_send) {
    nwrote = cdata[i].do_send(i, d, cdata[i].private);
  } else {
    if(cdata[i].is_udp) {
      nwrote =
	sendto(ufds[i].fd,
	       &d->buf[cdata[i].written],
	       d->dsize - cdata[i].written,
	       0, (struct sockaddr *) &cdata[i].remote,
	       sizeof(cdata[i].remote));
	debug(DBG_GLOBAL, 11, " -- sendto for udp returned: %d\n", nwrote);     
    } else {
      if(cdata[i].is_ssl) {
	nwrote =
	  SSL_write(cdata[i].ssl,
		    &d->buf[cdata[i].written], d->dsize - cdata[i].written);
      } else {
	nwrote =
	  write(ufds[i].fd,
		&d->buf[cdata[i].written], d->dsize - cdata[i].written);
      }
    }
  }
  return nwrote;
}

/* try to send the data with queueing, if needed */
int
sock_write_data(int i, dbuf_t *d)
{
  int nwrote = sock_send_data(i, d);
  if (nwrote < d->dsize) {
    if (nwrote <= 0) {
      /* FIXME: this will cause a problem in case of an error, methinks */
      dsend(i, d);
    } else {
      dbuf_t *d1 = dsubstrcpy(d, nwrote, d->dsize - nwrote);
      dsend(i, d1);
      dunlock(d);
    }
  }
  return nwrote;
}

/**
 * POLLOUT on the connected socket or on UDP socket
 *
 * @arg i index of the socket
 *
 * @see sock_connected_pollin
 */
void
sock_connected_pollout(int i, void *u_ptr)
{
  if(cdata[i].do_ssl) {
    sock_ssl_pollinout(i, u_ptr);
  } else {
    if(cdata[i].xmit_list != NULL) {
      dbuf_t *d = rpeek(&cdata[i].xmit_list);
      int nwrote;

      if(d->dsize > 0) {
        if(cdata[i].is_udp && cdata[i].rx_count == 0) {
          nwrote = d->dsize - cdata[i].written;
          debug(DBG_GLOBAL, 1, "UDP index %d not connected yet, discard data",
                i);
        } else {
          nwrote = sock_send_data(i, d);
        }
        debug(DBG_GLOBAL, 1, "written: %d out of %d", nwrote, d->dsize);
        if(nwrote + cdata[i].written == d->dsize) {
          debug(DBG_GLOBAL, 1, "done writing");
          dunlock(d);
          rpop(&cdata[i].xmit_list);    /* and delete the item off the list */
          cdata[i].written = 0; /* did not write anything yet from the next one */
          cdata[i].tx_count++;
        } else if(nwrote < d->dsize) {
          // Retry to write more data later
          cdata[i].written += nwrote;
          debug(DBG_GLOBAL, 1, "retry writing later");
        }
      } else {
        // empty data buffer means time to close.
        debug(DBG_GLOBAL, 10, "Seen empty xmit buf - closing index %d", i);
        close_idx(i, u_ptr);
      }
      if(cdata[i].xmit_list == NULL && 0) {
        debug(DBG_GLOBAL, 10, "Nothing left to send - closing index %d", i);
        close_idx(i, u_ptr);
      }
    }
    /*
     * if the list was or has become empty - we're not 
     * interested in knowing when we can write anymore
     */
    if(cdata[i].xmit_list == NULL && cdata[i].do_ssl == 0) {
      ufds[i].events &= ~POLLOUT;
      // TODO: also handling of the keepalive timer ? 
      /*
         debug(DBG_GLOBAL, 1, "Index %d was without keepalive, closing.", i);
         close(ufds[i].fd); ufds[i].fd = -1; 
       */
    }
  }
}

/**
 * Try to compact the nfds and cdata structures,
 * under the assumption that something got closed.
 * TODO: make this conditional.
 */

void
sock_remove_closed_fds(void)
{
  int i;

  for(i = nfds - 1; i >= 0; i--) {
    if(ufds[i].fd == -1) {
      ufds[i].events = 0;
      if(ufds[i].fd == nfds - 1) {
        nfds--;
      }
    }
  }
#if 0
  for(i = 0; i < nfds; i++) {
    if(ufds[i].fd == -1) {
      while(ufds[nfds - 1].fd == -1 && nfds > 0 && i < nfds) {
        nfds--;
      }
      if(i < nfds) {
        ufds[i].fd = ufds[nfds - 1].fd;
        ufds[i].events = ufds[nfds - 1].events;
        memcpy(&cdata[i], &cdata[nfds - 1], sizeof(cdata[0]));  /* also move the connection data */
        nfds--;
      }
    }
  }
#endif
}

int sock_one_cycle(int timeout, void *u_ptr) {
    int i;
    int nevents = 0;
    int nsocks = 0;
    int nfdscheck = nfds;

    debug(DBG_GLOBAL, 125, "poll(%d)", nfds);
    debug_redraw_if_needed();
    nsocks = poll(ufds, nfdscheck, timeout);
    if (nsocks == -1) {
      struct rlimit rlim;
      getrlimit(RLIMIT_NOFILE, &rlim);
      nfdscheck = rlim.rlim_cur-1;
      debug(DBG_GLOBAL, 0, "Error while poll: %s, rlimit on file#: %d(max %d)", 
        strerror(errno), rlim.rlim_cur, rlim.rlim_max, nfdscheck);
      return -1;
    }
    for(i = 0; i < nfdscheck; i++) {
      if(cdata[i].listener && cdata[i].is_udp == 0) {
        // TCP listener sockets 
        if(ufds[i].revents & POLLIN) {
          sock_tcp_listener_pollin(i, u_ptr);
          nevents++;
        }
      } else {
        // non-listener sockets - this includes UDP
        if(ufds[i].revents & POLLIN) {
          debug(DBG_GLOBAL, 10, "%d: POLLIN", i);
          if(cdata[i].connected == 0) {
            sock_unconnected_pollinout(i, u_ptr);
          } else {
            sock_connected_pollin(i, u_ptr);
          }
          nevents++;
        }
        if(ufds[i].revents & POLLOUT) {
          debug(DBG_GLOBAL, 10, "%d: POLLOUT", i);
          if(cdata[i].connected == 0) {
            debug(DBG_GLOBAL, 11, "..on a nonconnected socket...");
            sock_unconnected_pollinout(i, u_ptr);
          } else {
            debug(DBG_GLOBAL, 11, "on connected...");
            sock_connected_pollout(i, u_ptr);
          }
          nevents++;
        }
        if(ufds[i].revents & POLLERR) {
          debug(DBG_GLOBAL, 11, "%d: POLLERR", i);
        }
        if(ufds[i].revents & POLLHUP) {
          debug(DBG_GLOBAL, 11, "%d: POLLHUP", i);
        }
        if(ufds[i].revents & POLLNVAL) {
          debug(DBG_GLOBAL, 11, "%d: POLLNVAL", i);
        }
        ufds[i].revents = 0;
      }
    }
    sock_remove_closed_fds();
    return nevents;
}


