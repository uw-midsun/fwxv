#pragma once

#include <stdbool.h>

#include "i2c.h"
#include "log.h"
#include "semaphore.h"

// bq34z100g1 defines

#define BQ34Z100G1_SEALED_TO_UNSEALED_KEY 0x36720414

#define BQ34Z100G1_DESIGN_CAPACITY_VAL 1234U  // mAh
#define BQ34Z100G1_DESIGN_ENERGY 1234U        // in mWh

// value in celsius
#define BQ34Z100G1_JEITA_TEMP1 0U
#define BQ34Z100G1_JEITA_TEMP2 20U
#define BQ34Z100G1_JEITA_TEMP3 40U
#define BQ34Z100G1_JEITA_TEMP4 60U

#define BQ34Z100G1_CELL_CHARGE_VOLT_T1_TO_T2 1000U
#define BQ34Z100G1_CELL_CHARGE_VOLT_T2_TO_T3 1000U
#define BQ34Z100G1_CELL_CHARGE_VOLT_T3_TO_T4 1000U

#define BQ34Z100G1_NUM_OF_SERIES_CELLS 50
#define BQ34Z100G1_PACK_CONFIGURATION_VAL 0x0000

#define BQ34Z100G1_SENSE_RESISTOR_VALUE 100  // in mOhm

#define BQ34Z100G1_CELL_TERMINATE_VOLTAGE 20  // in mV
#define BQ34Z100G1_QUIT_CURRENT 100           // in mA
#define BQ34Z100G1_QMAX_CELL0 2000            // in mA

#define BQ34Z100G1_DEVICE_CHEMISTRY 0x00  // chem id

/*
CONTROL_STATUS register bit masks (p.13)
            | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 High Byte  | RSVD  | FAS   | SS    | CALEN | CCA   | BCA   | CSV   | RSVD  |
 Low Byte   | RSVD  | RSVD  |FULLSLP| SLEEP | LDMD  |RUP_DIS| VOK   | QEN   |
*/

#define BQ34Z100G1_CTL_STS_FAS 1U << 6U    // indicates FULL ACCESS SEALED state, active when set
#define BQ34Z100G1_CTL_STS_SS 1U << 5U     // indicates SEALED state, active when set
#define BQ34Z100G1_CTL_STS_CALEN 1U << 4U  // indicates calibration function active, active when set
#define BQ34Z100G1_CTL_STS_CCA \
  1U << 3U  // indicates Coulomb Counter Calibration routine active, active when set
#define BQ34Z100G1_CTL_STS_BCA \
  1U << 2U  // indicates Board Calibration routine is active, active when set
#define BQ34Z100G1_CTL_STS_CSV \
  1U << 1U  // indicates valid data flash checksum has been generated, active when set
#define BQ34Z100G1_CTL_STS_FULLSLEEP 1U << 5U  // indicates when in FULL SLEEP mode, true when set
#define BQ34Z100G1_CTL_STS_SLEEP 1U << 4U      // indicates when in SLEEP mode, true when set
#define BQ34Z100G1_CTL_STS_LDMD \
  1U << 3U  // indicates Impedance Track algorithm using constant-power mode, true when set
#define BQ34Z100G1_CTL_STS_RUP_DIS \
  1U << 2U  // indicates Ra table updates are disabled, true when set
#define BQ34Z100G1_CTL_STS_VOK \
  1U << 1U  // indicates cell voltages OK for Qmax updates, true when set
#define BQ34Z100G1_CTL_STS_QEN 1U << 0U  // indicates Qmax updates are enabled, true when set

/*
FLAGS register bit masks (p.13)
            | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 High Byte  | OTC   | OTD   | BATHI | BATLOW|CHG_INH| XCHG  | FC    | CHG   |
 Low Byte   |OCVTAKN| RSVD  | RSVD  | CF    | RSVD  | SOC1  | SOCF  | DSG   |
*/

#define BQ34Z100G1_FLG_OTC 1U << 7U
#define BQ34Z100G1_FLG_OTD 1U << 6U
#define BQ34Z100G1_FLG_BATHI 1U << 5U
#define BQ34Z100G1_FLG_BATLOW 1U << 4U
#define BQ34Z100G1_FLG_CHG_INH 1U << 3U
#define BQ34Z100G1_FLG_XCHG 1U << 2U
#define BQ34Z100G1_FLG_FC 1U << 1U
#define BQ34Z100G1_FLG_CHG 1U << 0U
#define BQ34Z100G1_FLG_OCVTAKN 1U << 7U
#define BQ34Z100G1_FLG_CF 1U << 4U
#define BQ34Z100G1_FLG_SOC1 1U << 2U
#define BQ34Z100G1_FLG_SOCF 1U << 1U
#define BQ34Z100G1_FLG_DSG 1U << 0U

/*
FLAGSB register bit masks (p.13)
            | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 High Byte  | SOH   | LIFE  |FRSTDOD| RSVD  | RSVD  | DODEOC| DTRC  | RSVD  |
 Low Byte   | RSVD  | RSVD  | RSVD  | RSVD  | RSVD  | RSVD  | RSVD  | RSVD  |
*/

#define BQ34Z100G1_FLGB_SOH 1U << 7U
#define BQ34Z100G1_FLGB_LIFE 1U << 6U
#define BQ34Z100G1_FLGB_FRSTDOD 1U << 5U
#define BQ34Z100G1_FLGB_DODEOC 1U << 4U
#define BQ34Z100G1_FLGB_DTRC 1U << 3U

// If not specified these commands void *ret should be uint16_t
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
  BQ34Z100G1_CONTROL_STATUS = 0x0000,
  BQ34Z100G1_DEVICE_TYPE = 0x0001,
  BQ34Z100G1_FW_VERSION = 0x0002,
  BQ34Z100G1_HW_VERSION = 0x0003,
  BQ34Z100G1_RESET_DATA = 0x0005,
  BQ34Z100G1_PREV_MACWRITE = 0x0007,
  BQ34Z100G1_CHEM_ID = 0x0008,
  BQ34Z100G1_BOARD_OFFSET = 0x0009,
  BQ34Z100G1_CC_OFFSET = 0x000A,
  BQ34Z100G1_CC_OFFSET_SLAVE = 0x000B,
  BQ34Z100G1_DF_VERSION = 0x000C,
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

StatusCode bq34z100g1_get(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand, void *ret);
StatusCode bq34z100g1_set(Bq34z100g1Storage *storage, Bq34z100g1Command command,
                          Bq34z100g1SubCommand subcommand);
StatusCode bq34z100g1_init(Bq34z100g1Storage *storage, Bq34z100g1Settings settings);
