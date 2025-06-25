/**
 ******************************************************************************
 * File Name          : I2C_Task.cpp
 * Description        : Primary I2CTask task.
 ******************************************************************************
 */
#include "SystemDefines.hpp"
#include "CANTx/CANTxTask.hpp"
#include "I2C_Task.hpp"
#include "IOExpander.hpp"
#include "CubeUtils.hpp"
#include "SystemDefines.hpp"
#include <stdio.h>
#include <telemetry_sensor.h>
#include "UbloxDriver.hpp"

/*----------------------- Macros -----------------------*/
#define TASK_FREQUENCY_HZ 1
constexpr uint32_t TASK_DELAY = 1000 / TASK_FREQUENCY_HZ;
//static uint8_t data[6];
int16_t testArray[3];
int16_t gyro_data[3];

uint8_t GPS_BUFFER[36];
#define BUFFER_SIZE 36
NavData gpsData;

/**
 * @brief Constructor for I2CTask
 */
I2CTask::I2CTask() : Task()
{

}

/**
 * @brief Initialize the I2CTask
 */
void I2CTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize I2C task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)I2CTask::RunTask,
                    (const char *)"I2CTask",
                    (uint16_t)I2C_TASK_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)I2C_TASK_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "I2CTask::InitTask() - xTaskCreate() failed");
}

uint8_t* I2CTask::PollData()
{

}


/**
 * @brief Instance Run loop for the I2C Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */

void I2CTask::Run(void *pvParams)
{
	CUBE_PRINT("Starting configurations\r\n");
//	GPS_Initialization();
	CUBE_PRINT("Configurations complete\r\n");

	int16_t x, y, z;
	int16_t gyro_x, gyro_y, gyro_z;
	int16_t temp;

	telemetry_sensor_Init();


	GPSDevice::GPS_Initialization();
	CUBE_PRINT("GPS Configurations complete\r\n");

	Queue* evtQ = CANTxTask::Inst().GetEventQueue();

	while (1) {
		int32_t sum_of_ax = 0, sum_of_ay = 0, sum_of_az = 0;
		int32_t sum_of_gx = 0, sum_of_gy = 0, sum_of_gz = 0;
		int16_t sum_of_t = 0;
		int16_t n = 10;

		GPSDevice::UBX_Transmit(UBX_CFG_MSG, UBX_CFG_MSG_LEN);
        GPSDevice::UBX_Receive(GPS_BUFFER, BUFFER_SIZE);

        uint16_t ck  = UBXMessage::calculateChecksum(GPS_BUFFER, BUFFER_SIZE);
        uint16_t exp = (GPS_BUFFER[BUFFER_SIZE - 2] << 8) |
                       (GPS_BUFFER[BUFFER_SIZE - 1]     );
        if (ck == exp) {
            GPSDevice::UBX_M8N_NAV_POSLLH_Parsing(GPS_BUFFER, &gpsData);
        }
        CUBE_PRINT(
            "Data, iTOW: %u\n lon: %ld\n lat: %ld\n height: %ld\n hMSL: %ld\n hAcc: %u\n vAcc: %u\n",
            gpsData.iTOW,
            gpsData.lon,
            gpsData.lat,
            gpsData.height,
            gpsData.hMSL,
            gpsData.hAcc,
            gpsData.vAcc
        );
        osDelay(500);

		//Calculating average accelerometer values
		for (int16_t a = 0; a<n; a++) {
			Accelerometer_Poll_Data(&x, &y, &z);
			sum_of_ax += x;
			sum_of_ay += y;
			sum_of_az += z;
		}

		int16_t average_accel_x = sum_of_ax / n;
		int16_t average_accel_y = sum_of_ay / n;
		int16_t average_accel_z = sum_of_az / n;

		testArray[0] = (int16_t)(average_accel_x);
		testArray[1] = (int16_t)(average_accel_y);
		testArray[2] = (int16_t)(average_accel_z);

		//Calculating average gyroscope values
		for (int16_t g = 0; g<n; g++) {
			gyroscope_data(&gyro_x, &gyro_y, &gyro_z);
			sum_of_gx += gyro_x;
			sum_of_gy += gyro_y;
			sum_of_gz += gyro_z;
		}

		int16_t average_gyro_x = sum_of_gx / n;
		int16_t average_gyro_y = sum_of_gy / n;
		int16_t average_gyro_z = sum_of_gz / n;

		gyro_data[0] = (int16_t)(average_gyro_x);
		gyro_data[1] = (int16_t)(average_gyro_y);
		gyro_data[2] = (int16_t)(average_gyro_z);

		//Calculating average temperature values
		for (int16_t t = 0; t<n; t++) {
			temperature_data(&temp);
			sum_of_t += temp;
		}
		int16_t avg_temp = sum_of_t / n;

        Command accel_cmd(DATA_COMMAND, ACCELEROMETER);
        bool res = evtQ->Send(accel_cmd);
        CUBE_PRINT("ACCELERATION X Data: %i\n", average_accel_x);
        CUBE_PRINT("ACCELERATION Y Data: %i\n", average_accel_y);
        CUBE_PRINT("ACCELERATION Z Data: %i\n", average_accel_z);

        osDelay(500);

        Command gyro_cmd(DATA_COMMAND, GYROSCOPE);
        bool res1 = evtQ->Send(gyro_cmd);

        CUBE_PRINT("GYROSCOPE X Data: %i\n", average_gyro_x);
        CUBE_PRINT("GYROSCOPE Y Data: %i\n", average_gyro_y);
        CUBE_PRINT("GYROSCOPE Z Data: %i\n", average_gyro_z);

        osDelay(500);

        Command temp_cmd(DATA_COMMAND, TEMPERATURE);
        bool res2 = evtQ->Send(temp_cmd);

        CUBE_PRINT("TEMPERATURE: %i\n", avg_temp);

        osDelay(500);





  }
}

