#include "can_hw.h"
#include <string.h>
#include "stm32f10x_interrupt.h"
#include "log.h"
#include "stm32f10x.h"

#define CAN_HW_BASE CAN1
#define CAN_HW_NUM_FILTER_BANKS 14
#define MAX_TX_RETRIES 3
#define MAX_TX_MS_TIMEOUT 500 // CAN Messages should only be sent max every 1s

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
static CanHwTiming s_timing[NUM_CAN_HW_BITRATES] = {  // For 48MHz clock
  [CAN_HW_BITRATE_125KBPS] = { .prescaler = 24, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_250KBPS] = { .prescaler = 12, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_500KBPS] = { .prescaler = 6, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_1000KBPS] = { .prescaler = 3, .bs1 = 12, .bs2 = 1 }
};
static uint8_t s_num_filters;
static CanQueue *s_g_rx_queue;

static SemaphoreHandle_t s_can_tx_ready_sem_handle;
static StaticSemaphore_t s_can_tx_ready_sem;
static bool s_tx_full = false;

//takes 1 for filter_in, 2 for filter_out and default is 0
static int s_can_filter_en = 0;         
static uint32_t can_filters[CAN_HW_NUM_FILTER_BANKS];

static void prv_add_filter_in(uint8_t filter_num, uint32_t mask, uint32_t filter) {

  CAN_FilterInitTypeDef filter_cfg = {
    .CAN_FilterNumber = filter_num,
    .CAN_FilterMode = CAN_FilterMode_IdMask,
    .CAN_FilterScale = CAN_FilterScale_32bit,
    .CAN_FilterIdHigh = filter >> 16,
    .CAN_FilterIdLow = filter,
    .CAN_FilterMaskIdHigh = mask >> 16,
    .CAN_FilterMaskIdLow = mask,
    .CAN_FilterFIFOAssignment = (filter_num % 2),
    .CAN_FilterActivation = ENABLE,
  };

  CAN_FilterInit(&filter_cfg);
}

static void prv_add_filter_out(uint8_t filter_num, uint32_t mask, uint32_t filter) {
  can_filters[filter_num] = filter;
}

StatusCode can_hw_init(const CanQueue* rx_queue, const CanSettings *settings) {
  s_num_filters = 0;

  gpio_init_pin(&settings->tx, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->rx, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

  CAN_DeInit(CAN_HW_BASE);

  CAN_InitTypeDef can_cfg;
  CAN_StructInit(&can_cfg);

  can_cfg.CAN_Mode = settings->loopback ? CAN_Mode_Silent_LoopBack : CAN_Mode_Normal;
  can_cfg.CAN_SJW = CAN_SJW_1tq;
  can_cfg.CAN_ABOM = ENABLE;
  can_cfg.CAN_BS1 = s_timing[settings->bitrate].bs1;
  can_cfg.CAN_BS2 = s_timing[settings->bitrate].bs2;
  can_cfg.CAN_Prescaler = s_timing[settings->bitrate].prescaler;
  CAN_Init(CAN_HW_BASE, &can_cfg);

  CAN_ITConfig(CAN_HW_BASE, CAN_IT_TME, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_FMP0, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_FMP1, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_ERR, ENABLE);
  stm32f10x_interrupt_nvic_enable(USB_HP_CAN1_TX_IRQn, INTERRUPT_PRIORITY_HIGH);
  stm32f10x_interrupt_nvic_enable(USB_LP_CAN1_RX0_IRQn, INTERRUPT_PRIORITY_HIGH);
  stm32f10x_interrupt_nvic_enable(CAN1_RX1_IRQn, INTERRUPT_PRIORITY_HIGH);
  stm32f10x_interrupt_nvic_enable(CAN1_SCE_IRQn, INTERRUPT_PRIORITY_HIGH);

  // Allow all messages by default, but reset the filter count so it's
  // overwritten on the first filter
  prv_add_filter_in(0, 0, 0);
  s_num_filters = 0;

  s_g_rx_queue = rx_queue;

  // Create available mailbox sem
  s_can_tx_ready_sem_handle = xSemaphoreCreateBinaryStatic(&s_can_tx_ready_sem);
  configASSERT(s_can_tx_ready_sem_handle);
  s_tx_full = false;

  LOG_DEBUG("CAN HW initialized on %s\n", CAN_HW_DEV_INTERFACE);

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended) {
  //check if s_can_filter_en has been set
  if (s_can_filter_en == 0){
    s_can_filter_en = 1;
  }

  if (s_num_filters >= CAN_HW_NUM_FILTER_BANKS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filter banks.");
  } else if (s_can_filter_en != 1) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "CAN: CAN filter out is enabled already");
  }

  // 32-bit Filter - Identifer Mask
  // STID[10:3] | STID[2:0] EXID[17:13] | EXID[12:5] | EXID[4:0] [IDE] [RTR] 0
  size_t offset = extended ? 3 : 21;
  // We always set the IDE bit for the mask so we distinguish between standard
  // and extended
  uint32_t mask_val = (mask << offset) | (1 << 2);
  uint32_t filter_val = (filter << offset) | ((uint32_t)extended << 2);

  prv_add_filter_in(s_num_filters, mask_val, filter_val);
  s_num_filters++;
  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter_out(uint32_t mask, uint32_t filter, bool extended) {
  //check if s_can_filter_en has been set
  if (s_can_filter_en == 0){
    s_can_filter_en = 2;
  }
    if (s_num_filters >= CAN_HW_NUM_FILTER_BANKS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filter banks.");
  } else if (s_can_filter_en != 2) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "CAN: CAN filter in is enabled already");
  }

  // 32-bit Filter - Identifer Mask
  // STID[10:3] | STID[2:0] EXID[17:13] | EXID[12:5] | EXID[4:0] [IDE] [RTR] 0
  size_t offset = extended ? 3 : 21;
  // We always set the IDE bit for the mask so we distinguish between standard
  // and extended
  uint32_t mask_val = (mask << offset) | (1 << 2); 
  uint32_t filter_val = (filter << offset) | ((uint32_t)extended << 2);

  prv_add_filter_out(s_num_filters, mask_val, filter_val);
  s_num_filters++;
  return STATUS_CODE_OK;
}

