#include <stdbool.h>

#include "cli_base.h"

void gpio_cmd(char *input);
void prv_set(char *args);
void prv_init_pin(char *args);
void prv_toggle(char *args);
void prv_get(char *args);

bool valid_addr(char *port);
bool valid_state(char *state);
int valid_pin_mode(char *pin_mode);
bool state_to_int(char *state);
