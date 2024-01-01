#include <stdio.h>
#include <unistd.h>

#include "log.h"

int main(void) {
  int in = 0;
  while (true) {
    in++;
    LOG_DEBUG("%d, Hello World\n", in);
  }

  return 0;
}
