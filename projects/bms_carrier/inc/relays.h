#pragma once

#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "can.h"
#include "delay.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "soft_timer.h"
#include "status.h"
#include "task.h"

#define BMS_CLOSE_RELAYS_DELAY_MS 200
#define KILLSWITCH_IT 3

// Enumerated set of relays to be closed
typedef enum RelayType {
  NO_RELAYS = 0,
  POS_RELAY,
  NEG_RELAY,
  SOLAR_RELAY,
  RELAY_CHECK,
} RelayType;

// Closes relays in sequence
StatusCode init_bms_relays(GpioAddress *killswitch);

// Independent solar control
void bms_open_solar();
void bms_close_solar();

// Turns off GPIOs to open relays
void bms_relay_fault(void);
