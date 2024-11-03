/**
 ******************************************************************************
 * File Name          : GPIOTask.cpp
 * Description        : Primary GPIO task. Handles writes and reads to PCA8575PW IO Expanders
 ******************************************************************************
*/
#include "SystemDefines.hpp"
#include "GPIOTask.hpp"
#include "IOExpander.hpp"

/*----------------------- Macros -----------------------*/
#define TASK_FREQUENCY 1
constexpr uint32_t TASK_DELAY = 1000 / TASK_FREQUENCY;

/**
 * @brief Constructor for GPIOTask
 */
GPIOTask::GPIOTask() : Task(GPIO_TASK_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the GPIOTask
 */
void GPIOTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize GPIO task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)GPIOTask::RunTask,
            (const char*)"GPIOTask",
            (uint16_t)GPIO_TASK_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)GPIO_TASK_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "GPIOTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the GPIO Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void GPIOTask::Run(void * pvParams)
{
    // Initialize Expander Objects
    IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0 ,0));
    IOExpander powerBoardExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(0, 0, 1));

    while (1) {
        Command cm;

        // Poll GPIO State of driver controls
        std::array<IOState,16> driverControlState = driverControlExpander.GetExpanderStateNow();

        // Do Something to power board >_< based on driver control state
        // Print for Now
        CUBE_PRINT("Driver Control State...\n");
        for (uint8_t i = 0; i < 8; i++) {
            switch (static_cast<IOPin>(i))
            {
            case IOPin::P00:
                CUBE_PRINT("    - P00 (Forward/Neutral/Reverse Combo): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P01:
                CUBE_PRINT("    - P01 (Forward/Neutral/Reverse Combo): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P02:
                CUBE_PRINT("    - P02 (Array Disconnect): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P03:
                CUBE_PRINT("    - P03 (Race Mode Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P04:
                CUBE_PRINT("    - P04 (Headlights Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P05:
                CUBE_PRINT("    - P05 (Display Screen Rotate): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P06:
                CUBE_PRINT("    - P06 (Proximity Sensor Mute): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
            case IOPin::P07:
                CUBE_PRINT("    - P07 (Lap Button): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                break;
			default:
				break;
            }
        }

        for (uint8_t i = 10; i < 18; i++) {
        	switch (static_cast<IOPin>(i))
        	{
				case IOPin::P10:
					CUBE_PRINT("    - P10 (Horn Enable): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P11:
					CUBE_PRINT("    - P11 (Left Signal Enable): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P12:
					CUBE_PRINT("    - P12 (Right Signal Enable): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P13:
					CUBE_PRINT("    - P13 (Emergency Hazard): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P14:
					CUBE_PRINT("    - P14 (Motor Reset): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P15:
					CUBE_PRINT("    - P15 (Parking Break Detect): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P16:
					CUBE_PRINT("    - P16 (Mechanical Break): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				case IOPin::P17:
					CUBE_PRINT("    - P17 (Green LED): %d\n", driverControlState[i-2]);
					if (driverControlState[i-2] == IOState::HIGH) {

					}
					break;
				default:
					break;
        	}
        }


        // Operate task at specified TASK_FREQUENCY
        osDelay(TASK_DELAY);
    }
}
