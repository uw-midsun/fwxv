#include "ads1115.h"     // For ADC (Analog to Digital Converter) functions
#include "delay.h"       // Include delay functions
#include "gpio.h"        // Include GPIO (General-Purpose Input/Output) functions for LED toggle
#include "i2c.h"         // For I2C (Communication Protocol) initialization
#include "log.h"         // For debug logging (printing messages)
#include "tasks.h"       // For task handling functions

// Define the address of the red LED pin (used for visual output)
GpioAddress led_addr = {
  .port = GPIO_PORT_B,  // Use GPIO Port B
  .pin = 3,             // Use pin 3 (connected to red LED)
};

// Define the ALRT/RDY pin from ADC (optional use)
GpioAddress ready_pin = {
  .port = GPIO_PORT_B,  // Use GPIO Port B
  .pin = 0,             // Use pin 0 (connected to ALRT/RDY pin of ADC)
};

// Define a task that toggles the LED and reads voltage from the ADC
TASK(toggle_led_task, TASK_STACK_512) {   // Create a task with a stack size of 512 bytes
 
  // Set up LED pin as output and initialize it to LOW (off)
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  // Configure the ADC settings
  ADS1115_Config config = {
    .handler_task = toggle_led_task,           // Set the task to run when ADC is ready (optional)
    .i2c_addr = ADS1115_ADDR_GND,              // Set ADC I2C address (ADDR pin connected to ground)
    .i2c_port = I2C_PORT_1,                    // Use I2C Port 1
    .ready_pin = &ready_pin,                   // Set the ready pin (optional)
  };

  // Initialize the ADC hardware (setup config register)
  ads1115_init(&config, ADS1115_ADDR_GND, &ready_pin);  // Start the ADC with the given settings

  while (true) {  // Repeat forever

    // Toggle the LED state
    gpio_toggle_state(&led_addr); // Change the LED from on to off or vice versa

    // Read the voltage from ADC Channel 0
    float voltage = 0.0f; // Variable to store the voltage reading

    StatusCode status = ads1115_read_converted(&config, ADS1115_CHANNEL_0, &voltage); // Read voltage from ADC channel 0

    // Log the voltage value or error
    if (status == STATUS_CODE_OK) { // If the read was successful
      LOG_DEBUG("Voltage reading: %.3f V\n", voltage);  // Print the voltage value
    } else {
      LOG_DEBUG("Failed to read from ADC!\n");  // Print an error message
    }

    // Wait 1 second before next reading
    delay_ms(1000); // Pause for 1000 milliseconds (1 second)
  }
}

int main() {  // Program starts here

  // Initialize all drivers needed by the project
  gpio_init();      // Set up the GPIO system (for pins)
  log_init();       // Set up logging (for printing debug messages)

  // I2C configuration struct
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,               // Set I2C speed to standard (100 kHz)
    .scl = { .port = GPIO_PORT_B, .pin = 11 }, // Set clock pin (SCL) to port B, pin 11
    .sda = { .port = GPIO_PORT_B, .pin = 10 }, // Set data pin (SDA) to port B, pin 10
  };

  // Initialize I2C on port 1 with above settings
  i2c_init(I2C_PORT_1, &i2c_settings);  // Start I2C communication with these settings

  // Setup and start task system
  tasks_init();                                          // Initialize task scheduler
  tasks_init_task(toggle_led_task, TASK_PRIORITY(1), NULL); // Register the toggle task
  tasks_start();                                         // Start running tasks

  return 0; // Should never return (program should keep running)
}
