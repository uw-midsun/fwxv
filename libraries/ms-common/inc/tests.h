#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

// Initializes test end and test start semaphore
void tests_init(void);

// Get for the test end and test start semaphores
void test_start_get(void);
void test_end_get(void);

// Give for the test start and test end semaphores
void test_start_give(void);
void test_end_give(void);

// Take for the test start anbd test end semaphores
void test_start_take(void);
void test_end_take(void);
