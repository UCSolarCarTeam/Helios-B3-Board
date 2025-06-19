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
#define TASK_FREQUENCY_HZ 5
constexpr uint32_t TASK_DELAY = 1000 / TASK_FREQUENCY_HZ;

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

    uint8_t headlightsSwitch = driverControlState[static_cast<int>(DriverControls::HEADLIGHTS_ENABLE)] == IOState::LOW;
    uint8_t signalRight = driverControlState[static_cast<int>(DriverControls::RIGHT_SIGNAL_ENABLE)] == IOState::LOW;
    uint8_t signalLeft = driverControlState[static_cast<int>(DriverControls::LEFT_SIGNAL_ENABLE)] == IOState::LOW;
    uint8_t hazardLights = driverControlState[static_cast<int>(DriverControls::HAZARD_LIGHT_ENABLE)] == IOState::LOW;

    output |= (signalRight ? 1 : 0) << 0;      // Bit 0
    output |= (signalLeft ? 1 : 0) << 1;       // Bit 1
    output |= (hazardLights ? 1 : 0) << 2;     // Bit 2
    output |= (headlightsSwitch ? 1 : 0) << 3; // Bit 3 , NOTE: True if HEADLIGHTS_ENABLE is HIGH

    return output;
}

uint16_t GPIOTask::DigitalInputs()
{
    IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0, 0));
    std::array<IOState, 16> driverControlState = driverControlExpander.GetExpanderStateNow();

    // TODO: Check ADC if brake has been pressed

    uint16_t output = 0;

    uint8_t raceModeEnable = driverControlState[static_cast<int>(DriverControls::RACE_MODE_ENABLE)] == IOState::LOW;
    uint8_t lap = driverControlState[static_cast<int>(DriverControls::LAP_BUTTON)] == IOState::LOW;
    uint8_t hornSwitch = driverControlState[static_cast<int>(DriverControls::HORN_ENABLE)] == IOState::LOW;
    uint8_t motorReset = driverControlState[static_cast<int>(DriverControls::MOTOR_RESET)] == IOState::LOW;
    
    // For now use spare CC
    uint8_t parkingBrake = driverControlState[static_cast<int>(DriverControls::SPARE_CC)] == IOState::LOW;

    uint8_t mechanicalBreak = driverControlState[static_cast<int>(DriverControls::MECHANICAL_BRAKE)] == IOState::LOW;
    uint8_t forward = driverControlState[static_cast<int>(DriverControls::FNR_STATE_HIGH)] == IOState::LOW && driverControlState[static_cast<int>(DriverControls::FNR_STATE_LOW)] == IOState::HIGH;
    uint8_t reverse = driverControlState[static_cast<int>(DriverControls::FNR_STATE_HIGH)] == IOState::HIGH && driverControlState[static_cast<int>(DriverControls::FNR_STATE_LOW)] == IOState::HIGH;
    uint8_t neutral = driverControlState[static_cast<int>(DriverControls::FNR_STATE_HIGH)] == IOState::HIGH  && driverControlState[static_cast<int>(DriverControls::FNR_STATE_LOW)] == IOState::LOW;

    /**Forward and Reverse Encoding from Electrical Team */
    /* 00 reverse
     * 01 neutral
     * 10 forward
     * 11 not implemented
     */

    output |= (forward ? 1 : 0) << 0;         // Bit 0
    output |= (neutral ? 1 : 0) << 1;         // Bit 1
    output |= (reverse ? 1 : 0) << 2;         // Bit 2
    output |= (hornSwitch ? 1 : 0) << 3;      // Bit 3
    output |= (mechanicalBreak ? 1 : 0) << 4; // Bit 4
    output |= (parkingBrake ? 1 : 0) << 5;    // Bit 5
    output |= (motorReset ? 1 : 0) << 6;      // Bit 6
    output |= (raceModeEnable ? 1 : 0) << 7;  // Bit 7
    output |= (lap ? 1 : 0) << 8;             // Bit 8

    return output;
}

