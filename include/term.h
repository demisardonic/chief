#ifndef TERM_H
#define TERM_H

#include <stdlib.h>
#include <termios.h>

typedef struct term{
  int w;
  int h;
  struct termios orig_termios;
  int cx;
  int cy;
  char *message;
  int m_len;
}term_t;

extern term_t chief;

void reset_terminal();
void initialize_terminal();
void free_terminal();
void terminal_loop();
char read_input_byte();
void clear_terminal();
int get_terminal_size(int *width, int *height);
void render_terminal();
void set_message(const char *m, int len);

#endif
