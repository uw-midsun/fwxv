#include <stdio.h>

#include "log.h"
#include "mppt.h"

MPPTData mppt = { .algorithm = MPPT_PERTURB_OBSERVE };

int main() {
  log_init();
  mppt_init(&mppt);

  while (1) {
    mppt_run();
  }

  LOG_DEBUG("exiting main?");
  return 0;
}
