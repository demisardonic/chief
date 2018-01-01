#ifndef CBUF_H
#define CBUF_H

#define NEW_CBUF {NULL, 0}

#define COLOR_RESET -1, -1
#define COLOR_BLK    0,  9
#define COLOR_RED    1,  9
#define COLOR_GRN    2,  9
#define COLOR_YLW    3,  9
#define COLOR_BLU    4,  9
#define COLOR_PUR    5,  9
#define COLOR_CYN    6,  9
#define COLOR_WHT    7,  9

//Character buffer
typedef struct cbuf{
  char *b;
  int l;
  int fg;
  int bg;
}cbuf_t;

int cbuf_free(cbuf_t *cb);
int cbuf_append(cbuf_t *cb, const char *b, int l);
int cbuf_appendf(cbuf_t *cb, const char *fmt, ...);
int cbuf_move(cbuf_t *cb, const int x, const int y);
int cbuf_clear(cbuf_t *cb);
int cbuf_bar(cbuf_t *cb);
int cbuf_color(cbuf_t *cb, int fg, int bg);

#endif
