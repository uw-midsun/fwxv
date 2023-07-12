#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

// I2C peripheral and GPIO pins
#define I2C_PERIPHERAL              I2C1
#define I2C_GPIO_PORT               GPIOB
#define I2C_GPIO_SCL_PIN            GPIO_Pin_6
#define I2C_GPIO_SDA_PIN            GPIO_Pin_7

// I2C address and buffer size
#define I2C_DEVICE_ADDRESS          0x50
#define BUFFER_SIZE                 10

// Global buffer for I2C data
uint8_t i2cBuffer[BUFFER_SIZE];
uint8_t bufferIndex = 0;

// Function prototypes
void I2C_Configuration(void);
void I2C_Write(uint8_t data);
void I2C_Read(void);

int main(void) {
    // Initialize I2C
    I2C_Configuration();

    while (1) {
        // Write data to the I2C device
        I2C_Write(0xAA);

        // Read data from the I2C device
        I2C_Read();

        // Do something with the received data
        // ...

        // Delay before next iteration
        for (volatile uint32_t i = 0; i < 100000; i++);
    }
}

// I2C event interrupt handler
void I2C1_EV_IRQHandler(void) {
    // Check if start condition was generated
    if (I2C_GetITStatus(I2C_PERIPHERAL, I2C_IT_SB) == SET) {
        // Send the device address with write bit
        I2C_Send7bitAddress(I2C_PERIPHERAL, I2C_DEVICE_ADDRESS, I2C_Direction_Transmitter);
        // Clear the start bit flag
        I2C_ClearITPendingBit(I2C_PERIPHERAL, I2C_IT_SB);
    }

    // Check if address was sent
    if (I2C_GetITStatus(I2C_PERIPHERAL, I2C_IT_ADDR) == SET) {
        // Clear the address flag
        I2C_ClearITPendingBit(I2C_PERIPHERAL, I2C_IT_ADDR);
    }

    // Check if data register is empty and more data can be transmitted
    if (I2C_GetITStatus(I2C_PERIPHERAL, I2C_IT_TXE) == SET) {
        // Check if there is more data to send
        if (bufferIndex < BUFFER_SIZE) {
            // Write data to the I2C peripheral
            I2C_SendData(I2C_PERIPHERAL, i2cBuffer[bufferIndex++]);
        } else {
            // Transfer complete, generate stop condition
            I2C_GenerateSTOP(I2C_PERIPHERAL, ENABLE);
        }

        // Clear the TXE flag
        I2C_ClearITPendingBit(I2C_PERIPHERAL, I2C_IT_TXE);
    }

    // Check if data register is not empty and can receive more data
    if (I2C_GetITStatus(I2C_PERIPHERAL, I2C_IT_RXNE) == SET) {
        // Read data from the I2C peripheral
        uint8_t receivedData = I2C_ReceiveData(I2C_PERIPHERAL);

        // Do something with the received data
        // ...

        // Clear the RXNE flag
        I2C_ClearITPendingBit(I2C_PERIPHERAL, I2C_IT_RXNE);
    }
}

// I2C error interrupt handler
void I2C1_ER_IRQHandler(void) {
    // Handle I2C errors if needed
}

// I2C peripheral configuration
void I2C_Configuration(void) {
    I2C_InitTypeDef I2C_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // Enable the peripheral clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // Configure GPIO pins for I2C
    GPIO_InitStruct.GPIO_Pin = I2C_GPIO_SCL_PIN | I2C_GPIO_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);

    // Configure and enable the I2C interrupt
    NVIC_InitStruct.NVIC_IRQChannel = I2C1_EV_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // Configure the I2C peripheral
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_Init(I2C_PERIPHERAL, &I2C_InitStruct);

    // Enable the I2C peripheral
    I2C_Cmd(I2C_PERIPHERAL, ENABLE);

    // Enable I2C event and error interrupts
    I2C_ITConfig(I2C_PERIPHERAL, I2C_IT_EVT | I2C_IT_ERR, ENABLE);
}

// Write data to the I2C device
void I2C_Write(uint8_t data) {
    bufferIndex = 0;
    i2cBuffer[bufferIndex++] = data;
    I2C_GenerateSTART(I2C_PERIPHERAL, ENABLE);
}

// Read data from the I2C device
void I2C_Read(void) {
    // Implement the read operation if needed
}
