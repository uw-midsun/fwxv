#pragma once
// requires mcp2515
// mcp2515 can be initialized before or after motor controller can

#include <stdbool.h>

static int s_missed_message;
static float s_target_current;
static float s_target_velocity;

#define CURRENT_SCALE 100
#define VOLTAGE_SCALE 100
#define VELOCITY_SCALE 100
#define TEMP_SCALE 100

void init_motor_controller_can();
