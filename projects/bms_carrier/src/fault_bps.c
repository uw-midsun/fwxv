#include "fault_bps.h"

static BpsStorage *s_bps_storage;

StatusCode fault_bps_init(BpsStorage *storage) {
  s_bps_storage = storage;
  return STATUS_CODE_OK;
}

// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
  // NOTE(Mitch): adding log statements in this function may cause hardfault
  bms_relay_fault();
  s_bps_storage->fault_bitset |= (1 << fault_bitmask);
  if (s_bps_storage->fault_bitset & 0x7) {
    s_bps_storage->fault_bitset |= (1 << 15);
  } else {
    s_bps_storage->fault_bitset |= (1 << 14);
  }
  set_battery_status_fault(s_bps_storage->fault_bitset);
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask
StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  s_bps_storage->fault_bitset &= ~(1 << fault_bitmask);
  return STATUS_CODE_OK;
}

uint8_t fault_bps_get(void) {
  return s_bps_storage->fault_bitset;
}
