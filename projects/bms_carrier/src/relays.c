#include "relays.h"

#include "bms.h"

static RelayType relay_toggle = NO_RELAYS;
static SoftTimer relays_timer;

static const GpioAddress pos_relay_en = { .port = GPIO_PORT_B, .pin = 8 };
// TODO(mitch) below address does not match schematic but is set when pos relay is on
static const GpioAddress pos_relay_sense = { .port = GPIO_PORT_B, .pin = 7 };

static const GpioAddress neg_relay_en = { .port = GPIO_PORT_B, .pin = 4 };
static const GpioAddress neg_relay_sense = { .port = GPIO_PORT_B, .pin = 3 };

static const GpioAddress solar_relay_en = { .port = GPIO_PORT_C, .pin = 13 };
static const GpioAddress solar_relay_sense = { .port = GPIO_PORT_B, .pin = 9 };

#define NUM_BMS_RELAYS 3
static const GpioAddress s_relays_sense[NUM_BMS_RELAYS] = { pos_relay_sense, neg_relay_sense,
                                                            solar_relay_sense };

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
      soft_timer_start(&relays_timer);
      break;
    }
    case SOLAR_RELAY: {
      GpioState sense_state;
      gpio_get_state(&pos_relay_sense, &sense_state);
      gpio_get_state(&neg_relay_sense, &sense_state);
      gpio_get_state(&solar_relay_sense, &sense_state);
      relay_toggle = RELAY_CHECK;
      soft_timer_start(&relays_timer);
      break;
    }
    case RELAY_CHECK: {
      GpioState sense_state;
      for (uint8_t i = 0; i < NUM_BMS_RELAYS; i++) {
        gpio_get_state(&s_relays_sense[i], &sense_state);
        if (sense_state != GPIO_STATE_HIGH) {
          fault_bps_set(BMS_FAULT_RELAY_CLOSE_FAILED);
        }
      }
      break;
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

  soft_timer_init(BMS_CLOSE_RELAYS_DELAY_MS, prv_close_relays, &relays_timer);
  set_battery_relay_info_state(EE_RELAY_STATE_CLOSE);
  soft_timer_start(&relays_timer);
  return STATUS_CODE_OK;
}
