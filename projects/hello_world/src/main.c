#include <stdio.h>
#include "log.h" // The library includes
#include <time.h>

int main(void) {
  int my_int=0;
  while (1){
    LOG_DEBUG("Hello World %d\n", my_int);
    my_int++;
    
    clock_t start_time = clock();
    while (((clock() - start_time) / CLOCKS_PER_SEC) < 1){
    }
  }
  return 0;
}



// scons new --project=hello_world
//scons sim --project=leds --platform=x86