CanHwBusStatus can_hw_bus_status(void) {
  if (CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_BOF) == SET) {
    return CAN_HW_BUS_STATUS_OFF;
  } else if (CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_EWG) == SET ||
             CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_EPV) == SET) {
    return CAN_HW_BUS_STATUS_ERROR;
  }

  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  // We can set both since the used ID is determined by tx_msg.IDE
  CanTxMsg tx_msg = {
    .StdId = id,                                          //
    .ExtId = id,                                          //
    .IDE = extended ? CAN_Id_Extended : CAN_Id_Standard,  //
    .DLC = len,                                           //
  };

  memcpy(tx_msg.Data, data, len);

  uint8_t tx_mailbox;
  // Enabling 3 retries
  for (size_t i = 0; i < MAX_TX_RETRIES; ++i)
  {
    tx_mailbox = CAN_Transmit(CAN_HW_BASE, &tx_msg);
    if (tx_mailbox == CAN_TxStatus_NoMailBox) {
      s_tx_full = true;
      if (xSemaphoreTake(s_can_tx_ready_sem_handle, pdMS_TO_TICKS(MAX_TX_MS_TIMEOUT)) == pdFALSE)
      {
        LOG_WARN("CAN HW TX failed");
      }
    } else {
      break;
    }
  }
  // If still fails after 3 retries
  if (tx_mailbox == CAN_TxStatus_NoMailBox) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW TX failed");
  }

  return STATUS_CODE_OK;
}

bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len) {
  // 0: No messages available
  // 1: FIFO0 has received a message
  // 2: FIFO1 has received a message
  // 3: Both have received messages: arbitrarily pick FIFO0
  uint8_t fifo_status = (CAN_MessagePending(CAN_HW_BASE, CAN_FIFO0) != 0) |
                        (CAN_MessagePending(CAN_HW_BASE, CAN_FIFO1) != 0) << 1;
  uint8_t fifo = (fifo_status == 2);

  if (fifo_status == 0) {
    // No messages available
    return false;
  }

  CanRxMsg rx_msg = { 0 };
  CAN_Receive(CAN_HW_BASE, fifo, &rx_msg);

  *extended = (rx_msg.IDE == CAN_Id_Extended);
  *id = *extended ? rx_msg.ExtId : rx_msg.StdId;
  *len = rx_msg.DLC;
  memcpy(data, rx_msg.Data, sizeof(*rx_msg.Data) * rx_msg.DLC);

  return true;
}

// TX handler
void USB_HP_CAN1_TX_IRQHandler(void) {
  //TX Irq only called if Transmit Mailbox Empty IT flag set
  if (CAN_GetITStatus(CAN_HW_BASE, CAN_IT_TME) == SET) {
    if (s_tx_full) {
      xSemaphoreGiveFromISR(s_can_tx_ready_sem_handle, NULL);
      s_tx_full = false;
    }
  }
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_TME);
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
  // Handle Message Pending in RX0 Fifo
  // TODO: Fifo RX 1/0 interrupts also trigger on FIFO full/Fifo overrun
  if (CAN_GetITStatus(CAN_HW_BASE, CAN_IT_FMP0) == SET) {
    CanMessage rx_msg = { 0 };
    if (can_hw_receive(&rx_msg.id.raw, (bool *) &rx_msg.extended, &rx_msg.data, &rx_msg.dlc)) {
      //check id against filter out, if matches any filter in filter out then dont push
      bool s_filter_id_match = false;
      for (int i = 0; i < CAN_HW_NUM_FILTER_BANKS; i++){
        if (can_filters[i] == rx_msg.id.raw){
          s_filter_id_match = true;
          break;
        }
      }
      // If filter match, do not push to rx queue
      if (!s_filter_id_match){
        can_queue_push(s_g_rx_queue, &rx_msg);
      }
    }
  }
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_FMP0);
}

void CAN1_RX1_IRQHandler(void) {
  // Handle Message Pending in RX1 Fifo
  // ISRs will not cause issues
  if (CAN_GetITStatus(CAN_HW_BASE, CAN_IT_FMP1) == SET) {
    CanMessage rx_msg = { 0 };
    if (can_hw_receive(&rx_msg.id.raw, (bool *) &rx_msg.extended, &rx_msg.data, &rx_msg.dlc)) {
      //check id against filter out, if matches any filter in filter out then dont push
      bool s_filter_id_match = false;
      for (int i = 0; i < CAN_HW_NUM_FILTER_BANKS; i++){
        if (can_filters[i] == rx_msg.id.raw){
          s_filter_id_match = true;
          break;
        }
      }
      // If filter match, do not push to rx queue
      if (!s_filter_id_match){
        can_queue_push(s_g_rx_queue, &rx_msg);
      }
    }
  }
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_FMP1);
}

void CAN1_SCE_IRQHandler(void) {
  // TODO(Mitch) Add error notififcations
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_ERR);
}

