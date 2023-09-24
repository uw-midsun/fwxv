#pragma once

#include "can_msg.h"
#include "queues.h"

#define CAN_QUEUE_SIZE 64

typedef struct CanQueue {
  Queue queue;
  CanMessage msg_nodes[CAN_QUEUE_SIZE];
} CanQueue;

// Using #defines to reduce instruction jumps

#define can_queue_init(can_queue)                                   \
  ({                                                                \
    Queue* queue       = &(can_queue)->queue;                       \
    queue->num_items   = CAN_QUEUE_SIZE;                            \
    queue->item_size   = sizeof(CanMessage);                        \
    queue->storage_buf = (uint8_t*) (&(can_queue)->msg_nodes);      \
    queue_init(&(can_queue)->queue);                                \
  })

/**
 * @brief add a message to a can queue.
 * 
 * @param can_queue Queue *, can queue to add the message to.
 * @param source CanMessage *, message to add.
 * 
 * @return
 * StatusCode: STATUS_CODE_OK if message was added successfully, otherwise STATUS_CODE_RESOURCE_EXHAUSTED.
 */
#define can_queue_push(can_queue, source)                           \
    queue_send(&(can_queue)->queue, (source), 0)

/**
 * @brief add a message to a can queue from inside an interrupt.
 * 
 * @param can_queue Queue *, can queue to add the message to.
 * @param source CanMessage *, message to add.
 * @param high_prio_woken BaseType_t *, pdTRUE if a higher priority task was woken by this queue push.
 * 
 * @return
 * StatusCode: STATUS_CODE_OK if message was added successfully, STATUS_CODE_RESOURCE_EXHAUSTED otherwise.
 */
#define can_queue_push_from_isr(can_queue, source, high_prio_woken) \
    queue_send_from_isr(&(can_queue)->queue, (source), high_prio_woken)

/**
 * @brief peek the first message in the can queue.
 * 
 * @param can_queue Queue *, can queue to peek the message from.
 * @param dest CanMessage *, address to store the message.
 * 
 * @return
 * StatusCode: STATUS_CODE_OK if there is a message, otherwise STATUS_CODE_EMPTY.
 */
#define can_queue_peek(can_queue, dest)                             \
    queue_peek(&(can_queue)->queue, (dest), 0)

/**
 * @brief remove the first message in the can queue.
 * 
 * @param can_queue Queue *, can queue to remove the message from.
 * @param dest CanMessage *, address to store the message.
 * 
 * @return
 * StatusCode: STATUS_CODE_OK if there is a message, otherwise STATUS_CODE_EMPTY.
 */
#define can_queue_pop(can_queue, dest)                              \
    queue_receive(&(can_queue)->queue, (dest), 0)

/**
 * @brief remove the first message in the can queue from ISR.
 * 
 * @param can_queue Queue *, can queue to remove the message from.
 * @param dest CanMessage *, address to store the message.
 * @param high_prio_woken BaseType_t *, pdTRUE if a higher priority task was woken by this queue pop.
 * 
 * @return
 * StatusCode: STATUS_CODE_OK if there is a message, otherwise STATUS_CODE_EMPTY.
 */
#define can_queue_pop_from_isr(can_queue, dest, higher_prio_woken)  \
    queue_receive_from_isr(&(can_queue)->queue, (dest), high_prio_woken)

/**
 * @brief get the number of items the can queue can hold.
 * 
 * @param can_queue Queue *, can queue.
 * 
 * @return
 * uint32_t: the number of items in the queue.
 */
#define can_queue_size(can_queue)                                   \
    queue_get_num_items(&(can_queue)->queue)
