#ifndef __REASM_H__
#define __REASM_H__

void *make_reasm_pile();

dbuf_t *dtry_reasm(void *pile, uint32_t xid, char *data, uint16_t len, uint16_t offs, int more);

#endif

