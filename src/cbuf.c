#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbuf.h"

int cbuf_free(cbuf_t *cb){
  if(cb){
    free(cb->buffer);
    cb->buffer = NULL;
    free(cb);
    return 1;
  }
  fprintf(stderr, "Cannot free a NULL cbuf_t.\n");
  return 0;
}

cbuf_t *cbuf_create(void){
  cbuf_t *cb = (cbuf_t *) malloc(sizeof(cbuf_t));
  if(!cb){
    fprintf(stderr, "Failed to allocate cbuf_t.\n");
    return NULL;
  }
  char *buffer = (char *) calloc(CBUF_BUFFER_SIZE, sizeof(char));
  if(!buffer){
    fprintf(stderr, "Failed to allocate cbuf_t buffer of size %d.\n", CBUF_BUFFER_SIZE);
    free(cb);
    return NULL;
  }
  cb->size = CBUF_BUFFER_SIZE;
  cb->len = 0;
  cb->buffer = buffer;
  return cb;
}

int cbuf_append(cbuf_t *cb, const char *b, int l){
  if(!cb){
    fprintf(stderr, "Cannot append to NULL cbuf_t.\n");
    return 0;
  }
  if(!b || l == 0){
    return 0;
  }

  int new_size = cb->len + l;
  //Resize the buffer to twice the length of the existing buffer plus the append buffer
  if(new_size > cb->size){
    char *larger_buffer = realloc(cb->buffer, new_size * 2 * sizeof(char));
    if(!larger_buffer){
      fprintf(stderr, "Failed to reallocate cbuf_t buffer to size %d.\n", new_size * 2);
      return 0;
    }
    cb->buffer = larger_buffer;
    cb->size = new_size * 2;
  }
  memcpy(cb->buffer + cb->len, b, l);
  cb->len += l;
  return 1;
}

//Variadic cbuf_append
int cbuf_appendf(cbuf_t *cb, const char *fmt, ...){
  char buffer[CBUF_APPENDF_LEN];
  int len;
  
  va_list argv;
  va_start(argv, fmt);
  len = vsnprintf(buffer, CBUF_APPENDF_LEN, fmt, argv);
  va_end(argv);

  if(len < 0 || len > CBUF_APPENDF_LEN){
    fprintf(stderr, "Failed to write variadic fmt to cbuf_appendf buffer.\n");
    return 0;
  }
  
  cbuf_append(cb, buffer, len);
  return 1;
}
