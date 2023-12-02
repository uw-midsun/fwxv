#define BPS_FAULT_INDICATOR_PERIOD_MS 375

static SoftTimer s_bps_indicator_timer;
static bool s_bps_fault_enabled;
static OutputState s_output_state;

static void prv_callback() {
  if (s_bps_fault_enabled) {
    // ternary statement: sets s_output_state to opposite of current state
    s_output_state = (s_output_state == OUPUT_STATE_OFF) ? OUTPUT_STATE_ON : OUTPUT_STATE_OFF;  
    // set OUTPUT_GROUP_BPS_FAULT to s_output_state
		pd_set_output_group(OUTPUT_GROUP_BPS_FAULT, s_output_state);
		// add s_bps_indicator_timer - 375 ms before running, run prv_callback after
    soft_timer_start(BPS_FAULT_INDICATOR_PERIOD_MS, prv_callback, &s_bps_indicator_timer);
  } else {
		// set OUTPUT_GROUP_BPS_FAULT to OUPUT_STATE_OFF
		// function terminates
    pd_set_output_group(OUTPUT_GROUP_BPS_FAULT, OUPUT_STATE_OFF);
  }
} 

static void start_bps_fault_indicator() {
  s_bps_fault_enabled = true;
  soft_timer_start(BPS_FAULT_INDICATOR_PERIOD_MS, prv_callback, &s_bps_indicator_timer);
}

static void stop_bps_fault_indicator() {
  s_bps_fault_enabled = false;
	soft_timer_cancel(&s_bps_indicator_timer);
}