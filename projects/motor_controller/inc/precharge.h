#pragma once

#include <stdbool.h>

#include "gpio.h"
#include "notify.h"
#include "status.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef struct PrechargeSettings {
  GpioAddress precharge_control;
  GpioAddress precharge_monitor;
} PrechargeSettings;

StatusCode precharge_init(const PrechargeSettings *settings, Event event, const Task *task);
