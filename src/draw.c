#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "draw.h"
#include "term.h"

void clear_terminal(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void draw_row(int i){
  int width, h;
  get_terminal_size(&width, &h);
  move_terminal(0, i);
  for(width--; width >= 0; width--){
    printf("#");
    //write(STDOUT_FILENO, "#", 1);
  }
  printf("\r\n");
}

void draw_ui(){
  int width, height;
  get_terminal_size(&width, &height);
  draw_row(height - 1);
}
