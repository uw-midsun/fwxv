#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "fsm.h"
#include "gpio.h"
#include "spi.h"
#include "status.h"

// This is an arbitrary limitation, can be increased/decreased if needed
#define LTC_AFE_MAX_DEVICES 5
// This is a device limitation
#define LTC_AFE_MAX_CELLS_PER_DEVICE 12
#define LTC_AFE_MAX_CELLS (LTC_AFE_MAX_DEVICES * LTC_AFE_MAX_CELLS_PER_DEVICE)
#define LTC_AFE_MAX_THERMISTORS LTC_AFE_MAX_CELLS

#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

#define LTC_AFE_FSM_CELL_CONV_DELAY_MS 10
#define LTC_AFE_FSM_AUX_CONV_DELAY_MS 6
// Maximum number of retry attempts to read cell/aux data once triggered
#define LTC_AFE_FSM_MAX_RETRY_COUNT 3

#define NUM_LTC_AFE_FSM_STATES 6
#define NUM_LTC_AFE_FSM_TRANSITIONS 14

#define NUM_AFES 3
#define NUM_CELL_MODULES_PER_AFE 6
#define NUM_TOTAL_CELLS (NUM_AFES * NUM_CELL_MODULES_PER_AFE)
#define NUM_THERMISTORS (NUM_TOTAL_CELLS * 2)
#define MAX_AFE_FAULTS 5

#define AFE_SPI_PORT SPI_PORT_1
#define AFE_SPI_SS \
  { .port = GPIO_PORT_A, .pin = 4 }
#define AFE_SPI_SCK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }

// select the ADC mode (trade-off between speed or minimizing noise)
// see p.50 for conversion times and p.23 for noise
typedef enum {
  LTC_AFE_ADC_MODE_27KHZ = 0,
  LTC_AFE_ADC_MODE_7KHZ,
  LTC_AFE_ADC_MODE_26HZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  NUM_LTC_AFE_ADC_MODES
} LtcAfeAdcMode;

typedef struct LtcAfeBitset {
  uint16_t cell_bitset;
  uint16_t aux_bitset;
} LtcAfeBitset;

typedef struct LtcAfeSettings {
  GpioAddress cs;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;

  const SpiPort spi_port;
  uint32_t spi_baudrate;

  LtcAfeAdcMode adc_mode;

  uint16_t cell_bitset[LTC_AFE_MAX_DEVICES];
  uint16_t aux_bitset[LTC_AFE_MAX_DEVICES];

  size_t num_devices;
  size_t num_cells;
  size_t num_thermistors;

  void *result_context;
} LtcAfeSettings;

typedef struct LtcAfeStorage {
  Fsm fsm;

  // Only used for storage in the FSM so we store data for the correct cells
  uint16_t aux_index;
  uint16_t retry_count;
  uint16_t cell_number;
  uint16_t time_elapsed;

  uint16_t cell_voltages[LTC_AFE_MAX_CELLS];
  uint16_t aux_voltages[LTC_AFE_MAX_THERMISTORS];

  uint16_t discharge_bitset[LTC_AFE_MAX_DEVICES];

  uint16_t cell_result_lookup[LTC_AFE_MAX_CELLS];
  uint16_t aux_result_lookup[LTC_AFE_MAX_THERMISTORS];
  uint16_t discharge_cell_lookup[LTC_AFE_MAX_CELLS];

  uint16_t device_cell;

  LtcAfeSettings settings;
} LtcAfeStorage;

// Wraps the LTC AFE module and handles all the sequencing.
// Requires LTC AFE, soft timers to be initialized.
//

DECLARE_FSM(ltc_afe_fsm);

typedef enum LtcAfeFsmStateId {
  LTC_AFE_IDLE = 0,
  LTC_AFE_TRIGGER_CELL_CONV,
  LTC_AFE_READ_CELLS,
  LTC_AFE_TRIGGER_AUX_CONV,
  LTC_AFE_READ_AUX,
  LTC_AFE_AUX_COMPLETE,
  LTC_AFE_FAULT
} LtcAfeFsmStateId;

// We can raise a fault using this when transitioning to LTC_AFE_FAULT to identify where it came
// from
typedef enum {
  LTC_AFE_FSM_FAULT_TRIGGER_CELL_CONV = 0,
  LTC_AFE_FSM_FAULT_READ_ALL_CELLS,
  LTC_AFE_FSM_FAULT_TRIGGER_AUX_CONV,
  LTC_AFE_FSM_FAULT_READ_AUX,
  NUM_LTC_AFE_FSM_FAULTS
} LtcAfeFsmFault;

typedef enum {
  LTC_TRIGGER_CELL_EVENT_START = 0,
  LTC_TRIGGER_AUX_EVENT_START,
  NUM_LTC_AFE_ADC_MODES
} LtcAfeOutputEvent;

typedef struct CellSenseSettings {
  // Units are 100 uV (or DeciMilliVolts)
  uint16_t undervoltage_dmv;
  uint16_t overvoltage_dmv;
  uint16_t charge_overtemp_dmv;
  uint16_t discharge_overtemp_dmv;
} CellSenseSettings;

typedef struct AfeReadings {
  // TODO(SOFT-9): total_voltage used to be stored here as well
  uint16_t voltages[NUM_TOTAL_CELLS];
  uint16_t temps[NUM_THERMISTORS];
} AfeReadings;

typedef struct CellSenseStorage {
  LtcAfeStorage *afe;
  AfeReadings *readings;
  uint16_t num_afe_faults;
  CellSenseSettings settings;
} CellSenseStorage;

// Initialize the LTC6811.
// |settings.cell_bitset| and |settings.aux_bitset| should be an array of bitsets where bits 0 to 11
// represent whether we should monitor the cell input for the given device.
// |settings.cell_result_cb| and |settings.aux_result_cb| will be called when the corresponding
// conversion is completed.
StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, settings.num_cells)
StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge);

StatusCode cell_sense_init(const CellSenseSettings *settings, AfeReadings *readings,
                           LtcAfeStorage *afe);

StatusCode ltc_afe_fsm_init(Fsm *fsm, LtcAfeStorage *afe);
