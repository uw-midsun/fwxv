#include "outputs.h"

#include "bts_load_switch.h"
#include "status.h"

StatusCode pd_output_init(void) {
  for (uint8_t out = 0; out < NUM_OUTPUTS; out++) {
    // If output is configured, we will init it
    if (g_output_config[out].enable_pin != NULL) {
      status_ok_or_return(bts_output_init(&g_output_config[out]));
    }
  }
  return STATUS_CODE_OK;
}

StatusCode pd_set_output_group(OutputGroup group, OutputState state) {
  if (group >= NUM_OUTPUT_GROUPS || state >= NUM_OUTPUT_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (group == OUTPUT_GROUP_ALL) {
    for (uint8_t out = 0; out < NUM_OUTPUTS; out++) {
      if (state == OUTPUT_STATE_ON) {
        status_ok_or_return(bts_output_enable_output(&g_output_config[out]));
      } else {
        status_ok_or_return(bts_output_disable_output(&g_output_config[out]));
      }
    }
  } else {
    // Get specific group, iterate through set of outputs
    OutputGroupDef *grp = g_output_group_map[group];
    if (grp == NULL) {
      return STATUS_CODE_UNINITIALIZED;
    }

    for (uint8_t out = 0; out < grp->num_outputs; out++) {
      uint8_t output = grp->outputs[out];
      if (state == OUTPUT_STATE_ON) {
        status_ok_or_return(bts_output_enable_output(&g_output_config[output]));
      } else {
        status_ok_or_return(bts_output_disable_output(&g_output_config[output]));
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode pd_set_active_output_group(OutputGroup group) {
  if (group >= NUM_OUTPUT_GROUPS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (group == OUTPUT_GROUP_ALL) {
    return pd_set_output_group(OUTPUT_GROUP_ALL, OUTPUT_STATE_ON);
  }

  OutputGroupDef *grp = g_output_group_map[group];
  if (grp == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  for (OutputPowerFsm output = 0; output < NUM_POWER_FSM_OUTPUTS; output++) {
    bool found = false;
    for (OutputGroup i = 0; i < grp->num_outputs; i++) {
      if (grp->outputs[i] == output) {
        status_ok_or_return(bts_output_enable_output(&g_output_config[output]));
        found = true;
        break;
      }
    }
    if (!found) {
      status_ok_or_return(bts_output_disable_output(&g_output_config[output]));
    }
  }
  return STATUS_CODE_OK;
}
