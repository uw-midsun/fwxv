#include "stack.h"

#include <string.h>

StatusCode stack_init(Stack *stack){
    if(stack->storage_buf == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }
    stack->current_item = 0;
    return STATUS_CODE_OK;
}

StatusCode stack_pop(Stack *stack, uint32_t *buf){
    if(stack->current_item == 0){
        return STATUS_CODE_EMPTY;
    }
    stack->current_item = (stack->current_item - stack->item_size) % (stack->num_items * stack->item_size);

    for(int i = 0; i < stack->item_size; i++){
        *buf |= (stack->storage_buf[stack->current_item + i] << (i * 8));
    }

    return STATUS_CODE_OK;
}

StatusCode stack_push(Stack *stack, uint32_t data){
    for(int i = 0; i < stack->item_size; i++){
        stack->storage_buf[stack->current_item + i] = (uint8_t)(data & (0xFF << (i * 8)));
    }
    stack->current_item = (stack->current_item + stack->item_size) % (stack->num_items * stack->item_size);

    return STATUS_CODE_OK;
}

StatusCode stack_peek(Stack *stack, uint32_t *buf){
    if(stack->current_item == 0){
        return STATUS_CODE_EMPTY;
    }
    stack->current_item = (stack->current_item - stack->item_size) % (stack->num_items * stack->item_size);

    for(int i = 0; i < stack->item_size; i++){
        *buf |= (stack->storage_buf[stack->current_item + i] << (i * 8));
    }

    stack->current_item = (stack->current_item + stack->item_size) % (stack->num_items * stack->item_size);

    return STATUS_CODE_OK;
}

uint32_t stack_get_num_items(Stack *stack){
    return stack->num_items;
}

void stack_reset(Stack *stack){
    for(int i = 0; i < stack->item_size * stack->num_items; i++){
        stack->storage_buf[i] = 0;
    }
}