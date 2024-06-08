#include "sensor_fusion.h"

static MahonyStorage *s_storage;

void mahony_update(uint16_t gx, uint16_t gy, uint16_t gz, uint16_t ax, uint16_t ay, uint16_t az,
                   uint16_t mx, uint16_t my, uint16_t mz, uint32_t time) {
  time = time - s_storage->last_time;

  // Compute feedback only if accelerometer measurement valid
  // (avoids dividing by 0 in accelerometer normalisation)
  if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
    

  }
}

StatusCode mahony_init(MahonyStorage *storage) {
  s_storage = storage;
  return STATUS_CODE_OK;
}