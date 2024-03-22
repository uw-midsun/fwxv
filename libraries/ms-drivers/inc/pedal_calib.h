#pragma once
#include "max11600.h"

#define NUM_SAMPLES 1000

typedef enum {
  PEDAL_PRESSED = 0,
  PEDAL_UNPRESSED,
  NUM_PEDAL_STATES,
} PedalState;

typedef struct PedalCalibrationData {
  // When the pedal is considered fully unpressed
  int16_t lower_value;
  // When the pedal is considered fully pressed
  int16_t upper_value;
} PedalCalibrationData;

typedef struct PedalCalibBlob {
  PedalCalibrationData throttle_calib;
  PedalCalibrationData brake_calib;
} PedalCalibBlob;

typedef struct PedalCalibrationStorage {
  int16_t min_reading;
  int16_t max_reading;
  volatile uint32_t sample_counter;
} PedalCalibrationStorage;

extern PedalCalibBlob global_calib_blob;

StatusCode pedal_calib_init(PedalCalibrationStorage *storage);

// Re-initializes the MAX11600 to the correct channel
StatusCode pedal_calib_sample(PedalCalibrationStorage *calib_storage, PedalCalibrationData *data, PedalState state, GpioAddress *address);
