#pragma once

#include <stdint.h>

#include "status.h"

// Initializes test end and test start semaphore.
void tests_init(void);

/**
 * @brief get the count for the test_start semaphore.
 *
 * @return uint8_t: semaphore count.
 */
uint8_t test_start_get(void);
/**
 * @brief get the count for the test_end semaphore.
 *
 * @return uint8_t: semaphore count.
 */
uint8_t test_end_get(void);

/**
 * @brief release the test_start semaphore.
 *
 * @return StatusCode: STATUS_CODE_OK if semaphore was released successfully,
 * otherwise STATUS_CODE_INTERNAL_ERROR.
 */
StatusCode test_start_give(void);
/**
 * @brief release the test_end semaphore.
 *
 * @return StatusCode: STATUS_CODE_OK if semaphore was released successfully,
 * otherwise STATUS_CODE_INTERNAL_ERROR.
 */
StatusCode test_end_give(void);

/**
 * @brief obtain the test_start semaphore.
 *
 * @return StatusCode: STATUS_CODE_OK if semaphore was obtained successfully,
 * otherwise STATUS_CODE_INTERNAL_ERROR.
 */
StatusCode test_start_take(void);
/**
 * @brief obtain the test_end semaphore.
 *
 * @return StatusCode: STATUS_CODE_OK if semaphore was obtained successfully,
 * otherwise STATUS_CODE_INTERNAL_ERROR.
 */
StatusCode test_end_take(void);
