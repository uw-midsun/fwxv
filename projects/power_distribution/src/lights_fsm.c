#include "lights_fsm.h"

#include "log.h"
#include "outputs.h"
#include "power_distribution_getters.h"

static void prv_brake_lights() {
  if (get_cc_pedal_brake_output())
    pd_set_output_group(OUTPUT_GROUP_BRAKE, OUTPUT_STATE_ON);
  else
    pd_set_output_group(OUTPUT_GROUP_BRAKE, OUTPUT_STATE_OFF);
}

// Placeholder GPIO Address, will be updated
GpioAddress RIGHT_LIGHT_ADDR = { .port = GPIO_PORT_B, .pin = 5 };
GpioAddress LEFT_LIGHT_ADDR = { .port = GPIO_PORT_A, .pin = 15 };

// Softtimer module setup for light blinkers
static SoftTimer s_timer_single;
static EELightType light_id_callback;

FSM(lights, NUM_LIGHTS_STATES, TASK_STACK_512);
static OutputState left_signal_state = OUTPUT_STATE_OFF;
static OutputState right_signal_state = OUTPUT_STATE_OFF;
static LightsStateId fsm_prev_state = INIT_STATE;

static void prv_lights_signal_blinker(SoftTimerId id) {
  switch (light_id_callback) {
    case EE_LIGHT_TYPE_SIGNAL_LEFT:
      left_signal_state ^= 1;
      pd_set_output_group(OUTPUT_GROUP_LIGHTS_LEFT_TURN, left_signal_state);
      if (right_signal_state == OUTPUT_STATE_ON) {
        right_signal_state = OUTPUT_STATE_OFF;
        pd_set_output_group(OUTPUT_GROUP_LIGHTS_RIGHT_TURN, right_signal_state);
      }
      soft_timer_start(&s_timer_single);
      break;
    case EE_LIGHT_TYPE_SIGNAL_RIGHT:
      right_signal_state ^= 1;
      pd_set_output_group(OUTPUT_GROUP_LIGHTS_RIGHT_TURN, right_signal_state);
      if (left_signal_state == OUTPUT_STATE_ON) {
        left_signal_state = OUTPUT_STATE_OFF;
        pd_set_output_group(OUTPUT_GROUP_LIGHTS_LEFT_TURN, left_signal_state);
      }
      soft_timer_start(&s_timer_single);
      break;
    case EE_LIGHT_TYPE_SIGNAL_HAZARD:
      if (left_signal_state != right_signal_state) {
        left_signal_state = OUTPUT_STATE_OFF;
        right_signal_state = OUTPUT_STATE_OFF;
      }
      left_signal_state ^= 1;
      right_signal_state ^= 1;
      pd_set_output_group(OUTPUT_GROUP_LIGHTS_HAZARD, left_signal_state);
      soft_timer_start(&s_timer_single);
      break;
    default:
      left_signal_state = OUTPUT_STATE_OFF;
      right_signal_state = OUTPUT_STATE_OFF;
      pd_set_output_group(OUTPUT_GROUP_LIGHTS_HAZARD, left_signal_state);
  }
}

static void prv_init_state_input(Fsm *fsm, void *context) {
  prv_brake_lights();
  // can transition to LEFT, RIGHT, HAZARD
  EELightType light_event = get_cc_steering_input_lights();
  HazardStatus hazard_status = get_cc_info_hazard_enabled();

  if (hazard_status == HAZARD_ON) {
    fsm_transition(fsm, HAZARD);
  } else if (light_event == EE_LIGHT_TYPE_SIGNAL_LEFT) {
    fsm_transition(fsm, LEFT_SIGNAL);
  } else if (light_event == EE_LIGHT_TYPE_SIGNAL_RIGHT) {
    fsm_transition(fsm, RIGHT_SIGNAL);
  }
}

static void prv_init_state_output(void *context) {
  LOG_DEBUG("Transitioned to INIT_STATE\n");
  light_id_callback = NUM_EE_LIGHT_TYPES;
  fsm_prev_state = INIT_STATE;
}

static void prv_left_signal_input(Fsm *fsm, void *context) {
  prv_brake_lights();
  // can transition to INIT, RIGHT, HAZARD
  EELightType light_event = get_cc_steering_input_lights();
  HazardStatus hazard_status = get_cc_info_hazard_enabled();

  if (hazard_status == HAZARD_ON) {
    fsm_transition(fsm, HAZARD);
  } else if (light_event == EE_LIGHT_TYPE_OFF) {
    fsm_transition(fsm, INIT_STATE);
  } else if (light_event == EE_LIGHT_TYPE_SIGNAL_RIGHT) {
    fsm_transition(fsm, RIGHT_SIGNAL);
  }
}

