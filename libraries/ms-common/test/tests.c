#include "FreeRTOS.h"
#include "semphr.h"

// Test end semaphore
static StaticSemaphore_t s_test_end_sem;
static SemaphoreHandle_t s_test_end_handle;

// Maximum number of tests that can take from test end semaphore
#define MAX_NUM_TESTS 10

void tests_init(void) {
    s_test_end_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TESTS, 0, &s_test_end_sem);
}
