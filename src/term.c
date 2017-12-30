#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "term.h"
#include "util.h"

//Holds default terminal settings
struct termios orig_termios;

void reset_terminal(){
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) err("tcsetattr");
}

void initialize_terminal(){ 
  if(tcgetattr(STDIN_FILENO, &orig_termios) == -1) err("tcgetattr");
  //Reset terminal to initial state
  atexit(reset_terminal);

  struct termios raw = orig_termios;

  //Echoing letters to terminal
  tcflag_t i_mask = ECHO;
  //Read input byte by byte instead of line by line
  i_mask |= ICANON;
  //ctrl-c ctrl-z signals
  i_mask |= ISIG;
  //Turn off i_mask settings
  raw.c_lflag &= ~i_mask;

  //ctrl-m carriage return
  tcflag_t l_mask = ICRNL;
  //ctrl-s and crtl-q signal
  l_mask |= IXON;
  //Turn off l_mask settings
  raw.c_iflag &= ~l_mask;

  //Turn off output processing ie \n -> \r\n
  tcflag_t o_mask = OPOST;
  raw.c_oflag &= ~o_mask;

  //Disable blocking for input 1/10 second
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) err("tcsetattr");
}

void terminal_loop(){
  clear_terminal();
  draw_ui();
  
  while(1){
    char c = read_input_byte();
    switch(c){
    case CTRL_KEY('q'):
      return;
    }
  }
}

char read_input_byte(){
  int num_read;
  char c;
  while((num_read = read(STDIN_FILENO, &c, 1)) != 1){
    if(num_read == -1) err("read");
  }
  return c;
}

int get_terminal_size(int *width, int *height){
  struct winsize ws;
  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    *width = 0;
    *height = 0;
    return -1;
  }else{
    *width = ws.ws_col;
    *height = ws.ws_row;
    return 0;
  }
}

int move_terminal(int x, int y){
  int w, h;
  char buf[80];
  memset(buf, 0, 80);
  if(x < 0 || y < 0){
    return -1;
  }
  get_terminal_size(&w, &h);
  if(x >= w || y >= h){
    return -1;
  }
  sprintf(buf, "\x1b[%d;%dH", y, x);
  write(STDOUT_FILENO, buf, strlen(buf));
  return 0;
}
