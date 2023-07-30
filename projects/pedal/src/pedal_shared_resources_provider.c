#include "pedal_shared_resources_provider.h"

#include "log.h"
#include "max11600.h"
#include "pedal_calib.h"
#include "string.h"

static Max11600Storage *s_max11600_storage;
static PedalCalibBlob *s_pedal_calib_blob;

StatusCode pedal_resources_init(Max11600Storage *storage, PedalCalibBlob *calib_blob) {
  // Set pointers to static variables that will be provided to all pedal files
  s_max11600_storage = storage;
  s_pedal_calib_blob = calib_blob;

  return STATUS_CODE_OK;
}

Max11600Storage *get_shared_max11600_storage() {
  return s_max11600_storage;
}

PedalCalibBlob *get_shared_pedal_calib_blob() {
  return s_pedal_calib_blob;
}
