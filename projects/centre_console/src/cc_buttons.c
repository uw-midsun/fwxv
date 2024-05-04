#include "cc_buttons.h"

static Task *cc_notify_task = NULL;
static GpioAddress buzzer = BUZZER;
static SoftTimer s_timer;
static bool beep_flag = true;

// static void prv_buzzer_beep(SoftTimerId id) {
//   if (beep_flag) {
//     pwm_set_dc(PWM_TIMER_4, 100);
//     soft_timer_start(&s_timer);
//     beep_flag = false;
//   } else {
//     pwm_set_dc(PWM_TIMER_4, 0);
//     beep_flag = true;
//   }
// }

// Notifies drive/power task of button press event
StatusCode get_button_press(void) {
  // TODO: Migrate this read to PCA9555
  uint16_t pca9555_reg_val = 0;
  i2c_read_reg(I2C_PORT_1, PCA9555_I2C_ADDR, INPUT0, (uint8_t *)&pca9555_reg_val, 2);

  if (pca9555_reg_val == PCA9555_REG_DEFAULT) {  // No button pressed
    return STATUS_CODE_OK;
  }
  if ((~(pca9555_reg_val)&REGEN_BTN_MASK) != 0) {
    LOG_DEBUG("REGEN PRESSED\n");
    notify(cc_notify_task, REGEN_BUTTON_EVENT);
  }
  if ((~(pca9555_reg_val)&HAZARD_BTN_MASK) != 0) {
    LOG_DEBUG("HAZARD PRESSED\n");
    notify(cc_notify_task, HAZARD_BUTTON_EVENT);
  }
  if ((~(pca9555_reg_val)&POWER_BTN_MASK) != 0) {
    LOG_DEBUG("POWER PRESSED\n");
    notify(cc_notify_task, POWER_BUTTON_EVENT);
  }

  if ((~(pca9555_reg_val)&DRIVE_BTN_MASK) != 0) {
    LOG_DEBUG("DRIVE PRESSED\n");
    notify(drive, DRIVE_BUTTON_EVENT);
  }
  if ((~(pca9555_reg_val)&NEUTRAL_BTN_MASK) != 0) {
    LOG_DEBUG("NEUT PRESSED\n");
    notify(drive, NEUTRAL_BUTTON_EVENT);
  }
  if ((~(pca9555_reg_val)&REVERSE_BTN_MASK) != 0) {
    LOG_DEBUG("REV PRESSED\n");
    notify(drive, REVERSE_BUTTON_EVENT);
  }
  // soft_timer_start(&s_timer);
  return STATUS_CODE_OK;
}

StatusCode init_cc_buttons(Task *task) {
  cc_notify_task = task;
  // gpio_init_pin(&buzzer, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  // pwm_init(PWM_TIMER_1, BUZZER_MICROSECONDS);
  // pwm_set_dc(PWM_TIMER_1, 5);
  // soft_timer_init(BEEP_MS, prv_buzzer_beep, &s_timer);
  return STATUS_CODE_OK;
}
