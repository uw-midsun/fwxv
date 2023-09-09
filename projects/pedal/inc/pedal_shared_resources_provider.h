#pragma once
#include "max11600.h"
#include "pedal_calib.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2

StatusCode pedal_resources_init(Max11600Storage *storage, PedalCalibBlob *calib_blob);

Max11600Storage *get_shared_max11600_storage();

PedalCalibBlob *get_shared_pedal_calib_blob();
