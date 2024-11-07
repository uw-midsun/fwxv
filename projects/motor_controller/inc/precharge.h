#pragma once

#include <stdbool.h>

#include "gpio.h"
#include "notify.h"
#include "status.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

#define PRECHARGE_EVENT 0

typedef struct PrechargeSettings {
  GpioAddress motor_sw;
  GpioAddress precharge_monitor;
} PrechargeSettings;

StatusCode precharge_init(const PrechargeSettings *settings, Event event, const Task *task);
