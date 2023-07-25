/*
Read SocketCAN to understand this file
https://www.kernel.org/doc/Documentation/networking/can.txt
*/
#include "can_hw.h"

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

// TODO: get rid of extra includes
#include <errno.h>

#include "log.h"

#define CAN_HW_MAX_FILTERS CAN_QUEUE_SIZE
#define CAN_HW_TX_QUEUE_LEN 8
// Check for thread exit once every 10ms
#define CAN_HW_THREAD_EXIT_PERIOD_US 10000

typedef struct CanHwSocketData {
  int can_fd;
  struct can_frame rx_frame;
  bool rx_frame_valid;
  // Queue rx_queue;
  // struct can_frame tx_frames[CAN_HW_TX_QUEUE_LEN];
  struct can_filter filters[CAN_HW_MAX_FILTERS];
  size_t num_filters;
  uint32_t delay_us;
  int loopback;
} CanHwSocketData;

static pthread_t s_rx_pthread_id;

static bool s_keep_alive = true;

static CanHwSocketData s_socket_data = { .can_fd = -1 };

static uint32_t prv_get_delay(CanHwBitrate bitrate) {
  const uint32_t delay_us[NUM_CAN_HW_BITRATES] = {
    1000,  // 125 kbps
    500,   // 250 kbps
    250,   // 500 kbps
    125,   // 1 mbps
  };

  return delay_us[bitrate];
}

static SemaphoreHandle_t s_prv_can_tx_sem_handle;
static StaticSemaphore_t s_prv_can_tx_sem;

static void *prv_rx_thread(void *arg) {
  LOG_DEBUG("CAN HW RX thread started\n");

  CanQueue *rx_queue = arg;
  CanMessage rx_msg = { 0 };

  // Using poll
  struct pollfd pfd;
  pfd.fd = s_socket_data.can_fd;
  pfd.revents = 0;
  pfd.events = POLLIN;

// TODO: will consider not having a thread if we're sending messages directly to rx queue
// but will use socket anyway
// Cons: with a network socket we could potentially connect this another application
  // if (s_socket_data.loopback) {
  //   return NULL;
  // }

  // Mutex is unlocked when the thread should exit
  while (s_keep_alive) {
    // Have infinite timeout
    // poll will constant be interrupted and restarted due to the way
    // FreeRTOS sends signals to tasks. No way to automatically restart it so,
    // just constant polling
    int res = poll(&pfd, 1, -1);

    if (res == -1)
    {
        // Have to do this since poll can't  be restarted
        // https://unix.stackexchange.com/questions/509375/what-is-interrupted-system-call
        if (errno == EINTR)
          continue;
        s_keep_alive = false;
    }
    else
    {
      if (pfd.revents & POLLIN)
      {
        // TODO: May need an read_all function? Maybe maybe
        int bytes =
            read(s_socket_data.can_fd, &s_socket_data.rx_frame, sizeof(s_socket_data.rx_frame));
        s_socket_data.rx_frame_valid = (bytes != -1);

        // TODO: push to the rx queue here
        // should also have sem to push onto queue
        // also error if queue is full
        if (s_socket_data.rx_frame_valid)
        {
          // TODO: go through hw_filters here to get rid of messages
          // TODO: I should check if they return status code ok or not
          can_hw_receive(&rx_msg.id.raw, (bool*) &rx_msg.extended, &rx_msg.data, &rx_msg.dlc);
          can_queue_push(rx_queue, &rx_msg);

          // For ensuring tx has succeeded
          BaseType_t ret = xSemaphoreGive(s_prv_can_tx_sem_handle);
          if (ret == pdFALSE) {
            LOG_CRITICAL("Failed to give s_prv_can_tx_sem_handle!");
          }
        }

        // Limit how often we can receive messages to simulate bus speed
        usleep(s_socket_data.delay_us);
      }
    }
  }

  return NULL;
}

StatusCode can_hw_init(const CanQueue* rx_queue, const CanSettings *settings) {
  // In case socket exists or function called twice
  if (s_socket_data.can_fd != -1) {
    // LOG_DEBUG("Exiting CAN HW\n");

    // Request threads to exit
    close(s_socket_data.can_fd);

    // Close RX thread
    s_keep_alive = false;
    pthread_join(s_rx_pthread_id, NULL);

    // TODO: need kill command here to destory all Tasks
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: socket exists. Aborting");
  }

  // Initialization
  memset(&s_socket_data, 0, sizeof(s_socket_data));
  s_socket_data.delay_us = prv_get_delay(settings->bitrate);
  s_socket_data.loopback = settings->loopback;

  // Initialize socket
  s_socket_data.can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s_socket_data.can_fd == -1) {
    LOG_DEBUG("CAN HW: Failed to open SocketCAN socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to open socket");
  }

