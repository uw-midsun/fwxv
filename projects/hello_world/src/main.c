#include <stdio.h>
#include "log.h"


int main(void) {
  int my_int = 0;
  while(true){
    printf("Hello World %d\n", my_int);
    ++my_int;
  }
}