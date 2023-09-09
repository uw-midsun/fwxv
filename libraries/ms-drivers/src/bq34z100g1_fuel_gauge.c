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

static uint8_t data_block[] = {
  0x00, 0x00,  // unspecified
  0x00, 0x00,  // Manufacture Date, default=0
  0x00, 0x01,  // Serial Number, default=1
  0x00, 0x00,  // Cycle Count, default=0
  0x03, 0x84,  // CC Threshold, default=900
  0x64,        // Max Error Limit, default=100
  0x03, 0xE8,  // Design Capacity, default=1000
  0x15, 0x18,  // Design Energy, default=5400
  0xFE, 0x70,  // SOH Load I, default=-400
  0x10, 0x68,  // Cell Charge Voltage T1-T2, default=4200
  0x10, 0x68,  // Cell Charge Voltage T2-T3, default=4200
  0x10, 0x68,  // Cell Charge Voltage T3-T4, default=4200
  0x0A,        // Charge Current T1-T2, default=10
  0x32,        // Charge Current T2-T3, default=50
  0x1E,        // Charge Current T3-T4, default=30
  0xF6,        // JEITA T1, default=-10
  0x0A,        // JEITA T2, default=10
  0x2D,        // JEITA T3, default=45
  0x37,        // JEITA T4, default=55
  0x01,        // Design Energy Scale
  0x42,        // First Letter in Device Name, default='B'
};

// Registers subclass

#define NUM_OF_SERIES_CELLS_OFFSET 7
#define PACK_CONFIGURATION_OFFSET 0

