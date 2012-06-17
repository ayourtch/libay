
/**************************************************************************
*
*  Copyright © 2012 Andrew Yourtchenko, ayourtch@gmail.com.
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

#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <sys/queue.h>
#include "dbuf-ay.h"
#include "debug-ay.h"
#include "hash-ay.h"
#include "reasm-ay.h"

/**********************************************

The contents of each chunk being reassembled are stored in dbuf,
at their correct places. 

We require that the fragments are made on 8 byte boundary,
and the maximum reassembled PDU size is 64K - so that
both IPv4 and IPv6 reassembly could be emulated.

This gives the possibility to track the reassembly process by using
the not-yet-received pieces within dbuf as the tracking mechanism.

Here is how it looks:

                      2 b     4..n     2b 
------|DATADATADATA| hllen |  empty | dlen |DATADATADATA|---- 


The problem exists if the first chunk of the reassembly buffer
gets filled - then we do not know the length of this piece, thus 
we can not access the lengths in the next empty piece.

To help with that, we have the "hole" field, which tells us the offset 
of the very first empty chunk.


***********************************************/



typedef struct reasm_chunk_t {
  uint32_t xid;  
  time_t deadline; /* reassembly deadline */
  time_t atime; /* last access time */
  uint16_t maxfrags;
  dbuf_t *d;
  uint16_t hole; /* the start of the first piece that is not filled */
  uint16_t esize; /* expected size. Set either to {M|F}TU, or by last frag */
  TAILQ_ENTRY(reasm_chunk_t) entries;
} reasm_chunk_t;

typedef struct reasm_pile_struct_t {
  htable_t *chs; /* chunks in reassembly */
  uint16_t maxfrags;  /* max frags per chunk for reassembly */
  int mtu;  /* max PDU size */
  int ftu;  /* frequent PDU size */ 
  time_t reasm_timeout; /* how many sec to keep the state for reassembly */
  TAILQ_HEAD(lru_head, reasm_chunk_t) lru;
} reasm_pile_struct_t;

#define HOLE_MIN_LENGTH (sizeof(uint16_t) + sizeof(uint16_t))

static inline uint16_t *du16(dbuf_t *d, int idx) {
  return (uint16_t *) &d->buf[idx];
}

static inline uint16_t *du16d(dbuf_t *d, int idx) {
  return (uint16_t *) du16(d, idx + *du16(d, idx) - 2);
}

void *make_reasm_pile() {
   reasm_pile_struct_t *rp;
   dbuf_t *d = dalloc(sizeof(reasm_pile_struct_t));
   rp = (void*) d->buf;
   TAILQ_INIT(&rp->lru);
   
   rp->chs = halloc(4, 16);
   rp->maxfrags = 32;
   rp->reasm_timeout = 20;
   rp->mtu = 500* 8;
   rp->ftu = 250* 8;

   rp->mtu = 50* 8;
   rp->ftu = 25* 8;

   return d;
}


void hole_set(reasm_chunk_t *chk, uint16_t offs, uint16_t holelen, uint16_t datalen) {
  dbuf_t *d = chk->d;
  assert((holelen & 7) == 0);
  assert(holelen > 0);
  *du16(d, offs) = holelen;
  *du16d(d, offs) = datalen;
}

int contains(int a, int b, int spot) {
  return (a<=spot) && (b>spot);
}


int hole_find(reasm_chunk_t *chk, uint16_t spot, uint16_t *hoffs, uint16_t *hoffs_prev) {
  dbuf_t *d = chk->d;
  uint16_t *px;
  int ix = chk->hole;
  while (!contains(ix, ix + *du16(d, ix), spot)) {
    if (ix >= d->size) return 0; /* didn't find and walked past the block size */
    if (hoffs_prev) *hoffs_prev = ix;
    ix += (*du16(d, ix) + *du16d(d, ix));
  }
  if (hoffs) {
    *hoffs = ix;
  }
  return 1;
}

/* 
 * NB: this assumes this is the last hole, so it does not have data 
 * The reassembly process never has the holes growing in the middle.
 */
int hole_grow(reasm_chunk_t *chk, uint16_t spot, uint16_t newsz) {
  dbuf_t *d = chk->d;
  uint16_t hoffs;
  uint16_t *px;
  if(!hole_find(chk, spot, &hoffs, NULL)) return 0;
  *du16(d, hoffs) = newsz;
  *du16d(d, hoffs) = 0;
  return 1;
}

