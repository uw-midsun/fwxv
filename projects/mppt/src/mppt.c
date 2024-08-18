#include "mppt.h"
#include "power_sense.h"

MPPTData *mppt;

void mppt_sweep() {
    for (uint8_t dc = 50; dc < 95; dc++) {}
}

void mppt_init(MPPTData *init_mppt) {
    mppt = init_mppt;

    mppt->power = 0;
    mppt->prev_power = 0;
    mppt->voltage = 0;
    mppt->prev_voltage = 0;
    mppt->current = 0;
    mppt->prev_current = 0;
    mppt->pwm_dc = 0;
}

void mppt_run_preturb() {
    if (mppt->power > mppt->prev_power) {
        if (mppt->voltage > mppt->prev_voltage) {
            mppt->pwm_dc+= DUTY_CYCLE_STEP;  // Power increased because voltage increased
        } else {
            mppt->pwm_dc-= DUTY_CYCLE_STEP;  // Power increased because voltage decreased
        }

    } else if (mppt->power < mppt-> prev_power) {
        if (mppt->voltage > mppt->prev_voltage) {
            mppt->pwm_dc+= DUTY_CYCLE_STEP;  // Power decreased because voltage increased
        } else {
            mppt->pwm_dc-= DUTY_CYCLE_STEP;  // Power decreased because voltage decreased
        }
    }

    mppt->prev_power = mppt->power;
    mppt->prev_voltage = mppt->voltage;
    mppt->prev_current = mppt->current;
}

void mppt_run_incremental_conductance() {
    int32_t d_voltage = (int32_t)mppt->voltage - (int32_t)mppt->prev_voltage;
    int32_t d_current = (int32_t)mppt->current - (int32_t)mppt->prev_current;
    // Instantaneous Conductance = Current/Voltage
    // Incremental Conductance = dCurrent / dVoltage (The two derivatives divded)
    int32_t dp_dv = (d_current / d_voltage) - ((int32_t)mppt->current / (int32_t)mppt->voltage);

    if (d_voltage != 0) {
        if (dp_dv > 0) {
            mppt->pwm_dc += DUTY_CYCLE_STEP;  // Increase PWM
        } else if (dp_dv < 0) {
            mppt->pwm_dc -= DUTY_CYCLE_STEP;  // Decrease PWM
        }
    }

    mppt->prev_voltage = mppt->voltage;
    mppt->prev_current = mppt->current;
}

void mppt_run_constant_voltage() {}  // TODO: do this

void mppt_stop() {
    mppt->algorithm = MPPT_STOPPED;
    mppt->pwm_dc = 0;
    // Call function to stop pwm
}

void mppt_run() {
    switch (mppt->algorithm) {
        case MPPT_PERTURB_OBSERVE:
            mppt_run_preturb();
            break;
        case MPPT_INCREMENTAL_CONDUCTANCE:
            mppt_run_incremental_conductance();
            break;
        case MPPT_CONSTANT_VOLTAGE:
            mppt_run_constant_voltage();
            break;
        case MPPT_STOPPED:
            mppt_stop();
            break;
        case MPPT_UNITIALIZED:
            mppt_stop();
            break;
        default:
            break;
    }
}

void mppt_set_algorithm(MPPTAlgorithm algo) {
    mppt->algorithm = algo;
}

void mppt_set_pwm(uint16_t pwm_dc) {
    mppt->pwm_dc = pwm_dc;
}
