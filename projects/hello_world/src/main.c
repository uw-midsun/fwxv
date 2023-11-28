#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

int main() {
  int in = 0;
  while (true) {
    in++;
    LOG_DEBUG("Hello World %d\n", in);
    sleep(1);
  }
  return 0;
}
