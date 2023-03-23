#pragma once

#include <stdbool.h>

#include "gpio.h"
#include "status.h"

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
  GpioAddress precharge_monitor2;
} PrechargeControlSettings;

typedef struct PrechargeControlStorage {
  GpioAddress precharge_control;
} PrechargeControlStorage;

PrechargeControlStorage *test_get_storage(void);

StatusCode prv_set_precharge_control(PrechargeControlStorage *storage, const GpioState state);

StatusCode run_precharge_rx_cycle();

PrechargeState get_precharge_state();

void prv_populate_storage(PrechargeControlStorage *storage,
                          const PrechargeControlSettings *settings);

StatusCode precharge_control_init(const PrechargeControlSettings *settings);
