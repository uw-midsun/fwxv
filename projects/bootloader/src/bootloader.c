#include "can_datagram.h"
#include "can_datagram_prv.h"
#include "log.h"


static BootloaderStateData prv_bootloader = {
    .state = BOOTLOADER_UNINITIALIZED,
    .error = BOOTLOADER_ERROR_NONE
};

BootloaderStates bootloader_get_state(void) { return prv_bootloader.state; }

BootloaderError bootloader_switch_states(const BootloaderStates new_state) { 
    BootloaderError return_err = BOOTLOADER_ERROR_NONE;
    BootloaderStates current_state = prv_bootloader.state;

    switch (current_state)
    {
        case BOOTLOADER_IDLE:
            switch (new_state)
            {
                case BOOTLOADER_JUMP_APP:
                    prv_bootloader.state = BOOTLOADER_JUMP_APP;
                    break;

                case BOOTLOADER_DATA_READY:
                    prv_bootloader.state = BOOTLOADER_DATA_READY;
                    break;

                case BOOTLOADER_FAULT:
                    prv_bootloader.state = BOOTLOADER_FAULT;
                    break;
                
                default:
                    return_err = BOOTLOADER_INVALID_ARGS;
                    prv_bootloader.state = BOOTLOADER_FAULT;
                    break;
            }
            break;
        
        case BOOTLOADER_DATA_READY:
            switch (new_state)
            {
                case BOOTLOADER_DATA_RECIEVE:
                    prv_bootloader.state = BOOTLOADER_DATA_RECIEVE;
                    break;

                case BOOTLOADER_IDLE:
                    prv_bootloader.state = BOOTLOADER_IDLE;
                    break;

                case BOOTLOADER_FAULT:
                    prv_bootloader.state = BOOTLOADER_FAULT;
                    break;
                
                default:
                    prv_bootloader.state = BOOTLOADER_FAULT;
                    return_err = BOOTLOADER_INVALID_ARGS;
                    break;
            }

        case BOOTLOADER_DATA_RECIEVE:
            switch (new_state)
            {
            
            case BOOTLOADER_IDLE:
                prv_bootloader.state = BOOTLOADER_IDLE;
                break;

            case BOOTLOADER_FAULT:
                    prv_bootloader.state = BOOTLOADER_FAULT;
                    break;
            
            default:
                return_err = BOOTLOADER_INVALID_ARGS;
                prv_bootloader.state = BOOTLOADER_FAULT;
                break;
            }

        case BOOTLOADER_JUMP_APP:
            switch (new_state)
            {
            case BOOTLOADER_FAULT:
                prv_bootloader.state = BOOTLOADER_FAULT;
                break;
            
            default:
                return_err = BOOTLOADER_INVALID_ARGS;
                prv_bootloader.state = BOOTLOADER_FAULT;
                break;
            }

        case BOOTLOADER_UNINITIALIZED:
            switch (new_state)
            {
            case BOOTLOADER_IDLE:
                prv_bootloader.state = BOOTLOADER_IDLE;
                break;
            
            default:
                return_err = BOOTLOADER_INVALID_ARGS;
                prv_bootloader.state = BOOTLOADER_FAULT;
                break;
            }
        
        default:
            return_err = BOOTLOADER_INVALID_ARGS;
            prv_bootloader.state = BOOTLOADER_FAULT;
            break;
    }

    return return_err;
}

BootloaderError bootloader_get_err() { return prv_bootloader.error; }