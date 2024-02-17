#include "operation_listener.h"
#include "spi.h"

bool s_keep_alive = true;

void gpio_set_sim(char buffer[]) {
  int param1, param2, param3;
  LOG_DEBUG("GPIO_SET\n");
  if (length != 3) {
    LOG_DEBUG("Invalid length: %d\n", length);
    return;
  }

  sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
  if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16 || param3 > 1) {
    LOG_DEBUG("Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
    return;
  }

  GpioAddress GPIO_SET_ADDR = { .port = param1, .pin = param2 };
  gpio_set_state(&GPIO_SET_ADDR, param3);
}

void gpio_toggle_sim(int length, char buffer[]) {
  int param1, param2;
  LOG_DEBUG("GPIO_TOGGLE\n");
  if (length != 2) {
    LOG_DEBUG("Invalid length: %d\n", length);
    return;
  }
  sscanf(buffer, "%d, %d, %d", &param1, &param2);
  if (param1 < 0 || param2 < 0 || param1 > 6 || param2 > 16) {
    LOG_DEBUG("Invalid param, 1: %d, 2: %d\n", param1, param2);
    return;
  }

  GpioAddress GPIO_TOGGLE_ADDR = { .port = param1, .pin = param2 };
  gpio_toggle_state(&GPIO_TOGGLE_ADDR);
}

void gpio_it_trigger_sim(int length, char buffer[]) {
  LOG_DEBUG("GPIO_IT_TRIGGER\n");
  int param1, param2;
  if (length != 2) {
    LOG_DEBUG("Invalid length: %d\n", length);
    return;
  }
  sscanf(buffer, "%d, %d", &param1, &param2);
  if (param1 < 0 || param2 < 0 || param1 > 6 || param2 > 16) {
    LOG_DEBUG("Invalid param, 1: %d, 2: %d\n", param1, param2);
    return;
  }

  GpioAddress IT_TRIGGER_ADDR = { .port = param1, .pin = param2 };
  gpio_it_trigger_interrupt(&IT_TRIGGER_ADDR);
}

void adc_set_reading_sim(int length, char buffer[]) {
  LOG_DEBUG("ADC_SET_READING\n");
  int param1, param2, param3;
  if (length != 3) {
    LOG_DEBUG("Invalid length: %d\n", length);
    return;
  }

  sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
  if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16) {
    LOG_DEBUG("Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
    return;
  }

  GpioAddress ADC_SET_READING_ADDR = { .port = param1, .pin = param2 };
  adc_set_reading(ADC_SET_READING_ADDR, param3);
}

void i2c_set_reading_sim(int length, char buffer[]) {
  int param1, param2;
  LOG_DEBUG("I2C_SET_READING\n");
  if (length != 3) {
    LOG_DEBUG("Invalid length: %d\n", length);
    return;
  }

  sscanf(buffer, "%d, %d, %[^\n\t]", &param1, &param2, buffer);
  if (param1 < 0 || param2 < 0 || param1 > 1) {
    LOG_DEBUG("Invalid param, param1: %d, param2: %d\n", param1, param2);
    return;
  }

  I2CPort i2c_port = param1;
  i2c_set_data(i2c_port, (uint8_t *)&buffer, (size_t)param2);
}

void spi_set_rx_sim(int length, char buffer[]) {
  LOG_DEBUG("SPI_SET_RX\n");
  int param1;
  if (length != 2) {
    LOG_DEBUG("Invalid length: %d\n", length);
    return;
  }

  sscanf(buffer, "%d, %[^\n\t]", &param1, buffer);
  if (param1 < 0) {
    LOG_DEBUG("Invalid param, param1: %d", param1);
    return;
  }

  spi_set_rx((uint8_t *)&buffer, param1);
}

void sim_init(int sock_num) {
  char buffer[2024];
  LOG_DEBUG("Simulation thread started\n");
  int res;
  int operation, length;
  char rcv[1];
  int p;
  Operations input;

  while (s_keep_alive) {
    res = 0;
    memset(buffer, 0, sizeof(buffer));
    p = 0;
    LOG_DEBUG("Listening\n");
    while (s_keep_alive) {
      memset(rcv, 0, sizeof(rcv));
      res = recv(sock_num, rcv, 1, 0);
      if (res == 1) {
        if (rcv[0] != '\n') {
          buffer[p] = rcv[0];
          p++;
        } else {
          break;
        }
      }
    }
    LOG_DEBUG("Message received!\n");
    sscanf(buffer, "%d: %d, %[^\n\t]", &operation, &length, buffer);
    if (operation < 0 || operation > 6) {
      LOG_DEBUG("Invalid operation: %d\n", operation);
      continue;
    }

    input = operation;
    switch (input) {
      case GPIO_SET:
        gpio_set_sim(length, buffer) break;
      case GPIO_TOGGLE:
        gpio_toggle_sim(length, buffer);
        break;
      case GPIO_IT_TRIGGER:
        gpio_it_trigger_sim(length, buffer);
        break;
      case ADC_SET_READING:
        adc_set_reading_sim(length, buffer);
        break;
      case I2C_SET_READING:
        i2c_set_reading_sim(length, buffer);
        break;
      case SPI_SET_RX:
        spi_set_rx_sim(length, buffer);
        break;
      case UART:
        LOG_DEBUG("UART\n");
        break;
      default:
        LOG_DEBUG("UNRECOGNIZED OPERATION: %d\n", input);
    }
    usleep(1000000);
  }
}

int main(int argc, char *argv[]) {
  LOG_DEBUG("Operation listener thread started\n");
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    LOG_DEBUG("Socket error: %d", socketfd);
    return 0;
  }
  LOG_DEBUG("Socket successful\n");
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2520);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int status = bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (status < 0) {
    LOG_DEBUG("Bind failed: %d\n", status);
    return 0;
  }
  LOG_DEBUG("Bind successful\n");
  if (listen(socketfd, 5) < 0) {
    LOG_DEBUG("Listen failed\n");
    return 0;
  }
  LOG_DEBUG("Listen successful\n");
  int newsockfd = accept(socketfd, NULL, NULL);
  if (newsockfd < 0) {
    LOG_DEBUG("Accept failed: %d \n", newsockfd);
    return 0;
  }
  LOG_DEBUG("Accept successful\n");

  sim_init(newsockfd);
  close(newsockfd);
  close(socketfd);
}
