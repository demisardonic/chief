#include <stdlib.h>
#include <string.h>

#include "cbuf.h"

void cbuf_free(cbuf_t *cb){
  free(cb->b);
}

void cbuf_append(cbuf_t *cb, const char *b, const int l){
  char *new_b = realloc(cb->b, cb->l + l);
  if(!new_b) return;
  memcpy(&new_b[cb->l], b, l);
  cb->b = new_b;
  cb->l += l;
}

void cbuf_move(cbuf_t *cb, const int x, const int y){

}

void cbuf_clear(cbuf_t *cb){
}
