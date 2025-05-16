/* COMMENTED THIS OUT FOR NOW, SO I CAN WORK ON 102 TASK 1

#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

int increment(int *value){
  *value += 1;
  return *value;
}

int main() {
  int num = 0;
  int *ptr = &num;
  while (1){
    sleep(1);
    LOG_DEBUG("Hello World %d\n", increment(ptr));
  }
  return 0;
}
*/

