#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbuf.h"
#include "term.h"
#include "util.h"

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

int cbuf_appendf(cbuf_t *cb, const char *fmt, ...){
  char buffer[256];
  int len;
  
  va_list argv;
  va_start(argv, fmt);
  len = vsprintf(buffer, fmt, argv);
  va_end(argv);

  cbuf_append(cb, buffer, len);
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
  cbuf_move(cb, 0, chief.h - 2);
  cbuf_append(cb, "\u250f", 3);
  while(--width >= 1){
    cbuf_append(cb, "\u2501", 3);
  }
  cbuf_append(cb, "\u2513", 3);
  cbuf_append(cb, "\x1b[K", 3);
  cbuf_append(cb, "\u2503", 3);
  cbuf_append(cb, chief.message, chief.m_len);
  cbuf_move(cb, chief.w-1, chief.h - 1);
  cbuf_append(cb, "\u2503", 3);

  return 0;
}

int cbuf_color(cbuf_t *cb, int fg, int bg){
  if(fg == -1 && bg == -1){
    cbuf_appendf(cb, "\x1b[0m");
    return 0;
  }
  fg = RANGE(fg, 0, 7);
  if(bg < 0 || bg > 7){
    //bg 9 is transparent
    bg = 9;
  }
  cb->fg = fg;
  cb->bg = bg;
  cbuf_appendf(cb, "\x1b[%d;%dm", 30+fg, 40+bg);
  return 0;
}
