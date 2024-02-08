#include "operation_listener.h"

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