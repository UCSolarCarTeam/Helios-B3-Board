#pragma once

#include "main.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "CANRegisters.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t ID;
    uint32_t extendedID;
    uint8_t DLC;
    uint8_t data[8];
} CANMsg;

typedef struct {
	uint32_t ID;
	uint8_t DLC;
	uint8_t data;
} ReceiveMsg;

typedef struct {
    GPIO_TypeDef *CS_PORT;
    uint16_t CS_PIN;
    SPI_HandleTypeDef *hspi;
} CANPeripheral;


#define TX_CHANNEL_CHECK_DELAY 1
#define CAN_TEST_SETUP 1

// CAN SPI Interface Functions
void CAN_IC_READ_REGISTER(uint8_t address, uint8_t* buffer, CANPeripheral *peripheral);
void CAN_IC_WRITE_REGISTER_BITWISE(uint8_t address, uint8_t mask, uint8_t value, CANPeripheral *peripheral);
void CAN_IC_WRITE_REGISTER(uint8_t address, uint8_t value, CANPeripheral *peripheral);
void CAN_IC_READ_STATUS(uint8_t* buffer, CANPeripheral *peripheral);
void CAN_IC_RESET(CANPeripheral *peripheral);
void CAN_IC_REQUEST_TO_SEND(uint8_t channel, CANPeripheral *peripheral);

// CAN Operation Functions
void ConfigureCANSPI(CANPeripheral *peripheral);
void sendCANMessage(CANMsg *msg, CANPeripheral *peripheral);
void sendExtendedCANMessage(CANMsg *msg, CANPeripheral *peripheral);
void receiveCANMessage(uint8_t channel, uint32_t* ID, uint8_t* DLC, uint8_t* data, CANPeripheral *peripheral);
uint8_t checkAvailableTXChannel(CANPeripheral *peripheral);

extern uint8_t blueStatus;
extern uint8_t greenStatus;

void pollCanSetup();

#ifdef __cplusplus
}
#endif
// https://www.codesdope.com/blog/article/making-a-queue-using-linked-list-in-c/
