#include "cli.h"

#include <stdio.h>

#include "ctype.h"
#include "interrupt.h"
#include "log.h"
#include "string.h"
#include "tasks.h"
#include "uart.h"

static const char cli_help[] =
    "MSXV Controller Board CLI. Usage: \n\r"
    "<peripheral> <action> <parameters> \n\r\n"
    "Enter \"help\" after any argument for detailed reference. \n\r\n"
    "List of Peripherals: gpio";

static char cmd_buffer[MAX_CMD_LEN + 1];
// Add additional peripherals to lookup array
static const CmdStruct cmd_lookup[] = { { .cmd_name = "gpio", .cmd_func = &gpio_cmd } };

void cli_init() {
  // Add peripheral init calls here
  gpio_init();
  setbuf(stdout, NULL);
  printf("\n\rCLI Launched\n\r");
}

void cli_run() {
  printf("\n\r> ");
  char *input = get_cmd();
  if (input != NULL) {
    cmd_parse(input);
  }
}

char *get_cmd() {
  size_t idx = 0;
  memset(cmd_buffer, 0, sizeof(cmd_buffer));

  while (true) {
    size_t len = 1;
    uint8_t data = 0;
    StatusCode status = STATUS_CODE_EMPTY;
    while (status != STATUS_CODE_OK) {
      len = 1;
      status = uart_rx(UART_PORT_1, &data, &len);
    }

    if (idx == MAX_CMD_LEN && (data != '\r' && data != '\b')) {
      continue;
    }

    if (data == '\r') {
      if (idx == 0) {
        return NULL;
      }
      printf("\n\r");
      return cmd_buffer;
    } else if (data == '\b') {
      if (idx == 0) {
        continue;
      }
      --idx;
      cmd_buffer[idx % MAX_CMD_LEN] = 0;
      printf("\b \b");
    } else {
      cmd_buffer[idx % MAX_CMD_LEN] = data;
      ++idx;
      printf("%c", data);
    }
  }
}

void cmd_parse(char *cmd) {
  for (size_t i = 0; i < MAX_CMD_LEN; ++i) {
    cmd[i] = tolower(cmd[i]);
  }
  char peripheral[MAX_CMD_LEN + 1] = { 0 };
  tok_cmd(cmd, peripheral);

  if (strcmp(peripheral, "help") == 0 || strcmp(peripheral, "h") == 0) {
    print_help();
    return;
  }

  for (size_t i = 0; i < SIZEOF_ARRAY(cmd_lookup); ++i) {
    if (strcmp(peripheral, cmd_lookup[i].cmd_name) == 0) {
      cmd_lookup[i].cmd_func(cmd);
      return;
    }
  }

  // ERROR: Invalid peripheral
  printf("Invalid peripheral\n\r");
  print_help();
}

void print_help() {
  printf("\r%s", cli_help);
  for (size_t i = 1; i < SIZEOF_ARRAY(cmd_lookup); ++i) {
    printf(", %s", cmd_lookup[i].cmd_name);
  }
  printf("\n\r");
}
