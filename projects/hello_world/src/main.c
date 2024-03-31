#include <stdio.h>

#include "log.h"

int main(void) {
  int count = 0;

  while (true) {
    LOG_DEBUG("Hello World %d\n", count);
    count++;
  }
}
