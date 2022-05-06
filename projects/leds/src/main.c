#include <stdbool.h>

#include "gpio.h"
#include "log.h"
#include "misc.h"

static const GpioAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 5 },   //
    { .port = GPIO_PORT_B, .pin = 4 },   //
    { .port = GPIO_PORT_B, .pin = 3 },   //
    { .port = GPIO_PORT_A, .pin = 15 },  //
};


int main(void) {
    LOG_DEBUG("Blinking LEDs...\n");

    const GpioSettings led_settings = {
        .direction = GPIO_DIR_OUT,        //
        .state = GPIO_STATE_HIGH,         //
        .alt_function = GPIO_ALTFN_NONE,  //
        .resistor = GPIO_RES_NONE,        //
    };

    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
        gpio_init_pin(&leds[i], &led_settings);
    }

    while (true) {
#ifdef MS_PLATFORM_X86
        LOG_DEBUG("blink\n");
#endif // MS_PLATFORM_X86
        for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
            gpio_toggle_state(&leds[i]);
        }
    }

    return 0;
}
