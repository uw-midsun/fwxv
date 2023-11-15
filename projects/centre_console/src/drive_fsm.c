#include "drive_fsm.h"

#include "cc_buttons.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "fsm_shared_mem.h"
#include "power_fsm.h"

FSM(drive, NUM_DRIVE_STATES);

#define NUM_DRIVE_FSM_BUTTONS 3
#define NUM_DRIVE_FSM_EVENTS 3

StateId next_state = NEUTRAL;

static uint32_t notification = 0;
static Event drive_fsm_event;

static uint8_t cycles_counter = 0;

typedef enum DriveLeds {
  DRIVE_LED = 0,
  NEUTRAL_LED,
  REVERSE_LED,
} DriveLeds;

static Pca9555GpioAddress s_drive_btn_leds[NUM_DRIVE_FSM_BUTTONS] = {
  [DRIVE_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_3 },
  [NEUTRAL_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_1 },
  [REVERSE_LED] = { .i2c_address = 0x20, .pin = PCA9555_PIN_IO0_0 },
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
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code();

  StateId power_state = fsm_shared_mem_get_power_state();
  int precharge_state = get_mc_status_precharge_status();  // needs to be got from MCI

  if (notify_get(&notification) == STATUS_CODE_OK && power_error_state == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (drive_fsm_event == DRIVE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN &&
          prv_speed_is_positive()) {
        next_state = DRIVE;
        pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_LOW);
        prv_start_sequence(fsm, precharge_state);
      } else if (drive_fsm_event == REVERSE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN &&
                 prv_speed_is_negative()) {
        next_state = REVERSE;
        pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_LOW);
        prv_start_sequence(fsm, precharge_state);
      }
    }
  }
}
static void prv_neutral_output(void *context) {
  pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_HIGH);
  set_drive_output_drive_state(NEUTRAL);
}

/**
 * Do Precharge state
 * @return Transitions to DRIVE OR REVERSE
 */
void prv_do_precharge_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code();
  if (power_error_state != STATUS_CODE_OK) {
    next_state = NEUTRAL;
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state();
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
    fsm_shared_mem_set_drive_error_code(STATUS_CODE_TIMEOUT);
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
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code();
  if (power_error_state != STATUS_CODE_OK) {
    next_state = NEUTRAL;
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state();
  if (power_state != POWER_FSM_STATE_MAIN) {
    next_state = NEUTRAL;
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
        next_state = NEUTRAL;
        pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
        fsm_transition(fsm, NEUTRAL);
      }
    }
  }
}

static void prv_drive_output(void *context) {
  pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_HIGH);
  set_drive_output_drive_state(DRIVE);
  fsm_shared_mem_set_drive_error_code(STATUS_CODE_OK);
}

/**
 * Reverse state
 * @return Transitions to NEUTRAL
 */
static void prv_reverse_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = fsm_shared_mem_get_power_error_code();
  if (power_error_state != STATUS_CODE_OK) {
    next_state = NEUTRAL;
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state();
  if (power_state != POWER_FSM_STATE_MAIN) {
    next_state = NEUTRAL;
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
        next_state = NEUTRAL;
        pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
        fsm_transition(fsm, NEUTRAL);
      }
    }
  }
}
static void prv_reverse_output(void *context) {
  pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_HIGH);
  set_drive_output_drive_state(REVERSE);
  fsm_shared_mem_set_drive_error_code(STATUS_CODE_OK);
}

// Declare states in state list
static FsmState s_drive_state_list[NUM_DRIVE_STATES] = {
  STATE(NEUTRAL, prv_neutral_input, prv_neutral_output),
  STATE(DRIVE, prv_drive_input, prv_drive_output),
  STATE(REVERSE, prv_reverse_input, prv_reverse_output),
  STATE(DO_PRECHARGE, prv_do_precharge_input, prv_do_precharge_output),
};

// Declares transition for state machine, must match those in input functions
static bool s_drive_transitions[NUM_DRIVE_STATES][NUM_DRIVE_STATES] = {
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
  // Add gpio init pins
  // Add gpio register interrupts
  Pca9555GpioSettings pca_settings = { .direction = PCA9555_GPIO_DIR_OUT,
                                       .state = PCA9555_GPIO_STATE_LOW };
  pca9555_gpio_init(I2C_PORT_1);
  for (int i = 0; i < NUM_DRIVE_FSM_BUTTONS; i++) {
    status_ok_or_return(pca9555_gpio_init_pin(&s_drive_btn_leds[i], &pca_settings));
  }

  pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_HIGH);
  fsm_init(drive, s_drive_state_list, s_drive_transitions, NEUTRAL, NULL);
  return STATUS_CODE_OK;
}
