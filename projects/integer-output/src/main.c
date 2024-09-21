#include <stdio.h>
#include "log.h"

function increment(var){
  return ++var;
}

int main() {
  int counter = 0;
  while(true) LOG_DEBUG("Hello World %d\n", increment(counter);
  return 0;
}
