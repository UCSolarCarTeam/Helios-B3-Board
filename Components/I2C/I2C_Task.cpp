/**
 ******************************************************************************
 * File Name          : I2C_Task.cpp
 * Description        : Primary I2CTask task.
 ******************************************************************************
 */
#include "SystemDefines.hpp"
#include "I2C_Task.hpp"
#include "IOExpander.hpp"

/*----------------------- Macros -----------------------*/
#define TASK_FREQUENCY_HZ 1
constexpr uint32_t TASK_DELAY = 1000 / TASK_FREQUENCY_HZ;

uint8_t GPS_BUFFER[36];
#define BUFFER_SIZE 36
NavData data;

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

uint8_t I2CTask::PollData()
{

}



/**
 * @brief Instance Run loop for the I2C Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void I2CTask::Run(void *pvParams)
{
	CUBE_PRINT("Starting configurations\r\n");
	GPS_Initialization();
	CUBE_PRINT("Configurations complete\r\n");

	while (1){
		UBX_Transmit(UBX_CFG_MSG, sizeof(BUFFER_SIZE));
		UBX_Receive(GPS_BUFFER, BUFFER_SIZE);

		int16_t computedChecksum = UBX_M8N_CHECKSUM(GPS_BUFFER, BUFFER_SIZE);
		uint16_t expectedChecksum = (GPS_BUFFER[BUFFER_SIZE - 2]<<8) | GPS_BUFFER[BUFFER_SIZE - 1];

		if (computedChecksum == expectedChecksum) {
			UBX_M8N_NAV_POSLLH_Parsing(GPS_BUFFER, &data);					      // parses data
			/*CUBE_PRINT("Data, iTOW: %u /n "
					"lon: %d /n "
					"lat: %d /n "
					"height: %d /n"
					"hMSL: %d /n"
					"hAcc: %u /n"
					"vAcc: %u /n", data.iTOW, data.lon, data.lat, data.height, data.hMSL, data.hAcc, data.vAcc); */
		}
		CUBE_PRINT("Data, iTOW: %u \n "
							"lon: %d \n "
							"lat: %d \n "
							"height: %d \n"
							"hMSL: %d \n"
							"hAcc: %u \n"
							"vAcc: %u \n", data.iTOW, data.lon, data.lat, data.height, data.hMSL, data.hAcc, data.vAcc);
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
