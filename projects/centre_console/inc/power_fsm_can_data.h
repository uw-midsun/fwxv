#pragma once

// Aux battery status bits
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/3149398021/FWXV+Power+Select+Design
#define AUX_STATUS_BITS 0x04
#define AUX_FAULT_BITS 0xE0

// PD messages (Todo)
#define PD_FAULT 0x00

// BMS send power notification (Todo)
#define SET_BMS_POWER_NOTIFY 0x01

// BPS hearbeat success message (Todo)
#define BPS_HEARTBEAT 0x00

// Relay setters (Todo)
#define SET_CLOSE_RELAY_STATE_MASK 0x01
#define SET_CLOSE_RELAY_STATE_STATE 0x01

// Relay closed status (Todo)
#define CLOSE_HV_STATUS 0x01
#define CLOSE_GND_STATUS 0x01

// DCDC status bits
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/3149398021/FWXV+Power+Select+Design
#define DCDC_STATUS_BITS 0x02
#define DCDC_FAULT_BITS 0x1C

// PD send turn on everything notification (Todo)
#define SET_TURN_ON_EVERYTHING_NOTIFICATION 0x00

// MCI send ready to drive (Todo)
#define SET_READY_TO_DRIVE 0x01

// MCI set discharge precharge (Todo)
#define SET_DISCHARGE_PRECHARGE 0x01

// MCI precharge completed
#define PRECHARGE_COMPLETED_NOTIFCIATION 0x01

// Relay setters (Todo)
#define SET_OPEN_RELAY_STATE_MASK 0x00
#define SET_OPEN_RELAY_STATE_STATE 0x00

// Relay open status (Todo)
#define OPEN_HV_STATUS 0x01
#define OPEN_GND_STATUS 0x01
