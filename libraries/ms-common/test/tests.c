#include "FreeRTOS.h"
#include "semphr.h"

// Test start semaphore
static StaticSemaphore_t s_test_start_sem;
static SemaphoreHandle_t s_test_start_handle;

// The max number of tests that we begin with
#define MAX_NUM_TESTS 10

// Initialize semaphore with max count 10, initial count 10, and pointer to variable of type
// Static_Sempahore_t The semaphore can be accessed through the handle called s_test_start_handle
// The value of the semaphore begins at 10 to indicate the number of tests that still need to run,
// decrementing after each iteration.
void tests_start_init(void) {
  s_test_start_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TESTS, 10, &s_test_start_sem);
}
