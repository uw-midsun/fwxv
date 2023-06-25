#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
#include "unity.h"
#include "adc.h"
#include "power_distrbution.h"

void mux_select(uint8_t sel_val) {
  for(uint8_t i = 0; i < SENSE_MUX_SEL; ++i) {
    gpio_set_state(&sense_mux_select_pins[i], (1 << i) & sel_val);
  }
}

void init_pd() {
  for(unsigned int k = 0; k < SENSE_MUX_SEL; ++k) {
    gpio_init_pin(&sense_mux_select_pins[k], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }
  for(unsigned int i = 0; i < NUM_SWITCHES; ++i) {
    switches[i].bias = 0;
    switches[i].resistor = 1200;
    switches[i].min_fault_voltage_mv = 10;
    
    enable_pins[i].pin_type = BTS7XXX_PIN_PCA9555;
    enable_pins[i].pin_pca9555 = &(conf[i].en_pca);
    switches[i].enable_pin = &enable_pins[i];

    switches[i].sense_pin = &sense_pin;

    if (i < NUM_SEL_SWITCHES) {
      select_pins[i].pin_type = BTS7XXX_PIN_PCA9555;
      select_pins[i].pin_pca9555 = &(conf[i].sel_pca);
      switches[i].select_pin = &select_pins[i];
      switches[i].select_state.select_state_pca9555 = conf[i].select;
    } else {
      switches[i].select_pin = NULL;
    }
    bts_switch_init(&switches[i]);
  }
  gpio_init_pin(&sense_pin, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(sense_pin);
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  uint16_t current;
  while (true) {
    for(unsigned int i = 0; i < NUM_SWITCHES; ++i) {

      // Turn pin on, read state and validate if HIGH
      CHECK_EQUAL(STATUS_CODE_OK, bts_output_enable_output(&switches[i]));
      CHECK_EQUAL(STATUS_CODE_OK, bts_output_get_output_enabled(&switches[i]));
      CHECK_EQUAL(GPIO_STATE_HIGH, switches[i].reading_out);
      
      mux_select(conf[i].sense_select_code);
      CHECK_EQUAL(STATUS_CODE_OK, bts_output_get_current(&switches[i], &current));
      LOG_DEBUG("sense current = %u\n", current);
      delay_ms(100);

      // Turn pin off, read state and validate if LOW
      CHECK_EQUAL(STATUS_CODE_OK, bts_output_disable_output(&switches[i]));
      CHECK_EQUAL(STATUS_CODE_OK, bts_output_get_output_enabled(&switches[i]));
      CHECK_EQUAL(GPIO_STATE_LOW, switches[i].reading_out);
      
      delay_ms(100);
    }
  }
}


int main() {
   tasks_init();
   log_init();
   gpio_init();
   adc_init(ADC_MODE_SINGLE);
   i2c_init(0, &i2c_settings);
   pca9555_gpio_init(0, 0); // second param (i2c_address) isn't needed/used by pca init
   init_pd();

  //  bts_switch_init(&pedal_switch);
   LOG_DEBUG("PD Test - %s!\n", PD_BOARD);
   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   LOG_DEBUG("exiting main?");
   return 0;
}

