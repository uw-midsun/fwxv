#include "diskio.h"
#include "ff.h"
#include "gpio.h"
#include "status.h"

DSTATUS data_logger_init();

FRESULT data_logger_mount();

FRESULT data_logger_write(const char *data);

FRESULT data_logger_stop();