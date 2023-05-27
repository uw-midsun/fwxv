#include "bts7004_load_switch.h"

DECLARE_TASK(periodic_measurement);

StatusCode bts7004_init(Bts7004Storage *storage, Bts7004Settings settings) {
  // copy settings into storage
  storage->settings = settings;
  storage->fault_in_progress = false;

  // initialize enable pin
  status_ok_or_return(
      gpio_init_pin(storage->settings.enable_pin, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));

  // initialize sense pin
  status_ok_or_return(gpio_init_pin(storage->settings.sense_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(*storage->settings.sense_pin));

  // initialize mutex
  status_ok_or_return(mutex_init(storage->data_mutex));

  // initialize periodic measurement task
  return tasks_init_task(periodic_measurement, TASK_PRIORITY(1), storage);
}

StatusCode bts7004_enable_output(Bts7004Storage *storage) {
  if (storage->fault_in_progress) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return gpio_set_state(storage->settings.enable_pin, GPIO_STATE_HIGH);
}

StatusCode bts7004_disable_output(Bts7004Storage *storage) {
  return gpio_set_state(storage->settings.enable_pin, GPIO_STATE_LOW);
}

bool bts7004_get_output_enabled(Bts7004Storage *storage) {
  GpioState input_state;
  gpio_get_state(storage->settings.enable_pin, &input_state);
  return (GPIO_STATE_HIGH == input_state);
}

// Convert voltage measurements to current
static void prv_convert_voltage_to_current(Bts7004Storage *storage, uint16_t *meas) {
  if (*meas <= BTS7004_MAX_LEAKAGE_VOLTAGE_MV) {
    *meas = 0;
  } else {
    // using 32 bits to avoid overflow, and signed ints to get around C's janky type system
    uint32_t meas32 = (uint32_t)*meas;
    meas32 *= BTS7004_IS_SCALING_NOMINAL;
    meas32 /= storage->settings.resistor;
    int32_t unbiased_meas32 = (int32_t)meas32 - storage->settings.bias;
    *meas = (uint16_t)MAX(unbiased_meas32, 0);
  }
}

StatusCode bts7004_get_measurement(Bts7004Storage *storage, uint16_t *meas) {
  mutex_lock(storage->data_mutex, BLOCK_INDEFINITELY);
  status_ok_or_return(adc_read_converted(*storage->settings.sense_pin, meas));

  if (*meas >= storage->settings.min_fault_voltage_mv) {
    // Handle fault, return either the error from the fault handling
    // or STATUS_CODE_INTERNAL_ERROR if the fault pin process works OK
    if (!storage->fault_in_progress) {
      storage->fault_in_progress = true;
      if (bts7004_get_output_enabled(storage)) {
        status_ok_or_return(bts7004_disable_output(storage));
        vTaskDelay(BTS7004_FAULT_RESTART_DELAY_MS / portTICK_PERIOD_MS);
        status_ok_or_return(bts7004_enable_output(storage));
      } else {
        vTaskDelay(BTS7004_FAULT_RESTART_DELAY_MS / portTICK_PERIOD_MS);
      }
      storage->fault_in_progress = false;
    }

    // notify task that fault has occured
    if (storage->settings.event_handler != NULL) {
      notify(storage->settings.event_handler, storage->settings.fault_event);
    }
    return STATUS_CODE_INTERNAL_ERROR;
  }

  prv_convert_voltage_to_current(storage, meas);
  mutex_unlock(storage->data_mutex);
  return STATUS_CODE_OK;
}

void bts7040_start(Bts7004Storage *storage) {
  vTaskResume(periodic_measurement->handle);
}

void bts7040_stop(Bts7004Storage *storage) {
  vTaskSuspend(periodic_measurement->handle);
}

TASK(periodic_measurement, TASK_STACK_256) {
  Bts7004Storage *storage = (Bts7004Storage *)context;
  vTaskSuspend(NULL);
  while (1) {
    bts7004_get_measurement(storage, &storage->reading_out);
    notify(storage->settings.event_handler, storage->settings.data_event);
    vTaskDelay(storage->settings.interval_ms / portTICK_PERIOD_MS);
  }
}
