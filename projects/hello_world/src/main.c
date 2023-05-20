#include <stdio.h>
#include <unistd.h>
#include "log.h" // The library includes
#include "gpio.h"

// #define BUF_SIZE 10 // Any defined constants or preprocessor directives
// static int buf[BUF_SIZE]; // Any static/global declarations

static int prv_my_func(int *input) { // Function declarations/definitions
  *input++;
  return *input; // Increments the value passed and returns it 
}

int main(void) {
   gpio_init();
   
   int num = 0;
   int nextNum;

   while(true) {
      num = prv_my_func(num);
      LOG_DEBUG("Hello World %d\n", num);
      sleep(1);
   }
   
   return 0;
}
