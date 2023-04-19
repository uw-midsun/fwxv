#include "drive_fsm_sequence.h"
#define DUMMY_VAL 1 // ask ShiCheng for corret values for precharge status
#define NUMBER_OF_CYCLES_TO_WAIT 10

int cycles_counter = 0;

void prv_do_precharge_input(Fsm *fsm, void *context){
    prv_set_or_get_error_state();
    LOG_DEBUG("DO_PRECHARGE\n");

    // talk with Mitchell about acks and MCI

    if(get_mc_status_precharge_status() == DUMMY_VAL){
        fsm_transition(fsm, TRANSMIT);
    } else {
        set_begin_precharge_signal1(DUMMY_VAL);
    } 


    // might need to add error handling and return to neutral 
    // like if power state is not main anymore
    cycles_counter++;
    if(cycles_counter == NUMBER_OF_CYCLES_TO_WAIT){
        fsm_transition(fsm, NEUTRAL); // this is a fail case | going back to neutral
    }

    // need to add fault handling to be able to transition back to neutral

    /**
     * Receive precharge ack from MCI
     * 
     * if received ack
     *  transition to TRANSMIT
     * 
     * if fail
     * transition to NEUTRAL
     * 
     */
}
void prv_do_precharge_output(void *context){
    set_begin_precharge_signal1(DUMMY_VAL);
    cycles_counter = 0;
    /**
     * Send out precharge request to MCI
     * 
     */
}

void prv_transmit_input(Fsm *fsm, void *context){
    prv_set_or_get_error_state();
    LOG_DEBUG("TRANSMIT\n");

    // either need to get an ack or just assume MCI gets it
    if(drive_storage.state == DRIVE){
        fsm_transition(fsm, DRIVE);
    } else if(drive_storage.state == REVERSE){
        fsm_transition(fsm, REVERSE);
    } else {
        fsm_transition(fsm, NEUTRAL);
    }

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
}
void prv_transmit_output(void *context){
    LOG_DEBUG("drive_storage->state: %d \n", drive_storage.state);
    set_drive_output_drive_state(DUMMY_VAL); // ask ShiCheng for correct values
    // ask ShiCheng if he wants only one message or if this should just be repeated over and over in states
    
    /**
     * Transmit state (from context) to MCI
     * 
     * state can be drive, reverse, or neutral
     */
}