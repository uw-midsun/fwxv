#include "relays_fsm.h"

FSM(bms_relays, NUM_RELAY_STATES, TASK_STACK_512);
static RelaysStateId fsm_prev_state = RELAYS_OPEN;

static BmsStorage *s_storage;

static const InterruptSettings it_settings = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
  .edge = INTERRUPT_EDGE_FALLING,
};

static const GpioAddress pos_relay_en = { .port = GPIO_PORT_B, .pin = 8 };
static const GpioAddress neg_relay_en = { .port = GPIO_PORT_B, .pin = 4 };
static const GpioAddress solar_relay_en = { .port = GPIO_PORT_C, .pin = 13 };
static const GpioAddress kill_switch_mntr = { .port = GPIO_PORT_A, .pin = 15 };

void close_relays() {
  // 150 MS GAP BETWEEN EACH RELAY BC OF CURRENT DRAW
  gpio_set_state(&pos_relay_en, GPIO_STATE_HIGH);
  delay_ms(150);
  gpio_set_state(&neg_relay_en, GPIO_STATE_HIGH);
  delay_ms(150);
  gpio_set_state(&solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(150);
}

void open_relays() {
  gpio_set_state(&pos_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&neg_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&solar_relay_en, GPIO_STATE_LOW);
}

static void prv_bms_fault_ok_or_transition(Fsm *fsm) {
  LOG_DEBUG("Bms fault check \n");
  StatusCode status = STATUS_CODE_OK;
  uint16_t max_voltage = 0;
  uint16_t min_voltage = 0xFFFF;
  GpioState state;
  gpio_get_state(&kill_switch_mntr, &state);
  if (state == GPIO_STATE_LOW) {
    LOG_DEBUG("KILLSWITCH PRESSED\n");
    set_battery_status_fault(BMS_FAULT_KILLSWITCH);
    set_battery_status_status(1);
    fsm_transition(fsm, RELAYS_FAULT);
  }

  status |= current_sense_fault_check();

  if (status != STATUS_CODE_OK) {
    set_battery_status_fault(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    set_battery_status_status(1);
    fsm_transition(fsm, RELAYS_FAULT);
    return;
  }

  status |= run_current_sense_cycle();
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("status (current sense cycle failed): %d\n", status);
    set_battery_status_fault(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    set_battery_status_status(1);
    fsm_transition(fsm, RELAYS_FAULT);
  }
  delay_ms(1);

  // Handling temp, current, voltage and SOC faults
  if (g_tx_struct.battery_vt_current >= MAX_CURRENT) {
    LOG_DEBUG("OVERCURRENT\n");
    fsm_transition(fsm, RELAYS_FAULT);
    set_battery_status_fault(BMS_FAULT_OVERCURRENT);
    set_battery_status_status(1);
  }
  if (g_tx_struct.battery_vt_temperature >= MAX_AMBIENT_TEMP) {
    LOG_DEBUG("AMBIENT OVER TEMPERATURE\n");
    fsm_transition(fsm, RELAYS_FAULT);
    set_battery_status_fault(BMS_FAULT_OVERTEMP_AMBIENT);
    set_battery_status_status(2);
  }

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("status (fault_check or read_cells failed): %d\n", status);
    fsm_transition(fsm, RELAYS_FAULT);
    set_battery_status_fault(BMS_FAULT_COMMS_LOSS_AFE);
    set_battery_status_status(1);
    return;
  }
}

static void prv_relays_open_output(void *context) {
  LOG_DEBUG("Transitioned to RELAYS_OPEN\n");
  open_relays();
  set_battery_relay_info_state(EE_RELAY_STATE_OPEN);
  fsm_prev_state = RELAYS_OPEN;
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
  close_relays();
  set_battery_relay_info_state(EE_RELAY_STATE_CLOSE);
  fsm_prev_state = RELAYS_CLOSED;
}

static void prv_relays_closed_input(Fsm *fsm, void *context) {
  prv_bms_fault_ok_or_transition(fsm);
  RelaysStateId relay_event = get_bms_relays_relays_state();
  if (relay_event == RELAYS_OPEN) {
    fsm_transition(fsm, RELAYS_OPEN);
  }
}

static void prv_relays_fault_output(void *context) {
  LOG_DEBUG("Transitioned to RELAYS_FAULT\n");
  open_relays();
  set_battery_relay_info_state(EE_RELAY_STATE_FAULT);
  fsm_prev_state = RELAYS_FAULT;
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
  // gpio_it_register_interrupt(&kill_switch_mntr, &it_settings, ALRT_GPIO_IT, current_sense);
  fsm_init(bms_relays, s_relays_state_list, s_relays_transitions, RELAYS_OPEN, NULL);
  return STATUS_CODE_OK;
}
