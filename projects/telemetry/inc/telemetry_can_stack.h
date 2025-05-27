#pragma once

#include "can_msg.h"
#include "stack.h"

#define CAN_STACK_SIZE 64

typedef struct CanStack{
    Stack stack;
    CanMessage msg_nodes[CAN_STACK_SIZE];
}CanStack;

#define can_stack_init(can_stack)({                             \
    Stack *stack        = &(can_stack)->stack;                  \
    stack->num_items    = CAN_STACK_SIZE;                       \
    stack->item_size    = sizeof(CanMessage);                   \
    stack->storage_buf  = (uint8_t*) (&(can_stack)->msg_nodes)  \
    stack_init(&(can_stack)->stack);                            \
})

#define can_stack_push(can_stack, source)({                     \
    stack_push(&(can_stack)->stack, source);                    \
})                                                              \

#define can_stack_push_from_isr(can_stack, source, high_prio_woken) \
    stack_send_from_isr(&(can_stack)->stack, (source), high_prio_woken)

#define can_stack_peek(can_stack, dest)({                       \
    stack_peek(&(can_stack)->stack, dest);                      \
})                                                              \

#define can_stack_pop(can_stack, dest)({                        \
    stack_pop(&(can_stack)->stack, dest);                       \
})                                                              \

#define can_stack_pop_from_isr(can_stack, dest, higher_prio_woken)  \
    stack_receive_from_isr(&(can_stack)->stack, (dest), high_prio_woken)

#define can_stack_size(can_stack)({                             \
    stack_get_num_items(&(can_stack)->stack);                   \
})                                                              \