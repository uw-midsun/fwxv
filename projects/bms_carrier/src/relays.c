#include "relays.h"

static RelayType relay_toggle = NO_RELAYS;
static SoftTimer relays_timer;

static const GpioAddress pos_relay_en = { .port = GPIO_PORT_B, .pin = 8 };
static const GpioAddress pos_relay_sense = { .port = GPIO_PORT_B, .pin = 5 };

static const GpioAddress neg_relay_en = { .port = GPIO_PORT_B, .pin = 4 };
static const GpioAddress neg_relay_sense = { .port = GPIO_PORT_B, .pin = 3 };

static const GpioAddress solar_relay_en = { .port = GPIO_PORT_C, .pin = 13 };
static const GpioAddress solar_relay_sense = { .port = GPIO_PORT_B, .pin = 9 };

static void prv_close_relays(SoftTimerId id) {
  // 200 MS GAP BETWEEN EACH RELAY BC OF CURRENT DRAW
  switch (relay_toggle) {
    case NO_RELAYS: {
      gpio_set_state(&pos_relay_en, GPIO_STATE_HIGH);
      relay_toggle = POS_RELAY;
      soft_timer_start(&relays_timer);
      break;
    }
    case POS_RELAY: {
      gpio_set_state(&neg_relay_en, GPIO_STATE_HIGH);
      relay_toggle = NEG_RELAY;
      soft_timer_start(&relays_timer);
      break;
    }
    case NEG_RELAY: {
      gpio_set_state(&solar_relay_en, GPIO_STATE_HIGH);
      relay_toggle = SOLAR_RELAY;
      break;
    }
    case SOLAR_RELAY: {
      GpioState sense_state;
      gpio_get_state(&pos_relay_sense, &sense_state);
      gpio_get_state(&neg_relay_sense, &sense_state);
      gpio_get_state(&solar_relay_sense, &sense_state);
    }
    default:
      // FAULT? SHOULD NEVER BE DEFAULT
      break;
  }
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

  soft_timer_init(BMS_CLOSE_RELAYS_DELAY, prv_close_relays, &relays_timer);
  set_battery_relay_info_state(EE_RELAY_STATE_CLOSE);
  soft_timer_start(&relays_timer);
  return STATUS_CODE_OK;
}
