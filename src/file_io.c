#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_io.h"
#include "term.h"

//Open given filepath
void open_file(const char *path){
  free_rows();
  chief.cx = 0;
  chief.cy = 0;
  chief.yoff = 0;
  chief.dirty = 0;
  chief.filepath_len = strlen(path);
  chief.filepath = (char *) realloc(chief.filepath, sizeof(char) * chief.filepath_len + 1);
  strncpy(chief.filepath, path, chief.filepath_len);
  chief.filepath[chief.filepath_len] = '\0';
  
  FILE *file = NULL;
  char *line = NULL;
  size_t file_len = 0;
  ssize_t read_len;
  
  file = fopen(chief.filepath, "r");
  if(!file) set_message("Failed to open file: %s", path);
  
  fseek(file, 0L, SEEK_END);
  file_len = ftell(file);
  rewind(file);
  while((read_len = getline(&line, &file_len, file)) > 0){
    int len = strlen(line);
    line[len-1] = '\0';
    insert_row(chief.num_rows, line);
  }
  
  //Close the file
  fclose(file);
  free(line);
  set_message("Opened file: %s", path);
}

//Save the current editor to the given filepath
void save_file(const char *path){
  FILE *outfile = fopen(path, "w");
  if(!outfile) set_message("Failed to saved file: %s", path);
  int i;
  for(i = 0; i < chief.num_rows; i++){
    fwrite(chief.rows[i].text, sizeof(char), chief.rows[i].len, outfile);
    fwrite("\n", sizeof(char), 1, outfile);
  }
  fclose(outfile);
  set_message("Saved file: %s", path);
  chief.dirty = 0;
}
