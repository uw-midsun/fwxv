#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "tasks.h"
#include "queues.h"
#include "status.h"
#include "delay.h"
#include "log.h"
#include "misc.h"

#define ITEM_SZ 6
#define QUEUE_LEN 5
#define BUF_SIZE (QUEUE_LEN * ITEM_SZ)

static const char s_list[QUEUE_LEN][ITEM_SZ] = {
    "Item1",
    "Item2",
    "Item3",
    "Item4",
    "Item5"
};

// Task static entities
static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
    .num_items = QUEUE_LEN,
    .item_size = ITEM_SZ,
    .storage_buf = s_queue1_buf,
};

TASK(task1, TASK_STACK_512) {
    printf("Task 1 initialized!\n");
    StatusCode ret;
    size_t to_send = 0;

    while (true) {
        ret = queue_send(&s_queue1, (void *)s_list[to_send], 0);
        if (ret != STATUS_CODE_OK) {
            printf("Task 1: write to queue failed at index %zu\n", to_send);
        }
        to_send = (to_send + 1) % QUEUE_LEN;
        delay_s(1);
    }
}

TASK(task2, TASK_STACK_512) {
    printf("Task 2 initialized!\n");
    char outstr[ITEM_SZ];  // Buffer to hold the received string
    StatusCode ret;

    while (true) {
        ret = queue_receive(&s_queue1, outstr, 0);
        if (ret == STATUS_CODE_OK) {
            printf("Task 2 received: %s\n", outstr);
        } else {
            printf("Task 2: read from queue failed\n");
        }
        delay_s(1);
    }
}

int main(void) {
    log_init();
    queue_init(&s_queue1);
    tasks_init();
    
    // Initialize tasks with appropriate priorities
    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(2), NULL);

    printf("Program start...\n");
    tasks_start();
    return 0;
}