void reset_timeout(reasm_pile_struct_t *rp, reasm_chunk_t *chk) {
  chk->deadline = chk->atime + rp->reasm_timeout;
  TAILQ_REMOVE(&rp->lru, chk, entries);
  TAILQ_INSERT_TAIL(&rp->lru, chk, entries);
}


void dispose_chk(reasm_pile_struct_t *rp, reasm_chunk_t *chk) {
  hdelete(rp->chs, &chk->xid, sizeof(chk->xid), NULL, NULL, NULL);
  TAILQ_REMOVE(&rp->lru, chk, entries);
  /* this is called from callback destructor => free(chk); */
}
  

/* Check if the reassembly is done for the PDU, and if yes, 
   perform the necessary cleanups and return the ready dbuf */

dbuf_t *check_reasm_done(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {
  dbuf_t *d = chk->d;

  if (!more) {
    debug(DBG_REASM, 20, "Set desired length %d <= %d + %d\n", chk->esize, offs, len);
    chk->esize = offs+len;
  }
  memcpy(&chk->d->buf[offs], data, len);
  reset_timeout(rp, chk);
  if (d->dsize < offs+len) {
    d->dsize = offs+len;
  }
  debug(DBG_REASM, 20, "Offs: %d, len: %d, chk->esize: %d, chk->hole: %d, d->dsize: %d", 
        offs, len, chk->esize, chk->hole, d->dsize);
  debug_dump(DBG_REASM, 100, d->buf, d->size);
  if (chk->esize == chk->hole) {
    /*
     * reassembly complete. Delete chunk, and return the dbuf.
     * do not unlock since we should have locked it anyway
     */
    d->dsize = chk->esize;
    dispose_chk(rp, chk);
    return d;
  }
  return NULL;
}

  /*
   * 1) ------HHHHHHHHHH------
   *          DDDDDDDDDD        => exact fill
   */

dbuf_t *hole_fill_exact(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {

  debug(DBG_REASM, 20, "hole_fill_exact hoffs1: %d, hoffs0: %d, data: %s, len: %d, offs: %d, more: %d", 
        hoffs1, hoffs0, data, len, offs, more);

  if(chk->hole == hoffs1) {
    /* was a first hole - just update the pointer in the chunk */
    chk->hole += *du16(chk->d, hoffs1) + *du16d(chk->d, hoffs1);
  } else {
    *du16d(chk->d, hoffs0) += *du16(chk->d, hoffs1) + *du16d(chk->d, hoffs1);
  }
  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}


  /*  
   * 2) ------HHHHHHHHHH------
   *          DDDDD             => data in the start
   */

dbuf_t *hole_fill_begin(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {

  debug(DBG_REASM, 20, "hole_fill_begin hoffs1: %d, hoffs0: %d, data: %s, len: %d, offs: %d, more: %d", 
        hoffs1, hoffs0, data, len, offs, more);
  if (*du16(chk->d, hoffs1) - len < HOLE_MIN_LENGTH) {
    debug(DBG_REASM, 0, "Attempting to hole_fill_begin with too small resulting hole (%d - %d), abort.", *du16(chk->d, hoffs1), len);
    return NULL;
  }

  /* prepare the new head of hole - store the shorter length at new start */
  *du16(chk->d, hoffs1 + len) = *du16(chk->d, hoffs1) - len;

  if(chk->hole == hoffs1) {
    chk->hole += len;
  } else {
    *du16d(chk->d, hoffs0) += len;
  }
  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}

  /*
   * 3) ------HHHHHHHHHH------
   *               DDDDD        => data in the end
   */

dbuf_t *hole_fill_end(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {

  debug(DBG_REASM, 20, "hole_fill_end hoffs1: %d, hoffs0: %d, data: %s, len: %d, offs: %d, more: %d", 
        hoffs1, hoffs0, data, len, offs, more);
  /* 
   * We're copying at the end of hole so we increase 
   * the data portion of it, decrease the empty len, and move 
   * the data len record more to the front, to end of empty space 
   */
  uint16_t odlen = *du16d(chk->d, hoffs1);
  if (*du16(chk->d, hoffs1) - len < HOLE_MIN_LENGTH) {
    debug(DBG_REASM, 0, "Attempting to hole_fill_end with too small resulting hole (%d - %d), abort.", *du16(chk->d, hoffs1), len);
    return NULL;
  }
  *du16(chk->d, hoffs1) -= len;
  *du16d(chk->d, hoffs1) = odlen + len;

  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}

  /*
   * 4) ------HHHHHHHHHH------
   *             DDDD           => data in the middle
   */

dbuf_t *hole_fill_middle(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {

  uint16_t ohlen = *du16(chk->d, hoffs1);

  debug(DBG_REASM, 20, "hole_fill_middle hoffs1: %d, hoffs0: %d, data: %s, len: %d, offs: %d, more: %d", 
        hoffs1, hoffs0, data, len, offs, more);

  if (offs - hoffs1 < HOLE_MIN_LENGTH) {
    debug(DBG_REASM, 0, "Attempting to hole_fill_middle with too small resulting hole in the start (%d - %d), abort.", 
          offs, hoffs1);
    return NULL;
  }
  if (ohlen - (offs - hoffs1) - len < HOLE_MIN_LENGTH) {
    debug(DBG_REASM, 0, "Attempting to hole_fill_middle with too small end hole %d=(%d-(%d-%d)-%d), abort.", 
          ohlen - (offs - hoffs1) - len, ohlen, offs, hoffs1, len);
    return NULL;
  }

  *du16(chk->d, hoffs1) = offs - hoffs1;
  *du16d(chk->d, hoffs1) = len;
  /* make a new hole */
  *du16(chk->d, offs+len) = ohlen - (offs - hoffs1) - len;

  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}


/*
 * called by dtry_reasm after it has prepared the dbuf as necessary - expanded,
 * verified that the data can fit, etc.
 */

dbuf_t *dperform_reasm(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint32_t xid, char *data, 
                       uint16_t len, uint16_t offs, int more) {
  uint16_t hoffs0 = 0xfafa;
  uint16_t hoffs1;
  uint16_t hoffs2;
  dbuf_t *d = chk->d;

  /* Check that the suitable hole exists and the fragment fits into it */
  
  if (!hole_find(chk, offs, &hoffs1, &hoffs0)) { 
    debug(DBG_REASM, 10, "No hole for start %d in chunk %lu", offs, xid);
    return NULL;
  }
  if (!hole_find(chk, offs+len-1, &hoffs2, NULL)) {
    debug(DBG_REASM, 10, "No hole for end (%d+%d=%d) in chunk %lu", offs, len, offs+len, xid);
    return NULL;
  }
  if (hoffs1 != hoffs2) {
    debug(DBG_REASM, 10, "Different holes for (%d+%d) in chunk %lu", offs, len, xid);
    return NULL; 
  }
  debug(DBG_REASM, 10, "Found holes %d for (%d, %d) in chunk %lu", hoffs1, offs, len, xid);
  if (!more && ((chk->esize < chk->d->size) || 
                ( (chk->d->dsize > 0) && (chk->d->dsize < chk->esize) && (offs < chk->d->dsize)) 
               ) ) { 
    debug(DBG_REASM, 10, "MF=0 in the middle of the packet, discard (offs: %d, len: %d), esize: %d, dsize: %d", 
                         offs, len, chk->esize, chk->d->dsize);
    return NULL;
  }

  /* Do the reassembly  */

  /* 
   * There can be 4 situations:
   *   
   * 1) ------HHHHHHHHHH------
   *          DDDDDDDDDD        => exact fill
   * 2) ------HHHHHHHHHH------
   *          DDDDD             => data in the start
   * 3) ------HHHHHHHHHH------
   *               DDDDD        => data in the end
   * 4) ------HHHHHHHHHH------
   *             DDDD           => data in the middle
   * all the other cases have been taken care of by the checks
   * above, since both start and the end fit within the same hole.
   *
   */
  
  if (hoffs1 == offs && *du16(d, hoffs1) == len) 
    return hole_fill_exact(rp, chk, hoffs1, hoffs0, data, len, offs, more); 
  if (hoffs1 == offs && *du16(d, hoffs1)  > len) 
    return hole_fill_begin(rp, chk, hoffs1, hoffs0, data, len, offs, more); 
  if (hoffs1 < offs && hoffs1 + *du16(d, hoffs1)  == offs + len) 
    return hole_fill_end(rp, chk, hoffs1, hoffs0, data, len, offs, more); 
  if (hoffs1 < offs && hoffs1 + *du16(d, hoffs1)  > offs + len) 
    return hole_fill_middle(rp, chk, hoffs1, hoffs0, data, len, offs, more); 
  
  /* we do not reach here */
  assert(0);

  return NULL;
}

int chk_destructor(void *key, int key_len, void *data_old, void *data_new) {
  free(data_old);
  return 1;
}

void check_timeouts(reasm_pile_struct_t *rp, time_t now) {
  reasm_chunk_t *chk; 
  reasm_chunk_t *chk_next;
  debug(DBG_REASM, 50, "LRU Traversing start, now: %ld", now);
  for (chk = rp->lru.tqh_first; chk != NULL; chk = chk_next) {
    chk_next = chk->entries.tqe_next;
    debug(DBG_REASM, 50, "LRU chunk xid: %lu, deadline: %ld", chk->xid, chk->deadline); 
    if (chk->deadline < now) {
      debug(DBG_REASM, 50, "chunk xid %lu deadline (%ld) < now (%ld), deleting", chk->xid, chk->deadline, now);
      dunlock(chk->d);
      dispose_chk(rp, chk);
    }
  }
  debug(DBG_REASM, 50, "LRU Traversing end");
}

dbuf_t *dtry_reasm_timed(void *pile, uint32_t xid, char *data, uint16_t len, uint16_t offs, int more, time_t now) {
  reasm_pile_struct_t *rp = (void *)(((dbuf_t *)pile)->buf);
  reasm_chunk_t *chk;
  
  if (now > 0) {
    check_timeouts(rp, now);
  }

  if(offs + len > rp->mtu) {
    debug(DBG_REASM, 10, "Offset + length (%d + %d) of fragment > MTU (%d), discard", offs, len, rp->mtu); 
    return NULL;
  }
  if ((offs > 0) && (offs < HOLE_MIN_LENGTH)) {
    debug(DBG_REASM, 10, "Offset %d less than min hole length %d\n", offs, HOLE_MIN_LENGTH);
    return NULL;
  }
  
  chk = hfind(rp->chs, &xid, sizeof(xid));
  if (!chk) {
    debug(DBG_REASM, 10, "Reasm chunk %lu not found, creating", xid); 
    chk = malloc(sizeof(reasm_chunk_t));
    chk->xid = xid;
    chk->maxfrags = rp->maxfrags;
    chk->hole = 0;
    chk->esize = rp->ftu;
    chk->d = dalloc(chk->esize);
    chk->deadline = now + rp->reasm_timeout;
    memset(chk->d->buf, 0xaa, chk->d->size);
    hole_set(chk, 0, chk->esize, 0);
    hinsert(rp->chs, &xid, sizeof(xid), chk, chk_destructor, NULL, NULL, NULL); 
    TAILQ_INSERT_TAIL(&rp->lru, chk, entries);
  } else {
    debug(DBG_REASM, 10, "Reasm chunk %lu found", xid); 
  }
  debug(DBG_REASM, 100, "Chunk data (hole: %d, esize: %d):", chk->hole, chk->esize); 
  debug_dump(DBG_REASM, 100, chk->d->buf, chk->d->size);

  if(offs + len > chk->d->size) {
    debug(DBG_REASM, 10, "Reasm chunk %lu overflow - %d + %d > %d", xid, offs, len, chk->d->size); 
    /* We already checked the MTU overflow above, so we can safely reallocate here */
    int oldsize = chk->d->size;
    dgrow(chk->d, rp->mtu - chk->d->size);
    hole_grow(chk, oldsize-1, chk->d->size);
    chk->esize = chk->d->size;
    debug(DBG_REASM, 100, "Fresh chunk data after growth to MTU:"); 
    debug_dump(DBG_REASM, 100, chk->d->buf, chk->d->size);
  } 
  chk->atime = now;


  return dperform_reasm(rp, chk, xid, data, len, offs, more);
}


dbuf_t *dtry_reasm(void *pile, uint32_t xid, char *data, uint16_t len, uint16_t offs, int more) {
  return dtry_reasm_timed(pile, xid, data, len, offs, more, 0);
}
