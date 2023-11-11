#include "cc_buttons.h"

// Notifies drive/power task of button press event
StatusCode get_button_press(void) {
  uint8_t pca9555_reg_val = 0;
  // TODO: Migrate this read to PCA9555
  i2c_read_reg(I2C_PORT_1, PCA9555_I2C_ADDR, INPUT1, &pca9555_reg_val, 1);

  if (pca9555_reg_val == PCA9555_REG_DEFAULT) {  // No button pressed
    return STATUS_CODE_OK;

  } else if ((~(pca9555_reg_val)&DRIVE_BTN_MASK) != 0) {
    LOG_DEBUG("DRIVE PRESSED\n");
    notify(drive, DRIVE_BUTTON_EVENT);

  } else if ((~(pca9555_reg_val)&NEUTRAL_BTN_MASK) != 0) {
    LOG_DEBUG("NEUT PRESSED\n");
    notify(drive, NEUTRAL_BUTTON_EVENT);

  } else if ((~(pca9555_reg_val)&REVERSE_BTN_MASK) != 0) {
    LOG_DEBUG("REV PRESSED\n");
    notify(drive, REVERSE_BUTTON_EVENT);

  } else if ((~(pca9555_reg_val)&POWER_BTN_MASK) != 0) {
    LOG_DEBUG("POWER PRESSED\n");
    notify(power, START_BUTTON_EVENT);

  } else {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  return STATUS_CODE_OK;
}
