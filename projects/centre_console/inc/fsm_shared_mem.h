#pragma once
#include "fsm.h"

typedef struct FSMStorage {
  StateId power_state;
  StatusCode error_code;
} FSMStorage;
