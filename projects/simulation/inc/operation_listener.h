#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "log.h"

#define x86 1
#define PORT 2520

void sim_init(int sock_num);

typedef enum {
  GPIO_SET = 0,
  GPIO_TOGGLE,
  GPIO_IT_TRIGGER,
  ADC_SET_READING,
  I2C_SET_READING,
  SPI_SET_RX,
  UART,
  NUM_OF_OPERATIONS
} Operations;
