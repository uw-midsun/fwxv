#include "sd_card_spi.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint8_t irqn;
  SPI_TypeDef *base;
  GpioAddress cs;
} SdSpiPortData;

static SdSpiPortData s_port[NUM_SD_SPI_PORTS] = {
  [SD_SPI_PORT_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                      .periph = RCC_APB2Periph_SPI1,
                      .base = SPI1,
                      .irqn = SPI1_IRQn },
  [SD_SPI_PORT_2] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                      .periph = RCC_APB1Periph_SPI2,
                      .base = SPI2,
                      .irqn = SPI2_IRQn },
};

StatusCode sd_spi_init(SdSpiPort spi, SdSpiSettings *settings) {
  if (spi >= NUM_SD_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  } else if (settings->mode >= NUM_SD_SPI_MODES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI mode.");
  }

  s_port[spi].rcc_cmd(s_port[spi].periph, ENABLE);
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);
  uint32_t clk_freq;

  if (spi == SD_SPI_PORT_1) {
    clk_freq = clocks.PCLK2_Frequency;  // SPI1 is on APB2 Bus
  } else {
    clk_freq = clocks.PCLK1_Frequency;  // SPI2 is on APB1 Bus
  }

  // SPI prescalers must be powers of two with a minimum prescaler of /2,
  // The equation is baudrate = Frequency/Prescaler, rounded to nearest power of 2
  // This is then offset to a valid SPI_BaudRate_Prescaler
  size_t index = 32 - (size_t)__builtin_clz(clk_freq / settings->baudrate);
  uint16_t prescaler = (index - 1) << 3;
  if (!IS_SPI_BAUDRATE_PRESCALER(prescaler)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid baudrate");
  }
  s_port[spi].cs = settings->cs;

  // Confifgure spi pins to correct modes
  gpio_init_pin(&settings->miso, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&settings->mosi, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  // TODO(mitchellostler): CRC is not used, could be a potential improvement on reliability
  SPI_InitTypeDef init = {
    .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .SPI_Mode = SPI_Mode_Master,
    .SPI_DataSize = SPI_DataSize_8b,
    .SPI_CPHA = (settings->mode & 0x01) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge,
    .SPI_CPOL = (settings->mode & 0x02) ? SPI_CPOL_High : SPI_CPOL_Low,
    .SPI_NSS = SPI_NSS_Soft,
    .SPI_BaudRatePrescaler = (index - 2) << 3,
    .SPI_FirstBit = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial = 7,
  };
  SPI_Init(s_port[spi].base, &init);

  return STATUS_CODE_OK;
}

StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *tx_data, size_t tx_len) {
  if (spi >= NUM_SD_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }
  SPI_TypeDef *SPIx = s_port[spi].base;

  for (size_t i = 0; i < tx_len; i++) {
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) {
    }
    SPI_I2S_SendData(SPIx, tx_data[i]);
  }
  return STATUS_CODE_OK;
}

StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  if (spi >= NUM_SD_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }
  SPI_TypeDef *SPIx = s_port[spi].base;

  for (size_t i = 0; i < rx_len; i++) {
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) {
    }
    SPI_I2S_SendData(SPIx, placeholder);

    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) {
    }
    rx_data[i] = SPI_I2S_ReceiveData(SPIx);
  }
  return STATUS_CODE_OK;
}

StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state) {
  if (spi >= NUM_SD_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }

  return gpio_set_state(&s_port[spi].cs, state);
}

StatusCode sd_spi_cs_get_state(SdSpiPort spi, GpioState *input_state) {
  if (spi >= NUM_SD_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }

  return gpio_get_state(&s_port[spi].cs, input_state);
}

StatusCode sd_spi_set_frequency(SdSpiPort spi, uint32_t baudrate) {
  if (spi >= NUM_SD_SPI_PORTS) return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");

  // Recompute prescaler
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);
  uint32_t clk_freq;

  if (spi == SD_SPI_PORT_1) {
    clk_freq = clocks.PCLK2_Frequency;  // SPI1 is on APB2 Bus
  } else {
    clk_freq = clocks.PCLK1_Frequency;  // SPI2 is on APB1 Bus
  }

  // SPI prescalers must be powers of two with a minimum prescaler of /2,
  // The equation is baudrate = Frequency/Prescaler, rounded to nearest power of 2
  // This is then offset to a valid SPI_BaudRate_Prescaler
  size_t index = 32 - (size_t)__builtin_clz(clk_freq / baudrate);
  uint16_t prescaler = (index - 1) << 3;
  if (!IS_SPI_BAUDRATE_PRESCALER(prescaler)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid baudrate");
  }

  SPI_TypeDef *SPIx = s_port[spi].base;
  SPI_Cmd(SPIx, DISABLE);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
  // clear existing BR[2:0] bits, then set new prescaler
  SPIx->CR1 = (SPIx->CR1 & ~SPI_CR1_BR) | prescaler;
#pragma GCC diagnostic pop

  SPI_Cmd(SPIx, ENABLE);
  return STATUS_CODE_OK;
}
