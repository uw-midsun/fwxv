#include "queues.h"

#include <string.h>

StatusCode queue_init(Queue *queue) {
  queue->handle =
      xQueueCreateStatic(queue->num_items, queue->item_size, queue->storage_buf, &queue->queue);

  if (queue->handle == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode queue_send(Queue *queue, const void *item, uint32_t delay_ms) {
  BaseType_t ret = xQueueSend(queue->handle, item, pdMS_TO_TICKS(delay_ms));

  if (ret == errQUEUE_FULL) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  return STATUS_CODE_OK;
}

StatusCode queue_receive(Queue *queue, void *buf, uint32_t delay_ms) {
  BaseType_t ret = xQueueReceive(queue->handle, buf, pdMS_TO_TICKS(delay_ms));

  if (ret == pdFALSE) {
    return STATUS_CODE_EMPTY;
  }

  return STATUS_CODE_OK;
}

StatusCode queue_peek(Queue *queue, void *buf, uint32_t delay_ms) {
  BaseType_t ret = xQueuePeek(queue->handle, buf, pdMS_TO_TICKS(delay_ms));

  if (ret == pdFALSE) {
    return STATUS_CODE_EMPTY;
  }

  return STATUS_CODE_OK;
}

void queue_reset(Queue *queue) {
  xQueueReset(queue->handle);
}

uint32_t queue_get_num_items(Queue *queue) {
  return queue->num_items;
}
