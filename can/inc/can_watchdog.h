#pragma once

#include "can.h"
#include "log.h"
typedef struct CanWatchDog {
  uint16_t cycles_over;
  uint16_t max_cycles;
} CanWatchDog;

StatusCode check_can_watchdogs();

void clear_rx_received();
