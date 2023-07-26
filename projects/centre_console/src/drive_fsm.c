#include "drive_fsm.h"

#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "fsm_shared_mem.h"
#include "power_fsm.h"

FSM(drive, NUM_DRIVE_TRANSITIONS);

#define NUM_DRIVE_FSM_BUTTONS 3

StateId next_state = NEUTRAL;

FSMStorage cc_storage = { 0 };

static uint32_t notification = 0;
static Event drive_fsm_event;

static uint8_t cycles_counter = 0;

static GpioAddress s_drive_fsm_button_lookup_table[NUM_DRIVE_FSM_BUTTONS] = {
  [NEUTRAL_BUTTON] = NEUTRAL_GPIO_ADDR,
  [DRIVE_BUTTON] = DRIVE_GPIO_ADDR,
  [REVERSE_BUTTON] = REVERSE_GPIO_ADDR,
};

static Event s_drive_fsm_event_lookup_table[NUM_DRIVE_FSM_EVENTS] = {
  [NEUTRAL_BUTTON] = NEUTRAL_BUTTON_EVENT,
  [DRIVE_BUTTON] = DRIVE_BUTTON_EVENT,
  [REVERSE_BUTTON] = REVERSE_BUTTON_EVENT,
};

static void prv_start_sequence(Fsm *fsm, int precharge_state) {
  if (precharge_state == PRECHARGE_STATE_COMPLETE) {
    fsm_transition(fsm, next_state);
  } else {
    fsm_transition(fsm, DO_PRECHARGE);
  }
}

static bool prv_speed_is_positive(void) {
  // Speed is in cm/s
  // Need to use type cast int16_t since motor values are uint16 for CAN
  int left_wheel_speed = (int16_t)get_motor_velocity_velocity_l();   // needs to be got from MCI
  int right_wheel_speed = (int16_t)get_motor_velocity_velocity_r();  // needs to be got from MCI

  // Tolerance of 135cm/s which is about 3mph (about 5km/h)
  return (left_wheel_speed >= -135 && right_wheel_speed >= -135) ? true : false;
}

static bool prv_speed_is_negative(void) {
  // Speed is in cm/s
  // Need to use type cast int16_t since motor values are uint16 for CAN
  int left_wheel_speed = (int16_t)get_motor_velocity_velocity_l();   // needs to be got from MCI
  int right_wheel_speed = (int16_t)get_motor_velocity_velocity_r();  // needs to be got from MCI

  // Tolerance of 135cm/s which is about 3mph (about 5km/h)
  return (left_wheel_speed <= 135 && right_wheel_speed <= 135) ? true : false;
}

/**
 * Neutral state | First state in state machine
 * @return Transitions to DO_PRECHARGE, DRIVE, OR REVERSE
 */
static void prv_neutral_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code(&cc_storage);

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  int precharge_state = get_mc_status_precharge_status();  // needs to be got from MCI

  if (notify_get(&notification) == STATUS_CODE_OK && power_error_state == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (drive_fsm_event == DRIVE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN &&
          prv_speed_is_positive()) {
        next_state = DRIVE;
        prv_start_sequence(fsm, precharge_state);
      } else if (drive_fsm_event == REVERSE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN &&
                 prv_speed_is_negative()) {
        next_state = REVERSE;
        prv_start_sequence(fsm, precharge_state);
      }
    }
  }
}
static void prv_neutral_output(void *context) {
  set_drive_output_drive_state(NEUTRAL);
}

/**
 * Do Precharge state
 * @return Transitions to DRIVE OR REVERSE
 */
void prv_do_precharge_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code(&cc_storage);
  if (power_error_state != STATUS_CODE_OK) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  if (get_mc_status_precharge_status() == PRECHARGE_STATE_COMPLETE) {
    fsm_transition(fsm, next_state);
  } else {
    set_begin_precharge_signal1(BEGIN_PRECHARGE_SIGNAL);
  }

  cycles_counter++;
  if (cycles_counter == NUMBER_OF_CYCLES_TO_WAIT) {
    // this is a fail case | going back to neutral
    fsm_shared_mem_set_drive_error_code(&cc_storage, STATUS_CODE_TIMEOUT);
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }
}
void prv_do_precharge_output(void *context) {
  set_begin_precharge_signal1(BEGIN_PRECHARGE_SIGNAL);
  cycles_counter = 0;
}

/**
 * Drive state
 * @return Transitions to NEUTRAL
 */
static void prv_drive_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code(&cc_storage);
  if (power_error_state != STATUS_CODE_OK) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
        next_state = NEUTRAL;
        fsm_transition(fsm, NEUTRAL);
      }
    }
  }
}
static void prv_drive_output(void *context) {
  set_drive_output_drive_state(DRIVE);
  fsm_shared_mem_set_drive_error_code(&cc_storage, STATUS_CODE_OK);
}

/**
 * Reverse state
 * @return Transitions to NEUTRAL
 */
static void prv_reverse_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code(&cc_storage);
  if (power_error_state != STATUS_CODE_OK) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
        next_state = NEUTRAL;
        fsm_transition(fsm, NEUTRAL);
      }
    }
  }
}
static void prv_reverse_output(void *context) {
  set_drive_output_drive_state(REVERSE);
  fsm_shared_mem_set_drive_error_code(&cc_storage, STATUS_CODE_OK);
}

// Declare states in state list
static FsmState s_drive_state_list[NUM_DRIVE_STATES] = {
  STATE(NEUTRAL, prv_neutral_input, prv_neutral_output),
  STATE(DRIVE, prv_drive_input, prv_drive_output),
  STATE(REVERSE, prv_reverse_input, prv_reverse_output),
  STATE(DO_PRECHARGE, prv_do_precharge_input, prv_do_precharge_output),
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_drive_transitions[NUM_DRIVE_TRANSITIONS] = {
  // NEUTRAL -> SEQ
  TRANSITION(NEUTRAL, DO_PRECHARGE),

  // SEQ -> DRIVE
  TRANSITION(DO_PRECHARGE, DRIVE),

  // SEQ -> REVERSE
  TRANSITION(DO_PRECHARGE, REVERSE),

  // SEQ -> NEUTRAL (for failure case)
  TRANSITION(DO_PRECHARGE, NEUTRAL),

  // NEUTRAL -> DRIVE
  TRANSITION(NEUTRAL, DRIVE),

  // NEUTRAL -> REVERSE
  TRANSITION(NEUTRAL, REVERSE),

  // DRIVE -> NEUTRAL
  TRANSITION(DRIVE, NEUTRAL),

  // REVERSE -> NEUTRAL
  TRANSITION(REVERSE, NEUTRAL),
};

StatusCode init_drive_fsm(void) {
  fsm_shared_mem_init(&cc_storage);
  FsmSettings settings = {
    .state_list = s_drive_state_list,
    .transitions = s_drive_transitions,
    .num_transitions = NUM_DRIVE_TRANSITIONS,
    .initial_state = NEUTRAL,
  };
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_FALLING,
  };

  // Add gpio init pins
  // Add gpio register interrupts
  for (int i = 0; i < NUM_DRIVE_FSM_BUTTONS; i++) {
    status_ok_or_return(
        gpio_init_pin(&s_drive_fsm_button_lookup_table[i], GPIO_INPUT_PULL_UP, GPIO_STATE_LOW));
    gpio_it_register_interrupt(&s_drive_fsm_button_lookup_table[i], &it_settings,
                               s_drive_fsm_event_lookup_table[i], drive);
  }

  fsm_init(drive, settings, NULL);
  return STATUS_CODE_OK;
}
