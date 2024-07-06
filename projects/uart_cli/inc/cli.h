#pragma once
#include <stdio.h>

/*
For new peripherals:
- Create a new C and header file titled "*peripheral*_cli.c" and "*peripheral*_cli.h"
- Peripheral files should include a general help message for the peripheral and a help message for
each function
    - Refer to gpio_cli.c for help message formatting
    - Create private functions within each peripheral file to process parameters after calling
tok_cmd
    - Create a CmdStruct lookup table to store functions for the peripheral - this structure maps
the keyword for an action to the corresponding prv function
- Add a CmdStruct object to cmd_lookup for the peripheral
- Add the necessary init function for the peripheral
*/

#include "gpio_cli.h"

void cli_init();
void cli_run();
char *get_cmd();
void cmd_parse(char *cmd);
void print_help();
