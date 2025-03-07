#include <stdio.h>
#include "log.h"

static void itterator(int *x){
	(*x)++;
}

int main(void) {
  int a = 0;
  while(1){
	itterator(&a);
	LOG_DEBUG("Hello World %d\n", a);
  }
  return 0;
}

