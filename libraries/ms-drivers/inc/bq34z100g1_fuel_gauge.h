#pragma once

#include <stdbool.h>

#include "bq34z100g1_fuel_gauge_defs.h"
#include "i2c.h"
#include "log.h"
#include "semaphore.h"

typedef enum {
  // Standard Data Commands
  BQ34Z100G1_CONTROL = 0x00,
  BQ34Z100G1_STATE_OF_CHARGE = 0x02,  // uint8_t
  BQ34Z100G1_MAX_ERROR = 0x03,        // uint8_t
  BQ34Z100G1_REMAINING_CAPACITY = 0x04,
  BQ34Z100G1_FULL_CHARGE_CAPACITY = 0x06,
  BQ34Z100G1_VOLTAGE = 0x08,
  BQ34Z100G1_AVERAGE_CURRENT = 0x0A,
  BQ34Z100G1_TEMPERATURE = 0x0C,
  BQ34Z100G1_FLAGS = 0x0E,
  BQ34Z100G1_CURRENT = 0x10,
  BQ34Z100G1_FLAGSB = 0x12,
  // Extended Data Commands
  BQ34Z100G1_AVERAGE_TIME_TO_EMPTY = 0x18,
  BQ34Z100G1_AVERAGE_TIME_TO_FULL = 0x1A,
  BQ34Z100G1_PASSED_CHARGE = 0x1C,
  BQ34Z100G1_DOD0_TIME = 0x1E,
  BQ34Z100G1_AVAILABLE_ENERGY = 0x24,
  BQ34Z100G1_AVERAGE_POWER = 0x26,
  BQ34Z100G1_SERIAL_NUMBER = 0x28,
  BQ34Z100G1_INTERNAL_TEMP = 0x2A,
  BQ34Z100G1_CYCLE_COUNT = 0x2C,
  BQ34Z100G1_STATE_OF_HEALTH = 0x2E,
  BQ34Z100G1_CHARGE_VOLTAGE = 0x30,
  BQ34Z100G1_CHARGE_CURRENT = 0x32,
  BQ34Z100G1_PACK_CONFIGURATION = 0x3A,
  BQ34Z100G1_DESIGN_CAPACITY = 0x3C,
  BQ34Z100G1_DATA_FLASH_CLASS = 0x3E,
  BQ34Z100G1_DATA_FLASH_BLOCK = 0x3F,
  BQ34Z100G1_BLOCK_DATA = 0x40,
  BQ34Z100G1_BLOCK_DATA_CHECKSUM = 0x60,
  BQ34Z100G1_BLOCK_DATA_CONTROL = 0x61,
  BQ34Z100G1_GRID_NUMBER = 0x62,     // uint8_t
  BQ34Z100G1_LEARNED_STATUS = 0x63,  // uint8_t
  BQ34Z100G1_DOD_AT_EOC = 0x64,
  BQ34Z100G1_Q_START = 0x66,
  BQ34Z100G1_TRUE_RC = 0x68,
  BQ34Z100G1_TRUE_FCC = 0x6A,
  BQ34Z100G1_STATE_TIME = 0x6C,
  BQ34Z100G1_Q_MAX_PASSED_Q = 0x6E,
  BQ34Z100G1_DOD0 = 0x70,
  BQ34Z100G1_Q_MAX_DOD0 = 0x72,
  BQ34Z100G1_Q_MAX_TIME = 0x74,
} Bq34z100g1Command;

typedef enum {
  // get commands
  BQ34Z100G1_CONTROL_STATUS = 0x0000,
  BQ34Z100G1_DEVICE_TYPE = 0x0001,
  BQ34Z100G1_FW_VERSION = 0x0002,
  BQ34Z100G1_HW_VERSION = 0x0003,
  BQ34Z100G1_RESET_DATA = 0x0005,
  BQ34Z100G1_PREV_MACWRITE = 0x0007,
  BQ34Z100G1_CHEM_ID = 0x0008,
  // set commands
  BQ34Z100G1_BOARD_OFFSET = 0x0009,
  BQ34Z100G1_CC_OFFSET = 0x000A,
  BQ34Z100G1_CC_OFFSET_SLAVE = 0x000B,
  BQ34Z100G1_DF_VERSION = 0x000C,  // get command
  BQ34Z100G1_SET_FULLSLEEP = 0x0010,
  BQ34Z100G1_STATIC_CHEM_CHKSUM = 0x0017,
  BQ34Z100G1_SEALED = 0x0020,
  BQ34Z100G1_IT_ENABLE = 0x0021,
  BQ34Z100G1_CAL_ENABLE = 0x002D,
  BQ34Z100G1_CONTROL_RESET = 0x0041,
  BQ34Z100G1_EXIT_CAL = 0x0080,
  BQ34Z100G1_ENTER_CAL = 0x0081,
  BQ34Z100G1_OFFSET_CAL = 0x0082,
} Bq34z100g1SubCommand;

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;
} Bq34z100g1Settings;

typedef struct {
  Bq34z100g1Settings settings;
} Bq34z100g1Storage;

/*
Brief: Used to get register values from the fuel gauge using the Bq34z100g1Command commands
Param:
  storage - pointer to an initialized Bq34z100g1Storage, really just to store the i2c information
  command - determines the value returned by ret. P.11 of the data sheet outlines the value
            returned from each command. Note that some commands are for utility purposes and do
            not return a value, if one of these commands is used in a call to this function it
            will return STATUS_CODE_INVALID_ARGS.
  subcommand - the BQ34Z100G1_CONTROL command requires a subcommand. Some of these subcommands
               are used to set values, if one of these commands is used in a call to this function
               it will return STATUS_CODE_INVALID_ARGS.
  ret - array provided by user to hold value returned by fuel gauge. The least signigicant bit will
        be stored in the first byte and most significat bit will be stored in the second. In case
        of a command that only returns an 8 bit val, the upper byte will be set to 0x00.
Return: Returns STATUS_CODE_OK on successful command, STATUS_CODE_INVALID_ARGS when incorrect
        command used, and forwards error codes from any i2c driver failures

*/
StatusCode bq34z100g1_get(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand, uint8_t ret[2]);

/*
Brief: Used to set register values from the fuel gauge using the BQ34Z100G1_CONTROL command and
       the Bq34z100g1SubCommand subcommands.
Param:
  storage - pointer to an initialized Bq34z100g1Storage, really just to store the i2c information
  command - only valid command is BQ34Z100G1_CONTROL, otherwise will return
            STATUS_CODE_INVALID_ARGS
  subcommand - the BQ34Z100G1_CONTROL command requires a subcommand P.11 of the data sheet
               outlines. Some of these subcommands are used to get values, if one of these
               commands is used in a call to this function it will return
               STATUS_CODE_INVALID_ARGS.
Return: Returns STATUS_CODE_OK on successful command, STATUS_CODE_INVALID_ARGS when incorrect
        command used, and forwards error codes from any i2c driver failures
*/
StatusCode bq34z100g1_set(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand);

/*
Brief: Initializes the i2c interface provided in the settings, configures the data flash
       to the values configured in the #defines above
Param:
  storage - pointer to an unitialized storage struct
  settings - i2c address for the fuel gauge and the i2c port connected to it
Return: Returns STATUS_CODE_OK on successful command, STATUS_CODE_INVALID_ARGS when incorrect
        command used, and forwards error codes from any i2c driver failures
*/
StatusCode bq34z100g1_init(Bq34z100g1Storage *storage, Bq34z100g1Settings settings);
