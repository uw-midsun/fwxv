#ifndef BOOTLOADER_PRIVATE_H
#define BOOTLOADER_PRIVATE_H
#include "can_datagram.h"

typedef struct {

    BootloaderStates state;
    BootloaderError error;

} BootloaderStateData;

#endif