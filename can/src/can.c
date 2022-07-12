
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

static SemaphoreHandle_t s_can_rx_sem_handle;
static StaticSemaphore_t s_can_rx_sem;

static SemaphoreHandle_t s_can_tx_sem_handle;
static StaticSemaphore_t s_can_tx_sem;

StatusCode run_can_rx_cycle()
{
  BaseType_t ret = xSemaphoreGive(s_can_rx_sem_handle);

  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode run_can_tx_cycle()
{
  BaseType_t ret = xSemaphoreGive(s_can_tx_sem_handle);

  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

TASK(CAN_RX, TASK_MIN_STACK_SIZE)
{
  int counter = 0;
  while (true)
  {
    xSemaphoreTake(s_can_rx_sem_handle, portMAX_DELAY);
    LOG_DEBUG("can_rx called: %d!\n", counter);
    counter++;

    can_rx_all();

    send_task_end();
  }
}

TASK(CAN_TX, TASK_MIN_STACK_SIZE)
{
  int counter = 0;
  while (true)
  {
    xSemaphoreTake(s_can_tx_sem_handle, portMAX_DELAY);
    LOG_DEBUG("can_tx called: %d!\n", counter);
    counter++;

    can_tx_all();

    send_task_end();
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
  s_can_rx_sem_handle = xSemaphoreCreateBinaryStatic(&s_can_rx_sem);
  configASSERT(s_can_rx_sem_handle);
  s_can_tx_sem_handle = xSemaphoreCreateBinaryStatic(&s_can_tx_sem);
  configASSERT(s_can_tx_sem_handle);

  status_ok_or_return(can_queue_init(&s_can_storage->rx_queue));
 
  // Initialize hardware settings
  status_ok_or_return(can_hw_init(&s_can_storage->rx_queue, settings));
  
  //when running the can_one_shot_mode
  //sudo modprobe can
  //sudo modprobe can_raw
  //sudo modprobe vcan
  //sudo ip link add dev vcan0 type vcan
  //sudo ip link set up vcan0
  //scons --project=can_one_shot_mode --platform=x86


  // Dsiable can tasks
  // all needed functions are in the slides
  if (settings->mode == 1){
    LOG_DEBUG("In can one shot mode\n");
  }

  else{
    // Create RX and TX Tasks
    // TODO: Figure out priorities
    status_ok_or_return(tasks_init_task(CAN_RX, TASK_PRIORITY(2), NULL));
    status_ok_or_return(tasks_init_task(CAN_TX, TASK_PRIORITY(2), NULL));

    status_ok_or_return(subscribe(CAN_TX->handle, TOPIC_1, CAN_RX_EVENT));
  }
  return STATUS_CODE_OK;
}

StatusCode can_receive(const CanMessage *msg)
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

StatusCode can_transmit(const CanMessage *msg)
{
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg->id.msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  return can_hw_transmit(msg->id.raw, msg->extended, msg->data_u8, msg->dlc);
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
