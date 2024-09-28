#include <stdio.h>
#include<unistd.h>

#include "log.h"

int increment(int i) {
  return ++i;
}

int main() {
  int my_int = 0;
  while (1) {
    my_int = increment(my_int);
    LOG_DEBUG("Hello World %d\n", my_int);
    sleep(1);
  }
  return 0;
}