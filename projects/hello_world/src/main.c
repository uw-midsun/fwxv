#include <stdio.h>

#include "log.h"
//#include "master_task.h"
#include "tasks.h"

static int counter(int input) {
  input++;
  return input;
}

int main() {
  int in = 0;
  int out;
  while(true) {
    out = counter(in);
    LOG_DEBUG("Hello World %d\n", out);
    in = out; //update the input for next loop

  }
  return 0;
}
