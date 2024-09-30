#include "fault_bps.h"

#include "relays.h"

static uint16_t *fault_bitset;

StatusCode fault_bps_init(BmsStorage *storage) {
  fault_bitset = &storage->fault_bitset;
  return STATUS_CODE_OK;
}

// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
  // NOTE(Mitch): adding log statements in this function may cause hardfault
  bms_relay_fault();
  *fault_bitset |= (1 << fault_bitmask);
  if (*fault_bitset & 0x7) {
    *fault_bitset |= (1 << 15);
  } else {
    *fault_bitset |= (1 << 14);
  }
  set_battery_status_fault(*fault_bitset);
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask
StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  *fault_bitset &= ~(1 << fault_bitmask);
  return STATUS_CODE_OK;
}

uint8_t fault_bps_get(void) {
  return *fault_bitset;
}
