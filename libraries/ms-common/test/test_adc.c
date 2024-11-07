#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

// temps in kelvin
#define ADC_INVALID_UNDER_TEMP 253
#define ADC_INVALID_OVER_TEMP 373

static const GpioAddress s_address[] = {
  { GPIO_PORT_A, 0 },
  { GPIO_PORT_A, 1 },
  { GPIO_PORT_A, 2 },
};

static const GpioAddress s_invalid_pin = { NUM_GPIO_PORTS, NUM_ADC_CHANNELS };
static const GpioAddress s_empty_pin = { GPIO_PORT_A, 3 };

// Check multiple samples to ensure they are within the correct range
void prv_adc_check_range_pin(GpioAddress address) {
  uint16_t raw_reading = 0;
  uint16_t conv_reading = 0;

  for (uint8_t i = 0; i < 12; i++) {
    adc_read_raw(address, &raw_reading);
    TEST_ASSERT_TRUE(raw_reading <= 4095);
    adc_read_converted(address, &conv_reading);
    TEST_ASSERT_TRUE(conv_reading <= 3000);
  }
}

void setup_test() {
  log_init();
  gpio_init();
  interrupt_init();
}

void teardown_test(void) {
  // wait for end of configMAX_SYSCALL_INTERRUPT_PRIORITY
  // clear
}

void test_adc_init(void) {
  GpioAddress address = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };
  gpio_init_pin(&address, GPIO_ANALOG, GPIO_STATE_LOW);

  // adc_init cannot be called with no channels
  TEST_ASSERT_NOT_OK(adc_init());

  // Add channel and init
  TEST_ASSERT_OK(adc_add_channel(address));
  TEST_ASSERT_NOT_OK(adc_add_channel(address));
  TEST_ASSERT_OK(adc_init());

  // Channels can't be init'd after adc_init
  TEST_ASSERT_NOT_OK(adc_add_channel(address));
  // adc_init can't be called twice
  TEST_ASSERT_NOT_OK(adc_init());
  adc_deinit();
}

TASK(adc_read1, TASK_STACK_512) {
  GpioAddress address = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };
  uint16_t reading;
  TEST_ASSERT_OK(adc_read_raw(address, &reading));
  LOG_DEBUG("read %d from adc\n", reading);
}

TEST_IN_TASK
void test_adc_pin_to_channel_conversion() {
  GpioAddress address = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };
  gpio_init_pin(&address, GPIO_ANALOG, GPIO_STATE_LOW);
  TEST_ASSERT_OK(adc_add_channel(address));
  TEST_ASSERT_OK(adc_init());
  tasks_init_task(adc_read1, TASK_PRIORITY(1), NULL);
  uint16_t reading;
  TEST_ASSERT_OK(adc_read_raw(address, &reading));
  LOG_DEBUG("read %d from adc\n", reading);
  delay_ms(100);
  TEST_ASSERT_OK(adc_read_raw(address, &reading));
  LOG_DEBUG("read %d from adc\n", reading);
  delay_ms(100);
}

// TEST_IN_TASK
// void test_adc_pin_to_channel_conversion() {
//   GpioAddress address = {
//     .port = GPIO_PORT_A,
//     .pin = 0,
//   };
//   gpio_init_pin(&address, GPIO_ANALOG);
//   TEST_ASSERT_OK(adc_add_channel(address));
//   TEST_ASSERT_OK(adc_init());
//
//   for (address.pin = 0; address.pin < 8; ++address.pin) {
//     TEST_ASSERT_OK(adc_set_channel(address, true));
//   }
//   TEST_ASSERT_NOT_OK(adc_set_channel(address, true));
//
//   address.port = GPIO_PORT_B;
//   for (address.pin = 0; address.pin < 2; ++address.pin) {
//     TEST_ASSERT_OK(adc_set_channel(address, true));
//   }
//   TEST_ASSERT_NOT_OK(adc_set_channel(address, true));
//
//   address.port = GPIO_PORT_C;
//   for (address.pin = 0; address.pin < 6; ++address.pin) {
//     TEST_ASSERT_OK(adc_set_channel(address, true));
//   }
//   TEST_ASSERT_NOT_OK(adc_set_channel(address, true));
//
//   TEST_ASSERT_OK(adc_set_channel(ADC_BAT, true));
//   TEST_ASSERT_OK(adc_set_channel(ADC_REF, true));
//   TEST_ASSERT_OK(adc_set_channel(ADC_TEMP, true));
// }

