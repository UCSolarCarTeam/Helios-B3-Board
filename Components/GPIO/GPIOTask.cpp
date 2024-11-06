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
        // Note on IOState array: index 0-7 are pins 0-7, index 8-15 are pins 10-17
        std::array<IOState,16> driverControlState = driverControlExpander.GetExpanderStateNow();

        // TODO: Do Something to power board >_< based on driver control state or something...
        // Print for Now
        CUBE_PRINT("Driver Control State...\n");
        for (uint8_t i = 0; i < 8; i++) {
            switch (static_cast<IOPin>(i))
            {
            case DriverControls::FORWARD_NEUTRAL_REVERSE_H:
                CUBE_PRINT("    - P00 (Forward/Neutral/Reverse Combo High): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {
                	powerBoardExpander.SetPin(PowerBoard::P13, IOState::HIGH);
                } 
                else if (driverControlState[i] == IOState::LOW) {
                	powerBoardExpander.SetPin(PowerBoard::P13, IOState::LOW);
                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::FORWARD_NEUTRAL_REVERSE_L:
                CUBE_PRINT("    - P01 (Forward/Neutral/Reverse Combo Low): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                else if (driverControlState[i] == IOState::LOW) {

                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::ARRAYS_DISCONNECT:
                CUBE_PRINT("    - P02 (Array Disconnect): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                else if (driverControlState[i] == IOState::LOW) {

                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::RACE_MODE_ENABLE:
                CUBE_PRINT("    - P03 (Race Mode Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                else if (driverControlState[i] == IOState::LOW) {

                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::HEADLIGHTS_ENABLE:
                CUBE_PRINT("    - P04 (Headlights Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {
                    powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i] == IOState::LOW) {
                    powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::DISPLAY_SCREEN_ROTATE:
                CUBE_PRINT("    - P05 (Display Screen Rotate): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                else if (driverControlState[i] == IOState::LOW) {

                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::PROXIMITY_SENSOR_ENABLE:
                CUBE_PRINT("    - P06 (Proximity Sensor Mute): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                else if (driverControlState[i] == IOState::LOW) {

                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
            case DriverControls::LAP_BUTTON:
                CUBE_PRINT("    - P07 (Lap Button): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH) {

                }
                else if (driverControlState[i] == IOState::LOW) {

                }
                else if (driverControlState[i] == IOState::ERROR) {

                }
                break;
			default:
				break;
            }
        }

        // Pins 10 - 17
        for (uint8_t i = 10; i < 18; i++) {
        	switch (static_cast<IOPin>(i))
        	{
            case DriverControls::HORN_ENABLE:
                CUBE_PRINT("    - P10 (Horn Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {
                    powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i-2] == IOState::LOW) {
                    powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i-2] == IOState::ERROR) {
                    
                }
                break;
            case DriverControls::LEFT_SIGNAL_ENABLE:
                CUBE_PRINT("    - P11 (Left Signal Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i-2] == IOState::LOW) {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            case DriverControls::RIGHT_SIGNAL_ENABLE:
                CUBE_PRINT("    - P12 (Right Signal Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i-2] == IOState::LOW) {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            case DriverControls::EMERGENCY_HAZARD:
                CUBE_PRINT("    - P13 (Emergency Hazard): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {

                }
                else if (driverControlState[i-2] == IOState::LOW) {

                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            case DriverControls::MOTOR_RESET:
                CUBE_PRINT("    - P14 (Motor Reset): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {

                }
                else if (driverControlState[i-2] == IOState::LOW) {

                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            case DriverControls::PARKING_BRAKE_DETECT:
                CUBE_PRINT("    - P15 (Parking Brake Detect): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {

                }
                else if (driverControlState[i-2] == IOState::LOW) {

                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            case DriverControls::MECHANICAL_BRAKE:
                CUBE_PRINT("    - P16 (Mechanical Brake): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {

                }
                else if (driverControlState[i-2] == IOState::LOW) {

                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            case DriverControls::GREEN_LED:
                CUBE_PRINT("    - P17 (Green LED): %d\n", driverControlState[i-2]);
                if (driverControlState[i-2] == IOState::HIGH) {

                }
                else if (driverControlState[i-2] == IOState::LOW) {

                }
                else if (driverControlState[i-2] == IOState::ERROR) {

                }
                break;
            default:
                break;
        	}
        }

        driverControlExpander.TogglePinNow(DriverControls::FORWARD_NEUTRAL_REVERSE_H);
        // powerBoardExpander.GetPinStateNow(PowerBoard::P13);

        // Commit changes to Power board
        powerBoardExpander.Commit();

        // Operate task at specified TASK_FREQUENCY
        osDelay(TASK_DELAY);
    }
}
