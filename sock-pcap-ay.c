

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

#include "lib_sock_intern.h"
#include "dbuf-ay.h"
#include "debug-ay.h"
#include "lists-ay.h"
#include "os-ay.h"
/*
#include "timers-ay.h"
*/


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

int attach_pcap(char *dev)
{
  char errbuf[PCAP_ERRBUF_SIZE] = "";
  int s;
  
  pcap_t *pcap = pcap_open_live(dev, CHUNK_SZ, 1, 1, errbuf);
  
  if (pcap) {
    int idx = sock_make_new(pcap_fileno(pcap), pcap);
    pcap_setnonblock(pcap, 1, errbuf);
    sock_set_hooks(pcap_send, pcap_recv);
    debug(DBG_GLOBAL, 1, "PCAP on device (%s) added to index %d", dev, idx);
    return idx;
  } else {
    debug(DBG_GLOBAL, 0, "Could not get PCAP on %s: %s", dev, errbuf);
    return -1;
  }
}

