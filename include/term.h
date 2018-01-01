#ifndef TERM_H
#define TERM_H

#include <stdlib.h>
#include <termios.h>

#include "row.h"

typedef struct term{
  int w;
  int h;
  struct termios orig_termios;
  int cx;
  int cy;
  char *message;
  int m_len;
  row_t *rows;
  int num_rows;
}term_t;

enum keys{
  ARROW_UP = 0,
  ARROW_LEFT = 1,
  ARROW_DOWN = 2,
  ARROW_RIGHT = 3,
  HOME_KEY = 4,
  END_KEY = 5
};

extern term_t chief;

void reset_terminal();
void initialize_terminal();
void free_terminal();

void terminal_loop();
char read_input();

void clear_terminal();
int get_terminal_size(int *width, int *height);
void render_terminal();
void set_message(const char *m, ...);

#endif
