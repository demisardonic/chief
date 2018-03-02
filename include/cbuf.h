#ifndef __CBUF_H
#define __CBUF_H

#define CBUF_APPENDF_LEN 1024
#define CBUF_BUFFER_SIZE 64

typedef struct cbuf cbuf_t;

struct cbuf {
  char *buffer;
  int len;
  int size;
};

int cbuf_free(cbuf_t *cb);
cbuf_t *cbuf_create(void);
int cbuf_append(cbuf_t *cb, const char *b, int l);
int cbuf_appendf(cbuf_t *cb, const char *fmt, ...);

#endif
