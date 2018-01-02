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
}

void initialize_editor(int argc, char **argv){
  get_terminal_size(&chief.w, &chief.h);
  chief.cx = 0;
  chief.cy = 0;
  chief.message = (char *) calloc(256, 1);
  set_message("Generic Welcome Message");

  if(argc > 1){
    open_file(argv[1]);
  }else{ 
    //TODO: Temporary for testing cursor control
    chief.num_rows = 3;
    chief.rows = (row_t*) malloc(sizeof(row_t) * chief.num_rows);
    char* m = "Henlo you stinky row";
    chief.rows[0].len = strlen(m);
    chief.rows[0].text = (char *) malloc(sizeof(char) * chief.rows[0].len);
    strcpy(chief.rows[0].text, m);
    m = "TEST";
    chief.rows[1].len = strlen(m);
    chief.rows[1].text = (char *) malloc(sizeof(char) * chief.rows[1].len);
    strcpy(chief.rows[1].text, m);
    m = "Go be the second, ugly!";
    chief.rows[2].len = strlen(m);
    chief.rows[2].text = (char *) malloc(sizeof(char) * chief.rows[2].len);
    strcpy(chief.rows[2].text, m); 
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
    render_terminal();
    int c = read_input();
    if(editor_input(c))
      break;
  }
}

