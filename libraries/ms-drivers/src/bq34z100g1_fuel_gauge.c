#include "bq34z100g1_fuel_gauge.h"

// PRIVATE DEFINES

#define CONCATENATE_BYTES(UP, LO) (UP << 8U) + LO
#define UPPER_BYTE(X) X >> 8
#define LOWER_BYTE(X) X & 0xFF
#define SET_IF_NULL(X, Y) X = (X == NULL) ? Y : X

// data flash offsets in bytes

// Data subclass

#define DESIGN_CAPACITY_OFFSET 11
#define DESIGN_ENERGY_OFFSET 13

#define CELL_CHARGE_VOLT_T1_TO_T2_OFFSET 17
#define CELL_CHARGE_VOLT_T2_TO_T3_OFFSET 19
#define CELL_CHARGE_VOLT_T3_TO_T4_OFFSET 21

#define JEITA_TEMP1_OFFSET 26
#define JEITA_TEMP2_OFFSET 27
#define JEITA_TEMP3_OFFSET 28
#define JEITA_TEMP4_OFFSET 29

#define DEVICE_CHEMISTRY_OFFSET 55

// Registers subclass

#define NUM_OF_SERIES_CELLS_OFFSET 7
#define PACK_CONFIGURATION_OFFSET 0

// IT Cfg subclass

#define CELL_TERMINATE_VOLTAGE_OFFSET 53

// Current Thresholds subclass

#define QUIT_CURRENT_OFFSET 4

// State subclass

#define QMAX_CELL0_OFFSET 0

// Calibration Subclass

#define CC_GAIN_OFFSET 0
#define CC_DELTA_OFFSET 0

typedef enum {
  CONFIGURATION_DATA = 48,
  CONFIGURATION_REGISTERS = 64,
  GAS_GAUGING_IT_CFG = 80,
  GAS_GAUGING_CURRENT_THRESHOLDS = 81,
  GAS_GAUGING_STATE = 82,
  CALIBRATION_DATA = 104
} Bq34z100g1DataSubClass;

// HELPER FUNCTIONS

StatusCode prv_send_command(Bq34z100g1Storage *storage, uint8_t *cmd, size_t cmd_size, uint8_t *val,
                            size_t val_size) {
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, cmd, cmd_size));
  status_ok_or_return(
      i2c_read(storage->settings.i2c_port, storage->settings.i2c_address, val, val_size));
  return STATUS_CODE_OK;
}

StatusCode prv_send_command_pair(Bq34z100g1Storage *storage, uint8_t command, uint8_t value[2]) {
  status_ok_or_return(prv_send_command(storage, &command, 1, value, 1));
  command++;
  status_ok_or_return(prv_send_command(storage, &command, 1, (value + 1), 1));
  return STATUS_CODE_OK;
}

StatusCode prv_send_subcommand(Bq34z100g1Storage *storage, uint8_t command, uint16_t subcommand,
                               uint8_t *value) {
  uint8_t command_list[3];
  command_list[0] = command;
  command_list[1] = LOWER_BYTE(subcommand);
  command_list[2] = UPPER_BYTE(subcommand);
  status_ok_or_return(prv_send_command(storage, command_list, 3, value, 1));
  return STATUS_CODE_OK;
}

StatusCode prv_send_subcommand_no_return(Bq34z100g1Storage *storage, uint8_t command,
                                         uint16_t subcommand) {
  uint8_t command_list[3];
  command_list[0] = command;
  command_list[1] = LOWER_BYTE(subcommand);
  command_list[2] = UPPER_BYTE(subcommand);
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command_list, 3));
  return STATUS_CODE_OK;
}

StatusCode prv_send_subcommand_pair(Bq34z100g1Storage *storage, uint8_t command,
                                    uint16_t subcommand, uint8_t value[2]) {
  status_ok_or_return(prv_send_subcommand(storage, command, subcommand, value));
  command++;
  status_ok_or_return(prv_send_subcommand(storage, command, subcommand, value + 1));
  return STATUS_CODE_OK;
}

