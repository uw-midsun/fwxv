#pragma once
// requires mcp2515
// mcp2515 can be initialized before or after motor controller can

#define CURRENT_SCALE 100
#define VOLTAGE_SCALE 100
#define VELOCITY_SCALE 100
#define TEMP_SCALE 100

// also used as the current for regen braking, might need to be seperated
#define ACCERLATION_FORCE 1
#define CRUISE_THROTTLE_THRESHOLD 0
#define TORQUE_CONTROL_VEL 20000  // unobtainable rpm for current control
#define VEL_TO_RPM_RATIO 0.57147  // TODO: set actual ratio, m/s to motor (rpm for m/s)
// wheel diameter 557mm
// 1000 / (557 * pi) = 0.57147

#define MAX_COASTING_THRESHOLD 0.4     // Max pedal threshold when coasting at speeds > 8 km/h
#define MAX_OPD_SPEED 8                // Max car speed before one pedal driving threshold maxes out
#define CONVERT_VELOCITY_TO_KPH 3.6    // Converts m/s to km/h
#define COASTING_THRESHOLD_SCALE 0.05  // Scaling value to determine coasting threshold

#define DRIVER_CONTROL_BASE 0x500
#define MOTOR_CONTROLLER_BASE_L 0x400
#define MOTOR_CONTROLLER_BASE_R 0x80  // TODO: set to actual values

void init_motor_controller_can();
