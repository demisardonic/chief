#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void err(const char *error){
  perror(error);
  exit(1);
}
