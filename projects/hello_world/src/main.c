#include <stdio.h>


#define BUF_SIZE 10 // Any defined constants or preprocessor directives
static int buf[BUF_SIZE]; // Any static/global declarations

static int prv_my_func(int *input) { // Function declarations/definitions
  (*input)++;
  return *input; // Increments the value passed and returns it 
}

int main(void) {
  int in = 0;
  int out;

  while (1) {
    out = prv_my_func(&in);
    printf("In %d, Out %d\n  Hello World", in, out);
  }
  return 0;
}
