#include "term.h"
#include "util.h"

term_t chief;

int main(int argc, char **argv){
  initialize_terminal();
  initialize_editor();
  terminal_loop();
  
  clear_terminal();

  free_terminal();
  return 0;
}
