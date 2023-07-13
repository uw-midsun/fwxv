#include "drive_fsm_sequence.h"

#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "power_fsm.h"

int cycles_counter = 0;

void prv_fault_to_neutral(Fsm *fsm) {
  drive_storage.state = NEUTRAL;
  set_drive_output_drive_state(drive_storage.state);
  fsm_transition(fsm, NEUTRAL);
}

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

  if (get_mc_status_precharge_status() == PRECHARGE_STATE_COMPLETE) {
    fsm_transition(fsm, TRANSMIT);
  } else {
    set_begin_precharge_signal1(BEGIN_PRECHARGE_SIGNAL);
  }

  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    prv_fault_to_neutral(fsm);
  }

  cycles_counter++;
  if (cycles_counter == NUMBER_OF_CYCLES_TO_WAIT) {
    // this is a fail case | going back to neutral
    prv_fault_to_neutral(fsm);
  }
}
void prv_do_precharge_output(void *context) {
  /**
   * Send out precharge request to MCI
   *
   */
  set_begin_precharge_signal1(BEGIN_PRECHARGE_SIGNAL);
  cycles_counter = 0;
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

  // either need to get an ack or just assume MCI gets it
  StateId power_state = fsm_shared_mem_get_power_state(&cc_storage);
  if (power_state != POWER_FSM_STATE_MAIN) {
    prv_fault_to_neutral(fsm);
  } else {
    if (drive_storage.state == DRIVE) {
      fsm_transition(fsm, DRIVE);
    } else if (drive_storage.state == REVERSE) {
      fsm_transition(fsm, REVERSE);
    } else {
      fsm_transition(fsm, NEUTRAL);
    }
  }
}
void prv_transmit_output(void *context) {
  /**
   * Transmit state (from context) to MCI
   *
   * state can be drive, reverse, or neutral
   */
  set_drive_output_drive_state(drive_storage.state);
}
