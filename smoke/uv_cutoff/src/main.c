#include <stdio.h>

#include "can.h"
#include "gpio.h"
#include "can_board_ids.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "tasks.h"
#include "uv_cutoff.h"
#include "uv_cutoff_getters.h"
#include "uv_cutoff_setters.h"

// TODO(devAdhiraj): update with actual ports and pins
#define UV_CUTOFF_PORT GPIO_PORT_A
#define UV_CUTOFF_PIN 5

#define HORN_PORT GPIO_PORT_A
#define HORN_PIN 5

#define LIGHTS_PORT GPIO_PORT_A
#define LIGHTS_PIN 5

static const GpioAddress uv_status = { .port = UV_CUTOFF_PORT, .pin = UV_CUTOFF_PIN };

static const GpioAddress horn = { .port = HORN_PORT, .pin = HORN_PIN };

static const GpioAddress lights = { .port = LIGHTS_PORT, .pin = LIGHTS_PIN };

  StatusCode status;
  GpioState state_val;
  int lights_check = 0;

static UVCutoffState state = UV_CUTOFF_ACTIVE;

void uv_smoke_logic() {
  GpioState uv_value;

  if (gpio_get_state(&uv_status, &uv_value) != STATUS_CODE_OK) {
    LOG_CRITICAL("Error reading UV_cutoff pin!\n");
    return;
  }
  if (uv_value == GPIO_STATE_LOW) {
    state = UV_CUTOFF_DISCONNECTED;
  } else { 
    state = UV_CUTOFF_ACTIVE;
  }

  set_uv_cutoff_notification_signal1(state);

  if (state == UV_CUTOFF_DISCONNECTED) {
    return;
  }
  

  if (get_horn_and_lights_horn_state() && !lights_check) {
    status = gpio_set_state(&horn, GPIO_STATE_HIGH);
    // if(status != STATUS_CODE_OK) {
    //   LOG_DEBUG("GPIO Horn State could not be set to high\n");
    // } 
    if (status == STATUS_CODE_OK) {
      gpio_get_state(&horn, &state_val);
      // if( state_val != GPIO_STATE_HIGH) {
      //   LOG_DEBUG("Horn State not set correctly\n")
      // } else if (state_val == GPIO_STATE_HIGH) {
      //   LOG_DEBUG("Horn State set correctly\n");
      // }
    }
  } else {
    status = gpio_set_state(&horn, GPIO_STATE_LOW);
    // if(status != STATUS_CODE_OK) {
    //   LOG_DEBUG("GPIO Horn State could not be set to low\n");
    // }
    if (status == STATUS_CODE_OK) {
      gpio_get_state(&horn, &state_val);
      // if( state_val != GPIO_STATE_LOW) {
      //   LOG_DEBUG("Horn State not set correctly\n")
      // } else if (state_val == GPIO_STATE_LOW) {
      //   LOG_DEBUG("Horn state set correctly\n");
      // }
    }
    
  }

  if (get_horn_and_lights_lights_state() && lights_check) {
    status = gpio_set_state(&lights, GPIO_STATE_HIGH);
    // if(status != STATUS_CODE_OK) {
    //   LOG_DEBUG("GPIO Lights State could not be set to high\n");  
    // }
    if (status == STATUS_CODE_OK) {
      gpio_get_state(&lights, &state_val);
      // if(state_Val != GPIO_STATE_HIGH) {
      //   LOG_DEBUG("Lights State not set correctly\n");
      // } else if (state_val == GPIO_STATE_HIGH) {
      //   LOG_DEBUG("Lights State set correctly\n")
      // }
    }
    
  } else {
    status = gpio_set_state(&lights, GPIO_STATE_LOW);
    // if(status != STATUS_CODE_OK) {
    //   LOG_DEBUG("GPIO Lights State could not be set to low\n");
    // }
    if (status == STATUS_CODE_OK) {
      gpio_get_state(&lights, &state_val);
      // if(state_val != GPIO_STATE_LOW) {
      //   LOG_DEBUG("Lights State not set correctly\n");
      // } else if (state_val == GPIO_STATE_LOW) {
      //   LOG_DEBUG("Lights State set correctly\n");
      // }
    }
  }
}

TASK(smoke_task, TASK_MIN_STACK_SIZE) {
  //
  //TEST 1 - Disconnected UV status
  gpio_set_state(&uv_status,GPIO_STATE_LOW);
  uv_smoke_logic();
  assert( g_tx_struct.uv_cutoff_notification_signal1 == UV_CUTOFF_DISCONNECTED );
  LOG_DEBUG("uv_cutoff notification signal set to 'UV_CUTOFF_DISCONNECTED'\n");
  //TEST 2 - Active UV status
  gpio_set_state(&uv_status,GPIO_STATE_HIGH);
  uv_smoke_logic();
  assert( g_tx_struct.uv_cutoff_notification_signal1 == UV_CUTOFF_ACTIVE );
  LOG_DEBUG("uv_cutoff notification signal set to 'UV_CUTOFF_ACTIVE'\n");
  //TEST 3 - Horn state HIGH
  gpio_set_state(&uv_status,GPIO_STATE_HIGH);
  gpio_set_state(&horn,GPIO_STATE_HIGH);
  lights_check = 0;
  uv_smoke_logic();
  assert(status == STATUS_CODE_OK);
  assert(state_val == GPIO_STATE_HIGH);
  //TEST 4 - Lights state HIGH
  gpio_set_state(&uv_status,GPIO_STATE_HIGH);
  gpio_set_state(&lights,GPIO_STATE_HIGH);
  lights_check = 1;
  uv_smoke_logic();
  assert(status == STATUS_CODE_OK);
  assert(state_val == GPIO_STATE_HIGH);
  //TEST 5 - Horn state LOW
  gpio_set_state(&uv_status,GPIO_STATE_HIGH);
  gpio_set_state(&horn,GPIO_STATE_LOW);
  lights_check = 0;
  uv_smoke_logic();
  assert(status == STATUS_CODE_OK);
  assert(state_val == GPIO_STATE_LOW);
  //TEST 6 - Lights state LOW
  gpio_set_state(&uv_status,GPIO_STATE_HIGH);
  gpio_set_state(&lights,GPIO_STATE_LOw);
  lights_check = 1;
  uv_smoke_logic();
  assert(status == STATUS_CODE_OK);
  assert(state_val == GPIO_STATE_LOW);
}

int main() {
  tasks_init();
  log_init();
  
  gpio_init();

  gpio_init_pin(&uv_status, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);
  gpio_init_pin(&horn, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&lights, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  tasks_init_task(smoke_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  LOG_DEBUG("UV cutoff task!\n");
  return 0;
}
