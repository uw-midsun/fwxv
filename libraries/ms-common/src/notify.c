#include "notify.h"
#include "log.h"

typedef struct Subscription {
  Event e;
  TaskHandle_t task;
} Subscription;

static uint8_t s_topic_indices[NUM_TOPICS];
static Subscription s_task_publish_table[NUM_TOPICS][NUM_TOPIC_ENTRIES];

bool event_from_notification(uint32_t *notification, Event *event) {
  if (notification == NULL) {
    // This should not occur
    LOG_WARN("Invalid Input"); 
    event = NULL;
    return true;
  }
  // Get index of first 1 in notification
  *event = 31 - __builtin_clz(*notification);
  // Clear bit
  *notification = *notification & ~(1<<*event);
  // Check if there are still bits to clear
  if (*notification) {
    return true;
  } else {
    return false;
  }
}
  
StatusCode notify_get(uint32_t *notification) {
  if (notification == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  BaseType_t result = xTaskNotifyWait(0, 0, notification, 0);
  if (result) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_EMPTY;
  }
}

StatusCode notify_wait(uint32_t *notification, uint32_t ms_to_wait) {
  TickType_t ticks_to_wait = 0;
  if (notification == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (ms_to_wait == BLOCK_INDEFINITELY) {
    ticks_to_wait = portMAX_DELAY;
  } else {
    ticks_to_wait = pdMS_TO_TICKS(ms_to_wait);
  }
  // Block on notification arriving
  BaseType_t result = xTaskNotifyWait(0, 0, notification, ticks_to_wait);
  if (result) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_TIMEOUT;
  }
}

StatusCode notify(TaskHandle_t task, Event event) {
  BaseType_t result = xTaskNotify(task, 1 << event, eSetBits);
  // Should always return true
  if (result) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNKNOWN;
  }
}

StatusCode subscribe(TaskHandle_t task, Topic topic, Event event) {
  // Subscribes must happen before scheduler start
  if (!(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)) {
    return STATUS_CODE_UNREACHABLE;
  }
  if (topic >= NUM_TOPICS || task == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Get topic and next index to place subscription at
  uint8_t index = s_topic_indices[topic];
  if (index >= NUM_TOPIC_ENTRIES) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  Subscription *sub = &s_task_publish_table[topic][index];
  // Update subscription
  sub->task = task;
  sub->e = event;
  // Increment index
  s_topic_indices[topic]++;
  return STATUS_CODE_OK;
}

StatusCode publish(Topic topic) {
  if (topic >= NUM_TOPICS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Iterate through all stored subscriptions, notifying the 
  // task at its specified event
  for (uint8_t sub = 0; sub < s_topic_indices[topic]; sub++) {
    Subscription *pub = &s_task_publish_table[topic][sub];
    status_ok_or_return(notify(pub->task, pub->e));
  }
  return STATUS_CODE_OK;
}