// See datasheet p.21 step 1 for this process
StatusCode prv_unseal(Bq34z100g1Storage *storage) {
  // write the first 2 bytes of the unseal key using the Control() command
  uint16_t unseal_key = BQ34Z100G1_SEALED_TO_UNSEALED_KEY & 0xFFFF;
  status_ok_or_return(prv_send_subcommand_no_return(storage, BQ34Z100G1_CONTROL, unseal_key));

  // write the second 2 bytes of the unseal key using the Control() command
  unseal_key = BQ34Z100G1_SEALED_TO_UNSEALED_KEY >> 16U;
  status_ok_or_return(prv_send_subcommand_no_return(storage, BQ34Z100G1_CONTROL, unseal_key));
  return STATUS_CODE_OK;
}

StatusCode prv_access_flash(Bq34z100g1Storage *storage, uint8_t data_flash_subclass,
                            uint8_t data_flash_block, uint8_t value[32]) {
  uint8_t command[2];
  // enable block data flash control
  command[0] = BQ34Z100G1_BLOCK_DATA_CONTROL;
  command[1] = 0x00;
  status_ok_or_return(i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command,
                                sizeof(command)));

  // specify data flash class
  command[0] = BQ34Z100G1_DATA_FLASH_CLASS;
  command[1] = data_flash_subclass;
  status_ok_or_return(i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command,
                                sizeof(command)));

  // specify data flash block
  command[0] = BQ34Z100G1_DATA_FLASH_BLOCK;
  command[1] = data_flash_block;
  status_ok_or_return(i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command,
                                sizeof(command)));

  // read 32-byte block
  for (int i = 0; i < 32; ++i) {
    command[0] = BQ34Z100G1_BLOCK_DATA + i;
    status_ok_or_return(prv_send_command(storage, command, 1, (value + i), 1));
  }
  return STATUS_CODE_OK;
}

StatusCode prv_write_flash(Bq34z100g1Storage *storage, uint8_t data_flash_subclass,
                           uint8_t data_flash_block, uint8_t value[32]) {
  uint8_t command[33];
  // enable block data flash control
  command[0] = BQ34Z100G1_BLOCK_DATA_CONTROL;
  command[1] = 0x00;
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command, 2));

  // specify data flash class
  command[0] = BQ34Z100G1_DATA_FLASH_CLASS;
  command[1] = data_flash_subclass;
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command, 2));

  // specify data flash block
  command[0] = BQ34Z100G1_DATA_FLASH_BLOCK;
  command[1] = data_flash_block;
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command, 2));

  // write block
  command[0] = BQ34Z100G1_BLOCK_DATA;
  for (int i = 0; i < 32; ++i) {  // replace with memcpy
    command[i + 1] = value[i];
  }
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command, 33));

  // write checksum
  uint8_t checksum = 0;
  for (int i = 0; i < 32; ++i) {
    checksum += value[i];
  }
  command[0] = BQ34Z100G1_BLOCK_DATA_CHECKSUM;
  command[1] = checksum;
  status_ok_or_return(
      i2c_write(storage->settings.i2c_port, storage->settings.i2c_address, command, 2));

  // reset the gauge
  status_ok_or_return(
      prv_send_subcommand_no_return(storage, BQ34Z100G1_CONTROL, BQ34Z100G1_CONTROL_RESET));
  return STATUS_CODE_OK;
}

