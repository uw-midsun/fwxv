#pragma once
// Basic wrapper for FreeRTOS queues.
#include "FreeRTOS.h"
#include "queue.h"
#include "status.h"

#define QUEUE_DELAY_BLOCKING portMAX_DELAY

typedef struct {
    uint32_t size;
    uint32_t item_size;
    uint8_t *storage_buf;
    StaticQueue_t queue;
    QueueHandle_t handle;
} Queue;

// Create a queue with the parameters specified in settings. Returns STATUS_CODE_OK if successful, STATUS_CODE_INVALID_ARGS otherwise.
StatusCode queue_init(Queue *queue);

// Attempt to place an item into the queue, delaying for delay_ms in ms before timing out.
StatusCode queue_send(Queue *queue, const void *item, uint32_t delay_ms);

// Attempt to receive an item from the queue, delaying for delay_ms in ms before timing out.
StatusCode queue_receive(Queue *queue, void *buf, uint32_t delay_ms);

// Attempt to receive an item from the queue without removing it from the queue, delaying for delay_ms in ms before timing out.
StatusCode queue_peek(Queue *queue, void *buf, uint32_t delay_ms);

// Return the size of the queue.
uint32_t queue_get_size(Queue *queue);
