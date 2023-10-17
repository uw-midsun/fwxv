#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
#include "adc.h"
#include "power_distribution.h"
#include "outputs.h"
#include "output_current_sense.h"
#include "interrupt.h"

static const OutputGroup output_groups_to_test[] = {
  OUTPUT_GROUP_TEST
};

static const GpioAddress test_gpio =   { .port = GPIO_PORT_B, .pin = 5 };

void pd_print_adc_readings(OutputGroup group) {
  if (group == OUTPUT_GROUP_ALL) {
    for (uint8_t out = 0; out < NUM_OUTPUTS; out++) {
      LOG_DEBUG("sense group: %d, value: %d\n", out, g_output_config[out].reading_out);
    }
  } else {
    // Get specific group, iterate through set of pins
    OutputGroupDef *grp = g_output_group_map[group];
    for (uint8_t out = 0; out < grp->num_outputs; out++) {
      Output output = grp->outputs[out];
      LOG_DEBUG("sense group: %d, value: %d\n", output, g_output_config[output].reading_out);
    }
  }
}

TASK(smoke_pd, TASK_STACK_512){
  gpio_init_pin(&test_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  
  pd_output_init();
  pd_sense_init();
  adc_init();
  uint16_t num_test_grps = SIZEOF_ARRAY(output_groups_to_test);

  while(true) {
    for(uint8_t i = 0; i < num_test_grps; i++) {
      uint16_t sense = 0;
      
      gpio_toggle_state(&test_gpio);
      pd_set_output_group(output_groups_to_test[i], OUTPUT_STATE_ON);
      delay_ms(2000);

      pd_sense_output_group(output_groups_to_test[i]);
      pd_print_adc_readings(output_groups_to_test[i]);

      pd_set_output_group(output_groups_to_test[i], OUTPUT_STATE_OFF);
      gpio_toggle_state(&test_gpio);
      delay_ms(2000);
    }
  }
}

int main() {
  tasks_init();
  log_init();
  interrupt_init();
  gpio_init();
  i2c_init(0, &i2c_settings);
  pca9555_gpio_init(I2C_PORT_1, 0); // second param (i2c_address) isn't needed/used by pca init

  tasks_init_task(smoke_pd, TASK_PRIORITY(2), NULL);
  LOG_DEBUG("PD Smoke!\n");

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}

