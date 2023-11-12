#pragma once
// Basic wrapper for FreeRTOS queues.
#include "FreeRTOS.h"
#include "queue.h"
#include "status.h"

#define QUEUE_DELAY_BLOCKING portMAX_DELAY

// Queue storage and access struct. Must be declared statically
typedef struct {
  uint32_t num_items;    // Number of items the queue can hold
  uint32_t item_size;    // Size of each item
  uint8_t *storage_buf;  // Must be declared statically, and have size num_items*item_size
  StaticQueue_t queue;   // Internal Queue storage
  QueueHandle_t handle;  // Handle used for all queue operations
} Queue;

// Create a queue with the parameters specified in settings. Returns STATUS_CODE_OK if successful,
// STATUS_CODE_INVALID_ARGS otherwise.
StatusCode queue_init(Queue *queue);

// Attempt to place an item into the queue, delaying for delay_ms in ms before timing out.
StatusCode queue_send(Queue *queue, const void *item, uint32_t delay_ms);
// This version must be inside of an interrupt
StatusCode queue_send_from_isr(Queue *queue, const void *item, BaseType_t *higher_prio_woken);

// Attempt to receive an item from the queue, delaying for delay_ms in ms before timing out.
StatusCode queue_receive(Queue *queue, void *buf, uint32_t delay_ms);
// This version must be inside of an interrupt
StatusCode queue_receive_from_isr(Queue *queue, void *buf, BaseType_t *higher_prio_woken);

// Attempt to receive an item from the queue without removing it from the queue, delaying for
// delay_ms in ms before timing out.
StatusCode queue_peek(Queue *queue, void *buf, uint32_t delay_ms);

// Empties all items from a queue
void queue_reset(Queue *queue);

// Return number of items the queue can hold.
uint32_t queue_get_num_items(Queue *queue);

// Return Number of free spaces in queue
uint32_t queue_get_spaces_available(Queue *queue);
