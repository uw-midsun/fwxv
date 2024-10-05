#include "adc_cli.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "log.h"
#include "string.h"

//Change these to match ADC
static const char adc_help[] =
    "CLI Reference for GPIO Pins. Usage: \n\r"
    "adc <add/run/raw/converted> <parameters> \n\r\n"

    "Initializes GPIO globally by setting all pins to their default state. \n\r"
    "ONLY CALL ONCE or it will deinit all current settings. Change \n\r"
    "setting by calling gpio_init_pin. \n\r\n"

    "Type \"help\" after any action for detailed reference.\n\r";

static const char adc_add_help[] =
    "Usage: Set the pin state by address.\n\r"
    "gpio set <address> <state> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r\n"

    "State: \"high\" or \"low\"\n\r";

static const char adc_run_help[] =
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

static const char adc_read_converted_help[] =
    "Usage: Toggles the output state of the pin. \n\r"
    "gpio toggle <address> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r";

static const char adc_read_raw_help[] =
    "Usage: Gets the value of the input register for a pin and assigns it to the state \n\r"
    "that is passed in. \n\r"
    "gpio get <address> <input_state> \n\r\n"

    "Address: <Port (a-g)><Pin (0-15)> \n\r"
    "Examples: A0, D5, E11, G15\n\r\n"

    "State: \"high\" or \"low\"\n\r";

static void prv_add_channel(char *args) {
  char addr[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, addr);
  char pin_mode[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, pin_mode);

  int pin_mode_enum = valid_pin_mode(pin_mode);
  char *state_str = args;

  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", adc_add_help);
    return;
  } else if (!(valid_addr(addr) && valid_state(state_str))) {
    printf("\r%s\n", adc_add_help);
    return;
  } else if (!(valid_addr(addr) && (pin_mode_enum != NUM_GPIO_MODES) && valid_state(state_str))) {
    printf("Invalid pin mode\n\r");
    printf("\r%s\n", adc_add_help);
    return;
  }

    //Might need to change port and pin
  GpioAddress address = { .port = addr[0] - 97, .pin = strtol(addr + 1, NULL, 10) };

  if (adc_add_channel(address) == STATUS_CODE_OK) {
    printf("SUCCESS\n\r");
  } else {
    printf("FAILED\n\r");
  }
}

static void prv_run(char *args) {
  char addr[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, addr);

  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", adc_run_help);
    return;
  }

  if (adc_run() == STATUS_CODE_OK) {
    printf("SUCCESS\n\r");
  }
  else {
    printf("FAILED\n\r");
  }
}

static void prv_read_raw(char *args) {
  char *addr = args;
  char pin_mode[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, pin_mode);

  int pin_mode_enum = valid_pin_mode(pin_mode);
  char *state_str = args;

  if (strcmp(addr, "help") == 0 || strcmp(addr, "h") == 0) {
    printf("\r%s\n", adc_read_raw_help);
    return;
  } else if (!valid_addr(addr)) {
    printf("\r%s\n", adc_read_raw_help);
    return;
  } else if (!(valid_addr(addr) && (pin_mode_enum != NUM_GPIO_MODES) && valid_state(state_str))) {
    printf("Invalid pin mode\n\r");
    printf("\r%s\n", adc_read_raw_help);
    return;
  }

  GpioAddress address = { .port = addr[0] - 97, .pin = strtol(addr + 1, NULL, 10) };
  uint16_t reading;

  if (adc_read_raw(address, &reading) == STATUS_CODE_OK) {
    printf("\r%d\n", reading);
  } else {
    printf("FAILED\n\r");
  }
}

static void prv_read_converted(char *args) {
  char *addr = args;
  char pin_mode[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(args, pin_mode);

  int pin_mode_enum = valid_pin_mode(pin_mode);
  char *state_str = args;

  if (strcmp(args, "help") == 0 || strcmp(args, "h") == 0) {
    printf("\r%s\n", adc_read_converted_help);
    return;
  } else if (!(valid_addr(args))) {
    printf("\r%s\n", adc_read_converted_help);
    return;
  } else if (!(valid_addr(addr) && (pin_mode_enum != NUM_GPIO_MODES) && valid_state(state_str))) {
    printf("Invalid pin mode\n\r");
    printf("\r%s\n", adc_read_converted_help);
    return;
  }

  GpioAddress address = { .port = args[0] - 97, .pin = strtol(args + 1, NULL, 10) };
  uint16_t reading;

  if (adc_read_converted(address, &reading) == STATUS_CODE_OK){
    printf("\r%d\n", reading);
  } else {
    printf("FAILED\n\r");
  }
}

bool valid_addr(char *addr) {
  if (addr[0] < 97 || addr[0] > 103) {
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
    printf("Invalid state - must be high or low\n\r");
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

static const CmdStruct adc_lookup[] = { { .cmd_name = "add", .cmd_func = &prv_add_channel },
                                         { .cmd_name = "run", .cmd_func = &prv_run },
                                         { .cmd_name = "raw", .cmd_func = &prv_read_raw },
                                         { .cmd_name = "converted", .cmd_func = &prv_read_converted } };

void gpio_cmd(char *cmd) {
  char action[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(cmd, action);

  if (strcmp(action, "help") == 0 || strcmp(action, "h") == 0) {
    printf("\r%s\n", adc_help);
    return;
  }

  for (size_t i = 0; i < SIZEOF_ARRAY(adc_lookup); ++i) {
    if (strcmp(action, adc_lookup[i].cmd_name) == 0) {
      adc_lookup[i].cmd_func(cmd);
      return;
    }
  }
  // ERROR: Invalid action
  printf("Invalid action\n\r");
  printf("\r%s\n", adc_help);
}
