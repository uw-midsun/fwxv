#include "fault_bps.h"

static BpsStorage *s_bps_storage;

StatusCode fault_bps_init(BpsStorage *storage) {
  s_bps_storage = storage;
  return STATUS_CODE_OK;
}

// TODO: These faulting mechanism will be changing substantially
// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
  s_bps_storage->fault_bitset |= fault_bitmask;
  set_battery_status_fault(fault_bitmask);
  if (fault_bitmask <= 2) {
    set_battery_status_status(2);
  } else {
    set_battery_status_status(1);
  }
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask
StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  s_bps_storage->fault_bitset &= ~(fault_bitmask);
  return STATUS_CODE_OK;
}
