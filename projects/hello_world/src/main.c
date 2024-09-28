#include <stdio.h>
#include "log.h"
#define BUF_SIZE 10 // Any defined constants or preprocessor directives
static int buf[BUF_SIZE]; // Any static/global declarations



int main(void) {
  int i=0;
while(true){
   LOG_DEBUG("Hello World %d\n", i);
   i++;
  } 
  return 0;
}
