#include <stdio.h>

#include "log.h"
int main(void) {
  int my_int = 0;
  for (;; my_int++) {
    LOG_DEBUG("Hello World %d\n", my_int);
  }
  return 0;
}
