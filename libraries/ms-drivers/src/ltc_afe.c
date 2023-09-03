#include "ltc_afe.h"

#include "ltc_afe_fsm.h"
#include "ltc_afe_impl.h"

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  status_ok_or_return(ltc_afe_impl_init(afe, settings));
  return ltc_afe_fsm_init(&afe->fsm, afe);
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return ltc_afe_impl_toggle_cell_discharge(afe, cell, discharge);
}
