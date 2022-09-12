#include "soft_timer.h"

StatusCode soft_timer_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  timer->id = xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE,  //
                                 NULL, callback, &timer->buffer);
  if (xTimerStart(timer->id, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

StatusCode soft_timer_cancel(SoftTimer *timer) {
  if (xTimerDelete(timer->id, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

StatusCode soft_timer_reset(SoftTimer *timer) {
  if (xTimerReset(timer->id, 0) != pdPASS) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "timer command queue is full");
  }
  return STATUS_CODE_OK;
}

bool soft_timer_inuse(SoftTimer *timer) {
  return xTimerIsTimerActive(timer->id);
}

uint32_t soft_timer_remaining_time(SoftTimer *timer) {
  if (!soft_timer_inuse(timer)) {
    return 0;
  }
  // convert to ms
  return (xTimerGetExpiryTime(timer->id) - xTaskGetTickCount()) * 1000U / configTICK_RATE_HZ;
}
