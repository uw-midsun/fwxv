#include "drive_fsm.h"
#include "drive_fsm_sequence.h"

FSM(drive_fsm, NUM_DRIVE_TRANSITIONS);

// Drive state
static void prv_drive_input(Fsm *fsm, void *context) {
    // fsm_transition();
}
static void prv_drive_output(void *context) {
    
}

// Reverse state 
static void prv_reverse_input(Fsm *fsm, void *context) {
    // fsm_transition();
}
static void prv_reverse_output(void *context) {
    
}

// Neutral state | First state in state machine
static void prv_neutral_input(Fsm *fsm, void *context) {
    LOG_DEBUG("NEUTRAL\n");

    /**
     * If Drive button pressed & power state is POWER_MAIN & speed >= 0
     *    transition to GET_PRECHARGE
     * IF Reverse button pressed & power state is POWER_MAIN & speed <= 0
     *   transition to GET_PRECHARGE
     */

    // fsm_transition();
}
static void prv_neutral_output(void *context) {
    
}

// Declare states in state list
static FsmState s_drive_state_list[NUM_DRIVE_STATES] = {
    STATE(NEUTRAL, prv_neutral_input, prv_neutral_output),
    STATE(DRIVE, prv_drive_input, prv_drive_output),
    STATE(REVERSE, prv_reverse_input, prv_reverse_output),
    STATE(GET_PRECHARGE, prv_get_precharge_input, prv_get_precharge_output),
    STATE(DO_PRECHARGE, prv_do_precharge_input, prv_do_precharge_output),
    STATE(TRANSMIT, prv_transmit_input, prv_transmit_output)
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_drive_transitions[NUM_DRIVE_TRANSITIONS] = {
    // SEQ: GET_PRECHARGE transitions to DO_PRECHARGE or TRANSMIT
    //      DO_PRECHARGE transitions to TRANSMIT
    TRANSITION(GET_PRECHARGE, DO_PRECHARGE),
    TRANSITION(GET_PRECHARGE, TRANSMIT),
    TRANSITION(DO_PRECHARGE, TRANSMIT),

    // NEUTRAL -> SEQ
    TRANSITION(NEUTRAL, GET_PRECHARGE),

    // SEQ -> DRIVE
    TRANSITION(TRANSMIT, DRIVE),

    // SEQ -> REVERSE 
    TRANSITION(TRANSMIT, REVERSE),

    // SEQ -> NEUTRAL
    TRANSITION(GET_PRECHARGE, NEUTRAL),
    TRANSITION(DO_PRECHARGE, NEUTRAL),
    TRANSITION(TRANSMIT, NEUTRAL),

    // DRIVE -> SEQ 
    TRANSITION(DRIVE, TRANSMIT),

    // REVERSE -> SEQ 
    TRANSITION(REVERSE, TRANSMIT), 
};


StatusCode init_drive_fsm(void) {
  FsmSettings settings = {
    .state_list = s_drive_state_list,
    .transitions = s_drive_transitions,
    .num_transitions = NUM_DRIVE_TRANSITIONS,
    .initial_state = NEUTRAL,
  };
  fsm_init(drive_fsm, settings, NULL);
  return STATUS_CODE_OK;
}