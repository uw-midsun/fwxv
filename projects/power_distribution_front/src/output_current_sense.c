#include "output_current_sense.h"

#include "bts_load_switch.h"
#include "status.h"

static const GpioAddress sense_mux_pin = PD_MUX_OUTPUT_PIN;
static const GpioAddress mux_select_pins[NUM_MUX_SEL_PINS] = {
  PD_MUX_SEL1_PIN,
  PD_MUX_SEL2_PIN,
  PD_MUX_SEL3_PIN,
  PD_MUX_SEL4_PIN,
};

StatusCode pd_sense_init(void) {
  // init mux output for adc
  status_ok_or_return(gpio_init_pin(&sense_mux_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(sense_mux_pin));

  // init mux sel pins
  for (uint8_t i = 0; i < NUM_MUX_SEL_PINS; ++i) {
    status_ok_or_return(gpio_init_pin(&mux_select_pins[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_set_sense_mux(uint8_t mux_val) {
  if (mux_val >= NUM_MUX_INPUTS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  for (uint8_t i = 0; i < NUM_MUX_SEL_PINS; ++i) {
    status_ok_or_return(gpio_set_state(&mux_select_pins[i], ((mux_val & (1 << i)) != 0)));
  }
  return STATUS_CODE_OK;
}

StatusCode pd_sense_output_group(OutputGroup group) {
  if (group >= NUM_OUTPUT_GROUPS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (group == OUTPUT_GROUP_ALL) {
    for (uint8_t out = 0; out < NUM_OUTPUTS; out++) {
      status_ok_or_return(prv_set_sense_mux(g_output_config[out].mux_val));
      status_ok_or_return(
          bts_output_get_current(&g_output_config[out], &(g_output_config[out].reading_out)));
    }
  } else {
    // Get specific group, iterate through set of pins
    OutputGroupDef *grp = g_output_group_map[group];
    if (!grp) {
      return STATUS_CODE_UNINITIALIZED;
    }

    for (uint8_t out = 0; out < grp->num_outputs; out++) {
      Output output = grp->outputs[out];
      status_ok_or_return(prv_set_sense_mux(g_output_config[output].mux_val));
      status_ok_or_return(bts_output_get_current(&g_output_config[output], &(g_output_config[output].reading_out)));
    }
  }
  return STATUS_CODE_OK;
}
