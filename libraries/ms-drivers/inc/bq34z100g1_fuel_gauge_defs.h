#pragma once

/*
CONTROL_STATUS register bit masks (p.13)
            | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
 High Byte  | RSVD  | FAS   | SS    | CALEN | CCA   | BCA   | CSV   | RSVD  |
 Low Byte   | RSVD  | RSVD  |FULLSLP| SLEEP | LDMD  |RUP_DIS| VOK   | QEN   |
*/

#define BQ34Z100G1_CTL_STS_FAS 1U << 6U
#define BQ34Z100G1_CTL_STS_SS 1U << 5U
#define BQ34Z100G1_CTL_STS_CALEN 1U << 4U
#define BQ34Z100G1_CTL_STS_CCA 1U << 3U
#define BQ34Z100G1_CTL_STS_BCA 1U << 2U
#define BQ34Z100G1_CTL_STS_CSV 1U << 1U
#define BQ34Z100G1_CTL_STS_FULLSLEEP 1U << 5U
#define BQ34Z100G1_CTL_STS_SLEEP 1U << 4U
#define BQ34Z100G1_CTL_STS_LDMD 1U << 3U
#define BQ34Z100G1_CTL_STS_RUP_DIS 1U << 2U
#define BQ34Z100G1_CTL_STS_VOK 1U << 1U
#define BQ34Z100G1_CTL_STS_QEN 1U << 0U

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

// key used for sealing and unsealing the fuel
#define BQ34Z100G1_SEALED_TO_UNSEALED_KEY 0x36720414

//////////////////////////////////////
// Measured Values from Battery Box //
//////////////////////////////////////

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
