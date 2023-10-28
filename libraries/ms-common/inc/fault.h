#include "status.h"
#include "stdint.h"

#define WATCHDOG_MAX_FAULTS 5

typedef void (*FaultHandler)(void);

typedef struct {
  // Faults to watch
  // Faults should always be zero for no fault and non-zero when action is needed
  uint8_t faults[WATCHDOG_MAX_FAULTS];
  uint8_t num_faults;

  // Messages to watch
  // Messages must kick watchdog every FAULT_CYCLES, otherwise trigger fault
  uint8_t watching[WATCHDOG_MAX_FAULTS];
  uint8_t num_watching;

  uint8_t missed_cycles[WATCHDOG_MAX_FAULTS];

  FaultHandler handler;

} FaultStorage;

StatusCode fault_init(FaultStorage *storage);