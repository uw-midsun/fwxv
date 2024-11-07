#pragma once
#include <stdbool.h>

#include "cli_base.h"

void gpio_cmd(char *input);

bool valid_addr(char *port);
bool valid_state(char *state);
int valid_pin_mode(char *pin_mode);
bool state_to_int(char *state);