StatusCode prv_init_flash(Bq34z100g1Storage *storage) {
  status_ok_or_return(prv_unseal(storage));
  uint8_t value[32];

  // Data Subclass
  status_ok_or_return(prv_access_flash(storage, CONFIGURATION_DATA, 0, value));
  value[DESIGN_CAPACITY_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_DESIGN_CAPACITY_VAL);
  value[DESIGN_CAPACITY_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_DESIGN_CAPACITY_VAL);

  value[DESIGN_ENERGY_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_DESIGN_ENERGY);
  value[DESIGN_ENERGY_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_DESIGN_ENERGY);

  value[CELL_CHARGE_VOLT_T1_TO_T2_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T1_TO_T2);
  value[CELL_CHARGE_VOLT_T1_TO_T2_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T1_TO_T2);

  value[CELL_CHARGE_VOLT_T2_TO_T3_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T2_TO_T3);
  value[CELL_CHARGE_VOLT_T2_TO_T3_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T2_TO_T3);

  value[CELL_CHARGE_VOLT_T3_TO_T4_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T3_TO_T4);
  value[CELL_CHARGE_VOLT_T3_TO_T4_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T3_TO_T4);

  value[JEITA_TEMP1_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP1);
  value[JEITA_TEMP2_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP2);
  value[JEITA_TEMP3_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP3);
  value[JEITA_TEMP4_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP4);
  status_ok_or_return(
      prv_write_flash(storage, CONFIGURATION_DATA, (CELL_CHARGE_VOLT_T3_TO_T4_OFFSET / 32), value));

  // Registers Subclass
  status_ok_or_return(prv_access_flash(storage, CONFIGURATION_REGISTERS, 0, value));
  value[PACK_CONFIGURATION_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_PACK_CONFIGURATION_VAL);
  value[PACK_CONFIGURATION_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_PACK_CONFIGURATION_VAL);

  value[NUM_OF_SERIES_CELLS_OFFSET % 32] = BQ34Z100G1_NUM_OF_SERIES_CELLS;
  status_ok_or_return(prv_write_flash(storage, CONFIGURATION_REGISTERS, 0, value));

  // IT Cfg Subclass
  status_ok_or_return(prv_access_flash(storage, GAS_GAUGING_IT_CFG, 0, value));
  value[CELL_TERMINATE_VOLTAGE_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_CELL_TERMINATE_VOLTAGE);
  value[CELL_TERMINATE_VOLTAGE_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_CELL_TERMINATE_VOLTAGE);
  status_ok_or_return(prv_write_flash(storage, GAS_GAUGING_IT_CFG, 0, value));

  // Current Thresholds Subclass
  status_ok_or_return(prv_access_flash(storage, GAS_GAUGING_CURRENT_THRESHOLDS, 0, value));
  value[QUIT_CURRENT_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_QUIT_CURRENT);
  value[QUIT_CURRENT_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_QUIT_CURRENT);
  status_ok_or_return(prv_write_flash(storage, GAS_GAUGING_CURRENT_THRESHOLDS, 0, value));

  // State Subclass
  status_ok_or_return(prv_access_flash(storage, GAS_GAUGING_STATE, 0, value));
  value[QMAX_CELL0_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_QMAX_CELL0);
  value[QMAX_CELL0_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_QMAX_CELL0);
  status_ok_or_return(prv_write_flash(storage, GAS_GAUGING_STATE, 0, value));

  // Calibration Data Subclass

  return STATUS_CODE_OK;
}

// PUBLIC FUNCTIONS