uint8_t GPIOTask::LightStatus()
{
    IOExpander powerBoardExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(0, 0, 1));
    std::array<IOState, 16> powerBoardState = powerBoardExpander.GetExpanderStateNow();

    uint8_t output = 0;

    // NOTE: Check if there's an offset like arr[X - 2]; Offset starts at P13
    uint8_t right_turn_light_signal = powerBoardState[static_cast<int>(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL)] == IOState::LOW;
    uint8_t left_turn_light_signal = powerBoardState[static_cast<int>(PowerBoard::LEFT_TURN_LIGHT_SIGNAL)] == IOState::LOW;
    uint8_t daytime_running_light_signal = powerBoardState[static_cast<int>(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL)] == IOState::LOW;
    uint8_t headlight_signal = powerBoardState[static_cast<int>(PowerBoard::HEADLIGHT_SIGNAL)] == IOState::LOW;
    uint8_t brake_light_signal = powerBoardState[static_cast<int>(PowerBoard::BRAKE_LIGHT_SIGNAL)] == IOState::LOW;
    uint8_t horn_signal = powerBoardState[static_cast<int>(PowerBoard::HORN_SIGNAL)] == IOState::LOW;

    output |= (right_turn_light_signal ? 1 : 0) << 0;      // Bit 0
    output |= (left_turn_light_signal ? 1 : 0) << 1;       // Bit 1
    output |= (daytime_running_light_signal ? 1 : 0) << 2; // Bit 2
    output |= (headlight_signal ? 1 : 0) << 3;             // Bit 3
    output |= (brake_light_signal ? 1 : 0) << 4;           // Bit 4
    output |= (horn_signal ? 1 : 0) << 5;                  // Bit 5

    return output;
}

/**
 * @brief Get the state of all GPIO pins needed for motor control
 * @return a uint8_t representing the state of the motor control GPIO pins.
 * Bit 0 = FNR Low bit,
 * Bit 1 = FNR High bit,
 * Bit 2 = Mechanical Brake,
 * Bit 3 = Motor Reset,
 *
 * FNR (MAY CHANGE)
 * 00 reverse
 * 01 neutral
 * 10 forward
 * 11 not implemented
 */
