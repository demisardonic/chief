#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "cbuf.h"
#include "term.h"
#include "util.h"

void reset_terminal(){
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &chief.orig_termios) == -1) err("tcsetattr");
}

void initialize_terminal(){
  if(tcgetattr(STDIN_FILENO, &chief.orig_termios) == -1) err("tcgetattr");
  atexit(reset_terminal);  //Reset terminal to initial state

  struct termios raw = chief.orig_termios;

  tcflag_t i_mask = ECHO;  //Echoing letters to terminal
  i_mask |= ICANON;  //Read input byte by byte instead of line by line
  i_mask |= ISIG;  //ctrl-c ctrl-z signals
  raw.c_lflag &= ~i_mask;  //Turn off i_mask settings

  tcflag_t l_mask = ICRNL;  //ctrl-m carriage return
  l_mask |= IXON;  //ctrl-s and crtl-q signal
  raw.c_iflag &= ~l_mask;  //Turn off l_mask settings

  tcflag_t o_mask = OPOST;  //Turn off output processing ie \n -> \r\n
  raw.c_oflag &= ~o_mask;

  //Disable blocking for input 1/10 second
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) err("tcsetattr");
}

void initialize_editor(int argc, char **argv){
  get_terminal_size(&chief.w, &chief.h);
  chief.cx = 0;
  chief.cy = 0;
  chief.yoff = 0;
  chief.dirty = 0;
  chief.message = (char *) calloc(256, 1);
  chief.prompted = 0;

  init_ui_editor(&chief.ui);
  
  if(argc > 1){
    open_file(argv[1]);
  }else{
    insert_row(chief.num_rows, "");
  }
}

void free_rows(){
  int i;
  for(i = 0; i < chief.num_rows; i++){
    free(chief.rows[i].text);
    chief.rows[i].text = NULL;
    chief.rows[i].len = 0;
  }
  chief.num_rows = 0;
}

void free_terminal(){
  free(chief.message);
  free_rows();
  free(chief.rows);
  chief.rows = NULL;
  free(chief.filepath);
  chief.filepath_len = 0;  
}

//Redraw terminal and read input
void terminal_loop(){
  while(1){
    clear_terminal();
    chief.ui.render(&chief.ui);
    int c = read_input();
    if(chief.ui.input(&chief.ui, c))
      break;
  }
  chief.ui.destroy(&chief.ui);
}

//Read raw STDIN stream byte by byte
int read_input(){
  int num_read;
  char c;
  while((num_read = read(STDIN_FILENO, &c, 1)) != 1){
    if(num_read == -1) err("read");
  }
  if(c == '\x1b'){
    char esc[3];
    if(read(STDIN_FILENO, esc, 2) != 2) return '\x1b';
    if(esc[0] == '['){
      if(esc[1] >= '0' && esc[1] <= '9'){
	if(read(STDIN_FILENO, &esc[2], 1) != 1) return '\x1b';
	if(esc[2] == '~'){
	  switch(esc[1]){ // \x1b[n~ where n is 0 thru 9
	  case '1':
	    return HOME_KEY;
	  case '3':
	    return DELETE_KEY;
	  case '4':
	    return END_KEY;
	  case '7':
	    return HOME_KEY;
	  case '8':
	    return END_KEY;
	  }
	}
      }else{
	switch(esc[1]){ // \x1b[n where n is a capital letter
	case 'A':
	  return ARROW_UP;
	case 'D':
	  return ARROW_LEFT;
	case 'B':
	  return ARROW_DOWN;
	case 'C':
	  return ARROW_RIGHT;
	case 'H':
	  return HOME_KEY;
	case 'F':
	  return END_KEY;
	}
      }
    }else if(esc[0] == 'O'){
      switch(esc[1]){ // \x1bOn where n is capital letter
      case 'H':
	return HOME_KEY;
      case 'F':
	return END_KEY;
      }
    }
    else{
      return '\x1b';
    }
  }
  return c;
}

//Finds the size of the terminal in characters 
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

//Clears the terminal and moves cursor to upper-left corner
void clear_terminal(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}
