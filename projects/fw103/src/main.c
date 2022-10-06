#include "log.h"
#include "queues.h"
#include "tasks.h"

#define NUM_ITEMS 5
#define ITEM_SIZE sizeof(uint32_t)

static uint8_t s_queue_buf[NUM_ITEMS*ITEM_SIZE];
