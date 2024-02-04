#pragma once
// requires mcp2515
// mcp2515 can be initialized before or after motor controller can

#define CURRENT_SCALE 100
#define VOLTAGE_SCALE 100
#define VELOCITY_SCALE 100
#define TEMP_SCALE 100

#define ACCERLATION_FORCE 1
#define CRUISE_THROTTLE_THRESHOLD 0
#define TORQUE_CONTROL_VEL 20000        // unobtainable rpm for current control
#define VEL_TO_RPM_RATIO 1.0            // TODO: set actual ratio, m/s to motor rpm

#define MAX_COASTING_THRESHOLD 0.4      // Max pedal threshold when coasting at speeds > 8 km/h
#define MAX_OPD_SPEED 8                 // Max car speed before one pedal driving threshold maxes out
#define COASTING_THERSHOLD_SCALE 0.2    // Scaling value to determine coasting threshold 

#define DRIVER_CONTROL_BASE 0x1
#define MOTOR_CONTROLLER_BASE_L 0x40
#define MOTOR_CONTROLLER_BASE_R 0x80

void init_motor_controller_can();
