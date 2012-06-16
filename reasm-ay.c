#include <stdint.h>
#include "dbuf-ay.h"
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
  uint32_t deadline; /* reassembly deadline */
  uint16_t maxfrags;
  dbuf_t *d;
  uint16_t hole; /* the start of the first piece that is not filled */
  uint16_t esize; /* expected size. Set either to {M|F}TU, or by last frag */
} reasm_chunk_t;

typedef struct reasm_pile_struct_t {
  htable_t *chs; /* chunks in reassembly */
  uint16_t maxfrags;  /* max frags per chunk for reassembly */
  int mtu;  /* max PDU size */
  int ftu;  /* frequent PDU size */
} reasm_pile_struct_t;

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
   
   rp->chs = halloc(4, 16);
   rp->maxfrags = 32;
   rp->mtu = 20000;
   rp->ftu = 2000;

   return d;
}


void hole_set(reasm_chunk_t *chk, uint16_t offs, uint16_t holelen, uint16_t datalen) {
  dbuf_t *d = chk->d;
  assert(holelen & 7 == 0);
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

/* Check if the reassembly is done for the PDU, and if yes, 
   perform the necessary cleanups and return the ready dbuf */

dbuf_t *check_reasm_done(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {
  dbuf_t *d = chk->d;

  if (!more) chk->esize = offs+len;
  if (chk->esize == chk->hole) {
    /*
     * reassembly complete. Delete chunk, and return the dbuf.
     * do not unlock since we should have locked it anyway
     */
    hdelete(rp->chs, &chk->xid, sizeof(chk->xid), NULL, NULL, NULL);
    /* this is called from callback destructor => free(chk); */
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

  if(chk->hole == hoffs1) {
    /* was a first hole - just update the pointer in the chunk */
    chk->hole += *du16(chk->d, hoffs1) + *du16d(chk->d, hoffs1);
  } else {
    *du16d(chk->d, hoffs0) += *du16(chk->d, hoffs1) + *du16d(chk->d, hoffs1);
  }
  memcpy(&chk->d->buf[offs], data, len);
  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}


  /*  
   * 2) ------HHHHHHHHHH------
   *          DDDDD             => data in the start
   */

dbuf_t *hole_fill_begin(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {

  /* prepare the new head of hole - store the shorter length at new start */
  *du16(chk->d, hoffs1 + len) = *du16(chk->d, hoffs1) - len;

  if(chk->hole == hoffs1) {
    chk->hole += len;
  } else {
    *du16d(chk->d, hoffs0) += len;
  }
  memcpy(&chk->d->buf[offs], data, len);
  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}

  /*
   * 3) ------HHHHHHHHHH------
   *               DDDDD        => data in the end
   */

dbuf_t *hole_fill_end(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {
  /* 
   * We're copying at the end of hole so we increase 
   * the data portion of it, decrease the empty len, and move 
   * the data len record more to the front, to end of empty space 
   */
  uint16_t odlen = *du16d(chk->d, hoffs1);
  *du16(chk->d, hoffs1) -= len;
  *du16d(chk->d, hoffs1) = odlen + len;

  memcpy(&chk->d->buf[offs], data, len);
  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}

  /*
   * 4) ------HHHHHHHHHH------
   *             DDDD           => data in the middle
   */

dbuf_t *hole_fill_middle(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint16_t hoffs1, uint16_t hoffs0,
                        char *data, uint16_t len, uint16_t offs, int more) {

  uint16_t ohlen = *du16(chk->d, hoffs1);
  *du16(chk->d, hoffs1) = offs - hoffs1;
  *du16d(chk->d, hoffs1) = len;
  /* make a new hole */
  *du16(chk->d, offs+len) = ohlen - (offs - hoffs1) - len;

  memcpy(&chk->d->buf[offs], data, len);
  return check_reasm_done(rp, chk, hoffs1, hoffs0, data, len, offs, more);
}


/*
 * called by dtry_reasm after it has prepared the dbuf as necessary - expanded,
 * verified that the data can fit, etc.
 */

dbuf_t *dperform_reasm(reasm_pile_struct_t *rp, reasm_chunk_t *chk, uint32_t xid, char *data, 
                       uint16_t len, uint16_t offs, int more) {
  uint16_t hoffs0;
  uint16_t hoffs1;
  uint16_t hoffs2;
  dbuf_t *d = chk->d;

  /* Check that the suitable hole exists and the fragment fits into it */
  
  if (!hole_find(chk, offs, &hoffs1, &hoffs0)) return NULL;
  if (!hole_find(chk, offs+len-1, &hoffs2, NULL)) return NULL;
  if (hoffs1 != hoffs2) return NULL; 

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
    return hole_fill_exact(rp, chk, hoffs1, hoffs0, data, offs, len, more); 
  if (hoffs1 == offs && *du16(d, hoffs1)  > len) 
    return hole_fill_begin(rp, chk, hoffs1, hoffs0, data, offs, len, more); 
  if (hoffs1 < offs && hoffs1 + *du16(d, hoffs1)  == offs + len) 
    return hole_fill_end(rp, chk, hoffs1, hoffs0, data, offs, len, more); 
  if (hoffs1 < offs && hoffs1 + *du16(d, hoffs1)  > offs + len) 
    return hole_fill_middle(rp, chk, hoffs1, hoffs0, data, offs, len, more); 
  
  /* we do not reach here */

  return NULL;
}

int chk_destructor(void *key, int key_len, void *data_old, void *data_new) {
  free(data_old);
  return 1;
}


dbuf_t *dtry_reasm(void *pile, uint32_t xid, char *data, uint16_t len, uint16_t offs, int more) {
  reasm_pile_struct_t *rp = pile;
  reasm_chunk_t *chk;

  if(offs + len > rp->mtu) return NULL;
  
  chk = hfind(rp->chs, &xid, sizeof(xid));
  if (!chk) {
    chk = malloc(sizeof(reasm_chunk_t));
    chk->xid = xid;
    chk->maxfrags = rp->maxfrags;
    chk->hole = 0;
    chk->esize = rp->ftu;
    chk->d = dalloc(chk->esize);
    hole_set(chk, 0, chk->esize, 0);
    hinsert(rp->chs, &xid, sizeof(xid), chk, chk_destructor, NULL, NULL, NULL); 
  }

  if(offs + len > chk->d->size) {
    /* We already checked the MTU overflow above, so we can safely reallocate here */
    int oldsize = chk->d->size;
    dgrow(chk->d, rp->mtu - chk->d->size);
    hole_grow(chk, oldsize-1, chk->d->size);
    chk->esize = chk->d->size;
  }

  return dperform_reasm(rp, chk, xid, data, len, offs, more);
}


