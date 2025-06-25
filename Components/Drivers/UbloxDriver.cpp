/*
 * UbloxDriver.cpp
 *
 *  Created on: Jun 18, 2025
 *      Author: jazebzafar
 */

#include "UbloxDriver.hpp"
#include "SystemDefines.hpp"
#include <cstring>

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

//:

// UBX  Buffers
uint8_t UBX_CFG_PRT[] = {
    0xB5, 0x62,             // Sync Chars
    0x06, 0x00,             // Class=CFG (0x06), ID=PRT (0x00)
    0x14, 0x00,             // Length = 20 bytes
    0x00, 0x00,             // Port ID = I2C (0)
    0x00, 0x00,             // txReady settings
    0x00, 0x00, 0x00, 0x00, // I2C mode flags (cleared)
    0x00, 0x00, 0x00, 0x00, // reserved
    0x03, 0x00,             // inProtoMask (NMEA + UBX)
    0x03, 0x00,             // outProtoMask (NMEA + UBX)
    0x00, 0x00,             // extended TX timeout
    0x00, 0x00,             // reserved
    0x20, 0x72              // CK_A, CK_B was changed from 0x8D,0x7A and recalculated 0x20,0x72)
};

uint8_t UBX_CFG_MSG[] = {
    0xB5, 0x62,             // Sync Chars
    0x06, 0x01,             // Class=CFG (0x06), ID=MSG (0x01)
    0x08, 0x00,             // Length = 8 bytes
    0x01, 0x02,             // Payload: Class=0x01, ID=0x02 (NAV-POSLLH)
    0x01, 0x00,             // Rate = 1 (every solution)
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x13, 0xBF              // CK_A, CK_B was changed from 0x32,0x32 and recalculated to 0x13,0xBF)
};

uint8_t UBX_CFG_RATE[] = {
    0xB5, 0x62,             // Sync Chars
    0x06, 0x08,             // Class=CFG (0x06), ID=RATE (0x08)
    0x06, 0x00,             // Length = 6 bytes
    0xE8, 0x03,             // measRate = 1000 ms (1 Hz)
    0x01, 0x00,             // navRate = 1 cycle
    0x01, 0x00,             // timeRef = GPS time
    0x01, 0x39              // CK_A, CK_B was changed from 0x0D,0x3D and recalculated 0x01,0x39)
};

uint8_t UBX_CFG_RESET[] = {
    0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, // header and length
    0xFF, 0xFF,                         // navBbrMask
    0x00, 0x00,                         // resetMode
    0x0C, 0x5D                          // CK_A, CK_B
};

uint8_t UBX_CFG_CFG[] = {
    0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, // header and length
    0xFF, 0xFF, 0x00, 0x00,             // clearMask
    0x00, 0x00, 0x00, 0x00,             // saveMask
    0xFF, 0xFF, 0x00, 0x00,             // loadMask
    0x17,                               // deviceMask
    0x2F, 0xAE                          // CK_A, CK_B
};

uint8_t UBX_ACK_ACK[] = {
    0xB5, 0x62,             // Sync Chars
    0x05, 0x01,             // Class=ACK (0x05), ID=ACK (0x01)
    0x02, 0x00,             // Length = 2 bytes
    0x00, 0x00,             // Payload: class=0x00, id=0x00
    0x08, 0x2B              // CK_A, CK_B was changed from 0x00,0x00 and recalculated to 0x08,0x2B)
};

uint8_t UBX_CFG_NAV_PVT[] = {
    0xB5, 0x62,            // Sync Chars
    0x06, 0x01,             // Class=CFG (0x06), ID=MSG (0x01)
    0x08, 0x00,             // Length = 8 bytes
    0x01, 0x07,             // Payload: Class=0x01, ID=0x07 (NAV-PVT)
    0x01, 0x00,             // Rate = 1
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x18, 0xE2              // CK_A, CK_B changed from 0x37,0x37 and recalculated to 0x18,0xE2)
};

uint8_t UBX_POLL_POSLLH[8] = {0xB5,0x62, 0x01,0x02, 0x00,0x00, 0x03,0x0A};
const uint16_t UBX_POLL_POSLLH_LEN = sizeof(UBX_POLL_POSLLH);

uint16_t UBX_CFG_PRT_LEN = sizeof(UBX_CFG_PRT);
uint16_t UBX_CFG_MSG_LEN = sizeof(UBX_CFG_MSG);
uint16_t UBX_CFG_RATE_LEN = sizeof(UBX_CFG_RATE);
uint16_t UBX_CFG_RESET_LEN = sizeof(UBX_CFG_RESET);
uint16_t UBX_CFG_CFG_LEN = sizeof(UBX_CFG_CFG);
uint16_t UBX_ACK_ACK_LEN = sizeof(UBX_ACK_ACK);
uint16_t UBX_CFG_NAV_PVT_LEN = sizeof(UBX_CFG_NAV_PVT);


// UBXMessage
UBXMessage::UBXMessage(uint8_t class_id, uint8_t msg_id, uint8_t length)
    : msg_class(class_id), msg_id(msg_id), length(length) {

}


uint16_t UBXMessage::calculateChecksum(uint8_t* buffer, uint8_t buflen) {
    uint8_t CK_A = 0, CK_B = 0;
    for (int i = 2; i < buflen - 2; i++) {
        CK_A = CK_A + buffer[i];
        CK_B = CK_B + CK_A;
    }
    return ((CK_A << 8) | CK_B);
}

