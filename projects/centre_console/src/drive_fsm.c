#include "drive_fsm.h"

#include "cc_buttons.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"

FSM(drive, NUM_DRIVE_STATES, TASK_STACK_512);

#define NUM_DRIVE_FSM_BUTTONS 3

static uint32_t notification = 0;
static CentreConsoleDriveState drive_context = EE_DRIVE_OUTPUT_NEUTRAL_STATE;
static Event drive_fsm_event;

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

static bool prv_speed_is_zero(void) {
  // Speed is in cm/s
  // Need to use type cast int16_t since motor values are uint16 for CAN
  int left_wheel_speed = (int16_t)get_motor_velocity_velocity_l();   // needs to be got from MCI
  int right_wheel_speed = (int16_t)get_motor_velocity_velocity_r();  // needs to be got from MCI

  // No Tolerance
  return (left_wheel_speed == 0 && right_wheel_speed == 0) ? true : false;
}

/**
 * Neutral state | First state in state machine
 * @return Transitions to DO_PRECHARGE, DRIVE, OR REVERSE
 */
static void prv_neutral_input(Fsm *fsm, void *context) {
  StatusCode power_error_state = get_pd_status_fault_bitset();
  PowerDistributionPowerState power_state = get_pd_status_power_state();

  if (power_error_state != STATUS_CODE_OK) {
    return;
  }

  // CHECK POWER STATE
  if (power_state == EE_POWER_OFF_STATE) {
    set_cc_info_drive_state(NEUTRAL);
  }
  if (power_state == EE_POWER_DRIVE_STATE) {
    if (drive_context == EE_DRIVE_OUTPUT_DRIVE_STATE)
      fsm_transition(fsm, DRIVE);
    else if (drive_context == EE_DRIVE_OUTPUT_REVERSE_STATE)
      fsm_transition(fsm, REVERSE);
  }

  if (notify_get(&notification) == STATUS_CODE_OK && power_error_state == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      bool can_transition = !check_pd_status_msg_watchdog() && prv_speed_is_zero();
      if (drive_fsm_event == DRIVE_BUTTON_EVENT && can_transition) {
        set_cc_info_drive_state(DRIVE);
        drive_context = EE_DRIVE_OUTPUT_DRIVE_STATE;
        break;
      } else if (drive_fsm_event == REVERSE_BUTTON_EVENT && can_transition) {
        set_cc_info_drive_state(REVERSE);
        drive_context = EE_DRIVE_OUTPUT_REVERSE_STATE;
        break;
      }
    }
  }
}

static void prv_neutral_output(void *context) {
  set_cc_info_drive_state(NEUTRAL);
  drive_context = EE_DRIVE_OUTPUT_NEUTRAL_STATE;
  pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_HIGH);
}

/**
 * Drive state
 * @return Transitions to NEUTRAL
 */
static void prv_drive_input(Fsm *fsm, void *context) {
  if (check_pd_status_msg_watchdog()) {
    // Did not receive power state for 3 cycles
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  StatusCode power_error_state = get_pd_status_fault_bitset();
  if (power_error_state != STATUS_CODE_OK) {
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = get_pd_status_power_state();
  if (power_state != EE_POWER_DRIVE_STATE) {
    pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (prv_speed_is_zero()) {
        if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
          pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
          fsm_transition(fsm, NEUTRAL);
        }
        if (drive_fsm_event == REVERSE_BUTTON_EVENT) {
          set_cc_info_drive_state(REVERSE);
          fsm_transition(fsm, REVERSE);
        }
      }
    }
  }
}

static void prv_drive_output(void *context) {
  pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_LOW);
  pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
  pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_HIGH);
}

/**
 * Reverse state
 * @return Transitions to NEUTRAL
 */
static void prv_reverse_input(Fsm *fsm, void *context) {
  if (check_pd_status_msg_watchdog()) {
    // Did not receive power state for 3 cycles
    pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  StatusCode power_error_state = get_pd_status_fault_bitset();
  if (power_error_state != STATUS_CODE_OK) {
    pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = get_pd_status_power_state();
  if (power_state != EE_POWER_DRIVE_STATE) {
    pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
    fsm_transition(fsm, NEUTRAL);
  }

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
      if (prv_speed_is_zero()) {
        if (drive_fsm_event == NEUTRAL_BUTTON_EVENT) {
          pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_LOW);
          fsm_transition(fsm, NEUTRAL);
        }
        if (drive_fsm_event == DRIVE_BUTTON_EVENT) {
          set_cc_info_drive_state(DRIVE);
          fsm_transition(fsm, DRIVE);
        }
      }
    }
  }
}
static void prv_reverse_output(void *context) {
  pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_LOW);
  pca9555_gpio_set_state(&s_drive_btn_leds[DRIVE_LED], PCA9555_GPIO_STATE_LOW);
  pca9555_gpio_set_state(&s_drive_btn_leds[REVERSE_LED], PCA9555_GPIO_STATE_HIGH);
}

// Declare states in state list
static FsmState s_drive_state_list[NUM_DRIVE_STATES] = {
  STATE(NEUTRAL, prv_neutral_input, prv_neutral_output),
  STATE(DRIVE, prv_drive_input, prv_drive_output),
  STATE(REVERSE, prv_reverse_input, prv_reverse_output),
};

// Declares transition for state machine, must match those in input functions
static bool s_drive_transitions[NUM_DRIVE_STATES][NUM_DRIVE_STATES] = {
  // NEUTRAL -> DRIVE
  TRANSITION(NEUTRAL, DRIVE),

  // NEUTRAL -> REVERSE
  TRANSITION(NEUTRAL, REVERSE),

  // DRIVE -> NEUTRAL
  TRANSITION(DRIVE, NEUTRAL),

  // DRIVE -> REVERSE
  TRANSITION(DRIVE, REVERSE),

  // REVERSE -> NEUTRAL
  TRANSITION(REVERSE, NEUTRAL),

  // REVERSE -> DRIVE
  TRANSITION(REVERSE, DRIVE)
};

StateId get_drive_state(void) {
  return drive_fsm->curr_state;
}

StatusCode init_drive_fsm(void) {
  // Add gpio init pins
  // Add gpio register interrupts
  Pca9555GpioSettings pca_settings = { .direction = PCA9555_GPIO_DIR_OUT,
                                       .state = PCA9555_GPIO_STATE_LOW };
  for (int i = 0; i < NUM_DRIVE_FSM_BUTTONS; i++) {
    status_ok_or_return(pca9555_gpio_init_pin(&s_drive_btn_leds[i], &pca_settings));
  }
  pca9555_gpio_set_state(&s_drive_btn_leds[NEUTRAL_LED], PCA9555_GPIO_STATE_HIGH);
  fsm_init(drive, s_drive_state_list, s_drive_transitions, NEUTRAL, NULL);
  return STATUS_CODE_OK;
}