uint8_t GPIOTask::getMotorControl(void) {

    IOExpander driverControlExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1, 0, 0));
    uint8_t motor_control = 0;

    driverControlExpander.Update();

    motor_control |= (driverControlExpander.GetPinState(DriverControls::FNR_STATE_LOW) == IOState::LOW) << 0;       // Bit 0
    motor_control |= (driverControlExpander.GetPinState(DriverControls::FNR_STATE_HIGH) == IOState::LOW) << 1;      // Bit 1
    motor_control |= (driverControlExpander.GetPinState(DriverControls::MECHANICAL_BRAKE) == IOState::LOW) << 2;    // Bit 2
    motor_control |= (driverControlExpander.GetPinState(DriverControls::MOTOR_RESET) == IOState::LOW) << 3;         // Bit 3

    return motor_control;
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

    // FNR_State
    uint8_t fnr_state = 0;

    // Hazard Toggle
    uint8_t hazard_toggle = 0;
    uint8_t blink_toggle = 0;
    uint8_t blink_counter = 5;

    // Boolean for Lap button toggle and counter
    uint8_t lap_counter = 0;
    uint8_t lap_toggle = 0;

    // Expander status for CAN messages
    uint16_t powerBoardStatus = 0;
    uint16_t driverControlStatus = 0;

    while (1)
    {
        // Poll GPIO State of driver controls
        // Note on IOState array: index 0-7 are pins 0-7, index 8-15 are pins 10-17
        // Note: Inputs are now active low
        std::array<IOState, 16> driverControlState = driverControlExpander.GetExpanderStateNow();

        // TODO: Do Something to power board >_< based on driver control state or something...
        // Print for Now
        // CUBE_PRINT("Driver Control State...\n");
        for (uint8_t i = 0; i < 8; i++)
        {
            switch (static_cast<IOPin>(i)){
#if 1
           case DriverControls::FNR_STATE_HIGH:
               CUBE_PRINT("    - P00 (FNR_STATE_HIGH): %d\n", driverControlState[i]);
               if (driverControlState[i] == IOState::HIGH)
               {
                   fnr_state &= 0b11111101;
               }
               else if (driverControlState[i] == IOState::LOW)
               {
                   fnr_state |= 0b10;  
               }
               else if (driverControlState[i] == IOState::ERROR)
               {
               }
               break;
           case DriverControls::FNR_STATE_LOW:
               CUBE_PRINT("    - P01 (FNR_STATE_LOW): %d\n", driverControlState[i]);
               if (driverControlState[i] == IOState::HIGH)
               {
                    fnr_state &= 0xFE;
               }
               else if (driverControlState[i] == IOState::LOW)
               {
                    fnr_state |= 0x01;   
               }
               else if (driverControlState[i] == IOState::ERROR)
               {
               }
               break;
           case DriverControls::LAP_BUTTON:
               CUBE_PRINT("    - P02 (Lap Button): %d\n", driverControlState[i]);
               if (driverControlState[i] == IOState::HIGH)
               {
                    lap_toggle = 0;
               }
               else if (driverControlState[i] == IOState::LOW)
               {
                    if (lap_toggle == 0) {
                        lap_counter++;
                        lap_toggle = 1;
                    }
               }
               else if (driverControlState[i] == IOState::ERROR)
               {
               }
               break;
           case DriverControls::SPARE_CC:
               CUBE_PRINT("    - P03 (Spare CC): %d\n", driverControlState[i]);
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
#endif

            case DriverControls::HEADLIGHTS_ENABLE:
                CUBE_PRINT("    - P04 (Headlights Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                    // powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::LOW);
                    powerBoardExpander.SetPin(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL, IOState::LOW);

                }
                else if (driverControlState[i] == IOState::LOW)
                {
                    // powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::HIGH);
                    powerBoardExpander.SetPin(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::HAZARD_LIGHT_ENABLE:
                CUBE_PRINT("    - P05 (Hazard Light Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                    hazard_toggle = 0;
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                    hazard_toggle = 1;
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::RACE_MODE_ENABLE:
                CUBE_PRINT("    - P06 (Race Mode Enable): %d\n", driverControlState[i]);
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
#if 0
            case DriverControls::P07:
                CUBE_PRINT("    - P07 (Unused): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
#endif
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
            case DriverControls::MECHANICAL_BRAKE:
                CUBE_PRINT("    - P10 (Mechanincal Brake): %d\n", driverControlState[i-2]);
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
#if 0
            case DriverControls::P11:
                CUBE_PRINT("    - P11 (Unused): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::P12:
                CUBE_PRINT("    - P12 (Unused): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::P13:
                CUBE_PRINT("    - P13 (Unused): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(DriverControls::MOTOR_RESET, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(DriverControls::MOTOR_RESET, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
#endif
            case DriverControls::HORN_ENABLE:
                CUBE_PRINT("    - P14 (Horn Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::RIGHT_SIGNAL_ENABLE:
                CUBE_PRINT("    - P15 (Right Signal): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::LEFT_SIGNAL_ENABLE:
                CUBE_PRINT("    - P16 (Left Signal): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::MOTOR_RESET:
                CUBE_PRINT("    - P17 (Motor Reset): %d\n", driverControlState[i-2]);
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

        // TODO: sync it properly
        if (hazard_toggle) {

            blink_counter--;

            if (blink_counter == 0) {
                blink_counter = 5;
                if (!blink_toggle) {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                    blink_toggle = 1;
                } else {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::LOW);
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::LOW);
                    blink_toggle = 0;   
                }
            }
            
        } else {
            blink_counter = 0;
        }

        // Commit changes to Power board
        powerBoardExpander.Commit();

        // checkCounterTick();
        // this->counterTick++;

        // Operate task at specified TASK_FREQUENCY
        osDelay(TASK_DELAY);
    }
}


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
void GPIOTask::checkCounterTick() {
    // Always send every 50 ms
    //NOTE: Currently not sending to queue 
    CANTxTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, DIGITAL_INPUTS));
    // CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ANALOG_INPUTS));

    if (this->counterTick == 2) { // 100 ms passed send LIGHTS_INPUT
        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_INPUT));
    }
    if (this->counterTick == 4) { // 200 ms passed send LIGHTS_INPUT and LIGHTS_STATUS_BASE
        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_INPUT));
        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_STATUS_BASE));
    }
    if(this->counterTick == 20){
        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND,HEARTBEAT));
        this->counterTick = 0; // Reset the counter for the next cycle
    }

}