int editor_input(int c){
  int r_boundary = 0;
  if(chief.cy < chief.num_rows) r_boundary = chief.rows[chief.cy].len;

  switch(c){
  case CTRL_KEY('q'):
    return 1; //Exit the program
  case CTRL_KEY('o'):
    open_file(chief.filepath);
    break;
  case CTRL_KEY('s'):
    save_file(chief.filepath);
    break;
  case CTRL_KEY('k'):
    delete_row(chief.cy);
    break;
  case CTRL_KEY('x'):
    set_message("cut");
    break;
  case CTRL_KEY('c'):
    set_message("copy");
    break;
  case CTRL_KEY('v'):
    set_message("paste");
    break;
  case CTRL_KEY('a'):
    set_message("select all");
    break;
  case CTRL_KEY('z'):
    set_message("undo");
    break;
  case CTRL_KEY('y'):
    set_message("redo");
    break;
  case ARROW_UP:
    if(chief.cy > 0){
      chief.cy--;
    }
    break;
  case ARROW_LEFT:
    if(MIN(chief.cx, chief.rows[chief.cy].len) > 0){
      chief.cx = MIN(chief.cx - 1, chief.rows[chief.cy].len - 1);
    }else if(MIN(chief.cx, chief.rows[chief.cy].len) == 0 && chief.cy > 0){
      chief.cy--;
      chief.cx = chief.rows[chief.cy].len;
    }
    break;
  case ARROW_DOWN:
    if(chief.cy + 1 < chief.num_rows){
      chief.cy++;
    }
    break;
  case ARROW_RIGHT:
    if(chief.cx < r_boundary){
      chief.cx++;
    }else if(MIN(chief.cx, chief.rows[chief.cy].len) == r_boundary && chief.cy < chief.num_rows){
      chief.cy++;
      chief.cx = 0;
    }
    break;
  case HOME_KEY:
    chief.cx = 0;
    break;
  case END_KEY:
    chief.cx = r_boundary;
    break;
  case BACKSPACE:
    if(MIN(chief.cx, chief.rows[chief.cy].len) > 0){
      delete_character(MIN(chief.cx, chief.rows[chief.cy].len));
      chief.cx--;
    }else if(MIN(chief.cx, chief.rows[chief.cy].len) == 0 && chief.cy > 0){
      int i;
      int len = chief.rows[chief.cy].len;
      int old_len = chief.rows[chief.cy - 1].len;
      for(i = 0; i < len; i++){
	insert_character(chief.rows[chief.cy].text[i], chief.rows[chief.cy - 1].len, chief.cy - 1);
      }
      delete_row(chief.cy);
      chief.cy--;
      chief.cx = old_len;
    }
    break;
  case ENTER_KEY:
    insert_row(++chief.cy, "");
    break;
  case DELETE_KEY:
    if(chief.cx < chief.rows[chief.cy].len){
      delete_character(MIN(chief.cx, chief.rows[chief.cy].len));
    }else if(chief.cy < chief.num_rows - 1){
      int i;
      int next_len = chief.rows[chief.cy + 1].len;
      for(i = 0; i < next_len; i++){
        insert_character(chief.rows[chief.cy + 1].text[i], chief.rows[chief.cy].len, chief.cy);
      }
      delete_row(chief.cy + 1);
    }
    break;
  default:
    insert_character(c, MIN(chief.cx, chief.rows[chief.cy].len), chief.cy);
    chief.cx++;
    break;
  }
  return 0;
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
	  // \x1b[n~ where n is 0 thru 9
	  switch(esc[1]){
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
	// \x1b[n where n is a capital letter
	switch(esc[1]){
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
      // \x1bOn where n is capital letter
      switch(esc[1]){
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

//Change the message within the bottom bar
void set_message(const char *m, ...){
  char buffer[256];
  
  va_list argv;
  va_start(argv, m);
  int len = vsprintf(buffer, m, argv);
  va_end(argv);
  
  len = MIN(len, 255);
  strncpy(chief.message, buffer, len);
  chief.message[len] = '\0';
  chief.m_len = len;
}

void render_terminal(){
  cbuf_t cb = NEW_CBUF;
  
  //Turn off cursor
  cbuf_append(&cb, "\x1b[?25l", 6);
  //Reset text color
  cbuf_color(&cb, COLOR_RESET);
  //Reset cursor position to top-left
  cbuf_append(&cb, "\x1b[H", 3);

  //Print each row of text
  int i;
  for(i = 0; i < chief.h - 1; i++){
    cbuf_append(&cb, "\x1b[K", 3);
    if(i < chief.num_rows){
      cbuf_append(&cb, chief.rows[i].text, chief.rows[i].len);
    }
    cbuf_append(&cb, "\r\n", 2);
  }

  //Print bottom bar and message
  cbuf_bar(&cb);
  //Return cursor position
  int real_x = chief.cx;
  if(chief.cy < chief.num_rows)
    real_x = MIN(real_x, chief.rows[chief.cy].len);
  cbuf_move(&cb, real_x, chief.cy);
  //Turn on cursor
  cbuf_append(&cb, "\x1b[?25h", 6);

  //Draw the character buffer the terminal and free the buffer
  write(STDOUT_FILENO, cb.b, cb.l);
  cbuf_free(&cb);
}

void append_row(const char *m){
  row_t *new_rows = (row_t *) realloc(chief.rows, sizeof(row_t) * (chief.num_rows + 1));
  if(new_rows){
    //Create another row and fill it with the row contents
    row_t row;
    memset(&row, 0, sizeof(row_t));
    row.len = strlen(m);
    row.text = (char *) calloc(sizeof(char) * (row.len + 1), 1);
    strcpy(row.text, m);

    //Store this row in the rows array
    new_rows[chief.num_rows] = row;
    chief.num_rows++;
    chief.rows = new_rows;
  }
}

void insert_row(int index, const char *m){
  row_t *new_rows = (row_t *) realloc(chief.rows, sizeof(row_t) * (chief.num_rows + 1));
  if(new_rows){
    //Create another row and fill it with the row contents
    row_t row;
    memset(&row, 0, sizeof(row_t));
    row.len = strlen(m);
    row.text = (char *) calloc(sizeof(char) * (row.len + 1), 1);
    strcpy(row.text, m);

    //Shift all values after index to the right and insert new
    //row into the array at the index
    int i;
    for(i = chief.num_rows; i > index; i--){
      new_rows[i] = new_rows[i - 1];
    }
    new_rows[index] = row;
    chief.num_rows++;
    chief.rows = new_rows;
  }
}

void delete_row(int index){  
  int i;

  free(chief.rows[index].text);
  chief.rows[index].len = 0;

  for(i = index; i < chief.num_rows - 1; i++){
      chief.rows[i] = chief.rows[i + 1];
  }
  chief.num_rows--;

  chief.rows = (row_t *) realloc(chief.rows, sizeof(row_t) * chief.num_rows);
}

void insert_character(char c, int index, int row_num){
  row_t *old_row = &chief.rows[row_num];
  char *text = (char *) realloc(old_row->text, old_row->len + 2);
  if(text){
    text[old_row->len + 1] = '\0';
    int i;
    for(i = old_row->len + 1; i > index; i--){
      text[i] = text[i-1];
    }
    text[index] = c;
    old_row->text = text;
    old_row->len++;
    chief.rows[row_num] = *old_row;
  }
}

void delete_character(int index){
  row_t *old_row = &chief.rows[chief.cy];
  char *new_text = (char *) calloc(old_row->len, sizeof(char));
  int i, j;
  for(i = 0, j = 0; i < old_row->len - 1; i++, j++){
    if(i == index){
      j++;
    }
    new_text[i] = old_row->text[j];
  }
  free(old_row->text);
  old_row->text = new_text;
  old_row->len--;
}

//Open given filepath
//TODO: null check file at open
void open_file(const char *path){
  free_rows();
  set_message("Opened file: %s", path);

  chief.filepath_len = strlen(path);
  chief.filepath = (char *) realloc(chief.filepath, sizeof(char) * chief.filepath_len + 1);
  strncpy(chief.filepath, path, chief.filepath_len);
  chief.filepath[chief.filepath_len] = '\0';
  
  FILE *file = NULL;
  char *line = NULL;
  size_t file_len = 0;
  ssize_t read_len;
  
  file = fopen(chief.filepath, "r");
  
  //Find the length of the file
  fseek(file, 0L, SEEK_END);
  file_len = ftell(file);
  rewind(file);
  while((read_len = getline(&line, &file_len, file)) > 0){
    int len = strlen(line);
    line[len-1] = '\0';
    append_row(line);
  }
  
  //Close the file
  fclose(file);
  free(line);
}

//Save the current editor to the given filepath
//TODO: null check file pointer
void save_file(const char *path){
  set_message("Saved file: %s", path);
  FILE *outfile = fopen(path, "w");
  int i;
  for(i = 0; i < chief.num_rows; i++){
    fwrite(chief.rows[i].text, sizeof(char), chief.rows[i].len, outfile);
    fwrite("\n", sizeof(char), 1, outfile);
  }
  fclose(outfile);
}
