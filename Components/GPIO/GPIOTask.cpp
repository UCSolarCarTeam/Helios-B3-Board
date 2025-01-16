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
                    (const char *)"GPIOTask",
                    (uint16_t)GPIO_TASK_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)GPIO_TASK_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "GPIOTask::InitTask() - xTaskCreate() failed");
}

uint8_t GPIOTask::LightsInputs()
{

    // Initialize Expander Objects
    IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0, 0));
    std::array<IOState, 16> driverControlState = driverControlExpander.GetExpanderStateNow();

    uint8_t output = 0;

    uint8_t headlightsOff = driverControlState[static_cast<int>(DriverControls::HEADLIGHTS_ENABLE)] == IOState::LOW; // True if headlights are off
    uint8_t signalRight = driverControlState[static_cast<int>(DriverControls::RIGHT_SIGNAL_ENABLE) - 2] == IOState::HIGH;
    uint8_t signalLeft = driverControlState[static_cast<int>(DriverControls::LEFT_SIGNAL_ENABLE) - 2] == IOState::HIGH;
    uint8_t hazardLights = driverControlState[static_cast<int>(DriverControls::EMERGENCY_HAZARD) - 2] == IOState::HIGH;


    // output |= (headlightsLow ? 1 : 0) << 1;  
    // output |= (headlightsHigh ? 1 : 0) << 2; 
    output |= (signalRight ? 1 : 0) << 0;    // Bit 0
    output |= (signalLeft ? 1 : 0) << 1;     // Bit 1
    output |= (hazardLights ? 1 : 0) << 2;         // Bit 5
    output |= (headlightsOff ? 1 : 0) << 3;  // Bit 3 , NOTE: True if headlights off
    // output |= (interior ? 1 : 0) << 6;      

    return output; 
}

uint16_t GPIOTask::DigitalInputs()
{
    IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0, 0));
    std::array<IOState, 16> driverControlState = driverControlExpander.GetExpanderStateNow();

    uint16_t output = 0;

    uint8_t raceModeEnable = driverControlState[static_cast<int>(DriverControls::RACE_MODE_ENABLE)] == IOState::HIGH;
    uint8_t lap = driverControlState[static_cast<int>(DriverControls::LAP_BUTTON)] == IOState::HIGH;
    uint8_t hornSwitch = driverControlState[static_cast<int>(DriverControls::HORN_ENABLE) - 2] == IOState::HIGH;
    uint8_t motorReset = driverControlState[static_cast<int>(DriverControls::MOTOR_RESET) - 2] == IOState::HIGH; // Assumed motorReset is motor_reset

    uint8_t parkingBrake = driverControlState[static_cast<int>(DriverControls::PARKING_BRAKE_DETECT) - 2] == IOState::HIGH;
    uint8_t mechanicalBreak = driverControlState[static_cast<int>(DriverControls::MECHANICAL_BRAKE) - 2] == IOState::HIGH;
    uint8_t zoomZoom = driverControlState[static_cast<int>(DriverControls::GREEN_LED) - 2] == IOState::HIGH;

    uint8_t forward = driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_H)] == IOState::HIGH
                      && driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_L)] == IOState::LOW;

    uint8_t reverse = driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_H)] == IOState::HIGH
                      && driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_L)] == IOState::HIGH;

    uint8_t neutral = driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_H)] == IOState::LOW    //Assumption
                      && driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_L)] == IOState::LOW;

    /**Forward and Reverse Encoding from Electrical Team */
     /* 10 forward 
     * 11 reverse
     * 00 TODO: CHECK ASSUMPTION THIS IS NEUTRAL
     * 01 not implemented 
     */


    output |= (forward ? 1 : 0) << 0;    // Bit 0
    output |= (neutral ? 1 : 0) << 1;    // Bit 1
    output |= (reverse ? 1 : 0) << 2;    // Bit 2
    output |= (hornSwitch ? 1 : 0) << 3;      //Bit 3
    output |= (mechanicalBreak ? 1 : 0) << 4;     // Bit 4
    output |= (parkingBrake ? 1 : 0) << 5;     // Bit 5
    output |= (motorReset ? 1 : 0) << 6;      // Bit 6
    output |= (raceModeEnable ? 1 : 0) << 7;      // Bit 7
    output |= (lap ? 1 : 0) << 8;               // Bit 8

    //TODO: What is zoom zoom ? Assumed to be greenLed                     Bit 9
    output |= (zoomZoom ? 1 : 0) << 9;               // Bit 9

    return output;
}

uint8_t GPIOTask::LightStatus()
{
    IOExpander powerBoardExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(0, 0, 1));
    std::array<IOState, 16> powerBoardState = powerBoardExpander.GetExpanderStateNow();

    uint8_t output = 0;

    //NOTE: Check if there's an offset like arr[X - 2]; Offset starts at P13
    uint8_t right_turn_light_signal = powerBoardState[static_cast<int>(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL)] == IOState::HIGH;
    uint8_t left_turn_light_signal = powerBoardState[static_cast<int>(PowerBoard::LEFT_TURN_LIGHT_SIGNAL)] == IOState::HIGH;
    uint8_t daytime_running_light_signal = powerBoardState[static_cast<int>(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL)] == IOState::HIGH;
    uint8_t headlight_signal = powerBoardState[static_cast<int>(PowerBoard::HEADLIGHT_SIGNAL)] == IOState::HIGH;
    uint8_t brake_light_signal = powerBoardState[static_cast<int>(PowerBoard::BRAKE_LIGHT_SIGNAL)] == IOState::HIGH;
    uint8_t horn_signal = powerBoardState[static_cast<int>(PowerBoard::HORN_SIGNAL)] == IOState::HIGH;

    output |= (right_turn_light_signal ? 1 : 0) << 0;       // Bit 0
    output |= (left_turn_light_signal ? 1 : 0) << 1;        // Bit 1
    output |= (daytime_running_light_signal ? 1 : 0) << 2;  // Bit 2
    output |= (headlight_signal ? 1 : 0) << 3;              // Bit 3
    output |= (brake_light_signal ? 1 : 0) << 4;            // Bit 4
    output |= (horn_signal ? 1 : 0) << 5;                   // Bit 5

    return output;
}

