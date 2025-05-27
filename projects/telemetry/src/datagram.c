#include "datagram.h"

#include "delay.h"
#include "log.h"
#include "telemetry.h"

#define FLIP_ENDIANESS_2BYTES(val) (((val & 0xFFU) << 8U) | ((val >> 8U) & 0xFFU))

void decode_can_message(Datagram *datagram, CanMessage *msg) {
  datagram->start_frame = DATAGRAM_START_FRAME;
  datagram->id = FLIP_ENDIANESS_2BYTES(msg->id.raw);
  datagram->dlc = msg->dlc;

  for (size_t i = 0; i < msg->dlc; ++i) {
    datagram->data[i] = msg->data_u8[i];
  }

  datagram->data[msg->dlc] = DATAGRAM_END_FRAME;
}

void log_decoded_message(Datagram *datagram) {
  delay_ms(5);
  // LOG_DEBUG("Start frame: 0x%0x. End frame: 0x%0x\n", datagram->start_frame,
            // datagram->data[datagram->dlsc]);
  LOG_DEBUG("Message ID: 0x%0x\n", datagram->id);
  LOG_DEBUG("Datagram DLC: 0x%0x\n", datagram->dlc);

  LOG_DEBUG("Data:\n");
  for (size_t i = 0; i < datagram->dlc; ++i) {
    delay_ms(5);
    // LOG_DEBUG("Byte %d: 0x%0x\n", i, datagram->data[i]);
  }
}
