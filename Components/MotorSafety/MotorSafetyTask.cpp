#include "MotorSafetyTask.hpp"
#include "main.h"
#include "CAN.h"
#include <cstring>
#include <cstdio>
#include <cmath>  // For pow()

#define CAN_TX_ADDRESS 0x500



void MotorSafetyTask::sendADCValues(UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma, uint16_t* dma_adc_buf, uint8_t* dma_uart_buf, uint8_t enable)
{
    if (enable) {
        uint16_t adc_vals[8] = {0};
        memcpy(adc_vals, dma_adc_buf, 8 * sizeof(uint16_t));

        sprintf((char*)dma_uart_buf, "9999 %d %d %d %d %d %d %d %d 8888\r\n",
                adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3],
                adc_vals[4], adc_vals[5], adc_vals[6], adc_vals[7]);

        HAL_UART_DMAStop(huart);
        HAL_UART_Transmit_DMA(huart, dma_uart_buf, strlen((char*)dma_uart_buf));
    }
}


double MotorSafetyTask::predictDcCurrent(double x, double y, double x_mean, double x_std, double y_mean, double y_std, const double coefficients[23]) {
    if (x_std == 0.0 || y_std == 0.0) {
        // Handle or return default to avoid division by zero
        return 0.0;
    }

    double x_norm = (x - x_mean) / x_std;
    double y_norm = (y - y_mean) / y_std;

    double x2 = x_norm * x_norm;
    double x3 = x2 * x_norm;
    double x4 = x3 * x_norm;
    double x5 = x4 * x_norm;
    double x6 = x5 * x_norm;

    double y2 = y_norm * y_norm;
    double y3 = y2 * y_norm;
    double y4 = y3 * y_norm;
    double y5 = y4 * y_norm;
    double y6 = y5 * y_norm;

    double z = coefficients[0] + coefficients[1] * x_norm + coefficients[2] * y_norm + coefficients[3] * x2
        + coefficients[4] * y2 + coefficients[5] * x_norm * y_norm + coefficients[6] * x3 + coefficients[7] * y3
        + coefficients[8] * x2 * y_norm + coefficients[9] * x_norm * y2 + coefficients[10] * x4 + coefficients[11] * y4
        + coefficients[12] * x3 * y_norm + coefficients[13] * x2 * y2 + coefficients[14] * x_norm * y3 + coefficients[15] * x5
        + coefficients[16] * y5 + coefficients[17] * x4 * y_norm + coefficients[18] * x3 * y2 + coefficients[19] * x2 * y3
        + coefficients[20] * x_norm * y4 + coefficients[21] * x6 + coefficients[22] * y6;

    return z;
}

uint8_t MotorSafetyTask::checkADCValues(const uint16_t* dma_adc_buf) {
    int16_t motor_rpm = 0, motor_torque = 0, inv_peak_cur = 0;

    motorSafetyTask(dma_adc_buf, &motor_rpm, &motor_torque, &inv_peak_cur);

    const double x_mean = 72.63720930232559;
    const double x_std = 42.52505156747794;
    const double y_mean = 469.7674418604651;
    const double y_std = 300.12391527727823;

    const double coefficients[23] = {
        89.9081610, 54.1331468, 54.2552952, 3.52967650,
        -1.39293173, 31.2836964, -0.300255167, 1.45315850,
        1.91373226, 2.22332187, -0.106146310, 2.61885344,
        0.189991053, 1.15873486, 1.74216668, 0.255780027,
        0.191376837, -0.132832021, -0.0602793556, 0.450207064,
        0.0149791194, -0.0662227062, -0.502206451
    };

    double predicted_current = predictDcCurrent(
        motor_torque, motor_rpm,
        x_mean, x_std, y_mean, y_std,
        coefficients
    );

    // TODO: Use predicted_current to determine if ADC values are within safety limits

    return 0;
}


//is this actually ever used??
void MotorSafetyTask::receiveCANMessageHandler(uint32_t id, uint8_t dlc, const uint8_t* data) {
    receivedCANMessage.ID = id;
    receivedCANMessage.DLC = dlc;
    if (dlc > 8) dlc = 8;
    memcpy(receivedCANMessage.data, data, dlc);
    canMessageFlag = 1;
}


uint8_t MotorSafetyTask::motorSafetyTask(const uint16_t* dma_adc_buf, int16_t *motor_rpm, int16_t *motor_torque, int16_t *inv_peak_cur) {
    if (canMessageFlag == 1) {
        canMessageFlag = 0;

        switch (receivedCANMessage.ID) {
            case (CAN_TX_ADDRESS + 0): {
                int16_t control_level = (receivedCANMessage.data[0] << 8) | receivedCANMessage.data[1];
                uint8_t control_mode = (receivedCANMessage.data[2] & 0b00000011);
                uint8_t motor_mode = (receivedCANMessage.data[2] >> 2) & 0b00000111;
                uint8_t sw_enable   = (receivedCANMessage.data[2] >> 5) & 0b00000001;
                uint8_t motor_state = (receivedCANMessage.data[2] >> 6) & 0b00000001;
                uint8_t debug_mode  = (receivedCANMessage.data[2] >> 7) & 0b00000001;

                *motor_torque = (receivedCANMessage.data[3] << 8) | receivedCANMessage.data[4];
                *motor_rpm    = (receivedCANMessage.data[5] << 8) | receivedCANMessage.data[6];
                int8_t motor_temp = receivedCANMessage.data[7];

                printf("Control Level: %d\n", control_level);
                printf("Control Mode: %d\n", control_mode);
                printf("Motor Mode: %d\n", motor_mode);
                printf("Software Enable: %d\n", sw_enable);
                printf("Motor State: %d\n", motor_state);
                printf("Debug Mode: %d\n", debug_mode);
                printf("Motor Temp: %d °C\n", motor_temp);
                break;
            }

            case (CAN_TX_ADDRESS + 1): {
                *inv_peak_cur = (receivedCANMessage.data[0] << 8) | receivedCANMessage.data[1];
                int16_t motor_power = (receivedCANMessage.data[2] << 8) | receivedCANMessage.data[3];
                uint16_t abs_position = (receivedCANMessage.data[4] << 8) | receivedCANMessage.data[5];

                printf("Motor Power: %d W\n", motor_power);
                printf("Motor Position: %u (1/2 degrees)\n", abs_position);
                break;
            }

            case (CAN_TX_ADDRESS + 2): {
                uint64_t warning_code = 0;
                for (int i = 0; i < 8; ++i) {
                    warning_code |= (static_cast<uint64_t>(receivedCANMessage.data[i]) << (56 - 8 * i));
                }

                if (warning_code != 0) {
                    printf("Warning/Error Detected: Warning Code = %llu\n", warning_code);
                }
                break;
            }

            case (CAN_TX_ADDRESS + 3): {
                uint64_t error_code = 0;
                for (int i = 0; i < 8; ++i) {
                    error_code |= (static_cast<uint64_t>(receivedCANMessage.data[i]) << (56 - 8 * i));
                }

                printf("Error Detected: Error Code = %llu\n", error_code);
                break;
            }

            default:
                // printf("Unknown message ID: %x\n", receivedCANMessage.ID);
                break;
        }
    }

    return 0;
}
