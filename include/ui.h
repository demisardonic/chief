#ifndef UI_H
#define UI_H

typedef struct ui ui_t;
struct ui{
  int (*input)(ui_t *, int);
  void (*render)(ui_t *);
  void (*destroy)(ui_t *);
};

void init_ui_editor(ui_t *);
#endif