StatusCode bq34z100g1_get(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand, void *ret) {
  uint8_t value[2];
  uint8_t *ret8;
  uint16_t *ret16;
  switch (command) {
    case BQ34Z100G1_CONTROL:
      switch (subcommand) {
        case BQ34Z100G1_CONTROL_STATUS:
        case BQ34Z100G1_DEVICE_TYPE:
        case BQ34Z100G1_FW_VERSION:
        case BQ34Z100G1_HW_VERSION:
        case BQ34Z100G1_RESET_DATA:
        case BQ34Z100G1_PREV_MACWRITE:
        case BQ34Z100G1_CHEM_ID:
        case BQ34Z100G1_DF_VERSION:
          ret16 = ret;
          status_ok_or_return(prv_send_subcommand_pair(storage, command, subcommand, value));
          *ret16 = CONCATENATE_BYTES(value[1], value[0]);
          break;
        default:
          LOG_CRITICAL("bq34z100q1: invalid subcommand (%d)", subcommand);
          return STATUS_CODE_INVALID_ARGS;
      }
      break;
    case BQ34Z100G1_STATE_OF_CHARGE:
    case BQ34Z100G1_MAX_ERROR:
    case BQ34Z100G1_GRID_NUMBER:
    case BQ34Z100G1_LEARNED_STATUS:
      ret8 = ret;
      status_ok_or_return(prv_send_command(storage, &command, 1, ret8, 1));
      break;
    case BQ34Z100G1_REMAINING_CAPACITY:
    case BQ34Z100G1_FULL_CHARGE_CAPACITY:
    case BQ34Z100G1_VOLTAGE:
    case BQ34Z100G1_AVERAGE_CURRENT:
    case BQ34Z100G1_TEMPERATURE:
    case BQ34Z100G1_FLAGS:
    case BQ34Z100G1_CURRENT:
    case BQ34Z100G1_FLAGSB:
    case BQ34Z100G1_AVERAGE_TIME_TO_EMPTY:
    case BQ34Z100G1_AVERAGE_TIME_TO_FULL:
    case BQ34Z100G1_PASSED_CHARGE:
    case BQ34Z100G1_DOD0_TIME:
    case BQ34Z100G1_AVAILABLE_ENERGY:
    case BQ34Z100G1_AVERAGE_POWER:
    case BQ34Z100G1_SERIAL_NUMBER:
    case BQ34Z100G1_INTERNAL_TEMP:
    case BQ34Z100G1_CYCLE_COUNT:
    case BQ34Z100G1_STATE_OF_HEALTH:
    case BQ34Z100G1_CHARGE_VOLTAGE:
    case BQ34Z100G1_CHARGE_CURRENT:
    case BQ34Z100G1_PACK_CONFIGURATION:
    case BQ34Z100G1_DESIGN_CAPACITY:
    case BQ34Z100G1_DOD_AT_EOC:
    case BQ34Z100G1_Q_START:
    case BQ34Z100G1_TRUE_RC:
    case BQ34Z100G1_TRUE_FCC:
    case BQ34Z100G1_STATE_TIME:
    case BQ34Z100G1_Q_MAX_PASSED_Q:
    case BQ34Z100G1_DOD0:
    case BQ34Z100G1_Q_MAX_DOD0:
    case BQ34Z100G1_Q_MAX_TIME:
      ret16 = ret;
      status_ok_or_return(prv_send_command_pair(storage, command, value));
      *ret16 = CONCATENATE_BYTES(value[1], value[0]);
      break;
    default:
      LOG_CRITICAL("bq34z100q1: invalid command (%d)", command);
      return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

StatusCode bq34z100g1_set(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand) {
  switch (command) {
    case BQ34Z100G1_CONTROL:
      switch (subcommand) {
        case BQ34Z100G1_BOARD_OFFSET:
        case BQ34Z100G1_CC_OFFSET:
        case BQ34Z100G1_CC_OFFSET_SLAVE:
        case BQ34Z100G1_DF_VERSION:
        case BQ34Z100G1_SET_FULLSLEEP:
        case BQ34Z100G1_STATIC_CHEM_CHKSUM:
        case BQ34Z100G1_SEALED:
        case BQ34Z100G1_IT_ENABLE:
        case BQ34Z100G1_CAL_ENABLE:
        case BQ34Z100G1_CONTROL_RESET:
        case BQ34Z100G1_EXIT_CAL:
        case BQ34Z100G1_ENTER_CAL:
        case BQ34Z100G1_OFFSET_CAL:
          status_ok_or_return(prv_send_subcommand_no_return(storage, command, subcommand));
          break;
        default:
          LOG_CRITICAL("bq34z100q1: invalid subcommand (%d)", subcommand);
          return STATUS_CODE_INVALID_ARGS;
      }
      break;
    default:
      LOG_CRITICAL("bq34z100q1: invalid command (%d)", command);
      return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

StatusCode bq34z100g1_init(Bq34z100g1Storage *storage, Bq34z100g1Settings settings) {
  if (settings.i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->settings = settings;

  return STATUS_CODE_OK;
}
