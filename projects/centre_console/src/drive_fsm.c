#include "drive_fsm.h"

#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm_sequence.h"
#include "fsm_shared_mem.h"
#include "power_fsm.h"

FSM(drive, NUM_DRIVE_TRANSITIONS);

#define NUM_DRIVE_FSM_BUTTONS 3

#define PRECHARGE_STATE_COMPLETE 2

DriveStorage drive_storage = { .state = NEUTRAL };

FSMStorage cc_storage = { 0 };

StatusCode error_state = STATUS_CODE_OK;

static uint32_t notification = 0;
static Event drive_fsm_event;

static GpioAddress s_drive_fsm_button_lookup_table[NUM_DRIVE_FSM_BUTTONS] = {
  [NEUTRAL_BUTTON] = FAKE_NEUTRAL_GPIO_ADDR,
  [DRIVE_BUTTON] = FAKE_DRIVE_GPIO_ADDR,
  [REVERSE_BUTTON] = FAKE_REVERSE_GPIO_ADDR,
};

static Event s_drive_fsm_event_lookup_table[NUM_DRIVE_FSM_EVENTS] = {
  [NEUTRAL_BUTTON] = NEUTRAL_BUTTON_EVENT,
  [DRIVE_BUTTON] = DRIVE_BUTTON_EVENT,
  [REVERSE_BUTTON] = REVERSE_BUTTON_EVENT,
};

void prv_set_or_get_error_state() {
  if (error_state != STATUS_CODE_OK) {
    fsm_shared_mem_set_error_code(&cc_storage, error_state);
  } else {
    error_state = fsm_shared_mem_get_error_code(&cc_storage);
  }
}

static void prv_check_error_state(Fsm *fsm) {
  if (error_state != STATUS_CODE_OK) {
    drive_storage.state = NEUTRAL;
    fsm_transition(fsm, TRANSMIT);
  }
}

static void prv_check_power_state(Fsm *fsm, StateId power_state) {
  if (power_state != POWER_FSM_STATE_MAIN) {
    drive_storage.state = NEUTRAL;
    fsm_transition(fsm, TRANSMIT);
  }
}

// Drive state
static void prv_drive_input(Fsm *fsm, void *context) {
  /**
   * If Neutral button pressed OR drive_state != main
   *  transition to TRANSMIT
   *
   */

  prv_set_or_get_error_state();
  prv_check_error_state(fsm);

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  prv_check_power_state(fsm, power_state);

  if (notify_get(&notification) == STATUS_CODE_OK) {
    event_from_notification(&notification, &drive_fsm_event);
    while (drive_fsm_event != INVALID_EVENT) {
      if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
        drive_storage.state = NEUTRAL;
        fsm_transition(fsm, TRANSMIT);
      }
      event_from_notification(&notification, &drive_fsm_event);
    }
  }
}
static void prv_drive_output(void *context) {}

// Reverse state
static void prv_reverse_input(Fsm *fsm, void *context) {
  /**
   * If Neutral button pressed OR drive_state != main
   *  transition to TRANSMIT
   *
   */

  prv_set_or_get_error_state();
  prv_check_error_state(fsm);

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  prv_check_power_state(fsm, power_state);

  if (notify_get(&notification) == STATUS_CODE_OK) {
    event_from_notification(&notification, &drive_fsm_event);
    while (drive_fsm_event != INVALID_EVENT) {
      if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
        drive_storage.state = NEUTRAL;
        fsm_transition(fsm, TRANSMIT);
      }
      event_from_notification(&notification, &drive_fsm_event);
    }
  }
}
static void prv_reverse_output(void *context) {}

static void prv_start_sequence(Fsm *fsm, int precharge_state) {
  if (precharge_state == PRECHARGE_STATE_COMPLETE) {
    fsm_transition(fsm, TRANSMIT);
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

// Neutral state | First state in state machine
static void prv_neutral_input(Fsm *fsm, void *context) {
  /**
   * If Drive button pressed & power state is POWER_MAIN & speed >= 0
   *    transition to GET_PRECHARGE
   * IF Reverse button pressed & power state is POWER_MAIN & speed <= 0
   *   transition to GET_PRECHARGE
   */

  prv_set_or_get_error_state();

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  int precharge_state = get_mc_status_precharge_status();  // needs to be got from MCI

  if (notify_get(&notification) == STATUS_CODE_OK) {
    event_from_notification(&notification, &drive_fsm_event);
    while (drive_fsm_event != INVALID_EVENT) {
      if (drive_fsm_event == DRIVE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN &&
          prv_speed_is_positive()) {
        drive_storage.state = DRIVE;
        prv_start_sequence(fsm, precharge_state);
      } else if (drive_fsm_event == REVERSE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN &&
                 prv_speed_is_negative()) {
        drive_storage.state = REVERSE;
        prv_start_sequence(fsm, precharge_state);
      }
      event_from_notification(&notification, &drive_fsm_event);
    }
  }
}
static void prv_neutral_output(void *context) {}

// Declare states in state list
static FsmState s_drive_state_list[NUM_DRIVE_STATES] = {
  STATE(NEUTRAL, prv_neutral_input, prv_neutral_output),
  STATE(DRIVE, prv_drive_input, prv_drive_output),
  STATE(REVERSE, prv_reverse_input, prv_reverse_output),
  STATE(DO_PRECHARGE, prv_do_precharge_input, prv_do_precharge_output),
  STATE(TRANSMIT, prv_transmit_input, prv_transmit_output)
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_drive_transitions[NUM_DRIVE_TRANSITIONS] = {
  // SEQ: DO_PRECHARGE transitions to TRANSMIT
  TRANSITION(DO_PRECHARGE, TRANSMIT),

  // NEUTRAL -> SEQ
  TRANSITION(NEUTRAL, DO_PRECHARGE),
  TRANSITION(NEUTRAL, TRANSMIT),

  // SEQ -> DRIVE
  TRANSITION(TRANSMIT, DRIVE),

  // SEQ -> REVERSE
  TRANSITION(TRANSMIT, REVERSE),

  // SEQ -> NEUTRAL
  TRANSITION(DO_PRECHARGE, NEUTRAL),
  TRANSITION(TRANSMIT, NEUTRAL),

  // DRIVE -> SEQ
  TRANSITION(DRIVE, TRANSMIT),

  // REVERSE -> SEQ
  TRANSITION(REVERSE, TRANSMIT),
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
    .edge = INTERRUPT_EDGE_RISING,  // not sure if this needs to be rising or falling
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
