#pragma once
// requires mcp2515
// mcp2515 can be initialized before or after motor controller can

#include <stdbool.h>

static bool s_send_message;
static float s_target_current;
static float s_target_velocity;

void init_motor_controller_can();
