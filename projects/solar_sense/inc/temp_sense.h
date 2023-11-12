#pragma once

#include <stdio.h>

#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "solar_sense_getters.h"
#include "solar_sense_setters.h"
#include "tasks.h"

#define MAX_TEMP 8000
#define MIN_TEMP 0

// Voltages for A0 and A1
#define MAX_VOLTS1 3334
#define MIN_VOLTS1 2570

// Voltages for A2 - A5
#define MAX_VOLTS2 758
#define MIN_VOLTS2 2760

// GPIO pin index
typedef enum {
  TEMP_SENSE0 = 0,
  TEMP_SENSE1,
  TEMP_SENSE2,
  TEMP_SENSE3,
  TEMP_SENSE4,
  TEMP_SENSE5,
  TEMP_SENSE_OVERTEMP,
  TEMP_SENSE_FULLSPEED,
  TEMP_SENSE_FANFAIL,
  NUM_TEMP_SENSE_PINS,
} TempSensePinsIndex;

// GPIO pins
#define TEMP_SENSE_GPIO0 \
  { .port = GPIO_PORT_A, .pin = 0 }
#define TEMP_SENSE_GPIO1 \
  { .port = GPIO_PORT_A, .pin = 1 }
#define TEMP_SENSE_GPIO2 \
  { .port = GPIO_PORT_A, .pin = 2 }
#define TEMP_SENSE_GPIO3 \
  { .port = GPIO_PORT_A, .pin = 3 }
#define TEMP_SENSE_GPIO4 \
  { .port = GPIO_PORT_A, .pin = 4 }
#define TEMP_SENSE_GPIO5 \
  { .port = GPIO_PORT_A, .pin = 5 }
#define TEMP_SENSE_OVERTEMP_GPIO \
  { .port = GPIO_PORT_B, .pin = 5 }
#define TEMP_SENSE_FULLSPEED_GPIO \
  { .port = GPIO_PORT_B, .pin = 6 }
#define TEMP_SENSE_FANFAIL_GPIO \
  { .port = GPIO_PORT_B, .pin = 7 }

// Initializes the temp sense gpio pins and adc channels
StatusCode temp_sense_adc_init();
