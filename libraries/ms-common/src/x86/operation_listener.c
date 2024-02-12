#include "operation_listener.h"

bool s_keep_alive = true;

// void *sim_thread(void *arg) {
//   sim_init(atoi(arg));
//   return NULL;
// }

void sim_init(int sock_num) {
  char buffer[BUFFER_SIZE];
  LOG_DEBUG("Simulation thread started\n");
  int res;
  int operation, length, param1, param2, param3;
  char rcv[1];
  char sen[50];
  int p;
  Operations input;

  while (s_keep_alive) {
    res = 0;
    memset(buffer, 0, BUFFER_SIZE);
    p = 0;
    LOG_DEBUG("Listening\n");
    
    while (s_keep_alive) {
      memset(rcv, 0, sizeof(rcv));
      res = recv(sock_num, rcv, 1, MSG_WAITALL);
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
    param1 = 0;
    param2 = 0;
    param3 = 0;
    sscanf(buffer, "%d: %d, %[^\n\t]", &operation, &length, buffer);
    if (operation < 0 || operation > 7) {
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
        LOG_DEBUG("PARAM1 %d, PARAM2 %d, PARAM3 %d\n", param1, param2, param3);
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
      case GPIO_READ:
        LOG_DEBUG("GPIO READ\n");
        if (length != 0) {
          LOG_DEBUG("Invalid length for operation: %d Length: %d\n", operation, length);
          break;
        }

        GpioState state;
        for (int port = 0; port < 2; port++) {
          for (int pin = 0; pin < 16; pin++) {
            GpioAddress ADDR = { .port = port, .pin = pin};
            state = gpio_get_state(&ADDR, &state);
            sprintf(sen, "GPIO_PIN %d%d | STATE = %d\n", port, pin, state);
            LOG_DEBUG("GPIO_PIN %d%d | STATE = %d\n", port, pin, state);
            res = send(sock_num, sen, strlen(sen), 0);
            if (res < 0) {
              LOG_WARN("ERROR: GPIO READ failed");
              break;
            }
          }
        }

        break;
      default:
        LOG_DEBUG("UNRECOGNIZED OPERATION: %d\n", input);
    }
    usleep(1000000);
  }
}

void x86_main_init(int socket_num) {
  pthread_t thread_id;
  LOG_DEBUG("Operation listener thread started\n");
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
    LOG_DEBUG("Bind failed: %d. Socket_num: %d \n", status, socket_num);
    return;
  }
  LOG_DEBUG("Bind successful\n");
  if (listen(socketfd, 5) < 0) {
    LOG_DEBUG("Listen failed\n");
    return;
  }
  LOG_DEBUG("Listen successful\n");
  LOG_DEBUG("%d \n", socketfd);
  int newsockfd = accept(socketfd, NULL, NULL);
  if (newsockfd < 0) {
    LOG_DEBUG("Accept failed: %d \n", newsockfd);
    return;
  }
  LOG_DEBUG("Accept successful\n");
  sim_init(newsockfd);
  // status = pthread_create(&thread_id, NULL, &sim_thread, &newsockfd);
  // if (status != 0) {
  //   LOG_WARN("THREAD CREATION FAILED");
  close(newsockfd);
  close(socketfd);
  // }
}