#pragma once

#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "adc.h"
#include "i2c.h"

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
  UART
} Operations;
