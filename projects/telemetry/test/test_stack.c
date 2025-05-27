#include "test_helpers.h"
#include "unity.h"
#include "stack.h"

void setup_test(){
    printf("setup\n");
    
}

void teardown_test(void){

}

void test_stack(){
    uint32_t stack_size = 256;
    uint32_t item_size = 32;
    uint8_t buffer[item_size * stack_size]; 
    Stack my_stack = {256, 32, buffer};
    uint32_t read;

    stack_push(&my_stack, 0x1111);
    stack_pop(&my_stack, read);
    printf(read);
    printf("\n");
    stack_push(&my_stack, 0x1111);
    stack_push(&my_stack, 0x2222);
    stack_push(&my_stack, 0x3333);
    stack_peek(&my_stack, read);
    printf(read);
    printf("\n");
    stack_pop(&my_stack, read);
    printf(read);
    printf("\n");
    stack_pop(&my_stack, read);
    printf(read);
    printf("\n");
}