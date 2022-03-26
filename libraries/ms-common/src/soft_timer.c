#include "soft_timer.h"

StatusCode soft_timer_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  return soft_timer_start_with_context(duration_ms, callback, timer, NULL);
}

StatusCode soft_timer_start_with_context(uint32_t duration_ms, SoftTimerCallback callback,
                                         SoftTimer *timer, void *context) {
  timer->id = xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE,  //
                                 context, callback, &timer->buffer);
  if (xTimerStart(timer->id, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

void *soft_timer_get_context(SoftTimerId id) {
  return pvTimerGetTimerID(id);
}

// Cancels the soft timer specified by name. Returns true if successful.
// the timer is not cancelled immediately,
StatusCode soft_timer_cancel(SoftTimer *timer) {
  if (xTimerDelete(timer->id, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

// restart the timer
StatusCode soft_timer_reset(SoftTimer *timer) {
  if (xTimerReset(timer->id, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

// Checks if the software timer is running
bool soft_timer_inuse(SoftTimer *timer) {
  return xTimerIsTimerActive(timer->id);
}

// Checks the time left in ticks on a particular timer. Returns a 0 if the timer
// has expired and is no longer in use.
uint32_t soft_timer_remaining_time(SoftTimer *timer) {
  if (!soft_timer_inuse(timer)) {
    return 0;
  }
  // convert to ms
  return (xTimerGetExpiryTime(timer->id) - xTaskGetTickCount()) * 1000U / configTICK_RATE_HZ;
}
