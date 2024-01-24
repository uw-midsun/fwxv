#pragma once

#include <stdbool.h>

#include "gpio.h"
#include "status.h"

#define PRECHARGE_EVENT 0
// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef enum {
  MCI_PRECHARGE_DISCHARGED = 0,
  MCI_PRECHARGE_INCONSISTENT,
  MCI_PRECHARGE_CHARGED
} PrechargeState;

typedef struct PrechargeControlSettings {
  GpioAddress precharge_control;
  GpioAddress precharge_monitor;
} PrechargeControlSettings;

StatusCode run_precharge_rx_cycle();

StatusCode precharge_control_init(const PrechargeControlSettings *settings);
