/**
 ******************************************************************************
 * File Name          : MotorSafetyTask.cpp
 * Description        :
 ******************************************************************************
 */
#include "SystemDefines.hpp"
#include "IOExpander.hpp"
#include "MotorSafetyTask.hpp"
#include "CANTxTask.cpp"
#include <cmath>  // For pow() function

/*----------------------- Macros -----------------------*/
#define TASK_FREQUENCY 1
constexpr uint32_t TASK_DELAY = 10 / TASK_FREQUENCY;

// Queue to hold CAN messages
volatile uint8_t canMessageFlag = 0; // 0: No new message, 1: New message available
CANMessage receivedCANMessage;

/**
 * @brief Construction of Motor Control Task
 * Leave empty to adhear to singleton pattern as Inst handles construction
 */
MotorSafetyTask::MotorSafetyTask() : Task(GPIO_TASK_QUEUE_DEPTH_OBJS)
{
}

/*
    * Sends ADC values over UART
    *
    * @param huart: The UART handle
    * @param dma_adc_buf: The ADC buffer
    * @param en: enable
    *
    * @return 0 if the motor command is within safety limits, 1 if the motor command is not within safety limits
*/
void sendADCValues(UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma, uint16_t* dma_adc_buf, uint8_t enable){
	if(enable){
        uint16_t adc_vals[8] = {0};

        memcpy(adc_vals, dma_adc_buf, 8*sizeof(uint16_t));

        char msg[80] = {0};
        sprintf(msg, "9999 %d %d %d %d %d %d %d %d 8888\r\n", adc_vals[0], adc_vals[1], adc_vals[2], adc_vals[3], adc_vals[4], adc_vals[5], adc_vals[6], adc_vals[7]);

        HAL_UART_DMAStop(huart);
        HAL_UART_Transmit_DMA(huart, (uint8_t*)msg, strlen(msg));

    //     HAL_UART_Transmit_DMA(huart, (uint8_t*)"9999 ", 5);

    //     for (int i = 0; i < 8; i++){
    //        sprintf(msg, "%d: %d ", i, adc_vals[i]);
    //        HAL_UART_Transmit_DMA(huart, (uint8_t*)msg, strlen(msg));
    //     }

    //    HAL_UART_Transmit_DMA(huart, (uint8_t*)"8888\r\n", 6);
	}


	return;
}

/*
    * Checks if the ADC values are within safety limits
    *
    * @param dma_adc_buf: The ADC buffer
    *
    * @return 0 if the motor command is within safety limits, 1 if the motor command is not within safety limits
*/
double predict_dc_current(double x, double y, double x_mean, double x_std, double y_mean, double y_std, double coefficients[]) {
    // Normalize the input values
    double x_norm = (x - x_mean) / x_std;
    double y_norm = (y - y_mean) / y_std;

    // Calculate each term of the polynomial function
    double z = coefficients[0] +
               coefficients[1] * x_norm +
               coefficients[2] * y_norm +
               coefficients[3] * pow(x_norm, 2) +
               coefficients[4] * pow(y_norm, 2) +
               coefficients[5] * x_norm * y_norm +
               coefficients[6] * pow(x_norm, 3) +
               coefficients[7] * pow(y_norm, 3) +
               coefficients[8] * pow(x_norm, 2) * y_norm +
               coefficients[9] * x_norm * pow(y_norm, 2) +
               coefficients[10] * pow(x_norm, 4) +
               coefficients[11] * pow(y_norm, 4) +
               coefficients[12] * pow(x_norm, 3) * y_norm +
               coefficients[13] * pow(x_norm, 2) * pow(y_norm, 2) +
               coefficients[14] * x_norm * pow(y_norm, 3) +
               coefficients[15] * pow(x_norm, 5) +
               coefficients[16] * pow(y_norm, 5) +
               coefficients[17] * pow(x_norm, 4) * y_norm +
               coefficients[18] * pow(x_norm, 3) * pow(y_norm, 2) +
               coefficients[19] * pow(x_norm, 2) * pow(y_norm, 3) +
               coefficients[20] * x_norm * pow(y_norm, 4) +
               coefficients[21] * pow(x_norm, 6) +
               coefficients[22] * pow(y_norm, 6);

    return z;
}

