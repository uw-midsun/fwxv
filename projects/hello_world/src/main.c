#include <stdio.h>
#include "log.h"

int main(void) {
  unsigned int my_int = 0;

  while(1) {
    LOG_DEBUG("Hello World %d\n", my_int);
    ++my_int;
  }

  return 0;
}