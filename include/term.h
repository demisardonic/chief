#ifndef TERM_H
#define TERM_H

#include <termios.h>

typedef struct editor{
  int w;
  int h;
  struct termios orig_termios;

}editor_t;

void reset_terminal();
void initialize_terminal();
void terminal_loop();
char read_input_byte();
void clear_terminal();
int get_terminal_size(int *width, int *height);
int move_terminal(int x, int y);
void draw_ui();
#endif
