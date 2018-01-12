#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbuf.h"
#include "term.h"
#include "util.h"

int cbuf_free(cbuf_t *cb){
  free(cb->b);
  free(cb);
  return 0;
}

cbuf_t *cbuf_create(void){
  cbuf_t *cb = (cbuf_t *) malloc(sizeof(cbuf_t));
  if(!cb) return NULL;

  cb->s = 10;
  cb->l = 0;
  cb->fg = 0;
  cb->bg = 9;
  cb->b = (char *) malloc(sizeof(char) * cb->s);

  if(!cb->b) return NULL;

  return cb;
}

int cbuf_append(cbuf_t *cb, const char *b, const int l){
  char *buffer;
  if(cb->l + l >= cb->s){
    buffer = (char *) realloc(cb->b, cb->s * 2);
    if(buffer){
      cb->s = cb->s * 2;
    }else{
      return 1;
    }
  }else{
    buffer = cb->b;
  }
  memcpy(buffer + cb->l, b, l);
  cb->b = buffer;
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

int cbuf_bar(cbuf_t *cb){
  int width = chief.w - 1;
  char bar[width];
  int len = sprintf(bar, "%c%s:%d", (chief.dirty > 0 ? '*' : ' '), chief.filepath, EFF_CY);
  cbuf_color(cb, 1, 7);

  cbuf_move(cb, 0, chief.h - 2);
  cbuf_append(cb, "\x1b[K", 3);
  cbuf_append(cb, bar, len);
  width -= len;
  while(--width >= 1){
    cbuf_append(cb, " ", 1);
  }
  cbuf_color(cb , COLOR_RESET);
  cbuf_append(cb, "\r\n", 2);

  cbuf_append(cb, "\x1b[K", 3);
  cbuf_append(cb, chief.message, chief.m_len);
  
  return 0;
}

int cbuf_color(cbuf_t *cb, int fg, int bg){
  if(fg == -1 && bg == -1){
    cbuf_appendf(cb, "\x1b[0m");
    return 0;
  }
  fg = RANGE(fg, 0, 7);
  if(bg < 0 || bg > 7){
    bg = 9; //bg 9 is transparent
  }
  cb->fg = fg;
  cb->bg = bg;
  cbuf_appendf(cb, "\x1b[%d;%dm", 30+fg, 40+bg);
  return 0;
}
