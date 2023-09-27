#include <stdio.h>

#include "log.h"

int increment(int *input) {
  (*input)++;
  return *input;
}

int main(void) {
  int num = 0;
  while (true) {
    LOG_DEBUG("Hello World %d\n", increment(&num));
  }
  return 0;
}
