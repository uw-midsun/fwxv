#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

// Initializes test end semaphore
void tests_init(void);

// Initializes test start semaphore
void tests_start_init(void);
