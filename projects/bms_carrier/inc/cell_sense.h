#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "bms.h"
#include "delay.h"
#include "fault_bps.h"
#include "gpio.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "relays.h"
#include "spi.h"

#define CONV_DELAY_MS 10
#define AUX_CONV_DELAY_MS 3
// Maximum number of retry attempts to read cell/aux data once triggered
#define RETRY_DELAY_MS 1
#define CELL_SENSE_CONVERSIONS 0

// Fault thresholds - units of mV*10
#define CELL_OVERVOLTAGE 42500
#define CELL_UNDERVOLTAGE 25000
#define CELL_UNBALANCED 5000
#define CELL_MAX_TEMPERATURE_DISCHARGE 60
#define CELL_MAX_TEMPERATURE_CHARGE 50

#define SOLAR_VOLTAGE_THRESHOLD 42000

#define AFE_BALANCING_UPPER_THRESHOLD 41500
#define AFE_BALANCING_LOWER_THRESHOLD 40000

#define AFE_SPI_PORT SPI_PORT_2
#define AFE_SPI_CS \
  { .port = GPIO_PORT_B, .pin = 12 }
#define AFE_SPI_SCK \
  { .port = GPIO_PORT_B, .pin = 13 }
#define AFE_SPI_MISO \
  { .port = GPIO_PORT_B, .pin = 14 }
#define AFE_SPI_MOSI \
  { .port = GPIO_PORT_B, .pin = 15 }

StatusCode cell_sense_init(BmsStorage *bms_store);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, settings.num_cells)

StatusCode cell_discharge(LtcAfeStorage *afe);
