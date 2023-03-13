#pragma once

#include "can.h"

// Initialize CAN Watchdogs
StatusCode can_watchdogs_init();

// Adds a watchdog for the specified message ID.
StatusCode can_add_watchdog(CanMessageId msg_id, uint16_t duration_ms);

// Tells if CAN Watchdogs are awake
bool can_watchdogs_awake();

StatusCode watchdog_check(CanMessageId msg_id);

StatusCode check_can_watchdogs();
