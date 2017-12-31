#ifndef CBUF_H
#define CBUF_H

#define NEW_CBUF {NULL, 0}

//Character buffer
typedef struct cbuf{
  char *b;
  int l;
}cbuf_t;

int cbuf_free(cbuf_t *cb);
int cbuf_append(cbuf_t *cb, const char *b, int l);
int cbuf_move(cbuf_t *cb, const int x, const int y);
int cbuf_clear(cbuf_t *cb);
int cbuf_bar(cbuf_t *cb);

#endif
