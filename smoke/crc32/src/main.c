#include <stdint.h>
#include "crc32.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"

static const uint32_t test_buffer[] = {0x12345678, 0x9ABCDEF0, 0x0FEDCBA9, 0x87654321};
static const uint32_t expected_crc = 0xA1D3D013;  // pre calculated
// Go to https://crccalc.com/?crc=123456789&method=&datatype=0&outtype=0
// Check crc value for CRC-32/BZIP2

// CRC32 test task
TASK(crc32_smoke_test_task, TASK_STACK_512) {

  // Calculate the CRC32 for the test buffer
  uint32_t calculated_crc = crc_calculate(test_buffer, sizeof(test_buffer) / sizeof(test_buffer[0]));

  // Verify that the calculated CRC matches the expected value
  if (calculated_crc == expected_crc) {
    LOG_DEBUG("CRC32 Smoke Test: PASS");
  } else {
    LOG_DEBUG("CRC32 Smoke Test: FAIL - Expected: 0x%08X, Got: 0x%08X", expected_crc, calculated_crc);
  }
}

int main(void) {
  tasks_init();
  log_init();

  // Start the CRC32 test task (single test run)
  tasks_init_task(crc32_smoke_test_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  return 0;
}
