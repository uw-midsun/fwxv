#pragma once
#include <stdio.h>

#include "gpio_cli.h"

void cli_init();
void cli_run();
char *get_cmd();
void cmd_parse(char *cmd);
