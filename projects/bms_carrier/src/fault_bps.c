#include "fault_bps.h"

static BpsStorage *s_bps_storage;

StatusCode fault_bps_init(BpsStorage *storage) {
  s_bps_storage = storage;
  return STATUS_CODE_OK;
}

// TODO: These faulting mechanism will be changing substantially
// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
  s_bps_storage->fault_bitset |= (1 << fault_bitmask);
  LOG_DEBUG("FAULT_BITMASK: %d\n", fault_bitmask);
  if (s_bps_storage->fault_bitset & 0x7) {
    s_bps_storage->fault_bitset |= (1 << 15);
  } else {
    s_bps_storage->fault_bitset |= (1 << 14);
  }
  set_battery_status_fault(fault_bitmask);
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask
StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  s_bps_storage->fault_bitset &= ~(1 << fault_bitmask);
  return STATUS_CODE_OK;
}
