

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
#include <pcap/pcap.h>
#ifdef LINUX
#include <netpacket/packet.h>
#endif
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip6.h>

#include "lib_sock_intern.h"
#include "dbuf-ay.h"
#include "debug-ay.h"
#include "lists-ay.h"
#include "os-ay.h"
#include "sock-pcap-ay.h"
/*
#include "timers-ay.h"
*/

char *pcap_usig = "PCAP socket info";

int 
pcap_recv(int idx, dbuf_t *d, void *private) 
{
  struct pcap_pkthdr ph;
  pcap_t *pcap = private;
  void *p = (void*) pcap_next(pcap, &ph);
  d->dsize = ph.caplen;
  memcpy(d->buf, p, ph.caplen);
  return d->dsize;
}

int 
pcap_send(int idx, dbuf_t *d, void *private) 
{
  pcap_t *pcap = private;
  int nwrote = pcap_inject(pcap, &d->buf[0], d->dsize);
  debug(DBG_GLOBAL, 11, "Wrote %d bytes to pcap out " 
            "of %d", nwrote, 
	    d->dsize);
  return nwrote;

}

/* Allocate structure to stuff various address info */
int pcap_alloc_info(int idx, char *dev) {

  static char errbuf[PCAP_ERRBUF_SIZE];
  pcap_if_t *all;
  pcap_if_t *pdev;
  pcap_addr_t *addr;
  pcap_socket_info_t *psi;
  int found_mac = 0;
  int found_v6 = 0;

  if (pcap_findalldevs(&all, errbuf) != 0) {
    debug(DBG_GLOBAL, 0, "pcap_findalldevs failed");
    return 0;
  }
  dbuf_t *d = dalloc(sizeof(pcap_socket_info_t));
  if (!d) {
    pcap_freealldevs(all);
    return 0;
  }
  dsetusig(d, pcap_usig);
  psi = (pcap_socket_info_t *)d->buf;
  memset(psi, 0, sizeof(*psi));

  for(pdev = all; pdev != NULL; pdev = pdev->next) {
    if (strcmp(dev, pdev->name) == 0) {
      for(addr = pdev->addresses; addr != NULL; addr = addr->next) {
        printf("AF: %d\n", addr->addr->sa_family);
#ifdef LINUX
	if (addr->addr->sa_family == AF_PACKET) {
	  struct sockaddr_ll *s = (void *) (addr->addr);
	  if (s->sll_halen == 6) {
	    memcpy(psi->mac_buf, s->sll_addr, 6);
	    psi->mac = psi->mac_buf;
	    found_mac = 1;
	  }
	} 
#endif
        if (addr->addr->sa_family == AF_INET6) {
	  struct sockaddr_in6 *s = (void *) (addr->addr);
	  if (IN6_IS_ADDR_LINKLOCAL(&s->sin6_addr)) {
	    psi->v6addr_buf = s->sin6_addr;
	    psi->v6addr = &psi->v6addr_buf;
	    found_v6 = 1;
	  }
	}
      }
    }
  }
  pcap_freealldevs(all);
  cdata_set_appdata_dbuf(idx, d);
  return (found_mac && found_v6);
}


int attach_pcap_with_filter(char *dev, char *filter)
{
  char errbuf[PCAP_ERRBUF_SIZE] = "";
  int s;
  
  pcap_t *pcap = pcap_open_live(dev, CHUNK_SZ, 1, 1, errbuf);
  
  if (pcap) {
    if (filter) {
      struct bpf_program bpf;
      int res = pcap_compile(pcap, &bpf, filter, 0, PCAP_NETMASK_UNKNOWN);
      if (res) {
        debug(DBG_GLOBAL, 0, "Could not get compile filter for a PCAP on %s: %s", dev, errbuf);
        return -1; 
      }
      res = pcap_setfilter(pcap, &bpf);
      if (res) {
        debug(DBG_GLOBAL, 0, "Could not get attach filter for a PCAP on %s: %s", dev, errbuf);
        return -1;
      }
    }
    int idx = sock_make_new(pcap_fileno(pcap), pcap);
    pcap_setnonblock(pcap, 1, errbuf);
    pcap_setdirection(pcap, PCAP_D_IN);
    sock_set_hooks(idx, pcap_send, pcap_recv);
    int res = pcap_alloc_info(idx, dev);
    debug(DBG_GLOBAL, 1, "PCAP%s on device (%s) added to index %d", (res ? "(with info)" : ""), dev, idx);
    return idx;
  } else {
    debug(DBG_GLOBAL, 0, "Could not get PCAP on %s: %s", dev, errbuf);
    return -1;
  }
}

int attach_pcap(char *dev) {
  return attach_pcap_with_filter(dev, NULL);
}

pcap_socket_info_t *get_pcap_socket_info(int idx) {
  dbuf_t *d = cdata_get_appdata_dbuf(idx, pcap_usig);
  if (d) {
    return (pcap_socket_info_t *) d->buf;
  } else {
    return NULL;
  }
}


