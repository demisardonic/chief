#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

//Clears the terminal and moves cursor to upper-left corner
void clear_terminal(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void err(const char *error){
  clear_terminal();
  perror(error);
  exit(1);
}
