#include "log.h"

static int inc(int a) {
  a++;
  return a;
}

int main(void) {
  int i = 1;
  while (true) {
    LOG_DEBUG("Hello World %d\n", i);
    i = inc(i);
  }
  return 0;
}
