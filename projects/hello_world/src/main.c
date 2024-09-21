#include <stdio.h>

#include "log.h"

int main() {
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  
  int my_int = 9;
  while (true){
    LOG_DEBUG("Hello World %d\n", my_int);
  }

  LOG_DEBUG("exiting main?");
  return 0;
}
