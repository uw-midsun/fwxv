#include "fault_bps.h"

#include "bms.h"
#include "bms_carrier_setters.h"
#include "exported_enums.h"

static BmsStorage *s_storage;

StatusCode fault_bps_init(BmsStorage *storage) {
  s_storage = storage;
  return STATUS_CODE_OK;
}

// TODO: These faulting mechanism will be changing substantially
// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
  if (fault_bitmask != EE_BPS_STATE_FAULT_RELAY) {
    set_bps_heartbeat_status(g_tx_struct.bps_heartbeat_status | fault_bitmask);
  } else {
    // Handle opening relays
  }
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask
StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  set_bps_heartbeat_status(g_tx_struct.bps_heartbeat_status & ~(fault_bitmask));
  return STATUS_CODE_OK;
}
