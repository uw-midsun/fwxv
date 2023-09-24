#pragma once

#include <stdint.h>

#include "status.h"

// Initializes test end and test start semaphore
void tests_init(void);

// Get for the test end and test start semaphores
uint8_t test_start_get(void);
uint8_t test_end_get(void);

// Give for the test start and test end semaphores
StatusCode test_start_give(void);
StatusCode test_end_give(void);

// Take for the test start anbd test end semaphores
StatusCode test_start_take(void);
StatusCode test_end_take(void);
