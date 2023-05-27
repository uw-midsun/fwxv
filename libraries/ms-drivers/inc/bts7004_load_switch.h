#pragma once
// Driver for the BTS7004 load switch.
// Requires GPIO, interrupts, soft timers, and ADC (in ADC_MODE_SINGLE) to be initialized.

// If using with PCA9539R, required I2C to be initialized.

#include "adc.h"
#include "gpio.h"
#include "semaphore.h"
#include "task.h"

// Upper maximum for the possible leakage voltage that may be read from the SENSE pin at
// T(env) < 80 C (see p.g. 27 of BTS7004 datasheet)
#define BTS7004_MAX_LEAKAGE_VOLTAGE_MV 2

// Nominal scaling factor k(ILIS) for current output at the SENSE pin in normal operation.
#define BTS7004_IS_SCALING_NOMINAL 20000

// Max possible delay after input pin pulled low on fault, + 10 ms for buffer
// (see p.g. 39 of datasheet)
#define BTS7004_FAULT_RESTART_DELAY_MS 110
#define BTS7004_FAULT_RESTART_DELAY_US (BTS7004_FAULT_RESTART_DELAY_MS * 1000)

typedef struct {
  GpioAddress *sense_pin;  // pin mapped to the sense current pin of the BTS7004
  GpioAddress
      *enable_pin;       // pin mapped to the enable pin of the BTS7004 (if not using gpio expander)
  Task *event_handler;   // task will be notified of both data and fault events
  Event data_event;      // event for periodic measurements
  Event fault_event;     // event for meaured voltage over min_fault_voltage
  uint32_t interval_ms;  // interval inbetween periodic measurements
  uint32_t resistor;     // resistor value (in ohms) used to convert SENSE voltage to current
  int32_t bias;          // experimental bias to be subtracted from the resulting current, in mA
  uint16_t min_fault_voltage_mv;  // min voltage representing a fault, in mV
} Bts7004Settings;

typedef struct {
  Bts7004Settings settings;  // store the settings of the BTS7004
  Mutex *data_mutex;         // mutex used to protect reading_out, must be locked before accessing
  bool fault_in_progress;    // for internal use to ensure enable pin is not set during fault
  uint16_t reading_out;      // Reading from IN pin, in mA
} Bts7004Storage;

// Initialize the BTS7040 with the given settings
StatusCode bts7004_init(Bts7004Storage *storage, Bts7004Settings settings);

// Enable output by pulling the IN pin high.
StatusCode bts7004_enable_output(Bts7004Storage *storage);

// Disable output by pulling the IN pin low.
StatusCode bts7004_disable_output(Bts7004Storage *storage);

// Returns whether the output is enabled or disabled.
bool bts7004_get_output_enabled(Bts7004Storage *storage);

// Read the latest input current measurement, in mA. This does not get the measurement from
// the storage but instead reads it from the BTS7040 itself. Note that, due to the fault handling
// implementation, the pointer to storage has to be valid for at least
// BTS7040_FAULT_RESTART_DELAY_MS after the function is called before it is freed to avoid
// segfaults.
StatusCode bts7004_get_measurement(Bts7004Storage *storage, uint16_t *meas);

// Resume task that notifies event_handler task when a fault or a periodic update to reading_out.
// This can be used alongside bts7004_get_measurement as there is a mutex to prevent multiple
// accesses to the adc at the same time.
StatusCode bts7004_start(Bts7004Storage *storage);

// Suspends task that notifies event_handler
void bts7004_stop(Bts7004Storage *storage);