/**
 * @brief Instance Run loop for the GPIO Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void GPIOTask::Run(void *pvParams)
{
    // Initialize Expander Objects
    IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0, 0));
    IOExpander powerBoardExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(0, 0, 1));

    // Expander status for CAN messages
    uint16_t powerBoardStatus = 0;
    uint16_t driverControlStatus = 0;

    while (1)
    {
        // Poll GPIO State of driver controls
        // Note on IOState array: index 0-7 are pins 0-7, index 8-15 are pins 10-17
        std::array<IOState, 16> driverControlState = driverControlExpander.GetExpanderStateNow();

        // TODO: Do Something to power board >_< based on driver control state or something...
        // Print for Now
        CUBE_PRINT("Driver Control State...\n");
        for (uint8_t i = 0; i < 8; i++)
        {
            switch (static_cast<IOPin>(i))
            {
            case DriverControls::FORWARD_NEUTRAL_REVERSE_H:
                // CUBE_PRINT("    - P00 (Forward/Neutral/Reverse Combo High): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                    // powerBoardExpander.SetPin(PowerBoard::P13, IOState::HIGH);
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                    // powerBoardExpander.SetPin(PowerBoard::P13, IOState::LOW);
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::FORWARD_NEUTRAL_REVERSE_L:
                // CUBE_PRINT("    - P01 (Forward/Neutral/Reverse Combo Low): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::ARRAYS_DISCONNECT:
                // CUBE_PRINT("    - P02 (Array Disconnect): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::RACE_MODE_ENABLE:
                // CUBE_PRINT("    - P03 (Race Mode Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::HEADLIGHTS_ENABLE:
                // CUBE_PRINT("    - P04 (Headlights Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                    // powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                    // powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::DISPLAY_SCREEN_ROTATE:
                // CUBE_PRINT("    - P05 (Display Screen Rotate): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::PROXIMITY_SENSOR_ENABLE:
                // CUBE_PRINT("    - P06 (Proximity Sensor Mute): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::LAP_BUTTON:
                // CUBE_PRINT("    - P07 (Lap Button): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            default:
                break;
            }
        }

        // Pins 10 - 17
        for (uint8_t i = 10; i < 18; i++)
        {
            switch (static_cast<IOPin>(i))
            {
            case DriverControls::HORN_ENABLE:
                // CUBE_PRINT("    - P10 (Horn Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    // powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    // powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::LEFT_SIGNAL_ENABLE:
                // CUBE_PRINT("    - P11 (Left Signal Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    // powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    // powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::RIGHT_SIGNAL_ENABLE:
                // CUBE_PRINT("    - P12 (Right Signal Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    // powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    // powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::EMERGENCY_HAZARD:
                // CUBE_PRINT("    - P13 (Emergency Hazard): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::MOTOR_RESET:
                // CUBE_PRINT("    - P14 (Motor Reset): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::PARKING_BRAKE_DETECT:
                // CUBE_PRINT("    - P15 (Parking Brake Detect): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::MECHANICAL_BRAKE:
                // CUBE_PRINT("    - P16 (Mechanical Brake): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::GREEN_LED:
                // CUBE_PRINT("    - P17 (Green LED): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            default:
                break;
            }
        }

        //        powerBoardExpander.TogglePin(PowerBoard::BRAKE_LIGHT_SIGNAL);
        //        powerBoardExpander.TogglePin(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL);
        //        powerBoardExpander.TogglePin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL);
        //        powerBoardExpander.TogglePin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL);
        //        powerBoardExpander.TogglePin(PowerBoard::HEADLIGHT_SIGNAL);
        //        powerBoardExpander.TogglePin(PowerBoard::HORN_SIGNAL);

        //        powerBoardExpander.SetPin(PowerBoard::BRAKE_LIGHT_SIGNAL, IOState::LOW);
        //        powerBoardExpander.SetPin(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL, IOState::LOW);
        //        powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::LOW);
        //        powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::LOW);
        //        powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::LOW);
        //        powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::LOW);

        powerBoardExpander.SetPin(PowerBoard::BRAKE_LIGHT_SIGNAL, IOState::HIGH);
        powerBoardExpander.SetPin(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL, IOState::HIGH);
        powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
        powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
        powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::HIGH);
        powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::HIGH);

        powerBoardExpander.TogglePin(PowerBoard::ORANGE_LED);
        powerBoardExpander.TogglePin(PowerBoard::GREEN_LED);

        // Commit changes to Power board
        // powerBoardExpander.Commit();

        // Operate task at specified TASK_FREQUENCY
        osDelay(TASK_DELAY);
    }
}
