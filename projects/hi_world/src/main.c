#include <stdio.h>

#include "log.h"
#include <unistd.h>


int main(void) {
  int count = 0;
  while(1) {
    LOG_DEBUG("Hello, World! %d\n", count);
    count++;
    sleep(1);
  }
  return 0;
}
