#include "gpio_cli.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"
#include "log.h"
#include "string.h"

const char gpio_help[] =
    "CLI Reference for GPIO Pins. Usage: \n\r"
    "gpio <set/init_pin/toggle/get> <parameters> \n\r\n"

    "Initializes GPIO globally by setting all pins to their default state. \n\r"
    "ONLY CALL ONCE or it will deinit all current settings. Change \n\r"
    "setting by calling gpio_init_pin. \n\r\n"

    "Type \"help\" after any action for detailed reference.\n\r";

const char gpio_set_help[] =
    "Usage: Set the pin state by address.\n\r"
    "gpio set <address> <state> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r\n"

    "State: \"high\" or \"low\"\n\r";

const char gpio_init_pin_help[] =
    "Usage: Initializes a GPIO pin by address. GPIOs are configured \n\r"
    "to a specified mode, at the max refresh speed \n\r"
    "The init_state only matters if the pin is configured as an output \n\r"
    "gpio init_pin <address> <pin_mode> <init_state> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r\n"

    "Pin Modes: \"ANALOG\", \"INPUT_FLOATING\", \"INPUT_PULL_DOWN\", \n\r"
    "\"INPUT_PULL_UP\", \"OUTPUT_OPEN_DRAIN\", \"OUTPUT_PUSH_PULL\", \n\r"
    "\"ALFTN_OPEN_DRAIN\", \"ALTFN_PUSH_PULL\" \n\r\n"

    "State: \"high\" or \"low\"\n\r";

const char gpio_toggle_help[] =
    "Usage: Toggles the output state of the pin. \n\r"
    "gpio toggle <address> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r";

const char gpio_get_help[] =
    "Usage: Gets the value of the input register for a pin and assigns it to the state \n\r"
    "that is passed in. \n\r"
    "gpio get <address> <input_state> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r\n"

    "State: \"high\" or \"low\"\n\r";

static const CmdStruct gpio_lookup[] = { { .cmd_name = "set", .cmd_func = &prv_set },
                                         { .cmd_name = "init_pin", .cmd_func = &prv_init_pin },
                                         { .cmd_name = "toggle", .cmd_func = &prv_toggle },
                                         { .cmd_name = "get", .cmd_func = &prv_get } };

void gpio_cmd(char *cmd) {
  char action[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(cmd, action);

  if (strcmp(action, "help") == 0 || strcmp(action, "h") == 0) {
    printf("\r%s\n", gpio_help);
    return;
  }

  for (size_t i = 0; i < SIZEOF_ARRAY(gpio_lookup); ++i) {
    if (strcmp(action, gpio_lookup[i].cmd_name) == 0) {
      gpio_lookup[i].cmd_func(cmd);
      return;
    }
  }
  // ERROR: Invalid action
  printf("Invalid action\n\r");
  printf("\r%s\n", gpio_help);
}

static void prv_set(char *args) {
  char addr[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, addr);

  char *state_str = args;

  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", gpio_set_help);
    return;
  } else if (!(valid_addr(addr) && valid_state(state_str))) {
    printf("\r%s\n", gpio_set_help);
    return;
  }

  GpioAddress address = { .port = addr[0] - 65, .pin = strtol(addr + 1, NULL, 10) };
  GpioState state = state_to_int(state_str);

  if (gpio_set_state(&address, state) == STATUS_CODE_OK) {
    printf("SUCCESS\n\r");
  } else {
    printf("FAILED\n\r");
  }
}

static void prv_init_pin(char *args) {
  char addr[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, addr);
  char pin_mode[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, pin_mode);

  int pin_mode_enum = valid_pin_mode(pin_mode);
  char *state_str = args;

  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", gpio_init_pin_help);
    return;
  } else if (!(valid_addr(addr) && (pin_mode_enum != NUM_GPIO_MODES) && valid_state(state_str))) {
    printf("Invalid pin mode\n\r");
    printf("\r%s\n", gpio_init_pin_help);
    return;
  }

  GpioAddress address = { .port = addr[0] - 65, .pin = strtol(addr + 1, NULL, 10) };
  GpioMode mode = pin_mode_enum;
  GpioState init_state = state_to_int(state_str);

  if (gpio_init_pin(&address, mode, init_state) == STATUS_CODE_OK) {
    printf("SUCCESS\n\r");
  } else {
    printf("FAILED\n\r");
  }
}

static void prv_toggle(char *args) {
  char *addr = args;
  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", gpio_toggle_help);
    return;
  } else if (!valid_addr(addr)) {
    printf("\r%s\n", gpio_toggle_help);
    return;
  }

  GpioAddress address = { .port = addr[0] - 65, .pin = strtol(addr + 1, NULL, 10) };

  if (gpio_toggle_state(&address) == STATUS_CODE_OK) {
    printf("SUCCESS\n\r");
  } else {
    printf("FAILED\n\r");
  }
}

static void prv_get(char *args) {
  char addr[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, addr);

  char *state_str = args;

  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", gpio_get_help);
    return;
  } else if (!(valid_addr(addr) && valid_state(state_str))) {
    printf("\r%s\n", gpio_get_help);
    return;
  }

  GpioAddress address = { .port = addr[0] - 65, .pin = strtol(addr + 1, NULL, 10) };
  GpioState input_state = state_to_int(state_str);

  if (gpio_get_state(&address, &input_state) == STATUS_CODE_OK) {
    printf("SUCCESS\n\r");
  } else {
    printf("FAILED\n\r");
  }
}

bool valid_addr(char *addr) {
  if (addr[0] < 97 || addr[0] > 122) {
    printf("Invalid port - must be a value from a to g\n\r");
    return false;
  }

  char *end_ptr = NULL;
  const uint8_t pin = strtol(addr + 1, &end_ptr, 10);

  if (pin > 15 || *end_ptr != '\0') {
    printf("Invalid pin - must be a value from 0 to 15\n\r");
    return false;
  }

  return true;
}

bool valid_state(char *state) {
  if (strcmp(state, "high") != 0 && strcmp(state, "low") != 0) {
    printf("Invalid state - must be HIGH or LOW (case sensitive)\n\r");
    return false;
  } else {
    return true;
  }
}

int valid_pin_mode(char *pin_mode) {
  const char *pin_modes[NUM_GPIO_MODES] = {
    "analog",           "input_floating",   "input_pull_down", "input_pull_up", "output_open_drain",
    "output_push_pull", "alftn_open_drain", "altfn_push_pull"
  };

  for (int i = 0; i < NUM_GPIO_MODES; ++i) {
    if (strcmp(pin_mode, pin_modes[i]) == 0) {
      return i;
    }
  }
  return NUM_GPIO_MODES;
}

bool state_to_int(char *state) {
  return strcmp(state, "low");
}
