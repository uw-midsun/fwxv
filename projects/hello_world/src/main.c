

#include "log.h"

static int return_val = 0;
void increase_value(int * val){
  *val += 1;
}

int main() {
  int * p_val ;
  p_val = &return_val;
  while (true){
    increase_value(p_val);
    LOG_DEBUG("Hello World %d\n",return_val);
  }
  return 0;
}
