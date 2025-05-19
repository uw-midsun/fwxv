#pragma once

// Basic wrapper for FreeRTOS queues.
#include "FreeRTOS.h"
#include "queue.h"
#include "status.h"

#define QUEUE_DELAY_BLOCKING portMAX_DELAY

//Stack storage and access struct. Must be declared statically
typedef struct {
  uint32_t num_items;    // Number of items the queue can hold
  uint32_t item_size;    // Size of each item
  uint8_t *storage_buf;  // Must be declared statically, and have size num_items*item_size
  uint32_t current_item;
} Stack;

StatusCode stack_init(Stack *stack);
StatusCode stack_pop(Stack *stack, uint32_t *buf);
StatusCode stack_push(Stack *stack, uint32_t data);
StatusCode stack_peek(Stack *stack, uint32_t *buf);
uint32_t stack_get_num_items(Stack *stack);
void stack_reset(Stack *stack);