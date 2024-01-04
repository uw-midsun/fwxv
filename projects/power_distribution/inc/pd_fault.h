#pragma once

#include "status.h"

/*
 * @brief Checks for PD fault states and updates fault bitset CAN message
 * @return STATUS_CODE_OK on success or appropriate error code
 */
StatusCode check_pd_fault(void);
