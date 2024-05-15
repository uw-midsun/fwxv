#ifndef PROJECTS_UART_CLI_INC_CLI_H_
#define PROJECTS_UART_CLI_INC_CLI_H_
#include <stdio.h>

#include "gpio_cli.h"

void cli_init();
void cli_run();
char *get_cmd();
void cmd_parse(char *cmd);
#endif  // PROJECTS_UART_CLI_INC_CLI_H_
