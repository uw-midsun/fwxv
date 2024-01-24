#include "precharge_control.h"

#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "motor_controller_getters.h"
#include "motor_controller_setters.h"
#include "status.h"

static GpioAddress s_precharge_control;

StatusCode prv_set_precharge_control(const GpioState state) {
  gpio_set_state(&s_precharge_control, state);
  return STATUS_CODE_OK;
}

StatusCode precharge_control_init(const PrechargeControlSettings *settings) {
  s_precharge_control = settings->precharge_control;

  // gpio_init_pin(&settings->precharge_control, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->precharge_monitor, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  // gpio_init_pin(&settings->precharge_monitor2, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  InterruptSettings monitor_it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .edge = INTERRUPT_EDGE_RISING,
  };
  return gpio_it_register_interrupt(&settings->precharge_monitor, &monitor_it_settings,
                                    PRECHARGE_EVENT, get_master_task());
}
