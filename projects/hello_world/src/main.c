
#include "log.h"


void increment(int *input) { 
  (*input)++;
}


int main() {
  
  int my_int = 5;

  LOG_DEBUG("Before entering main loop");
  
  while(true){
    increment(&my_int);
    LOG_DEBUG("Hello World %d\n", my_int);
  }

  return 0;
}
