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
#define TASK_FREQUENCY_HZ 20
constexpr uint32_t TASK_DELAY = 1000 / TASK_FREQUENCY_HZ;


extern uint8_t dead_common_heartbeat;
extern uint8_t dead_motor_heartbeat;
extern uint8_t dead_array_heartbeat;
extern uint8_t dead_lv_heartbeat;
extern uint8_t dead_charge_heartbeat;

extern uint8_t hard_high_common;
extern uint8_t hard_high_motor;
extern uint8_t hard_high_array;
extern uint8_t hard_high_lv;
extern uint8_t hard_high_charge;

extern uint8_t soft_high_common;
extern uint8_t soft_high_motor;
extern uint8_t soft_high_array;
extern uint8_t soft_high_lv;
extern uint8_t soft_high_charge;

extern uint8_t close_common;
extern uint8_t close_motor;
extern uint8_t close_array;
extern uint8_t close_lv;
extern uint8_t close_charge;

extern uint8_t open_common;
extern uint8_t open_motor;
extern uint8_t open_array;
extern uint8_t open_lv;
extern uint8_t open_charge;

extern uint8_t stop_orion;
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
#if 0
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
#endif

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
    //NOTE: Currently not sending to queue 
    //CANTxTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, DIGITAL_INPUTS));
    // CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ANALOG_INPUTS));
    
    if ((this->counterTick & 0x1) == 0) { // 100 ms passed send LIGHTS_INPUT
        //CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_INPUT));


    	//common
    	if(hard_high_common) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, HARD_HIGH_COMMON));
    	}
    	else if(soft_high_common) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, SOFT_HIGH_COMMON));
    	}
#if 0
    	if(close_common) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CLOSE_COMMON));
    	}
    	else if (open_common) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, OPEN_COMMON));
    	}
    	else {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, COMMON_BOARD_STATUS));
    	}

#endif
    	//motor
    	if(hard_high_motor) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, HARD_HIGH_MOTOR));
    	}
    	else if(soft_high_motor) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, SOFT_HIGH_MOTOR));
    	}

    	if(close_motor) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CLOSE_MOTOR));
    	}
    	else if (open_motor) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, OPEN_MOTOR));
    	}
    	else {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, MOTOR_BOARD_STATUS));
    	}


    	//array
    	if(hard_high_array) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, HARD_HIGH_ARRAY));
    	}
    	else if(soft_high_array) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, SOFT_HIGH_ARRAY));
    	}

    	if(close_array) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CLOSE_ARRAY));
    	}
    	else if (open_array) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, OPEN_ARRAY));
    	}
    	else {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ARRAY_BOARD_STATUS));
    	}


    	//lv
    	if(hard_high_lv) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, HARD_HIGH_LV));
    	}
    	else if(soft_high_lv) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, SOFT_HIGH_LV));
    	}

    	if(close_lv) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CLOSE_LV));
    	}
    	else if (open_lv) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, OPEN_LV));
    	}
    	else {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LV_BOARD_STATUS));
    	}

    	//charge
    	if(hard_high_charge) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, HARD_HIGH_CHARGE));
    	}
    	else if(soft_high_charge) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, SOFT_HIGH_CHARGE));
    	}

    	if(close_charge) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CLOSE_CHARGE));
    	}
    	else if (open_charge) {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, OPEN_CHARGE));
    	}
    	else {
    		CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CHARGE_BOARD_STATUS));
    	}



//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ARRAY_BOARD_STATUS));
//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LV_BOARD_STATUS));
//        CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CHARGE_BOARD_STATUS));
    }
    if ((this->counterTick & 0x3) == 0) { // 200 ms passed send LIGHTS_INPUT and LIGHTS_STATUS_BASE
        //CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_INPUT));
        //CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LIGHTS_STATUS_BASE));

    	//if (!stop_orion) {
            CANTxTask::Inst().SendCommand(Command(DATA_COMMAND,TEMPERATURE_INFO));
            CANTxTask::Inst().SendCommand(Command(DATA_COMMAND,CELL_VOLTAGES));
    	//}

    }
    if(this->counterTick == 20){
        //CANTxTask::Inst().SendCommand(Command(DATA_COMMAND,HEARTBEAT));

    	if(!stop_orion) {
    		 CANTxTask::Inst().SendCommand(Command(DATA_COMMAND,PACK_INFO));
    	}

#if 0
        if(!dead_common_heartbeat) {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, COMMON_BOARD_HEARTBEAT));
        }
        else {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, DEAD_COMMON_HEARTBEAT));
        }
#endif

        if(!dead_motor_heartbeat) {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, MOTOR_BOARD_HEARTBEAT));
        }
        else {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, DEAD_MOTOR_HEARTBEAT));
        }

        if(!dead_array_heartbeat) {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, ARRAY_BOARD_HEARTBEAT));
        }
        else {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, DEAD_ARRAY_HEARTBEAT));
        }

        if(!dead_lv_heartbeat) {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, LV_BOARD_HEARTBEAT));
        }
        else {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, DEAD_LV_HEARTBEAT));
        }
        if(!dead_charge_heartbeat) {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, CHARGE_BOARD_HEARTBEAT));
        }
        else {
        	CANTxTask::Inst().SendCommand(Command(DATA_COMMAND, DEAD_CHARGE_HEARTBEAT));
        }
        
        this->counterTick = 0; // Reset the counter for the next cycle
    }

}
