#include "relays_fsm.h"

FSM(bms_relays, NUM_RELAY_STATES, TASK_STACK_256);
static RelaysStateId fsm_state = RELAYS_OPEN;
static RelayType relay_toggle = NO_RELAYS;

static BmsStorage *s_storage;
static SoftTimer relays_timer;

static const GpioAddress pos_relay_en = { .port = GPIO_PORT_B, .pin = 8 };
static const GpioAddress pos_relay_sense = { .port = GPIO_PORT_B, .pin = 5 };

static const GpioAddress neg_relay_en = { .port = GPIO_PORT_B, .pin = 4 };
static const GpioAddress neg_relay_sense = { .port = GPIO_PORT_B, .pin = 3 };

static const GpioAddress solar_relay_en = { .port = GPIO_PORT_C, .pin = 13 };
static const GpioAddress solar_relay_sense = { .port = GPIO_PORT_B, .pin = 9 };

static void prv_close_relays(SoftTimerId id) {
  // 150 MS GAP BETWEEN EACH RELAY BC OF CURRENT DRAW
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
    default:
      // FAULT? SHOULD NEVER BE DEFAULT
      break;
  }
}

static void prv_open_relays() {
  gpio_set_state(&pos_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&neg_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&solar_relay_en, GPIO_STATE_LOW);
}

static void prv_bms_fault_ok_or_transition(Fsm *fsm) {
  LOG_DEBUG("FSM RUN CYCLE\n");
  if (s_storage->bps_storage.fault_bitset) {
    LOG_DEBUG("Fault %d\n", s_storage->bps_storage.fault_bitset);
    fsm_transition(fsm, RELAYS_FAULT);
  }
}

static void prv_relays_open_output(void *context) {
  LOG_DEBUG("Transitioned to RELAYS_OPEN\n");
  prv_open_relays();
  set_battery_relay_info_state(EE_RELAY_STATE_OPEN);
  fsm_state = RELAYS_OPEN;
  relay_toggle = NO_RELAYS;
}

static void prv_relays_open_input(Fsm *fsm, void *context) {
  prv_bms_fault_ok_or_transition(fsm);
  RelaysStateId relay_event = get_bms_relays_relays_state();
  if (relay_event == RELAYS_CLOSED) {
    fsm_transition(fsm, RELAYS_CLOSED);
  }
}

static void prv_relays_closed_output(void *context) {
  LOG_DEBUG("Transitioned to RELAYS_CLOSED\n");
  set_battery_relay_info_state(EE_RELAY_STATE_CLOSE);
  fsm_state = RELAYS_CLOSED;
  soft_timer_start(&relays_timer);
}

static void prv_relays_closed_input(Fsm *fsm, void *context) {
  prv_bms_fault_ok_or_transition(fsm);
  GpioState state;
  RelaysStateId relay_event = get_bms_relays_relays_state();
  gpio_get_state(&pos_relay_sense, &state);
  if (relay_event == RELAYS_OPEN) {
    fsm_transition(fsm, RELAYS_OPEN);
  }
}

static void prv_relays_fault_output(void *context) {
  LOG_DEBUG("Transitioned to RELAYS_FAULT\n");
  prv_open_relays();
  set_battery_relay_info_state(EE_RELAY_STATE_FAULT);
  fsm_state = RELAYS_FAULT;
}

static void prv_relays_fault_input(Fsm *fsm, void *context) {
  prv_bms_fault_ok_or_transition(fsm);
}

// Relays FSM declaration for states and transitions
static FsmState s_relays_state_list[NUM_RELAY_STATES] = {
  STATE(RELAYS_OPEN, prv_relays_open_input, prv_relays_open_output),
  STATE(RELAYS_CLOSED, prv_relays_closed_input, prv_relays_closed_output),
  STATE(RELAYS_FAULT, prv_relays_fault_input, prv_relays_fault_output)
};

static bool s_relays_transitions[NUM_RELAY_STATES][NUM_RELAY_STATES] = {
  TRANSITION(RELAYS_OPEN, RELAYS_CLOSED), TRANSITION(RELAYS_OPEN, RELAYS_FAULT),
  TRANSITION(RELAYS_CLOSED, RELAYS_OPEN), TRANSITION(RELAYS_CLOSED, RELAYS_FAULT),
  TRANSITION(RELAYS_FAULT, RELAYS_FAULT),
};

StatusCode init_bms_relays(BmsStorage *bms_storage) {
  s_storage = bms_storage;
  gpio_init_pin(&pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  soft_timer_init(BMS_CLOSE_RELAYS_DELAY, prv_close_relays, &relays_timer);
  fsm_init(bms_relays, s_relays_state_list, s_relays_transitions, RELAYS_OPEN, NULL);
  return STATUS_CODE_OK;
}
