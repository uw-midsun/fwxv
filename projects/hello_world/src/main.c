#include "log.h"

void increment(int* counter){
  *counter = *counter + 1;
}

int main() {
  int counter = 0;
  int* add = &counter;
  while(counter < 10000){
    increment(add);
    LOG_DEBUG("Hello world! %d \n", counter);
  }
  return 0;
}
