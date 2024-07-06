#include "relays.h"

#include "bms.h"

static const GpioAddress pos_relay_en = { .port = GPIO_PORT_B, .pin = 8 };
static const GpioAddress pos_relay_sense = { .port = GPIO_PORT_B, .pin = 5 };

static const GpioAddress neg_relay_en = { .port = GPIO_PORT_B, .pin = 4 };
static const GpioAddress neg_relay_sense = { .port = GPIO_PORT_B, .pin = 3 };

static const GpioAddress solar_relay_en = { .port = GPIO_PORT_C, .pin = 13 };
static const GpioAddress solar_relay_sense = { .port = GPIO_PORT_B, .pin = 9 };

#define NUM_BMS_RELAYS 3
static const GpioAddress s_relays_sense[NUM_BMS_RELAYS] = { pos_relay_sense, neg_relay_sense,
                                                            solar_relay_sense };

static StatusCode prv_close_relays(void) {
  // 200 MS GAP BETWEEN EACH RELAY BC OF CURRENT DRAW
  gpio_set_state(&pos_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
  gpio_set_state(&neg_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
  gpio_set_state(&solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
  GpioState sense_state;
  for (uint8_t i = 0; i < NUM_BMS_RELAYS; i++) {
    gpio_get_state(&s_relays_sense[i], &sense_state);
    if (sense_state != GPIO_STATE_HIGH) {
      LOG_DEBUG("Relay %d not closed\n", i);
      fault_bps_set(BMS_FAULT_RELAY_CLOSE_FAILED);
      bms_relay_fault();
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

void bms_relay_fault() {
  LOG_DEBUG("Transitioned to RELAYS_FAULT\n");
  // OPEN RELAYS
  gpio_set_state(&pos_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&neg_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&solar_relay_en, GPIO_STATE_LOW);
  set_battery_relay_info_state(EE_RELAY_STATE_FAULT);
}

StatusCode init_bms_relays() {
  gpio_init_pin(&pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_init_pin(&pos_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&neg_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&solar_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  status_ok_or_return(prv_close_relays());
  set_battery_relay_info_state(EE_RELAY_STATE_CLOSE);
  return STATUS_CODE_OK;
}
