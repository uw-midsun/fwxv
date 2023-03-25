#include "drive_fsm.h"
#include "drive_fsm_sequence.h"
#include "power_fsm.h"

FSM(drive_fsm, NUM_DRIVE_TRANSITIONS);

#define NUM_DRIVE_FSM_BUTTONS 3

StatusCode error_state = STATUS_CODE_OK; 

static uint32_t notification = 0;
static Event drive_fsm_event;

static GpioAddress s_drive_fsm_button_lookup_table[NUM_DRIVE_FSM_BUTTONS] = {
  [NEUTRAL_BUTTON] = 1,
  [DRIVE_BUTTON] = 2,
  [REVERSE_BUTTON] = 3,
};

static Event s_drive_fsm_event_lookup_table[NUM_DRIVE_FSM_EVENTS] = {
  [NEUTRAL_BUTTON] = NEUTRAL_BUTTON_EVENT,
  [DRIVE_BUTTON] = DRIVE_BUTTON_EVENT,
  [REVERSE_BUTTON] = REVERSE_BUTTON_EVENT,
};

static uint32_t notification = 0;
static Event drive_fsm_event;


// Drive state
static void prv_drive_input(Fsm *fsm, void *context) {
    LOG_DEBUG("DRIVE\n");

    /**
     * If Neutral button pressed OR drive_state != main
     *  transition to TRANSMIT
     * 
     */

    // fsm_transition();
}
static void prv_drive_output(void *context) {
    
}

// Reverse state 
static void prv_reverse_input(Fsm *fsm, void *context) {
    LOG_DEBUG("REVERSE\n");

    /**
     * If Neutral button pressed OR drive_state != main
     *  transition to TRANSMIT
     * 
     */

    // fsm_transition();
}
static void prv_reverse_output(void *context) {
    
}

// Neutral state | First state in state machine
static void prv_neutral_input(Fsm *fsm, void *context) {
    LOG_DEBUG("NEUTRAL\n");
    LOG_DEBUG("counter: %d\n", counter);

    if(error_state != STATUS_CODE_OK) {
        fsm_shared_mem_set_error_code(&cc_storage, error_state);
    } else{
        error_state = fsm_shared_mem_get_error_code(&cc_storage);
    }

    StateId power_state = fsm_shared_mem_get_power_state(&cc_storage); 
    int speed = 0; // needs to be got from MCI 

    // button press probably using notify
    if (notify_get(&notification) == STATUS_CODE_OK) {
        while(event_from_notification(&notification, &drive_fsm_event) == STATUS_CODE_INCOMPLETE) {
            if (drive_fsm_event == DRIVE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN && speed >= 0) { 
                fsm_transition(fsm, GET_PRECHARGE);
            }else if (drive_fsm_event == REVERSE_BUTTON_EVENT && power_state == POWER_FSM_STATE_MAIN && speed <= 0) { 
                fsm_transition(fsm, GET_PRECHARGE);
            }
        }
    }

    // getting power state is tentative 
        // can have a notification var that is static for drive fsm
        // initial value will be inital state of power fsm 
        // power fsm will send drive fsm notification during output function 
        // this should ensure that drive picks it up by next cycle 

    // I think MCI is sending speed to drive fsm
        // this should come in the form of a CAN msg


    /**
     * If Drive button pressed & power state is POWER_MAIN & speed >= 0
     *    transition to GET_PRECHARGE
     * IF Reverse button pressed & power state is POWER_MAIN & speed <= 0
     *   transition to GET_PRECHARGE
     */

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
    InterruptSettings it_settings = {
        .priority = INTERRUPT_PRIORITY_NORMAL,
        .type = INTERRUPT_TYPE_INTERRUPT,
        .edge = INTERRUPT_EDGE_RISING, // not sure if this needs to be rising or falling
    };
    // Add gpio init pins 
    // Add gpio register interrupts


  fsm_init(drive_fsm, settings, NULL);
  return STATUS_CODE_OK;
}