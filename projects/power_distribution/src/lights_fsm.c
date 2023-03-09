#include "lights_fsm.h"

#include "lights_getters.h"

// Softtimer module setup for light blinkers
static SoftTimer s_timer_single;
static EELightType light_id_callback;

static void prv_lights_signal_blinker(SoftTimerId id) {
  if (light_id_callback == EE_LIGHT_TYPE_SIGNAL_LEFT) {
    gpio_toggle_state(&LEFT_LIGHT_ADDR);
    soft_timer_start(600, prv_lights_signal_blinker, &s_timer_single);

  } else if (light_id_callback == EE_LIGHT_TYPE_SIGNAL_RIGHT) {
    gpio_toggle_state(&RIGHT_LIGHT_ADDR);
    soft_timer_start(600, prv_lights_signal_blinker, &s_timer_single);

  } else if (light_id_callback == EE_LIGHT_TYPE_SIGNAL_HAZARD) {
    gpio_toggle_state(&LEFT_LIGHT_ADDR);
    gpio_toggle_state(&RIGHT_LIGHT_ADDR);
    soft_timer_start(600, prv_lights_signal_blinker, &s_timer_single);
  } else {
    // in the initial state light_id_callback == NUM_EE_LIGHT_TYPES
    return;
  }
}

// Lights FSM declaration for states and transitions
FSM(lights, NUM_LIGHTS_STATES);

static FsmState s_PD_lights_list[NUM_LIGHTS_STATES] = {
  STATE(INIT_STATE, prv_init_state_input, prv_init_state_output),
  STATE(LEFT_SIGNAL, prv_left_signal_input, prv_left_signal_output),
  STATE(RIGHT_SIGNAL, prv_right_signal_input, prv_right_signal_output),
  STATE(HAZARD, prv_hazard_input, prv_hazard_output)
};

static FsmTransition s_PD_transition_list[NUM_LIGHTS_TRANSITIONS] = {
  TRANSITION(INIT_STATE, LEFT_SIGNAL),   TRANSITION(INIT_STATE, RIGHT_SIGNAL),
  TRANSITION(INIT_STATE, HAZARD),        TRANSITION(LEFT_SIGNAL, INIT_STATE),
  TRANSITION(LEFT_SIGNAL, HAZARD),       TRANSITION(LEFT_SIGNAL, RIGHT_SIGNAL),
  TRANSITION(RIGHT_SIGNAL, INIT_STATE),  TRANSITION(RIGHT_SIGNAL, HAZARD),
  TRANSITION(RIGHT_SIGNAL, LEFT_SIGNAL), TRANSITION(HAZARD, INIT_STATE)
};

static void prv_init_state_input(Fsm *fsm, void *context) {
  // can transition to LEFT, RIGHT, HAZARD
  EELightType light_event = get_lights_lights_id();
  EELightState light_state = get_lights_state();

  if ((light_event == EE_LIGHT_TYPE_SIGNAL_HAZARD) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, HAZARD);
  } else if ((light_event == EE_LIGHT_TYPE_SIGNAL_LEFT) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, LEFT_SIGNAL);
  } else if ((light_event == EE_LIGHT_TYPE_SIGNAL_RIGHT) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, RIGHT_SIGNAL);
  }
}

static void prv_init_state_output(void *context) {
  LOG_DEBUG("Transitioned to INIT_STATE");
  light_id_callback = NUM_EE_LIGHT_TYPES;
}

static void prv_left_signal_input(Fsm *fsm, void *context) {
  // can transition to INIT, RIGHT, HAZARD
  EELightType light_event = get_lights_lights_id();
  EELightState light_state = get_lights_state();

  if ((light_event == EE_LIGHT_TYPE_SIGNAL_HAZARD) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, HAZARD);
  } else if ((light_event == EE_LIGHT_TYPE_SIGNAL_LEFT) && (light_state == EE_LIGHT_STATE_OFF)) {
    fsm_transition(fsm, INIT_STATE);
  } else if ((light_event == EE_LIGHT_TYPE_SIGNAL_RIGHT) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, RIGHT_SIGNAL);
  }
}

static void prv_left_signal_output(void *context) {
  LOG_DEBUG("Transitioned to LEFT_SIGNAL");
  // Toggle Left Signal blinkers at 100 BPM -> 0.6s
  light_id_callback = EE_LIGHT_TYPE_SIGNAL_LEFT;
  soft_timer_start(600, prv_lights_signal_blinker, &s_timer_single);
}

static void prv_right_signal_input(Fsm *fsm, void *context) {
  // can transition to INIT, LEFT, HAZARD
  EELightType light_event = get_lights_lights_id();
  EELightState light_state = get_lights_state();

  if ((light_event == EE_LIGHT_TYPE_SIGNAL_HAZARD) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, HAZARD);
  } else if ((light_event == EE_LIGHT_TYPE_SIGNAL_RIGHT) && (light_state == EE_LIGHT_STATE_OFF)) {
    fsm_transition(fsm, INIT_STATE);
  } else if ((light_event == EE_LIGHT_TYPE_SIGNAL_LEFT) && (light_state == EE_LIGHT_STATE_ON)) {
    fsm_transition(fsm, LEFT_SIGNAL);
  }
}

static void prv_right_signal_output(void *context) {
  LOG_DEBUG("Transitioned to RIGHT_SIGNAL");
  // Toggle Right Signal blinkers at 100 BPM -> 0.6 s
  light_id_callback = EE_LIGHT_TYPE_SIGNAL_RIGHT;
  soft_timer_start(600, prv_lights_signal_blinker, &s_timer_single);
}

static void prv_hazard_input(Fsm *fsm, void *context) {
  // can transition to INIT, BPS_FAULT
  EELightType light_event = get_lights_lights_id();
  EELightState light_state = get_lights_state();

  if (light_event == (EE_LIGHT_TYPE_SIGNAL_HAZARD) && (light_state == EE_LIGHT_STATE_OFF)) {
    fsm_transition(fsm, INIT_STATE);
  }
}

static void prv_hazard_output(void *context) {
  LOG_DEBUG("Transitioned to HAZARD");
  // Toggle Left and Right Signal blinkers at 100 BPM -> 0.6s
  light_id_callback = EE_LIGHT_TYPE_SIGNAL_HAZARD;
  soft_timer_start(600, prv_lights_signal_blinker, &s_timer_single);
}

StatusCode init_lights(void) {
  const FsmSettings lights_settings = {
    .state_list = s_PD_lights_list,
    .transitions = s_PD_transition_list,
    .num_transitions = NUM_LIGHTS_TRANSITIONS,
    .initial_state = INIT_STATE,
  };
  fsm_init(lights, lights_settings, NULL);

  gpio_init();
  status_ok_or_return(gpio_init_pin(LEFT_LIGHT_ADDR, GPIO_OUTPUT_OPEN_DRAIN, GPIO_STATE_LOW));
  status_ok_or_return(gpio_init_pin(RIGHT_LIGHT_ADDR, GPIO_OUTPUT_OPEN_DRAIN, GPIO_STATE_LOW));

  return STATUS_CODE_OK;
}
