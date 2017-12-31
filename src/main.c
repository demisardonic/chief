#include "term.h"
#include "util.h"

term_t chief;

int main(int argc, char **argv){
  initialize_terminal();

  terminal_loop();
  
  clear_terminal();
  return 0;
}
