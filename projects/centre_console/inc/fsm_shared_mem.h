#pragma once
#include "fsm.h"
#include "log.h"
#include "semaphore.h"

typedef struct FSMStorage {
  StateId power_state;
  StatusCode error_code;
  Mutex mutex;
} FSMStorage;
