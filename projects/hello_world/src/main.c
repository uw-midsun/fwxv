#include <stdio.h>

#include "log.h"

int main() {
  tasks_init();
  log_init();
  int i = 0;
  LOG_DEBUG("Welcome to TEST!");
  while(true){
    LOG_DEBUG("Hello World %d\n",i);
    i++;
  }
  


  LOG_DEBUG("exiting main?");
  return 0;
}
// scons sim --project=hello_world --platform=x86