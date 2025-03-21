#include <stdio.h>
#include "log.h"

static int prv_my_func(int input) {
  input++;
  return input;
}

int main() {
  unsigned int my_int = 0;

  while (true) {
    my_int = prv_my_func(my_int);
    LOG_DEBUG("Hello World %d\n", my_int);
  }
  return 0;
}