// Will consider this. Faking loopback and directly pushing onto rx queue
  // Rather than using real loopback, we short-circuit the socket manually to improve determinism
  // in tests by eliminating varying network delay.
  // int no_loopback = (int)false;
  // if (
  //   setsockopt(s_socket_data.can_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &no_loopback,
  //                sizeof(no_loopback)) < 0) {
  //   // LOG_CRITICAL("CAN HW: Failed to set loopback mode on socket\n");
  //   return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set loopback mode on socket");
  // }
  // Set loopback options
  // Technically loopback is enabled by default, but lets just do it explicitly here
  if (setsockopt(
          s_socket_data.can_fd,
          SOL_CAN_RAW,
          CAN_RAW_LOOPBACK,
          &s_socket_data.loopback,
          sizeof(s_socket_data.loopback)) < 0)
  {
    LOG_DEBUG("CAN HW: Failed to set loopback mode on socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set loopback mode on socket");
  }
  if (setsockopt(
          s_socket_data.can_fd,
          SOL_CAN_RAW,
          CAN_RAW_RECV_OWN_MSGS,
          &s_socket_data.loopback,
          sizeof(s_socket_data.loopback)) < 0)
  {
    LOG_DEBUG("CAN HW: Failed to set recv own msg on socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set recv own msg on socket");
  }

  // Set non-blocking socket
  // TODO: Why do I need to do this? If it's blocking then maybe I can just
  // block on read() and not use poll()
  if (fcntl(s_socket_data.can_fd, F_SETFL, O_NONBLOCK) < 0)
  {
    LOG_DEBUG("CAN HW: Failed to set non-blocking socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set non-blocking socket");
  }

  // Setting interface index
  struct ifreq ifr = {0};
  snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", CAN_HW_DEV_INTERFACE);
  if (ioctl(s_socket_data.can_fd, SIOCGIFINDEX, &ifr) < 0)
  {
    LOG_CRITICAL("CAN HW: Device %s not found\n", CAN_HW_DEV_INTERFACE);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Device not found");
  }

  // Bind socket
  struct sockaddr_can addr = {
      .can_family = AF_CAN,
      .can_ifindex = ifr.ifr_ifindex,
  };
  if (bind(s_socket_data.can_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    LOG_DEBUG("CAN HW: Failed to bind socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to bind socket");
  }

  // Start RX thread
  s_keep_alive = true;
  pthread_create(&s_rx_pthread_id, NULL, prv_rx_thread, rx_queue);
  s_prv_can_tx_sem_handle = xSemaphoreCreateBinaryStatic(&s_prv_can_tx_sem);
  configASSERT(s_prv_can_tx_sem_handle);

  LOG_DEBUG("CAN HW initialized on %s\n", CAN_HW_DEV_INTERFACE);

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended) {
  if (s_socket_data.num_filters >= CAN_HW_MAX_FILTERS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filters.");
  }

  uint32_t reg_mask = extended ? CAN_EFF_MASK : CAN_SFF_MASK;
  uint32_t ide = extended ? CAN_EFF_FLAG : 0;
  s_socket_data.filters[s_socket_data.num_filters].can_id = (filter & reg_mask) | ide;
  s_socket_data.filters[s_socket_data.num_filters].can_mask = (mask & reg_mask) | CAN_EFF_FLAG;
  s_socket_data.num_filters++;

  if (setsockopt(s_socket_data.can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, s_socket_data.filters,
                 sizeof(s_socket_data.filters[0]) * s_socket_data.num_filters) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set raw filters");
  }
  // LOG_DEBUG("Set the filter\n");
  // LOG_DEBUG("CAN ID: %u\n", s_socket_data.filters[s_socket_data.num_filters].can_id);
  // LOG_DEBUG("filter: %u\n", filter);
  // LOG_DEBUG("num_filters: %lu\n", s_socket_data.num_filters);

  return STATUS_CODE_OK;
}

CanHwBusStatus can_hw_bus_status(void) {
  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  uint32_t mask = extended ? CAN_EFF_MASK : CAN_SFF_MASK;
  uint32_t extended_bit = extended ? CAN_EFF_FLAG : 0;
  struct can_frame frame = { .can_id = (id & mask) | extended_bit, .can_dlc = len };
  memcpy(&frame.data, data, len);

  if (!s_socket_data.loopback) {
    // TODO: Don't think need to anything here
    // Unblock TX thread
    // sem_post(&s_tx_sem);
  } else {
    int bytes = write(s_socket_data.can_fd, &frame, sizeof(frame));

    s_socket_data.rx_frame_valid = true;

    // TODO: Add back if getting rid of SocketCAN
    // Apply filters that would normally be applied within socketcan
    
    // if (filter_match && s_socket_data.handlers[CAN_HW_EVENT_MSG_RX].callback != NULL) {
    //   s_socket_data.handlers[CAN_HW_EVENT_MSG_RX].callback(
    //       s_socket_data.handlers[CAN_HW_EVENT_TX_READY].context);
    // }
  }
  // Ensure that socket has consumed the transmission
  xSemaphoreTake(s_prv_can_tx_sem_handle, portMAX_DELAY);

  return STATUS_CODE_OK;
}

bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len) {

  if (!s_socket_data.rx_frame_valid) {
    return false;
  }

  *extended = !!(s_socket_data.rx_frame.can_id & CAN_EFF_FLAG);
  uint32_t mask = *extended ? CAN_EFF_MASK : CAN_SFF_MASK;
  *id = s_socket_data.rx_frame.can_id & mask;
  memcpy(data, s_socket_data.rx_frame.data, sizeof(*data));
  *len = s_socket_data.rx_frame.can_dlc;

  memset(&s_socket_data.rx_frame, 0, sizeof(s_socket_data.rx_frame));
  s_socket_data.rx_frame_valid = false;

  return true;
}

