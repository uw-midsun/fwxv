#pragma once

#include "gpio.h"
#include "i2c.h"
#include "notify.h"
#include "tasks.h"

typedef struct {
  GpioAddress smbalert_pin;
  Event event;
  Task *task;
  I2CPort i2c;
  I2CAddress i2c_read_addr;
  I2CAddress i2c_write_addr;
} Adt7476aSettings;

typedef struct {
  GpioAddress smbalert_pin;
  Event event;
  Task *task;
  I2CPort i2c;
} Adt7476aStorage;

// 3 possible pwm outputs, typically only use 2 to control 4 fans

typedef enum { ADT_PWM_PORT_1, ADT_PWM_PORT_2, ADT_PWM_PORT_3, NUM_ADT_PWM_PORTS } AdtPwmPort;

// Initialize the Adt7476a with the given settings; the select pin is an
// STM32 GPIO pin.
StatusCode adt7476a_init(Adt7476aStorage *storage, Adt7476aSettings *settings);

// Translate and write the new speed
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed_percent, AdtPwmPort pwm_port,
                              uint8_t adt7476a_i2c_addr);

// Reads the interrupt status registers
StatusCode adt7476a_get_status(I2CPort port, uint8_t adt7476a_i2c_addr, uint8_t *register_1_data,
                               uint8_t *register_2_data);
