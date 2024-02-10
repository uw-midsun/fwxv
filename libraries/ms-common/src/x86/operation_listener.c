#include "operation_listener.h"

bool s_keep_alive = true;

void sim_init(int sock_num) {
  char buffer[BUFFER_SIZE];
  LOG_DEBUG("Simulation thread started\n");
  LOG_DEBUG("%u \n", sock_num);
  int res;
  int operation, length, param1, param2, param3;
  char rcv[1];
  int p;
  Operations input;

  while (s_keep_alive) {
    res = 0;
    memset(buffer, 0, BUFFER_SIZE);
    p = 0;
    LOG_DEBUG("Listening\n");
    
    while (s_keep_alive) {
      memset(rcv, 0, sizeof(rcv));
      res = recv(sock_num, rcv, 1, 0);
      // LOG_DEBUG("%d \n", sock_num);
      // LOG_DEBUG("%u \n", res);
      if (res == 1) {
        if (rcv[0] != '\n') {
          buffer[p] = rcv[0];
          LOG_DEBUG("%u", rcv[0]);
          p++;
        } else {
          break;
        }
      }
    }
    LOG_DEBUG("Message received!\n");
    param1 = 0;
    param2 = 0;
    param3 = 0;
    sscanf(buffer, "%d: %d, %[^\n\t]", &operation, &length, buffer);
    if (operation < 0 || operation > 6) {
      LOG_DEBUG("Invalid operation: %d\n", operation);
      continue;
    }

    input = operation;
    switch (input) {
      case GPIO_SET:
        LOG_DEBUG("GPIO_SET\n");
          if (length != 3) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
        if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16 || param3 > 1) {
          LOG_DEBUG("Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
          continue;
        }

        GpioAddress GPIO_SET_ADDR = { .port = param1, .pin = param2 };
        gpio_set_state(&GPIO_SET_ADDR, param3);
        break;
      case GPIO_TOGGLE:
        LOG_DEBUG("GPIO_TOGGLE\n");
        if (length != 2) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
        if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16) {
          LOG_DEBUG("Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
          continue;
        }

        GpioAddress GPIO_TOGGLE_ADDR = { .port = param1, .pin = param2 };
        gpio_toggle_state(&GPIO_TOGGLE_ADDR);
        break;
      case GPIO_IT_TRIGGER:
        LOG_DEBUG("GPIO_IT_TRIGGER\n");
        if (length != 2) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
        if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16) {
          LOG_DEBUG("Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
          continue;
        }

        GpioAddress IT_TRIGGER_ADDR = { .port = param1, .pin = param2 };
        gpio_it_trigger_interrupt(&IT_TRIGGER_ADDR);
        break;
      case ADC_SET_READING:
        LOG_DEBUG("ADC_SET_READING\n");
        if (length != 3) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
        if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16) {
          LOG_DEBUG("Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
          continue;
        }

        GpioAddress ADC_SET_READING_ADDR = { .port = param1, .pin = param2 };
        adc_set_reading(ADC_SET_READING_ADDR, param3);
        break;
      case I2C_SET_READING:
        LOG_DEBUG("I2C_SET_READING\n");
        if (length != 3) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        sscanf(buffer, "%d, %d, %[^\n\t]", &param1, &param2, buffer);
        if (param1 < 0 || param2 < 0 || param1 > 1) {
          LOG_DEBUG("Invalid param, param1: %d, param2: %d\n", param1, param2);
          continue;
        }

        I2CPort i2c_port = param1;
        i2c_set_data(i2c_port, (uint8_t *)&buffer, (size_t)param2);
        break;
      case SPI_SET_RX:
        LOG_DEBUG("SPI_SET_RX\n");
        if (length != 2) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        sscanf(buffer, "%d, %[^\n\t]", &param1, buffer);
        if (param1 < 0) {
          LOG_DEBUG("Invalid param, param1: %d, param2: %d\n", param1, param2);
          continue;
        }

        spi_set_rx((uint8_t *)&buffer, param1);
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

void x86_main_init(int socket_num) {
  LOG_DEBUG("Operation listener thread started\n");
  LOG_DEBUG("%d \n", socket_num);
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    LOG_DEBUG("Socket error: %d", socketfd);
    return;
  }
  LOG_DEBUG("Socket successful\n");
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(socket_num);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int status = bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (status < 0) {
    LOG_DEBUG("Bind failed: %d\n", status);
    return;
  }
  LOG_DEBUG("Bind successful\n");
  if (listen(socketfd, 5) < 0) {
    LOG_DEBUG("Listen failed\n");
    return;
  }
  LOG_DEBUG("Listen successful\n");
  int newsockfd = accept(socketfd, NULL, NULL);
  if (newsockfd < 0) {
    LOG_DEBUG("Accept failed: %d \n", newsockfd);
    return;
  }
  LOG_DEBUG("%d", newsockfd);
  LOG_DEBUG("Accept successful\n");

  sim_init(newsockfd);
  close(newsockfd);
  close(socketfd);
}