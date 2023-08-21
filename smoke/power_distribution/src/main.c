#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
#include "adc.h"
#include "power_distrbution.h"
#include "outputs.h"

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
    if (!grp) {
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

TASK(master_task, TASK_MIN_STACK_SIZE) {
  while (true) {
    for(uint8_t i = 1; i < NUM_OUTPUT_GROUPS; ++i) {
      // Turn pin on, read state and validate if HIGH
      CHECK_EQUAL(STATUS_CODE_OK, pd_set_output_group(i, OUTPUT_STATE_ON));
      CHECK_EQUAL(STATUS_CODE_OK, prv_check_pd_output_group(i, OUTPUT_STATE_ON));
      
      // LOG_DEBUG("sense current = %u\n", &switches[i].reading_out);
      delay_ms(100);

      // Turn pin off, read state and validate if LOW
      CHECK_EQUAL(STATUS_CODE_OK, pd_set_output_group(i, OUTPUT_STATE_OFF));
      CHECK_EQUAL(STATUS_CODE_OK, prv_check_pd_output_group(i, OUTPUT_STATE_OFF));
      
      delay_ms(100);
    }
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
   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   LOG_DEBUG("exiting main?");
   return 0;
}

