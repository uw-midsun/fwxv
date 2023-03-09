#include "drive_fsm_sequence.h"

void prv_get_precharge_input(Fsm *fsm, void *context){
    LOG_DEBUG("GET_PRECHARGE\n");
    /**
     * Receive precharge state request from MCI
     * 
     * if state is down
     *    transition to DO_PRECHARGE
     * else
     *  transition to TRANSMIT
     * 
     * if fail
     *  transition to NEUTRAL
     * 
     */
}
void prv_get_precharge_output(void *context){
    /**
     * Send out precharge state request to MCI
     * 
     */
}

void prv_do_precharge_input(Fsm *fsm, void *context){
    LOG_DEBUG("DO_PRECHARGE\n");
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
    /**
     * Send out precharge request to MCI
     * 
     */
}

void prv_transmit_input(Fsm *fsm, void *context){
    LOG_DEBUG("TRANSMIT\n");
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
    /**
     * Transmit state (from context) to MCI
     * 
     * state can be drive, reverse, or neutral
     */
}