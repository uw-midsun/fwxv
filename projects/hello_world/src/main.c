#include <stdio.h>
#include "log.h"
#include "delay.h"

static int increment_input(int *input) { 
  (*input)++;
  return (*input); 
}

int main(void) {
  int in = 0;

  while (true) {
    in = increment_input(&in);

    LOG_DEBUG("Hello World %d\n", in);
    delay_s(1);
  }

  return 0;
}