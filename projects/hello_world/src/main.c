#include <stdio.h>
#include <unistd.h>  
#include "log.h"     

void increment(int *input) {
  (*input)++;
  sleep(1);
}

int main() {
  int count = 0;
  while (1) {
    LOG_DEBUG("Hello World %d\n", count);
    increment(&count);
  }
  return 0;
}
