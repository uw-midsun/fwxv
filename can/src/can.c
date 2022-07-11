
#include "tasks.h"
#include "notify.h"
#include "semphr.h"
#include "event_groups.h"
// #include "can.h"
#include "can_codegen.h"

#include "log.h"

rx_struct g_rx_struct;
tx_struct g_tx_struct;

static CanStorage *s_can_storage;

static SemaphoreHandle_t s_tx_sem_handle;
static StaticSemaphore_t s_tx_sem;

static EventGroupHandle_t s_rx_event_handle;
static StaticEventGroup_t s_rx_event;

#define CAN_RX_EVENT (1 << 0)

TASK(CAN_RX, TASK_MIN_STACK_SIZE)
{
  int counter = 0;
  while (true)
  {
    // TODO: Do we want to wait forever on a sem?
    xSemaphoreTake(s_tx_sem_handle, portMAX_DELAY);
    LOG_DEBUG("can_rx called: %d!\n", counter);
    counter++;

    can_rx_all();

    // xEventGroupSetBits(s_rx_event_handle, CAN_RX_EVENT);
    publish(TOPIC_1);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

TASK(CAN_TX, TASK_MIN_STACK_SIZE)
{
  int counter = 0;
  uint32_t notification;
  Event e;
  while (true)
  {
    // TODO: Need receive all module to get all tasks beforehand
    notify_wait(&notification, BLOCK_INDEFINITELY);
    event_from_notification(&notification, &e);
    // TODO: Need to assert event
    // assert(0, e);
    // TODO: need to check return bits to see if function failed or not
    // xEventGroupWaitBits(s_rx_event_handle, CAN_RX_EVENT, pdTRUE, pdFALSE, portMAX_DELAY);
    LOG_DEBUG("can_tx called: %d!\n", counter);
    counter++;

    can_tx_all();

    // TODO: Technically this can fail if didn't get semaphore
    // vTaskDelay(pdMS_TO_TICKS(1000));
    xSemaphoreGive(s_tx_sem_handle);
  }
}

StatusCode can_init(CanStorage *storage, const CanSettings *settings)
{
  if (settings->device_id >= CAN_MSG_MAX_DEVICES)
  {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid device ID");
  }

  // Initializing storage
  memset(storage, 0, sizeof(*storage));
  storage->device_id = settings->device_id;

  s_can_storage = storage;

  // Initializing global structs
  memset(&g_rx_struct, 0, sizeof(g_rx_struct));
  memset(&g_tx_struct, 0, sizeof(g_tx_struct));

  // Create Semaphores
  s_tx_sem_handle = xSemaphoreCreateBinaryStatic(&s_tx_sem);
  configASSERT(s_tx_sem_handle);
  // start off RX Task
  xSemaphoreGive(s_tx_sem_handle);

  // Create Event Group
  s_rx_event_handle = xEventGroupCreateStatic(&s_rx_event);
  configASSERT(s_rx_event_handle);

  status_ok_or_return(can_queue_init(&s_can_storage->rx_queue));
  status_ok_or_return(can_queue_init(&s_can_storage->tx_queue));
  // status_ok_or_return(can_ack_init(&s_can_storage->ack_requests));
 
  // Initialize hardware settings
  status_ok_or_return(can_hw_init(&s_can_storage->rx_queue, &s_can_storage->tx_queue, settings));

  // Create RX and TX Tasks
  // TODO: Figure out priorities
  status_ok_or_return(tasks_init_task(CAN_RX, TASK_PRIORITY(2), NULL));
  status_ok_or_return(tasks_init_task(CAN_TX, TASK_PRIORITY(2), NULL));

  status_ok_or_return(subscribe(CAN_TX, TOPIC_1, CAN_RX_EVENT));

  return STATUS_CODE_OK;
}

StatusCode can_receive(const CanMessage *msg, const CanAckRequest *ack_request)
{
  // TODO: Figure out the ack_request
  StatusCode ret = can_queue_pop(&s_can_storage->rx_queue, msg);

  if (ret == STATUS_CODE_OK)
  {
    // LOG_DEBUG("Source Id: %d\n", msg->id);
    // LOG_DEBUG("Data: %lx\n", msg->data);
    // LOG_DEBUG("DLC: %ld\n", msg->dlc);
    // LOG_DEBUG("ret: %d\n", ret);
  }

  return ret;
}

StatusCode can_transmit(const CanMessage *msg, const CanAckRequest *ack_request)
{
  // TODO: this is where autogenearted happens
  // messages get packed and pushed onto transmit queue
  // messages come off the queue and out can_hw_transmit

  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg->msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  // if (ack_request != NULL) {
  //   if (!CAN_MSG_IS_CRITICAL(msg)) {
  //     return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: ACK requested for non-critical message");
  //   }

  //   StatusCode ret = can_ack_add_request(&s_can_storage->ack_requests, msg->msg_id, ack_request);
  //   status_ok_or_return(ret);
  // }

  CanId can_id = { .raw = msg->msg_id };

  bool extended = (can_id.raw >= CAN_MSG_ID_STD_MAX);
  return can_hw_transmit(can_id.raw, extended, msg->data_u8, msg->dlc);
}

StatusCode can_add_filter(CanMessageId msg_id) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  CanId can_id = { .raw = msg_id };
  CanId mask = { 0 };
  mask.raw = (uint32_t)~mask.msg_id;

  return can_hw_add_filter(mask.raw, can_id.raw, false);
}
