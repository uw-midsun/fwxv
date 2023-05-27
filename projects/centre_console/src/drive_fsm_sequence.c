#include "drive_fsm_sequence.h"

#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "power_fsm.h"

#define DUMMY_VAL 1  // ask ShiCheng for corret values for precharge status
#define BEGIN_PRECHARGE_SIGNAL 2
#define PRECHARGE_STATE_COMPLETE 2
#define NUMBER_OF_CYCLES_TO_WAIT 10

int cycles_counter = 0;

void prv_do_precharge_input(Fsm *fsm, void *context) {
  /**
   * Receive precharge ack from MCI
   *
   * if received ack
   *  transition to TRANSMIT
   *
   * if fail
   *  transition to NEUTRAL
   *
   */
  prv_set_or_get_error_state();
  LOG_DEBUG("DO_PRECHARGE\n");

  if (get_mc_status_precharge_status() == PRECHARGE_STATE_COMPLETE) {
    fsm_transition(fsm, TRANSMIT);
  } else {
    set_begin_precharge_signal1(BEGIN_PRECHARGE_SIGNAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    fsm_transition(fsm, NEUTRAL);
  }

  // might need to add error handling and return to neutral
  // like if power state is not main anymore

  cycles_counter++;
  if (cycles_counter == NUMBER_OF_CYCLES_TO_WAIT) {
    fsm_transition(fsm, NEUTRAL);  // this is a fail case | going back to neutral
  }
}
void prv_do_precharge_output(void *context) {
  set_begin_precharge_signal1(BEGIN_PRECHARGE_SIGNAL);
  cycles_counter = 0;
  /**
   * Send out precharge request to MCI
   *
   */
}

void prv_transmit_input(Fsm *fsm, void *context) {
  /**
   * Receive state ack from MCI
   *
   * if received ack
   *  transition to DRIVE or REVERSE or NEUTRAL
   *
   * if fail
   *  transition to NEUTRAL
   *
   */
  prv_set_or_get_error_state();
  LOG_DEBUG("TRANSMIT\n");

  // either need to get an ack or just assume MCI gets it
  if (drive_storage.state == DRIVE) {
    fsm_transition(fsm, DRIVE);
  } else if (drive_storage.state == REVERSE) {
    fsm_transition(fsm, REVERSE);
  } else {
    fsm_transition(fsm, NEUTRAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    fsm_transition(fsm, NEUTRAL);
  }
}
void prv_transmit_output(void *context) {
  /**
   * Transmit state (from context) to MCI
   *
   * state can be drive, reverse, or neutral
   */
  LOG_DEBUG("drive_storage->state: %d \n", drive_storage.state);
  set_drive_output_drive_state(drive_storage.state);
}
