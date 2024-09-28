#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include <unistd.h> 

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
    int number = 0;
    int i=0;
    while(i<10) {
        // Call the increment function
        number+=1;
        LOG_DEBUG("Number: %d, Hello world\n", number);
        sleep(1);
        i+=1;
    }

  return 0;
}