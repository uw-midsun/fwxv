#pragma once

// ADT4747A Configuration and control commands
// Datasheet: https://www.onsemi.com/pub/Collateral/ADT7476A-D.PDF

#define ADT7476A_CONFIG_REGISTER_1 \
  0x40  // used for STRT bit
        // default 0x04
#define ADT7476A_CONFIG_REGISTER_3 \
  0x78  // used for pwm2 as SMBALERT
        // default 0x00

#define ADT7476A_INTERRUPT_STATUS_REGISTER_1 0x41
#define ADT7476A_INTERRUPT_STATUS_REGISTER_2 0x42

// bits 7:5 control mode - 111 for manual
#define ADT7476A_FAN_MODE_REGISTER_1 0x5C
#define ADT7476A_FAN_MODE_REGISTER_3 0x5E

#define ADT7476A_INTERRUPT_MASK_REGISTER_1 0x74
#define ADT7476A_INTERRUPT_MASK_REGISTER_2 0x75

// default 0xFF, recommend using 1 and 3 as PWM outputs so that pwm2 can be
// used for SMBALERT
#define ADT7476A_PWM_1 0x30
#define ADT7476A_PWM_3 0x32

#define ADT7476A_TACH_1_LOW 0x28   // default 0xFF
#define ADT7476A_TACH_1_HIGH 0x29  // default 0xFF
#define ADT7476A_TACH_2_LOW 0x2A   // default 0xFF
#define ADT7476A_TACH_2_HIGH 0x2B  // default 0xFF
#define ADT7476A_TACH_3_LOW 0x2C   // default 0xFF
#define ADT7476A_TACH_3_HIGH 0x2D  // default 0xFF
#define ADT7476A_TACH_4_LOW 0x2E   // default 0xFF
#define ADT7476A_TACH_4_HIGH 0x2F  // default 0xFF

#define ADT7476A_MANUAL_MODE_MASK 0b11100010   // Sets manual mode for pwm outputs
#define ADT7476A_CONFIG_REG_1_MASK 0b00000001  // Sets STRT bit
#define ADT7476A_CONFIG_REG_3_MASK 0b00000001  // Configures pwm2 as SMBALERT

#define ADT7476A_REG_SIZE 1

// Interrupt status register bit definitions
// ISR 1 overtemp/overvoltage bits -
// Indicates that Input voltage high or low limit exceeded
#define VCC_EXCEEDED 0x04
#define VCCP_EXCEEDED 0x02

// ISR 2 fan status bits - will be set if fans drop below threshold speed
// LSB of u16 fan data
#define FAN1_STATUS 0x04
#define FAN2_STATUS 0x08
#define FAN3_STATUS 0x10
#define FAN4_STATUS 0x20
