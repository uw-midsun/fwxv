#include <stdio.h>
#include "log.h"

int main(void) {

	int myint = 0;

  while(1){
    LOG_DEBUG("Hello World %d\n", myint);
	  myint++;
  }

  return 0;
}

