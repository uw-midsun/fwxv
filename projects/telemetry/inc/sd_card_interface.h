#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sd_card_spi.h"
#include "status.h"

/**
 * @brief   Link the SD card driver with the FATFs API
 */
StatusCode sd_card_link_driver(SdSpiPort spi, SdSpiSettings *settings);
