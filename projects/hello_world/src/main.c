#include <stdio.h>

#define BUF_SIZE 10 // Defined constant for buffer size
static int buf[BUF_SIZE]; // Static buffer with size BUF_SIZE

static int prv_my_func(int *input) { // Function to increment the value of input
  (*input)++;  // Increment the value pointed to by input
  return *input;  // Return the incremented value
}

int main(void) {
  int in = 0;  // Initialize input to 0
  int out;
  
  for (int i = 0; i < BUF_SIZE; i++) {  // Loop through buffer size
    out = prv_my_func(&in);  // Increment 'in' and store the result in 'out'
    buf[i] = out;  // Store the result in the buffer
    printf("Hello World %d\n", i + 1);  // Print Hello World with incremented value
  }
  
  return 0;
}