void UBXMessage::poll() {
    // Default behavior
}

void UBXMessage::parse(uint8_t* buffer) {
    // Default behavior
}

// -------------------- GPSDevice Implementation --------------------
void GPSDevice::UBX_Transmit(uint8_t* buffer, uint16_t buflen) {
    HAL_StatusTypeDef hal = HAL_I2C_Mem_Write(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_REGISTER, 1, buffer, buflen, 100);
    if (hal != HAL_OK) {
        // CUBE_PRINT("HAL Status: %d | I2C Error: %d | Class and ID: %#X %#X\r\n", hal, hi2c1.ErrorCode, buffer[2], buffer[3]);
    } else {
        CUBE_PRINT("[ ^ ]UBX Transmit Successful");
    }
}

//Recieve function
void GPSDevice::UBX_Receive(uint8_t* buffer, uint16_t buflen) {
    HAL_StatusTypeDef hal = HAL_I2C_Mem_Read(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_REGISTER, 1, buffer, buflen, 100);
    if (hal != HAL_OK) {
        // CUBE_PRINT("HAL Status: %d | I2C Error: %\r\n", hal, hi2c1.ErrorCode);
    } else {
        CUBE_PRINT("[ ^ ]UBX Receive Successful");
    }
}

// POSLLH parsing function
void GPSDevice::UBX_M8N_NAV_POSLLH_Parsing(uint8_t* buffer, NavData* data) {
    data->iTOW = buffer[9]<<24 | buffer[8]<<16 | buffer[7]<<8 | buffer[6];
    data->lon = buffer[13]<<24 | buffer[12]<<16 | buffer[11]<<8 | buffer[10];
    data->lat = buffer[17]<<24 | buffer[16]<<16 | buffer[15]<<8 | buffer[14];
    data->height = buffer[21]<<24 | buffer[20]<<16 | buffer[19]<<8 | buffer[18];
    data->hMSL = buffer[25]<<24 | buffer[24]<<16 | buffer[23]<<8 | buffer[22];
    data->hAcc = buffer[29]<<24 | buffer[28]<<16 | buffer[27]<<8 | buffer[26];
    data->vAcc = buffer[33]<<24 | buffer[32]<<16 | buffer[31]<<8 | buffer[30];
}

// Transmit function
void GPSDevice::CONFIG_Transmit(uint8_t* buffer, uint16_t buflen) {
    HAL_StatusTypeDef hal = HAL_I2C_Master_Transmit(&hi2c1, GPS_DEVICE_ADDRESS, buffer, buflen, HAL_MAX_DELAY);
    if (hal != HAL_OK) {
        CUBE_PRINT("CONFIG transmit went wrong\r\n");
        CUBE_PRINT("    [i]0x%x\r\n", hal);
    }
    uint16_t message_length = UBX_GET_LENGTH();
    CUBE_PRINT("Message Length: %d\r\n", message_length);

    if (message_length != 0) {
        uint8_t config_response[message_length];
        config_response[0] = GPS_DATA_REGISTER;

        hal = HAL_I2C_Master_Receive(&hi2c1, GPS_DEVICE_ADDRESS | 0x01, config_response, message_length, HAL_MAX_DELAY);
        if (hal != HAL_OK) {
            CUBE_PRINT("CONFIG response went wrong!\r\n");
            CUBE_PRINT("Error code: %08lX\r\n", hi2c1.ErrorCode);
        } else {
            CUBE_PRINT("Length: %d | Headers: %X %X |Class: %X | ID: %X | rest: %X %X %X %X %X %X\r\n",
                message_length,
                config_response[0], config_response[1],
                config_response[2], config_response[3],
                config_response[4], config_response[5], config_response[6], config_response[7], config_response[8], config_response[9]);
        }
    }
}

// Get length for UBX
uint16_t GPSDevice::UBX_GET_LENGTH() {
    uint8_t ubx_length[2];
    HAL_StatusTypeDef hal = HAL_I2C_Mem_Read(&hi2c1, GPS_DEVICE_ADDRESS | 0x01, GPS_DATA_LENGTH_HIGH, 1, ubx_length, sizeof(ubx_length), 100);
    if (hal != HAL_OK) {
        CUBE_PRINT("Read for length went wrong");
    }
    //returns the Ubx_length mentioned prior
    CUBE_PRINT("High: %X | Low: %X\r\n", ubx_length[0], ubx_length[1]);
    return ((ubx_length[0] << 8) | (ubx_length[1]));
}

// Init func
void GPSDevice::GPS_Initialization() {
	//Size calulcataion based on previous implementation
    CUBE_PRINT("Starting MSG\r\n");
    CONFIG_Transmit(UBX_CFG_MSG, sizeof(UBX_CFG_MSG)/sizeof(UBX_CFG_MSG[0]));
    CUBE_PRINT("Starting PRT\r\n");
    CONFIG_Transmit(UBX_CFG_PRT, sizeof(UBX_CFG_PRT)/sizeof(UBX_CFG_PRT[0]));
    CUBE_PRINT("Starting RATE\r\n");
    CONFIG_Transmit(UBX_CFG_RATE, sizeof(UBX_CFG_RATE)/sizeof(UBX_CFG_RATE[0]));
    CUBE_PRINT("Starting NAV\r\n");
    CONFIG_Transmit(UBX_CFG_NAV_PVT, sizeof(UBX_CFG_NAV_PVT)/sizeof(UBX_CFG_NAV_PVT[0]));
    osDelay(500);
}