static void prv_left_signal_output(void *context) {
  LOG_DEBUG("Transitioned to LEFT_SIGNAL\n");
  // Toggle Left Signal blinkers at 100 BPM -> 0.6s
  light_id_callback = EE_LIGHT_TYPE_SIGNAL_LEFT;
  if (fsm_prev_state == INIT_STATE) {
    soft_timer_start(&s_timer_single);
  }
  fsm_prev_state = LEFT_SIGNAL;
}

static void prv_right_signal_input(Fsm *fsm, void *context) {
  prv_brake_lights();
  // can transition to INIT, LEFT, HAZARD
  EELightType light_event = get_cc_steering_input_lights();
  HazardStatus hazard_status = get_cc_info_hazard_enabled();

  if (hazard_status == HAZARD_ON) {
    fsm_transition(fsm, HAZARD);
  } else if (light_event == EE_LIGHT_TYPE_OFF) {
    fsm_transition(fsm, INIT_STATE);
  } else if (light_event == EE_LIGHT_TYPE_SIGNAL_LEFT) {
    fsm_transition(fsm, LEFT_SIGNAL);
  }
}

static void prv_right_signal_output(void *context) {
  LOG_DEBUG("Transitioned to RIGHT_SIGNAL\n");
  // Toggle Right Signal blinkers at 100 BPM -> 0.6 s
  light_id_callback = EE_LIGHT_TYPE_SIGNAL_RIGHT;
  if (fsm_prev_state == INIT_STATE) {
    soft_timer_start(&s_timer_single);
  }
  fsm_prev_state = RIGHT_SIGNAL;
}

static void prv_hazard_input(Fsm *fsm, void *context) {
  prv_brake_lights();
  // can transition to INIT, BPS_FAULT
  EELightType light_event = get_cc_steering_input_lights();
  HazardStatus hazard_status = get_cc_info_hazard_enabled();

  if (hazard_status == HAZARD_ON) {
    return;
  }
  if (light_event == EE_LIGHT_TYPE_SIGNAL_LEFT) {
    fsm_transition(fsm, LEFT_SIGNAL);
  } else if (light_event == EE_LIGHT_TYPE_SIGNAL_RIGHT) {
    fsm_transition(fsm, RIGHT_SIGNAL);
  } else {
    fsm_transition(fsm, INIT_STATE);
  }
}

static void prv_hazard_output(void *context) {
  LOG_DEBUG("Transitioned to HAZARD\n");
  // Toggle Left and Right Signal blinkers at 100 BPM -> 0.6s
  light_id_callback = EE_LIGHT_TYPE_SIGNAL_HAZARD;
  if (fsm_prev_state == INIT_STATE) {
    soft_timer_start(&s_timer_single);
  }
  fsm_prev_state = HAZARD;
}

// Lights FSM declaration for states and transitions
static FsmState s_PD_lights_list[NUM_LIGHTS_STATES] = {
  STATE(INIT_STATE, prv_init_state_input, prv_init_state_output),
  STATE(LEFT_SIGNAL, prv_left_signal_input, prv_left_signal_output),
  STATE(RIGHT_SIGNAL, prv_right_signal_input, prv_right_signal_output),
  STATE(HAZARD, prv_hazard_input, prv_hazard_output)
};

static bool s_PD_transition_list[NUM_LIGHTS_STATES][NUM_LIGHTS_STATES] = {
  TRANSITION(INIT_STATE, LEFT_SIGNAL),   TRANSITION(INIT_STATE, RIGHT_SIGNAL),
  TRANSITION(INIT_STATE, HAZARD),        TRANSITION(LEFT_SIGNAL, INIT_STATE),
  TRANSITION(LEFT_SIGNAL, HAZARD),       TRANSITION(LEFT_SIGNAL, RIGHT_SIGNAL),
  TRANSITION(RIGHT_SIGNAL, INIT_STATE),  TRANSITION(RIGHT_SIGNAL, HAZARD),
  TRANSITION(RIGHT_SIGNAL, LEFT_SIGNAL), TRANSITION(HAZARD, INIT_STATE),
  TRANSITION(HAZARD, LEFT_SIGNAL),       TRANSITION(HAZARD, RIGHT_SIGNAL)
};

StatusCode init_lights(void) {
  soft_timer_init(SIGNAL_BLINK_PERIOD_MS, prv_lights_signal_blinker, &s_timer_single);
  fsm_init(lights, s_PD_lights_list, s_PD_transition_list, INIT_STATE, NULL);
  return STATUS_CODE_OK;
}
