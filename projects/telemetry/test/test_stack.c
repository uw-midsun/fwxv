#include "test_helpers.h"
#include "unity.h"
#include "stack.h"

void setup_test(){
    printf("setup\n");
    
}

void teardown_test(void){

}

void test_stack(){
    uint32_t stack_size = 3;
    uint32_t item_size = 2;
    uint32_t current_item = 0;
    uint32_t head = 0;
    uint8_t buffer[item_size * (stack_size + 1)]; //our stack will always have an empty slot between head and tail
    Stack my_stack = {stack_size, item_size, buffer, current_item, head};
    uint32_t read = 0;
    uint32_t curr_index;

    stack_init(&my_stack);

    stack_push(&my_stack, 0x1);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    stack_pop(&my_stack, &read);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    printf("val: %x\n", read);
    stack_push(&my_stack, 0x02);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    stack_push(&my_stack, 0x03);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    stack_push(&my_stack, 0x04);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    stack_push(&my_stack, 0x05);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    stack_pop(&my_stack, &read);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    printf("val: %x\n", read);
    stack_pop(&my_stack, &read);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    printf("val: %x\n", read);
    stack_pop(&my_stack, &read);
    printf("index: %d\n", stack_get_current_index(&my_stack));
    printf("val: %x\n", read);
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