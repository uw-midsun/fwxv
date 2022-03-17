#include "soft_timer.h"

StatusCode soft_timer_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  timer->handle =
      xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE, NULL, callback, &timer->buffer);
  if (xTimerStart(timer->handle, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

// Cancels the soft timer specified by name. Returns true if successful.
// the timer is not cancelled immediately,
StatusCode soft_timer_cancel(SoftTimer *timer) {
  if (xTimerDelete(timer->handle, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

// restart the timer
StatusCode soft_timer_reset(SoftTimer *timer) {
  if (xTimerReset(timer->handle, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

// Checks if the software timer is running
bool soft_timer_inuse(SoftTimer *timer) {
  return xTimerIsTimerActive(timer->handle);
}

// Checks the time left in ticks on a particular timer. Returns a 0 if the timer
// has expired and is no longer in use.
uint32_t soft_timer_remaining_time(SoftTimer *timer) {
  if (!soft_timer_inuse(timer)) {
    return 0;
  }
  // convert to ms
  return (xTimerGetExpiryTime(timer->handle) - xTaskGetTickCount()) * 1000U / configTICK_RATE_HZ;
}
