// Basic wrapper for FreeRTOS queues.
#include "FreeRTOS.h"
#include "queue.h"
#include "status.h"

#define MS_QUEUE_DELAY_BLOCKING portMAX_DELAY

#define TICKS_TO_MS portTICK_PERIOD_MS 

typedef struct {
    uint32_t size;
    uint32_t item_size;
    uint8_t *storage_buf;
    StaticQueue_t *queue_buf;
} MsQueueSettings;


typedef struct {
    QueueHandle_t handle;
    MsQueueSettings settings;
} MsQueueStorage;

// Create a queue with the parameters specified in settings. Returns STATUS_CODE_OK if successful, STATUS_CODE_INVALID_ARGS otherwise.
StatusCode ms_queue_create(MsQueueStorage *storage, MsQueueSettings *settings);

// Place an item into the queue, delaying for delay_ticks in ticks (TODO(SOFT-608): add a wrapper to have delay in ms).
StatusCode ms_queue_send(MsQueueStorage *storage, const void *item, uint32_t delay_ticks);

// Receive an item from the queue, delaying for delay_ticks in ticks (TODO(SOFT-608): add a wrapper to have delay in ms).
StatusCode ms_queue_receive(MsQueueStorage *storage, void *buf, uint32_t delay_ticks);

// Attempt to receive an item from the queue without removing it from the queue, delaying for delay_ticks in ticks (TODO(SOFT-608): add a wrapper to have delay in ms).
StatusCode ms_queue_peek(MsQueueStorage *storage, void *buf, uint32_t delay_ticks);

// Return the size of the queue.
uint32_t ms_queue_get_size(MsQueueStorage *storage);




