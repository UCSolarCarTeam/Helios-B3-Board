/**
 ******************************************************************************
 * File Name          : MotorControlTask.cpp
 * Description        : Sends commands to motor driver
 ******************************************************************************
 */
#include "SystemDefines.hpp"
#include "GPIOTask.hpp"
#include "IOExpander.hpp"
#include "MotorControlTask.hpp"
#include "CAN.h"
#include "CANTxTask.cpp"

/*----------------------- Macros -----------------------*/
#define TASK_FREQUENCY 1
constexpr uint32_t TASK_DELAY = 10 / TASK_FREQUENCY;

// Forward Decloration
void SendMotorCommand(Motor_cmd* motor_cmd, Motor_cmd* last_motor_cmd);
void PrintMotorCommand(UART_HandleTypeDef* huart, Motor_cmd* motor_cmd);

/**
 * @brief Construction of Motor Control Task
 * Leave empty to adhear to singleton pattern as Inst handles construction
 */
MotorControlTask::MotorControlTask() : Task(GPIO_TASK_QUEUE_DEPTH_OBJS)
{
}

uint8_t CheckBuffer(uint8_t* buffer, uint8_t* buffer_index) {
    if ((uint8_t)(*buffer_index) == '\r') {
        // Command finished
        return 1;
    }
    else {
        // Command not finished
        return 0;
    }

    return 0;
}