uint8_t checkADCValues(const uint16_t* dma_adc_buf) {
    // Get data Torque, Speed data from CAN
	int16_t motor_rpm = 0, motor_torque = 0, inv_peak_cur = 0;

    // Get expected current, voltage, torque, speed from formula

    // Get actual current, voltage, torque, speed from ADC values
	double x_mean = 72.63720930232559;    //update this value from the dataset being stored
	double x_std = 42.52505156747794;    //update this value from the dataset being stored
	double y_mean = 469.7674418604651;  //update this value from the dataset being stored
	double y_std = 300.12391527727823; //update this value from the dataset being stored

	    // Coefficients from the model
	double coefficients[] = {
	        8.99081610e+01,  5.41331468e+01,  5.42552952e+01,  3.52967650e+00,
	        -1.39293173e+00,  3.12836964e+01, -3.00255167e-01,  1.45315850e+00,
	        1.91373226e+00,  2.22332187e+00, -1.06146310e-01,  2.61885344e+00,
	        1.89991053e-01,  1.15873486e+00,  1.74216668e+00,  2.55780027e-01,
	        1.91376837e-01, -1.32832021e-01, -6.02793556e-02,  4.50207064e-01,
	        1.49791194e-02, -6.62227062e-02, -5.02206451e-01
	    };

	    // Example inputs
	    // Get the predicted current
	double predicted_current = predict_dc_current(motor_torque, motor_rpm, x_mean, x_std, y_mean, y_std, coefficients);

	    // Print the result

    // Do necessary conversions

    // Check if the values are within safety limits

    return 0;
}

// Function to receive and handle CAN messages (ISR or main loop context)
void receiveCANMessageHandler(CANPeripheral* peripheral) {
    uint32_t messageID;
    uint8_t DLC;
    uint8_t data[8];

    // Receive CAN message from the hardware
    receiveCANMessage(0, &messageID, &DLC, data, peripheral); // Assuming channel 0

    // Fill the global CANMessage structure with the received data
    receivedCANMessage.ID = messageID;
    receivedCANMessage.DLC = DLC;
    memcpy(receivedCANMessage.data, data, DLC);

    // Set the flag to indicate a new message is available
    canMessageFlag = 1;
}


