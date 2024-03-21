#include "ltc_afe.h"

#include "ltc_afe_impl.h"

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  status_ok_or_return(ltc_afe_impl_init(afe, settings));
  return STATUS_CODE_OK;
}