//Extra stuff below
//Need to switch this stuff out with Digital Inputs, Analog Inputs, Lights_Input and Lights Status
/**
 * @brief Handles periodic tasks based on the `counterTick`.
 *
 * This function checks the `counterTick` value and triggers specific tasks
 * according to the following schedule:
 * - `Digital Inputs`: 20 Hz -> 50 ms (sent every execution cycle)
 * - `Analog Inputs`: 20 Hz -> 50 ms (sent every execution cycle)
 * - `Lights_Input`: 10 Hz -> 100 ms (`counterTick == 2`)
 * - `Lights Status`: 5 Hz -> 200 ms (`counterTick == 4`)
 * - 'Heartbeat' : 1 Hz -> 1000 ms (`counterTick == 20`)
 *
 * @details
 * - A base delay of 50 ms is required to increment `counterTick` properly.
 * - `Digital Inputs` and `Analog Inputs` are sent every 50 ms.
 * - `Lights_Input` is sent every 100 ms.
 * - `Lights Status` is sent every 200 ms along with `Lights_Input`.
 * - After 200 ms (`counterTick == 4`), the counter is reset to 0 for the next cycle.
 */
//void I2CTask::checkCounterTick() {
//    // Always send every 50 ms
//    //NOTE: Currently not sending to queue
//    CANTxTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, DIGITAL_INPUTS));
//    // CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ANALOG_INPUTS));
//
//    if (this->counterTick == 2) { // 100 ms passed send LIGHTS_INPUT
//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_INPUT));
//    }
//    if (this->counterTick == 4) { // 200 ms passed send LIGHTS_INPUT and LIGHTS_STATUS_BASE
//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_INPUT));
//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_STATUS_BASE));
//    }
//    if(this->counterTick == 20){
//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND,HEARTBEAT));
//        this->counterTick = 0; // Reset the counter for the next cycle
//    }
//
//}


