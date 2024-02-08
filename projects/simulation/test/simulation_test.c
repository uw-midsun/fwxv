#include "operation_listener.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "gpio.h"
#include "gpio_it.h"
#include "log.h"

#define PORT 2520

int main() {
  int socketfd;
  struct sockaddr_in server_addr;
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    LOG_DEBUG("Socket error: %d \n", socketfd);
    return 0;
  }
  LOG_DEBUG("Socket successful\n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    LOG_DEBUG("Connect error\n");
    return 0;
  }
  LOG_DEBUG("Connect successful\n");
  const char *test1 = "0: 3, 2, 16, 0\n";
  const char *test2 = "1: 2, 2, 16\n";
  const char *test3 = "2: 2, 2, 16\n";
  const char *test4 = "3: 3, 2, 16, 3\n";
  const char *test5 = "4: 3, 1, 5, hello\n";
  const char *test6 = "5: 3, 1, 5, hello\n";
  const char *test7 = "6: 3, 2, 5, hello\n";
  const char *test8 = "7: 3, 2, 16, 0";
  send(socketfd, test1, strlen(test1), 0);
  send(socketfd, test2, strlen(test2), 0);
  send(socketfd, test3, strlen(test3), 0);
  send(socketfd, test4, strlen(test4), 0);
  send(socketfd, test5, strlen(test5), 0);
  send(socketfd, test6, strlen(test6), 0);
  send(socketfd, test7, strlen(test7), 0);
  send(socketfd, test8, strlen(test8), 0);
  close(socketfd);
  return 0;
}