/*
    * Parses the buffer into a Motor_cmd
    *
    * @param motor_cmd: The Motor_cmd struct to store the parsed data
    * @param buffer: The buffer to parse
    * @param end_index: The index of the buffer to end parsing
    * @param last_message: The last message received
    *
    * @return A Motor_cmd struct containing the parsed data
*/
uint8_t ParseMotorCommand(Motor_cmd* motor_cmd, uint8_t* buffer, uint8_t* last_message, uint8_t* adc_log_enable, Motor_cmd* last_motor_cmd, UART_HandleTypeDef* huart) {
    for(int i = 0; i < UART_BUF_LEN; i++){
        last_message[i] = buffer[i];
    }

    char motor[5] = {0};
    char option[20] = {0};
    int16_t number = {0};

    // format: m[id] [option] [number]
    sscanf((char *)last_message, "%s %s %hd", motor, option, &number);

    char update_msg[50] = {0};

    // Motor 1
    if (strcmp(motor, "m1") == 0){
        // Status
        if(strcmp(option, "on") == 0){
            motor_cmd->m1_status = 1;
            CUBE_PRINT(update_msg, "Motor 1 Status: %d -> %d\r\n", last_motor_cmd->m1_status, motor_cmd->m1_status);
        }
        else if(strcmp(option, "off") == 0){
            motor_cmd->m1_status = 0;
            CUBE_PRINT(update_msg, "Motor 1 Status: %d -> %d\r\n", last_motor_cmd->m1_status, motor_cmd->m1_status);
        }

        // Control Mode and Value
        else if(strcmp(option, "torque") == 0){
            if (number > 100){
                number = 100;
            } else if(number < -100){
                number = -100;
            }

            motor_cmd->m1_val = number;
            motor_cmd->m1_control = 1;

            if(number < 0){
                motor_cmd->m1_dir = 1;
            } else {
                motor_cmd->m1_dir = 0;
            }

            CUBE_PRINT(update_msg, "Motor 1 Torque: %d%% -> %d%%\r\n", last_motor_cmd->m1_val, motor_cmd->m1_val);

        }
        else if(strcmp(option, "speed") == 0){
            if(number > 10000){
                number = 10000;
            } else if(number < -10000){
                number = -10000;
            }

            motor_cmd->m1_val = number;
            motor_cmd->m1_control = 2;
            if (number < 0){
                motor_cmd->m1_dir = 1;
            } else {
                motor_cmd->m1_dir = 0;
            }
            CUBE_PRINT(update_msg, "Motor 1 Speed: %0.1f RPM -> %0.1f RPM\r\n", (float)last_motor_cmd->m1_val/10, (float)motor_cmd->m1_val/10);
        }

        // Motor Mode
        else if(strcmp(option, "default") == 0){
            motor_cmd->m1_mode = 0;
            CUBE_PRINT(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
        } else if(strcmp(option, "boost") == 0){
            motor_cmd->m1_mode = 1;
            CUBE_PRINT(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
        } else if(strcmp(option, "reverse") == 0){
            motor_cmd->m1_mode = 2;
            CUBE_PRINT(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
        } else if(strcmp(option, "regen") == 0){
            motor_cmd->m1_mode = 3;
            CUBE_PRINT(update_msg, "Motor 1 Mode: %d -> %d\r\n", last_motor_cmd->m1_mode, motor_cmd->m1_mode);
        }

        else {
            // unsupported option
            return -3;
        }
    }

    // Motor 2
    else if(strcmp(motor, "m2") == 0){
        // Status
        if(strcmp(option, "on") == 0) {
            motor_cmd->m2_status = 1;
            CUBE_PRINT(update_msg, "Motor 2 Status: %d -> %d\r\n", last_motor_cmd->m2_status, motor_cmd->m2_status);
        }
        else if(strcmp(option, "off") == 0){
            motor_cmd->m2_status = 0;
            CUBE_PRINT(update_msg, "Motor 2 Status: %d -> %d\r\n", last_motor_cmd->m2_status, motor_cmd->m2_status);
        }

        // Control Mode and Value
        else if(strcmp(option, "torque") == 0){
            if (number > 100){
                number = 100;
            } else if(number < -100){
                number = -100;
            }
            motor_cmd->m2_val = number;
            motor_cmd->m2_control = 1;
            if(number < 0){
                motor_cmd->m2_dir = 1;
            } else {
                motor_cmd->m2_dir = 0;
            }
            CUBE_PRINT(update_msg, "Motor 2 Torque: %d%% -> %d%%\r\n", last_motor_cmd->m2_val, motor_cmd->m2_val);
        }
        else if(strcmp(option, "speed") == 0){
            if(number > 10000){
                number = 10000;
            } else if(number < -10000){
                number = -10000;
            }

            motor_cmd->m2_val = number;
            motor_cmd->m2_control = 2;
            if (number < 0){
                motor_cmd->m2_dir = 1;
            } else {
                motor_cmd->m2_dir = 0;
            }
            CUBE_PRINT(update_msg, "Motor 2 Speed: %d RPM -> %d RPM\r\n", last_motor_cmd->m2_val/10, motor_cmd->m2_val/10);
        }

        // Motor Mode
        else if(strcmp(option, "default") == 0){
            motor_cmd->m2_mode = 0;
            CUBE_PRINT(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
        } else if(strcmp(option, "boost") == 0){
            motor_cmd->m2_mode = 1;
            CUBE_PRINT(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
        } else if(strcmp(option, "reverse") == 0){
            motor_cmd->m2_mode = 2;
            CUBE_PRINT(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
        } else if(strcmp(option, "regen") == 0){
            motor_cmd->m2_mode = 3;
            CUBE_PRINT(update_msg, "Motor 2 Mode: %d -> %d\r\n", last_motor_cmd->m2_mode, motor_cmd->m2_mode);
        }

        else { // unsupported option
            return -3;
        }
    }

    else if (strcmp(motor, "send") == 0){ // Send Command to Motors
        SendMotorCommand(motor_cmd, last_motor_cmd);
    }

    else if (strcmp(motor, "print") == 0){ // Print to UART Terminal
    	if(strcmp(option, "all") == 0){
    		// Print both commands
    		char msg[20] = {0};
    		CUBE_PRINT(msg, "Last Sent Command\r\n");
    		HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), 100);
    		PrintMotorCommand(huart, last_motor_cmd);

    		CUBE_PRINT(msg, "Current Command\r\n");
    		HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), 100);
    		PrintMotorCommand(huart, motor_cmd);

    	} else if(strcmp(option, "last") == 0){
    		// print last motor_cmd
    		PrintMotorCommand(huart, last_motor_cmd);

    	} else if(strcmp(option, "") == 0){
    		// Print current command
    		PrintMotorCommand(huart, motor_cmd);
    	} else {
            // unsupported option
            return -3;
        }
    }

    else if(strcmp(motor, "logadc") == 0){ // ADC Log
    	if(strcmp(option, "on") == 0){
    		*adc_log_enable = 1;
            CUBE_PRINT(update_msg, "ADC Logging %s\r\n", option);

    	} else if(strcmp(option, "off") == 0){
    		*adc_log_enable = 0;
            CUBE_PRINT(update_msg, "ADC Logging %s\r\n", option);

    	} else { // unsupported option
    		return -3;
    	}
    }

    else { // unsupported motor
        return -2;
    }

    HAL_UART_Transmit(huart, (uint8_t*)update_msg, strlen(update_msg), 100);

    return 1;
}

/*
    * Parse Motor_cmd into a CAN message
    * Set the motor command using CAN
    *
    * @param motor_cmd: The motor command to send
    * @param last_motor_cmd: save the previous command
*/
void SendMotorCommand(Motor_cmd* motor_cmd, Motor_cmd* last_motor_cmd) {
    // Copy the current motor command to the last motor command
    memcpy(last_motor_cmd, motor_cmd, sizeof(Motor_cmd));

    uint8_t m1_message[8] = {0}; // 8 bytes for CAN m1_message
    uint8_t m2_message[8] = {0}; // 8 bytes for CAN m2_message

    if(motor_cmd->m1_status == 1){
        m1_message[1] |= motor_cmd->m1_dir << 7;           // Direction
        m1_message[1] = motor_cmd->m1_val & 0xFF;          // Control Value Top
        m1_message[0] = (motor_cmd->m1_val >> 8) & 0xFF;   // Control Value Bottom

        uint8_t byte_3 = 0b00100000;            // bit 5 = 1 for software enable
        byte_3 |= motor_cmd->m1_control;        // Control Mode 0 = Default, 1 = Torque, 2 = Speed
        byte_3 |= (motor_cmd->m1_mode << 2);    // Motor Mode   0 = Default, 1 = Boost,  2 = Reverse, 3 = Regen

        m1_message[2] = byte_3;
    }

    if(motor_cmd->m2_status == 1){
        m2_message[1] |= motor_cmd->m2_dir << 7;           // Direction
        m2_message[1] = motor_cmd->m2_val & 0xFF;          // Control Value Top
        m2_message[0] = (motor_cmd->m2_val >> 8) & 0xFF;   // Control Value Bottom

        uint8_t byte_3 = 0b00100000;            // bit 5 = 1 for software enable
        byte_3 |= motor_cmd->m2_control;        // Control Mode 0 = Default, 1 = Torque, 2 = Speed
        byte_3 |= motor_cmd->m2_mode << 2;      // Motor Mode   0 = Default, 1 = Boost,  2 = Reverse, 3 = Regen

        m2_message[2] = byte_3;
    }


    // Create CAN message struct object for both m1 and m2 msg
    // Send through extendedID 0x550
    CANMsg m1_can_msg = {
    		0,
			0x550,
			8,
			{0}
    };
       memcpy(m1_can_msg.data, m1_message, 8);

       CANMsg m2_can_msg = {
    		   0,
			   0x550,
			   8,
			   {0}
       };
       memcpy(m2_can_msg.data, m2_message, 8);

       // Send CAN messages
       sendExtendedCANMessage(&m1_can_msg, &peripheral1);
       sendExtendedCANMessage(&m2_can_msg, &peripheral1);
       // Send CAN message here


}

/*
    * Print the Motor_cmd via UART
    *
    * @param huart: UART Handler
    * @param motor_cmd: The Motor_cmd to print
*/
void PrintMotorCommand(UART_HandleTypeDef* huart, Motor_cmd* motor_cmd) {
    char message[100] = {0};

    CUBE_PRINT(message, "Motor: 1\r\n Status: %d\r\n Control: %d\r\n Mode: %d\r\n Dir: %d\r\n Val: %d\r\n\r\n", motor_cmd->m1_status, motor_cmd->m1_control, motor_cmd->m1_mode, motor_cmd->m1_dir, motor_cmd->m1_val);
    HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), 100);

    CUBE_PRINT(message, "Motor: 2\r\n Status: %d\r\n Control: %d\r\n Mode: %d\r\n Dir: %d\r\n Val: %d\r\n\r\n", motor_cmd->m2_status, motor_cmd->m2_control, motor_cmd->m2_mode, motor_cmd->m2_dir, motor_cmd->m2_val);
    HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), 100);
}

