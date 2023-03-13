#pragma once
#include "tasks.h"
#include "log.h"

void set_master_cycle_time(uint8_t time_ms);
void set_medium_cycle_count(uint8_t cycles);
void set_slow_cycle_count(uint8_t cycles);

void run_fast_cycle();
void run_medium_cycle();
void run_slow_cycle();

StatusCode init_master_task();
Task* get_master_task();
