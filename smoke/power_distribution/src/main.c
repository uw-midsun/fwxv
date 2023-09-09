#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
#include "adc.h"
#include "power_distribution.h"
#include "outputs.h"

static uint8_t out_state = OUTPUT_STATE_ON;
static uint8_t out_grp = 1;

static StatusCode prv_check_pd_output_group(OutputGroup group, OutputState state) {
  if (group >= NUM_OUTPUT_GROUPS || state >= NUM_OUTPUT_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t pin_state;
  if (group == OUTPUT_GROUP_ALL) {
    for (uint8_t out = 0; out < NUM_OUTPUTS; out++) {
      status_ok_or_return(bts_output_get_output_enabled(&g_output_config[out], &pin_state));
      if (pin_state != state) {
        return STATUS_CODE_INTERNAL_ERROR;
      }
    }
  } else {
    // Get specific group, iterate through set of outputs
    OutputGroupDef *grp = g_output_group_map[group];
    if (grp == NULL) {
      return STATUS_CODE_UNINITIALIZED;
    }

    for (uint8_t out = 0; out < grp->num_outputs; out++) {
      Output output = grp->outputs[out];
      status_ok_or_return(bts_output_get_output_enabled(&g_output_config[out], &pin_state));
      if(pin_state != state) {
        return STATUS_CODE_INTERNAL_ERROR;
      }
    }
  }
  return STATUS_CODE_OK;
}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {
  CHECK_EQUAL(STATUS_CODE_OK, pd_set_output_group(out_grp, out_state));
  CHECK_EQUAL(STATUS_CODE_OK, prv_check_pd_output_group(out_grp, out_state));
  if(out_state == OUTPUT_STATE_OFF) {
    out_state = OUTPUT_STATE_ON;
    out_grp++;
    if(out_grp == NUM_OUTPUT_GROUPS) {
      out_grp = 1;
    }
  } else {
    out_state = OUTPUT_STATE_OFF;
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  adc_init();
  i2c_init(0, &i2c_settings);
  pca9555_gpio_init(0, 0); // second param (i2c_address) isn't needed/used by pca init
  pd_output_init();
  // TODO(devAdhiraj): add pd sense init and print out pd sense values in main task

  LOG_DEBUG("PD Smoke!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}

