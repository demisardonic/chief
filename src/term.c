#include <ctype.h>
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
  //Reset terminal to initial state
  atexit(reset_terminal);

  struct termios raw = chief.orig_termios;

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

  get_terminal_size(&chief.w, &chief.h);
  chief.cx = 0;
  chief.cy = 0;
  chief.message = (char *) calloc(256, 1);
  char *m = "Generic Welcome Message";
  set_message(m, strlen(m));
}

void free_terminal(){
  free(chief.message);
}

//Redraw terminal and read input
void terminal_loop(){
  while(1){
    clear_terminal();
    render_terminal();

    char c = read_input();
    switch(c){
    case CTRL_KEY('q'):
      return; //Exit the program
      //Move the arrows
    case ARROW_UP:
      if(chief.cy > 0)
	chief.cy--;
      break;
    case ARROW_LEFT:
      if(chief.cx > 0)
	chief.cx--;
      break;
    case ARROW_DOWN:
      if(chief.cy <= chief.h)
	chief.cy++;
      break;
    case ARROW_RIGHT:
      if(chief.cx <= chief.w)
	chief.cx++;
      break;
    }
  }
}

//Read raw STDIN stream byte by byte
char read_input(){
  int num_read;
  char c;
  while((num_read = read(STDIN_FILENO, &c, 1)) != 1){
    if(num_read == -1) err("read");
  }
  if(c == '\x1b'){
    char esc[2];
    if(read(STDIN_FILENO, esc, 2) != 2)
      return '\x1b';
    if(esc[0] == '['){
      switch(esc[1]){
      case 'A':
	return ARROW_UP;
      case 'D':
	return ARROW_LEFT;
      case 'B':
	return ARROW_DOWN;
      case 'C':
	return ARROW_RIGHT;
      }
    }
    else{
      return '\x1b';
    }
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

void clear_terminal(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

//Change the message within the bottom bar
void set_message(const char *m, int len){
  if(len > 255) len = 255;
  strncpy(chief.message, m, len);
  chief.message[len] = '\0';
  chief.m_len = len;
}

void render_terminal(){
  cbuf_t cb = NEW_CBUF;
  //Turn off cursor
  cbuf_append(&cb, "\x1b[?25l", 6);

  //Clear terminal
  cbuf_append(&cb, "\x1b[2J", 4);
  cbuf_append(&cb, "\x1b[H", 3);

  //Print bottom bar and message
  cbuf_bar(&cb);
  cbuf_append(&cb, "\u2503", 3);
  cbuf_append(&cb, chief.message, chief.m_len);
  cbuf_move(&cb, chief.w-1, chief.h-1);
  cbuf_append(&cb, "\u2503", 3);

  cbuf_append(&cb, "\x1b[H", 3);

  //Return cursor position
  cbuf_move(&cb, chief.cx, chief.cy);
  //Turn on cursor
  cbuf_append(&cb, "\x1b[?25h", 6);

  //Draw the character buffer the terminal and free the buffer
  write(STDOUT_FILENO, cb.b, cb.l);
  cbuf_free(&cb);
}