/**
 * @brief Initialize the GPIOTask
 */
void GPIOTask::InitTask()
{
    // Make sure the task2 is not already initialized
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
    uint8_t hazardLights = driverControlState[static_cast<int>(DriverControls::EMERGENCY_HAZARD)] == IOState::LOW;

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

    uint16_t output = 0;

    uint8_t raceModeEnable = driverControlState[static_cast<int>(DriverControls::RACE_MODE_ENABLE)] == IOState::LOW;
    uint8_t lap = driverControlState[static_cast<int>(DriverControls::LAP_BUTTON)] == IOState::LOW;
    uint8_t hornSwitch = driverControlState[static_cast<int>(DriverControls::HORN_ENABLE)] == IOState::LOW;
    uint8_t motorReset = driverControlState[static_cast<int>(DriverControls::MOTOR_RESET)] == IOState::LOW;

    uint8_t parkingBrake = driverControlState[static_cast<int>(DriverControls::PARKING_BRAKE_DETECT)] == IOState::LOW;
    uint8_t mechanicalBreak = driverControlState[static_cast<int>(DriverControls::MECHANICAL_BRAKE)] == IOState::LOW;
    uint8_t zoomZoom = driverControlState[static_cast<int>(DriverControls::GREEN_LED)] == IOState::LOW;

    uint8_t forward = driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_H)] == IOState::HIGH && driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_L)] == IOState::LOW;

    uint8_t reverse = driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_H)] == IOState::HIGH && driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_L)] == IOState::HIGH;

    uint8_t neutral = driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_H)] == IOState::LOW // Assumption
                      && driverControlState[static_cast<int>(DriverControls::FORWARD_NEUTRAL_REVERSE_L)] == IOState::LOW;

    /**Forward and Reverse Encoding from Electrical Team */
    /* 10 forward
     * 11 reverse
     * 00 TODO: CHECK ASSUMPTION THIS IS NEUTRAL
     * 01 not implemented
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

    // TODO: What is zoom zoom ? Assumed to be greenLed                     Bit 9
    output |= (zoomZoom ? 1 : 0) << 9; // Bit 9

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
        // CUBE_PRINT("Driver Control State...\n");
        for (uint8_t i = 0; i < 8; i++)
        {
            switch (static_cast<IOPin>(i))
            {
            case DriverControls::FORWARD_NEUTRAL_REVERSE_H:
                CUBE_PRINT("    - P00 (Forward/Neutral/Reverse Combo High): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::P13, IOState::HIGH);
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::P13, IOState::LOW);
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::FORWARD_NEUTRAL_REVERSE_L:
                CUBE_PRINT("    - P01 (Forward/Neutral/Reverse Combo Low): %d\n", driverControlState[i]);
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
                CUBE_PRINT("    - P02 (Array Disconnect): %d\n", driverControlState[i]);
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
                CUBE_PRINT("    - P03 (Race Mode Enable): %d\n", driverControlState[i]);
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
                CUBE_PRINT("    - P04 (Headlights Enable): %d\n", driverControlState[i]);
                if (driverControlState[i] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::DISPLAY_SCREEN_ROTATE:
                CUBE_PRINT("    - P05 (Display Screen Rotate): %d\n", driverControlState[i]);
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
                CUBE_PRINT("    - P06 (Proximity Sensor Mute): %d\n", driverControlState[i]);
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
                CUBE_PRINT("    - P07 (Lap Button): %d\n", driverControlState[i]);
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
                CUBE_PRINT("    - P10 (Horn Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::LEFT_SIGNAL_ENABLE:
                CUBE_PRINT("    - P11 (Left Signal Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::LOW);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::RIGHT_SIGNAL_ENABLE:
                CUBE_PRINT("    - P12 (Right Signal Enable): %d\n", driverControlState[i-2]);
                if (driverControlState[i - 2] == IOState::HIGH)
                {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::LOW)
                {
                    powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
                }
                else if (driverControlState[i - 2] == IOState::ERROR)
                {
                }
                break;
            case DriverControls::EMERGENCY_HAZARD:
                CUBE_PRINT("    - P13 (Emergency Hazard): %d\n", driverControlState[i-2]);
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
                CUBE_PRINT("    - P14 (Motor Reset): %d\n", driverControlState[i-2]);
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
                CUBE_PRINT("    - P15 (Parking Brake Detect): %d\n", driverControlState[i-2]);
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
                CUBE_PRINT("    - P16 (Mechanical Brake): %d\n", driverControlState[i-2]);
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
                CUBE_PRINT("    - P17 (Green LED): %d\n", driverControlState[i-2]);
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

        // powerBoardExpander.SetPin(PowerBoard::BRAKE_LIGHT_SIGNAL, IOState::HIGH);
        // powerBoardExpander.SetPin(PowerBoard::DAYTIME_RUNNING_LIGHT_SIGNAL, IOState::HIGH);
        // powerBoardExpander.SetPin(PowerBoard::LEFT_TURN_LIGHT_SIGNAL, IOState::HIGH);
        // powerBoardExpander.SetPin(PowerBoard::RIGHT_TURN_LIGHT_SIGNAL, IOState::HIGH);
        // powerBoardExpander.SetPin(PowerBoard::HEADLIGHT_SIGNAL, IOState::HIGH);
        // powerBoardExpander.SetPin(PowerBoard::HORN_SIGNAL, IOState::HIGH);

        // powerBoardExpander.TogglePin(PowerBoard::ORANGE_LED);
        // powerBoardExpander.TogglePin(PowerBoard::GREEN_LED);

        // Commit changes to Power board
        powerBoardExpander.Commit();

        checkCounterTick();
        this->counterTick++;

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
    CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, DIGITAL_INPUTS));
    CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ANALOG_INPUTS));

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
