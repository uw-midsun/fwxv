#include "boot_can.h"

typedef struct CanHwTiming {
  uint16_t prescaler;
  uint8_t bs1;
  uint8_t bs2;
} CanHwTiming;

// Generated settings using http://www.bittiming.can-wiki.info/
// Note that the BS1/BS2 register values are used +1, so we need to subtract 1
// from the calculated value to compenstate. The same is true for the prescaler,
// but the library subtracts 1 internally. The total time quanta is thus (BS1 +
// 1) + (BS2 + 1) + SJW (1) ~= 16 tq.
static CanHwTiming s_timing[NUM_CAN_HW_BITRATES] = {  // For 32MHz clock
  [CAN_HW_BITRATE_125KBPS] = { .prescaler = 16, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_250KBPS] = { .prescaler = 8, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_500KBPS] = { .prescaler = 4, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_1000KBPS] = { .prescaler = 2, .bs1 = 12, .bs2 = 1 }
};

static void boot_can_init_gpio() {
  GPIO_InitTypeDef gpio_cfg;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  // Init CAN RX pin
  gpio_cfg.GPIO_Pin = GPIO_Pin_11;
  gpio_cfg.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_cfg.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &gpio_cfg);

  // Init CAN TX pin
  gpio_cfg.GPIO_Pin = GPIO_Pin_12;
  gpio_cfg.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_cfg.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &gpio_cfg);
}

BootloaderError boot_can_init(const Boot_CanSettings *settings) {
  boot_can_init_gpio();
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

  CAN_DeInit(CAN1);

  CAN_InitTypeDef can_cfg;
  CAN_StructInit(&can_cfg);

  can_cfg.CAN_Mode = settings->loopback ? CAN_Mode_Silent_LoopBack : CAN_Mode_Normal;
  can_cfg.CAN_SJW = CAN_SJW_1tq;
  can_cfg.CAN_ABOM = ENABLE;
  can_cfg.CAN_BS1 = s_timing[settings->bitrate].bs1;
  can_cfg.CAN_BS2 = s_timing[settings->bitrate].bs2;
  can_cfg.CAN_Prescaler = s_timing[settings->bitrate].prescaler;
  if (CAN_Init(CAN1, &can_cfg) != CAN_InitStatus_Success) {
    return BOOTLOADER_CAN_INIT_ERR;
  }

  CAN_FilterInitTypeDef filter_cfg = {
    .CAN_FilterNumber = 0,
    .CAN_FilterMode = CAN_FilterMode_IdMask,
    .CAN_FilterScale = CAN_FilterScale_32bit,
    .CAN_FilterIdHigh = 0,
    .CAN_FilterIdLow = 0,
    .CAN_FilterMaskIdHigh = 0,
    .CAN_FilterMaskIdLow = 0,
    .CAN_FilterFIFOAssignment = 0,
    .CAN_FilterActivation = ENABLE,
  };

  CAN_FilterInit(&filter_cfg);

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  CanTxMsg tx_msg = {
    .StdId = id,
    .ExtId = id,
    .IDE = extended ? CAN_Id_Extended : CAN_Id_Standard,
    .DLC = len,
  };

  memcpy(tx_msg.Data, data, len);

  CAN_Transmit(CAN1, &tx_msg);
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_receive(Boot_CanMessage *msg) {
  uint8_t fifo_status =
      (CAN_MessagePending(CAN1, CAN_FIFO0) != 0) | (CAN_MessagePending(CAN1, CAN_FIFO1) != 0) << 1;

  if (fifo_status == 0) {
    // No messages available
    return BOOTLOADER_CAN_INIT_ERR;
  }

  uint8_t fifo = (fifo_status & 1) ? CAN_FIFO0 : CAN_FIFO1;

  CanRxMsg rx_msg = { 0 };
  CAN_Receive(CAN1, fifo, &rx_msg);

  msg->extended = (rx_msg.IDE == CAN_Id_Extended);
  msg->id = msg->extended ? rx_msg.ExtId : rx_msg.StdId;
  msg->dlc = rx_msg.DLC;
  memcpy(&msg->data, rx_msg.Data, sizeof(*rx_msg.Data) * rx_msg.DLC);

  return BOOTLOADER_ERROR_NONE;
}
