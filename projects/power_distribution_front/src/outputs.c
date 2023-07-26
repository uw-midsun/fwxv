#include "outputs.h"

StatusCode pd_output_init(void) {
  for(uint8_t out = 0; out < NUM_OUTPUTS; out++) {
    // If output is configured, we will init it
    if (g_output_config[out].enable_pin != NULL) {
     status_ok_or_return(bts_switch_init(&g_output_config[out]);
    }
  }
  return STATUS_CODE_OK;
}

StatusCode pd_set_output_group(OutputGroup group, OutputState state) {
  if (group >= NUM_OUTPUT_GROUPS || state >= NUM_OUTPUT_STATES) {
    return STATUS_CODE_INVALID_INPUTS;
  }

  if (group == OUTPUT_GROUP_ALL) {
    for(uint8_t out = 0; out < NUM_OUTPUTS; out++) {
      if (state == OUTPUT_STATE_ON) {
        status_ok_or_return(bts_output_enable_output(&g_output_config[out]));
      } else {
        status_ok_or_return(bts_output_disable_output(&g_output_config[out]));
      }
    }
  } else {
    // Get specific group, iterate through set of outputs
    OutputGroupDef *grp = g_output_group_map[group]; 
    if (!grp) {
      return STATUS_CODE_UNINITIALIZED;
    }

    for(uint8_t out = 0; out < grp->num_outputs; out++) {
       if (state == OUTPUT_STATE_ON) {
        status_ok_or_return(bts_output_enable_output(&grp->outputs[out]));
      } else {
        status_ok_or_return(bts_output_disable_output(&grp->outputs[out]));
      }
    }
  }
    return STATUS_CODE_OK;
}



