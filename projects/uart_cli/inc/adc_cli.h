#pragma once
#include <stdbool.h>

#include "cli_base.h"

void adc_cmd(char *input);

bool adc_valid_addr(char *port);
bool adc_valid_state(char *state);
int adc_valid_pin_mode(char *pin_mode);
bool adc_state_to_int(char *state);
