#include "operation_listener.h"

bool s_keep_alive = true;
int newsockfd;

void *sim_thread() {
  sim_init(newsockfd);
  return NULL;
}

void sim_init(int sock_num) {
  char buffer[BUFFER_SIZE];
  LOG_DEBUG("SIMULATION: Simulation thread started\n");
  int res;
  int operation, length, param1, param2, param3;
  char rcv[1];
  char sen[100];
  int p;
  Operations input;

  struct pollfd poll_fd;
  poll_fd.fd = sock_num;
  poll_fd.revents = 0;
  poll_fd.events = POLLIN;

  while (s_keep_alive) {
    res = poll(&poll_fd, 1, -1);
    if (res == -1) {
      if (errno == EINTR) continue;
      s_keep_alive = false;
      LOG_DEBUG("SIMULATION: ERROR: polling failed\n");
    } else {
      if (poll_fd.revents & POLLIN) {
        memset(buffer, NUM_OF_OPERATIONS, BUFFER_SIZE);
        p = 0;
        while (s_keep_alive) {
          res = recv(poll_fd.fd, &rcv, sizeof(rcv), 0);
          if (res == 1) {
            if (rcv[0] != '\n') {
              buffer[p] = rcv[0];
              p++;
            } else {
              break;
            }
          }
        } 
        LOG_DEBUG("SIMULATION: Message received!\n");
        param1 = 0;
        param2 = 0;
        param3 = 0;
        sscanf(buffer, "%d: %d, %[^\n\t]", &operation, &length, buffer);
        if ((operation < 0 || operation > 7 ) && (operation != NUM_OF_OPERATIONS)) {
          LOG_DEBUG("SIMULATION: Invalid operation: %d\n", operation);
          continue;
        }

        input = operation;
        switch (input) {
          case GPIO_SET:
            LOG_DEBUG("SIMULATION: GPIO_SET\n");
              if (length != 3) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
            if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16 || param3 > 1) {
              LOG_DEBUG("SIMULATION: Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
              continue;
            }
            LOG_DEBUG("SIMULATION: PARAM1 %d, PARAM2 %d, PARAM3 %d\n", param1, param2, param3);
            GpioAddress GPIO_SET_ADDR = { .port = param1, .pin = param2 };
            StatusCode status_code = gpio_set_state(&GPIO_SET_ADDR, (GpioState)param3);
            LOG_DEBUG("STATUS CODE: %d\n", status_code);
            break;
          case GPIO_TOGGLE:
            LOG_DEBUG("SIMULATION: GPIO_TOGGLE\n");
            if (length != 2) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
            if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16) {
              LOG_DEBUG("SIMULATION: Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
              continue;
            }
            GpioAddress GPIO_TOGGLE_ADDR = { .port = param1, .pin = param2 };
            gpio_toggle_state(&GPIO_TOGGLE_ADDR);
            break;
          case GPIO_IT_TRIGGER:
            LOG_DEBUG("SIMULATION: GPIO_IT_TRIGGER\n");
            if (length != 2) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            sscanf(buffer, "%d, %d", &param1, &param2);
            if (param1 < 0 || param2 < 0 || param1 > 6 || param2 > 16) {
              LOG_DEBUG("SIMULATION: Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
              continue;
            }

            GpioAddress IT_TRIGGER_ADDR = { .port = param1, .pin = param2 };
            gpio_it_trigger_interrupt(&IT_TRIGGER_ADDR);
            break;
          case ADC_SET_READING:
            LOG_DEBUG("SIMULATION: ADC_SET_READING\n");
            if (length != 3) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            sscanf(buffer, "%d, %d, %d", &param1, &param2, &param3);
            if (param1 < 0 || param2 < 0 || param3 < 0 || param1 > 6 || param2 > 16) {
              LOG_DEBUG("SIMULATION: Invalid param, 1: %d, 2: %d, 3: %d\n", param1, param2, param3);
              continue;
            }

            GpioAddress ADC_SET_READING_ADDR = { .port = param1, .pin = param2 };
            adc_set_reading(ADC_SET_READING_ADDR, param3);
            break;
          case I2C_SET_READING:
            LOG_DEBUG("SIMULATION: I2C_SET_READING\n");
            if (length != 3) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            sscanf(buffer, "%d, %d, %[^\n\t]", &param1, &param2, buffer);
            if (param1 < 0 || param2 < 0 || param1 > 1) {
              LOG_DEBUG("SIMULATION: Invalid param, param1: %d, param2: %d\n", param1, param2);
              continue;
            }

            I2CPort i2c_port = param1;
            i2c_set_data(i2c_port, (uint8_t *)&buffer, (size_t)param2);
            break;
          case SPI_SET_RX:
            LOG_DEBUG("SIMULATION: SPI_SET_RX\n");
            if (length != 3) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            sscanf(buffer, "%d, %d, %[^\n\t]", &param1, &param2, buffer);
            if (param1 < 0) {
              LOG_DEBUG("SIMULATION: Invalid param, param1: %d, param2: %d\n", param1, param2);
              continue;
            }

            spi_set_rx(param1, (uint8_t *)&buffer, param2);
            break;
          case UART:
            LOG_DEBUG("SIMULATION: UART\n");
            break;
          case GPIO_READ:
            LOG_DEBUG("SIMULATION: GPIO READ\n");
            if (length != 0) {
              LOG_DEBUG("SIMULATION: Invalid length for operation: %d Length: %d\n", operation, length);
              break;
            }

            char* gpio_mode_strings[] = {
              "ANALOG",
              "INPUT_FLOATING",
              "INPUT_PULL_DOWN",
              "INPUT_PULL_UP",
              "OUTPUT_OPEN_DRAIN",
              "OUTPUT_PUSH_PULL",
              "ALTERNATE_FUNCTION_OPEN_DRAIN",
              "ALTERNATE_FUNCTION_PUSH_PULL",
              "UNKNOWN_MODE"
            };

            for (int port = 0; port < 2; port++) {
              for (int pin = 0; pin < 16; pin++) {
                GpioAddress ADDR = { .port = port, .pin = pin};
                GpioState gpio_state;
                GpioMode gpio_mode;
                gpio_get_state(&ADDR, &gpio_state);
                gpio_get_mode(&ADDR, &gpio_mode);
                sprintf(sen, "GPIO_PIN %c%d | STATE = %s | MODE = %s\n", port == 0? 'A':'B', pin, gpio_state == 0? "LOW":"HIGH",
                gpio_mode >= 0 && gpio_mode < NUM_GPIO_MODES? gpio_mode_strings[gpio_mode] : gpio_mode_strings[NUM_GPIO_MODES]);
                res = send(sock_num, sen, strlen(sen), 0);
                if (res < 0) {
                  LOG_WARN("ERROR: GPIO READ failed");
                  break;
                }
              }
            }
            break;
          default:
            LOG_DEBUG("SIMULATION: UNRECOGNIZED OPERATION: %d\n", input);
        }
        sleep(5);
      }
    }
  }
}

void x86_main_init(int socket_num) {
  pthread_t thread_id;
  LOG_DEBUG("SIMULATION: Operation listener thread started\n");
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    LOG_DEBUG("SIMULATION: Socket error: %d", socketfd);
    return;
  }
  LOG_DEBUG("SIMULATION: Socket successful\n");
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(socket_num);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int status = bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (status < 0) {
    LOG_DEBUG("SIMULATION: Bind failed: %d. Socket_num: %d \n", status, socket_num);
    return;
  }
  LOG_DEBUG("SIMULATION: Bind successful\n");
  if (listen(socketfd, 5) < 0) {
    LOG_DEBUG("SIMULATION: Listen failed\n");
    return;
  }
  LOG_DEBUG("SIMULATION: Listen successful\n");
  LOG_DEBUG("SIMULATION: %d \n", socketfd);
  newsockfd = accept(socketfd, NULL, NULL);
  if (newsockfd < 0) {
    LOG_DEBUG("SIMULATION: Accept failed: %d \n", newsockfd);
    return;
  }
  LOG_DEBUG("SIMULATION: Accept successful\n");
  status = pthread_create(&thread_id, NULL, sim_thread, NULL);
  if (status != 0) {
    LOG_WARN("THREAD CREATION FAILED");
    close(newsockfd);
    close(socketfd);
  }
} 