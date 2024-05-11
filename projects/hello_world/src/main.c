#include <stdio.h>

#include "delay.h"
#include "log.h"

static int increment_input(int *input) {
  (*input)++;
  return (*input);
}

int main(void) {
  int in = 0;

  while (true) {
    in = increment_input(&in);

    LOG_DEBUG("Hello World %d\n", in);
    delay_ms(1000);
  }

  return 0;
}
