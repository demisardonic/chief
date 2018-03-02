#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "term.h"
#include "ui.h"
#include "util.h"

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

void move_cursor(int c){
  switch(c){
  case ARROW_UP:
    if(chief.cy > 0){
      chief.cy--;
    }else if(chief.yoff > 0){
      chief.yoff--;
    }
    break;
  case ARROW_LEFT:
    if(EFF_CX > 0){
      chief.cx = EFF_CX - 1;
    }else if(EFF_CX == 0 && EFF_CY > 0){
      move_cursor(ARROW_UP);
      move_cursor(END_KEY);
    }
    break;
  case ARROW_DOWN:
    if(EFF_CY < chief.num_rows - 1){
      if(EFF_CY < chief.h - 3){
	chief.cy++;
      }else{
	chief.yoff++;
      }
    }
    break;
  case ARROW_RIGHT:
    if(chief.cx < chief.rows[EFF_CY].len){
      chief.cx++;
    }else if(EFF_CX == chief.rows[EFF_CY].len && EFF_CY + 1< chief.num_rows){
      move_cursor(ARROW_DOWN);
      move_cursor(HOME_KEY);
    }
    break;
  case HOME_KEY:
    chief.cx = 0;
    break;
  case END_KEY:
    chief.cx = chief.rows[EFF_CY].len;
    break;
  }
}

void insert_row(int index, const char *m){
  row_t *new_rows = (row_t *) realloc(chief.rows, sizeof(row_t) * (chief.num_rows + 1));
  if(new_rows){
    //Create another row and fill it with the old row contents
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
  }
}

void delete_character(int index, int row_num){
  row_t *old_row = &chief.rows[row_num];
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


int render_set_color(term_render_t *render, int fg, int bg){
  cbuf_t *cb = render->cb; 
  if(fg == -1 && bg == -1){
    cbuf_appendf(cb, "\x1b[0m");
    return 0;
  }
  fg = RANGE(fg, 0, 7);
  if(bg < 0 || bg > 7){
    bg = 9; //bg 9 is transparent
  }
  render->fg = fg;
  render->bg = bg;
  return cbuf_appendf(cb, "\x1b[%d;%dm", 30+fg, 40+bg);
}

int render_move_cursor(term_render_t *render, int x, int y){
  if(x < 0 || y < 0 || x >= chief.w || y >= chief.h)
    return 0;
  cbuf_t *cb = render->cb;
  char tmp[64];
  memset(tmp, 0, 64);
  int len = snprintf(tmp, 64, "\x1b[%d;%dH", y+1, x+1);
  cbuf_append(cb, tmp, len);
  return 1;
}

int render_bar(term_render_t *render){
  cbuf_t *cb = render->cb;
  int width = chief.w - 1;
  char bar[width];
  int len = sprintf(bar, "%c%s:%d", (chief.dirty > 0 ? '*' : ' '), chief.filepath, EFF_CY);
  render_set_color(render, 1, 7);

  render_move_cursor(render, 0, chief.h - 2);
  cbuf_append(cb, "\x1b[K", 3);
  cbuf_append(cb, bar, len);
  width -= len;
  while(--width >= 1){
    cbuf_append(cb, " ", 1);
  }
  render_set_color(render, COLOR_RESET);
  cbuf_append(cb, "\r\n", 2);

  cbuf_append(cb, "\x1b[K", 3);
  cbuf_append(cb, chief.message, chief.m_len);
  
  return 1;
}

static int _editor_input(ui_t *ui, int c){
  int i, temp;
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
    if(chief.rows[EFF_CY].len == 0){
      delete_row(EFF_CY);
    }else{
      delete_row(EFF_CY);
      insert_row(EFF_CY, "");
    }
    move_cursor(HOME_KEY);
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
  case ARROW_LEFT:
  case ARROW_DOWN:
  case ARROW_RIGHT:
  case HOME_KEY:
  case END_KEY:
    move_cursor(c);
    break;
  case BACKSPACE:
    chief.dirty++;
    if(EFF_CX > 0){
      move_cursor(ARROW_LEFT);
      delete_character(EFF_CX, EFF_CY);
    }else if(EFF_CX == 0 && EFF_CY > 0){
      int len = chief.rows[EFF_CY].len;
      int old_len = chief.rows[EFF_CY - 1].len;
      for(i = 0; i < len; i++){
	insert_character(chief.rows[EFF_CY].text[i], chief.rows[EFF_CY - 1].len, EFF_CY -1);
      }
      move_cursor(ARROW_UP);
      delete_row(EFF_CY + 1);
      chief.cx = old_len;
    }else{
      chief.dirty--;
    }
    break;
  case ENTER_KEY:
    chief.dirty++;
    insert_row(EFF_CY + 1, "");
    temp = chief.rows[EFF_CY].len;
    for(i = EFF_CX; i < temp; i++){
      insert_character(chief.rows[EFF_CY].text[i], chief.rows[EFF_CY + 1].len, EFF_CY + 1);
    }
    for(i = EFF_CX; i < temp; i++){
      delete_character(i, EFF_CY);
    }
    move_cursor(ARROW_DOWN);
    move_cursor(HOME_KEY);
    break;
  case DELETE_KEY:
    chief.dirty++;
    if(chief.cx < chief.rows[EFF_CY].len){
      delete_character(EFF_CX, EFF_CY);
    }else if(EFF_CY < chief.num_rows - 1){
      int next_len = chief.rows[EFF_CY + 1].len;
      for(i = 0; i < next_len; i++){
        insert_character(chief.rows[EFF_CY + 1].text[i], chief.rows[EFF_CY].len, EFF_CY);
      }
      delete_row(EFF_CY + 1);
    }else{
      chief.dirty--;
    }
    break;
  default:
    chief.dirty++;
    insert_character(c, EFF_CX, EFF_CY);
    move_cursor(ARROW_RIGHT);
    break;
  }
  return 0;
}

static void _editor_render(ui_t *ui){
  term_render_t render = NEW_RENDER;
  cbuf_t *cb = cbuf_create();
  if(!cb){
    err("Failed to create cbuf_t.\n");
  }
  render.cb = cb;
  
  cbuf_append(cb, "\x1b[?25l", 6);  //Turn off cursor
  render_set_color(&render, COLOR_RESET);  //Reset text color
  cbuf_append(cb, "\x1b[H", 3);  //Reset cursor position to top-left

  //Print each row of text
  int i;
  for(i = 0; i < chief.h - 1; i++){
    cbuf_append(cb, "\x1b[K", 3);
    if(i < chief.num_rows){
      cbuf_append(cb, chief.rows[i + chief.yoff].text, MIN(chief.rows[i + chief.yoff].len, chief.w));
    }
    cbuf_append(cb, "\r\n", 2);
  }

  render_bar(&render);  //Print bottom bar and message
  int real_x = EFF_CY < chief.num_rows ? EFF_CX : chief.cx;
  render_move_cursor(&render, real_x, chief.cy);  //Return cursor position
  cbuf_append(cb, "\x1b[?25h", 6);  //Turn on cursor

  //Draw the character buffer the terminal and free the buffer
  write(STDOUT_FILENO, cb->buffer, cb->len);
  cbuf_free(cb);

}

static void _editor_destroy(ui_t *ui){

}

void init_ui_editor(ui_t *ui){
  ui->input = &_editor_input;
  ui->render = &_editor_render;
  ui->destroy = &_editor_destroy;
}
