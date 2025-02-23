#include <stdio.h>
#include <unistd.h>

#include "log.h"

static int prv_my_func(int *input) {
  ++(*input);
  return *input; 
}

int main(void) {
	int my_int = 0;

	while(true) {
		my_int = prv_my_func(&my_int);
		
		LOG_DEBUG("Hello World %d\n", my_int);

		sleep(1);
	}				

  return 0;
}