static uint8_t reg_block[] = { 0x01, 0x61,  // Pack Configuration, default=0x161
                               0x00, 0xFF,  // Pack Configuration B, default=0xFF
                               0x00, 0x30,  // Pack Configuration C, default=0x30
                               0x00, 0x00,  // LED_Comm Configuration, default=0x00
                               0x00, 0x00,  // Alert Configuration, default=0x00
                               0x00,        // Num series cell, default=1
                               0x00, 0x00,  // unspecified
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// IT Cfg subclass

#define CELL_TERMINATE_VOLTAGE_OFFSET 53

static uint8_t it_cfg_block[] = {
  0x00,                                            // Load Select, default=1
  0x00,                                            // Load Mode, default=0
  0x00, 0x00,                                      // unspecified
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A,  // Res Current, default=10
  0x00, 0x00,                                      // unspecified data field
  0x32,                                            // Max Res Factor, default=50
  0x01,                                            // Min Res Factor, default=1
  0x00,                                            // unspecified
  0x01, 0xF4,                                      // RA Filter
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Current Thresholds subclass

#define QUIT_CURRENT_OFFSET 4

static uint8_t curr_thresh_block[] = { 0x00, 0x3C,  // Dsg Current Threshold, default=60
                                       0x00, 0x4B,  // Chg Current Threshold, default=75
                                       0x00, 0x28,  // Quit Current, default=40
                                       0x00, 0x3C,  // Dsg Relax Time, default=60
                                       0x00,        // Chg Relax Time, default=60
                                       0x01, 0x90,  // Cell Max IR Correct, default=400
                                       0x00, 0x00,  // unspecified
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// State subclass

#define QMAX_CELL0_OFFSET 0

static uint8_t state_block[] = { 0x03, 0xE8,  // Qmax Cell 0, default=1000
                                 0x00, 0x00,  // Cycle Count, default=0
                                 0x00,        // Update Status, default=0
                                 0x10, 0x68,  // Cell V at Chg Term, default=4200
                                 0xFE, 0xD5,  // Avg I Last Run, default=-299
                                 0xFB, 0x95,  // Avg P Last Run, default=-1131
                                 0x00, 0x02,  // Cell Delta Voltage, default=2
                                 0x00, 0x14,  // T Rise, default=20
                                 0x03, 0xE8,  // T Time Constant, default=1000
                                 0x00, 0x00,  // unspecified
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Calibration Subclass

#define CC_GAIN_OFFSET 0
#define CC_DELTA_OFFSET 0

static uint8_t cali_block[] = {
  0x3E, 0xF4, 0x1F, 0x21,  // CC Gain, 4 byte float, default=0.4768
  0x49, 0x0A, 0x9C, 0x09,  // CC Delta, 4 byte float, default=567744.56
  0xFB, 0x50,              // CC Offset, default=-1200
  0x00,                    // Board Offset, default=0
  0x00,                    // Int Temp Offset, default=0
  0x00,                    // Ext Temp Offset, default=0
  0x13, 0x88,              // Voltage Divider, default=5000
  0x00, 0x00,              // unspecified
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

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
  // NOTE: might be incorrect framing, might have to use one write of 2 bytes
  // and one read of 2 bytes
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
  data_block[DESIGN_CAPACITY_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_DESIGN_CAPACITY_VAL);
  data_block[DESIGN_CAPACITY_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_DESIGN_CAPACITY_VAL);

  data_block[DESIGN_ENERGY_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_DESIGN_ENERGY);
  data_block[DESIGN_ENERGY_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_DESIGN_ENERGY);

  data_block[CELL_CHARGE_VOLT_T1_TO_T2_OFFSET % 32] =
      LOWER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T1_TO_T2);
  data_block[CELL_CHARGE_VOLT_T1_TO_T2_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T1_TO_T2);

  data_block[CELL_CHARGE_VOLT_T2_TO_T3_OFFSET % 32] =
      LOWER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T2_TO_T3);
  data_block[CELL_CHARGE_VOLT_T2_TO_T3_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T2_TO_T3);

  data_block[CELL_CHARGE_VOLT_T3_TO_T4_OFFSET % 32] =
      LOWER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T3_TO_T4);
  data_block[CELL_CHARGE_VOLT_T3_TO_T4_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_CHARGE_VOLT_T3_TO_T4);

  data_block[JEITA_TEMP1_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP1);
  data_block[JEITA_TEMP2_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP2);
  data_block[JEITA_TEMP3_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP3);
  data_block[JEITA_TEMP4_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_JEITA_TEMP4);
  status_ok_or_return(prv_write_flash(storage, CONFIGURATION_DATA, 0, data_block));

  // Registers Subclass
  reg_block[PACK_CONFIGURATION_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_PACK_CONFIGURATION_VAL);
  reg_block[PACK_CONFIGURATION_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_PACK_CONFIGURATION_VAL);

  reg_block[NUM_OF_SERIES_CELLS_OFFSET % 32] = BQ34Z100G1_NUM_OF_SERIES_CELLS;
  status_ok_or_return(prv_write_flash(storage, CONFIGURATION_REGISTERS, 0, reg_block));

  // IT Cfg Subclass
  it_cfg_block[CELL_TERMINATE_VOLTAGE_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_CELL_TERMINATE_VOLTAGE);
  it_cfg_block[CELL_TERMINATE_VOLTAGE_OFFSET % 32 + 1] =
      UPPER_BYTE(BQ34Z100G1_CELL_TERMINATE_VOLTAGE);
  status_ok_or_return(prv_write_flash(storage, GAS_GAUGING_IT_CFG, 0, it_cfg_block));

  // Current Thresholds Subclass
  curr_thresh_block[QUIT_CURRENT_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_QUIT_CURRENT);
  curr_thresh_block[QUIT_CURRENT_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_QUIT_CURRENT);
  status_ok_or_return(
      prv_write_flash(storage, GAS_GAUGING_CURRENT_THRESHOLDS, 0, curr_thresh_block));

  // State Subclass
  state_block[QMAX_CELL0_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_QMAX_CELL0);
  state_block[QMAX_CELL0_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_QMAX_CELL0);
  status_ok_or_return(prv_write_flash(storage, GAS_GAUGING_STATE, 0, state_block));

  // Calibration Data Subclass
  cali_block[CC_GAIN_OFFSET % 32] = LOWER_BYTE(BQ34Z100G1_SENSE_RESISTOR_VALUE);
  cali_block[CC_DELTA_OFFSET % 32 + 1] = UPPER_BYTE(BQ34Z100G1_SENSE_RESISTOR_VALUE);
  status_ok_or_return(prv_write_flash(storage, CALIBRATION_DATA, 0, cali_block));
  return STATUS_CODE_OK;
}

// PUBLIC FUNCTIONS

StatusCode bq34z100g1_get(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand, uint8_t ret[2]) {
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
          status_ok_or_return(prv_send_subcommand_pair(storage, command, subcommand, ret));
          break;
        default:
          LOG_CRITICAL("bq34z100q1: invalid subcommand (%d)", subcommand);
          return STATUS_CODE_INVALID_ARGS;
      }
      break;
    // these 4 commands return 8-bit values
    case BQ34Z100G1_STATE_OF_CHARGE:
    case BQ34Z100G1_MAX_ERROR:
    case BQ34Z100G1_GRID_NUMBER:
    case BQ34Z100G1_LEARNED_STATUS:
      status_ok_or_return(prv_send_command(storage, (uint8_t *)&command, 1, ret, 1));
      ret[1] = 0x00;
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
      status_ok_or_return(prv_send_command_pair(storage, command, ret));
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
  prv_init_flash(storage);
  return STATUS_CODE_OK;
}
