#include <stdio.h>

#include "delay.h"
#include "log.h"

#define TRUE 1
#define FALSE 0

static int increment(int *input) {  // Function declarations/definitions
  *input += 1;
  return *input;  // Increments the value passed and returns it
}

int main(void) {
  int my_int = 0;
  while (TRUE) {
    my_int = increment(&my_int);
    LOG_DEBUG("Hello World %d\n", my_int);
    // delay_ms(50);
  }
  return 0;
}
