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
#include "spi.h"

#define CONV_DELAY_MS 10
// Maximum number of retry attempts to read cell/aux data once triggered
#define RETRY_DELAY_MS 3
#define CELL_SENSE_CONVERSIONS 0

#define NUM_AFES 3
#define NUM_CELL_MODULES_PER_AFE 12
#define NUM_TOTAL_CELLS (NUM_AFES * NUM_CELL_MODULES_PER_AFE)
#define NUM_THERMISTORS (NUM_TOTAL_CELLS * 2)

// Fault thresholds
// TODO: Verify these values!
#define CELL_OVERVOLTAGE 42500
#define CELL_UNDERVOLTAGE 25000
#define CELL_UNBALANCED 5000
#define CELL_MAX_TEMPERATURE 60

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

StatusCode cell_sense_init(LtcAfeStorage *afe_storage);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, settings.num_cells)

StatusCode cell_conversions(void);

StatusCode cell_sense_run(void);
