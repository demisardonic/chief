#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbuf.h"
#include "term.h"

int cbuf_free(cbuf_t *cb){
  free(cb->b);
  return 0;
}

int cbuf_append(cbuf_t *cb, const char *b, const int l){
  char *new_b = realloc(cb->b, cb->l + l);
  if(!new_b) return 1;
  memcpy(&new_b[cb->l], b, l);
  cb->b = new_b;
  cb->l += l;
  return 0;
}

int cbuf_move(cbuf_t *cb, const int x, const int y){
  if(x < 0 || y < 0 || x >= chief.w || y >= chief.h)
    return 1;
  char tmp[64];
  memset(tmp, 0, 64);
  int len = snprintf(tmp, 64, "\x1b[%d;%dH", y+1, x+1);
  cbuf_append(cb, tmp, len);
  return 0;
}

int cbuf_clear(cbuf_t *cb){
  cbuf_append(cb, "\x1b[2J", 3);
  return 0;
}

int cbuf_bar(cbuf_t *cb){
  int width = chief.w - 1;
  cbuf_move(cb, 0, chief.h - 1);
  cbuf_append(cb, "\u250f", 3);
  while(--width >= 1){
    cbuf_append(cb, "\u2501", 3);
  }
  cbuf_append(cb, "\u2513", 3);
  cbuf_append(cb, "\r\n", 2);
  return 0;
}
