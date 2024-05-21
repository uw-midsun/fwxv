#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"

void run_fast_cycle()
{

}

void run_medium_cycle()
{

}

void run_slow_cycle()
{

}

static int increment(int *input){
   *input += 1;
   return *input;
}

int main() {
   tasks_init();
   log_init();
   LOG_DEBUG("Welcome to TEST!");
   int counter = 0;
   while(true){
      increment(&counter);
      LOG_DEBUG("Hello World %d\n", counter);
   }

   LOG_DEBUG("exiting main?");
   return 0;
}

