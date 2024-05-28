#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status.h"

#define BPS_FAULT_LIGHT_PERIOD_MS 500  // Signal blink frequency of 1Hz
/*
 * @brief Checks for PD fault states and updates fault bitset CAN message
 * @return STATUS_CODE_OK on success or appropriate error code
 */
uint8_t check_pd_fault(void);
bool check_aux_fault(void);
StatusCode init_bps_fault(void);
