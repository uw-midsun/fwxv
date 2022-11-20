#include <stdio.h>
#include <unistd.h>

#include "delay.h"
#include "log.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

int increment(int *num) {
  return *num + 1;
}

int main() {
  int num = 0;

  while (true) {
    LOG_DEBUG("Hello World! %d\n", num);
    num = increment(&num);
    delay_s(1);
  }

  return 0;
}
