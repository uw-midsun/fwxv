#include "FreeRTOS.h"
#include "semphr.h"

// Test start semaphore
static StaticSemaphore_t s_test_start_sem;
static SemaphoreHandle_t s_test_start_handle;

// Test end semaphore
static StaticSemaphore_t s_test_end_sem;
static SemaphoreHandle_t s_test_end_handle;

// Maximum number of tests that can take from test end semaphore
// and the max number of tests that we begin with for test start semaphore
#define MAX_NUM_TESTS 10

// The number of ticks to delay for take functions
TickType_t TICKS_TO_DELAY = 100;

// Initialize start semaphore with max count 10, initial count 10, and pointer to variable of type
// Static_Sempahore_t The semaphore can be accessed through the handle called s_test_start_handle
// The value of the semaphore begins at 10 to indicate the number of tests that still need to run,
// decrementing after each iteration.
void tests_init(void) {
  s_test_start_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TESTS, 10, &s_test_start_sem);
  s_test_end_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TESTS, 0, &s_test_end_sem);
}

uint8_t test_start_get(void) {
  uxSemaphoreGetCount(s_test_start_handle);
}

uint8_t test_end_get(void) {
  uxSemaphoreGetCount(s_test_end_handle);
}

StatusCode test_start_give(void) {
  return xSemaphoreGive(s_test_start_handle);
}

StatusCode test_end_give(void) {
  return xSemaphoreGive(s_test_end_handle);
}

StatusCode test_start_take(void) {
  return xSemaphoreTake(s_test_start_handle, TICKS_TO_DELAY);
}

StatusCode test_end_take(void) {
  return xSemaphoreTake(s_test_end_handle, TICKS_TO_DELAY);
}
