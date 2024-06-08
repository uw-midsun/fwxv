#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "status.h"
/*
 * @brief Checks for PD fault states and updates fault bitset CAN message
 * @return STATUS_CODE_OK on success or appropriate error code
 */
uint8_t check_pd_fault(void);
bool check_aux_fault(void);
