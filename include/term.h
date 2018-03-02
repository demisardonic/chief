#ifndef TERM_H
#define TERM_H

#include <stdlib.h>
#include <termios.h>

//Gives the effective row and character position instead of the cursor position
#define EFF_CX MIN(chief.cx, chief.rows[chief.cy + chief.yoff].len)
#define EFF_CY chief.cy + chief.yoff

#define COLOR_RESET -1, -1
#define COLOR_BLK    0,  9
#define COLOR_RED    1,  9
#define COLOR_GRN    2,  9
#define COLOR_YLW    3,  9
#define COLOR_BLU    4,  9
#define COLOR_PUR    5,  9
#define COLOR_CYN    6,  9
#define COLOR_WHT    7,  9

typedef struct row{
  char *text;
  int len;
}row_t;

typedef struct term{
  int w;
  int h;
  struct termios orig_termios;
  int cx;
  int cy;
  int yoff;
  char *message;
  int m_len;
  row_t *rows;
  int num_rows;
  char *filepath;
  int filepath_len;
  int dirty;
  int prompted;
}term_t;


#define NEW_RENDER {NULL, -1, -1}

typedef struct term_render{
  cbuf_t *cb;
  int fg;
  int bg;
}term_render_t;

enum keys{
  BACKSPACE = 127,
  ENTER_KEY = '\r',
  ARROW_UP = 256,
  ARROW_LEFT,
  ARROW_DOWN,
  ARROW_RIGHT,
  HOME_KEY,
  END_KEY,
  DELETE_KEY
};

extern term_t chief;

void reset_terminal();
void initialize_terminal();
void initialize_editor(int argc, char **argv);
void free_terminal();

void terminal_loop();
int editor_input(int c);
int read_input();
int read_prompt_input();

int get_terminal_size(int *width, int *height);
void clear_terminal();
void render_terminal();
void set_message(const char *m, ...);

void insert_row(int index, const char *m);
void delete_row(int index);
void insert_character(char c, int index, int row_num);
void delete_character(int index, int row_num);

void open_file(const char *path);
void save_file(const char *path);

#endif
