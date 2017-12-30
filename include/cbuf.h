#ifndef CBUF_H
#define CBUF_H

#define NEW_CBUF {NULL, 0}

//Character buffer
typedef struct cbuf{
  char *b;
  int l;
}cbuf_t;

void cbuf_free(cbuf_t *cb);
void cbuf_append(cbuf_t *cb, const char *b, int l);

#endif