// void test_adc_read_temp() {
//   adc_init();
//   adc_set_channel(ADC_TEMP, true);
//
//   uint16_t reading;
//   adc_read_converted(ADC_TEMP, &reading);
//
//   // ensure value is within reason
//   TEST_ASSERT_TRUE(ADC_INVALID_UNDER_TEMP < reading);
//   TEST_ASSERT_TRUE(ADC_INVALID_OVER_TEMP > reading);
// }
//
// void test_pin_set_channel(void) {
//   TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel(s_invalid_pin, true));
//
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(s_address[i], false));
//   }
//
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(s_address[i], true));
//   }
// }
//
// void test_pin_set_notification(void) {
//   TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_register_event(s_invalid_pin, test_task, 0));
//
//   TEST_ASSERT_OK(adc_set_channel(s_empty_pin, false));
//   TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_event(s_empty_pin, test_task, 0));
//
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_event(s_address[i], test_task, i));
//   }
// }
//
// void test_pin_read_single(void) {
//   // Check that both the raw readings and converted readings are within the
//   // expected range
//   adc_init();
//
//   adc_set_channel(s_address[0], true);
//   adc_register_event(s_address[0], test_task, 0);
//
//   prv_adc_check_range_pin(s_address[0]);
// }
//
// void test_pin_read_continuous(void) {
//   // Check that both the raw readings and converted readings are within the
//   // expected range
//   adc_init(ADC_MODE_CONTINUOUS);
//
//   adc_set_channel(s_address[0], true);
//   adc_register_event(s_address[0], test_task, 0);
//
//   prv_adc_check_range_pin(s_address[0]);
// }
//
// // test to help with other tests
// void test_adc_mock_reading() {
//   adc_init();
//   GpioAddress address = { .port = GPIO_PORT_A, .pin = 0 };
//
//   adc_set_channel(address, true);
//
//   uint16_t reading = 0;
//   adc_read_raw(address, &reading);
//   TEST_ASSERT_TRUE(reading != 0);
// }
//
// TEST_IN_TASK
// void test_pin_single() {
//   uint16_t reading;
//   // Initialize the ADC to single mode and configure the channels
//   adc_init();
//
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     adc_set_channel(s_address[i], true);
//   }
//
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     adc_register_event(s_address[i], test_task, i);
//   }
//
//   // Callbacks must not run in single mode unless a read occurs
//   uint32_t notification = 0;
//   notify_get(&notification);
//   TEST_ASSERT_FALSE(notification);
//
//   // Ensure that the conversions happen once adc_read_value is called
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(s_address[i], &reading));
//   }
//
//   TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_read_raw(s_invalid_pin, &reading));
//   TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_read_raw(s_empty_pin, &reading));
//
//   TEST_ASSERT_TRUE(reading < 4095);
//   TEST_ASSERT_TRUE(false);
// }
//
// TEST_IN_TASK
// void test_pin_continuous() {
//   // Initialize ADC and check that adc_init() can properly reset the ADC
//   adc_init(ADC_MODE_CONTINUOUS);
//
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     adc_set_channel(s_address[i], true);
//   }
//   for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
//     adc_register_event(s_address[i], test_task, i);
//   }
//
//   vTaskDelay(60);
//
//   uint32_t notification = 0;
//   notify_get(&notification);
//   TEST_ASSERT_NOT_EQUAL(notification, 0);
// }