uint8_t motorSafetyTask(uint16_t* dma_adc_buf, int16_t *motor_rpm, int16_t *motor_torque, int16_t *inv_peak_cur){

	    // Attempt to receive a message from the CAN queue
	if (canMessageFlag ==1) {
		//clear the flag to say message is being processed
		canMessageFlag = 0;
		switch (receivedCANMessage.ID) {
			case (CAN_TX_ADDRESS + 0): {  //need to find can_tx_address
				int16_t control_level = (receivedCANMessage.data[0] << 8) | receivedCANMessage.data[1]; // Signed 16-bit (Bytes 1-2)
				uint8_t control_mode = (receivedCANMessage.data[2] & 0b00000011);    // Bits 0-1: Control Mode (00 = TORQUE, 01 = SPEED)
				uint8_t motor_mode = (receivedCANMessage.data[2] >> 2) & 0b00000111;  // Bits 2-4: Motor Mode (Normal, Boost, Reverse, etc.)
				uint8_t sw_enable = (receivedCANMessage.data[2] >> 5) & 0b00000001;   // Bit 5: Software Enable (0 - DISABLED, 1 - ENABLED)
				uint8_t motor_state = (receivedCANMessage.data[2] >> 6) & 0b00000001; // Bit 6: Motor State (0 - IDLE, 1 - RUN)
				uint8_t debug_mode = (receivedCANMessage.data[2] >> 7) & 0b00000001;  // Bit 7: Debug Mode (0 - Normal, 1 - Debug)

				// Parse the remaining bytes for motor torque, rpm, and temperature
				*motor_torque = (receivedCANMessage.data[3] << 8) | receivedCANMessage.data[4]; // Signed 16-bit (Bytes 4-5)
				*motor_rpm = (receivedCANMessage.data[5] << 8) | receivedCANMessage.data[6];    // Signed 16-bit (Bytes 6-7)
				int8_t motor_temp = receivedCANMessage.data[7];                     // Signed 8-bit (Byte 8)

				// Output parsed values for the first message
				printf("Control Level: %d\n", control_level);
				printf("Control Mode: %d\n", control_mode);
				printf("Motor Mode: %d\n", motor_mode);
				printf("Software Enable: %d\n", sw_enable);
				printf("Motor State: %d\n", motor_state);
				printf("Debug Mode: %d\n", debug_mode);
				//printf("Motor Torque: %d Nm\n", motor_torque);
				//printf("Motor RPM: %d (0.1 RPM resolution)\n", motor_rpm);
				printf("Motor Temp: %d °C\n", motor_temp);
				break;
			}

			case (CAN_TX_ADDRESS + 1):{
				// Process motor power and position message
				*inv_peak_cur = (receivedCANMessage.data[0] << 8) | receivedCANMessage.data[1]; // Signed 16-bit
				int16_t motor_power = (receivedCANMessage.data[2] << 8) | receivedCANMessage.data[3];  // Signed 16-bit
				uint16_t abs_position = (receivedCANMessage.data[4] << 8) | receivedCANMessage.data[5]; // Unsigned 16-bit

				//printf("Inverter Peak Current: %d A\n", inv_peak_cur);
				printf("Motor Power: %d W\n", motor_power);
				printf("Motor Position: %u (1/2 degrees)\n", abs_position);
				break;
			}
			// More cases for other messages



		   case (CAN_TX_ADDRESS + 2): { // Assuming this is the message ID for the warning code message
				// Combine the bytes into a 64-bit unsigned integer (warning_code)
				uint64_t warning_code = 0;
				warning_code |= ((uint64_t)receivedCANMessage.data[0] << 56);
				warning_code |= ((uint64_t)receivedCANMessage.data[1] << 48);
				warning_code |= ((uint64_t)receivedCANMessage.data[2] << 40);
				warning_code |= ((uint64_t)receivedCANMessage.data[3] << 32);
				warning_code |= ((uint64_t)receivedCANMessage.data[4] << 24);
				warning_code |= ((uint64_t)receivedCANMessage.data[5] << 16);
				warning_code |= ((uint64_t)receivedCANMessage.data[6] << 8);
				warning_code |= (uint64_t)receivedCANMessage.data[7];

				// Check if the warning code is non-zero
				if (warning_code != 0) {
					// Print a generic error or warning message (you could expand this with specific details later)
					printf("Warning/Error Detected: Warning Code = %llu\n", warning_code);
				}
				break;
		   }
		   case (CAN_TX_ADDRESS + 3): { // Assuming this is the message ID for the error code message
				   // Combine the bytes into a 64-bit unsigned integer (error_code)
				   uint64_t error_code = 0;
				   error_code |= ((uint64_t)receivedCANMessage.data[0] << 56);
				   error_code |= ((uint64_t)receivedCANMessage.data[1] << 48);
				   error_code |= ((uint64_t)receivedCANMessage.data[2] << 40);
				   error_code |= ((uint64_t)receivedCANMessage.data[3] << 32);
				   error_code |= ((uint64_t)receivedCANMessage.data[4] << 24);
				   error_code |= ((uint64_t)receivedCANMessage.data[5] << 16);
				   error_code |= ((uint64_t)receivedCANMessage.data[6] << 8);
				   error_code |= (uint64_t)receivedCANMessage.data[7];

				   // Handle the Initialization error (specific error code = 0)

				   printf("Error Detected: Error Code = %llu\n", error_code);

				   break;
		   }
		   default: {
					// Handle unknown or unexpected message ID
					//printf("Unknown message ID: %x\n", receivedCANMessage.ID);
					break;
		  }

		}
	}
    // recv CAN

    // get expected val from formula

    // parse data

    // compare values with error

    // return error or not
	return 0;
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
