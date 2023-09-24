#pragma once
// Library encompassing task-to-task and interrupt to task notifications

#include "status.h"
#include "tasks.h"

// Each Task has an associated 32-bit notification value which other tasks can send a value to

// Defines a statically-allocated 2D array of task notification objects
// Each is comprised of an 8-bit event and a 32-bit task handle
// If space savings are needed, a constant event can be used,
// or the number of topics can be reduced
#define NUM_TOPICS 4
#define NUM_TOPIC_ENTRIES 10
#define BLOCK_INDEFINITELY UINT16_MAX

// Value indicates that no valid event exists
#define INVALID_EVENT 32
typedef uint8_t Event;

// Topics for group publish
typedef enum {
  TOPIC_1 = 0,
  TOPIC_2,
  TOPIC_3,
  TOPIC_4,
  NUM_PUB_TOPICS,
} Topic;

// Gets highest priority (highest value)  event available, and clears it
// Returns STATUS_CODE_OK if all events processed, STATUS_CODE_INCOMPLETE if any remaining
// Notification value should be processed until all events are cleared
StatusCode event_from_notification(uint32_t *notification, Event *event);

// Checks if Notification is available in event
// Clears event and returns true if it exists
bool notify_check_event(uint32_t *notification, Event event);

// Gets the current notification value for the calling task
// Returns STATUS_CODE_OK if value was updated, STATUS_CODE_EMPTY otherwise
StatusCode notify_get(uint32_t *notification);

// Blocks on receiving a notification with a timeout
// Returns STATUS_CODE_OK if notification received, STATUS_CODE_TIMEOUT if timeout occurred
StatusCode notify_wait(uint32_t *notification, uint32_t ms_to_wait);

// Sends an event notification to a specific task
StatusCode notify(Task *task, Event event);

// ISR specific notify method
void notify_from_isr(Task *task, Event event);

// Subscribes a task to a topic via its task handle
// The event is what the task will receive when the topic is published to
// Subscribes should happen before the scheduler is started, and are not mutable
StatusCode subscribe(Task *task, Topic topic, Event event);

// Sends a notification to all tasks in a topic from calling task
StatusCode publish(Topic topic);
